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
#ਅਨ੍ਤਰ੍ਭਾਵ "btreeInt.h"

/*
** Suppress false-positive compiler warnings from GCC.  Warnings are
** re-enabled at the bottom of this source file.
*/
#ਜੇ defined(__GNUC__) && __GNUC__>=11
# pragma GCC diagnostic ਧੱਕੋ
# pragma GCC diagnostic ignored "-Wstringop-overread"
# pragma GCC diagnostic ignored "-Wstringop-overflow"
#ਜੇ_ਅੰਤ

/*
** The header string that appears at the beginning of every
** SQLite database.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ z_Magic_ਸਿਰਲੇਖ[] = SQLITE_FILE_ਸਿਰਲੇਖ;

/*
** Set this global variable to 1 to enable tracing using the TRACE
** macro.
*/
#ਜੇ 0
ਪੂਰਨ sqlite3BtreeTrace=1;  /* True to enable tracing */
# define TRACE(X)  ਜੇ(sqlite3BtreeTrace){ਲਿਖੋ X;fflush(stdout);}
#ਨਹੀਂਤਾਂ
# define TRACE(X)
#ਜੇ_ਅੰਤ

/*
** Extract a 2-byte big-endian integer from an array of unsigned bytes.
** But if the value is zero, make it 65536.
**
** This routine is used to extract the "offset to cell content area" value
** from the header of a btree page.  If the page size is 65536 and the page
** is empty, the offset should be 65536, but the 2-byte value stores zero.
** This routine makes the necessary adjustment to 65536.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ get2byteNotZero(X)  (((((ਪੂਰਨ)get2byte(X))-1)&0xffff)+1)

/*
** Values passed as the 5th argument to allocateBtreePage()
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTALLOC_ANY   0           /* Allocate any page */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTALLOC_EXACT 1           /* Allocate exact page if possible */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTALLOC_LE    2           /* Allocate any page <= the parameter */

/*
** Macro IfNotOmitAV(x) returns (x) if SQLITE_OMIT_AUTOVACUUM is not
** defined, or 0 if it is. For example:
**
**   bIncrVacuum = IfNotOmitAV(pBtShared->incrVacuum);
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IfNotOmitAV(ਵਿਅੰਜਕ_2) (ਵਿਅੰਜਕ_2)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IfNotOmitAV(ਵਿਅੰਜਕ_2) 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
/*
** A list of BtShared objects that are eligible for participation
** in shared cache.  This variable has file scope during normal builds,
** but the test harness needs to access it so we make it global for
** test builds.
**
** Access to this variable is protected by SQLITE_MUTEX_STATIC_MAIN.
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
BtShared *SQLITE_WSD sqlite3SharedCacheList = 0;
#ਨਹੀਂਤਾਂ
ਸਥਿਰ BtShared *SQLITE_WSD sqlite3SharedCacheList = 0;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_SHARED_CACHE */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
/*
** Enable or disable the shared pager and schema features.
**
** This routine has no effect on existing database connections.
** The shared cache setting effects only future calls to
** sqlite3_open(), sqlite3_open16(), or sqlite3_open_v2().
*/
ਪੂਰਨ sqlite3_enable_shared_cache(ਪੂਰਨ enable){
  sqlite3GlobalConfig.sharedCacheEnabled = enable;
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ



#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
  /*
  ** The functions querySharedCacheTableLock(), setSharedCacheTableLock(),
  ** and clearAllSharedCacheTableLocks()
  ** manipulate entries in the BtShared.pLock linked list used to store
  ** shared-cache table level locks. If the library is compiled with the
  ** shared-cache feature disabled, then there is only ever one user
  ** of each BtShared structure and so this locking is not necessary.
  ** So define the lock related functions as no-ops.
  */
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ query_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(a,b,c) SQLITE_OK
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(a,b,c) SQLITE_OK
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ clear_All_Shared_Cache_ਸਾਰਣੀ_Locks(a)
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ downgrade_All_Shared_Cache_ਸਾਰਣੀ_Locks(a)
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ has_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(a,b,c,d) 1
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ hasReadConflicts(a, b) 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** Return and reset the seek counter for a Btree object.
*/
sqlite3_uint64 sqlite3_Btree_Seek_ਗਿਣਤੀ(Btree *pBt){
  u64 n =  pBt->nSeek;
  pBt->nSeek = 0;
  ਵਾਪਸ n;
}
#ਜੇ_ਅੰਤ

/*
** Implementation of the SQLITE_CORRUPT_PAGE() macro. Takes a single
** (MemPage*) as an argument. The (MemPage*) must not be NULL.
**
** If SQLITE_DEBUG is not defined, then this macro is equivalent to
** SQLITE_CORRUPT_BKPT. Or, if SQLITE_DEBUG is set, then the log message
** normally produced as a side-effect of SQLITE_CORRUPT_BKPT is augmented
** with the page number and filename associated with the (MemPage*).
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਪੂਰਨ corrupt_ਪੰਨਾ_Error(ਪੂਰਨ lineno, Mem_ਪੰਨਾ *p){
  ਅੱਖਰ *z_ਸੁਨੇਹਾ;
  sqlite3BeginBenignMalloc();
  z_ਸੁਨੇਹਾ = sqlite3_mprintf("database corruption page %u of %s",
             p->pgno, sqlite3PagerFilename(p->pBt->pPager, 0)
  );
  sqlite3_ਸਿਰਾ_Benign_Malloc();
  ਜੇ( z_ਸੁਨੇਹਾ ){
    sqlite3ReportError(SQLITE_CORRUPT, lineno, z_ਸੁਨੇਹਾ);
  }
  sqlite3_free(z_ਸੁਨੇਹਾ);
  ਵਾਪਸ SQLITE_CORRUPT_BKPT;
}
# define SQLITE_CORRUPT_ਪੰਨਾ(p_Mem_ਪੰਨਾ) corrupt_ਪੰਨਾ_Error(__LINE__, p_Mem_ਪੰਨਾ)
#ਨਹੀਂਤਾਂ
# define SQLITE_CORRUPT_ਪੰਨਾ(p_Mem_ਪੰਨਾ) SQLITE_CORRUPT_PGNO(p_Mem_ਪੰਨਾ->pgno)
#ਜੇ_ਅੰਤ

/* Default value for SHARED_LOCK_TRACE macro if shared-cache is disabled
** or if the lock tracking is disabled.  This is always the value for
** release builds.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHARED_ਤਾਲਾ_TRACE(X,ਸੁਨੇਹਾ,TAB,TYPE)  /*no-op*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE

#ਜੇ 0
/*  ^----  Change to 1 and recompile to enable shared-lock tracing
**         for debugging purposes.
**
** Print all shared-cache locks on a BtShared.  Debugging use only.
*/
ਸਥਿਰ ਖਾਲੀ shared_ਤਾਲਾ_Trace(
  BtShared *pBt,
  ਅਚਲ ਅੱਖਰ *z_ਸੁਨੇਹਾ,
  ਪੂਰਨ i_ਜੜ੍ਹ,
  ਪੂਰਨ e_ਤਾਲਾ_Type
){
  Bt_ਤਾਲਾ *p_ਤਾਲਾ;
  ਜੇ( i_ਜੜ੍ਹ>0 ){
    ਲਿਖੋ("%s-%p %u%s:", z_ਸੁਨੇਹਾ, pBt, i_ਜੜ੍ਹ, e_ਤਾਲਾ_Type==READ_ਤਾਲਾ?"R":"W");
  }ਨਹੀਂਤਾਂ{
    ਲਿਖੋ("%s-%p:", z_ਸੁਨੇਹਾ, pBt);
  }
  ਲਈ(p_ਤਾਲਾ=pBt->p_ਤਾਲਾ; p_ਤਾਲਾ; p_ਤਾਲਾ=p_ਤਾਲਾ->p_ਅਗਲਾ){
    ਲਿਖੋ(" %p/%u%s", p_ਤਾਲਾ->pBtree, p_ਤਾਲਾ->i_ਸਾਰਣੀ,
           p_ਤਾਲਾ->e_ਤਾਲਾ==READ_ਤਾਲਾ ? "R" : "W");
    ਜਦੋਂ( p_ਤਾਲਾ->p_ਅਗਲਾ && p_ਤਾਲਾ->pBtree==p_ਤਾਲਾ->p_ਅਗਲਾ->pBtree ){
      p_ਤਾਲਾ = p_ਤਾਲਾ->p_ਅਗਲਾ;
      ਲਿਖੋ(",%u%s", p_ਤਾਲਾ->i_ਸਾਰਣੀ, p_ਤਾਲਾ->e_ਤਾਲਾ==READ_ਤਾਲਾ ? "R" : "W");
    }
  }
  ਲਿਖੋ("\n");
  fflush(stdout);
}
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ SHARED_ਤਾਲਾ_TRACE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SHARED_ਤਾਲਾ_TRACE(X,ਸੁਨੇਹਾ,TAB,TYPE)  shared_ਤਾਲਾ_Trace(X,ਸੁਨੇਹਾ,TAB,TYPE)
#ਜੇ_ਅੰਤ /* Shared-lock tracing */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
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
ਸਥਿਰ ਪੂਰਨ has_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(
  Btree *pBtree,         /* Handle that must hold lock */
  Pgno i_ਜੜ੍ਹ,            /* Root page of b-tree */
  ਪੂਰਨ is_ਸੂਚਕਾਂਕ,           /* True if iRoot is the root of an index b-tree */
  ਪੂਰਨ e_ਤਾਲਾ_Type          /* Required lock type (READ_LOCK or WRITE_LOCK) */
){
  Schema *pSchema = (Schema *)pBtree->pBt->pSchema;
  Pgno iTab = 0;
  Bt_ਤਾਲਾ *p_ਤਾਲਾ;

  /* If this database is not shareable, or if the client is reading
  ** and has the read-uncommitted flag set, then no lock is required.
  ** Return true immediately.
  */
  ਜੇ( (pBtree->sharable==0)
   || (e_ਤਾਲਾ_Type==READ_ਤਾਲਾ && (pBtree->db->flags & SQLITE_ReadUncommit))
  ){
    ਵਾਪਸ 1;
  }

  /* If the client is reading  or writing an index and the schema is
  ** not loaded, then it is too difficult to actually check to see if
  ** the correct locks are held.  So do not bother - just return true.
  ** This case does not come up very often anyhow.
  */
  ਜੇ( is_ਸੂਚਕਾਂਕ && (!pSchema || (pSchema->schemaFlags&DB_SchemaLoaded)==0) ){
    ਵਾਪਸ 1;
  }

  /* Figure out the root-page that the lock should be held on. For table
  ** b-trees, this is just the root page of the b-tree being read or
  ** written. For index b-trees, it is the root page of the associated
  ** table.  */
  ਜੇ( is_ਸੂਚਕਾਂਕ ){
    HashElem *p;
    ਪੂਰਨ b_ਵੇਖਿਆ = 0;
    ਲਈ(p=sqlite_Hash_ਪਹਿਲਾ(&pSchema->idxHash); p; p=sqlite_Hash_ਅਗਲਾ(p)){
      ਸੂਚਕਾਂਕ *pIdx = (ਸੂਚਕਾਂਕ *)sqlite_Hash_ਅੰਕੜੇ(p);
      ਜੇ( pIdx->tnum==i_ਜੜ੍ਹ ){
        ਜੇ( b_ਵੇਖਿਆ ){
          /* Two or more indexes share the same root page.  There must
          ** be imposter tables.  So just return true.  The assert is not
          ** useful in that case. */
          ਵਾਪਸ 1;
        }
        iTab = pIdx->p_ਸਾਰਣੀ->tnum;
        b_ਵੇਖਿਆ = 1;
      }
    }
  }ਨਹੀਂਤਾਂ{
    iTab = i_ਜੜ੍ਹ;
  }

  SHARED_ਤਾਲਾ_TRACE(pBtree->pBt,"hasLock",i_ਜੜ੍ਹ,e_ਤਾਲਾ_Type);

  /* Search for the required lock. Either a write-lock on root-page iTab, a
  ** write-lock on the schema table, or (if the client is reading) a
  ** read-lock on iTab will suffice. Return 1 if any of these are found.  */
  ਲਈ(p_ਤਾਲਾ=pBtree->pBt->p_ਤਾਲਾ; p_ਤਾਲਾ; p_ਤਾਲਾ=p_ਤਾਲਾ->p_ਅਗਲਾ){
    ਜੇ( p_ਤਾਲਾ->pBtree==pBtree
     && (p_ਤਾਲਾ->i_ਸਾਰਣੀ==iTab || (p_ਤਾਲਾ->e_ਤਾਲਾ==WRITE_ਤਾਲਾ && p_ਤਾਲਾ->i_ਸਾਰਣੀ==1))
     && p_ਤਾਲਾ->e_ਤਾਲਾ>=e_ਤਾਲਾ_Type
    ){
      ਵਾਪਸ 1;
    }
  }

  /* Failed to find the required lock. */
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
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
ਸਥਿਰ ਪੂਰਨ hasReadConflicts(Btree *pBtree, Pgno i_ਜੜ੍ਹ){
  BtCursor *p;
  ਲਈ(p=pBtree->pBt->pCursor; p; p=p->p_ਅਗਲਾ){
    ਜੇ( p->pgno_ਜੜ੍ਹ==i_ਜੜ੍ਹ
     && p->pBtree!=pBtree
     && 0==(p->pBtree->db->flags & SQLITE_ReadUncommit)
    ){
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ    /* #ifdef SQLITE_DEBUG */

/*
** Query to see if Btree handle p may obtain a lock of type eLock
** (READ_LOCK or WRITE_LOCK) on the table with root-page iTab. Return
** SQLITE_OK if the lock may be obtained (by calling
** setSharedCacheTableLock()), or SQLITE_LOCKED if not.
*/
ਸਥਿਰ ਪੂਰਨ query_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(Btree *p, Pgno iTab, u8 e_ਤਾਲਾ){
  BtShared *pBt = p->pBt;
  Bt_ਤਾਲਾ *pIter;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsMutex(p) );
  ਪ੍ਰਤਿਜ੍ਞਾ( e_ਤਾਲਾ==READ_ਤਾਲਾ || e_ਤਾਲਾ==WRITE_ਤਾਲਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !(p->db->flags&SQLITE_ReadUncommit)||e_ਤਾਲਾ==WRITE_ਤਾਲਾ||iTab==1 );
 
  /* If requesting a write-lock, then the Btree must have an open write
  ** transaction on this file. And, obviously, for this to be so there
  ** must be an open write transaction on the file itself.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( e_ਤਾਲਾ==READ_ਤਾਲਾ || (p==pBt->pWriter && p->ਵਿੱਚ_Trans==TRANS_WRITE) );
  ਪ੍ਰਤਿਜ੍ਞਾ( e_ਤਾਲਾ==READ_ਤਾਲਾ || pBt->ਵਿੱਚ_Transaction==TRANS_WRITE );
 
  /* This routine is a no-op if the shared-cache is not enabled */
  ਜੇ( !p->sharable ){
    ਵਾਪਸ SQLITE_OK;
  }

  /* If some other connection is holding an exclusive lock, the
  ** requested lock may not be obtained.
  */
  ਜੇ( pBt->pWriter!=p && (pBt->btsFlags & BTS_EXCLUSIVE)!=0 ){
    sqlite3ConnectionBlocked(p->db, pBt->pWriter->db);
    ਵਾਪਸ SQLITE_LOCKED_SHAREDCACHE;
  }

  ਲਈ(pIter=pBt->p_ਤਾਲਾ; pIter; pIter=pIter->p_ਅਗਲਾ){
    /* The condition (pIter->eLock!=eLock) in the following if(...)
    ** statement is a simplification of:
    **
    **   (eLock==WRITE_LOCK || pIter->eLock==WRITE_LOCK)
    **
    ** since we know that if eLock==WRITE_LOCK, then no other connection
    ** may hold a WRITE_LOCK on any table in this file (since there can
    ** only be a single writer).
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( pIter->e_ਤਾਲਾ==READ_ਤਾਲਾ || pIter->e_ਤਾਲਾ==WRITE_ਤਾਲਾ );
    ਪ੍ਰਤਿਜ੍ਞਾ( e_ਤਾਲਾ==READ_ਤਾਲਾ || pIter->pBtree==p || pIter->e_ਤਾਲਾ==READ_ਤਾਲਾ);
    ਜੇ( pIter->pBtree!=p && pIter->i_ਸਾਰਣੀ==iTab && pIter->e_ਤਾਲਾ!=e_ਤਾਲਾ ){
      sqlite3ConnectionBlocked(p->db, pIter->pBtree->db);
      ਜੇ( e_ਤਾਲਾ==WRITE_ਤਾਲਾ ){
        ਪ੍ਰਤਿਜ੍ਞਾ( p==pBt->pWriter );
        pBt->btsFlags |= BTS_PENDING;
      }
      ਵਾਪਸ SQLITE_LOCKED_SHAREDCACHE;
    }
  }
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* !SQLITE_OMIT_SHARED_CACHE */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
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
ਸਥਿਰ ਪੂਰਨ ਸੈੱਟ_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(Btree *p, Pgno i_ਸਾਰਣੀ, u8 e_ਤਾਲਾ){
  BtShared *pBt = p->pBt;
  Bt_ਤਾਲਾ *p_ਤਾਲਾ = 0;
  Bt_ਤਾਲਾ *pIter;

  SHARED_ਤਾਲਾ_TRACE(pBt,"setLock", i_ਸਾਰਣੀ, e_ਤਾਲਾ);

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsMutex(p) );
  ਪ੍ਰਤਿਜ੍ਞਾ( e_ਤਾਲਾ==READ_ਤਾਲਾ || e_ਤਾਲਾ==WRITE_ਤਾਲਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->db!=0 );

  /* A connection with the read-uncommitted flag set will never try to
  ** obtain a read-lock using this function. The only read-lock obtained
  ** by a connection in read-uncommitted mode is on the sqlite_schema
  ** table, and that lock is obtained in BtreeBeginTrans().  */
  ਪ੍ਰਤਿਜ੍ਞਾ( 0==(p->db->flags&SQLITE_ReadUncommit) || e_ਤਾਲਾ==WRITE_ਤਾਲਾ );

  /* This function should only be called on a sharable b-tree after it
  ** has been determined that no other b-tree holds a conflicting lock.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( p->sharable );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_OK==query_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(p, i_ਸਾਰਣੀ, e_ਤਾਲਾ) );

  /* First search the list for an existing lock on this table. */
  ਲਈ(pIter=pBt->p_ਤਾਲਾ; pIter; pIter=pIter->p_ਅਗਲਾ){
    ਜੇ( pIter->i_ਸਾਰਣੀ==i_ਸਾਰਣੀ && pIter->pBtree==p ){
      p_ਤਾਲਾ = pIter;
      ਰੋਕੋ;
    }
  }

  /* If the above search did not find a BtLock struct associating Btree p
  ** with table iTable, allocate one and link it into the list.
  */
  ਜੇ( !p_ਤਾਲਾ ){
    p_ਤਾਲਾ = (Bt_ਤਾਲਾ *)sqlite3MallocZero(ਆਕਾਰ(Bt_ਤਾਲਾ));
    ਜੇ( !p_ਤਾਲਾ ){
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
    p_ਤਾਲਾ->i_ਸਾਰਣੀ = i_ਸਾਰਣੀ;
    p_ਤਾਲਾ->pBtree = p;
    p_ਤਾਲਾ->p_ਅਗਲਾ = pBt->p_ਤਾਲਾ;
    pBt->p_ਤਾਲਾ = p_ਤਾਲਾ;
  }

  /* Set the BtLock.eLock variable to the maximum of the current lock
  ** and the requested lock. This means if a write-lock was already held
  ** and a read-lock requested, we don't incorrectly downgrade the lock.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( WRITE_ਤਾਲਾ>READ_ਤਾਲਾ );
  ਜੇ( e_ਤਾਲਾ>p_ਤਾਲਾ->e_ਤਾਲਾ ){
    p_ਤਾਲਾ->e_ਤਾਲਾ = e_ਤਾਲਾ;
  }

  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* !SQLITE_OMIT_SHARED_CACHE */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
/*
** Release all the table locks (locks obtained via calls to
** the setSharedCacheTableLock() procedure) held by Btree object p.
**
** This function assumes that Btree p has an open read or write
** transaction. If it does not, then the BTS_PENDING flag
** may be incorrectly cleared.
*/
ਸਥਿਰ ਖਾਲੀ clear_All_Shared_Cache_ਸਾਰਣੀ_Locks(Btree *p){
  BtShared *pBt = p->pBt;
  Bt_ਤਾਲਾ **ppIter = &pBt->p_ਤਾਲਾ;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsMutex(p) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->sharable || 0==*ppIter );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->ਵਿੱਚ_Trans>0 );

  SHARED_ਤਾਲਾ_TRACE(pBt, "clearAllLocks", 0, 0);

  ਜਦੋਂ( *ppIter ){
    Bt_ਤਾਲਾ *p_ਤਾਲਾ = *ppIter;
    ਪ੍ਰਤਿਜ੍ਞਾ( (pBt->btsFlags & BTS_EXCLUSIVE)==0 || pBt->pWriter==p_ਤਾਲਾ->pBtree );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਤਾਲਾ->pBtree->ਵਿੱਚ_Trans>=p_ਤਾਲਾ->e_ਤਾਲਾ );
    ਜੇ( p_ਤਾਲਾ->pBtree==p ){
      *ppIter = p_ਤਾਲਾ->p_ਅਗਲਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਤਾਲਾ->i_ਸਾਰਣੀ!=1 || p_ਤਾਲਾ==&p->ਤਾਲਾ );
      ਜੇ( p_ਤਾਲਾ->i_ਸਾਰਣੀ!=1 ){
        sqlite3_free(p_ਤਾਲਾ);
      }
    }ਨਹੀਂਤਾਂ{
      ppIter = &p_ਤਾਲਾ->p_ਅਗਲਾ;
    }
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( (pBt->btsFlags & BTS_PENDING)==0 || pBt->pWriter );
  ਜੇ( pBt->pWriter==p ){
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(BTS_EXCLUSIVE|BTS_PENDING);
  }ਨਹੀਂਤਾਂ ਜੇ( pBt->nTransaction==2 ){
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
ਸਥਿਰ ਖਾਲੀ downgrade_All_Shared_Cache_ਸਾਰਣੀ_Locks(Btree *p){
  BtShared *pBt = p->pBt;

  SHARED_ਤਾਲਾ_TRACE(pBt, "downgradeLocks", 0, 0);

  ਜੇ( pBt->pWriter==p ){
    Bt_ਤਾਲਾ *p_ਤਾਲਾ;
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(BTS_EXCLUSIVE|BTS_PENDING);
    ਲਈ(p_ਤਾਲਾ=pBt->p_ਤਾਲਾ; p_ਤਾਲਾ; p_ਤਾਲਾ=p_ਤਾਲਾ->p_ਅਗਲਾ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਤਾਲਾ->e_ਤਾਲਾ==READ_ਤਾਲਾ || p_ਤਾਲਾ->pBtree==p );
      p_ਤਾਲਾ->e_ਤਾਲਾ = READ_ਤਾਲਾ;
    }
  }
}

#ਜੇ_ਅੰਤ /* SQLITE_OMIT_SHARED_CACHE */

ਸਥਿਰ ਖਾਲੀ release_ਪੰਨਾ(Mem_ਪੰਨਾ *p_ਪੰਨਾ);         /* Forward reference */
ਸਥਿਰ ਖਾਲੀ release_ਪੰਨਾ_One(Mem_ਪੰਨਾ *p_ਪੰਨਾ);      /* Forward reference */
ਸਥਿਰ ਖਾਲੀ release_ਪੰਨਾ_Not_Null(Mem_ਪੰਨਾ *p_ਪੰਨਾ);  /* Forward reference */

/*
***** This routine is used inside of assert() only ****
**
** Verify that the cursor holds the mutex on its BtShared
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਸਥਿਰ ਪੂਰਨ cursorHoldsMutex(BtCursor *p){
  ਵਾਪਸ sqlite3_mutex_held(p->pBt->mutex);
}

/* Verify that the cursor and the BtShared agree about what is the current
** database connetion. This is important in shared-cache mode. If the database
** connection pointers get out-of-sync, it is possible for routines like
** btreeInitPage() to reference an stale connection pointer that references a
** a connection that has already closed.  This routine is used inside assert()
** statements only and for the purpose of double-checking that the btree code
** does keep the database connection pointers up-to-date.
*/
ਸਥਿਰ ਪੂਰਨ cursorOwnsBtShared(BtCursor *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorHoldsMutex(p) );
  ਵਾਪਸ (p->pBtree->db==p->pBt->db);
}
#ਜੇ_ਅੰਤ

/*
** Invalidate the overflow cache of the cursor passed as the first argument.
** on the shared btree structure pBt.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ invalidate_ਉਛਾਲ_Cache(pCur) (pCur->curFlags &= ~BTCF_ValidOvfl)

/*
** Invalidate the overflow page-list cache for all cursors opened
** on the shared btree structure pBt.
*/
ਸਥਿਰ ਖਾਲੀ invalidate_All_ਉਛਾਲ_Cache(BtShared *pBt){
  BtCursor *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਲਈ(p=pBt->pCursor; p; p=p->p_ਅਗਲਾ){
    invalidate_ਉਛਾਲ_Cache(p);
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
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
ਸਥਿਰ ਖਾਲੀ invalidateIncrblobCursors(
  Btree *pBtree,          /* The database file to check */
  Pgno pgno_ਜੜ੍ਹ,          /* The table that might be changing */
  i64 i_ਪੰਕਤੀ,               /* The rowid that might be changing */
  ਪੂਰਨ is_Clear_ਸਾਰਣੀ        /* True if all rows are being deleted */
){
  BtCursor *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( pBtree->hasIncrblobCur );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsMutex(pBtree) );
  pBtree->hasIncrblobCur = 0;
  ਲਈ(p=pBtree->pBt->pCursor; p; p=p->p_ਅਗਲਾ){
    ਜੇ( (p->curFlags & BTCF_Incrblob)!=0 ){
      pBtree->hasIncrblobCur = 1;
      ਜੇ( p->pgno_ਜੜ੍ਹ==pgno_ਜੜ੍ਹ && (is_Clear_ਸਾਰਣੀ || p->info.n_ਕੁੰਜੀ==i_ਪੰਕਤੀ) ){
        p->eState = CURSOR_INVALID;
      }
    }
  }
}

#ਨਹੀਂਤਾਂ
  /* Stub function when INCRBLOB is omitted */
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ invalidateIncrblobCursors(w,x,y,z)
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_INCRBLOB */

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
ਸਥਿਰ ਪੂਰਨ btree_ਸੈੱਟ_Has_Content(BtShared *pBt, Pgno pgno){
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( !pBt->pHasContent ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pgno<=pBt->n_ਪੰਨਾ );
    pBt->pHasContent = sqlite3BitvecCreate(pBt->n_ਪੰਨਾ);
    ਜੇ( !pBt->pHasContent ){
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  ਜੇ( rc==SQLITE_OK && pgno<=sqlite3_Bitvec_ਮਾਪ(pBt->pHasContent) ){
    rc = sqlite3_Bitvec_ਸੈੱਟ(pBt->pHasContent, pgno);
  }
  ਵਾਪਸ rc;
}

/*
** Query the BtShared.pHasContent vector.
**
** This function is called when a free-list leaf page is removed from the
** free-list for reuse. It returns false if it is safe to retrieve the
** page from the pager layer with the 'no-content' flag set. True otherwise.
*/
ਸਥਿਰ ਪੂਰਨ btreeGetHasContent(BtShared *pBt, Pgno pgno){
  Bitvec *p = pBt->pHasContent;
  ਵਾਪਸ p && (pgno>sqlite3_Bitvec_ਮਾਪ(p) || sqlite3BitvecTestNotNull(p, pgno));
}

/*
** Clear (destroy) the BtShared.pHasContent bitvec. This should be
** invoked at the conclusion of each write-transaction.
*/
ਸਥਿਰ ਖਾਲੀ btreeClearHasContent(BtShared *pBt){
  sqlite3BitvecDestroy(pBt->pHasContent);
  pBt->pHasContent = 0;
}

/*
** Release all of the apPage[] pages for a cursor.
*/
ਸਥਿਰ ਖਾਲੀ btree_Release_All_Cursor_ਪੰਨੇ(BtCursor *pCur){
  ਪੂਰਨ i;
  ਜੇ( pCur->i_ਪੰਨਾ>=0 ){
    ਲਈ(i=0; i<pCur->i_ਪੰਨਾ; i++){
      release_ਪੰਨਾ_Not_Null(pCur->ap_ਪੰਨਾ[i]);
    }
    release_ਪੰਨਾ_Not_Null(pCur->p_ਪੰਨਾ);
    pCur->i_ਪੰਨਾ = -1;
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
ਸਥਿਰ ਪੂਰਨ save_Cursor_ਕੁੰਜੀ(BtCursor *pCur){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪ੍ਰਤਿਜ੍ਞਾ( CURSOR_VALID==pCur->eState );
  ਪ੍ਰਤਿਜ੍ਞਾ( 0==pCur->p_ਕੁੰਜੀ );
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorHoldsMutex(pCur) );

  ਜੇ( pCur->cur_Int_ਕੁੰਜੀ ){
    /* Only the rowid is required for a table btree */
    pCur->n_ਕੁੰਜੀ = sqlite3_Btree_Integer_ਕੁੰਜੀ(pCur);
  }ਨਹੀਂਤਾਂ{
    /* For an index btree, save the complete key content. It is possible
    ** that the current key is corrupt. In that case, it is possible that
    ** the sqlite3VdbeRecordUnpack() function may overread the buffer by
    ** up to the size of 1 varint plus 1 8-byte value when the cursor
    ** position is restored. Hence the 17 bytes of padding allocated
    ** below. */
    ਖਾਲੀ *p_ਕੁੰਜੀ;
    pCur->n_ਕੁੰਜੀ = sqlite3_Btree_Payload_ਮਾਪ(pCur);
    p_ਕੁੰਜੀ = sqlite3Malloc( ((i64)pCur->n_ਕੁੰਜੀ) + 9 + 8 );
    ਜੇ( p_ਕੁੰਜੀ ){
      rc = sqlite3BtreePayload(pCur, 0, (ਪੂਰਨ)pCur->n_ਕੁੰਜੀ, p_ਕੁੰਜੀ);
      ਜੇ( rc==SQLITE_OK ){
        memset(((u8*)p_ਕੁੰਜੀ)+pCur->n_ਕੁੰਜੀ, 0, 9+8);
        pCur->p_ਕੁੰਜੀ = p_ਕੁੰਜੀ;
      }ਨਹੀਂਤਾਂ{
        sqlite3_free(p_ਕੁੰਜੀ);
      }
    }ਨਹੀਂਤਾਂ{
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( !pCur->cur_Int_ਕੁੰਜੀ || !pCur->p_ਕੁੰਜੀ );
  ਵਾਪਸ rc;
}

/*
** Save the current cursor position in the variables BtCursor.nKey
** and BtCursor.pKey. The cursor's state is set to CURSOR_REQUIRESEEK.
**
** The caller must ensure that the cursor is valid (has eState==CURSOR_VALID)
** prior to calling this routine. 
*/
ਸਥਿਰ ਪੂਰਨ saveCursorPosition(BtCursor *pCur){
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( CURSOR_VALID==pCur->eState || CURSOR_SKIPNEXT==pCur->eState );
  ਪ੍ਰਤਿਜ੍ਞਾ( 0==pCur->p_ਕੁੰਜੀ );
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorHoldsMutex(pCur) );

  ਜੇ( pCur->curFlags & BTCF_Pinned ){
    ਵਾਪਸ SQLITE_CONSTRAINT_PINNED;
  }
  ਜੇ( pCur->eState==CURSOR_SKIPNEXT ){
    pCur->eState = CURSOR_VALID;
  }ਨਹੀਂਤਾਂ{
    pCur->skip_ਅਗਲਾ = 0;
  }

  rc = save_Cursor_ਕੁੰਜੀ(pCur);
  ਜੇ( rc==SQLITE_OK ){
    btree_Release_All_Cursor_ਪੰਨੇ(pCur);
    pCur->eState = CURSOR_REQUIRESEEK;
  }

  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl|BTCF_At_ਆਖਰੀ);
  ਵਾਪਸ rc;
}

/* Forward reference */
ਸਥਿਰ ਪੂਰਨ SQLITE_NOINLINE saveCursorsOnList(BtCursor*,Pgno,BtCursor*);

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
ਸਥਿਰ ਪੂਰਨ saveAllCursors(BtShared *pBt, Pgno i_ਜੜ੍ਹ, BtCursor *pExcept){
  BtCursor *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pExcept==0 || pExcept->pBt==pBt );
  ਲਈ(p=pBt->pCursor; p; p=p->p_ਅਗਲਾ){
    ਜੇ( p!=pExcept && (0==i_ਜੜ੍ਹ || p->pgno_ਜੜ੍ਹ==i_ਜੜ੍ਹ) ) ਰੋਕੋ;
  }
  ਜੇ( p ) ਵਾਪਸ saveCursorsOnList(p, i_ਜੜ੍ਹ, pExcept);
  ਜੇ( pExcept ) pExcept->curFlags &= ~BTCF_ਗੁਣਜ;
  ਵਾਪਸ SQLITE_OK;
}

/* This helper routine to saveAllCursors does the actual work of saving
** the cursors if and when a cursor is found that actually requires saving.
** The common case is that no cursors need to be saved, so this routine is
** broken out from its caller to avoid unnecessary stack pointer movement.
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_NOINLINE saveCursorsOnList(
  BtCursor *p,         /* The first cursor that needs saving */
  Pgno i_ਜੜ੍ਹ,          /* Only save cursor with this iRoot. Save all if zero */
  BtCursor *pExcept    /* Do not save this cursor */
){
  ਕਰੋ{
    ਜੇ( p!=pExcept && (0==i_ਜੜ੍ਹ || p->pgno_ਜੜ੍ਹ==i_ਜੜ੍ਹ) ){
      ਜੇ( p->eState==CURSOR_VALID || p->eState==CURSOR_SKIPNEXT ){
        ਪੂਰਨ rc = saveCursorPosition(p);
        ਜੇ( SQLITE_OK!=rc ){
          ਵਾਪਸ rc;
        }
      }ਨਹੀਂਤਾਂ{
        testcase( p->i_ਪੰਨਾ>=0 );
        btree_Release_All_Cursor_ਪੰਨੇ(p);
      }
    }
    p = p->p_ਅਗਲਾ;
  }ਜਦੋਂ( p );
  ਵਾਪਸ SQLITE_OK;
}

/*
** Clear the current cursor position.
*/
ਖਾਲੀ sqlite3BtreeClearCursor(BtCursor *pCur){
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorHoldsMutex(pCur) );
  sqlite3_free(pCur->p_ਕੁੰਜੀ);
  pCur->p_ਕੁੰਜੀ = 0;
  pCur->eState = CURSOR_INVALID;
}

/*
** In this version of BtreeMoveto, pKey is a packed index record
** such as is generated by the OP_MakeRecord opcode.  Unpack the
** record and then call sqlite3BtreeIndexMoveto() to do the work.
*/
ਸਥਿਰ ਪੂਰਨ btreeMoveto(
  BtCursor *pCur,     /* Cursor open on the btree to be searched */
  ਅਚਲ ਖਾਲੀ *p_ਕੁੰਜੀ,   /* Packed key if the btree is an index */
  i64 n_ਕੁੰਜੀ,           /* Integer key for tables.  Size of pKey for indices */
  ਪੂਰਨ bias,           /* Bias search to the high end */
  ਪੂਰਨ *pRes           /* Write search results here */
){
  ਪੂਰਨ rc;                    /* Status code */
  Unpacked_ਰਿਕਾਰਡ *p_Idx_ਕੁੰਜੀ;   /* Unpacked index key */

  ਜੇ( p_ਕੁੰਜੀ ){
    ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info = pCur->p_ਕੁੰਜੀ_Info;
    ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕੁੰਜੀ==(i64)(ਪੂਰਨ)n_ਕੁੰਜੀ );
    p_Idx_ਕੁੰਜੀ = sqlite3_Vdbe_Alloc_Unpacked_ਰਿਕਾਰਡ(p_ਕੁੰਜੀ_Info);
    ਜੇ( p_Idx_ਕੁੰਜੀ==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
    sqlite3_Vdbe_ਰਿਕਾਰਡ_Unpack((ਪੂਰਨ)n_ਕੁੰਜੀ, p_ਕੁੰਜੀ, p_Idx_ਕੁੰਜੀ);
    ਜੇ( p_Idx_ਕੁੰਜੀ->n_ਖੇਤਰ==0 || p_Idx_ਕੁੰਜੀ->n_ਖੇਤਰ>p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ ){
      rc = SQLITE_CORRUPT_BKPT;
    }ਨਹੀਂਤਾਂ{
      rc = sqlite3_Btree_ਸੂਚਕਾਂਕ_Moveto(pCur, p_Idx_ਕੁੰਜੀ, pRes);
    }
    sqlite3DbFree(pCur->p_ਕੁੰਜੀ_Info->db, p_Idx_ਕੁੰਜੀ);
  }ਨਹੀਂਤਾਂ{
    p_Idx_ਕੁੰਜੀ = 0;
    rc = sqlite3_Btree_ਸਾਰਣੀ_Moveto(pCur, n_ਕੁੰਜੀ, bias, pRes);
  }
  ਵਾਪਸ rc;
}

/*
** Restore the cursor to the position it was in (or as close to as possible)
** when saveCursorPosition() was called. Note that this call deletes the
** saved position info stored by saveCursorPosition(), so there can be
** at most one effective restoreCursorPosition() call after each
** saveCursorPosition().
*/
ਸਥਿਰ ਪੂਰਨ btreeRestoreCursorPosition(BtCursor *pCur){
  ਪੂਰਨ rc;
  ਪੂਰਨ skip_ਅਗਲਾ = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState>=CURSOR_REQUIRESEEK );
  ਜੇ( pCur->eState==CURSOR_FAULT ){
    ਵਾਪਸ pCur->skip_ਅਗਲਾ;
  }
  pCur->eState = CURSOR_INVALID;
  ਜੇ( sqlite3FaultSim(410) ){
    rc = SQLITE_IOERR;
  }ਨਹੀਂਤਾਂ{
    rc = btreeMoveto(pCur, pCur->p_ਕੁੰਜੀ, pCur->n_ਕੁੰਜੀ, 0, &skip_ਅਗਲਾ);
  }
  ਜੇ( rc==SQLITE_OK ){
    sqlite3_free(pCur->p_ਕੁੰਜੀ);
    pCur->p_ਕੁੰਜੀ = 0;
    ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID || pCur->eState==CURSOR_INVALID );
    ਜੇ( skip_ਅਗਲਾ ) pCur->skip_ਅਗਲਾ = skip_ਅਗਲਾ;
    ਜੇ( pCur->skip_ਅਗਲਾ && pCur->eState==CURSOR_VALID ){
      pCur->eState = CURSOR_SKIPNEXT;
    }
  }
  ਵਾਪਸ rc;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ restoreCursorPosition(p) \
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
ਪੂਰਨ sqlite3BtreeCursorHasMoved(BtCursor *pCur){
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(pCur)
       || pCur==sqlite3BtreeFakeValidCursor() );
  ਪ੍ਰਤਿਜ੍ਞਾ( offsetof(BtCursor, eState)==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(pCur->eState)==1 );
  ਵਾਪਸ CURSOR_VALID != *(u8*)pCur;
}

/*
** Return a pointer to a fake BtCursor object that will always answer
** false to the sqlite3BtreeCursorHasMoved() routine above.  The fake
** cursor returned must not be used with any other Btree interface.
*/
BtCursor *sqlite3BtreeFakeValidCursor(ਖਾਲੀ){
  ਸਥਿਰ u8 fakeCursor = CURSOR_VALID;
  ਪ੍ਰਤਿਜ੍ਞਾ( offsetof(BtCursor, eState)==0 );
  ਵਾਪਸ (BtCursor*)&fakeCursor;
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
ਪੂਰਨ sqlite3BtreeCursorRestore(BtCursor *pCur, ਪੂਰਨ *p_Different_ਪੰਕਤੀ){
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( pCur!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState!=CURSOR_VALID );
  rc = restoreCursorPosition(pCur);
  ਜੇ( rc ){
    *p_Different_ਪੰਕਤੀ = 1;
    ਵਾਪਸ rc;
  }
  ਜੇ( pCur->eState!=CURSOR_VALID ){
    *p_Different_ਪੰਕਤੀ = 1;
  }ਨਹੀਂਤਾਂ{
    *p_Different_ਪੰਕਤੀ = 0;
  }
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_CURSOR_HINTS
/*
** Provide hints to the cursor.  The particular hint given (and the type
** and number of the varargs parameters) is determined by the eHintType
** parameter.  See the definitions of the BTREE_HINT_* macros for details.
*/
ਖਾਲੀ sqlite3BtreeCursorHint(BtCursor *pCur, ਪੂਰਨ eHintType, ...){
  /* Used only by system that substitute their own storage engine */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  va_list ap;
  va_start(ap, eHintType);
  ਜੇ( eHintType==BTREE_HINT_RANGE ){
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ;
    Walker w;
    memset(&w, 0, ਆਕਾਰ(w));
    w.x_ਵਿਅੰਜਕ_Callback = sqlite3_Cursor_Range_Hint_ਵਿਅੰਜਕ_Check;
    p_ਵਿਅੰਜਕ = va_arg(ap, ਵਿਅੰਜਕ*);
    w.u.aMem = va_arg(ap, Mem*);
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( w.u.aMem!=0 );
    sqlite3_Walk_ਵਿਅੰਜਕ(&w, p_ਵਿਅੰਜਕ);
  }ਨਹੀਂਤਾਂ ਜੇ( ALWAYS(eHintType==BTREE_HINT_TABLECURSOR) ){
    BtCursor *pCsr = va_arg(ap, BtCursor*);
    ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_Cursor_Hint_ਸਾਰਣੀ_Cursor==0 
         || pCur->p_Cursor_Hint_ਸਾਰਣੀ_Cursor==pCsr
    );
    ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->p_ਕੁੰਜੀ_Info==0 || CORRUPT_DB );
    pCur->p_Cursor_Hint_ਸਾਰਣੀ_Cursor = pCsr;
  }
  va_end(ap);
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_CURSOR_HINTS */

#ਜੇ defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** Return the pointer configured via the BTREE_HINT_TABLECURSOR hint on
** cursor pCsr. This is used from OP_DeferredSeek to assert() that the
** index cursor has been correctly configured with the table cursor.
*/
BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor *pCsr){
  ਵਾਪਸ pCsr->p_Cursor_Hint_ਸਾਰਣੀ_Cursor;
}
#ਜੇ_ਅੰਤ

/*
** Provide flag hints to the cursor.
*/
ਖਾਲੀ sqlite3BtreeCursorHintFlags(BtCursor *pCur, ਬਿਨਾਨਿਸ਼ਾਨੀ x){
  ਪ੍ਰਤਿਜ੍ਞਾ( x==BTREE_SEEK_EQ || x==BTREE_BULKLOAD || x==0 );
  pCur->hints = (u8)x;
}


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
/*
** Given a page number of a regular database page, return the page
** number for the pointer-map page that contains the entry for the
** input page number.
**
** Return 0 (not a valid page) for pgno==1 since there is
** no pointer map associated with page 1.  The integrity_check logic
** requires that ptrmapPageno(*,1)!=1.
*/
ਸਥਿਰ Pgno ptrmapPageno(BtShared *pBt, Pgno pgno){
  ਪੂਰਨ n_ਪੰਨੇ_ਪ੍ਰਤੀ_Map_ਪੰਨਾ;
  Pgno iPtrMap, ret;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਜੇ( pgno<2 ) ਵਾਪਸ 0;
  n_ਪੰਨੇ_ਪ੍ਰਤੀ_Map_ਪੰਨਾ = (pBt->usable_ਮਾਪ/5)+1;
  iPtrMap = (pgno-2)/n_ਪੰਨੇ_ਪ੍ਰਤੀ_Map_ਪੰਨਾ;
  ret = (iPtrMap*n_ਪੰਨੇ_ਪ੍ਰਤੀ_Map_ਪੰਨਾ) + 2;
  ਜੇ( ret==PENDING_BYTE_ਪੰਨਾ(pBt) ){
    ret++;
  }
  ਵਾਪਸ ret;
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
ਸਥਿਰ ਖਾਲੀ ptrmapPut(BtShared *pBt, Pgno ਕੁੰਜੀ, u8 eType, Pgno ਮਾਪਾ, ਪੂਰਨ *pRC){
  Db_ਪੰਨਾ *p_Db_ਪੰਨਾ;  /* The pointer map page */
  u8 *pPtrmap;      /* The pointer map data */
  Pgno iPtrmap;     /* The pointer map page number */
  ਪੂਰਨ ਵਿੱਥ;       /* Offset in pointer map page */
  ਪੂਰਨ rc;           /* Return code from subfunctions */

  ਜੇ( *pRC ) ਵਾਪਸ;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  /* The super-journal page number must never be used as a pointer map page */
  ਪ੍ਰਤਿਜ੍ਞਾ( 0==PTRMAP_ISPAGE(pBt, PENDING_BYTE_ਪੰਨਾ(pBt)) );

  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->autoVacuum );
  ਜੇ( ਕੁੰਜੀ==0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    ਵਾਪਸ;
  }
  iPtrmap = PTRMAP_PAGENO(pBt, ਕੁੰਜੀ);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &p_Db_ਪੰਨਾ, 0);
  ਜੇ( rc!=SQLITE_OK ){
    *pRC = rc;
    ਵਾਪਸ;
  }
  ਜੇ( ((ਅੱਖਰ*)sqlite3PagerGetExtra(p_Db_ਪੰਨਾ))[0]!=0 ){
    /* The first byte of the extra data is the MemPage.isInit byte.
    ** If that byte is set, it means this page is also being used
    ** as a btree page. */
    *pRC = SQLITE_CORRUPT_BKPT;
    ਜਾਓ ptrmap_exit;
  }
  ਵਿੱਥ = PTRMAP_PTROFFSET(iPtrmap, ਕੁੰਜੀ);
  ਜੇ( ਵਿੱਥ<0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    ਜਾਓ ptrmap_exit;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿੱਥ <= (ਪੂਰਨ)pBt->usable_ਮਾਪ-5 );
  pPtrmap = (u8 *)sqlite3_Pager_Get_ਅੰਕੜੇ(p_Db_ਪੰਨਾ);

  ਜੇ( eType!=pPtrmap[ਵਿੱਥ] || get4byte(&pPtrmap[ਵਿੱਥ+1])!=ਮਾਪਾ ){
    TRACE(("PTRMAP_UPDATE: %u->(%u,%u)\n", ਕੁੰਜੀ, eType, ਮਾਪਾ));
    *pRC= rc = sqlite3PagerWrite(p_Db_ਪੰਨਾ);
    ਜੇ( rc==SQLITE_OK ){
      pPtrmap[ਵਿੱਥ] = eType;
      put4byte(&pPtrmap[ਵਿੱਥ+1], ਮਾਪਾ);
    }
  }

ptrmap_exit:
  sqlite3PagerUnref(p_Db_ਪੰਨਾ);
}

/*
** Read an entry from the pointer map.
**
** This routine retrieves the pointer map entry for page 'key', writing
** the type and parent page number to *pEType and *pPgno respectively.
** An error code is returned if something goes wrong, otherwise SQLITE_OK.
*/
ਸਥਿਰ ਪੂਰਨ ptrmapGet(BtShared *pBt, Pgno ਕੁੰਜੀ, u8 *pEType, Pgno *pPgno){
  Db_ਪੰਨਾ *p_Db_ਪੰਨਾ;   /* The pointer map page */
  ਪੂਰਨ iPtrmap;       /* Pointer map page index */
  u8 *pPtrmap;       /* Pointer map page data */
  ਪੂਰਨ ਵਿੱਥ;        /* Offset of entry in pointer map */
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );

  iPtrmap = PTRMAP_PAGENO(pBt, ਕੁੰਜੀ);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &p_Db_ਪੰਨਾ, 0);
  ਜੇ( rc!=0 ){
    ਵਾਪਸ rc;
  }
  pPtrmap = (u8 *)sqlite3_Pager_Get_ਅੰਕੜੇ(p_Db_ਪੰਨਾ);

  ਵਿੱਥ = PTRMAP_PTROFFSET(iPtrmap, ਕੁੰਜੀ);
  ਜੇ( ਵਿੱਥ<0 ){
    sqlite3PagerUnref(p_Db_ਪੰਨਾ);
    ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿੱਥ <= (ਪੂਰਨ)pBt->usable_ਮਾਪ-5 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pEType!=0 );
  *pEType = pPtrmap[ਵਿੱਥ];
  ਜੇ( pPgno ) *pPgno = get4byte(&pPtrmap[ਵਿੱਥ+1]);

  sqlite3PagerUnref(p_Db_ਪੰਨਾ);
  ਜੇ( *pEType<1 || *pEType>5 ) ਵਾਪਸ SQLITE_CORRUPT_PGNO(iPtrmap);
  ਵਾਪਸ SQLITE_OK;
}

#ਨਹੀਂਤਾਂ /* if defined SQLITE_OMIT_AUTOVACUUM */
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ptrmapPut(w,x,y,z,rc)
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ptrmapGet(w,x,y,z) SQLITE_OK
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ptrmapPutOvflPtr(x, y, z, rc)
#ਜੇ_ਅੰਤ

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਲੱਭੋ_Cell(P,I) \
  ((P)->a_ਅੰਕੜੇ + ((P)->ਨਕਾਬ_ਪੰਨਾ & get2byteAligned(&(P)->aCellIdx[2*(I)])))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਲੱਭੋ_Cell_Past_Ptr(P,I) \
  ((P)->a_ਅੰਕੜੇ_Ofst + ((P)->ਨਕਾਬ_ਪੰਨਾ & get2byteAligned(&(P)->aCellIdx[2*(I)])))


/*
** This is common tail processing for btreeParseCellPtr() and
** btreeParseCellPtrIndex() for the case when the cell does not fit entirely
** on a single B-tree page.  Make necessary adjustments to the CellInfo
** structure.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ btree_Parse_Cell_Adjust_ਮਾਪ_For_ਉਛਾਲ(
  Mem_ਪੰਨਾ *p_ਪੰਨਾ,         /* Page containing the cell */
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
  ਪੂਰਨ minLocal;  /* Minimum amount of payload held locally */
  ਪੂਰਨ maxLocal;  /* Maximum amount of payload held locally */
  ਪੂਰਨ surplus;   /* Overflow payload available for local storage */

  minLocal = p_ਪੰਨਾ->minLocal;
  maxLocal = p_ਪੰਨਾ->maxLocal;
  surplus = minLocal + (pInfo->nPayload - minLocal)%(p_ਪੰਨਾ->pBt->usable_ਮਾਪ-4);
  testcase( surplus==maxLocal );
  testcase( surplus==maxLocal+1 );
  ਜੇ( surplus <= maxLocal ){
    pInfo->nLocal = (u16)surplus;
  }ਨਹੀਂਤਾਂ{
    pInfo->nLocal = (u16)minLocal;
  }
  pInfo->n_ਮਾਪ = (u16)(&pInfo->pPayload[pInfo->nLocal] - pCell) + 4;
}

/*
** Given a record with nPayload bytes of payload stored within btree
** page pPage, return the number of bytes of payload stored locally.
*/
ਸਥਿਰ ਪੂਰਨ btree_Payload_ਮੰਜ਼ਿਲ_Local(Mem_ਪੰਨਾ *p_ਪੰਨਾ, i64 nPayload){
  ਪੂਰਨ maxLocal;  /* Maximum amount of payload held locally */
  maxLocal = p_ਪੰਨਾ->maxLocal;
  ਪ੍ਰਤਿਜ੍ਞਾ( nPayload>=0 );
  ਜੇ( nPayload<=maxLocal ){
    ਵਾਪਸ (ਪੂਰਨ)nPayload;
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ minLocal;  /* Minimum amount of payload held locally */
    ਪੂਰਨ surplus;   /* Overflow payload available for local storage */
    minLocal = p_ਪੰਨਾ->minLocal;
    surplus = (ਪੂਰਨ)(minLocal +(nPayload - minLocal)%(p_ਪੰਨਾ->pBt->usable_ਮਾਪ-4));
    ਵਾਪਸ (surplus <= maxLocal) ? surplus : minLocal;
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
ਸਥਿਰ ਖਾਲੀ btreeParseCellPtrNoPayload(
  Mem_ਪੰਨਾ *p_ਪੰਨਾ,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->ਪੱਤੀ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ==4 );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਅਣਵਰਤਿਆ_PARAMETER(p_ਪੰਨਾ);
#ਜੇ_ਅੰਤ
  pInfo->n_ਮਾਪ = 4 + sqlite3GetVarint(&pCell[4], (u64*)&pInfo->n_ਕੁੰਜੀ);
  pInfo->nPayload = 0;
  pInfo->nLocal = 0;
  pInfo->pPayload = 0;
  ਵਾਪਸ;
}
ਸਥਿਰ ਖਾਲੀ btreeParseCellPtr(
  Mem_ਪੰਨਾ *p_ਪੰਨਾ,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u64 nPayload;           /* Number of bytes of cell payload */
  u64 i_ਕੁੰਜੀ;               /* Extracted Key value */

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->ਪੱਤੀ==0 || p_ਪੰਨਾ->ਪੱਤੀ==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->int_ਕੁੰਜੀ_ਪੱਤੀ );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ==0 );
  pIter = pCell;

  /* The next block of code is equivalent to:
  **
  **     pIter += getVarint32(pIter, nPayload);
  **
  ** The code is inlined to avoid a function call.
  */
  nPayload = *pIter;
  ਜੇ( nPayload>=0x80 ){
    u8 *p_ਸਿਰਾ = &pIter[8];
    nPayload &= 0x7f;
    ਕਰੋ{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }ਜਦੋਂ( (*pIter)>=0x80 && pIter<p_ਸਿਰਾ );
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
  i_ਕੁੰਜੀ = *pIter;
  ਜੇ( i_ਕੁੰਜੀ>=0x80 ){
    u8 x;
    i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ (x = *++pIter);
    ਜੇ( x>=0x80 ){
      i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ (x = *++pIter);
      ਜੇ( x>=0x80 ){
        i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x10204000 ^ (x = *++pIter);
        ਜੇ( x>=0x80 ){
          i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x4000 ^ (x = *++pIter);
          ਜੇ( x>=0x80 ){
            i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x4000 ^ (x = *++pIter);
            ਜੇ( x>=0x80 ){
              i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x4000 ^ (x = *++pIter);
              ਜੇ( x>=0x80 ){
                i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<7) ^ 0x4000 ^ (x = *++pIter);
                ਜੇ( x>=0x80 ){
                  i_ਕੁੰਜੀ = (i_ਕੁੰਜੀ<<8) ^ 0x8000 ^ (*++pIter);
                }
              }
            }
          }
        }
      }ਨਹੀਂਤਾਂ{
        i_ਕੁੰਜੀ ^= 0x204000;
      }
    }ਨਹੀਂਤਾਂ{
      i_ਕੁੰਜੀ ^= 0x4000;
    }
  }
  pIter++;

  pInfo->n_ਕੁੰਜੀ = *(i64*)&i_ਕੁੰਜੀ;
  pInfo->nPayload = (u32)nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==p_ਪੰਨਾ->maxLocal );
  testcase( nPayload==(u32)p_ਪੰਨਾ->maxLocal+1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->maxLocal <= BT_MAX_LOCAL );
  ਜੇ( nPayload<=p_ਪੰਨਾ->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->n_ਮਾਪ = (u16)nPayload + (u16)(pIter - pCell);
    ਜੇ( pInfo->n_ਮਾਪ<4 ) pInfo->n_ਮਾਪ = 4;
    pInfo->nLocal = (u16)nPayload;
  }ਨਹੀਂਤਾਂ{
    btree_Parse_Cell_Adjust_ਮਾਪ_For_ਉਛਾਲ(p_ਪੰਨਾ, pCell, pInfo);
  }
}
ਸਥਿਰ ਖਾਲੀ btree_Parse_Cell_Ptr_ਸੂਚਕਾਂਕ(
  Mem_ਪੰਨਾ *p_ਪੰਨਾ,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u32 nPayload;           /* Number of bytes of cell payload */

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->ਪੱਤੀ==0 || p_ਪੰਨਾ->ਪੱਤੀ==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->int_ਕੁੰਜੀ_ਪੱਤੀ==0 );
  pIter = pCell + p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ;
  nPayload = *pIter;
  ਜੇ( nPayload>=0x80 ){
    u8 *p_ਸਿਰਾ = &pIter[8];
    nPayload &= 0x7f;
    ਕਰੋ{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }ਜਦੋਂ( *(pIter)>=0x80 && pIter<p_ਸਿਰਾ );
  }
  pIter++;
  pInfo->n_ਕੁੰਜੀ = nPayload;
  pInfo->nPayload = nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==p_ਪੰਨਾ->maxLocal );
  testcase( nPayload==(u32)p_ਪੰਨਾ->maxLocal+1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( nPayload>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->maxLocal <= BT_MAX_LOCAL );
  ਜੇ( nPayload<=p_ਪੰਨਾ->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->n_ਮਾਪ = (u16)nPayload + (u16)(pIter - pCell);
    ਜੇ( pInfo->n_ਮਾਪ<4 ) pInfo->n_ਮਾਪ = 4;
    pInfo->nLocal = (u16)nPayload;
  }ਨਹੀਂਤਾਂ{
    btree_Parse_Cell_Adjust_ਮਾਪ_For_ਉਛਾਲ(p_ਪੰਨਾ, pCell, pInfo);
  }
}
ਸਥਿਰ ਖਾਲੀ btreeParseCell(
  Mem_ਪੰਨਾ *p_ਪੰਨਾ,         /* Page containing the cell */
  ਪੂਰਨ iCell,              /* The cell index.  First cell is 0 */
  CellInfo *pInfo         /* Fill in this structure */
){
  p_ਪੰਨਾ->xParseCell(p_ਪੰਨਾ, ਲੱਭੋ_Cell(p_ਪੰਨਾ, iCell), pInfo);
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
ਸਥਿਰ u16 cell_ਮਾਪ_Ptr(Mem_ਪੰਨਾ *p_ਪੰਨਾ, u8 *pCell){
  u8 *pIter = pCell + 4;                   /* For looping over bytes of pCell */
  u8 *p_ਸਿਰਾ;                                /* End mark for a varint */
  u32 n_ਮਾਪ;                               /* Size value to return */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_ਪੰਨਾ->xParseCell(p_ਪੰਨਾ, pCell, &debuginfo);
#ਜੇ_ਅੰਤ

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ==4 );
  n_ਮਾਪ = *pIter;
  ਜੇ( n_ਮਾਪ>=0x80 ){
    p_ਸਿਰਾ = &pIter[8];
    n_ਮਾਪ &= 0x7f;
    ਕਰੋ{
      n_ਮਾਪ = (n_ਮਾਪ<<7) | (*++pIter & 0x7f);
    }ਜਦੋਂ( *(pIter)>=0x80 && pIter<p_ਸਿਰਾ );
  }
  pIter++;
  testcase( n_ਮਾਪ==p_ਪੰਨਾ->maxLocal );
  testcase( n_ਮਾਪ==(u32)p_ਪੰਨਾ->maxLocal+1 );
  ਜੇ( n_ਮਾਪ<=p_ਪੰਨਾ->maxLocal ){
    n_ਮਾਪ += (u32)(pIter - pCell);
    ਪ੍ਰਤਿਜ੍ਞਾ( n_ਮਾਪ>4 );
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ minLocal = p_ਪੰਨਾ->minLocal;
    n_ਮਾਪ = minLocal + (n_ਮਾਪ - minLocal) % (p_ਪੰਨਾ->pBt->usable_ਮਾਪ - 4);
    testcase( n_ਮਾਪ==p_ਪੰਨਾ->maxLocal );
    testcase( n_ਮਾਪ==(u32)p_ਪੰਨਾ->maxLocal+1 );
    ਜੇ( n_ਮਾਪ>p_ਪੰਨਾ->maxLocal ){
      n_ਮਾਪ = minLocal;
    }
    n_ਮਾਪ += 4 + (u16)(pIter - pCell);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਮਾਪ==debuginfo.n_ਮਾਪ || CORRUPT_DB );
  ਵਾਪਸ (u16)n_ਮਾਪ;
}
ਸਥਿਰ u16 cell_ਮਾਪ_Ptr_Idx_ਪੱਤੀ(Mem_ਪੰਨਾ *p_ਪੰਨਾ, u8 *pCell){
  u8 *pIter = pCell;                       /* For looping over bytes of pCell */
  u8 *p_ਸਿਰਾ;                                /* End mark for a varint */
  u32 n_ਮਾਪ;                               /* Size value to return */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_ਪੰਨਾ->xParseCell(p_ਪੰਨਾ, pCell, &debuginfo);
#ਜੇ_ਅੰਤ

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ==0 );
  n_ਮਾਪ = *pIter;
  ਜੇ( n_ਮਾਪ>=0x80 ){
    p_ਸਿਰਾ = &pIter[8];
    n_ਮਾਪ &= 0x7f;
    ਕਰੋ{
      n_ਮਾਪ = (n_ਮਾਪ<<7) | (*++pIter & 0x7f);
    }ਜਦੋਂ( *(pIter)>=0x80 && pIter<p_ਸਿਰਾ );
  }
  pIter++;
  testcase( n_ਮਾਪ==p_ਪੰਨਾ->maxLocal );
  testcase( n_ਮਾਪ==(u32)p_ਪੰਨਾ->maxLocal+1 );
  ਜੇ( n_ਮਾਪ<=p_ਪੰਨਾ->maxLocal ){
    n_ਮਾਪ += (u32)(pIter - pCell);
    ਜੇ( n_ਮਾਪ<4 ) n_ਮਾਪ = 4;
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ minLocal = p_ਪੰਨਾ->minLocal;
    n_ਮਾਪ = minLocal + (n_ਮਾਪ - minLocal) % (p_ਪੰਨਾ->pBt->usable_ਮਾਪ - 4);
    testcase( n_ਮਾਪ==p_ਪੰਨਾ->maxLocal );
    testcase( n_ਮਾਪ==(u32)p_ਪੰਨਾ->maxLocal+1 );
    ਜੇ( n_ਮਾਪ>p_ਪੰਨਾ->maxLocal ){
      n_ਮਾਪ = minLocal;
    }
    n_ਮਾਪ += 4 + (u16)(pIter - pCell);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਮਾਪ==debuginfo.n_ਮਾਪ || CORRUPT_DB );
  ਵਾਪਸ (u16)n_ਮਾਪ;
}
ਸਥਿਰ u16 cell_ਮਾਪ_Ptr_No_Payload(Mem_ਪੰਨਾ *p_ਪੰਨਾ, u8 *pCell){
  u8 *pIter = pCell + 4; /* For looping over bytes of pCell */
  u8 *p_ਸਿਰਾ;              /* End mark for a varint */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_ਪੰਨਾ->xParseCell(p_ਪੰਨਾ, pCell, &debuginfo);
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(p_ਪੰਨਾ);
#ਜੇ_ਅੰਤ

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ==4 );
  p_ਸਿਰਾ = pIter + 9;
  ਜਦੋਂ( (*pIter++)&0x80 && pIter<p_ਸਿਰਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( debuginfo.n_ਮਾਪ==(u16)(pIter - pCell) || CORRUPT_DB );
  ਵਾਪਸ (u16)(pIter - pCell);
}
ਸਥਿਰ u16 cell_ਮਾਪ_Ptr_ਸਾਰਣੀ_ਪੱਤੀ(Mem_ਪੰਨਾ *p_ਪੰਨਾ, u8 *pCell){
  u8 *pIter = pCell;   /* For looping over bytes of pCell */
  u8 *p_ਸਿਰਾ;            /* End mark for a varint */
  u32 n_ਮਾਪ;           /* Size value to return */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_ਪੰਨਾ->xParseCell(p_ਪੰਨਾ, pCell, &debuginfo);
#ਜੇ_ਅੰਤ

  n_ਮਾਪ = *pIter;
  ਜੇ( n_ਮਾਪ>=0x80 ){
    p_ਸਿਰਾ = &pIter[8];
    n_ਮਾਪ &= 0x7f;
    ਕਰੋ{
      n_ਮਾਪ = (n_ਮਾਪ<<7) | (*++pIter & 0x7f);
    }ਜਦੋਂ( *(pIter)>=0x80 && pIter<p_ਸਿਰਾ );
  }
  pIter++;
  /* pIter now points at the 64-bit integer key value, a variable length
  ** integer. The following block moves pIter to point at the first byte
  ** past the end of the key value. */
  ਜੇ( (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80 ){ pIter++; }
  testcase( n_ਮਾਪ==p_ਪੰਨਾ->maxLocal );
  testcase( n_ਮਾਪ==(u32)p_ਪੰਨਾ->maxLocal+1 );
  ਜੇ( n_ਮਾਪ<=p_ਪੰਨਾ->maxLocal ){
    n_ਮਾਪ += (u32)(pIter - pCell);
    ਜੇ( n_ਮਾਪ<4 ) n_ਮਾਪ = 4;
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ minLocal = p_ਪੰਨਾ->minLocal;
    n_ਮਾਪ = minLocal + (n_ਮਾਪ - minLocal) % (p_ਪੰਨਾ->pBt->usable_ਮਾਪ - 4);
    testcase( n_ਮਾਪ==p_ਪੰਨਾ->maxLocal );
    testcase( n_ਮਾਪ==(u32)p_ਪੰਨਾ->maxLocal+1 );
    ਜੇ( n_ਮਾਪ>p_ਪੰਨਾ->maxLocal ){
      n_ਮਾਪ = minLocal;
    }
    n_ਮਾਪ += 4 + (u16)(pIter - pCell);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਮਾਪ==debuginfo.n_ਮਾਪ || CORRUPT_DB );
  ਵਾਪਸ (u16)n_ਮਾਪ;
}


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/* This variation on cellSizePtr() is used inside of assert() statements
** only. */
ਸਥਿਰ u16 cell_ਮਾਪ(Mem_ਪੰਨਾ *p_ਪੰਨਾ, ਪੂਰਨ iCell){
  ਵਾਪਸ p_ਪੰਨਾ->x_Cell_ਮਾਪ(p_ਪੰਨਾ, ਲੱਭੋ_Cell(p_ਪੰਨਾ, iCell));
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
/*
** The cell pCell is currently part of page pSrc but will ultimately be part
** of pPage.  (pSrc and pPage are often the same.)  If pCell contains a
** pointer to an overflow page, insert an entry into the pointer-map for
** the overflow page that will be valid after pCell has been moved to pPage.
*/
ਸਥਿਰ ਖਾਲੀ ptrmapPutOvflPtr(Mem_ਪੰਨਾ *p_ਪੰਨਾ, Mem_ਪੰਨਾ *pSrc, u8 *pCell,ਪੂਰਨ *pRC){
  CellInfo info;
  ਜੇ( *pRC ) ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pCell!=0 );
  p_ਪੰਨਾ->xParseCell(p_ਪੰਨਾ, pCell, &info);
  ਜੇ( info.nLocal<info.nPayload ){
    Pgno ovfl;
    ਜੇ( SQLITE_ਉਛਾਲ(pSrc->a_ਅੰਕੜੇ_ਸਿਰਾ, pCell, pCell+info.nLocal) ){
      testcase( pSrc!=p_ਪੰਨਾ );
      *pRC = SQLITE_CORRUPT_BKPT;
      ਵਾਪਸ;
    }
    ovfl = get4byte(&pCell[info.n_ਮਾਪ-4]);
    ptrmapPut(p_ਪੰਨਾ->pBt, ovfl, PTRMAP_OVERFLOW1, p_ਪੰਨਾ->pgno, pRC);
  }
}
#ਜੇ_ਅੰਤ


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
ਸਥਿਰ ਪੂਰਨ defragment_ਪੰਨਾ(Mem_ਪੰਨਾ *p_ਪੰਨਾ, ਪੂਰਨ nMaxFrag){
  ਪੂਰਨ i;                     /* Loop counter */
  ਪੂਰਨ pc;                    /* Address of the i-th cell */
  ਪੂਰਨ hdr;                   /* Offset to the page header */
  ਪੂਰਨ ਮਾਪ;                  /* Size of a cell */
  ਪੂਰਨ usable_ਮਾਪ;            /* Number of usable bytes on a page */
  ਪੂਰਨ cell_ਵਿੱਥ;            /* Offset to the cell pointer array */
  ਪੂਰਨ cbrk;                  /* Offset to the cell content area */
  ਪੂਰਨ nCell;                 /* Number of cells on the page */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅੰਕੜੇ;       /* The page data */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਆਰਜ਼ੀ;       /* Temp area for cell content */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *src;        /* Source of content */
  ਪੂਰਨ i_Cell_ਪਹਿਲਾ;            /* First allowable cell index */
  ਪੂਰਨ i_Cell_ਆਖਰੀ;             /* Last possible cell index */
  ਪੂਰਨ i_Cell_ਸ਼ੁਰੂ;            /* First cell offset in input */

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pBt!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pBt->usable_ਮਾਪ <= SQLITE_MAX_ਪੰਨਾ_ਮਾਪ );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->n_ਉਛਾਲ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਅੰਕੜੇ = p_ਪੰਨਾ->a_ਅੰਕੜੇ;
  hdr = p_ਪੰਨਾ->hdr_ਵਿੱਥ;
  cell_ਵਿੱਥ = p_ਪੰਨਾ->cell_ਵਿੱਥ;
  nCell = p_ਪੰਨਾ->nCell;
  ਪ੍ਰਤਿਜ੍ਞਾ( nCell==get2byte(&ਅੰਕੜੇ[hdr+3]) || CORRUPT_DB );
  i_Cell_ਪਹਿਲਾ = cell_ਵਿੱਥ + 2*nCell;
  usable_ਮਾਪ = p_ਪੰਨਾ->pBt->usable_ਮਾਪ;

  /* This block handles pages with two or fewer free blocks and nMaxFrag
  ** or fewer fragmented bytes. In this case it is faster to move the
  ** two (or one) blocks of cells using memmove() and add the required
  ** offsets to each pointer in the cell-pointer array than it is to
  ** reconstruct the entire page.  */
  ਜੇ( (ਪੂਰਨ)ਅੰਕੜੇ[hdr+7]<=nMaxFrag ){
    ਪੂਰਨ iFree = get2byte(&ਅੰਕੜੇ[hdr+1]);
    ਜੇ( iFree>usable_ਮਾਪ-4 ) ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    ਜੇ( iFree ){
      ਪੂਰਨ iFree2 = get2byte(&ਅੰਕੜੇ[iFree]);
      ਜੇ( iFree2>usable_ਮਾਪ-4 ) ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
      ਜੇ( 0==iFree2 || (ਅੰਕੜੇ[iFree2]==0 && ਅੰਕੜੇ[iFree2+1]==0) ){
        u8 *p_ਸਿਰਾ = &ਅੰਕੜੇ[cell_ਵਿੱਥ + nCell*2];
        u8 *pAddr;
        ਪੂਰਨ sz2 = 0;
        ਪੂਰਨ sz = get2byte(&ਅੰਕੜੇ[iFree+2]);
        ਪੂਰਨ ਸਿਖਰ = get2byte(&ਅੰਕੜੇ[hdr+5]);
        ਜੇ( ਸਿਖਰ>=iFree ){
          ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
        }
        ਜੇ( iFree2 ){
          ਜੇ( iFree+sz>iFree2 ) ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
          sz2 = get2byte(&ਅੰਕੜੇ[iFree2+2]);
          ਜੇ( iFree2+sz2 > usable_ਮਾਪ ) ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
          memmove(&ਅੰਕੜੇ[iFree+sz+sz2], &ਅੰਕੜੇ[iFree+sz], iFree2-(iFree+sz));
          sz += sz2;
        }ਨਹੀਂਤਾਂ ਜੇ( iFree+sz>usable_ਮਾਪ ){
          ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
        }

        cbrk = ਸਿਖਰ+sz;
        ਪ੍ਰਤਿਜ੍ਞਾ( cbrk+(iFree-ਸਿਖਰ) <= usable_ਮਾਪ );
        memmove(&ਅੰਕੜੇ[cbrk], &ਅੰਕੜੇ[ਸਿਖਰ], iFree-ਸਿਖਰ);
        ਲਈ(pAddr=&ਅੰਕੜੇ[cell_ਵਿੱਥ]; pAddr<p_ਸਿਰਾ; pAddr+=2){
          pc = get2byte(pAddr);
          ਜੇ( pc<iFree ){ put2byte(pAddr, pc+sz); }
          ਨਹੀਂਤਾਂ ਜੇ( pc<iFree2 ){ put2byte(pAddr, pc+sz2); }
        }
        ਜਾਓ defragment_ਬਾਹਰਲਾ;
      }
    }
  }

  cbrk = usable_ਮਾਪ;
  i_Cell_ਆਖਰੀ = usable_ਮਾਪ - 4;
  i_Cell_ਸ਼ੁਰੂ = get2byte(&ਅੰਕੜੇ[hdr+5]);
  ਜੇ( nCell>0 ){
    ਆਰਜ਼ੀ = sqlite3_Pager_ਆਰਜ਼ੀ_Space(p_ਪੰਨਾ->pBt->pPager);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਆਰਜ਼ੀ, ਅੰਕੜੇ, usable_ਮਾਪ);
    src = ਆਰਜ਼ੀ;
    ਲਈ(i=0; i<nCell; i++){
      u8 *pAddr;     /* The i-th cell pointer */
      pAddr = &ਅੰਕੜੇ[cell_ਵਿੱਥ + i*2];
      pc = get2byte(pAddr);
      testcase( pc==i_Cell_ਪਹਿਲਾ );
      testcase( pc==i_Cell_ਆਖਰੀ );
      /* These conditions have already been verified in btreeInitPage()
      ** if PRAGMA cell_size_check=ON.
      */
      ਜੇ( pc>i_Cell_ਆਖਰੀ ){
        ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( pc>=0 && pc<=i_Cell_ਆਖਰੀ );
      ਮਾਪ = p_ਪੰਨਾ->x_Cell_ਮਾਪ(p_ਪੰਨਾ, &src[pc]);
      cbrk -= ਮਾਪ;
      ਜੇ( cbrk<i_Cell_ਸ਼ੁਰੂ || pc+ਮਾਪ>usable_ਮਾਪ ){
        ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( cbrk+ਮਾਪ<=usable_ਮਾਪ && cbrk>=i_Cell_ਸ਼ੁਰੂ );
      testcase( cbrk+ਮਾਪ==usable_ਮਾਪ );
      testcase( pc+ਮਾਪ==usable_ਮਾਪ );
      put2byte(pAddr, cbrk);
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&ਅੰਕੜੇ[cbrk], &src[pc], ਮਾਪ);
    }
  }
  ਅੰਕੜੇ[hdr+7] = 0;

defragment_ਬਾਹਰਲਾ:
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nFree>=0 );
  ਜੇ( ਅੰਕੜੇ[hdr+7]+cbrk-i_Cell_ਪਹਿਲਾ!=p_ਪੰਨਾ->nFree ){
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( cbrk>=i_Cell_ਪਹਿਲਾ );
  put2byte(&ਅੰਕੜੇ[hdr+5], cbrk);
  ਅੰਕੜੇ[hdr+1] = 0;
  ਅੰਕੜੇ[hdr+2] = 0;
  memset(&ਅੰਕੜੇ[i_Cell_ਪਹਿਲਾ], 0, cbrk-i_Cell_ਪਹਿਲਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ u8 *ਪੰਨਾ_ਲੱਭੋ_ਥਾਂ(Mem_ਪੰਨਾ *pPg, ਪੂਰਨ nByte, ਪੂਰਨ *pRc){
  ਅਚਲ ਪੂਰਨ hdr = pPg->hdr_ਵਿੱਥ;            /* Offset to page header */
  u8 * ਅਚਲ a_ਅੰਕੜੇ = pPg->a_ਅੰਕੜੇ;             /* Page data */
  ਪੂਰਨ iAddr = hdr + 1;                       /* Address of ptr to pc */
  u8 *pTmp = &a_ਅੰਕੜੇ[iAddr];                  /* Temporary ptr into aData[] */
  ਪੂਰਨ pc = get2byte(pTmp);                   /* Address of a free slot */
  ਪੂਰਨ x;                                     /* Excess size of the slot */
  ਪੂਰਨ maxPC = pPg->pBt->usable_ਮਾਪ - nByte;  /* Max address for a usable slot */
  ਪੂਰਨ ਮਾਪ;                                  /* Size of the free slot */

  ਪ੍ਰਤਿਜ੍ਞਾ( pc>0 );
  ਜਦੋਂ( pc<=maxPC ){
    /* EVIDENCE-OF: R-22710-53328 The third and fourth bytes of each
    ** freeblock form a big-endian integer which is the size of the freeblock
    ** in bytes, including the 4-byte header. */
    pTmp = &a_ਅੰਕੜੇ[pc+2];
    ਮਾਪ = get2byte(pTmp);
    ਜੇ( (x = ਮਾਪ - nByte)>=0 ){
      testcase( x==4 );
      testcase( x==3 );
      ਜੇ( x<4 ){
        /* EVIDENCE-OF: R-11498-58022 In a well-formed b-tree page, the total
        ** number of bytes in fragments may not exceed 60. */
        ਜੇ( a_ਅੰਕੜੇ[hdr+7]>57 ) ਵਾਪਸ 0;

        /* Remove the slot from the free-list. Update the number of
        ** fragmented bytes within the page. */
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(&a_ਅੰਕੜੇ[iAddr], &a_ਅੰਕੜੇ[pc], 2);
        a_ਅੰਕੜੇ[hdr+7] += (u8)x;
        ਵਾਪਸ &a_ਅੰਕੜੇ[pc];
      }ਨਹੀਂਤਾਂ ਜੇ( x+pc > maxPC ){
        /* This slot extends off the end of the usable part of the page */
        *pRc = SQLITE_CORRUPT_ਪੰਨਾ(pPg);
        ਵਾਪਸ 0;
      }ਨਹੀਂਤਾਂ{
        /* The slot remains on the free-list. Reduce its size to account
        ** for the portion used by the new allocation. */
        put2byte(&a_ਅੰਕੜੇ[pc+2], x);
      }
      ਵਾਪਸ &a_ਅੰਕੜੇ[pc + x];
    }
    iAddr = pc;
    pTmp = &a_ਅੰਕੜੇ[pc];
    pc = get2byte(pTmp);
    ਜੇ( pc<=iAddr ){
      ਜੇ( pc ){
        /* The next slot in the chain comes before the current slot */
        *pRc = SQLITE_CORRUPT_ਪੰਨਾ(pPg);
      }
      ਵਾਪਸ 0;
    }
  }
  ਜੇ( pc>maxPC+nByte-4 ){
    /* The free slot chain extends off the end of the page */
    *pRc = SQLITE_CORRUPT_ਪੰਨਾ(pPg);
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ SQLITE_INLINE ਪੂਰਨ allocateSpace(Mem_ਪੰਨਾ *p_ਪੰਨਾ, ਪੂਰਨ nByte, ਪੂਰਨ *pIdx){
  ਅਚਲ ਪੂਰਨ hdr = p_ਪੰਨਾ->hdr_ਵਿੱਥ;    /* Local cache of pPage->hdrOffset */
  u8 * ਅਚਲ ਅੰਕੜੇ = p_ਪੰਨਾ->a_ਅੰਕੜੇ;      /* Local cache of pPage->aData */
  ਪੂਰਨ ਸਿਖਰ;                             /* First byte of cell content area */
  ਪੂਰਨ rc = SQLITE_OK;                  /* Integer return code */
  u8 *pTmp;                            /* Temp ptr into data[] */
  ਪੂਰਨ gap;        /* First byte of gap between cell pointers and cell content */
 
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pBt );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( nByte>=0 );  /* Minimum cell size is 4 */
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nFree>=nByte );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->n_ਉਛਾਲ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( nByte < (ਪੂਰਨ)(p_ਪੰਨਾ->pBt->usable_ਮਾਪ-8) );

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->cell_ਵਿੱਥ == hdr + 12 - 4*p_ਪੰਨਾ->ਪੱਤੀ );
  gap = p_ਪੰਨਾ->cell_ਵਿੱਥ + 2*p_ਪੰਨਾ->nCell;
  ਪ੍ਰਤਿਜ੍ਞਾ( gap<=65536 );
  /* EVIDENCE-OF: R-29356-02391 If the database uses a 65536-byte page size
  ** and the reserved space is zero (the usual value for reserved space)
  ** then the cell content offset of an empty page wants to be 65536.
  ** However, that integer is too large to be stored in a 2-byte unsigned
  ** integer, so a value of 0 is used in its place. */
  pTmp = &ਅੰਕੜੇ[hdr+5];
  ਸਿਖਰ = get2byte(pTmp);
  ਜੇ( gap>ਸਿਖਰ ){
    ਜੇ( ਸਿਖਰ==0 && p_ਪੰਨਾ->pBt->usable_ਮਾਪ==65536 ){
      ਸਿਖਰ = 65536;
    }ਨਹੀਂਤਾਂ{
      ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( ਸਿਖਰ>(ਪੂਰਨ)p_ਪੰਨਾ->pBt->usable_ਮਾਪ ){
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
  }

  /* If there is enough space between gap and top for one more cell pointer,
  ** and if the freelist is not empty, then search the
  ** freelist looking for a slot big enough to satisfy the request.
  */
  testcase( gap+2==ਸਿਖਰ );
  testcase( gap+1==ਸਿਖਰ );
  testcase( gap==ਸਿਖਰ );
  ਜੇ( (ਅੰਕੜੇ[hdr+2] || ਅੰਕੜੇ[hdr+1]) && gap+2<=ਸਿਖਰ ){
    u8 *pSpace = ਪੰਨਾ_ਲੱਭੋ_ਥਾਂ(p_ਪੰਨਾ, nByte, &rc);
    ਜੇ( pSpace ){
      ਪੂਰਨ g2;
      ਪ੍ਰਤਿਜ੍ਞਾ( pSpace+nByte<=ਅੰਕੜੇ+p_ਪੰਨਾ->pBt->usable_ਮਾਪ );
      *pIdx = g2 = (ਪੂਰਨ)(pSpace-ਅੰਕੜੇ);
      ਜੇ( g2<=gap ){
        ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
      }ਨਹੀਂਤਾਂ{
        ਵਾਪਸ SQLITE_OK;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( rc ){
      ਵਾਪਸ rc;
    }
  }

  /* The request could not be fulfilled using a freelist slot.  Check
  ** to see if defragmentation is necessary.
  */
  testcase( gap+2+nByte==ਸਿਖਰ );
  ਜੇ( gap+2+nByte>ਸਿਖਰ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nCell>0 || CORRUPT_DB );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nFree>=0 );
    rc = defragment_ਪੰਨਾ(p_ਪੰਨਾ, MIN(4, p_ਪੰਨਾ->nFree - (2+nByte)));
    ਜੇ( rc ) ਵਾਪਸ rc;
    ਸਿਖਰ = get2byteNotZero(&ਅੰਕੜੇ[hdr+5]);
    ਪ੍ਰਤਿਜ੍ਞਾ( gap+2+nByte<=ਸਿਖਰ );
  }


  /* Allocate memory from the gap in between the cell pointer array
  ** and the cell content area.  The btreeComputeFreeSpace() call has already
  ** validated the freelist.  Given that the freelist is valid, there
  ** is no way that the allocation can extend off the end of the page.
  ** The assert() below verifies the previous sentence.
  */
  ਸਿਖਰ -= nByte;
  put2byte(&ਅੰਕੜੇ[hdr+5], ਸਿਖਰ);
  ਪ੍ਰਤਿਜ੍ਞਾ( ਸਿਖਰ+nByte <= (ਪੂਰਨ)p_ਪੰਨਾ->pBt->usable_ਮਾਪ );
  *pIdx = ਸਿਖਰ;
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ freeSpace(Mem_ਪੰਨਾ *p_ਪੰਨਾ, ਪੂਰਨ i_ਸ਼ੁਰੂ, ਪੂਰਨ i_ਮਾਪ){
  ਪੂਰਨ iPtr;                             /* Address of ptr to next freeblock */
  ਪੂਰਨ iFreeBlk;                         /* Address of the next freeblock */
  u8 hdr;                               /* Page header size.  0 or 100 */
  ਪੂਰਨ nFrag = 0;                        /* Reduction in fragmentation */
  ਪੂਰਨ i_Orig_ਮਾਪ = i_ਮਾਪ;                /* Original value of iSize */
  ਪੂਰਨ x;                                /* Offset to cell content area */
  ਪੂਰਨ i_ਸਿਰਾ = i_ਸ਼ੁਰੂ + i_ਮਾਪ;            /* First byte past the iStart buffer */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅੰਕੜੇ = p_ਪੰਨਾ->a_ਅੰਕੜੇ;   /* Page content */
  u8 *pTmp;                             /* Temporary ptr into data[] */

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pBt!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB || i_ਸ਼ੁਰੂ>=p_ਪੰਨਾ->hdr_ਵਿੱਥ+6+p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ );
  ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB || i_ਸਿਰਾ <= (ਪੂਰਨ)p_ਪੰਨਾ->pBt->usable_ਮਾਪ );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਮਾਪ>=4 );   /* Minimum cell size is 4 */
  ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB || i_ਸ਼ੁਰੂ<=(ਪੂਰਨ)p_ਪੰਨਾ->pBt->usable_ਮਾਪ-4 );

  /* The list of freeblocks must be in ascending order.  Find the
  ** spot on the list where iStart should be inserted.
  */
  hdr = p_ਪੰਨਾ->hdr_ਵਿੱਥ;
  iPtr = hdr + 1;
  ਜੇ( ਅੰਕੜੇ[iPtr+1]==0 && ਅੰਕੜੇ[iPtr]==0 ){
    iFreeBlk = 0;  /* Shortcut for the case when the freelist is empty */
  }ਨਹੀਂਤਾਂ{
    ਜਦੋਂ( (iFreeBlk = get2byte(&ਅੰਕੜੇ[iPtr]))<i_ਸ਼ੁਰੂ ){
      ਜੇ( iFreeBlk<=iPtr ){
        ਜੇ( iFreeBlk==0 ) ਰੋਕੋ; /* TH3: corrupt082.100 */
        ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
      }
      iPtr = iFreeBlk;
    }
    ਜੇ( iFreeBlk>(ਪੂਰਨ)p_ਪੰਨਾ->pBt->usable_ਮਾਪ-4 ){ /* TH3: corrupt081.100 */
      ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( iFreeBlk>iPtr || iFreeBlk==0 || CORRUPT_DB );
 
    /* At this point:
    **    iFreeBlk:   First freeblock after iStart, or zero if none
    **    iPtr:       The address of a pointer to iFreeBlk
    **
    ** Check to see if iFreeBlk should be coalesced onto the end of iStart.
    */
    ਜੇ( iFreeBlk && i_ਸਿਰਾ+3>=iFreeBlk ){
      nFrag = iFreeBlk - i_ਸਿਰਾ;
      ਜੇ( i_ਸਿਰਾ>iFreeBlk ) ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
      i_ਸਿਰਾ = iFreeBlk + get2byte(&ਅੰਕੜੇ[iFreeBlk+2]);
      ਜੇ( i_ਸਿਰਾ > (ਪੂਰਨ)p_ਪੰਨਾ->pBt->usable_ਮਾਪ ){
        ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
      }
      i_ਮਾਪ = i_ਸਿਰਾ - i_ਸ਼ੁਰੂ;
      iFreeBlk = get2byte(&ਅੰਕੜੇ[iFreeBlk]);
    }
 
    /* If iPtr is another freeblock (that is, if iPtr is not the freelist
    ** pointer in the page header) then check to see if iStart should be
    ** coalesced onto the end of iPtr.
    */
    ਜੇ( iPtr>hdr+1 ){
      ਪੂਰਨ i_Ptr_ਸਿਰਾ = iPtr + get2byte(&ਅੰਕੜੇ[iPtr+2]);
      ਜੇ( i_Ptr_ਸਿਰਾ+3>=i_ਸ਼ੁਰੂ ){
        ਜੇ( i_Ptr_ਸਿਰਾ>i_ਸ਼ੁਰੂ ) ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
        nFrag += i_ਸ਼ੁਰੂ - i_Ptr_ਸਿਰਾ;
        i_ਮਾਪ = i_ਸਿਰਾ - iPtr;
        i_ਸ਼ੁਰੂ = iPtr;
      }
    }
    ਜੇ( nFrag>ਅੰਕੜੇ[hdr+7] ) ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    ਅੰਕੜੇ[hdr+7] -= (u8)nFrag;
  }
  pTmp = &ਅੰਕੜੇ[hdr+5];
  x = get2byte(pTmp);
  ਜੇ( p_ਪੰਨਾ->pBt->btsFlags & BTS_FAST_SECURE ){
    /* Overwrite deleted information with zeros when the secure_delete
    ** option is enabled */
    memset(&ਅੰਕੜੇ[i_ਸ਼ੁਰੂ], 0, i_ਮਾਪ);
  }
  ਜੇ( i_ਸ਼ੁਰੂ<=x ){
    /* The new freeblock is at the beginning of the cell content area,
    ** so just extend the cell content area rather than create another
    ** freelist entry */
    ਜੇ( i_ਸ਼ੁਰੂ<x ) ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    ਜੇ( iPtr!=hdr+1 ) ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    put2byte(&ਅੰਕੜੇ[hdr+1], iFreeBlk);
    put2byte(&ਅੰਕੜੇ[hdr+5], i_ਸਿਰਾ);
  }ਨਹੀਂਤਾਂ{
    /* Insert the new freeblock into the freelist */
    put2byte(&ਅੰਕੜੇ[iPtr], i_ਸ਼ੁਰੂ);
    put2byte(&ਅੰਕੜੇ[i_ਸ਼ੁਰੂ], iFreeBlk);
    ਪ੍ਰਤਿਜ੍ਞਾ( i_ਮਾਪ>=0 && i_ਮਾਪ<=0xffff );
    put2byte(&ਅੰਕੜੇ[i_ਸ਼ੁਰੂ+2], (u16)i_ਮਾਪ);
  }
  p_ਪੰਨਾ->nFree += i_Orig_ਮਾਪ;
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ decodeFlags(Mem_ਪੰਨਾ *p_ਪੰਨਾ, ਪੂਰਨ flagByte){
  BtShared *pBt;     /* A copy of pPage->pBt */

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->hdr_ਵਿੱਥ==(p_ਪੰਨਾ->pgno==1 ? 100 : 0) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  pBt = p_ਪੰਨਾ->pBt;
  p_ਪੰਨਾ->max1bytePayload = pBt->max1bytePayload;
  ਜੇ( flagByte>=(PTF_ZERODATA | PTF_ਪੱਤੀ) ){
    p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ = 0;
    p_ਪੰਨਾ->ਪੱਤੀ = 1;
    ਜੇ( flagByte==(PTF_LEAFDATA | PTF_INTKEY | PTF_ਪੱਤੀ) ){
      p_ਪੰਨਾ->int_ਕੁੰਜੀ_ਪੱਤੀ = 1;
      p_ਪੰਨਾ->x_Cell_ਮਾਪ = cell_ਮਾਪ_Ptr_ਸਾਰਣੀ_ਪੱਤੀ;
      p_ਪੰਨਾ->xParseCell = btreeParseCellPtr;
      p_ਪੰਨਾ->int_ਕੁੰਜੀ = 1;
      p_ਪੰਨਾ->maxLocal = pBt->max_ਪੱਤੀ;
      p_ਪੰਨਾ->minLocal = pBt->min_ਪੱਤੀ;
    }ਨਹੀਂਤਾਂ ਜੇ( flagByte==(PTF_ZERODATA | PTF_ਪੱਤੀ) ){
      p_ਪੰਨਾ->int_ਕੁੰਜੀ = 0;
      p_ਪੰਨਾ->int_ਕੁੰਜੀ_ਪੱਤੀ = 0;
      p_ਪੰਨਾ->x_Cell_ਮਾਪ = cell_ਮਾਪ_Ptr_Idx_ਪੱਤੀ;
      p_ਪੰਨਾ->xParseCell = btree_Parse_Cell_Ptr_ਸੂਚਕਾਂਕ;
      p_ਪੰਨਾ->maxLocal = pBt->maxLocal;
      p_ਪੰਨਾ->minLocal = pBt->minLocal;
    }ਨਹੀਂਤਾਂ{
      p_ਪੰਨਾ->int_ਕੁੰਜੀ = 0;
      p_ਪੰਨਾ->int_ਕੁੰਜੀ_ਪੱਤੀ = 0;
      p_ਪੰਨਾ->x_Cell_ਮਾਪ = cell_ਮਾਪ_Ptr_Idx_ਪੱਤੀ;
      p_ਪੰਨਾ->xParseCell = btree_Parse_Cell_Ptr_ਸੂਚਕਾਂਕ;
      ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }
  }ਨਹੀਂਤਾਂ{
    p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ = 4;
    p_ਪੰਨਾ->ਪੱਤੀ = 0;
    ਜੇ( flagByte==(PTF_ZERODATA) ){
      p_ਪੰਨਾ->int_ਕੁੰਜੀ = 0;
      p_ਪੰਨਾ->int_ਕੁੰਜੀ_ਪੱਤੀ = 0;
      p_ਪੰਨਾ->x_Cell_ਮਾਪ = cell_ਮਾਪ_Ptr;
      p_ਪੰਨਾ->xParseCell = btree_Parse_Cell_Ptr_ਸੂਚਕਾਂਕ;
      p_ਪੰਨਾ->maxLocal = pBt->maxLocal;
      p_ਪੰਨਾ->minLocal = pBt->minLocal;
    }ਨਹੀਂਤਾਂ ਜੇ( flagByte==(PTF_LEAFDATA | PTF_INTKEY) ){
      p_ਪੰਨਾ->int_ਕੁੰਜੀ_ਪੱਤੀ = 0;
      p_ਪੰਨਾ->x_Cell_ਮਾਪ = cell_ਮਾਪ_Ptr_No_Payload;
      p_ਪੰਨਾ->xParseCell = btreeParseCellPtrNoPayload;
      p_ਪੰਨਾ->int_ਕੁੰਜੀ = 1;
      p_ਪੰਨਾ->maxLocal = pBt->max_ਪੱਤੀ;
      p_ਪੰਨਾ->minLocal = pBt->min_ਪੱਤੀ;
    }ਨਹੀਂਤਾਂ{
      p_ਪੰਨਾ->int_ਕੁੰਜੀ = 0;
      p_ਪੰਨਾ->int_ਕੁੰਜੀ_ਪੱਤੀ = 0;
      p_ਪੰਨਾ->x_Cell_ਮਾਪ = cell_ਮਾਪ_Ptr;
      p_ਪੰਨਾ->xParseCell = btree_Parse_Cell_Ptr_ਸੂਚਕਾਂਕ;
      ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Compute the amount of freespace on the page.  In other words, fill
** in the pPage->nFree field.
*/
ਸਥਿਰ ਪੂਰਨ btreeComputeFreeSpace(Mem_ਪੰਨਾ *p_ਪੰਨਾ){
  ਪੂਰਨ pc;            /* Address of a freeblock within pPage->aData[] */
  u8 hdr;            /* Offset to beginning of page header */
  u8 *ਅੰਕੜੇ;          /* Equal to pPage->aData */
  ਪੂਰਨ usable_ਮਾਪ;    /* Amount of usable space on each page */
  ਪੂਰਨ nFree;         /* Number of unused bytes on the page */
  ਪੂਰਨ ਸਿਖਰ;           /* First byte of the cell content area */
  ਪੂਰਨ i_Cell_ਪਹਿਲਾ;    /* First allowable cell or freeblock offset */
  ਪੂਰਨ i_Cell_ਆਖਰੀ;     /* Last possible cell or freeblock offset */

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pBt!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pBt->db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pgno==sqlite3PagerPagenumber(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ == sqlite3PagerGetExtra(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->a_ਅੰਕੜੇ == sqlite3_Pager_Get_ਅੰਕੜੇ(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->isInit==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nFree<0 );

  usable_ਮਾਪ = p_ਪੰਨਾ->pBt->usable_ਮਾਪ;
  hdr = p_ਪੰਨਾ->hdr_ਵਿੱਥ;
  ਅੰਕੜੇ = p_ਪੰਨਾ->a_ਅੰਕੜੇ;
  /* EVIDENCE-OF: R-58015-48175 The two-byte integer at offset 5 designates
  ** the start of the cell content area. A zero value for this integer is
  ** interpreted as 65536. */
  ਸਿਖਰ = get2byteNotZero(&ਅੰਕੜੇ[hdr+5]);
  i_Cell_ਪਹਿਲਾ = hdr + 8 + p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ + 2*p_ਪੰਨਾ->nCell;
  i_Cell_ਆਖਰੀ = usable_ਮਾਪ - 4;

  /* Compute the total free space on the page
  ** EVIDENCE-OF: R-23588-34450 The two-byte integer at offset 1 gives the
  ** start of the first freeblock on the page, or is zero if there are no
  ** freeblocks. */
  pc = get2byte(&ਅੰਕੜੇ[hdr+1]);
  nFree = ਅੰਕੜੇ[hdr+7] + ਸਿਖਰ;  /* Init nFree to non-freeblock free space */
  ਜੇ( pc>0 ){
    u32 ਅਗਲਾ, ਮਾਪ;
    ਜੇ( pc<ਸਿਖਰ ){
      /* EVIDENCE-OF: R-55530-52930 In a well-formed b-tree page, there will
      ** always be at least one cell before the first freeblock.
      */
      ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }
    ਜਦੋਂ( 1 ){
      ਜੇ( pc>i_Cell_ਆਖਰੀ ){
        /* Freeblock off the end of the page */
        ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
      }
      ਅਗਲਾ = get2byte(&ਅੰਕੜੇ[pc]);
      ਮਾਪ = get2byte(&ਅੰਕੜੇ[pc+2]);
      ਜੇ( ਮਾਪ<4 && sqlite3FaultSim(422)==SQLITE_OK ){
        /* Minimum freeblock size is 4.  Enable fault-sim 422 to disable this
        ** check to reach interesting error states.  However, disabling this
        ** check can cause assertion faults due to min-heap overflow.  All
        ** fault-sims are for testing use only, but this one especially so. */
        ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
      }
      nFree = nFree + ਮਾਪ;
      ਜੇ( ਅਗਲਾ<pc+ਮਾਪ+4 ) ਰੋਕੋ;
      pc = ਅਗਲਾ;
    }
    ਜੇ( ਅਗਲਾ>0 ){
      /* Freeblock not in ascending order */
      ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }
    ਜੇ( pc+ਮਾਪ>(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)usable_ਮਾਪ ){
      /* Last freeblock extends past page end */
      ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }
  }

  /* At this point, nFree contains the sum of the offset to the start
  ** of the cell-content area plus the number of free bytes within
  ** the cell-content area. If this is greater than the usable-size
  ** of the page, then the page must be corrupted. This check also
  ** serves to verify that the offset to the start of the cell-content
  ** area, according to the page header, lies within the page.
  */
  ਜੇ( nFree>usable_ਮਾਪ || nFree<i_Cell_ਪਹਿਲਾ ){
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
  }
  p_ਪੰਨਾ->nFree = (u16)(nFree - i_Cell_ਪਹਿਲਾ);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Do additional sanity check after btreeInitPage() if
** PRAGMA cell_size_check=ON
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ btree_Cell_ਮਾਪ_Check(Mem_ਪੰਨਾ *p_ਪੰਨਾ){
  ਪੂਰਨ i_Cell_ਪਹਿਲਾ;    /* First allowable cell or freeblock offset */
  ਪੂਰਨ i_Cell_ਆਖਰੀ;     /* Last possible cell or freeblock offset */
  ਪੂਰਨ i;             /* Index into the cell pointer array */
  ਪੂਰਨ sz;            /* Size of a cell */
  ਪੂਰਨ pc;            /* Address of a freeblock within pPage->aData[] */
  u8 *ਅੰਕੜੇ;          /* Equal to pPage->aData */
  ਪੂਰਨ usable_ਮਾਪ;    /* Maximum usable space on the page */
  ਪੂਰਨ cell_ਵਿੱਥ;    /* Start of cell content area */

  i_Cell_ਪਹਿਲਾ = p_ਪੰਨਾ->cell_ਵਿੱਥ + 2*p_ਪੰਨਾ->nCell;
  usable_ਮਾਪ = p_ਪੰਨਾ->pBt->usable_ਮਾਪ;
  i_Cell_ਆਖਰੀ = usable_ਮਾਪ - 4;
  ਅੰਕੜੇ = p_ਪੰਨਾ->a_ਅੰਕੜੇ;
  cell_ਵਿੱਥ = p_ਪੰਨਾ->cell_ਵਿੱਥ;
  ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ) i_Cell_ਆਖਰੀ--;
  ਲਈ(i=0; i<p_ਪੰਨਾ->nCell; i++){
    pc = get2byteAligned(&ਅੰਕੜੇ[cell_ਵਿੱਥ+i*2]);
    testcase( pc==i_Cell_ਪਹਿਲਾ );
    testcase( pc==i_Cell_ਆਖਰੀ );
    ਜੇ( pc<i_Cell_ਪਹਿਲਾ || pc>i_Cell_ਆਖਰੀ ){
      ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }
    sz = p_ਪੰਨਾ->x_Cell_ਮਾਪ(p_ਪੰਨਾ, &ਅੰਕੜੇ[pc]);
    testcase( pc+sz==usable_ਮਾਪ );
    ਜੇ( pc+sz>usable_ਮਾਪ ){
      ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }
  }
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ btree_Init_ਪੰਨਾ(Mem_ਪੰਨਾ *p_ਪੰਨਾ){
  u8 *ਅੰਕੜੇ;          /* Equal to pPage->aData */
  BtShared *pBt;        /* The main btree structure */

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pBt!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pBt->db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pgno==sqlite3PagerPagenumber(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ == sqlite3PagerGetExtra(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->a_ਅੰਕੜੇ == sqlite3_Pager_Get_ਅੰਕੜੇ(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->isInit==0 );

  pBt = p_ਪੰਨਾ->pBt;
  ਅੰਕੜੇ = p_ਪੰਨਾ->a_ਅੰਕੜੇ + p_ਪੰਨਾ->hdr_ਵਿੱਥ;
  /* EVIDENCE-OF: R-28594-02890 The one-byte flag at offset 0 indicating
  ** the b-tree page type. */
  ਜੇ( decodeFlags(p_ਪੰਨਾ, ਅੰਕੜੇ[0]) ){
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->ਪੰਨਾ_ਮਾਪ>=512 && pBt->ਪੰਨਾ_ਮਾਪ<=65536 );
  p_ਪੰਨਾ->ਨਕਾਬ_ਪੰਨਾ = (u16)(pBt->ਪੰਨਾ_ਮਾਪ - 1);
  p_ਪੰਨਾ->n_ਉਛਾਲ = 0;
  p_ਪੰਨਾ->cell_ਵਿੱਥ = (u16)(p_ਪੰਨਾ->hdr_ਵਿੱਥ + 8 + p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ);
  p_ਪੰਨਾ->aCellIdx = ਅੰਕੜੇ + p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ + 8;
  p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ = p_ਪੰਨਾ->a_ਅੰਕੜੇ + pBt->ਪੰਨਾ_ਮਾਪ;
  p_ਪੰਨਾ->a_ਅੰਕੜੇ_Ofst = p_ਪੰਨਾ->a_ਅੰਕੜੇ + p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ;
  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  p_ਪੰਨਾ->nCell = get2byte(&ਅੰਕੜੇ[3]);
  ਜੇ( p_ਪੰਨਾ->nCell>MX_CELL(pBt) ){
    /* To many cells for a single page.  The page must be corrupt */
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
  }
  testcase( p_ਪੰਨਾ->nCell==MX_CELL(pBt) );
  /* EVIDENCE-OF: R-24089-57979 If a page contains no cells (which is only
  ** possible for a root page of a table that contains no rows) then the
  ** offset to the cell content area will equal the page size minus the
  ** bytes of reserved space. */
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nCell>0
       || get2byteNotZero(&ਅੰਕੜੇ[5])==(ਪੂਰਨ)pBt->usable_ਮਾਪ
       || CORRUPT_DB );
  p_ਪੰਨਾ->nFree = -1;  /* Indicate that this value is yet uncomputed */
  p_ਪੰਨਾ->isInit = 1;
  ਜੇ( pBt->db->flags & SQLITE_Cell_ਮਾਪ_Ck ){
    ਵਾਪਸ btree_Cell_ਮਾਪ_Check(p_ਪੰਨਾ);
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Set up a raw page so that it looks like a database page holding
** no entries.
*/
ਸਥਿਰ ਖਾਲੀ zero_ਪੰਨਾ(Mem_ਪੰਨਾ *p_ਪੰਨਾ, ਪੂਰਨ flags){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅੰਕੜੇ = p_ਪੰਨਾ->a_ਅੰਕੜੇ;
  BtShared *pBt = p_ਪੰਨਾ->pBt;
  ਪੂਰਨ hdr = p_ਪੰਨਾ->hdr_ਵਿੱਥ;
  ਪੂਰਨ ਪਹਿਲਾ;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerPagenumber(p_ਪੰਨਾ->p_Db_ਪੰਨਾ)==p_ਪੰਨਾ->pgno || CORRUPT_DB );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerGetExtra(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) == (ਖਾਲੀ*)p_ਪੰਨਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pager_Get_ਅੰਕੜੇ(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) == ਅੰਕੜੇ );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਜੇ( pBt->btsFlags & BTS_FAST_SECURE ){
    memset(&ਅੰਕੜੇ[hdr], 0, pBt->usable_ਮਾਪ - hdr);
  }
  ਅੰਕੜੇ[hdr] = (ਅੱਖਰ)flags;
  ਪਹਿਲਾ = hdr + ((flags&PTF_ਪੱਤੀ)==0 ? 12 : 8);
  memset(&ਅੰਕੜੇ[hdr+1], 0, 4);
  ਅੰਕੜੇ[hdr+7] = 0;
  put2byte(&ਅੰਕੜੇ[hdr+5], pBt->usable_ਮਾਪ);
  p_ਪੰਨਾ->nFree = (u16)(pBt->usable_ਮਾਪ - ਪਹਿਲਾ);
  decodeFlags(p_ਪੰਨਾ, flags);
  p_ਪੰਨਾ->cell_ਵਿੱਥ = (u16)ਪਹਿਲਾ;
  p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ = &ਅੰਕੜੇ[pBt->ਪੰਨਾ_ਮਾਪ];
  p_ਪੰਨਾ->aCellIdx = &ਅੰਕੜੇ[ਪਹਿਲਾ];
  p_ਪੰਨਾ->a_ਅੰਕੜੇ_Ofst = &ਅੰਕੜੇ[p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ];
  p_ਪੰਨਾ->n_ਉਛਾਲ = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->ਪੰਨਾ_ਮਾਪ>=512 && pBt->ਪੰਨਾ_ਮਾਪ<=65536 );
  p_ਪੰਨਾ->ਨਕਾਬ_ਪੰਨਾ = (u16)(pBt->ਪੰਨਾ_ਮਾਪ - 1);
  p_ਪੰਨਾ->nCell = 0;
  p_ਪੰਨਾ->isInit = 1;
}


/*
** Convert a DbPage obtained from the pager into a MemPage used by
** the btree layer.
*/
ਸਥਿਰ Mem_ਪੰਨਾ *btree_ਪੰਨਾ_From_Db_ਪੰਨਾ(Db_ਪੰਨਾ *p_Db_ਪੰਨਾ, Pgno pgno, BtShared *pBt){
  Mem_ਪੰਨਾ *p_ਪੰਨਾ = (Mem_ਪੰਨਾ*)sqlite3PagerGetExtra(p_Db_ਪੰਨਾ);
  ਜੇ( pgno!=p_ਪੰਨਾ->pgno ){
    p_ਪੰਨਾ->a_ਅੰਕੜੇ = sqlite3_Pager_Get_ਅੰਕੜੇ(p_Db_ਪੰਨਾ);
    p_ਪੰਨਾ->p_Db_ਪੰਨਾ = p_Db_ਪੰਨਾ;
    p_ਪੰਨਾ->pBt = pBt;
    p_ਪੰਨਾ->pgno = pgno;
    p_ਪੰਨਾ->hdr_ਵਿੱਥ = pgno==1 ? 100 : 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->a_ਅੰਕੜੇ==sqlite3_Pager_Get_ਅੰਕੜੇ(p_Db_ਪੰਨਾ) );
  ਵਾਪਸ p_ਪੰਨਾ;
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
ਸਥਿਰ ਪੂਰਨ btree_Get_ਪੰਨਾ(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  Mem_ਪੰਨਾ **pp_ਪੰਨਾ,    /* Return the page in this parameter */
  ਪੂਰਨ flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  ਪੂਰਨ rc;
  Db_ਪੰਨਾ *p_Db_ਪੰਨਾ;

  ਪ੍ਰਤਿਜ੍ਞਾ( flags==0 || flags==PAGER_GET_NOCONTENT || flags==PAGER_GET_READONLY );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  rc = sqlite3PagerGet(pBt->pPager, pgno, (Db_ਪੰਨਾ**)&p_Db_ਪੰਨਾ, flags);
  ਜੇ( rc ) ਵਾਪਸ rc;
  *pp_ਪੰਨਾ = btree_ਪੰਨਾ_From_Db_ਪੰਨਾ(p_Db_ਪੰਨਾ, pgno, pBt);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Retrieve a page from the pager cache. If the requested page is not
** already in the pager cache return NULL. Initialize the MemPage.pBt and
** MemPage.aData elements if needed.
*/
ਸਥਿਰ Mem_ਪੰਨਾ *btree_ਪੰਨਾ_Lookup(BtShared *pBt, Pgno pgno){
  Db_ਪੰਨਾ *p_Db_ਪੰਨਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  p_Db_ਪੰਨਾ = sqlite3PagerLookup(pBt->pPager, pgno);
  ਜੇ( p_Db_ਪੰਨਾ ){
    ਵਾਪਸ btree_ਪੰਨਾ_From_Db_ਪੰਨਾ(p_Db_ਪੰਨਾ, pgno, pBt);
  }
  ਵਾਪਸ 0;
}

/*
** Return the size of the database file in pages. If there is any kind of
** error, return ((unsigned int)-1).
*/
ਸਥਿਰ Pgno btreePagecount(BtShared *pBt){
  ਵਾਪਸ pBt->n_ਪੰਨਾ;
}
Pgno sqlite3_Btree_ਆਖਰੀ_ਪੰਨਾ(Btree *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsMutex(p) );
  ਵਾਪਸ btreePagecount(p->pBt);
}

/*
** Get a page from the pager and initialize it.
*/
ਸਥਿਰ ਪੂਰਨ get_And_Init_ਪੰਨਾ(
  BtShared *pBt,                  /* The database file */
  Pgno pgno,                      /* Number of the page to get */
  Mem_ਪੰਨਾ **pp_ਪੰਨਾ,               /* Write the page pointer here */
  ਪੂਰਨ bReadOnly                   /* True for a read-only page */
){
  ਪੂਰਨ rc;
  Db_ਪੰਨਾ *p_Db_ਪੰਨਾ;
  Mem_ਪੰਨਾ *p_ਪੰਨਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );

  ਜੇ( pgno>btreePagecount(pBt) ){
    *pp_ਪੰਨਾ = 0;
    ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  }
  rc = sqlite3PagerGet(pBt->pPager, pgno, (Db_ਪੰਨਾ**)&p_Db_ਪੰਨਾ, bReadOnly);
  ਜੇ( rc ){
    *pp_ਪੰਨਾ = 0;
    ਵਾਪਸ rc;
  }
  p_ਪੰਨਾ = (Mem_ਪੰਨਾ*)sqlite3PagerGetExtra(p_Db_ਪੰਨਾ);
  ਜੇ( p_ਪੰਨਾ->isInit==0 ){
    btree_ਪੰਨਾ_From_Db_ਪੰਨਾ(p_Db_ਪੰਨਾ, pgno, pBt);
    rc = btree_Init_ਪੰਨਾ(p_ਪੰਨਾ);
    ਜੇ( rc!=SQLITE_OK ){
      release_ਪੰਨਾ(p_ਪੰਨਾ);
      *pp_ਪੰਨਾ = 0;
      ਵਾਪਸ rc;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pgno==pgno || CORRUPT_DB );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->a_ਅੰਕੜੇ==sqlite3_Pager_Get_ਅੰਕੜੇ(p_Db_ਪੰਨਾ) );
  *pp_ਪੰਨਾ = p_ਪੰਨਾ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Release a MemPage.  This should be called once for each prior
** call to btreeGetPage.
**
** Page1 is a special case and must be released using releasePageOne().
*/
ਸਥਿਰ ਖਾਲੀ release_ਪੰਨਾ_Not_Null(Mem_ਪੰਨਾ *p_ਪੰਨਾ){
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->a_ਅੰਕੜੇ );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pBt );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->p_Db_ਪੰਨਾ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerGetExtra(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) == (ਖਾਲੀ*)p_ਪੰਨਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pager_Get_ਅੰਕੜੇ(p_ਪੰਨਾ->p_Db_ਪੰਨਾ)==p_ਪੰਨਾ->a_ਅੰਕੜੇ );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  sqlite3PagerUnrefNotNull(p_ਪੰਨਾ->p_Db_ਪੰਨਾ);
}
ਸਥਿਰ ਖਾਲੀ release_ਪੰਨਾ(Mem_ਪੰਨਾ *p_ਪੰਨਾ){
  ਜੇ( p_ਪੰਨਾ ) release_ਪੰਨਾ_Not_Null(p_ਪੰਨਾ);
}
ਸਥਿਰ ਖਾਲੀ release_ਪੰਨਾ_One(Mem_ਪੰਨਾ *p_ਪੰਨਾ){
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->a_ਅੰਕੜੇ );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pBt );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->p_Db_ਪੰਨਾ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerGetExtra(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) == (ਖਾਲੀ*)p_ਪੰਨਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pager_Get_ਅੰਕੜੇ(p_ਪੰਨਾ->p_Db_ਪੰਨਾ)==p_ਪੰਨਾ->a_ਅੰਕੜੇ );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  sqlite3_Pager_Unref_ਪੰਨਾ_One(p_ਪੰਨਾ->p_Db_ਪੰਨਾ);
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
ਸਥਿਰ ਪੂਰਨ btree_Get_ਅਣਵਰਤਿਆ_ਪੰਨਾ(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  Mem_ਪੰਨਾ **pp_ਪੰਨਾ,    /* Return the page in this parameter */
  ਪੂਰਨ flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  ਪੂਰਨ rc = btree_Get_ਪੰਨਾ(pBt, pgno, pp_ਪੰਨਾ, flags);
  ਜੇ( rc==SQLITE_OK ){
    ਜੇ( sqlite3_Pager_ਪੰਨਾ_Refcount((*pp_ਪੰਨਾ)->p_Db_ਪੰਨਾ)>1 ){
      release_ਪੰਨਾ(*pp_ਪੰਨਾ);
      *pp_ਪੰਨਾ = 0;
      ਵਾਪਸ SQLITE_CORRUPT_BKPT;
    }
    (*pp_ਪੰਨਾ)->isInit = 0;
  }ਨਹੀਂਤਾਂ{
    *pp_ਪੰਨਾ = 0;
  }
  ਵਾਪਸ rc;
}


/*
** During a rollback, when the pager reloads information into the cache
** so that the cache is restored to its original state at the start of
** the transaction, for each page restored this routine is called.
**
** This routine needs to reset the extra data section at the end of the
** page to agree with the restored data.
*/
ਸਥਿਰ ਖਾਲੀ ਪੰਨਾ_Reinit(Db_ਪੰਨਾ *p_ਅੰਕੜੇ){
  Mem_ਪੰਨਾ *p_ਪੰਨਾ;
  p_ਪੰਨਾ = (Mem_ਪੰਨਾ *)sqlite3PagerGetExtra(p_ਅੰਕੜੇ);
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pager_ਪੰਨਾ_Refcount(p_ਅੰਕੜੇ)>0 );
  ਜੇ( p_ਪੰਨਾ->isInit ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
    p_ਪੰਨਾ->isInit = 0;
    ਜੇ( sqlite3_Pager_ਪੰਨਾ_Refcount(p_ਅੰਕੜੇ)>1 ){
      /* pPage might not be a btree page;  it might be an overflow page
      ** or ptrmap page or a free page.  In those cases, the following
      ** call to btreeInitPage() will likely return SQLITE_CORRUPT.
      ** But no harm is done by this.  And it is very important that
      ** btreeInitPage() be called on every btree page so we make
      ** the call for every page that comes in for re-initializing. */
      btree_Init_ਪੰਨਾ(p_ਪੰਨਾ);
    }
  }
}

/*
** Invoke the busy handler for a btree.
*/
ਸਥਿਰ ਪੂਰਨ btreeInvokeBusyHandler(ਖਾਲੀ *pArg){
  BtShared *pBt = (BtShared*)pArg;
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->db );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->db->mutex) );
  ਵਾਪਸ sqlite3InvokeBusyHandler(&pBt->db->busyHandler);
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
ਪੂਰਨ sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,      /* VFS to use for this b-tree */
  ਅਚਲ ਅੱਖਰ *zFilename,  /* Name of the file containing the BTree database */
  sqlite3 *db,            /* Associated database handle */
  Btree **ppBtree,        /* Pointer to new Btree object written here */
  ਪੂਰਨ flags,              /* Options */
  ਪੂਰਨ vfsFlags            /* Flags passed through to sqlite3_vfs.xOpen() */
){
  BtShared *pBt = 0;             /* Shared part of btree structure */
  Btree *p;                      /* Handle to return */
  sqlite3_mutex *mutexOpen = 0;  /* Prevents a race condition. Ticket #3537 */
  ਪੂਰਨ rc = SQLITE_OK;            /* Result code from this function */
  u8 nReserve;                   /* Byte of unused space on each page */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ z_Db_ਸਿਰਲੇਖ[100];  /* Database header content */

  /* True if opening an ephemeral, temporary database */
  ਅਚਲ ਪੂਰਨ is_ਆਰਜ਼ੀ_Db = zFilename==0 || zFilename[0]==0;

  /* Set the variable isMemdb to true for an in-memory database, or
  ** false for a file-based database.
  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_MEMORYDB
  ਅਚਲ ਪੂਰਨ isMemdb = 0;
#ਨਹੀਂਤਾਂ
  ਅਚਲ ਪੂਰਨ isMemdb = (zFilename && strcmp(zFilename, ":memory:")==0)
                       || (is_ਆਰਜ਼ੀ_Db && sqlite3_ਆਰਜ਼ੀ_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ(db))
                       || (vfsFlags & SQLITE_OPEN_ਸਮ੍ਰਿਤੀ)!=0;
#ਜੇ_ਅੰਤ

  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pVfs!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( (flags&0xff)==flags );   /* flags fit in 8 bits */

  /* Only a BTREE_SINGLE database can be BTREE_UNORDERED */
  ਪ੍ਰਤਿਜ੍ਞਾ( (flags & BTREE_UNORDERED)==0 || (flags & BTREE_SINGLE)!=0 );

  /* A BTREE_SINGLE database is always a temporary and/or ephemeral */
  ਪ੍ਰਤਿਜ੍ਞਾ( (flags & BTREE_SINGLE)==0 || is_ਆਰਜ਼ੀ_Db );

  ਜੇ( isMemdb ){
    flags |= BTREE_ਸਮ੍ਰਿਤੀ;
  }
  ਜੇ( (vfsFlags & SQLITE_OPEN_MAIN_DB)!=0 && (isMemdb || is_ਆਰਜ਼ੀ_Db) ){
    vfsFlags = (vfsFlags & ~SQLITE_OPEN_MAIN_DB) | SQLITE_OPEN_ਆਰਜ਼ੀ_DB;
  }
  p = sqlite3MallocZero(ਆਕਾਰ(Btree));
  ਜੇ( !p ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  p->ਵਿੱਚ_Trans = TRANS_NONE;
  p->db = db;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
  p->ਤਾਲਾ.pBtree = p;
  p->ਤਾਲਾ.i_ਸਾਰਣੀ = 1;
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
  /*
  ** If this Btree is a candidate for shared cache, try to find an
  ** existing BtShared object that we can share with
  */
  ਜੇ( is_ਆਰਜ਼ੀ_Db==0 && (isMemdb==0 || (vfsFlags&SQLITE_OPEN_URI)!=0) ){
    ਜੇ( vfsFlags & SQLITE_OPEN_SHAREDCACHE ){
      ਪੂਰਨ nFilename = sqlite3Strlen30(zFilename)+1;
      ਪੂਰਨ n_ਭਰਿਆ_Pathname = pVfs->mxPathname+1;
      ਅੱਖਰ *z_ਭਰਿਆ_Pathname = sqlite3Malloc(MAX(n_ਭਰਿਆ_Pathname,nFilename));
      MUTEX_LOGIC( sqlite3_mutex *mutexShared; )

      p->sharable = 1;
      ਜੇ( !z_ਭਰਿਆ_Pathname ){
        sqlite3_free(p);
        ਵਾਪਸ SQLITE_NOMEM_BKPT;
      }
      ਜੇ( isMemdb ){
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_ਭਰਿਆ_Pathname, zFilename, nFilename);
      }ਨਹੀਂਤਾਂ{
        rc = sqlite3_Os_ਭਰਿਆ_Pathname(pVfs, zFilename,
                                   n_ਭਰਿਆ_Pathname, z_ਭਰਿਆ_Pathname);
        ਜੇ( rc ){
          ਜੇ( rc==SQLITE_OK_SYMLINK ){
            rc = SQLITE_OK;
          }ਨਹੀਂਤਾਂ{
            sqlite3_free(z_ਭਰਿਆ_Pathname);
            sqlite3_free(p);
            ਵਾਪਸ rc;
          }
        }
      }
#ਜੇ SQLITE_THREADSAFE
      mutexOpen = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_OPEN);
      sqlite3_mutex_enter(mutexOpen);
      mutexShared = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
      sqlite3_mutex_enter(mutexShared);
#ਜੇ_ਅੰਤ
      ਲਈ(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->p_ਅਗਲਾ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pBt->nRef>0 );
        ਜੇ( 0==strcmp(z_ਭਰਿਆ_Pathname, sqlite3PagerFilename(pBt->pPager, 0))
                 && sqlite3PagerVfs(pBt->pPager)==pVfs ){
          ਪੂਰਨ iDb;
          ਲਈ(iDb=db->nDb-1; iDb>=0; iDb--){
            Btree *pExisting = db->aDb[iDb].pBt;
            ਜੇ( pExisting && pExisting->pBt==pBt ){
              sqlite3_mutex_leave(mutexShared);
              sqlite3_mutex_leave(mutexOpen);
              sqlite3_free(z_ਭਰਿਆ_Pathname);
              sqlite3_free(p);
              ਵਾਪਸ SQLITE_CONSTRAINT;
            }
          }
          p->pBt = pBt;
          pBt->nRef++;
          ਰੋਕੋ;
        }
      }
      sqlite3_mutex_leave(mutexShared);
      sqlite3_free(z_ਭਰਿਆ_Pathname);
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    ਨਹੀਂਤਾਂ{
      /* In debug mode, we mark all persistent databases as sharable
      ** even when they are not.  This exercises the locking code and
      ** gives more opportunity for asserts(sqlite3_mutex_held())
      ** statements to find locking problems.
      */
      p->sharable = 1;
    }
#ਜੇ_ਅੰਤ
  }
#ਜੇ_ਅੰਤ
  ਜੇ( pBt==0 ){
    /*
    ** The following asserts make sure that structures used by the btree are
    ** the right size.  This is to guard against size changes that result
    ** when compiling on a different architecture.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(i64)==8 );
    ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(u64)==8 );
    ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(u32)==4 );
    ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(u16)==2 );
    ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(Pgno)==4 );

    /* Suppress false-positive compiler warning from PVS-Studio */
    memset(&z_Db_ਸਿਰਲੇਖ[16], 0, 8);
 
    pBt = sqlite3MallocZero( ਆਕਾਰ(*pBt) );
    ਜੇ( pBt==0 ){
      rc = SQLITE_NOMEM_BKPT;
      ਜਾਓ btree_open_ਬਾਹਰਲਾ;
    }
    rc = sqlite3PagerOpen(pVfs, &pBt->pPager, zFilename,
                          ਆਕਾਰ(Mem_ਪੰਨਾ), flags, vfsFlags, ਪੰਨਾ_Reinit);
    ਜੇ( rc==SQLITE_OK ){
      sqlite3_Pager_ਸੈੱਟ_Mmap_ਹੱਦਬੰਦੀ(pBt->pPager, db->szMmap);
      rc = sqlite3PagerReadFileheader(pBt->pPager,ਆਕਾਰ(z_Db_ਸਿਰਲੇਖ),z_Db_ਸਿਰਲੇਖ);
    }
    ਜੇ( rc!=SQLITE_OK ){
      ਜਾਓ btree_open_ਬਾਹਰਲਾ;
    }
    pBt->openFlags = (u8)flags;
    pBt->db = db;
    sqlite3_Pager_ਸੈੱਟ_Busy_Handler(pBt->pPager, btreeInvokeBusyHandler, pBt);
    p->pBt = pBt;
 
    pBt->pCursor = 0;
    pBt->pPage1 = 0;
    ਜੇ( sqlite3PagerIsreadonly(pBt->pPager) ) pBt->btsFlags |= BTS_READ_ONLY;
#ਜੇ defined(SQLITE_SECURE_DELETE)
    pBt->btsFlags |= BTS_SECURE_DELETE;
#ਜਾਂਜੇ defined(SQLITE_FAST_SECURE_DELETE)
    pBt->btsFlags |= BTS_OVERWRITE;
#ਜੇ_ਅੰਤ
    /* EVIDENCE-OF: R-51873-39618 The page size for a database file is
    ** determined by the 2-byte integer located at an offset of 16 bytes from
    ** the beginning of the database file. */
    pBt->ਪੰਨਾ_ਮਾਪ = (z_Db_ਸਿਰਲੇਖ[16]<<8) | (z_Db_ਸਿਰਲੇਖ[17]<<16);
    ਜੇ( pBt->ਪੰਨਾ_ਮਾਪ<512 || pBt->ਪੰਨਾ_ਮਾਪ>SQLITE_MAX_ਪੰਨਾ_ਮਾਪ
         || ((pBt->ਪੰਨਾ_ਮਾਪ-1)&pBt->ਪੰਨਾ_ਮਾਪ)!=0 ){
      pBt->ਪੰਨਾ_ਮਾਪ = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
      /* If the magic name ":memory:" will create an in-memory database, then
      ** leave the autoVacuum mode at 0 (do not auto-vacuum), even if
      ** SQLITE_DEFAULT_AUTOVACUUM is true. On the other hand, if
      ** SQLITE_OMIT_MEMORYDB has been defined, then ":memory:" is just a
      ** regular file-name. In this case the auto-vacuum applies as per normal.
      */
      ਜੇ( zFilename && !isMemdb ){
        pBt->autoVacuum = (SQLITE_DEFAULT_AUTOVACUUM ? 1 : 0);
        pBt->incrVacuum = (SQLITE_DEFAULT_AUTOVACUUM==2 ? 1 : 0);
      }
#ਜੇ_ਅੰਤ
      nReserve = 0;
    }ਨਹੀਂਤਾਂ{
      /* EVIDENCE-OF: R-37497-42412 The size of the reserved region is
      ** determined by the one-byte unsigned integer found at an offset of 20
      ** into the database file header. */
      nReserve = z_Db_ਸਿਰਲੇਖ[20];
      pBt->btsFlags |= BTS_PAGESIZE_FIXED;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
      pBt->autoVacuum = (get4byte(&z_Db_ਸਿਰਲੇਖ[36 + 4*4])?1:0);
      pBt->incrVacuum = (get4byte(&z_Db_ਸਿਰਲੇਖ[36 + 7*4])?1:0);
#ਜੇ_ਅੰਤ
    }
    rc = sqlite3_Pager_ਸੈੱਟ_Pagesize(pBt->pPager, &pBt->ਪੰਨਾ_ਮਾਪ, nReserve);
    ਜੇ( rc ) ਜਾਓ btree_open_ਬਾਹਰਲਾ;
    pBt->usable_ਮਾਪ = pBt->ਪੰਨਾ_ਮਾਪ - nReserve;
    ਪ੍ਰਤਿਜ੍ਞਾ( (pBt->ਪੰਨਾ_ਮਾਪ & 7)==0 );  /* 8-byte alignment of pageSize */
  
#ਜੇ !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
    /* Add the new BtShared object to the linked list sharable BtShareds.
    */
    pBt->nRef = 1;
    ਜੇ( p->sharable ){
      MUTEX_LOGIC( sqlite3_mutex *mutexShared; )
      MUTEX_LOGIC( mutexShared = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);)
      ਜੇ( SQLITE_THREADSAFE && sqlite3GlobalConfig.bCoreMutex ){
        pBt->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_FAST);
        ਜੇ( pBt->mutex==0 ){
          rc = SQLITE_NOMEM_BKPT;
          ਜਾਓ btree_open_ਬਾਹਰਲਾ;
        }
      }
      sqlite3_mutex_enter(mutexShared);
      pBt->p_ਅਗਲਾ = GLOBAL(BtShared*,sqlite3SharedCacheList);
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt;
      sqlite3_mutex_leave(mutexShared);
    }
#ਜੇ_ਅੰਤ
  }

#ਜੇ !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
  /* If the new Btree uses a sharable pBtShared, then link the new
  ** Btree into the list of all sharable Btrees for the same connection.
  ** The list is kept in ascending order by pBt address.
  */
  ਜੇ( p->sharable ){
    ਪੂਰਨ i;
    Btree *pSib;
    ਲਈ(i=0; i<db->nDb; i++){
      ਜੇ( (pSib = db->aDb[i].pBt)!=0 && pSib->sharable ){
        ਜਦੋਂ( pSib->pPrev ){ pSib = pSib->pPrev; }
        ਜੇ( (uptr)p->pBt<(uptr)pSib->pBt ){
          p->p_ਅਗਲਾ = pSib;
          p->pPrev = 0;
          pSib->pPrev = p;
        }ਨਹੀਂਤਾਂ{
          ਜਦੋਂ( pSib->p_ਅਗਲਾ && (uptr)pSib->p_ਅਗਲਾ->pBt<(uptr)p->pBt ){
            pSib = pSib->p_ਅਗਲਾ;
          }
          p->p_ਅਗਲਾ = pSib->p_ਅਗਲਾ;
          p->pPrev = pSib;
          ਜੇ( p->p_ਅਗਲਾ ){
            p->p_ਅਗਲਾ->pPrev = p;
          }
          pSib->p_ਅਗਲਾ = p;
        }
        ਰੋਕੋ;
      }
    }
  }
#ਜੇ_ਅੰਤ
  *ppBtree = p;

btree_open_ਬਾਹਰਲਾ:
  ਜੇ( rc!=SQLITE_OK ){
    ਜੇ( pBt && pBt->pPager ){
      sqlite3PagerClose(pBt->pPager, 0);
    }
    sqlite3_free(pBt);
    sqlite3_free(p);
    *ppBtree = 0;
  }ਨਹੀਂਤਾਂ{
    sqlite3_file *pFile;

    /* If the B-Tree was successfully opened, set the pager-cache size to the
    ** default value. Except, when opening on an existing shared pager-cache,
    ** do not change the pager-cache size.
    */
    ਜੇ( sqlite3BtreeSchema(p, 0, 0)==0 ){
      sqlite3_Btree_ਸੈੱਟ_Cache_ਮਾਪ(p, SQLITE_DEFAULT_CACHE_ਮਾਪ);
    }

    pFile = sqlite3PagerFile(pBt->pPager);
    ਜੇ( pFile->pMethods ){
      sqlite3OsFileControlHint(pFile, SQLITE_FCNTL_PDB, (ਖਾਲੀ*)&pBt->db);
    }
  }
  ਜੇ( mutexOpen ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(mutexOpen) );
    sqlite3_mutex_leave(mutexOpen);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || sqlite3_Btree_Connection_ਗਿਣਤੀ(*ppBtree)>0 );
  ਵਾਪਸ rc;
}

/*
** Decrement the BtShared.nRef counter.  When it reaches zero,
** remove the BtShared structure from the sharing list.  Return
** true if the BtShared.nRef counter reaches zero and return
** false if it is still positive.
*/
ਸਥਿਰ ਪੂਰਨ removeFromSharingList(BtShared *pBt){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )
  BtShared *pList;
  ਪੂਰਨ removed = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_notheld(pBt->mutex) );
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  pBt->nRef--;
  ਜੇ( pBt->nRef<=0 ){
    ਜੇ( GLOBAL(BtShared*,sqlite3SharedCacheList)==pBt ){
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt->p_ਅਗਲਾ;
    }ਨਹੀਂਤਾਂ{
      pList = GLOBAL(BtShared*,sqlite3SharedCacheList);
      ਜਦੋਂ( ALWAYS(pList) && pList->p_ਅਗਲਾ!=pBt ){
        pList=pList->p_ਅਗਲਾ;
      }
      ਜੇ( ALWAYS(pList) ){
        pList->p_ਅਗਲਾ = pBt->p_ਅਗਲਾ;
      }
    }
    ਜੇ( SQLITE_THREADSAFE ){
      sqlite3_mutex_free(pBt->mutex);
    }
    removed = 1;
  }
  sqlite3_mutex_leave(pMainMtx);
  ਵਾਪਸ removed;
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER( pBt );
  ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ
}

/*
** Make sure pBt->pTmpSpace points to an allocation of
** MX_CELL_SIZE(pBt) bytes with a 4-byte prefix for a left-child
** pointer.
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ allocate_ਆਰਜ਼ੀ_Space(BtShared *pBt){
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->pTmpSpace==0 );
  /* This routine is called only by btreeCursor() when allocating the
  ** first write cursor for the BtShared object */
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->pCursor!=0 && (pBt->pCursor->curFlags & BTCF_WriteFlag)!=0 );
  pBt->pTmpSpace = sqlite3_ਪੰਨਾ_Malloc( pBt->ਪੰਨਾ_ਮਾਪ );
  ਜੇ( pBt->pTmpSpace==0 ){
    BtCursor *pCur = pBt->pCursor;
    pBt->pCursor = pCur->p_ਅਗਲਾ;  /* Unlink the cursor */
    memset(pCur, 0, ਆਕਾਰ(*pCur));
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
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
  ਵਾਪਸ SQLITE_OK;
}

/*
** Free the pBt->pTmpSpace allocation
*/
ਸਥਿਰ ਖਾਲੀ free_ਆਰਜ਼ੀ_Space(BtShared *pBt){
  ਜੇ( pBt->pTmpSpace ){
    pBt->pTmpSpace -= 4;
    sqlite3_ਪੰਨਾ_Free(pBt->pTmpSpace);
    pBt->pTmpSpace = 0;
  }
}

/*
** Close an open database and invalidate all cursors.
*/
ਪੂਰਨ sqlite3BtreeClose(Btree *p){
  BtShared *pBt = p->pBt;

  /* Close all cursors opened via this handle.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);

  /* Verify that no other cursors have this Btree open */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  {
    BtCursor *pCur = pBt->pCursor;
    ਜਦੋਂ( pCur ){
      BtCursor *pTmp = pCur;
      pCur = pCur->p_ਅਗਲਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( pTmp->pBtree!=p );

    }
  }
#ਜੇ_ਅੰਤ

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
  ਪ੍ਰਤਿਜ੍ਞਾ( p->want_ਮੰਜ਼ਿਲ_ਤਾਲਾ==0 && p->locked==0 );
  ਜੇ( !p->sharable || removeFromSharingList(pBt) ){
    /* The pBt is no longer on the sharing list, so we can access
    ** it without having to hold the mutex.
    **
    ** Clean out and delete the BtShared object.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( !pBt->pCursor );
    sqlite3PagerClose(pBt->pPager, p->db);
    ਜੇ( pBt->xFreeSchema && pBt->pSchema ){
      pBt->xFreeSchema(pBt->pSchema);
    }
    sqlite3DbFree(0, pBt->pSchema);
    free_ਆਰਜ਼ੀ_Space(pBt);
    sqlite3_free(pBt);
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
  ਪ੍ਰਤਿਜ੍ਞਾ( p->want_ਮੰਜ਼ਿਲ_ਤਾਲਾ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->locked==0 );
  ਜੇ( p->pPrev ) p->pPrev->p_ਅਗਲਾ = p->p_ਅਗਲਾ;
  ਜੇ( p->p_ਅਗਲਾ ) p->p_ਅਗਲਾ->pPrev = p->pPrev;
#ਜੇ_ਅੰਤ

  sqlite3_free(p);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Change the "soft" limit on the number of pages in the cache.
** Unused and unmodified pages will be recycled when the number of
** pages in the cache exceeds this soft limit.  But the size of the
** cache is allowed to grow larger than this limit if it contains
** dirty pages or pages still in active use.
*/
ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_Cache_ਮਾਪ(Btree *p, ਪੂਰਨ mx_ਪੰਨਾ){
  BtShared *pBt = p->pBt;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_ਸੈੱਟ_Cachesize(pBt->pPager, mx_ਪੰਨਾ);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ SQLITE_OK;
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
ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_Spill_ਮਾਪ(Btree *p, ਪੂਰਨ mx_ਪੰਨਾ){
  BtShared *pBt = p->pBt;
  ਪੂਰਨ res;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  res = sqlite3_Pager_ਸੈੱਟ_Spillsize(pBt->pPager, mx_ਪੰਨਾ);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ res;
}

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
/*
** Change the limit on the amount of the database file that may be
** memory mapped.
*/
ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_Mmap_ਹੱਦਬੰਦੀ(Btree *p, sqlite3_int64 szMmap){
  BtShared *pBt = p->pBt;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_ਸੈੱਟ_Mmap_ਹੱਦਬੰਦੀ(pBt->pPager, szMmap);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ /* SQLITE_MAX_MMAP_SIZE>0 */

/*
** Change the way data is synced to disk in order to increase or decrease
** how well the database resists damage due to OS crashes and power
** failures.  Level 1 is the same as asynchronous (no syncs() occur and
** there is a high probability of damage)  Level 2 is the default.  There
** is a very low but non-zero probability of damage.  Level 3 reduces the
** probability of damage to near zero but with a write performance reduction.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_PAGER_PRAGMAS
ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_Pager_Flags(
  Btree *p,              /* The btree to set the safety level on */
  ਬਿਨਾਨਿਸ਼ਾਨੀ pgFlags       /* Various PAGER_* flags */
){
  BtShared *pBt = p->pBt;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_ਸੈੱਟ_Flags(pBt->pPager, pgFlags);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ

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
ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_ਪੰਨਾ_ਮਾਪ(Btree *p, ਪੂਰਨ ਪੰਨਾ_ਮਾਪ, ਪੂਰਨ nReserve, ਪੂਰਨ iFix){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ x;
  BtShared *pBt = p->pBt;
  ਪ੍ਰਤਿਜ੍ਞਾ( nReserve>=0 && nReserve<=255 );
  sqlite3BtreeEnter(p);
  pBt->nReserveWanted = (u8)nReserve;
  x = pBt->ਪੰਨਾ_ਮਾਪ - pBt->usable_ਮਾਪ;
  ਜੇ( x==nReserve && (ਪੰਨਾ_ਮਾਪ==0 || (u32)ਪੰਨਾ_ਮਾਪ==pBt->ਪੰਨਾ_ਮਾਪ) ){
    sqlite3BtreeLeave(p);
    ਵਾਪਸ SQLITE_OK;
  }
  ਜੇ( nReserve<x ) nReserve = x;
  ਜੇ( pBt->btsFlags & BTS_PAGESIZE_FIXED ){
    sqlite3BtreeLeave(p);
    ਵਾਪਸ SQLITE_READONLY;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( nReserve>=0 && nReserve<=255 );
  ਜੇ( ਪੰਨਾ_ਮਾਪ>=512 && ਪੰਨਾ_ਮਾਪ<=SQLITE_MAX_ਪੰਨਾ_ਮਾਪ &&
        ((ਪੰਨਾ_ਮਾਪ-1)&ਪੰਨਾ_ਮਾਪ)==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( (ਪੰਨਾ_ਮਾਪ & 7)==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( !pBt->pCursor );
    ਜੇ( nReserve>32 && ਪੰਨਾ_ਮਾਪ==512 ) ਪੰਨਾ_ਮਾਪ = 1024;
    pBt->ਪੰਨਾ_ਮਾਪ = (u32)ਪੰਨਾ_ਮਾਪ;
    free_ਆਰਜ਼ੀ_Space(pBt);
  }
  rc = sqlite3_Pager_ਸੈੱਟ_Pagesize(pBt->pPager, &pBt->ਪੰਨਾ_ਮਾਪ, nReserve);
  pBt->usable_ਮਾਪ = pBt->ਪੰਨਾ_ਮਾਪ - (u16)nReserve;
  ਜੇ( iFix ) pBt->btsFlags |= BTS_PAGESIZE_FIXED;
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
}

/*
** Return the currently defined page size
*/
ਪੂਰਨ sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(Btree *p){
  ਵਾਪਸ p->pBt->ਪੰਨਾ_ਮਾਪ;
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
ਪੂਰਨ sqlite3BtreeGetReserveNoMutex(Btree *p){
  ਪੂਰਨ n;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->pBt->mutex) );
  n = p->pBt->ਪੰਨਾ_ਮਾਪ - p->pBt->usable_ਮਾਪ;
  ਵਾਪਸ n;
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
ਪੂਰਨ sqlite3BtreeGetRequestedReserve(Btree *p){
  ਪੂਰਨ n1, n2;
  sqlite3BtreeEnter(p);
  n1 = (ਪੂਰਨ)p->pBt->nReserveWanted;
  n2 = sqlite3BtreeGetReserveNoMutex(p);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ n1>n2 ? n1 : n2;
}


/*
** Set the maximum page count for a database if mxPage is positive.
** No changes are made if mxPage is 0 or negative.
** Regardless of the value of mxPage, return the maximum page count.
*/
Pgno sqlite3_Btree_Max_ਪੰਨਾ_ਗਿਣਤੀ(Btree *p, Pgno mx_ਪੰਨਾ){
  Pgno n;
  sqlite3BtreeEnter(p);
  n = sqlite3_Pager_Max_ਪੰਨਾ_ਗਿਣਤੀ(p->pBt->pPager, mx_ਪੰਨਾ);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ n;
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
ਪੂਰਨ sqlite3BtreeSecureDelete(Btree *p, ਪੂਰਨ newFlag){
  ਪੂਰਨ b;
  ਜੇ( p==0 ) ਵਾਪਸ 0;
  sqlite3BtreeEnter(p);
  ਪ੍ਰਤਿਜ੍ਞਾ( BTS_OVERWRITE==BTS_SECURE_DELETE*2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( BTS_FAST_SECURE==(BTS_OVERWRITE|BTS_SECURE_DELETE) );
  ਜੇ( newFlag>=0 ){
    p->pBt->btsFlags &= ~BTS_FAST_SECURE;
    p->pBt->btsFlags |= (u16)(BTS_SECURE_DELETE*newFlag);
  }
  b = (p->pBt->btsFlags & BTS_FAST_SECURE)/BTS_SECURE_DELETE;
  sqlite3BtreeLeave(p);
  ਵਾਪਸ b;
}

/*
** Change the 'auto-vacuum' property of the database. If the 'autoVacuum'
** parameter is non-zero, then auto-vacuum mode is enabled. If zero, it
** is disabled. The default value for the auto-vacuum property is
** determined by the SQLITE_DEFAULT_AUTOVACUUM macro.
*/
ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_Auto_Vacuum(Btree *p, ਪੂਰਨ autoVacuum){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
  ਵਾਪਸ SQLITE_READONLY;
#ਨਹੀਂਤਾਂ
  BtShared *pBt = p->pBt;
  ਪੂਰਨ rc = SQLITE_OK;
  u8 av = (u8)autoVacuum;

  sqlite3BtreeEnter(p);
  ਜੇ( (pBt->btsFlags & BTS_PAGESIZE_FIXED)!=0 && (av ?1:0)!=pBt->autoVacuum ){
    rc = SQLITE_READONLY;
  }ਨਹੀਂਤਾਂ{
    pBt->autoVacuum = av ?1:0;
    pBt->incrVacuum = av==2 ?1:0;
  }
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
#ਜੇ_ਅੰਤ
}

/*
** Return the value of the 'auto-vacuum' property. If auto-vacuum is
** enabled 1 is returned. Otherwise 0.
*/
ਪੂਰਨ sqlite3BtreeGetAutoVacuum(Btree *p){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
  ਵਾਪਸ BTREE_AUTOVACUUM_NONE;
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ rc;
  sqlite3BtreeEnter(p);
  rc = (
    (!p->pBt->autoVacuum)?BTREE_AUTOVACUUM_NONE:
    (!p->pBt->incrVacuum)?BTREE_AUTOVACUUM_ਭਰਿਆ:
    BTREE_AUTOVACUUM_INCR
  );
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
#ਜੇ_ਅੰਤ
}

/*
** If the user has not set the safety-level for this database connection
** using "PRAGMA synchronous", and if the safety-level is not already
** set to the value passed to this function as the second parameter,
** set it so.
*/
#ਜੇ SQLITE_DEFAULT_SYNCHRONOUS!=SQLITE_DEFAULT_WAL_SYNCHRONOUS \
    && !defined(SQLITE_OMIT_WAL)
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_Default_Sync_Flag(BtShared *pBt, u8 safety_level){
  sqlite3 *db;
  Db *pDb;
  ਜੇ( (db=pBt->db)!=0 && (pDb=db->aDb)!=0 ){
    ਜਦੋਂ( pDb->pBt==0 || pDb->pBt->pBt!=pBt ){ pDb++; }
    ਜੇ( pDb->b_Sync_ਸੈੱਟ==0
     && pDb->safety_level!=safety_level
     && pDb!=&db->aDb[1]
    ){
      pDb->safety_level = safety_level;
      sqlite3_Pager_ਸੈੱਟ_Flags(pBt->pPager,
          pDb->safety_level | (db->flags & PAGER_FLAGS_ਨਕਾਬ));
    }
  }
}
#ਨਹੀਂਤਾਂ
# define ਸੈੱਟ_Default_Sync_Flag(pBt,safety_level)
#ਜੇ_ਅੰਤ

/* Forward declaration */
ਸਥਿਰ ਪੂਰਨ newDatabase(BtShared*);


/*
** Get a reference to pPage1 of the database file.  This will
** also acquire a readlock on that file.
**
** SQLITE_OK is returned on success.  If the file is not a
** well-formed database file, then SQLITE_CORRUPT is returned.
** SQLITE_BUSY is returned if the database is locked.  SQLITE_NOMEM
** is returned if we run out of memory.
*/
ਸਥਿਰ ਪੂਰਨ ਤਾਲਾ_Btree(BtShared *pBt){
  ਪੂਰਨ rc;              /* Result code from subfunctions */
  Mem_ਪੰਨਾ *pPage1;     /* Page 1 of the database file */
  u32 n_ਪੰਨਾ;           /* Number of pages in the database */
  u32 n_ਪੰਨਾ_File = 0;   /* Number of pages in the database file */

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->pPage1==0 );
  rc = sqlite3_Pager_Shared_ਤਾਲਾ(pBt->pPager);
  ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
  rc = btree_Get_ਪੰਨਾ(pBt, 1, &pPage1, 0);
  ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;

  /* Do some checking to help insure the file we opened really is
  ** a valid database file.
  */
  n_ਪੰਨਾ = get4byte(28+(u8*)pPage1->a_ਅੰਕੜੇ);
  sqlite3PagerPagecount(pBt->pPager, (ਪੂਰਨ*)&n_ਪੰਨਾ_File);
  ਜੇ( n_ਪੰਨਾ==0 || memcmp(24+(u8*)pPage1->a_ਅੰਕੜੇ, 92+(u8*)pPage1->a_ਅੰਕੜੇ,4)!=0 ){
    n_ਪੰਨਾ = n_ਪੰਨਾ_File;
  }
  ਜੇ( (pBt->db->flags & SQLITE_ResetDatabase)!=0 ){
    n_ਪੰਨਾ = 0;
  }
  ਜੇ( n_ਪੰਨਾ>0 ){
    u32 ਪੰਨਾ_ਮਾਪ;
    u32 usable_ਮਾਪ;
    u8 *page1 = pPage1->a_ਅੰਕੜੇ;
    rc = SQLITE_NOTADB;
    /* EVIDENCE-OF: R-43737-39999 Every valid SQLite database file begins
    ** with the following 16 bytes (in hex): 53 51 4c 69 74 65 20 66 6f 72 6d
    ** 61 74 20 33 00. */
    ਜੇ( memcmp(page1, z_Magic_ਸਿਰਲੇਖ, 16)!=0 ){
      ਜਾਓ page1_init_failed;
    }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
    ਜੇ( page1[18]>1 ){
      pBt->btsFlags |= BTS_READ_ONLY;
    }
    ਜੇ( page1[19]>1 ){
      ਜਾਓ page1_init_failed;
    }
#ਨਹੀਂਤਾਂ
    ਜੇ( page1[18]>2 ){
      pBt->btsFlags |= BTS_READ_ONLY;
    }
    ਜੇ( page1[19]>2 ){
      ਜਾਓ page1_init_failed;
    }

    /* If the read version is set to 2, this database should be accessed
    ** in WAL mode. If the log is not already open, open it now. Then
    ** return SQLITE_OK and return without populating BtShared.pPage1.
    ** The caller detects this and calls this function again. This is
    ** required as the version of page 1 currently in the page1 buffer
    ** may not be the latest version - there may be a newer one in the log
    ** file.
    */
    ਜੇ( page1[19]==2 && (pBt->btsFlags & BTS_NO_WAL)==0 ){
      ਪੂਰਨ isOpen = 0;
      rc = sqlite3PagerOpenWal(pBt->pPager, &isOpen);
      ਜੇ( rc!=SQLITE_OK ){
        ਜਾਓ page1_init_failed;
      }ਨਹੀਂਤਾਂ{
        ਸੈੱਟ_Default_Sync_Flag(pBt, SQLITE_DEFAULT_WAL_SYNCHRONOUS+1);
        ਜੇ( isOpen==0 ){
          release_ਪੰਨਾ_One(pPage1);
          ਵਾਪਸ SQLITE_OK;
        }
      }
      rc = SQLITE_NOTADB;
    }ਨਹੀਂਤਾਂ{
      ਸੈੱਟ_Default_Sync_Flag(pBt, SQLITE_DEFAULT_SYNCHRONOUS+1);
    }
#ਜੇ_ਅੰਤ

    /* EVIDENCE-OF: R-15465-20813 The maximum and minimum embedded payload
    ** fractions and the leaf payload fraction values must be 64, 32, and 32.
    **
    ** The original design allowed these amounts to vary, but as of
    ** version 3.6.0, we require them to be fixed.
    */
    ਜੇ( memcmp(&page1[21], "\100\040\040",3)!=0 ){
      ਜਾਓ page1_init_failed;
    }
    /* EVIDENCE-OF: R-51873-39618 The page size for a database file is
    ** determined by the 2-byte integer located at an offset of 16 bytes from
    ** the beginning of the database file. */
    ਪੰਨਾ_ਮਾਪ = (page1[16]<<8) | (page1[17]<<16);
    /* EVIDENCE-OF: R-25008-21688 The size of a page is a power of two
    ** between 512 and 65536 inclusive. */
    ਜੇ( ((ਪੰਨਾ_ਮਾਪ-1)&ਪੰਨਾ_ਮਾਪ)!=0
     || ਪੰਨਾ_ਮਾਪ>SQLITE_MAX_ਪੰਨਾ_ਮਾਪ
     || ਪੰਨਾ_ਮਾਪ<=256
    ){
      ਜਾਓ page1_init_failed;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( (ਪੰਨਾ_ਮਾਪ & 7)==0 );
    /* EVIDENCE-OF: R-59310-51205 The "reserved space" size in the 1-byte
    ** integer at offset 20 is the number of bytes of space at the end of
    ** each page to reserve for extensions.
    **
    ** EVIDENCE-OF: R-37497-42412 The size of the reserved region is
    ** determined by the one-byte unsigned integer found at an offset of 20
    ** into the database file header. */
    usable_ਮਾਪ = ਪੰਨਾ_ਮਾਪ - page1[20];
    ਜੇ( (u32)ਪੰਨਾ_ਮਾਪ!=pBt->ਪੰਨਾ_ਮਾਪ ){
      /* After reading the first page of the database assuming a page size
      ** of BtShared.pageSize, we have discovered that the page-size is
      ** actually pageSize. Unlock the database, leave pBt->pPage1 at
      ** zero and return SQLITE_OK. The caller will call this function
      ** again with the correct page-size.
      */
      release_ਪੰਨਾ_One(pPage1);
      pBt->usable_ਮਾਪ = usable_ਮਾਪ;
      pBt->ਪੰਨਾ_ਮਾਪ = ਪੰਨਾ_ਮਾਪ;
      pBt->btsFlags |= BTS_PAGESIZE_FIXED;
      free_ਆਰਜ਼ੀ_Space(pBt);
      rc = sqlite3_Pager_ਸੈੱਟ_Pagesize(pBt->pPager, &pBt->ਪੰਨਾ_ਮਾਪ,
                                   ਪੰਨਾ_ਮਾਪ-usable_ਮਾਪ);
      ਵਾਪਸ rc;
    }
    ਜੇ( n_ਪੰਨਾ>n_ਪੰਨਾ_File ){
      ਜੇ( sqlite3WritableSchema(pBt->db)==0 ){
        rc = SQLITE_CORRUPT_BKPT;
        ਜਾਓ page1_init_failed;
      }ਨਹੀਂਤਾਂ{
        n_ਪੰਨਾ = n_ਪੰਨਾ_File;
      }
    }
    /* EVIDENCE-OF: R-28312-64704 However, the usable size is not allowed to
    ** be less than 480. In other words, if the page size is 512, then the
    ** reserved space size cannot exceed 32. */
    ਜੇ( usable_ਮਾਪ<480 ){
      ਜਾਓ page1_init_failed;
    }
    pBt->btsFlags |= BTS_PAGESIZE_FIXED;
    pBt->ਪੰਨਾ_ਮਾਪ = ਪੰਨਾ_ਮਾਪ;
    pBt->usable_ਮਾਪ = usable_ਮਾਪ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
    pBt->autoVacuum = (get4byte(&page1[36 + 4*4])?1:0);
    pBt->incrVacuum = (get4byte(&page1[36 + 7*4])?1:0);
#ਜੇ_ਅੰਤ
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
  pBt->maxLocal = (u16)((pBt->usable_ਮਾਪ-12)*64/255 - 23);
  pBt->minLocal = (u16)((pBt->usable_ਮਾਪ-12)*32/255 - 23);
  pBt->max_ਪੱਤੀ = (u16)(pBt->usable_ਮਾਪ - 35);
  pBt->min_ਪੱਤੀ = (u16)((pBt->usable_ਮਾਪ-12)*32/255 - 23);
  ਜੇ( pBt->maxLocal>127 ){
    pBt->max1bytePayload = 127;
  }ਨਹੀਂਤਾਂ{
    pBt->max1bytePayload = (u8)pBt->maxLocal;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->max_ਪੱਤੀ + 23 <= MX_CELL_ਮਾਪ(pBt) );
  pBt->pPage1 = pPage1;
  pBt->n_ਪੰਨਾ = n_ਪੰਨਾ;
  ਵਾਪਸ SQLITE_OK;

page1_init_failed:
  release_ਪੰਨਾ_One(pPage1);
  pBt->pPage1 = 0;
  ਵਾਪਸ rc;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
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
ਸਥਿਰ ਪੂਰਨ ਗਿਣਤੀ_Valid_Cursors(BtShared *pBt, ਪੂਰਨ wrOnly){
  BtCursor *pCur;
  ਪੂਰਨ r = 0;
  ਲਈ(pCur=pBt->pCursor; pCur; pCur=pCur->p_ਅਗਲਾ){
    ਜੇ( (wrOnly==0 || (pCur->curFlags & BTCF_WriteFlag)!=0)
     && pCur->eState!=CURSOR_FAULT ) r++;
  }
  ਵਾਪਸ r;
}
#ਜੇ_ਅੰਤ

/*
** If there are no outstanding cursors and we are not in the middle
** of a transaction but there is a read lock on the database, then
** this routine unrefs the first page of the database file which
** has the effect of releasing the read lock.
**
** If there is a transaction in progress, this routine is a no-op.
*/
ਸਥਿਰ ਖਾਲੀ unlock_Btree_If_ਅਣਵਰਤਿਆ(BtShared *pBt){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਗਿਣਤੀ_Valid_Cursors(pBt,0)==0 || pBt->ਵਿੱਚ_Transaction>TRANS_NONE );
  ਜੇ( pBt->ਵਿੱਚ_Transaction==TRANS_NONE && pBt->pPage1!=0 ){
    Mem_ਪੰਨਾ *pPage1 = pBt->pPage1;
    ਪ੍ਰਤਿਜ੍ਞਾ( pPage1->a_ਅੰਕੜੇ );
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerRefcount(pBt->pPager)==1 );
    pBt->pPage1 = 0;
    release_ਪੰਨਾ_One(pPage1);
  }
}

/*
** If pBt points to an empty file then convert that empty file
** into a new empty database by initializing the first page of
** the database.
*/
ਸਥਿਰ ਪੂਰਨ newDatabase(BtShared *pBt){
  Mem_ਪੰਨਾ *pP1;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਅੰਕੜੇ;
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਜੇ( pBt->n_ਪੰਨਾ>0 ){
    ਵਾਪਸ SQLITE_OK;
  }
  pP1 = pBt->pPage1;
  ਪ੍ਰਤਿਜ੍ਞਾ( pP1!=0 );
  ਅੰਕੜੇ = pP1->a_ਅੰਕੜੇ;
  rc = sqlite3PagerWrite(pP1->p_Db_ਪੰਨਾ);
  ਜੇ( rc ) ਵਾਪਸ rc;
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਅੰਕੜੇ, z_Magic_ਸਿਰਲੇਖ, ਆਕਾਰ(z_Magic_ਸਿਰਲੇਖ));
  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(z_Magic_ਸਿਰਲੇਖ)==16 );
  ਅੰਕੜੇ[16] = (u8)((pBt->ਪੰਨਾ_ਮਾਪ>>8)&0xff);
  ਅੰਕੜੇ[17] = (u8)((pBt->ਪੰਨਾ_ਮਾਪ>>16)&0xff);
  ਅੰਕੜੇ[18] = 1;
  ਅੰਕੜੇ[19] = 1;
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->usable_ਮਾਪ<=pBt->ਪੰਨਾ_ਮਾਪ && pBt->usable_ਮਾਪ+255>=pBt->ਪੰਨਾ_ਮਾਪ);
  ਅੰਕੜੇ[20] = (u8)(pBt->ਪੰਨਾ_ਮਾਪ - pBt->usable_ਮਾਪ);
  ਅੰਕੜੇ[21] = 64;
  ਅੰਕੜੇ[22] = 32;
  ਅੰਕੜੇ[23] = 32;
  memset(&ਅੰਕੜੇ[24], 0, 100-24);
  zero_ਪੰਨਾ(pP1, PTF_INTKEY|PTF_ਪੱਤੀ|PTF_LEAFDATA );
  pBt->btsFlags |= BTS_PAGESIZE_FIXED;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->autoVacuum==1 || pBt->autoVacuum==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->incrVacuum==1 || pBt->incrVacuum==0 );
  put4byte(&ਅੰਕੜੇ[36 + 4*4], pBt->autoVacuum);
  put4byte(&ਅੰਕੜੇ[36 + 7*4], pBt->incrVacuum);
#ਜੇ_ਅੰਤ
  pBt->n_ਪੰਨਾ = 1;
  ਅੰਕੜੇ[31] = 1;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Initialize the first page of the database file (creating a database
** consisting of a single page and no schema objects). Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
ਪੂਰਨ sqlite3BtreeNewDb(Btree *p){
  ਪੂਰਨ rc;
  sqlite3BtreeEnter(p);
  p->pBt->n_ਪੰਨਾ = 0;
  rc = newDatabase(p->pBt);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
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
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ btreeBeginTrans(
  Btree *p,                 /* The btree in which to start the transaction */
  ਪੂਰਨ wrflag,               /* True to start a write transaction */
  ਪੂਰਨ *p_Schema_ਸੰਸਕਰਣ       /* Put schema version number here, if not NULL */
){
  BtShared *pBt = p->pBt;
  Pager *pPager = pBt->pPager;
  ਪੂਰਨ rc = SQLITE_OK;

  sqlite3BtreeEnter(p);
  btreeIntegrity(p);

  /* If the btree is already in a write-transaction, or it
  ** is already in a read-transaction and a read-transaction
  ** is requested, this is a no-op.
  */
  ਜੇ( p->ਵਿੱਚ_Trans==TRANS_WRITE || (p->ਵਿੱਚ_Trans==TRANS_READ && !wrflag) ){
    ਜਾਓ trans_begun;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->ਵਿੱਚ_Transaction==TRANS_WRITE || IfNotOmitAV(pBt->bDoTruncate)==0 );

  ਜੇ( (p->db->flags & SQLITE_ResetDatabase)
   && sqlite3PagerIsreadonly(pPager)==0
  ){
    pBt->btsFlags &= ~BTS_READ_ONLY;
  }

  /* Write transactions are not possible on a read-only database */
  ਜੇ( (pBt->btsFlags & BTS_READ_ONLY)!=0 && wrflag ){
    rc = SQLITE_READONLY;
    ਜਾਓ trans_begun;
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
  {
    sqlite3 *p_ਖੰਡ = 0;
    /* If another database handle has already opened a write transaction
    ** on this shared-btree structure and a second write transaction is
    ** requested, return SQLITE_LOCKED.
    */
    ਜੇ( (wrflag && pBt->ਵਿੱਚ_Transaction==TRANS_WRITE)
     || (pBt->btsFlags & BTS_PENDING)!=0
    ){
      p_ਖੰਡ = pBt->pWriter->db;
    }ਨਹੀਂਤਾਂ ਜੇ( wrflag>1 ){
      Bt_ਤਾਲਾ *pIter;
      ਲਈ(pIter=pBt->p_ਤਾਲਾ; pIter; pIter=pIter->p_ਅਗਲਾ){
        ਜੇ( pIter->pBtree!=p ){
          p_ਖੰਡ = pIter->pBtree->db;
          ਰੋਕੋ;
        }
      }
    }
    ਜੇ( p_ਖੰਡ ){
      sqlite3ConnectionBlocked(p->db, p_ਖੰਡ);
      rc = SQLITE_LOCKED_SHAREDCACHE;
      ਜਾਓ trans_begun;
    }
  }
#ਜੇ_ਅੰਤ

  /* Any read-only or read-write transaction implies a read-lock on
  ** page 1. So if some other shared-cache client already has a write-lock
  ** on page 1, the transaction cannot be opened. */
  rc = query_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(p, SCHEMA_ਜੜ੍ਹ, READ_ਤਾਲਾ);
  ਜੇ( SQLITE_OK!=rc ) ਜਾਓ trans_begun;

  pBt->btsFlags &= ~BTS_INITIALLY_ਖਾਲੀ_ਥਾਵਾਂ;
  ਜੇ( pBt->n_ਪੰਨਾ==0 ) pBt->btsFlags |= BTS_INITIALLY_ਖਾਲੀ_ਥਾਵਾਂ;
  ਕਰੋ {
    sqlite3PagerWalDb(pPager, p->db);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
    /* If transitioning from no transaction directly to a write transaction,
    ** block for the WRITER lock first if possible. */
    ਜੇ( pBt->pPage1==0 && wrflag ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pBt->ਵਿੱਚ_Transaction==TRANS_NONE );
      rc = sqlite3_Pager_Wal_Write_ਤਾਲਾ(pPager, 1);
      ਜੇ( rc!=SQLITE_BUSY && rc!=SQLITE_OK ) ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_EXPERIMENTAL_PRAGMA_20251114
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
    ਜੇ( pBt->pPage1==0 && wrflag ){
      sqlite3_file *fd = sqlite3PagerFile(pPager);
      ਅੱਖਰ *aFcntl[3] = {0,0,0};
      aFcntl[1] = "experimental_pragma_20251114";
      ਪ੍ਰਤਿਜ੍ਞਾ( wrflag==1 || wrflag==2 );
      aFcntl[2] = (wrflag==1 ? "1" : "2");
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_PRAGMA, (ਖਾਲੀ*)aFcntl);
      sqlite3_free(aFcntl[0]);
    }
#ਜੇ_ਅੰਤ

    /* Call lockBtree() until either pBt->pPage1 is populated or
    ** lockBtree() returns something other than SQLITE_OK. lockBtree()
    ** may return SQLITE_OK but leave pBt->pPage1 set to 0 if after
    ** reading page 1 it discovers that the page-size of the database
    ** file is not pBt->pageSize. In this case lockBtree() will update
    ** pBt->pageSize to the page-size of the file on disk.
    */
    ਜਦੋਂ( pBt->pPage1==0 && SQLITE_OK==(rc = ਤਾਲਾ_Btree(pBt)) );

    ਜੇ( rc==SQLITE_OK && wrflag ){
      ਜੇ( (pBt->btsFlags & BTS_READ_ONLY)!=0 ){
        rc = SQLITE_READONLY;
      }ਨਹੀਂਤਾਂ{
        rc = sqlite3PagerBegin(pPager, wrflag>1, sqlite3_ਆਰਜ਼ੀ_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ(p->db));
        ਜੇ( rc==SQLITE_OK ){
          rc = newDatabase(pBt);
        }ਨਹੀਂਤਾਂ ਜੇ( rc==SQLITE_BUSY_SNAPSHOT && pBt->ਵਿੱਚ_Transaction==TRANS_NONE ){
          /* if there was no transaction opened when this function was
          ** called and SQLITE_BUSY_SNAPSHOT is returned, change the error
          ** code to SQLITE_BUSY. */
          rc = SQLITE_BUSY;
        }
      }
    }
 
    ਜੇ( rc!=SQLITE_OK ){
      (ਖਾਲੀ)sqlite3_Pager_Wal_Write_ਤਾਲਾ(pPager, 0);
      unlock_Btree_If_ਅਣਵਰਤਿਆ(pBt);
    }
#ਜੇ defined(SQLITE_ENABLE_SETLK_TIMEOUT)
    ਜੇ( rc==SQLITE_BUSY_TIMEOUT ){
      /* If a blocking lock timed out, break out of the loop here so that
      ** the busy-handler is not invoked.  */
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
  }ਜਦੋਂ( (rc&0xFF)==SQLITE_BUSY && pBt->ਵਿੱਚ_Transaction==TRANS_NONE &&
          btreeInvokeBusyHandler(pBt) );
  sqlite3PagerWalDb(pPager, 0);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SETLK_TIMEOUT
  ਜੇ( rc==SQLITE_BUSY_TIMEOUT ) rc = SQLITE_BUSY;
#ਜੇ_ਅੰਤ

  ਜੇ( rc==SQLITE_OK ){
    ਜੇ( p->ਵਿੱਚ_Trans==TRANS_NONE ){
      pBt->nTransaction++;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
      ਜੇ( p->sharable ){
        ਪ੍ਰਤਿਜ੍ਞਾ( p->ਤਾਲਾ.pBtree==p && p->ਤਾਲਾ.i_ਸਾਰਣੀ==1 );
        p->ਤਾਲਾ.e_ਤਾਲਾ = READ_ਤਾਲਾ;
        p->ਤਾਲਾ.p_ਅਗਲਾ = pBt->p_ਤਾਲਾ;
        pBt->p_ਤਾਲਾ = &p->ਤਾਲਾ;
      }
#ਜੇ_ਅੰਤ
    }
    p->ਵਿੱਚ_Trans = (wrflag?TRANS_WRITE:TRANS_READ);
    ਜੇ( p->ਵਿੱਚ_Trans>pBt->ਵਿੱਚ_Transaction ){
      pBt->ਵਿੱਚ_Transaction = p->ਵਿੱਚ_Trans;
    }
    ਜੇ( wrflag ){
      Mem_ਪੰਨਾ *pPage1 = pBt->pPage1;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
      ਪ੍ਰਤਿਜ੍ਞਾ( !pBt->pWriter );
      pBt->pWriter = p;
      pBt->btsFlags &= ~BTS_EXCLUSIVE;
      ਜੇ( wrflag>1 ) pBt->btsFlags |= BTS_EXCLUSIVE;
#ਜੇ_ਅੰਤ

      /* If the db-size header field is incorrect (as it may be if an old
      ** client has been writing the database file), update it now. Doing
      ** this sooner rather than later means the database size can safely
      ** re-read the database size from page 1 if a savepoint or transaction
      ** rollback occurs within the transaction.
      */
      ਜੇ( pBt->n_ਪੰਨਾ!=get4byte(&pPage1->a_ਅੰਕੜੇ[28]) ){
        rc = sqlite3PagerWrite(pPage1->p_Db_ਪੰਨਾ);
        ਜੇ( rc==SQLITE_OK ){
          put4byte(&pPage1->a_ਅੰਕੜੇ[28], pBt->n_ਪੰਨਾ);
        }
      }
    }
  }

trans_begun:
  ਜੇ( rc==SQLITE_OK ){
    ਜੇ( p_Schema_ਸੰਸਕਰਣ ){
      *p_Schema_ਸੰਸਕਰਣ = get4byte(&pBt->pPage1->a_ਅੰਕੜੇ[40]);
    }
    ਜੇ( wrflag ){
      /* This call makes sure that the pager has the correct number of
      ** open savepoints. If the second parameter is greater than 0 and
      ** the sub-journal is not already open, then it will be opened here.
      */
      rc = sqlite3PagerOpenSavepoint(pPager, p->db->nSavepoint);
    }
  }

  btreeIntegrity(p);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3BtreeBeginTrans(Btree *p, ਪੂਰਨ wrflag, ਪੂਰਨ *p_Schema_ਸੰਸਕਰਣ){
  BtShared *pBt;
  ਜੇ( p->sharable
   || p->ਵਿੱਚ_Trans==TRANS_NONE
   || (p->ਵਿੱਚ_Trans==TRANS_READ && wrflag!=0)
  ){
    ਵਾਪਸ btreeBeginTrans(p,wrflag,p_Schema_ਸੰਸਕਰਣ);
  }
  pBt = p->pBt;
  ਜੇ( p_Schema_ਸੰਸਕਰਣ ){
    *p_Schema_ਸੰਸਕਰਣ = get4byte(&pBt->pPage1->a_ਅੰਕੜੇ[40]);
  }
  ਜੇ( wrflag ){
    /* This call makes sure that the pager has the correct number of
    ** open savepoints. If the second parameter is greater than 0 and
    ** the sub-journal is not already open, then it will be opened here.
    */
    ਵਾਪਸ sqlite3PagerOpenSavepoint(pBt->pPager, p->db->nSavepoint);
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ SQLITE_OK;
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM

/*
** Set the pointer-map entries for all children of page pPage. Also, if
** pPage contains cells that point to overflow pages, set the pointer
** map entries for the overflow pages as well.
*/
ਸਥਿਰ ਪੂਰਨ ਸੈੱਟ_ਬੱਚਾ_Ptrmaps(Mem_ਪੰਨਾ *p_ਪੰਨਾ){
  ਪੂਰਨ i;                             /* Counter variable */
  ਪੂਰਨ nCell;                         /* Number of cells in page pPage */
  ਪੂਰਨ rc;                            /* Return code */
  BtShared *pBt = p_ਪੰਨਾ->pBt;
  Pgno pgno = p_ਪੰਨਾ->pgno;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  rc = p_ਪੰਨਾ->isInit ? SQLITE_OK : btree_Init_ਪੰਨਾ(p_ਪੰਨਾ);
  ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
  nCell = p_ਪੰਨਾ->nCell;

  ਲਈ(i=0; i<nCell; i++){
    u8 *pCell = ਲੱਭੋ_Cell(p_ਪੰਨਾ, i);

    ptrmapPutOvflPtr(p_ਪੰਨਾ, p_ਪੰਨਾ, pCell, &rc);

    ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
      Pgno ਬੱਚਾ_Pgno = get4byte(pCell);
      ptrmapPut(pBt, ਬੱਚਾ_Pgno, PTRMAP_BTREE, pgno, &rc);
    }
  }

  ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
    Pgno ਬੱਚਾ_Pgno = get4byte(&p_ਪੰਨਾ->a_ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+8]);
    ptrmapPut(pBt, ਬੱਚਾ_Pgno, PTRMAP_BTREE, pgno, &rc);
  }

  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ modify_ਪੰਨਾ_ਸੂਚਕ(Mem_ਪੰਨਾ *p_ਪੰਨਾ, Pgno iFrom, Pgno i_ਮੰਜ਼ਿਲ, u8 eType){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
  ਜੇ( eType==PTRMAP_OVERFLOW2 ){
    /* The pointer is always the first 4 bytes of the page in this case.  */
    ਜੇ( get4byte(p_ਪੰਨਾ->a_ਅੰਕੜੇ)!=iFrom ){
      ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }
    put4byte(p_ਪੰਨਾ->a_ਅੰਕੜੇ, i_ਮੰਜ਼ਿਲ);
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ i;
    ਪੂਰਨ nCell;
    ਪੂਰਨ rc;

    rc = p_ਪੰਨਾ->isInit ? SQLITE_OK : btree_Init_ਪੰਨਾ(p_ਪੰਨਾ);
    ਜੇ( rc ) ਵਾਪਸ rc;
    nCell = p_ਪੰਨਾ->nCell;

    ਲਈ(i=0; i<nCell; i++){
      u8 *pCell = ਲੱਭੋ_Cell(p_ਪੰਨਾ, i);
      ਜੇ( eType==PTRMAP_OVERFLOW1 ){
        CellInfo info;
        p_ਪੰਨਾ->xParseCell(p_ਪੰਨਾ, pCell, &info);
        ਜੇ( info.nLocal<info.nPayload ){
          ਜੇ( pCell+info.n_ਮਾਪ > p_ਪੰਨਾ->a_ਅੰਕੜੇ+p_ਪੰਨਾ->pBt->usable_ਮਾਪ ){
            ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
          }
          ਜੇ( iFrom==get4byte(pCell+info.n_ਮਾਪ-4) ){
            put4byte(pCell+info.n_ਮਾਪ-4, i_ਮੰਜ਼ਿਲ);
            ਰੋਕੋ;
          }
        }
      }ਨਹੀਂਤਾਂ{
        ਜੇ( pCell+4 > p_ਪੰਨਾ->a_ਅੰਕੜੇ+p_ਪੰਨਾ->pBt->usable_ਮਾਪ ){
          ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
        }
        ਜੇ( get4byte(pCell)==iFrom ){
          put4byte(pCell, i_ਮੰਜ਼ਿਲ);
          ਰੋਕੋ;
        }
      }
    }
 
    ਜੇ( i==nCell ){
      ਜੇ( eType!=PTRMAP_BTREE ||
          get4byte(&p_ਪੰਨਾ->a_ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+8])!=iFrom ){
        ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
      }
      put4byte(&p_ਪੰਨਾ->a_ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+8], i_ਮੰਜ਼ਿਲ);
    }
  }
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ relocate_ਪੰਨਾ(
  BtShared *pBt,           /* Btree */
  Mem_ਪੰਨਾ *p_Db_ਪੰਨਾ,        /* Open page to move */
  u8 eType,                /* Pointer map 'type' entry for pDbPage */
  Pgno i_Ptr_ਪੰਨਾ,           /* Pointer map 'page-no' entry for pDbPage */
  Pgno i_Free_ਪੰਨਾ,          /* The location to move pDbPage to */
  ਪੂਰਨ isCommit             /* isCommit flag passed to sqlite3PagerMovepage */
){
  Mem_ਪੰਨਾ *p_Ptr_ਪੰਨਾ;   /* The page that contains a pointer to pDbPage */
  Pgno i_Db_ਪੰਨਾ = p_Db_ਪੰਨਾ->pgno;
  Pager *pPager = pBt->pPager;
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( eType==PTRMAP_OVERFLOW2 || eType==PTRMAP_OVERFLOW1 ||
      eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Db_ਪੰਨਾ->pBt==pBt );
  ਜੇ( i_Db_ਪੰਨਾ<3 ) ਵਾਪਸ SQLITE_CORRUPT_BKPT;

  /* Move page iDbPage from its current location to page number iFreePage */
  TRACE(("AUTOVACUUM: Moving %u to free page %u (ptr page %u type %u)\n",
      i_Db_ਪੰਨਾ, i_Free_ਪੰਨਾ, i_Ptr_ਪੰਨਾ, eType));
  rc = sqlite3PagerMovepage(pPager, p_Db_ਪੰਨਾ->p_Db_ਪੰਨਾ, i_Free_ਪੰਨਾ, isCommit);
  ਜੇ( rc!=SQLITE_OK ){
    ਵਾਪਸ rc;
  }
  p_Db_ਪੰਨਾ->pgno = i_Free_ਪੰਨਾ;

  /* If pDbPage was a btree-page, then it may have child pages and/or cells
  ** that point to overflow pages. The pointer map entries for all these
  ** pages need to be changed.
  **
  ** If pDbPage is an overflow page, then the first 4 bytes may store a
  ** pointer to a subsequent overflow page. If this is the case, then
  ** the pointer map needs to be updated for the subsequent overflow page.
  */
  ਜੇ( eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE ){
    rc = ਸੈੱਟ_ਬੱਚਾ_Ptrmaps(p_Db_ਪੰਨਾ);
    ਜੇ( rc!=SQLITE_OK ){
      ਵਾਪਸ rc;
    }
  }ਨਹੀਂਤਾਂ{
    Pgno ਅਗਲਾ_Ovfl = get4byte(p_Db_ਪੰਨਾ->a_ਅੰਕੜੇ);
    ਜੇ( ਅਗਲਾ_Ovfl!=0 ){
      ptrmapPut(pBt, ਅਗਲਾ_Ovfl, PTRMAP_OVERFLOW2, i_Free_ਪੰਨਾ, &rc);
      ਜੇ( rc!=SQLITE_OK ){
        ਵਾਪਸ rc;
      }
    }
  }

  /* Fix the database pointer on page iPtrPage that pointed at iDbPage so
  ** that it points at iFreePage. Also fix the pointer map entry for
  ** iPtrPage.
  */
  ਜੇ( eType!=PTRMAP_ROOTPAGE ){
    rc = btree_Get_ਪੰਨਾ(pBt, i_Ptr_ਪੰਨਾ, &p_Ptr_ਪੰਨਾ, 0);
    ਜੇ( rc!=SQLITE_OK ){
      ਵਾਪਸ rc;
    }
    rc = sqlite3PagerWrite(p_Ptr_ਪੰਨਾ->p_Db_ਪੰਨਾ);
    ਜੇ( rc!=SQLITE_OK ){
      release_ਪੰਨਾ(p_Ptr_ਪੰਨਾ);
      ਵਾਪਸ rc;
    }
    rc = modify_ਪੰਨਾ_ਸੂਚਕ(p_Ptr_ਪੰਨਾ, i_Db_ਪੰਨਾ, i_Free_ਪੰਨਾ, eType);
    release_ਪੰਨਾ(p_Ptr_ਪੰਨਾ);
    ਜੇ( rc==SQLITE_OK ){
      ptrmapPut(pBt, i_Free_ਪੰਨਾ, eType, i_Ptr_ਪੰਨਾ, &rc);
    }
  }
  ਵਾਪਸ rc;
}

/* Forward declaration required by incrVacuumStep(). */
ਸਥਿਰ ਪੂਰਨ allocate_Btree_ਪੰਨਾ(BtShared *, Mem_ਪੰਨਾ **, Pgno *, Pgno, u8);

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
ਸਥਿਰ ਪੂਰਨ incr_Vacuum_ਕਦਮ(BtShared *pBt, Pgno nFin, Pgno i_ਆਖਰੀ_Pg, ਪੂਰਨ bCommit){
  Pgno nFreeList;           /* Number of pages still on the free-list */
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਆਖਰੀ_Pg>nFin );

  ਜੇ( !PTRMAP_ISPAGE(pBt, i_ਆਖਰੀ_Pg) && i_ਆਖਰੀ_Pg!=PENDING_BYTE_ਪੰਨਾ(pBt) ){
    u8 eType;
    Pgno i_Ptr_ਪੰਨਾ;

    nFreeList = get4byte(&pBt->pPage1->a_ਅੰਕੜੇ[36]);
    ਜੇ( nFreeList==0 ){
      ਵਾਪਸ SQLITE_ਪੂਰਾ;
    }

    rc = ptrmapGet(pBt, i_ਆਖਰੀ_Pg, &eType, &i_Ptr_ਪੰਨਾ);
    ਜੇ( rc!=SQLITE_OK ){
      ਵਾਪਸ rc;
    }
    ਜੇ( eType==PTRMAP_ROOTPAGE ){
      ਵਾਪਸ SQLITE_CORRUPT_BKPT;
    }

    ਜੇ( eType==PTRMAP_FREEPAGE ){
      ਜੇ( bCommit==0 ){
        /* Remove the page from the files free-list. This is not required
        ** if bCommit is non-zero. In that case, the free-list will be
        ** truncated to zero after this function returns, so it doesn't
        ** matter if it still contains some garbage entries.
        */
        Pgno iFreePg;
        Mem_ਪੰਨਾ *pFreePg;
        rc = allocate_Btree_ਪੰਨਾ(pBt, &pFreePg, &iFreePg, i_ਆਖਰੀ_Pg, BTALLOC_EXACT);
        ਜੇ( rc!=SQLITE_OK ){
          ਵਾਪਸ rc;
        }
        ਪ੍ਰਤਿਜ੍ਞਾ( iFreePg==i_ਆਖਰੀ_Pg );
        release_ਪੰਨਾ(pFreePg);
      }
    } ਨਹੀਂਤਾਂ {
      Pgno iFreePg;             /* Index of free page to move pLastPg to */
      Mem_ਪੰਨਾ *p_ਆਖਰੀ_Pg;
      u8 e_ਬਹੁਲਕ = BTALLOC_ANY;   /* Mode parameter for allocateBtreePage() */
      Pgno iNear = 0;           /* nearby parameter for allocateBtreePage() */

      rc = btree_Get_ਪੰਨਾ(pBt, i_ਆਖਰੀ_Pg, &p_ਆਖਰੀ_Pg, 0);
      ਜੇ( rc!=SQLITE_OK ){
        ਵਾਪਸ rc;
      }

      /* If bCommit is zero, this loop runs exactly once and page pLastPg
      ** is swapped with the first free page pulled off the free list.
      **
      ** On the other hand, if bCommit is greater than zero, then keep
      ** looping until a free-page located within the first nFin pages
      ** of the file is found.
      */
      ਜੇ( bCommit==0 ){
        e_ਬਹੁਲਕ = BTALLOC_LE;
        iNear = nFin;
      }
      ਕਰੋ {
        Mem_ਪੰਨਾ *pFreePg;
        Pgno db_ਮਾਪ = btreePagecount(pBt);
        rc = allocate_Btree_ਪੰਨਾ(pBt, &pFreePg, &iFreePg, iNear, e_ਬਹੁਲਕ);
        ਜੇ( rc!=SQLITE_OK ){
          release_ਪੰਨਾ(p_ਆਖਰੀ_Pg);
          ਵਾਪਸ rc;
        }
        release_ਪੰਨਾ(pFreePg);
        ਜੇ( iFreePg>db_ਮਾਪ ){
          release_ਪੰਨਾ(p_ਆਖਰੀ_Pg);
          ਵਾਪਸ SQLITE_CORRUPT_BKPT;
        }
      }ਜਦੋਂ( bCommit && iFreePg>nFin );
      ਪ੍ਰਤਿਜ੍ਞਾ( iFreePg<i_ਆਖਰੀ_Pg );
     
      rc = relocate_ਪੰਨਾ(pBt, p_ਆਖਰੀ_Pg, eType, i_Ptr_ਪੰਨਾ, iFreePg, bCommit);
      release_ਪੰਨਾ(p_ਆਖਰੀ_Pg);
      ਜੇ( rc!=SQLITE_OK ){
        ਵਾਪਸ rc;
      }
    }
  }

  ਜੇ( bCommit==0 ){
    ਕਰੋ {
      i_ਆਖਰੀ_Pg--;
    }ਜਦੋਂ( i_ਆਖਰੀ_Pg==PENDING_BYTE_ਪੰਨਾ(pBt) || PTRMAP_ISPAGE(pBt, i_ਆਖਰੀ_Pg) );
    pBt->bDoTruncate = 1;
    pBt->n_ਪੰਨਾ = i_ਆਖਰੀ_Pg;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** The database opened by the first argument is an auto-vacuum database
** nOrig pages in size containing nFree free pages. Return the expected
** size of the database in pages following an auto-vacuum operation.
*/
ਸਥਿਰ Pgno final_Db_ਮਾਪ(BtShared *pBt, Pgno nOrig, Pgno nFree){
  ਪੂਰਨ n_ਇੰਦਰਾਜ;                     /* Number of entries on one ptrmap page */
  Pgno nPtrmap;                   /* Number of PtrMap pages to be freed */
  Pgno nFin;                      /* Return value */

  n_ਇੰਦਰਾਜ = pBt->usable_ਮਾਪ/5;
  nPtrmap = (nFree-nOrig+PTRMAP_PAGENO(pBt, nOrig)+n_ਇੰਦਰਾਜ)/n_ਇੰਦਰਾਜ;
  nFin = nOrig - nFree - nPtrmap;
  ਜੇ( nOrig>PENDING_BYTE_ਪੰਨਾ(pBt) && nFin<PENDING_BYTE_ਪੰਨਾ(pBt) ){
    nFin--;
  }
  ਜਦੋਂ( PTRMAP_ISPAGE(pBt, nFin) || nFin==PENDING_BYTE_ਪੰਨਾ(pBt) ){
    nFin--;
  }

  ਵਾਪਸ nFin;
}

/*
** A write-transaction must be opened before calling this function.
** It performs a single unit of work towards an incremental vacuum.
**
** If the incremental vacuum is finished after this function has run,
** SQLITE_DONE is returned. If it is not finished, but no error occurred,
** SQLITE_OK is returned. Otherwise an SQLite error code.
*/
ਪੂਰਨ sqlite3BtreeIncrVacuum(Btree *p){
  ਪੂਰਨ rc;
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->ਵਿੱਚ_Transaction==TRANS_WRITE && p->ਵਿੱਚ_Trans==TRANS_WRITE );
  ਜੇ( !pBt->autoVacuum ){
    rc = SQLITE_ਪੂਰਾ;
  }ਨਹੀਂਤਾਂ{
    Pgno nOrig = btreePagecount(pBt);
    Pgno nFree = get4byte(&pBt->pPage1->a_ਅੰਕੜੇ[36]);
    Pgno nFin = final_Db_ਮਾਪ(pBt, nOrig, nFree);

    ਜੇ( nOrig<nFin || nFree>=nOrig ){
      rc = SQLITE_CORRUPT_BKPT;
    }ਨਹੀਂਤਾਂ ਜੇ( nFree>0 ){
      rc = saveAllCursors(pBt, 0, 0);
      ਜੇ( rc==SQLITE_OK ){
        invalidate_All_ਉਛਾਲ_Cache(pBt);
        rc = incr_Vacuum_ਕਦਮ(pBt, nFin, nOrig, 0);
      }
      ਜੇ( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ਪੰਨਾ);
        put4byte(&pBt->pPage1->a_ਅੰਕੜੇ[28], pBt->n_ਪੰਨਾ);
      }
    }ਨਹੀਂਤਾਂ{
      rc = SQLITE_ਪੂਰਾ;
    }
  }
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
}

/*
** This routine is called prior to sqlite3PagerCommit when a transaction
** is committed for an auto-vacuum database.
*/
ਸਥਿਰ ਪੂਰਨ autoVacuumCommit(Btree *p){
  ਪੂਰਨ rc = SQLITE_OK;
  Pager *pPager;
  BtShared *pBt;
  sqlite3 *db;
  VVA_ONLY( ਪੂਰਨ nRef );

  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  pBt = p->pBt; 
  pPager = pBt->pPager;
  VVA_ONLY( nRef = sqlite3PagerRefcount(pPager); )

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  invalidate_All_ਉਛਾਲ_Cache(pBt);
  ਪ੍ਰਤਿਜ੍ਞਾ(pBt->autoVacuum);
  ਜੇ( !pBt->incrVacuum ){
    Pgno nFin;         /* Number of pages in database after autovacuuming */
    Pgno nFree;        /* Number of pages on the freelist initially */
    Pgno nVac;         /* Number of pages to vacuum */
    Pgno iFree;        /* The next page to be freed */
    Pgno nOrig;        /* Database size before freeing */

    nOrig = btreePagecount(pBt);
    ਜੇ( PTRMAP_ISPAGE(pBt, nOrig) || nOrig==PENDING_BYTE_ਪੰਨਾ(pBt) ){
      /* It is not possible to create a database for which the final page
      ** is either a pointer-map page or the pending-byte page. If one
      ** is encountered, this indicates corruption.
      */
      ਵਾਪਸ SQLITE_CORRUPT_BKPT;
    }

    nFree = get4byte(&pBt->pPage1->a_ਅੰਕੜੇ[36]);
    db = p->db;
    ਜੇ( db->x_Autovac_ਪੰਨੇ ){
      ਪੂਰਨ iDb;
      ਲਈ(iDb=0; ALWAYS(iDb<db->nDb); iDb++){
        ਜੇ( db->aDb[iDb].pBt==p ) ਰੋਕੋ;
      }
      nVac = db->x_Autovac_ਪੰਨੇ(
        db->p_Autovac_ਪੰਨੇ_Arg,
        db->aDb[iDb].zDbSName,
        nOrig,
        nFree,
        pBt->ਪੰਨਾ_ਮਾਪ
      );
      ਜੇ( nVac>nFree ){
        nVac = nFree;
      }
      ਜੇ( nVac==0 ){
        ਵਾਪਸ SQLITE_OK;
      }
    }ਨਹੀਂਤਾਂ{
      nVac = nFree;
    }
    nFin = final_Db_ਮਾਪ(pBt, nOrig, nVac);
    ਜੇ( nFin>nOrig ) ਵਾਪਸ SQLITE_CORRUPT_BKPT;
    ਜੇ( nFin<nOrig ){
      rc = saveAllCursors(pBt, 0, 0);
    }
    ਲਈ(iFree=nOrig; iFree>nFin && rc==SQLITE_OK; iFree--){
      rc = incr_Vacuum_ਕਦਮ(pBt, nFin, iFree, nVac==nFree);
    }
    ਜੇ( (rc==SQLITE_ਪੂਰਾ || rc==SQLITE_OK) && nFree>0 ){
      rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ਪੰਨਾ);
      ਜੇ( nVac==nFree ){
        put4byte(&pBt->pPage1->a_ਅੰਕੜੇ[32], 0);
        put4byte(&pBt->pPage1->a_ਅੰਕੜੇ[36], 0);
      }
      put4byte(&pBt->pPage1->a_ਅੰਕੜੇ[28], nFin);
      pBt->bDoTruncate = 1;
      pBt->n_ਪੰਨਾ = nFin;
    }
    ਜੇ( rc!=SQLITE_OK ){
      sqlite3PagerRollback(pPager);
    }
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( nRef>=sqlite3PagerRefcount(pPager) );
  ਵਾਪਸ rc;
}

#ਨਹੀਂਤਾਂ /* ifndef SQLITE_OMIT_AUTOVACUUM */
# define ਸੈੱਟ_ਬੱਚਾ_Ptrmaps(x) SQLITE_OK
#ਜੇ_ਅੰਤ

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
ਪੂਰਨ sqlite3BtreeCommitPhaseOne(Btree *p, ਅਚਲ ਅੱਖਰ *zSuperJrnl){
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( p->ਵਿੱਚ_Trans==TRANS_WRITE ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
    ਜੇ( pBt->autoVacuum ){
      rc = autoVacuumCommit(p);
      ਜੇ( rc!=SQLITE_OK ){
        sqlite3BtreeLeave(p);
        ਵਾਪਸ rc;
      }
    }
    ਜੇ( pBt->bDoTruncate ){
      sqlite3PagerTruncateImage(pBt->pPager, pBt->n_ਪੰਨਾ);
    }
#ਜੇ_ਅੰਤ
    rc = sqlite3PagerCommitPhaseOne(pBt->pPager, zSuperJrnl, 0);
    sqlite3BtreeLeave(p);
  }
  ਵਾਪਸ rc;
}

/*
** This function is called from both BtreeCommitPhaseTwo() and BtreeRollback()
** at the conclusion of a transaction.
*/
ਸਥਿਰ ਖਾਲੀ btree_ਸਿਰਾ_Transaction(Btree *p){
  BtShared *pBt = p->pBt;
  sqlite3 *db = p->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsMutex(p) );

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
  pBt->bDoTruncate = 0;
#ਜੇ_ਅੰਤ
  ਜੇ( p->ਵਿੱਚ_Trans>TRANS_NONE && db->nVdbeRead>1 ){
    /* If there are other active statements that belong to this database
    ** handle, downgrade to a read-only transaction. The other statements
    ** may still be reading from the database.  */
    downgrade_All_Shared_Cache_ਸਾਰਣੀ_Locks(p);
    p->ਵਿੱਚ_Trans = TRANS_READ;
  }ਨਹੀਂਤਾਂ{
    /* If the handle had any kind of transaction open, decrement the
    ** transaction count of the shared btree. If the transaction count
    ** reaches 0, set the shared state to TRANS_NONE. The unlockBtreeIfUnused()
    ** call below will unlock the pager.  */
    ਜੇ( p->ਵਿੱਚ_Trans!=TRANS_NONE ){
      clear_All_Shared_Cache_ਸਾਰਣੀ_Locks(p);
      pBt->nTransaction--;
      ਜੇ( 0==pBt->nTransaction ){
        pBt->ਵਿੱਚ_Transaction = TRANS_NONE;
      }
    }

    /* Set the current transaction state to TRANS_NONE and unlock the
    ** pager if this call closed the only read or write transaction.  */
    p->ਵਿੱਚ_Trans = TRANS_NONE;
    unlock_Btree_If_ਅਣਵਰਤਿਆ(pBt);
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
ਪੂਰਨ sqlite3BtreeCommitPhaseTwo(Btree *p, ਪੂਰਨ bCleanup){

  ਜੇ( p->ਵਿੱਚ_Trans==TRANS_NONE ) ਵਾਪਸ SQLITE_OK;
  sqlite3BtreeEnter(p);
  btreeIntegrity(p);

  /* If the handle has a write-transaction open, commit the shared-btrees
  ** transaction and set the shared state to TRANS_READ.
  */
  ਜੇ( p->ਵਿੱਚ_Trans==TRANS_WRITE ){
    ਪੂਰਨ rc;
    BtShared *pBt = p->pBt;
    ਪ੍ਰਤਿਜ੍ਞਾ( pBt->ਵਿੱਚ_Transaction==TRANS_WRITE );
    ਪ੍ਰਤਿਜ੍ਞਾ( pBt->nTransaction>0 );
    rc = sqlite3PagerCommitPhaseTwo(pBt->pPager);
    ਜੇ( rc!=SQLITE_OK && bCleanup==0 ){
      sqlite3BtreeLeave(p);
      ਵਾਪਸ rc;
    }
    p->i_BData_ਸੰਸਕਰਣ--;  /* Compensate for pPager->iDataVersion++; */
    pBt->ਵਿੱਚ_Transaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btree_ਸਿਰਾ_Transaction(p);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Do both phases of a commit.
*/
ਪੂਰਨ sqlite3BtreeCommit(Btree *p){
  ਪੂਰਨ rc;
  sqlite3BtreeEnter(p);
  rc = sqlite3BtreeCommitPhaseOne(p, 0);
  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3BtreeCommitPhaseTwo(p, 0);
  }
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
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
ਪੂਰਨ sqlite3BtreeTripAllCursors(Btree *pBtree, ਪੂਰਨ err_ਕੋਡ, ਪੂਰਨ writeOnly){
  BtCursor *p;
  ਪੂਰਨ rc = SQLITE_OK;

  ਪ੍ਰਤਿਜ੍ਞਾ( (writeOnly==0 || writeOnly==1) && BTCF_WriteFlag==1 );
  ਜੇ( pBtree ){
    sqlite3BtreeEnter(pBtree);
    ਲਈ(p=pBtree->pBt->pCursor; p; p=p->p_ਅਗਲਾ){
      ਜੇ( writeOnly && (p->curFlags & BTCF_WriteFlag)==0 ){
        ਜੇ( p->eState==CURSOR_VALID || p->eState==CURSOR_SKIPNEXT ){
          rc = saveCursorPosition(p);
          ਜੇ( rc!=SQLITE_OK ){
            (ਖਾਲੀ)sqlite3BtreeTripAllCursors(pBtree, rc, 0);
            ਰੋਕੋ;
          }
        }
      }ਨਹੀਂਤਾਂ{
        sqlite3BtreeClearCursor(p);
        p->eState = CURSOR_FAULT;
        p->skip_ਅਗਲਾ = err_ਕੋਡ;
      }
      btree_Release_All_Cursor_ਪੰਨੇ(p);
    }
    sqlite3BtreeLeave(pBtree);
  }
  ਵਾਪਸ rc;
}

/*
** Set the pBt->nPage field correctly, according to the current
** state of the database.  Assume pBt->pPage1 is valid.
*/
ਸਥਿਰ ਖਾਲੀ btree_ਸੈੱਟ_NPage(BtShared *pBt, Mem_ਪੰਨਾ *pPage1){
  ਪੂਰਨ n_ਪੰਨਾ = get4byte(&pPage1->a_ਅੰਕੜੇ[28]);
  testcase( n_ਪੰਨਾ==0 );
  ਜੇ( n_ਪੰਨਾ==0 ) sqlite3PagerPagecount(pBt->pPager, &n_ਪੰਨਾ);
  testcase( pBt->n_ਪੰਨਾ!=(u32)n_ਪੰਨਾ );
  pBt->n_ਪੰਨਾ = n_ਪੰਨਾ;
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
ਪੂਰਨ sqlite3BtreeRollback(Btree *p, ਪੂਰਨ trip_ਕੋਡ, ਪੂਰਨ writeOnly){
  ਪੂਰਨ rc;
  BtShared *pBt = p->pBt;
  Mem_ਪੰਨਾ *pPage1;

  ਪ੍ਰਤਿਜ੍ਞਾ( writeOnly==1 || writeOnly==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( trip_ਕੋਡ==SQLITE_ABORT_ROLLBACK || trip_ਕੋਡ==SQLITE_OK );
  sqlite3BtreeEnter(p);
  ਜੇ( trip_ਕੋਡ==SQLITE_OK ){
    rc = trip_ਕੋਡ = saveAllCursors(pBt, 0, 0);
    ਜੇ( rc ) writeOnly = 0;
  }ਨਹੀਂਤਾਂ{
    rc = SQLITE_OK;
  }
  ਜੇ( trip_ਕੋਡ ){
    ਪੂਰਨ rc2 = sqlite3BtreeTripAllCursors(p, trip_ਕੋਡ, writeOnly);
    ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || (writeOnly==0 && rc2==SQLITE_OK) );
    ਜੇ( rc2!=SQLITE_OK ) rc = rc2;
  }
  btreeIntegrity(p);

  ਜੇ( p->ਵਿੱਚ_Trans==TRANS_WRITE ){
    ਪੂਰਨ rc2;

    ਪ੍ਰਤਿਜ੍ਞਾ( TRANS_WRITE==pBt->ਵਿੱਚ_Transaction );
    rc2 = sqlite3PagerRollback(pBt->pPager);
    ਜੇ( rc2!=SQLITE_OK ){
      rc = rc2;
    }

    /* The rollback may have destroyed the pPage1->aData value.  So
    ** call btreeGetPage() on page 1 again to make
    ** sure pPage1->aData is set correctly. */
    ਜੇ( btree_Get_ਪੰਨਾ(pBt, 1, &pPage1, 0)==SQLITE_OK ){
      btree_ਸੈੱਟ_NPage(pBt, pPage1);
      release_ਪੰਨਾ_One(pPage1);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( ਗਿਣਤੀ_Valid_Cursors(pBt, 1)==0 );
    pBt->ਵਿੱਚ_Transaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btree_ਸਿਰਾ_Transaction(p);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
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
ਪੂਰਨ sqlite3BtreeBeginStmt(Btree *p, ਪੂਰਨ iStatement){
  ਪੂਰਨ rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);
  ਪ੍ਰਤਿਜ੍ਞਾ( p->ਵਿੱਚ_Trans==TRANS_WRITE );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( iStatement>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( iStatement>p->db->nSavepoint );
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->ਵਿੱਚ_Transaction==TRANS_WRITE );
  /* At the pager level, a statement transaction is a savepoint with
  ** an index greater than all savepoints created explicitly using
  ** SQL statements. It is illegal to open, release or rollback any
  ** such savepoints while the statement transaction savepoint is active.
  */
  rc = sqlite3PagerOpenSavepoint(pBt->pPager, iStatement);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
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
ਪੂਰਨ sqlite3BtreeSavepoint(Btree *p, ਪੂਰਨ op, ਪੂਰਨ iSavepoint){
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( p && p->ਵਿੱਚ_Trans==TRANS_WRITE ){
    BtShared *pBt = p->pBt;
    ਪ੍ਰਤਿਜ੍ਞਾ( op==SAVEPOINT_RELEASE || op==SAVEPOINT_ROLLBACK );
    ਪ੍ਰਤਿਜ੍ਞਾ( iSavepoint>=0 || (iSavepoint==-1 && op==SAVEPOINT_ROLLBACK) );
    sqlite3BtreeEnter(p);
    ਜੇ( op==SAVEPOINT_ROLLBACK ){
      rc = saveAllCursors(pBt, 0, 0);
    }
    ਜੇ( rc==SQLITE_OK ){
      rc = sqlite3PagerSavepoint(pBt->pPager, op, iSavepoint);
    }
    ਜੇ( rc==SQLITE_OK ){
      ਜੇ( iSavepoint<0 && (pBt->btsFlags & BTS_INITIALLY_ਖਾਲੀ_ਥਾਵਾਂ)!=0 ){
        pBt->n_ਪੰਨਾ = 0;
      }
      rc = newDatabase(pBt);
      btree_ਸੈੱਟ_NPage(pBt, pBt->pPage1);

      /* pBt->nPage might be zero if the database was corrupt when
      ** the transaction was started. Otherwise, it must be at least 1.  */
      ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB || pBt->n_ਪੰਨਾ>0 );
    }
    sqlite3BtreeLeave(p);
  }
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ btreeCursor(
  Btree *p,                              /* The btree */
  Pgno i_ਸਾਰਣੀ,                           /* Root page of table to open */
  ਪੂਰਨ wrFlag,                            /* 1 to write. 0 read-only */
  ਬਣਤਰ ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info,              /* First arg to comparison function */
  BtCursor *pCur                         /* Space for new cursor */
){
  BtShared *pBt = p->pBt;                /* Shared b-tree handle */
  BtCursor *pX;                          /* Looping over other all cursors */

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsMutex(p) );
  ਪ੍ਰਤਿਜ੍ਞਾ( wrFlag==0
       || wrFlag==BTREE_WRCSR
       || wrFlag==(BTREE_WRCSR|BTREE_FORDELETE)
  );

  /* The following assert statements verify that if this is a sharable
  ** b-tree database, the connection is holding the required table locks,
  ** and that no other connection has any open cursor that conflicts with
  ** this lock.  The iTable<1 term disables the check for corrupt schemas. */
  ਪ੍ਰਤਿਜ੍ਞਾ( has_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(p, i_ਸਾਰਣੀ, p_ਕੁੰਜੀ_Info!=0, (wrFlag?2:1))
          || i_ਸਾਰਣੀ<1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( wrFlag==0 || !hasReadConflicts(p, i_ਸਾਰਣੀ) );

  /* Assert that the caller has opened the required transaction. */
  ਪ੍ਰਤਿਜ੍ਞਾ( p->ਵਿੱਚ_Trans>TRANS_NONE );
  ਪ੍ਰਤਿਜ੍ਞਾ( wrFlag==0 || p->ਵਿੱਚ_Trans==TRANS_WRITE );
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->pPage1 && pBt->pPage1->a_ਅੰਕੜੇ );
  ਪ੍ਰਤਿਜ੍ਞਾ( wrFlag==0 || (pBt->btsFlags & BTS_READ_ONLY)==0 );

  ਜੇ( i_ਸਾਰਣੀ<=1 ){
    ਜੇ( i_ਸਾਰਣੀ<1 ){
      ਵਾਪਸ SQLITE_CORRUPT_BKPT;
    }ਨਹੀਂਤਾਂ ਜੇ( btreePagecount(pBt)==0 ){
      ਪ੍ਰਤਿਜ੍ਞਾ( wrFlag==0 );
      i_ਸਾਰਣੀ = 0;
    }
  }

  /* Now that no other errors can occur, finish filling in the BtCursor
  ** variables and link the cursor into the BtShared list.  */
  pCur->pgno_ਜੜ੍ਹ = i_ਸਾਰਣੀ;
  pCur->i_ਪੰਨਾ = -1;
  pCur->p_ਕੁੰਜੀ_Info = p_ਕੁੰਜੀ_Info;
  pCur->pBtree = p;
  pCur->pBt = pBt;
  pCur->curFlags = 0;
  /* If there are two or more cursors on the same btree, then all such
  ** cursors *must* have the BTCF_Multiple flag set. */
  ਲਈ(pX=pBt->pCursor; pX; pX=pX->p_ਅਗਲਾ){
    ਜੇ( pX->pgno_ਜੜ੍ਹ==i_ਸਾਰਣੀ ){
      pX->curFlags |= BTCF_ਗੁਣਜ;
      pCur->curFlags = BTCF_ਗੁਣਜ;
    }
  }
  pCur->eState = CURSOR_INVALID;
  pCur->p_ਅਗਲਾ = pBt->pCursor;
  pBt->pCursor = pCur;
  ਜੇ( wrFlag ){
    pCur->curFlags |= BTCF_WriteFlag;
    pCur->curPagerFlags = 0;
    ਜੇ( pBt->pTmpSpace==0 ) ਵਾਪਸ allocate_ਆਰਜ਼ੀ_Space(pBt);
  }ਨਹੀਂਤਾਂ{
    pCur->curPagerFlags = PAGER_GET_READONLY;
  }
  ਵਾਪਸ SQLITE_OK;
}
ਸਥਿਰ ਪੂਰਨ btree_Cursor_With_ਤਾਲਾ(
  Btree *p,                              /* The btree */
  Pgno i_ਸਾਰਣੀ,                           /* Root page of table to open */
  ਪੂਰਨ wrFlag,                            /* 1 to write. 0 read-only */
  ਬਣਤਰ ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info,              /* First arg to comparison function */
  BtCursor *pCur                         /* Space for new cursor */
){
  ਪੂਰਨ rc;
  sqlite3BtreeEnter(p);
  rc = btreeCursor(p, i_ਸਾਰਣੀ, wrFlag, p_ਕੁੰਜੀ_Info, pCur);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3BtreeCursor(
  Btree *p,                                   /* The btree */
  Pgno i_ਸਾਰਣੀ,                                /* Root page of table to open */
  ਪੂਰਨ wrFlag,                                 /* 1 to write. 0 read-only */
  ਬਣਤਰ ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info,                   /* First arg to xCompare() */
  BtCursor *pCur                              /* Write new cursor here */
){
  ਜੇ( p->sharable ){
    ਵਾਪਸ btree_Cursor_With_ਤਾਲਾ(p, i_ਸਾਰਣੀ, wrFlag, p_ਕੁੰਜੀ_Info, pCur);
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ btreeCursor(p, i_ਸਾਰਣੀ, wrFlag, p_ਕੁੰਜੀ_Info, pCur);
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
ਪੂਰਨ sqlite3_Btree_Cursor_ਮਾਪ(ਖਾਲੀ){
  ਵਾਪਸ ROUND8(ਆਕਾਰ(BtCursor));
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** Return true if and only if the Btree object will be automatically
** closed with the BtCursor closes.  This is used within assert() statements
** only.
*/
ਪੂਰਨ sqlite3BtreeClosesWithCursor(
  Btree *pBtree,       /* the btree object */
  BtCursor *pCur       /* Corresponding cursor */
){
  BtShared *pBt = pBtree->pBt;
  ਜੇ( (pBt->openFlags & BTREE_SINGLE)==0 ) ਵਾਪਸ 0;
  ਜੇ( pBt->pCursor!=pCur ) ਵਾਪਸ 0;
  ਜੇ( pCur->p_ਅਗਲਾ!=0 ) ਵਾਪਸ 0;
  ਜੇ( pCur->pBtree!=pBtree ) ਵਾਪਸ 0;
  ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ

/*
** Initialize memory that will be converted into a BtCursor object.
**
** The simple approach here would be to memset() the entire object
** to zero.  But it turns out that the apPage[] and aiIdx[] arrays
** do not need to be zeroed and they are large, so we can save a lot
** of run-time by skipping the initialization of those elements.
*/
ਖਾਲੀ sqlite3BtreeCursorZero(BtCursor *p){
  memset(p, 0, offsetof(BtCursor, BTCURSOR_ਪਹਿਲਾ_UNINIT));
}

/*
** Close a cursor.  The read lock on the database file is released
** when the last cursor is closed.
*/
ਪੂਰਨ sqlite3BtreeCloseCursor(BtCursor *pCur){
  Btree *pBtree = pCur->pBtree;
  ਜੇ( pBtree ){
    BtShared *pBt = pCur->pBt;
    sqlite3BtreeEnter(pBtree);
    ਪ੍ਰਤਿਜ੍ਞਾ( pBt->pCursor!=0 );
    ਜੇ( pBt->pCursor==pCur ){
      pBt->pCursor = pCur->p_ਅਗਲਾ;
    }ਨਹੀਂਤਾਂ{
      BtCursor *pPrev = pBt->pCursor;
      ਕਰੋ{
        ਜੇ( pPrev->p_ਅਗਲਾ==pCur ){
          pPrev->p_ਅਗਲਾ = pCur->p_ਅਗਲਾ;
          ਰੋਕੋ;
        }
        pPrev = pPrev->p_ਅਗਲਾ;
      }ਜਦੋਂ( ALWAYS(pPrev) );
    }
    btree_Release_All_Cursor_ਪੰਨੇ(pCur);
    unlock_Btree_If_ਅਣਵਰਤਿਆ(pBt);
    sqlite3_free(pCur->a_ਉਛਾਲ);
    sqlite3_free(pCur->p_ਕੁੰਜੀ);
    ਜੇ( (pBt->openFlags & BTREE_SINGLE) && pBt->pCursor==0 ){
      /* Since the BtShared is not sharable, there is no need to
      ** worry about the missing sqlite3BtreeLeave() call here.  */
      ਪ੍ਰਤਿਜ੍ਞਾ( pBtree->sharable==0 );
      sqlite3BtreeClose(pBtree);
    }ਨਹੀਂਤਾਂ{
      sqlite3BtreeLeave(pBtree);
    }
    pCur->pBtree = 0;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Make sure the BtCursor* given in the argument has a valid
** BtCursor.info structure.  If it is not already valid, call
** btreeParseCell() to fill it in.
**
** BtCursor.info is a cache of the information in the current cell.
** Using this cache reduces the number of calls to btreeParseCell().
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  ਸਥਿਰ ਪੂਰਨ cellInfoEqual(CellInfo *a, CellInfo *b){
    ਜੇ( a->n_ਕੁੰਜੀ!=b->n_ਕੁੰਜੀ ) ਵਾਪਸ 0;
    ਜੇ( a->pPayload!=b->pPayload ) ਵਾਪਸ 0;
    ਜੇ( a->nPayload!=b->nPayload ) ਵਾਪਸ 0;
    ਜੇ( a->nLocal!=b->nLocal ) ਵਾਪਸ 0;
    ਜੇ( a->n_ਮਾਪ!=b->n_ਮਾਪ ) ਵਾਪਸ 0;
    ਵਾਪਸ 1;
  }
  ਸਥਿਰ ਖਾਲੀ assertCellInfo(BtCursor *pCur){
    CellInfo info;
    memset(&info, 0, ਆਕਾਰ(info));
    btreeParseCell(pCur->p_ਪੰਨਾ, pCur->ix, &info);
    ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB || cellInfoEqual(&info, &pCur->info) );
  }
#ਨਹੀਂਤਾਂ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ assertCellInfo(x)
#ਜੇ_ਅੰਤ
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ getCellInfo(BtCursor *pCur){
  ਜੇ( pCur->info.n_ਮਾਪ==0 ){
    pCur->curFlags |= BTCF_ValidNKey;
    btreeParseCell(pCur->p_ਪੰਨਾ,pCur->ix,&pCur->info);
  }ਨਹੀਂਤਾਂ{
    assertCellInfo(pCur);
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG  /* The next routine used only within assert() statements */
/*
** Return true if the given BtCursor is valid.  A valid cursor is one
** that is currently pointing to a row in a (non-empty) table.
** This is a verification routine is used only within assert() statements.
*/
ਪੂਰਨ sqlite3BtreeCursorIsValid(BtCursor *pCur){
  ਵਾਪਸ pCur && pCur->eState==CURSOR_VALID;
}
#ਜੇ_ਅੰਤ /* NDEBUG */
ਪੂਰਨ sqlite3BtreeCursorIsValidNN(BtCursor *pCur){
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur!=0 );
  ਵਾਪਸ pCur->eState==CURSOR_VALID;
}

/*
** Return the value of the integer key or "rowid" for a table btree.
** This routine is only valid for a cursor that is pointing into a
** ordinary table btree.  If the cursor points to an index btree or
** is invalid, the result of this routine is undefined.
*/
i64 sqlite3_Btree_Integer_ਕੁੰਜੀ(BtCursor *pCur){
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorHoldsMutex(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->cur_Int_ਕੁੰਜੀ );
  getCellInfo(pCur);
  ਵਾਪਸ pCur->info.n_ਕੁੰਜੀ;
}

/*
** Pin or unpin a cursor.
*/
ਖਾਲੀ sqlite3BtreeCursorPin(BtCursor *pCur){
  ਪ੍ਰਤਿਜ੍ਞਾ( (pCur->curFlags & BTCF_Pinned)==0 );
  pCur->curFlags |= BTCF_Pinned;
}
ਖਾਲੀ sqlite3BtreeCursorUnpin(BtCursor *pCur){
  ਪ੍ਰਤਿਜ੍ਞਾ( (pCur->curFlags & BTCF_Pinned)!=0 );
  pCur->curFlags &= ~BTCF_Pinned;
}

/*
** Return the offset into the database file for the start of the
** payload to which the cursor is pointing.
*/
i64 sqlite3_Btree_ਵਿੱਥ(BtCursor *pCur){
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorHoldsMutex(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  getCellInfo(pCur);
  ਵਾਪਸ (i64)pCur->pBt->ਪੰਨਾ_ਮਾਪ*((i64)pCur->p_ਪੰਨਾ->pgno - 1) +
         (i64)(pCur->info.pPayload - pCur->p_ਪੰਨਾ->a_ਅੰਕੜੇ);
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
u32 sqlite3_Btree_Payload_ਮਾਪ(BtCursor *pCur){
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorHoldsMutex(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  getCellInfo(pCur);
  ਵਾਪਸ pCur->info.nPayload;
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
sqlite3_int64 sqlite3_Btree_Max_ਰਿਕਾਰਡ_ਮਾਪ(BtCursor *pCur){
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorHoldsMutex(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  ਵਾਪਸ pCur->pBt->ਪੰਨਾ_ਮਾਪ * (sqlite3_int64)pCur->pBt->n_ਪੰਨਾ;
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
ਸਥਿਰ ਪੂਰਨ get_ਉਛਾਲ_ਪੰਨਾ(
  BtShared *pBt,               /* The database file */
  Pgno ovfl,                   /* Current overflow page number */
  Mem_ਪੰਨਾ **pp_ਪੰਨਾ,            /* OUT: MemPage handle (may be NULL) */
  Pgno *p_Pgno_ਅਗਲਾ              /* OUT: Next overflow page number */
){
  Pgno ਅਗਲਾ = 0;
  Mem_ਪੰਨਾ *p_ਪੰਨਾ = 0;
  ਪੂਰਨ rc = SQLITE_OK;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ(p_Pgno_ਅਗਲਾ);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
  /* Try to find the next page in the overflow list using the
  ** autovacuum pointer-map pages. Guess that the next page in
  ** the overflow list is page number (ovfl+1). If that guess turns
  ** out to be wrong, fall back to loading the data of page
  ** number ovfl to determine the next page number.
  */
  ਜੇ( pBt->autoVacuum ){
    Pgno pgno;
    Pgno iGuess = ovfl+1;
    u8 eType;

    ਜਦੋਂ( PTRMAP_ISPAGE(pBt, iGuess) || iGuess==PENDING_BYTE_ਪੰਨਾ(pBt) ){
      iGuess++;
    }

    ਜੇ( iGuess<=btreePagecount(pBt) ){
      rc = ptrmapGet(pBt, iGuess, &eType, &pgno);
      ਜੇ( rc==SQLITE_OK && eType==PTRMAP_OVERFLOW2 && pgno==ovfl ){
        ਅਗਲਾ = iGuess;
        rc = SQLITE_ਪੂਰਾ;
      }
    }
  }
#ਜੇ_ਅੰਤ

  ਪ੍ਰਤਿਜ੍ਞਾ( ਅਗਲਾ==0 || rc==SQLITE_ਪੂਰਾ );
  ਜੇ( rc==SQLITE_OK ){
    rc = btree_Get_ਪੰਨਾ(pBt, ovfl, &p_ਪੰਨਾ, (pp_ਪੰਨਾ==0) ? PAGER_GET_READONLY : 0);
    ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || p_ਪੰਨਾ==0 );
    ਜੇ( rc==SQLITE_OK ){
      ਅਗਲਾ = get4byte(p_ਪੰਨਾ->a_ਅੰਕੜੇ);
    }
  }

  *p_Pgno_ਅਗਲਾ = ਅਗਲਾ;
  ਜੇ( pp_ਪੰਨਾ ){
    *pp_ਪੰਨਾ = p_ਪੰਨਾ;
  }ਨਹੀਂਤਾਂ{
    release_ਪੰਨਾ(p_ਪੰਨਾ);
  }
  ਵਾਪਸ (rc==SQLITE_ਪੂਰਾ ? SQLITE_OK : rc);
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
ਸਥਿਰ ਪੂਰਨ copyPayload(
  ਖਾਲੀ *pPayload,           /* Pointer to page data */
  ਖਾਲੀ *pBuf,               /* Pointer to buffer */
  ਪੂਰਨ nByte,                /* Number of bytes to copy */
  ਪੂਰਨ eOp,                  /* 0 -> copy from page, 1 -> copy to page */
  Db_ਪੰਨਾ *p_Db_ਪੰਨਾ           /* Page containing pPayload */
){
  ਜੇ( eOp ){
    /* Copy data from buffer to page (a write operation) */
    ਪੂਰਨ rc = sqlite3PagerWrite(p_Db_ਪੰਨਾ);
    ਜੇ( rc!=SQLITE_OK ){
      ਵਾਪਸ rc;
    }
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pPayload, pBuf, nByte);
  }ਨਹੀਂਤਾਂ{
    /* Copy data from page to buffer (a read operation) */
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pBuf, pPayload, nByte);
  }
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ accessPayload(
  BtCursor *pCur,      /* Cursor pointing to entry to read from */
  u32 ਵਿੱਥ,          /* Begin reading this far into payload */
  u32 amt,             /* Read this many bytes */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pBuf, /* Write the bytes into this buffer */
  ਪੂਰਨ eOp              /* zero to read. non-zero to write. */
){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *aPayload;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ iIdx = 0;
  Mem_ਪੰਨਾ *p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;               /* Btree page of current entry */
  BtShared *pBt = pCur->pBt;                  /* Btree this cursor belongs to */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ਸਿੱਧਾ_ਉਛਾਲ_READ
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ * ਅਚਲ p_Buf_ਸ਼ੁਰੂ = pBuf;     /* Start of original out buffer */
#ਜੇ_ਅੰਤ

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( eOp==0 || eOp==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  ਜੇ( pCur->ix>=p_ਪੰਨਾ->nCell ){
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorHoldsMutex(pCur) );

  getCellInfo(pCur);
  aPayload = pCur->info.pPayload;
  ਪ੍ਰਤਿਜ੍ਞਾ( (u64)ਵਿੱਥ+(u64)amt <= (u64)pCur->info.nPayload );

  ਪ੍ਰਤਿਜ੍ਞਾ( aPayload > p_ਪੰਨਾ->a_ਅੰਕੜੇ );
  ਜੇ( (uptr)(aPayload - p_ਪੰਨਾ->a_ਅੰਕੜੇ) > (pBt->usable_ਮਾਪ - pCur->info.nLocal) ){
    /* Trying to read or write past the end of the data is an error.  The
    ** conditional above is really:
    **    &aPayload[pCur->info.nLocal] > &pPage->aData[pBt->usableSize]
    ** but is recast into its current form to avoid integer overflow problems
    */
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
  }

  /* Check if data must be read/written to/from the btree page itself. */
  ਜੇ( ਵਿੱਥ<pCur->info.nLocal ){
    ਪੂਰਨ a = amt;
    ਜੇ( a+ਵਿੱਥ>pCur->info.nLocal ){
      a = pCur->info.nLocal - ਵਿੱਥ;
    }
    rc = copyPayload(&aPayload[ਵਿੱਥ], pBuf, a, eOp, p_ਪੰਨਾ->p_Db_ਪੰਨਾ);
    ਵਿੱਥ = 0;
    pBuf += a;
    amt -= a;
  }ਨਹੀਂਤਾਂ{
    ਵਿੱਥ -= pCur->info.nLocal;
  }


  ਜੇ( rc==SQLITE_OK && amt>0 ){
    ਅਚਲ u32 ovfl_ਮਾਪ = pBt->usable_ਮਾਪ - 4;  /* Bytes content per ovfl page */
    Pgno ਅਗਲਾ_ਪੰਨਾ;

    ਅਗਲਾ_ਪੰਨਾ = get4byte(&aPayload[pCur->info.nLocal]);

    /* If the BtCursor.aOverflow[] has not been allocated, allocate it now.
    **
    ** The aOverflow[] array is sized at one entry for each overflow page
    ** in the overflow chain. The page number of the first overflow page is
    ** stored in aOverflow[0], etc. A value of 0 in the aOverflow[] array
    ** means "not yet known" (the cache is lazily populated).
    */
    ਜੇ( (pCur->curFlags & BTCF_ValidOvfl)==0 ){
      i64 nOvfl = pCur->info.nPayload;
      testcase( nOvfl - pCur->info.nLocal + ovfl_ਮਾਪ - 1 > 0xffffffffU );
      nOvfl = (nOvfl - pCur->info.nLocal + ovfl_ਮਾਪ-1)/ovfl_ਮਾਪ;
      ਜੇ( pCur->a_ਉਛਾਲ==0
       || nOvfl*(ਪੂਰਨ)ਆਕਾਰ(Pgno) > sqlite3_Malloc_ਮਾਪ(pCur->a_ਉਛਾਲ)
      ){
        Pgno *aNew;
        ਜੇ( sqlite3FaultSim(413) ){
          aNew = 0;
        }ਨਹੀਂਤਾਂ{
          aNew = (Pgno*)sqlite3Realloc(pCur->a_ਉਛਾਲ, nOvfl*2*ਆਕਾਰ(Pgno));
        }
        ਜੇ( aNew==0 ){
          ਵਾਪਸ SQLITE_NOMEM_BKPT;
        }ਨਹੀਂਤਾਂ{
          pCur->a_ਉਛਾਲ = aNew;
        }
      }
      memset(pCur->a_ਉਛਾਲ, 0, nOvfl*ਆਕਾਰ(Pgno));
      pCur->curFlags |= BTCF_ValidOvfl;
    }ਨਹੀਂਤਾਂ{
      /* Sanity check the validity of the overflow page cache */
      ਪ੍ਰਤਿਜ੍ਞਾ( pCur->a_ਉਛਾਲ[0]==ਅਗਲਾ_ਪੰਨਾ
           || pCur->a_ਉਛਾਲ[0]==0
           || CORRUPT_DB );
      ਪ੍ਰਤਿਜ੍ਞਾ( pCur->a_ਉਛਾਲ[0]!=0 || pCur->a_ਉਛਾਲ[ਵਿੱਥ/ovfl_ਮਾਪ]==0 );

      /* If the overflow page-list cache has been allocated and the
      ** entry for the first required overflow page is valid, skip
      ** directly to it.
      */
      ਜੇ( pCur->a_ਉਛਾਲ[ਵਿੱਥ/ovfl_ਮਾਪ] ){
        iIdx = (ਵਿੱਥ/ovfl_ਮਾਪ);
        ਅਗਲਾ_ਪੰਨਾ = pCur->a_ਉਛਾਲ[iIdx];
        ਵਿੱਥ = (ਵਿੱਥ%ovfl_ਮਾਪ);
      }
    }

    ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK && amt>0 );
    ਜਦੋਂ( ਅਗਲਾ_ਪੰਨਾ ){
      /* If required, populate the overflow page-list cache. */
      ਜੇ( ਅਗਲਾ_ਪੰਨਾ > pBt->n_ਪੰਨਾ ) ਵਾਪਸ SQLITE_CORRUPT_BKPT;
      ਪ੍ਰਤਿਜ੍ਞਾ( pCur->a_ਉਛਾਲ[iIdx]==0
              || pCur->a_ਉਛਾਲ[iIdx]==ਅਗਲਾ_ਪੰਨਾ
              || CORRUPT_DB );
      pCur->a_ਉਛਾਲ[iIdx] = ਅਗਲਾ_ਪੰਨਾ;

      ਜੇ( ਵਿੱਥ>=ovfl_ਮਾਪ ){
        /* The only reason to read this page is to obtain the page
        ** number for the next page in the overflow chain. The page
        ** data is not required. So first try to lookup the overflow
        ** page-list cache, if any, then fall back to the getOverflowPage()
        ** function.
        */
        ਪ੍ਰਤਿਜ੍ਞਾ( pCur->curFlags & BTCF_ValidOvfl );
        ਪ੍ਰਤਿਜ੍ਞਾ( pCur->pBtree->db==pBt->db );
        ਜੇ( pCur->a_ਉਛਾਲ[iIdx+1] ){
          ਅਗਲਾ_ਪੰਨਾ = pCur->a_ਉਛਾਲ[iIdx+1];
        }ਨਹੀਂਤਾਂ{
          rc = get_ਉਛਾਲ_ਪੰਨਾ(pBt, ਅਗਲਾ_ਪੰਨਾ, 0, &ਅਗਲਾ_ਪੰਨਾ);
        }
        ਵਿੱਥ -= ovfl_ਮਾਪ;
      }ਨਹੀਂਤਾਂ{
        /* Need to read this page properly. It contains some of the
        ** range of data that is being read (eOp==0) or written (eOp!=0).
        */
        ਪੂਰਨ a = amt;
        ਜੇ( a + ਵਿੱਥ > ovfl_ਮਾਪ ){
          a = ovfl_ਮਾਪ - ਵਿੱਥ;
        }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ਸਿੱਧਾ_ਉਛਾਲ_READ
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
        ਜੇ( eOp==0                                             /* (1) */
         && ਵਿੱਥ==0                                          /* (2) */
         && sqlite3_Pager_ਸਿੱਧਾ_Read_Ok(pBt->pPager, ਅਗਲਾ_ਪੰਨਾ)    /* (3,4,5) */
         && &pBuf[-4]>=p_Buf_ਸ਼ੁਰੂ                               /* (6) */
        ){
          sqlite3_file *fd = sqlite3PagerFile(pBt->pPager);
          u8 aSave[4];
          u8 *aWrite = &pBuf[-4];
          ਪ੍ਰਤਿਜ੍ਞਾ( aWrite>=p_Buf_ਸ਼ੁਰੂ );                         /* due to (6) */
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(aSave, aWrite, 4);
          rc = sqlite3OsRead(fd, aWrite, a+4, (i64)pBt->ਪੰਨਾ_ਮਾਪ*(ਅਗਲਾ_ਪੰਨਾ-1));
          ਅਗਲਾ_ਪੰਨਾ = get4byte(aWrite);
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(aWrite, aSave, 4);
        }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ

        {
          Db_ਪੰਨਾ *p_Db_ਪੰਨਾ;
          rc = sqlite3PagerGet(pBt->pPager, ਅਗਲਾ_ਪੰਨਾ, &p_Db_ਪੰਨਾ,
              (eOp==0 ? PAGER_GET_READONLY : 0)
          );
          ਜੇ( rc==SQLITE_OK ){
            ਜੇ( eOp!=0
             && (sqlite3_Pager_ਪੰਨਾ_Refcount(p_Db_ਪੰਨਾ)!=1
                 || NEVER(((Mem_ਪੰਨਾ*)sqlite3PagerGetExtra(p_Db_ਪੰਨਾ))->isInit))
             && sqlite3FaultSim(411)==SQLITE_OK
            ){
              sqlite3PagerUnref(p_Db_ਪੰਨਾ);
              ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
            }
            aPayload = sqlite3_Pager_Get_ਅੰਕੜੇ(p_Db_ਪੰਨਾ);
            ਅਗਲਾ_ਪੰਨਾ = get4byte(aPayload);
            rc = copyPayload(&aPayload[ਵਿੱਥ+4], pBuf, a, eOp, p_Db_ਪੰਨਾ);
            sqlite3PagerUnref(p_Db_ਪੰਨਾ);
            ਵਿੱਥ = 0;
          }
        }
        amt -= a;
        ਜੇ( amt==0 ) ਵਾਪਸ rc;
        pBuf += a;
      }
      ਜੇ( rc ) ਰੋਕੋ;
      iIdx++;
    }
  }

  ਜੇ( rc==SQLITE_OK && amt>0 ){
    /* Overflow chain ends prematurely */
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
  }
  ਵਾਪਸ rc;
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
ਪੂਰਨ sqlite3BtreePayload(BtCursor *pCur, u32 ਵਿੱਥ, u32 amt, ਖਾਲੀ *pBuf){
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorHoldsMutex(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->i_ਪੰਨਾ>=0 && pCur->p_ਪੰਨਾ );
  ਵਾਪਸ accessPayload(pCur, ਵਿੱਥ, amt, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)pBuf, 0);
}

/*
** This variant of sqlite3BtreePayload() works even if the cursor has not
** in the CURSOR_VALID state.  It is only used by the sqlite3_blob_read()
** interface.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ accessPayloadChecked(
  BtCursor *pCur,
  u32 ਵਿੱਥ,
  u32 amt,
  ਖਾਲੀ *pBuf
){
  ਪੂਰਨ rc;
  ਜੇ ( pCur->eState==CURSOR_INVALID ){
    ਵਾਪਸ SQLITE_ABORT;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  rc = btreeRestoreCursorPosition(pCur);
  ਵਾਪਸ rc ? rc : accessPayload(pCur, ਵਿੱਥ, amt, pBuf, 0);
}
ਪੂਰਨ sqlite3BtreePayloadChecked(BtCursor *pCur, u32 ਵਿੱਥ, u32 amt, ਖਾਲੀ *pBuf){
  ਜੇ( pCur->eState==CURSOR_VALID ){
    ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
    ਵਾਪਸ accessPayload(pCur, ਵਿੱਥ, amt, pBuf, 0);
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ accessPayloadChecked(pCur, ਵਿੱਥ, amt, pBuf);
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_INCRBLOB */

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
ਸਥਿਰ ਅਚਲ ਖਾਲੀ *fetchPayload(
  BtCursor *pCur,      /* Cursor pointing to entry to read from */
  u32 *pAmt            /* Write the number of available bytes here */
){
  ਪੂਰਨ amt;
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur!=0 && pCur->i_ਪੰਨਾ>=0 && pCur->p_ਪੰਨਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->ix<pCur->p_ਪੰਨਾ->nCell || CORRUPT_DB );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->info.n_ਮਾਪ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->info.pPayload>pCur->p_ਪੰਨਾ->a_ਅੰਕੜੇ || CORRUPT_DB );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->info.pPayload<pCur->p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ ||CORRUPT_DB);
  amt = pCur->info.nLocal;
  ਜੇ( amt>(ਪੂਰਨ)(pCur->p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ - pCur->info.pPayload) ){
    /* There is too little space on the page for the expected amount
    ** of local content. Database must be corrupt. */
    ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB );
    amt = MAX(0, (ਪੂਰਨ)(pCur->p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ - pCur->info.pPayload));
  }
  *pAmt = (u32)amt;
  ਵਾਪਸ (ਖਾਲੀ*)pCur->info.pPayload;
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
ਅਚਲ ਖਾਲੀ *sqlite3BtreePayloadFetch(BtCursor *pCur, u32 *pAmt){
  ਵਾਪਸ fetchPayload(pCur, pAmt);
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
ਸਥਿਰ ਪੂਰਨ move_ਮੰਜ਼ਿਲ_ਬੱਚਾ(BtCursor *pCur, u32 newPgno){
  ਪੂਰਨ rc;
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->i_ਪੰਨਾ<BTCURSOR_MAX_ਡੂੰਘਾਈ );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->i_ਪੰਨਾ>=0 );
  ਜੇ( pCur->i_ਪੰਨਾ>=(BTCURSOR_MAX_ਡੂੰਘਾਈ-1) ){
    ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  }
  pCur->info.n_ਮਾਪ = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  pCur->aiIdx[pCur->i_ਪੰਨਾ] = pCur->ix;
  pCur->ap_ਪੰਨਾ[pCur->i_ਪੰਨਾ] = pCur->p_ਪੰਨਾ;
  pCur->ix = 0;
  pCur->i_ਪੰਨਾ++;
  rc = get_And_Init_ਪੰਨਾ(pCur->pBt, newPgno, &pCur->p_ਪੰਨਾ, pCur->curPagerFlags);
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਪੰਨਾ!=0 || rc!=SQLITE_OK );
  ਜੇ( rc==SQLITE_OK
   && (pCur->p_ਪੰਨਾ->nCell<1 || pCur->p_ਪੰਨਾ->int_ਕੁੰਜੀ!=pCur->cur_Int_ਕੁੰਜੀ)
  ){
    release_ਪੰਨਾ(pCur->p_ਪੰਨਾ);
    rc = SQLITE_CORRUPT_PGNO(newPgno);
  }
  ਜੇ( rc ){
    pCur->p_ਪੰਨਾ = pCur->ap_ਪੰਨਾ[--pCur->i_ਪੰਨਾ];
  }
  ਵਾਪਸ rc;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** Page pParent is an internal (non-leaf) tree page. This function
** asserts that page number iChild is the left-child if the iIdx'th
** cell in page pParent. Or, if iIdx is equal to the total number of
** cells in pParent, that page number iChild is the right-child of
** the page.
*/
ਸਥਿਰ ਖਾਲੀ assert_ਮਾਪਾ_ਸੂਚਕਾਂਕ(Mem_ਪੰਨਾ *p_ਮਾਪਾ, ਪੂਰਨ iIdx, Pgno i_ਬੱਚਾ){
  ਜੇ( CORRUPT_DB ) ਵਾਪਸ;  /* The conditions tested below might not be true
                            ** in a corrupt database */
  ਪ੍ਰਤਿਜ੍ਞਾ( iIdx<=p_ਮਾਪਾ->nCell );
  ਜੇ( iIdx==p_ਮਾਪਾ->nCell ){
    ਪ੍ਰਤਿਜ੍ਞਾ( get4byte(&p_ਮਾਪਾ->a_ਅੰਕੜੇ[p_ਮਾਪਾ->hdr_ਵਿੱਥ+8])==i_ਬੱਚਾ );
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( get4byte(ਲੱਭੋ_Cell(p_ਮਾਪਾ, iIdx))==i_ਬੱਚਾ );
  }
}
#ਨਹੀਂਤਾਂ
#  define assert_ਮਾਪਾ_ਸੂਚਕਾਂਕ(x,y,z)
#ਜੇ_ਅੰਤ

/*
** Move the cursor up to the parent page.
**
** pCur->idx is set to the cell index that contains the pointer
** to the page we are coming from.  If we are coming from the
** right-most child page then pCur->idx is set to one more than
** the largest cell index.
*/
ਸਥਿਰ ਖਾਲੀ move_ਮੰਜ਼ਿਲ_ਮਾਪਾ(BtCursor *pCur){
  Mem_ਪੰਨਾ *p_ਪੱਤੀ;
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->i_ਪੰਨਾ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਪੰਨਾ );
  assert_ਮਾਪਾ_ਸੂਚਕਾਂਕ(
    pCur->ap_ਪੰਨਾ[pCur->i_ਪੰਨਾ-1],
    pCur->aiIdx[pCur->i_ਪੰਨਾ-1],
    pCur->p_ਪੰਨਾ->pgno
  );
  testcase( pCur->aiIdx[pCur->i_ਪੰਨਾ-1] > pCur->ap_ਪੰਨਾ[pCur->i_ਪੰਨਾ-1]->nCell );
  pCur->info.n_ਮਾਪ = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  pCur->ix = pCur->aiIdx[pCur->i_ਪੰਨਾ-1];
  p_ਪੱਤੀ = pCur->p_ਪੰਨਾ;
  pCur->p_ਪੰਨਾ = pCur->ap_ਪੰਨਾ[--pCur->i_ਪੰਨਾ];
  release_ਪੰਨਾ_Not_Null(p_ਪੱਤੀ);
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
ਸਥਿਰ ਪੂਰਨ move_ਮੰਜ਼ਿਲ_ਜੜ੍ਹ(BtCursor *pCur){
  Mem_ਪੰਨਾ *p_ਜੜ੍ਹ;
  ਪੂਰਨ rc = SQLITE_OK;

  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( CURSOR_INVALID < CURSOR_REQUIRESEEK );
  ਪ੍ਰਤਿਜ੍ਞਾ( CURSOR_VALID   < CURSOR_REQUIRESEEK );
  ਪ੍ਰਤਿਜ੍ਞਾ( CURSOR_FAULT   > CURSOR_REQUIRESEEK );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState < CURSOR_REQUIRESEEK || pCur->i_ਪੰਨਾ<0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->pgno_ਜੜ੍ਹ>0 || pCur->i_ਪੰਨਾ<0 );

  ਜੇ( pCur->i_ਪੰਨਾ>=0 ){
    ਜੇ( pCur->i_ਪੰਨਾ ){
      release_ਪੰਨਾ_Not_Null(pCur->p_ਪੰਨਾ);
      ਜਦੋਂ( --pCur->i_ਪੰਨਾ ){
        release_ਪੰਨਾ_Not_Null(pCur->ap_ਪੰਨਾ[pCur->i_ਪੰਨਾ]);
      }
      p_ਜੜ੍ਹ = pCur->p_ਪੰਨਾ = pCur->ap_ਪੰਨਾ[0];
      ਜਾਓ skip_init;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( pCur->pgno_ਜੜ੍ਹ==0 ){
    pCur->eState = CURSOR_INVALID;
    ਵਾਪਸ SQLITE_ਖਾਲੀ_ਥਾਵਾਂ;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pCur->i_ਪੰਨਾ==(-1) );
    ਜੇ( pCur->eState>=CURSOR_REQUIRESEEK ){
      ਜੇ( pCur->eState==CURSOR_FAULT ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pCur->skip_ਅਗਲਾ!=SQLITE_OK );
        ਵਾਪਸ pCur->skip_ਅਗਲਾ;
      }
      sqlite3BtreeClearCursor(pCur);
    }
    rc = get_And_Init_ਪੰਨਾ(pCur->pBt, pCur->pgno_ਜੜ੍ਹ, &pCur->p_ਪੰਨਾ,
                        pCur->curPagerFlags);
    ਜੇ( rc!=SQLITE_OK ){
      pCur->eState = CURSOR_INVALID;
      ਵਾਪਸ rc;
    }
    pCur->i_ਪੰਨਾ = 0;
    pCur->cur_Int_ਕੁੰਜੀ = pCur->p_ਪੰਨਾ->int_ਕੁੰਜੀ;
  }
  p_ਜੜ੍ਹ = pCur->p_ਪੰਨਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਜੜ੍ਹ->pgno==pCur->pgno_ਜੜ੍ਹ || CORRUPT_DB );

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
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਜੜ੍ਹ->int_ਕੁੰਜੀ==1 || p_ਜੜ੍ਹ->int_ਕੁੰਜੀ==0 );
  ਜੇ( p_ਜੜ੍ਹ->isInit==0 || (pCur->p_ਕੁੰਜੀ_Info==0)!=p_ਜੜ੍ਹ->int_ਕੁੰਜੀ ){
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(pCur->p_ਪੰਨਾ);
  }

skip_init: 
  pCur->ix = 0;
  pCur->info.n_ਮਾਪ = 0;
  pCur->curFlags &= ~(BTCF_At_ਆਖਰੀ|BTCF_ValidNKey|BTCF_ValidOvfl);

  ਜੇ( p_ਜੜ੍ਹ->nCell>0 ){
    pCur->eState = CURSOR_VALID;
  }ਨਹੀਂਤਾਂ ਜੇ( !p_ਜੜ੍ਹ->ਪੱਤੀ ){
    Pgno subpage;
    ਜੇ( p_ਜੜ੍ਹ->pgno!=1 ) ਵਾਪਸ SQLITE_CORRUPT_BKPT;
    subpage = get4byte(&p_ਜੜ੍ਹ->a_ਅੰਕੜੇ[p_ਜੜ੍ਹ->hdr_ਵਿੱਥ+8]);
    pCur->eState = CURSOR_VALID;
    rc = move_ਮੰਜ਼ਿਲ_ਬੱਚਾ(pCur, subpage);
  }ਨਹੀਂਤਾਂ{
    pCur->eState = CURSOR_INVALID;
    rc = SQLITE_ਖਾਲੀ_ਥਾਵਾਂ;
  }
  ਵਾਪਸ rc;
}

/*
** Move the cursor down to the left-most leaf entry beneath the
** entry to which it is currently pointing.
**
** The left-most leaf is the one with the smallest key - the first
** in ascending order.
*/
ਸਥਿਰ ਪੂਰਨ move_ਮੰਜ਼ਿਲ_Leftmost(BtCursor *pCur){
  Pgno pgno;
  ਪੂਰਨ rc = SQLITE_OK;
  Mem_ਪੰਨਾ *p_ਪੰਨਾ;

  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  ਜਦੋਂ( rc==SQLITE_OK && !(p_ਪੰਨਾ = pCur->p_ਪੰਨਾ)->ਪੱਤੀ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pCur->ix<p_ਪੰਨਾ->nCell );
    pgno = get4byte(ਲੱਭੋ_Cell(p_ਪੰਨਾ, pCur->ix));
    rc = move_ਮੰਜ਼ਿਲ_ਬੱਚਾ(pCur, pgno);
  }
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ move_ਮੰਜ਼ਿਲ_Rightmost(BtCursor *pCur){
  Pgno pgno;
  ਪੂਰਨ rc = SQLITE_OK;
  Mem_ਪੰਨਾ *p_ਪੰਨਾ = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  ਜਦੋਂ( !(p_ਪੰਨਾ = pCur->p_ਪੰਨਾ)->ਪੱਤੀ ){
    pgno = get4byte(&p_ਪੰਨਾ->a_ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+8]);
    pCur->ix = p_ਪੰਨਾ->nCell;
    rc = move_ਮੰਜ਼ਿਲ_ਬੱਚਾ(pCur, pgno);
    ਜੇ( rc ) ਵਾਪਸ rc;
  }
  pCur->ix = p_ਪੰਨਾ->nCell-1;
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->info.n_ਮਾਪ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pCur->curFlags & BTCF_ValidNKey)==0 );
  ਵਾਪਸ SQLITE_OK;
}

/* Move the cursor to the first entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
ਪੂਰਨ sqlite3_Btree_ਪਹਿਲਾ(BtCursor *pCur, ਪੂਰਨ *pRes){
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  rc = move_ਮੰਜ਼ਿਲ_ਜੜ੍ਹ(pCur);
  ਜੇ( rc==SQLITE_OK ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਪੰਨਾ->nCell>0 );
    *pRes = 0;
    rc = move_ਮੰਜ਼ਿਲ_Leftmost(pCur);
  }ਨਹੀਂਤਾਂ ਜੇ( rc==SQLITE_ਖਾਲੀ_ਥਾਵਾਂ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pCur->pgno_ਜੜ੍ਹ==0 || (pCur->p_ਪੰਨਾ!=0 && pCur->p_ਪੰਨਾ->nCell==0) );
    *pRes = 1;
    rc = SQLITE_OK;
  }
  ਵਾਪਸ rc;
}

/* Set *pRes to 1 (true) if the BTree pointed to by cursor pCur contains zero
** rows of content.  Set *pRes to 0 (false) if the table contains content.
** Return SQLITE_OK on success or some error code (ex: SQLITE_NOMEM) if
** something goes wrong.
*/
ਪੂਰਨ sqlite3_Btree_Is_ਖਾਲੀ_ਥਾਵਾਂ(BtCursor *pCur, ਪੂਰਨ *pRes){
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ਜੇ( NEVER(pCur->eState==CURSOR_VALID) ){
    *pRes = 0;
    ਵਾਪਸ SQLITE_OK;
  }
  rc = move_ਮੰਜ਼ਿਲ_ਜੜ੍ਹ(pCur);
  ਜੇ( rc==SQLITE_ਖਾਲੀ_ਥਾਵਾਂ ){
    *pRes = 1;
    rc = SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    *pRes = 0;
  }
  ਵਾਪਸ rc;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/* The cursors is CURSOR_VALID and has BTCF_AtLast set.  Verify that
** this flags are true for a consistent database.
**
** This routine is is called from within assert() statements only.
** It is an internal verification routine and does not appear in production
** builds.
*/
ਸਥਿਰ ਪੂਰਨ cursor_Is_At_ਆਖਰੀ_ਇੰਦਰਾਜ(BtCursor *pCur){
  ਪੂਰਨ ii;
  ਲਈ(ii=0; ii<pCur->i_ਪੰਨਾ; ii++){
    ਜੇ( pCur->aiIdx[ii]!=pCur->ap_ਪੰਨਾ[ii]->nCell ) ਵਾਪਸ 0;
  }
  ਵਾਪਸ pCur->ix==pCur->p_ਪੰਨਾ->nCell-1 && pCur->p_ਪੰਨਾ->ਪੱਤੀ!=0;
}
#ਜੇ_ਅੰਤ

/* Move the cursor to the last entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ btree_ਆਖਰੀ(BtCursor *pCur, ਪੂਰਨ *pRes){
  ਪੂਰਨ rc = move_ਮੰਜ਼ਿਲ_ਜੜ੍ਹ(pCur);
  ਜੇ( rc==SQLITE_OK ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
    *pRes = 0;
    rc = move_ਮੰਜ਼ਿਲ_Rightmost(pCur);
    ਜੇ( rc==SQLITE_OK ){
      pCur->curFlags |= BTCF_At_ਆਖਰੀ;
    }ਨਹੀਂਤਾਂ{
      pCur->curFlags &= ~BTCF_At_ਆਖਰੀ;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( rc==SQLITE_ਖਾਲੀ_ਥਾਵਾਂ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pCur->pgno_ਜੜ੍ਹ==0 || pCur->p_ਪੰਨਾ->nCell==0 );
    *pRes = 1;
    rc = SQLITE_OK;
  }
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3_Btree_ਆਖਰੀ(BtCursor *pCur, ਪੂਰਨ *pRes){
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* If the cursor already points to the last entry, this is a no-op. */
  ਜੇ( CURSOR_VALID==pCur->eState && (pCur->curFlags & BTCF_At_ਆਖਰੀ)!=0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( cursor_Is_At_ਆਖਰੀ_ਇੰਦਰਾਜ(pCur) || CORRUPT_DB );
    *pRes = 0;
    ਵਾਪਸ SQLITE_OK;
  }
  ਵਾਪਸ btree_ਆਖਰੀ(pCur, pRes);
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
ਪੂਰਨ sqlite3_Btree_ਸਾਰਣੀ_Moveto(
  BtCursor *pCur,          /* The cursor to be moved */
  i64 int_ਕੁੰਜੀ,              /* The table key */
  ਪੂਰਨ bias_ਸੱਜਾ,           /* If true, bias the search to the high end */
  ਪੂਰਨ *pRes                /* Write search results here */
){
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pRes );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਕੁੰਜੀ_Info==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState!=CURSOR_VALID || pCur->cur_Int_ਕੁੰਜੀ!=0 );

  /* If the cursor is already positioned at the point we are trying
  ** to move to, then just return without doing any work */
  ਜੇ( pCur->eState==CURSOR_VALID && (pCur->curFlags & BTCF_ValidNKey)!=0 ){
    ਜੇ( pCur->info.n_ਕੁੰਜੀ==int_ਕੁੰਜੀ ){
      *pRes = 0;
      ਵਾਪਸ SQLITE_OK;
    }
    ਜੇ( pCur->info.n_ਕੁੰਜੀ<int_ਕੁੰਜੀ ){
      ਜੇ( (pCur->curFlags & BTCF_At_ਆਖਰੀ)!=0 ){
        ਪ੍ਰਤਿਜ੍ਞਾ( cursor_Is_At_ਆਖਰੀ_ਇੰਦਰਾਜ(pCur) || CORRUPT_DB );
        *pRes = -1;
        ਵਾਪਸ SQLITE_OK;
      }
      /* If the requested key is one more than the previous key, then
      ** try to get there using sqlite3BtreeNext() rather than a full
      ** binary search.  This is an optimization only.  The correct answer
      ** is still obtained without this case, only a little more slowly. */
      ਜੇ( pCur->info.n_ਕੁੰਜੀ+1==int_ਕੁੰਜੀ ){
        *pRes = 0;
        rc = sqlite3_Btree_ਅਗਲਾ(pCur, 0);
        ਜੇ( rc==SQLITE_OK ){
          getCellInfo(pCur);
          ਜੇ( pCur->info.n_ਕੁੰਜੀ==int_ਕੁੰਜੀ ){
            ਵਾਪਸ SQLITE_OK;
          }
        }ਨਹੀਂਤਾਂ ਜੇ( rc!=SQLITE_ਪੂਰਾ ){
          ਵਾਪਸ rc;
        }
      }
    }
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#ਜੇ_ਅੰਤ

  rc = move_ਮੰਜ਼ਿਲ_ਜੜ੍ਹ(pCur);
  ਜੇ( rc ){
    ਜੇ( rc==SQLITE_ਖਾਲੀ_ਥਾਵਾਂ ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pCur->pgno_ਜੜ੍ਹ==0 || pCur->p_ਪੰਨਾ->nCell==0 );
      *pRes = -1;
      ਵਾਪਸ SQLITE_OK;
    }
    ਵਾਪਸ rc;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਪੰਨਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਪੰਨਾ->isInit );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਪੰਨਾ->nCell > 0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->i_ਪੰਨਾ==0 || pCur->ap_ਪੰਨਾ[0]->int_ਕੁੰਜੀ==pCur->cur_Int_ਕੁੰਜੀ );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->cur_Int_ਕੁੰਜੀ );

  ਲਈ(;;){
    ਪੂਰਨ lwr, upr, idx, c;
    Pgno chldPg;
    Mem_ਪੰਨਾ *p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nCell>0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->int_ਕੁੰਜੀ );
    lwr = 0;
    upr = p_ਪੰਨਾ->nCell-1;
    ਪ੍ਰਤਿਜ੍ਞਾ( bias_ਸੱਜਾ==0 || bias_ਸੱਜਾ==1 );
    idx = upr>>(1-bias_ਸੱਜਾ); /* idx = biasRight ? upr : (lwr+upr)/2; */
    ਲਈ(;;){
      i64 n_Cell_ਕੁੰਜੀ;
      pCell = ਲੱਭੋ_Cell_Past_Ptr(p_ਪੰਨਾ, idx);
      ਜੇ( p_ਪੰਨਾ->int_ਕੁੰਜੀ_ਪੱਤੀ ){
        ਜਦੋਂ( 0x80 <= *(pCell++) ){
          ਜੇ( pCell>=p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ ){
            ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
          }
        }
      }
      n_Cell_ਕੁੰਜੀ = sqlite3_Varint_ਮੁੱਲ(pCell);
      ਜੇ( n_Cell_ਕੁੰਜੀ<int_ਕੁੰਜੀ ){
        lwr = idx+1;
        ਜੇ( lwr>upr ){ c = -1; ਰੋਕੋ; }
      }ਨਹੀਂਤਾਂ ਜੇ( n_Cell_ਕੁੰਜੀ>int_ਕੁੰਜੀ ){
        upr = idx-1;
        ਜੇ( lwr>upr ){ c = +1; ਰੋਕੋ; }
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( n_Cell_ਕੁੰਜੀ==int_ਕੁੰਜੀ );
        pCur->ix = (u16)idx;
        ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
          lwr = idx;
          ਜਾਓ moveto_ਸਾਰਣੀ_ਅਗਲਾ_layer;
        }ਨਹੀਂਤਾਂ{
          pCur->curFlags |= BTCF_ValidNKey;
          pCur->info.n_ਕੁੰਜੀ = n_Cell_ਕੁੰਜੀ;
          pCur->info.n_ਮਾਪ = 0;
          *pRes = 0;
          ਵਾਪਸ SQLITE_OK;
        }
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2; */
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( lwr==upr+1 || !p_ਪੰਨਾ->ਪੱਤੀ );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->isInit );
    ਜੇ( p_ਪੰਨਾ->ਪੱਤੀ ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pCur->ix<pCur->p_ਪੰਨਾ->nCell );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      ਜਾਓ moveto_ਸਾਰਣੀ_ਅੰਤ;
    }
moveto_ਸਾਰਣੀ_ਅਗਲਾ_layer:
    ਜੇ( lwr>=p_ਪੰਨਾ->nCell ){
      chldPg = get4byte(&p_ਪੰਨਾ->a_ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+8]);
    }ਨਹੀਂਤਾਂ{
      chldPg = get4byte(ਲੱਭੋ_Cell(p_ਪੰਨਾ, lwr));
    }
    pCur->ix = (u16)lwr;
    rc = move_ਮੰਜ਼ਿਲ_ਬੱਚਾ(pCur, chldPg);
    ਜੇ( rc ) ਰੋਕੋ;
  }
moveto_ਸਾਰਣੀ_ਅੰਤ:
  pCur->info.n_ਮਾਪ = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( (pCur->curFlags & BTCF_ValidOvfl)==0 );
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ ਸੂਚਕਾਂਕ_Cell_Compare(
  Mem_ਪੰਨਾ *p_ਪੰਨਾ,
  ਪੂਰਨ idx,
  Unpacked_ਰਿਕਾਰਡ *p_Idx_ਕੁੰਜੀ,
  ਰਿਕਾਰਡ_Compare x_ਰਿਕਾਰਡ_Compare
){
  ਪੂਰਨ c;
  ਪੂਰਨ nCell;  /* Size of the pCell cell in bytes */
  u8 *pCell = ਲੱਭੋ_Cell_Past_Ptr(p_ਪੰਨਾ, idx);

  nCell = pCell[0];
  ਜੇ( nCell<=p_ਪੰਨਾ->max1bytePayload ){
    /* This branch runs if the record-size field of the cell is a
    ** single byte varint and the record fits entirely on the main
    ** b-tree page.  */
    ਜੇ( pCell + nCell >= p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ ) ਵਾਪਸ 99;
    c = x_ਰਿਕਾਰਡ_Compare(nCell, (ਖਾਲੀ*)&pCell[1], p_Idx_ਕੁੰਜੀ);
  }ਨਹੀਂਤਾਂ ਜੇ( !(pCell[1] & 0x80)
    && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=p_ਪੰਨਾ->maxLocal
  ){
    /* The record-size field is a 2 byte varint and the record
    ** fits entirely on the main b-tree page.  */
    ਜੇ( pCell + nCell >= p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ ) ਵਾਪਸ 99;
    c = x_ਰਿਕਾਰਡ_Compare(nCell, (ਖਾਲੀ*)&pCell[2], p_Idx_ਕੁੰਜੀ);
  }ਨਹੀਂਤਾਂ{
    /* If the record extends into overflow pages, do not attempt
    ** the optimization. */
    c = 99;
  }
  ਵਾਪਸ c;
}

/*
** Return true (non-zero) if pCur is current pointing to the last
** page of a table.
*/
ਸਥਿਰ ਪੂਰਨ cursor_On_ਆਖਰੀ_ਪੰਨਾ(BtCursor *pCur){
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  ਲਈ(i=0; i<pCur->i_ਪੰਨਾ; i++){
    Mem_ਪੰਨਾ *p_ਪੰਨਾ = pCur->ap_ਪੰਨਾ[i];
    ਜੇ( pCur->aiIdx[i]<p_ਪੰਨਾ->nCell ) ਵਾਪਸ 0;
  }
  ਵਾਪਸ 1;
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
ਪੂਰਨ sqlite3_Btree_ਸੂਚਕਾਂਕ_Moveto(
  BtCursor *pCur,          /* The cursor to be moved */
  Unpacked_ਰਿਕਾਰਡ *p_Idx_ਕੁੰਜੀ, /* Unpacked index key */
  ਪੂਰਨ *pRes                /* Write search results here */
){
  ਪੂਰਨ rc;
  ਰਿਕਾਰਡ_Compare x_ਰਿਕਾਰਡ_Compare;

  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pRes );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਕੁੰਜੀ_Info!=0 );

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#ਜੇ_ਅੰਤ

  x_ਰਿਕਾਰਡ_Compare = sqlite3_Vdbe_ਲੱਭੋ_Compare(p_Idx_ਕੁੰਜੀ);
  p_Idx_ਕੁੰਜੀ->err_ਕੋਡ = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Idx_ਕੁੰਜੀ->default_rc==1
       || p_Idx_ਕੁੰਜੀ->default_rc==0
       || p_Idx_ਕੁੰਜੀ->default_rc==-1
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
  ਜੇ( pCur->eState==CURSOR_VALID
   && pCur->p_ਪੰਨਾ->ਪੱਤੀ
   && cursor_On_ਆਖਰੀ_ਪੰਨਾ(pCur)
  ){
    ਪੂਰਨ c;
    ਜੇ( pCur->ix==pCur->p_ਪੰਨਾ->nCell-1
     && (c = ਸੂਚਕਾਂਕ_Cell_Compare(pCur->p_ਪੰਨਾ,pCur->ix,p_Idx_ਕੁੰਜੀ,x_ਰਿਕਾਰਡ_Compare))<=0
     && p_Idx_ਕੁੰਜੀ->err_ਕੋਡ==SQLITE_OK
    ){
      *pRes = c;
      ਵਾਪਸ SQLITE_OK;  /* Cursor already pointing at the correct spot */
    }
    ਜੇ( pCur->i_ਪੰਨਾ>0
     && ਸੂਚਕਾਂਕ_Cell_Compare(pCur->p_ਪੰਨਾ, 0, p_Idx_ਕੁੰਜੀ, x_ਰਿਕਾਰਡ_Compare)<=0
     && p_Idx_ਕੁੰਜੀ->err_ਕੋਡ==SQLITE_OK
    ){
      pCur->curFlags &= ~(BTCF_ValidOvfl|BTCF_At_ਆਖਰੀ);
      ਜੇ( !pCur->p_ਪੰਨਾ->isInit ){
        ਵਾਪਸ SQLITE_CORRUPT_BKPT;
      }
      ਜਾਓ bypass_moveto_ਜੜ੍ਹ;  /* Start search on the current page */
    }
    p_Idx_ਕੁੰਜੀ->err_ਕੋਡ = SQLITE_OK;
  }

  rc = move_ਮੰਜ਼ਿਲ_ਜੜ੍ਹ(pCur);
  ਜੇ( rc ){
    ਜੇ( rc==SQLITE_ਖਾਲੀ_ਥਾਵਾਂ ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pCur->pgno_ਜੜ੍ਹ==0 || pCur->p_ਪੰਨਾ->nCell==0 );
      *pRes = -1;
      ਵਾਪਸ SQLITE_OK;
    }
    ਵਾਪਸ rc;
  }

bypass_moveto_ਜੜ੍ਹ:
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਪੰਨਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਪੰਨਾ->isInit );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਪੰਨਾ->nCell > 0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->cur_Int_ਕੁੰਜੀ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_Idx_ਕੁੰਜੀ!=0 );
  ਲਈ(;;){
    ਪੂਰਨ lwr, upr, idx, c;
    Pgno chldPg;
    Mem_ਪੰਨਾ *p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nCell>0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->int_ਕੁੰਜੀ==0 );
    lwr = 0;
    upr = p_ਪੰਨਾ->nCell-1;
    idx = upr>>1; /* idx = (lwr+upr)/2; */
    ਲਈ(;;){
      ਪੂਰਨ nCell;  /* Size of the pCell cell in bytes */
      pCell = ਲੱਭੋ_Cell_Past_Ptr(p_ਪੰਨਾ, idx);

      /* The maximum supported page-size is 65536 bytes. This means that
      ** the maximum number of record bytes stored on an index B-Tree
      ** page is less than 16384 bytes and may be stored as a 2-byte
      ** varint. This information is used to attempt to avoid parsing
      ** the entire cell by checking for the cases where the record is
      ** stored entirely within the b-tree page by inspecting the first
      ** 2 bytes of the cell.
      */
      nCell = pCell[0];
      ਜੇ( nCell<=p_ਪੰਨਾ->max1bytePayload ){
        /* This branch runs if the record-size field of the cell is a
        ** single byte varint and the record fits entirely on the main
        ** b-tree page.  */
        ਜੇ( pCell + nCell >= p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ ){
          rc = SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
          ਜਾਓ moveto_ਸੂਚਕਾਂਕ_ਅੰਤ;
        }
        c = x_ਰਿਕਾਰਡ_Compare(nCell, (ਖਾਲੀ*)&pCell[1], p_Idx_ਕੁੰਜੀ);
      }ਨਹੀਂਤਾਂ ਜੇ( !(pCell[1] & 0x80)
        && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=p_ਪੰਨਾ->maxLocal
        && pCell + nCell < p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ
      ){
        /* The record-size field is a 2 byte varint and the record
        ** fits entirely on the main b-tree page.  */
        c = x_ਰਿਕਾਰਡ_Compare(nCell, (ਖਾਲੀ*)&pCell[2], p_Idx_ਕੁੰਜੀ);
      }ਨਹੀਂਤਾਂ{
        /* The record flows over onto one or more overflow pages. In
        ** this case the whole cell needs to be parsed, a buffer allocated
        ** and accessPayload() used to retrieve the record into the
        ** buffer before VdbeRecordCompare() can be called.
        **
        ** If the record is corrupt, the xRecordCompare routine may read
        ** up to two varints past the end of the buffer. An extra 18
        ** bytes of padding is allocated at the end of the buffer in
        ** case this happens.  */
        ਖਾਲੀ *p_Cell_ਕੁੰਜੀ;
        u8 * ਅਚਲ pCellBody = pCell - p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ;
        ਅਚਲ ਪੂਰਨ nOverrun = 18;  /* Size of the overrun padding */
        p_ਪੰਨਾ->xParseCell(p_ਪੰਨਾ, pCellBody, &pCur->info);
        nCell = (ਪੂਰਨ)pCur->info.n_ਕੁੰਜੀ;
        testcase( nCell<0 );   /* True if key size is 2^32 or more */
        testcase( nCell==0 );  /* Invalid key size:  0x80 0x80 0x00 */
        testcase( nCell==1 );  /* Invalid key size:  0x80 0x80 0x01 */
        testcase( nCell==2 );  /* Minimum legal index key size */
        ਜੇ( nCell<2 || nCell/pCur->pBt->usable_ਮਾਪ>pCur->pBt->n_ਪੰਨਾ ){
          rc = SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
          ਜਾਓ moveto_ਸੂਚਕਾਂਕ_ਅੰਤ;
        }
        p_Cell_ਕੁੰਜੀ = sqlite3Malloc( (u64)nCell+(u64)nOverrun );
        ਜੇ( p_Cell_ਕੁੰਜੀ==0 ){
          rc = SQLITE_NOMEM_BKPT;
          ਜਾਓ moveto_ਸੂਚਕਾਂਕ_ਅੰਤ;
        }
        pCur->ix = (u16)idx;
        rc = accessPayload(pCur, 0, nCell, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)p_Cell_ਕੁੰਜੀ, 0);
        memset(((u8*)p_Cell_ਕੁੰਜੀ)+nCell,0,nOverrun); /* Fix uninit warnings */
        pCur->curFlags &= ~BTCF_ValidOvfl;
        ਜੇ( rc ){
          sqlite3_free(p_Cell_ਕੁੰਜੀ);
          ਜਾਓ moveto_ਸੂਚਕਾਂਕ_ਅੰਤ;
        }
        c = sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare(nCell, p_Cell_ਕੁੰਜੀ, p_Idx_ਕੁੰਜੀ);
        sqlite3_free(p_Cell_ਕੁੰਜੀ);
      }
      ਪ੍ਰਤਿਜ੍ਞਾ(
          (p_Idx_ਕੁੰਜੀ->err_ਕੋਡ!=SQLITE_CORRUPT || c==0)
       && (p_Idx_ਕੁੰਜੀ->err_ਕੋਡ!=SQLITE_NOMEM || pCur->pBtree->db->mallocFailed)
      );
      ਜੇ( c<0 ){
        lwr = idx+1;
      }ਨਹੀਂਤਾਂ ਜੇ( c>0 ){
        upr = idx-1;
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( c==0 );
        *pRes = 0;
        rc = SQLITE_OK;
        pCur->ix = (u16)idx;
        ਜੇ( p_Idx_ਕੁੰਜੀ->err_ਕੋਡ ) rc = SQLITE_CORRUPT_BKPT;
        ਜਾਓ moveto_ਸੂਚਕਾਂਕ_ਅੰਤ;
      }
      ਜੇ( lwr>upr ) ਰੋਕੋ;
      ਪ੍ਰਤਿਜ੍ਞਾ( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2 */
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( lwr==upr+1 || (p_ਪੰਨਾ->int_ਕੁੰਜੀ && !p_ਪੰਨਾ->ਪੱਤੀ) );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->isInit );
    ਜੇ( p_ਪੰਨਾ->ਪੱਤੀ ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pCur->ix<pCur->p_ਪੰਨਾ->nCell || CORRUPT_DB );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      ਜਾਓ moveto_ਸੂਚਕਾਂਕ_ਅੰਤ;
    }
    ਜੇ( lwr>=p_ਪੰਨਾ->nCell ){
      chldPg = get4byte(&p_ਪੰਨਾ->a_ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+8]);
    }ਨਹੀਂਤਾਂ{
      chldPg = get4byte(ਲੱਭੋ_Cell(p_ਪੰਨਾ, lwr));
    }

    /* This block is similar to an in-lined version of:
    **
    **    pCur->ix = (u16)lwr;
    **    rc = moveToChild(pCur, chldPg);
    **    if( rc ) break;
    */
    pCur->info.n_ਮਾਪ = 0;
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
    ਜੇ( pCur->i_ਪੰਨਾ>=(BTCURSOR_MAX_ਡੂੰਘਾਈ-1) ){
      ਵਾਪਸ SQLITE_CORRUPT_BKPT;
    }
    pCur->aiIdx[pCur->i_ਪੰਨਾ] = (u16)lwr;
    pCur->ap_ਪੰਨਾ[pCur->i_ਪੰਨਾ] = pCur->p_ਪੰਨਾ;
    pCur->ix = 0;
    pCur->i_ਪੰਨਾ++;
    rc = get_And_Init_ਪੰਨਾ(pCur->pBt, chldPg, &pCur->p_ਪੰਨਾ, pCur->curPagerFlags);
    ਜੇ( rc==SQLITE_OK
     && (pCur->p_ਪੰਨਾ->nCell<1 || pCur->p_ਪੰਨਾ->int_ਕੁੰਜੀ!=pCur->cur_Int_ਕੁੰਜੀ)
    ){
      release_ਪੰਨਾ(pCur->p_ਪੰਨਾ);
      rc = SQLITE_CORRUPT_PGNO(chldPg);
    }
    ਜੇ( rc ){
      pCur->p_ਪੰਨਾ = pCur->ap_ਪੰਨਾ[--pCur->i_ਪੰਨਾ];
      ਰੋਕੋ;
    }
    /*
    ***** End of in-lined moveToChild() call */
 }
moveto_ਸੂਚਕਾਂਕ_ਅੰਤ:
  pCur->info.n_ਮਾਪ = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( (pCur->curFlags & BTCF_ValidOvfl)==0 );
  ਵਾਪਸ rc;
}


/*
** Return TRUE if the cursor is not pointing at an entry of the table.
**
** TRUE will be returned after a call to sqlite3BtreeNext() moves
** past the last entry in the table or sqlite3BtreePrev() moves past
** the first entry.  TRUE is also returned if the table is empty.
*/
ਪੂਰਨ sqlite3BtreeEof(BtCursor *pCur){
  /* TODO: What if the cursor is in CURSOR_REQUIRESEEK but all table entries
  ** have been deleted? This API will need to change to return an error code
  ** as well as the boolean result value.
  */
  ਵਾਪਸ (CURSOR_VALID!=pCur->eState);
}

/*
** Return an estimate for the number of rows in the table that pCur is
** pointing to.  Return a negative number if no estimate is currently
** available.
*/
i64 sqlite3_Btree_ਪੰਕਤੀ_ਗਿਣਤੀ_Est(BtCursor *pCur){
  i64 n;
  u8 i;

  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* Currently this interface is only called by the OP_IfSizeBetween
  ** opcode and the OP_Count opcode with P3=1.  In either case,
  ** the cursor will always be valid unless the btree is empty. */
  ਜੇ( pCur->eState!=CURSOR_VALID ) ਵਾਪਸ 0;
  ਜੇ( NEVER(pCur->p_ਪੰਨਾ->ਪੱਤੀ==0) ) ਵਾਪਸ -1;

  n = pCur->p_ਪੰਨਾ->nCell;
  ਲਈ(i=0; i<pCur->i_ਪੰਨਾ; i++){
    n *= pCur->ap_ਪੰਨਾ[i]->nCell+1;
  }
  ਵਾਪਸ n;
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
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ btree_ਅਗਲਾ(BtCursor *pCur){
  ਪੂਰਨ rc;
  ਪੂਰਨ idx;
  Mem_ਪੰਨਾ *p_ਪੰਨਾ;

  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਜੇ( pCur->eState!=CURSOR_VALID ){
    ਪ੍ਰਤਿਜ੍ਞਾ( (pCur->curFlags & BTCF_ValidOvfl)==0 );
    rc = restoreCursorPosition(pCur);
    ਜੇ( rc!=SQLITE_OK ){
      ਵਾਪਸ rc;
    }
    ਜੇ( CURSOR_INVALID==pCur->eState ){
      ਵਾਪਸ SQLITE_ਪੂਰਾ;
    }
    ਜੇ( pCur->eState==CURSOR_SKIPNEXT ){
      pCur->eState = CURSOR_VALID;
      ਜੇ( pCur->skip_ਅਗਲਾ>0 ) ਵਾਪਸ SQLITE_OK;
    }
  }

  p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;
  idx = ++pCur->ix;
  ਜੇ( sqlite3FaultSim(412) ) p_ਪੰਨਾ->isInit = 0;
  ਜੇ( !p_ਪੰਨਾ->isInit ){
    ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  }

  ਜੇ( idx>=p_ਪੰਨਾ->nCell ){
    ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
      rc = move_ਮੰਜ਼ਿਲ_ਬੱਚਾ(pCur, get4byte(&p_ਪੰਨਾ->a_ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+8]));
      ਜੇ( rc ) ਵਾਪਸ rc;
      ਵਾਪਸ move_ਮੰਜ਼ਿਲ_Leftmost(pCur);
    }
    ਕਰੋ{
      ਜੇ( pCur->i_ਪੰਨਾ==0 ){
        pCur->eState = CURSOR_INVALID;
        ਵਾਪਸ SQLITE_ਪੂਰਾ;
      }
      move_ਮੰਜ਼ਿਲ_ਮਾਪਾ(pCur);
      p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;
    }ਜਦੋਂ( pCur->ix>=p_ਪੰਨਾ->nCell );
    ਜੇ( p_ਪੰਨਾ->int_ਕੁੰਜੀ ){
      ਵਾਪਸ sqlite3_Btree_ਅਗਲਾ(pCur, 0);
    }ਨਹੀਂਤਾਂ{
      ਵਾਪਸ SQLITE_OK;
    }
  }
  ਜੇ( p_ਪੰਨਾ->ਪੱਤੀ ){
    ਵਾਪਸ SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ move_ਮੰਜ਼ਿਲ_Leftmost(pCur);
  }
}
ਪੂਰਨ sqlite3_Btree_ਅਗਲਾ(BtCursor *pCur, ਪੂਰਨ flags){
  Mem_ਪੰਨਾ *p_ਪੰਨਾ;
  ਅਣਵਰਤਿਆ_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( flags==0 || flags==1 );
  pCur->info.n_ਮਾਪ = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  ਜੇ( pCur->eState!=CURSOR_VALID ) ਵਾਪਸ btree_ਅਗਲਾ(pCur);
  p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;
  ਜੇ( (++pCur->ix)>=p_ਪੰਨਾ->nCell ){
    pCur->ix--;
    ਵਾਪਸ btree_ਅਗਲਾ(pCur);
  }
  ਜੇ( p_ਪੰਨਾ->ਪੱਤੀ ){
    ਵਾਪਸ SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ move_ਮੰਜ਼ਿਲ_Leftmost(pCur);
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
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ btree_ਪਿਛਲਾ(BtCursor *pCur){
  ਪੂਰਨ rc;
  Mem_ਪੰਨਾ *p_ਪੰਨਾ;

  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pCur->curFlags & (BTCF_At_ਆਖਰੀ|BTCF_ValidOvfl|BTCF_ValidNKey))==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->info.n_ਮਾਪ==0 );
  ਜੇ( pCur->eState!=CURSOR_VALID ){
    rc = restoreCursorPosition(pCur);
    ਜੇ( rc!=SQLITE_OK ){
      ਵਾਪਸ rc;
    }
    ਜੇ( CURSOR_INVALID==pCur->eState ){
      ਵਾਪਸ SQLITE_ਪੂਰਾ;
    }
    ਜੇ( CURSOR_SKIPNEXT==pCur->eState ){
      pCur->eState = CURSOR_VALID;
      ਜੇ( pCur->skip_ਅਗਲਾ<0 ) ਵਾਪਸ SQLITE_OK;
    }
  }

  p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;
  ਜੇ( sqlite3FaultSim(412) ) p_ਪੰਨਾ->isInit = 0;
  ਜੇ( !p_ਪੰਨਾ->isInit ){
    ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  }
  ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
    ਪੂਰਨ idx = pCur->ix;
    rc = move_ਮੰਜ਼ਿਲ_ਬੱਚਾ(pCur, get4byte(ਲੱਭੋ_Cell(p_ਪੰਨਾ, idx)));
    ਜੇ( rc ) ਵਾਪਸ rc;
    rc = move_ਮੰਜ਼ਿਲ_Rightmost(pCur);
  }ਨਹੀਂਤਾਂ{
    ਜਦੋਂ( pCur->ix==0 ){
      ਜੇ( pCur->i_ਪੰਨਾ==0 ){
        pCur->eState = CURSOR_INVALID;
        ਵਾਪਸ SQLITE_ਪੂਰਾ;
      }
      move_ਮੰਜ਼ਿਲ_ਮਾਪਾ(pCur);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pCur->info.n_ਮਾਪ==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( (pCur->curFlags & (BTCF_ValidOvfl))==0 );

    pCur->ix--;
    p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;
    ਜੇ( p_ਪੰਨਾ->int_ਕੁੰਜੀ && !p_ਪੰਨਾ->ਪੱਤੀ ){
      rc = sqlite3_Btree_ਪਿਛਲਾ(pCur, 0);
    }ਨਹੀਂਤਾਂ{
      rc = SQLITE_OK;
    }
  }
  ਵਾਪਸ rc;
}
ਪੂਰਨ sqlite3_Btree_ਪਿਛਲਾ(BtCursor *pCur, ਪੂਰਨ flags){
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( flags==0 || flags==1 );
  ਅਣਵਰਤਿਆ_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  pCur->curFlags &= ~(BTCF_At_ਆਖਰੀ|BTCF_ValidOvfl|BTCF_ValidNKey);
  pCur->info.n_ਮਾਪ = 0;
  ਜੇ( pCur->eState!=CURSOR_VALID
   || pCur->ix==0
   || pCur->p_ਪੰਨਾ->ਪੱਤੀ==0
  ){
    ਵਾਪਸ btree_ਪਿਛਲਾ(pCur);
  }
  pCur->ix--;
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ allocate_Btree_ਪੰਨਾ(
  BtShared *pBt,         /* The btree */
  Mem_ਪੰਨਾ **pp_ਪੰਨਾ,      /* Store pointer to the allocated page here */
  Pgno *pPgno,           /* Store the page number here */
  Pgno nearby,           /* Search for a page near this one */
  u8 e_ਬਹੁਲਕ               /* BTALLOC_EXACT, BTALLOC_LT, or BTALLOC_ANY */
){
  Mem_ਪੰਨਾ *pPage1;
  ਪੂਰਨ rc;
  u32 n;     /* Number of pages on the freelist */
  u32 k;     /* Number of leaves on the trunk of the freelist */
  Mem_ਪੰਨਾ *pTrunk = 0;
  Mem_ਪੰਨਾ *pPrevTrunk = 0;
  Pgno mx_ਪੰਨਾ;     /* Total size of the database file */

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( e_ਬਹੁਲਕ==BTALLOC_ANY || (nearby>0 && IfNotOmitAV(pBt->autoVacuum)) );
  pPage1 = pBt->pPage1;
  mx_ਪੰਨਾ = btreePagecount(pBt);
  /* EVIDENCE-OF: R-21003-45125 The 4-byte big-endian integer at offset 36
  ** stores the total number of pages on the freelist. */
  n = get4byte(&pPage1->a_ਅੰਕੜੇ[36]);
  testcase( n==mx_ਪੰਨਾ-1 );
  ਜੇ( n>=mx_ਪੰਨਾ ){
    ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  }
  ਜੇ( n>0 ){
    /* There are pages on the freelist.  Reuse one of those pages. */
    Pgno iTrunk;
    u8 ਖੋਜ_List = 0; /* If the free-list must be searched for 'nearby' */
    u32 n_ਖੋਜ = 0;   /* Count of the number of search attempts */
   
    /* If eMode==BTALLOC_EXACT and a query of the pointer-map
    ** shows that the page 'nearby' is somewhere on the free-list, then
    ** the entire-list will be searched for that page.
    */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
    ਜੇ( e_ਬਹੁਲਕ==BTALLOC_EXACT ){
      ਜੇ( nearby<=mx_ਪੰਨਾ ){
        u8 eType;
        ਪ੍ਰਤਿਜ੍ਞਾ( nearby>0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( pBt->autoVacuum );
        rc = ptrmapGet(pBt, nearby, &eType, 0);
        ਜੇ( rc ) ਵਾਪਸ rc;
        ਜੇ( eType==PTRMAP_FREEPAGE ){
          ਖੋਜ_List = 1;
        }
      }
    }ਨਹੀਂਤਾਂ ਜੇ( e_ਬਹੁਲਕ==BTALLOC_LE ){
      ਖੋਜ_List = 1;
    }
#ਜੇ_ਅੰਤ

    /* Decrement the free-list count by 1. Set iTrunk to the index of the
    ** first free-list trunk page. iPrevTrunk is initially 1.
    */
    rc = sqlite3PagerWrite(pPage1->p_Db_ਪੰਨਾ);
    ਜੇ( rc ) ਵਾਪਸ rc;
    put4byte(&pPage1->a_ਅੰਕੜੇ[36], n-1);

    /* The code within this loop is run only once if the 'searchList' variable
    ** is not true. Otherwise, it runs once for each trunk-page on the
    ** free-list until the page 'nearby' is located (eMode==BTALLOC_EXACT)
    ** or until a page less than 'nearby' is located (eMode==BTALLOC_LT)
    */
    ਕਰੋ {
      pPrevTrunk = pTrunk;
      ਜੇ( pPrevTrunk ){
        /* EVIDENCE-OF: R-01506-11053 The first integer on a freelist trunk page
        ** is the page number of the next freelist trunk page in the list or
        ** zero if this is the last freelist trunk page. */
        iTrunk = get4byte(&pPrevTrunk->a_ਅੰਕੜੇ[0]);
      }ਨਹੀਂਤਾਂ{
        /* EVIDENCE-OF: R-59841-13798 The 4-byte big-endian integer at offset 32
        ** stores the page number of the first page of the freelist, or zero if
        ** the freelist is empty. */
        iTrunk = get4byte(&pPage1->a_ਅੰਕੜੇ[32]);
      }
      testcase( iTrunk==mx_ਪੰਨਾ );
      ਜੇ( iTrunk>mx_ਪੰਨਾ || n_ਖੋਜ++ > n ){
        rc = SQLITE_CORRUPT_PGNO(pPrevTrunk ? pPrevTrunk->pgno : 1);
      }ਨਹੀਂਤਾਂ{
        rc = btree_Get_ਅਣਵਰਤਿਆ_ਪੰਨਾ(pBt, iTrunk, &pTrunk, 0);
      }
      ਜੇ( rc ){
        pTrunk = 0;
        ਜਾਓ ਸਿਰਾ_allocate_ਪੰਨਾ;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( pTrunk!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pTrunk->a_ਅੰਕੜੇ!=0 );
      /* EVIDENCE-OF: R-13523-04394 The second integer on a freelist trunk page
      ** is the number of leaf page pointers to follow. */
      k = get4byte(&pTrunk->a_ਅੰਕੜੇ[4]);
      ਜੇ( k==0 && !ਖੋਜ_List ){
        /* The trunk has no leaves and the list is not being searched.
        ** So extract the trunk page itself and use it as the newly
        ** allocated page */
        ਪ੍ਰਤਿਜ੍ਞਾ( pPrevTrunk==0 );
        rc = sqlite3PagerWrite(pTrunk->p_Db_ਪੰਨਾ);
        ਜੇ( rc ){
          ਜਾਓ ਸਿਰਾ_allocate_ਪੰਨਾ;
        }
        *pPgno = iTrunk;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pPage1->a_ਅੰਕੜੇ[32], &pTrunk->a_ਅੰਕੜੇ[0], 4);
        *pp_ਪੰਨਾ = pTrunk;
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
      }ਨਹੀਂਤਾਂ ਜੇ( k>(u32)(pBt->usable_ਮਾਪ/4 - 2) ){
        /* Value of k is out of range.  Database corruption */
        rc = SQLITE_CORRUPT_PGNO(iTrunk);
        ਜਾਓ ਸਿਰਾ_allocate_ਪੰਨਾ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
      }ਨਹੀਂਤਾਂ ਜੇ( ਖੋਜ_List
            && (nearby==iTrunk || (iTrunk<nearby && e_ਬਹੁਲਕ==BTALLOC_LE))
      ){
        /* The list is being searched and this trunk page is the page
        ** to allocate, regardless of whether it has leaves.
        */
        *pPgno = iTrunk;
        *pp_ਪੰਨਾ = pTrunk;
        ਖੋਜ_List = 0;
        rc = sqlite3PagerWrite(pTrunk->p_Db_ਪੰਨਾ);
        ਜੇ( rc ){
          ਜਾਓ ਸਿਰਾ_allocate_ਪੰਨਾ;
        }
        ਜੇ( k==0 ){
          ਜੇ( !pPrevTrunk ){
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pPage1->a_ਅੰਕੜੇ[32], &pTrunk->a_ਅੰਕੜੇ[0], 4);
          }ਨਹੀਂਤਾਂ{
            rc = sqlite3PagerWrite(pPrevTrunk->p_Db_ਪੰਨਾ);
            ਜੇ( rc!=SQLITE_OK ){
              ਜਾਓ ਸਿਰਾ_allocate_ਪੰਨਾ;
            }
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pPrevTrunk->a_ਅੰਕੜੇ[0], &pTrunk->a_ਅੰਕੜੇ[0], 4);
          }
        }ਨਹੀਂਤਾਂ{
          /* The trunk page is required by the caller but it contains
          ** pointers to free-list leaves. The first leaf becomes a trunk
          ** page in this case.
          */
          Mem_ਪੰਨਾ *pNewTrunk;
          Pgno iNewTrunk = get4byte(&pTrunk->a_ਅੰਕੜੇ[8]);
          ਜੇ( iNewTrunk>mx_ਪੰਨਾ ){
            rc = SQLITE_CORRUPT_PGNO(iTrunk);
            ਜਾਓ ਸਿਰਾ_allocate_ਪੰਨਾ;
          }
          testcase( iNewTrunk==mx_ਪੰਨਾ );
          rc = btree_Get_ਅਣਵਰਤਿਆ_ਪੰਨਾ(pBt, iNewTrunk, &pNewTrunk, 0);
          ਜੇ( rc!=SQLITE_OK ){
            ਜਾਓ ਸਿਰਾ_allocate_ਪੰਨਾ;
          }
          rc = sqlite3PagerWrite(pNewTrunk->p_Db_ਪੰਨਾ);
          ਜੇ( rc!=SQLITE_OK ){
            release_ਪੰਨਾ(pNewTrunk);
            ਜਾਓ ਸਿਰਾ_allocate_ਪੰਨਾ;
          }
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pNewTrunk->a_ਅੰਕੜੇ[0], &pTrunk->a_ਅੰਕੜੇ[0], 4);
          put4byte(&pNewTrunk->a_ਅੰਕੜੇ[4], k-1);
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pNewTrunk->a_ਅੰਕੜੇ[8], &pTrunk->a_ਅੰਕੜੇ[12], (k-1)*4);
          release_ਪੰਨਾ(pNewTrunk);
          ਜੇ( !pPrevTrunk ){
            ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(pPage1->p_Db_ਪੰਨਾ) );
            put4byte(&pPage1->a_ਅੰਕੜੇ[32], iNewTrunk);
          }ਨਹੀਂਤਾਂ{
            rc = sqlite3PagerWrite(pPrevTrunk->p_Db_ਪੰਨਾ);
            ਜੇ( rc ){
              ਜਾਓ ਸਿਰਾ_allocate_ਪੰਨਾ;
            }
            put4byte(&pPrevTrunk->a_ਅੰਕੜੇ[0], iNewTrunk);
          }
        }
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
#ਜੇ_ਅੰਤ
      }ਨਹੀਂਤਾਂ ਜੇ( k>0 ){
        /* Extract a leaf from the trunk */
        u32 closest;
        Pgno i_ਪੰਨਾ;
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a_ਅੰਕੜੇ = pTrunk->a_ਅੰਕੜੇ;
        ਜੇ( nearby>0 ){
          u32 i;
          closest = 0;
          ਜੇ( e_ਬਹੁਲਕ==BTALLOC_LE ){
            ਲਈ(i=0; i<k; i++){
              i_ਪੰਨਾ = get4byte(&a_ਅੰਕੜੇ[8+i*4]);
              ਜੇ( i_ਪੰਨਾ<=nearby ){
                closest = i;
                ਰੋਕੋ;
              }
            }
          }ਨਹੀਂਤਾਂ{
            ਪੂਰਨ dist;
            dist = sqlite3AbsInt32(get4byte(&a_ਅੰਕੜੇ[8]) - nearby);
            ਲਈ(i=1; i<k; i++){
              ਪੂਰਨ d2 = sqlite3AbsInt32(get4byte(&a_ਅੰਕੜੇ[8+i*4]) - nearby);
              ਜੇ( d2<dist ){
                closest = i;
                dist = d2;
              }
            }
          }
        }ਨਹੀਂਤਾਂ{
          closest = 0;
        }

        i_ਪੰਨਾ = get4byte(&a_ਅੰਕੜੇ[8+closest*4]);
        testcase( i_ਪੰਨਾ==mx_ਪੰਨਾ );
        ਜੇ( i_ਪੰਨਾ>mx_ਪੰਨਾ || i_ਪੰਨਾ<2 ){
          rc = SQLITE_CORRUPT_PGNO(iTrunk);
          ਜਾਓ ਸਿਰਾ_allocate_ਪੰਨਾ;
        }
        testcase( i_ਪੰਨਾ==mx_ਪੰਨਾ );
        ਜੇ( !ਖੋਜ_List
         || (i_ਪੰਨਾ==nearby || (i_ਪੰਨਾ<nearby && e_ਬਹੁਲਕ==BTALLOC_LE))
        ){
          ਪੂਰਨ noContent;
          *pPgno = i_ਪੰਨਾ;
          TRACE(("ALLOCATE: %u was leaf %u of %u on trunk %u"
                 ": %u more free pages\n",
                 *pPgno, closest+1, k, pTrunk->pgno, n-1));
          rc = sqlite3PagerWrite(pTrunk->p_Db_ਪੰਨਾ);
          ਜੇ( rc ) ਜਾਓ ਸਿਰਾ_allocate_ਪੰਨਾ;
          ਜੇ( closest<k-1 ){
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(&a_ਅੰਕੜੇ[8+closest*4], &a_ਅੰਕੜੇ[4+k*4], 4);
          }
          put4byte(&a_ਅੰਕੜੇ[4], k-1);
          noContent = !btreeGetHasContent(pBt, *pPgno)? PAGER_GET_NOCONTENT : 0;
          rc = btree_Get_ਅਣਵਰਤਿਆ_ਪੰਨਾ(pBt, *pPgno, pp_ਪੰਨਾ, noContent);
          ਜੇ( rc==SQLITE_OK ){
            rc = sqlite3PagerWrite((*pp_ਪੰਨਾ)->p_Db_ਪੰਨਾ);
            ਜੇ( rc!=SQLITE_OK ){
              release_ਪੰਨਾ(*pp_ਪੰਨਾ);
              *pp_ਪੰਨਾ = 0;
            }
          }
          ਖੋਜ_List = 0;
        }
      }
      release_ਪੰਨਾ(pPrevTrunk);
      pPrevTrunk = 0;
    }ਜਦੋਂ( ਖੋਜ_List );
  }ਨਹੀਂਤਾਂ{
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
    ਪੂਰਨ bNoContent = (0==IfNotOmitAV(pBt->bDoTruncate))? PAGER_GET_NOCONTENT:0;

    rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ਪੰਨਾ);
    ਜੇ( rc ) ਵਾਪਸ rc;
    pBt->n_ਪੰਨਾ++;
    ਜੇ( pBt->n_ਪੰਨਾ==PENDING_BYTE_ਪੰਨਾ(pBt) ) pBt->n_ਪੰਨਾ++;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
    ਜੇ( pBt->autoVacuum && PTRMAP_ISPAGE(pBt, pBt->n_ਪੰਨਾ) ){
      /* If *pPgno refers to a pointer-map page, allocate two new pages
      ** at the end of the file instead of one. The first allocated page
      ** becomes a new pointer-map page, the second is used by the caller.
      */
      Mem_ਪੰਨਾ *pPg = 0;
      TRACE(("ALLOCATE: %u from end of file (pointer-map page)\n", pBt->n_ਪੰਨਾ));
      ਪ੍ਰਤਿਜ੍ਞਾ( pBt->n_ਪੰਨਾ!=PENDING_BYTE_ਪੰਨਾ(pBt) );
      rc = btree_Get_ਅਣਵਰਤਿਆ_ਪੰਨਾ(pBt, pBt->n_ਪੰਨਾ, &pPg, bNoContent);
      ਜੇ( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pPg->p_Db_ਪੰਨਾ);
        release_ਪੰਨਾ(pPg);
      }
      ਜੇ( rc ) ਵਾਪਸ rc;
      pBt->n_ਪੰਨਾ++;
      ਜੇ( pBt->n_ਪੰਨਾ==PENDING_BYTE_ਪੰਨਾ(pBt) ){ pBt->n_ਪੰਨਾ++; }
    }
#ਜੇ_ਅੰਤ
    put4byte(28 + (u8*)pBt->pPage1->a_ਅੰਕੜੇ, pBt->n_ਪੰਨਾ);
    *pPgno = pBt->n_ਪੰਨਾ;

    ਪ੍ਰਤਿਜ੍ਞਾ( *pPgno!=PENDING_BYTE_ਪੰਨਾ(pBt) );
    rc = btree_Get_ਅਣਵਰਤਿਆ_ਪੰਨਾ(pBt, *pPgno, pp_ਪੰਨਾ, bNoContent);
    ਜੇ( rc ) ਵਾਪਸ rc;
    rc = sqlite3PagerWrite((*pp_ਪੰਨਾ)->p_Db_ਪੰਨਾ);
    ਜੇ( rc!=SQLITE_OK ){
      release_ਪੰਨਾ(*pp_ਪੰਨਾ);
      *pp_ਪੰਨਾ = 0;
    }
    TRACE(("ALLOCATE: %u from end of file\n", *pPgno));
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB || *pPgno!=PENDING_BYTE_ਪੰਨਾ(pBt) );

ਸਿਰਾ_allocate_ਪੰਨਾ:
  release_ਪੰਨਾ(pTrunk);
  release_ਪੰਨਾ(pPrevTrunk);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || sqlite3_Pager_ਪੰਨਾ_Refcount((*pp_ਪੰਨਾ)->p_Db_ਪੰਨਾ)<=1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || (*pp_ਪੰਨਾ)->isInit==0 );
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ freePage2(BtShared *pBt, Mem_ਪੰਨਾ *p_Mem_ਪੰਨਾ, Pgno i_ਪੰਨਾ){
  Mem_ਪੰਨਾ *pTrunk = 0;                /* Free-list trunk page */
  Pgno iTrunk = 0;                    /* Page number of free-list trunk page */
  Mem_ਪੰਨਾ *pPage1 = pBt->pPage1;      /* Local reference to page 1 */
  Mem_ਪੰਨਾ *p_ਪੰਨਾ;                     /* Page being freed. May be NULL. */
  ਪੂਰਨ rc;                             /* Return Code */
  u32 nFree;                          /* Initial number of pages on free-list */

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB || i_ਪੰਨਾ>1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !p_Mem_ਪੰਨਾ || p_Mem_ਪੰਨਾ->pgno==i_ਪੰਨਾ );

  ਜੇ( i_ਪੰਨਾ<2 || i_ਪੰਨਾ>pBt->n_ਪੰਨਾ ){
    ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  }
  ਜੇ( p_Mem_ਪੰਨਾ ){
    p_ਪੰਨਾ = p_Mem_ਪੰਨਾ;
    sqlite3PagerRef(p_ਪੰਨਾ->p_Db_ਪੰਨਾ);
  }ਨਹੀਂਤਾਂ{
    p_ਪੰਨਾ = btree_ਪੰਨਾ_Lookup(pBt, i_ਪੰਨਾ);
  }

  /* Increment the free page count on pPage1 */
  rc = sqlite3PagerWrite(pPage1->p_Db_ਪੰਨਾ);
  ਜੇ( rc ) ਜਾਓ freepage_ਬਾਹਰਲਾ;
  nFree = get4byte(&pPage1->a_ਅੰਕੜੇ[36]);
  put4byte(&pPage1->a_ਅੰਕੜੇ[36], nFree+1);

  ਜੇ( pBt->btsFlags & BTS_SECURE_DELETE ){
    /* If the secure_delete option is enabled, then
    ** always fully overwrite deleted information with zeros.
    */
    ਜੇ( (!p_ਪੰਨਾ && ((rc = btree_Get_ਪੰਨਾ(pBt, i_ਪੰਨਾ, &p_ਪੰਨਾ, 0))!=0) )
     ||            ((rc = sqlite3PagerWrite(p_ਪੰਨਾ->p_Db_ਪੰਨਾ))!=0)
    ){
      ਜਾਓ freepage_ਬਾਹਰਲਾ;
    }
    memset(p_ਪੰਨਾ->a_ਅੰਕੜੇ, 0, p_ਪੰਨਾ->pBt->ਪੰਨਾ_ਮਾਪ);
  }

  /* If the database supports auto-vacuum, write an entry in the pointer-map
  ** to indicate that the page is free.
  */
  ਜੇ( ISAUTOVACUUM(pBt) ){
    ptrmapPut(pBt, i_ਪੰਨਾ, PTRMAP_FREEPAGE, 0, &rc);
    ਜੇ( rc ) ਜਾਓ freepage_ਬਾਹਰਲਾ;
  }

  /* Now manipulate the actual database free-list structure. There are two
  ** possibilities. If the free-list is currently empty, or if the first
  ** trunk page in the free-list is full, then this page will become a
  ** new free-list trunk page. Otherwise, it will become a leaf of the
  ** first trunk page in the current free-list. This block tests if it
  ** is possible to add the page as a new free-list leaf.
  */
  ਜੇ( nFree!=0 ){
    u32 n_ਪੱਤੀ;                /* Initial number of leaf cells on trunk page */

    iTrunk = get4byte(&pPage1->a_ਅੰਕੜੇ[32]);
    ਜੇ( iTrunk>btreePagecount(pBt) ){
      rc = SQLITE_CORRUPT_BKPT;
      ਜਾਓ freepage_ਬਾਹਰਲਾ;
    }
    rc = btree_Get_ਪੰਨਾ(pBt, iTrunk, &pTrunk, 0);
    ਜੇ( rc!=SQLITE_OK ){
      ਜਾਓ freepage_ਬਾਹਰਲਾ;
    }

    n_ਪੱਤੀ = get4byte(&pTrunk->a_ਅੰਕੜੇ[4]);
    ਪ੍ਰਤਿਜ੍ਞਾ( pBt->usable_ਮਾਪ>32 );
    ਜੇ( n_ਪੱਤੀ > (u32)pBt->usable_ਮਾਪ/4 - 2 ){
      rc = SQLITE_CORRUPT_BKPT;
      ਜਾਓ freepage_ਬਾਹਰਲਾ;
    }
    ਜੇ( n_ਪੱਤੀ < (u32)pBt->usable_ਮਾਪ/4 - 8 ){
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
      rc = sqlite3PagerWrite(pTrunk->p_Db_ਪੰਨਾ);
      ਜੇ( rc==SQLITE_OK ){
        put4byte(&pTrunk->a_ਅੰਕੜੇ[4], n_ਪੱਤੀ+1);
        put4byte(&pTrunk->a_ਅੰਕੜੇ[8+n_ਪੱਤੀ*4], i_ਪੰਨਾ);
        ਜੇ( p_ਪੰਨਾ && (pBt->btsFlags & BTS_SECURE_DELETE)==0 ){
          sqlite3PagerDontWrite(p_ਪੰਨਾ->p_Db_ਪੰਨਾ);
        }
        rc = btree_ਸੈੱਟ_Has_Content(pBt, i_ਪੰਨਾ);
      }
      TRACE(("FREE-PAGE: %u leaf on trunk page %u\n",p_ਪੰਨਾ->pgno,pTrunk->pgno));
      ਜਾਓ freepage_ਬਾਹਰਲਾ;
    }
  }

  /* If control flows to this point, then it was not possible to add the
  ** the page being freed as a leaf page of the first trunk in the free-list.
  ** Possibly because the free-list is empty, or possibly because the
  ** first trunk in the free-list is full. Either way, the page being freed
  ** will become the new first trunk page in the free-list.
  */
  ਜੇ( p_ਪੰਨਾ==0 && SQLITE_OK!=(rc = btree_Get_ਪੰਨਾ(pBt, i_ਪੰਨਾ, &p_ਪੰਨਾ, 0)) ){
    ਜਾਓ freepage_ਬਾਹਰਲਾ;
  }
  rc = sqlite3PagerWrite(p_ਪੰਨਾ->p_Db_ਪੰਨਾ);
  ਜੇ( rc!=SQLITE_OK ){
    ਜਾਓ freepage_ਬਾਹਰਲਾ;
  }
  put4byte(p_ਪੰਨਾ->a_ਅੰਕੜੇ, iTrunk);
  put4byte(&p_ਪੰਨਾ->a_ਅੰਕੜੇ[4], 0);
  put4byte(&pPage1->a_ਅੰਕੜੇ[32], i_ਪੰਨਾ);
  TRACE(("FREE-PAGE: %u new trunk page replacing %u\n", p_ਪੰਨਾ->pgno, iTrunk));

freepage_ਬਾਹਰਲਾ:
  ਜੇ( p_ਪੰਨਾ ){
    p_ਪੰਨਾ->isInit = 0;
  }
  release_ਪੰਨਾ(p_ਪੰਨਾ);
  release_ਪੰਨਾ(pTrunk);
  ਵਾਪਸ rc;
}
ਸਥਿਰ ਖਾਲੀ free_ਪੰਨਾ(Mem_ਪੰਨਾ *p_ਪੰਨਾ, ਪੂਰਨ *pRC){
  ਜੇ( (*pRC)==SQLITE_OK ){
    *pRC = freePage2(p_ਪੰਨਾ->pBt, p_ਪੰਨਾ, p_ਪੰਨਾ->pgno);
  }
}

/*
** Free the overflow pages associated with the given Cell.
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ clear_Cell_ਉਛਾਲ(
  Mem_ਪੰਨਾ *p_ਪੰਨਾ,          /* The page that contains the Cell */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pCell,    /* First byte of the Cell */
  CellInfo *pInfo          /* Size information about the cell */
){
  BtShared *pBt;
  Pgno ovflPgno;
  ਪੂਰਨ rc;
  ਪੂਰਨ nOvfl;
  u32 ovfl_ਪੰਨਾ_ਮਾਪ;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pInfo->nLocal!=pInfo->nPayload );
  testcase( pCell + pInfo->n_ਮਾਪ == p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ );
  testcase( pCell + (pInfo->n_ਮਾਪ-1) == p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ );
  ਜੇ( pCell + pInfo->n_ਮਾਪ > p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ ){
    /* Cell extends past end of page */
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
  }
  ovflPgno = get4byte(pCell + pInfo->n_ਮਾਪ - 4);
  pBt = p_ਪੰਨਾ->pBt;
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->usable_ਮਾਪ > 4 );
  ovfl_ਪੰਨਾ_ਮਾਪ = pBt->usable_ਮਾਪ - 4;
  nOvfl = (pInfo->nPayload - pInfo->nLocal + ovfl_ਪੰਨਾ_ਮਾਪ - 1)/ovfl_ਪੰਨਾ_ਮਾਪ;
  ਪ੍ਰਤਿਜ੍ਞਾ( nOvfl>0 ||
    (CORRUPT_DB && (pInfo->nPayload + ovfl_ਪੰਨਾ_ਮਾਪ)<ovfl_ਪੰਨਾ_ਮਾਪ)
  );
  ਜਦੋਂ( nOvfl-- ){
    Pgno i_ਅਗਲਾ = 0;
    Mem_ਪੰਨਾ *pOvfl = 0;
    ਜੇ( ovflPgno<2 || ovflPgno>btreePagecount(pBt) ){
      /* 0 is not a legal page number and page 1 cannot be an
      ** overflow page. Therefore if ovflPgno<2 or past the end of the
      ** file the database must be corrupt. */
      ਵਾਪਸ SQLITE_CORRUPT_BKPT;
    }
    ਜੇ( nOvfl ){
      rc = get_ਉਛਾਲ_ਪੰਨਾ(pBt, ovflPgno, &pOvfl, &i_ਅਗਲਾ);
      ਜੇ( rc ) ਵਾਪਸ rc;
    }

    ਜੇ( ( pOvfl || ((pOvfl = btree_ਪੰਨਾ_Lookup(pBt, ovflPgno))!=0) )
     && sqlite3_Pager_ਪੰਨਾ_Refcount(pOvfl->p_Db_ਪੰਨਾ)!=1
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
    }ਨਹੀਂਤਾਂ{
      rc = freePage2(pBt, pOvfl, ovflPgno);
    }

    ਜੇ( pOvfl ){
      sqlite3PagerUnref(pOvfl->p_Db_ਪੰਨਾ);
    }
    ਜੇ( rc ) ਵਾਪਸ rc;
    ovflPgno = i_ਅਗਲਾ;
  }
  ਵਾਪਸ SQLITE_OK;
}

/* Call xParseCell to compute the size of a cell.  If the cell contains
** overflow, then invoke cellClearOverflow to clear out that overflow.
** Store the result code (SQLITE_OK or some error code) in rc.
**
** Implemented as macro to force inlining for performance.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_CLEAR_CELL(rc, p_ਪੰਨਾ, pCell, sInfo)   \
  p_ਪੰਨਾ->xParseCell(p_ਪੰਨਾ, pCell, &sInfo);          \
  ਜੇ( sInfo.nLocal!=sInfo.nPayload ){               \
    rc = clear_Cell_ਉਛਾਲ(p_ਪੰਨਾ, pCell, &sInfo);   \
  }ਨਹੀਂਤਾਂ{                                            \
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
ਸਥਿਰ ਪੂਰਨ fill_ਵਿੱਚ_Cell(
  Mem_ਪੰਨਾ *p_ਪੰਨਾ,                /* The page that contains the cell */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pCell,          /* Complete text of the cell */
  ਅਚਲ BtreePayload *pX,        /* Payload with which to construct the cell */
  ਪੂਰਨ *pn_ਮਾਪ                    /* Write cell size here */
){
  ਪੂਰਨ nPayload;
  ਅਚਲ u8 *pSrc;
  ਪੂਰਨ nSrc, n, rc, mn;
  ਪੂਰਨ space_ਖੱਬਾ;
  Mem_ਪੰਨਾ *p_ਮੰਜ਼ਿਲ_Release;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p_ਪੂਰਵ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pPayload;
  BtShared *pBt;
  Pgno pgnoOvfl;
  ਪੂਰਨ n_ਸਿਰਲੇਖ;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );

  /* pPage is not necessarily writeable since pCell might be auxiliary
  ** buffer space that is separate from the pPage buffer area */
  ਪ੍ਰਤਿਜ੍ਞਾ( pCell<p_ਪੰਨਾ->a_ਅੰਕੜੇ || pCell>=&p_ਪੰਨਾ->a_ਅੰਕੜੇ[p_ਪੰਨਾ->pBt->ਪੰਨਾ_ਮਾਪ]
            || sqlite3PagerIswriteable(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );

  /* Fill in the header. */
  n_ਸਿਰਲੇਖ = p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ;
  ਜੇ( p_ਪੰਨਾ->int_ਕੁੰਜੀ ){
    nPayload = pX->n_ਅੰਕੜੇ + pX->nZero;
    pSrc = pX->p_ਅੰਕੜੇ;
    nSrc = pX->n_ਅੰਕੜੇ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->int_ਕੁੰਜੀ_ਪੱਤੀ ); /* fillInCell() only called for leaves */
    n_ਸਿਰਲੇਖ += putVarint32(&pCell[n_ਸਿਰਲੇਖ], nPayload);
    n_ਸਿਰਲੇਖ += sqlite3PutVarint(&pCell[n_ਸਿਰਲੇਖ], *(u64*)&pX->n_ਕੁੰਜੀ);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pX->n_ਕੁੰਜੀ<=0x7fffffff && pX->p_ਕੁੰਜੀ!=0 );
    nSrc = nPayload = (ਪੂਰਨ)pX->n_ਕੁੰਜੀ;
    pSrc = pX->p_ਕੁੰਜੀ;
    n_ਸਿਰਲੇਖ += putVarint32(&pCell[n_ਸਿਰਲੇਖ], nPayload);
  }
 
  /* Fill in the payload */
  pPayload = &pCell[n_ਸਿਰਲੇਖ];
  ਜੇ( nPayload<=p_ਪੰਨਾ->maxLocal ){
    /* This is the common case where everything fits on the btree page
    ** and no overflow pages are required. */
    n = n_ਸਿਰਲੇਖ + nPayload;
    testcase( n==3 );
    testcase( n==4 );
    ਜੇ( n<4 ){
      n = 4;
      pPayload[nPayload] = 0;
    }
    *pn_ਮਾਪ = n;
    ਪ੍ਰਤਿਜ੍ਞਾ( nSrc<=nPayload );
    testcase( nSrc<nPayload );
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pPayload, pSrc, nSrc);
    memset(pPayload+nSrc, 0, nPayload-nSrc);
    ਵਾਪਸ SQLITE_OK;
  }

  /* If we reach this point, it means that some of the content will need
  ** to spill onto overflow pages.
  */
  mn = p_ਪੰਨਾ->minLocal;
  n = mn + (nPayload - mn) % (p_ਪੰਨਾ->pBt->usable_ਮਾਪ - 4);
  testcase( n==p_ਪੰਨਾ->maxLocal );
  testcase( n==p_ਪੰਨਾ->maxLocal+1 );
  ਜੇ( n > p_ਪੰਨਾ->maxLocal ) n = mn;
  space_ਖੱਬਾ = n;
  *pn_ਮਾਪ = n + n_ਸਿਰਲੇਖ + 4;
  p_ਪੂਰਵ = &pCell[n_ਸਿਰਲੇਖ+n];
  p_ਮੰਜ਼ਿਲ_Release = 0;
  pgnoOvfl = 0;
  pBt = p_ਪੰਨਾ->pBt;

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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  {
    CellInfo info;
    p_ਪੰਨਾ->xParseCell(p_ਪੰਨਾ, pCell, &info);
    ਪ੍ਰਤਿਜ੍ਞਾ( n_ਸਿਰਲੇਖ==(ਪੂਰਨ)(info.pPayload - pCell) );
    ਪ੍ਰਤਿਜ੍ਞਾ( info.n_ਕੁੰਜੀ==pX->n_ਕੁੰਜੀ );
    ਪ੍ਰਤਿਜ੍ਞਾ( *pn_ਮਾਪ == info.n_ਮਾਪ );
    ਪ੍ਰਤਿਜ੍ਞਾ( space_ਖੱਬਾ == info.nLocal );
  }
#ਜੇ_ਅੰਤ

  /* Write the payload into the local Cell and any extra into overflow pages */
  ਜਦੋਂ( 1 ){
    n = nPayload;
    ਜੇ( n>space_ਖੱਬਾ ) n = space_ਖੱਬਾ;

    /* If pToRelease is not zero than pPayload points into the data area
    ** of pToRelease.  Make sure pToRelease is still writeable. */
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮੰਜ਼ਿਲ_Release==0 || sqlite3PagerIswriteable(p_ਮੰਜ਼ਿਲ_Release->p_Db_ਪੰਨਾ) );

    /* If pPayload is part of the data area of pPage, then make sure pPage
    ** is still writeable */
    ਪ੍ਰਤਿਜ੍ਞਾ( pPayload<p_ਪੰਨਾ->a_ਅੰਕੜੇ || pPayload>=&p_ਪੰਨਾ->a_ਅੰਕੜੇ[pBt->ਪੰਨਾ_ਮਾਪ]
            || sqlite3PagerIswriteable(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );

    ਜੇ( nSrc>=n ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(pPayload, pSrc, n);
    }ਨਹੀਂਤਾਂ ਜੇ( nSrc>0 ){
      n = nSrc;
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(pPayload, pSrc, n);
    }ਨਹੀਂਤਾਂ{
      memset(pPayload, 0, n);
    }
    nPayload -= n;
    ਜੇ( nPayload<=0 ) ਰੋਕੋ;
    pPayload += n;
    pSrc += n;
    nSrc -= n;
    space_ਖੱਬਾ -= n;
    ਜੇ( space_ਖੱਬਾ==0 ){
      Mem_ਪੰਨਾ *pOvfl = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
      Pgno pgnoPtrmap = pgnoOvfl; /* Overflow page pointer-map entry page */
      ਜੇ( pBt->autoVacuum ){
        ਕਰੋ{
          pgnoOvfl++;
        } ਜਦੋਂ(
          PTRMAP_ISPAGE(pBt, pgnoOvfl) || pgnoOvfl==PENDING_BYTE_ਪੰਨਾ(pBt)
        );
      }
#ਜੇ_ਅੰਤ
      rc = allocate_Btree_ਪੰਨਾ(pBt, &pOvfl, &pgnoOvfl, pgnoOvfl, 0);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
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
      ਜੇ( pBt->autoVacuum && rc==SQLITE_OK ){
        u8 eType = (pgnoPtrmap?PTRMAP_OVERFLOW2:PTRMAP_OVERFLOW1);
        ptrmapPut(pBt, pgnoOvfl, eType, pgnoPtrmap, &rc);
        ਜੇ( rc ){
          release_ਪੰਨਾ(pOvfl);
        }
      }
#ਜੇ_ਅੰਤ
      ਜੇ( rc ){
        release_ਪੰਨਾ(p_ਮੰਜ਼ਿਲ_Release);
        ਵਾਪਸ rc;
      }

      /* If pToRelease is not zero than pPrior points into the data area
      ** of pToRelease.  Make sure pToRelease is still writeable. */
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮੰਜ਼ਿਲ_Release==0 || sqlite3PagerIswriteable(p_ਮੰਜ਼ਿਲ_Release->p_Db_ਪੰਨਾ) );

      /* If pPrior is part of the data area of pPage, then make sure pPage
      ** is still writeable */
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੂਰਵ<p_ਪੰਨਾ->a_ਅੰਕੜੇ || p_ਪੂਰਵ>=&p_ਪੰਨਾ->a_ਅੰਕੜੇ[pBt->ਪੰਨਾ_ਮਾਪ]
            || sqlite3PagerIswriteable(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );

      put4byte(p_ਪੂਰਵ, pgnoOvfl);
      release_ਪੰਨਾ(p_ਮੰਜ਼ਿਲ_Release);
      p_ਮੰਜ਼ਿਲ_Release = pOvfl;
      p_ਪੂਰਵ = pOvfl->a_ਅੰਕੜੇ;
      put4byte(p_ਪੂਰਵ, 0);
      pPayload = &pOvfl->a_ਅੰਕੜੇ[4];
      space_ਖੱਬਾ = pBt->usable_ਮਾਪ - 4;
    }
  }
  release_ਪੰਨਾ(p_ਮੰਜ਼ਿਲ_Release);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Remove the i-th cell from pPage.  This routine effects pPage only.
** The cell content is not freed or deallocated.  It is assumed that
** the cell content has been copied someplace else.  This routine just
** removes the reference to the cell from pPage.
**
** "sz" must be the number of bytes in the cell.
*/
ਸਥਿਰ ਖਾਲੀ dropCell(Mem_ਪੰਨਾ *p_ਪੰਨਾ, ਪੂਰਨ idx, ਪੂਰਨ sz, ਪੂਰਨ *pRC){
  u32 pc;         /* Offset to cell content of cell being deleted */
  u8 *ਅੰਕੜੇ;       /* pPage->aData */
  u8 *ptr;        /* Used to move bytes around within data[] */
  ਪੂਰਨ rc;         /* The return code */
  ਪੂਰਨ hdr;        /* Beginning of the header.  0 most pages.  100 page 1 */

  ਜੇ( *pRC ) ਵਾਪਸ;
  ਪ੍ਰਤਿਜ੍ਞਾ( idx>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( idx<p_ਪੰਨਾ->nCell );
  ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB || sz==cell_ਮਾਪ(p_ਪੰਨਾ, idx) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nFree>=0 );
  ਅੰਕੜੇ = p_ਪੰਨਾ->a_ਅੰਕੜੇ;
  ptr = &p_ਪੰਨਾ->aCellIdx[2*idx];
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pBt->usable_ਮਾਪ > (u32)(ptr-ਅੰਕੜੇ) );
  pc = get2byte(ptr);
  hdr = p_ਪੰਨਾ->hdr_ਵਿੱਥ;
  testcase( pc==(u32)get2byte(&ਅੰਕੜੇ[hdr+5]) );
  testcase( pc+sz==p_ਪੰਨਾ->pBt->usable_ਮਾਪ );
  ਜੇ( pc+sz > p_ਪੰਨਾ->pBt->usable_ਮਾਪ ){
    *pRC = SQLITE_CORRUPT_BKPT;
    ਵਾਪਸ;
  }
  rc = freeSpace(p_ਪੰਨਾ, pc, sz);
  ਜੇ( rc ){
    *pRC = rc;
    ਵਾਪਸ;
  }
  p_ਪੰਨਾ->nCell--;
  ਜੇ( p_ਪੰਨਾ->nCell==0 ){
    memset(&ਅੰਕੜੇ[hdr+1], 0, 4);
    ਅੰਕੜੇ[hdr+7] = 0;
    put2byte(&ਅੰਕੜੇ[hdr+5], p_ਪੰਨਾ->pBt->usable_ਮਾਪ);
    p_ਪੰਨਾ->nFree = p_ਪੰਨਾ->pBt->usable_ਮਾਪ - p_ਪੰਨਾ->hdr_ਵਿੱਥ
                       - p_ਪੰਨਾ->ਬੱਚਾ_Ptr_ਮਾਪ - 8;
  }ਨਹੀਂਤਾਂ{
    memmove(ptr, ptr+2, 2*(p_ਪੰਨਾ->nCell - idx));
    put2byte(&ਅੰਕੜੇ[hdr+3], p_ਪੰਨਾ->nCell);
    p_ਪੰਨਾ->nFree += 2;
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
ਸਥਿਰ ਪੂਰਨ ਪਾਓ_Cell(
  Mem_ਪੰਨਾ *p_ਪੰਨਾ,   /* Page into which we are copying */
  ਪੂਰਨ i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  ਪੂਰਨ sz,           /* Bytes of content in pCell */
  u8 *p_ਆਰਜ਼ੀ,        /* Temp storage space for pCell, if needed */
  Pgno i_ਬੱਚਾ       /* If non-zero, replace first 4 bytes with this value */
){
  ਪੂਰਨ idx = 0;      /* Where to write new cell content in data[] */
  ਪੂਰਨ j;            /* Loop counter */
  u8 *ਅੰਕੜੇ;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  ਪ੍ਰਤਿਜ੍ਞਾ( i>=0 && i<=p_ਪੰਨਾ->nCell+p_ਪੰਨਾ->n_ਉਛਾਲ );
  ਪ੍ਰਤਿਜ੍ਞਾ( MX_CELL(p_ਪੰਨਾ->pBt)<=10921 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nCell<=MX_CELL(p_ਪੰਨਾ->pBt) || CORRUPT_DB );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->n_ਉਛਾਲ<=Array_ਮਾਪ(p_ਪੰਨਾ->apOvfl) );
  ਪ੍ਰਤਿਜ੍ਞਾ( Array_ਮਾਪ(p_ਪੰਨਾ->apOvfl)==Array_ਮਾਪ(p_ਪੰਨਾ->aiOvfl) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sz==p_ਪੰਨਾ->x_Cell_ਮਾਪ(p_ਪੰਨਾ, pCell) || CORRUPT_DB );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nFree>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਬੱਚਾ>0 );
  ਜੇ( p_ਪੰਨਾ->n_ਉਛਾਲ || sz+2>p_ਪੰਨਾ->nFree ){
    ਜੇ( p_ਆਰਜ਼ੀ ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਆਰਜ਼ੀ, pCell, sz);
      pCell = p_ਆਰਜ਼ੀ;
    }
    put4byte(pCell, i_ਬੱਚਾ);
    j = p_ਪੰਨਾ->n_ਉਛਾਲ++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    ਪ੍ਰਤਿਜ੍ਞਾ( j < Array_ਮਾਪ(p_ਪੰਨਾ->apOvfl)-1 );
    p_ਪੰਨਾ->apOvfl[j] = pCell;
    p_ਪੰਨਾ->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( j==0 || p_ਪੰਨਾ->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    ਪ੍ਰਤਿਜ੍ਞਾ( j==0 || i==p_ਪੰਨਾ->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ rc = sqlite3PagerWrite(p_ਪੰਨਾ->p_Db_ਪੰਨਾ);
    ਜੇ( NEVER(rc!=SQLITE_OK) ){
      ਵਾਪਸ rc;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
    ਅੰਕੜੇ = p_ਪੰਨਾ->a_ਅੰਕੜੇ;
    ਪ੍ਰਤਿਜ੍ਞਾ( &ਅੰਕੜੇ[p_ਪੰਨਾ->cell_ਵਿੱਥ]==p_ਪੰਨਾ->aCellIdx );
    rc = allocateSpace(p_ਪੰਨਾ, sz, &idx);
    ਜੇ( rc ){ ਵਾਪਸ rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    ਪ੍ਰਤਿਜ੍ਞਾ( idx >= 0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( idx >= p_ਪੰਨਾ->cell_ਵਿੱਥ+2*p_ਪੰਨਾ->nCell+2 || CORRUPT_DB );
    ਪ੍ਰਤਿਜ੍ਞਾ( idx+sz <= (ਪੂਰਨ)p_ਪੰਨਾ->pBt->usable_ਮਾਪ );
    p_ਪੰਨਾ->nFree -= (u16)(2 + sz);
    /* In a corrupt database where an entry in the cell index section of
    ** a btree page has a value of 3 or less, the pCell value might point
    ** as many as 4 bytes in front of the start of the aData buffer for
    ** the source page.  Make sure this does not cause problems by not
    ** reading the first 4 bytes */
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&ਅੰਕੜੇ[idx+4], pCell+4, sz-4);
    put4byte(&ਅੰਕੜੇ[idx], i_ਬੱਚਾ);
    pIns = p_ਪੰਨਾ->aCellIdx + i*2;
    ਜੇ( i<p_ਪੰਨਾ->nCell ) memmove(pIns+2, pIns, 2*(p_ਪੰਨਾ->nCell - i));
    put2byte(pIns, idx);
    p_ਪੰਨਾ->nCell++;
    /* increment the cell count */
    ਜੇ( (++ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+4])==0 ) ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+3]++;
    ਪ੍ਰਤਿਜ੍ਞਾ( get2byte(&ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+3])==p_ਪੰਨਾ->nCell || CORRUPT_DB );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
    ਜੇ( p_ਪੰਨਾ->pBt->autoVacuum ){
      ਪੂਰਨ rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(p_ਪੰਨਾ, p_ਪੰਨਾ, pCell, &rc2);
      ਜੇ( rc2 ) ਵਾਪਸ rc2;
    }
#ਜੇ_ਅੰਤ
  }
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ ਪਾਓ_Cell_Fast(
  Mem_ਪੰਨਾ *p_ਪੰਨਾ,   /* Page into which we are copying */
  ਪੂਰਨ i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  ਪੂਰਨ sz            /* Bytes of content in pCell */
){
  ਪੂਰਨ idx = 0;      /* Where to write new cell content in data[] */
  ਪੂਰਨ j;            /* Loop counter */
  u8 *ਅੰਕੜੇ;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  ਪ੍ਰਤਿਜ੍ਞਾ( i>=0 && i<=p_ਪੰਨਾ->nCell+p_ਪੰਨਾ->n_ਉਛਾਲ );
  ਪ੍ਰਤਿਜ੍ਞਾ( MX_CELL(p_ਪੰਨਾ->pBt)<=10921 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nCell<=MX_CELL(p_ਪੰਨਾ->pBt) || CORRUPT_DB );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->n_ਉਛਾਲ<=Array_ਮਾਪ(p_ਪੰਨਾ->apOvfl) );
  ਪ੍ਰਤਿਜ੍ਞਾ( Array_ਮਾਪ(p_ਪੰਨਾ->apOvfl)==Array_ਮਾਪ(p_ਪੰਨਾ->aiOvfl) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sz==p_ਪੰਨਾ->x_Cell_ਮਾਪ(p_ਪੰਨਾ, pCell) || CORRUPT_DB );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nFree>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->n_ਉਛਾਲ==0 );
  ਜੇ( sz+2>p_ਪੰਨਾ->nFree ){
    j = p_ਪੰਨਾ->n_ਉਛਾਲ++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    ਪ੍ਰਤਿਜ੍ਞਾ( j < Array_ਮਾਪ(p_ਪੰਨਾ->apOvfl)-1 );
    p_ਪੰਨਾ->apOvfl[j] = pCell;
    p_ਪੰਨਾ->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( j==0 || p_ਪੰਨਾ->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    ਪ੍ਰਤਿਜ੍ਞਾ( j==0 || i==p_ਪੰਨਾ->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ rc = sqlite3PagerWrite(p_ਪੰਨਾ->p_Db_ਪੰਨਾ);
    ਜੇ( rc!=SQLITE_OK ){
      ਵਾਪਸ rc;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) );
    ਅੰਕੜੇ = p_ਪੰਨਾ->a_ਅੰਕੜੇ;
    ਪ੍ਰਤਿਜ੍ਞਾ( &ਅੰਕੜੇ[p_ਪੰਨਾ->cell_ਵਿੱਥ]==p_ਪੰਨਾ->aCellIdx );
    rc = allocateSpace(p_ਪੰਨਾ, sz, &idx);
    ਜੇ( rc ){ ਵਾਪਸ rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    ਪ੍ਰਤਿਜ੍ਞਾ( idx >= 0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( idx >= p_ਪੰਨਾ->cell_ਵਿੱਥ+2*p_ਪੰਨਾ->nCell+2 || CORRUPT_DB );
    ਪ੍ਰਤਿਜ੍ਞਾ( idx+sz <= (ਪੂਰਨ)p_ਪੰਨਾ->pBt->usable_ਮਾਪ );
    p_ਪੰਨਾ->nFree -= (u16)(2 + sz);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&ਅੰਕੜੇ[idx], pCell, sz);
    pIns = p_ਪੰਨਾ->aCellIdx + i*2;
    ਜੇ( i<p_ਪੰਨਾ->nCell ) memmove(pIns+2, pIns, 2*(p_ਪੰਨਾ->nCell - i));
    put2byte(pIns, idx);
    p_ਪੰਨਾ->nCell++;
    /* increment the cell count */
    ਜੇ( (++ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+4])==0 ) ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+3]++;
    ਪ੍ਰਤਿਜ੍ਞਾ( get2byte(&ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+3])==p_ਪੰਨਾ->nCell || CORRUPT_DB );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
    ਜੇ( p_ਪੰਨਾ->pBt->autoVacuum ){
      ਪੂਰਨ rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(p_ਪੰਨਾ, p_ਪੰਨਾ, pCell, &rc2);
      ਜੇ( rc2 ) ਵਾਪਸ rc2;
    }
#ਜੇ_ਅੰਤ
  }
  ਵਾਪਸ SQLITE_OK;
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NN 1             /* Number of neighbors on either side of pPage */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NB 3             /* (NN*2+1): Total pages involved in the balance */

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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ CellArray CellArray;
ਬਣਤਰ CellArray {
  ਪੂਰਨ nCell;              /* Number of cells in apCell[] */
  Mem_ਪੰਨਾ *pRef;          /* Reference page */
  u8 **apCell;            /* All cells begin balanced */
  u16 *szCell;            /* Local size of all cells in apCell[] */
  u8 *ap_ਸਿਰਾ[NB*2];        /* MemPage.aDataEnd values */
  ਪੂਰਨ ixNx[NB*2];         /* Index of at which we move to the next apEnd[] */
};

/*
** Make sure the cell sizes at idx, idx+1, ..., idx+N-1 have been
** computed.
*/
ਸਥਿਰ ਖਾਲੀ populateCellCache(CellArray *p, ਪੂਰਨ idx, ਪੂਰਨ N){
  Mem_ਪੰਨਾ *pRef = p->pRef;
  u16 *szCell = p->szCell;
  ਪ੍ਰਤਿਜ੍ਞਾ( idx>=0 && idx+N<=p->nCell );
  ਜਦੋਂ( N>0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->apCell[idx]!=0 );
    ਜੇ( szCell[idx]==0 ){
      szCell[idx] = pRef->x_Cell_ਮਾਪ(pRef, p->apCell[idx]);
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB ||
              szCell[idx]==pRef->x_Cell_ਮਾਪ(pRef, p->apCell[idx]) );
    }
    idx++;
    N--;
  }
}

/*
** Return the size of the Nth element of the cell array
*/
ਸਥਿਰ SQLITE_NOINLINE u16 compute_Cell_ਮਾਪ(CellArray *p, ਪੂਰਨ N){
  ਪ੍ਰਤਿਜ੍ਞਾ( N>=0 && N<p->nCell );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->szCell[N]==0 );
  p->szCell[N] = p->pRef->x_Cell_ਮਾਪ(p->pRef, p->apCell[N]);
  ਵਾਪਸ p->szCell[N];
}
ਸਥਿਰ u16 cached_Cell_ਮਾਪ(CellArray *p, ਪੂਰਨ N){
  ਪ੍ਰਤਿਜ੍ਞਾ( N>=0 && N<p->nCell );
  ਜੇ( p->szCell[N] ) ਵਾਪਸ p->szCell[N];
  ਵਾਪਸ compute_Cell_ਮਾਪ(p, N);
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
ਸਥਿਰ ਪੂਰਨ rebuild_ਪੰਨਾ(
  CellArray *pCArray,             /* Content to be added to page pPg */
  ਪੂਰਨ i_ਪਹਿਲਾ,                     /* First cell in pCArray to use */
  ਪੂਰਨ nCell,                      /* Final number of cells on page */
  Mem_ਪੰਨਾ *pPg                    /* The page to be reconstructed */
){
  ਅਚਲ ਪੂਰਨ hdr = pPg->hdr_ਵਿੱਥ;          /* Offset of header on pPg */
  u8 * ਅਚਲ a_ਅੰਕੜੇ = pPg->a_ਅੰਕੜੇ;           /* Pointer to data for pPg */
  ਅਚਲ ਪੂਰਨ usable_ਮਾਪ = pPg->pBt->usable_ਮਾਪ;
  u8 * ਅਚਲ p_ਸਿਰਾ = &a_ਅੰਕੜੇ[usable_ਮਾਪ];
  ਪੂਰਨ i = i_ਪਹਿਲਾ;                 /* Which cell to copy from pCArray*/
  u32 j;                          /* Start of cell content area */
  ਪੂਰਨ i_ਸਿਰਾ = i+nCell;             /* Loop terminator */
  u8 *pCellptr = pPg->aCellIdx;
  u8 *pTmp = sqlite3_Pager_ਆਰਜ਼ੀ_Space(pPg->pBt->pPager);
  u8 *p_ਅੰਕੜੇ;
  ਪੂਰਨ k;                          /* Current slot in pCArray->apEnd[] */
  u8 *p_Src_ਸਿਰਾ;                    /* Current pCArray->apEnd[k] value */

  ਪ੍ਰਤਿਜ੍ਞਾ( nCell>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( i<i_ਸਿਰਾ );
  j = get2byte(&a_ਅੰਕੜੇ[hdr+5]);
  ਜੇ( j>(u32)usable_ਮਾਪ ){ j = 0; }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pTmp[j], &a_ਅੰਕੜੇ[j], usable_ਮਾਪ - j);

  ਪ੍ਰਤਿਜ੍ਞਾ( pCArray->ixNx[NB*2-1]>i );
  ਲਈ(k=0; pCArray->ixNx[k]<=i; k++){}
  p_Src_ਸਿਰਾ = pCArray->ap_ਸਿਰਾ[k];

  p_ਅੰਕੜੇ = p_ਸਿਰਾ;
  ਜਦੋਂ( 1/*exit by break*/ ){
    u8 *pCell = pCArray->apCell[i];
    u16 sz = pCArray->szCell[i];
    ਪ੍ਰਤਿਜ੍ਞਾ( sz>0 );
    ਜੇ( SQLITE_WITHIN(pCell,a_ਅੰਕੜੇ+j,p_ਸਿਰਾ) ){
      ਜੇ( ((uptr)(pCell+sz))>(uptr)p_ਸਿਰਾ ) ਵਾਪਸ SQLITE_CORRUPT_BKPT;
      pCell = &pTmp[pCell - a_ਅੰਕੜੇ];
    }ਨਹੀਂਤਾਂ ਜੇ( (uptr)(pCell+sz)>(uptr)p_Src_ਸਿਰਾ
           && (uptr)(pCell)<(uptr)p_Src_ਸਿਰਾ
    ){
      ਵਾਪਸ SQLITE_CORRUPT_BKPT;
    }

    p_ਅੰਕੜੇ -= sz;
    put2byte(pCellptr, (p_ਅੰਕੜੇ - a_ਅੰਕੜੇ));
    pCellptr += 2;
    ਜੇ( p_ਅੰਕੜੇ < pCellptr ) ਵਾਪਸ SQLITE_CORRUPT_BKPT;
    memmove(p_ਅੰਕੜੇ, pCell, sz);
    ਪ੍ਰਤਿਜ੍ਞਾ( sz==pPg->x_Cell_ਮਾਪ(pPg, pCell) || CORRUPT_DB );
    i++;
    ਜੇ( i>=i_ਸਿਰਾ ) ਰੋਕੋ;
    ਜੇ( pCArray->ixNx[k]<=i ){
      k++;
      p_Src_ਸਿਰਾ = pCArray->ap_ਸਿਰਾ[k];
    }
  }

  /* The pPg->nFree field is now set incorrectly. The caller will fix it. */
  ਪ੍ਰਤਿਜ੍ਞਾ( nCell < 10922 );
  pPg->nCell = (u16)nCell;
  pPg->n_ਉਛਾਲ = 0;

  put2byte(&a_ਅੰਕੜੇ[hdr+1], 0);
  put2byte(&a_ਅੰਕੜੇ[hdr+3], pPg->nCell);
  put2byte(&a_ਅੰਕੜੇ[hdr+5], p_ਅੰਕੜੇ - a_ਅੰਕੜੇ);
  a_ਅੰਕੜੇ[hdr+7] = 0x00;
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ ਪੰਨਾ_ਪਾਓ_Array(
  Mem_ਪੰਨਾ *pPg,                   /* Page to add cells to */
  u8 *pBegin,                     /* End of cell-pointer array */
  u8 **pp_ਅੰਕੜੇ,                    /* IN/OUT: Page content-area pointer */
  u8 *pCellptr,                   /* Pointer to cell-pointer area */
  ਪੂਰਨ i_ਪਹਿਲਾ,                     /* Index of first cell to add */
  ਪੂਰਨ nCell,                      /* Number of cells to add to pPg */
  CellArray *pCArray              /* Array of cells */
){
  ਪੂਰਨ i = i_ਪਹਿਲਾ;                 /* Loop counter - cell index to insert */
  u8 *a_ਅੰਕੜੇ = pPg->a_ਅੰਕੜੇ;         /* Complete page */
  u8 *p_ਅੰਕੜੇ = *pp_ਅੰਕੜੇ;            /* Content area.  A subset of aData[] */
  ਪੂਰਨ i_ਸਿਰਾ = i_ਪਹਿਲਾ + nCell;      /* End of loop. One past last cell to ins */
  ਪੂਰਨ k;                          /* Current slot in pCArray->apEnd[] */
  u8 *p_ਸਿਰਾ;                       /* Maximum extent of cell data */
  ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB || pPg->hdr_ਵਿੱਥ==0 );    /* Never called on page 1 */
  ਜੇ( i_ਸਿਰਾ<=i_ਪਹਿਲਾ ) ਵਾਪਸ 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( pCArray->ixNx[NB*2-1]>i );
  ਲਈ(k=0; pCArray->ixNx[k]<=i ; k++){}
  p_ਸਿਰਾ = pCArray->ap_ਸਿਰਾ[k];
  ਜਦੋਂ( 1 /*Exit by break*/ ){
    ਪੂਰਨ sz, rc;
    u8 *p_ਥਾਂ;
    ਪ੍ਰਤਿਜ੍ਞਾ( pCArray->szCell[i]!=0 );
    sz = pCArray->szCell[i];
    ਜੇ( (a_ਅੰਕੜੇ[1]==0 && a_ਅੰਕੜੇ[2]==0) || (p_ਥਾਂ = ਪੰਨਾ_ਲੱਭੋ_ਥਾਂ(pPg,sz,&rc))==0 ){
      ਜੇ( (p_ਅੰਕੜੇ - pBegin)<sz ) ਵਾਪਸ 1;
      p_ਅੰਕੜੇ -= sz;
      p_ਥਾਂ = p_ਅੰਕੜੇ;
    }
    /* pSlot and pCArray->apCell[i] will never overlap on a well-formed
    ** database.  But they might for a corrupt database.  Hence use memmove()
    ** since memcpy() sends SIGABORT with overlapping buffers on OpenBSD */
    ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਥਾਂ+sz)<=pCArray->apCell[i]
         || p_ਥਾਂ>=(pCArray->apCell[i]+sz)
         || CORRUPT_DB );
    ਜੇ( (uptr)(pCArray->apCell[i]+sz)>(uptr)p_ਸਿਰਾ
     && (uptr)(pCArray->apCell[i])<(uptr)p_ਸਿਰਾ
    ){
      ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB );
      (ਖਾਲੀ)SQLITE_CORRUPT_BKPT;
      ਵਾਪਸ 1;
    }
    memmove(p_ਥਾਂ, pCArray->apCell[i], sz);
    put2byte(pCellptr, (p_ਥਾਂ - a_ਅੰਕੜੇ));
    pCellptr += 2;
    i++;
    ਜੇ( i>=i_ਸਿਰਾ ) ਰੋਕੋ;
    ਜੇ( pCArray->ixNx[k]<=i ){
      k++;
      p_ਸਿਰਾ = pCArray->ap_ਸਿਰਾ[k];
    }
  }
  *pp_ਅੰਕੜੇ = p_ਅੰਕੜੇ;
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ ਪੰਨਾ_Free_Array(
  Mem_ਪੰਨਾ *pPg,                   /* Page to edit */
  ਪੂਰਨ i_ਪਹਿਲਾ,                     /* First cell to delete */
  ਪੂਰਨ nCell,                      /* Cells to delete */
  CellArray *pCArray              /* Array of cells */
){
  u8 * ਅਚਲ a_ਅੰਕੜੇ = pPg->a_ਅੰਕੜੇ;
  u8 * ਅਚਲ p_ਸਿਰਾ = &a_ਅੰਕੜੇ[pPg->pBt->usable_ਮਾਪ];
  u8 * ਅਚਲ p_ਸ਼ੁਰੂ = &a_ਅੰਕੜੇ[pPg->hdr_ਵਿੱਥ + 8 + pPg->ਬੱਚਾ_Ptr_ਮਾਪ];
  ਪੂਰਨ nRet = 0;
  ਪੂਰਨ i, j;
  ਪੂਰਨ i_ਸਿਰਾ = i_ਪਹਿਲਾ + nCell;
  ਪੂਰਨ nFree = 0;
  ਪੂਰਨ aOfst[10];
  ਪੂਰਨ aAfter[10];

  ਲਈ(i=i_ਪਹਿਲਾ; i<i_ਸਿਰਾ; i++){
    u8 *pCell = pCArray->apCell[i];
    ਜੇ( SQLITE_WITHIN(pCell, p_ਸ਼ੁਰੂ, p_ਸਿਰਾ) ){
      ਪੂਰਨ sz;
      ਪੂਰਨ iAfter;
      ਪੂਰਨ iOfst;
      /* No need to use cachedCellSize() here.  The sizes of all cells that
      ** are to be freed have already been computing while deciding which
      ** cells need freeing */
      sz = pCArray->szCell[i];  ਪ੍ਰਤਿਜ੍ਞਾ( sz>0 );
      iOfst = (u16)(pCell - a_ਅੰਕੜੇ);
      iAfter = iOfst+sz;
      ਲਈ(j=0; j<nFree; j++){
        ਜੇ( aOfst[j]==iAfter ){
          aOfst[j] = iOfst;
          ਰੋਕੋ;
        }ਨਹੀਂਤਾਂ ਜੇ( aAfter[j]==iOfst ){
          aAfter[j] = iAfter;
          ਰੋਕੋ;
        }
      }
      ਜੇ( j>=nFree ){
        ਜੇ( nFree>=(ਪੂਰਨ)(ਆਕਾਰ(aOfst)/ਆਕਾਰ(aOfst[0])) ){
          ਲਈ(j=0; j<nFree; j++){
            freeSpace(pPg, aOfst[j], aAfter[j]-aOfst[j]);
          }
          nFree = 0;
        }
        aOfst[nFree] = iOfst;
        aAfter[nFree] = iAfter;
        ਜੇ( &a_ਅੰਕੜੇ[iAfter]>p_ਸਿਰਾ ) ਵਾਪਸ 0;
        nFree++;
      }
      nRet++;
    }
  }
  ਲਈ(j=0; j<nFree; j++){
    freeSpace(pPg, aOfst[j], aAfter[j]-aOfst[j]);
  }
  ਵਾਪਸ nRet;
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
ਸਥਿਰ ਪੂਰਨ edit_ਪੰਨਾ(
  Mem_ਪੰਨਾ *pPg,                   /* Edit this page */
  ਪੂਰਨ iOld,                       /* Index of first cell currently on page */
  ਪੂਰਨ iNew,                       /* Index of new first cell on page */
  ਪੂਰਨ nNew,                       /* Final number of cells on page */
  CellArray *pCArray              /* Array of cells and sizes */
){
  u8 * ਅਚਲ a_ਅੰਕੜੇ = pPg->a_ਅੰਕੜੇ;
  ਅਚਲ ਪੂਰਨ hdr = pPg->hdr_ਵਿੱਥ;
  u8 *pBegin = &pPg->aCellIdx[nNew * 2];
  ਪੂਰਨ nCell = pPg->nCell;       /* Cells stored on pPg */
  u8 *p_ਅੰਕੜੇ;
  u8 *pCellptr;
  ਪੂਰਨ i;
  ਪੂਰਨ i_Old_ਸਿਰਾ = iOld + pPg->nCell + pPg->n_ਉਛਾਲ;
  ਪੂਰਨ i_New_ਸਿਰਾ = iNew + nNew;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  u8 *pTmp = sqlite3_Pager_ਆਰਜ਼ੀ_Space(pPg->pBt->pPager);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pTmp, a_ਅੰਕੜੇ, pPg->pBt->usable_ਮਾਪ);
#ਜੇ_ਅੰਤ

  /* Remove cells from the start and end of the page */
  ਪ੍ਰਤਿਜ੍ਞਾ( nCell>=0 );
  ਜੇ( iOld<iNew ){
    ਪੂਰਨ n_ਖਿਸਕਾਓ = ਪੰਨਾ_Free_Array(pPg, iOld, iNew-iOld, pCArray);
    ਜੇ( NEVER(n_ਖਿਸਕਾਓ>nCell) ) ਵਾਪਸ SQLITE_CORRUPT_BKPT;
    memmove(pPg->aCellIdx, &pPg->aCellIdx[n_ਖਿਸਕਾਓ*2], nCell*2);
    nCell -= n_ਖਿਸਕਾਓ;
  }
  ਜੇ( i_New_ਸਿਰਾ < i_Old_ਸਿਰਾ ){
    ਪੂਰਨ n_ਪੂਛ = ਪੰਨਾ_Free_Array(pPg, i_New_ਸਿਰਾ, i_Old_ਸਿਰਾ - i_New_ਸਿਰਾ, pCArray);
    ਪ੍ਰਤਿਜ੍ਞਾ( nCell>=n_ਪੂਛ );
    nCell -= n_ਪੂਛ;
  }

  p_ਅੰਕੜੇ = &a_ਅੰਕੜੇ[get2byte(&a_ਅੰਕੜੇ[hdr+5])];
  ਜੇ( p_ਅੰਕੜੇ<pBegin ) ਜਾਓ editpage_fail;
  ਜੇ( NEVER(p_ਅੰਕੜੇ>pPg->a_ਅੰਕੜੇ_ਸਿਰਾ) ) ਜਾਓ editpage_fail;

  /* Add cells to the start of the page */
  ਜੇ( iNew<iOld ){
    ਪੂਰਨ n_ਜੋੜੋ = MIN(nNew,iOld-iNew);
    ਪ੍ਰਤਿਜ੍ਞਾ( (iOld-iNew)<nNew || nCell==0 || CORRUPT_DB );
    ਪ੍ਰਤਿਜ੍ਞਾ( n_ਜੋੜੋ>=0 );
    pCellptr = pPg->aCellIdx;
    memmove(&pCellptr[n_ਜੋੜੋ*2], pCellptr, nCell*2);
    ਜੇ( ਪੰਨਾ_ਪਾਓ_Array(
          pPg, pBegin, &p_ਅੰਕੜੇ, pCellptr,
          iNew, n_ਜੋੜੋ, pCArray
    ) ) ਜਾਓ editpage_fail;
    nCell += n_ਜੋੜੋ;
  }

  /* Add any overflow cells */
  ਲਈ(i=0; i<pPg->n_ਉਛਾਲ; i++){
    ਪੂਰਨ iCell = (iOld + pPg->aiOvfl[i]) - iNew;
    ਜੇ( iCell>=0 && iCell<nNew ){
      pCellptr = &pPg->aCellIdx[iCell * 2];
      ਜੇ( nCell>iCell ){
        memmove(&pCellptr[2], pCellptr, (nCell - iCell) * 2);
      }
      nCell++;
      cached_Cell_ਮਾਪ(pCArray, iCell+iNew);
      ਜੇ( ਪੰਨਾ_ਪਾਓ_Array(
            pPg, pBegin, &p_ਅੰਕੜੇ, pCellptr,
            iCell+iNew, 1, pCArray
      ) ) ਜਾਓ editpage_fail;
    }
  }

  /* Append cells to the end of the page */
  ਪ੍ਰਤਿਜ੍ਞਾ( nCell>=0 );
  pCellptr = &pPg->aCellIdx[nCell*2];
  ਜੇ( ਪੰਨਾ_ਪਾਓ_Array(
        pPg, pBegin, &p_ਅੰਕੜੇ, pCellptr,
        iNew+nCell, nNew-nCell, pCArray
      )
  ){
    ਜਾਓ editpage_fail;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( nNew < 10922 );
  pPg->nCell = (u16)nNew;
  pPg->n_ਉਛਾਲ = 0;

  put2byte(&a_ਅੰਕੜੇ[hdr+3], pPg->nCell);
  put2byte(&a_ਅੰਕੜੇ[hdr+5], p_ਅੰਕੜੇ - a_ਅੰਕੜੇ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਲਈ(i=0; i<nNew && !CORRUPT_DB; i++){
    u8 *pCell = pCArray->apCell[i+iNew];
    ਪੂਰਨ iOff = get2byteAligned(&pPg->aCellIdx[i*2]);
    ਜੇ( SQLITE_WITHIN(pCell, a_ਅੰਕੜੇ, &a_ਅੰਕੜੇ[pPg->pBt->usable_ਮਾਪ]) ){
      pCell = &pTmp[pCell - a_ਅੰਕੜੇ];
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( 0==memcmp(pCell, &a_ਅੰਕੜੇ[iOff],
            pCArray->pRef->x_Cell_ਮਾਪ(pCArray->pRef, pCArray->apCell[i+iNew])) );
  }
#ਜੇ_ਅੰਤ

  ਵਾਪਸ SQLITE_OK;
 editpage_fail:
  /* Unable to edit this page. Rebuild it from scratch instead. */
  ਜੇ( nNew<1 ) ਵਾਪਸ SQLITE_CORRUPT_BKPT;
  populateCellCache(pCArray, iNew, nNew);
  ਵਾਪਸ rebuild_ਪੰਨਾ(pCArray, iNew, nNew, pPg);
}


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_QUICKBALANCE
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
ਸਥਿਰ ਪੂਰਨ balance_ਤੇਜ਼(Mem_ਪੰਨਾ *p_ਮਾਪਾ, Mem_ਪੰਨਾ *p_ਪੰਨਾ, u8 *pSpace){
  BtShared *ਅਚਲ pBt = p_ਪੰਨਾ->pBt;    /* B-Tree Database */
  Mem_ਪੰਨਾ *pNew;                       /* Newly allocated page */
  ਪੂਰਨ rc;                              /* Return Code */
  Pgno pgnoNew;                        /* Page number of pNew */

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਮਾਪਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->n_ਉਛਾਲ==1 );
 
  ਜੇ( p_ਪੰਨਾ->nCell==0 ) ਵਾਪਸ SQLITE_CORRUPT_BKPT;  /* dbfuzz001.test */
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nFree>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮਾਪਾ->nFree>=0 );

  /* Allocate a new page. This page will become the right-sibling of
  ** pPage. Make the parent page writable, so that the new divider cell
  ** may be inserted. If both these operations are successful, proceed.
  */
  rc = allocate_Btree_ਪੰਨਾ(pBt, &pNew, &pgnoNew, 0, 0);

  ਜੇ( rc==SQLITE_OK ){

    u8 *p_ਬਾਹਰਲਾ = &pSpace[4];
    u8 *pCell = p_ਪੰਨਾ->apOvfl[0];
    u16 szCell = p_ਪੰਨਾ->x_Cell_ਮਾਪ(p_ਪੰਨਾ, pCell);
    u8 *pStop;
    CellArray b;

    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(pNew->p_Db_ਪੰਨਾ) );
    ਪ੍ਰਤਿਜ੍ਞਾ( CORRUPT_DB || p_ਪੰਨਾ->a_ਅੰਕੜੇ[0]==(PTF_INTKEY|PTF_LEAFDATA|PTF_ਪੱਤੀ) );
    zero_ਪੰਨਾ(pNew, PTF_INTKEY|PTF_LEAFDATA|PTF_ਪੱਤੀ);
    b.nCell = 1;
    b.pRef = p_ਪੰਨਾ;
    b.apCell = &pCell;
    b.szCell = &szCell;
    b.ap_ਸਿਰਾ[0] = p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ;
    b.ixNx[0] = 2;
    b.ixNx[NB*2-1] = 0x7fffffff;
    rc = rebuild_ਪੰਨਾ(&b, 0, 1, pNew);
    ਜੇ( NEVER(rc) ){
      release_ਪੰਨਾ(pNew);
      ਵਾਪਸ rc;
    }
    pNew->nFree = pBt->usable_ਮਾਪ - pNew->cell_ਵਿੱਥ - 2 - szCell;

    /* If this is an auto-vacuum database, update the pointer map
    ** with entries for the new page, and any pointer from the
    ** cell on the page to an overflow page. If either of these
    ** operations fails, the return code is set, but the contents
    ** of the parent page are still manipulated by the code below.
    ** That is Ok, at this point the parent page is guaranteed to
    ** be marked as dirty. Returning an error code will cause a
    ** rollback, undoing any changes made to the parent page.
    */
    ਜੇ( ISAUTOVACUUM(pBt) ){
      ptrmapPut(pBt, pgnoNew, PTRMAP_BTREE, p_ਮਾਪਾ->pgno, &rc);
      ਜੇ( szCell>pNew->minLocal ){
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
    pCell = ਲੱਭੋ_Cell(p_ਪੰਨਾ, p_ਪੰਨਾ->nCell-1);
    pStop = &pCell[9];
    ਜਦੋਂ( (*(pCell++)&0x80) && pCell<pStop );
    pStop = &pCell[9];
    ਜਦੋਂ( ((*(p_ਬਾਹਰਲਾ++) = *(pCell++))&0x80) && pCell<pStop );

    /* Insert the new divider cell into pParent. */
    ਜੇ( rc==SQLITE_OK ){
      rc = ਪਾਓ_Cell(p_ਮਾਪਾ, p_ਮਾਪਾ->nCell, pSpace, (ਪੂਰਨ)(p_ਬਾਹਰਲਾ-pSpace),
                      0, p_ਪੰਨਾ->pgno);
    }

    /* Set the right-child pointer of pParent to point to the new page. */
    put4byte(&p_ਮਾਪਾ->a_ਅੰਕੜੇ[p_ਮਾਪਾ->hdr_ਵਿੱਥ+8], pgnoNew);
 
    /* Release the reference to the new page. */
    release_ਪੰਨਾ(pNew);
  }

  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_QUICKBALANCE */

#ਜੇ 0
/*
** This function does not contribute anything to the operation of SQLite.
** it is sometimes activated temporarily while debugging code responsible
** for setting pointer-map entries.
*/
ਸਥਿਰ ਪੂਰਨ ptrmap_Check_ਪੰਨੇ(Mem_ਪੰਨਾ **ap_ਪੰਨਾ, ਪੂਰਨ n_ਪੰਨਾ){
  ਪੂਰਨ i, j;
  ਲਈ(i=0; i<n_ਪੰਨਾ; i++){
    Pgno n;
    u8 e;
    Mem_ਪੰਨਾ *p_ਪੰਨਾ = ap_ਪੰਨਾ[i];
    BtShared *pBt = p_ਪੰਨਾ->pBt;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->isInit );

    ਲਈ(j=0; j<p_ਪੰਨਾ->nCell; j++){
      CellInfo info;
      u8 *z;
    
      z = ਲੱਭੋ_Cell(p_ਪੰਨਾ, j);
      p_ਪੰਨਾ->xParseCell(p_ਪੰਨਾ, z, &info);
      ਜੇ( info.nLocal<info.nPayload ){
        Pgno ovfl = get4byte(&z[info.n_ਮਾਪ-4]);
        ptrmapGet(pBt, ovfl, &e, &n);
        ਪ੍ਰਤਿਜ੍ਞਾ( n==p_ਪੰਨਾ->pgno && e==PTRMAP_OVERFLOW1 );
      }
      ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
        Pgno ਬੱਚਾ = get4byte(z);
        ptrmapGet(pBt, ਬੱਚਾ, &e, &n);
        ਪ੍ਰਤਿਜ੍ਞਾ( n==p_ਪੰਨਾ->pgno && e==PTRMAP_BTREE );
      }
    }
    ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
      Pgno ਬੱਚਾ = get4byte(&p_ਪੰਨਾ->a_ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+8]);
      ptrmapGet(pBt, ਬੱਚਾ, &e, &n);
      ਪ੍ਰਤਿਜ੍ਞਾ( n==p_ਪੰਨਾ->pgno && e==PTRMAP_BTREE );
    }
  }
  ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਖਾਲੀ copy_ਗੰਢ_Content(Mem_ਪੰਨਾ *pFrom, Mem_ਪੰਨਾ *p_ਮੰਜ਼ਿਲ, ਪੂਰਨ *pRC){
  ਜੇ( (*pRC)==SQLITE_OK ){
    BtShared * ਅਚਲ pBt = pFrom->pBt;
    u8 * ਅਚਲ aFrom = pFrom->a_ਅੰਕੜੇ;
    u8 * ਅਚਲ a_ਮੰਜ਼ਿਲ = p_ਮੰਜ਼ਿਲ->a_ਅੰਕੜੇ;
    ਪੂਰਨ ਅਚਲ iFromHdr = pFrom->hdr_ਵਿੱਥ;
    ਪੂਰਨ ਅਚਲ i_ਮੰਜ਼ਿਲ_Hdr = ((p_ਮੰਜ਼ਿਲ->pgno==1) ? 100 : 0);
    ਪੂਰਨ rc;
    ਪੂਰਨ i_ਅੰਕੜੇ;
 
 
    ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->isInit );
    ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->nFree>=i_ਮੰਜ਼ਿਲ_Hdr );
    ਪ੍ਰਤਿਜ੍ਞਾ( get2byte(&aFrom[iFromHdr+5]) <= (ਪੂਰਨ)pBt->usable_ਮਾਪ );
 
    /* Copy the b-tree node content from page pFrom to page pTo. */
    i_ਅੰਕੜੇ = get2byte(&aFrom[iFromHdr+5]);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&a_ਮੰਜ਼ਿਲ[i_ਅੰਕੜੇ], &aFrom[i_ਅੰਕੜੇ], pBt->usable_ਮਾਪ-i_ਅੰਕੜੇ);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&a_ਮੰਜ਼ਿਲ[i_ਮੰਜ਼ਿਲ_Hdr], &aFrom[iFromHdr], pFrom->cell_ਵਿੱਥ + 2*pFrom->nCell);
 
    /* Reinitialize page pTo so that the contents of the MemPage structure
    ** match the new data. The initialization of pTo can actually fail under
    ** fairly obscure circumstances, even though it is a copy of initialized
    ** page pFrom.
    */
    p_ਮੰਜ਼ਿਲ->isInit = 0;
    rc = btree_Init_ਪੰਨਾ(p_ਮੰਜ਼ਿਲ);
    ਜੇ( rc==SQLITE_OK ) rc = btreeComputeFreeSpace(p_ਮੰਜ਼ਿਲ);
    ਜੇ( rc!=SQLITE_OK ){
      *pRC = rc;
      ਵਾਪਸ;
    }
 
    /* If this is an auto-vacuum database, update the pointer-map entries
    ** for any b-tree or overflow pages that pTo now contains the pointers to.
    */
    ਜੇ( ISAUTOVACUUM(pBt) ){
      *pRC = ਸੈੱਟ_ਬੱਚਾ_Ptrmaps(p_ਮੰਜ਼ਿਲ);
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
ਸਥਿਰ ਪੂਰਨ balance_nonroot(
  Mem_ਪੰਨਾ *p_ਮਾਪਾ,               /* Parent page of siblings being balanced */
  ਪੂਰਨ i_ਮਾਪਾ_Idx,                 /* Index of "the page" in pParent */
  u8 *aOvflSpace,                 /* page-size bytes of space for parent ovfl */
  ਪੂਰਨ is_ਜੜ੍ਹ,                     /* True if pParent is a root-page */
  ਪੂਰਨ bBulk                       /* True if this call is part of a bulk load */
){
  BtShared *pBt;               /* The whole database */
  ਪੂਰਨ nMaxCells = 0;           /* Allocated size of apCell, szCell, aFrom. */
  ਪੂਰਨ nNew = 0;                /* Number of pages in apNew[] */
  ਪੂਰਨ nOld;                    /* Number of pages in apOld[] */
  ਪੂਰਨ i, j, k;                 /* Loop counters */
  ਪੂਰਨ nxDiv;                   /* Next divider slot in pParent->aCell[] */
  ਪੂਰਨ rc = SQLITE_OK;          /* The return code */
  u16 ਪੱਤੀ_Correction;          /* 4 if pPage is a leaf.  0 if not */
  ਪੂਰਨ ਪੱਤੀ_ਅੰਕੜੇ;                /* True if pPage is a leaf of a LEAFDATA tree */
  ਪੂਰਨ usableSpace;             /* Bytes in pPage beyond the header */
  ਪੂਰਨ ਪੰਨਾ_Flags;               /* Value of pPage->aData[0] */
  ਪੂਰਨ iSpace1 = 0;             /* First unused byte of aSpace1[] */
  ਪੂਰਨ iOvflSpace = 0;          /* First unused byte of aOvflSpace[] */
  u64 szScratch;               /* Size of scratch memory requested */
  Mem_ਪੰਨਾ *apOld[NB];          /* pPage and up to two siblings */
  Mem_ਪੰਨਾ *apNew[NB+2];        /* pPage and up to NB siblings after balancing */
  u8 *p_ਸੱਜਾ;                  /* Location in parent of right-sibling pointer */
  u8 *apDiv[NB-1];             /* Divider cells in pParent */
  ਪੂਰਨ cntNew[NB+2];            /* Index in b.paCell[] of cell after i-th page */
  ਪੂਰਨ cntOld[NB+2];            /* Old index in b.apCell[] */
  ਪੂਰਨ szNew[NB+2];             /* Combined size of cells placed on i-th page */
  u8 *aSpace1;                 /* Space for copies of dividers cells */
  Pgno pgno;                   /* Temp var to store a page number in */
  u8 ab_ਪੂਰਾ[NB+2];             /* True after i'th new page is populated */
  Pgno aPgno[NB+2];            /* Page numbers of new pages before shuffling */
  CellArray b;                 /* Parsed information on cells being balanced */

  memset(ab_ਪੂਰਾ, 0, ਆਕਾਰ(ab_ਪੂਰਾ));
  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(b) - ਆਕਾਰ(b.ixNx) == offsetof(CellArray,ixNx) );
  memset(&b, 0, ਆਕਾਰ(b)-ਆਕਾਰ(b.ixNx[0]));
  b.ixNx[NB*2-1] = 0x7fffffff;
  pBt = p_ਮਾਪਾ->pBt;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਮਾਪਾ->p_Db_ਪੰਨਾ) );

  /* At this point pParent may have at most one overflow cell. And if
  ** this overflow cell is present, it must be the cell with
  ** index iParentIdx. This scenario comes about when this function
  ** is called (indirectly) from sqlite3BtreeDelete().
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮਾਪਾ->n_ਉਛਾਲ==0 || p_ਮਾਪਾ->n_ਉਛਾਲ==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮਾਪਾ->n_ਉਛਾਲ==0 || p_ਮਾਪਾ->aiOvfl[0]==i_ਮਾਪਾ_Idx );

  ਜੇ( !aOvflSpace ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮਾਪਾ->nFree>=0 );

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
  i = p_ਮਾਪਾ->n_ਉਛਾਲ + p_ਮਾਪਾ->nCell;
  ਜੇ( i<2 ){
    nxDiv = 0;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( bBulk==0 || bBulk==1 );
    ਜੇ( i_ਮਾਪਾ_Idx==0 ){                
      nxDiv = 0;
    }ਨਹੀਂਤਾਂ ਜੇ( i_ਮਾਪਾ_Idx==i ){
      nxDiv = i-2+bBulk;
    }ਨਹੀਂਤਾਂ{
      nxDiv = i_ਮਾਪਾ_Idx-1;
    }
    i = 2-bBulk;
  }
  nOld = i+1;
  ਜੇ( (i+nxDiv-p_ਮਾਪਾ->n_ਉਛਾਲ)==p_ਮਾਪਾ->nCell ){
    p_ਸੱਜਾ = &p_ਮਾਪਾ->a_ਅੰਕੜੇ[p_ਮਾਪਾ->hdr_ਵਿੱਥ+8];
  }ਨਹੀਂਤਾਂ{
    p_ਸੱਜਾ = ਲੱਭੋ_Cell(p_ਮਾਪਾ, i+nxDiv-p_ਮਾਪਾ->n_ਉਛਾਲ);
  }
  pgno = get4byte(p_ਸੱਜਾ);
  ਜਦੋਂ( 1 ){
    ਜੇ( rc==SQLITE_OK ){
      rc = get_And_Init_ਪੰਨਾ(pBt, pgno, &apOld[i], 0);
    }
    ਜੇ( rc ){
      memset(apOld, 0, (i+1)*ਆਕਾਰ(Mem_ਪੰਨਾ*));
      ਜਾਓ balance_cleanup;
    }
    ਜੇ( apOld[i]->nFree<0 ){
      rc = btreeComputeFreeSpace(apOld[i]);
      ਜੇ( rc ){
        memset(apOld, 0, (i)*ਆਕਾਰ(Mem_ਪੰਨਾ*));
        ਜਾਓ balance_cleanup;
      }
    }
    nMaxCells += apOld[i]->nCell + Array_ਮਾਪ(p_ਮਾਪਾ->apOvfl);
    ਜੇ( (i--)==0 ) ਰੋਕੋ;

    ਜੇ( p_ਮਾਪਾ->n_ਉਛਾਲ && i+nxDiv==p_ਮਾਪਾ->aiOvfl[0] ){
      apDiv[i] = p_ਮਾਪਾ->apOvfl[0];
      pgno = get4byte(apDiv[i]);
      szNew[i] = p_ਮਾਪਾ->x_Cell_ਮਾਪ(p_ਮਾਪਾ, apDiv[i]);
      p_ਮਾਪਾ->n_ਉਛਾਲ = 0;
    }ਨਹੀਂਤਾਂ{
      apDiv[i] = ਲੱਭੋ_Cell(p_ਮਾਪਾ, i+nxDiv-p_ਮਾਪਾ->n_ਉਛਾਲ);
      pgno = get4byte(apDiv[i]);
      szNew[i] = p_ਮਾਪਾ->x_Cell_ਮਾਪ(p_ਮਾਪਾ, apDiv[i]);

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
      ਜੇ( pBt->btsFlags & BTS_FAST_SECURE ){
        ਪੂਰਨ iOff;

        /* If the following if() condition is not true, the db is corrupted.
        ** The call to dropCell() below will detect this.  */
        iOff = SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(apDiv[i]) - SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(p_ਮਾਪਾ->a_ਅੰਕੜੇ);
        ਜੇ( (iOff+szNew[i])<=(ਪੂਰਨ)pBt->usable_ਮਾਪ ){
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(&aOvflSpace[iOff], apDiv[i], szNew[i]);
          apDiv[i] = &aOvflSpace[apDiv[i]-p_ਮਾਪਾ->a_ਅੰਕੜੇ];
        }
      }
      dropCell(p_ਮਾਪਾ, i+nxDiv-p_ਮਾਪਾ->n_ਉਛਾਲ, szNew[i], &rc);
    }
  }

  /* Make nMaxCells a multiple of 4 in order to preserve 8-byte
  ** alignment */
  nMaxCells = (nMaxCells + 3)&~3;

  /*
  ** Allocate space for memory structures
  */
  szScratch =
       nMaxCells*ਆਕਾਰ(u8*)                       /* b.apCell */
     + nMaxCells*ਆਕਾਰ(u16)                       /* b.szCell */
     + pBt->ਪੰਨਾ_ਮਾਪ;                              /* aSpace1 */

  ਪ੍ਰਤਿਜ੍ਞਾ( szScratch<=7*(ਪੂਰਨ)pBt->ਪੰਨਾ_ਮਾਪ );
  b.apCell = sqlite3_ਢੇਰ_Alloc_Raw(0, szScratch );
  ਜੇ( b.apCell==0 ){
    rc = SQLITE_NOMEM_BKPT;
    ਜਾਓ balance_cleanup;
  }
  b.szCell = (u16*)&b.apCell[nMaxCells];
  aSpace1 = (u8*)&b.szCell[nMaxCells];
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT(aSpace1) );

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
  ਪੱਤੀ_Correction = b.pRef->ਪੱਤੀ*4;
  ਪੱਤੀ_ਅੰਕੜੇ = b.pRef->int_ਕੁੰਜੀ_ਪੱਤੀ;
  ਲਈ(i=0; i<nOld; i++){
    Mem_ਪੰਨਾ *pOld = apOld[i];
    ਪੂਰਨ ਹੱਦਬੰਦੀ = pOld->nCell;
    u8 *a_ਅੰਕੜੇ = pOld->a_ਅੰਕੜੇ;
    u16 ਨਕਾਬ_ਪੰਨਾ = pOld->ਨਕਾਬ_ਪੰਨਾ;
    u8 *piCell = a_ਅੰਕੜੇ + pOld->cell_ਵਿੱਥ;
    u8 *pi_ਸਿਰਾ;
    u8 **ppCell;
    VVA_ONLY( ਪੂਰਨ n_Cell_At_ਸ਼ੁਰੂ = b.nCell; )

    /* Verify that all sibling pages are of the same "type" (table-leaf,
    ** table-interior, index-leaf, or index-interior).
    */
    ਜੇ( pOld->a_ਅੰਕੜੇ[0]!=apOld[0]->a_ਅੰਕੜੇ[0] ){
      rc = SQLITE_CORRUPT_ਪੰਨਾ(pOld);
      ਜਾਓ balance_cleanup;
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
    memset(&b.szCell[b.nCell], 0, ਆਕਾਰ(b.szCell[0])*(ਹੱਦਬੰਦੀ+pOld->n_ਉਛਾਲ));
    ਜੇ( pOld->n_ਉਛਾਲ>0 ){
      ਜੇ( NEVER(ਹੱਦਬੰਦੀ<pOld->aiOvfl[0]) ){
        rc = SQLITE_CORRUPT_ਪੰਨਾ(pOld);
        ਜਾਓ balance_cleanup;
      }
      ਹੱਦਬੰਦੀ = pOld->aiOvfl[0];
      ਲਈ(j=0; j<ਹੱਦਬੰਦੀ; j++){
        b.apCell[b.nCell+j] = a_ਅੰਕੜੇ + (ਨਕਾਬ_ਪੰਨਾ & get2byteAligned(piCell));
        piCell += 2;
      }
      b.nCell += ਹੱਦਬੰਦੀ;
      ਲਈ(k=0; k<pOld->n_ਉਛਾਲ; k++){
        ਪ੍ਰਤਿਜ੍ਞਾ( k==0 || pOld->aiOvfl[k-1]+1==pOld->aiOvfl[k] );/* NOTE 1 */
        b.apCell[b.nCell] = pOld->apOvfl[k];
        b.nCell++;
      }
    }
    pi_ਸਿਰਾ = a_ਅੰਕੜੇ + pOld->cell_ਵਿੱਥ + 2*pOld->nCell;
    /* Collect cell pointers before updating the cell count. */
    ppCell = &b.apCell[b.nCell];
    ਜਦੋਂ( piCell<pi_ਸਿਰਾ ){
      ਪ੍ਰਤਿਜ੍ਞਾ( ppCell<&b.apCell[nMaxCells] );
      *ppCell++ = a_ਅੰਕੜੇ + (ਨਕਾਬ_ਪੰਨਾ & get2byteAligned(piCell));
      piCell += 2;
    }
    b.nCell = (ਪੂਰਨ)(ppCell - b.apCell);
    ਪ੍ਰਤਿਜ੍ਞਾ( (b.nCell-n_Cell_At_ਸ਼ੁਰੂ)==(pOld->nCell+pOld->n_ਉਛਾਲ) );

    cntOld[i] = b.nCell;
    ਜੇ( i<nOld-1 && !ਪੱਤੀ_ਅੰਕੜੇ){
      u16 sz = (u16)szNew[i];
      u8 *p_ਆਰਜ਼ੀ;
      ਪ੍ਰਤਿਜ੍ਞਾ( b.nCell<nMaxCells );
      b.szCell[b.nCell] = sz;
      p_ਆਰਜ਼ੀ = &aSpace1[iSpace1];
      iSpace1 += sz;
      ਪ੍ਰਤਿਜ੍ਞਾ( sz<=pBt->maxLocal+23 );
      ਪ੍ਰਤਿਜ੍ਞਾ( iSpace1 <= (ਪੂਰਨ)pBt->ਪੰਨਾ_ਮਾਪ );
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਆਰਜ਼ੀ, apDiv[i], sz);
      b.apCell[b.nCell] = p_ਆਰਜ਼ੀ+ਪੱਤੀ_Correction;
      ਪ੍ਰਤਿਜ੍ਞਾ( ਪੱਤੀ_Correction==0 || ਪੱਤੀ_Correction==4 );
      b.szCell[b.nCell] = b.szCell[b.nCell] - ਪੱਤੀ_Correction;
      ਜੇ( !pOld->ਪੱਤੀ ){
        ਪ੍ਰਤਿਜ੍ਞਾ( ਪੱਤੀ_Correction==0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( pOld->hdr_ਵਿੱਥ==0 || CORRUPT_DB );
        /* The right pointer of the child page pOld becomes the left
        ** pointer of the divider cell */
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(b.apCell[b.nCell], &pOld->a_ਅੰਕੜੇ[8], 4);
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( ਪੱਤੀ_Correction==4 );
        ਜਦੋਂ( b.szCell[b.nCell]<4 ){
          /* Do not allow any cells smaller than 4 bytes. If a smaller cell
          ** does exist, pad it with 0x00 bytes. */
          ਪ੍ਰਤਿਜ੍ਞਾ( b.szCell[b.nCell]==3 || CORRUPT_DB );
          ਪ੍ਰਤਿਜ੍ਞਾ( b.apCell[b.nCell]==&aSpace1[iSpace1-3] || CORRUPT_DB );
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
  usableSpace = pBt->usable_ਮਾਪ - 12 + ਪੱਤੀ_Correction;
  ਲਈ(i=k=0; i<nOld; i++, k++){
    Mem_ਪੰਨਾ *p = apOld[i];
    b.ap_ਸਿਰਾ[k] = p->a_ਅੰਕੜੇ_ਸਿਰਾ;
    b.ixNx[k] = cntOld[i];
    ਜੇ( k && b.ixNx[k]==b.ixNx[k-1] ){
      k--;  /* Omit b.ixNx[] entry for child pages with no cells */
    }
    ਜੇ( !ਪੱਤੀ_ਅੰਕੜੇ ){
      k++;
      b.ap_ਸਿਰਾ[k] = p_ਮਾਪਾ->a_ਅੰਕੜੇ_ਸਿਰਾ;
      b.ixNx[k] = cntOld[i]+1;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( p->nFree>=0 );
    szNew[i] = usableSpace - p->nFree;
    ਲਈ(j=0; j<p->n_ਉਛਾਲ; j++){
      szNew[i] += 2 + p->x_Cell_ਮਾਪ(p, p->apOvfl[j]);
    }
    cntNew[i] = cntOld[i];
  }
  k = nOld;
  ਲਈ(i=0; i<k; i++){
    ਪੂਰਨ sz;
    ਜਦੋਂ( szNew[i]>usableSpace ){
      ਜੇ( i+1>=k ){
        k = i+2;
        ਜੇ( k>NB+2 ){ rc = SQLITE_CORRUPT_BKPT; ਜਾਓ balance_cleanup; }
        szNew[k-1] = 0;
        cntNew[k-1] = b.nCell;
      }
      sz = 2 + cached_Cell_ਮਾਪ(&b, cntNew[i]-1);
      szNew[i] -= sz;
      ਜੇ( !ਪੱਤੀ_ਅੰਕੜੇ ){
        ਜੇ( cntNew[i]<b.nCell ){
          sz = 2 + cached_Cell_ਮਾਪ(&b, cntNew[i]);
        }ਨਹੀਂਤਾਂ{
          sz = 0;
        }
      }
      szNew[i+1] += sz;
      cntNew[i]--;
    }
    ਜਦੋਂ( cntNew[i]<b.nCell ){
      sz = 2 + cached_Cell_ਮਾਪ(&b, cntNew[i]);
      ਜੇ( szNew[i]+sz>usableSpace ) ਰੋਕੋ;
      szNew[i] += sz;
      cntNew[i]++;
      ਜੇ( !ਪੱਤੀ_ਅੰਕੜੇ ){
        ਜੇ( cntNew[i]<b.nCell ){
          sz = 2 + cached_Cell_ਮਾਪ(&b, cntNew[i]);
        }ਨਹੀਂਤਾਂ{
          sz = 0;
        }
      }
      szNew[i+1] -= sz;
    }
    ਜੇ( cntNew[i]>=b.nCell ){
      k = i+1;
    }ਨਹੀਂਤਾਂ ਜੇ( cntNew[i] <= (i>0 ? cntNew[i-1] : 0) ){
      rc = SQLITE_CORRUPT_BKPT;
      ਜਾਓ balance_cleanup;
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
  ਲਈ(i=k-1; i>0; i--){
    ਪੂਰਨ sz_ਸੱਜਾ = szNew[i];  /* Size of sibling on the right */
    ਪੂਰਨ sz_ਖੱਬਾ = szNew[i-1]; /* Size of sibling on the left */
    ਪੂਰਨ r;              /* Index of right-most cell in left sibling */
    ਪੂਰਨ d;              /* Index of first cell to the left of right sibling */

    r = cntNew[i-1] - 1;
    d = r + 1 - ਪੱਤੀ_ਅੰਕੜੇ;
    (ਖਾਲੀ)cached_Cell_ਮਾਪ(&b, d);
    ਕਰੋ{
      ਪੂਰਨ szR, szD;
      ਪ੍ਰਤਿਜ੍ਞਾ( d<nMaxCells );
      ਪ੍ਰਤਿਜ੍ਞਾ( r<nMaxCells );
      szR = cached_Cell_ਮਾਪ(&b, r);
      szD = b.szCell[d];
      ਜੇ( sz_ਸੱਜਾ!=0
       && (bBulk || sz_ਸੱਜਾ+szD+2 > sz_ਖੱਬਾ-(szR+(i==k-1?0:2)))){
        ਰੋਕੋ;
      }
      sz_ਸੱਜਾ += szD + 2;
      sz_ਖੱਬਾ -= szR + 2;
      cntNew[i-1] = r;
      r--;
      d--;
    }ਜਦੋਂ( r>=0 );
    szNew[i] = sz_ਸੱਜਾ;
    szNew[i-1] = sz_ਖੱਬਾ;
    ਜੇ( cntNew[i-1] <= (i>1 ? cntNew[i-2] : 0) ){
      rc = SQLITE_CORRUPT_BKPT;
      ਜਾਓ balance_cleanup;
    }
  }

  /* Sanity check:  For a non-corrupt database file one of the following
  ** must be true:
  **    (1) We found one or more cells (cntNew[0])>0), or
  **    (2) pPage is a virtual root page.  A virtual root page is when
  **        the real root page is page 1 and we are the only child of
  **        that page.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( cntNew[0]>0 || (p_ਮਾਪਾ->pgno==1 && p_ਮਾਪਾ->nCell==0) || CORRUPT_DB);
  TRACE(("BALANCE: old: %u(nc=%u) %u(nc=%u) %u(nc=%u)\n",
    apOld[0]->pgno, apOld[0]->nCell,
    nOld>=2 ? apOld[1]->pgno : 0, nOld>=2 ? apOld[1]->nCell : 0,
    nOld>=3 ? apOld[2]->pgno : 0, nOld>=3 ? apOld[2]->nCell : 0
  ));

  /*
  ** Allocate k new pages.  Reuse old pages where possible.
  */
  ਪੰਨਾ_Flags = apOld[0]->a_ਅੰਕੜੇ[0];
  ਲਈ(i=0; i<k; i++){
    Mem_ਪੰਨਾ *pNew;
    ਜੇ( i<nOld ){
      pNew = apNew[i] = apOld[i];
      apOld[i] = 0;
      rc = sqlite3PagerWrite(pNew->p_Db_ਪੰਨਾ);
      nNew++;
      ਜੇ( sqlite3_Pager_ਪੰਨਾ_Refcount(pNew->p_Db_ਪੰਨਾ)!=1+(i==(i_ਮਾਪਾ_Idx-nxDiv))
       && rc==SQLITE_OK
      ){
        rc = SQLITE_CORRUPT_BKPT;
      }
      ਜੇ( rc ) ਜਾਓ balance_cleanup;
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( i>0 );
      rc = allocate_Btree_ਪੰਨਾ(pBt, &pNew, &pgno, (bBulk ? 1 : pgno), 0);
      ਜੇ( rc ) ਜਾਓ balance_cleanup;
      zero_ਪੰਨਾ(pNew, ਪੰਨਾ_Flags);
      apNew[i] = pNew;
      nNew++;
      cntOld[i] = b.nCell;

      /* Set the pointer-map entry for the new sibling page. */
      ਜੇ( ISAUTOVACUUM(pBt) ){
        ptrmapPut(pBt, pNew->pgno, PTRMAP_BTREE, p_ਮਾਪਾ->pgno, &rc);
        ਜੇ( rc!=SQLITE_OK ){
          ਜਾਓ balance_cleanup;
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
  ਲਈ(i=0; i<nNew; i++){
    aPgno[i] = apNew[i]->pgno;
    ਪ੍ਰਤਿਜ੍ਞਾ( apNew[i]->p_Db_ਪੰਨਾ->flags & PGHDR_WRITEABLE );
    ਪ੍ਰਤਿਜ੍ਞਾ( apNew[i]->p_Db_ਪੰਨਾ->flags & PGHDR_DIRTY );
  }
  ਲਈ(i=0; i<nNew-1; i++){
    ਪੂਰਨ iB = i;
    ਲਈ(j=i+1; j<nNew; j++){
      ਜੇ( apNew[j]->pgno < apNew[iB]->pgno ) iB = j;
    }

    /* If apNew[i] has a page number that is bigger than any of the
    ** subsequence apNew[i] entries, then swap apNew[i] with the subsequent
    ** entry that has the smallest page number (which we know to be
    ** entry apNew[iB]).
    */
    ਜੇ( iB!=i ){
      Pgno pgnoA = apNew[i]->pgno;
      Pgno pgnoB = apNew[iB]->pgno;
      Pgno pgno_ਆਰਜ਼ੀ = (PENDING_BYTE/pBt->ਪੰਨਾ_ਮਾਪ)+1;
      u16 fgA = apNew[i]->p_Db_ਪੰਨਾ->flags;
      u16 fgB = apNew[iB]->p_Db_ਪੰਨਾ->flags;
      sqlite3PagerRekey(apNew[i]->p_Db_ਪੰਨਾ, pgno_ਆਰਜ਼ੀ, fgB);
      sqlite3PagerRekey(apNew[iB]->p_Db_ਪੰਨਾ, pgnoA, fgA);
      sqlite3PagerRekey(apNew[i]->p_Db_ਪੰਨਾ, pgnoB, fgB);
      apNew[i]->pgno = pgnoB;
      apNew[iB]->pgno = pgnoA;
    }
  }

  TRACE(("BALANCE: new: %u(%u nc=%u) %u(%u nc=%u) %u(%u nc=%u) "
         "%u(%u nc=%u) %u(%u nc=%u)\n",
    apNew[0]->pgno, szNew[0], cntNew[0],
    nNew>=2 ? apNew[1]->pgno : 0, nNew>=2 ? szNew[1] : 0,
    nNew>=2 ? cntNew[1] - cntNew[0] - !ਪੱਤੀ_ਅੰਕੜੇ : 0,
    nNew>=3 ? apNew[2]->pgno : 0, nNew>=3 ? szNew[2] : 0,
    nNew>=3 ? cntNew[2] - cntNew[1] - !ਪੱਤੀ_ਅੰਕੜੇ : 0,
    nNew>=4 ? apNew[3]->pgno : 0, nNew>=4 ? szNew[3] : 0,
    nNew>=4 ? cntNew[3] - cntNew[2] - !ਪੱਤੀ_ਅੰਕੜੇ : 0,
    nNew>=5 ? apNew[4]->pgno : 0, nNew>=5 ? szNew[4] : 0,
    nNew>=5 ? cntNew[4] - cntNew[3] - !ਪੱਤੀ_ਅੰਕੜੇ : 0
  ));

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਮਾਪਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( nNew>=1 && nNew<=Array_ਮਾਪ(apNew) );
  ਪ੍ਰਤਿਜ੍ਞਾ( apNew[nNew-1]!=0 );
  put4byte(p_ਸੱਜਾ, apNew[nNew-1]->pgno);

  /* If the sibling pages are not leaves, ensure that the right-child pointer
  ** of the right-most new sibling page is set to the value that was
  ** originally in the same field of the right-most old sibling page. */
  ਜੇ( (ਪੰਨਾ_Flags & PTF_ਪੱਤੀ)==0 && nOld!=nNew ){
    Mem_ਪੰਨਾ *pOld;
    ਜੇ( nNew>nOld ){
      pOld = apNew[nOld-1];
    }ਨਹੀਂਤਾਂ{
      pOld = apOld[nOld-1];
    }
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&apNew[nNew-1]->a_ਅੰਕੜੇ[8], &pOld->a_ਅੰਕੜੇ[8], 4);
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
  ਜੇ( ISAUTOVACUUM(pBt) ){
    Mem_ਪੰਨਾ *pOld;
    Mem_ਪੰਨਾ *pNew = pOld = apNew[0];
    ਪੂਰਨ cnt_Old_ਅਗਲਾ = pNew->nCell + pNew->n_ਉਛਾਲ;
    ਪੂਰਨ iNew = 0;
    ਪੂਰਨ iOld = 0;

    ਲਈ(i=0; i<b.nCell; i++){
      u8 *pCell = b.apCell[i];
      ਜਦੋਂ( i==cnt_Old_ਅਗਲਾ ){
        iOld++;
        ਪ੍ਰਤਿਜ੍ਞਾ( iOld<nNew || iOld<nOld );
        ਪ੍ਰਤਿਜ੍ਞਾ( iOld>=0 && iOld<NB );
        pOld = iOld<nNew ? apNew[iOld] : apOld[iOld];
        cnt_Old_ਅਗਲਾ += pOld->nCell + pOld->n_ਉਛਾਲ + !ਪੱਤੀ_ਅੰਕੜੇ;
      }
      ਜੇ( i==cntNew[iNew] ){
        pNew = apNew[++iNew];
        ਜੇ( !ਪੱਤੀ_ਅੰਕੜੇ ) ਜਾਰੀ;
      }

      /* Cell pCell is destined for new sibling page pNew. Originally, it
      ** was either part of sibling page iOld (possibly an overflow cell),
      ** or else the divider cell to the left of sibling page iOld. So,
      ** if sibling page iOld had the same page number as pNew, and if
      ** pCell really was a part of sibling page iOld (not a divider or
      ** overflow cell), we can skip updating the pointer map entries.  */
      ਜੇ( iOld>=nNew
       || pNew->pgno!=aPgno[iOld]
       || !SQLITE_WITHIN(pCell,pOld->a_ਅੰਕੜੇ,pOld->a_ਅੰਕੜੇ_ਸਿਰਾ)
      ){
        ਜੇ( !ਪੱਤੀ_Correction ){
          ptrmapPut(pBt, get4byte(pCell), PTRMAP_BTREE, pNew->pgno, &rc);
        }
        ਜੇ( cached_Cell_ਮਾਪ(&b,i)>pNew->minLocal ){
          ptrmapPutOvflPtr(pNew, pOld, pCell, &rc);
        }
        ਜੇ( rc ) ਜਾਓ balance_cleanup;
      }
    }
  }

  /* Insert new divider cells into pParent. */
  ਲਈ(i=0; i<nNew-1; i++){
    u8 *pCell;
    u8 *p_ਆਰਜ਼ੀ;
    ਪੂਰਨ sz;
    u8 *p_Src_ਸਿਰਾ;
    Mem_ਪੰਨਾ *pNew = apNew[i];
    j = cntNew[i];

    ਪ੍ਰਤਿਜ੍ਞਾ( j<nMaxCells );
    ਪ੍ਰਤਿਜ੍ਞਾ( b.apCell[j]!=0 );
    pCell = b.apCell[j];
    sz = b.szCell[j] + ਪੱਤੀ_Correction;
    p_ਆਰਜ਼ੀ = &aOvflSpace[iOvflSpace];
    ਜੇ( !pNew->ਪੱਤੀ ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&pNew->a_ਅੰਕੜੇ[8], pCell, 4);
    }ਨਹੀਂਤਾਂ ਜੇ( ਪੱਤੀ_ਅੰਕੜੇ ){
      /* If the tree is a leaf-data tree, and the siblings are leaves,
      ** then there is no divider cell in b.apCell[]. Instead, the divider
      ** cell consists of the integer key for the right-most cell of
      ** the sibling-page assembled above only.
      */
      CellInfo info;
      j--;
      pNew->xParseCell(pNew, b.apCell[j], &info);
      pCell = p_ਆਰਜ਼ੀ;
      sz = 4 + sqlite3PutVarint(&pCell[4], info.n_ਕੁੰਜੀ);
      p_ਆਰਜ਼ੀ = 0;
    }ਨਹੀਂਤਾਂ{
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
      ਜੇ( b.szCell[j]==4 ){
        ਪ੍ਰਤਿਜ੍ਞਾ(ਪੱਤੀ_Correction==4);
        sz = p_ਮਾਪਾ->x_Cell_ਮਾਪ(p_ਮਾਪਾ, pCell);
      }
    }
    iOvflSpace += sz;
    ਪ੍ਰਤਿਜ੍ਞਾ( sz<=pBt->maxLocal+23 );
    ਪ੍ਰਤਿਜ੍ਞਾ( iOvflSpace <= (ਪੂਰਨ)pBt->ਪੰਨਾ_ਮਾਪ );
    ਪ੍ਰਤਿਜ੍ਞਾ( b.ixNx[NB*2-1]>j );
    ਲਈ(k=0; b.ixNx[k]<=j; k++){}
    p_Src_ਸਿਰਾ = b.ap_ਸਿਰਾ[k];
    ਜੇ( SQLITE_ਉਛਾਲ(p_Src_ਸਿਰਾ, pCell, pCell+sz) ){
      rc = SQLITE_CORRUPT_BKPT;
      ਜਾਓ balance_cleanup;
    }
    rc = ਪਾਓ_Cell(p_ਮਾਪਾ, nxDiv+i, pCell, sz, p_ਆਰਜ਼ੀ, pNew->pgno);
    ਜੇ( rc!=SQLITE_OK ) ਜਾਓ balance_cleanup;
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਮਾਪਾ->p_Db_ਪੰਨਾ) );
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
  ਲਈ(i=1-nNew; i<nNew; i++){
    ਪੂਰਨ iPg = i<0 ? -i : i;
    ਪ੍ਰਤਿਜ੍ਞਾ( iPg>=0 && iPg<nNew );
    ਪ੍ਰਤਿਜ੍ਞਾ( iPg>=1 || i>=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( iPg<Array_ਮਾਪ(cntOld) );
    ਜੇ( ab_ਪੂਰਾ[iPg] ) ਜਾਰੀ;         /* Skip pages already processed */
    ਜੇ( i>=0                            /* On the upwards pass, or... */
     || cntOld[iPg-1]>=cntNew[iPg-1]    /* Condition (1) is true */
    ){
      ਪੂਰਨ iNew;
      ਪੂਰਨ iOld;
      ਪੂਰਨ nNewCell;

      /* Verify condition (1):  If cells are moving left, update iPg
      ** only after iPg-1 has already been updated. */
      ਪ੍ਰਤਿਜ੍ਞਾ( iPg==0 || cntOld[iPg-1]>=cntNew[iPg-1] || ab_ਪੂਰਾ[iPg-1] );

      /* Verify condition (2):  If cells are moving right, update iPg
      ** only after iPg+1 has already been updated. */
      ਪ੍ਰਤਿਜ੍ਞਾ( cntNew[iPg]>=cntOld[iPg] || ab_ਪੂਰਾ[iPg+1] );

      ਜੇ( iPg==0 ){
        iNew = iOld = 0;
        nNewCell = cntNew[0];
      }ਨਹੀਂਤਾਂ{
        iOld = iPg<nOld ? (cntOld[iPg-1] + !ਪੱਤੀ_ਅੰਕੜੇ) : b.nCell;
        iNew = cntNew[iPg-1] + !ਪੱਤੀ_ਅੰਕੜੇ;
        nNewCell = cntNew[iPg] - iNew;
      }

      rc = edit_ਪੰਨਾ(apNew[iPg], iOld, iNew, nNewCell, &b);
      ਜੇ( rc ) ਜਾਓ balance_cleanup;
      ab_ਪੂਰਾ[iPg]++;
      apNew[iPg]->nFree = usableSpace-szNew[iPg];
      ਪ੍ਰਤਿਜ੍ਞਾ( apNew[iPg]->n_ਉਛਾਲ==0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( apNew[iPg]->nCell==nNewCell );
    }
  }

  /* All pages have been processed exactly once */
  ਪ੍ਰਤਿਜ੍ਞਾ( memcmp(ab_ਪੂਰਾ, "\01\01\01\01\01", nNew)==0 );

  ਪ੍ਰਤਿਜ੍ਞਾ( nOld>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( nNew>0 );

  ਜੇ( is_ਜੜ੍ਹ && p_ਮਾਪਾ->nCell==0 && p_ਮਾਪਾ->hdr_ਵਿੱਥ<=apNew[0]->nFree ){
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
    ਪ੍ਰਤਿਜ੍ਞਾ( nNew==1 || CORRUPT_DB );
    rc = defragment_ਪੰਨਾ(apNew[0], -1);
    testcase( rc!=SQLITE_OK );
    ਪ੍ਰਤਿਜ੍ਞਾ( apNew[0]->nFree ==
        (get2byteNotZero(&apNew[0]->a_ਅੰਕੜੇ[5]) - apNew[0]->cell_ਵਿੱਥ
          - apNew[0]->nCell*2)
      || rc!=SQLITE_OK
    );
    copy_ਗੰਢ_Content(apNew[0], p_ਮਾਪਾ, &rc);
    free_ਪੰਨਾ(apNew[0], &rc);
  }ਨਹੀਂਤਾਂ ਜੇ( ISAUTOVACUUM(pBt) && !ਪੱਤੀ_Correction ){
    /* Fix the pointer map entries associated with the right-child of each
    ** sibling page. All other pointer map entries have already been taken
    ** care of.  */
    ਲਈ(i=0; i<nNew; i++){
      u32 ਕੁੰਜੀ = get4byte(&apNew[i]->a_ਅੰਕੜੇ[8]);
      ptrmapPut(pBt, ਕੁੰਜੀ, PTRMAP_BTREE, apNew[i]->pgno, &rc);
    }
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮਾਪਾ->isInit );
  TRACE(("BALANCE: finished: old=%u new=%u cells=%u\n",
          nOld, nNew, b.nCell));

  /* Free any old pages that were not reused as new pages.
  */
  ਲਈ(i=nNew; i<nOld; i++){
    free_ਪੰਨਾ(apOld[i], &rc);
  }

#ਜੇ 0
  ਜੇ( ISAUTOVACUUM(pBt) && rc==SQLITE_OK && apNew[0]->isInit ){
    /* The ptrmapCheckPages() contains assert() statements that verify that
    ** all pointer map pages are set correctly. This is helpful while
    ** debugging. This is usually disabled because a corrupt database may
    ** cause an assert() statement to fail.  */
    ptrmap_Check_ਪੰਨੇ(apNew, nNew);
    ptrmap_Check_ਪੰਨੇ(&p_ਮਾਪਾ, 1);
  }
#ਜੇ_ਅੰਤ

  /*
  ** Cleanup before returning.
  */
balance_cleanup:
  sqlite3_ਢੇਰ_Free(0, b.apCell);
  ਲਈ(i=0; i<nOld; i++){
    release_ਪੰਨਾ(apOld[i]);
  }
  ਲਈ(i=0; i<nNew; i++){
    release_ਪੰਨਾ(apNew[i]);
  }

  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ balance_deeper(Mem_ਪੰਨਾ *p_ਜੜ੍ਹ, Mem_ਪੰਨਾ **pp_ਬੱਚਾ){
  ਪੂਰਨ rc;                        /* Return value from subprocedures */
  Mem_ਪੰਨਾ *p_ਬੱਚਾ = 0;           /* Pointer to a new child page */
  Pgno pgno_ਬੱਚਾ = 0;            /* Page number of the new child page */
  BtShared *pBt = p_ਜੜ੍ਹ->pBt;    /* The BTree */

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਜੜ੍ਹ->n_ਉਛਾਲ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );

  /* Make pRoot, the root page of the b-tree, writable. Allocate a new
  ** page that will become the new right-child of pPage. Copy the contents
  ** of the node stored on pRoot into the new child page.
  */
  rc = sqlite3PagerWrite(p_ਜੜ੍ਹ->p_Db_ਪੰਨਾ);
  ਜੇ( rc==SQLITE_OK ){
    rc = allocate_Btree_ਪੰਨਾ(pBt,&p_ਬੱਚਾ,&pgno_ਬੱਚਾ,p_ਜੜ੍ਹ->pgno,0);
    copy_ਗੰਢ_Content(p_ਜੜ੍ਹ, p_ਬੱਚਾ, &rc);
    ਜੇ( ISAUTOVACUUM(pBt) ){
      ptrmapPut(pBt, pgno_ਬੱਚਾ, PTRMAP_BTREE, p_ਜੜ੍ਹ->pgno, &rc);
    }
  }
  ਜੇ( rc ){
    *pp_ਬੱਚਾ = 0;
    release_ਪੰਨਾ(p_ਬੱਚਾ);
    ਵਾਪਸ rc;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਬੱਚਾ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਜੜ੍ਹ->p_Db_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਬੱਚਾ->nCell==p_ਜੜ੍ਹ->nCell || CORRUPT_DB );

  TRACE(("BALANCE: copy root %u into %u\n", p_ਜੜ੍ਹ->pgno, p_ਬੱਚਾ->pgno));

  /* Copy the overflow cells from pRoot to pChild */
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਬੱਚਾ->aiOvfl, p_ਜੜ੍ਹ->aiOvfl,
         p_ਜੜ੍ਹ->n_ਉਛਾਲ*ਆਕਾਰ(p_ਜੜ੍ਹ->aiOvfl[0]));
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਬੱਚਾ->apOvfl, p_ਜੜ੍ਹ->apOvfl,
         p_ਜੜ੍ਹ->n_ਉਛਾਲ*ਆਕਾਰ(p_ਜੜ੍ਹ->apOvfl[0]));
  p_ਬੱਚਾ->n_ਉਛਾਲ = p_ਜੜ੍ਹ->n_ਉਛਾਲ;

  /* Zero the contents of pRoot. Then install pChild as the right-child. */
  zero_ਪੰਨਾ(p_ਜੜ੍ਹ, p_ਬੱਚਾ->a_ਅੰਕੜੇ[0] & ~PTF_ਪੱਤੀ);
  put4byte(&p_ਜੜ੍ਹ->a_ਅੰਕੜੇ[p_ਜੜ੍ਹ->hdr_ਵਿੱਥ+8], pgno_ਬੱਚਾ);

  *pp_ਬੱਚਾ = p_ਬੱਚਾ;
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ anotherValidCursor(BtCursor *pCur){
  BtCursor *pOther;
  ਲਈ(pOther=pCur->pBt->pCursor; pOther; pOther=pOther->p_ਅਗਲਾ){
    ਜੇ( pOther!=pCur
     && pOther->eState==CURSOR_VALID
     && pOther->p_ਪੰਨਾ==pCur->p_ਪੰਨਾ
    ){
      ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(pCur->p_ਪੰਨਾ);
    }
  }
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ balance(BtCursor *pCur){
  ਪੂਰਨ rc = SQLITE_OK;
  u8 a_Balance_ਤੇਜ਼_Space[13];
  u8 *pFree = 0;

  VVA_ONLY( ਪੂਰਨ balance_ਤੇਜ਼_called = 0 );
  VVA_ONLY( ਪੂਰਨ balance_deeper_called = 0 );

  ਕਰੋ {
    ਪੂਰਨ i_ਪੰਨਾ;
    Mem_ਪੰਨਾ *p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;

    ਜੇ( NEVER(p_ਪੰਨਾ->nFree<0) && btreeComputeFreeSpace(p_ਪੰਨਾ) ) ਰੋਕੋ;
    ਜੇ( p_ਪੰਨਾ->n_ਉਛਾਲ==0 && p_ਪੰਨਾ->nFree*3<=(ਪੂਰਨ)pCur->pBt->usable_ਮਾਪ*2 ){
      /* No rebalance required as long as:
      **   (1) There are no overflow cells
      **   (2) The amount of free space on the page is less than 2/3rds of
      **       the total usable space on the page. */
      ਰੋਕੋ;
    }ਨਹੀਂਤਾਂ ਜੇ( (i_ਪੰਨਾ = pCur->i_ਪੰਨਾ)==0 ){
      ਜੇ( p_ਪੰਨਾ->n_ਉਛਾਲ && (rc = anotherValidCursor(pCur))==SQLITE_OK ){
        /* The root page of the b-tree is overfull. In this case call the
        ** balance_deeper() function to create a new child for the root-page
        ** and copy the current contents of the root-page to it. The
        ** next iteration of the do-loop will balance the child page.
        */
        ਪ੍ਰਤਿਜ੍ਞਾ( balance_deeper_called==0 );
        VVA_ONLY( balance_deeper_called++ );
        rc = balance_deeper(p_ਪੰਨਾ, &pCur->ap_ਪੰਨਾ[1]);
        ਜੇ( rc==SQLITE_OK ){
          pCur->i_ਪੰਨਾ = 1;
          pCur->ix = 0;
          pCur->aiIdx[0] = 0;
          pCur->ap_ਪੰਨਾ[0] = p_ਪੰਨਾ;
          pCur->p_ਪੰਨਾ = pCur->ap_ਪੰਨਾ[1];
          ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਪੰਨਾ->n_ਉਛਾਲ );
        }
      }ਨਹੀਂਤਾਂ{
        ਰੋਕੋ;
      }
    }ਨਹੀਂਤਾਂ ਜੇ( sqlite3_Pager_ਪੰਨਾ_Refcount(p_ਪੰਨਾ->p_Db_ਪੰਨਾ)>1 ){
      /* The page being written is not a root page, and there is currently
      ** more than one reference to it. This only happens if the page is one
      ** of its own ancestor pages. Corruption. */
      rc = SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }ਨਹੀਂਤਾਂ{
      Mem_ਪੰਨਾ * ਅਚਲ p_ਮਾਪਾ = pCur->ap_ਪੰਨਾ[i_ਪੰਨਾ-1];
      ਪੂਰਨ ਅਚਲ iIdx = pCur->aiIdx[i_ਪੰਨਾ-1];

      rc = sqlite3PagerWrite(p_ਮਾਪਾ->p_Db_ਪੰਨਾ);
      ਜੇ( rc==SQLITE_OK && p_ਮਾਪਾ->nFree<0 ){
        rc = btreeComputeFreeSpace(p_ਮਾਪਾ);
      }
      ਜੇ( rc==SQLITE_OK ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_QUICKBALANCE
        ਜੇ( p_ਪੰਨਾ->int_ਕੁੰਜੀ_ਪੱਤੀ
         && p_ਪੰਨਾ->n_ਉਛਾਲ==1
         && p_ਪੰਨਾ->aiOvfl[0]==p_ਪੰਨਾ->nCell
         && p_ਮਾਪਾ->pgno!=1
         && p_ਮਾਪਾ->nCell==iIdx
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
          ਪ੍ਰਤਿਜ੍ਞਾ( balance_ਤੇਜ਼_called==0 );
          VVA_ONLY( balance_ਤੇਜ਼_called++ );
          rc = balance_ਤੇਜ਼(p_ਮਾਪਾ, p_ਪੰਨਾ, a_Balance_ਤੇਜ਼_Space);
        }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
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
          u8 *pSpace = sqlite3_ਪੰਨਾ_Malloc(pCur->pBt->ਪੰਨਾ_ਮਾਪ);
          rc = balance_nonroot(p_ਮਾਪਾ, iIdx, pSpace, i_ਪੰਨਾ==1,
                               pCur->hints&BTREE_BULKLOAD);
          ਜੇ( pFree ){
            /* If pFree is not NULL, it points to the pSpace buffer used
            ** by a previous call to balance_nonroot(). Its contents are
            ** now stored either on real database pages or within the
            ** new pSpace buffer, so it may be safely freed here. */
            sqlite3_ਪੰਨਾ_Free(pFree);
          }

          /* The pSpace buffer will be freed after the next call to
          ** balance_nonroot(), or just before this function returns, whichever
          ** comes first. */
          pFree = pSpace;
        }
      }

      p_ਪੰਨਾ->n_ਉਛਾਲ = 0;

      /* The next iteration of the do-loop balances the parent page. */
      release_ਪੰਨਾ(p_ਪੰਨਾ);
      pCur->i_ਪੰਨਾ--;
      ਪ੍ਰਤਿਜ੍ਞਾ( pCur->i_ਪੰਨਾ>=0 );
      pCur->p_ਪੰਨਾ = pCur->ap_ਪੰਨਾ[pCur->i_ਪੰਨਾ];
    }
  }ਜਦੋਂ( rc==SQLITE_OK );

  ਜੇ( pFree ){
    sqlite3_ਪੰਨਾ_Free(pFree);
  }
  ਵਾਪਸ rc;
}

/* Overwrite content from pX into pDest.  Only do the write if the
** content is different from what is already there.
*/
ਸਥਿਰ ਪੂਰਨ btreeOverwriteContent(
  Mem_ਪੰਨਾ *p_ਪੰਨਾ,           /* MemPage on which writing will occur */
  u8 *pDest,                /* Pointer to the place to start writing */
  ਅਚਲ BtreePayload *pX,   /* Source of data to write */
  ਪੂਰਨ i_ਵਿੱਥ,              /* Offset of first byte to write */
  ਪੂਰਨ iAmt                  /* Number of bytes to be written */
){
  ਪੂਰਨ n_ਅੰਕੜੇ = pX->n_ਅੰਕੜੇ - i_ਵਿੱਥ;
  ਜੇ( n_ਅੰਕੜੇ<=0 ){
    /* Overwriting with zeros */
    ਪੂਰਨ i;
    ਲਈ(i=0; i<iAmt && pDest[i]==0; i++){}
    ਜੇ( i<iAmt ){
      ਪੂਰਨ rc = sqlite3PagerWrite(p_ਪੰਨਾ->p_Db_ਪੰਨਾ);
      ਜੇ( rc ) ਵਾਪਸ rc;
      memset(pDest + i, 0, iAmt - i);
    }
  }ਨਹੀਂਤਾਂ{
    ਜੇ( n_ਅੰਕੜੇ<iAmt ){
      /* Mixed read data and zeros at the end.  Make a recursive call
      ** to write the zeros then fall through to write the real data */
      ਪੂਰਨ rc = btreeOverwriteContent(p_ਪੰਨਾ, pDest+n_ਅੰਕੜੇ, pX, i_ਵਿੱਥ+n_ਅੰਕੜੇ,
                                 iAmt-n_ਅੰਕੜੇ);
      ਜੇ( rc ) ਵਾਪਸ rc;
      iAmt = n_ਅੰਕੜੇ;
    }
    ਜੇ( memcmp(pDest, ((u8*)pX->p_ਅੰਕੜੇ) + i_ਵਿੱਥ, iAmt)!=0 ){
      ਪੂਰਨ rc = sqlite3PagerWrite(p_ਪੰਨਾ->p_Db_ਪੰਨਾ);
      ਜੇ( rc ) ਵਾਪਸ rc;
      /* In a corrupt database, it is possible for the source and destination
      ** buffers to overlap.  This is harmless since the database is already
      ** corrupt but it does cause valgrind and ASAN warnings.  So use
      ** memmove(). */
      memmove(pDest, ((u8*)pX->p_ਅੰਕੜੇ) + i_ਵਿੱਥ, iAmt);
    }
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.  In this variant, pCur is pointing to an overflow
** cell.
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ btree_Overwrite_ਉਛਾਲ_Cell(
  BtCursor *pCur,                     /* Cursor pointing to cell to overwrite */
  ਅਚਲ BtreePayload *pX              /* Content to write into the cell */
){
  ਪੂਰਨ i_ਵਿੱਥ;                        /* Next byte of pX->pData to write */
  ਪੂਰਨ n_ਕੁੱਲ = pX->n_ਅੰਕੜੇ + pX->nZero; /* Total bytes of to write */
  ਪੂਰਨ rc;                             /* Return code */
  Mem_ਪੰਨਾ *p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;       /* Page being written */
  BtShared *pBt;                      /* Btree */
  Pgno ovflPgno;                      /* Next overflow page to write */
  u32 ovfl_ਪੰਨਾ_ਮਾਪ;                   /* Size to write on overflow page */

  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->info.nLocal<n_ਕੁੱਲ );  /* pCur is an overflow cell */

  /* Overwrite the local portion first */
  rc = btreeOverwriteContent(p_ਪੰਨਾ, pCur->info.pPayload, pX,
                             0, pCur->info.nLocal);
  ਜੇ( rc ) ਵਾਪਸ rc;

  /* Now overwrite the overflow pages */
  i_ਵਿੱਥ = pCur->info.nLocal;
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕੁੱਲ>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਵਿੱਥ>=0 );
  ovflPgno = get4byte(pCur->info.pPayload + i_ਵਿੱਥ);
  pBt = p_ਪੰਨਾ->pBt;
  ovfl_ਪੰਨਾ_ਮਾਪ = pBt->usable_ਮਾਪ - 4;
  ਕਰੋ{
    rc = btree_Get_ਪੰਨਾ(pBt, ovflPgno, &p_ਪੰਨਾ, 0);
    ਜੇ( rc ) ਵਾਪਸ rc;
    ਜੇ( sqlite3_Pager_ਪੰਨਾ_Refcount(p_ਪੰਨਾ->p_Db_ਪੰਨਾ)!=1 || p_ਪੰਨਾ->isInit ){
      rc = SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }ਨਹੀਂਤਾਂ{
      ਜੇ( i_ਵਿੱਥ+ovfl_ਪੰਨਾ_ਮਾਪ<(u32)n_ਕੁੱਲ ){
        ovflPgno = get4byte(p_ਪੰਨਾ->a_ਅੰਕੜੇ);
      }ਨਹੀਂਤਾਂ{
        ovfl_ਪੰਨਾ_ਮਾਪ = n_ਕੁੱਲ - i_ਵਿੱਥ;
      }
      rc = btreeOverwriteContent(p_ਪੰਨਾ, p_ਪੰਨਾ->a_ਅੰਕੜੇ+4, pX,
                                 i_ਵਿੱਥ, ovfl_ਪੰਨਾ_ਮਾਪ);
    }
    sqlite3PagerUnref(p_ਪੰਨਾ->p_Db_ਪੰਨਾ);
    ਜੇ( rc ) ਵਾਪਸ rc;
    i_ਵਿੱਥ += ovfl_ਪੰਨਾ_ਮਾਪ;
  }ਜਦੋਂ( i_ਵਿੱਥ<n_ਕੁੱਲ );
  ਵਾਪਸ SQLITE_OK;   
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.
*/
ਸਥਿਰ ਪੂਰਨ btreeOverwriteCell(BtCursor *pCur, ਅਚਲ BtreePayload *pX){
  ਪੂਰਨ n_ਕੁੱਲ = pX->n_ਅੰਕੜੇ + pX->nZero; /* Total bytes of to write */
  Mem_ਪੰਨਾ *p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;       /* Page being written */

  ਜੇ( pCur->info.pPayload + pCur->info.nLocal > p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ
   || pCur->info.pPayload < p_ਪੰਨਾ->a_ਅੰਕੜੇ + p_ਪੰਨਾ->cell_ਵਿੱਥ
  ){
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
  }
  ਜੇ( pCur->info.nLocal==n_ਕੁੱਲ ){
    /* The entire cell is local */
    ਵਾਪਸ btreeOverwriteContent(p_ਪੰਨਾ, pCur->info.pPayload, pX,
                                 0, pCur->info.nLocal);
  }ਨਹੀਂਤਾਂ{
    /* The cell contains overflow content */
    ਵਾਪਸ btree_Overwrite_ਉਛਾਲ_Cell(pCur, pX);
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
ਪੂਰਨ sqlite3_Btree_ਪਾਓ(
  BtCursor *pCur,                /* Insert data into the table of this cursor */
  ਅਚਲ BtreePayload *pX,        /* Content of the row to be inserted */
  ਪੂਰਨ flags,                     /* True if this is likely an append */
  ਪੂਰਨ seek_ਨਤੀਜਾ                 /* Result of prior IndexMoveto() call */
){
  ਪੂਰਨ rc;
  ਪੂਰਨ loc = seek_ਨਤੀਜਾ;          /* -1: before desired location  +1: after */
  ਪੂਰਨ szNew = 0;
  ਪੂਰਨ idx;
  Mem_ਪੰਨਾ *p_ਪੰਨਾ;
  Btree *p = pCur->pBtree;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *oldCell;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *newCell = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( (flags & (BTREE_SAVEPOSITION|BTREE_APPEND|BTREE_PREFORMAT))==flags );
  ਪ੍ਰਤਿਜ੍ਞਾ( (flags & BTREE_PREFORMAT)==0 || seek_ਨਤੀਜਾ || pCur->p_ਕੁੰਜੀ_Info==0 );

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
  ਜੇ( pCur->curFlags & BTCF_ਗੁਣਜ ){
    rc = saveAllCursors(p->pBt, pCur->pgno_ਜੜ੍ਹ, pCur);
    ਜੇ( rc ) ਵਾਪਸ rc;
    ਜੇ( loc && pCur->i_ਪੰਨਾ<0 ){
      /* This can only happen if the schema is corrupt such that there is more
      ** than one table or index with the same root page as used by the cursor.
      ** Which can only happen if the SQLITE_NoSchemaError flag was set when
      ** the schema was loaded. This cannot be asserted though, as a user might
      ** set the flag, load the schema, and then unset the flag.  */
      ਵਾਪਸ SQLITE_CORRUPT_PGNO(pCur->pgno_ਜੜ੍ਹ);
    }
  }

  /* Ensure that the cursor is not in the CURSOR_FAULT state and that it
  ** points to a valid cell.
  */
  ਜੇ( pCur->eState>=CURSOR_REQUIRESEEK ){
    testcase( pCur->eState==CURSOR_REQUIRESEEK );
    testcase( pCur->eState==CURSOR_FAULT );
    rc = move_ਮੰਜ਼ਿਲ_ਜੜ੍ਹ(pCur);
    ਜੇ( rc && rc!=SQLITE_ਖਾਲੀ_ਥਾਵਾਂ ) ਵਾਪਸ rc;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pCur->curFlags & BTCF_WriteFlag)!=0
              && p->pBt->ਵਿੱਚ_Transaction==TRANS_WRITE
              && (p->pBt->btsFlags & BTS_READ_ONLY)==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( has_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(p, pCur->pgno_ਜੜ੍ਹ, pCur->p_ਕੁੰਜੀ_Info!=0, 2) );

  /* Assert that the caller has been consistent. If this cursor was opened
  ** expecting an index b-tree, then the caller should be inserting blob
  ** keys with no associated data. If the cursor was opened expecting an
  ** intkey table, the caller should be inserting integer keys with a
  ** blob of associated data.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( (flags & BTREE_PREFORMAT) || (pX->p_ਕੁੰਜੀ==0)==(pCur->p_ਕੁੰਜੀ_Info==0) );

  ਜੇ( pCur->p_ਕੁੰਜੀ_Info==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pX->p_ਕੁੰਜੀ==0 );
    /* If this is an insert into a table b-tree, invalidate any incrblob
    ** cursors open on the row being replaced */
    ਜੇ( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, pCur->pgno_ਜੜ੍ਹ, pX->n_ਕੁੰਜੀ, 0);
    }

    /* If BTREE_SAVEPOSITION is set, the cursor must already be pointing
    ** to a row with the same key as the new entry being inserted.
    */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    ਜੇ( flags & BTREE_SAVEPOSITION ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pCur->curFlags & BTCF_ValidNKey );
      ਪ੍ਰਤਿਜ੍ਞਾ( pX->n_ਕੁੰਜੀ==pCur->info.n_ਕੁੰਜੀ );
      ਪ੍ਰਤਿਜ੍ਞਾ( loc==0 );
    }
#ਜੇ_ਅੰਤ

    /* On the other hand, BTREE_SAVEPOSITION==0 does not imply
    ** that the cursor is not pointing to a row to be overwritten.
    ** So do a complete check.
    */
    ਜੇ( (pCur->curFlags&BTCF_ValidNKey)!=0 && pX->n_ਕੁੰਜੀ==pCur->info.n_ਕੁੰਜੀ ){
      /* The cursor is pointing to the entry that is to be
      ** overwritten */
      ਪ੍ਰਤਿਜ੍ਞਾ( pX->n_ਅੰਕੜੇ>=0 && pX->nZero>=0 );
      ਜੇ( pCur->info.n_ਮਾਪ!=0
       && pCur->info.nPayload==(u32)pX->n_ਅੰਕੜੇ+pX->nZero
      ){
        /* New entry is the same size as the old.  Do an overwrite */
        ਵਾਪਸ btreeOverwriteCell(pCur, pX);
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( loc==0 );
    }ਨਹੀਂਤਾਂ ਜੇ( loc==0 ){
      /* The cursor is *not* pointing to the cell to be overwritten, nor
      ** to an adjacent cell.  Move the cursor so that it is pointing either
      ** to the cell to be overwritten or an adjacent cell.
      */
      rc = sqlite3_Btree_ਸਾਰਣੀ_Moveto(pCur, pX->n_ਕੁੰਜੀ,
               (flags & BTREE_APPEND)!=0, &loc);
      ਜੇ( rc ) ਵਾਪਸ rc;
    }
  }ਨਹੀਂਤਾਂ{
    /* This is an index or a WITHOUT ROWID table */

    /* If BTREE_SAVEPOSITION is set, the cursor must already be pointing
    ** to a row with the same key as the new entry being inserted.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( (flags & BTREE_SAVEPOSITION)==0 || loc==0 );

    /* If the cursor is not already pointing either to the cell to be
    ** overwritten, or if a new cell is being inserted, if the cursor is
    ** not pointing to an immediately adjacent cell, then move the cursor
    ** so that it does.
    */
    ਜੇ( loc==0 && (flags & BTREE_SAVEPOSITION)==0 ){
      ਜੇ( pX->nMem ){
        Unpacked_ਰਿਕਾਰਡ r;
        r.p_ਕੁੰਜੀ_Info = pCur->p_ਕੁੰਜੀ_Info;
        r.aMem = pX->aMem;
        r.n_ਖੇਤਰ = pX->nMem;
        r.default_rc = 0;
        r.eq_ਵੇਖਿਆ = 0;
        rc = sqlite3_Btree_ਸੂਚਕਾਂਕ_Moveto(pCur, &r, &loc);
      }ਨਹੀਂਤਾਂ{
        rc = btreeMoveto(pCur, pX->p_ਕੁੰਜੀ, pX->n_ਕੁੰਜੀ,
                    (flags & BTREE_APPEND)!=0, &loc);
      }
      ਜੇ( rc ) ਵਾਪਸ rc;
    }

    /* If the cursor is currently pointing to an entry to be overwritten
    ** and the new content is the same as as the old, then use the
    ** overwrite optimization.
    */
    ਜੇ( loc==0 ){
      getCellInfo(pCur);
      ਜੇ( pCur->info.n_ਕੁੰਜੀ==pX->n_ਕੁੰਜੀ ){
        BtreePayload x2;
        x2.p_ਅੰਕੜੇ = pX->p_ਕੁੰਜੀ;
        x2.n_ਅੰਕੜੇ = (ਪੂਰਨ)pX->n_ਕੁੰਜੀ;  ਪ੍ਰਤਿਜ੍ਞਾ( pX->n_ਕੁੰਜੀ<=0x7fffffff );
        x2.nZero = 0;
        ਵਾਪਸ btreeOverwriteCell(pCur, &x2);
      }
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID
       || (pCur->eState==CURSOR_INVALID && loc) || CORRUPT_DB );

  p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->int_ਕੁੰਜੀ || pX->n_ਕੁੰਜੀ>=0 || (flags & BTREE_PREFORMAT) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->ਪੱਤੀ || !p_ਪੰਨਾ->int_ਕੁੰਜੀ );
  ਜੇ( p_ਪੰਨਾ->nFree<0 ){
    ਜੇ( NEVER(pCur->eState>CURSOR_INVALID) ){
     /* ^^^^^--- due to the moveToRoot() call above */
      rc = SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }ਨਹੀਂਤਾਂ{
      rc = btreeComputeFreeSpace(p_ਪੰਨਾ);
    }
    ਜੇ( rc ) ਵਾਪਸ rc;
  }

  TRACE(("INSERT: table=%u nkey=%lld ndata=%u page=%u %s\n",
          pCur->pgno_ਜੜ੍ਹ, pX->n_ਕੁੰਜੀ, pX->n_ਅੰਕੜੇ, p_ਪੰਨਾ->pgno,
          loc==0 ? "overwrite" : "new entry"));
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->isInit || CORRUPT_DB );
  newCell = p->pBt->pTmpSpace;
  ਪ੍ਰਤਿਜ੍ਞਾ( newCell!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( BTREE_PREFORMAT==OPFLAG_PREFORMAT );
  ਜੇ( flags & BTREE_PREFORMAT ){
    rc = SQLITE_OK;
    szNew = p->pBt->n_Preformat_ਮਾਪ;
    ਜੇ( szNew<4 ){
      szNew = 4;
      newCell[3] = 0;
    }
    ਜੇ( ISAUTOVACUUM(p->pBt) && szNew>p_ਪੰਨਾ->maxLocal ){
      CellInfo info;
      p_ਪੰਨਾ->xParseCell(p_ਪੰਨਾ, newCell, &info);
      ਜੇ( info.nPayload!=info.nLocal ){
        Pgno ovfl = get4byte(&newCell[szNew-4]);
        ptrmapPut(p->pBt, ovfl, PTRMAP_OVERFLOW1, p_ਪੰਨਾ->pgno, &rc);
        ਜੇ( NEVER(rc) ) ਜਾਓ ਸਿਰਾ_ਪਾਓ;
      }
    }
  }ਨਹੀਂਤਾਂ{
    rc = fill_ਵਿੱਚ_Cell(p_ਪੰਨਾ, newCell, pX, &szNew);
    ਜੇ( rc ) ਜਾਓ ਸਿਰਾ_ਪਾਓ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( szNew==p_ਪੰਨਾ->x_Cell_ਮਾਪ(p_ਪੰਨਾ, newCell) );
  ਪ੍ਰਤਿਜ੍ਞਾ( szNew <= MX_CELL_ਮਾਪ(p->pBt) );
  idx = pCur->ix;
  pCur->info.n_ਮਾਪ = 0;
  ਜੇ( loc==0 ){
    CellInfo info;
    ਪ੍ਰਤਿਜ੍ਞਾ( idx>=0 );
    ਜੇ( idx>=p_ਪੰਨਾ->nCell ){
      ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    }
    rc = sqlite3PagerWrite(p_ਪੰਨਾ->p_Db_ਪੰਨਾ);
    ਜੇ( rc ){
      ਜਾਓ ਸਿਰਾ_ਪਾਓ;
    }
    oldCell = ਲੱਭੋ_Cell(p_ਪੰਨਾ, idx);
    ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(newCell, oldCell, 4);
    }
    BTREE_CLEAR_CELL(rc, p_ਪੰਨਾ, oldCell, info);
    testcase( pCur->curFlags & BTCF_ValidOvfl );
    invalidate_ਉਛਾਲ_Cache(pCur);
    ਜੇ( info.n_ਮਾਪ==szNew && info.nLocal==info.nPayload
     && (!ISAUTOVACUUM(p->pBt) || szNew<p_ਪੰਨਾ->minLocal)
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
      ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK ); /* clearCell never fails when nLocal==nPayload */
      ਜੇ( oldCell < p_ਪੰਨਾ->a_ਅੰਕੜੇ+p_ਪੰਨਾ->hdr_ਵਿੱਥ+10 ){
        ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
      }
      ਜੇ( oldCell+szNew > p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ ){
        ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
      }
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(oldCell, newCell, szNew);
      ਵਾਪਸ SQLITE_OK;
    }
    dropCell(p_ਪੰਨਾ, idx, info.n_ਮਾਪ, &rc);
    ਜੇ( rc ) ਜਾਓ ਸਿਰਾ_ਪਾਓ;
  }ਨਹੀਂਤਾਂ ਜੇ( loc<0 && p_ਪੰਨਾ->nCell>0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->ਪੱਤੀ );
    idx = ++pCur->ix;
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->ਪੱਤੀ );
  }
  rc = ਪਾਓ_Cell_Fast(p_ਪੰਨਾ, idx, newCell, szNew);
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->n_ਉਛਾਲ==0 || rc==SQLITE_OK );
  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || p_ਪੰਨਾ->nCell>0 || p_ਪੰਨਾ->n_ਉਛਾਲ>0 );

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
  ਜੇ( p_ਪੰਨਾ->n_ਉਛਾਲ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK );
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
    rc = balance(pCur);

    /* Must make sure nOverflow is reset to zero even if the balance()
    ** fails. Internal data structure corruption will result otherwise.
    ** Also, set the cursor state to invalid. This stops saveCursorPosition()
    ** from trying to save the current position of the cursor.  */
    pCur->p_ਪੰਨਾ->n_ਉਛਾਲ = 0;
    pCur->eState = CURSOR_INVALID;
    ਜੇ( (flags & BTREE_SAVEPOSITION) && rc==SQLITE_OK ){
      btree_Release_All_Cursor_ਪੰਨੇ(pCur);
      ਜੇ( pCur->p_ਕੁੰਜੀ_Info ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਕੁੰਜੀ==0 );
        pCur->p_ਕੁੰਜੀ = sqlite3Malloc( pX->n_ਕੁੰਜੀ );
        ਜੇ( pCur->p_ਕੁੰਜੀ==0 ){
          rc = SQLITE_NOMEM;
        }ਨਹੀਂਤਾਂ{
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(pCur->p_ਕੁੰਜੀ, pX->p_ਕੁੰਜੀ, pX->n_ਕੁੰਜੀ);
        }
      }
      pCur->eState = CURSOR_REQUIRESEEK;
      pCur->n_ਕੁੰਜੀ = pX->n_ਕੁੰਜੀ;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->i_ਪੰਨਾ<0 || pCur->p_ਪੰਨਾ->n_ਉਛਾਲ==0 );

ਸਿਰਾ_ਪਾਓ:
  ਵਾਪਸ rc;
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
ਪੂਰਨ sqlite3_Btree_Transfer_ਪੰਕਤੀ(BtCursor *pDest, BtCursor *pSrc, i64 i_ਕੁੰਜੀ){
  BtShared *pBt = pDest->pBt;
  u8 *a_ਬਾਹਰਲਾ = pBt->pTmpSpace;    /* Pointer to next output buffer */
  ਅਚਲ u8 *a_ਵਿੱਚ;                /* Pointer to next input buffer */
  u32 n_ਵਿੱਚ;                      /* Size of input buffer aIn[] */
  u32 nRem;                     /* Bytes of data still to copy */

  getCellInfo(pSrc);
  ਜੇ( pSrc->info.nPayload<0x80 ){
    *(a_ਬਾਹਰਲਾ++) = (u8)pSrc->info.nPayload;
  }ਨਹੀਂਤਾਂ{
    a_ਬਾਹਰਲਾ += sqlite3PutVarint(a_ਬਾਹਰਲਾ, pSrc->info.nPayload);
  }
  ਜੇ( pDest->p_ਕੁੰਜੀ_Info==0 ) a_ਬਾਹਰਲਾ += sqlite3PutVarint(a_ਬਾਹਰਲਾ, i_ਕੁੰਜੀ);
  n_ਵਿੱਚ = pSrc->info.nLocal;
  a_ਵਿੱਚ = pSrc->info.pPayload;
  ਜੇ( a_ਵਿੱਚ+n_ਵਿੱਚ>pSrc->p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ ){
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(pSrc->p_ਪੰਨਾ);
  }
  nRem = pSrc->info.nPayload;
  ਜੇ( n_ਵਿੱਚ==nRem && n_ਵਿੱਚ<pDest->p_ਪੰਨਾ->maxLocal ){
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(a_ਬਾਹਰਲਾ, a_ਵਿੱਚ, n_ਵਿੱਚ);
    pBt->n_Preformat_ਮਾਪ = n_ਵਿੱਚ + (ਪੂਰਨ)(a_ਬਾਹਰਲਾ - pBt->pTmpSpace);
    ਵਾਪਸ SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ rc = SQLITE_OK;
    Pager *pSrcPager = pSrc->pBt->pPager;
    u8 *p_Pgno_ਬਾਹਰਲਾ = 0;
    Pgno ovfl_ਵਿੱਚ = 0;
    Db_ਪੰਨਾ *p_ਪੰਨਾ_ਵਿੱਚ = 0;
    Mem_ਪੰਨਾ *p_ਪੰਨਾ_ਬਾਹਰਲਾ = 0;
    u32 n_ਬਾਹਰਲਾ;                     /* Size of output buffer aOut[] */

    n_ਬਾਹਰਲਾ = btree_Payload_ਮੰਜ਼ਿਲ_Local(pDest->p_ਪੰਨਾ, pSrc->info.nPayload);
    pBt->n_Preformat_ਮਾਪ = (ਪੂਰਨ)n_ਬਾਹਰਲਾ + (ਪੂਰਨ)(a_ਬਾਹਰਲਾ - pBt->pTmpSpace);
    ਜੇ( n_ਬਾਹਰਲਾ<pSrc->info.nPayload ){
      p_Pgno_ਬਾਹਰਲਾ = &a_ਬਾਹਰਲਾ[n_ਬਾਹਰਲਾ];
      pBt->n_Preformat_ਮਾਪ += 4;
    }
 
    ਜੇ( nRem>n_ਵਿੱਚ ){
      ਜੇ( a_ਵਿੱਚ+n_ਵਿੱਚ+4>pSrc->p_ਪੰਨਾ->a_ਅੰਕੜੇ_ਸਿਰਾ ){
        ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(pSrc->p_ਪੰਨਾ);
      }
      ovfl_ਵਿੱਚ = get4byte(&pSrc->info.pPayload[n_ਵਿੱਚ]);
    }
 
    ਕਰੋ {
      nRem -= n_ਬਾਹਰਲਾ;
      ਕਰੋ{
        ਪ੍ਰਤਿਜ੍ਞਾ( n_ਬਾਹਰਲਾ>0 );
        ਜੇ( n_ਵਿੱਚ>0 ){
          ਪੂਰਨ nCopy = MIN(n_ਬਾਹਰਲਾ, n_ਵਿੱਚ);
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(a_ਬਾਹਰਲਾ, a_ਵਿੱਚ, nCopy);
          n_ਬਾਹਰਲਾ -= nCopy;
          n_ਵਿੱਚ -= nCopy;
          a_ਬਾਹਰਲਾ += nCopy;
          a_ਵਿੱਚ += nCopy;
        }
        ਜੇ( n_ਬਾਹਰਲਾ>0 ){
          sqlite3PagerUnref(p_ਪੰਨਾ_ਵਿੱਚ);
          p_ਪੰਨਾ_ਵਿੱਚ = 0;
          rc = sqlite3PagerGet(pSrcPager, ovfl_ਵਿੱਚ, &p_ਪੰਨਾ_ਵਿੱਚ, PAGER_GET_READONLY);
          ਜੇ( rc==SQLITE_OK ){
            a_ਵਿੱਚ = (ਅਚਲ u8*)sqlite3_Pager_Get_ਅੰਕੜੇ(p_ਪੰਨਾ_ਵਿੱਚ);
            ovfl_ਵਿੱਚ = get4byte(a_ਵਿੱਚ);
            a_ਵਿੱਚ += 4;
            n_ਵਿੱਚ = pSrc->pBt->usable_ਮਾਪ - 4;
          }
        }
      }ਜਦੋਂ( rc==SQLITE_OK && n_ਬਾਹਰਲਾ>0 );
 
      ਜੇ( rc==SQLITE_OK && nRem>0 && ALWAYS(p_Pgno_ਬਾਹਰਲਾ) ){
        Pgno pgnoNew = 0;  /* Prevent harmless static-analyzer warning */
        Mem_ਪੰਨਾ *pNew = 0;
        rc = allocate_Btree_ਪੰਨਾ(pBt, &pNew, &pgnoNew, 0, 0);
        put4byte(p_Pgno_ਬਾਹਰਲਾ, pgnoNew);
        ਜੇ( ISAUTOVACUUM(pBt) && p_ਪੰਨਾ_ਬਾਹਰਲਾ ){
          ptrmapPut(pBt, pgnoNew, PTRMAP_OVERFLOW2, p_ਪੰਨਾ_ਬਾਹਰਲਾ->pgno, &rc);
        }
        release_ਪੰਨਾ(p_ਪੰਨਾ_ਬਾਹਰਲਾ);
        p_ਪੰਨਾ_ਬਾਹਰਲਾ = pNew;
        ਜੇ( p_ਪੰਨਾ_ਬਾਹਰਲਾ ){
          p_Pgno_ਬਾਹਰਲਾ = p_ਪੰਨਾ_ਬਾਹਰਲਾ->a_ਅੰਕੜੇ;
          put4byte(p_Pgno_ਬਾਹਰਲਾ, 0);
          a_ਬਾਹਰਲਾ = &p_Pgno_ਬਾਹਰਲਾ[4];
          n_ਬਾਹਰਲਾ = MIN(pBt->usable_ਮਾਪ - 4, nRem);
        }
      }
    }ਜਦੋਂ( nRem>0 && rc==SQLITE_OK );
 
    release_ਪੰਨਾ(p_ਪੰਨਾ_ਬਾਹਰਲਾ);
    sqlite3PagerUnref(p_ਪੰਨਾ_ਵਿੱਚ);
    ਵਾਪਸ rc;
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
ਪੂਰਨ sqlite3BtreeDelete(BtCursor *pCur, u8 flags){
  Btree *p = pCur->pBtree;
  BtShared *pBt = p->pBt;             
  ਪੂਰਨ rc;                    /* Return code */
  Mem_ਪੰਨਾ *p_ਪੰਨਾ;            /* Page to delete cell from */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pCell;      /* Pointer to cell to delete */
  ਪੂਰਨ iCellIdx;              /* Index of cell to delete */
  ਪੂਰਨ i_Cell_ਡੂੰਘਾਈ;            /* Depth of node containing pCell */
  CellInfo info;             /* Size of the cell being deleted */
  u8 bPreserve;              /* Keep cursor valid.  2 for CURSOR_SKIPNEXT */

  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCur) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->ਵਿੱਚ_Transaction==TRANS_WRITE );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->curFlags & BTCF_WriteFlag );
  ਪ੍ਰਤਿਜ੍ਞਾ( has_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(p, pCur->pgno_ਜੜ੍ਹ, pCur->p_ਕੁੰਜੀ_Info!=0, 2) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !hasReadConflicts(p, pCur->pgno_ਜੜ੍ਹ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( (flags & ~(BTREE_SAVEPOSITION | BTREE_AUXDELETE))==0 );
  ਜੇ( pCur->eState!=CURSOR_VALID ){
    ਜੇ( pCur->eState>=CURSOR_REQUIRESEEK ){
      rc = btreeRestoreCursorPosition(pCur);
      ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || CORRUPT_DB || pCur->eState==CURSOR_VALID );
      ਜੇ( rc || pCur->eState!=CURSOR_VALID ) ਵਾਪਸ rc;
    }ਨਹੀਂਤਾਂ{
      ਵਾਪਸ SQLITE_CORRUPT_PGNO(pCur->pgno_ਜੜ੍ਹ);
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->eState==CURSOR_VALID );

  i_Cell_ਡੂੰਘਾਈ = pCur->i_ਪੰਨਾ;
  iCellIdx = pCur->ix;
  p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;
  ਜੇ( p_ਪੰਨਾ->nCell<=iCellIdx ){
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
  }
  pCell = ਲੱਭੋ_Cell(p_ਪੰਨਾ, iCellIdx);
  ਜੇ( p_ਪੰਨਾ->nFree<0 && btreeComputeFreeSpace(p_ਪੰਨਾ) ){
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
  }
  ਜੇ( pCell<&p_ਪੰਨਾ->aCellIdx[p_ਪੰਨਾ->nCell] ){
    ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
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
  ਜੇ( bPreserve ){
    ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ
     || (p_ਪੰਨਾ->nFree+p_ਪੰਨਾ->x_Cell_ਮਾਪ(p_ਪੰਨਾ,pCell)+2) >
                                                   (ਪੂਰਨ)(pBt->usable_ਮਾਪ*2/3)
     || p_ਪੰਨਾ->nCell==1  /* See dbfuzz001.test for a test case */
    ){
      /* A b-tree rebalance will be required after deleting this entry.
      ** Save the cursor key.  */
      rc = save_Cursor_ਕੁੰਜੀ(pCur);
      ਜੇ( rc ) ਵਾਪਸ rc;
    }ਨਹੀਂਤਾਂ{
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
  ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
    rc = sqlite3_Btree_ਪਿਛਲਾ(pCur, 0);
    ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_ਪੂਰਾ );
    ਜੇ( rc ) ਵਾਪਸ rc;
  }

  /* Save the positions of any other cursors open on this table before
  ** making any modifications.  */
  ਜੇ( pCur->curFlags & BTCF_ਗੁਣਜ ){
    rc = saveAllCursors(pBt, pCur->pgno_ਜੜ੍ਹ, pCur);
    ਜੇ( rc ) ਵਾਪਸ rc;
  }

  /* If this is a delete operation to remove a row from a table b-tree,
  ** invalidate any incrblob cursors open on the row being deleted.  */
  ਜੇ( pCur->p_ਕੁੰਜੀ_Info==0 && p->hasIncrblobCur ){
    invalidateIncrblobCursors(p, pCur->pgno_ਜੜ੍ਹ, pCur->info.n_ਕੁੰਜੀ, 0);
  }

  /* Make the page containing the entry to be deleted writable. Then free any
  ** overflow pages associated with the entry and finally remove the cell
  ** itself from within the page.  */
  rc = sqlite3PagerWrite(p_ਪੰਨਾ->p_Db_ਪੰਨਾ);
  ਜੇ( rc ) ਵਾਪਸ rc;
  BTREE_CLEAR_CELL(rc, p_ਪੰਨਾ, pCell, info);
  dropCell(p_ਪੰਨਾ, iCellIdx, info.n_ਮਾਪ, &rc);
  ਜੇ( rc ) ਵਾਪਸ rc;

  /* If the cell deleted was not located on a leaf page, then the cursor
  ** is currently pointing to the largest entry in the sub-tree headed
  ** by the child-page of the cell that was just deleted from an internal
  ** node. The cell from the leaf node needs to be moved to the internal
  ** node to replace the deleted cell.  */
  ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
    Mem_ਪੰਨਾ *p_ਪੱਤੀ = pCur->p_ਪੰਨਾ;
    ਪੂਰਨ nCell;
    Pgno n;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pTmp;

    ਜੇ( p_ਪੱਤੀ->nFree<0 ){
      rc = btreeComputeFreeSpace(p_ਪੱਤੀ);
      ਜੇ( rc ) ਵਾਪਸ rc;
    }
    ਜੇ( i_Cell_ਡੂੰਘਾਈ<pCur->i_ਪੰਨਾ-1 ){
      n = pCur->ap_ਪੰਨਾ[i_Cell_ਡੂੰਘਾਈ+1]->pgno;
    }ਨਹੀਂਤਾਂ{
      n = pCur->p_ਪੰਨਾ->pgno;
    }
    pCell = ਲੱਭੋ_Cell(p_ਪੱਤੀ, p_ਪੱਤੀ->nCell-1);
    ਜੇ( pCell<&p_ਪੱਤੀ->a_ਅੰਕੜੇ[4] ) ਵਾਪਸ SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੱਤੀ);
    nCell = p_ਪੱਤੀ->x_Cell_ਮਾਪ(p_ਪੱਤੀ, pCell);
    ਪ੍ਰਤਿਜ੍ਞਾ( MX_CELL_ਮਾਪ(pBt) >= nCell );
    pTmp = pBt->pTmpSpace;
    ਪ੍ਰਤਿਜ੍ਞਾ( pTmp!=0 );
    rc = sqlite3PagerWrite(p_ਪੱਤੀ->p_Db_ਪੰਨਾ);
    ਜੇ( rc==SQLITE_OK ){
      rc = ਪਾਓ_Cell(p_ਪੰਨਾ, iCellIdx, pCell-4, nCell+4-(pCell[0]==2), pTmp, n);
    }
    dropCell(p_ਪੱਤੀ, p_ਪੱਤੀ->nCell-1, nCell, &rc);
    ਜੇ( rc ) ਵਾਪਸ rc;
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
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਪੰਨਾ->n_ਉਛਾਲ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCur->p_ਪੰਨਾ->nFree>=0 );
  ਜੇ( pCur->p_ਪੰਨਾ->nFree*3<=(ਪੂਰਨ)pCur->pBt->usable_ਮਾਪ*2 ){
    /* Optimization: If the free space is less than 2/3rds of the page,
    ** then balance() will always be a no-op.  No need to invoke it. */
    rc = SQLITE_OK;
  }ਨਹੀਂਤਾਂ{
    rc = balance(pCur);
  }
  ਜੇ( rc==SQLITE_OK && pCur->i_ਪੰਨਾ>i_Cell_ਡੂੰਘਾਈ ){
    release_ਪੰਨਾ_Not_Null(pCur->p_ਪੰਨਾ);
    pCur->i_ਪੰਨਾ--;
    ਜਦੋਂ( pCur->i_ਪੰਨਾ>i_Cell_ਡੂੰਘਾਈ ){
      release_ਪੰਨਾ(pCur->ap_ਪੰਨਾ[pCur->i_ਪੰਨਾ--]);
    }
    pCur->p_ਪੰਨਾ = pCur->ap_ਪੰਨਾ[pCur->i_ਪੰਨਾ];
    rc = balance(pCur);
  }

  ਜੇ( rc==SQLITE_OK ){
    ਜੇ( bPreserve>1 ){
      ਪ੍ਰਤਿਜ੍ਞਾ( (pCur->i_ਪੰਨਾ==i_Cell_ਡੂੰਘਾਈ || CORRUPT_DB) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ==pCur->p_ਪੰਨਾ || CORRUPT_DB );
      ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਪੰਨਾ->nCell>0 || CORRUPT_DB) && iCellIdx<=p_ਪੰਨਾ->nCell );
      pCur->eState = CURSOR_SKIPNEXT;
      ਜੇ( iCellIdx>=p_ਪੰਨਾ->nCell ){
        pCur->skip_ਅਗਲਾ = -1;
        pCur->ix = p_ਪੰਨਾ->nCell-1;
      }ਨਹੀਂਤਾਂ{
        pCur->skip_ਅਗਲਾ = 1;
      }
      /* The pCur->info structure is currently populated with the details
      ** of the cell that was just removed. But pCur->ix now points to a
      ** different cell in the b-tree. So zero info.nSize here so that if
      ** saveCursorPosition() is called on this cursor it does not trust
      ** the contents of pCur->info.  */
      pCur->info.n_ਮਾਪ = 0;
    }ਨਹੀਂਤਾਂ{
      rc = move_ਮੰਜ਼ਿਲ_ਜੜ੍ਹ(pCur);
      ਜੇ( bPreserve ){
        btree_Release_All_Cursor_ਪੰਨੇ(pCur);
        pCur->eState = CURSOR_REQUIRESEEK;
      }
      ਜੇ( rc==SQLITE_ਖਾਲੀ_ਥਾਵਾਂ ) rc = SQLITE_OK;
    }
  }
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ btree_Create_ਸਾਰਣੀ(Btree *p, Pgno *pi_ਸਾਰਣੀ, ਪੂਰਨ createTabFlags){
  BtShared *pBt = p->pBt;
  Mem_ਪੰਨਾ *p_ਜੜ੍ਹ;
  Pgno pgno_ਜੜ੍ਹ;
  ਪੂਰਨ rc;
  ਪੂਰਨ ptfFlags;          /* Page-type flags for the root page of new table */

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsMutex(p) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->ਵਿੱਚ_Transaction==TRANS_WRITE );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pBt->btsFlags & BTS_READ_ONLY)==0 );

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
  rc = allocate_Btree_ਪੰਨਾ(pBt, &p_ਜੜ੍ਹ, &pgno_ਜੜ੍ਹ, 1, 0);
  ਜੇ( rc ){
    ਵਾਪਸ rc;
  }
#ਨਹੀਂਤਾਂ
  ਜੇ( pBt->autoVacuum ){
    Pgno pgnoMove;      /* Move a page here to make room for the root-page */
    Mem_ਪੰਨਾ *p_ਪੰਨਾ_Move; /* The page to move to. */

    /* Creating a new table may probably require moving an existing database
    ** to make room for the new tables root page. In case this page turns
    ** out to be an overflow page, delete all overflow page-map caches
    ** held by open cursors.
    */
    invalidate_All_ਉਛਾਲ_Cache(pBt);

    /* Read the value of meta[3] from the database to determine where the
    ** root page of the new table should go. meta[3] is the largest root-page
    ** created so far, so the new root-page is (meta[3]+1).
    */
    sqlite3BtreeGetMeta(p, BTREE_ਸਭ_ਤੋਂ_ਵੱਡਾ_ਜੜ੍ਹ_ਪੰਨਾ, &pgno_ਜੜ੍ਹ);
    ਜੇ( pgno_ਜੜ੍ਹ>btreePagecount(pBt) ){
      ਵਾਪਸ SQLITE_CORRUPT_PGNO(pgno_ਜੜ੍ਹ);
    }
    pgno_ਜੜ੍ਹ++;

    /* The new root-page may not be allocated on a pointer-map page, or the
    ** PENDING_BYTE page.
    */
    ਜਦੋਂ( pgno_ਜੜ੍ਹ==PTRMAP_PAGENO(pBt, pgno_ਜੜ੍ਹ) ||
        pgno_ਜੜ੍ਹ==PENDING_BYTE_ਪੰਨਾ(pBt) ){
      pgno_ਜੜ੍ਹ++;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pgno_ਜੜ੍ਹ>=3 );

    /* Allocate a page. The page that currently resides at pgnoRoot will
    ** be moved to the allocated page (unless the allocated page happens
    ** to reside at pgnoRoot).
    */
    rc = allocate_Btree_ਪੰਨਾ(pBt, &p_ਪੰਨਾ_Move, &pgnoMove, pgno_ਜੜ੍ਹ, BTALLOC_EXACT);
    ਜੇ( rc!=SQLITE_OK ){
      ਵਾਪਸ rc;
    }

    ਜੇ( pgnoMove!=pgno_ਜੜ੍ਹ ){
      /* pgnoRoot is the page that will be used for the root-page of
      ** the new table (assuming an error did not occur). But we were
      ** allocated pgnoMove. If required (i.e. if it was not allocated
      ** by extending the file), the current page at position pgnoMove
      ** is already journaled.
      */
      u8 eType = 0;
      Pgno i_Ptr_ਪੰਨਾ = 0;

      /* Save the positions of any open cursors. This is required in
      ** case they are holding a reference to an xFetch reference
      ** corresponding to page pgnoRoot.  */
      rc = saveAllCursors(pBt, 0, 0);
      release_ਪੰਨਾ(p_ਪੰਨਾ_Move);
      ਜੇ( rc!=SQLITE_OK ){
        ਵਾਪਸ rc;
      }

      /* Move the page currently at pgnoRoot to pgnoMove. */
      rc = btree_Get_ਪੰਨਾ(pBt, pgno_ਜੜ੍ਹ, &p_ਜੜ੍ਹ, 0);
      ਜੇ( rc!=SQLITE_OK ){
        ਵਾਪਸ rc;
      }
      rc = ptrmapGet(pBt, pgno_ਜੜ੍ਹ, &eType, &i_Ptr_ਪੰਨਾ);
      ਜੇ( eType==PTRMAP_ROOTPAGE || eType==PTRMAP_FREEPAGE ){
        rc = SQLITE_CORRUPT_PGNO(pgno_ਜੜ੍ਹ);
      }
      ਜੇ( rc!=SQLITE_OK ){
        release_ਪੰਨਾ(p_ਜੜ੍ਹ);
        ਵਾਪਸ rc;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( eType!=PTRMAP_ROOTPAGE );
      ਪ੍ਰਤਿਜ੍ਞਾ( eType!=PTRMAP_FREEPAGE );
      rc = relocate_ਪੰਨਾ(pBt, p_ਜੜ੍ਹ, eType, i_Ptr_ਪੰਨਾ, pgnoMove, 0);
      release_ਪੰਨਾ(p_ਜੜ੍ਹ);

      /* Obtain the page at pgnoRoot */
      ਜੇ( rc!=SQLITE_OK ){
        ਵਾਪਸ rc;
      }
      rc = btree_Get_ਪੰਨਾ(pBt, pgno_ਜੜ੍ਹ, &p_ਜੜ੍ਹ, 0);
      ਜੇ( rc!=SQLITE_OK ){
        ਵਾਪਸ rc;
      }
      rc = sqlite3PagerWrite(p_ਜੜ੍ਹ->p_Db_ਪੰਨਾ);
      ਜੇ( rc!=SQLITE_OK ){
        release_ਪੰਨਾ(p_ਜੜ੍ਹ);
        ਵਾਪਸ rc;
      }
    }ਨਹੀਂਤਾਂ{
      p_ਜੜ੍ਹ = p_ਪੰਨਾ_Move;
    }

    /* Update the pointer-map and meta-data with the new root-page number. */
    ptrmapPut(pBt, pgno_ਜੜ੍ਹ, PTRMAP_ROOTPAGE, 0, &rc);
    ਜੇ( rc ){
      release_ਪੰਨਾ(p_ਜੜ੍ਹ);
      ਵਾਪਸ rc;
    }

    /* When the new root page was allocated, page 1 was made writable in
    ** order either to increase the database filesize, or to decrement the
    ** freelist count.  Hence, the sqlite3BtreeUpdateMeta() call cannot fail.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(pBt->pPage1->p_Db_ਪੰਨਾ) );
    rc = sqlite3BtreeUpdateMeta(p, 4, pgno_ਜੜ੍ਹ);
    ਜੇ( NEVER(rc) ){
      release_ਪੰਨਾ(p_ਜੜ੍ਹ);
      ਵਾਪਸ rc;
    }

  }ਨਹੀਂਤਾਂ{
    rc = allocate_Btree_ਪੰਨਾ(pBt, &p_ਜੜ੍ਹ, &pgno_ਜੜ੍ਹ, 1, 0);
    ਜੇ( rc ) ਵਾਪਸ rc;
  }
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3PagerIswriteable(p_ਜੜ੍ਹ->p_Db_ਪੰਨਾ) );
  ਜੇ( createTabFlags & BTREE_INTKEY ){
    ptfFlags = PTF_INTKEY | PTF_LEAFDATA | PTF_ਪੱਤੀ;
  }ਨਹੀਂਤਾਂ{
    ptfFlags = PTF_ZERODATA | PTF_ਪੱਤੀ;
  }
  zero_ਪੰਨਾ(p_ਜੜ੍ਹ, ptfFlags);
  sqlite3PagerUnref(p_ਜੜ੍ਹ->p_Db_ਪੰਨਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ( (pBt->openFlags & BTREE_SINGLE)==0 || pgno_ਜੜ੍ਹ==2 );
  *pi_ਸਾਰਣੀ = pgno_ਜੜ੍ਹ;
  ਵਾਪਸ SQLITE_OK;
}
ਪੂਰਨ sqlite3_Btree_Create_ਸਾਰਣੀ(Btree *p, Pgno *pi_ਸਾਰਣੀ, ਪੂਰਨ flags){
  ਪੂਰਨ rc;
  sqlite3BtreeEnter(p);
  rc = btree_Create_ਸਾਰਣੀ(p, pi_ਸਾਰਣੀ, flags);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
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
ਸਥਿਰ ਪੂਰਨ clear_Database_ਪੰਨਾ(
  BtShared *pBt,           /* The BTree that contains the table */
  Pgno pgno,               /* Page number to clear */
  ਪੂਰਨ free_ਪੰਨਾ_Flag,        /* bit 0: Deallocate page.  Bits 1-31: depth */
  i64 *pnChange            /* Add number of Cells freed to this counter */
){
  Mem_ਪੰਨਾ *p_ਪੰਨਾ;
  ਪੂਰਨ rc;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pCell;
  ਪੂਰਨ i;
  ਪੂਰਨ hdr;
  CellInfo info;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pBt->mutex) );
  ਜੇ( pgno>btreePagecount(pBt) ){
    ਵਾਪਸ SQLITE_CORRUPT_PGNO(pgno);
  }
  ਜੇ( (free_ਪੰਨਾ_Flag>>1) > BTCURSOR_MAX_ਡੂੰਘਾਈ ){
    ਵਾਪਸ SQLITE_CORRUPT_PGNO(pgno);
  }
  rc = get_And_Init_ਪੰਨਾ(pBt, pgno, &p_ਪੰਨਾ, 0);
  ਜੇ( rc ) ਵਾਪਸ rc;
  ਜੇ( (pBt->openFlags & BTREE_SINGLE)==0
   && sqlite3_Pager_ਪੰਨਾ_Refcount(p_ਪੰਨਾ->p_Db_ਪੰਨਾ) != (1 + (pgno==1))
  ){
    rc = SQLITE_CORRUPT_ਪੰਨਾ(p_ਪੰਨਾ);
    ਜਾਓ cleardatabasepage_ਬਾਹਰਲਾ;
  }
  hdr = p_ਪੰਨਾ->hdr_ਵਿੱਥ;
  ਲਈ(i=0; i<p_ਪੰਨਾ->nCell; i++){
    pCell = ਲੱਭੋ_Cell(p_ਪੰਨਾ, i);
    ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
      rc = clear_Database_ਪੰਨਾ(pBt, get4byte(pCell),
                             (free_ਪੰਨਾ_Flag+2)|1, pnChange);
      ਜੇ( rc ) ਜਾਓ cleardatabasepage_ਬਾਹਰਲਾ;
    }
    BTREE_CLEAR_CELL(rc, p_ਪੰਨਾ, pCell, info);
    ਜੇ( rc ) ਜਾਓ cleardatabasepage_ਬਾਹਰਲਾ;
  }
  ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
    rc = clear_Database_ਪੰਨਾ(pBt, get4byte(&p_ਪੰਨਾ->a_ਅੰਕੜੇ[hdr+8]), 
                           (free_ਪੰਨਾ_Flag+2)|1, pnChange);
    ਜੇ( rc ) ਜਾਓ cleardatabasepage_ਬਾਹਰਲਾ;
    ਜੇ( p_ਪੰਨਾ->int_ਕੁੰਜੀ ) pnChange = 0;
  }
  ਜੇ( pnChange ){
    testcase( !p_ਪੰਨਾ->int_ਕੁੰਜੀ );
    *pnChange += p_ਪੰਨਾ->nCell;
  }
  ਜੇ( (free_ਪੰਨਾ_Flag&1)!=0 ){
    free_ਪੰਨਾ(p_ਪੰਨਾ, &rc);
  }ਨਹੀਂਤਾਂ ਜੇ( (rc = sqlite3PagerWrite(p_ਪੰਨਾ->p_Db_ਪੰਨਾ))==0 ){
    zero_ਪੰਨਾ(p_ਪੰਨਾ, p_ਪੰਨਾ->a_ਅੰਕੜੇ[hdr] | PTF_ਪੱਤੀ);
  }

cleardatabasepage_ਬਾਹਰਲਾ:
  release_ਪੰਨਾ(p_ਪੰਨਾ);
  ਵਾਪਸ rc;
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
ਪੂਰਨ sqlite3_Btree_Clear_ਸਾਰਣੀ(Btree *p, ਪੂਰਨ i_ਸਾਰਣੀ, i64 *pnChange){
  ਪੂਰਨ rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);
  ਪ੍ਰਤਿਜ੍ਞਾ( p->ਵਿੱਚ_Trans==TRANS_WRITE );

  rc = saveAllCursors(pBt, (Pgno)i_ਸਾਰਣੀ, 0);

  ਜੇ( SQLITE_OK==rc ){
    /* Invalidate all incrblob cursors open on table iTable (assuming iTable
    ** is the root of a table b-tree - if it is not, the following call is
    ** a no-op).  */
    ਜੇ( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, (Pgno)i_ਸਾਰਣੀ, 0, 1);
    }
    rc = clear_Database_ਪੰਨਾ(pBt, (Pgno)i_ਸਾਰਣੀ, 0, pnChange);
  }
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
}

/*
** Delete all information from the single table that pCur is open on.
**
** This routine only work for pCur on an ephemeral table.
*/
ਪੂਰਨ sqlite3_Btree_Clear_ਸਾਰਣੀ_ਦਾ_Cursor(BtCursor *pCur){
  ਵਾਪਸ sqlite3_Btree_Clear_ਸਾਰਣੀ(pCur->pBtree, pCur->pgno_ਜੜ੍ਹ, 0);
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
ਸਥਿਰ ਪੂਰਨ btree_Drop_ਸਾਰਣੀ(Btree *p, Pgno i_ਸਾਰਣੀ, ਪੂਰਨ *piMoved){
  ਪੂਰਨ rc;
  Mem_ਪੰਨਾ *p_ਪੰਨਾ = 0;
  BtShared *pBt = p->pBt;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3BtreeHoldsMutex(p) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->ਵਿੱਚ_Trans==TRANS_WRITE );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਸਾਰਣੀ>=2 );
  ਜੇ( i_ਸਾਰਣੀ>btreePagecount(pBt) ){
    ਵਾਪਸ SQLITE_CORRUPT_PGNO(i_ਸਾਰਣੀ);
  }

  rc = sqlite3_Btree_Clear_ਸਾਰਣੀ(p, i_ਸਾਰਣੀ, 0);
  ਜੇ( rc ) ਵਾਪਸ rc;
  rc = btree_Get_ਪੰਨਾ(pBt, (Pgno)i_ਸਾਰਣੀ, &p_ਪੰਨਾ, 0);
  ਜੇ( NEVER(rc) ){
    release_ਪੰਨਾ(p_ਪੰਨਾ);
    ਵਾਪਸ rc;
  }

  *piMoved = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
  free_ਪੰਨਾ(p_ਪੰਨਾ, &rc);
  release_ਪੰਨਾ(p_ਪੰਨਾ);
#ਨਹੀਂਤਾਂ
  ਜੇ( pBt->autoVacuum ){
    Pgno max_ਜੜ੍ਹ_Pgno;
    sqlite3BtreeGetMeta(p, BTREE_ਸਭ_ਤੋਂ_ਵੱਡਾ_ਜੜ੍ਹ_ਪੰਨਾ, &max_ਜੜ੍ਹ_Pgno);

    ਜੇ( i_ਸਾਰਣੀ==max_ਜੜ੍ਹ_Pgno ){
      /* If the table being dropped is the table with the largest root-page
      ** number in the database, put the root page on the free list.
      */
      free_ਪੰਨਾ(p_ਪੰਨਾ, &rc);
      release_ਪੰਨਾ(p_ਪੰਨਾ);
      ਜੇ( rc!=SQLITE_OK ){
        ਵਾਪਸ rc;
      }
    }ਨਹੀਂਤਾਂ{
      /* The table being dropped does not have the largest root-page
      ** number in the database. So move the page that does into the
      ** gap left by the deleted root-page.
      */
      Mem_ਪੰਨਾ *pMove;
      release_ਪੰਨਾ(p_ਪੰਨਾ);
      rc = btree_Get_ਪੰਨਾ(pBt, max_ਜੜ੍ਹ_Pgno, &pMove, 0);
      ਜੇ( rc!=SQLITE_OK ){
        ਵਾਪਸ rc;
      }
      rc = relocate_ਪੰਨਾ(pBt, pMove, PTRMAP_ROOTPAGE, 0, i_ਸਾਰਣੀ, 0);
      release_ਪੰਨਾ(pMove);
      ਜੇ( rc!=SQLITE_OK ){
        ਵਾਪਸ rc;
      }
      pMove = 0;
      rc = btree_Get_ਪੰਨਾ(pBt, max_ਜੜ੍ਹ_Pgno, &pMove, 0);
      ਜੇ( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pMove->p_Db_ਪੰਨਾ);
      }
      free_ਪੰਨਾ(pMove, &rc);
      release_ਪੰਨਾ(pMove);
      ਜੇ( rc!=SQLITE_OK ){
        ਵਾਪਸ rc;
      }
      *piMoved = max_ਜੜ੍ਹ_Pgno;
    }

    /* Set the new 'max-root-page' value in the database header. This
    ** is the old value less one, less one more if that happens to
    ** be a root-page number, less one again if that is the
    ** PENDING_BYTE_PAGE.
    */
    max_ਜੜ੍ਹ_Pgno--;
    ਜਦੋਂ( max_ਜੜ੍ਹ_Pgno==PENDING_BYTE_ਪੰਨਾ(pBt)
           || PTRMAP_ISPAGE(pBt, max_ਜੜ੍ਹ_Pgno) ){
      max_ਜੜ੍ਹ_Pgno--;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( max_ਜੜ੍ਹ_Pgno!=PENDING_BYTE_ਪੰਨਾ(pBt) );

    rc = sqlite3BtreeUpdateMeta(p, 4, max_ਜੜ੍ਹ_Pgno);
  }ਨਹੀਂਤਾਂ{
    free_ਪੰਨਾ(p_ਪੰਨਾ, &rc);
    release_ਪੰਨਾ(p_ਪੰਨਾ);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ rc; 
}
ਪੂਰਨ sqlite3_Btree_Drop_ਸਾਰਣੀ(Btree *p, ਪੂਰਨ i_ਸਾਰਣੀ, ਪੂਰਨ *piMoved){
  ਪੂਰਨ rc;
  sqlite3BtreeEnter(p);
  rc = btree_Drop_ਸਾਰਣੀ(p, i_ਸਾਰਣੀ, piMoved);
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
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
ਖਾਲੀ sqlite3BtreeGetMeta(Btree *p, ਪੂਰਨ idx, u32 *pMeta){
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  ਪ੍ਰਤਿਜ੍ਞਾ( p->ਵਿੱਚ_Trans>TRANS_NONE );
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_OK==query_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(p, SCHEMA_ਜੜ੍ਹ, READ_ਤਾਲਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->pPage1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( idx>=0 && idx<=15 );

  ਜੇ( idx==BTREE_ਅੰਕੜੇ_ਸੰਸਕਰਣ ){
    *pMeta = sqlite3_Pager_ਅੰਕੜੇ_ਸੰਸਕਰਣ(pBt->pPager) + p->i_BData_ਸੰਸਕਰਣ;
  }ਨਹੀਂਤਾਂ{
    *pMeta = get4byte(&pBt->pPage1->a_ਅੰਕੜੇ[36 + idx*4]);
  }

  /* If auto-vacuum is disabled in this build and this is an auto-vacuum
  ** database, mark the database as read-only.  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
  ਜੇ( idx==BTREE_ਸਭ_ਤੋਂ_ਵੱਡਾ_ਜੜ੍ਹ_ਪੰਨਾ && *pMeta>0 ){
    pBt->btsFlags |= BTS_READ_ONLY;
  }
#ਜੇ_ਅੰਤ

  sqlite3BtreeLeave(p);
}

/*
** Write meta-information back into the database.  Meta[0] is
** read-only and may not be written.
*/
ਪੂਰਨ sqlite3BtreeUpdateMeta(Btree *p, ਪੂਰਨ idx, u32 iMeta){
  BtShared *pBt = p->pBt;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pP1;
  ਪੂਰਨ rc;
  ਪ੍ਰਤਿਜ੍ਞਾ( idx>=1 && idx<=15 );
  sqlite3BtreeEnter(p);
  ਪ੍ਰਤਿਜ੍ਞਾ( p->ਵਿੱਚ_Trans==TRANS_WRITE );
  ਪ੍ਰਤਿਜ੍ਞਾ( pBt->pPage1!=0 );
  pP1 = pBt->pPage1->a_ਅੰਕੜੇ;
  rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ਪੰਨਾ);
  ਜੇ( rc==SQLITE_OK ){
    put4byte(&pP1[36 + idx*4], iMeta);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
    ਜੇ( idx==BTREE_INCR_VACUUM ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pBt->autoVacuum || iMeta==0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( iMeta==0 || iMeta==1 );
      pBt->incrVacuum = (u8)iMeta;
    }
#ਜੇ_ਅੰਤ
  }
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
}

/*
** The first argument, pCur, is a cursor opened on some b-tree. Count the
** number of entries in the b-tree and write the result to *pnEntry.
**
** SQLITE_OK is returned if the operation is successfully executed.
** Otherwise, if an error is encountered (i.e. an IO error or database
** corruption) an SQLite error code is returned.
*/
ਪੂਰਨ sqlite3_Btree_ਗਿਣਤੀ(sqlite3 *db, BtCursor *pCur, i64 *pn_ਇੰਦਰਾਜ){
  i64 n_ਇੰਦਰਾਜ = 0;                      /* Value to return in *pnEntry */
  ਪੂਰਨ rc;                              /* Return code */

  rc = move_ਮੰਜ਼ਿਲ_ਜੜ੍ਹ(pCur);
  ਜੇ( rc==SQLITE_ਖਾਲੀ_ਥਾਵਾਂ ){
    *pn_ਇੰਦਰਾਜ = 0;
    ਵਾਪਸ SQLITE_OK;
  }

  /* Unless an error occurs, the following loop runs one iteration for each
  ** page in the B-Tree structure (not including overflow pages).
  */
  ਜਦੋਂ( rc==SQLITE_OK && !Atomic_ਲੱਦੋ(&db->u1.isInterrupted) ){
    ਪੂਰਨ iIdx;                          /* Index of child node in parent */
    Mem_ਪੰਨਾ *p_ਪੰਨਾ;                    /* Current page of the b-tree */

    /* If this is a leaf page or the tree is not an int-key tree, then
    ** this page contains countable entries. Increment the entry counter
    ** accordingly.
    */
    p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;
    ਜੇ( p_ਪੰਨਾ->ਪੱਤੀ || !p_ਪੰਨਾ->int_ਕੁੰਜੀ ){
      n_ਇੰਦਰਾਜ += p_ਪੰਨਾ->nCell;
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
    ਜੇ( p_ਪੰਨਾ->ਪੱਤੀ ){
      ਕਰੋ {
        ਜੇ( pCur->i_ਪੰਨਾ==0 ){
          /* All pages of the b-tree have been visited. Return successfully. */
          *pn_ਇੰਦਰਾਜ = n_ਇੰਦਰਾਜ;
          ਵਾਪਸ move_ਮੰਜ਼ਿਲ_ਜੜ੍ਹ(pCur);
        }
        move_ਮੰਜ਼ਿਲ_ਮਾਪਾ(pCur);
      }ਜਦੋਂ ( pCur->ix>=pCur->p_ਪੰਨਾ->nCell );

      pCur->ix++;
      p_ਪੰਨਾ = pCur->p_ਪੰਨਾ;
    }

    /* Descend to the child node of the cell that the cursor currently
    ** points at. This is the right-child if (iIdx==pPage->nCell).
    */
    iIdx = pCur->ix;
    ਜੇ( iIdx==p_ਪੰਨਾ->nCell ){
      rc = move_ਮੰਜ਼ਿਲ_ਬੱਚਾ(pCur, get4byte(&p_ਪੰਨਾ->a_ਅੰਕੜੇ[p_ਪੰਨਾ->hdr_ਵਿੱਥ+8]));
    }ਨਹੀਂਤਾਂ{
      rc = move_ਮੰਜ਼ਿਲ_ਬੱਚਾ(pCur, get4byte(ਲੱਭੋ_Cell(p_ਪੰਨਾ, iIdx)));
    }
  }

  /* An error has occurred. Return an error code. */
  ਵਾਪਸ rc;
}

/*
** Return the pager associated with a BTree.  This routine is used for
** testing and debugging only.
*/
Pager *sqlite3BtreePager(Btree *p){
  ਵਾਪਸ p->pBt->pPager;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INTEGRITY_CHECK
/*
** Record an OOM error during integrity_check
*/
ਸਥਿਰ ਖਾਲੀ checkOom(IntegrityCk *pCheck){
  pCheck->rc = SQLITE_NOMEM;
  pCheck->mxErr = 0;  /* Causes integrity_check processing to stop */
  ਜੇ( pCheck->nErr==0 ) pCheck->nErr++;
}

/*
** Invoke the progress handler, if appropriate.  Also check for an
** interrupt.
*/
ਸਥਿਰ ਖਾਲੀ check_ਤਰੱਕੀ(IntegrityCk *pCheck){
  sqlite3 *db = pCheck->db;
  ਜੇ( Atomic_ਲੱਦੋ(&db->u1.isInterrupted) ){
    pCheck->rc = SQLITE_ਰੁਕਾਵਟ;
    pCheck->nErr++;
    pCheck->mxErr = 0;
  }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਤਰੱਕੀ_CALLBACK
  ਜੇ( db->x_ਤਰੱਕੀ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( db->n_ਤਰੱਕੀ_Ops>0 );
    pCheck->n_ਕਦਮ++;
    ਜੇ( (pCheck->n_ਕਦਮ % db->n_ਤਰੱਕੀ_Ops)==0
     && db->x_ਤਰੱਕੀ(db->p_ਤਰੱਕੀ_Arg)
    ){
      pCheck->rc = SQLITE_ਰੁਕਾਵਟ;
      pCheck->nErr++;
      pCheck->mxErr = 0;
    }
  }
#ਜੇ_ਅੰਤ
}

/*
** Append a message to the error message string.
*/
ਸਥਿਰ ਖਾਲੀ check_Append_ਸੁਨੇਹਾ(
  IntegrityCk *pCheck,
  ਅਚਲ ਅੱਖਰ *zFormat,
  ...
){
  va_list ap;
  check_ਤਰੱਕੀ(pCheck);
  ਜੇ( !pCheck->mxErr ) ਵਾਪਸ;
  pCheck->mxErr--;
  pCheck->nErr++;
  va_start(ap, zFormat);
  ਜੇ( pCheck->err_ਸੁਨੇਹਾ.nChar ){
    sqlite3_str_append(&pCheck->err_ਸੁਨੇਹਾ, "\n", 1);
  }
  ਜੇ( pCheck->zPfx ){
    sqlite3_str_appendf(&pCheck->err_ਸੁਨੇਹਾ, pCheck->zPfx,
                        pCheck->v0, pCheck->v1, pCheck->v2);
  }
  sqlite3_str_vappendf(&pCheck->err_ਸੁਨੇਹਾ, zFormat, ap);
  va_end(ap);
  ਜੇ( pCheck->err_ਸੁਨੇਹਾ.accError==SQLITE_NOMEM ){
    checkOom(pCheck);
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_INTEGRITY_CHECK */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INTEGRITY_CHECK

/*
** Return non-zero if the bit in the IntegrityCk.aPgRef[] array that
** corresponds to page iPg is already set.
*/
ਸਥਿਰ ਪੂਰਨ get_ਪੰਨਾ_Referenced(IntegrityCk *pCheck, Pgno iPg){
  ਪ੍ਰਤਿਜ੍ਞਾ( pCheck->aPgRef!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( iPg<=pCheck->n_Ck_ਪੰਨਾ && ਆਕਾਰ(pCheck->aPgRef[0])==1 );
  ਵਾਪਸ (pCheck->aPgRef[iPg/8] & (1 << (iPg & 0x07)));
}

/*
** Set the bit in the IntegrityCk.aPgRef[] array that corresponds to page iPg.
*/
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_ਪੰਨਾ_Referenced(IntegrityCk *pCheck, Pgno iPg){
  ਪ੍ਰਤਿਜ੍ਞਾ( pCheck->aPgRef!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( iPg<=pCheck->n_Ck_ਪੰਨਾ && ਆਕਾਰ(pCheck->aPgRef[0])==1 );
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
ਸਥਿਰ ਪੂਰਨ checkRef(IntegrityCk *pCheck, Pgno i_ਪੰਨਾ){
  ਜੇ( i_ਪੰਨਾ>pCheck->n_Ck_ਪੰਨਾ || i_ਪੰਨਾ==0 ){
    check_Append_ਸੁਨੇਹਾ(pCheck, "invalid page number %u", i_ਪੰਨਾ);
    ਵਾਪਸ 1;
  }
  ਜੇ( get_ਪੰਨਾ_Referenced(pCheck, i_ਪੰਨਾ) ){
    check_Append_ਸੁਨੇਹਾ(pCheck, "2nd reference to page %u", i_ਪੰਨਾ);
    ਵਾਪਸ 1;
  }
  ਸੈੱਟ_ਪੰਨਾ_Referenced(pCheck, i_ਪੰਨਾ);
  ਵਾਪਸ 0;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
/*
** Check that the entry in the pointer-map for page iChild maps to
** page iParent, pointer type ptrType. If not, append an error message
** to pCheck.
*/
ਸਥਿਰ ਖਾਲੀ checkPtrmap(
  IntegrityCk *pCheck,   /* Integrity check context */
  Pgno i_ਬੱਚਾ,           /* Child page number */
  u8 eType,              /* Expected pointer map type */
  Pgno i_ਮਾਪਾ           /* Expected pointer map parent page number */
){
  ਪੂਰਨ rc;
  u8 ePtrmapType;
  Pgno i_Ptrmap_ਮਾਪਾ;

  rc = ptrmapGet(pCheck->pBt, i_ਬੱਚਾ, &ePtrmapType, &i_Ptrmap_ਮਾਪਾ);
  ਜੇ( rc!=SQLITE_OK ){
    ਜੇ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ) checkOom(pCheck);
    check_Append_ਸੁਨੇਹਾ(pCheck, "Failed to read ptrmap key=%u", i_ਬੱਚਾ);
    ਵਾਪਸ;
  }

  ਜੇ( ePtrmapType!=eType || i_Ptrmap_ਮਾਪਾ!=i_ਮਾਪਾ ){
    check_Append_ਸੁਨੇਹਾ(pCheck,
      "Bad ptr map entry key=%u expected=(%u,%u) got=(%u,%u)",
      i_ਬੱਚਾ, eType, i_ਮਾਪਾ, ePtrmapType, i_Ptrmap_ਮਾਪਾ);
  }
}
#ਜੇ_ਅੰਤ

/*
** Check the integrity of the freelist or of an overflow page list.
** Verify that the number of pages on the list is N.
*/
ਸਥਿਰ ਖਾਲੀ checkList(
  IntegrityCk *pCheck,  /* Integrity checking context */
  ਪੂਰਨ isFreeList,       /* True for a freelist.  False for overflow page list */
  Pgno i_ਪੰਨਾ,           /* Page number for first page in the list */
  u32 N                 /* Expected number of pages in the list */
){
  ਪੂਰਨ i;
  u32 expected = N;
  ਪੂਰਨ n_Err_At_ਸ਼ੁਰੂ = pCheck->nErr;
  ਜਦੋਂ( i_ਪੰਨਾ!=0 && pCheck->mxErr ){
    Db_ਪੰਨਾ *p_Ovfl_ਪੰਨਾ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p_Ovfl_ਅੰਕੜੇ;
    ਜੇ( checkRef(pCheck, i_ਪੰਨਾ) ) ਰੋਕੋ;
    N--;
    ਜੇ( sqlite3PagerGet(pCheck->pPager, (Pgno)i_ਪੰਨਾ, &p_Ovfl_ਪੰਨਾ, 0) ){
      check_Append_ਸੁਨੇਹਾ(pCheck, "failed to get page %u", i_ਪੰਨਾ);
      ਰੋਕੋ;
    }
    p_Ovfl_ਅੰਕੜੇ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)sqlite3_Pager_Get_ਅੰਕੜੇ(p_Ovfl_ਪੰਨਾ);
    ਜੇ( isFreeList ){
      u32 n = (u32)get4byte(&p_Ovfl_ਅੰਕੜੇ[4]);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
      ਜੇ( pCheck->pBt->autoVacuum ){
        checkPtrmap(pCheck, i_ਪੰਨਾ, PTRMAP_FREEPAGE, 0);
      }
#ਜੇ_ਅੰਤ
      ਜੇ( n>pCheck->pBt->usable_ਮਾਪ/4-2 ){
        check_Append_ਸੁਨੇਹਾ(pCheck,
           "freelist leaf count too big on page %u", i_ਪੰਨਾ);
        N--;
      }ਨਹੀਂਤਾਂ{
        ਲਈ(i=0; i<(ਪੂਰਨ)n; i++){
          Pgno i_Free_ਪੰਨਾ = get4byte(&p_Ovfl_ਅੰਕੜੇ[8+i*4]);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
          ਜੇ( pCheck->pBt->autoVacuum ){
            checkPtrmap(pCheck, i_Free_ਪੰਨਾ, PTRMAP_FREEPAGE, 0);
          }
#ਜੇ_ਅੰਤ
          checkRef(pCheck, i_Free_ਪੰਨਾ);
        }
        N -= n;
      }
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
    ਨਹੀਂਤਾਂ{
      /* If this database supports auto-vacuum and iPage is not the last
      ** page in this overflow list, check that the pointer-map entry for
      ** the following page matches iPage.
      */
      ਜੇ( pCheck->pBt->autoVacuum && N>0 ){
        i = get4byte(p_Ovfl_ਅੰਕੜੇ);
        checkPtrmap(pCheck, i, PTRMAP_OVERFLOW2, i_ਪੰਨਾ);
      }
    }
#ਜੇ_ਅੰਤ
    i_ਪੰਨਾ = get4byte(p_Ovfl_ਅੰਕੜੇ);
    sqlite3PagerUnref(p_Ovfl_ਪੰਨਾ);
  }
  ਜੇ( N && n_Err_At_ਸ਼ੁਰੂ==pCheck->nErr ){
    check_Append_ਸੁਨੇਹਾ(pCheck,
      "%s is %u but should be %u",
      isFreeList ? "size" : "overflow list length",
      expected-N, expected);
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_INTEGRITY_CHECK */

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
ਸਥਿਰ ਖਾਲੀ btree_ਢੇਰ_ਪਾਓ(u32 *a_ਢੇਰ, u32 x){
  u32 j, i;
  ਪ੍ਰਤਿਜ੍ਞਾ( a_ਢੇਰ!=0 );
  i = ++a_ਢੇਰ[0];
  a_ਢੇਰ[i] = x;
  ਜਦੋਂ( (j = i/2)>0 && a_ਢੇਰ[j]>a_ਢੇਰ[i] ){
    x = a_ਢੇਰ[j];
    a_ਢੇਰ[j] = a_ਢੇਰ[i];
    a_ਢੇਰ[i] = x;
    i = j;
  }
}
ਸਥਿਰ ਪੂਰਨ btree_ਢੇਰ_Pull(u32 *a_ਢੇਰ, u32 *p_ਬਾਹਰਲਾ){
  u32 j, i, x;
  ਜੇ( (x = a_ਢੇਰ[0])==0 ) ਵਾਪਸ 0;
  *p_ਬਾਹਰਲਾ = a_ਢੇਰ[1];
  a_ਢੇਰ[1] = a_ਢੇਰ[x];
  a_ਢੇਰ[x] = 0xffffffff;
  a_ਢੇਰ[0]--;
  i = 1;
  ਜਦੋਂ( (j = i*2)<=a_ਢੇਰ[0] ){
    ਜੇ( a_ਢੇਰ[j]>a_ਢੇਰ[j+1] ) j++;
    ਜੇ( a_ਢੇਰ[i]<a_ਢੇਰ[j] ) ਰੋਕੋ;
    x = a_ਢੇਰ[i];
    a_ਢੇਰ[i] = a_ਢੇਰ[j];
    a_ਢੇਰ[j] = x;
    i = j;
  }
  ਵਾਪਸ 1; 
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INTEGRITY_CHECK
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
ਸਥਿਰ ਪੂਰਨ check_ਰੁੱਖ_ਪੰਨਾ(
  IntegrityCk *pCheck,  /* Context for the sanity check */
  Pgno i_ਪੰਨਾ,           /* Page number of the page to check */
  i64 *pi_Min_ਕੁੰਜੀ,        /* Write minimum integer primary key here */
  i64 max_ਕੁੰਜੀ            /* Error if integer primary key greater than this */
){
  Mem_ਪੰਨਾ *p_ਪੰਨਾ = 0;      /* The page being analyzed */
  ਪੂਰਨ i;                   /* Loop counter */
  ਪੂਰਨ rc;                  /* Result code from subroutine call */
  ਪੂਰਨ ਡੂੰਘਾਈ = -1, d2;      /* Depth of a subtree */
  ਪੂਰਨ pgno;                /* Page number */
  ਪੂਰਨ nFrag;               /* Number of fragmented bytes on the page */
  ਪੂਰਨ hdr;                 /* Offset to the page header */
  ਪੂਰਨ cell_ਸ਼ੁਰੂ;           /* Offset to the start of the cell pointer array */
  ਪੂਰਨ nCell;               /* Number of cells */
  ਪੂਰਨ doCoverageCheck = 1; /* True if cell coverage checking should be done */
  ਪੂਰਨ ਕੁੰਜੀ_Can_Be_Equal = 1;   /* True if IPK can be equal to maxKey
                           ** False if IPK must be strictly less than maxKey */
  u8 *ਅੰਕੜੇ;                /* Page content */
  u8 *pCell;               /* Cell content */
  u8 *pCellIdx;            /* Next element of the cell pointer array */
  BtShared *pBt;           /* The BtShared object that owns pPage */
  u32 pc;                  /* Address of a cell */
  u32 usable_ਮਾਪ;          /* Usable size of the page */
  u32 content_ਵਿੱਥ;       /* Offset to the start of the cell content area */
  u32 *ਢੇਰ = 0;           /* Min-heap used for checking cell coverage */
  u32 x, prev = 0;         /* Next and previous entry on the min-heap */
  ਅਚਲ ਅੱਖਰ *saved_zPfx = pCheck->zPfx;
  ਪੂਰਨ saved_v1 = pCheck->v1;
  ਪੂਰਨ saved_v2 = pCheck->v2;
  u8 savedIsInit = 0;

  /* Check that the page exists
  */
  check_ਤਰੱਕੀ(pCheck);
  ਜੇ( pCheck->mxErr==0 ) ਵਾਪਸ 0;
  pBt = pCheck->pBt;
  usable_ਮਾਪ = pBt->usable_ਮਾਪ;
  ਜੇ( i_ਪੰਨਾ==0 ) ਵਾਪਸ 0;
  ਜੇ( checkRef(pCheck, i_ਪੰਨਾ) ) ਵਾਪਸ 0;
  pCheck->zPfx = "Tree %u page %u: ";
  pCheck->v1 = i_ਪੰਨਾ;
  pCheck->nAbove++;
  ਜੇ( (rc = btree_Get_ਪੰਨਾ(pBt, i_ਪੰਨਾ, &p_ਪੰਨਾ, 0))!=0 ){
    check_Append_ਸੁਨੇਹਾ(pCheck,
       "unable to get the page. error code=%d", rc);
    ਜੇ( rc==SQLITE_IOERR_NOMEM ) pCheck->rc = SQLITE_NOMEM;
    ਜਾਓ ਸਿਰਾ_ਦਾ_check;
  }
  ਜੇ( pCheck->nAbove > BTCURSOR_MAX_ਡੂੰਘਾਈ ){
    check_Append_ਸੁਨੇਹਾ(pCheck,"btree depth exceeds %d",BTCURSOR_MAX_ਡੂੰਘਾਈ);
    ਜਾਓ ਸਿਰਾ_ਦਾ_check;
  }

  /* Clear MemPage.isInit to make sure the corruption detection code in
  ** btreeInitPage() is executed.  */
  savedIsInit = p_ਪੰਨਾ->isInit;
  p_ਪੰਨਾ->isInit = 0;
  ਜੇ( (rc = btree_Init_ਪੰਨਾ(p_ਪੰਨਾ))!=0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_CORRUPT );  /* The only possible error from InitPage */
    check_Append_ਸੁਨੇਹਾ(pCheck,
                   "btreeInitPage() returns error code %d", rc);
    ਜਾਓ ਸਿਰਾ_ਦਾ_check;
  }
  ਜੇ( (rc = btreeComputeFreeSpace(p_ਪੰਨਾ))!=0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_CORRUPT );
    check_Append_ਸੁਨੇਹਾ(pCheck, "free space corruption", rc);
    ਜਾਓ ਸਿਰਾ_ਦਾ_check;
  }
  ਅੰਕੜੇ = p_ਪੰਨਾ->a_ਅੰਕੜੇ;
  hdr = p_ਪੰਨਾ->hdr_ਵਿੱਥ;

  /* Set up for cell analysis */
  pCheck->zPfx = "Tree %u page %u cell %u: ";
  content_ਵਿੱਥ = get2byteNotZero(&ਅੰਕੜੇ[hdr+5]);
  ਪ੍ਰਤਿਜ੍ਞਾ( content_ਵਿੱਥ<=usable_ਮਾਪ );  /* Enforced by btreeInitPage() */

  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  nCell = get2byte(&ਅੰਕੜੇ[hdr+3]);
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->nCell==nCell );
  ਜੇ( p_ਪੰਨਾ->ਪੱਤੀ || p_ਪੰਨਾ->int_ਕੁੰਜੀ==0 ){
    pCheck->n_ਪੰਕਤੀ += nCell;
  }

  /* EVIDENCE-OF: R-23882-45353 The cell pointer array of a b-tree page
  ** immediately follows the b-tree page header. */
  cell_ਸ਼ੁਰੂ = hdr + 12 - 4*p_ਪੰਨਾ->ਪੱਤੀ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->aCellIdx==&ਅੰਕੜੇ[cell_ਸ਼ੁਰੂ] );
  pCellIdx = &ਅੰਕੜੇ[cell_ਸ਼ੁਰੂ + 2*(nCell-1)];

  ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
    /* Analyze the right-child page of internal pages */
    pgno = get4byte(&ਅੰਕੜੇ[hdr+8]);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
    ਜੇ( pBt->autoVacuum ){
      pCheck->zPfx = "Tree %u page %u right child: ";
      checkPtrmap(pCheck, pgno, PTRMAP_BTREE, i_ਪੰਨਾ);
    }
#ਜੇ_ਅੰਤ
    ਡੂੰਘਾਈ = check_ਰੁੱਖ_ਪੰਨਾ(pCheck, pgno, &max_ਕੁੰਜੀ, max_ਕੁੰਜੀ);
    ਕੁੰਜੀ_Can_Be_Equal = 0;
  }ਨਹੀਂਤਾਂ{
    /* For leaf pages, the coverage check will occur in the same loop
    ** as the other cell checks, so initialize the heap.  */
    ਢੇਰ = pCheck->ਢੇਰ;
    ਢੇਰ[0] = 0;
  }

  /* EVIDENCE-OF: R-02776-14802 The cell pointer array consists of K 2-byte
  ** integer offsets to the cell contents. */
  ਲਈ(i=nCell-1; i>=0 && pCheck->mxErr; i--){
    CellInfo info;

    /* Check cell size */
    pCheck->v2 = i;
    ਪ੍ਰਤਿਜ੍ਞਾ( pCellIdx==&ਅੰਕੜੇ[cell_ਸ਼ੁਰੂ + i*2] );
    pc = get2byteAligned(pCellIdx);
    pCellIdx -= 2;
    ਜੇ( pc<content_ਵਿੱਥ || pc>usable_ਮਾਪ-4 ){
      check_Append_ਸੁਨੇਹਾ(pCheck, "Offset %u out of range %u..%u",
                             pc, content_ਵਿੱਥ, usable_ਮਾਪ-4);
      doCoverageCheck = 0;
      ਜਾਰੀ;
    }
    pCell = &ਅੰਕੜੇ[pc];
    p_ਪੰਨਾ->xParseCell(p_ਪੰਨਾ, pCell, &info);
    ਜੇ( pc+info.n_ਮਾਪ>usable_ਮਾਪ ){
      check_Append_ਸੁਨੇਹਾ(pCheck, "Extends off end of page");
      doCoverageCheck = 0;
      ਜਾਰੀ;
    }
    ਜੇ( info.nPayload && info.pPayload[0]<2 ){
      check_Append_ਸੁਨੇਹਾ(pCheck, "Bad cell header size");
      doCoverageCheck = 0;
      ਜਾਰੀ;
    }

    /* Check for integer primary key out of range */
    ਜੇ( p_ਪੰਨਾ->int_ਕੁੰਜੀ ){
      ਜੇ( ਕੁੰਜੀ_Can_Be_Equal ? (info.n_ਕੁੰਜੀ > max_ਕੁੰਜੀ) : (info.n_ਕੁੰਜੀ >= max_ਕੁੰਜੀ) ){
        check_Append_ਸੁਨੇਹਾ(pCheck, "Rowid %lld out of order", info.n_ਕੁੰਜੀ);
      }
      max_ਕੁੰਜੀ = info.n_ਕੁੰਜੀ;
      ਕੁੰਜੀ_Can_Be_Equal = 0;     /* Only the first key on the page may ==maxKey */
    }

    /* Check the content overflow list */
    ਜੇ( info.nPayload>info.nLocal ){
      u32 n_ਪੰਨਾ;       /* Number of pages on the overflow chain */
      Pgno pgnoOvfl;   /* First page of the overflow chain */
      ਪ੍ਰਤਿਜ੍ਞਾ( pc + info.n_ਮਾਪ - 4 <= usable_ਮਾਪ );
      n_ਪੰਨਾ = (info.nPayload - info.nLocal + usable_ਮਾਪ - 5)/(usable_ਮਾਪ - 4);
      pgnoOvfl = get4byte(&pCell[info.n_ਮਾਪ - 4]);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
      ਜੇ( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgnoOvfl, PTRMAP_OVERFLOW1, i_ਪੰਨਾ);
      }
#ਜੇ_ਅੰਤ
      checkList(pCheck, 0, pgnoOvfl, n_ਪੰਨਾ);
    }

    ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
      /* Check sanity of left child page for internal pages */
      pgno = get4byte(pCell);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
      ਜੇ( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgno, PTRMAP_BTREE, i_ਪੰਨਾ);
      }
#ਜੇ_ਅੰਤ
      d2 = check_ਰੁੱਖ_ਪੰਨਾ(pCheck, pgno, &max_ਕੁੰਜੀ, max_ਕੁੰਜੀ);
      ਕੁੰਜੀ_Can_Be_Equal = 0;
      ਜੇ( d2!=ਡੂੰਘਾਈ ){
        check_Append_ਸੁਨੇਹਾ(pCheck, "Child page depth differs");
        ਡੂੰਘਾਈ = d2;
      }
    }ਨਹੀਂਤਾਂ{
      /* Populate the coverage-checking heap for leaf pages */
      ਪ੍ਰਤਿਜ੍ਞਾ( ਢੇਰ!=0 && ਢੇਰ[0] < pCheck->mx_ਢੇਰ );
      btree_ਢੇਰ_ਪਾਓ(ਢੇਰ, (pc<<16)|(pc+info.n_ਮਾਪ-1));
    }
  }
  *pi_Min_ਕੁੰਜੀ = max_ਕੁੰਜੀ;

  /* Check for complete coverage of the page
  */
  pCheck->zPfx = 0;
  ਜੇ( doCoverageCheck && pCheck->mxErr>0 ){
    /* For leaf pages, the min-heap has already been initialized and the
    ** cells have already been inserted.  But for internal pages, that has
    ** not yet been done, so do it now */
    ਜੇ( !p_ਪੰਨਾ->ਪੱਤੀ ){
      ਢੇਰ = pCheck->ਢੇਰ;
      ਢੇਰ[0] = 0;
      ਲਈ(i=nCell-1; i>=0; i--){
        u32 ਮਾਪ;
        pc = get2byteAligned(&ਅੰਕੜੇ[cell_ਸ਼ੁਰੂ+i*2]);
        ਮਾਪ = p_ਪੰਨਾ->x_Cell_ਮਾਪ(p_ਪੰਨਾ, &ਅੰਕੜੇ[pc]);
        ਪ੍ਰਤਿਜ੍ਞਾ( ਢੇਰ!=0 && ਢੇਰ[0] < pCheck->mx_ਢੇਰ );
        btree_ਢੇਰ_ਪਾਓ(ਢੇਰ, (pc<<16)|(pc+ਮਾਪ-1));
      }
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( ਢੇਰ!=0 );
    /* Add the freeblocks to the min-heap
    **
    ** EVIDENCE-OF: R-20690-50594 The second field of the b-tree page header
    ** is the offset of the first freeblock, or zero if there are no
    ** freeblocks on the page.
    */
    i = get2byte(&ਅੰਕੜੇ[hdr+1]);
    ਜਦੋਂ( i>0 ){
      ਪੂਰਨ ਮਾਪ, j;
      ਪ੍ਰਤਿਜ੍ਞਾ( (u32)i<=usable_ਮਾਪ-4 ); /* Enforced by btreeComputeFreeSpace() */
      ਮਾਪ = get2byte(&ਅੰਕੜੇ[i+2]);
      ਪ੍ਰਤਿਜ੍ਞਾ( (u32)(i+ਮਾਪ)<=usable_ਮਾਪ ); /* due to btreeComputeFreeSpace() */
      ਪ੍ਰਤਿਜ੍ਞਾ( ਢੇਰ!=0 && ਢੇਰ[0] < pCheck->mx_ਢੇਰ );
      btree_ਢੇਰ_ਪਾਓ(ਢੇਰ, (((u32)i)<<16)|(i+ਮਾਪ-1));
      /* EVIDENCE-OF: R-58208-19414 The first 2 bytes of a freeblock are a
      ** big-endian integer which is the offset in the b-tree page of the next
      ** freeblock in the chain, or zero if the freeblock is the last on the
      ** chain. */
      j = get2byte(&ਅੰਕੜੇ[i]);
      /* EVIDENCE-OF: R-06866-39125 Freeblocks are always connected in order of
      ** increasing offset. */
      ਪ੍ਰਤਿਜ੍ਞਾ( j==0 || j>i+ਮਾਪ );     /* Enforced by btreeComputeFreeSpace() */
      ਪ੍ਰਤਿਜ੍ਞਾ( (u32)j<=usable_ਮਾਪ-4 ); /* Enforced by btreeComputeFreeSpace() */
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
    prev = content_ਵਿੱਥ - 1;   /* Implied first min-heap entry */
    ਜਦੋਂ( btree_ਢੇਰ_Pull(ਢੇਰ,&x) ){
      ਜੇ( (prev&0xffff)>=(x>>16) ){
        check_Append_ਸੁਨੇਹਾ(pCheck,
          "Multiple uses for byte %u of page %u", x>>16, i_ਪੰਨਾ);
        ਰੋਕੋ;
      }ਨਹੀਂਤਾਂ{
        nFrag += (x>>16) - (prev&0xffff) - 1;
        prev = x;
      }
    }
    nFrag += usable_ਮਾਪ - (prev&0xffff) - 1;
    /* EVIDENCE-OF: R-43263-13491 The total number of bytes in all fragments
    ** is stored in the fifth field of the b-tree page header.
    ** EVIDENCE-OF: R-07161-27322 The one-byte integer at offset 7 gives the
    ** number of fragmented free bytes within the cell content area.
    */
    ਜੇ( ਢੇਰ[0]==0 && nFrag!=ਅੰਕੜੇ[hdr+7] ){
      check_Append_ਸੁਨੇਹਾ(pCheck,
          "Fragmentation of %u bytes reported as %u on page %u",
          nFrag, ਅੰਕੜੇ[hdr+7], i_ਪੰਨਾ);
    }
  }

ਸਿਰਾ_ਦਾ_check:
  ਜੇ( !doCoverageCheck ) p_ਪੰਨਾ->isInit = savedIsInit;
  release_ਪੰਨਾ(p_ਪੰਨਾ);
  pCheck->zPfx = saved_zPfx;
  pCheck->v1 = saved_v1;
  pCheck->v2 = saved_v2;
  pCheck->nAbove--;
  ਵਾਪਸ ਡੂੰਘਾਈ+1;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_INTEGRITY_CHECK */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INTEGRITY_CHECK
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
ਪੂਰਨ sqlite3BtreeIntegrityCheck(
  sqlite3 *db,  /* Database connection that is running the check */
  Btree *p,     /* The btree to be checked */
  Pgno *a_ਜੜ੍ਹ,  /* An array of root pages numbers for individual trees */
  Mem *aCnt,    /* Memory cells to write counts for each tree to */
  ਪੂਰਨ n_ਜੜ੍ਹ,    /* Number of entries in aRoot[] */
  ਪੂਰਨ mxErr,    /* Stop reporting errors after this many */
  ਪੂਰਨ *pnErr,   /* OUT: Write number of errors seen to this variable */
  ਅੱਖਰ **pz_ਬਾਹਰਲਾ  /* OUT: Write the error message string here */
){
  Pgno i;
  IntegrityCk sCheck;
  BtShared *pBt = p->pBt;
  u64 savedDbFlags = pBt->db->flags;
  ਅੱਖਰ zErr[100];
  ਪੂਰਨ bPartial = 0;            /* True if not checking all btrees */
  ਪੂਰਨ bCkFreelist = 1;         /* True to scan the freelist */
  VVA_ONLY( ਪੂਰਨ nRef );

  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਜੜ੍ਹ>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( aCnt!=0 );

  /* aRoot[0]==0 means this is a partial check */
  ਜੇ( a_ਜੜ੍ਹ[0]==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( n_ਜੜ੍ਹ>1 );
    bPartial = 1;
    ਜੇ( a_ਜੜ੍ਹ[1]!=1 ) bCkFreelist = 0;
  }

  sqlite3BtreeEnter(p);
  ਪ੍ਰਤਿਜ੍ਞਾ( p->ਵਿੱਚ_Trans>TRANS_NONE && pBt->ਵਿੱਚ_Transaction>TRANS_NONE );
  VVA_ONLY( nRef = sqlite3PagerRefcount(pBt->pPager) );
  ਪ੍ਰਤਿਜ੍ਞਾ( nRef>=0 );
  memset(&sCheck, 0, ਆਕਾਰ(sCheck));
  sCheck.db = db;
  sCheck.pBt = pBt;
  sCheck.pPager = pBt->pPager;
  sCheck.n_Ck_ਪੰਨਾ = btreePagecount(sCheck.pBt);
  sCheck.mxErr = mxErr;
  sqlite3StrAccumInit(&sCheck.err_ਸੁਨੇਹਾ, 0, zErr, ਆਕਾਰ(zErr), SQLITE_MAX_ਲੰਮਾਈ);
  sCheck.err_ਸੁਨੇਹਾ.printfFlags = SQLITE_PRINTF_ਅੰਦਰੂਨੀ;
  ਜੇ( sCheck.n_Ck_ਪੰਨਾ==0 ){
    ਜਾਓ integrity_ck_cleanup;
  }

  sCheck.aPgRef = sqlite3MallocZero((sCheck.n_Ck_ਪੰਨਾ / 8)+ 1);
  ਜੇ( !sCheck.aPgRef ){
    checkOom(&sCheck);
    ਜਾਓ integrity_ck_cleanup;
  }
  sCheck.ਢੇਰ = (u32*)sqlite3Malloc( pBt->ਪੰਨਾ_ਮਾਪ*2 );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  sCheck.mx_ਢੇਰ = pBt->ਪੰਨਾ_ਮਾਪ/2 - 1;
#ਜੇ_ਅੰਤ
  ਜੇ( sCheck.ਢੇਰ==0 ){
    checkOom(&sCheck);
    ਜਾਓ integrity_ck_cleanup;
  }

  i = PENDING_BYTE_ਪੰਨਾ(pBt);
  ਜੇ( i<=sCheck.n_Ck_ਪੰਨਾ ) ਸੈੱਟ_ਪੰਨਾ_Referenced(&sCheck, i);

  /* Check the integrity of the freelist
  */
  ਜੇ( bCkFreelist ){
    sCheck.zPfx = "Freelist: ";
    checkList(&sCheck, 1, get4byte(&pBt->pPage1->a_ਅੰਕੜੇ[32]),
              get4byte(&pBt->pPage1->a_ਅੰਕੜੇ[36]));
    sCheck.zPfx = 0;
  }

  /* Check all the tables.
  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
  ਜੇ( !bPartial ){
    ਜੇ( pBt->autoVacuum ){
      Pgno mx = 0;
      Pgno mx_ਵਿੱਚ_Hdr;
      ਲਈ(i=0; (ਪੂਰਨ)i<n_ਜੜ੍ਹ; i++) ਜੇ( mx<a_ਜੜ੍ਹ[i] ) mx = a_ਜੜ੍ਹ[i];
      mx_ਵਿੱਚ_Hdr = get4byte(&pBt->pPage1->a_ਅੰਕੜੇ[52]);
      ਜੇ( mx!=mx_ਵਿੱਚ_Hdr ){
        check_Append_ਸੁਨੇਹਾ(&sCheck,
          "max rootpage (%u) disagrees with header (%u)",
          mx, mx_ਵਿੱਚ_Hdr
        );
      }
    }ਨਹੀਂਤਾਂ ਜੇ( get4byte(&pBt->pPage1->a_ਅੰਕੜੇ[64])!=0 ){
      check_Append_ਸੁਨੇਹਾ(&sCheck,
        "incremental_vacuum enabled with a max rootpage of zero"
      );
    }
  }
#ਜੇ_ਅੰਤ
  testcase( pBt->db->flags & SQLITE_Cell_ਮਾਪ_Ck );
  pBt->db->flags &= ~(u64)SQLITE_Cell_ਮਾਪ_Ck;
  ਲਈ(i=0; (ਪੂਰਨ)i<n_ਜੜ੍ਹ && sCheck.mxErr; i++){
    sCheck.n_ਪੰਕਤੀ = 0;
    ਜੇ( a_ਜੜ੍ਹ[i] ){
      i64 not_ਵਰਤਿਆ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
      ਜੇ( pBt->autoVacuum && a_ਜੜ੍ਹ[i]>1 && !bPartial ){
        checkPtrmap(&sCheck, a_ਜੜ੍ਹ[i], PTRMAP_ROOTPAGE, 0);
      }
#ਜੇ_ਅੰਤ
      sCheck.v0 = a_ਜੜ੍ਹ[i];
      ਪ੍ਰਤਿਜ੍ਞਾ( sCheck.nAbove==0 );
      check_ਰੁੱਖ_ਪੰਨਾ(&sCheck, a_ਜੜ੍ਹ[i], &not_ਵਰਤਿਆ, ਸਭ_ਤੋਂ_ਵੱਡਾ_INT64);
      ਪ੍ਰਤਿਜ੍ਞਾ( sCheck.nAbove==0 );
    }
    sqlite3_Mem_ਸੈੱਟ_Array_Int64(aCnt, i, sCheck.n_ਪੰਕਤੀ);
  }
  pBt->db->flags = savedDbFlags;

  /* Make sure every page in the file is referenced
  */
  ਜੇ( !bPartial ){
    ਲਈ(i=1; i<=sCheck.n_Ck_ਪੰਨਾ && sCheck.mxErr; i++){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTOVACUUM
      ਜੇ( get_ਪੰਨਾ_Referenced(&sCheck, i)==0 ){
        check_Append_ਸੁਨੇਹਾ(&sCheck, "Page %u: never used", i);
      }
#ਨਹੀਂਤਾਂ
      /* If the database supports auto-vacuum, make sure no tables contain
      ** references to pointer-map pages.
      */
      ਜੇ( get_ਪੰਨਾ_Referenced(&sCheck, i)==0 &&
         (PTRMAP_PAGENO(pBt, i)!=i || !pBt->autoVacuum) ){
        check_Append_ਸੁਨੇਹਾ(&sCheck, "Page %u: never used", i);
      }
      ਜੇ( get_ਪੰਨਾ_Referenced(&sCheck, i)!=0 &&
         (PTRMAP_PAGENO(pBt, i)==i && pBt->autoVacuum) ){
        check_Append_ਸੁਨੇਹਾ(&sCheck, "Page %u: pointer map referenced", i);
      }
#ਜੇ_ਅੰਤ
    }
  }

  /* Clean  up and report errors.
  */
integrity_ck_cleanup:
  sqlite3_free(sCheck.ਢੇਰ);
  sqlite3_free(sCheck.aPgRef);
  *pnErr = sCheck.nErr;
  ਜੇ( sCheck.nErr==0 ){
    sqlite3_str_reset(&sCheck.err_ਸੁਨੇਹਾ);
    *pz_ਬਾਹਰਲਾ = 0;
  }ਨਹੀਂਤਾਂ{
    *pz_ਬਾਹਰਲਾ = sqlite3_Str_Accum_ਅੰਤ(&sCheck.err_ਸੁਨੇਹਾ);
  }
  /* Make sure this analysis did not leave any unref() pages. */
  ਪ੍ਰਤਿਜ੍ਞਾ( nRef==sqlite3PagerRefcount(pBt->pPager) );
  sqlite3BtreeLeave(p);
  ਵਾਪਸ sCheck.rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_INTEGRITY_CHECK */

/*
** Return the full pathname of the underlying database file.  Return
** an empty string if the database is in-memory or a TEMP database.
**
** The pager filename is invariant as long as the pager is
** open so it is safe to access without the BtShared mutex.
*/
ਅਚਲ ਅੱਖਰ *sqlite3BtreeGetFilename(Btree *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pBt->pPager!=0 );
  ਵਾਪਸ sqlite3PagerFilename(p->pBt->pPager, 1);
}

/*
** Return the pathname of the journal file for this database. The return
** value of this routine is the same regardless of whether the journal file
** has been created or not.
**
** The pager journal filename is invariant as long as the pager is
** open so it is safe to access without the BtShared mutex.
*/
ਅਚਲ ਅੱਖਰ *sqlite3BtreeGetJournalname(Btree *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pBt->pPager!=0 );
  ਵਾਪਸ sqlite3PagerJournalname(p->pBt->pPager);
}

/*
** Return one of SQLITE_TXN_NONE, SQLITE_TXN_READ, or SQLITE_TXN_WRITE
** to describe the current transaction state of Btree p.
*/
ਪੂਰਨ sqlite3BtreeTxnState(Btree *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p==0 || sqlite3_mutex_held(p->db->mutex) );
  ਵਾਪਸ p ? p->ਵਿੱਚ_Trans : 0;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
/*
** Run a checkpoint on the Btree passed as the first argument.
**
** Return SQLITE_LOCKED if this or any other connection has an open
** transaction on the shared-cache the argument Btree is connected to.
**
** Parameter eMode is one of SQLITE_CHECKPOINT_PASSIVE, FULL or RESTART.
*/
ਪੂਰਨ sqlite3BtreeCheckpoint(Btree *p, ਪੂਰਨ e_ਬਹੁਲਕ, ਪੂਰਨ *pnLog, ਪੂਰਨ *pnCkpt){
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( p ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
    ਜੇ( pBt->ਵਿੱਚ_Transaction!=TRANS_NONE ){
      rc = SQLITE_LOCKED;
    }ਨਹੀਂਤਾਂ{
      rc = sqlite3PagerCheckpoint(pBt->pPager, p->db, e_ਬਹੁਲਕ, pnLog, pnCkpt);
    }
    sqlite3BtreeLeave(p);
  }
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ

/*
** Return true if there is currently a backup running on Btree p.
*/
ਪੂਰਨ sqlite3_Btree_Is_ਵਿੱਚ_Backup(Btree *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->db->mutex) );
  ਵਾਪਸ p->nBackup!=0;
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
ਖਾਲੀ *sqlite3BtreeSchema(Btree *p, ਪੂਰਨ n_ਬਾਈਟਾਂ, ਖਾਲੀ(*xFree)(ਖਾਲੀ *)){
  BtShared *pBt = p->pBt;
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਬਾਈਟਾਂ==0 || n_ਬਾਈਟਾਂ==ਆਕਾਰ(Schema) );
  sqlite3BtreeEnter(p);
  ਜੇ( !pBt->pSchema && n_ਬਾਈਟਾਂ ){
    pBt->pSchema = sqlite3DbMallocZero(0, n_ਬਾਈਟਾਂ);
    pBt->xFreeSchema = xFree;
  }
  sqlite3BtreeLeave(p);
  ਵਾਪਸ pBt->pSchema;
}

/*
** Return SQLITE_LOCKED_SHAREDCACHE if another user of the same shared
** btree as the argument handle holds an exclusive lock on the
** sqlite_schema table. Otherwise SQLITE_OK.
*/
ਪੂਰਨ sqlite3BtreeSchemaLocked(Btree *p){
  ਪੂਰਨ rc;
  ਅਣਵਰਤਿਆ_PARAMETER(p);  /* only used in DEBUG builds */
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  rc = query_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(p, SCHEMA_ਜੜ੍ਹ, READ_ਤਾਲਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || rc==SQLITE_LOCKED_SHAREDCACHE );
  sqlite3BtreeLeave(p);
  ਵਾਪਸ rc;
}


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
/*
** Obtain a lock on the table whose root page is iTab.  The
** lock is a write lock if isWritelock is true or a read lock
** if it is false.
*/
ਪੂਰਨ sqlite3_Btree_ਤਾਲਾ_ਸਾਰਣੀ(Btree *p, ਪੂਰਨ iTab, u8 is_Write_ਤਾਲਾ){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->ਵਿੱਚ_Trans!=TRANS_NONE );
  ਜੇ( p->sharable ){
    u8 ਤਾਲਾ_Type = READ_ਤਾਲਾ + is_Write_ਤਾਲਾ;
    ਪ੍ਰਤਿਜ੍ਞਾ( READ_ਤਾਲਾ+1==WRITE_ਤਾਲਾ );
    ਪ੍ਰਤਿਜ੍ਞਾ( is_Write_ਤਾਲਾ==0 || is_Write_ਤਾਲਾ==1 );

    sqlite3BtreeEnter(p);
    rc = query_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(p, iTab, ਤਾਲਾ_Type);
    ਜੇ( rc==SQLITE_OK ){
      rc = ਸੈੱਟ_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(p, iTab, ਤਾਲਾ_Type);
    }
    sqlite3BtreeLeave(p);
  }
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
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
ਪੂਰਨ sqlite3_Btree_Put_ਅੰਕੜੇ(BtCursor *pCsr, u32 ਵਿੱਥ, u32 amt, ਖਾਲੀ *z){
  ਪੂਰਨ rc;
  ਪ੍ਰਤਿਜ੍ਞਾ( cursorOwnsBtShared(pCsr) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCsr->pBtree->db->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->curFlags & BTCF_Incrblob );

  rc = restoreCursorPosition(pCsr);
  ਜੇ( rc!=SQLITE_OK ){
    ਵਾਪਸ rc;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->eState!=CURSOR_REQUIRESEEK );
  ਜੇ( pCsr->eState!=CURSOR_VALID ){
    ਵਾਪਸ SQLITE_ABORT;
  }

  /* Save the positions of all other cursors open on this table. This is
  ** required in case any of them are holding references to an xFetch
  ** version of the b-tree page modified by the accessPayload call below.
  **
  ** Note that pCsr must be open on a INTKEY table and saveCursorPosition()
  ** and hence saveAllCursors() cannot fail on a BTREE_INTKEY table, hence
  ** saveAllCursors can only return SQLITE_OK.
  */
  VVA_ONLY(rc =) saveAllCursors(pCsr->pBt, pCsr->pgno_ਜੜ੍ਹ, pCsr);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK );

  /* Check some assumptions:
  **   (a) the cursor is open for writing,
  **   (b) there is a read/write transaction open,
  **   (c) the connection holds a write-lock on the table (if required),
  **   (d) there are no conflicting read-locks, and
  **   (e) the cursor points at a valid row of an intKey table.
  */
  ਜੇ( (pCsr->curFlags & BTCF_WriteFlag)==0 ){
    ਵਾਪਸ SQLITE_READONLY;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( (pCsr->pBt->btsFlags & BTS_READ_ONLY)==0
              && pCsr->pBt->ਵਿੱਚ_Transaction==TRANS_WRITE );
  ਪ੍ਰਤਿਜ੍ਞਾ( has_Shared_Cache_ਸਾਰਣੀ_ਤਾਲਾ(pCsr->pBtree, pCsr->pgno_ਜੜ੍ਹ, 0, 2) );
  ਪ੍ਰਤਿਜ੍ਞਾ( !hasReadConflicts(pCsr->pBtree, pCsr->pgno_ਜੜ੍ਹ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->p_ਪੰਨਾ->int_ਕੁੰਜੀ );

  ਵਾਪਸ accessPayload(pCsr, ਵਿੱਥ, amt, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)z, 1);
}

/*
** Mark this cursor as an incremental blob cursor.
*/
ਖਾਲੀ sqlite3BtreeIncrblobCursor(BtCursor *pCur){
  pCur->curFlags |= BTCF_Incrblob;
  pCur->pBtree->hasIncrblobCur = 1;
}
#ਜੇ_ਅੰਤ

/*
** Set both the "read version" (single byte at byte offset 18) and
** "write version" (single byte at byte offset 19) fields in the database
** header to iVersion.
*/
ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_ਸੰਸਕਰਣ(Btree *pBtree, ਪੂਰਨ i_ਸੰਸਕਰਣ){
  BtShared *pBt = pBtree->pBt;
  ਪੂਰਨ rc;                         /* Return code */

  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਸੰਸਕਰਣ==1 || i_ਸੰਸਕਰਣ==2 );

  /* If setting the version fields to 1, do not automatically open the
  ** WAL connection, even if the version fields are currently set to 2.
  */
  pBt->btsFlags &= ~BTS_NO_WAL;
  ਜੇ( i_ਸੰਸਕਰਣ==1 ) pBt->btsFlags |= BTS_NO_WAL;

  rc = sqlite3BtreeBeginTrans(pBtree, 0, 0);
  ਜੇ( rc==SQLITE_OK ){
    u8 *a_ਅੰਕੜੇ = pBt->pPage1->a_ਅੰਕੜੇ;
    ਜੇ( a_ਅੰਕੜੇ[18]!=(u8)i_ਸੰਸਕਰਣ || a_ਅੰਕੜੇ[19]!=(u8)i_ਸੰਸਕਰਣ ){
      rc = sqlite3BtreeBeginTrans(pBtree, 2, 0);
      ਜੇ( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ਪੰਨਾ);
        ਜੇ( rc==SQLITE_OK ){
          a_ਅੰਕੜੇ[18] = (u8)i_ਸੰਸਕਰਣ;
          a_ਅੰਕੜੇ[19] = (u8)i_ਸੰਸਕਰਣ;
        }
      }
    }
  }

  pBt->btsFlags &= ~BTS_NO_WAL;
  ਵਾਪਸ rc;
}

/*
** Return true if the cursor has a hint specified.  This routine is
** only used from within assert() statements
*/
ਪੂਰਨ sqlite3BtreeCursorHasHint(BtCursor *pCsr, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਨਕਾਬ){
  ਵਾਪਸ (pCsr->hints & ਨਕਾਬ)!=0;
}

/*
** Return true if the given Btree is read-only.
*/
ਪੂਰਨ sqlite3BtreeIsReadonly(Btree *p){
  ਵਾਪਸ (p->pBt->btsFlags & BTS_READ_ONLY)!=0;
}

/*
** Return the size of the header added to each page by this module.
*/
ਪੂਰਨ sqlite3_ਸਿਰਲੇਖ_ਮਾਪ_Btree(ਖਾਲੀ){ ਵਾਪਸ ROUND8(ਆਕਾਰ(Mem_ਪੰਨਾ)); }

/*
** If no transaction is active and the database is not a temp-db, clear
** the in-memory pager cache.
*/
ਖਾਲੀ sqlite3BtreeClearCache(Btree *p){
  BtShared *pBt = p->pBt;
  ਜੇ( pBt->ਵਿੱਚ_Transaction==TRANS_NONE ){
    sqlite3PagerClearCache(pBt->pPager);
  }
}

#ਜੇ !defined(SQLITE_OMIT_SHARED_CACHE)
/*
** Return true if the Btree passed as the only argument is sharable.
*/
ਪੂਰਨ sqlite3BtreeSharable(Btree *p){
  ਵਾਪਸ p->sharable;
}

/*
** Return the number of connections to the BtShared object accessed by
** the Btree handle passed as the only argument. For private caches
** this is always 1. For shared caches it may be 1 or greater.
*/
ਪੂਰਨ sqlite3_Btree_Connection_ਗਿਣਤੀ(Btree *p){
  testcase( p->sharable );
  ਵਾਪਸ p->pBt->nRef;
}
#ਜੇ_ਅੰਤ

/* Re-enable GCC compiler warnings that were suppressed at the top
** of this source file to prevent annoying false-positives.
*/
#ਜੇ defined(__GNUC__) && __GNUC__>=11
# pragma GCC diagnostic ਕੱਢੋ
#ਜੇ_ਅੰਤ
