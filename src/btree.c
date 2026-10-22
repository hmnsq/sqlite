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
#ಅನ್ತರ್ಭಾವ "btreeInt.h"

/*
** Suppress false-positive compiler warnings from GCC.  Warnings are
** re-enabled at the bottom of this source file.
*/
#ಆದರೆ defined(__GNUC__) && __GNUC__>=11
# pragma GCC diagnostic ತಳ್ಳು
# pragma GCC diagnostic ignored "-Wstringop-overread"
# pragma GCC diagnostic ignored "-Wstringop-overflow"
#ಆದರೆ_ಅಂತ್ಯ

/*
** The header string that appears at the beginning of every
** SQLite database.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ z_Magic_ಶೀರ್ಷಿಕೆ[] = SQLITE_FILE_ಶೀರ್ಷಿಕೆ;

/*
** Set this global variable to 1 to enable tracing using the TRACE
** macro.
*/
#ಆದರೆ 0
ಪೂರ್ಣಾಂಕ sqlite3BtreeTrace=1;  /* True to enable tracing */
# define TRACE(X)  ಆದರೆ(sqlite3BtreeTrace){ಬರೆ X;fflush(stdout);}
#ಇಲ್ಲದಿದ್ದರೆ
# define TRACE(X)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Extract a 2-byte big-endian integer from an array of unsigned bytes.
** But if the value is zero, make it 65536.
**
** This routine is used to extract the "offset to cell content area" value
** from the header of a btree page.  If the page size is 65536 and the page
** is empty, the offset should be 65536, but the 2-byte value stores zero.
** This routine makes the necessary adjustment to 65536.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು get2byteNotZero(X)  (((((ಪೂರ್ಣಾಂಕ)get2byte(X))-1)&0xffff)+1)

/*
** Values passed as the 5th argument to allocateBtreePage()
*/
#ವ್ಯಾಖ್ಯಾನಿಸು BTALLOC_ANY   0           /* Allocate any page */
#ವ್ಯಾಖ್ಯಾನಿಸು BTALLOC_EXACT 1           /* Allocate exact page if possible */
#ವ್ಯಾಖ್ಯಾನಿಸು BTALLOC_LE    2           /* Allocate any page <= the parameter */

/*
** Macro IfNotOmitAV(x) returns (x) if SQLITE_OMIT_AUTOVACUUM is not
** defined, or 0 if it is. For example:
**
**   bIncrVacuum = IfNotOmitAV(pBtShared->incrVacuum);
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
#ವ್ಯಾಖ್ಯಾನಿಸು IfNotOmitAV(ಅಭಿವ್ಯಕ್ತಿ_2) (ಅಭಿವ್ಯಕ್ತಿ_2)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು IfNotOmitAV(ಅಭಿವ್ಯಕ್ತಿ_2) 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
/*
** A list of BtShared objects that are eligible for participation
** in shared cache.  This variable has file scope during normal builds,
** but the test harness needs to access it so we make it global for
** test builds.
**
** Access to this variable is protected by SQLITE_MUTEX_STATIC_MAIN.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
BtShared *SQLITE_WSD sqlite3SharedCacheList = 0;
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ BtShared *SQLITE_WSD sqlite3SharedCacheList = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_SHARED_CACHE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
/*
** Enable or disable the shared pager and schema features.
**
** This routine has no effect on existing database connections.
** The shared cache setting effects only future calls to
** sqlite3_open(), sqlite3_open16(), or sqlite3_open_v2().
*/
ಪೂರ್ಣಾಂಕ sqlite3_enable_shared_cache(ಪೂರ್ಣಾಂಕ enable){
  sqlite3GlobalConfig.sharedCacheEnabled = enable;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ



#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_SHARED_CACHE
  /*
  ** The functions querySharedCacheTableLock(), setSharedCacheTableLock(),
  ** and clearAllSharedCacheTableLocks()
  ** manipulate entries in the BtShared.pLock linked list used to store
  ** shared-cache table level locks. If the library is compiled with the
  ** shared-cache feature disabled, then there is only ever one user
  ** of each BtShared structure and so this locking is not necessary.
  ** So define the lock related functions as no-ops.
  */
  #ವ್ಯಾಖ್ಯಾನಿಸು query_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(a,b,c) SQLITE_OK
  #ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(a,b,c) SQLITE_OK
  #ವ್ಯಾಖ್ಯಾನಿಸು clear_All_Shared_Cache_ಕೋಷ್ಟಕ_Locks(a)
  #ವ್ಯಾಖ್ಯಾನಿಸು downgrade_All_Shared_Cache_ಕೋಷ್ಟಕ_Locks(a)
  #ವ್ಯಾಖ್ಯಾನಿಸು has_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(a,b,c,d) 1
  #ವ್ಯಾಖ್ಯಾನಿಸು hasReadConflicts(a, b) 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Return and reset the seek counter for a Btree object.
*/
sqlite3_uint64 sqlite3_Btree_Seek_ಎಣಿಕೆ(Btree *pBt){
  u64 n =  pBt->nSeek;
  pBt->nSeek = 0;
  ಹಿಂತಿರುಗಿ n;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Implementation of the SQLITE_CORRUPT_PAGE() macro. Takes a single
** (MemPage*) as an argument. The (MemPage*) must not be NULL.
**
** If SQLITE_DEBUG is not defined, then this macro is equivalent to
** SQLITE_CORRUPT_BKPT. Or, if SQLITE_DEBUG is set, then the log message
** normally produced as a side-effect of SQLITE_CORRUPT_BKPT is augmented
** with the page number and filename associated with the (MemPage*).
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
ಪೂರ್ಣಾಂಕ corrupt_ಪುಟ_Error(ಪೂರ್ಣಾಂಕ lineno, Mem_ಪುಟ *p){
  ಅಕ್ಷರ *z_ಸಂದೇಶ;
  sqlite3BeginBenignMalloc();
  z_ಸಂದೇಶ = sqlite3_mprintf("database corruption page %u of %s",
             p->pgno, sqlite3PagerFilename(p->pBt->pPager, 0)
  );
  sqlite3_ಕೊನೆ_Benign_Malloc();
  ಆದರೆ( z_ಸಂದೇಶ ){
    sqlite3ReportError(SQLITE_CORRUPT, lineno, z_ಸಂದೇಶ);
  }
  sqlite3_free(z_ಸಂದೇಶ);
  ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
}
# define SQLITE_CORRUPT_ಪುಟ(p_Mem_ಪುಟ) corrupt_ಪುಟ_Error(__LINE__, p_Mem_ಪುಟ)
#ಇಲ್ಲದಿದ್ದರೆ
# define SQLITE_CORRUPT_ಪುಟ(p_Mem_ಪುಟ) SQLITE_CORRUPT_PGNO(p_Mem_ಪುಟ->pgno)
#ಆದರೆ_ಅಂತ್ಯ

/* Default value for SHARED_LOCK_TRACE macro if shared-cache is disabled
** or if the lock tracking is disabled.  This is always the value for
** release builds.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SHARED_ಬೀಗ_TRACE(X,ಸಂದೇಶ,TAB,TYPE)  /*no-op*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE

#ಆದರೆ 0
/*  ^----  Change to 1 and recompile to enable shared-lock tracing
**         for debugging purposes.
**
** Print all shared-cache locks on a BtShared.  Debugging use only.
*/
ಸ್ಥಿರ ಶೂನ್ಯ shared_ಬೀಗ_Trace(
  BtShared *pBt,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಸಂದೇಶ,
  ಪೂರ್ಣಾಂಕ i_ಬೇರು,
  ಪೂರ್ಣಾಂಕ e_ಬೀಗ_Type
){
  Bt_ಬೀಗ *p_ಬೀಗ;
  ಆದರೆ( i_ಬೇರು>0 ){
    ಬರೆ("%s-%p %u%s:", z_ಸಂದೇಶ, pBt, i_ಬೇರು, e_ಬೀಗ_Type==READ_ಬೀಗ?"R":"W");
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಬರೆ("%s-%p:", z_ಸಂದೇಶ, pBt);
  }
  ಗಾಗಿ(p_ಬೀಗ=pBt->p_ಬೀಗ; p_ಬೀಗ; p_ಬೀಗ=p_ಬೀಗ->p_ಮುಂದಿನ){
    ಬರೆ(" %p/%u%s", p_ಬೀಗ->pBtree, p_ಬೀಗ->i_ಕೋಷ್ಟಕ,
           p_ಬೀಗ->e_ಬೀಗ==READ_ಬೀಗ ? "R" : "W");
    ಆಗ( p_ಬೀಗ->p_ಮುಂದಿನ && p_ಬೀಗ->pBtree==p_ಬೀಗ->p_ಮುಂದಿನ->pBtree ){
      p_ಬೀಗ = p_ಬೀಗ->p_ಮುಂದಿನ;
      ಬರೆ(",%u%s", p_ಬೀಗ->i_ಕೋಷ್ಟಕ, p_ಬೀಗ->e_ಬೀಗ==READ_ಬೀಗ ? "R" : "W");
    }
  }
  ಬರೆ("\n");
  fflush(stdout);
}
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು SHARED_ಬೀಗ_TRACE
#ವ್ಯಾಖ್ಯಾನಿಸು SHARED_ಬೀಗ_TRACE(X,ಸಂದೇಶ,TAB,TYPE)  shared_ಬೀಗ_Trace(X,ಸಂದೇಶ,TAB,TYPE)
#ಆದರೆ_ಅಂತ್ಯ /* Shared-lock tracing */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ has_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(
  Btree *pBtree,         /* Handle that must hold lock */
  Pgno i_ಬೇರು,            /* Root page of b-tree */
  ಪೂರ್ಣಾಂಕ is_ಸೂಚ್ಯಂಕ,           /* True if iRoot is the root of an index b-tree */
  ಪೂರ್ಣಾಂಕ e_ಬೀಗ_Type          /* Required lock type (READ_LOCK or WRITE_LOCK) */
){
  Schema *pSchema = (Schema *)pBtree->pBt->pSchema;
  Pgno iTab = 0;
  Bt_ಬೀಗ *p_ಬೀಗ;

  /* If this database is not shareable, or if the client is reading
  ** and has the read-uncommitted flag set, then no lock is required.
  ** Return true immediately.
  */
  ಆದರೆ( (pBtree->sharable==0)
   || (e_ಬೀಗ_Type==READ_ಬೀಗ && (pBtree->db->flags & SQLITE_ReadUncommit))
  ){
    ಹಿಂತಿರುಗಿ 1;
  }

  /* If the client is reading  or writing an index and the schema is
  ** not loaded, then it is too difficult to actually check to see if
  ** the correct locks are held.  So do not bother - just return true.
  ** This case does not come up very often anyhow.
  */
  ಆದರೆ( is_ಸೂಚ್ಯಂಕ && (!pSchema || (pSchema->schemaFlags&DB_SchemaLoaded)==0) ){
    ಹಿಂತಿರುಗಿ 1;
  }

  /* Figure out the root-page that the lock should be held on. For table
  ** b-trees, this is just the root page of the b-tree being read or
  ** written. For index b-trees, it is the root page of the associated
  ** table.  */
  ಆದರೆ( is_ಸೂಚ್ಯಂಕ ){
    HashElem *p;
    ಪೂರ್ಣಾಂಕ b_ನೋಡಿದ = 0;
    ಗಾಗಿ(p=sqlite_Hash_ಮೊದಲ(&pSchema->idxHash); p; p=sqlite_Hash_ಮುಂದಿನ(p)){
      ಸೂಚ್ಯಂಕ *pIdx = (ಸೂಚ್ಯಂಕ *)sqlite_Hash_ದತ್ತಾಂಶ(p);
      ಆದರೆ( pIdx->tnum==i_ಬೇರು ){
        ಆದರೆ( b_ನೋಡಿದ ){
          /* Two or more indexes share the same root page.  There must
          ** be imposter tables.  So just return true.  The assert is not
          ** useful in that case. */
          ಹಿಂತಿರುಗಿ 1;
        }
        iTab = pIdx->p_ಕೋಷ್ಟಕ->tnum;
        b_ನೋಡಿದ = 1;
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    iTab = i_ಬೇರು;
  }

  SHARED_ಬೀಗ_TRACE(pBtree->pBt,"hasLock",i_ಬೇರು,e_ಬೀಗ_Type);

  /* Search for the required lock. Either a write-lock on root-page iTab, a
  ** write-lock on the schema table, or (if the client is reading) a
  ** read-lock on iTab will suffice. Return 1 if any of these are found.  */
  ಗಾಗಿ(p_ಬೀಗ=pBtree->pBt->p_ಬೀಗ; p_ಬೀಗ; p_ಬೀಗ=p_ಬೀಗ->p_ಮುಂದಿನ){
    ಆದರೆ( p_ಬೀಗ->pBtree==pBtree
     && (p_ಬೀಗ->i_ಕೋಷ್ಟಕ==iTab || (p_ಬೀಗ->e_ಬೀಗ==WRITE_ಬೀಗ && p_ಬೀಗ->i_ಕೋಷ್ಟಕ==1))
     && p_ಬೀಗ->e_ಬೀಗ>=e_ಬೀಗ_Type
    ){
      ಹಿಂತಿರುಗಿ 1;
    }
  }

  /* Failed to find the required lock. */
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hasReadConflicts(Btree *pBtree, Pgno i_ಬೇರು){
  BtCursor *p;
  ಗಾಗಿ(p=pBtree->pBt->pCursor; p; p=p->p_ಮುಂದಿನ){
    ಆದರೆ( p->pgno_ಬೇರು==i_ಬೇರು
     && p->pBtree!=pBtree
     && 0==(p->pBtree->db->flags & SQLITE_ReadUncommit)
    ){
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ    /* #ifdef SQLITE_DEBUG */

/*
** Query to see if Btree handle p may obtain a lock of type eLock
** (READ_LOCK or WRITE_LOCK) on the table with root-page iTab. Return
** SQLITE_OK if the lock may be obtained (by calling
** setSharedCacheTableLock()), or SQLITE_LOCKED if not.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ query_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(Btree *p, Pgno iTab, u8 e_ಬೀಗ){
  BtShared *pBt = p->pBt;
  Bt_ಬೀಗ *pIter;

  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(p) );
  ಪ್ರತಿಜ್ಞಾ( e_ಬೀಗ==READ_ಬೀಗ || e_ಬೀಗ==WRITE_ಬೀಗ );
  ಪ್ರತಿಜ್ಞಾ( p->db!=0 );
  ಪ್ರತಿಜ್ಞಾ( !(p->db->flags&SQLITE_ReadUncommit)||e_ಬೀಗ==WRITE_ಬೀಗ||iTab==1 );
 
  /* If requesting a write-lock, then the Btree must have an open write
  ** transaction on this file. And, obviously, for this to be so there
  ** must be an open write transaction on the file itself.
  */
  ಪ್ರತಿಜ್ಞಾ( e_ಬೀಗ==READ_ಬೀಗ || (p==pBt->pWriter && p->ಒಳ_Trans==TRANS_WRITE) );
  ಪ್ರತಿಜ್ಞಾ( e_ಬೀಗ==READ_ಬೀಗ || pBt->ಒಳ_Transaction==TRANS_WRITE );
 
  /* This routine is a no-op if the shared-cache is not enabled */
  ಆದರೆ( !p->sharable ){
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }

  /* If some other connection is holding an exclusive lock, the
  ** requested lock may not be obtained.
  */
  ಆದರೆ( pBt->pWriter!=p && (pBt->btsFlags & BTS_EXCLUSIVE)!=0 ){
    sqlite3ConnectionBlocked(p->db, pBt->pWriter->db);
    ಹಿಂತಿರುಗಿ SQLITE_LOCKED_SHAREDCACHE;
  }

  ಗಾಗಿ(pIter=pBt->p_ಬೀಗ; pIter; pIter=pIter->p_ಮುಂದಿನ){
    /* The condition (pIter->eLock!=eLock) in the following if(...)
    ** statement is a simplification of:
    **
    **   (eLock==WRITE_LOCK || pIter->eLock==WRITE_LOCK)
    **
    ** since we know that if eLock==WRITE_LOCK, then no other connection
    ** may hold a WRITE_LOCK on any table in this file (since there can
    ** only be a single writer).
    */
    ಪ್ರತಿಜ್ಞಾ( pIter->e_ಬೀಗ==READ_ಬೀಗ || pIter->e_ಬೀಗ==WRITE_ಬೀಗ );
    ಪ್ರತಿಜ್ಞಾ( e_ಬೀಗ==READ_ಬೀಗ || pIter->pBtree==p || pIter->e_ಬೀಗ==READ_ಬೀಗ);
    ಆದರೆ( pIter->pBtree!=p && pIter->i_ಕೋಷ್ಟಕ==iTab && pIter->e_ಬೀಗ!=e_ಬೀಗ ){
      sqlite3ConnectionBlocked(p->db, pIter->pBtree->db);
      ಆದರೆ( e_ಬೀಗ==WRITE_ಬೀಗ ){
        ಪ್ರತಿಜ್ಞಾ( p==pBt->pWriter );
        pBt->btsFlags |= BTS_PENDING;
      }
      ಹಿಂತಿರುಗಿ SQLITE_LOCKED_SHAREDCACHE;
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* !SQLITE_OMIT_SHARED_CACHE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಗಣ_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(Btree *p, Pgno i_ಕೋಷ್ಟಕ, u8 e_ಬೀಗ){
  BtShared *pBt = p->pBt;
  Bt_ಬೀಗ *p_ಬೀಗ = 0;
  Bt_ಬೀಗ *pIter;

  SHARED_ಬೀಗ_TRACE(pBt,"setLock", i_ಕೋಷ್ಟಕ, e_ಬೀಗ);

  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(p) );
  ಪ್ರತಿಜ್ಞಾ( e_ಬೀಗ==READ_ಬೀಗ || e_ಬೀಗ==WRITE_ಬೀಗ );
  ಪ್ರತಿಜ್ಞಾ( p->db!=0 );

  /* A connection with the read-uncommitted flag set will never try to
  ** obtain a read-lock using this function. The only read-lock obtained
  ** by a connection in read-uncommitted mode is on the sqlite_schema
  ** table, and that lock is obtained in BtreeBeginTrans().  */
  ಪ್ರತಿಜ್ಞಾ( 0==(p->db->flags&SQLITE_ReadUncommit) || e_ಬೀಗ==WRITE_ಬೀಗ );

  /* This function should only be called on a sharable b-tree after it
  ** has been determined that no other b-tree holds a conflicting lock.  */
  ಪ್ರತಿಜ್ಞಾ( p->sharable );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_OK==query_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(p, i_ಕೋಷ್ಟಕ, e_ಬೀಗ) );

  /* First search the list for an existing lock on this table. */
  ಗಾಗಿ(pIter=pBt->p_ಬೀಗ; pIter; pIter=pIter->p_ಮುಂದಿನ){
    ಆದರೆ( pIter->i_ಕೋಷ್ಟಕ==i_ಕೋಷ್ಟಕ && pIter->pBtree==p ){
      p_ಬೀಗ = pIter;
      ನಿಲ್ಲಿಸು;
    }
  }

  /* If the above search did not find a BtLock struct associating Btree p
  ** with table iTable, allocate one and link it into the list.
  */
  ಆದರೆ( !p_ಬೀಗ ){
    p_ಬೀಗ = (Bt_ಬೀಗ *)sqlite3MallocZero(ಗಾತ್ರ(Bt_ಬೀಗ));
    ಆದರೆ( !p_ಬೀಗ ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    }
    p_ಬೀಗ->i_ಕೋಷ್ಟಕ = i_ಕೋಷ್ಟಕ;
    p_ಬೀಗ->pBtree = p;
    p_ಬೀಗ->p_ಮುಂದಿನ = pBt->p_ಬೀಗ;
    pBt->p_ಬೀಗ = p_ಬೀಗ;
  }

  /* Set the BtLock.eLock variable to the maximum of the current lock
  ** and the requested lock. This means if a write-lock was already held
  ** and a read-lock requested, we don't incorrectly downgrade the lock.
  */
  ಪ್ರತಿಜ್ಞಾ( WRITE_ಬೀಗ>READ_ಬೀಗ );
  ಆದರೆ( e_ಬೀಗ>p_ಬೀಗ->e_ಬೀಗ ){
    p_ಬೀಗ->e_ಬೀಗ = e_ಬೀಗ;
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* !SQLITE_OMIT_SHARED_CACHE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
/*
** Release all the table locks (locks obtained via calls to
** the setSharedCacheTableLock() procedure) held by Btree object p.
**
** This function assumes that Btree p has an open read or write
** transaction. If it does not, then the BTS_PENDING flag
** may be incorrectly cleared.
*/
ಸ್ಥಿರ ಶೂನ್ಯ clear_All_Shared_Cache_ಕೋಷ್ಟಕ_Locks(Btree *p){
  BtShared *pBt = p->pBt;
  Bt_ಬೀಗ **ppIter = &pBt->p_ಬೀಗ;

  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(p) );
  ಪ್ರತಿಜ್ಞಾ( p->sharable || 0==*ppIter );
  ಪ್ರತಿಜ್ಞಾ( p->ಒಳ_Trans>0 );

  SHARED_ಬೀಗ_TRACE(pBt, "clearAllLocks", 0, 0);

  ಆಗ( *ppIter ){
    Bt_ಬೀಗ *p_ಬೀಗ = *ppIter;
    ಪ್ರತಿಜ್ಞಾ( (pBt->btsFlags & BTS_EXCLUSIVE)==0 || pBt->pWriter==p_ಬೀಗ->pBtree );
    ಪ್ರತಿಜ್ಞಾ( p_ಬೀಗ->pBtree->ಒಳ_Trans>=p_ಬೀಗ->e_ಬೀಗ );
    ಆದರೆ( p_ಬೀಗ->pBtree==p ){
      *ppIter = p_ಬೀಗ->p_ಮುಂದಿನ;
      ಪ್ರತಿಜ್ಞಾ( p_ಬೀಗ->i_ಕೋಷ್ಟಕ!=1 || p_ಬೀಗ==&p->ಬೀಗ );
      ಆದರೆ( p_ಬೀಗ->i_ಕೋಷ್ಟಕ!=1 ){
        sqlite3_free(p_ಬೀಗ);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ppIter = &p_ಬೀಗ->p_ಮುಂದಿನ;
    }
  }

  ಪ್ರತಿಜ್ಞಾ( (pBt->btsFlags & BTS_PENDING)==0 || pBt->pWriter );
  ಆದರೆ( pBt->pWriter==p ){
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(BTS_EXCLUSIVE|BTS_PENDING);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pBt->nTransaction==2 ){
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
ಸ್ಥಿರ ಶೂನ್ಯ downgrade_All_Shared_Cache_ಕೋಷ್ಟಕ_Locks(Btree *p){
  BtShared *pBt = p->pBt;

  SHARED_ಬೀಗ_TRACE(pBt, "downgradeLocks", 0, 0);

  ಆದರೆ( pBt->pWriter==p ){
    Bt_ಬೀಗ *p_ಬೀಗ;
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(BTS_EXCLUSIVE|BTS_PENDING);
    ಗಾಗಿ(p_ಬೀಗ=pBt->p_ಬೀಗ; p_ಬೀಗ; p_ಬೀಗ=p_ಬೀಗ->p_ಮುಂದಿನ){
      ಪ್ರತಿಜ್ಞಾ( p_ಬೀಗ->e_ಬೀಗ==READ_ಬೀಗ || p_ಬೀಗ->pBtree==p );
      p_ಬೀಗ->e_ಬೀಗ = READ_ಬೀಗ;
    }
  }
}

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_SHARED_CACHE */

ಸ್ಥಿರ ಶೂನ್ಯ release_ಪುಟ(Mem_ಪುಟ *p_ಪುಟ);         /* Forward reference */
ಸ್ಥಿರ ಶೂನ್ಯ release_ಪುಟ_One(Mem_ಪುಟ *p_ಪುಟ);      /* Forward reference */
ಸ್ಥಿರ ಶೂನ್ಯ release_ಪುಟ_Not_Null(Mem_ಪುಟ *p_ಪುಟ);  /* Forward reference */

/*
***** This routine is used inside of assert() only ****
**
** Verify that the cursor holds the mutex on its BtShared
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cursorHoldsMutex(BtCursor *p){
  ಹಿಂತಿರುಗಿ sqlite3_mutex_held(p->pBt->mutex);
}

/* Verify that the cursor and the BtShared agree about what is the current
** database connetion. This is important in shared-cache mode. If the database
** connection pointers get out-of-sync, it is possible for routines like
** btreeInitPage() to reference an stale connection pointer that references a
** a connection that has already closed.  This routine is used inside assert()
** statements only and for the purpose of double-checking that the btree code
** does keep the database connection pointers up-to-date.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cursorOwnsBtShared(BtCursor *p){
  ಪ್ರತಿಜ್ಞಾ( cursorHoldsMutex(p) );
  ಹಿಂತಿರುಗಿ (p->pBtree->db==p->pBt->db);
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Invalidate the overflow cache of the cursor passed as the first argument.
** on the shared btree structure pBt.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು invalidate_ಅತಿಪ್ರವಾಹ_Cache(pCur) (pCur->curFlags &= ~BTCF_ValidOvfl)

/*
** Invalidate the overflow page-list cache for all cursors opened
** on the shared btree structure pBt.
*/
ಸ್ಥಿರ ಶೂನ್ಯ invalidate_All_ಅತಿಪ್ರವಾಹ_Cache(BtShared *pBt){
  BtCursor *p;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಗಾಗಿ(p=pBt->pCursor; p; p=p->p_ಮುಂದಿನ){
    invalidate_ಅತಿಪ್ರವಾಹ_Cache(p);
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB
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
ಸ್ಥಿರ ಶೂನ್ಯ invalidateIncrblobCursors(
  Btree *pBtree,          /* The database file to check */
  Pgno pgno_ಬೇರು,          /* The table that might be changing */
  i64 i_ಅಡ್ಡಸಾಲು,               /* The rowid that might be changing */
  ಪೂರ್ಣಾಂಕ is_Clear_ಕೋಷ್ಟಕ        /* True if all rows are being deleted */
){
  BtCursor *p;
  ಪ್ರತಿಜ್ಞಾ( pBtree->hasIncrblobCur );
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(pBtree) );
  pBtree->hasIncrblobCur = 0;
  ಗಾಗಿ(p=pBtree->pBt->pCursor; p; p=p->p_ಮುಂದಿನ){
    ಆದರೆ( (p->curFlags & BTCF_Incrblob)!=0 ){
      pBtree->hasIncrblobCur = 1;
      ಆದರೆ( p->pgno_ಬೇರು==pgno_ಬೇರು && (is_Clear_ಕೋಷ್ಟಕ || p->info.n_ಕೀಲಿ==i_ಅಡ್ಡಸಾಲು) ){
        p->eState = CURSOR_INVALID;
      }
    }
  }
}

#ಇಲ್ಲದಿದ್ದರೆ
  /* Stub function when INCRBLOB is omitted */
  #ವ್ಯಾಖ್ಯಾನಿಸು invalidateIncrblobCursors(w,x,y,z)
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_INCRBLOB */

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btree_ಗಣ_Has_Content(BtShared *pBt, Pgno pgno){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( !pBt->pHasContent ){
    ಪ್ರತಿಜ್ಞಾ( pgno<=pBt->n_ಪುಟ );
    pBt->pHasContent = sqlite3BitvecCreate(pBt->n_ಪುಟ);
    ಆದರೆ( !pBt->pHasContent ){
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  ಆದರೆ( rc==SQLITE_OK && pgno<=sqlite3_Bitvec_ಗಾತ್ರ(pBt->pHasContent) ){
    rc = sqlite3_Bitvec_ಗಣ(pBt->pHasContent, pgno);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Query the BtShared.pHasContent vector.
**
** This function is called when a free-list leaf page is removed from the
** free-list for reuse. It returns false if it is safe to retrieve the
** page from the pager layer with the 'no-content' flag set. True otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btreeGetHasContent(BtShared *pBt, Pgno pgno){
  Bitvec *p = pBt->pHasContent;
  ಹಿಂತಿರುಗಿ p && (pgno>sqlite3_Bitvec_ಗಾತ್ರ(p) || sqlite3BitvecTestNotNull(p, pgno));
}

/*
** Clear (destroy) the BtShared.pHasContent bitvec. This should be
** invoked at the conclusion of each write-transaction.
*/
ಸ್ಥಿರ ಶೂನ್ಯ btreeClearHasContent(BtShared *pBt){
  sqlite3BitvecDestroy(pBt->pHasContent);
  pBt->pHasContent = 0;
}

/*
** Release all of the apPage[] pages for a cursor.
*/
ಸ್ಥಿರ ಶೂನ್ಯ btree_Release_All_Cursor_ಪುಟಗಳು(BtCursor *pCur){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( pCur->i_ಪುಟ>=0 ){
    ಗಾಗಿ(i=0; i<pCur->i_ಪುಟ; i++){
      release_ಪುಟ_Not_Null(pCur->ap_ಪುಟ[i]);
    }
    release_ಪುಟ_Not_Null(pCur->p_ಪುಟ);
    pCur->i_ಪುಟ = -1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ save_Cursor_ಕೀಲಿ(BtCursor *pCur){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪ್ರತಿಜ್ಞಾ( CURSOR_VALID==pCur->eState );
  ಪ್ರತಿಜ್ಞಾ( 0==pCur->p_ಕೀಲಿ );
  ಪ್ರತಿಜ್ಞಾ( cursorHoldsMutex(pCur) );

  ಆದರೆ( pCur->cur_Int_ಕೀಲಿ ){
    /* Only the rowid is required for a table btree */
    pCur->n_ಕೀಲಿ = sqlite3_Btree_Integer_ಕೀಲಿ(pCur);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* For an index btree, save the complete key content. It is possible
    ** that the current key is corrupt. In that case, it is possible that
    ** the sqlite3VdbeRecordUnpack() function may overread the buffer by
    ** up to the size of 1 varint plus 1 8-byte value when the cursor
    ** position is restored. Hence the 17 bytes of padding allocated
    ** below. */
    ಶೂನ್ಯ *p_ಕೀಲಿ;
    pCur->n_ಕೀಲಿ = sqlite3_Btree_Payload_ಗಾತ್ರ(pCur);
    p_ಕೀಲಿ = sqlite3Malloc( ((i64)pCur->n_ಕೀಲಿ) + 9 + 8 );
    ಆದರೆ( p_ಕೀಲಿ ){
      rc = sqlite3BtreePayload(pCur, 0, (ಪೂರ್ಣಾಂಕ)pCur->n_ಕೀಲಿ, p_ಕೀಲಿ);
      ಆದರೆ( rc==SQLITE_OK ){
        memset(((u8*)p_ಕೀಲಿ)+pCur->n_ಕೀಲಿ, 0, 9+8);
        pCur->p_ಕೀಲಿ = p_ಕೀಲಿ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_free(p_ಕೀಲಿ);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( !pCur->cur_Int_ಕೀಲಿ || !pCur->p_ಕೀಲಿ );
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Save the current cursor position in the variables BtCursor.nKey
** and BtCursor.pKey. The cursor's state is set to CURSOR_REQUIRESEEK.
**
** The caller must ensure that the cursor is valid (has eState==CURSOR_VALID)
** prior to calling this routine. 
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ saveCursorPosition(BtCursor *pCur){
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( CURSOR_VALID==pCur->eState || CURSOR_SKIPNEXT==pCur->eState );
  ಪ್ರತಿಜ್ಞಾ( 0==pCur->p_ಕೀಲಿ );
  ಪ್ರತಿಜ್ಞಾ( cursorHoldsMutex(pCur) );

  ಆದರೆ( pCur->curFlags & BTCF_Pinned ){
    ಹಿಂತಿರುಗಿ SQLITE_CONSTRAINT_PINNED;
  }
  ಆದರೆ( pCur->eState==CURSOR_SKIPNEXT ){
    pCur->eState = CURSOR_VALID;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pCur->skip_ಮುಂದಿನ = 0;
  }

  rc = save_Cursor_ಕೀಲಿ(pCur);
  ಆದರೆ( rc==SQLITE_OK ){
    btree_Release_All_Cursor_ಪುಟಗಳು(pCur);
    pCur->eState = CURSOR_REQUIRESEEK;
  }

  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl|BTCF_At_ಕೊನೆಯ);
  ಹಿಂತಿರುಗಿ rc;
}

/* Forward reference */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_NOINLINE saveCursorsOnList(BtCursor*,Pgno,BtCursor*);

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ saveAllCursors(BtShared *pBt, Pgno i_ಬೇರು, BtCursor *pExcept){
  BtCursor *p;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( pExcept==0 || pExcept->pBt==pBt );
  ಗಾಗಿ(p=pBt->pCursor; p; p=p->p_ಮುಂದಿನ){
    ಆದರೆ( p!=pExcept && (0==i_ಬೇರು || p->pgno_ಬೇರು==i_ಬೇರು) ) ನಿಲ್ಲಿಸು;
  }
  ಆದರೆ( p ) ಹಿಂತಿರುಗಿ saveCursorsOnList(p, i_ಬೇರು, pExcept);
  ಆದರೆ( pExcept ) pExcept->curFlags &= ~BTCF_ಅಪವರ್ತ್ಯ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* This helper routine to saveAllCursors does the actual work of saving
** the cursors if and when a cursor is found that actually requires saving.
** The common case is that no cursors need to be saved, so this routine is
** broken out from its caller to avoid unnecessary stack pointer movement.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_NOINLINE saveCursorsOnList(
  BtCursor *p,         /* The first cursor that needs saving */
  Pgno i_ಬೇರು,          /* Only save cursor with this iRoot. Save all if zero */
  BtCursor *pExcept    /* Do not save this cursor */
){
  ಮಾಡು{
    ಆದರೆ( p!=pExcept && (0==i_ಬೇರು || p->pgno_ಬೇರು==i_ಬೇರು) ){
      ಆದರೆ( p->eState==CURSOR_VALID || p->eState==CURSOR_SKIPNEXT ){
        ಪೂರ್ಣಾಂಕ rc = saveCursorPosition(p);
        ಆದರೆ( SQLITE_OK!=rc ){
          ಹಿಂತಿರುಗಿ rc;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        testcase( p->i_ಪುಟ>=0 );
        btree_Release_All_Cursor_ಪುಟಗಳು(p);
      }
    }
    p = p->p_ಮುಂದಿನ;
  }ಆಗ( p );
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Clear the current cursor position.
*/
ಶೂನ್ಯ sqlite3BtreeClearCursor(BtCursor *pCur){
  ಪ್ರತಿಜ್ಞಾ( cursorHoldsMutex(pCur) );
  sqlite3_free(pCur->p_ಕೀಲಿ);
  pCur->p_ಕೀಲಿ = 0;
  pCur->eState = CURSOR_INVALID;
}

/*
** In this version of BtreeMoveto, pKey is a packed index record
** such as is generated by the OP_MakeRecord opcode.  Unpack the
** record and then call sqlite3BtreeIndexMoveto() to do the work.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btreeMoveto(
  BtCursor *pCur,     /* Cursor open on the btree to be searched */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p_ಕೀಲಿ,   /* Packed key if the btree is an index */
  i64 n_ಕೀಲಿ,           /* Integer key for tables.  Size of pKey for indices */
  ಪೂರ್ಣಾಂಕ bias,           /* Bias search to the high end */
  ಪೂರ್ಣಾಂಕ *pRes           /* Write search results here */
){
  ಪೂರ್ಣಾಂಕ rc;                    /* Status code */
  Unpacked_ದಾಖಲೆ *p_Idx_ಕೀಲಿ;   /* Unpacked index key */

  ಆದರೆ( p_ಕೀಲಿ ){
    ಕೀಲಿ_Info *p_ಕೀಲಿ_Info = pCur->p_ಕೀಲಿ_Info;
    ಪ್ರತಿಜ್ಞಾ( n_ಕೀಲಿ==(i64)(ಪೂರ್ಣಾಂಕ)n_ಕೀಲಿ );
    p_Idx_ಕೀಲಿ = sqlite3_Vdbe_Alloc_Unpacked_ದಾಖಲೆ(p_ಕೀಲಿ_Info);
    ಆದರೆ( p_Idx_ಕೀಲಿ==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    sqlite3_Vdbe_ದಾಖಲೆ_Unpack((ಪೂರ್ಣಾಂಕ)n_ಕೀಲಿ, p_ಕೀಲಿ, p_Idx_ಕೀಲಿ);
    ಆದರೆ( p_Idx_ಕೀಲಿ->n_ಕ್ಷೇತ್ರ==0 || p_Idx_ಕೀಲಿ->n_ಕ್ಷೇತ್ರ>p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ ){
      rc = SQLITE_CORRUPT_BKPT;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = sqlite3_Btree_ಸೂಚ್ಯಂಕ_Moveto(pCur, p_Idx_ಕೀಲಿ, pRes);
    }
    sqlite3DbFree(pCur->p_ಕೀಲಿ_Info->db, p_Idx_ಕೀಲಿ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_Idx_ಕೀಲಿ = 0;
    rc = sqlite3_Btree_ಕೋಷ್ಟಕ_Moveto(pCur, n_ಕೀಲಿ, bias, pRes);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Restore the cursor to the position it was in (or as close to as possible)
** when saveCursorPosition() was called. Note that this call deletes the
** saved position info stored by saveCursorPosition(), so there can be
** at most one effective restoreCursorPosition() call after each
** saveCursorPosition().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btreeRestoreCursorPosition(BtCursor *pCur){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ skip_ಮುಂದಿನ = 0;
  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState>=CURSOR_REQUIRESEEK );
  ಆದರೆ( pCur->eState==CURSOR_FAULT ){
    ಹಿಂತಿರುಗಿ pCur->skip_ಮುಂದಿನ;
  }
  pCur->eState = CURSOR_INVALID;
  ಆದರೆ( sqlite3FaultSim(410) ){
    rc = SQLITE_IOERR;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = btreeMoveto(pCur, pCur->p_ಕೀಲಿ, pCur->n_ಕೀಲಿ, 0, &skip_ಮುಂದಿನ);
  }
  ಆದರೆ( rc==SQLITE_OK ){
    sqlite3_free(pCur->p_ಕೀಲಿ);
    pCur->p_ಕೀಲಿ = 0;
    ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID || pCur->eState==CURSOR_INVALID );
    ಆದರೆ( skip_ಮುಂದಿನ ) pCur->skip_ಮುಂದಿನ = skip_ಮುಂದಿನ;
    ಆದರೆ( pCur->skip_ಮುಂದಿನ && pCur->eState==CURSOR_VALID ){
      pCur->eState = CURSOR_SKIPNEXT;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿಸು restoreCursorPosition(p) \
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
ಪೂರ್ಣಾಂಕ sqlite3BtreeCursorHasMoved(BtCursor *pCur){
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(pCur)
       || pCur==sqlite3BtreeFakeValidCursor() );
  ಪ್ರತಿಜ್ಞಾ( offsetof(BtCursor, eState)==0 );
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(pCur->eState)==1 );
  ಹಿಂತಿರುಗಿ CURSOR_VALID != *(u8*)pCur;
}

/*
** Return a pointer to a fake BtCursor object that will always answer
** false to the sqlite3BtreeCursorHasMoved() routine above.  The fake
** cursor returned must not be used with any other Btree interface.
*/
BtCursor *sqlite3BtreeFakeValidCursor(ಶೂನ್ಯ){
  ಸ್ಥಿರ u8 fakeCursor = CURSOR_VALID;
  ಪ್ರತಿಜ್ಞಾ( offsetof(BtCursor, eState)==0 );
  ಹಿಂತಿರುಗಿ (BtCursor*)&fakeCursor;
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
ಪೂರ್ಣಾಂಕ sqlite3BtreeCursorRestore(BtCursor *pCur, ಪೂರ್ಣಾಂಕ *p_Different_ಅಡ್ಡಸಾಲು){
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( pCur!=0 );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState!=CURSOR_VALID );
  rc = restoreCursorPosition(pCur);
  ಆದರೆ( rc ){
    *p_Different_ಅಡ್ಡಸಾಲು = 1;
    ಹಿಂತಿರುಗಿ rc;
  }
  ಆದರೆ( pCur->eState!=CURSOR_VALID ){
    *p_Different_ಅಡ್ಡಸಾಲು = 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    *p_Different_ಅಡ್ಡಸಾಲು = 0;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_CURSOR_HINTS
/*
** Provide hints to the cursor.  The particular hint given (and the type
** and number of the varargs parameters) is determined by the eHintType
** parameter.  See the definitions of the BTREE_HINT_* macros for details.
*/
ಶೂನ್ಯ sqlite3BtreeCursorHint(BtCursor *pCur, ಪೂರ್ಣಾಂಕ eHintType, ...){
  /* Used only by system that substitute their own storage engine */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  va_list ap;
  va_start(ap, eHintType);
  ಆದರೆ( eHintType==BTREE_HINT_RANGE ){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;
    Walker w;
    memset(&w, 0, ಗಾತ್ರ(w));
    w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = sqlite3_Cursor_Range_Hint_ಅಭಿವ್ಯಕ್ತಿ_Check;
    p_ಅಭಿವ್ಯಕ್ತಿ = va_arg(ap, ಅಭಿವ್ಯಕ್ತಿ*);
    w.u.aMem = va_arg(ap, Mem*);
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
    ಪ್ರತಿಜ್ಞಾ( w.u.aMem!=0 );
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p_ಅಭಿವ್ಯಕ್ತಿ);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ALWAYS(eHintType==BTREE_HINT_TABLECURSOR) ){
    BtCursor *pCsr = va_arg(ap, BtCursor*);
    ಪ್ರತಿಜ್ಞಾ( pCur->p_Cursor_Hint_ಕೋಷ್ಟಕ_Cursor==0 
         || pCur->p_Cursor_Hint_ಕೋಷ್ಟಕ_Cursor==pCsr
    );
    ಪ್ರತಿಜ್ಞಾ( pCsr->p_ಕೀಲಿ_Info==0 || CORRUPT_DB );
    pCur->p_Cursor_Hint_ಕೋಷ್ಟಕ_Cursor = pCsr;
  }
  va_end(ap);
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_CURSOR_HINTS */

#ಆದರೆ defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** Return the pointer configured via the BTREE_HINT_TABLECURSOR hint on
** cursor pCsr. This is used from OP_DeferredSeek to assert() that the
** index cursor has been correctly configured with the table cursor.
*/
BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor *pCsr){
  ಹಿಂತಿರುಗಿ pCsr->p_Cursor_Hint_ಕೋಷ್ಟಕ_Cursor;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Provide flag hints to the cursor.
*/
ಶೂನ್ಯ sqlite3BtreeCursorHintFlags(BtCursor *pCur, ಚಿಹ್ನೆರಹಿತ x){
  ಪ್ರತಿಜ್ಞಾ( x==BTREE_SEEK_EQ || x==BTREE_BULKLOAD || x==0 );
  pCur->hints = (u8)x;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
/*
** Given a page number of a regular database page, return the page
** number for the pointer-map page that contains the entry for the
** input page number.
**
** Return 0 (not a valid page) for pgno==1 since there is
** no pointer map associated with page 1.  The integrity_check logic
** requires that ptrmapPageno(*,1)!=1.
*/
ಸ್ಥಿರ Pgno ptrmapPageno(BtShared *pBt, Pgno pgno){
  ಪೂರ್ಣಾಂಕ n_ಪುಟಗಳು_ಪ್ರತಿ_Map_ಪುಟ;
  Pgno iPtrMap, ret;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಆದರೆ( pgno<2 ) ಹಿಂತಿರುಗಿ 0;
  n_ಪುಟಗಳು_ಪ್ರತಿ_Map_ಪುಟ = (pBt->usable_ಗಾತ್ರ/5)+1;
  iPtrMap = (pgno-2)/n_ಪುಟಗಳು_ಪ್ರತಿ_Map_ಪುಟ;
  ret = (iPtrMap*n_ಪುಟಗಳು_ಪ್ರತಿ_Map_ಪುಟ) + 2;
  ಆದರೆ( ret==PENDING_BYTE_ಪುಟ(pBt) ){
    ret++;
  }
  ಹಿಂತಿರುಗಿ ret;
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
ಸ್ಥಿರ ಶೂನ್ಯ ptrmapPut(BtShared *pBt, Pgno ಕೀಲಿ, u8 eType, Pgno ಪೋಷಕ, ಪೂರ್ಣಾಂಕ *pRC){
  Db_ಪುಟ *p_Db_ಪುಟ;  /* The pointer map page */
  u8 *pPtrmap;      /* The pointer map data */
  Pgno iPtrmap;     /* The pointer map page number */
  ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ;       /* Offset in pointer map page */
  ಪೂರ್ಣಾಂಕ rc;           /* Return code from subfunctions */

  ಆದರೆ( *pRC ) ಹಿಂತಿರುಗಿ;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  /* The super-journal page number must never be used as a pointer map page */
  ಪ್ರತಿಜ್ಞಾ( 0==PTRMAP_ISPAGE(pBt, PENDING_BYTE_ಪುಟ(pBt)) );

  ಪ್ರತಿಜ್ಞಾ( pBt->autoVacuum );
  ಆದರೆ( ಕೀಲಿ==0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    ಹಿಂತಿರುಗಿ;
  }
  iPtrmap = PTRMAP_PAGENO(pBt, ಕೀಲಿ);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &p_Db_ಪುಟ, 0);
  ಆದರೆ( rc!=SQLITE_OK ){
    *pRC = rc;
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( ((ಅಕ್ಷರ*)sqlite3PagerGetExtra(p_Db_ಪುಟ))[0]!=0 ){
    /* The first byte of the extra data is the MemPage.isInit byte.
    ** If that byte is set, it means this page is also being used
    ** as a btree page. */
    *pRC = SQLITE_CORRUPT_BKPT;
    ಹೋಗು ptrmap_exit;
  }
  ಸ್ಥಾನಾಂತರ = PTRMAP_PTROFFSET(iPtrmap, ಕೀಲಿ);
  ಆದರೆ( ಸ್ಥಾನಾಂತರ<0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    ಹೋಗು ptrmap_exit;
  }
  ಪ್ರತಿಜ್ಞಾ( ಸ್ಥಾನಾಂತರ <= (ಪೂರ್ಣಾಂಕ)pBt->usable_ಗಾತ್ರ-5 );
  pPtrmap = (u8 *)sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_Db_ಪುಟ);

  ಆದರೆ( eType!=pPtrmap[ಸ್ಥಾನಾಂತರ] || get4byte(&pPtrmap[ಸ್ಥಾನಾಂತರ+1])!=ಪೋಷಕ ){
    TRACE(("PTRMAP_UPDATE: %u->(%u,%u)\n", ಕೀಲಿ, eType, ಪೋಷಕ));
    *pRC= rc = sqlite3PagerWrite(p_Db_ಪುಟ);
    ಆದರೆ( rc==SQLITE_OK ){
      pPtrmap[ಸ್ಥಾನಾಂತರ] = eType;
      put4byte(&pPtrmap[ಸ್ಥಾನಾಂತರ+1], ಪೋಷಕ);
    }
  }

ptrmap_exit:
  sqlite3PagerUnref(p_Db_ಪುಟ);
}

/*
** Read an entry from the pointer map.
**
** This routine retrieves the pointer map entry for page 'key', writing
** the type and parent page number to *pEType and *pPgno respectively.
** An error code is returned if something goes wrong, otherwise SQLITE_OK.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ptrmapGet(BtShared *pBt, Pgno ಕೀಲಿ, u8 *pEType, Pgno *pPgno){
  Db_ಪುಟ *p_Db_ಪುಟ;   /* The pointer map page */
  ಪೂರ್ಣಾಂಕ iPtrmap;       /* Pointer map page index */
  u8 *pPtrmap;       /* Pointer map page data */
  ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ;        /* Offset of entry in pointer map */
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );

  iPtrmap = PTRMAP_PAGENO(pBt, ಕೀಲಿ);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &p_Db_ಪುಟ, 0);
  ಆದರೆ( rc!=0 ){
    ಹಿಂತಿರುಗಿ rc;
  }
  pPtrmap = (u8 *)sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_Db_ಪುಟ);

  ಸ್ಥಾನಾಂತರ = PTRMAP_PTROFFSET(iPtrmap, ಕೀಲಿ);
  ಆದರೆ( ಸ್ಥಾನಾಂತರ<0 ){
    sqlite3PagerUnref(p_Db_ಪುಟ);
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  }
  ಪ್ರತಿಜ್ಞಾ( ಸ್ಥಾನಾಂತರ <= (ಪೂರ್ಣಾಂಕ)pBt->usable_ಗಾತ್ರ-5 );
  ಪ್ರತಿಜ್ಞಾ( pEType!=0 );
  *pEType = pPtrmap[ಸ್ಥಾನಾಂತರ];
  ಆದರೆ( pPgno ) *pPgno = get4byte(&pPtrmap[ಸ್ಥಾನಾಂತರ+1]);

  sqlite3PagerUnref(p_Db_ಪುಟ);
  ಆದರೆ( *pEType<1 || *pEType>5 ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_PGNO(iPtrmap);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

#ಇಲ್ಲದಿದ್ದರೆ /* if defined SQLITE_OMIT_AUTOVACUUM */
  #ವ್ಯಾಖ್ಯಾನಿಸು ptrmapPut(w,x,y,z,rc)
  #ವ್ಯಾಖ್ಯಾನಿಸು ptrmapGet(w,x,y,z) SQLITE_OK
  #ವ್ಯಾಖ್ಯಾನಿಸು ptrmapPutOvflPtr(x, y, z, rc)
#ಆದರೆ_ಅಂತ್ಯ

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
#ವ್ಯಾಖ್ಯಾನಿಸು ಕಂಡುಹಿಡಿ_Cell(P,I) \
  ((P)->a_ದತ್ತಾಂಶ + ((P)->ಮುಖವಾಡ_ಪುಟ & get2byteAligned(&(P)->aCellIdx[2*(I)])))
#ವ್ಯಾಖ್ಯಾನಿಸು ಕಂಡುಹಿಡಿ_Cell_Past_Ptr(P,I) \
  ((P)->a_ದತ್ತಾಂಶ_Ofst + ((P)->ಮುಖವಾಡ_ಪುಟ & get2byteAligned(&(P)->aCellIdx[2*(I)])))


/*
** This is common tail processing for btreeParseCellPtr() and
** btreeParseCellPtrIndex() for the case when the cell does not fit entirely
** on a single B-tree page.  Make necessary adjustments to the CellInfo
** structure.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ btree_Parse_Cell_Adjust_ಗಾತ್ರ_For_ಅತಿಪ್ರವಾಹ(
  Mem_ಪುಟ *p_ಪುಟ,         /* Page containing the cell */
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
  ಪೂರ್ಣಾಂಕ minLocal;  /* Minimum amount of payload held locally */
  ಪೂರ್ಣಾಂಕ maxLocal;  /* Maximum amount of payload held locally */
  ಪೂರ್ಣಾಂಕ surplus;   /* Overflow payload available for local storage */

  minLocal = p_ಪುಟ->minLocal;
  maxLocal = p_ಪುಟ->maxLocal;
  surplus = minLocal + (pInfo->nPayload - minLocal)%(p_ಪುಟ->pBt->usable_ಗಾತ್ರ-4);
  testcase( surplus==maxLocal );
  testcase( surplus==maxLocal+1 );
  ಆದರೆ( surplus <= maxLocal ){
    pInfo->nLocal = (u16)surplus;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pInfo->nLocal = (u16)minLocal;
  }
  pInfo->n_ಗಾತ್ರ = (u16)(&pInfo->pPayload[pInfo->nLocal] - pCell) + 4;
}

/*
** Given a record with nPayload bytes of payload stored within btree
** page pPage, return the number of bytes of payload stored locally.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btree_Payload_ಗಮ್ಯ_Local(Mem_ಪುಟ *p_ಪುಟ, i64 nPayload){
  ಪೂರ್ಣಾಂಕ maxLocal;  /* Maximum amount of payload held locally */
  maxLocal = p_ಪುಟ->maxLocal;
  ಪ್ರತಿಜ್ಞಾ( nPayload>=0 );
  ಆದರೆ( nPayload<=maxLocal ){
    ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)nPayload;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ minLocal;  /* Minimum amount of payload held locally */
    ಪೂರ್ಣಾಂಕ surplus;   /* Overflow payload available for local storage */
    minLocal = p_ಪುಟ->minLocal;
    surplus = (ಪೂರ್ಣಾಂಕ)(minLocal +(nPayload - minLocal)%(p_ಪುಟ->pBt->usable_ಗಾತ್ರ-4));
    ಹಿಂತಿರುಗಿ (surplus <= maxLocal) ? surplus : minLocal;
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
ಸ್ಥಿರ ಶೂನ್ಯ btreeParseCellPtrNoPayload(
  Mem_ಪುಟ *p_ಪುಟ,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->ಎಲೆ==0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ==4 );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEBUG
  ಬಳಸದ_PARAMETER(p_ಪುಟ);
#ಆದರೆ_ಅಂತ್ಯ
  pInfo->n_ಗಾತ್ರ = 4 + sqlite3GetVarint(&pCell[4], (u64*)&pInfo->n_ಕೀಲಿ);
  pInfo->nPayload = 0;
  pInfo->nLocal = 0;
  pInfo->pPayload = 0;
  ಹಿಂತಿರುಗಿ;
}
ಸ್ಥಿರ ಶೂನ್ಯ btreeParseCellPtr(
  Mem_ಪುಟ *p_ಪುಟ,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u64 nPayload;           /* Number of bytes of cell payload */
  u64 i_ಕೀಲಿ;               /* Extracted Key value */

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->ಎಲೆ==0 || p_ಪುಟ->ಎಲೆ==1 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->int_ಕೀಲಿ_ಎಲೆ );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ==0 );
  pIter = pCell;

  /* The next block of code is equivalent to:
  **
  **     pIter += getVarint32(pIter, nPayload);
  **
  ** The code is inlined to avoid a function call.
  */
  nPayload = *pIter;
  ಆದರೆ( nPayload>=0x80 ){
    u8 *p_ಕೊನೆ = &pIter[8];
    nPayload &= 0x7f;
    ಮಾಡು{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }ಆಗ( (*pIter)>=0x80 && pIter<p_ಕೊನೆ );
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
  i_ಕೀಲಿ = *pIter;
  ಆದರೆ( i_ಕೀಲಿ>=0x80 ){
    u8 x;
    i_ಕೀಲಿ = (i_ಕೀಲಿ<<7) ^ (x = *++pIter);
    ಆದರೆ( x>=0x80 ){
      i_ಕೀಲಿ = (i_ಕೀಲಿ<<7) ^ (x = *++pIter);
      ಆದರೆ( x>=0x80 ){
        i_ಕೀಲಿ = (i_ಕೀಲಿ<<7) ^ 0x10204000 ^ (x = *++pIter);
        ಆದರೆ( x>=0x80 ){
          i_ಕೀಲಿ = (i_ಕೀಲಿ<<7) ^ 0x4000 ^ (x = *++pIter);
          ಆದರೆ( x>=0x80 ){
            i_ಕೀಲಿ = (i_ಕೀಲಿ<<7) ^ 0x4000 ^ (x = *++pIter);
            ಆದರೆ( x>=0x80 ){
              i_ಕೀಲಿ = (i_ಕೀಲಿ<<7) ^ 0x4000 ^ (x = *++pIter);
              ಆದರೆ( x>=0x80 ){
                i_ಕೀಲಿ = (i_ಕೀಲಿ<<7) ^ 0x4000 ^ (x = *++pIter);
                ಆದರೆ( x>=0x80 ){
                  i_ಕೀಲಿ = (i_ಕೀಲಿ<<8) ^ 0x8000 ^ (*++pIter);
                }
              }
            }
          }
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        i_ಕೀಲಿ ^= 0x204000;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      i_ಕೀಲಿ ^= 0x4000;
    }
  }
  pIter++;

  pInfo->n_ಕೀಲಿ = *(i64*)&i_ಕೀಲಿ;
  pInfo->nPayload = (u32)nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==p_ಪುಟ->maxLocal );
  testcase( nPayload==(u32)p_ಪುಟ->maxLocal+1 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->maxLocal <= BT_MAX_LOCAL );
  ಆದರೆ( nPayload<=p_ಪುಟ->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->n_ಗಾತ್ರ = (u16)nPayload + (u16)(pIter - pCell);
    ಆದರೆ( pInfo->n_ಗಾತ್ರ<4 ) pInfo->n_ಗಾತ್ರ = 4;
    pInfo->nLocal = (u16)nPayload;
  }ಇಲ್ಲದಿದ್ದರೆ{
    btree_Parse_Cell_Adjust_ಗಾತ್ರ_For_ಅತಿಪ್ರವಾಹ(p_ಪುಟ, pCell, pInfo);
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ btree_Parse_Cell_Ptr_ಸೂಚ್ಯಂಕ(
  Mem_ಪುಟ *p_ಪುಟ,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u32 nPayload;           /* Number of bytes of cell payload */

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->ಎಲೆ==0 || p_ಪುಟ->ಎಲೆ==1 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->int_ಕೀಲಿ_ಎಲೆ==0 );
  pIter = pCell + p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ;
  nPayload = *pIter;
  ಆದರೆ( nPayload>=0x80 ){
    u8 *p_ಕೊನೆ = &pIter[8];
    nPayload &= 0x7f;
    ಮಾಡು{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }ಆಗ( *(pIter)>=0x80 && pIter<p_ಕೊನೆ );
  }
  pIter++;
  pInfo->n_ಕೀಲಿ = nPayload;
  pInfo->nPayload = nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==p_ಪುಟ->maxLocal );
  testcase( nPayload==(u32)p_ಪುಟ->maxLocal+1 );
  ಪ್ರತಿಜ್ಞಾ( nPayload>=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->maxLocal <= BT_MAX_LOCAL );
  ಆದರೆ( nPayload<=p_ಪುಟ->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->n_ಗಾತ್ರ = (u16)nPayload + (u16)(pIter - pCell);
    ಆದರೆ( pInfo->n_ಗಾತ್ರ<4 ) pInfo->n_ಗಾತ್ರ = 4;
    pInfo->nLocal = (u16)nPayload;
  }ಇಲ್ಲದಿದ್ದರೆ{
    btree_Parse_Cell_Adjust_ಗಾತ್ರ_For_ಅತಿಪ್ರವಾಹ(p_ಪುಟ, pCell, pInfo);
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ btreeParseCell(
  Mem_ಪುಟ *p_ಪುಟ,         /* Page containing the cell */
  ಪೂರ್ಣಾಂಕ iCell,              /* The cell index.  First cell is 0 */
  CellInfo *pInfo         /* Fill in this structure */
){
  p_ಪುಟ->xParseCell(p_ಪುಟ, ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, iCell), pInfo);
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
ಸ್ಥಿರ u16 cell_ಗಾತ್ರ_Ptr(Mem_ಪುಟ *p_ಪುಟ, u8 *pCell){
  u8 *pIter = pCell + 4;                   /* For looping over bytes of pCell */
  u8 *p_ಕೊನೆ;                                /* End mark for a varint */
  u32 n_ಗಾತ್ರ;                               /* Size value to return */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_ಪುಟ->xParseCell(p_ಪುಟ, pCell, &debuginfo);
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ==4 );
  n_ಗಾತ್ರ = *pIter;
  ಆದರೆ( n_ಗಾತ್ರ>=0x80 ){
    p_ಕೊನೆ = &pIter[8];
    n_ಗಾತ್ರ &= 0x7f;
    ಮಾಡು{
      n_ಗಾತ್ರ = (n_ಗಾತ್ರ<<7) | (*++pIter & 0x7f);
    }ಆಗ( *(pIter)>=0x80 && pIter<p_ಕೊನೆ );
  }
  pIter++;
  testcase( n_ಗಾತ್ರ==p_ಪುಟ->maxLocal );
  testcase( n_ಗಾತ್ರ==(u32)p_ಪುಟ->maxLocal+1 );
  ಆದರೆ( n_ಗಾತ್ರ<=p_ಪುಟ->maxLocal ){
    n_ಗಾತ್ರ += (u32)(pIter - pCell);
    ಪ್ರತಿಜ್ಞಾ( n_ಗಾತ್ರ>4 );
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ minLocal = p_ಪುಟ->minLocal;
    n_ಗಾತ್ರ = minLocal + (n_ಗಾತ್ರ - minLocal) % (p_ಪುಟ->pBt->usable_ಗಾತ್ರ - 4);
    testcase( n_ಗಾತ್ರ==p_ಪುಟ->maxLocal );
    testcase( n_ಗಾತ್ರ==(u32)p_ಪುಟ->maxLocal+1 );
    ಆದರೆ( n_ಗಾತ್ರ>p_ಪುಟ->maxLocal ){
      n_ಗಾತ್ರ = minLocal;
    }
    n_ಗಾತ್ರ += 4 + (u16)(pIter - pCell);
  }
  ಪ್ರತಿಜ್ಞಾ( n_ಗಾತ್ರ==debuginfo.n_ಗಾತ್ರ || CORRUPT_DB );
  ಹಿಂತಿರುಗಿ (u16)n_ಗಾತ್ರ;
}
ಸ್ಥಿರ u16 cell_ಗಾತ್ರ_Ptr_Idx_ಎಲೆ(Mem_ಪುಟ *p_ಪುಟ, u8 *pCell){
  u8 *pIter = pCell;                       /* For looping over bytes of pCell */
  u8 *p_ಕೊನೆ;                                /* End mark for a varint */
  u32 n_ಗಾತ್ರ;                               /* Size value to return */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_ಪುಟ->xParseCell(p_ಪುಟ, pCell, &debuginfo);
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ==0 );
  n_ಗಾತ್ರ = *pIter;
  ಆದರೆ( n_ಗಾತ್ರ>=0x80 ){
    p_ಕೊನೆ = &pIter[8];
    n_ಗಾತ್ರ &= 0x7f;
    ಮಾಡು{
      n_ಗಾತ್ರ = (n_ಗಾತ್ರ<<7) | (*++pIter & 0x7f);
    }ಆಗ( *(pIter)>=0x80 && pIter<p_ಕೊನೆ );
  }
  pIter++;
  testcase( n_ಗಾತ್ರ==p_ಪುಟ->maxLocal );
  testcase( n_ಗಾತ್ರ==(u32)p_ಪುಟ->maxLocal+1 );
  ಆದರೆ( n_ಗಾತ್ರ<=p_ಪುಟ->maxLocal ){
    n_ಗಾತ್ರ += (u32)(pIter - pCell);
    ಆದರೆ( n_ಗಾತ್ರ<4 ) n_ಗಾತ್ರ = 4;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ minLocal = p_ಪುಟ->minLocal;
    n_ಗಾತ್ರ = minLocal + (n_ಗಾತ್ರ - minLocal) % (p_ಪುಟ->pBt->usable_ಗಾತ್ರ - 4);
    testcase( n_ಗಾತ್ರ==p_ಪುಟ->maxLocal );
    testcase( n_ಗಾತ್ರ==(u32)p_ಪುಟ->maxLocal+1 );
    ಆದರೆ( n_ಗಾತ್ರ>p_ಪುಟ->maxLocal ){
      n_ಗಾತ್ರ = minLocal;
    }
    n_ಗಾತ್ರ += 4 + (u16)(pIter - pCell);
  }
  ಪ್ರತಿಜ್ಞಾ( n_ಗಾತ್ರ==debuginfo.n_ಗಾತ್ರ || CORRUPT_DB );
  ಹಿಂತಿರುಗಿ (u16)n_ಗಾತ್ರ;
}
ಸ್ಥಿರ u16 cell_ಗಾತ್ರ_Ptr_No_Payload(Mem_ಪುಟ *p_ಪುಟ, u8 *pCell){
  u8 *pIter = pCell + 4; /* For looping over bytes of pCell */
  u8 *p_ಕೊನೆ;              /* End mark for a varint */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_ಪುಟ->xParseCell(p_ಪುಟ, pCell, &debuginfo);
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಳಸದ_PARAMETER(p_ಪುಟ);
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ==4 );
  p_ಕೊನೆ = pIter + 9;
  ಆಗ( (*pIter++)&0x80 && pIter<p_ಕೊನೆ );
  ಪ್ರತಿಜ್ಞಾ( debuginfo.n_ಗಾತ್ರ==(u16)(pIter - pCell) || CORRUPT_DB );
  ಹಿಂತಿರುಗಿ (u16)(pIter - pCell);
}
ಸ್ಥಿರ u16 cell_ಗಾತ್ರ_Ptr_ಕೋಷ್ಟಕ_ಎಲೆ(Mem_ಪುಟ *p_ಪುಟ, u8 *pCell){
  u8 *pIter = pCell;   /* For looping over bytes of pCell */
  u8 *p_ಕೊನೆ;            /* End mark for a varint */
  u32 n_ಗಾತ್ರ;           /* Size value to return */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_ಪುಟ->xParseCell(p_ಪುಟ, pCell, &debuginfo);
#ಆದರೆ_ಅಂತ್ಯ

  n_ಗಾತ್ರ = *pIter;
  ಆದರೆ( n_ಗಾತ್ರ>=0x80 ){
    p_ಕೊನೆ = &pIter[8];
    n_ಗಾತ್ರ &= 0x7f;
    ಮಾಡು{
      n_ಗಾತ್ರ = (n_ಗಾತ್ರ<<7) | (*++pIter & 0x7f);
    }ಆಗ( *(pIter)>=0x80 && pIter<p_ಕೊನೆ );
  }
  pIter++;
  /* pIter now points at the 64-bit integer key value, a variable length
  ** integer. The following block moves pIter to point at the first byte
  ** past the end of the key value. */
  ಆದರೆ( (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80 ){ pIter++; }
  testcase( n_ಗಾತ್ರ==p_ಪುಟ->maxLocal );
  testcase( n_ಗಾತ್ರ==(u32)p_ಪುಟ->maxLocal+1 );
  ಆದರೆ( n_ಗಾತ್ರ<=p_ಪುಟ->maxLocal ){
    n_ಗಾತ್ರ += (u32)(pIter - pCell);
    ಆದರೆ( n_ಗಾತ್ರ<4 ) n_ಗಾತ್ರ = 4;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ minLocal = p_ಪುಟ->minLocal;
    n_ಗಾತ್ರ = minLocal + (n_ಗಾತ್ರ - minLocal) % (p_ಪುಟ->pBt->usable_ಗಾತ್ರ - 4);
    testcase( n_ಗಾತ್ರ==p_ಪುಟ->maxLocal );
    testcase( n_ಗಾತ್ರ==(u32)p_ಪುಟ->maxLocal+1 );
    ಆದರೆ( n_ಗಾತ್ರ>p_ಪುಟ->maxLocal ){
      n_ಗಾತ್ರ = minLocal;
    }
    n_ಗಾತ್ರ += 4 + (u16)(pIter - pCell);
  }
  ಪ್ರತಿಜ್ಞಾ( n_ಗಾತ್ರ==debuginfo.n_ಗಾತ್ರ || CORRUPT_DB );
  ಹಿಂತಿರುಗಿ (u16)n_ಗಾತ್ರ;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/* This variation on cellSizePtr() is used inside of assert() statements
** only. */
ಸ್ಥಿರ u16 cell_ಗಾತ್ರ(Mem_ಪುಟ *p_ಪುಟ, ಪೂರ್ಣಾಂಕ iCell){
  ಹಿಂತಿರುಗಿ p_ಪುಟ->x_Cell_ಗಾತ್ರ(p_ಪುಟ, ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, iCell));
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
/*
** The cell pCell is currently part of page pSrc but will ultimately be part
** of pPage.  (pSrc and pPage are often the same.)  If pCell contains a
** pointer to an overflow page, insert an entry into the pointer-map for
** the overflow page that will be valid after pCell has been moved to pPage.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ptrmapPutOvflPtr(Mem_ಪುಟ *p_ಪುಟ, Mem_ಪುಟ *pSrc, u8 *pCell,ಪೂರ್ಣಾಂಕ *pRC){
  CellInfo info;
  ಆದರೆ( *pRC ) ಹಿಂತಿರುಗಿ;
  ಪ್ರತಿಜ್ಞಾ( pCell!=0 );
  p_ಪುಟ->xParseCell(p_ಪುಟ, pCell, &info);
  ಆದರೆ( info.nLocal<info.nPayload ){
    Pgno ovfl;
    ಆದರೆ( SQLITE_ಅತಿಪ್ರವಾಹ(pSrc->a_ದತ್ತಾಂಶ_ಕೊನೆ, pCell, pCell+info.nLocal) ){
      testcase( pSrc!=p_ಪುಟ );
      *pRC = SQLITE_CORRUPT_BKPT;
      ಹಿಂತಿರುಗಿ;
    }
    ovfl = get4byte(&pCell[info.n_ಗಾತ್ರ-4]);
    ptrmapPut(p_ಪುಟ->pBt, ovfl, PTRMAP_OVERFLOW1, p_ಪುಟ->pgno, pRC);
  }
}
#ಆದರೆ_ಅಂತ್ಯ


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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ defragment_ಪುಟ(Mem_ಪುಟ *p_ಪುಟ, ಪೂರ್ಣಾಂಕ nMaxFrag){
  ಪೂರ್ಣಾಂಕ i;                     /* Loop counter */
  ಪೂರ್ಣಾಂಕ pc;                    /* Address of the i-th cell */
  ಪೂರ್ಣಾಂಕ hdr;                   /* Offset to the page header */
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2;                  /* Size of a cell */
  ಪೂರ್ಣಾಂಕ usable_ಗಾತ್ರ;            /* Number of usable bytes on a page */
  ಪೂರ್ಣಾಂಕ cell_ಸ್ಥಾನಾಂತರ;            /* Offset to the cell pointer array */
  ಪೂರ್ಣಾಂಕ cbrk;                  /* Offset to the cell content area */
  ಪೂರ್ಣಾಂಕ nCell;                 /* Number of cells on the page */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ದತ್ತಾಂಶ;       /* The page data */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ತಾತ್ಕಾಲಿಕ;       /* Temp area for cell content */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *src;        /* Source of content */
  ಪೂರ್ಣಾಂಕ i_Cell_ಮೊದಲ;            /* First allowable cell index */
  ಪೂರ್ಣಾಂಕ i_Cell_ಕೊನೆಯ;             /* Last possible cell index */
  ಪೂರ್ಣಾಂಕ i_Cell_ಆರಂಭ;            /* First cell offset in input */

  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಪುಟ->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pBt!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pBt->usable_ಗಾತ್ರ <= SQLITE_MAX_ಪುಟ_ಗಾತ್ರ );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ==0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ದತ್ತಾಂಶ = p_ಪುಟ->a_ದತ್ತಾಂಶ;
  hdr = p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ;
  cell_ಸ್ಥಾನಾಂತರ = p_ಪುಟ->cell_ಸ್ಥಾನಾಂತರ;
  nCell = p_ಪುಟ->nCell;
  ಪ್ರತಿಜ್ಞಾ( nCell==get2byte(&ದತ್ತಾಂಶ[hdr+3]) || CORRUPT_DB );
  i_Cell_ಮೊದಲ = cell_ಸ್ಥಾನಾಂತರ + 2*nCell;
  usable_ಗಾತ್ರ = p_ಪುಟ->pBt->usable_ಗಾತ್ರ;

  /* This block handles pages with two or fewer free blocks and nMaxFrag
  ** or fewer fragmented bytes. In this case it is faster to move the
  ** two (or one) blocks of cells using memmove() and add the required
  ** offsets to each pointer in the cell-pointer array than it is to
  ** reconstruct the entire page.  */
  ಆದರೆ( (ಪೂರ್ಣಾಂಕ)ದತ್ತಾಂಶ[hdr+7]<=nMaxFrag ){
    ಪೂರ್ಣಾಂಕ iFree = get2byte(&ದತ್ತಾಂಶ[hdr+1]);
    ಆದರೆ( iFree>usable_ಗಾತ್ರ-4 ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    ಆದರೆ( iFree ){
      ಪೂರ್ಣಾಂಕ iFree2 = get2byte(&ದತ್ತಾಂಶ[iFree]);
      ಆದರೆ( iFree2>usable_ಗಾತ್ರ-4 ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
      ಆದರೆ( 0==iFree2 || (ದತ್ತಾಂಶ[iFree2]==0 && ದತ್ತಾಂಶ[iFree2+1]==0) ){
        u8 *p_ಕೊನೆ = &ದತ್ತಾಂಶ[cell_ಸ್ಥಾನಾಂತರ + nCell*2];
        u8 *pAddr;
        ಪೂರ್ಣಾಂಕ sz2 = 0;
        ಪೂರ್ಣಾಂಕ sz = get2byte(&ದತ್ತಾಂಶ[iFree+2]);
        ಪೂರ್ಣಾಂಕ ತುದಿ = get2byte(&ದತ್ತಾಂಶ[hdr+5]);
        ಆದರೆ( ತುದಿ>=iFree ){
          ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
        }
        ಆದರೆ( iFree2 ){
          ಆದರೆ( iFree+sz>iFree2 ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
          sz2 = get2byte(&ದತ್ತಾಂಶ[iFree2+2]);
          ಆದರೆ( iFree2+sz2 > usable_ಗಾತ್ರ ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
          memmove(&ದತ್ತಾಂಶ[iFree+sz+sz2], &ದತ್ತಾಂಶ[iFree+sz], iFree2-(iFree+sz));
          sz += sz2;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( iFree+sz>usable_ಗಾತ್ರ ){
          ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
        }

        cbrk = ತುದಿ+sz;
        ಪ್ರತಿಜ್ಞಾ( cbrk+(iFree-ತುದಿ) <= usable_ಗಾತ್ರ );
        memmove(&ದತ್ತಾಂಶ[cbrk], &ದತ್ತಾಂಶ[ತುದಿ], iFree-ತುದಿ);
        ಗಾಗಿ(pAddr=&ದತ್ತಾಂಶ[cell_ಸ್ಥಾನಾಂತರ]; pAddr<p_ಕೊನೆ; pAddr+=2){
          pc = get2byte(pAddr);
          ಆದರೆ( pc<iFree ){ put2byte(pAddr, pc+sz); }
          ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pc<iFree2 ){ put2byte(pAddr, pc+sz2); }
        }
        ಹೋಗು defragment_ಹೊರಹೋಗು;
      }
    }
  }

  cbrk = usable_ಗಾತ್ರ;
  i_Cell_ಕೊನೆಯ = usable_ಗಾತ್ರ - 4;
  i_Cell_ಆರಂಭ = get2byte(&ದತ್ತಾಂಶ[hdr+5]);
  ಆದರೆ( nCell>0 ){
    ತಾತ್ಕಾಲಿಕ = sqlite3_Pager_ತಾತ್ಕಾಲಿಕ_Space(p_ಪುಟ->pBt->pPager);
    ಸ್ಮರಣೆ_ನಕಲು(ತಾತ್ಕಾಲಿಕ, ದತ್ತಾಂಶ, usable_ಗಾತ್ರ);
    src = ತಾತ್ಕಾಲಿಕ;
    ಗಾಗಿ(i=0; i<nCell; i++){
      u8 *pAddr;     /* The i-th cell pointer */
      pAddr = &ದತ್ತಾಂಶ[cell_ಸ್ಥಾನಾಂತರ + i*2];
      pc = get2byte(pAddr);
      testcase( pc==i_Cell_ಮೊದಲ );
      testcase( pc==i_Cell_ಕೊನೆಯ );
      /* These conditions have already been verified in btreeInitPage()
      ** if PRAGMA cell_size_check=ON.
      */
      ಆದರೆ( pc>i_Cell_ಕೊನೆಯ ){
        ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
      }
      ಪ್ರತಿಜ್ಞಾ( pc>=0 && pc<=i_Cell_ಕೊನೆಯ );
      ಗಾತ್ರ_2 = p_ಪುಟ->x_Cell_ಗಾತ್ರ(p_ಪುಟ, &src[pc]);
      cbrk -= ಗಾತ್ರ_2;
      ಆದರೆ( cbrk<i_Cell_ಆರಂಭ || pc+ಗಾತ್ರ_2>usable_ಗಾತ್ರ ){
        ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
      }
      ಪ್ರತಿಜ್ಞಾ( cbrk+ಗಾತ್ರ_2<=usable_ಗಾತ್ರ && cbrk>=i_Cell_ಆರಂಭ );
      testcase( cbrk+ಗಾತ್ರ_2==usable_ಗಾತ್ರ );
      testcase( pc+ಗಾತ್ರ_2==usable_ಗಾತ್ರ );
      put2byte(pAddr, cbrk);
      ಸ್ಮರಣೆ_ನಕಲು(&ದತ್ತಾಂಶ[cbrk], &src[pc], ಗಾತ್ರ_2);
    }
  }
  ದತ್ತಾಂಶ[hdr+7] = 0;

defragment_ಹೊರಹೋಗು:
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nFree>=0 );
  ಆದರೆ( ದತ್ತಾಂಶ[hdr+7]+cbrk-i_Cell_ಮೊದಲ!=p_ಪುಟ->nFree ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
  }
  ಪ್ರತಿಜ್ಞಾ( cbrk>=i_Cell_ಮೊದಲ );
  put2byte(&ದತ್ತಾಂಶ[hdr+5], cbrk);
  ದತ್ತಾಂಶ[hdr+1] = 0;
  ದತ್ತಾಂಶ[hdr+2] = 0;
  memset(&ದತ್ತಾಂಶ[i_Cell_ಮೊದಲ], 0, cbrk-i_Cell_ಮೊದಲ);
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಪುಟ->p_Db_ಪುಟ) );
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ u8 *ಪುಟ_ಕಂಡುಹಿಡಿ_ಸ್ಥಾನ(Mem_ಪುಟ *pPg, ಪೂರ್ಣಾಂಕ nByte, ಪೂರ್ಣಾಂಕ *pRc){
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ hdr = pPg->hdr_ಸ್ಥಾನಾಂತರ;            /* Offset to page header */
  u8 * ಸ್ಥಿರಾಂಕ a_ದತ್ತಾಂಶ = pPg->a_ದತ್ತಾಂಶ;             /* Page data */
  ಪೂರ್ಣಾಂಕ iAddr = hdr + 1;                       /* Address of ptr to pc */
  u8 *pTmp = &a_ದತ್ತಾಂಶ[iAddr];                  /* Temporary ptr into aData[] */
  ಪೂರ್ಣಾಂಕ pc = get2byte(pTmp);                   /* Address of a free slot */
  ಪೂರ್ಣಾಂಕ x;                                     /* Excess size of the slot */
  ಪೂರ್ಣಾಂಕ maxPC = pPg->pBt->usable_ಗಾತ್ರ - nByte;  /* Max address for a usable slot */
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2;                                  /* Size of the free slot */

  ಪ್ರತಿಜ್ಞಾ( pc>0 );
  ಆಗ( pc<=maxPC ){
    /* EVIDENCE-OF: R-22710-53328 The third and fourth bytes of each
    ** freeblock form a big-endian integer which is the size of the freeblock
    ** in bytes, including the 4-byte header. */
    pTmp = &a_ದತ್ತಾಂಶ[pc+2];
    ಗಾತ್ರ_2 = get2byte(pTmp);
    ಆದರೆ( (x = ಗಾತ್ರ_2 - nByte)>=0 ){
      testcase( x==4 );
      testcase( x==3 );
      ಆದರೆ( x<4 ){
        /* EVIDENCE-OF: R-11498-58022 In a well-formed b-tree page, the total
        ** number of bytes in fragments may not exceed 60. */
        ಆದರೆ( a_ದತ್ತಾಂಶ[hdr+7]>57 ) ಹಿಂತಿರುಗಿ 0;

        /* Remove the slot from the free-list. Update the number of
        ** fragmented bytes within the page. */
        ಸ್ಮರಣೆ_ನಕಲು(&a_ದತ್ತಾಂಶ[iAddr], &a_ದತ್ತಾಂಶ[pc], 2);
        a_ದತ್ತಾಂಶ[hdr+7] += (u8)x;
        ಹಿಂತಿರುಗಿ &a_ದತ್ತಾಂಶ[pc];
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( x+pc > maxPC ){
        /* This slot extends off the end of the usable part of the page */
        *pRc = SQLITE_CORRUPT_ಪುಟ(pPg);
        ಹಿಂತಿರುಗಿ 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* The slot remains on the free-list. Reduce its size to account
        ** for the portion used by the new allocation. */
        put2byte(&a_ದತ್ತಾಂಶ[pc+2], x);
      }
      ಹಿಂತಿರುಗಿ &a_ದತ್ತಾಂಶ[pc + x];
    }
    iAddr = pc;
    pTmp = &a_ದತ್ತಾಂಶ[pc];
    pc = get2byte(pTmp);
    ಆದರೆ( pc<=iAddr ){
      ಆದರೆ( pc ){
        /* The next slot in the chain comes before the current slot */
        *pRc = SQLITE_CORRUPT_ಪುಟ(pPg);
      }
      ಹಿಂತಿರುಗಿ 0;
    }
  }
  ಆದರೆ( pc>maxPC+nByte-4 ){
    /* The free slot chain extends off the end of the page */
    *pRc = SQLITE_CORRUPT_ಪುಟ(pPg);
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ SQLITE_INLINE ಪೂರ್ಣಾಂಕ allocateSpace(Mem_ಪುಟ *p_ಪುಟ, ಪೂರ್ಣಾಂಕ nByte, ಪೂರ್ಣಾಂಕ *pIdx){
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ hdr = p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ;    /* Local cache of pPage->hdrOffset */
  u8 * ಸ್ಥಿರಾಂಕ ದತ್ತಾಂಶ = p_ಪುಟ->a_ದತ್ತಾಂಶ;      /* Local cache of pPage->aData */
  ಪೂರ್ಣಾಂಕ ತುದಿ;                             /* First byte of cell content area */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;                  /* Integer return code */
  u8 *pTmp;                            /* Temp ptr into data[] */
  ಪೂರ್ಣಾಂಕ gap;        /* First byte of gap between cell pointers and cell content */
 
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಪುಟ->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pBt );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( nByte>=0 );  /* Minimum cell size is 4 */
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nFree>=nByte );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ==0 );
  ಪ್ರತಿಜ್ಞಾ( nByte < (ಪೂರ್ಣಾಂಕ)(p_ಪುಟ->pBt->usable_ಗಾತ್ರ-8) );

  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->cell_ಸ್ಥಾನಾಂತರ == hdr + 12 - 4*p_ಪುಟ->ಎಲೆ );
  gap = p_ಪುಟ->cell_ಸ್ಥಾನಾಂತರ + 2*p_ಪುಟ->nCell;
  ಪ್ರತಿಜ್ಞಾ( gap<=65536 );
  /* EVIDENCE-OF: R-29356-02391 If the database uses a 65536-byte page size
  ** and the reserved space is zero (the usual value for reserved space)
  ** then the cell content offset of an empty page wants to be 65536.
  ** However, that integer is too large to be stored in a 2-byte unsigned
  ** integer, so a value of 0 is used in its place. */
  pTmp = &ದತ್ತಾಂಶ[hdr+5];
  ತುದಿ = get2byte(pTmp);
  ಆದರೆ( gap>ತುದಿ ){
    ಆದರೆ( ತುದಿ==0 && p_ಪುಟ->pBt->usable_ಗಾತ್ರ==65536 ){
      ತುದಿ = 65536;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ತುದಿ>(ಪೂರ್ಣಾಂಕ)p_ಪುಟ->pBt->usable_ಗಾತ್ರ ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
  }

  /* If there is enough space between gap and top for one more cell pointer,
  ** and if the freelist is not empty, then search the
  ** freelist looking for a slot big enough to satisfy the request.
  */
  testcase( gap+2==ತುದಿ );
  testcase( gap+1==ತುದಿ );
  testcase( gap==ತುದಿ );
  ಆದರೆ( (ದತ್ತಾಂಶ[hdr+2] || ದತ್ತಾಂಶ[hdr+1]) && gap+2<=ತುದಿ ){
    u8 *pSpace = ಪುಟ_ಕಂಡುಹಿಡಿ_ಸ್ಥಾನ(p_ಪುಟ, nByte, &rc);
    ಆದರೆ( pSpace ){
      ಪೂರ್ಣಾಂಕ g2;
      ಪ್ರತಿಜ್ಞಾ( pSpace+nByte<=ದತ್ತಾಂಶ+p_ಪುಟ->pBt->usable_ಗಾತ್ರ );
      *pIdx = g2 = (ಪೂರ್ಣಾಂಕ)(pSpace-ದತ್ತಾಂಶ);
      ಆದರೆ( g2<=gap ){
        ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಹಿಂತಿರುಗಿ SQLITE_OK;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc ){
      ಹಿಂತಿರುಗಿ rc;
    }
  }

  /* The request could not be fulfilled using a freelist slot.  Check
  ** to see if defragmentation is necessary.
  */
  testcase( gap+2+nByte==ತುದಿ );
  ಆದರೆ( gap+2+nByte>ತುದಿ ){
    ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nCell>0 || CORRUPT_DB );
    ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nFree>=0 );
    rc = defragment_ಪುಟ(p_ಪುಟ, MIN(4, p_ಪುಟ->nFree - (2+nByte)));
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    ತುದಿ = get2byteNotZero(&ದತ್ತಾಂಶ[hdr+5]);
    ಪ್ರತಿಜ್ಞಾ( gap+2+nByte<=ತುದಿ );
  }


  /* Allocate memory from the gap in between the cell pointer array
  ** and the cell content area.  The btreeComputeFreeSpace() call has already
  ** validated the freelist.  Given that the freelist is valid, there
  ** is no way that the allocation can extend off the end of the page.
  ** The assert() below verifies the previous sentence.
  */
  ತುದಿ -= nByte;
  put2byte(&ದತ್ತಾಂಶ[hdr+5], ತುದಿ);
  ಪ್ರತಿಜ್ಞಾ( ತುದಿ+nByte <= (ಪೂರ್ಣಾಂಕ)p_ಪುಟ->pBt->usable_ಗಾತ್ರ );
  *pIdx = ತುದಿ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ freeSpace(Mem_ಪುಟ *p_ಪುಟ, ಪೂರ್ಣಾಂಕ i_ಆರಂಭ, ಪೂರ್ಣಾಂಕ i_ಗಾತ್ರ){
  ಪೂರ್ಣಾಂಕ iPtr;                             /* Address of ptr to next freeblock */
  ಪೂರ್ಣಾಂಕ iFreeBlk;                         /* Address of the next freeblock */
  u8 hdr;                               /* Page header size.  0 or 100 */
  ಪೂರ್ಣಾಂಕ nFrag = 0;                        /* Reduction in fragmentation */
  ಪೂರ್ಣಾಂಕ i_Orig_ಗಾತ್ರ = i_ಗಾತ್ರ;                /* Original value of iSize */
  ಪೂರ್ಣಾಂಕ x;                                /* Offset to cell content area */
  ಪೂರ್ಣಾಂಕ i_ಕೊನೆ = i_ಆರಂಭ + i_ಗಾತ್ರ;            /* First byte past the iStart buffer */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ದತ್ತಾಂಶ = p_ಪುಟ->a_ದತ್ತಾಂಶ;   /* Page content */
  u8 *pTmp;                             /* Temporary ptr into data[] */

  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pBt!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಪುಟ->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB || i_ಆರಂಭ>=p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+6+p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ );
  ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB || i_ಕೊನೆ <= (ಪೂರ್ಣಾಂಕ)p_ಪುಟ->pBt->usable_ಗಾತ್ರ );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( i_ಗಾತ್ರ>=4 );   /* Minimum cell size is 4 */
  ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB || i_ಆರಂಭ<=(ಪೂರ್ಣಾಂಕ)p_ಪುಟ->pBt->usable_ಗಾತ್ರ-4 );

  /* The list of freeblocks must be in ascending order.  Find the
  ** spot on the list where iStart should be inserted.
  */
  hdr = p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ;
  iPtr = hdr + 1;
  ಆದರೆ( ದತ್ತಾಂಶ[iPtr+1]==0 && ದತ್ತಾಂಶ[iPtr]==0 ){
    iFreeBlk = 0;  /* Shortcut for the case when the freelist is empty */
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆಗ( (iFreeBlk = get2byte(&ದತ್ತಾಂಶ[iPtr]))<i_ಆರಂಭ ){
      ಆದರೆ( iFreeBlk<=iPtr ){
        ಆದರೆ( iFreeBlk==0 ) ನಿಲ್ಲಿಸು; /* TH3: corrupt082.100 */
        ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
      }
      iPtr = iFreeBlk;
    }
    ಆದರೆ( iFreeBlk>(ಪೂರ್ಣಾಂಕ)p_ಪುಟ->pBt->usable_ಗಾತ್ರ-4 ){ /* TH3: corrupt081.100 */
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }
    ಪ್ರತಿಜ್ಞಾ( iFreeBlk>iPtr || iFreeBlk==0 || CORRUPT_DB );
 
    /* At this point:
    **    iFreeBlk:   First freeblock after iStart, or zero if none
    **    iPtr:       The address of a pointer to iFreeBlk
    **
    ** Check to see if iFreeBlk should be coalesced onto the end of iStart.
    */
    ಆದರೆ( iFreeBlk && i_ಕೊನೆ+3>=iFreeBlk ){
      nFrag = iFreeBlk - i_ಕೊನೆ;
      ಆದರೆ( i_ಕೊನೆ>iFreeBlk ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
      i_ಕೊನೆ = iFreeBlk + get2byte(&ದತ್ತಾಂಶ[iFreeBlk+2]);
      ಆದರೆ( i_ಕೊನೆ > (ಪೂರ್ಣಾಂಕ)p_ಪುಟ->pBt->usable_ಗಾತ್ರ ){
        ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
      }
      i_ಗಾತ್ರ = i_ಕೊನೆ - i_ಆರಂಭ;
      iFreeBlk = get2byte(&ದತ್ತಾಂಶ[iFreeBlk]);
    }
 
    /* If iPtr is another freeblock (that is, if iPtr is not the freelist
    ** pointer in the page header) then check to see if iStart should be
    ** coalesced onto the end of iPtr.
    */
    ಆದರೆ( iPtr>hdr+1 ){
      ಪೂರ್ಣಾಂಕ i_Ptr_ಕೊನೆ = iPtr + get2byte(&ದತ್ತಾಂಶ[iPtr+2]);
      ಆದರೆ( i_Ptr_ಕೊನೆ+3>=i_ಆರಂಭ ){
        ಆದರೆ( i_Ptr_ಕೊನೆ>i_ಆರಂಭ ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
        nFrag += i_ಆರಂಭ - i_Ptr_ಕೊನೆ;
        i_ಗಾತ್ರ = i_ಕೊನೆ - iPtr;
        i_ಆರಂಭ = iPtr;
      }
    }
    ಆದರೆ( nFrag>ದತ್ತಾಂಶ[hdr+7] ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    ದತ್ತಾಂಶ[hdr+7] -= (u8)nFrag;
  }
  pTmp = &ದತ್ತಾಂಶ[hdr+5];
  x = get2byte(pTmp);
  ಆದರೆ( p_ಪುಟ->pBt->btsFlags & BTS_FAST_SECURE ){
    /* Overwrite deleted information with zeros when the secure_delete
    ** option is enabled */
    memset(&ದತ್ತಾಂಶ[i_ಆರಂಭ], 0, i_ಗಾತ್ರ);
  }
  ಆದರೆ( i_ಆರಂಭ<=x ){
    /* The new freeblock is at the beginning of the cell content area,
    ** so just extend the cell content area rather than create another
    ** freelist entry */
    ಆದರೆ( i_ಆರಂಭ<x ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    ಆದರೆ( iPtr!=hdr+1 ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    put2byte(&ದತ್ತಾಂಶ[hdr+1], iFreeBlk);
    put2byte(&ದತ್ತಾಂಶ[hdr+5], i_ಕೊನೆ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Insert the new freeblock into the freelist */
    put2byte(&ದತ್ತಾಂಶ[iPtr], i_ಆರಂಭ);
    put2byte(&ದತ್ತಾಂಶ[i_ಆರಂಭ], iFreeBlk);
    ಪ್ರತಿಜ್ಞಾ( i_ಗಾತ್ರ>=0 && i_ಗಾತ್ರ<=0xffff );
    put2byte(&ದತ್ತಾಂಶ[i_ಆರಂಭ+2], (u16)i_ಗಾತ್ರ);
  }
  p_ಪುಟ->nFree += i_Orig_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ decodeFlags(Mem_ಪುಟ *p_ಪುಟ, ಪೂರ್ಣಾಂಕ flagByte){
  BtShared *pBt;     /* A copy of pPage->pBt */

  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ==(p_ಪುಟ->pgno==1 ? 100 : 0) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  pBt = p_ಪುಟ->pBt;
  p_ಪುಟ->max1bytePayload = pBt->max1bytePayload;
  ಆದರೆ( flagByte>=(PTF_ZERODATA | PTF_ಎಲೆ) ){
    p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ = 0;
    p_ಪುಟ->ಎಲೆ = 1;
    ಆದರೆ( flagByte==(PTF_LEAFDATA | PTF_INTKEY | PTF_ಎಲೆ) ){
      p_ಪುಟ->int_ಕೀಲಿ_ಎಲೆ = 1;
      p_ಪುಟ->x_Cell_ಗಾತ್ರ = cell_ಗಾತ್ರ_Ptr_ಕೋಷ್ಟಕ_ಎಲೆ;
      p_ಪುಟ->xParseCell = btreeParseCellPtr;
      p_ಪುಟ->int_ಕೀಲಿ = 1;
      p_ಪುಟ->maxLocal = pBt->max_ಎಲೆ;
      p_ಪುಟ->minLocal = pBt->min_ಎಲೆ;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flagByte==(PTF_ZERODATA | PTF_ಎಲೆ) ){
      p_ಪುಟ->int_ಕೀಲಿ = 0;
      p_ಪುಟ->int_ಕೀಲಿ_ಎಲೆ = 0;
      p_ಪುಟ->x_Cell_ಗಾತ್ರ = cell_ಗಾತ್ರ_Ptr_Idx_ಎಲೆ;
      p_ಪುಟ->xParseCell = btree_Parse_Cell_Ptr_ಸೂಚ್ಯಂಕ;
      p_ಪುಟ->maxLocal = pBt->maxLocal;
      p_ಪುಟ->minLocal = pBt->minLocal;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಪುಟ->int_ಕೀಲಿ = 0;
      p_ಪುಟ->int_ಕೀಲಿ_ಎಲೆ = 0;
      p_ಪುಟ->x_Cell_ಗಾತ್ರ = cell_ಗಾತ್ರ_Ptr_Idx_ಎಲೆ;
      p_ಪುಟ->xParseCell = btree_Parse_Cell_Ptr_ಸೂಚ್ಯಂಕ;
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ = 4;
    p_ಪುಟ->ಎಲೆ = 0;
    ಆದರೆ( flagByte==(PTF_ZERODATA) ){
      p_ಪುಟ->int_ಕೀಲಿ = 0;
      p_ಪುಟ->int_ಕೀಲಿ_ಎಲೆ = 0;
      p_ಪುಟ->x_Cell_ಗಾತ್ರ = cell_ಗಾತ್ರ_Ptr;
      p_ಪುಟ->xParseCell = btree_Parse_Cell_Ptr_ಸೂಚ್ಯಂಕ;
      p_ಪುಟ->maxLocal = pBt->maxLocal;
      p_ಪುಟ->minLocal = pBt->minLocal;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( flagByte==(PTF_LEAFDATA | PTF_INTKEY) ){
      p_ಪುಟ->int_ಕೀಲಿ_ಎಲೆ = 0;
      p_ಪುಟ->x_Cell_ಗಾತ್ರ = cell_ಗಾತ್ರ_Ptr_No_Payload;
      p_ಪುಟ->xParseCell = btreeParseCellPtrNoPayload;
      p_ಪುಟ->int_ಕೀಲಿ = 1;
      p_ಪುಟ->maxLocal = pBt->max_ಎಲೆ;
      p_ಪುಟ->minLocal = pBt->min_ಎಲೆ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಪುಟ->int_ಕೀಲಿ = 0;
      p_ಪುಟ->int_ಕೀಲಿ_ಎಲೆ = 0;
      p_ಪುಟ->x_Cell_ಗಾತ್ರ = cell_ಗಾತ್ರ_Ptr;
      p_ಪುಟ->xParseCell = btree_Parse_Cell_Ptr_ಸೂಚ್ಯಂಕ;
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Compute the amount of freespace on the page.  In other words, fill
** in the pPage->nFree field.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btreeComputeFreeSpace(Mem_ಪುಟ *p_ಪುಟ){
  ಪೂರ್ಣಾಂಕ pc;            /* Address of a freeblock within pPage->aData[] */
  u8 hdr;            /* Offset to beginning of page header */
  u8 *ದತ್ತಾಂಶ;          /* Equal to pPage->aData */
  ಪೂರ್ಣಾಂಕ usable_ಗಾತ್ರ;    /* Amount of usable space on each page */
  ಪೂರ್ಣಾಂಕ nFree;         /* Number of unused bytes on the page */
  ಪೂರ್ಣಾಂಕ ತುದಿ;           /* First byte of the cell content area */
  ಪೂರ್ಣಾಂಕ i_Cell_ಮೊದಲ;    /* First allowable cell or freeblock offset */
  ಪೂರ್ಣಾಂಕ i_Cell_ಕೊನೆಯ;     /* Last possible cell or freeblock offset */

  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pBt!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pBt->db!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pgno==sqlite3PagerPagenumber(p_ಪುಟ->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ == sqlite3PagerGetExtra(p_ಪುಟ->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->a_ದತ್ತಾಂಶ == sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_ಪುಟ->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->isInit==1 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nFree<0 );

  usable_ಗಾತ್ರ = p_ಪುಟ->pBt->usable_ಗಾತ್ರ;
  hdr = p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ;
  ದತ್ತಾಂಶ = p_ಪುಟ->a_ದತ್ತಾಂಶ;
  /* EVIDENCE-OF: R-58015-48175 The two-byte integer at offset 5 designates
  ** the start of the cell content area. A zero value for this integer is
  ** interpreted as 65536. */
  ತುದಿ = get2byteNotZero(&ದತ್ತಾಂಶ[hdr+5]);
  i_Cell_ಮೊದಲ = hdr + 8 + p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ + 2*p_ಪುಟ->nCell;
  i_Cell_ಕೊನೆಯ = usable_ಗಾತ್ರ - 4;

  /* Compute the total free space on the page
  ** EVIDENCE-OF: R-23588-34450 The two-byte integer at offset 1 gives the
  ** start of the first freeblock on the page, or is zero if there are no
  ** freeblocks. */
  pc = get2byte(&ದತ್ತಾಂಶ[hdr+1]);
  nFree = ದತ್ತಾಂಶ[hdr+7] + ತುದಿ;  /* Init nFree to non-freeblock free space */
  ಆದರೆ( pc>0 ){
    u32 ಮುಂದಿನ, ಗಾತ್ರ_2;
    ಆದರೆ( pc<ತುದಿ ){
      /* EVIDENCE-OF: R-55530-52930 In a well-formed b-tree page, there will
      ** always be at least one cell before the first freeblock.
      */
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }
    ಆಗ( 1 ){
      ಆದರೆ( pc>i_Cell_ಕೊನೆಯ ){
        /* Freeblock off the end of the page */
        ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
      }
      ಮುಂದಿನ = get2byte(&ದತ್ತಾಂಶ[pc]);
      ಗಾತ್ರ_2 = get2byte(&ದತ್ತಾಂಶ[pc+2]);
      ಆದರೆ( ಗಾತ್ರ_2<4 && sqlite3FaultSim(422)==SQLITE_OK ){
        /* Minimum freeblock size is 4.  Enable fault-sim 422 to disable this
        ** check to reach interesting error states.  However, disabling this
        ** check can cause assertion faults due to min-heap overflow.  All
        ** fault-sims are for testing use only, but this one especially so. */
        ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
      }
      nFree = nFree + ಗಾತ್ರ_2;
      ಆದರೆ( ಮುಂದಿನ<pc+ಗಾತ್ರ_2+4 ) ನಿಲ್ಲಿಸು;
      pc = ಮುಂದಿನ;
    }
    ಆದರೆ( ಮುಂದಿನ>0 ){
      /* Freeblock not in ascending order */
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }
    ಆದರೆ( pc+ಗಾತ್ರ_2>(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)usable_ಗಾತ್ರ ){
      /* Last freeblock extends past page end */
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }
  }

  /* At this point, nFree contains the sum of the offset to the start
  ** of the cell-content area plus the number of free bytes within
  ** the cell-content area. If this is greater than the usable-size
  ** of the page, then the page must be corrupted. This check also
  ** serves to verify that the offset to the start of the cell-content
  ** area, according to the page header, lies within the page.
  */
  ಆದರೆ( nFree>usable_ಗಾತ್ರ || nFree<i_Cell_ಮೊದಲ ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
  }
  p_ಪುಟ->nFree = (u16)(nFree - i_Cell_ಮೊದಲ);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Do additional sanity check after btreeInitPage() if
** PRAGMA cell_size_check=ON
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ btree_Cell_ಗಾತ್ರ_Check(Mem_ಪುಟ *p_ಪುಟ){
  ಪೂರ್ಣಾಂಕ i_Cell_ಮೊದಲ;    /* First allowable cell or freeblock offset */
  ಪೂರ್ಣಾಂಕ i_Cell_ಕೊನೆಯ;     /* Last possible cell or freeblock offset */
  ಪೂರ್ಣಾಂಕ i;             /* Index into the cell pointer array */
  ಪೂರ್ಣಾಂಕ sz;            /* Size of a cell */
  ಪೂರ್ಣಾಂಕ pc;            /* Address of a freeblock within pPage->aData[] */
  u8 *ದತ್ತಾಂಶ;          /* Equal to pPage->aData */
  ಪೂರ್ಣಾಂಕ usable_ಗಾತ್ರ;    /* Maximum usable space on the page */
  ಪೂರ್ಣಾಂಕ cell_ಸ್ಥಾನಾಂತರ;    /* Start of cell content area */

  i_Cell_ಮೊದಲ = p_ಪುಟ->cell_ಸ್ಥಾನಾಂತರ + 2*p_ಪುಟ->nCell;
  usable_ಗಾತ್ರ = p_ಪುಟ->pBt->usable_ಗಾತ್ರ;
  i_Cell_ಕೊನೆಯ = usable_ಗಾತ್ರ - 4;
  ದತ್ತಾಂಶ = p_ಪುಟ->a_ದತ್ತಾಂಶ;
  cell_ಸ್ಥಾನಾಂತರ = p_ಪುಟ->cell_ಸ್ಥಾನಾಂತರ;
  ಆದರೆ( !p_ಪುಟ->ಎಲೆ ) i_Cell_ಕೊನೆಯ--;
  ಗಾಗಿ(i=0; i<p_ಪುಟ->nCell; i++){
    pc = get2byteAligned(&ದತ್ತಾಂಶ[cell_ಸ್ಥಾನಾಂತರ+i*2]);
    testcase( pc==i_Cell_ಮೊದಲ );
    testcase( pc==i_Cell_ಕೊನೆಯ );
    ಆದರೆ( pc<i_Cell_ಮೊದಲ || pc>i_Cell_ಕೊನೆಯ ){
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }
    sz = p_ಪುಟ->x_Cell_ಗಾತ್ರ(p_ಪುಟ, &ದತ್ತಾಂಶ[pc]);
    testcase( pc+sz==usable_ಗಾತ್ರ );
    ಆದರೆ( pc+sz>usable_ಗಾತ್ರ ){
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btree_Init_ಪುಟ(Mem_ಪುಟ *p_ಪುಟ){
  u8 *ದತ್ತಾಂಶ;          /* Equal to pPage->aData */
  BtShared *pBt;        /* The main btree structure */

  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pBt!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pBt->db!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pgno==sqlite3PagerPagenumber(p_ಪುಟ->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ == sqlite3PagerGetExtra(p_ಪುಟ->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->a_ದತ್ತಾಂಶ == sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_ಪುಟ->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->isInit==0 );

  pBt = p_ಪುಟ->pBt;
  ದತ್ತಾಂಶ = p_ಪುಟ->a_ದತ್ತಾಂಶ + p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ;
  /* EVIDENCE-OF: R-28594-02890 The one-byte flag at offset 0 indicating
  ** the b-tree page type. */
  ಆದರೆ( decodeFlags(p_ಪುಟ, ದತ್ತಾಂಶ[0]) ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
  }
  ಪ್ರತಿಜ್ಞಾ( pBt->ಪುಟ_ಗಾತ್ರ>=512 && pBt->ಪುಟ_ಗಾತ್ರ<=65536 );
  p_ಪುಟ->ಮುಖವಾಡ_ಪುಟ = (u16)(pBt->ಪುಟ_ಗಾತ್ರ - 1);
  p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ = 0;
  p_ಪುಟ->cell_ಸ್ಥಾನಾಂತರ = (u16)(p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ + 8 + p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ);
  p_ಪುಟ->aCellIdx = ದತ್ತಾಂಶ + p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ + 8;
  p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ = p_ಪುಟ->a_ದತ್ತಾಂಶ + pBt->ಪುಟ_ಗಾತ್ರ;
  p_ಪುಟ->a_ದತ್ತಾಂಶ_Ofst = p_ಪುಟ->a_ದತ್ತಾಂಶ + p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ;
  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  p_ಪುಟ->nCell = get2byte(&ದತ್ತಾಂಶ[3]);
  ಆದರೆ( p_ಪುಟ->nCell>MX_CELL(pBt) ){
    /* To many cells for a single page.  The page must be corrupt */
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
  }
  testcase( p_ಪುಟ->nCell==MX_CELL(pBt) );
  /* EVIDENCE-OF: R-24089-57979 If a page contains no cells (which is only
  ** possible for a root page of a table that contains no rows) then the
  ** offset to the cell content area will equal the page size minus the
  ** bytes of reserved space. */
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nCell>0
       || get2byteNotZero(&ದತ್ತಾಂಶ[5])==(ಪೂರ್ಣಾಂಕ)pBt->usable_ಗಾತ್ರ
       || CORRUPT_DB );
  p_ಪುಟ->nFree = -1;  /* Indicate that this value is yet uncomputed */
  p_ಪುಟ->isInit = 1;
  ಆದರೆ( pBt->db->flags & SQLITE_Cell_ಗಾತ್ರ_Ck ){
    ಹಿಂತಿರುಗಿ btree_Cell_ಗಾತ್ರ_Check(p_ಪುಟ);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Set up a raw page so that it looks like a database page holding
** no entries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ zero_ಪುಟ(Mem_ಪುಟ *p_ಪುಟ, ಪೂರ್ಣಾಂಕ flags){
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ದತ್ತಾಂಶ = p_ಪುಟ->a_ದತ್ತಾಂಶ;
  BtShared *pBt = p_ಪುಟ->pBt;
  ಪೂರ್ಣಾಂಕ hdr = p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ;
  ಪೂರ್ಣಾಂಕ ಮೊದಲ;

  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerPagenumber(p_ಪುಟ->p_Db_ಪುಟ)==p_ಪುಟ->pgno || CORRUPT_DB );
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerGetExtra(p_ಪುಟ->p_Db_ಪುಟ) == (ಶೂನ್ಯ*)p_ಪುಟ );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_ಪುಟ->p_Db_ಪುಟ) == ದತ್ತಾಂಶ );
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಪುಟ->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಆದರೆ( pBt->btsFlags & BTS_FAST_SECURE ){
    memset(&ದತ್ತಾಂಶ[hdr], 0, pBt->usable_ಗಾತ್ರ - hdr);
  }
  ದತ್ತಾಂಶ[hdr] = (ಅಕ್ಷರ)flags;
  ಮೊದಲ = hdr + ((flags&PTF_ಎಲೆ)==0 ? 12 : 8);
  memset(&ದತ್ತಾಂಶ[hdr+1], 0, 4);
  ದತ್ತಾಂಶ[hdr+7] = 0;
  put2byte(&ದತ್ತಾಂಶ[hdr+5], pBt->usable_ಗಾತ್ರ);
  p_ಪುಟ->nFree = (u16)(pBt->usable_ಗಾತ್ರ - ಮೊದಲ);
  decodeFlags(p_ಪುಟ, flags);
  p_ಪುಟ->cell_ಸ್ಥಾನಾಂತರ = (u16)ಮೊದಲ;
  p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ = &ದತ್ತಾಂಶ[pBt->ಪುಟ_ಗಾತ್ರ];
  p_ಪುಟ->aCellIdx = &ದತ್ತಾಂಶ[ಮೊದಲ];
  p_ಪುಟ->a_ದತ್ತಾಂಶ_Ofst = &ದತ್ತಾಂಶ[p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ];
  p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ = 0;
  ಪ್ರತಿಜ್ಞಾ( pBt->ಪುಟ_ಗಾತ್ರ>=512 && pBt->ಪುಟ_ಗಾತ್ರ<=65536 );
  p_ಪುಟ->ಮುಖವಾಡ_ಪುಟ = (u16)(pBt->ಪುಟ_ಗಾತ್ರ - 1);
  p_ಪುಟ->nCell = 0;
  p_ಪುಟ->isInit = 1;
}


/*
** Convert a DbPage obtained from the pager into a MemPage used by
** the btree layer.
*/
ಸ್ಥಿರ Mem_ಪುಟ *btree_ಪುಟ_From_Db_ಪುಟ(Db_ಪುಟ *p_Db_ಪುಟ, Pgno pgno, BtShared *pBt){
  Mem_ಪುಟ *p_ಪುಟ = (Mem_ಪುಟ*)sqlite3PagerGetExtra(p_Db_ಪುಟ);
  ಆದರೆ( pgno!=p_ಪುಟ->pgno ){
    p_ಪುಟ->a_ದತ್ತಾಂಶ = sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_Db_ಪುಟ);
    p_ಪುಟ->p_Db_ಪುಟ = p_Db_ಪುಟ;
    p_ಪುಟ->pBt = pBt;
    p_ಪುಟ->pgno = pgno;
    p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ = pgno==1 ? 100 : 0;
  }
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->a_ದತ್ತಾಂಶ==sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_Db_ಪುಟ) );
  ಹಿಂತಿರುಗಿ p_ಪುಟ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btree_Get_ಪುಟ(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  Mem_ಪುಟ **pp_ಪುಟ,    /* Return the page in this parameter */
  ಪೂರ್ಣಾಂಕ flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  ಪೂರ್ಣಾಂಕ rc;
  Db_ಪುಟ *p_Db_ಪುಟ;

  ಪ್ರತಿಜ್ಞಾ( flags==0 || flags==PAGER_GET_NOCONTENT || flags==PAGER_GET_READONLY );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  rc = sqlite3PagerGet(pBt->pPager, pgno, (Db_ಪುಟ**)&p_Db_ಪುಟ, flags);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
  *pp_ಪುಟ = btree_ಪುಟ_From_Db_ಪುಟ(p_Db_ಪುಟ, pgno, pBt);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Retrieve a page from the pager cache. If the requested page is not
** already in the pager cache return NULL. Initialize the MemPage.pBt and
** MemPage.aData elements if needed.
*/
ಸ್ಥಿರ Mem_ಪುಟ *btree_ಪುಟ_Lookup(BtShared *pBt, Pgno pgno){
  Db_ಪುಟ *p_Db_ಪುಟ;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  p_Db_ಪುಟ = sqlite3PagerLookup(pBt->pPager, pgno);
  ಆದರೆ( p_Db_ಪುಟ ){
    ಹಿಂತಿರುಗಿ btree_ಪುಟ_From_Db_ಪುಟ(p_Db_ಪುಟ, pgno, pBt);
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Return the size of the database file in pages. If there is any kind of
** error, return ((unsigned int)-1).
*/
ಸ್ಥಿರ Pgno btreePagecount(BtShared *pBt){
  ಹಿಂತಿರುಗಿ pBt->n_ಪುಟ;
}
Pgno sqlite3_Btree_ಕೊನೆಯ_ಪುಟ(Btree *p){
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(p) );
  ಹಿಂತಿರುಗಿ btreePagecount(p->pBt);
}

/*
** Get a page from the pager and initialize it.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_And_Init_ಪುಟ(
  BtShared *pBt,                  /* The database file */
  Pgno pgno,                      /* Number of the page to get */
  Mem_ಪುಟ **pp_ಪುಟ,               /* Write the page pointer here */
  ಪೂರ್ಣಾಂಕ bReadOnly                   /* True for a read-only page */
){
  ಪೂರ್ಣಾಂಕ rc;
  Db_ಪುಟ *p_Db_ಪುಟ;
  Mem_ಪುಟ *p_ಪುಟ;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );

  ಆದರೆ( pgno>btreePagecount(pBt) ){
    *pp_ಪುಟ = 0;
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  }
  rc = sqlite3PagerGet(pBt->pPager, pgno, (Db_ಪುಟ**)&p_Db_ಪುಟ, bReadOnly);
  ಆದರೆ( rc ){
    *pp_ಪುಟ = 0;
    ಹಿಂತಿರುಗಿ rc;
  }
  p_ಪುಟ = (Mem_ಪುಟ*)sqlite3PagerGetExtra(p_Db_ಪುಟ);
  ಆದರೆ( p_ಪುಟ->isInit==0 ){
    btree_ಪುಟ_From_Db_ಪುಟ(p_Db_ಪುಟ, pgno, pBt);
    rc = btree_Init_ಪುಟ(p_ಪುಟ);
    ಆದರೆ( rc!=SQLITE_OK ){
      release_ಪುಟ(p_ಪುಟ);
      *pp_ಪುಟ = 0;
      ಹಿಂತಿರುಗಿ rc;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pgno==pgno || CORRUPT_DB );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->a_ದತ್ತಾಂಶ==sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_Db_ಪುಟ) );
  *pp_ಪುಟ = p_ಪುಟ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Release a MemPage.  This should be called once for each prior
** call to btreeGetPage.
**
** Page1 is a special case and must be released using releasePageOne().
*/
ಸ್ಥಿರ ಶೂನ್ಯ release_ಪುಟ_Not_Null(Mem_ಪುಟ *p_ಪುಟ){
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->a_ದತ್ತಾಂಶ );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pBt );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->p_Db_ಪುಟ!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerGetExtra(p_ಪುಟ->p_Db_ಪುಟ) == (ಶೂನ್ಯ*)p_ಪುಟ );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_ಪುಟ->p_Db_ಪುಟ)==p_ಪುಟ->a_ದತ್ತಾಂಶ );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  sqlite3PagerUnrefNotNull(p_ಪುಟ->p_Db_ಪುಟ);
}
ಸ್ಥಿರ ಶೂನ್ಯ release_ಪುಟ(Mem_ಪುಟ *p_ಪುಟ){
  ಆದರೆ( p_ಪುಟ ) release_ಪುಟ_Not_Null(p_ಪುಟ);
}
ಸ್ಥಿರ ಶೂನ್ಯ release_ಪುಟ_One(Mem_ಪುಟ *p_ಪುಟ){
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->a_ದತ್ತಾಂಶ );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pBt );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->p_Db_ಪುಟ!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerGetExtra(p_ಪುಟ->p_Db_ಪುಟ) == (ಶೂನ್ಯ*)p_ಪುಟ );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_ಪುಟ->p_Db_ಪುಟ)==p_ಪುಟ->a_ದತ್ತಾಂಶ );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  sqlite3_Pager_Unref_ಪುಟ_One(p_ಪುಟ->p_Db_ಪುಟ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btree_Get_ಬಳಸದ_ಪುಟ(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  Mem_ಪುಟ **pp_ಪುಟ,    /* Return the page in this parameter */
  ಪೂರ್ಣಾಂಕ flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  ಪೂರ್ಣಾಂಕ rc = btree_Get_ಪುಟ(pBt, pgno, pp_ಪುಟ, flags);
  ಆದರೆ( rc==SQLITE_OK ){
    ಆದರೆ( sqlite3_Pager_ಪುಟ_Refcount((*pp_ಪುಟ)->p_Db_ಪುಟ)>1 ){
      release_ಪುಟ(*pp_ಪುಟ);
      *pp_ಪುಟ = 0;
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
    }
    (*pp_ಪುಟ)->isInit = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    *pp_ಪುಟ = 0;
  }
  ಹಿಂತಿರುಗಿ rc;
}


/*
** During a rollback, when the pager reloads information into the cache
** so that the cache is restored to its original state at the start of
** the transaction, for each page restored this routine is called.
**
** This routine needs to reset the extra data section at the end of the
** page to agree with the restored data.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಪುಟ_Reinit(Db_ಪುಟ *p_ದತ್ತಾಂಶ){
  Mem_ಪುಟ *p_ಪುಟ;
  p_ಪುಟ = (Mem_ಪುಟ *)sqlite3PagerGetExtra(p_ದತ್ತಾಂಶ);
  ಪ್ರತಿಜ್ಞಾ( sqlite3_Pager_ಪುಟ_Refcount(p_ದತ್ತಾಂಶ)>0 );
  ಆದರೆ( p_ಪುಟ->isInit ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
    p_ಪುಟ->isInit = 0;
    ಆದರೆ( sqlite3_Pager_ಪುಟ_Refcount(p_ದತ್ತಾಂಶ)>1 ){
      /* pPage might not be a btree page;  it might be an overflow page
      ** or ptrmap page or a free page.  In those cases, the following
      ** call to btreeInitPage() will likely return SQLITE_CORRUPT.
      ** But no harm is done by this.  And it is very important that
      ** btreeInitPage() be called on every btree page so we make
      ** the call for every page that comes in for re-initializing. */
      btree_Init_ಪುಟ(p_ಪುಟ);
    }
  }
}

/*
** Invoke the busy handler for a btree.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btreeInvokeBusyHandler(ಶೂನ್ಯ *pArg){
  BtShared *pBt = (BtShared*)pArg;
  ಪ್ರತಿಜ್ಞಾ( pBt->db );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->db->mutex) );
  ಹಿಂತಿರುಗಿ sqlite3InvokeBusyHandler(&pBt->db->busyHandler);
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
ಪೂರ್ಣಾಂಕ sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,      /* VFS to use for this b-tree */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename,  /* Name of the file containing the BTree database */
  sqlite3 *db,            /* Associated database handle */
  Btree **ppBtree,        /* Pointer to new Btree object written here */
  ಪೂರ್ಣಾಂಕ flags,              /* Options */
  ಪೂರ್ಣಾಂಕ vfsFlags            /* Flags passed through to sqlite3_vfs.xOpen() */
){
  BtShared *pBt = 0;             /* Shared part of btree structure */
  Btree *p;                      /* Handle to return */
  sqlite3_mutex *mutexOpen = 0;  /* Prevents a race condition. Ticket #3537 */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;            /* Result code from this function */
  u8 nReserve;                   /* Byte of unused space on each page */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ z_Db_ಶೀರ್ಷಿಕೆ[100];  /* Database header content */

  /* True if opening an ephemeral, temporary database */
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ is_ತಾತ್ಕಾಲಿಕ_Db = zFilename==0 || zFilename[0]==0;

  /* Set the variable isMemdb to true for an in-memory database, or
  ** false for a file-based database.
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_MEMORYDB
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ isMemdb = 0;
#ಇಲ್ಲದಿದ್ದರೆ
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ isMemdb = (zFilename && strcmp(zFilename, ":memory:")==0)
                       || (is_ತಾತ್ಕಾಲಿಕ_Db && sqlite3_ತಾತ್ಕಾಲಿಕ_ಒಳ_ಸ್ಮರಣೆ(db))
                       || (vfsFlags & SQLITE_OPEN_ಸ್ಮರಣೆ)!=0;
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಪ್ರತಿಜ್ಞಾ( pVfs!=0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( (flags&0xff)==flags );   /* flags fit in 8 bits */

  /* Only a BTREE_SINGLE database can be BTREE_UNORDERED */
  ಪ್ರತಿಜ್ಞಾ( (flags & BTREE_UNORDERED)==0 || (flags & BTREE_SINGLE)!=0 );

  /* A BTREE_SINGLE database is always a temporary and/or ephemeral */
  ಪ್ರತಿಜ್ಞಾ( (flags & BTREE_SINGLE)==0 || is_ತಾತ್ಕಾಲಿಕ_Db );

  ಆದರೆ( isMemdb ){
    flags |= BTREE_ಸ್ಮರಣೆ;
  }
  ಆದರೆ( (vfsFlags & SQLITE_OPEN_MAIN_DB)!=0 && (isMemdb || is_ತಾತ್ಕಾಲಿಕ_Db) ){
    vfsFlags = (vfsFlags & ~SQLITE_OPEN_MAIN_DB) | SQLITE_OPEN_ತಾತ್ಕಾಲಿಕ_DB;
  }
  p = sqlite3MallocZero(ಗಾತ್ರ(Btree));
  ಆದರೆ( !p ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  p->ಒಳ_Trans = TRANS_NONE;
  p->db = db;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
  p->ಬೀಗ.pBtree = p;
  p->ಬೀಗ.i_ಕೋಷ್ಟಕ = 1;
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
  /*
  ** If this Btree is a candidate for shared cache, try to find an
  ** existing BtShared object that we can share with
  */
  ಆದರೆ( is_ತಾತ್ಕಾಲಿಕ_Db==0 && (isMemdb==0 || (vfsFlags&SQLITE_OPEN_URI)!=0) ){
    ಆದರೆ( vfsFlags & SQLITE_OPEN_SHAREDCACHE ){
      ಪೂರ್ಣಾಂಕ nFilename = sqlite3Strlen30(zFilename)+1;
      ಪೂರ್ಣಾಂಕ n_ತುಂಬಿದ_Pathname = pVfs->mxPathname+1;
      ಅಕ್ಷರ *z_ತುಂಬಿದ_Pathname = sqlite3Malloc(MAX(n_ತುಂಬಿದ_Pathname,nFilename));
      MUTEX_LOGIC( sqlite3_mutex *mutexShared; )

      p->sharable = 1;
      ಆದರೆ( !z_ತುಂಬಿದ_Pathname ){
        sqlite3_free(p);
        ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
      }
      ಆದರೆ( isMemdb ){
        ಸ್ಮರಣೆ_ನಕಲು(z_ತುಂಬಿದ_Pathname, zFilename, nFilename);
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = sqlite3_Os_ತುಂಬಿದ_Pathname(pVfs, zFilename,
                                   n_ತುಂಬಿದ_Pathname, z_ತುಂಬಿದ_Pathname);
        ಆದರೆ( rc ){
          ಆದರೆ( rc==SQLITE_OK_SYMLINK ){
            rc = SQLITE_OK;
          }ಇಲ್ಲದಿದ್ದರೆ{
            sqlite3_free(z_ತುಂಬಿದ_Pathname);
            sqlite3_free(p);
            ಹಿಂತಿರುಗಿ rc;
          }
        }
      }
#ಆದರೆ SQLITE_THREADSAFE
      mutexOpen = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_OPEN);
      sqlite3_mutex_enter(mutexOpen);
      mutexShared = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
      sqlite3_mutex_enter(mutexShared);
#ಆದರೆ_ಅಂತ್ಯ
      ಗಾಗಿ(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->p_ಮುಂದಿನ){
        ಪ್ರತಿಜ್ಞಾ( pBt->nRef>0 );
        ಆದರೆ( 0==strcmp(z_ತುಂಬಿದ_Pathname, sqlite3PagerFilename(pBt->pPager, 0))
                 && sqlite3PagerVfs(pBt->pPager)==pVfs ){
          ಪೂರ್ಣಾಂಕ iDb;
          ಗಾಗಿ(iDb=db->nDb-1; iDb>=0; iDb--){
            Btree *pExisting = db->aDb[iDb].pBt;
            ಆದರೆ( pExisting && pExisting->pBt==pBt ){
              sqlite3_mutex_leave(mutexShared);
              sqlite3_mutex_leave(mutexOpen);
              sqlite3_free(z_ತುಂಬಿದ_Pathname);
              sqlite3_free(p);
              ಹಿಂತಿರುಗಿ SQLITE_CONSTRAINT;
            }
          }
          p->pBt = pBt;
          pBt->nRef++;
          ನಿಲ್ಲಿಸು;
        }
      }
      sqlite3_mutex_leave(mutexShared);
      sqlite3_free(z_ತುಂಬಿದ_Pathname);
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    ಇಲ್ಲದಿದ್ದರೆ{
      /* In debug mode, we mark all persistent databases as sharable
      ** even when they are not.  This exercises the locking code and
      ** gives more opportunity for asserts(sqlite3_mutex_held())
      ** statements to find locking problems.
      */
      p->sharable = 1;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( pBt==0 ){
    /*
    ** The following asserts make sure that structures used by the btree are
    ** the right size.  This is to guard against size changes that result
    ** when compiling on a different architecture.
    */
    ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(i64)==8 );
    ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(u64)==8 );
    ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(u32)==4 );
    ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(u16)==2 );
    ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(Pgno)==4 );

    /* Suppress false-positive compiler warning from PVS-Studio */
    memset(&z_Db_ಶೀರ್ಷಿಕೆ[16], 0, 8);
 
    pBt = sqlite3MallocZero( ಗಾತ್ರ(*pBt) );
    ಆದರೆ( pBt==0 ){
      rc = SQLITE_NOMEM_BKPT;
      ಹೋಗು btree_open_ಹೊರಹೋಗು;
    }
    rc = sqlite3PagerOpen(pVfs, &pBt->pPager, zFilename,
                          ಗಾತ್ರ(Mem_ಪುಟ), flags, vfsFlags, ಪುಟ_Reinit);
    ಆದರೆ( rc==SQLITE_OK ){
      sqlite3_Pager_ಗಣ_Mmap_ಎಲ್ಲೆ(pBt->pPager, db->szMmap);
      rc = sqlite3PagerReadFileheader(pBt->pPager,ಗಾತ್ರ(z_Db_ಶೀರ್ಷಿಕೆ),z_Db_ಶೀರ್ಷಿಕೆ);
    }
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹೋಗು btree_open_ಹೊರಹೋಗು;
    }
    pBt->openFlags = (u8)flags;
    pBt->db = db;
    sqlite3_Pager_ಗಣ_Busy_Handler(pBt->pPager, btreeInvokeBusyHandler, pBt);
    p->pBt = pBt;
 
    pBt->pCursor = 0;
    pBt->pPage1 = 0;
    ಆದರೆ( sqlite3PagerIsreadonly(pBt->pPager) ) pBt->btsFlags |= BTS_READ_ONLY;
#ಆದರೆ defined(SQLITE_SECURE_DELETE)
    pBt->btsFlags |= BTS_SECURE_DELETE;
#ಅಥವಾಆದರೆ defined(SQLITE_FAST_SECURE_DELETE)
    pBt->btsFlags |= BTS_OVERWRITE;
#ಆದರೆ_ಅಂತ್ಯ
    /* EVIDENCE-OF: R-51873-39618 The page size for a database file is
    ** determined by the 2-byte integer located at an offset of 16 bytes from
    ** the beginning of the database file. */
    pBt->ಪುಟ_ಗಾತ್ರ = (z_Db_ಶೀರ್ಷಿಕೆ[16]<<8) | (z_Db_ಶೀರ್ಷಿಕೆ[17]<<16);
    ಆದರೆ( pBt->ಪುಟ_ಗಾತ್ರ<512 || pBt->ಪುಟ_ಗಾತ್ರ>SQLITE_MAX_ಪುಟ_ಗಾತ್ರ
         || ((pBt->ಪುಟ_ಗಾತ್ರ-1)&pBt->ಪುಟ_ಗಾತ್ರ)!=0 ){
      pBt->ಪುಟ_ಗಾತ್ರ = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
      /* If the magic name ":memory:" will create an in-memory database, then
      ** leave the autoVacuum mode at 0 (do not auto-vacuum), even if
      ** SQLITE_DEFAULT_AUTOVACUUM is true. On the other hand, if
      ** SQLITE_OMIT_MEMORYDB has been defined, then ":memory:" is just a
      ** regular file-name. In this case the auto-vacuum applies as per normal.
      */
      ಆದರೆ( zFilename && !isMemdb ){
        pBt->autoVacuum = (SQLITE_DEFAULT_AUTOVACUUM ? 1 : 0);
        pBt->incrVacuum = (SQLITE_DEFAULT_AUTOVACUUM==2 ? 1 : 0);
      }
#ಆದರೆ_ಅಂತ್ಯ
      nReserve = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* EVIDENCE-OF: R-37497-42412 The size of the reserved region is
      ** determined by the one-byte unsigned integer found at an offset of 20
      ** into the database file header. */
      nReserve = z_Db_ಶೀರ್ಷಿಕೆ[20];
      pBt->btsFlags |= BTS_PAGESIZE_FIXED;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
      pBt->autoVacuum = (get4byte(&z_Db_ಶೀರ್ಷಿಕೆ[36 + 4*4])?1:0);
      pBt->incrVacuum = (get4byte(&z_Db_ಶೀರ್ಷಿಕೆ[36 + 7*4])?1:0);
#ಆದರೆ_ಅಂತ್ಯ
    }
    rc = sqlite3_Pager_ಗಣ_Pagesize(pBt->pPager, &pBt->ಪುಟ_ಗಾತ್ರ, nReserve);
    ಆದರೆ( rc ) ಹೋಗು btree_open_ಹೊರಹೋಗು;
    pBt->usable_ಗಾತ್ರ = pBt->ಪುಟ_ಗಾತ್ರ - nReserve;
    ಪ್ರತಿಜ್ಞಾ( (pBt->ಪುಟ_ಗಾತ್ರ & 7)==0 );  /* 8-byte alignment of pageSize */
  
#ಆದರೆ !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
    /* Add the new BtShared object to the linked list sharable BtShareds.
    */
    pBt->nRef = 1;
    ಆದರೆ( p->sharable ){
      MUTEX_LOGIC( sqlite3_mutex *mutexShared; )
      MUTEX_LOGIC( mutexShared = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);)
      ಆದರೆ( SQLITE_THREADSAFE && sqlite3GlobalConfig.bCoreMutex ){
        pBt->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_FAST);
        ಆದರೆ( pBt->mutex==0 ){
          rc = SQLITE_NOMEM_BKPT;
          ಹೋಗು btree_open_ಹೊರಹೋಗು;
        }
      }
      sqlite3_mutex_enter(mutexShared);
      pBt->p_ಮುಂದಿನ = GLOBAL(BtShared*,sqlite3SharedCacheList);
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt;
      sqlite3_mutex_leave(mutexShared);
    }
#ಆದರೆ_ಅಂತ್ಯ
  }

#ಆದರೆ !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
  /* If the new Btree uses a sharable pBtShared, then link the new
  ** Btree into the list of all sharable Btrees for the same connection.
  ** The list is kept in ascending order by pBt address.
  */
  ಆದರೆ( p->sharable ){
    ಪೂರ್ಣಾಂಕ i;
    Btree *pSib;
    ಗಾಗಿ(i=0; i<db->nDb; i++){
      ಆದರೆ( (pSib = db->aDb[i].pBt)!=0 && pSib->sharable ){
        ಆಗ( pSib->pPrev ){ pSib = pSib->pPrev; }
        ಆದರೆ( (uptr)p->pBt<(uptr)pSib->pBt ){
          p->p_ಮುಂದಿನ = pSib;
          p->pPrev = 0;
          pSib->pPrev = p;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಆಗ( pSib->p_ಮುಂದಿನ && (uptr)pSib->p_ಮುಂದಿನ->pBt<(uptr)p->pBt ){
            pSib = pSib->p_ಮುಂದಿನ;
          }
          p->p_ಮುಂದಿನ = pSib->p_ಮುಂದಿನ;
          p->pPrev = pSib;
          ಆದರೆ( p->p_ಮುಂದಿನ ){
            p->p_ಮುಂದಿನ->pPrev = p;
          }
          pSib->p_ಮುಂದಿನ = p;
        }
        ನಿಲ್ಲಿಸು;
      }
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  *ppBtree = p;

btree_open_ಹೊರಹೋಗು:
  ಆದರೆ( rc!=SQLITE_OK ){
    ಆದರೆ( pBt && pBt->pPager ){
      sqlite3PagerClose(pBt->pPager, 0);
    }
    sqlite3_free(pBt);
    sqlite3_free(p);
    *ppBtree = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_file *pFile;

    /* If the B-Tree was successfully opened, set the pager-cache size to the
    ** default value. Except, when opening on an existing shared pager-cache,
    ** do not change the pager-cache size.
    */
    ಆದರೆ( sqlite3BtreeSchema(p, 0, 0)==0 ){
      sqlite3_Btree_ಗಣ_Cache_ಗಾತ್ರ(p, SQLITE_DEFAULT_CACHE_ಗಾತ್ರ);
    }

    pFile = sqlite3PagerFile(pBt->pPager);
    ಆದರೆ( pFile->pMethods ){
      sqlite3OsFileControlHint(pFile, SQLITE_FCNTL_PDB, (ಶೂನ್ಯ*)&pBt->db);
    }
  }
  ಆದರೆ( mutexOpen ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(mutexOpen) );
    sqlite3_mutex_leave(mutexOpen);
  }
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || sqlite3_Btree_Connection_ಎಣಿಕೆ(*ppBtree)>0 );
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Decrement the BtShared.nRef counter.  When it reaches zero,
** remove the BtShared structure from the sharing list.  Return
** true if the BtShared.nRef counter reaches zero and return
** false if it is still positive.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ removeFromSharingList(BtShared *pBt){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )
  BtShared *pList;
  ಪೂರ್ಣಾಂಕ removed = 0;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_notheld(pBt->mutex) );
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  pBt->nRef--;
  ಆದರೆ( pBt->nRef<=0 ){
    ಆದರೆ( GLOBAL(BtShared*,sqlite3SharedCacheList)==pBt ){
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt->p_ಮುಂದಿನ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pList = GLOBAL(BtShared*,sqlite3SharedCacheList);
      ಆಗ( ALWAYS(pList) && pList->p_ಮುಂದಿನ!=pBt ){
        pList=pList->p_ಮುಂದಿನ;
      }
      ಆದರೆ( ALWAYS(pList) ){
        pList->p_ಮುಂದಿನ = pBt->p_ಮುಂದಿನ;
      }
    }
    ಆದರೆ( SQLITE_THREADSAFE ){
      sqlite3_mutex_free(pBt->mutex);
    }
    removed = 1;
  }
  sqlite3_mutex_leave(pMainMtx);
  ಹಿಂತಿರುಗಿ removed;
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಳಸದ_PARAMETER( pBt );
  ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Make sure pBt->pTmpSpace points to an allocation of
** MX_CELL_SIZE(pBt) bytes with a 4-byte prefix for a left-child
** pointer.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ allocate_ತಾತ್ಕಾಲಿಕ_Space(BtShared *pBt){
  ಪ್ರತಿಜ್ಞಾ( pBt!=0 );
  ಪ್ರತಿಜ್ಞಾ( pBt->pTmpSpace==0 );
  /* This routine is called only by btreeCursor() when allocating the
  ** first write cursor for the BtShared object */
  ಪ್ರತಿಜ್ಞಾ( pBt->pCursor!=0 && (pBt->pCursor->curFlags & BTCF_WriteFlag)!=0 );
  pBt->pTmpSpace = sqlite3_ಪುಟ_Malloc( pBt->ಪುಟ_ಗಾತ್ರ );
  ಆದರೆ( pBt->pTmpSpace==0 ){
    BtCursor *pCur = pBt->pCursor;
    pBt->pCursor = pCur->p_ಮುಂದಿನ;  /* Unlink the cursor */
    memset(pCur, 0, ಗಾತ್ರ(*pCur));
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
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
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Free the pBt->pTmpSpace allocation
*/
ಸ್ಥಿರ ಶೂನ್ಯ free_ತಾತ್ಕಾಲಿಕ_Space(BtShared *pBt){
  ಆದರೆ( pBt->pTmpSpace ){
    pBt->pTmpSpace -= 4;
    sqlite3_ಪುಟ_Free(pBt->pTmpSpace);
    pBt->pTmpSpace = 0;
  }
}

/*
** Close an open database and invalidate all cursors.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeClose(Btree *p){
  BtShared *pBt = p->pBt;

  /* Close all cursors opened via this handle.  */
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);

  /* Verify that no other cursors have this Btree open */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  {
    BtCursor *pCur = pBt->pCursor;
    ಆಗ( pCur ){
      BtCursor *pTmp = pCur;
      pCur = pCur->p_ಮುಂದಿನ;
      ಪ್ರತಿಜ್ಞಾ( pTmp->pBtree!=p );

    }
  }
#ಆದರೆ_ಅಂತ್ಯ

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
  ಪ್ರತಿಜ್ಞಾ( p->want_ಗಮ್ಯ_ಬೀಗ==0 && p->locked==0 );
  ಆದರೆ( !p->sharable || removeFromSharingList(pBt) ){
    /* The pBt is no longer on the sharing list, so we can access
    ** it without having to hold the mutex.
    **
    ** Clean out and delete the BtShared object.
    */
    ಪ್ರತಿಜ್ಞಾ( !pBt->pCursor );
    sqlite3PagerClose(pBt->pPager, p->db);
    ಆದರೆ( pBt->xFreeSchema && pBt->pSchema ){
      pBt->xFreeSchema(pBt->pSchema);
    }
    sqlite3DbFree(0, pBt->pSchema);
    free_ತಾತ್ಕಾಲಿಕ_Space(pBt);
    sqlite3_free(pBt);
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
  ಪ್ರತಿಜ್ಞಾ( p->want_ಗಮ್ಯ_ಬೀಗ==0 );
  ಪ್ರತಿಜ್ಞಾ( p->locked==0 );
  ಆದರೆ( p->pPrev ) p->pPrev->p_ಮುಂದಿನ = p->p_ಮುಂದಿನ;
  ಆದರೆ( p->p_ಮುಂದಿನ ) p->p_ಮುಂದಿನ->pPrev = p->pPrev;
#ಆದರೆ_ಅಂತ್ಯ

  sqlite3_free(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Change the "soft" limit on the number of pages in the cache.
** Unused and unmodified pages will be recycled when the number of
** pages in the cache exceeds this soft limit.  But the size of the
** cache is allowed to grow larger than this limit if it contains
** dirty pages or pages still in active use.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_Cache_ಗಾತ್ರ(Btree *p, ಪೂರ್ಣಾಂಕ mx_ಪುಟ){
  BtShared *pBt = p->pBt;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_ಗಣ_Cachesize(pBt->pPager, mx_ಪುಟ);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_Spill_ಗಾತ್ರ(Btree *p, ಪೂರ್ಣಾಂಕ mx_ಪುಟ){
  BtShared *pBt = p->pBt;
  ಪೂರ್ಣಾಂಕ res;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  res = sqlite3_Pager_ಗಣ_Spillsize(pBt->pPager, mx_ಪುಟ);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ res;
}

#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
/*
** Change the limit on the amount of the database file that may be
** memory mapped.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_Mmap_ಎಲ್ಲೆ(Btree *p, sqlite3_int64 szMmap){
  BtShared *pBt = p->pBt;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_ಗಣ_Mmap_ಎಲ್ಲೆ(pBt->pPager, szMmap);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_MAX_MMAP_SIZE>0 */

/*
** Change the way data is synced to disk in order to increase or decrease
** how well the database resists damage due to OS crashes and power
** failures.  Level 1 is the same as asynchronous (no syncs() occur and
** there is a high probability of damage)  Level 2 is the default.  There
** is a very low but non-zero probability of damage.  Level 3 reduces the
** probability of damage to near zero but with a write performance reduction.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_PAGER_PRAGMAS
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_Pager_Flags(
  Btree *p,              /* The btree to set the safety level on */
  ಚಿಹ್ನೆರಹಿತ pgFlags       /* Various PAGER_* flags */
){
  BtShared *pBt = p->pBt;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_ಗಣ_Flags(pBt->pPager, pgFlags);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ

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
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_ಪುಟ_ಗಾತ್ರ(Btree *p, ಪೂರ್ಣಾಂಕ ಪುಟ_ಗಾತ್ರ, ಪೂರ್ಣಾಂಕ nReserve, ಪೂರ್ಣಾಂಕ iFix){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ x;
  BtShared *pBt = p->pBt;
  ಪ್ರತಿಜ್ಞಾ( nReserve>=0 && nReserve<=255 );
  sqlite3BtreeEnter(p);
  pBt->nReserveWanted = (u8)nReserve;
  x = pBt->ಪುಟ_ಗಾತ್ರ - pBt->usable_ಗಾತ್ರ;
  ಆದರೆ( x==nReserve && (ಪುಟ_ಗಾತ್ರ==0 || (u32)ಪುಟ_ಗಾತ್ರ==pBt->ಪುಟ_ಗಾತ್ರ) ){
    sqlite3BtreeLeave(p);
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  ಆದರೆ( nReserve<x ) nReserve = x;
  ಆದರೆ( pBt->btsFlags & BTS_PAGESIZE_FIXED ){
    sqlite3BtreeLeave(p);
    ಹಿಂತಿರುಗಿ SQLITE_READONLY;
  }
  ಪ್ರತಿಜ್ಞಾ( nReserve>=0 && nReserve<=255 );
  ಆದರೆ( ಪುಟ_ಗಾತ್ರ>=512 && ಪುಟ_ಗಾತ್ರ<=SQLITE_MAX_ಪುಟ_ಗಾತ್ರ &&
        ((ಪುಟ_ಗಾತ್ರ-1)&ಪುಟ_ಗಾತ್ರ)==0 ){
    ಪ್ರತಿಜ್ಞಾ( (ಪುಟ_ಗಾತ್ರ & 7)==0 );
    ಪ್ರತಿಜ್ಞಾ( !pBt->pCursor );
    ಆದರೆ( nReserve>32 && ಪುಟ_ಗಾತ್ರ==512 ) ಪುಟ_ಗಾತ್ರ = 1024;
    pBt->ಪುಟ_ಗಾತ್ರ = (u32)ಪುಟ_ಗಾತ್ರ;
    free_ತಾತ್ಕಾಲಿಕ_Space(pBt);
  }
  rc = sqlite3_Pager_ಗಣ_Pagesize(pBt->pPager, &pBt->ಪುಟ_ಗಾತ್ರ, nReserve);
  pBt->usable_ಗಾತ್ರ = pBt->ಪುಟ_ಗಾತ್ರ - (u16)nReserve;
  ಆದರೆ( iFix ) pBt->btsFlags |= BTS_PAGESIZE_FIXED;
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return the currently defined page size
*/
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(Btree *p){
  ಹಿಂತಿರುಗಿ p->pBt->ಪುಟ_ಗಾತ್ರ;
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
ಪೂರ್ಣಾಂಕ sqlite3BtreeGetReserveNoMutex(Btree *p){
  ಪೂರ್ಣಾಂಕ n;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->pBt->mutex) );
  n = p->pBt->ಪುಟ_ಗಾತ್ರ - p->pBt->usable_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ n;
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
ಪೂರ್ಣಾಂಕ sqlite3BtreeGetRequestedReserve(Btree *p){
  ಪೂರ್ಣಾಂಕ n1, n2;
  sqlite3BtreeEnter(p);
  n1 = (ಪೂರ್ಣಾಂಕ)p->pBt->nReserveWanted;
  n2 = sqlite3BtreeGetReserveNoMutex(p);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ n1>n2 ? n1 : n2;
}


/*
** Set the maximum page count for a database if mxPage is positive.
** No changes are made if mxPage is 0 or negative.
** Regardless of the value of mxPage, return the maximum page count.
*/
Pgno sqlite3_Btree_Max_ಪುಟ_ಎಣಿಕೆ(Btree *p, Pgno mx_ಪುಟ){
  Pgno n;
  sqlite3BtreeEnter(p);
  n = sqlite3_Pager_Max_ಪುಟ_ಎಣಿಕೆ(p->pBt->pPager, mx_ಪುಟ);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ n;
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
ಪೂರ್ಣಾಂಕ sqlite3BtreeSecureDelete(Btree *p, ಪೂರ್ಣಾಂಕ newFlag){
  ಪೂರ್ಣಾಂಕ b;
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ 0;
  sqlite3BtreeEnter(p);
  ಪ್ರತಿಜ್ಞಾ( BTS_OVERWRITE==BTS_SECURE_DELETE*2 );
  ಪ್ರತಿಜ್ಞಾ( BTS_FAST_SECURE==(BTS_OVERWRITE|BTS_SECURE_DELETE) );
  ಆದರೆ( newFlag>=0 ){
    p->pBt->btsFlags &= ~BTS_FAST_SECURE;
    p->pBt->btsFlags |= (u16)(BTS_SECURE_DELETE*newFlag);
  }
  b = (p->pBt->btsFlags & BTS_FAST_SECURE)/BTS_SECURE_DELETE;
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ b;
}

/*
** Change the 'auto-vacuum' property of the database. If the 'autoVacuum'
** parameter is non-zero, then auto-vacuum mode is enabled. If zero, it
** is disabled. The default value for the auto-vacuum property is
** determined by the SQLITE_DEFAULT_AUTOVACUUM macro.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_Auto_Vacuum(Btree *p, ಪೂರ್ಣಾಂಕ autoVacuum){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_AUTOVACUUM
  ಹಿಂತಿರುಗಿ SQLITE_READONLY;
#ಇಲ್ಲದಿದ್ದರೆ
  BtShared *pBt = p->pBt;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  u8 av = (u8)autoVacuum;

  sqlite3BtreeEnter(p);
  ಆದರೆ( (pBt->btsFlags & BTS_PAGESIZE_FIXED)!=0 && (av ?1:0)!=pBt->autoVacuum ){
    rc = SQLITE_READONLY;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pBt->autoVacuum = av ?1:0;
    pBt->incrVacuum = av==2 ?1:0;
  }
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Return the value of the 'auto-vacuum' property. If auto-vacuum is
** enabled 1 is returned. Otherwise 0.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeGetAutoVacuum(Btree *p){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_AUTOVACUUM
  ಹಿಂತಿರುಗಿ BTREE_AUTOVACUUM_NONE;
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3BtreeEnter(p);
  rc = (
    (!p->pBt->autoVacuum)?BTREE_AUTOVACUUM_NONE:
    (!p->pBt->incrVacuum)?BTREE_AUTOVACUUM_ತುಂಬಿದ:
    BTREE_AUTOVACUUM_INCR
  );
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** If the user has not set the safety-level for this database connection
** using "PRAGMA synchronous", and if the safety-level is not already
** set to the value passed to this function as the second parameter,
** set it so.
*/
#ಆದರೆ SQLITE_DEFAULT_SYNCHRONOUS!=SQLITE_DEFAULT_WAL_SYNCHRONOUS \
    && !defined(SQLITE_OMIT_WAL)
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_Default_Sync_Flag(BtShared *pBt, u8 safety_level){
  sqlite3 *db;
  Db *pDb;
  ಆದರೆ( (db=pBt->db)!=0 && (pDb=db->aDb)!=0 ){
    ಆಗ( pDb->pBt==0 || pDb->pBt->pBt!=pBt ){ pDb++; }
    ಆದರೆ( pDb->b_Sync_ಗಣ==0
     && pDb->safety_level!=safety_level
     && pDb!=&db->aDb[1]
    ){
      pDb->safety_level = safety_level;
      sqlite3_Pager_ಗಣ_Flags(pBt->pPager,
          pDb->safety_level | (db->flags & PAGER_FLAGS_ಮುಖವಾಡ));
    }
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define ಗಣ_Default_Sync_Flag(pBt,safety_level)
#ಆದರೆ_ಅಂತ್ಯ

/* Forward declaration */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ newDatabase(BtShared*);


/*
** Get a reference to pPage1 of the database file.  This will
** also acquire a readlock on that file.
**
** SQLITE_OK is returned on success.  If the file is not a
** well-formed database file, then SQLITE_CORRUPT is returned.
** SQLITE_BUSY is returned if the database is locked.  SQLITE_NOMEM
** is returned if we run out of memory.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಬೀಗ_Btree(BtShared *pBt){
  ಪೂರ್ಣಾಂಕ rc;              /* Result code from subfunctions */
  Mem_ಪುಟ *pPage1;     /* Page 1 of the database file */
  u32 n_ಪುಟ;           /* Number of pages in the database */
  u32 n_ಪುಟ_File = 0;   /* Number of pages in the database file */

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( pBt->pPage1==0 );
  rc = sqlite3_Pager_Shared_ಬೀಗ(pBt->pPager);
  ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;
  rc = btree_Get_ಪುಟ(pBt, 1, &pPage1, 0);
  ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;

  /* Do some checking to help insure the file we opened really is
  ** a valid database file.
  */
  n_ಪುಟ = get4byte(28+(u8*)pPage1->a_ದತ್ತಾಂಶ);
  sqlite3PagerPagecount(pBt->pPager, (ಪೂರ್ಣಾಂಕ*)&n_ಪುಟ_File);
  ಆದರೆ( n_ಪುಟ==0 || memcmp(24+(u8*)pPage1->a_ದತ್ತಾಂಶ, 92+(u8*)pPage1->a_ದತ್ತಾಂಶ,4)!=0 ){
    n_ಪುಟ = n_ಪುಟ_File;
  }
  ಆದರೆ( (pBt->db->flags & SQLITE_ResetDatabase)!=0 ){
    n_ಪುಟ = 0;
  }
  ಆದರೆ( n_ಪುಟ>0 ){
    u32 ಪುಟ_ಗಾತ್ರ;
    u32 usable_ಗಾತ್ರ;
    u8 *page1 = pPage1->a_ದತ್ತಾಂಶ;
    rc = SQLITE_NOTADB;
    /* EVIDENCE-OF: R-43737-39999 Every valid SQLite database file begins
    ** with the following 16 bytes (in hex): 53 51 4c 69 74 65 20 66 6f 72 6d
    ** 61 74 20 33 00. */
    ಆದರೆ( memcmp(page1, z_Magic_ಶೀರ್ಷಿಕೆ, 16)!=0 ){
      ಹೋಗು page1_init_failed;
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_WAL
    ಆದರೆ( page1[18]>1 ){
      pBt->btsFlags |= BTS_READ_ONLY;
    }
    ಆದರೆ( page1[19]>1 ){
      ಹೋಗು page1_init_failed;
    }
#ಇಲ್ಲದಿದ್ದರೆ
    ಆದರೆ( page1[18]>2 ){
      pBt->btsFlags |= BTS_READ_ONLY;
    }
    ಆದರೆ( page1[19]>2 ){
      ಹೋಗು page1_init_failed;
    }

    /* If the read version is set to 2, this database should be accessed
    ** in WAL mode. If the log is not already open, open it now. Then
    ** return SQLITE_OK and return without populating BtShared.pPage1.
    ** The caller detects this and calls this function again. This is
    ** required as the version of page 1 currently in the page1 buffer
    ** may not be the latest version - there may be a newer one in the log
    ** file.
    */
    ಆದರೆ( page1[19]==2 && (pBt->btsFlags & BTS_NO_WAL)==0 ){
      ಪೂರ್ಣಾಂಕ isOpen = 0;
      rc = sqlite3PagerOpenWal(pBt->pPager, &isOpen);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹೋಗು page1_init_failed;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಗಣ_Default_Sync_Flag(pBt, SQLITE_DEFAULT_WAL_SYNCHRONOUS+1);
        ಆದರೆ( isOpen==0 ){
          release_ಪುಟ_One(pPage1);
          ಹಿಂತಿರುಗಿ SQLITE_OK;
        }
      }
      rc = SQLITE_NOTADB;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಗಣ_Default_Sync_Flag(pBt, SQLITE_DEFAULT_SYNCHRONOUS+1);
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* EVIDENCE-OF: R-15465-20813 The maximum and minimum embedded payload
    ** fractions and the leaf payload fraction values must be 64, 32, and 32.
    **
    ** The original design allowed these amounts to vary, but as of
    ** version 3.6.0, we require them to be fixed.
    */
    ಆದರೆ( memcmp(&page1[21], "\100\040\040",3)!=0 ){
      ಹೋಗು page1_init_failed;
    }
    /* EVIDENCE-OF: R-51873-39618 The page size for a database file is
    ** determined by the 2-byte integer located at an offset of 16 bytes from
    ** the beginning of the database file. */
    ಪುಟ_ಗಾತ್ರ = (page1[16]<<8) | (page1[17]<<16);
    /* EVIDENCE-OF: R-25008-21688 The size of a page is a power of two
    ** between 512 and 65536 inclusive. */
    ಆದರೆ( ((ಪುಟ_ಗಾತ್ರ-1)&ಪುಟ_ಗಾತ್ರ)!=0
     || ಪುಟ_ಗಾತ್ರ>SQLITE_MAX_ಪುಟ_ಗಾತ್ರ
     || ಪುಟ_ಗಾತ್ರ<=256
    ){
      ಹೋಗು page1_init_failed;
    }
    ಪ್ರತಿಜ್ಞಾ( (ಪುಟ_ಗಾತ್ರ & 7)==0 );
    /* EVIDENCE-OF: R-59310-51205 The "reserved space" size in the 1-byte
    ** integer at offset 20 is the number of bytes of space at the end of
    ** each page to reserve for extensions.
    **
    ** EVIDENCE-OF: R-37497-42412 The size of the reserved region is
    ** determined by the one-byte unsigned integer found at an offset of 20
    ** into the database file header. */
    usable_ಗಾತ್ರ = ಪುಟ_ಗಾತ್ರ - page1[20];
    ಆದರೆ( (u32)ಪುಟ_ಗಾತ್ರ!=pBt->ಪುಟ_ಗಾತ್ರ ){
      /* After reading the first page of the database assuming a page size
      ** of BtShared.pageSize, we have discovered that the page-size is
      ** actually pageSize. Unlock the database, leave pBt->pPage1 at
      ** zero and return SQLITE_OK. The caller will call this function
      ** again with the correct page-size.
      */
      release_ಪುಟ_One(pPage1);
      pBt->usable_ಗಾತ್ರ = usable_ಗಾತ್ರ;
      pBt->ಪುಟ_ಗಾತ್ರ = ಪುಟ_ಗಾತ್ರ;
      pBt->btsFlags |= BTS_PAGESIZE_FIXED;
      free_ತಾತ್ಕಾಲಿಕ_Space(pBt);
      rc = sqlite3_Pager_ಗಣ_Pagesize(pBt->pPager, &pBt->ಪುಟ_ಗಾತ್ರ,
                                   ಪುಟ_ಗಾತ್ರ-usable_ಗಾತ್ರ);
      ಹಿಂತಿರುಗಿ rc;
    }
    ಆದರೆ( n_ಪುಟ>n_ಪುಟ_File ){
      ಆದರೆ( sqlite3WritableSchema(pBt->db)==0 ){
        rc = SQLITE_CORRUPT_BKPT;
        ಹೋಗು page1_init_failed;
      }ಇಲ್ಲದಿದ್ದರೆ{
        n_ಪುಟ = n_ಪುಟ_File;
      }
    }
    /* EVIDENCE-OF: R-28312-64704 However, the usable size is not allowed to
    ** be less than 480. In other words, if the page size is 512, then the
    ** reserved space size cannot exceed 32. */
    ಆದರೆ( usable_ಗಾತ್ರ<480 ){
      ಹೋಗು page1_init_failed;
    }
    pBt->btsFlags |= BTS_PAGESIZE_FIXED;
    pBt->ಪುಟ_ಗಾತ್ರ = ಪುಟ_ಗಾತ್ರ;
    pBt->usable_ಗಾತ್ರ = usable_ಗಾತ್ರ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
    pBt->autoVacuum = (get4byte(&page1[36 + 4*4])?1:0);
    pBt->incrVacuum = (get4byte(&page1[36 + 7*4])?1:0);
#ಆದರೆ_ಅಂತ್ಯ
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
  pBt->maxLocal = (u16)((pBt->usable_ಗಾತ್ರ-12)*64/255 - 23);
  pBt->minLocal = (u16)((pBt->usable_ಗಾತ್ರ-12)*32/255 - 23);
  pBt->max_ಎಲೆ = (u16)(pBt->usable_ಗಾತ್ರ - 35);
  pBt->min_ಎಲೆ = (u16)((pBt->usable_ಗಾತ್ರ-12)*32/255 - 23);
  ಆದರೆ( pBt->maxLocal>127 ){
    pBt->max1bytePayload = 127;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pBt->max1bytePayload = (u8)pBt->maxLocal;
  }
  ಪ್ರತಿಜ್ಞಾ( pBt->max_ಎಲೆ + 23 <= MX_CELL_ಗಾತ್ರ(pBt) );
  pBt->pPage1 = pPage1;
  pBt->n_ಪುಟ = n_ಪುಟ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;

page1_init_failed:
  release_ಪುಟ_One(pPage1);
  pBt->pPage1 = 0;
  ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_Valid_Cursors(BtShared *pBt, ಪೂರ್ಣಾಂಕ wrOnly){
  BtCursor *pCur;
  ಪೂರ್ಣಾಂಕ r = 0;
  ಗಾಗಿ(pCur=pBt->pCursor; pCur; pCur=pCur->p_ಮುಂದಿನ){
    ಆದರೆ( (wrOnly==0 || (pCur->curFlags & BTCF_WriteFlag)!=0)
     && pCur->eState!=CURSOR_FAULT ) r++;
  }
  ಹಿಂತಿರುಗಿ r;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** If there are no outstanding cursors and we are not in the middle
** of a transaction but there is a read lock on the database, then
** this routine unrefs the first page of the database file which
** has the effect of releasing the read lock.
**
** If there is a transaction in progress, this routine is a no-op.
*/
ಸ್ಥಿರ ಶೂನ್ಯ unlock_Btree_If_ಬಳಸದ(BtShared *pBt){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( ಎಣಿಕೆ_Valid_Cursors(pBt,0)==0 || pBt->ಒಳ_Transaction>TRANS_NONE );
  ಆದರೆ( pBt->ಒಳ_Transaction==TRANS_NONE && pBt->pPage1!=0 ){
    Mem_ಪುಟ *pPage1 = pBt->pPage1;
    ಪ್ರತಿಜ್ಞಾ( pPage1->a_ದತ್ತಾಂಶ );
    ಪ್ರತಿಜ್ಞಾ( sqlite3PagerRefcount(pBt->pPager)==1 );
    pBt->pPage1 = 0;
    release_ಪುಟ_One(pPage1);
  }
}

/*
** If pBt points to an empty file then convert that empty file
** into a new empty database by initializing the first page of
** the database.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ newDatabase(BtShared *pBt){
  Mem_ಪುಟ *pP1;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ದತ್ತಾಂಶ;
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಆದರೆ( pBt->n_ಪುಟ>0 ){
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  pP1 = pBt->pPage1;
  ಪ್ರತಿಜ್ಞಾ( pP1!=0 );
  ದತ್ತಾಂಶ = pP1->a_ದತ್ತಾಂಶ;
  rc = sqlite3PagerWrite(pP1->p_Db_ಪುಟ);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
  ಸ್ಮರಣೆ_ನಕಲು(ದತ್ತಾಂಶ, z_Magic_ಶೀರ್ಷಿಕೆ, ಗಾತ್ರ(z_Magic_ಶೀರ್ಷಿಕೆ));
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(z_Magic_ಶೀರ್ಷಿಕೆ)==16 );
  ದತ್ತಾಂಶ[16] = (u8)((pBt->ಪುಟ_ಗಾತ್ರ>>8)&0xff);
  ದತ್ತಾಂಶ[17] = (u8)((pBt->ಪುಟ_ಗಾತ್ರ>>16)&0xff);
  ದತ್ತಾಂಶ[18] = 1;
  ದತ್ತಾಂಶ[19] = 1;
  ಪ್ರತಿಜ್ಞಾ( pBt->usable_ಗಾತ್ರ<=pBt->ಪುಟ_ಗಾತ್ರ && pBt->usable_ಗಾತ್ರ+255>=pBt->ಪುಟ_ಗಾತ್ರ);
  ದತ್ತಾಂಶ[20] = (u8)(pBt->ಪುಟ_ಗಾತ್ರ - pBt->usable_ಗಾತ್ರ);
  ದತ್ತಾಂಶ[21] = 64;
  ದತ್ತಾಂಶ[22] = 32;
  ದತ್ತಾಂಶ[23] = 32;
  memset(&ದತ್ತಾಂಶ[24], 0, 100-24);
  zero_ಪುಟ(pP1, PTF_INTKEY|PTF_ಎಲೆ|PTF_LEAFDATA );
  pBt->btsFlags |= BTS_PAGESIZE_FIXED;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
  ಪ್ರತಿಜ್ಞಾ( pBt->autoVacuum==1 || pBt->autoVacuum==0 );
  ಪ್ರತಿಜ್ಞಾ( pBt->incrVacuum==1 || pBt->incrVacuum==0 );
  put4byte(&ದತ್ತಾಂಶ[36 + 4*4], pBt->autoVacuum);
  put4byte(&ದತ್ತಾಂಶ[36 + 7*4], pBt->incrVacuum);
#ಆದರೆ_ಅಂತ್ಯ
  pBt->n_ಪುಟ = 1;
  ದತ್ತಾಂಶ[31] = 1;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Initialize the first page of the database file (creating a database
** consisting of a single page and no schema objects). Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeNewDb(Btree *p){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3BtreeEnter(p);
  p->pBt->n_ಪುಟ = 0;
  rc = newDatabase(p->pBt);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ btreeBeginTrans(
  Btree *p,                 /* The btree in which to start the transaction */
  ಪೂರ್ಣಾಂಕ wrflag,               /* True to start a write transaction */
  ಪೂರ್ಣಾಂಕ *p_Schema_ಆವೃತ್ತಿ       /* Put schema version number here, if not NULL */
){
  BtShared *pBt = p->pBt;
  Pager *pPager = pBt->pPager;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  sqlite3BtreeEnter(p);
  btreeIntegrity(p);

  /* If the btree is already in a write-transaction, or it
  ** is already in a read-transaction and a read-transaction
  ** is requested, this is a no-op.
  */
  ಆದರೆ( p->ಒಳ_Trans==TRANS_WRITE || (p->ಒಳ_Trans==TRANS_READ && !wrflag) ){
    ಹೋಗು trans_begun;
  }
  ಪ್ರತಿಜ್ಞಾ( pBt->ಒಳ_Transaction==TRANS_WRITE || IfNotOmitAV(pBt->bDoTruncate)==0 );

  ಆದರೆ( (p->db->flags & SQLITE_ResetDatabase)
   && sqlite3PagerIsreadonly(pPager)==0
  ){
    pBt->btsFlags &= ~BTS_READ_ONLY;
  }

  /* Write transactions are not possible on a read-only database */
  ಆದರೆ( (pBt->btsFlags & BTS_READ_ONLY)!=0 && wrflag ){
    rc = SQLITE_READONLY;
    ಹೋಗು trans_begun;
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
  {
    sqlite3 *p_ಖಂಡ = 0;
    /* If another database handle has already opened a write transaction
    ** on this shared-btree structure and a second write transaction is
    ** requested, return SQLITE_LOCKED.
    */
    ಆದರೆ( (wrflag && pBt->ಒಳ_Transaction==TRANS_WRITE)
     || (pBt->btsFlags & BTS_PENDING)!=0
    ){
      p_ಖಂಡ = pBt->pWriter->db;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( wrflag>1 ){
      Bt_ಬೀಗ *pIter;
      ಗಾಗಿ(pIter=pBt->p_ಬೀಗ; pIter; pIter=pIter->p_ಮುಂದಿನ){
        ಆದರೆ( pIter->pBtree!=p ){
          p_ಖಂಡ = pIter->pBtree->db;
          ನಿಲ್ಲಿಸು;
        }
      }
    }
    ಆದರೆ( p_ಖಂಡ ){
      sqlite3ConnectionBlocked(p->db, p_ಖಂಡ);
      rc = SQLITE_LOCKED_SHAREDCACHE;
      ಹೋಗು trans_begun;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Any read-only or read-write transaction implies a read-lock on
  ** page 1. So if some other shared-cache client already has a write-lock
  ** on page 1, the transaction cannot be opened. */
  rc = query_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(p, SCHEMA_ಬೇರು, READ_ಬೀಗ);
  ಆದರೆ( SQLITE_OK!=rc ) ಹೋಗು trans_begun;

  pBt->btsFlags &= ~BTS_INITIALLY_ಖಾಲಿಗಳು;
  ಆದರೆ( pBt->n_ಪುಟ==0 ) pBt->btsFlags |= BTS_INITIALLY_ಖಾಲಿಗಳು;
  ಮಾಡು {
    sqlite3PagerWalDb(pPager, p->db);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
    /* If transitioning from no transaction directly to a write transaction,
    ** block for the WRITER lock first if possible. */
    ಆದರೆ( pBt->pPage1==0 && wrflag ){
      ಪ್ರತಿಜ್ಞಾ( pBt->ಒಳ_Transaction==TRANS_NONE );
      rc = sqlite3_Pager_Wal_Write_ಬೀಗ(pPager, 1);
      ಆದರೆ( rc!=SQLITE_BUSY && rc!=SQLITE_OK ) ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_EXPERIMENTAL_PRAGMA_20251114
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
    ಆದರೆ( pBt->pPage1==0 && wrflag ){
      sqlite3_file *fd = sqlite3PagerFile(pPager);
      ಅಕ್ಷರ *aFcntl[3] = {0,0,0};
      aFcntl[1] = "experimental_pragma_20251114";
      ಪ್ರತಿಜ್ಞಾ( wrflag==1 || wrflag==2 );
      aFcntl[2] = (wrflag==1 ? "1" : "2");
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_PRAGMA, (ಶೂನ್ಯ*)aFcntl);
      sqlite3_free(aFcntl[0]);
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* Call lockBtree() until either pBt->pPage1 is populated or
    ** lockBtree() returns something other than SQLITE_OK. lockBtree()
    ** may return SQLITE_OK but leave pBt->pPage1 set to 0 if after
    ** reading page 1 it discovers that the page-size of the database
    ** file is not pBt->pageSize. In this case lockBtree() will update
    ** pBt->pageSize to the page-size of the file on disk.
    */
    ಆಗ( pBt->pPage1==0 && SQLITE_OK==(rc = ಬೀಗ_Btree(pBt)) );

    ಆದರೆ( rc==SQLITE_OK && wrflag ){
      ಆದರೆ( (pBt->btsFlags & BTS_READ_ONLY)!=0 ){
        rc = SQLITE_READONLY;
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = sqlite3PagerBegin(pPager, wrflag>1, sqlite3_ತಾತ್ಕಾಲಿಕ_ಒಳ_ಸ್ಮರಣೆ(p->db));
        ಆದರೆ( rc==SQLITE_OK ){
          rc = newDatabase(pBt);
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==SQLITE_BUSY_SNAPSHOT && pBt->ಒಳ_Transaction==TRANS_NONE ){
          /* if there was no transaction opened when this function was
          ** called and SQLITE_BUSY_SNAPSHOT is returned, change the error
          ** code to SQLITE_BUSY. */
          rc = SQLITE_BUSY;
        }
      }
    }
 
    ಆದರೆ( rc!=SQLITE_OK ){
      (ಶೂನ್ಯ)sqlite3_Pager_Wal_Write_ಬೀಗ(pPager, 0);
      unlock_Btree_If_ಬಳಸದ(pBt);
    }
#ಆದರೆ defined(SQLITE_ENABLE_SETLK_TIMEOUT)
    ಆದರೆ( rc==SQLITE_BUSY_TIMEOUT ){
      /* If a blocking lock timed out, break out of the loop here so that
      ** the busy-handler is not invoked.  */
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }ಆಗ( (rc&0xFF)==SQLITE_BUSY && pBt->ಒಳ_Transaction==TRANS_NONE &&
          btreeInvokeBusyHandler(pBt) );
  sqlite3PagerWalDb(pPager, 0);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SETLK_TIMEOUT
  ಆದರೆ( rc==SQLITE_BUSY_TIMEOUT ) rc = SQLITE_BUSY;
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( rc==SQLITE_OK ){
    ಆದರೆ( p->ಒಳ_Trans==TRANS_NONE ){
      pBt->nTransaction++;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
      ಆದರೆ( p->sharable ){
        ಪ್ರತಿಜ್ಞಾ( p->ಬೀಗ.pBtree==p && p->ಬೀಗ.i_ಕೋಷ್ಟಕ==1 );
        p->ಬೀಗ.e_ಬೀಗ = READ_ಬೀಗ;
        p->ಬೀಗ.p_ಮುಂದಿನ = pBt->p_ಬೀಗ;
        pBt->p_ಬೀಗ = &p->ಬೀಗ;
      }
#ಆದರೆ_ಅಂತ್ಯ
    }
    p->ಒಳ_Trans = (wrflag?TRANS_WRITE:TRANS_READ);
    ಆದರೆ( p->ಒಳ_Trans>pBt->ಒಳ_Transaction ){
      pBt->ಒಳ_Transaction = p->ಒಳ_Trans;
    }
    ಆದರೆ( wrflag ){
      Mem_ಪುಟ *pPage1 = pBt->pPage1;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
      ಪ್ರತಿಜ್ಞಾ( !pBt->pWriter );
      pBt->pWriter = p;
      pBt->btsFlags &= ~BTS_EXCLUSIVE;
      ಆದರೆ( wrflag>1 ) pBt->btsFlags |= BTS_EXCLUSIVE;
#ಆದರೆ_ಅಂತ್ಯ

      /* If the db-size header field is incorrect (as it may be if an old
      ** client has been writing the database file), update it now. Doing
      ** this sooner rather than later means the database size can safely
      ** re-read the database size from page 1 if a savepoint or transaction
      ** rollback occurs within the transaction.
      */
      ಆದರೆ( pBt->n_ಪುಟ!=get4byte(&pPage1->a_ದತ್ತಾಂಶ[28]) ){
        rc = sqlite3PagerWrite(pPage1->p_Db_ಪುಟ);
        ಆದರೆ( rc==SQLITE_OK ){
          put4byte(&pPage1->a_ದತ್ತಾಂಶ[28], pBt->n_ಪುಟ);
        }
      }
    }
  }

trans_begun:
  ಆದರೆ( rc==SQLITE_OK ){
    ಆದರೆ( p_Schema_ಆವೃತ್ತಿ ){
      *p_Schema_ಆವೃತ್ತಿ = get4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[40]);
    }
    ಆದರೆ( wrflag ){
      /* This call makes sure that the pager has the correct number of
      ** open savepoints. If the second parameter is greater than 0 and
      ** the sub-journal is not already open, then it will be opened here.
      */
      rc = sqlite3PagerOpenSavepoint(pPager, p->db->nSavepoint);
    }
  }

  btreeIntegrity(p);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3BtreeBeginTrans(Btree *p, ಪೂರ್ಣಾಂಕ wrflag, ಪೂರ್ಣಾಂಕ *p_Schema_ಆವೃತ್ತಿ){
  BtShared *pBt;
  ಆದರೆ( p->sharable
   || p->ಒಳ_Trans==TRANS_NONE
   || (p->ಒಳ_Trans==TRANS_READ && wrflag!=0)
  ){
    ಹಿಂತಿರುಗಿ btreeBeginTrans(p,wrflag,p_Schema_ಆವೃತ್ತಿ);
  }
  pBt = p->pBt;
  ಆದರೆ( p_Schema_ಆವೃತ್ತಿ ){
    *p_Schema_ಆವೃತ್ತಿ = get4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[40]);
  }
  ಆದರೆ( wrflag ){
    /* This call makes sure that the pager has the correct number of
    ** open savepoints. If the second parameter is greater than 0 and
    ** the sub-journal is not already open, then it will be opened here.
    */
    ಹಿಂತಿರುಗಿ sqlite3PagerOpenSavepoint(pBt->pPager, p->db->nSavepoint);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM

/*
** Set the pointer-map entries for all children of page pPage. Also, if
** pPage contains cells that point to overflow pages, set the pointer
** map entries for the overflow pages as well.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಗಣ_ಮಗು_Ptrmaps(Mem_ಪುಟ *p_ಪುಟ){
  ಪೂರ್ಣಾಂಕ i;                             /* Counter variable */
  ಪೂರ್ಣಾಂಕ nCell;                         /* Number of cells in page pPage */
  ಪೂರ್ಣಾಂಕ rc;                            /* Return code */
  BtShared *pBt = p_ಪುಟ->pBt;
  Pgno pgno = p_ಪುಟ->pgno;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  rc = p_ಪುಟ->isInit ? SQLITE_OK : btree_Init_ಪುಟ(p_ಪುಟ);
  ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;
  nCell = p_ಪುಟ->nCell;

  ಗಾಗಿ(i=0; i<nCell; i++){
    u8 *pCell = ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, i);

    ptrmapPutOvflPtr(p_ಪುಟ, p_ಪುಟ, pCell, &rc);

    ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
      Pgno ಮಗು_Pgno = get4byte(pCell);
      ptrmapPut(pBt, ಮಗು_Pgno, PTRMAP_BTREE, pgno, &rc);
    }
  }

  ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
    Pgno ಮಗು_Pgno = get4byte(&p_ಪುಟ->a_ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+8]);
    ptrmapPut(pBt, ಮಗು_Pgno, PTRMAP_BTREE, pgno, &rc);
  }

  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ modify_ಪುಟ_ಸೂಚಕ(Mem_ಪುಟ *p_ಪುಟ, Pgno iFrom, Pgno i_ಗಮ್ಯ, u8 eType){
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಪುಟ->p_Db_ಪುಟ) );
  ಆದರೆ( eType==PTRMAP_OVERFLOW2 ){
    /* The pointer is always the first 4 bytes of the page in this case.  */
    ಆದರೆ( get4byte(p_ಪುಟ->a_ದತ್ತಾಂಶ)!=iFrom ){
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }
    put4byte(p_ಪುಟ->a_ದತ್ತಾಂಶ, i_ಗಮ್ಯ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ i;
    ಪೂರ್ಣಾಂಕ nCell;
    ಪೂರ್ಣಾಂಕ rc;

    rc = p_ಪುಟ->isInit ? SQLITE_OK : btree_Init_ಪುಟ(p_ಪುಟ);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    nCell = p_ಪುಟ->nCell;

    ಗಾಗಿ(i=0; i<nCell; i++){
      u8 *pCell = ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, i);
      ಆದರೆ( eType==PTRMAP_OVERFLOW1 ){
        CellInfo info;
        p_ಪುಟ->xParseCell(p_ಪುಟ, pCell, &info);
        ಆದರೆ( info.nLocal<info.nPayload ){
          ಆದರೆ( pCell+info.n_ಗಾತ್ರ > p_ಪುಟ->a_ದತ್ತಾಂಶ+p_ಪುಟ->pBt->usable_ಗಾತ್ರ ){
            ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
          }
          ಆದರೆ( iFrom==get4byte(pCell+info.n_ಗಾತ್ರ-4) ){
            put4byte(pCell+info.n_ಗಾತ್ರ-4, i_ಗಮ್ಯ);
            ನಿಲ್ಲಿಸು;
          }
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( pCell+4 > p_ಪುಟ->a_ದತ್ತಾಂಶ+p_ಪುಟ->pBt->usable_ಗಾತ್ರ ){
          ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
        }
        ಆದರೆ( get4byte(pCell)==iFrom ){
          put4byte(pCell, i_ಗಮ್ಯ);
          ನಿಲ್ಲಿಸು;
        }
      }
    }
 
    ಆದರೆ( i==nCell ){
      ಆದರೆ( eType!=PTRMAP_BTREE ||
          get4byte(&p_ಪುಟ->a_ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+8])!=iFrom ){
        ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
      }
      put4byte(&p_ಪುಟ->a_ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+8], i_ಗಮ್ಯ);
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ relocate_ಪುಟ(
  BtShared *pBt,           /* Btree */
  Mem_ಪುಟ *p_Db_ಪುಟ,        /* Open page to move */
  u8 eType,                /* Pointer map 'type' entry for pDbPage */
  Pgno i_Ptr_ಪುಟ,           /* Pointer map 'page-no' entry for pDbPage */
  Pgno i_Free_ಪುಟ,          /* The location to move pDbPage to */
  ಪೂರ್ಣಾಂಕ isCommit             /* isCommit flag passed to sqlite3PagerMovepage */
){
  Mem_ಪುಟ *p_Ptr_ಪುಟ;   /* The page that contains a pointer to pDbPage */
  Pgno i_Db_ಪುಟ = p_Db_ಪುಟ->pgno;
  Pager *pPager = pBt->pPager;
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( eType==PTRMAP_OVERFLOW2 || eType==PTRMAP_OVERFLOW1 ||
      eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p_Db_ಪುಟ->pBt==pBt );
  ಆದರೆ( i_Db_ಪುಟ<3 ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;

  /* Move page iDbPage from its current location to page number iFreePage */
  TRACE(("AUTOVACUUM: Moving %u to free page %u (ptr page %u type %u)\n",
      i_Db_ಪುಟ, i_Free_ಪುಟ, i_Ptr_ಪುಟ, eType));
  rc = sqlite3PagerMovepage(pPager, p_Db_ಪುಟ->p_Db_ಪುಟ, i_Free_ಪುಟ, isCommit);
  ಆದರೆ( rc!=SQLITE_OK ){
    ಹಿಂತಿರುಗಿ rc;
  }
  p_Db_ಪುಟ->pgno = i_Free_ಪುಟ;

  /* If pDbPage was a btree-page, then it may have child pages and/or cells
  ** that point to overflow pages. The pointer map entries for all these
  ** pages need to be changed.
  **
  ** If pDbPage is an overflow page, then the first 4 bytes may store a
  ** pointer to a subsequent overflow page. If this is the case, then
  ** the pointer map needs to be updated for the subsequent overflow page.
  */
  ಆದರೆ( eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE ){
    rc = ಗಣ_ಮಗು_Ptrmaps(p_Db_ಪುಟ);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹಿಂತಿರುಗಿ rc;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    Pgno ಮುಂದಿನ_Ovfl = get4byte(p_Db_ಪುಟ->a_ದತ್ತಾಂಶ);
    ಆದರೆ( ಮುಂದಿನ_Ovfl!=0 ){
      ptrmapPut(pBt, ಮುಂದಿನ_Ovfl, PTRMAP_OVERFLOW2, i_Free_ಪುಟ, &rc);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ rc;
      }
    }
  }

  /* Fix the database pointer on page iPtrPage that pointed at iDbPage so
  ** that it points at iFreePage. Also fix the pointer map entry for
  ** iPtrPage.
  */
  ಆದರೆ( eType!=PTRMAP_ROOTPAGE ){
    rc = btree_Get_ಪುಟ(pBt, i_Ptr_ಪುಟ, &p_Ptr_ಪುಟ, 0);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹಿಂತಿರುಗಿ rc;
    }
    rc = sqlite3PagerWrite(p_Ptr_ಪುಟ->p_Db_ಪುಟ);
    ಆದರೆ( rc!=SQLITE_OK ){
      release_ಪುಟ(p_Ptr_ಪುಟ);
      ಹಿಂತಿರುಗಿ rc;
    }
    rc = modify_ಪುಟ_ಸೂಚಕ(p_Ptr_ಪುಟ, i_Db_ಪುಟ, i_Free_ಪುಟ, eType);
    release_ಪುಟ(p_Ptr_ಪುಟ);
    ಆದರೆ( rc==SQLITE_OK ){
      ptrmapPut(pBt, i_Free_ಪುಟ, eType, i_Ptr_ಪುಟ, &rc);
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/* Forward declaration required by incrVacuumStep(). */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ allocate_Btree_ಪುಟ(BtShared *, Mem_ಪುಟ **, Pgno *, Pgno, u8);

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ incr_Vacuum_ಹೆಜ್ಜೆ(BtShared *pBt, Pgno nFin, Pgno i_ಕೊನೆಯ_Pg, ಪೂರ್ಣಾಂಕ bCommit){
  Pgno nFreeList;           /* Number of pages still on the free-list */
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( i_ಕೊನೆಯ_Pg>nFin );

  ಆದರೆ( !PTRMAP_ISPAGE(pBt, i_ಕೊನೆಯ_Pg) && i_ಕೊನೆಯ_Pg!=PENDING_BYTE_ಪುಟ(pBt) ){
    u8 eType;
    Pgno i_Ptr_ಪುಟ;

    nFreeList = get4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[36]);
    ಆದರೆ( nFreeList==0 ){
      ಹಿಂತಿರುಗಿ SQLITE_ಮುಗಿದ;
    }

    rc = ptrmapGet(pBt, i_ಕೊನೆಯ_Pg, &eType, &i_Ptr_ಪುಟ);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹಿಂತಿರುಗಿ rc;
    }
    ಆದರೆ( eType==PTRMAP_ROOTPAGE ){
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
    }

    ಆದರೆ( eType==PTRMAP_FREEPAGE ){
      ಆದರೆ( bCommit==0 ){
        /* Remove the page from the files free-list. This is not required
        ** if bCommit is non-zero. In that case, the free-list will be
        ** truncated to zero after this function returns, so it doesn't
        ** matter if it still contains some garbage entries.
        */
        Pgno iFreePg;
        Mem_ಪುಟ *pFreePg;
        rc = allocate_Btree_ಪುಟ(pBt, &pFreePg, &iFreePg, i_ಕೊನೆಯ_Pg, BTALLOC_EXACT);
        ಆದರೆ( rc!=SQLITE_OK ){
          ಹಿಂತಿರುಗಿ rc;
        }
        ಪ್ರತಿಜ್ಞಾ( iFreePg==i_ಕೊನೆಯ_Pg );
        release_ಪುಟ(pFreePg);
      }
    } ಇಲ್ಲದಿದ್ದರೆ {
      Pgno iFreePg;             /* Index of free page to move pLastPg to */
      Mem_ಪುಟ *p_ಕೊನೆಯ_Pg;
      u8 e_ಬಹುಲಕ = BTALLOC_ANY;   /* Mode parameter for allocateBtreePage() */
      Pgno iNear = 0;           /* nearby parameter for allocateBtreePage() */

      rc = btree_Get_ಪುಟ(pBt, i_ಕೊನೆಯ_Pg, &p_ಕೊನೆಯ_Pg, 0);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ rc;
      }

      /* If bCommit is zero, this loop runs exactly once and page pLastPg
      ** is swapped with the first free page pulled off the free list.
      **
      ** On the other hand, if bCommit is greater than zero, then keep
      ** looping until a free-page located within the first nFin pages
      ** of the file is found.
      */
      ಆದರೆ( bCommit==0 ){
        e_ಬಹುಲಕ = BTALLOC_LE;
        iNear = nFin;
      }
      ಮಾಡು {
        Mem_ಪುಟ *pFreePg;
        Pgno db_ಗಾತ್ರ = btreePagecount(pBt);
        rc = allocate_Btree_ಪುಟ(pBt, &pFreePg, &iFreePg, iNear, e_ಬಹುಲಕ);
        ಆದರೆ( rc!=SQLITE_OK ){
          release_ಪುಟ(p_ಕೊನೆಯ_Pg);
          ಹಿಂತಿರುಗಿ rc;
        }
        release_ಪುಟ(pFreePg);
        ಆದರೆ( iFreePg>db_ಗಾತ್ರ ){
          release_ಪುಟ(p_ಕೊನೆಯ_Pg);
          ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
        }
      }ಆಗ( bCommit && iFreePg>nFin );
      ಪ್ರತಿಜ್ಞಾ( iFreePg<i_ಕೊನೆಯ_Pg );
     
      rc = relocate_ಪುಟ(pBt, p_ಕೊನೆಯ_Pg, eType, i_Ptr_ಪುಟ, iFreePg, bCommit);
      release_ಪುಟ(p_ಕೊನೆಯ_Pg);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ rc;
      }
    }
  }

  ಆದರೆ( bCommit==0 ){
    ಮಾಡು {
      i_ಕೊನೆಯ_Pg--;
    }ಆಗ( i_ಕೊನೆಯ_Pg==PENDING_BYTE_ಪುಟ(pBt) || PTRMAP_ISPAGE(pBt, i_ಕೊನೆಯ_Pg) );
    pBt->bDoTruncate = 1;
    pBt->n_ಪುಟ = i_ಕೊನೆಯ_Pg;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** The database opened by the first argument is an auto-vacuum database
** nOrig pages in size containing nFree free pages. Return the expected
** size of the database in pages following an auto-vacuum operation.
*/
ಸ್ಥಿರ Pgno final_Db_ಗಾತ್ರ(BtShared *pBt, Pgno nOrig, Pgno nFree){
  ಪೂರ್ಣಾಂಕ n_ನಮೂದು;                     /* Number of entries on one ptrmap page */
  Pgno nPtrmap;                   /* Number of PtrMap pages to be freed */
  Pgno nFin;                      /* Return value */

  n_ನಮೂದು = pBt->usable_ಗಾತ್ರ/5;
  nPtrmap = (nFree-nOrig+PTRMAP_PAGENO(pBt, nOrig)+n_ನಮೂದು)/n_ನಮೂದು;
  nFin = nOrig - nFree - nPtrmap;
  ಆದರೆ( nOrig>PENDING_BYTE_ಪುಟ(pBt) && nFin<PENDING_BYTE_ಪುಟ(pBt) ){
    nFin--;
  }
  ಆಗ( PTRMAP_ISPAGE(pBt, nFin) || nFin==PENDING_BYTE_ಪುಟ(pBt) ){
    nFin--;
  }

  ಹಿಂತಿರುಗಿ nFin;
}

/*
** A write-transaction must be opened before calling this function.
** It performs a single unit of work towards an incremental vacuum.
**
** If the incremental vacuum is finished after this function has run,
** SQLITE_DONE is returned. If it is not finished, but no error occurred,
** SQLITE_OK is returned. Otherwise an SQLite error code.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeIncrVacuum(Btree *p){
  ಪೂರ್ಣಾಂಕ rc;
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  ಪ್ರತಿಜ್ಞಾ( pBt->ಒಳ_Transaction==TRANS_WRITE && p->ಒಳ_Trans==TRANS_WRITE );
  ಆದರೆ( !pBt->autoVacuum ){
    rc = SQLITE_ಮುಗಿದ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    Pgno nOrig = btreePagecount(pBt);
    Pgno nFree = get4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[36]);
    Pgno nFin = final_Db_ಗಾತ್ರ(pBt, nOrig, nFree);

    ಆದರೆ( nOrig<nFin || nFree>=nOrig ){
      rc = SQLITE_CORRUPT_BKPT;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nFree>0 ){
      rc = saveAllCursors(pBt, 0, 0);
      ಆದರೆ( rc==SQLITE_OK ){
        invalidate_All_ಅತಿಪ್ರವಾಹ_Cache(pBt);
        rc = incr_Vacuum_ಹೆಜ್ಜೆ(pBt, nFin, nOrig, 0);
      }
      ಆದರೆ( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ಪುಟ);
        put4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[28], pBt->n_ಪುಟ);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = SQLITE_ಮುಗಿದ;
    }
  }
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This routine is called prior to sqlite3PagerCommit when a transaction
** is committed for an auto-vacuum database.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ autoVacuumCommit(Btree *p){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  Pager *pPager;
  BtShared *pBt;
  sqlite3 *db;
  VVA_ONLY( ಪೂರ್ಣಾಂಕ nRef );

  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  pBt = p->pBt; 
  pPager = pBt->pPager;
  VVA_ONLY( nRef = sqlite3PagerRefcount(pPager); )

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  invalidate_All_ಅತಿಪ್ರವಾಹ_Cache(pBt);
  ಪ್ರತಿಜ್ಞಾ(pBt->autoVacuum);
  ಆದರೆ( !pBt->incrVacuum ){
    Pgno nFin;         /* Number of pages in database after autovacuuming */
    Pgno nFree;        /* Number of pages on the freelist initially */
    Pgno nVac;         /* Number of pages to vacuum */
    Pgno iFree;        /* The next page to be freed */
    Pgno nOrig;        /* Database size before freeing */

    nOrig = btreePagecount(pBt);
    ಆದರೆ( PTRMAP_ISPAGE(pBt, nOrig) || nOrig==PENDING_BYTE_ಪುಟ(pBt) ){
      /* It is not possible to create a database for which the final page
      ** is either a pointer-map page or the pending-byte page. If one
      ** is encountered, this indicates corruption.
      */
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
    }

    nFree = get4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[36]);
    db = p->db;
    ಆದರೆ( db->x_Autovac_ಪುಟಗಳು ){
      ಪೂರ್ಣಾಂಕ iDb;
      ಗಾಗಿ(iDb=0; ALWAYS(iDb<db->nDb); iDb++){
        ಆದರೆ( db->aDb[iDb].pBt==p ) ನಿಲ್ಲಿಸು;
      }
      nVac = db->x_Autovac_ಪುಟಗಳು(
        db->p_Autovac_ಪುಟಗಳು_Arg,
        db->aDb[iDb].zDbSName,
        nOrig,
        nFree,
        pBt->ಪುಟ_ಗಾತ್ರ
      );
      ಆದರೆ( nVac>nFree ){
        nVac = nFree;
      }
      ಆದರೆ( nVac==0 ){
        ಹಿಂತಿರುಗಿ SQLITE_OK;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      nVac = nFree;
    }
    nFin = final_Db_ಗಾತ್ರ(pBt, nOrig, nVac);
    ಆದರೆ( nFin>nOrig ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
    ಆದರೆ( nFin<nOrig ){
      rc = saveAllCursors(pBt, 0, 0);
    }
    ಗಾಗಿ(iFree=nOrig; iFree>nFin && rc==SQLITE_OK; iFree--){
      rc = incr_Vacuum_ಹೆಜ್ಜೆ(pBt, nFin, iFree, nVac==nFree);
    }
    ಆದರೆ( (rc==SQLITE_ಮುಗಿದ || rc==SQLITE_OK) && nFree>0 ){
      rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ಪುಟ);
      ಆದರೆ( nVac==nFree ){
        put4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[32], 0);
        put4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[36], 0);
      }
      put4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[28], nFin);
      pBt->bDoTruncate = 1;
      pBt->n_ಪುಟ = nFin;
    }
    ಆದರೆ( rc!=SQLITE_OK ){
      sqlite3PagerRollback(pPager);
    }
  }

  ಪ್ರತಿಜ್ಞಾ( nRef>=sqlite3PagerRefcount(pPager) );
  ಹಿಂತಿರುಗಿ rc;
}

#ಇಲ್ಲದಿದ್ದರೆ /* ifndef SQLITE_OMIT_AUTOVACUUM */
# define ಗಣ_ಮಗು_Ptrmaps(x) SQLITE_OK
#ಆದರೆ_ಅಂತ್ಯ

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
ಪೂರ್ಣಾಂಕ sqlite3BtreeCommitPhaseOne(Btree *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSuperJrnl){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( p->ಒಳ_Trans==TRANS_WRITE ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
    ಆದರೆ( pBt->autoVacuum ){
      rc = autoVacuumCommit(p);
      ಆದರೆ( rc!=SQLITE_OK ){
        sqlite3BtreeLeave(p);
        ಹಿಂತಿರುಗಿ rc;
      }
    }
    ಆದರೆ( pBt->bDoTruncate ){
      sqlite3PagerTruncateImage(pBt->pPager, pBt->n_ಪುಟ);
    }
#ಆದರೆ_ಅಂತ್ಯ
    rc = sqlite3PagerCommitPhaseOne(pBt->pPager, zSuperJrnl, 0);
    sqlite3BtreeLeave(p);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** This function is called from both BtreeCommitPhaseTwo() and BtreeRollback()
** at the conclusion of a transaction.
*/
ಸ್ಥಿರ ಶೂನ್ಯ btree_ಕೊನೆ_Transaction(Btree *p){
  BtShared *pBt = p->pBt;
  sqlite3 *db = p->db;
  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(p) );

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
  pBt->bDoTruncate = 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( p->ಒಳ_Trans>TRANS_NONE && db->nVdbeRead>1 ){
    /* If there are other active statements that belong to this database
    ** handle, downgrade to a read-only transaction. The other statements
    ** may still be reading from the database.  */
    downgrade_All_Shared_Cache_ಕೋಷ್ಟಕ_Locks(p);
    p->ಒಳ_Trans = TRANS_READ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* If the handle had any kind of transaction open, decrement the
    ** transaction count of the shared btree. If the transaction count
    ** reaches 0, set the shared state to TRANS_NONE. The unlockBtreeIfUnused()
    ** call below will unlock the pager.  */
    ಆದರೆ( p->ಒಳ_Trans!=TRANS_NONE ){
      clear_All_Shared_Cache_ಕೋಷ್ಟಕ_Locks(p);
      pBt->nTransaction--;
      ಆದರೆ( 0==pBt->nTransaction ){
        pBt->ಒಳ_Transaction = TRANS_NONE;
      }
    }

    /* Set the current transaction state to TRANS_NONE and unlock the
    ** pager if this call closed the only read or write transaction.  */
    p->ಒಳ_Trans = TRANS_NONE;
    unlock_Btree_If_ಬಳಸದ(pBt);
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
ಪೂರ್ಣಾಂಕ sqlite3BtreeCommitPhaseTwo(Btree *p, ಪೂರ್ಣಾಂಕ bCleanup){

  ಆದರೆ( p->ಒಳ_Trans==TRANS_NONE ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  sqlite3BtreeEnter(p);
  btreeIntegrity(p);

  /* If the handle has a write-transaction open, commit the shared-btrees
  ** transaction and set the shared state to TRANS_READ.
  */
  ಆದರೆ( p->ಒಳ_Trans==TRANS_WRITE ){
    ಪೂರ್ಣಾಂಕ rc;
    BtShared *pBt = p->pBt;
    ಪ್ರತಿಜ್ಞಾ( pBt->ಒಳ_Transaction==TRANS_WRITE );
    ಪ್ರತಿಜ್ಞಾ( pBt->nTransaction>0 );
    rc = sqlite3PagerCommitPhaseTwo(pBt->pPager);
    ಆದರೆ( rc!=SQLITE_OK && bCleanup==0 ){
      sqlite3BtreeLeave(p);
      ಹಿಂತಿರುಗಿ rc;
    }
    p->i_BData_ಆವೃತ್ತಿ--;  /* Compensate for pPager->iDataVersion++; */
    pBt->ಒಳ_Transaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btree_ಕೊನೆ_Transaction(p);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Do both phases of a commit.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeCommit(Btree *p){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3BtreeEnter(p);
  rc = sqlite3BtreeCommitPhaseOne(p, 0);
  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3BtreeCommitPhaseTwo(p, 0);
  }
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3BtreeTripAllCursors(Btree *pBtree, ಪೂರ್ಣಾಂಕ err_ಸಂಕೇತ, ಪೂರ್ಣಾಂಕ writeOnly){
  BtCursor *p;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಪ್ರತಿಜ್ಞಾ( (writeOnly==0 || writeOnly==1) && BTCF_WriteFlag==1 );
  ಆದರೆ( pBtree ){
    sqlite3BtreeEnter(pBtree);
    ಗಾಗಿ(p=pBtree->pBt->pCursor; p; p=p->p_ಮುಂದಿನ){
      ಆದರೆ( writeOnly && (p->curFlags & BTCF_WriteFlag)==0 ){
        ಆದರೆ( p->eState==CURSOR_VALID || p->eState==CURSOR_SKIPNEXT ){
          rc = saveCursorPosition(p);
          ಆದರೆ( rc!=SQLITE_OK ){
            (ಶೂನ್ಯ)sqlite3BtreeTripAllCursors(pBtree, rc, 0);
            ನಿಲ್ಲಿಸು;
          }
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3BtreeClearCursor(p);
        p->eState = CURSOR_FAULT;
        p->skip_ಮುಂದಿನ = err_ಸಂಕೇತ;
      }
      btree_Release_All_Cursor_ಪುಟಗಳು(p);
    }
    sqlite3BtreeLeave(pBtree);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Set the pBt->nPage field correctly, according to the current
** state of the database.  Assume pBt->pPage1 is valid.
*/
ಸ್ಥಿರ ಶೂನ್ಯ btree_ಗಣ_NPage(BtShared *pBt, Mem_ಪುಟ *pPage1){
  ಪೂರ್ಣಾಂಕ n_ಪುಟ = get4byte(&pPage1->a_ದತ್ತಾಂಶ[28]);
  testcase( n_ಪುಟ==0 );
  ಆದರೆ( n_ಪುಟ==0 ) sqlite3PagerPagecount(pBt->pPager, &n_ಪುಟ);
  testcase( pBt->n_ಪುಟ!=(u32)n_ಪುಟ );
  pBt->n_ಪುಟ = n_ಪುಟ;
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
ಪೂರ್ಣಾಂಕ sqlite3BtreeRollback(Btree *p, ಪೂರ್ಣಾಂಕ trip_ಸಂಕೇತ, ಪೂರ್ಣಾಂಕ writeOnly){
  ಪೂರ್ಣಾಂಕ rc;
  BtShared *pBt = p->pBt;
  Mem_ಪುಟ *pPage1;

  ಪ್ರತಿಜ್ಞಾ( writeOnly==1 || writeOnly==0 );
  ಪ್ರತಿಜ್ಞಾ( trip_ಸಂಕೇತ==SQLITE_ABORT_ROLLBACK || trip_ಸಂಕೇತ==SQLITE_OK );
  sqlite3BtreeEnter(p);
  ಆದರೆ( trip_ಸಂಕೇತ==SQLITE_OK ){
    rc = trip_ಸಂಕೇತ = saveAllCursors(pBt, 0, 0);
    ಆದರೆ( rc ) writeOnly = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = SQLITE_OK;
  }
  ಆದರೆ( trip_ಸಂಕೇತ ){
    ಪೂರ್ಣಾಂಕ rc2 = sqlite3BtreeTripAllCursors(p, trip_ಸಂಕೇತ, writeOnly);
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || (writeOnly==0 && rc2==SQLITE_OK) );
    ಆದರೆ( rc2!=SQLITE_OK ) rc = rc2;
  }
  btreeIntegrity(p);

  ಆದರೆ( p->ಒಳ_Trans==TRANS_WRITE ){
    ಪೂರ್ಣಾಂಕ rc2;

    ಪ್ರತಿಜ್ಞಾ( TRANS_WRITE==pBt->ಒಳ_Transaction );
    rc2 = sqlite3PagerRollback(pBt->pPager);
    ಆದರೆ( rc2!=SQLITE_OK ){
      rc = rc2;
    }

    /* The rollback may have destroyed the pPage1->aData value.  So
    ** call btreeGetPage() on page 1 again to make
    ** sure pPage1->aData is set correctly. */
    ಆದರೆ( btree_Get_ಪುಟ(pBt, 1, &pPage1, 0)==SQLITE_OK ){
      btree_ಗಣ_NPage(pBt, pPage1);
      release_ಪುಟ_One(pPage1);
    }
    ಪ್ರತಿಜ್ಞಾ( ಎಣಿಕೆ_Valid_Cursors(pBt, 1)==0 );
    pBt->ಒಳ_Transaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btree_ಕೊನೆ_Transaction(p);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3BtreeBeginStmt(Btree *p, ಪೂರ್ಣಾಂಕ iStatement){
  ಪೂರ್ಣಾಂಕ rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);
  ಪ್ರತಿಜ್ಞಾ( p->ಒಳ_Trans==TRANS_WRITE );
  ಪ್ರತಿಜ್ಞಾ( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  ಪ್ರತಿಜ್ಞಾ( iStatement>0 );
  ಪ್ರತಿಜ್ಞಾ( iStatement>p->db->nSavepoint );
  ಪ್ರತಿಜ್ಞಾ( pBt->ಒಳ_Transaction==TRANS_WRITE );
  /* At the pager level, a statement transaction is a savepoint with
  ** an index greater than all savepoints created explicitly using
  ** SQL statements. It is illegal to open, release or rollback any
  ** such savepoints while the statement transaction savepoint is active.
  */
  rc = sqlite3PagerOpenSavepoint(pBt->pPager, iStatement);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3BtreeSavepoint(Btree *p, ಪೂರ್ಣಾಂಕ op, ಪೂರ್ಣಾಂಕ iSavepoint){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( p && p->ಒಳ_Trans==TRANS_WRITE ){
    BtShared *pBt = p->pBt;
    ಪ್ರತಿಜ್ಞಾ( op==SAVEPOINT_RELEASE || op==SAVEPOINT_ROLLBACK );
    ಪ್ರತಿಜ್ಞಾ( iSavepoint>=0 || (iSavepoint==-1 && op==SAVEPOINT_ROLLBACK) );
    sqlite3BtreeEnter(p);
    ಆದರೆ( op==SAVEPOINT_ROLLBACK ){
      rc = saveAllCursors(pBt, 0, 0);
    }
    ಆದರೆ( rc==SQLITE_OK ){
      rc = sqlite3PagerSavepoint(pBt->pPager, op, iSavepoint);
    }
    ಆದರೆ( rc==SQLITE_OK ){
      ಆದರೆ( iSavepoint<0 && (pBt->btsFlags & BTS_INITIALLY_ಖಾಲಿಗಳು)!=0 ){
        pBt->n_ಪುಟ = 0;
      }
      rc = newDatabase(pBt);
      btree_ಗಣ_NPage(pBt, pBt->pPage1);

      /* pBt->nPage might be zero if the database was corrupt when
      ** the transaction was started. Otherwise, it must be at least 1.  */
      ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB || pBt->n_ಪುಟ>0 );
    }
    sqlite3BtreeLeave(p);
  }
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btreeCursor(
  Btree *p,                              /* The btree */
  Pgno i_ಕೋಷ್ಟಕ,                           /* Root page of table to open */
  ಪೂರ್ಣಾಂಕ wrFlag,                            /* 1 to write. 0 read-only */
  ರಚನೆ ಕೀಲಿ_Info *p_ಕೀಲಿ_Info,              /* First arg to comparison function */
  BtCursor *pCur                         /* Space for new cursor */
){
  BtShared *pBt = p->pBt;                /* Shared b-tree handle */
  BtCursor *pX;                          /* Looping over other all cursors */

  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(p) );
  ಪ್ರತಿಜ್ಞಾ( wrFlag==0
       || wrFlag==BTREE_WRCSR
       || wrFlag==(BTREE_WRCSR|BTREE_FORDELETE)
  );

  /* The following assert statements verify that if this is a sharable
  ** b-tree database, the connection is holding the required table locks,
  ** and that no other connection has any open cursor that conflicts with
  ** this lock.  The iTable<1 term disables the check for corrupt schemas. */
  ಪ್ರತಿಜ್ಞಾ( has_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(p, i_ಕೋಷ್ಟಕ, p_ಕೀಲಿ_Info!=0, (wrFlag?2:1))
          || i_ಕೋಷ್ಟಕ<1 );
  ಪ್ರತಿಜ್ಞಾ( wrFlag==0 || !hasReadConflicts(p, i_ಕೋಷ್ಟಕ) );

  /* Assert that the caller has opened the required transaction. */
  ಪ್ರತಿಜ್ಞಾ( p->ಒಳ_Trans>TRANS_NONE );
  ಪ್ರತಿಜ್ಞಾ( wrFlag==0 || p->ಒಳ_Trans==TRANS_WRITE );
  ಪ್ರತಿಜ್ಞಾ( pBt->pPage1 && pBt->pPage1->a_ದತ್ತಾಂಶ );
  ಪ್ರತಿಜ್ಞಾ( wrFlag==0 || (pBt->btsFlags & BTS_READ_ONLY)==0 );

  ಆದರೆ( i_ಕೋಷ್ಟಕ<=1 ){
    ಆದರೆ( i_ಕೋಷ್ಟಕ<1 ){
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( btreePagecount(pBt)==0 ){
      ಪ್ರತಿಜ್ಞಾ( wrFlag==0 );
      i_ಕೋಷ್ಟಕ = 0;
    }
  }

  /* Now that no other errors can occur, finish filling in the BtCursor
  ** variables and link the cursor into the BtShared list.  */
  pCur->pgno_ಬೇರು = i_ಕೋಷ್ಟಕ;
  pCur->i_ಪುಟ = -1;
  pCur->p_ಕೀಲಿ_Info = p_ಕೀಲಿ_Info;
  pCur->pBtree = p;
  pCur->pBt = pBt;
  pCur->curFlags = 0;
  /* If there are two or more cursors on the same btree, then all such
  ** cursors *must* have the BTCF_Multiple flag set. */
  ಗಾಗಿ(pX=pBt->pCursor; pX; pX=pX->p_ಮುಂದಿನ){
    ಆದರೆ( pX->pgno_ಬೇರು==i_ಕೋಷ್ಟಕ ){
      pX->curFlags |= BTCF_ಅಪವರ್ತ್ಯ;
      pCur->curFlags = BTCF_ಅಪವರ್ತ್ಯ;
    }
  }
  pCur->eState = CURSOR_INVALID;
  pCur->p_ಮುಂದಿನ = pBt->pCursor;
  pBt->pCursor = pCur;
  ಆದರೆ( wrFlag ){
    pCur->curFlags |= BTCF_WriteFlag;
    pCur->curPagerFlags = 0;
    ಆದರೆ( pBt->pTmpSpace==0 ) ಹಿಂತಿರುಗಿ allocate_ತಾತ್ಕಾಲಿಕ_Space(pBt);
  }ಇಲ್ಲದಿದ್ದರೆ{
    pCur->curPagerFlags = PAGER_GET_READONLY;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btree_Cursor_With_ಬೀಗ(
  Btree *p,                              /* The btree */
  Pgno i_ಕೋಷ್ಟಕ,                           /* Root page of table to open */
  ಪೂರ್ಣಾಂಕ wrFlag,                            /* 1 to write. 0 read-only */
  ರಚನೆ ಕೀಲಿ_Info *p_ಕೀಲಿ_Info,              /* First arg to comparison function */
  BtCursor *pCur                         /* Space for new cursor */
){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3BtreeEnter(p);
  rc = btreeCursor(p, i_ಕೋಷ್ಟಕ, wrFlag, p_ಕೀಲಿ_Info, pCur);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3BtreeCursor(
  Btree *p,                                   /* The btree */
  Pgno i_ಕೋಷ್ಟಕ,                                /* Root page of table to open */
  ಪೂರ್ಣಾಂಕ wrFlag,                                 /* 1 to write. 0 read-only */
  ರಚನೆ ಕೀಲಿ_Info *p_ಕೀಲಿ_Info,                   /* First arg to xCompare() */
  BtCursor *pCur                              /* Write new cursor here */
){
  ಆದರೆ( p->sharable ){
    ಹಿಂತಿರುಗಿ btree_Cursor_With_ಬೀಗ(p, i_ಕೋಷ್ಟಕ, wrFlag, p_ಕೀಲಿ_Info, pCur);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ btreeCursor(p, i_ಕೋಷ್ಟಕ, wrFlag, p_ಕೀಲಿ_Info, pCur);
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
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Cursor_ಗಾತ್ರ(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ ROUND8(ಗಾತ್ರ(BtCursor));
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Return true if and only if the Btree object will be automatically
** closed with the BtCursor closes.  This is used within assert() statements
** only.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeClosesWithCursor(
  Btree *pBtree,       /* the btree object */
  BtCursor *pCur       /* Corresponding cursor */
){
  BtShared *pBt = pBtree->pBt;
  ಆದರೆ( (pBt->openFlags & BTREE_SINGLE)==0 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( pBt->pCursor!=pCur ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( pCur->p_ಮುಂದಿನ!=0 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( pCur->pBtree!=pBtree ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Initialize memory that will be converted into a BtCursor object.
**
** The simple approach here would be to memset() the entire object
** to zero.  But it turns out that the apPage[] and aiIdx[] arrays
** do not need to be zeroed and they are large, so we can save a lot
** of run-time by skipping the initialization of those elements.
*/
ಶೂನ್ಯ sqlite3BtreeCursorZero(BtCursor *p){
  memset(p, 0, offsetof(BtCursor, BTCURSOR_ಮೊದಲ_UNINIT));
}

/*
** Close a cursor.  The read lock on the database file is released
** when the last cursor is closed.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeCloseCursor(BtCursor *pCur){
  Btree *pBtree = pCur->pBtree;
  ಆದರೆ( pBtree ){
    BtShared *pBt = pCur->pBt;
    sqlite3BtreeEnter(pBtree);
    ಪ್ರತಿಜ್ಞಾ( pBt->pCursor!=0 );
    ಆದರೆ( pBt->pCursor==pCur ){
      pBt->pCursor = pCur->p_ಮುಂದಿನ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      BtCursor *pPrev = pBt->pCursor;
      ಮಾಡು{
        ಆದರೆ( pPrev->p_ಮುಂದಿನ==pCur ){
          pPrev->p_ಮುಂದಿನ = pCur->p_ಮುಂದಿನ;
          ನಿಲ್ಲಿಸು;
        }
        pPrev = pPrev->p_ಮುಂದಿನ;
      }ಆಗ( ALWAYS(pPrev) );
    }
    btree_Release_All_Cursor_ಪುಟಗಳು(pCur);
    unlock_Btree_If_ಬಳಸದ(pBt);
    sqlite3_free(pCur->a_ಅತಿಪ್ರವಾಹ);
    sqlite3_free(pCur->p_ಕೀಲಿ);
    ಆದರೆ( (pBt->openFlags & BTREE_SINGLE) && pBt->pCursor==0 ){
      /* Since the BtShared is not sharable, there is no need to
      ** worry about the missing sqlite3BtreeLeave() call here.  */
      ಪ್ರತಿಜ್ಞಾ( pBtree->sharable==0 );
      sqlite3BtreeClose(pBtree);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3BtreeLeave(pBtree);
    }
    pCur->pBtree = 0;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Make sure the BtCursor* given in the argument has a valid
** BtCursor.info structure.  If it is not already valid, call
** btreeParseCell() to fill it in.
**
** BtCursor.info is a cache of the information in the current cell.
** Using this cache reduces the number of calls to btreeParseCell().
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cellInfoEqual(CellInfo *a, CellInfo *b){
    ಆದರೆ( a->n_ಕೀಲಿ!=b->n_ಕೀಲಿ ) ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ( a->pPayload!=b->pPayload ) ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ( a->nPayload!=b->nPayload ) ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ( a->nLocal!=b->nLocal ) ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ( a->n_ಗಾತ್ರ!=b->n_ಗಾತ್ರ ) ಹಿಂತಿರುಗಿ 0;
    ಹಿಂತಿರುಗಿ 1;
  }
  ಸ್ಥಿರ ಶೂನ್ಯ assertCellInfo(BtCursor *pCur){
    CellInfo info;
    memset(&info, 0, ಗಾತ್ರ(info));
    btreeParseCell(pCur->p_ಪುಟ, pCur->ix, &info);
    ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB || cellInfoEqual(&info, &pCur->info) );
  }
#ಇಲ್ಲದಿದ್ದರೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು assertCellInfo(x)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ getCellInfo(BtCursor *pCur){
  ಆದರೆ( pCur->info.n_ಗಾತ್ರ==0 ){
    pCur->curFlags |= BTCF_ValidNKey;
    btreeParseCell(pCur->p_ಪುಟ,pCur->ix,&pCur->info);
  }ಇಲ್ಲದಿದ್ದರೆ{
    assertCellInfo(pCur);
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG  /* The next routine used only within assert() statements */
/*
** Return true if the given BtCursor is valid.  A valid cursor is one
** that is currently pointing to a row in a (non-empty) table.
** This is a verification routine is used only within assert() statements.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeCursorIsValid(BtCursor *pCur){
  ಹಿಂತಿರುಗಿ pCur && pCur->eState==CURSOR_VALID;
}
#ಆದರೆ_ಅಂತ್ಯ /* NDEBUG */
ಪೂರ್ಣಾಂಕ sqlite3BtreeCursorIsValidNN(BtCursor *pCur){
  ಪ್ರತಿಜ್ಞಾ( pCur!=0 );
  ಹಿಂತಿರುಗಿ pCur->eState==CURSOR_VALID;
}

/*
** Return the value of the integer key or "rowid" for a table btree.
** This routine is only valid for a cursor that is pointing into a
** ordinary table btree.  If the cursor points to an index btree or
** is invalid, the result of this routine is undefined.
*/
i64 sqlite3_Btree_Integer_ಕೀಲಿ(BtCursor *pCur){
  ಪ್ರತಿಜ್ಞಾ( cursorHoldsMutex(pCur) );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  ಪ್ರತಿಜ್ಞಾ( pCur->cur_Int_ಕೀಲಿ );
  getCellInfo(pCur);
  ಹಿಂತಿರುಗಿ pCur->info.n_ಕೀಲಿ;
}

/*
** Pin or unpin a cursor.
*/
ಶೂನ್ಯ sqlite3BtreeCursorPin(BtCursor *pCur){
  ಪ್ರತಿಜ್ಞಾ( (pCur->curFlags & BTCF_Pinned)==0 );
  pCur->curFlags |= BTCF_Pinned;
}
ಶೂನ್ಯ sqlite3BtreeCursorUnpin(BtCursor *pCur){
  ಪ್ರತಿಜ್ಞಾ( (pCur->curFlags & BTCF_Pinned)!=0 );
  pCur->curFlags &= ~BTCF_Pinned;
}

/*
** Return the offset into the database file for the start of the
** payload to which the cursor is pointing.
*/
i64 sqlite3_Btree_ಸ್ಥಾನಾಂತರ(BtCursor *pCur){
  ಪ್ರತಿಜ್ಞಾ( cursorHoldsMutex(pCur) );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  getCellInfo(pCur);
  ಹಿಂತಿರುಗಿ (i64)pCur->pBt->ಪುಟ_ಗಾತ್ರ*((i64)pCur->p_ಪುಟ->pgno - 1) +
         (i64)(pCur->info.pPayload - pCur->p_ಪುಟ->a_ದತ್ತಾಂಶ);
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
u32 sqlite3_Btree_Payload_ಗಾತ್ರ(BtCursor *pCur){
  ಪ್ರತಿಜ್ಞಾ( cursorHoldsMutex(pCur) );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  getCellInfo(pCur);
  ಹಿಂತಿರುಗಿ pCur->info.nPayload;
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
sqlite3_int64 sqlite3_Btree_Max_ದಾಖಲೆ_ಗಾತ್ರ(BtCursor *pCur){
  ಪ್ರತಿಜ್ಞಾ( cursorHoldsMutex(pCur) );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  ಹಿಂತಿರುಗಿ pCur->pBt->ಪುಟ_ಗಾತ್ರ * (sqlite3_int64)pCur->pBt->n_ಪುಟ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_ಅತಿಪ್ರವಾಹ_ಪುಟ(
  BtShared *pBt,               /* The database file */
  Pgno ovfl,                   /* Current overflow page number */
  Mem_ಪುಟ **pp_ಪುಟ,            /* OUT: MemPage handle (may be NULL) */
  Pgno *p_Pgno_ಮುಂದಿನ              /* OUT: Next overflow page number */
){
  Pgno ಮುಂದಿನ = 0;
  Mem_ಪುಟ *p_ಪುಟ = 0;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ(p_Pgno_ಮುಂದಿನ);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
  /* Try to find the next page in the overflow list using the
  ** autovacuum pointer-map pages. Guess that the next page in
  ** the overflow list is page number (ovfl+1). If that guess turns
  ** out to be wrong, fall back to loading the data of page
  ** number ovfl to determine the next page number.
  */
  ಆದರೆ( pBt->autoVacuum ){
    Pgno pgno;
    Pgno iGuess = ovfl+1;
    u8 eType;

    ಆಗ( PTRMAP_ISPAGE(pBt, iGuess) || iGuess==PENDING_BYTE_ಪುಟ(pBt) ){
      iGuess++;
    }

    ಆದರೆ( iGuess<=btreePagecount(pBt) ){
      rc = ptrmapGet(pBt, iGuess, &eType, &pgno);
      ಆದರೆ( rc==SQLITE_OK && eType==PTRMAP_OVERFLOW2 && pgno==ovfl ){
        ಮುಂದಿನ = iGuess;
        rc = SQLITE_ಮುಗಿದ;
      }
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( ಮುಂದಿನ==0 || rc==SQLITE_ಮುಗಿದ );
  ಆದರೆ( rc==SQLITE_OK ){
    rc = btree_Get_ಪುಟ(pBt, ovfl, &p_ಪುಟ, (pp_ಪುಟ==0) ? PAGER_GET_READONLY : 0);
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || p_ಪುಟ==0 );
    ಆದರೆ( rc==SQLITE_OK ){
      ಮುಂದಿನ = get4byte(p_ಪುಟ->a_ದತ್ತಾಂಶ);
    }
  }

  *p_Pgno_ಮುಂದಿನ = ಮುಂದಿನ;
  ಆದರೆ( pp_ಪುಟ ){
    *pp_ಪುಟ = p_ಪುಟ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    release_ಪುಟ(p_ಪುಟ);
  }
  ಹಿಂತಿರುಗಿ (rc==SQLITE_ಮುಗಿದ ? SQLITE_OK : rc);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ copyPayload(
  ಶೂನ್ಯ *pPayload,           /* Pointer to page data */
  ಶೂನ್ಯ *pBuf,               /* Pointer to buffer */
  ಪೂರ್ಣಾಂಕ nByte,                /* Number of bytes to copy */
  ಪೂರ್ಣಾಂಕ eOp,                  /* 0 -> copy from page, 1 -> copy to page */
  Db_ಪುಟ *p_Db_ಪುಟ           /* Page containing pPayload */
){
  ಆದರೆ( eOp ){
    /* Copy data from buffer to page (a write operation) */
    ಪೂರ್ಣಾಂಕ rc = sqlite3PagerWrite(p_Db_ಪುಟ);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹಿಂತಿರುಗಿ rc;
    }
    ಸ್ಮರಣೆ_ನಕಲು(pPayload, pBuf, nByte);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Copy data from page to buffer (a read operation) */
    ಸ್ಮರಣೆ_ನಕಲು(pBuf, pPayload, nByte);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ accessPayload(
  BtCursor *pCur,      /* Cursor pointing to entry to read from */
  u32 ಸ್ಥಾನಾಂತರ,          /* Begin reading this far into payload */
  u32 amt,             /* Read this many bytes */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pBuf, /* Write the bytes into this buffer */
  ಪೂರ್ಣಾಂಕ eOp              /* zero to read. non-zero to write. */
){
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *aPayload;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ iIdx = 0;
  Mem_ಪುಟ *p_ಪುಟ = pCur->p_ಪುಟ;               /* Btree page of current entry */
  BtShared *pBt = pCur->pBt;                  /* Btree this cursor belongs to */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ನೇರ_ಅತಿಪ್ರವಾಹ_READ
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ p_Buf_ಆರಂಭ = pBuf;     /* Start of original out buffer */
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ );
  ಪ್ರತಿಜ್ಞಾ( eOp==0 || eOp==1 );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  ಆದರೆ( pCur->ix>=p_ಪುಟ->nCell ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
  }
  ಪ್ರತಿಜ್ಞಾ( cursorHoldsMutex(pCur) );

  getCellInfo(pCur);
  aPayload = pCur->info.pPayload;
  ಪ್ರತಿಜ್ಞಾ( (u64)ಸ್ಥಾನಾಂತರ+(u64)amt <= (u64)pCur->info.nPayload );

  ಪ್ರತಿಜ್ಞಾ( aPayload > p_ಪುಟ->a_ದತ್ತಾಂಶ );
  ಆದರೆ( (uptr)(aPayload - p_ಪುಟ->a_ದತ್ತಾಂಶ) > (pBt->usable_ಗಾತ್ರ - pCur->info.nLocal) ){
    /* Trying to read or write past the end of the data is an error.  The
    ** conditional above is really:
    **    &aPayload[pCur->info.nLocal] > &pPage->aData[pBt->usableSize]
    ** but is recast into its current form to avoid integer overflow problems
    */
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
  }

  /* Check if data must be read/written to/from the btree page itself. */
  ಆದರೆ( ಸ್ಥಾನಾಂತರ<pCur->info.nLocal ){
    ಪೂರ್ಣಾಂಕ a = amt;
    ಆದರೆ( a+ಸ್ಥಾನಾಂತರ>pCur->info.nLocal ){
      a = pCur->info.nLocal - ಸ್ಥಾನಾಂತರ;
    }
    rc = copyPayload(&aPayload[ಸ್ಥಾನಾಂತರ], pBuf, a, eOp, p_ಪುಟ->p_Db_ಪುಟ);
    ಸ್ಥಾನಾಂತರ = 0;
    pBuf += a;
    amt -= a;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಸ್ಥಾನಾಂತರ -= pCur->info.nLocal;
  }


  ಆದರೆ( rc==SQLITE_OK && amt>0 ){
    ಸ್ಥಿರಾಂಕ u32 ovfl_ಗಾತ್ರ = pBt->usable_ಗಾತ್ರ - 4;  /* Bytes content per ovfl page */
    Pgno ಮುಂದಿನ_ಪುಟ;

    ಮುಂದಿನ_ಪುಟ = get4byte(&aPayload[pCur->info.nLocal]);

    /* If the BtCursor.aOverflow[] has not been allocated, allocate it now.
    **
    ** The aOverflow[] array is sized at one entry for each overflow page
    ** in the overflow chain. The page number of the first overflow page is
    ** stored in aOverflow[0], etc. A value of 0 in the aOverflow[] array
    ** means "not yet known" (the cache is lazily populated).
    */
    ಆದರೆ( (pCur->curFlags & BTCF_ValidOvfl)==0 ){
      i64 nOvfl = pCur->info.nPayload;
      testcase( nOvfl - pCur->info.nLocal + ovfl_ಗಾತ್ರ - 1 > 0xffffffffU );
      nOvfl = (nOvfl - pCur->info.nLocal + ovfl_ಗಾತ್ರ-1)/ovfl_ಗಾತ್ರ;
      ಆದರೆ( pCur->a_ಅತಿಪ್ರವಾಹ==0
       || nOvfl*(ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(Pgno) > sqlite3_Malloc_ಗಾತ್ರ(pCur->a_ಅತಿಪ್ರವಾಹ)
      ){
        Pgno *aNew;
        ಆದರೆ( sqlite3FaultSim(413) ){
          aNew = 0;
        }ಇಲ್ಲದಿದ್ದರೆ{
          aNew = (Pgno*)sqlite3Realloc(pCur->a_ಅತಿಪ್ರವಾಹ, nOvfl*2*ಗಾತ್ರ(Pgno));
        }
        ಆದರೆ( aNew==0 ){
          ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
        }ಇಲ್ಲದಿದ್ದರೆ{
          pCur->a_ಅತಿಪ್ರವಾಹ = aNew;
        }
      }
      memset(pCur->a_ಅತಿಪ್ರವಾಹ, 0, nOvfl*ಗಾತ್ರ(Pgno));
      pCur->curFlags |= BTCF_ValidOvfl;
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Sanity check the validity of the overflow page cache */
      ಪ್ರತಿಜ್ಞಾ( pCur->a_ಅತಿಪ್ರವಾಹ[0]==ಮುಂದಿನ_ಪುಟ
           || pCur->a_ಅತಿಪ್ರವಾಹ[0]==0
           || CORRUPT_DB );
      ಪ್ರತಿಜ್ಞಾ( pCur->a_ಅತಿಪ್ರವಾಹ[0]!=0 || pCur->a_ಅತಿಪ್ರವಾಹ[ಸ್ಥಾನಾಂತರ/ovfl_ಗಾತ್ರ]==0 );

      /* If the overflow page-list cache has been allocated and the
      ** entry for the first required overflow page is valid, skip
      ** directly to it.
      */
      ಆದರೆ( pCur->a_ಅತಿಪ್ರವಾಹ[ಸ್ಥಾನಾಂತರ/ovfl_ಗಾತ್ರ] ){
        iIdx = (ಸ್ಥಾನಾಂತರ/ovfl_ಗಾತ್ರ);
        ಮುಂದಿನ_ಪುಟ = pCur->a_ಅತಿಪ್ರವಾಹ[iIdx];
        ಸ್ಥಾನಾಂತರ = (ಸ್ಥಾನಾಂತರ%ovfl_ಗಾತ್ರ);
      }
    }

    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK && amt>0 );
    ಆಗ( ಮುಂದಿನ_ಪುಟ ){
      /* If required, populate the overflow page-list cache. */
      ಆದರೆ( ಮುಂದಿನ_ಪುಟ > pBt->n_ಪುಟ ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
      ಪ್ರತಿಜ್ಞಾ( pCur->a_ಅತಿಪ್ರವಾಹ[iIdx]==0
              || pCur->a_ಅತಿಪ್ರವಾಹ[iIdx]==ಮುಂದಿನ_ಪುಟ
              || CORRUPT_DB );
      pCur->a_ಅತಿಪ್ರವಾಹ[iIdx] = ಮುಂದಿನ_ಪುಟ;

      ಆದರೆ( ಸ್ಥಾನಾಂತರ>=ovfl_ಗಾತ್ರ ){
        /* The only reason to read this page is to obtain the page
        ** number for the next page in the overflow chain. The page
        ** data is not required. So first try to lookup the overflow
        ** page-list cache, if any, then fall back to the getOverflowPage()
        ** function.
        */
        ಪ್ರತಿಜ್ಞಾ( pCur->curFlags & BTCF_ValidOvfl );
        ಪ್ರತಿಜ್ಞಾ( pCur->pBtree->db==pBt->db );
        ಆದರೆ( pCur->a_ಅತಿಪ್ರವಾಹ[iIdx+1] ){
          ಮುಂದಿನ_ಪುಟ = pCur->a_ಅತಿಪ್ರವಾಹ[iIdx+1];
        }ಇಲ್ಲದಿದ್ದರೆ{
          rc = get_ಅತಿಪ್ರವಾಹ_ಪುಟ(pBt, ಮುಂದಿನ_ಪುಟ, 0, &ಮುಂದಿನ_ಪುಟ);
        }
        ಸ್ಥಾನಾಂತರ -= ovfl_ಗಾತ್ರ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* Need to read this page properly. It contains some of the
        ** range of data that is being read (eOp==0) or written (eOp!=0).
        */
        ಪೂರ್ಣಾಂಕ a = amt;
        ಆದರೆ( a + ಸ್ಥಾನಾಂತರ > ovfl_ಗಾತ್ರ ){
          a = ovfl_ಗಾತ್ರ - ಸ್ಥಾನಾಂತರ;
        }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ನೇರ_ಅತಿಪ್ರವಾಹ_READ
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
        ಆದರೆ( eOp==0                                             /* (1) */
         && ಸ್ಥಾನಾಂತರ==0                                          /* (2) */
         && sqlite3_Pager_ನೇರ_Read_Ok(pBt->pPager, ಮುಂದಿನ_ಪುಟ)    /* (3,4,5) */
         && &pBuf[-4]>=p_Buf_ಆರಂಭ                               /* (6) */
        ){
          sqlite3_file *fd = sqlite3PagerFile(pBt->pPager);
          u8 aSave[4];
          u8 *aWrite = &pBuf[-4];
          ಪ್ರತಿಜ್ಞಾ( aWrite>=p_Buf_ಆರಂಭ );                         /* due to (6) */
          ಸ್ಮರಣೆ_ನಕಲು(aSave, aWrite, 4);
          rc = sqlite3OsRead(fd, aWrite, a+4, (i64)pBt->ಪುಟ_ಗಾತ್ರ*(ಮುಂದಿನ_ಪುಟ-1));
          ಮುಂದಿನ_ಪುಟ = get4byte(aWrite);
          ಸ್ಮರಣೆ_ನಕಲು(aWrite, aSave, 4);
        }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ

        {
          Db_ಪುಟ *p_Db_ಪುಟ;
          rc = sqlite3PagerGet(pBt->pPager, ಮುಂದಿನ_ಪುಟ, &p_Db_ಪುಟ,
              (eOp==0 ? PAGER_GET_READONLY : 0)
          );
          ಆದರೆ( rc==SQLITE_OK ){
            ಆದರೆ( eOp!=0
             && (sqlite3_Pager_ಪುಟ_Refcount(p_Db_ಪುಟ)!=1
                 || NEVER(((Mem_ಪುಟ*)sqlite3PagerGetExtra(p_Db_ಪುಟ))->isInit))
             && sqlite3FaultSim(411)==SQLITE_OK
            ){
              sqlite3PagerUnref(p_Db_ಪುಟ);
              ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
            }
            aPayload = sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_Db_ಪುಟ);
            ಮುಂದಿನ_ಪುಟ = get4byte(aPayload);
            rc = copyPayload(&aPayload[ಸ್ಥಾನಾಂತರ+4], pBuf, a, eOp, p_Db_ಪುಟ);
            sqlite3PagerUnref(p_Db_ಪುಟ);
            ಸ್ಥಾನಾಂತರ = 0;
          }
        }
        amt -= a;
        ಆದರೆ( amt==0 ) ಹಿಂತಿರುಗಿ rc;
        pBuf += a;
      }
      ಆದರೆ( rc ) ನಿಲ್ಲಿಸು;
      iIdx++;
    }
  }

  ಆದರೆ( rc==SQLITE_OK && amt>0 ){
    /* Overflow chain ends prematurely */
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
  }
  ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3BtreePayload(BtCursor *pCur, u32 ಸ್ಥಾನಾಂತರ, u32 amt, ಶೂನ್ಯ *pBuf){
  ಪ್ರತಿಜ್ಞಾ( cursorHoldsMutex(pCur) );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  ಪ್ರತಿಜ್ಞಾ( pCur->i_ಪುಟ>=0 && pCur->p_ಪುಟ );
  ಹಿಂತಿರುಗಿ accessPayload(pCur, ಸ್ಥಾನಾಂತರ, amt, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)pBuf, 0);
}

/*
** This variant of sqlite3BtreePayload() works even if the cursor has not
** in the CURSOR_VALID state.  It is only used by the sqlite3_blob_read()
** interface.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ accessPayloadChecked(
  BtCursor *pCur,
  u32 ಸ್ಥಾನಾಂತರ,
  u32 amt,
  ಶೂನ್ಯ *pBuf
){
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ ( pCur->eState==CURSOR_INVALID ){
    ಹಿಂತಿರುಗಿ SQLITE_ABORT;
  }
  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  rc = btreeRestoreCursorPosition(pCur);
  ಹಿಂತಿರುಗಿ rc ? rc : accessPayload(pCur, ಸ್ಥಾನಾಂತರ, amt, pBuf, 0);
}
ಪೂರ್ಣಾಂಕ sqlite3BtreePayloadChecked(BtCursor *pCur, u32 ಸ್ಥಾನಾಂತರ, u32 amt, ಶೂನ್ಯ *pBuf){
  ಆದರೆ( pCur->eState==CURSOR_VALID ){
    ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
    ಹಿಂತಿರುಗಿ accessPayload(pCur, ಸ್ಥಾನಾಂತರ, amt, pBuf, 0);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ accessPayloadChecked(pCur, ಸ್ಥಾನಾಂತರ, amt, pBuf);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_INCRBLOB */

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
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *fetchPayload(
  BtCursor *pCur,      /* Cursor pointing to entry to read from */
  u32 *pAmt            /* Write the number of available bytes here */
){
  ಪೂರ್ಣಾಂಕ amt;
  ಪ್ರತಿಜ್ಞಾ( pCur!=0 && pCur->i_ಪುಟ>=0 && pCur->p_ಪುಟ);
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( pCur->ix<pCur->p_ಪುಟ->nCell || CORRUPT_DB );
  ಪ್ರತಿಜ್ಞಾ( pCur->info.n_ಗಾತ್ರ>0 );
  ಪ್ರತಿಜ್ಞಾ( pCur->info.pPayload>pCur->p_ಪುಟ->a_ದತ್ತಾಂಶ || CORRUPT_DB );
  ಪ್ರತಿಜ್ಞಾ( pCur->info.pPayload<pCur->p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ ||CORRUPT_DB);
  amt = pCur->info.nLocal;
  ಆದರೆ( amt>(ಪೂರ್ಣಾಂಕ)(pCur->p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ - pCur->info.pPayload) ){
    /* There is too little space on the page for the expected amount
    ** of local content. Database must be corrupt. */
    ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB );
    amt = MAX(0, (ಪೂರ್ಣಾಂಕ)(pCur->p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ - pCur->info.pPayload));
  }
  *pAmt = (u32)amt;
  ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ*)pCur->info.pPayload;
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
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3BtreePayloadFetch(BtCursor *pCur, u32 *pAmt){
  ಹಿಂತಿರುಗಿ fetchPayload(pCur, pAmt);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ move_ಗಮ್ಯ_ಮಗು(BtCursor *pCur, u32 newPgno){
  ಪೂರ್ಣಾಂಕ rc;
  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  ಪ್ರತಿಜ್ಞಾ( pCur->i_ಪುಟ<BTCURSOR_MAX_ಆಳ );
  ಪ್ರತಿಜ್ಞಾ( pCur->i_ಪುಟ>=0 );
  ಆದರೆ( pCur->i_ಪುಟ>=(BTCURSOR_MAX_ಆಳ-1) ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  }
  pCur->info.n_ಗಾತ್ರ = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  pCur->aiIdx[pCur->i_ಪುಟ] = pCur->ix;
  pCur->ap_ಪುಟ[pCur->i_ಪುಟ] = pCur->p_ಪುಟ;
  pCur->ix = 0;
  pCur->i_ಪುಟ++;
  rc = get_And_Init_ಪುಟ(pCur->pBt, newPgno, &pCur->p_ಪುಟ, pCur->curPagerFlags);
  ಪ್ರತಿಜ್ಞಾ( pCur->p_ಪುಟ!=0 || rc!=SQLITE_OK );
  ಆದರೆ( rc==SQLITE_OK
   && (pCur->p_ಪುಟ->nCell<1 || pCur->p_ಪುಟ->int_ಕೀಲಿ!=pCur->cur_Int_ಕೀಲಿ)
  ){
    release_ಪುಟ(pCur->p_ಪುಟ);
    rc = SQLITE_CORRUPT_PGNO(newPgno);
  }
  ಆದರೆ( rc ){
    pCur->p_ಪುಟ = pCur->ap_ಪುಟ[--pCur->i_ಪುಟ];
  }
  ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Page pParent is an internal (non-leaf) tree page. This function
** asserts that page number iChild is the left-child if the iIdx'th
** cell in page pParent. Or, if iIdx is equal to the total number of
** cells in pParent, that page number iChild is the right-child of
** the page.
*/
ಸ್ಥಿರ ಶೂನ್ಯ assert_ಪೋಷಕ_ಸೂಚ್ಯಂಕ(Mem_ಪುಟ *p_ಪೋಷಕ, ಪೂರ್ಣಾಂಕ iIdx, Pgno i_ಮಗು){
  ಆದರೆ( CORRUPT_DB ) ಹಿಂತಿರುಗಿ;  /* The conditions tested below might not be true
                            ** in a corrupt database */
  ಪ್ರತಿಜ್ಞಾ( iIdx<=p_ಪೋಷಕ->nCell );
  ಆದರೆ( iIdx==p_ಪೋಷಕ->nCell ){
    ಪ್ರತಿಜ್ಞಾ( get4byte(&p_ಪೋಷಕ->a_ದತ್ತಾಂಶ[p_ಪೋಷಕ->hdr_ಸ್ಥಾನಾಂತರ+8])==i_ಮಗು );
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( get4byte(ಕಂಡುಹಿಡಿ_Cell(p_ಪೋಷಕ, iIdx))==i_ಮಗು );
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
#  define assert_ಪೋಷಕ_ಸೂಚ್ಯಂಕ(x,y,z)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Move the cursor up to the parent page.
**
** pCur->idx is set to the cell index that contains the pointer
** to the page we are coming from.  If we are coming from the
** right-most child page then pCur->idx is set to one more than
** the largest cell index.
*/
ಸ್ಥಿರ ಶೂನ್ಯ move_ಗಮ್ಯ_ಪೋಷಕ(BtCursor *pCur){
  Mem_ಪುಟ *p_ಎಲೆ;
  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  ಪ್ರತಿಜ್ಞಾ( pCur->i_ಪುಟ>0 );
  ಪ್ರತಿಜ್ಞಾ( pCur->p_ಪುಟ );
  assert_ಪೋಷಕ_ಸೂಚ್ಯಂಕ(
    pCur->ap_ಪುಟ[pCur->i_ಪುಟ-1],
    pCur->aiIdx[pCur->i_ಪುಟ-1],
    pCur->p_ಪುಟ->pgno
  );
  testcase( pCur->aiIdx[pCur->i_ಪುಟ-1] > pCur->ap_ಪುಟ[pCur->i_ಪುಟ-1]->nCell );
  pCur->info.n_ಗಾತ್ರ = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  pCur->ix = pCur->aiIdx[pCur->i_ಪುಟ-1];
  p_ಎಲೆ = pCur->p_ಪುಟ;
  pCur->p_ಪುಟ = pCur->ap_ಪುಟ[--pCur->i_ಪುಟ];
  release_ಪುಟ_Not_Null(p_ಎಲೆ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ move_ಗಮ್ಯ_ಬೇರು(BtCursor *pCur){
  Mem_ಪುಟ *p_ಬೇರು;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( CURSOR_INVALID < CURSOR_REQUIRESEEK );
  ಪ್ರತಿಜ್ಞಾ( CURSOR_VALID   < CURSOR_REQUIRESEEK );
  ಪ್ರತಿಜ್ಞಾ( CURSOR_FAULT   > CURSOR_REQUIRESEEK );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState < CURSOR_REQUIRESEEK || pCur->i_ಪುಟ<0 );
  ಪ್ರತಿಜ್ಞಾ( pCur->pgno_ಬೇರು>0 || pCur->i_ಪುಟ<0 );

  ಆದರೆ( pCur->i_ಪುಟ>=0 ){
    ಆದರೆ( pCur->i_ಪುಟ ){
      release_ಪುಟ_Not_Null(pCur->p_ಪುಟ);
      ಆಗ( --pCur->i_ಪುಟ ){
        release_ಪುಟ_Not_Null(pCur->ap_ಪುಟ[pCur->i_ಪುಟ]);
      }
      p_ಬೇರು = pCur->p_ಪುಟ = pCur->ap_ಪುಟ[0];
      ಹೋಗು skip_init;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pCur->pgno_ಬೇರು==0 ){
    pCur->eState = CURSOR_INVALID;
    ಹಿಂತಿರುಗಿ SQLITE_ಖಾಲಿಗಳು;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pCur->i_ಪುಟ==(-1) );
    ಆದರೆ( pCur->eState>=CURSOR_REQUIRESEEK ){
      ಆದರೆ( pCur->eState==CURSOR_FAULT ){
        ಪ್ರತಿಜ್ಞಾ( pCur->skip_ಮುಂದಿನ!=SQLITE_OK );
        ಹಿಂತಿರುಗಿ pCur->skip_ಮುಂದಿನ;
      }
      sqlite3BtreeClearCursor(pCur);
    }
    rc = get_And_Init_ಪುಟ(pCur->pBt, pCur->pgno_ಬೇರು, &pCur->p_ಪುಟ,
                        pCur->curPagerFlags);
    ಆದರೆ( rc!=SQLITE_OK ){
      pCur->eState = CURSOR_INVALID;
      ಹಿಂತಿರುಗಿ rc;
    }
    pCur->i_ಪುಟ = 0;
    pCur->cur_Int_ಕೀಲಿ = pCur->p_ಪುಟ->int_ಕೀಲಿ;
  }
  p_ಬೇರು = pCur->p_ಪುಟ;
  ಪ್ರತಿಜ್ಞಾ( p_ಬೇರು->pgno==pCur->pgno_ಬೇರು || CORRUPT_DB );

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
  ಪ್ರತಿಜ್ಞಾ( p_ಬೇರು->int_ಕೀಲಿ==1 || p_ಬೇರು->int_ಕೀಲಿ==0 );
  ಆದರೆ( p_ಬೇರು->isInit==0 || (pCur->p_ಕೀಲಿ_Info==0)!=p_ಬೇರು->int_ಕೀಲಿ ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(pCur->p_ಪುಟ);
  }

skip_init: 
  pCur->ix = 0;
  pCur->info.n_ಗಾತ್ರ = 0;
  pCur->curFlags &= ~(BTCF_At_ಕೊನೆಯ|BTCF_ValidNKey|BTCF_ValidOvfl);

  ಆದರೆ( p_ಬೇರು->nCell>0 ){
    pCur->eState = CURSOR_VALID;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !p_ಬೇರು->ಎಲೆ ){
    Pgno subpage;
    ಆದರೆ( p_ಬೇರು->pgno!=1 ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
    subpage = get4byte(&p_ಬೇರು->a_ದತ್ತಾಂಶ[p_ಬೇರು->hdr_ಸ್ಥಾನಾಂತರ+8]);
    pCur->eState = CURSOR_VALID;
    rc = move_ಗಮ್ಯ_ಮಗು(pCur, subpage);
  }ಇಲ್ಲದಿದ್ದರೆ{
    pCur->eState = CURSOR_INVALID;
    rc = SQLITE_ಖಾಲಿಗಳು;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Move the cursor down to the left-most leaf entry beneath the
** entry to which it is currently pointing.
**
** The left-most leaf is the one with the smallest key - the first
** in ascending order.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ move_ಗಮ್ಯ_Leftmost(BtCursor *pCur){
  Pgno pgno;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  Mem_ಪುಟ *p_ಪುಟ;

  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  ಆಗ( rc==SQLITE_OK && !(p_ಪುಟ = pCur->p_ಪುಟ)->ಎಲೆ ){
    ಪ್ರತಿಜ್ಞಾ( pCur->ix<p_ಪುಟ->nCell );
    pgno = get4byte(ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, pCur->ix));
    rc = move_ಗಮ್ಯ_ಮಗು(pCur, pgno);
  }
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ move_ಗಮ್ಯ_Rightmost(BtCursor *pCur){
  Pgno pgno;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  Mem_ಪುಟ *p_ಪುಟ = 0;

  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  ಆಗ( !(p_ಪುಟ = pCur->p_ಪುಟ)->ಎಲೆ ){
    pgno = get4byte(&p_ಪುಟ->a_ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+8]);
    pCur->ix = p_ಪುಟ->nCell;
    rc = move_ಗಮ್ಯ_ಮಗು(pCur, pgno);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
  }
  pCur->ix = p_ಪುಟ->nCell-1;
  ಪ್ರತಿಜ್ಞಾ( pCur->info.n_ಗಾತ್ರ==0 );
  ಪ್ರತಿಜ್ಞಾ( (pCur->curFlags & BTCF_ValidNKey)==0 );
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Move the cursor to the first entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಮೊದಲ(BtCursor *pCur, ಪೂರ್ಣಾಂಕ *pRes){
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  rc = move_ಗಮ್ಯ_ಬೇರು(pCur);
  ಆದರೆ( rc==SQLITE_OK ){
    ಪ್ರತಿಜ್ಞಾ( pCur->p_ಪುಟ->nCell>0 );
    *pRes = 0;
    rc = move_ಗಮ್ಯ_Leftmost(pCur);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==SQLITE_ಖಾಲಿಗಳು ){
    ಪ್ರತಿಜ್ಞಾ( pCur->pgno_ಬೇರು==0 || (pCur->p_ಪುಟ!=0 && pCur->p_ಪುಟ->nCell==0) );
    *pRes = 1;
    rc = SQLITE_OK;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/* Set *pRes to 1 (true) if the BTree pointed to by cursor pCur contains zero
** rows of content.  Set *pRes to 0 (false) if the table contains content.
** Return SQLITE_OK on success or some error code (ex: SQLITE_NOMEM) if
** something goes wrong.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Is_ಖಾಲಿಗಳು(BtCursor *pCur, ಪೂರ್ಣಾಂಕ *pRes){
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ಆದರೆ( NEVER(pCur->eState==CURSOR_VALID) ){
    *pRes = 0;
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  rc = move_ಗಮ್ಯ_ಬೇರು(pCur);
  ಆದರೆ( rc==SQLITE_ಖಾಲಿಗಳು ){
    *pRes = 1;
    rc = SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    *pRes = 0;
  }
  ಹಿಂತಿರುಗಿ rc;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/* The cursors is CURSOR_VALID and has BTCF_AtLast set.  Verify that
** this flags are true for a consistent database.
**
** This routine is is called from within assert() statements only.
** It is an internal verification routine and does not appear in production
** builds.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cursor_Is_At_ಕೊನೆಯ_ನಮೂದು(BtCursor *pCur){
  ಪೂರ್ಣಾಂಕ ii;
  ಗಾಗಿ(ii=0; ii<pCur->i_ಪುಟ; ii++){
    ಆದರೆ( pCur->aiIdx[ii]!=pCur->ap_ಪುಟ[ii]->nCell ) ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ pCur->ix==pCur->p_ಪುಟ->nCell-1 && pCur->p_ಪುಟ->ಎಲೆ!=0;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Move the cursor to the last entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ btree_ಕೊನೆಯ(BtCursor *pCur, ಪೂರ್ಣಾಂಕ *pRes){
  ಪೂರ್ಣಾಂಕ rc = move_ಗಮ್ಯ_ಬೇರು(pCur);
  ಆದರೆ( rc==SQLITE_OK ){
    ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
    *pRes = 0;
    rc = move_ಗಮ್ಯ_Rightmost(pCur);
    ಆದರೆ( rc==SQLITE_OK ){
      pCur->curFlags |= BTCF_At_ಕೊನೆಯ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pCur->curFlags &= ~BTCF_At_ಕೊನೆಯ;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc==SQLITE_ಖಾಲಿಗಳು ){
    ಪ್ರತಿಜ್ಞಾ( pCur->pgno_ಬೇರು==0 || pCur->p_ಪುಟ->nCell==0 );
    *pRes = 1;
    rc = SQLITE_OK;
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಕೊನೆಯ(BtCursor *pCur, ಪೂರ್ಣಾಂಕ *pRes){
  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* If the cursor already points to the last entry, this is a no-op. */
  ಆದರೆ( CURSOR_VALID==pCur->eState && (pCur->curFlags & BTCF_At_ಕೊನೆಯ)!=0 ){
    ಪ್ರತಿಜ್ಞಾ( cursor_Is_At_ಕೊನೆಯ_ನಮೂದು(pCur) || CORRUPT_DB );
    *pRes = 0;
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }
  ಹಿಂತಿರುಗಿ btree_ಕೊನೆಯ(pCur, pRes);
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
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಕೋಷ್ಟಕ_Moveto(
  BtCursor *pCur,          /* The cursor to be moved */
  i64 int_ಕೀಲಿ,              /* The table key */
  ಪೂರ್ಣಾಂಕ bias_ಬಲ,           /* If true, bias the search to the high end */
  ಪೂರ್ಣಾಂಕ *pRes                /* Write search results here */
){
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( pRes );
  ಪ್ರತಿಜ್ಞಾ( pCur->p_ಕೀಲಿ_Info==0 );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState!=CURSOR_VALID || pCur->cur_Int_ಕೀಲಿ!=0 );

  /* If the cursor is already positioned at the point we are trying
  ** to move to, then just return without doing any work */
  ಆದರೆ( pCur->eState==CURSOR_VALID && (pCur->curFlags & BTCF_ValidNKey)!=0 ){
    ಆದರೆ( pCur->info.n_ಕೀಲಿ==int_ಕೀಲಿ ){
      *pRes = 0;
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಆದರೆ( pCur->info.n_ಕೀಲಿ<int_ಕೀಲಿ ){
      ಆದರೆ( (pCur->curFlags & BTCF_At_ಕೊನೆಯ)!=0 ){
        ಪ್ರತಿಜ್ಞಾ( cursor_Is_At_ಕೊನೆಯ_ನಮೂದು(pCur) || CORRUPT_DB );
        *pRes = -1;
        ಹಿಂತಿರುಗಿ SQLITE_OK;
      }
      /* If the requested key is one more than the previous key, then
      ** try to get there using sqlite3BtreeNext() rather than a full
      ** binary search.  This is an optimization only.  The correct answer
      ** is still obtained without this case, only a little more slowly. */
      ಆದರೆ( pCur->info.n_ಕೀಲಿ+1==int_ಕೀಲಿ ){
        *pRes = 0;
        rc = sqlite3_Btree_ಮುಂದಿನ(pCur, 0);
        ಆದರೆ( rc==SQLITE_OK ){
          getCellInfo(pCur);
          ಆದರೆ( pCur->info.n_ಕೀಲಿ==int_ಕೀಲಿ ){
            ಹಿಂತಿರುಗಿ SQLITE_OK;
          }
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( rc!=SQLITE_ಮುಗಿದ ){
          ಹಿಂತಿರುಗಿ rc;
        }
      }
    }
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#ಆದರೆ_ಅಂತ್ಯ

  rc = move_ಗಮ್ಯ_ಬೇರು(pCur);
  ಆದರೆ( rc ){
    ಆದರೆ( rc==SQLITE_ಖಾಲಿಗಳು ){
      ಪ್ರತಿಜ್ಞಾ( pCur->pgno_ಬೇರು==0 || pCur->p_ಪುಟ->nCell==0 );
      *pRes = -1;
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಹಿಂತಿರುಗಿ rc;
  }
  ಪ್ರತಿಜ್ಞಾ( pCur->p_ಪುಟ );
  ಪ್ರತಿಜ್ಞಾ( pCur->p_ಪುಟ->isInit );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  ಪ್ರತಿಜ್ಞಾ( pCur->p_ಪುಟ->nCell > 0 );
  ಪ್ರತಿಜ್ಞಾ( pCur->i_ಪುಟ==0 || pCur->ap_ಪುಟ[0]->int_ಕೀಲಿ==pCur->cur_Int_ಕೀಲಿ );
  ಪ್ರತಿಜ್ಞಾ( pCur->cur_Int_ಕೀಲಿ );

  ಗಾಗಿ(;;){
    ಪೂರ್ಣಾಂಕ lwr, upr, idx, c;
    Pgno chldPg;
    Mem_ಪುಟ *p_ಪುಟ = pCur->p_ಪುಟ;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nCell>0 );
    ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->int_ಕೀಲಿ );
    lwr = 0;
    upr = p_ಪುಟ->nCell-1;
    ಪ್ರತಿಜ್ಞಾ( bias_ಬಲ==0 || bias_ಬಲ==1 );
    idx = upr>>(1-bias_ಬಲ); /* idx = biasRight ? upr : (lwr+upr)/2; */
    ಗಾಗಿ(;;){
      i64 n_Cell_ಕೀಲಿ;
      pCell = ಕಂಡುಹಿಡಿ_Cell_Past_Ptr(p_ಪುಟ, idx);
      ಆದರೆ( p_ಪುಟ->int_ಕೀಲಿ_ಎಲೆ ){
        ಆಗ( 0x80 <= *(pCell++) ){
          ಆದರೆ( pCell>=p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ ){
            ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
          }
        }
      }
      n_Cell_ಕೀಲಿ = sqlite3_Varint_ಮೌಲ್ಯ(pCell);
      ಆದರೆ( n_Cell_ಕೀಲಿ<int_ಕೀಲಿ ){
        lwr = idx+1;
        ಆದರೆ( lwr>upr ){ c = -1; ನಿಲ್ಲಿಸು; }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( n_Cell_ಕೀಲಿ>int_ಕೀಲಿ ){
        upr = idx-1;
        ಆದರೆ( lwr>upr ){ c = +1; ನಿಲ್ಲಿಸು; }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( n_Cell_ಕೀಲಿ==int_ಕೀಲಿ );
        pCur->ix = (u16)idx;
        ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
          lwr = idx;
          ಹೋಗು moveto_ಕೋಷ್ಟಕ_ಮುಂದಿನ_layer;
        }ಇಲ್ಲದಿದ್ದರೆ{
          pCur->curFlags |= BTCF_ValidNKey;
          pCur->info.n_ಕೀಲಿ = n_Cell_ಕೀಲಿ;
          pCur->info.n_ಗಾತ್ರ = 0;
          *pRes = 0;
          ಹಿಂತಿರುಗಿ SQLITE_OK;
        }
      }
      ಪ್ರತಿಜ್ಞಾ( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2; */
    }
    ಪ್ರತಿಜ್ಞಾ( lwr==upr+1 || !p_ಪುಟ->ಎಲೆ );
    ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->isInit );
    ಆದರೆ( p_ಪುಟ->ಎಲೆ ){
      ಪ್ರತಿಜ್ಞಾ( pCur->ix<pCur->p_ಪುಟ->nCell );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      ಹೋಗು moveto_ಕೋಷ್ಟಕ_ಮುಕ್ತಾಯ;
    }
moveto_ಕೋಷ್ಟಕ_ಮುಂದಿನ_layer:
    ಆದರೆ( lwr>=p_ಪುಟ->nCell ){
      chldPg = get4byte(&p_ಪುಟ->a_ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+8]);
    }ಇಲ್ಲದಿದ್ದರೆ{
      chldPg = get4byte(ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, lwr));
    }
    pCur->ix = (u16)lwr;
    rc = move_ಗಮ್ಯ_ಮಗು(pCur, chldPg);
    ಆದರೆ( rc ) ನಿಲ್ಲಿಸು;
  }
moveto_ಕೋಷ್ಟಕ_ಮುಕ್ತಾಯ:
  pCur->info.n_ಗಾತ್ರ = 0;
  ಪ್ರತಿಜ್ಞಾ( (pCur->curFlags & BTCF_ValidOvfl)==0 );
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ_Cell_Compare(
  Mem_ಪುಟ *p_ಪುಟ,
  ಪೂರ್ಣಾಂಕ idx,
  Unpacked_ದಾಖಲೆ *p_Idx_ಕೀಲಿ,
  ದಾಖಲೆ_Compare x_ದಾಖಲೆ_Compare
){
  ಪೂರ್ಣಾಂಕ c;
  ಪೂರ್ಣಾಂಕ nCell;  /* Size of the pCell cell in bytes */
  u8 *pCell = ಕಂಡುಹಿಡಿ_Cell_Past_Ptr(p_ಪುಟ, idx);

  nCell = pCell[0];
  ಆದರೆ( nCell<=p_ಪುಟ->max1bytePayload ){
    /* This branch runs if the record-size field of the cell is a
    ** single byte varint and the record fits entirely on the main
    ** b-tree page.  */
    ಆದರೆ( pCell + nCell >= p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ ) ಹಿಂತಿರುಗಿ 99;
    c = x_ದಾಖಲೆ_Compare(nCell, (ಶೂನ್ಯ*)&pCell[1], p_Idx_ಕೀಲಿ);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !(pCell[1] & 0x80)
    && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=p_ಪುಟ->maxLocal
  ){
    /* The record-size field is a 2 byte varint and the record
    ** fits entirely on the main b-tree page.  */
    ಆದರೆ( pCell + nCell >= p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ ) ಹಿಂತಿರುಗಿ 99;
    c = x_ದಾಖಲೆ_Compare(nCell, (ಶೂನ್ಯ*)&pCell[2], p_Idx_ಕೀಲಿ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* If the record extends into overflow pages, do not attempt
    ** the optimization. */
    c = 99;
  }
  ಹಿಂತಿರುಗಿ c;
}

/*
** Return true (non-zero) if pCur is current pointing to the last
** page of a table.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cursor_On_ಕೊನೆಯ_ಪುಟ(BtCursor *pCur){
  ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  ಗಾಗಿ(i=0; i<pCur->i_ಪುಟ; i++){
    Mem_ಪುಟ *p_ಪುಟ = pCur->ap_ಪುಟ[i];
    ಆದರೆ( pCur->aiIdx[i]<p_ಪುಟ->nCell ) ಹಿಂತಿರುಗಿ 0;
  }
  ಹಿಂತಿರುಗಿ 1;
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
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಸೂಚ್ಯಂಕ_Moveto(
  BtCursor *pCur,          /* The cursor to be moved */
  Unpacked_ದಾಖಲೆ *p_Idx_ಕೀಲಿ, /* Unpacked index key */
  ಪೂರ್ಣಾಂಕ *pRes                /* Write search results here */
){
  ಪೂರ್ಣಾಂಕ rc;
  ದಾಖಲೆ_Compare x_ದಾಖಲೆ_Compare;

  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( pRes );
  ಪ್ರತಿಜ್ಞಾ( pCur->p_ಕೀಲಿ_Info!=0 );

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#ಆದರೆ_ಅಂತ್ಯ

  x_ದಾಖಲೆ_Compare = sqlite3_Vdbe_ಕಂಡುಹಿಡಿ_Compare(p_Idx_ಕೀಲಿ);
  p_Idx_ಕೀಲಿ->err_ಸಂಕೇತ = 0;
  ಪ್ರತಿಜ್ಞಾ( p_Idx_ಕೀಲಿ->default_rc==1
       || p_Idx_ಕೀಲಿ->default_rc==0
       || p_Idx_ಕೀಲಿ->default_rc==-1
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
  ಆದರೆ( pCur->eState==CURSOR_VALID
   && pCur->p_ಪುಟ->ಎಲೆ
   && cursor_On_ಕೊನೆಯ_ಪುಟ(pCur)
  ){
    ಪೂರ್ಣಾಂಕ c;
    ಆದರೆ( pCur->ix==pCur->p_ಪುಟ->nCell-1
     && (c = ಸೂಚ್ಯಂಕ_Cell_Compare(pCur->p_ಪುಟ,pCur->ix,p_Idx_ಕೀಲಿ,x_ದಾಖಲೆ_Compare))<=0
     && p_Idx_ಕೀಲಿ->err_ಸಂಕೇತ==SQLITE_OK
    ){
      *pRes = c;
      ಹಿಂತಿರುಗಿ SQLITE_OK;  /* Cursor already pointing at the correct spot */
    }
    ಆದರೆ( pCur->i_ಪುಟ>0
     && ಸೂಚ್ಯಂಕ_Cell_Compare(pCur->p_ಪುಟ, 0, p_Idx_ಕೀಲಿ, x_ದಾಖಲೆ_Compare)<=0
     && p_Idx_ಕೀಲಿ->err_ಸಂಕೇತ==SQLITE_OK
    ){
      pCur->curFlags &= ~(BTCF_ValidOvfl|BTCF_At_ಕೊನೆಯ);
      ಆದರೆ( !pCur->p_ಪುಟ->isInit ){
        ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
      }
      ಹೋಗು bypass_moveto_ಬೇರು;  /* Start search on the current page */
    }
    p_Idx_ಕೀಲಿ->err_ಸಂಕೇತ = SQLITE_OK;
  }

  rc = move_ಗಮ್ಯ_ಬೇರು(pCur);
  ಆದರೆ( rc ){
    ಆದರೆ( rc==SQLITE_ಖಾಲಿಗಳು ){
      ಪ್ರತಿಜ್ಞಾ( pCur->pgno_ಬೇರು==0 || pCur->p_ಪುಟ->nCell==0 );
      *pRes = -1;
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    ಹಿಂತಿರುಗಿ rc;
  }

bypass_moveto_ಬೇರು:
  ಪ್ರತಿಜ್ಞಾ( pCur->p_ಪುಟ );
  ಪ್ರತಿಜ್ಞಾ( pCur->p_ಪುಟ->isInit );
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );
  ಪ್ರತಿಜ್ಞಾ( pCur->p_ಪುಟ->nCell > 0 );
  ಪ್ರತಿಜ್ಞಾ( pCur->cur_Int_ಕೀಲಿ==0 );
  ಪ್ರತಿಜ್ಞಾ( p_Idx_ಕೀಲಿ!=0 );
  ಗಾಗಿ(;;){
    ಪೂರ್ಣಾಂಕ lwr, upr, idx, c;
    Pgno chldPg;
    Mem_ಪುಟ *p_ಪುಟ = pCur->p_ಪುಟ;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nCell>0 );
    ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->int_ಕೀಲಿ==0 );
    lwr = 0;
    upr = p_ಪುಟ->nCell-1;
    idx = upr>>1; /* idx = (lwr+upr)/2; */
    ಗಾಗಿ(;;){
      ಪೂರ್ಣಾಂಕ nCell;  /* Size of the pCell cell in bytes */
      pCell = ಕಂಡುಹಿಡಿ_Cell_Past_Ptr(p_ಪುಟ, idx);

      /* The maximum supported page-size is 65536 bytes. This means that
      ** the maximum number of record bytes stored on an index B-Tree
      ** page is less than 16384 bytes and may be stored as a 2-byte
      ** varint. This information is used to attempt to avoid parsing
      ** the entire cell by checking for the cases where the record is
      ** stored entirely within the b-tree page by inspecting the first
      ** 2 bytes of the cell.
      */
      nCell = pCell[0];
      ಆದರೆ( nCell<=p_ಪುಟ->max1bytePayload ){
        /* This branch runs if the record-size field of the cell is a
        ** single byte varint and the record fits entirely on the main
        ** b-tree page.  */
        ಆದರೆ( pCell + nCell >= p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ ){
          rc = SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
          ಹೋಗು moveto_ಸೂಚ್ಯಂಕ_ಮುಕ್ತಾಯ;
        }
        c = x_ದಾಖಲೆ_Compare(nCell, (ಶೂನ್ಯ*)&pCell[1], p_Idx_ಕೀಲಿ);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( !(pCell[1] & 0x80)
        && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=p_ಪುಟ->maxLocal
        && pCell + nCell < p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ
      ){
        /* The record-size field is a 2 byte varint and the record
        ** fits entirely on the main b-tree page.  */
        c = x_ದಾಖಲೆ_Compare(nCell, (ಶೂನ್ಯ*)&pCell[2], p_Idx_ಕೀಲಿ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* The record flows over onto one or more overflow pages. In
        ** this case the whole cell needs to be parsed, a buffer allocated
        ** and accessPayload() used to retrieve the record into the
        ** buffer before VdbeRecordCompare() can be called.
        **
        ** If the record is corrupt, the xRecordCompare routine may read
        ** up to two varints past the end of the buffer. An extra 18
        ** bytes of padding is allocated at the end of the buffer in
        ** case this happens.  */
        ಶೂನ್ಯ *p_Cell_ಕೀಲಿ;
        u8 * ಸ್ಥಿರಾಂಕ pCellBody = pCell - p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ;
        ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ nOverrun = 18;  /* Size of the overrun padding */
        p_ಪುಟ->xParseCell(p_ಪುಟ, pCellBody, &pCur->info);
        nCell = (ಪೂರ್ಣಾಂಕ)pCur->info.n_ಕೀಲಿ;
        testcase( nCell<0 );   /* True if key size is 2^32 or more */
        testcase( nCell==0 );  /* Invalid key size:  0x80 0x80 0x00 */
        testcase( nCell==1 );  /* Invalid key size:  0x80 0x80 0x01 */
        testcase( nCell==2 );  /* Minimum legal index key size */
        ಆದರೆ( nCell<2 || nCell/pCur->pBt->usable_ಗಾತ್ರ>pCur->pBt->n_ಪುಟ ){
          rc = SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
          ಹೋಗು moveto_ಸೂಚ್ಯಂಕ_ಮುಕ್ತಾಯ;
        }
        p_Cell_ಕೀಲಿ = sqlite3Malloc( (u64)nCell+(u64)nOverrun );
        ಆದರೆ( p_Cell_ಕೀಲಿ==0 ){
          rc = SQLITE_NOMEM_BKPT;
          ಹೋಗು moveto_ಸೂಚ್ಯಂಕ_ಮುಕ್ತಾಯ;
        }
        pCur->ix = (u16)idx;
        rc = accessPayload(pCur, 0, nCell, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*)p_Cell_ಕೀಲಿ, 0);
        memset(((u8*)p_Cell_ಕೀಲಿ)+nCell,0,nOverrun); /* Fix uninit warnings */
        pCur->curFlags &= ~BTCF_ValidOvfl;
        ಆದರೆ( rc ){
          sqlite3_free(p_Cell_ಕೀಲಿ);
          ಹೋಗು moveto_ಸೂಚ್ಯಂಕ_ಮುಕ್ತಾಯ;
        }
        c = sqlite3_Vdbe_ದಾಖಲೆ_Compare(nCell, p_Cell_ಕೀಲಿ, p_Idx_ಕೀಲಿ);
        sqlite3_free(p_Cell_ಕೀಲಿ);
      }
      ಪ್ರತಿಜ್ಞಾ(
          (p_Idx_ಕೀಲಿ->err_ಸಂಕೇತ!=SQLITE_CORRUPT || c==0)
       && (p_Idx_ಕೀಲಿ->err_ಸಂಕೇತ!=SQLITE_NOMEM || pCur->pBtree->db->mallocFailed)
      );
      ಆದರೆ( c<0 ){
        lwr = idx+1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( c>0 ){
        upr = idx-1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( c==0 );
        *pRes = 0;
        rc = SQLITE_OK;
        pCur->ix = (u16)idx;
        ಆದರೆ( p_Idx_ಕೀಲಿ->err_ಸಂಕೇತ ) rc = SQLITE_CORRUPT_BKPT;
        ಹೋಗು moveto_ಸೂಚ್ಯಂಕ_ಮುಕ್ತಾಯ;
      }
      ಆದರೆ( lwr>upr ) ನಿಲ್ಲಿಸು;
      ಪ್ರತಿಜ್ಞಾ( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2 */
    }
    ಪ್ರತಿಜ್ಞಾ( lwr==upr+1 || (p_ಪುಟ->int_ಕೀಲಿ && !p_ಪುಟ->ಎಲೆ) );
    ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->isInit );
    ಆದರೆ( p_ಪುಟ->ಎಲೆ ){
      ಪ್ರತಿಜ್ಞಾ( pCur->ix<pCur->p_ಪುಟ->nCell || CORRUPT_DB );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      ಹೋಗು moveto_ಸೂಚ್ಯಂಕ_ಮುಕ್ತಾಯ;
    }
    ಆದರೆ( lwr>=p_ಪುಟ->nCell ){
      chldPg = get4byte(&p_ಪುಟ->a_ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+8]);
    }ಇಲ್ಲದಿದ್ದರೆ{
      chldPg = get4byte(ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, lwr));
    }

    /* This block is similar to an in-lined version of:
    **
    **    pCur->ix = (u16)lwr;
    **    rc = moveToChild(pCur, chldPg);
    **    if( rc ) break;
    */
    pCur->info.n_ಗಾತ್ರ = 0;
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
    ಆದರೆ( pCur->i_ಪುಟ>=(BTCURSOR_MAX_ಆಳ-1) ){
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
    }
    pCur->aiIdx[pCur->i_ಪುಟ] = (u16)lwr;
    pCur->ap_ಪುಟ[pCur->i_ಪುಟ] = pCur->p_ಪುಟ;
    pCur->ix = 0;
    pCur->i_ಪುಟ++;
    rc = get_And_Init_ಪುಟ(pCur->pBt, chldPg, &pCur->p_ಪುಟ, pCur->curPagerFlags);
    ಆದರೆ( rc==SQLITE_OK
     && (pCur->p_ಪುಟ->nCell<1 || pCur->p_ಪುಟ->int_ಕೀಲಿ!=pCur->cur_Int_ಕೀಲಿ)
    ){
      release_ಪುಟ(pCur->p_ಪುಟ);
      rc = SQLITE_CORRUPT_PGNO(chldPg);
    }
    ಆದರೆ( rc ){
      pCur->p_ಪುಟ = pCur->ap_ಪುಟ[--pCur->i_ಪುಟ];
      ನಿಲ್ಲಿಸು;
    }
    /*
    ***** End of in-lined moveToChild() call */
 }
moveto_ಸೂಚ್ಯಂಕ_ಮುಕ್ತಾಯ:
  pCur->info.n_ಗಾತ್ರ = 0;
  ಪ್ರತಿಜ್ಞಾ( (pCur->curFlags & BTCF_ValidOvfl)==0 );
  ಹಿಂತಿರುಗಿ rc;
}


/*
** Return TRUE if the cursor is not pointing at an entry of the table.
**
** TRUE will be returned after a call to sqlite3BtreeNext() moves
** past the last entry in the table or sqlite3BtreePrev() moves past
** the first entry.  TRUE is also returned if the table is empty.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeEof(BtCursor *pCur){
  /* TODO: What if the cursor is in CURSOR_REQUIRESEEK but all table entries
  ** have been deleted? This API will need to change to return an error code
  ** as well as the boolean result value.
  */
  ಹಿಂತಿರುಗಿ (CURSOR_VALID!=pCur->eState);
}

/*
** Return an estimate for the number of rows in the table that pCur is
** pointing to.  Return a negative number if no estimate is currently
** available.
*/
i64 sqlite3_Btree_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ_Est(BtCursor *pCur){
  i64 n;
  u8 i;

  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* Currently this interface is only called by the OP_IfSizeBetween
  ** opcode and the OP_Count opcode with P3=1.  In either case,
  ** the cursor will always be valid unless the btree is empty. */
  ಆದರೆ( pCur->eState!=CURSOR_VALID ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( NEVER(pCur->p_ಪುಟ->ಎಲೆ==0) ) ಹಿಂತಿರುಗಿ -1;

  n = pCur->p_ಪುಟ->nCell;
  ಗಾಗಿ(i=0; i<pCur->i_ಪುಟ; i++){
    n *= pCur->ap_ಪುಟ[i]->nCell+1;
  }
  ಹಿಂತಿರುಗಿ n;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ btree_ಮುಂದಿನ(BtCursor *pCur){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ idx;
  Mem_ಪುಟ *p_ಪುಟ;

  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಆದರೆ( pCur->eState!=CURSOR_VALID ){
    ಪ್ರತಿಜ್ಞಾ( (pCur->curFlags & BTCF_ValidOvfl)==0 );
    rc = restoreCursorPosition(pCur);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹಿಂತಿರುಗಿ rc;
    }
    ಆದರೆ( CURSOR_INVALID==pCur->eState ){
      ಹಿಂತಿರುಗಿ SQLITE_ಮುಗಿದ;
    }
    ಆದರೆ( pCur->eState==CURSOR_SKIPNEXT ){
      pCur->eState = CURSOR_VALID;
      ಆದರೆ( pCur->skip_ಮುಂದಿನ>0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
  }

  p_ಪುಟ = pCur->p_ಪುಟ;
  idx = ++pCur->ix;
  ಆದರೆ( sqlite3FaultSim(412) ) p_ಪುಟ->isInit = 0;
  ಆದರೆ( !p_ಪುಟ->isInit ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  }

  ಆದರೆ( idx>=p_ಪುಟ->nCell ){
    ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
      rc = move_ಗಮ್ಯ_ಮಗು(pCur, get4byte(&p_ಪುಟ->a_ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+8]));
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
      ಹಿಂತಿರುಗಿ move_ಗಮ್ಯ_Leftmost(pCur);
    }
    ಮಾಡು{
      ಆದರೆ( pCur->i_ಪುಟ==0 ){
        pCur->eState = CURSOR_INVALID;
        ಹಿಂತಿರುಗಿ SQLITE_ಮುಗಿದ;
      }
      move_ಗಮ್ಯ_ಪೋಷಕ(pCur);
      p_ಪುಟ = pCur->p_ಪುಟ;
    }ಆಗ( pCur->ix>=p_ಪುಟ->nCell );
    ಆದರೆ( p_ಪುಟ->int_ಕೀಲಿ ){
      ಹಿಂತಿರುಗಿ sqlite3_Btree_ಮುಂದಿನ(pCur, 0);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
  }
  ಆದರೆ( p_ಪುಟ->ಎಲೆ ){
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ move_ಗಮ್ಯ_Leftmost(pCur);
  }
}
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಮುಂದಿನ(BtCursor *pCur, ಪೂರ್ಣಾಂಕ flags){
  Mem_ಪುಟ *p_ಪುಟ;
  ಬಳಸದ_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( flags==0 || flags==1 );
  pCur->info.n_ಗಾತ್ರ = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  ಆದರೆ( pCur->eState!=CURSOR_VALID ) ಹಿಂತಿರುಗಿ btree_ಮುಂದಿನ(pCur);
  p_ಪುಟ = pCur->p_ಪುಟ;
  ಆದರೆ( (++pCur->ix)>=p_ಪುಟ->nCell ){
    pCur->ix--;
    ಹಿಂತಿರುಗಿ btree_ಮುಂದಿನ(pCur);
  }
  ಆದರೆ( p_ಪುಟ->ಎಲೆ ){
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ move_ಗಮ್ಯ_Leftmost(pCur);
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ btree_ಹಿಂದಿನ(BtCursor *pCur){
  ಪೂರ್ಣಾಂಕ rc;
  Mem_ಪುಟ *p_ಪುಟ;

  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( (pCur->curFlags & (BTCF_At_ಕೊನೆಯ|BTCF_ValidOvfl|BTCF_ValidNKey))==0 );
  ಪ್ರತಿಜ್ಞಾ( pCur->info.n_ಗಾತ್ರ==0 );
  ಆದರೆ( pCur->eState!=CURSOR_VALID ){
    rc = restoreCursorPosition(pCur);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹಿಂತಿರುಗಿ rc;
    }
    ಆದರೆ( CURSOR_INVALID==pCur->eState ){
      ಹಿಂತಿರುಗಿ SQLITE_ಮುಗಿದ;
    }
    ಆದರೆ( CURSOR_SKIPNEXT==pCur->eState ){
      pCur->eState = CURSOR_VALID;
      ಆದರೆ( pCur->skip_ಮುಂದಿನ<0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
  }

  p_ಪುಟ = pCur->p_ಪುಟ;
  ಆದರೆ( sqlite3FaultSim(412) ) p_ಪುಟ->isInit = 0;
  ಆದರೆ( !p_ಪುಟ->isInit ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  }
  ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
    ಪೂರ್ಣಾಂಕ idx = pCur->ix;
    rc = move_ಗಮ್ಯ_ಮಗು(pCur, get4byte(ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, idx)));
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    rc = move_ಗಮ್ಯ_Rightmost(pCur);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆಗ( pCur->ix==0 ){
      ಆದರೆ( pCur->i_ಪುಟ==0 ){
        pCur->eState = CURSOR_INVALID;
        ಹಿಂತಿರುಗಿ SQLITE_ಮುಗಿದ;
      }
      move_ಗಮ್ಯ_ಪೋಷಕ(pCur);
    }
    ಪ್ರತಿಜ್ಞಾ( pCur->info.n_ಗಾತ್ರ==0 );
    ಪ್ರತಿಜ್ಞಾ( (pCur->curFlags & (BTCF_ValidOvfl))==0 );

    pCur->ix--;
    p_ಪುಟ = pCur->p_ಪುಟ;
    ಆದರೆ( p_ಪುಟ->int_ಕೀಲಿ && !p_ಪುಟ->ಎಲೆ ){
      rc = sqlite3_Btree_ಹಿಂದಿನ(pCur, 0);
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = SQLITE_OK;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಹಿಂದಿನ(BtCursor *pCur, ಪೂರ್ಣಾಂಕ flags){
  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( flags==0 || flags==1 );
  ಬಳಸದ_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  pCur->curFlags &= ~(BTCF_At_ಕೊನೆಯ|BTCF_ValidOvfl|BTCF_ValidNKey);
  pCur->info.n_ಗಾತ್ರ = 0;
  ಆದರೆ( pCur->eState!=CURSOR_VALID
   || pCur->ix==0
   || pCur->p_ಪುಟ->ಎಲೆ==0
  ){
    ಹಿಂತಿರುಗಿ btree_ಹಿಂದಿನ(pCur);
  }
  pCur->ix--;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ allocate_Btree_ಪುಟ(
  BtShared *pBt,         /* The btree */
  Mem_ಪುಟ **pp_ಪುಟ,      /* Store pointer to the allocated page here */
  Pgno *pPgno,           /* Store the page number here */
  Pgno nearby,           /* Search for a page near this one */
  u8 e_ಬಹುಲಕ               /* BTALLOC_EXACT, BTALLOC_LT, or BTALLOC_ANY */
){
  Mem_ಪುಟ *pPage1;
  ಪೂರ್ಣಾಂಕ rc;
  u32 n;     /* Number of pages on the freelist */
  u32 k;     /* Number of leaves on the trunk of the freelist */
  Mem_ಪುಟ *pTrunk = 0;
  Mem_ಪುಟ *pPrevTrunk = 0;
  Pgno mx_ಪುಟ;     /* Total size of the database file */

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( e_ಬಹುಲಕ==BTALLOC_ANY || (nearby>0 && IfNotOmitAV(pBt->autoVacuum)) );
  pPage1 = pBt->pPage1;
  mx_ಪುಟ = btreePagecount(pBt);
  /* EVIDENCE-OF: R-21003-45125 The 4-byte big-endian integer at offset 36
  ** stores the total number of pages on the freelist. */
  n = get4byte(&pPage1->a_ದತ್ತಾಂಶ[36]);
  testcase( n==mx_ಪುಟ-1 );
  ಆದರೆ( n>=mx_ಪುಟ ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  }
  ಆದರೆ( n>0 ){
    /* There are pages on the freelist.  Reuse one of those pages. */
    Pgno iTrunk;
    u8 ಹುಡುಕು_List = 0; /* If the free-list must be searched for 'nearby' */
    u32 n_ಹುಡುಕು = 0;   /* Count of the number of search attempts */
   
    /* If eMode==BTALLOC_EXACT and a query of the pointer-map
    ** shows that the page 'nearby' is somewhere on the free-list, then
    ** the entire-list will be searched for that page.
    */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
    ಆದರೆ( e_ಬಹುಲಕ==BTALLOC_EXACT ){
      ಆದರೆ( nearby<=mx_ಪುಟ ){
        u8 eType;
        ಪ್ರತಿಜ್ಞಾ( nearby>0 );
        ಪ್ರತಿಜ್ಞಾ( pBt->autoVacuum );
        rc = ptrmapGet(pBt, nearby, &eType, 0);
        ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
        ಆದರೆ( eType==PTRMAP_FREEPAGE ){
          ಹುಡುಕು_List = 1;
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( e_ಬಹುಲಕ==BTALLOC_LE ){
      ಹುಡುಕು_List = 1;
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* Decrement the free-list count by 1. Set iTrunk to the index of the
    ** first free-list trunk page. iPrevTrunk is initially 1.
    */
    rc = sqlite3PagerWrite(pPage1->p_Db_ಪುಟ);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    put4byte(&pPage1->a_ದತ್ತಾಂಶ[36], n-1);

    /* The code within this loop is run only once if the 'searchList' variable
    ** is not true. Otherwise, it runs once for each trunk-page on the
    ** free-list until the page 'nearby' is located (eMode==BTALLOC_EXACT)
    ** or until a page less than 'nearby' is located (eMode==BTALLOC_LT)
    */
    ಮಾಡು {
      pPrevTrunk = pTrunk;
      ಆದರೆ( pPrevTrunk ){
        /* EVIDENCE-OF: R-01506-11053 The first integer on a freelist trunk page
        ** is the page number of the next freelist trunk page in the list or
        ** zero if this is the last freelist trunk page. */
        iTrunk = get4byte(&pPrevTrunk->a_ದತ್ತಾಂಶ[0]);
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* EVIDENCE-OF: R-59841-13798 The 4-byte big-endian integer at offset 32
        ** stores the page number of the first page of the freelist, or zero if
        ** the freelist is empty. */
        iTrunk = get4byte(&pPage1->a_ದತ್ತಾಂಶ[32]);
      }
      testcase( iTrunk==mx_ಪುಟ );
      ಆದರೆ( iTrunk>mx_ಪುಟ || n_ಹುಡುಕು++ > n ){
        rc = SQLITE_CORRUPT_PGNO(pPrevTrunk ? pPrevTrunk->pgno : 1);
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = btree_Get_ಬಳಸದ_ಪುಟ(pBt, iTrunk, &pTrunk, 0);
      }
      ಆದರೆ( rc ){
        pTrunk = 0;
        ಹೋಗು ಕೊನೆ_allocate_ಪುಟ;
      }
      ಪ್ರತಿಜ್ಞಾ( pTrunk!=0 );
      ಪ್ರತಿಜ್ಞಾ( pTrunk->a_ದತ್ತಾಂಶ!=0 );
      /* EVIDENCE-OF: R-13523-04394 The second integer on a freelist trunk page
      ** is the number of leaf page pointers to follow. */
      k = get4byte(&pTrunk->a_ದತ್ತಾಂಶ[4]);
      ಆದರೆ( k==0 && !ಹುಡುಕು_List ){
        /* The trunk has no leaves and the list is not being searched.
        ** So extract the trunk page itself and use it as the newly
        ** allocated page */
        ಪ್ರತಿಜ್ಞಾ( pPrevTrunk==0 );
        rc = sqlite3PagerWrite(pTrunk->p_Db_ಪುಟ);
        ಆದರೆ( rc ){
          ಹೋಗು ಕೊನೆ_allocate_ಪುಟ;
        }
        *pPgno = iTrunk;
        ಸ್ಮರಣೆ_ನಕಲು(&pPage1->a_ದತ್ತಾಂಶ[32], &pTrunk->a_ದತ್ತಾಂಶ[0], 4);
        *pp_ಪುಟ = pTrunk;
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( k>(u32)(pBt->usable_ಗಾತ್ರ/4 - 2) ){
        /* Value of k is out of range.  Database corruption */
        rc = SQLITE_CORRUPT_PGNO(iTrunk);
        ಹೋಗು ಕೊನೆ_allocate_ಪುಟ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಹುಡುಕು_List
            && (nearby==iTrunk || (iTrunk<nearby && e_ಬಹುಲಕ==BTALLOC_LE))
      ){
        /* The list is being searched and this trunk page is the page
        ** to allocate, regardless of whether it has leaves.
        */
        *pPgno = iTrunk;
        *pp_ಪುಟ = pTrunk;
        ಹುಡುಕು_List = 0;
        rc = sqlite3PagerWrite(pTrunk->p_Db_ಪುಟ);
        ಆದರೆ( rc ){
          ಹೋಗು ಕೊನೆ_allocate_ಪುಟ;
        }
        ಆದರೆ( k==0 ){
          ಆದರೆ( !pPrevTrunk ){
            ಸ್ಮರಣೆ_ನಕಲು(&pPage1->a_ದತ್ತಾಂಶ[32], &pTrunk->a_ದತ್ತಾಂಶ[0], 4);
          }ಇಲ್ಲದಿದ್ದರೆ{
            rc = sqlite3PagerWrite(pPrevTrunk->p_Db_ಪುಟ);
            ಆದರೆ( rc!=SQLITE_OK ){
              ಹೋಗು ಕೊನೆ_allocate_ಪುಟ;
            }
            ಸ್ಮರಣೆ_ನಕಲು(&pPrevTrunk->a_ದತ್ತಾಂಶ[0], &pTrunk->a_ದತ್ತಾಂಶ[0], 4);
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          /* The trunk page is required by the caller but it contains
          ** pointers to free-list leaves. The first leaf becomes a trunk
          ** page in this case.
          */
          Mem_ಪುಟ *pNewTrunk;
          Pgno iNewTrunk = get4byte(&pTrunk->a_ದತ್ತಾಂಶ[8]);
          ಆದರೆ( iNewTrunk>mx_ಪುಟ ){
            rc = SQLITE_CORRUPT_PGNO(iTrunk);
            ಹೋಗು ಕೊನೆ_allocate_ಪುಟ;
          }
          testcase( iNewTrunk==mx_ಪುಟ );
          rc = btree_Get_ಬಳಸದ_ಪುಟ(pBt, iNewTrunk, &pNewTrunk, 0);
          ಆದರೆ( rc!=SQLITE_OK ){
            ಹೋಗು ಕೊನೆ_allocate_ಪುಟ;
          }
          rc = sqlite3PagerWrite(pNewTrunk->p_Db_ಪುಟ);
          ಆದರೆ( rc!=SQLITE_OK ){
            release_ಪುಟ(pNewTrunk);
            ಹೋಗು ಕೊನೆ_allocate_ಪುಟ;
          }
          ಸ್ಮರಣೆ_ನಕಲು(&pNewTrunk->a_ದತ್ತಾಂಶ[0], &pTrunk->a_ದತ್ತಾಂಶ[0], 4);
          put4byte(&pNewTrunk->a_ದತ್ತಾಂಶ[4], k-1);
          ಸ್ಮರಣೆ_ನಕಲು(&pNewTrunk->a_ದತ್ತಾಂಶ[8], &pTrunk->a_ದತ್ತಾಂಶ[12], (k-1)*4);
          release_ಪುಟ(pNewTrunk);
          ಆದರೆ( !pPrevTrunk ){
            ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(pPage1->p_Db_ಪುಟ) );
            put4byte(&pPage1->a_ದತ್ತಾಂಶ[32], iNewTrunk);
          }ಇಲ್ಲದಿದ್ದರೆ{
            rc = sqlite3PagerWrite(pPrevTrunk->p_Db_ಪುಟ);
            ಆದರೆ( rc ){
              ಹೋಗು ಕೊನೆ_allocate_ಪುಟ;
            }
            put4byte(&pPrevTrunk->a_ದತ್ತಾಂಶ[0], iNewTrunk);
          }
        }
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
#ಆದರೆ_ಅಂತ್ಯ
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( k>0 ){
        /* Extract a leaf from the trunk */
        u32 closest;
        Pgno i_ಪುಟ;
        ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a_ದತ್ತಾಂಶ = pTrunk->a_ದತ್ತಾಂಶ;
        ಆದರೆ( nearby>0 ){
          u32 i;
          closest = 0;
          ಆದರೆ( e_ಬಹುಲಕ==BTALLOC_LE ){
            ಗಾಗಿ(i=0; i<k; i++){
              i_ಪುಟ = get4byte(&a_ದತ್ತಾಂಶ[8+i*4]);
              ಆದರೆ( i_ಪುಟ<=nearby ){
                closest = i;
                ನಿಲ್ಲಿಸು;
              }
            }
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಪೂರ್ಣಾಂಕ dist;
            dist = sqlite3AbsInt32(get4byte(&a_ದತ್ತಾಂಶ[8]) - nearby);
            ಗಾಗಿ(i=1; i<k; i++){
              ಪೂರ್ಣಾಂಕ d2 = sqlite3AbsInt32(get4byte(&a_ದತ್ತಾಂಶ[8+i*4]) - nearby);
              ಆದರೆ( d2<dist ){
                closest = i;
                dist = d2;
              }
            }
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          closest = 0;
        }

        i_ಪುಟ = get4byte(&a_ದತ್ತಾಂಶ[8+closest*4]);
        testcase( i_ಪುಟ==mx_ಪುಟ );
        ಆದರೆ( i_ಪುಟ>mx_ಪುಟ || i_ಪುಟ<2 ){
          rc = SQLITE_CORRUPT_PGNO(iTrunk);
          ಹೋಗು ಕೊನೆ_allocate_ಪುಟ;
        }
        testcase( i_ಪುಟ==mx_ಪುಟ );
        ಆದರೆ( !ಹುಡುಕು_List
         || (i_ಪುಟ==nearby || (i_ಪುಟ<nearby && e_ಬಹುಲಕ==BTALLOC_LE))
        ){
          ಪೂರ್ಣಾಂಕ noContent;
          *pPgno = i_ಪುಟ;
          TRACE(("ALLOCATE: %u was leaf %u of %u on trunk %u"
                 ": %u more free pages\n",
                 *pPgno, closest+1, k, pTrunk->pgno, n-1));
          rc = sqlite3PagerWrite(pTrunk->p_Db_ಪುಟ);
          ಆದರೆ( rc ) ಹೋಗು ಕೊನೆ_allocate_ಪುಟ;
          ಆದರೆ( closest<k-1 ){
            ಸ್ಮರಣೆ_ನಕಲು(&a_ದತ್ತಾಂಶ[8+closest*4], &a_ದತ್ತಾಂಶ[4+k*4], 4);
          }
          put4byte(&a_ದತ್ತಾಂಶ[4], k-1);
          noContent = !btreeGetHasContent(pBt, *pPgno)? PAGER_GET_NOCONTENT : 0;
          rc = btree_Get_ಬಳಸದ_ಪುಟ(pBt, *pPgno, pp_ಪುಟ, noContent);
          ಆದರೆ( rc==SQLITE_OK ){
            rc = sqlite3PagerWrite((*pp_ಪುಟ)->p_Db_ಪುಟ);
            ಆದರೆ( rc!=SQLITE_OK ){
              release_ಪುಟ(*pp_ಪುಟ);
              *pp_ಪುಟ = 0;
            }
          }
          ಹುಡುಕು_List = 0;
        }
      }
      release_ಪುಟ(pPrevTrunk);
      pPrevTrunk = 0;
    }ಆಗ( ಹುಡುಕು_List );
  }ಇಲ್ಲದಿದ್ದರೆ{
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
    ಪೂರ್ಣಾಂಕ bNoContent = (0==IfNotOmitAV(pBt->bDoTruncate))? PAGER_GET_NOCONTENT:0;

    rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ಪುಟ);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    pBt->n_ಪುಟ++;
    ಆದರೆ( pBt->n_ಪುಟ==PENDING_BYTE_ಪುಟ(pBt) ) pBt->n_ಪುಟ++;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
    ಆದರೆ( pBt->autoVacuum && PTRMAP_ISPAGE(pBt, pBt->n_ಪುಟ) ){
      /* If *pPgno refers to a pointer-map page, allocate two new pages
      ** at the end of the file instead of one. The first allocated page
      ** becomes a new pointer-map page, the second is used by the caller.
      */
      Mem_ಪುಟ *pPg = 0;
      TRACE(("ALLOCATE: %u from end of file (pointer-map page)\n", pBt->n_ಪುಟ));
      ಪ್ರತಿಜ್ಞಾ( pBt->n_ಪುಟ!=PENDING_BYTE_ಪುಟ(pBt) );
      rc = btree_Get_ಬಳಸದ_ಪುಟ(pBt, pBt->n_ಪುಟ, &pPg, bNoContent);
      ಆದರೆ( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pPg->p_Db_ಪುಟ);
        release_ಪುಟ(pPg);
      }
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
      pBt->n_ಪುಟ++;
      ಆದರೆ( pBt->n_ಪುಟ==PENDING_BYTE_ಪುಟ(pBt) ){ pBt->n_ಪುಟ++; }
    }
#ಆದರೆ_ಅಂತ್ಯ
    put4byte(28 + (u8*)pBt->pPage1->a_ದತ್ತಾಂಶ, pBt->n_ಪುಟ);
    *pPgno = pBt->n_ಪುಟ;

    ಪ್ರತಿಜ್ಞಾ( *pPgno!=PENDING_BYTE_ಪುಟ(pBt) );
    rc = btree_Get_ಬಳಸದ_ಪುಟ(pBt, *pPgno, pp_ಪುಟ, bNoContent);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    rc = sqlite3PagerWrite((*pp_ಪುಟ)->p_Db_ಪುಟ);
    ಆದರೆ( rc!=SQLITE_OK ){
      release_ಪುಟ(*pp_ಪುಟ);
      *pp_ಪುಟ = 0;
    }
    TRACE(("ALLOCATE: %u from end of file\n", *pPgno));
  }

  ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB || *pPgno!=PENDING_BYTE_ಪುಟ(pBt) );

ಕೊನೆ_allocate_ಪುಟ:
  release_ಪುಟ(pTrunk);
  release_ಪುಟ(pPrevTrunk);
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || sqlite3_Pager_ಪುಟ_Refcount((*pp_ಪುಟ)->p_Db_ಪುಟ)<=1 );
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || (*pp_ಪುಟ)->isInit==0 );
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ freePage2(BtShared *pBt, Mem_ಪುಟ *p_Mem_ಪುಟ, Pgno i_ಪುಟ){
  Mem_ಪುಟ *pTrunk = 0;                /* Free-list trunk page */
  Pgno iTrunk = 0;                    /* Page number of free-list trunk page */
  Mem_ಪುಟ *pPage1 = pBt->pPage1;      /* Local reference to page 1 */
  Mem_ಪುಟ *p_ಪುಟ;                     /* Page being freed. May be NULL. */
  ಪೂರ್ಣಾಂಕ rc;                             /* Return Code */
  u32 nFree;                          /* Initial number of pages on free-list */

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB || i_ಪುಟ>1 );
  ಪ್ರತಿಜ್ಞಾ( !p_Mem_ಪುಟ || p_Mem_ಪುಟ->pgno==i_ಪುಟ );

  ಆದರೆ( i_ಪುಟ<2 || i_ಪುಟ>pBt->n_ಪುಟ ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  }
  ಆದರೆ( p_Mem_ಪುಟ ){
    p_ಪುಟ = p_Mem_ಪುಟ;
    sqlite3PagerRef(p_ಪುಟ->p_Db_ಪುಟ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಪುಟ = btree_ಪುಟ_Lookup(pBt, i_ಪುಟ);
  }

  /* Increment the free page count on pPage1 */
  rc = sqlite3PagerWrite(pPage1->p_Db_ಪುಟ);
  ಆದರೆ( rc ) ಹೋಗು freepage_ಹೊರಹೋಗು;
  nFree = get4byte(&pPage1->a_ದತ್ತಾಂಶ[36]);
  put4byte(&pPage1->a_ದತ್ತಾಂಶ[36], nFree+1);

  ಆದರೆ( pBt->btsFlags & BTS_SECURE_DELETE ){
    /* If the secure_delete option is enabled, then
    ** always fully overwrite deleted information with zeros.
    */
    ಆದರೆ( (!p_ಪುಟ && ((rc = btree_Get_ಪುಟ(pBt, i_ಪುಟ, &p_ಪುಟ, 0))!=0) )
     ||            ((rc = sqlite3PagerWrite(p_ಪುಟ->p_Db_ಪುಟ))!=0)
    ){
      ಹೋಗು freepage_ಹೊರಹೋಗು;
    }
    memset(p_ಪುಟ->a_ದತ್ತಾಂಶ, 0, p_ಪುಟ->pBt->ಪುಟ_ಗಾತ್ರ);
  }

  /* If the database supports auto-vacuum, write an entry in the pointer-map
  ** to indicate that the page is free.
  */
  ಆದರೆ( ISAUTOVACUUM(pBt) ){
    ptrmapPut(pBt, i_ಪುಟ, PTRMAP_FREEPAGE, 0, &rc);
    ಆದರೆ( rc ) ಹೋಗು freepage_ಹೊರಹೋಗು;
  }

  /* Now manipulate the actual database free-list structure. There are two
  ** possibilities. If the free-list is currently empty, or if the first
  ** trunk page in the free-list is full, then this page will become a
  ** new free-list trunk page. Otherwise, it will become a leaf of the
  ** first trunk page in the current free-list. This block tests if it
  ** is possible to add the page as a new free-list leaf.
  */
  ಆದರೆ( nFree!=0 ){
    u32 n_ಎಲೆ;                /* Initial number of leaf cells on trunk page */

    iTrunk = get4byte(&pPage1->a_ದತ್ತಾಂಶ[32]);
    ಆದರೆ( iTrunk>btreePagecount(pBt) ){
      rc = SQLITE_CORRUPT_BKPT;
      ಹೋಗು freepage_ಹೊರಹೋಗು;
    }
    rc = btree_Get_ಪುಟ(pBt, iTrunk, &pTrunk, 0);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹೋಗು freepage_ಹೊರಹೋಗು;
    }

    n_ಎಲೆ = get4byte(&pTrunk->a_ದತ್ತಾಂಶ[4]);
    ಪ್ರತಿಜ್ಞಾ( pBt->usable_ಗಾತ್ರ>32 );
    ಆದರೆ( n_ಎಲೆ > (u32)pBt->usable_ಗಾತ್ರ/4 - 2 ){
      rc = SQLITE_CORRUPT_BKPT;
      ಹೋಗು freepage_ಹೊರಹೋಗು;
    }
    ಆದರೆ( n_ಎಲೆ < (u32)pBt->usable_ಗಾತ್ರ/4 - 8 ){
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
      rc = sqlite3PagerWrite(pTrunk->p_Db_ಪುಟ);
      ಆದರೆ( rc==SQLITE_OK ){
        put4byte(&pTrunk->a_ದತ್ತಾಂಶ[4], n_ಎಲೆ+1);
        put4byte(&pTrunk->a_ದತ್ತಾಂಶ[8+n_ಎಲೆ*4], i_ಪುಟ);
        ಆದರೆ( p_ಪುಟ && (pBt->btsFlags & BTS_SECURE_DELETE)==0 ){
          sqlite3PagerDontWrite(p_ಪುಟ->p_Db_ಪುಟ);
        }
        rc = btree_ಗಣ_Has_Content(pBt, i_ಪುಟ);
      }
      TRACE(("FREE-PAGE: %u leaf on trunk page %u\n",p_ಪುಟ->pgno,pTrunk->pgno));
      ಹೋಗು freepage_ಹೊರಹೋಗು;
    }
  }

  /* If control flows to this point, then it was not possible to add the
  ** the page being freed as a leaf page of the first trunk in the free-list.
  ** Possibly because the free-list is empty, or possibly because the
  ** first trunk in the free-list is full. Either way, the page being freed
  ** will become the new first trunk page in the free-list.
  */
  ಆದರೆ( p_ಪುಟ==0 && SQLITE_OK!=(rc = btree_Get_ಪುಟ(pBt, i_ಪುಟ, &p_ಪುಟ, 0)) ){
    ಹೋಗು freepage_ಹೊರಹೋಗು;
  }
  rc = sqlite3PagerWrite(p_ಪುಟ->p_Db_ಪುಟ);
  ಆದರೆ( rc!=SQLITE_OK ){
    ಹೋಗು freepage_ಹೊರಹೋಗು;
  }
  put4byte(p_ಪುಟ->a_ದತ್ತಾಂಶ, iTrunk);
  put4byte(&p_ಪುಟ->a_ದತ್ತಾಂಶ[4], 0);
  put4byte(&pPage1->a_ದತ್ತಾಂಶ[32], i_ಪುಟ);
  TRACE(("FREE-PAGE: %u new trunk page replacing %u\n", p_ಪುಟ->pgno, iTrunk));

freepage_ಹೊರಹೋಗು:
  ಆದರೆ( p_ಪುಟ ){
    p_ಪುಟ->isInit = 0;
  }
  release_ಪುಟ(p_ಪುಟ);
  release_ಪುಟ(pTrunk);
  ಹಿಂತಿರುಗಿ rc;
}
ಸ್ಥಿರ ಶೂನ್ಯ free_ಪುಟ(Mem_ಪುಟ *p_ಪುಟ, ಪೂರ್ಣಾಂಕ *pRC){
  ಆದರೆ( (*pRC)==SQLITE_OK ){
    *pRC = freePage2(p_ಪುಟ->pBt, p_ಪುಟ, p_ಪುಟ->pgno);
  }
}

/*
** Free the overflow pages associated with the given Cell.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ clear_Cell_ಅತಿಪ್ರವಾಹ(
  Mem_ಪುಟ *p_ಪುಟ,          /* The page that contains the Cell */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pCell,    /* First byte of the Cell */
  CellInfo *pInfo          /* Size information about the cell */
){
  BtShared *pBt;
  Pgno ovflPgno;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ nOvfl;
  u32 ovfl_ಪುಟ_ಗಾತ್ರ;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( pInfo->nLocal!=pInfo->nPayload );
  testcase( pCell + pInfo->n_ಗಾತ್ರ == p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ );
  testcase( pCell + (pInfo->n_ಗಾತ್ರ-1) == p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ );
  ಆದರೆ( pCell + pInfo->n_ಗಾತ್ರ > p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ ){
    /* Cell extends past end of page */
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
  }
  ovflPgno = get4byte(pCell + pInfo->n_ಗಾತ್ರ - 4);
  pBt = p_ಪುಟ->pBt;
  ಪ್ರತಿಜ್ಞಾ( pBt->usable_ಗಾತ್ರ > 4 );
  ovfl_ಪುಟ_ಗಾತ್ರ = pBt->usable_ಗಾತ್ರ - 4;
  nOvfl = (pInfo->nPayload - pInfo->nLocal + ovfl_ಪುಟ_ಗಾತ್ರ - 1)/ovfl_ಪುಟ_ಗಾತ್ರ;
  ಪ್ರತಿಜ್ಞಾ( nOvfl>0 ||
    (CORRUPT_DB && (pInfo->nPayload + ovfl_ಪುಟ_ಗಾತ್ರ)<ovfl_ಪುಟ_ಗಾತ್ರ)
  );
  ಆಗ( nOvfl-- ){
    Pgno i_ಮುಂದಿನ = 0;
    Mem_ಪುಟ *pOvfl = 0;
    ಆದರೆ( ovflPgno<2 || ovflPgno>btreePagecount(pBt) ){
      /* 0 is not a legal page number and page 1 cannot be an
      ** overflow page. Therefore if ovflPgno<2 or past the end of the
      ** file the database must be corrupt. */
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
    }
    ಆದರೆ( nOvfl ){
      rc = get_ಅತಿಪ್ರವಾಹ_ಪುಟ(pBt, ovflPgno, &pOvfl, &i_ಮುಂದಿನ);
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    }

    ಆದರೆ( ( pOvfl || ((pOvfl = btree_ಪುಟ_Lookup(pBt, ovflPgno))!=0) )
     && sqlite3_Pager_ಪುಟ_Refcount(pOvfl->p_Db_ಪುಟ)!=1
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
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = freePage2(pBt, pOvfl, ovflPgno);
    }

    ಆದರೆ( pOvfl ){
      sqlite3PagerUnref(pOvfl->p_Db_ಪುಟ);
    }
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    ovflPgno = i_ಮುಂದಿನ;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Call xParseCell to compute the size of a cell.  If the cell contains
** overflow, then invoke cellClearOverflow to clear out that overflow.
** Store the result code (SQLITE_OK or some error code) in rc.
**
** Implemented as macro to force inlining for performance.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_CLEAR_CELL(rc, p_ಪುಟ, pCell, sInfo)   \
  p_ಪುಟ->xParseCell(p_ಪುಟ, pCell, &sInfo);          \
  ಆದರೆ( sInfo.nLocal!=sInfo.nPayload ){               \
    rc = clear_Cell_ಅತಿಪ್ರವಾಹ(p_ಪುಟ, pCell, &sInfo);   \
  }ಇಲ್ಲದಿದ್ದರೆ{                                            \
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fill_ಒಳ_Cell(
  Mem_ಪುಟ *p_ಪುಟ,                /* The page that contains the cell */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pCell,          /* Complete text of the cell */
  ಸ್ಥಿರಾಂಕ BtreePayload *pX,        /* Payload with which to construct the cell */
  ಪೂರ್ಣಾಂಕ *pn_ಗಾತ್ರ                    /* Write cell size here */
){
  ಪೂರ್ಣಾಂಕ nPayload;
  ಸ್ಥಿರಾಂಕ u8 *pSrc;
  ಪೂರ್ಣಾಂಕ nSrc, n, rc, mn;
  ಪೂರ್ಣಾಂಕ space_ಎಡ;
  Mem_ಪುಟ *p_ಗಮ್ಯ_Release;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p_ಪೂರ್ವ;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pPayload;
  BtShared *pBt;
  Pgno pgnoOvfl;
  ಪೂರ್ಣಾಂಕ n_ಶೀರ್ಷಿಕೆ;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );

  /* pPage is not necessarily writeable since pCell might be auxiliary
  ** buffer space that is separate from the pPage buffer area */
  ಪ್ರತಿಜ್ಞಾ( pCell<p_ಪುಟ->a_ದತ್ತಾಂಶ || pCell>=&p_ಪುಟ->a_ದತ್ತಾಂಶ[p_ಪುಟ->pBt->ಪುಟ_ಗಾತ್ರ]
            || sqlite3PagerIswriteable(p_ಪುಟ->p_Db_ಪುಟ) );

  /* Fill in the header. */
  n_ಶೀರ್ಷಿಕೆ = p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ;
  ಆದರೆ( p_ಪುಟ->int_ಕೀಲಿ ){
    nPayload = pX->n_ದತ್ತಾಂಶ + pX->nZero;
    pSrc = pX->p_ದತ್ತಾಂಶ;
    nSrc = pX->n_ದತ್ತಾಂಶ;
    ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->int_ಕೀಲಿ_ಎಲೆ ); /* fillInCell() only called for leaves */
    n_ಶೀರ್ಷಿಕೆ += putVarint32(&pCell[n_ಶೀರ್ಷಿಕೆ], nPayload);
    n_ಶೀರ್ಷಿಕೆ += sqlite3PutVarint(&pCell[n_ಶೀರ್ಷಿಕೆ], *(u64*)&pX->n_ಕೀಲಿ);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pX->n_ಕೀಲಿ<=0x7fffffff && pX->p_ಕೀಲಿ!=0 );
    nSrc = nPayload = (ಪೂರ್ಣಾಂಕ)pX->n_ಕೀಲಿ;
    pSrc = pX->p_ಕೀಲಿ;
    n_ಶೀರ್ಷಿಕೆ += putVarint32(&pCell[n_ಶೀರ್ಷಿಕೆ], nPayload);
  }
 
  /* Fill in the payload */
  pPayload = &pCell[n_ಶೀರ್ಷಿಕೆ];
  ಆದರೆ( nPayload<=p_ಪುಟ->maxLocal ){
    /* This is the common case where everything fits on the btree page
    ** and no overflow pages are required. */
    n = n_ಶೀರ್ಷಿಕೆ + nPayload;
    testcase( n==3 );
    testcase( n==4 );
    ಆದರೆ( n<4 ){
      n = 4;
      pPayload[nPayload] = 0;
    }
    *pn_ಗಾತ್ರ = n;
    ಪ್ರತಿಜ್ಞಾ( nSrc<=nPayload );
    testcase( nSrc<nPayload );
    ಸ್ಮರಣೆ_ನಕಲು(pPayload, pSrc, nSrc);
    memset(pPayload+nSrc, 0, nPayload-nSrc);
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }

  /* If we reach this point, it means that some of the content will need
  ** to spill onto overflow pages.
  */
  mn = p_ಪುಟ->minLocal;
  n = mn + (nPayload - mn) % (p_ಪುಟ->pBt->usable_ಗಾತ್ರ - 4);
  testcase( n==p_ಪುಟ->maxLocal );
  testcase( n==p_ಪುಟ->maxLocal+1 );
  ಆದರೆ( n > p_ಪುಟ->maxLocal ) n = mn;
  space_ಎಡ = n;
  *pn_ಗಾತ್ರ = n + n_ಶೀರ್ಷಿಕೆ + 4;
  p_ಪೂರ್ವ = &pCell[n_ಶೀರ್ಷಿಕೆ+n];
  p_ಗಮ್ಯ_Release = 0;
  pgnoOvfl = 0;
  pBt = p_ಪುಟ->pBt;

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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  {
    CellInfo info;
    p_ಪುಟ->xParseCell(p_ಪುಟ, pCell, &info);
    ಪ್ರತಿಜ್ಞಾ( n_ಶೀರ್ಷಿಕೆ==(ಪೂರ್ಣಾಂಕ)(info.pPayload - pCell) );
    ಪ್ರತಿಜ್ಞಾ( info.n_ಕೀಲಿ==pX->n_ಕೀಲಿ );
    ಪ್ರತಿಜ್ಞಾ( *pn_ಗಾತ್ರ == info.n_ಗಾತ್ರ );
    ಪ್ರತಿಜ್ಞಾ( space_ಎಡ == info.nLocal );
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Write the payload into the local Cell and any extra into overflow pages */
  ಆಗ( 1 ){
    n = nPayload;
    ಆದರೆ( n>space_ಎಡ ) n = space_ಎಡ;

    /* If pToRelease is not zero than pPayload points into the data area
    ** of pToRelease.  Make sure pToRelease is still writeable. */
    ಪ್ರತಿಜ್ಞಾ( p_ಗಮ್ಯ_Release==0 || sqlite3PagerIswriteable(p_ಗಮ್ಯ_Release->p_Db_ಪುಟ) );

    /* If pPayload is part of the data area of pPage, then make sure pPage
    ** is still writeable */
    ಪ್ರತಿಜ್ಞಾ( pPayload<p_ಪುಟ->a_ದತ್ತಾಂಶ || pPayload>=&p_ಪುಟ->a_ದತ್ತಾಂಶ[pBt->ಪುಟ_ಗಾತ್ರ]
            || sqlite3PagerIswriteable(p_ಪುಟ->p_Db_ಪುಟ) );

    ಆದರೆ( nSrc>=n ){
      ಸ್ಮರಣೆ_ನಕಲು(pPayload, pSrc, n);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nSrc>0 ){
      n = nSrc;
      ಸ್ಮರಣೆ_ನಕಲು(pPayload, pSrc, n);
    }ಇಲ್ಲದಿದ್ದರೆ{
      memset(pPayload, 0, n);
    }
    nPayload -= n;
    ಆದರೆ( nPayload<=0 ) ನಿಲ್ಲಿಸು;
    pPayload += n;
    pSrc += n;
    nSrc -= n;
    space_ಎಡ -= n;
    ಆದರೆ( space_ಎಡ==0 ){
      Mem_ಪುಟ *pOvfl = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
      Pgno pgnoPtrmap = pgnoOvfl; /* Overflow page pointer-map entry page */
      ಆದರೆ( pBt->autoVacuum ){
        ಮಾಡು{
          pgnoOvfl++;
        } ಆಗ(
          PTRMAP_ISPAGE(pBt, pgnoOvfl) || pgnoOvfl==PENDING_BYTE_ಪುಟ(pBt)
        );
      }
#ಆದರೆ_ಅಂತ್ಯ
      rc = allocate_Btree_ಪುಟ(pBt, &pOvfl, &pgnoOvfl, pgnoOvfl, 0);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
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
      ಆದರೆ( pBt->autoVacuum && rc==SQLITE_OK ){
        u8 eType = (pgnoPtrmap?PTRMAP_OVERFLOW2:PTRMAP_OVERFLOW1);
        ptrmapPut(pBt, pgnoOvfl, eType, pgnoPtrmap, &rc);
        ಆದರೆ( rc ){
          release_ಪುಟ(pOvfl);
        }
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಆದರೆ( rc ){
        release_ಪುಟ(p_ಗಮ್ಯ_Release);
        ಹಿಂತಿರುಗಿ rc;
      }

      /* If pToRelease is not zero than pPrior points into the data area
      ** of pToRelease.  Make sure pToRelease is still writeable. */
      ಪ್ರತಿಜ್ಞಾ( p_ಗಮ್ಯ_Release==0 || sqlite3PagerIswriteable(p_ಗಮ್ಯ_Release->p_Db_ಪುಟ) );

      /* If pPrior is part of the data area of pPage, then make sure pPage
      ** is still writeable */
      ಪ್ರತಿಜ್ಞಾ( p_ಪೂರ್ವ<p_ಪುಟ->a_ದತ್ತಾಂಶ || p_ಪೂರ್ವ>=&p_ಪುಟ->a_ದತ್ತಾಂಶ[pBt->ಪುಟ_ಗಾತ್ರ]
            || sqlite3PagerIswriteable(p_ಪುಟ->p_Db_ಪುಟ) );

      put4byte(p_ಪೂರ್ವ, pgnoOvfl);
      release_ಪುಟ(p_ಗಮ್ಯ_Release);
      p_ಗಮ್ಯ_Release = pOvfl;
      p_ಪೂರ್ವ = pOvfl->a_ದತ್ತಾಂಶ;
      put4byte(p_ಪೂರ್ವ, 0);
      pPayload = &pOvfl->a_ದತ್ತಾಂಶ[4];
      space_ಎಡ = pBt->usable_ಗಾತ್ರ - 4;
    }
  }
  release_ಪುಟ(p_ಗಮ್ಯ_Release);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Remove the i-th cell from pPage.  This routine effects pPage only.
** The cell content is not freed or deallocated.  It is assumed that
** the cell content has been copied someplace else.  This routine just
** removes the reference to the cell from pPage.
**
** "sz" must be the number of bytes in the cell.
*/
ಸ್ಥಿರ ಶೂನ್ಯ dropCell(Mem_ಪುಟ *p_ಪುಟ, ಪೂರ್ಣಾಂಕ idx, ಪೂರ್ಣಾಂಕ sz, ಪೂರ್ಣಾಂಕ *pRC){
  u32 pc;         /* Offset to cell content of cell being deleted */
  u8 *ದತ್ತಾಂಶ;       /* pPage->aData */
  u8 *ptr;        /* Used to move bytes around within data[] */
  ಪೂರ್ಣಾಂಕ rc;         /* The return code */
  ಪೂರ್ಣಾಂಕ hdr;        /* Beginning of the header.  0 most pages.  100 page 1 */

  ಆದರೆ( *pRC ) ಹಿಂತಿರುಗಿ;
  ಪ್ರತಿಜ್ಞಾ( idx>=0 );
  ಪ್ರತಿಜ್ಞಾ( idx<p_ಪುಟ->nCell );
  ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB || sz==cell_ಗಾತ್ರ(p_ಪುಟ, idx) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಪುಟ->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nFree>=0 );
  ದತ್ತಾಂಶ = p_ಪುಟ->a_ದತ್ತಾಂಶ;
  ptr = &p_ಪುಟ->aCellIdx[2*idx];
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pBt->usable_ಗಾತ್ರ > (u32)(ptr-ದತ್ತಾಂಶ) );
  pc = get2byte(ptr);
  hdr = p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ;
  testcase( pc==(u32)get2byte(&ದತ್ತಾಂಶ[hdr+5]) );
  testcase( pc+sz==p_ಪುಟ->pBt->usable_ಗಾತ್ರ );
  ಆದರೆ( pc+sz > p_ಪುಟ->pBt->usable_ಗಾತ್ರ ){
    *pRC = SQLITE_CORRUPT_BKPT;
    ಹಿಂತಿರುಗಿ;
  }
  rc = freeSpace(p_ಪುಟ, pc, sz);
  ಆದರೆ( rc ){
    *pRC = rc;
    ಹಿಂತಿರುಗಿ;
  }
  p_ಪುಟ->nCell--;
  ಆದರೆ( p_ಪುಟ->nCell==0 ){
    memset(&ದತ್ತಾಂಶ[hdr+1], 0, 4);
    ದತ್ತಾಂಶ[hdr+7] = 0;
    put2byte(&ದತ್ತಾಂಶ[hdr+5], p_ಪುಟ->pBt->usable_ಗಾತ್ರ);
    p_ಪುಟ->nFree = p_ಪುಟ->pBt->usable_ಗಾತ್ರ - p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ
                       - p_ಪುಟ->ಮಗು_Ptr_ಗಾತ್ರ - 8;
  }ಇಲ್ಲದಿದ್ದರೆ{
    memmove(ptr, ptr+2, 2*(p_ಪುಟ->nCell - idx));
    put2byte(&ದತ್ತಾಂಶ[hdr+3], p_ಪುಟ->nCell);
    p_ಪುಟ->nFree += 2;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸೇರಿಸು_Cell(
  Mem_ಪುಟ *p_ಪುಟ,   /* Page into which we are copying */
  ಪೂರ್ಣಾಂಕ i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  ಪೂರ್ಣಾಂಕ sz,           /* Bytes of content in pCell */
  u8 *p_ತಾತ್ಕಾಲಿಕ,        /* Temp storage space for pCell, if needed */
  Pgno i_ಮಗು       /* If non-zero, replace first 4 bytes with this value */
){
  ಪೂರ್ಣಾಂಕ idx = 0;      /* Where to write new cell content in data[] */
  ಪೂರ್ಣಾಂಕ j;            /* Loop counter */
  u8 *ದತ್ತಾಂಶ;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  ಪ್ರತಿಜ್ಞಾ( i>=0 && i<=p_ಪುಟ->nCell+p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ );
  ಪ್ರತಿಜ್ಞಾ( MX_CELL(p_ಪುಟ->pBt)<=10921 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nCell<=MX_CELL(p_ಪುಟ->pBt) || CORRUPT_DB );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ<=Array_ಗಾತ್ರ(p_ಪುಟ->apOvfl) );
  ಪ್ರತಿಜ್ಞಾ( Array_ಗಾತ್ರ(p_ಪುಟ->apOvfl)==Array_ಗಾತ್ರ(p_ಪುಟ->aiOvfl) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( sz==p_ಪುಟ->x_Cell_ಗಾತ್ರ(p_ಪುಟ, pCell) || CORRUPT_DB );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nFree>=0 );
  ಪ್ರತಿಜ್ಞಾ( i_ಮಗು>0 );
  ಆದರೆ( p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ || sz+2>p_ಪುಟ->nFree ){
    ಆದರೆ( p_ತಾತ್ಕಾಲಿಕ ){
      ಸ್ಮರಣೆ_ನಕಲು(p_ತಾತ್ಕಾಲಿಕ, pCell, sz);
      pCell = p_ತಾತ್ಕಾಲಿಕ;
    }
    put4byte(pCell, i_ಮಗು);
    j = p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    ಪ್ರತಿಜ್ಞಾ( j < Array_ಗಾತ್ರ(p_ಪುಟ->apOvfl)-1 );
    p_ಪುಟ->apOvfl[j] = pCell;
    p_ಪುಟ->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    ಪ್ರತಿಜ್ಞಾ( j==0 || p_ಪುಟ->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    ಪ್ರತಿಜ್ಞಾ( j==0 || i==p_ಪುಟ->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ rc = sqlite3PagerWrite(p_ಪುಟ->p_Db_ಪುಟ);
    ಆದರೆ( NEVER(rc!=SQLITE_OK) ){
      ಹಿಂತಿರುಗಿ rc;
    }
    ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಪುಟ->p_Db_ಪುಟ) );
    ದತ್ತಾಂಶ = p_ಪುಟ->a_ದತ್ತಾಂಶ;
    ಪ್ರತಿಜ್ಞಾ( &ದತ್ತಾಂಶ[p_ಪುಟ->cell_ಸ್ಥಾನಾಂತರ]==p_ಪುಟ->aCellIdx );
    rc = allocateSpace(p_ಪುಟ, sz, &idx);
    ಆದರೆ( rc ){ ಹಿಂತಿರುಗಿ rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    ಪ್ರತಿಜ್ಞಾ( idx >= 0 );
    ಪ್ರತಿಜ್ಞಾ( idx >= p_ಪುಟ->cell_ಸ್ಥಾನಾಂತರ+2*p_ಪುಟ->nCell+2 || CORRUPT_DB );
    ಪ್ರತಿಜ್ಞಾ( idx+sz <= (ಪೂರ್ಣಾಂಕ)p_ಪುಟ->pBt->usable_ಗಾತ್ರ );
    p_ಪುಟ->nFree -= (u16)(2 + sz);
    /* In a corrupt database where an entry in the cell index section of
    ** a btree page has a value of 3 or less, the pCell value might point
    ** as many as 4 bytes in front of the start of the aData buffer for
    ** the source page.  Make sure this does not cause problems by not
    ** reading the first 4 bytes */
    ಸ್ಮರಣೆ_ನಕಲು(&ದತ್ತಾಂಶ[idx+4], pCell+4, sz-4);
    put4byte(&ದತ್ತಾಂಶ[idx], i_ಮಗು);
    pIns = p_ಪುಟ->aCellIdx + i*2;
    ಆದರೆ( i<p_ಪುಟ->nCell ) memmove(pIns+2, pIns, 2*(p_ಪುಟ->nCell - i));
    put2byte(pIns, idx);
    p_ಪುಟ->nCell++;
    /* increment the cell count */
    ಆದರೆ( (++ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+4])==0 ) ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+3]++;
    ಪ್ರತಿಜ್ಞಾ( get2byte(&ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+3])==p_ಪುಟ->nCell || CORRUPT_DB );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
    ಆದರೆ( p_ಪುಟ->pBt->autoVacuum ){
      ಪೂರ್ಣಾಂಕ rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(p_ಪುಟ, p_ಪುಟ, pCell, &rc2);
      ಆದರೆ( rc2 ) ಹಿಂತಿರುಗಿ rc2;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸೇರಿಸು_Cell_Fast(
  Mem_ಪುಟ *p_ಪುಟ,   /* Page into which we are copying */
  ಪೂರ್ಣಾಂಕ i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  ಪೂರ್ಣಾಂಕ sz            /* Bytes of content in pCell */
){
  ಪೂರ್ಣಾಂಕ idx = 0;      /* Where to write new cell content in data[] */
  ಪೂರ್ಣಾಂಕ j;            /* Loop counter */
  u8 *ದತ್ತಾಂಶ;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  ಪ್ರತಿಜ್ಞಾ( i>=0 && i<=p_ಪುಟ->nCell+p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ );
  ಪ್ರತಿಜ್ಞಾ( MX_CELL(p_ಪುಟ->pBt)<=10921 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nCell<=MX_CELL(p_ಪುಟ->pBt) || CORRUPT_DB );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ<=Array_ಗಾತ್ರ(p_ಪುಟ->apOvfl) );
  ಪ್ರತಿಜ್ಞಾ( Array_ಗಾತ್ರ(p_ಪುಟ->apOvfl)==Array_ಗಾತ್ರ(p_ಪುಟ->aiOvfl) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( sz==p_ಪುಟ->x_Cell_ಗಾತ್ರ(p_ಪುಟ, pCell) || CORRUPT_DB );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nFree>=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ==0 );
  ಆದರೆ( sz+2>p_ಪುಟ->nFree ){
    j = p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    ಪ್ರತಿಜ್ಞಾ( j < Array_ಗಾತ್ರ(p_ಪುಟ->apOvfl)-1 );
    p_ಪುಟ->apOvfl[j] = pCell;
    p_ಪುಟ->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    ಪ್ರತಿಜ್ಞಾ( j==0 || p_ಪುಟ->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    ಪ್ರತಿಜ್ಞಾ( j==0 || i==p_ಪುಟ->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ rc = sqlite3PagerWrite(p_ಪುಟ->p_Db_ಪುಟ);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹಿಂತಿರುಗಿ rc;
    }
    ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಪುಟ->p_Db_ಪುಟ) );
    ದತ್ತಾಂಶ = p_ಪುಟ->a_ದತ್ತಾಂಶ;
    ಪ್ರತಿಜ್ಞಾ( &ದತ್ತಾಂಶ[p_ಪುಟ->cell_ಸ್ಥಾನಾಂತರ]==p_ಪುಟ->aCellIdx );
    rc = allocateSpace(p_ಪುಟ, sz, &idx);
    ಆದರೆ( rc ){ ಹಿಂತಿರುಗಿ rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    ಪ್ರತಿಜ್ಞಾ( idx >= 0 );
    ಪ್ರತಿಜ್ಞಾ( idx >= p_ಪುಟ->cell_ಸ್ಥಾನಾಂತರ+2*p_ಪುಟ->nCell+2 || CORRUPT_DB );
    ಪ್ರತಿಜ್ಞಾ( idx+sz <= (ಪೂರ್ಣಾಂಕ)p_ಪುಟ->pBt->usable_ಗಾತ್ರ );
    p_ಪುಟ->nFree -= (u16)(2 + sz);
    ಸ್ಮರಣೆ_ನಕಲು(&ದತ್ತಾಂಶ[idx], pCell, sz);
    pIns = p_ಪುಟ->aCellIdx + i*2;
    ಆದರೆ( i<p_ಪುಟ->nCell ) memmove(pIns+2, pIns, 2*(p_ಪುಟ->nCell - i));
    put2byte(pIns, idx);
    p_ಪುಟ->nCell++;
    /* increment the cell count */
    ಆದರೆ( (++ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+4])==0 ) ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+3]++;
    ಪ್ರತಿಜ್ಞಾ( get2byte(&ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+3])==p_ಪುಟ->nCell || CORRUPT_DB );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
    ಆದರೆ( p_ಪುಟ->pBt->autoVacuum ){
      ಪೂರ್ಣಾಂಕ rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(p_ಪುಟ, p_ಪುಟ, pCell, &rc2);
      ಆದರೆ( rc2 ) ಹಿಂತಿರುಗಿ rc2;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
#ವ್ಯಾಖ್ಯಾನಿಸು NN 1             /* Number of neighbors on either side of pPage */
#ವ್ಯಾಖ್ಯಾನಿಸು NB 3             /* (NN*2+1): Total pages involved in the balance */

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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ CellArray CellArray;
ರಚನೆ CellArray {
  ಪೂರ್ಣಾಂಕ nCell;              /* Number of cells in apCell[] */
  Mem_ಪುಟ *pRef;          /* Reference page */
  u8 **apCell;            /* All cells begin balanced */
  u16 *szCell;            /* Local size of all cells in apCell[] */
  u8 *ap_ಕೊನೆ[NB*2];        /* MemPage.aDataEnd values */
  ಪೂರ್ಣಾಂಕ ixNx[NB*2];         /* Index of at which we move to the next apEnd[] */
};

/*
** Make sure the cell sizes at idx, idx+1, ..., idx+N-1 have been
** computed.
*/
ಸ್ಥಿರ ಶೂನ್ಯ populateCellCache(CellArray *p, ಪೂರ್ಣಾಂಕ idx, ಪೂರ್ಣಾಂಕ N){
  Mem_ಪುಟ *pRef = p->pRef;
  u16 *szCell = p->szCell;
  ಪ್ರತಿಜ್ಞಾ( idx>=0 && idx+N<=p->nCell );
  ಆಗ( N>0 ){
    ಪ್ರತಿಜ್ಞಾ( p->apCell[idx]!=0 );
    ಆದರೆ( szCell[idx]==0 ){
      szCell[idx] = pRef->x_Cell_ಗಾತ್ರ(pRef, p->apCell[idx]);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB ||
              szCell[idx]==pRef->x_Cell_ಗಾತ್ರ(pRef, p->apCell[idx]) );
    }
    idx++;
    N--;
  }
}

/*
** Return the size of the Nth element of the cell array
*/
ಸ್ಥಿರ SQLITE_NOINLINE u16 compute_Cell_ಗಾತ್ರ(CellArray *p, ಪೂರ್ಣಾಂಕ N){
  ಪ್ರತಿಜ್ಞಾ( N>=0 && N<p->nCell );
  ಪ್ರತಿಜ್ಞಾ( p->szCell[N]==0 );
  p->szCell[N] = p->pRef->x_Cell_ಗಾತ್ರ(p->pRef, p->apCell[N]);
  ಹಿಂತಿರುಗಿ p->szCell[N];
}
ಸ್ಥಿರ u16 cached_Cell_ಗಾತ್ರ(CellArray *p, ಪೂರ್ಣಾಂಕ N){
  ಪ್ರತಿಜ್ಞಾ( N>=0 && N<p->nCell );
  ಆದರೆ( p->szCell[N] ) ಹಿಂತಿರುಗಿ p->szCell[N];
  ಹಿಂತಿರುಗಿ compute_Cell_ಗಾತ್ರ(p, N);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rebuild_ಪುಟ(
  CellArray *pCArray,             /* Content to be added to page pPg */
  ಪೂರ್ಣಾಂಕ i_ಮೊದಲ,                     /* First cell in pCArray to use */
  ಪೂರ್ಣಾಂಕ nCell,                      /* Final number of cells on page */
  Mem_ಪುಟ *pPg                    /* The page to be reconstructed */
){
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ hdr = pPg->hdr_ಸ್ಥಾನಾಂತರ;          /* Offset of header on pPg */
  u8 * ಸ್ಥಿರಾಂಕ a_ದತ್ತಾಂಶ = pPg->a_ದತ್ತಾಂಶ;           /* Pointer to data for pPg */
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ usable_ಗಾತ್ರ = pPg->pBt->usable_ಗಾತ್ರ;
  u8 * ಸ್ಥಿರಾಂಕ p_ಕೊನೆ = &a_ದತ್ತಾಂಶ[usable_ಗಾತ್ರ];
  ಪೂರ್ಣಾಂಕ i = i_ಮೊದಲ;                 /* Which cell to copy from pCArray*/
  u32 j;                          /* Start of cell content area */
  ಪೂರ್ಣಾಂಕ i_ಕೊನೆ = i+nCell;             /* Loop terminator */
  u8 *pCellptr = pPg->aCellIdx;
  u8 *pTmp = sqlite3_Pager_ತಾತ್ಕಾಲಿಕ_Space(pPg->pBt->pPager);
  u8 *p_ದತ್ತಾಂಶ;
  ಪೂರ್ಣಾಂಕ k;                          /* Current slot in pCArray->apEnd[] */
  u8 *p_Src_ಕೊನೆ;                    /* Current pCArray->apEnd[k] value */

  ಪ್ರತಿಜ್ಞಾ( nCell>0 );
  ಪ್ರತಿಜ್ಞಾ( i<i_ಕೊನೆ );
  j = get2byte(&a_ದತ್ತಾಂಶ[hdr+5]);
  ಆದರೆ( j>(u32)usable_ಗಾತ್ರ ){ j = 0; }
  ಸ್ಮರಣೆ_ನಕಲು(&pTmp[j], &a_ದತ್ತಾಂಶ[j], usable_ಗಾತ್ರ - j);

  ಪ್ರತಿಜ್ಞಾ( pCArray->ixNx[NB*2-1]>i );
  ಗಾಗಿ(k=0; pCArray->ixNx[k]<=i; k++){}
  p_Src_ಕೊನೆ = pCArray->ap_ಕೊನೆ[k];

  p_ದತ್ತಾಂಶ = p_ಕೊನೆ;
  ಆಗ( 1/*exit by break*/ ){
    u8 *pCell = pCArray->apCell[i];
    u16 sz = pCArray->szCell[i];
    ಪ್ರತಿಜ್ಞಾ( sz>0 );
    ಆದರೆ( SQLITE_WITHIN(pCell,a_ದತ್ತಾಂಶ+j,p_ಕೊನೆ) ){
      ಆದರೆ( ((uptr)(pCell+sz))>(uptr)p_ಕೊನೆ ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
      pCell = &pTmp[pCell - a_ದತ್ತಾಂಶ];
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (uptr)(pCell+sz)>(uptr)p_Src_ಕೊನೆ
           && (uptr)(pCell)<(uptr)p_Src_ಕೊನೆ
    ){
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
    }

    p_ದತ್ತಾಂಶ -= sz;
    put2byte(pCellptr, (p_ದತ್ತಾಂಶ - a_ದತ್ತಾಂಶ));
    pCellptr += 2;
    ಆದರೆ( p_ದತ್ತಾಂಶ < pCellptr ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
    memmove(p_ದತ್ತಾಂಶ, pCell, sz);
    ಪ್ರತಿಜ್ಞಾ( sz==pPg->x_Cell_ಗಾತ್ರ(pPg, pCell) || CORRUPT_DB );
    i++;
    ಆದರೆ( i>=i_ಕೊನೆ ) ನಿಲ್ಲಿಸು;
    ಆದರೆ( pCArray->ixNx[k]<=i ){
      k++;
      p_Src_ಕೊನೆ = pCArray->ap_ಕೊನೆ[k];
    }
  }

  /* The pPg->nFree field is now set incorrectly. The caller will fix it. */
  ಪ್ರತಿಜ್ಞಾ( nCell < 10922 );
  pPg->nCell = (u16)nCell;
  pPg->n_ಅತಿಪ್ರವಾಹ = 0;

  put2byte(&a_ದತ್ತಾಂಶ[hdr+1], 0);
  put2byte(&a_ದತ್ತಾಂಶ[hdr+3], pPg->nCell);
  put2byte(&a_ದತ್ತಾಂಶ[hdr+5], p_ದತ್ತಾಂಶ - a_ದತ್ತಾಂಶ);
  a_ದತ್ತಾಂಶ[hdr+7] = 0x00;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಪುಟ_ಸೇರಿಸು_Array(
  Mem_ಪುಟ *pPg,                   /* Page to add cells to */
  u8 *pBegin,                     /* End of cell-pointer array */
  u8 **pp_ದತ್ತಾಂಶ,                    /* IN/OUT: Page content-area pointer */
  u8 *pCellptr,                   /* Pointer to cell-pointer area */
  ಪೂರ್ಣಾಂಕ i_ಮೊದಲ,                     /* Index of first cell to add */
  ಪೂರ್ಣಾಂಕ nCell,                      /* Number of cells to add to pPg */
  CellArray *pCArray              /* Array of cells */
){
  ಪೂರ್ಣಾಂಕ i = i_ಮೊದಲ;                 /* Loop counter - cell index to insert */
  u8 *a_ದತ್ತಾಂಶ = pPg->a_ದತ್ತಾಂಶ;         /* Complete page */
  u8 *p_ದತ್ತಾಂಶ = *pp_ದತ್ತಾಂಶ;            /* Content area.  A subset of aData[] */
  ಪೂರ್ಣಾಂಕ i_ಕೊನೆ = i_ಮೊದಲ + nCell;      /* End of loop. One past last cell to ins */
  ಪೂರ್ಣಾಂಕ k;                          /* Current slot in pCArray->apEnd[] */
  u8 *p_ಕೊನೆ;                       /* Maximum extent of cell data */
  ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB || pPg->hdr_ಸ್ಥಾನಾಂತರ==0 );    /* Never called on page 1 */
  ಆದರೆ( i_ಕೊನೆ<=i_ಮೊದಲ ) ಹಿಂತಿರುಗಿ 0;
  ಪ್ರತಿಜ್ಞಾ( pCArray->ixNx[NB*2-1]>i );
  ಗಾಗಿ(k=0; pCArray->ixNx[k]<=i ; k++){}
  p_ಕೊನೆ = pCArray->ap_ಕೊನೆ[k];
  ಆಗ( 1 /*Exit by break*/ ){
    ಪೂರ್ಣಾಂಕ sz, rc;
    u8 *p_ಸ್ಥಾನ;
    ಪ್ರತಿಜ್ಞಾ( pCArray->szCell[i]!=0 );
    sz = pCArray->szCell[i];
    ಆದರೆ( (a_ದತ್ತಾಂಶ[1]==0 && a_ದತ್ತಾಂಶ[2]==0) || (p_ಸ್ಥಾನ = ಪುಟ_ಕಂಡುಹಿಡಿ_ಸ್ಥಾನ(pPg,sz,&rc))==0 ){
      ಆದರೆ( (p_ದತ್ತಾಂಶ - pBegin)<sz ) ಹಿಂತಿರುಗಿ 1;
      p_ದತ್ತಾಂಶ -= sz;
      p_ಸ್ಥಾನ = p_ದತ್ತಾಂಶ;
    }
    /* pSlot and pCArray->apCell[i] will never overlap on a well-formed
    ** database.  But they might for a corrupt database.  Hence use memmove()
    ** since memcpy() sends SIGABORT with overlapping buffers on OpenBSD */
    ಪ್ರತಿಜ್ಞಾ( (p_ಸ್ಥಾನ+sz)<=pCArray->apCell[i]
         || p_ಸ್ಥಾನ>=(pCArray->apCell[i]+sz)
         || CORRUPT_DB );
    ಆದರೆ( (uptr)(pCArray->apCell[i]+sz)>(uptr)p_ಕೊನೆ
     && (uptr)(pCArray->apCell[i])<(uptr)p_ಕೊನೆ
    ){
      ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB );
      (ಶೂನ್ಯ)SQLITE_CORRUPT_BKPT;
      ಹಿಂತಿರುಗಿ 1;
    }
    memmove(p_ಸ್ಥಾನ, pCArray->apCell[i], sz);
    put2byte(pCellptr, (p_ಸ್ಥಾನ - a_ದತ್ತಾಂಶ));
    pCellptr += 2;
    i++;
    ಆದರೆ( i>=i_ಕೊನೆ ) ನಿಲ್ಲಿಸು;
    ಆದರೆ( pCArray->ixNx[k]<=i ){
      k++;
      p_ಕೊನೆ = pCArray->ap_ಕೊನೆ[k];
    }
  }
  *pp_ದತ್ತಾಂಶ = p_ದತ್ತಾಂಶ;
  ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಪುಟ_Free_Array(
  Mem_ಪುಟ *pPg,                   /* Page to edit */
  ಪೂರ್ಣಾಂಕ i_ಮೊದಲ,                     /* First cell to delete */
  ಪೂರ್ಣಾಂಕ nCell,                      /* Cells to delete */
  CellArray *pCArray              /* Array of cells */
){
  u8 * ಸ್ಥಿರಾಂಕ a_ದತ್ತಾಂಶ = pPg->a_ದತ್ತಾಂಶ;
  u8 * ಸ್ಥಿರಾಂಕ p_ಕೊನೆ = &a_ದತ್ತಾಂಶ[pPg->pBt->usable_ಗಾತ್ರ];
  u8 * ಸ್ಥಿರಾಂಕ p_ಆರಂಭ = &a_ದತ್ತಾಂಶ[pPg->hdr_ಸ್ಥಾನಾಂತರ + 8 + pPg->ಮಗು_Ptr_ಗಾತ್ರ];
  ಪೂರ್ಣಾಂಕ nRet = 0;
  ಪೂರ್ಣಾಂಕ i, j;
  ಪೂರ್ಣಾಂಕ i_ಕೊನೆ = i_ಮೊದಲ + nCell;
  ಪೂರ್ಣಾಂಕ nFree = 0;
  ಪೂರ್ಣಾಂಕ aOfst[10];
  ಪೂರ್ಣಾಂಕ aAfter[10];

  ಗಾಗಿ(i=i_ಮೊದಲ; i<i_ಕೊನೆ; i++){
    u8 *pCell = pCArray->apCell[i];
    ಆದರೆ( SQLITE_WITHIN(pCell, p_ಆರಂಭ, p_ಕೊನೆ) ){
      ಪೂರ್ಣಾಂಕ sz;
      ಪೂರ್ಣಾಂಕ iAfter;
      ಪೂರ್ಣಾಂಕ iOfst;
      /* No need to use cachedCellSize() here.  The sizes of all cells that
      ** are to be freed have already been computing while deciding which
      ** cells need freeing */
      sz = pCArray->szCell[i];  ಪ್ರತಿಜ್ಞಾ( sz>0 );
      iOfst = (u16)(pCell - a_ದತ್ತಾಂಶ);
      iAfter = iOfst+sz;
      ಗಾಗಿ(j=0; j<nFree; j++){
        ಆದರೆ( aOfst[j]==iAfter ){
          aOfst[j] = iOfst;
          ನಿಲ್ಲಿಸು;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( aAfter[j]==iOfst ){
          aAfter[j] = iAfter;
          ನಿಲ್ಲಿಸು;
        }
      }
      ಆದರೆ( j>=nFree ){
        ಆದರೆ( nFree>=(ಪೂರ್ಣಾಂಕ)(ಗಾತ್ರ(aOfst)/ಗಾತ್ರ(aOfst[0])) ){
          ಗಾಗಿ(j=0; j<nFree; j++){
            freeSpace(pPg, aOfst[j], aAfter[j]-aOfst[j]);
          }
          nFree = 0;
        }
        aOfst[nFree] = iOfst;
        aAfter[nFree] = iAfter;
        ಆದರೆ( &a_ದತ್ತಾಂಶ[iAfter]>p_ಕೊನೆ ) ಹಿಂತಿರುಗಿ 0;
        nFree++;
      }
      nRet++;
    }
  }
  ಗಾಗಿ(j=0; j<nFree; j++){
    freeSpace(pPg, aOfst[j], aAfter[j]-aOfst[j]);
  }
  ಹಿಂತಿರುಗಿ nRet;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ edit_ಪುಟ(
  Mem_ಪುಟ *pPg,                   /* Edit this page */
  ಪೂರ್ಣಾಂಕ iOld,                       /* Index of first cell currently on page */
  ಪೂರ್ಣಾಂಕ iNew,                       /* Index of new first cell on page */
  ಪೂರ್ಣಾಂಕ nNew,                       /* Final number of cells on page */
  CellArray *pCArray              /* Array of cells and sizes */
){
  u8 * ಸ್ಥಿರಾಂಕ a_ದತ್ತಾಂಶ = pPg->a_ದತ್ತಾಂಶ;
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ hdr = pPg->hdr_ಸ್ಥಾನಾಂತರ;
  u8 *pBegin = &pPg->aCellIdx[nNew * 2];
  ಪೂರ್ಣಾಂಕ nCell = pPg->nCell;       /* Cells stored on pPg */
  u8 *p_ದತ್ತಾಂಶ;
  u8 *pCellptr;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ i_Old_ಕೊನೆ = iOld + pPg->nCell + pPg->n_ಅತಿಪ್ರವಾಹ;
  ಪೂರ್ಣಾಂಕ i_New_ಕೊನೆ = iNew + nNew;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  u8 *pTmp = sqlite3_Pager_ತಾತ್ಕಾಲಿಕ_Space(pPg->pBt->pPager);
  ಸ್ಮರಣೆ_ನಕಲು(pTmp, a_ದತ್ತಾಂಶ, pPg->pBt->usable_ಗಾತ್ರ);
#ಆದರೆ_ಅಂತ್ಯ

  /* Remove cells from the start and end of the page */
  ಪ್ರತಿಜ್ಞಾ( nCell>=0 );
  ಆದರೆ( iOld<iNew ){
    ಪೂರ್ಣಾಂಕ n_ಸರಿಸು = ಪುಟ_Free_Array(pPg, iOld, iNew-iOld, pCArray);
    ಆದರೆ( NEVER(n_ಸರಿಸು>nCell) ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
    memmove(pPg->aCellIdx, &pPg->aCellIdx[n_ಸರಿಸು*2], nCell*2);
    nCell -= n_ಸರಿಸು;
  }
  ಆದರೆ( i_New_ಕೊನೆ < i_Old_ಕೊನೆ ){
    ಪೂರ್ಣಾಂಕ n_ಬಾಲ = ಪುಟ_Free_Array(pPg, i_New_ಕೊನೆ, i_Old_ಕೊನೆ - i_New_ಕೊನೆ, pCArray);
    ಪ್ರತಿಜ್ಞಾ( nCell>=n_ಬಾಲ );
    nCell -= n_ಬಾಲ;
  }

  p_ದತ್ತಾಂಶ = &a_ದತ್ತಾಂಶ[get2byte(&a_ದತ್ತಾಂಶ[hdr+5])];
  ಆದರೆ( p_ದತ್ತಾಂಶ<pBegin ) ಹೋಗು editpage_fail;
  ಆದರೆ( NEVER(p_ದತ್ತಾಂಶ>pPg->a_ದತ್ತಾಂಶ_ಕೊನೆ) ) ಹೋಗು editpage_fail;

  /* Add cells to the start of the page */
  ಆದರೆ( iNew<iOld ){
    ಪೂರ್ಣಾಂಕ n_ಕೂಡಿಸು = MIN(nNew,iOld-iNew);
    ಪ್ರತಿಜ್ಞಾ( (iOld-iNew)<nNew || nCell==0 || CORRUPT_DB );
    ಪ್ರತಿಜ್ಞಾ( n_ಕೂಡಿಸು>=0 );
    pCellptr = pPg->aCellIdx;
    memmove(&pCellptr[n_ಕೂಡಿಸು*2], pCellptr, nCell*2);
    ಆದರೆ( ಪುಟ_ಸೇರಿಸು_Array(
          pPg, pBegin, &p_ದತ್ತಾಂಶ, pCellptr,
          iNew, n_ಕೂಡಿಸು, pCArray
    ) ) ಹೋಗು editpage_fail;
    nCell += n_ಕೂಡಿಸು;
  }

  /* Add any overflow cells */
  ಗಾಗಿ(i=0; i<pPg->n_ಅತಿಪ್ರವಾಹ; i++){
    ಪೂರ್ಣಾಂಕ iCell = (iOld + pPg->aiOvfl[i]) - iNew;
    ಆದರೆ( iCell>=0 && iCell<nNew ){
      pCellptr = &pPg->aCellIdx[iCell * 2];
      ಆದರೆ( nCell>iCell ){
        memmove(&pCellptr[2], pCellptr, (nCell - iCell) * 2);
      }
      nCell++;
      cached_Cell_ಗಾತ್ರ(pCArray, iCell+iNew);
      ಆದರೆ( ಪುಟ_ಸೇರಿಸು_Array(
            pPg, pBegin, &p_ದತ್ತಾಂಶ, pCellptr,
            iCell+iNew, 1, pCArray
      ) ) ಹೋಗು editpage_fail;
    }
  }

  /* Append cells to the end of the page */
  ಪ್ರತಿಜ್ಞಾ( nCell>=0 );
  pCellptr = &pPg->aCellIdx[nCell*2];
  ಆದರೆ( ಪುಟ_ಸೇರಿಸು_Array(
        pPg, pBegin, &p_ದತ್ತಾಂಶ, pCellptr,
        iNew+nCell, nNew-nCell, pCArray
      )
  ){
    ಹೋಗು editpage_fail;
  }

  ಪ್ರತಿಜ್ಞಾ( nNew < 10922 );
  pPg->nCell = (u16)nNew;
  pPg->n_ಅತಿಪ್ರವಾಹ = 0;

  put2byte(&a_ದತ್ತಾಂಶ[hdr+3], pPg->nCell);
  put2byte(&a_ದತ್ತಾಂಶ[hdr+5], p_ದತ್ತಾಂಶ - a_ದತ್ತಾಂಶ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಗಾಗಿ(i=0; i<nNew && !CORRUPT_DB; i++){
    u8 *pCell = pCArray->apCell[i+iNew];
    ಪೂರ್ಣಾಂಕ iOff = get2byteAligned(&pPg->aCellIdx[i*2]);
    ಆದರೆ( SQLITE_WITHIN(pCell, a_ದತ್ತಾಂಶ, &a_ದತ್ತಾಂಶ[pPg->pBt->usable_ಗಾತ್ರ]) ){
      pCell = &pTmp[pCell - a_ದತ್ತಾಂಶ];
    }
    ಪ್ರತಿಜ್ಞಾ( 0==memcmp(pCell, &a_ದತ್ತಾಂಶ[iOff],
            pCArray->pRef->x_Cell_ಗಾತ್ರ(pCArray->pRef, pCArray->apCell[i+iNew])) );
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ SQLITE_OK;
 editpage_fail:
  /* Unable to edit this page. Rebuild it from scratch instead. */
  ಆದರೆ( nNew<1 ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;
  populateCellCache(pCArray, iNew, nNew);
  ಹಿಂತಿರುಗಿ rebuild_ಪುಟ(pCArray, iNew, nNew, pPg);
}


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_QUICKBALANCE
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ balance_ಶೀಘ್ರ(Mem_ಪುಟ *p_ಪೋಷಕ, Mem_ಪುಟ *p_ಪುಟ, u8 *pSpace){
  BtShared *ಸ್ಥಿರಾಂಕ pBt = p_ಪುಟ->pBt;    /* B-Tree Database */
  Mem_ಪುಟ *pNew;                       /* Newly allocated page */
  ಪೂರ್ಣಾಂಕ rc;                              /* Return Code */
  Pgno pgnoNew;                        /* Page number of pNew */

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಪೋಷಕ->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ==1 );
 
  ಆದರೆ( p_ಪುಟ->nCell==0 ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_BKPT;  /* dbfuzz001.test */
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nFree>=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪೋಷಕ->nFree>=0 );

  /* Allocate a new page. This page will become the right-sibling of
  ** pPage. Make the parent page writable, so that the new divider cell
  ** may be inserted. If both these operations are successful, proceed.
  */
  rc = allocate_Btree_ಪುಟ(pBt, &pNew, &pgnoNew, 0, 0);

  ಆದರೆ( rc==SQLITE_OK ){

    u8 *p_ಹೊರಹೋಗು = &pSpace[4];
    u8 *pCell = p_ಪುಟ->apOvfl[0];
    u16 szCell = p_ಪುಟ->x_Cell_ಗಾತ್ರ(p_ಪುಟ, pCell);
    u8 *pStop;
    CellArray b;

    ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(pNew->p_Db_ಪುಟ) );
    ಪ್ರತಿಜ್ಞಾ( CORRUPT_DB || p_ಪುಟ->a_ದತ್ತಾಂಶ[0]==(PTF_INTKEY|PTF_LEAFDATA|PTF_ಎಲೆ) );
    zero_ಪುಟ(pNew, PTF_INTKEY|PTF_LEAFDATA|PTF_ಎಲೆ);
    b.nCell = 1;
    b.pRef = p_ಪುಟ;
    b.apCell = &pCell;
    b.szCell = &szCell;
    b.ap_ಕೊನೆ[0] = p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ;
    b.ixNx[0] = 2;
    b.ixNx[NB*2-1] = 0x7fffffff;
    rc = rebuild_ಪುಟ(&b, 0, 1, pNew);
    ಆದರೆ( NEVER(rc) ){
      release_ಪುಟ(pNew);
      ಹಿಂತಿರುಗಿ rc;
    }
    pNew->nFree = pBt->usable_ಗಾತ್ರ - pNew->cell_ಸ್ಥಾನಾಂತರ - 2 - szCell;

    /* If this is an auto-vacuum database, update the pointer map
    ** with entries for the new page, and any pointer from the
    ** cell on the page to an overflow page. If either of these
    ** operations fails, the return code is set, but the contents
    ** of the parent page are still manipulated by the code below.
    ** That is Ok, at this point the parent page is guaranteed to
    ** be marked as dirty. Returning an error code will cause a
    ** rollback, undoing any changes made to the parent page.
    */
    ಆದರೆ( ISAUTOVACUUM(pBt) ){
      ptrmapPut(pBt, pgnoNew, PTRMAP_BTREE, p_ಪೋಷಕ->pgno, &rc);
      ಆದರೆ( szCell>pNew->minLocal ){
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
    pCell = ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, p_ಪುಟ->nCell-1);
    pStop = &pCell[9];
    ಆಗ( (*(pCell++)&0x80) && pCell<pStop );
    pStop = &pCell[9];
    ಆಗ( ((*(p_ಹೊರಹೋಗು++) = *(pCell++))&0x80) && pCell<pStop );

    /* Insert the new divider cell into pParent. */
    ಆದರೆ( rc==SQLITE_OK ){
      rc = ಸೇರಿಸು_Cell(p_ಪೋಷಕ, p_ಪೋಷಕ->nCell, pSpace, (ಪೂರ್ಣಾಂಕ)(p_ಹೊರಹೋಗು-pSpace),
                      0, p_ಪುಟ->pgno);
    }

    /* Set the right-child pointer of pParent to point to the new page. */
    put4byte(&p_ಪೋಷಕ->a_ದತ್ತಾಂಶ[p_ಪೋಷಕ->hdr_ಸ್ಥಾನಾಂತರ+8], pgnoNew);
 
    /* Release the reference to the new page. */
    release_ಪುಟ(pNew);
  }

  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_QUICKBALANCE */

#ಆದರೆ 0
/*
** This function does not contribute anything to the operation of SQLite.
** it is sometimes activated temporarily while debugging code responsible
** for setting pointer-map entries.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ptrmap_Check_ಪುಟಗಳು(Mem_ಪುಟ **ap_ಪುಟ, ಪೂರ್ಣಾಂಕ n_ಪುಟ){
  ಪೂರ್ಣಾಂಕ i, j;
  ಗಾಗಿ(i=0; i<n_ಪುಟ; i++){
    Pgno n;
    u8 e;
    Mem_ಪುಟ *p_ಪುಟ = ap_ಪುಟ[i];
    BtShared *pBt = p_ಪುಟ->pBt;
    ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->isInit );

    ಗಾಗಿ(j=0; j<p_ಪುಟ->nCell; j++){
      CellInfo info;
      u8 *z;
    
      z = ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, j);
      p_ಪುಟ->xParseCell(p_ಪುಟ, z, &info);
      ಆದರೆ( info.nLocal<info.nPayload ){
        Pgno ovfl = get4byte(&z[info.n_ಗಾತ್ರ-4]);
        ptrmapGet(pBt, ovfl, &e, &n);
        ಪ್ರತಿಜ್ಞಾ( n==p_ಪುಟ->pgno && e==PTRMAP_OVERFLOW1 );
      }
      ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
        Pgno ಮಗು = get4byte(z);
        ptrmapGet(pBt, ಮಗು, &e, &n);
        ಪ್ರತಿಜ್ಞಾ( n==p_ಪುಟ->pgno && e==PTRMAP_BTREE );
      }
    }
    ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
      Pgno ಮಗು = get4byte(&p_ಪುಟ->a_ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+8]);
      ptrmapGet(pBt, ಮಗು, &e, &n);
      ಪ್ರತಿಜ್ಞಾ( n==p_ಪುಟ->pgno && e==PTRMAP_BTREE );
    }
  }
  ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಶೂನ್ಯ copy_ಗಂಟು_Content(Mem_ಪುಟ *pFrom, Mem_ಪುಟ *p_ಗಮ್ಯ, ಪೂರ್ಣಾಂಕ *pRC){
  ಆದರೆ( (*pRC)==SQLITE_OK ){
    BtShared * ಸ್ಥಿರಾಂಕ pBt = pFrom->pBt;
    u8 * ಸ್ಥಿರಾಂಕ aFrom = pFrom->a_ದತ್ತಾಂಶ;
    u8 * ಸ್ಥಿರಾಂಕ a_ಗಮ್ಯ = p_ಗಮ್ಯ->a_ದತ್ತಾಂಶ;
    ಪೂರ್ಣಾಂಕ ಸ್ಥಿರಾಂಕ iFromHdr = pFrom->hdr_ಸ್ಥಾನಾಂತರ;
    ಪೂರ್ಣಾಂಕ ಸ್ಥಿರಾಂಕ i_ಗಮ್ಯ_Hdr = ((p_ಗಮ್ಯ->pgno==1) ? 100 : 0);
    ಪೂರ್ಣಾಂಕ rc;
    ಪೂರ್ಣಾಂಕ i_ದತ್ತಾಂಶ;
 
 
    ಪ್ರತಿಜ್ಞಾ( pFrom->isInit );
    ಪ್ರತಿಜ್ಞಾ( pFrom->nFree>=i_ಗಮ್ಯ_Hdr );
    ಪ್ರತಿಜ್ಞಾ( get2byte(&aFrom[iFromHdr+5]) <= (ಪೂರ್ಣಾಂಕ)pBt->usable_ಗಾತ್ರ );
 
    /* Copy the b-tree node content from page pFrom to page pTo. */
    i_ದತ್ತಾಂಶ = get2byte(&aFrom[iFromHdr+5]);
    ಸ್ಮರಣೆ_ನಕಲು(&a_ಗಮ್ಯ[i_ದತ್ತಾಂಶ], &aFrom[i_ದತ್ತಾಂಶ], pBt->usable_ಗಾತ್ರ-i_ದತ್ತಾಂಶ);
    ಸ್ಮರಣೆ_ನಕಲು(&a_ಗಮ್ಯ[i_ಗಮ್ಯ_Hdr], &aFrom[iFromHdr], pFrom->cell_ಸ್ಥಾನಾಂತರ + 2*pFrom->nCell);
 
    /* Reinitialize page pTo so that the contents of the MemPage structure
    ** match the new data. The initialization of pTo can actually fail under
    ** fairly obscure circumstances, even though it is a copy of initialized
    ** page pFrom.
    */
    p_ಗಮ್ಯ->isInit = 0;
    rc = btree_Init_ಪುಟ(p_ಗಮ್ಯ);
    ಆದರೆ( rc==SQLITE_OK ) rc = btreeComputeFreeSpace(p_ಗಮ್ಯ);
    ಆದರೆ( rc!=SQLITE_OK ){
      *pRC = rc;
      ಹಿಂತಿರುಗಿ;
    }
 
    /* If this is an auto-vacuum database, update the pointer-map entries
    ** for any b-tree or overflow pages that pTo now contains the pointers to.
    */
    ಆದರೆ( ISAUTOVACUUM(pBt) ){
      *pRC = ಗಣ_ಮಗು_Ptrmaps(p_ಗಮ್ಯ);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ balance_nonroot(
  Mem_ಪುಟ *p_ಪೋಷಕ,               /* Parent page of siblings being balanced */
  ಪೂರ್ಣಾಂಕ i_ಪೋಷಕ_Idx,                 /* Index of "the page" in pParent */
  u8 *aOvflSpace,                 /* page-size bytes of space for parent ovfl */
  ಪೂರ್ಣಾಂಕ is_ಬೇರು,                     /* True if pParent is a root-page */
  ಪೂರ್ಣಾಂಕ bBulk                       /* True if this call is part of a bulk load */
){
  BtShared *pBt;               /* The whole database */
  ಪೂರ್ಣಾಂಕ nMaxCells = 0;           /* Allocated size of apCell, szCell, aFrom. */
  ಪೂರ್ಣಾಂಕ nNew = 0;                /* Number of pages in apNew[] */
  ಪೂರ್ಣಾಂಕ nOld;                    /* Number of pages in apOld[] */
  ಪೂರ್ಣಾಂಕ i, j, k;                 /* Loop counters */
  ಪೂರ್ಣಾಂಕ nxDiv;                   /* Next divider slot in pParent->aCell[] */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;          /* The return code */
  u16 ಎಲೆ_Correction;          /* 4 if pPage is a leaf.  0 if not */
  ಪೂರ್ಣಾಂಕ ಎಲೆ_ದತ್ತಾಂಶ;                /* True if pPage is a leaf of a LEAFDATA tree */
  ಪೂರ್ಣಾಂಕ usableSpace;             /* Bytes in pPage beyond the header */
  ಪೂರ್ಣಾಂಕ ಪುಟ_Flags;               /* Value of pPage->aData[0] */
  ಪೂರ್ಣಾಂಕ iSpace1 = 0;             /* First unused byte of aSpace1[] */
  ಪೂರ್ಣಾಂಕ iOvflSpace = 0;          /* First unused byte of aOvflSpace[] */
  u64 szScratch;               /* Size of scratch memory requested */
  Mem_ಪುಟ *apOld[NB];          /* pPage and up to two siblings */
  Mem_ಪುಟ *apNew[NB+2];        /* pPage and up to NB siblings after balancing */
  u8 *p_ಬಲ;                  /* Location in parent of right-sibling pointer */
  u8 *apDiv[NB-1];             /* Divider cells in pParent */
  ಪೂರ್ಣಾಂಕ cntNew[NB+2];            /* Index in b.paCell[] of cell after i-th page */
  ಪೂರ್ಣಾಂಕ cntOld[NB+2];            /* Old index in b.apCell[] */
  ಪೂರ್ಣಾಂಕ szNew[NB+2];             /* Combined size of cells placed on i-th page */
  u8 *aSpace1;                 /* Space for copies of dividers cells */
  Pgno pgno;                   /* Temp var to store a page number in */
  u8 ab_ಮುಗಿದ[NB+2];             /* True after i'th new page is populated */
  Pgno aPgno[NB+2];            /* Page numbers of new pages before shuffling */
  CellArray b;                 /* Parsed information on cells being balanced */

  memset(ab_ಮುಗಿದ, 0, ಗಾತ್ರ(ab_ಮುಗಿದ));
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(b) - ಗಾತ್ರ(b.ixNx) == offsetof(CellArray,ixNx) );
  memset(&b, 0, ಗಾತ್ರ(b)-ಗಾತ್ರ(b.ixNx[0]));
  b.ixNx[NB*2-1] = 0x7fffffff;
  pBt = p_ಪೋಷಕ->pBt;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಪೋಷಕ->p_Db_ಪುಟ) );

  /* At this point pParent may have at most one overflow cell. And if
  ** this overflow cell is present, it must be the cell with
  ** index iParentIdx. This scenario comes about when this function
  ** is called (indirectly) from sqlite3BtreeDelete().
  */
  ಪ್ರತಿಜ್ಞಾ( p_ಪೋಷಕ->n_ಅತಿಪ್ರವಾಹ==0 || p_ಪೋಷಕ->n_ಅತಿಪ್ರವಾಹ==1 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪೋಷಕ->n_ಅತಿಪ್ರವಾಹ==0 || p_ಪೋಷಕ->aiOvfl[0]==i_ಪೋಷಕ_Idx );

  ಆದರೆ( !aOvflSpace ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  ಪ್ರತಿಜ್ಞಾ( p_ಪೋಷಕ->nFree>=0 );

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
  i = p_ಪೋಷಕ->n_ಅತಿಪ್ರವಾಹ + p_ಪೋಷಕ->nCell;
  ಆದರೆ( i<2 ){
    nxDiv = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( bBulk==0 || bBulk==1 );
    ಆದರೆ( i_ಪೋಷಕ_Idx==0 ){                
      nxDiv = 0;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( i_ಪೋಷಕ_Idx==i ){
      nxDiv = i-2+bBulk;
    }ಇಲ್ಲದಿದ್ದರೆ{
      nxDiv = i_ಪೋಷಕ_Idx-1;
    }
    i = 2-bBulk;
  }
  nOld = i+1;
  ಆದರೆ( (i+nxDiv-p_ಪೋಷಕ->n_ಅತಿಪ್ರವಾಹ)==p_ಪೋಷಕ->nCell ){
    p_ಬಲ = &p_ಪೋಷಕ->a_ದತ್ತಾಂಶ[p_ಪೋಷಕ->hdr_ಸ್ಥಾನಾಂತರ+8];
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ಬಲ = ಕಂಡುಹಿಡಿ_Cell(p_ಪೋಷಕ, i+nxDiv-p_ಪೋಷಕ->n_ಅತಿಪ್ರವಾಹ);
  }
  pgno = get4byte(p_ಬಲ);
  ಆಗ( 1 ){
    ಆದರೆ( rc==SQLITE_OK ){
      rc = get_And_Init_ಪುಟ(pBt, pgno, &apOld[i], 0);
    }
    ಆದರೆ( rc ){
      memset(apOld, 0, (i+1)*ಗಾತ್ರ(Mem_ಪುಟ*));
      ಹೋಗು balance_cleanup;
    }
    ಆದರೆ( apOld[i]->nFree<0 ){
      rc = btreeComputeFreeSpace(apOld[i]);
      ಆದರೆ( rc ){
        memset(apOld, 0, (i)*ಗಾತ್ರ(Mem_ಪುಟ*));
        ಹೋಗು balance_cleanup;
      }
    }
    nMaxCells += apOld[i]->nCell + Array_ಗಾತ್ರ(p_ಪೋಷಕ->apOvfl);
    ಆದರೆ( (i--)==0 ) ನಿಲ್ಲಿಸು;

    ಆದರೆ( p_ಪೋಷಕ->n_ಅತಿಪ್ರವಾಹ && i+nxDiv==p_ಪೋಷಕ->aiOvfl[0] ){
      apDiv[i] = p_ಪೋಷಕ->apOvfl[0];
      pgno = get4byte(apDiv[i]);
      szNew[i] = p_ಪೋಷಕ->x_Cell_ಗಾತ್ರ(p_ಪೋಷಕ, apDiv[i]);
      p_ಪೋಷಕ->n_ಅತಿಪ್ರವಾಹ = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      apDiv[i] = ಕಂಡುಹಿಡಿ_Cell(p_ಪೋಷಕ, i+nxDiv-p_ಪೋಷಕ->n_ಅತಿಪ್ರವಾಹ);
      pgno = get4byte(apDiv[i]);
      szNew[i] = p_ಪೋಷಕ->x_Cell_ಗಾತ್ರ(p_ಪೋಷಕ, apDiv[i]);

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
      ಆದರೆ( pBt->btsFlags & BTS_FAST_SECURE ){
        ಪೂರ್ಣಾಂಕ iOff;

        /* If the following if() condition is not true, the db is corrupted.
        ** The call to dropCell() below will detect this.  */
        iOff = SQLITE_PTR_ಗಮ್ಯ_INT(apDiv[i]) - SQLITE_PTR_ಗಮ್ಯ_INT(p_ಪೋಷಕ->a_ದತ್ತಾಂಶ);
        ಆದರೆ( (iOff+szNew[i])<=(ಪೂರ್ಣಾಂಕ)pBt->usable_ಗಾತ್ರ ){
          ಸ್ಮರಣೆ_ನಕಲು(&aOvflSpace[iOff], apDiv[i], szNew[i]);
          apDiv[i] = &aOvflSpace[apDiv[i]-p_ಪೋಷಕ->a_ದತ್ತಾಂಶ];
        }
      }
      dropCell(p_ಪೋಷಕ, i+nxDiv-p_ಪೋಷಕ->n_ಅತಿಪ್ರವಾಹ, szNew[i], &rc);
    }
  }

  /* Make nMaxCells a multiple of 4 in order to preserve 8-byte
  ** alignment */
  nMaxCells = (nMaxCells + 3)&~3;

  /*
  ** Allocate space for memory structures
  */
  szScratch =
       nMaxCells*ಗಾತ್ರ(u8*)                       /* b.apCell */
     + nMaxCells*ಗಾತ್ರ(u16)                       /* b.szCell */
     + pBt->ಪುಟ_ಗಾತ್ರ;                              /* aSpace1 */

  ಪ್ರತಿಜ್ಞಾ( szScratch<=7*(ಪೂರ್ಣಾಂಕ)pBt->ಪುಟ_ಗಾತ್ರ );
  b.apCell = sqlite3_ರಾಶಿ_Alloc_Raw(0, szScratch );
  ಆದರೆ( b.apCell==0 ){
    rc = SQLITE_NOMEM_BKPT;
    ಹೋಗು balance_cleanup;
  }
  b.szCell = (u16*)&b.apCell[nMaxCells];
  aSpace1 = (u8*)&b.szCell[nMaxCells];
  ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT(aSpace1) );

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
  ಎಲೆ_Correction = b.pRef->ಎಲೆ*4;
  ಎಲೆ_ದತ್ತಾಂಶ = b.pRef->int_ಕೀಲಿ_ಎಲೆ;
  ಗಾಗಿ(i=0; i<nOld; i++){
    Mem_ಪುಟ *pOld = apOld[i];
    ಪೂರ್ಣಾಂಕ ಎಲ್ಲೆ = pOld->nCell;
    u8 *a_ದತ್ತಾಂಶ = pOld->a_ದತ್ತಾಂಶ;
    u16 ಮುಖವಾಡ_ಪುಟ = pOld->ಮುಖವಾಡ_ಪುಟ;
    u8 *piCell = a_ದತ್ತಾಂಶ + pOld->cell_ಸ್ಥಾನಾಂತರ;
    u8 *pi_ಕೊನೆ;
    u8 **ppCell;
    VVA_ONLY( ಪೂರ್ಣಾಂಕ n_Cell_At_ಆರಂಭ = b.nCell; )

    /* Verify that all sibling pages are of the same "type" (table-leaf,
    ** table-interior, index-leaf, or index-interior).
    */
    ಆದರೆ( pOld->a_ದತ್ತಾಂಶ[0]!=apOld[0]->a_ದತ್ತಾಂಶ[0] ){
      rc = SQLITE_CORRUPT_ಪುಟ(pOld);
      ಹೋಗು balance_cleanup;
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
    memset(&b.szCell[b.nCell], 0, ಗಾತ್ರ(b.szCell[0])*(ಎಲ್ಲೆ+pOld->n_ಅತಿಪ್ರವಾಹ));
    ಆದರೆ( pOld->n_ಅತಿಪ್ರವಾಹ>0 ){
      ಆದರೆ( NEVER(ಎಲ್ಲೆ<pOld->aiOvfl[0]) ){
        rc = SQLITE_CORRUPT_ಪುಟ(pOld);
        ಹೋಗು balance_cleanup;
      }
      ಎಲ್ಲೆ = pOld->aiOvfl[0];
      ಗಾಗಿ(j=0; j<ಎಲ್ಲೆ; j++){
        b.apCell[b.nCell+j] = a_ದತ್ತಾಂಶ + (ಮುಖವಾಡ_ಪುಟ & get2byteAligned(piCell));
        piCell += 2;
      }
      b.nCell += ಎಲ್ಲೆ;
      ಗಾಗಿ(k=0; k<pOld->n_ಅತಿಪ್ರವಾಹ; k++){
        ಪ್ರತಿಜ್ಞಾ( k==0 || pOld->aiOvfl[k-1]+1==pOld->aiOvfl[k] );/* NOTE 1 */
        b.apCell[b.nCell] = pOld->apOvfl[k];
        b.nCell++;
      }
    }
    pi_ಕೊನೆ = a_ದತ್ತಾಂಶ + pOld->cell_ಸ್ಥಾನಾಂತರ + 2*pOld->nCell;
    /* Collect cell pointers before updating the cell count. */
    ppCell = &b.apCell[b.nCell];
    ಆಗ( piCell<pi_ಕೊನೆ ){
      ಪ್ರತಿಜ್ಞಾ( ppCell<&b.apCell[nMaxCells] );
      *ppCell++ = a_ದತ್ತಾಂಶ + (ಮುಖವಾಡ_ಪುಟ & get2byteAligned(piCell));
      piCell += 2;
    }
    b.nCell = (ಪೂರ್ಣಾಂಕ)(ppCell - b.apCell);
    ಪ್ರತಿಜ್ಞಾ( (b.nCell-n_Cell_At_ಆರಂಭ)==(pOld->nCell+pOld->n_ಅತಿಪ್ರವಾಹ) );

    cntOld[i] = b.nCell;
    ಆದರೆ( i<nOld-1 && !ಎಲೆ_ದತ್ತಾಂಶ){
      u16 sz = (u16)szNew[i];
      u8 *p_ತಾತ್ಕಾಲಿಕ;
      ಪ್ರತಿಜ್ಞಾ( b.nCell<nMaxCells );
      b.szCell[b.nCell] = sz;
      p_ತಾತ್ಕಾಲಿಕ = &aSpace1[iSpace1];
      iSpace1 += sz;
      ಪ್ರತಿಜ್ಞಾ( sz<=pBt->maxLocal+23 );
      ಪ್ರತಿಜ್ಞಾ( iSpace1 <= (ಪೂರ್ಣಾಂಕ)pBt->ಪುಟ_ಗಾತ್ರ );
      ಸ್ಮರಣೆ_ನಕಲು(p_ತಾತ್ಕಾಲಿಕ, apDiv[i], sz);
      b.apCell[b.nCell] = p_ತಾತ್ಕಾಲಿಕ+ಎಲೆ_Correction;
      ಪ್ರತಿಜ್ಞಾ( ಎಲೆ_Correction==0 || ಎಲೆ_Correction==4 );
      b.szCell[b.nCell] = b.szCell[b.nCell] - ಎಲೆ_Correction;
      ಆದರೆ( !pOld->ಎಲೆ ){
        ಪ್ರತಿಜ್ಞಾ( ಎಲೆ_Correction==0 );
        ಪ್ರತಿಜ್ಞಾ( pOld->hdr_ಸ್ಥಾನಾಂತರ==0 || CORRUPT_DB );
        /* The right pointer of the child page pOld becomes the left
        ** pointer of the divider cell */
        ಸ್ಮರಣೆ_ನಕಲು(b.apCell[b.nCell], &pOld->a_ದತ್ತಾಂಶ[8], 4);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( ಎಲೆ_Correction==4 );
        ಆಗ( b.szCell[b.nCell]<4 ){
          /* Do not allow any cells smaller than 4 bytes. If a smaller cell
          ** does exist, pad it with 0x00 bytes. */
          ಪ್ರತಿಜ್ಞಾ( b.szCell[b.nCell]==3 || CORRUPT_DB );
          ಪ್ರತಿಜ್ಞಾ( b.apCell[b.nCell]==&aSpace1[iSpace1-3] || CORRUPT_DB );
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
  usableSpace = pBt->usable_ಗಾತ್ರ - 12 + ಎಲೆ_Correction;
  ಗಾಗಿ(i=k=0; i<nOld; i++, k++){
    Mem_ಪುಟ *p = apOld[i];
    b.ap_ಕೊನೆ[k] = p->a_ದತ್ತಾಂಶ_ಕೊನೆ;
    b.ixNx[k] = cntOld[i];
    ಆದರೆ( k && b.ixNx[k]==b.ixNx[k-1] ){
      k--;  /* Omit b.ixNx[] entry for child pages with no cells */
    }
    ಆದರೆ( !ಎಲೆ_ದತ್ತಾಂಶ ){
      k++;
      b.ap_ಕೊನೆ[k] = p_ಪೋಷಕ->a_ದತ್ತಾಂಶ_ಕೊನೆ;
      b.ixNx[k] = cntOld[i]+1;
    }
    ಪ್ರತಿಜ್ಞಾ( p->nFree>=0 );
    szNew[i] = usableSpace - p->nFree;
    ಗಾಗಿ(j=0; j<p->n_ಅತಿಪ್ರವಾಹ; j++){
      szNew[i] += 2 + p->x_Cell_ಗಾತ್ರ(p, p->apOvfl[j]);
    }
    cntNew[i] = cntOld[i];
  }
  k = nOld;
  ಗಾಗಿ(i=0; i<k; i++){
    ಪೂರ್ಣಾಂಕ sz;
    ಆಗ( szNew[i]>usableSpace ){
      ಆದರೆ( i+1>=k ){
        k = i+2;
        ಆದರೆ( k>NB+2 ){ rc = SQLITE_CORRUPT_BKPT; ಹೋಗು balance_cleanup; }
        szNew[k-1] = 0;
        cntNew[k-1] = b.nCell;
      }
      sz = 2 + cached_Cell_ಗಾತ್ರ(&b, cntNew[i]-1);
      szNew[i] -= sz;
      ಆದರೆ( !ಎಲೆ_ದತ್ತಾಂಶ ){
        ಆದರೆ( cntNew[i]<b.nCell ){
          sz = 2 + cached_Cell_ಗಾತ್ರ(&b, cntNew[i]);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sz = 0;
        }
      }
      szNew[i+1] += sz;
      cntNew[i]--;
    }
    ಆಗ( cntNew[i]<b.nCell ){
      sz = 2 + cached_Cell_ಗಾತ್ರ(&b, cntNew[i]);
      ಆದರೆ( szNew[i]+sz>usableSpace ) ನಿಲ್ಲಿಸು;
      szNew[i] += sz;
      cntNew[i]++;
      ಆದರೆ( !ಎಲೆ_ದತ್ತಾಂಶ ){
        ಆದರೆ( cntNew[i]<b.nCell ){
          sz = 2 + cached_Cell_ಗಾತ್ರ(&b, cntNew[i]);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sz = 0;
        }
      }
      szNew[i+1] -= sz;
    }
    ಆದರೆ( cntNew[i]>=b.nCell ){
      k = i+1;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( cntNew[i] <= (i>0 ? cntNew[i-1] : 0) ){
      rc = SQLITE_CORRUPT_BKPT;
      ಹೋಗು balance_cleanup;
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
  ಗಾಗಿ(i=k-1; i>0; i--){
    ಪೂರ್ಣಾಂಕ sz_ಬಲ = szNew[i];  /* Size of sibling on the right */
    ಪೂರ್ಣಾಂಕ sz_ಎಡ = szNew[i-1]; /* Size of sibling on the left */
    ಪೂರ್ಣಾಂಕ r;              /* Index of right-most cell in left sibling */
    ಪೂರ್ಣಾಂಕ d;              /* Index of first cell to the left of right sibling */

    r = cntNew[i-1] - 1;
    d = r + 1 - ಎಲೆ_ದತ್ತಾಂಶ;
    (ಶೂನ್ಯ)cached_Cell_ಗಾತ್ರ(&b, d);
    ಮಾಡು{
      ಪೂರ್ಣಾಂಕ szR, szD;
      ಪ್ರತಿಜ್ಞಾ( d<nMaxCells );
      ಪ್ರತಿಜ್ಞಾ( r<nMaxCells );
      szR = cached_Cell_ಗಾತ್ರ(&b, r);
      szD = b.szCell[d];
      ಆದರೆ( sz_ಬಲ!=0
       && (bBulk || sz_ಬಲ+szD+2 > sz_ಎಡ-(szR+(i==k-1?0:2)))){
        ನಿಲ್ಲಿಸು;
      }
      sz_ಬಲ += szD + 2;
      sz_ಎಡ -= szR + 2;
      cntNew[i-1] = r;
      r--;
      d--;
    }ಆಗ( r>=0 );
    szNew[i] = sz_ಬಲ;
    szNew[i-1] = sz_ಎಡ;
    ಆದರೆ( cntNew[i-1] <= (i>1 ? cntNew[i-2] : 0) ){
      rc = SQLITE_CORRUPT_BKPT;
      ಹೋಗು balance_cleanup;
    }
  }

  /* Sanity check:  For a non-corrupt database file one of the following
  ** must be true:
  **    (1) We found one or more cells (cntNew[0])>0), or
  **    (2) pPage is a virtual root page.  A virtual root page is when
  **        the real root page is page 1 and we are the only child of
  **        that page.
  */
  ಪ್ರತಿಜ್ಞಾ( cntNew[0]>0 || (p_ಪೋಷಕ->pgno==1 && p_ಪೋಷಕ->nCell==0) || CORRUPT_DB);
  TRACE(("BALANCE: old: %u(nc=%u) %u(nc=%u) %u(nc=%u)\n",
    apOld[0]->pgno, apOld[0]->nCell,
    nOld>=2 ? apOld[1]->pgno : 0, nOld>=2 ? apOld[1]->nCell : 0,
    nOld>=3 ? apOld[2]->pgno : 0, nOld>=3 ? apOld[2]->nCell : 0
  ));

  /*
  ** Allocate k new pages.  Reuse old pages where possible.
  */
  ಪುಟ_Flags = apOld[0]->a_ದತ್ತಾಂಶ[0];
  ಗಾಗಿ(i=0; i<k; i++){
    Mem_ಪುಟ *pNew;
    ಆದರೆ( i<nOld ){
      pNew = apNew[i] = apOld[i];
      apOld[i] = 0;
      rc = sqlite3PagerWrite(pNew->p_Db_ಪುಟ);
      nNew++;
      ಆದರೆ( sqlite3_Pager_ಪುಟ_Refcount(pNew->p_Db_ಪುಟ)!=1+(i==(i_ಪೋಷಕ_Idx-nxDiv))
       && rc==SQLITE_OK
      ){
        rc = SQLITE_CORRUPT_BKPT;
      }
      ಆದರೆ( rc ) ಹೋಗು balance_cleanup;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( i>0 );
      rc = allocate_Btree_ಪುಟ(pBt, &pNew, &pgno, (bBulk ? 1 : pgno), 0);
      ಆದರೆ( rc ) ಹೋಗು balance_cleanup;
      zero_ಪುಟ(pNew, ಪುಟ_Flags);
      apNew[i] = pNew;
      nNew++;
      cntOld[i] = b.nCell;

      /* Set the pointer-map entry for the new sibling page. */
      ಆದರೆ( ISAUTOVACUUM(pBt) ){
        ptrmapPut(pBt, pNew->pgno, PTRMAP_BTREE, p_ಪೋಷಕ->pgno, &rc);
        ಆದರೆ( rc!=SQLITE_OK ){
          ಹೋಗು balance_cleanup;
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
  ಗಾಗಿ(i=0; i<nNew; i++){
    aPgno[i] = apNew[i]->pgno;
    ಪ್ರತಿಜ್ಞಾ( apNew[i]->p_Db_ಪುಟ->flags & PGHDR_WRITEABLE );
    ಪ್ರತಿಜ್ಞಾ( apNew[i]->p_Db_ಪುಟ->flags & PGHDR_DIRTY );
  }
  ಗಾಗಿ(i=0; i<nNew-1; i++){
    ಪೂರ್ಣಾಂಕ iB = i;
    ಗಾಗಿ(j=i+1; j<nNew; j++){
      ಆದರೆ( apNew[j]->pgno < apNew[iB]->pgno ) iB = j;
    }

    /* If apNew[i] has a page number that is bigger than any of the
    ** subsequence apNew[i] entries, then swap apNew[i] with the subsequent
    ** entry that has the smallest page number (which we know to be
    ** entry apNew[iB]).
    */
    ಆದರೆ( iB!=i ){
      Pgno pgnoA = apNew[i]->pgno;
      Pgno pgnoB = apNew[iB]->pgno;
      Pgno pgno_ತಾತ್ಕಾಲಿಕ = (PENDING_BYTE/pBt->ಪುಟ_ಗಾತ್ರ)+1;
      u16 fgA = apNew[i]->p_Db_ಪುಟ->flags;
      u16 fgB = apNew[iB]->p_Db_ಪುಟ->flags;
      sqlite3PagerRekey(apNew[i]->p_Db_ಪುಟ, pgno_ತಾತ್ಕಾಲಿಕ, fgB);
      sqlite3PagerRekey(apNew[iB]->p_Db_ಪುಟ, pgnoA, fgA);
      sqlite3PagerRekey(apNew[i]->p_Db_ಪುಟ, pgnoB, fgB);
      apNew[i]->pgno = pgnoB;
      apNew[iB]->pgno = pgnoA;
    }
  }

  TRACE(("BALANCE: new: %u(%u nc=%u) %u(%u nc=%u) %u(%u nc=%u) "
         "%u(%u nc=%u) %u(%u nc=%u)\n",
    apNew[0]->pgno, szNew[0], cntNew[0],
    nNew>=2 ? apNew[1]->pgno : 0, nNew>=2 ? szNew[1] : 0,
    nNew>=2 ? cntNew[1] - cntNew[0] - !ಎಲೆ_ದತ್ತಾಂಶ : 0,
    nNew>=3 ? apNew[2]->pgno : 0, nNew>=3 ? szNew[2] : 0,
    nNew>=3 ? cntNew[2] - cntNew[1] - !ಎಲೆ_ದತ್ತಾಂಶ : 0,
    nNew>=4 ? apNew[3]->pgno : 0, nNew>=4 ? szNew[3] : 0,
    nNew>=4 ? cntNew[3] - cntNew[2] - !ಎಲೆ_ದತ್ತಾಂಶ : 0,
    nNew>=5 ? apNew[4]->pgno : 0, nNew>=5 ? szNew[4] : 0,
    nNew>=5 ? cntNew[4] - cntNew[3] - !ಎಲೆ_ದತ್ತಾಂಶ : 0
  ));

  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಪೋಷಕ->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( nNew>=1 && nNew<=Array_ಗಾತ್ರ(apNew) );
  ಪ್ರತಿಜ್ಞಾ( apNew[nNew-1]!=0 );
  put4byte(p_ಬಲ, apNew[nNew-1]->pgno);

  /* If the sibling pages are not leaves, ensure that the right-child pointer
  ** of the right-most new sibling page is set to the value that was
  ** originally in the same field of the right-most old sibling page. */
  ಆದರೆ( (ಪುಟ_Flags & PTF_ಎಲೆ)==0 && nOld!=nNew ){
    Mem_ಪುಟ *pOld;
    ಆದರೆ( nNew>nOld ){
      pOld = apNew[nOld-1];
    }ಇಲ್ಲದಿದ್ದರೆ{
      pOld = apOld[nOld-1];
    }
    ಸ್ಮರಣೆ_ನಕಲು(&apNew[nNew-1]->a_ದತ್ತಾಂಶ[8], &pOld->a_ದತ್ತಾಂಶ[8], 4);
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
  ಆದರೆ( ISAUTOVACUUM(pBt) ){
    Mem_ಪುಟ *pOld;
    Mem_ಪುಟ *pNew = pOld = apNew[0];
    ಪೂರ್ಣಾಂಕ cnt_Old_ಮುಂದಿನ = pNew->nCell + pNew->n_ಅತಿಪ್ರವಾಹ;
    ಪೂರ್ಣಾಂಕ iNew = 0;
    ಪೂರ್ಣಾಂಕ iOld = 0;

    ಗಾಗಿ(i=0; i<b.nCell; i++){
      u8 *pCell = b.apCell[i];
      ಆಗ( i==cnt_Old_ಮುಂದಿನ ){
        iOld++;
        ಪ್ರತಿಜ್ಞಾ( iOld<nNew || iOld<nOld );
        ಪ್ರತಿಜ್ಞಾ( iOld>=0 && iOld<NB );
        pOld = iOld<nNew ? apNew[iOld] : apOld[iOld];
        cnt_Old_ಮುಂದಿನ += pOld->nCell + pOld->n_ಅತಿಪ್ರವಾಹ + !ಎಲೆ_ದತ್ತಾಂಶ;
      }
      ಆದರೆ( i==cntNew[iNew] ){
        pNew = apNew[++iNew];
        ಆದರೆ( !ಎಲೆ_ದತ್ತಾಂಶ ) ಮುಂದುವರಿ;
      }

      /* Cell pCell is destined for new sibling page pNew. Originally, it
      ** was either part of sibling page iOld (possibly an overflow cell),
      ** or else the divider cell to the left of sibling page iOld. So,
      ** if sibling page iOld had the same page number as pNew, and if
      ** pCell really was a part of sibling page iOld (not a divider or
      ** overflow cell), we can skip updating the pointer map entries.  */
      ಆದರೆ( iOld>=nNew
       || pNew->pgno!=aPgno[iOld]
       || !SQLITE_WITHIN(pCell,pOld->a_ದತ್ತಾಂಶ,pOld->a_ದತ್ತಾಂಶ_ಕೊನೆ)
      ){
        ಆದರೆ( !ಎಲೆ_Correction ){
          ptrmapPut(pBt, get4byte(pCell), PTRMAP_BTREE, pNew->pgno, &rc);
        }
        ಆದರೆ( cached_Cell_ಗಾತ್ರ(&b,i)>pNew->minLocal ){
          ptrmapPutOvflPtr(pNew, pOld, pCell, &rc);
        }
        ಆದರೆ( rc ) ಹೋಗು balance_cleanup;
      }
    }
  }

  /* Insert new divider cells into pParent. */
  ಗಾಗಿ(i=0; i<nNew-1; i++){
    u8 *pCell;
    u8 *p_ತಾತ್ಕಾಲಿಕ;
    ಪೂರ್ಣಾಂಕ sz;
    u8 *p_Src_ಕೊನೆ;
    Mem_ಪುಟ *pNew = apNew[i];
    j = cntNew[i];

    ಪ್ರತಿಜ್ಞಾ( j<nMaxCells );
    ಪ್ರತಿಜ್ಞಾ( b.apCell[j]!=0 );
    pCell = b.apCell[j];
    sz = b.szCell[j] + ಎಲೆ_Correction;
    p_ತಾತ್ಕಾಲಿಕ = &aOvflSpace[iOvflSpace];
    ಆದರೆ( !pNew->ಎಲೆ ){
      ಸ್ಮರಣೆ_ನಕಲು(&pNew->a_ದತ್ತಾಂಶ[8], pCell, 4);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಎಲೆ_ದತ್ತಾಂಶ ){
      /* If the tree is a leaf-data tree, and the siblings are leaves,
      ** then there is no divider cell in b.apCell[]. Instead, the divider
      ** cell consists of the integer key for the right-most cell of
      ** the sibling-page assembled above only.
      */
      CellInfo info;
      j--;
      pNew->xParseCell(pNew, b.apCell[j], &info);
      pCell = p_ತಾತ್ಕಾಲಿಕ;
      sz = 4 + sqlite3PutVarint(&pCell[4], info.n_ಕೀಲಿ);
      p_ತಾತ್ಕಾಲಿಕ = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
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
      ಆದರೆ( b.szCell[j]==4 ){
        ಪ್ರತಿಜ್ಞಾ(ಎಲೆ_Correction==4);
        sz = p_ಪೋಷಕ->x_Cell_ಗಾತ್ರ(p_ಪೋಷಕ, pCell);
      }
    }
    iOvflSpace += sz;
    ಪ್ರತಿಜ್ಞಾ( sz<=pBt->maxLocal+23 );
    ಪ್ರತಿಜ್ಞಾ( iOvflSpace <= (ಪೂರ್ಣಾಂಕ)pBt->ಪುಟ_ಗಾತ್ರ );
    ಪ್ರತಿಜ್ಞಾ( b.ixNx[NB*2-1]>j );
    ಗಾಗಿ(k=0; b.ixNx[k]<=j; k++){}
    p_Src_ಕೊನೆ = b.ap_ಕೊನೆ[k];
    ಆದರೆ( SQLITE_ಅತಿಪ್ರವಾಹ(p_Src_ಕೊನೆ, pCell, pCell+sz) ){
      rc = SQLITE_CORRUPT_BKPT;
      ಹೋಗು balance_cleanup;
    }
    rc = ಸೇರಿಸು_Cell(p_ಪೋಷಕ, nxDiv+i, pCell, sz, p_ತಾತ್ಕಾಲಿಕ, pNew->pgno);
    ಆದರೆ( rc!=SQLITE_OK ) ಹೋಗು balance_cleanup;
    ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಪೋಷಕ->p_Db_ಪುಟ) );
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
  ಗಾಗಿ(i=1-nNew; i<nNew; i++){
    ಪೂರ್ಣಾಂಕ iPg = i<0 ? -i : i;
    ಪ್ರತಿಜ್ಞಾ( iPg>=0 && iPg<nNew );
    ಪ್ರತಿಜ್ಞಾ( iPg>=1 || i>=0 );
    ಪ್ರತಿಜ್ಞಾ( iPg<Array_ಗಾತ್ರ(cntOld) );
    ಆದರೆ( ab_ಮುಗಿದ[iPg] ) ಮುಂದುವರಿ;         /* Skip pages already processed */
    ಆದರೆ( i>=0                            /* On the upwards pass, or... */
     || cntOld[iPg-1]>=cntNew[iPg-1]    /* Condition (1) is true */
    ){
      ಪೂರ್ಣಾಂಕ iNew;
      ಪೂರ್ಣಾಂಕ iOld;
      ಪೂರ್ಣಾಂಕ nNewCell;

      /* Verify condition (1):  If cells are moving left, update iPg
      ** only after iPg-1 has already been updated. */
      ಪ್ರತಿಜ್ಞಾ( iPg==0 || cntOld[iPg-1]>=cntNew[iPg-1] || ab_ಮುಗಿದ[iPg-1] );

      /* Verify condition (2):  If cells are moving right, update iPg
      ** only after iPg+1 has already been updated. */
      ಪ್ರತಿಜ್ಞಾ( cntNew[iPg]>=cntOld[iPg] || ab_ಮುಗಿದ[iPg+1] );

      ಆದರೆ( iPg==0 ){
        iNew = iOld = 0;
        nNewCell = cntNew[0];
      }ಇಲ್ಲದಿದ್ದರೆ{
        iOld = iPg<nOld ? (cntOld[iPg-1] + !ಎಲೆ_ದತ್ತಾಂಶ) : b.nCell;
        iNew = cntNew[iPg-1] + !ಎಲೆ_ದತ್ತಾಂಶ;
        nNewCell = cntNew[iPg] - iNew;
      }

      rc = edit_ಪುಟ(apNew[iPg], iOld, iNew, nNewCell, &b);
      ಆದರೆ( rc ) ಹೋಗು balance_cleanup;
      ab_ಮುಗಿದ[iPg]++;
      apNew[iPg]->nFree = usableSpace-szNew[iPg];
      ಪ್ರತಿಜ್ಞಾ( apNew[iPg]->n_ಅತಿಪ್ರವಾಹ==0 );
      ಪ್ರತಿಜ್ಞಾ( apNew[iPg]->nCell==nNewCell );
    }
  }

  /* All pages have been processed exactly once */
  ಪ್ರತಿಜ್ಞಾ( memcmp(ab_ಮುಗಿದ, "\01\01\01\01\01", nNew)==0 );

  ಪ್ರತಿಜ್ಞಾ( nOld>0 );
  ಪ್ರತಿಜ್ಞಾ( nNew>0 );

  ಆದರೆ( is_ಬೇರು && p_ಪೋಷಕ->nCell==0 && p_ಪೋಷಕ->hdr_ಸ್ಥಾನಾಂತರ<=apNew[0]->nFree ){
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
    ಪ್ರತಿಜ್ಞಾ( nNew==1 || CORRUPT_DB );
    rc = defragment_ಪುಟ(apNew[0], -1);
    testcase( rc!=SQLITE_OK );
    ಪ್ರತಿಜ್ಞಾ( apNew[0]->nFree ==
        (get2byteNotZero(&apNew[0]->a_ದತ್ತಾಂಶ[5]) - apNew[0]->cell_ಸ್ಥಾನಾಂತರ
          - apNew[0]->nCell*2)
      || rc!=SQLITE_OK
    );
    copy_ಗಂಟು_Content(apNew[0], p_ಪೋಷಕ, &rc);
    free_ಪುಟ(apNew[0], &rc);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ISAUTOVACUUM(pBt) && !ಎಲೆ_Correction ){
    /* Fix the pointer map entries associated with the right-child of each
    ** sibling page. All other pointer map entries have already been taken
    ** care of.  */
    ಗಾಗಿ(i=0; i<nNew; i++){
      u32 ಕೀಲಿ = get4byte(&apNew[i]->a_ದತ್ತಾಂಶ[8]);
      ptrmapPut(pBt, ಕೀಲಿ, PTRMAP_BTREE, apNew[i]->pgno, &rc);
    }
  }

  ಪ್ರತಿಜ್ಞಾ( p_ಪೋಷಕ->isInit );
  TRACE(("BALANCE: finished: old=%u new=%u cells=%u\n",
          nOld, nNew, b.nCell));

  /* Free any old pages that were not reused as new pages.
  */
  ಗಾಗಿ(i=nNew; i<nOld; i++){
    free_ಪುಟ(apOld[i], &rc);
  }

#ಆದರೆ 0
  ಆದರೆ( ISAUTOVACUUM(pBt) && rc==SQLITE_OK && apNew[0]->isInit ){
    /* The ptrmapCheckPages() contains assert() statements that verify that
    ** all pointer map pages are set correctly. This is helpful while
    ** debugging. This is usually disabled because a corrupt database may
    ** cause an assert() statement to fail.  */
    ptrmap_Check_ಪುಟಗಳು(apNew, nNew);
    ptrmap_Check_ಪುಟಗಳು(&p_ಪೋಷಕ, 1);
  }
#ಆದರೆ_ಅಂತ್ಯ

  /*
  ** Cleanup before returning.
  */
balance_cleanup:
  sqlite3_ರಾಶಿ_Free(0, b.apCell);
  ಗಾಗಿ(i=0; i<nOld; i++){
    release_ಪುಟ(apOld[i]);
  }
  ಗಾಗಿ(i=0; i<nNew; i++){
    release_ಪುಟ(apNew[i]);
  }

  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ balance_deeper(Mem_ಪುಟ *p_ಬೇರು, Mem_ಪುಟ **pp_ಮಗು){
  ಪೂರ್ಣಾಂಕ rc;                        /* Return value from subprocedures */
  Mem_ಪುಟ *p_ಮಗು = 0;           /* Pointer to a new child page */
  Pgno pgno_ಮಗು = 0;            /* Page number of the new child page */
  BtShared *pBt = p_ಬೇರು->pBt;    /* The BTree */

  ಪ್ರತಿಜ್ಞಾ( p_ಬೇರು->n_ಅತಿಪ್ರವಾಹ>0 );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );

  /* Make pRoot, the root page of the b-tree, writable. Allocate a new
  ** page that will become the new right-child of pPage. Copy the contents
  ** of the node stored on pRoot into the new child page.
  */
  rc = sqlite3PagerWrite(p_ಬೇರು->p_Db_ಪುಟ);
  ಆದರೆ( rc==SQLITE_OK ){
    rc = allocate_Btree_ಪುಟ(pBt,&p_ಮಗು,&pgno_ಮಗು,p_ಬೇರು->pgno,0);
    copy_ಗಂಟು_Content(p_ಬೇರು, p_ಮಗು, &rc);
    ಆದರೆ( ISAUTOVACUUM(pBt) ){
      ptrmapPut(pBt, pgno_ಮಗು, PTRMAP_BTREE, p_ಬೇರು->pgno, &rc);
    }
  }
  ಆದರೆ( rc ){
    *pp_ಮಗು = 0;
    release_ಪುಟ(p_ಮಗು);
    ಹಿಂತಿರುಗಿ rc;
  }
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಮಗು->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಬೇರು->p_Db_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( p_ಮಗು->nCell==p_ಬೇರು->nCell || CORRUPT_DB );

  TRACE(("BALANCE: copy root %u into %u\n", p_ಬೇರು->pgno, p_ಮಗು->pgno));

  /* Copy the overflow cells from pRoot to pChild */
  ಸ್ಮರಣೆ_ನಕಲು(p_ಮಗು->aiOvfl, p_ಬೇರು->aiOvfl,
         p_ಬೇರು->n_ಅತಿಪ್ರವಾಹ*ಗಾತ್ರ(p_ಬೇರು->aiOvfl[0]));
  ಸ್ಮರಣೆ_ನಕಲು(p_ಮಗು->apOvfl, p_ಬೇರು->apOvfl,
         p_ಬೇರು->n_ಅತಿಪ್ರವಾಹ*ಗಾತ್ರ(p_ಬೇರು->apOvfl[0]));
  p_ಮಗು->n_ಅತಿಪ್ರವಾಹ = p_ಬೇರು->n_ಅತಿಪ್ರವಾಹ;

  /* Zero the contents of pRoot. Then install pChild as the right-child. */
  zero_ಪುಟ(p_ಬೇರು, p_ಮಗು->a_ದತ್ತಾಂಶ[0] & ~PTF_ಎಲೆ);
  put4byte(&p_ಬೇರು->a_ದತ್ತಾಂಶ[p_ಬೇರು->hdr_ಸ್ಥಾನಾಂತರ+8], pgno_ಮಗು);

  *pp_ಮಗು = p_ಮಗು;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ anotherValidCursor(BtCursor *pCur){
  BtCursor *pOther;
  ಗಾಗಿ(pOther=pCur->pBt->pCursor; pOther; pOther=pOther->p_ಮುಂದಿನ){
    ಆದರೆ( pOther!=pCur
     && pOther->eState==CURSOR_VALID
     && pOther->p_ಪುಟ==pCur->p_ಪುಟ
    ){
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(pCur->p_ಪುಟ);
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ balance(BtCursor *pCur){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  u8 a_Balance_ಶೀಘ್ರ_Space[13];
  u8 *pFree = 0;

  VVA_ONLY( ಪೂರ್ಣಾಂಕ balance_ಶೀಘ್ರ_called = 0 );
  VVA_ONLY( ಪೂರ್ಣಾಂಕ balance_deeper_called = 0 );

  ಮಾಡು {
    ಪೂರ್ಣಾಂಕ i_ಪುಟ;
    Mem_ಪುಟ *p_ಪುಟ = pCur->p_ಪುಟ;

    ಆದರೆ( NEVER(p_ಪುಟ->nFree<0) && btreeComputeFreeSpace(p_ಪುಟ) ) ನಿಲ್ಲಿಸು;
    ಆದರೆ( p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ==0 && p_ಪುಟ->nFree*3<=(ಪೂರ್ಣಾಂಕ)pCur->pBt->usable_ಗಾತ್ರ*2 ){
      /* No rebalance required as long as:
      **   (1) There are no overflow cells
      **   (2) The amount of free space on the page is less than 2/3rds of
      **       the total usable space on the page. */
      ನಿಲ್ಲಿಸು;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (i_ಪುಟ = pCur->i_ಪುಟ)==0 ){
      ಆದರೆ( p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ && (rc = anotherValidCursor(pCur))==SQLITE_OK ){
        /* The root page of the b-tree is overfull. In this case call the
        ** balance_deeper() function to create a new child for the root-page
        ** and copy the current contents of the root-page to it. The
        ** next iteration of the do-loop will balance the child page.
        */
        ಪ್ರತಿಜ್ಞಾ( balance_deeper_called==0 );
        VVA_ONLY( balance_deeper_called++ );
        rc = balance_deeper(p_ಪುಟ, &pCur->ap_ಪುಟ[1]);
        ಆದರೆ( rc==SQLITE_OK ){
          pCur->i_ಪುಟ = 1;
          pCur->ix = 0;
          pCur->aiIdx[0] = 0;
          pCur->ap_ಪುಟ[0] = p_ಪುಟ;
          pCur->p_ಪುಟ = pCur->ap_ಪುಟ[1];
          ಪ್ರತಿಜ್ಞಾ( pCur->p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ );
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ನಿಲ್ಲಿಸು;
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3_Pager_ಪುಟ_Refcount(p_ಪುಟ->p_Db_ಪುಟ)>1 ){
      /* The page being written is not a root page, and there is currently
      ** more than one reference to it. This only happens if the page is one
      ** of its own ancestor pages. Corruption. */
      rc = SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      Mem_ಪುಟ * ಸ್ಥಿರಾಂಕ p_ಪೋಷಕ = pCur->ap_ಪುಟ[i_ಪುಟ-1];
      ಪೂರ್ಣಾಂಕ ಸ್ಥಿರಾಂಕ iIdx = pCur->aiIdx[i_ಪುಟ-1];

      rc = sqlite3PagerWrite(p_ಪೋಷಕ->p_Db_ಪುಟ);
      ಆದರೆ( rc==SQLITE_OK && p_ಪೋಷಕ->nFree<0 ){
        rc = btreeComputeFreeSpace(p_ಪೋಷಕ);
      }
      ಆದರೆ( rc==SQLITE_OK ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_QUICKBALANCE
        ಆದರೆ( p_ಪುಟ->int_ಕೀಲಿ_ಎಲೆ
         && p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ==1
         && p_ಪುಟ->aiOvfl[0]==p_ಪುಟ->nCell
         && p_ಪೋಷಕ->pgno!=1
         && p_ಪೋಷಕ->nCell==iIdx
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
          ಪ್ರತಿಜ್ಞಾ( balance_ಶೀಘ್ರ_called==0 );
          VVA_ONLY( balance_ಶೀಘ್ರ_called++ );
          rc = balance_ಶೀಘ್ರ(p_ಪೋಷಕ, p_ಪುಟ, a_Balance_ಶೀಘ್ರ_Space);
        }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
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
          u8 *pSpace = sqlite3_ಪುಟ_Malloc(pCur->pBt->ಪುಟ_ಗಾತ್ರ);
          rc = balance_nonroot(p_ಪೋಷಕ, iIdx, pSpace, i_ಪುಟ==1,
                               pCur->hints&BTREE_BULKLOAD);
          ಆದರೆ( pFree ){
            /* If pFree is not NULL, it points to the pSpace buffer used
            ** by a previous call to balance_nonroot(). Its contents are
            ** now stored either on real database pages or within the
            ** new pSpace buffer, so it may be safely freed here. */
            sqlite3_ಪುಟ_Free(pFree);
          }

          /* The pSpace buffer will be freed after the next call to
          ** balance_nonroot(), or just before this function returns, whichever
          ** comes first. */
          pFree = pSpace;
        }
      }

      p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ = 0;

      /* The next iteration of the do-loop balances the parent page. */
      release_ಪುಟ(p_ಪುಟ);
      pCur->i_ಪುಟ--;
      ಪ್ರತಿಜ್ಞಾ( pCur->i_ಪುಟ>=0 );
      pCur->p_ಪುಟ = pCur->ap_ಪುಟ[pCur->i_ಪುಟ];
    }
  }ಆಗ( rc==SQLITE_OK );

  ಆದರೆ( pFree ){
    sqlite3_ಪುಟ_Free(pFree);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/* Overwrite content from pX into pDest.  Only do the write if the
** content is different from what is already there.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btreeOverwriteContent(
  Mem_ಪುಟ *p_ಪುಟ,           /* MemPage on which writing will occur */
  u8 *pDest,                /* Pointer to the place to start writing */
  ಸ್ಥಿರಾಂಕ BtreePayload *pX,   /* Source of data to write */
  ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ,              /* Offset of first byte to write */
  ಪೂರ್ಣಾಂಕ iAmt                  /* Number of bytes to be written */
){
  ಪೂರ್ಣಾಂಕ n_ದತ್ತಾಂಶ = pX->n_ದತ್ತಾಂಶ - i_ಸ್ಥಾನಾಂತರ;
  ಆದರೆ( n_ದತ್ತಾಂಶ<=0 ){
    /* Overwriting with zeros */
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<iAmt && pDest[i]==0; i++){}
    ಆದರೆ( i<iAmt ){
      ಪೂರ್ಣಾಂಕ rc = sqlite3PagerWrite(p_ಪುಟ->p_Db_ಪುಟ);
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
      memset(pDest + i, 0, iAmt - i);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( n_ದತ್ತಾಂಶ<iAmt ){
      /* Mixed read data and zeros at the end.  Make a recursive call
      ** to write the zeros then fall through to write the real data */
      ಪೂರ್ಣಾಂಕ rc = btreeOverwriteContent(p_ಪುಟ, pDest+n_ದತ್ತಾಂಶ, pX, i_ಸ್ಥಾನಾಂತರ+n_ದತ್ತಾಂಶ,
                                 iAmt-n_ದತ್ತಾಂಶ);
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
      iAmt = n_ದತ್ತಾಂಶ;
    }
    ಆದರೆ( memcmp(pDest, ((u8*)pX->p_ದತ್ತಾಂಶ) + i_ಸ್ಥಾನಾಂತರ, iAmt)!=0 ){
      ಪೂರ್ಣಾಂಕ rc = sqlite3PagerWrite(p_ಪುಟ->p_Db_ಪುಟ);
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
      /* In a corrupt database, it is possible for the source and destination
      ** buffers to overlap.  This is harmless since the database is already
      ** corrupt but it does cause valgrind and ASAN warnings.  So use
      ** memmove(). */
      memmove(pDest, ((u8*)pX->p_ದತ್ತಾಂಶ) + i_ಸ್ಥಾನಾಂತರ, iAmt);
    }
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.  In this variant, pCur is pointing to an overflow
** cell.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ btree_Overwrite_ಅತಿಪ್ರವಾಹ_Cell(
  BtCursor *pCur,                     /* Cursor pointing to cell to overwrite */
  ಸ್ಥಿರಾಂಕ BtreePayload *pX              /* Content to write into the cell */
){
  ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ;                        /* Next byte of pX->pData to write */
  ಪೂರ್ಣಾಂಕ n_ಒಟ್ಟು = pX->n_ದತ್ತಾಂಶ + pX->nZero; /* Total bytes of to write */
  ಪೂರ್ಣಾಂಕ rc;                             /* Return code */
  Mem_ಪುಟ *p_ಪುಟ = pCur->p_ಪುಟ;       /* Page being written */
  BtShared *pBt;                      /* Btree */
  Pgno ovflPgno;                      /* Next overflow page to write */
  u32 ovfl_ಪುಟ_ಗಾತ್ರ;                   /* Size to write on overflow page */

  ಪ್ರತಿಜ್ಞಾ( pCur->info.nLocal<n_ಒಟ್ಟು );  /* pCur is an overflow cell */

  /* Overwrite the local portion first */
  rc = btreeOverwriteContent(p_ಪುಟ, pCur->info.pPayload, pX,
                             0, pCur->info.nLocal);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;

  /* Now overwrite the overflow pages */
  i_ಸ್ಥಾನಾಂತರ = pCur->info.nLocal;
  ಪ್ರತಿಜ್ಞಾ( n_ಒಟ್ಟು>=0 );
  ಪ್ರತಿಜ್ಞಾ( i_ಸ್ಥಾನಾಂತರ>=0 );
  ovflPgno = get4byte(pCur->info.pPayload + i_ಸ್ಥಾನಾಂತರ);
  pBt = p_ಪುಟ->pBt;
  ovfl_ಪುಟ_ಗಾತ್ರ = pBt->usable_ಗಾತ್ರ - 4;
  ಮಾಡು{
    rc = btree_Get_ಪುಟ(pBt, ovflPgno, &p_ಪುಟ, 0);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    ಆದರೆ( sqlite3_Pager_ಪುಟ_Refcount(p_ಪುಟ->p_Db_ಪುಟ)!=1 || p_ಪುಟ->isInit ){
      rc = SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( i_ಸ್ಥಾನಾಂತರ+ovfl_ಪುಟ_ಗಾತ್ರ<(u32)n_ಒಟ್ಟು ){
        ovflPgno = get4byte(p_ಪುಟ->a_ದತ್ತಾಂಶ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ovfl_ಪುಟ_ಗಾತ್ರ = n_ಒಟ್ಟು - i_ಸ್ಥಾನಾಂತರ;
      }
      rc = btreeOverwriteContent(p_ಪುಟ, p_ಪುಟ->a_ದತ್ತಾಂಶ+4, pX,
                                 i_ಸ್ಥಾನಾಂತರ, ovfl_ಪುಟ_ಗಾತ್ರ);
    }
    sqlite3PagerUnref(p_ಪುಟ->p_Db_ಪುಟ);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    i_ಸ್ಥಾನಾಂತರ += ovfl_ಪುಟ_ಗಾತ್ರ;
  }ಆಗ( i_ಸ್ಥಾನಾಂತರ<n_ಒಟ್ಟು );
  ಹಿಂತಿರುಗಿ SQLITE_OK;   
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btreeOverwriteCell(BtCursor *pCur, ಸ್ಥಿರಾಂಕ BtreePayload *pX){
  ಪೂರ್ಣಾಂಕ n_ಒಟ್ಟು = pX->n_ದತ್ತಾಂಶ + pX->nZero; /* Total bytes of to write */
  Mem_ಪುಟ *p_ಪುಟ = pCur->p_ಪುಟ;       /* Page being written */

  ಆದರೆ( pCur->info.pPayload + pCur->info.nLocal > p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ
   || pCur->info.pPayload < p_ಪುಟ->a_ದತ್ತಾಂಶ + p_ಪುಟ->cell_ಸ್ಥಾನಾಂತರ
  ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
  }
  ಆದರೆ( pCur->info.nLocal==n_ಒಟ್ಟು ){
    /* The entire cell is local */
    ಹಿಂತಿರುಗಿ btreeOverwriteContent(p_ಪುಟ, pCur->info.pPayload, pX,
                                 0, pCur->info.nLocal);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* The cell contains overflow content */
    ಹಿಂತಿರುಗಿ btree_Overwrite_ಅತಿಪ್ರವಾಹ_Cell(pCur, pX);
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
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಸೇರಿಸು(
  BtCursor *pCur,                /* Insert data into the table of this cursor */
  ಸ್ಥಿರಾಂಕ BtreePayload *pX,        /* Content of the row to be inserted */
  ಪೂರ್ಣಾಂಕ flags,                     /* True if this is likely an append */
  ಪೂರ್ಣಾಂಕ seek_ಫಲಿತಾಂಶ                 /* Result of prior IndexMoveto() call */
){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ loc = seek_ಫಲಿತಾಂಶ;          /* -1: before desired location  +1: after */
  ಪೂರ್ಣಾಂಕ szNew = 0;
  ಪೂರ್ಣಾಂಕ idx;
  Mem_ಪುಟ *p_ಪುಟ;
  Btree *p = pCur->pBtree;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *oldCell;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *newCell = 0;

  ಪ್ರತಿಜ್ಞಾ( (flags & (BTREE_SAVEPOSITION|BTREE_APPEND|BTREE_PREFORMAT))==flags );
  ಪ್ರತಿಜ್ಞಾ( (flags & BTREE_PREFORMAT)==0 || seek_ಫಲಿತಾಂಶ || pCur->p_ಕೀಲಿ_Info==0 );

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
  ಆದರೆ( pCur->curFlags & BTCF_ಅಪವರ್ತ್ಯ ){
    rc = saveAllCursors(p->pBt, pCur->pgno_ಬೇರು, pCur);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    ಆದರೆ( loc && pCur->i_ಪುಟ<0 ){
      /* This can only happen if the schema is corrupt such that there is more
      ** than one table or index with the same root page as used by the cursor.
      ** Which can only happen if the SQLITE_NoSchemaError flag was set when
      ** the schema was loaded. This cannot be asserted though, as a user might
      ** set the flag, load the schema, and then unset the flag.  */
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_PGNO(pCur->pgno_ಬೇರು);
    }
  }

  /* Ensure that the cursor is not in the CURSOR_FAULT state and that it
  ** points to a valid cell.
  */
  ಆದರೆ( pCur->eState>=CURSOR_REQUIRESEEK ){
    testcase( pCur->eState==CURSOR_REQUIRESEEK );
    testcase( pCur->eState==CURSOR_FAULT );
    rc = move_ಗಮ್ಯ_ಬೇರು(pCur);
    ಆದರೆ( rc && rc!=SQLITE_ಖಾಲಿಗಳು ) ಹಿಂತಿರುಗಿ rc;
  }

  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( (pCur->curFlags & BTCF_WriteFlag)!=0
              && p->pBt->ಒಳ_Transaction==TRANS_WRITE
              && (p->pBt->btsFlags & BTS_READ_ONLY)==0 );
  ಪ್ರತಿಜ್ಞಾ( has_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(p, pCur->pgno_ಬೇರು, pCur->p_ಕೀಲಿ_Info!=0, 2) );

  /* Assert that the caller has been consistent. If this cursor was opened
  ** expecting an index b-tree, then the caller should be inserting blob
  ** keys with no associated data. If the cursor was opened expecting an
  ** intkey table, the caller should be inserting integer keys with a
  ** blob of associated data.  */
  ಪ್ರತಿಜ್ಞಾ( (flags & BTREE_PREFORMAT) || (pX->p_ಕೀಲಿ==0)==(pCur->p_ಕೀಲಿ_Info==0) );

  ಆದರೆ( pCur->p_ಕೀಲಿ_Info==0 ){
    ಪ್ರತಿಜ್ಞಾ( pX->p_ಕೀಲಿ==0 );
    /* If this is an insert into a table b-tree, invalidate any incrblob
    ** cursors open on the row being replaced */
    ಆದರೆ( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, pCur->pgno_ಬೇರು, pX->n_ಕೀಲಿ, 0);
    }

    /* If BTREE_SAVEPOSITION is set, the cursor must already be pointing
    ** to a row with the same key as the new entry being inserted.
    */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    ಆದರೆ( flags & BTREE_SAVEPOSITION ){
      ಪ್ರತಿಜ್ಞಾ( pCur->curFlags & BTCF_ValidNKey );
      ಪ್ರತಿಜ್ಞಾ( pX->n_ಕೀಲಿ==pCur->info.n_ಕೀಲಿ );
      ಪ್ರತಿಜ್ಞಾ( loc==0 );
    }
#ಆದರೆ_ಅಂತ್ಯ

    /* On the other hand, BTREE_SAVEPOSITION==0 does not imply
    ** that the cursor is not pointing to a row to be overwritten.
    ** So do a complete check.
    */
    ಆದರೆ( (pCur->curFlags&BTCF_ValidNKey)!=0 && pX->n_ಕೀಲಿ==pCur->info.n_ಕೀಲಿ ){
      /* The cursor is pointing to the entry that is to be
      ** overwritten */
      ಪ್ರತಿಜ್ಞಾ( pX->n_ದತ್ತಾಂಶ>=0 && pX->nZero>=0 );
      ಆದರೆ( pCur->info.n_ಗಾತ್ರ!=0
       && pCur->info.nPayload==(u32)pX->n_ದತ್ತಾಂಶ+pX->nZero
      ){
        /* New entry is the same size as the old.  Do an overwrite */
        ಹಿಂತಿರುಗಿ btreeOverwriteCell(pCur, pX);
      }
      ಪ್ರತಿಜ್ಞಾ( loc==0 );
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( loc==0 ){
      /* The cursor is *not* pointing to the cell to be overwritten, nor
      ** to an adjacent cell.  Move the cursor so that it is pointing either
      ** to the cell to be overwritten or an adjacent cell.
      */
      rc = sqlite3_Btree_ಕೋಷ್ಟಕ_Moveto(pCur, pX->n_ಕೀಲಿ,
               (flags & BTREE_APPEND)!=0, &loc);
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* This is an index or a WITHOUT ROWID table */

    /* If BTREE_SAVEPOSITION is set, the cursor must already be pointing
    ** to a row with the same key as the new entry being inserted.
    */
    ಪ್ರತಿಜ್ಞಾ( (flags & BTREE_SAVEPOSITION)==0 || loc==0 );

    /* If the cursor is not already pointing either to the cell to be
    ** overwritten, or if a new cell is being inserted, if the cursor is
    ** not pointing to an immediately adjacent cell, then move the cursor
    ** so that it does.
    */
    ಆದರೆ( loc==0 && (flags & BTREE_SAVEPOSITION)==0 ){
      ಆದರೆ( pX->nMem ){
        Unpacked_ದಾಖಲೆ r;
        r.p_ಕೀಲಿ_Info = pCur->p_ಕೀಲಿ_Info;
        r.aMem = pX->aMem;
        r.n_ಕ್ಷೇತ್ರ = pX->nMem;
        r.default_rc = 0;
        r.eq_ನೋಡಿದ = 0;
        rc = sqlite3_Btree_ಸೂಚ್ಯಂಕ_Moveto(pCur, &r, &loc);
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = btreeMoveto(pCur, pX->p_ಕೀಲಿ, pX->n_ಕೀಲಿ,
                    (flags & BTREE_APPEND)!=0, &loc);
      }
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    }

    /* If the cursor is currently pointing to an entry to be overwritten
    ** and the new content is the same as as the old, then use the
    ** overwrite optimization.
    */
    ಆದರೆ( loc==0 ){
      getCellInfo(pCur);
      ಆದರೆ( pCur->info.n_ಕೀಲಿ==pX->n_ಕೀಲಿ ){
        BtreePayload x2;
        x2.p_ದತ್ತಾಂಶ = pX->p_ಕೀಲಿ;
        x2.n_ದತ್ತಾಂಶ = (ಪೂರ್ಣಾಂಕ)pX->n_ಕೀಲಿ;  ಪ್ರತಿಜ್ಞಾ( pX->n_ಕೀಲಿ<=0x7fffffff );
        x2.nZero = 0;
        ಹಿಂತಿರುಗಿ btreeOverwriteCell(pCur, &x2);
      }
    }
  }
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID
       || (pCur->eState==CURSOR_INVALID && loc) || CORRUPT_DB );

  p_ಪುಟ = pCur->p_ಪುಟ;
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->int_ಕೀಲಿ || pX->n_ಕೀಲಿ>=0 || (flags & BTREE_PREFORMAT) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->ಎಲೆ || !p_ಪುಟ->int_ಕೀಲಿ );
  ಆದರೆ( p_ಪುಟ->nFree<0 ){
    ಆದರೆ( NEVER(pCur->eState>CURSOR_INVALID) ){
     /* ^^^^^--- due to the moveToRoot() call above */
      rc = SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = btreeComputeFreeSpace(p_ಪುಟ);
    }
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
  }

  TRACE(("INSERT: table=%u nkey=%lld ndata=%u page=%u %s\n",
          pCur->pgno_ಬೇರು, pX->n_ಕೀಲಿ, pX->n_ದತ್ತಾಂಶ, p_ಪುಟ->pgno,
          loc==0 ? "overwrite" : "new entry"));
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->isInit || CORRUPT_DB );
  newCell = p->pBt->pTmpSpace;
  ಪ್ರತಿಜ್ಞಾ( newCell!=0 );
  ಪ್ರತಿಜ್ಞಾ( BTREE_PREFORMAT==OPFLAG_PREFORMAT );
  ಆದರೆ( flags & BTREE_PREFORMAT ){
    rc = SQLITE_OK;
    szNew = p->pBt->n_Preformat_ಗಾತ್ರ;
    ಆದರೆ( szNew<4 ){
      szNew = 4;
      newCell[3] = 0;
    }
    ಆದರೆ( ISAUTOVACUUM(p->pBt) && szNew>p_ಪುಟ->maxLocal ){
      CellInfo info;
      p_ಪುಟ->xParseCell(p_ಪುಟ, newCell, &info);
      ಆದರೆ( info.nPayload!=info.nLocal ){
        Pgno ovfl = get4byte(&newCell[szNew-4]);
        ptrmapPut(p->pBt, ovfl, PTRMAP_OVERFLOW1, p_ಪುಟ->pgno, &rc);
        ಆದರೆ( NEVER(rc) ) ಹೋಗು ಕೊನೆ_ಸೇರಿಸು;
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = fill_ಒಳ_Cell(p_ಪುಟ, newCell, pX, &szNew);
    ಆದರೆ( rc ) ಹೋಗು ಕೊನೆ_ಸೇರಿಸು;
  }
  ಪ್ರತಿಜ್ಞಾ( szNew==p_ಪುಟ->x_Cell_ಗಾತ್ರ(p_ಪುಟ, newCell) );
  ಪ್ರತಿಜ್ಞಾ( szNew <= MX_CELL_ಗಾತ್ರ(p->pBt) );
  idx = pCur->ix;
  pCur->info.n_ಗಾತ್ರ = 0;
  ಆದರೆ( loc==0 ){
    CellInfo info;
    ಪ್ರತಿಜ್ಞಾ( idx>=0 );
    ಆದರೆ( idx>=p_ಪುಟ->nCell ){
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    }
    rc = sqlite3PagerWrite(p_ಪುಟ->p_Db_ಪುಟ);
    ಆದರೆ( rc ){
      ಹೋಗು ಕೊನೆ_ಸೇರಿಸು;
    }
    oldCell = ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, idx);
    ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
      ಸ್ಮರಣೆ_ನಕಲು(newCell, oldCell, 4);
    }
    BTREE_CLEAR_CELL(rc, p_ಪುಟ, oldCell, info);
    testcase( pCur->curFlags & BTCF_ValidOvfl );
    invalidate_ಅತಿಪ್ರವಾಹ_Cache(pCur);
    ಆದರೆ( info.n_ಗಾತ್ರ==szNew && info.nLocal==info.nPayload
     && (!ISAUTOVACUUM(p->pBt) || szNew<p_ಪುಟ->minLocal)
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
      ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK ); /* clearCell never fails when nLocal==nPayload */
      ಆದರೆ( oldCell < p_ಪುಟ->a_ದತ್ತಾಂಶ+p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+10 ){
        ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
      }
      ಆದರೆ( oldCell+szNew > p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ ){
        ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
      }
      ಸ್ಮರಣೆ_ನಕಲು(oldCell, newCell, szNew);
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
    dropCell(p_ಪುಟ, idx, info.n_ಗಾತ್ರ, &rc);
    ಆದರೆ( rc ) ಹೋಗು ಕೊನೆ_ಸೇರಿಸು;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( loc<0 && p_ಪುಟ->nCell>0 ){
    ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->ಎಲೆ );
    idx = ++pCur->ix;
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->ಎಲೆ );
  }
  rc = ಸೇರಿಸು_Cell_Fast(p_ಪುಟ, idx, newCell, szNew);
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ==0 || rc==SQLITE_OK );
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || p_ಪುಟ->nCell>0 || p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ>0 );

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
  ಆದರೆ( p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ ){
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
    rc = balance(pCur);

    /* Must make sure nOverflow is reset to zero even if the balance()
    ** fails. Internal data structure corruption will result otherwise.
    ** Also, set the cursor state to invalid. This stops saveCursorPosition()
    ** from trying to save the current position of the cursor.  */
    pCur->p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ = 0;
    pCur->eState = CURSOR_INVALID;
    ಆದರೆ( (flags & BTREE_SAVEPOSITION) && rc==SQLITE_OK ){
      btree_Release_All_Cursor_ಪುಟಗಳು(pCur);
      ಆದರೆ( pCur->p_ಕೀಲಿ_Info ){
        ಪ್ರತಿಜ್ಞಾ( pCur->p_ಕೀಲಿ==0 );
        pCur->p_ಕೀಲಿ = sqlite3Malloc( pX->n_ಕೀಲಿ );
        ಆದರೆ( pCur->p_ಕೀಲಿ==0 ){
          rc = SQLITE_NOMEM;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಸ್ಮರಣೆ_ನಕಲು(pCur->p_ಕೀಲಿ, pX->p_ಕೀಲಿ, pX->n_ಕೀಲಿ);
        }
      }
      pCur->eState = CURSOR_REQUIRESEEK;
      pCur->n_ಕೀಲಿ = pX->n_ಕೀಲಿ;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( pCur->i_ಪುಟ<0 || pCur->p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ==0 );

ಕೊನೆ_ಸೇರಿಸು:
  ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Transfer_ಅಡ್ಡಸಾಲು(BtCursor *pDest, BtCursor *pSrc, i64 i_ಕೀಲಿ){
  BtShared *pBt = pDest->pBt;
  u8 *a_ಹೊರಹೋಗು = pBt->pTmpSpace;    /* Pointer to next output buffer */
  ಸ್ಥಿರಾಂಕ u8 *a_ಒಳ;                /* Pointer to next input buffer */
  u32 n_ಒಳ;                      /* Size of input buffer aIn[] */
  u32 nRem;                     /* Bytes of data still to copy */

  getCellInfo(pSrc);
  ಆದರೆ( pSrc->info.nPayload<0x80 ){
    *(a_ಹೊರಹೋಗು++) = (u8)pSrc->info.nPayload;
  }ಇಲ್ಲದಿದ್ದರೆ{
    a_ಹೊರಹೋಗು += sqlite3PutVarint(a_ಹೊರಹೋಗು, pSrc->info.nPayload);
  }
  ಆದರೆ( pDest->p_ಕೀಲಿ_Info==0 ) a_ಹೊರಹೋಗು += sqlite3PutVarint(a_ಹೊರಹೋಗು, i_ಕೀಲಿ);
  n_ಒಳ = pSrc->info.nLocal;
  a_ಒಳ = pSrc->info.pPayload;
  ಆದರೆ( a_ಒಳ+n_ಒಳ>pSrc->p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(pSrc->p_ಪುಟ);
  }
  nRem = pSrc->info.nPayload;
  ಆದರೆ( n_ಒಳ==nRem && n_ಒಳ<pDest->p_ಪುಟ->maxLocal ){
    ಸ್ಮರಣೆ_ನಕಲು(a_ಹೊರಹೋಗು, a_ಒಳ, n_ಒಳ);
    pBt->n_Preformat_ಗಾತ್ರ = n_ಒಳ + (ಪೂರ್ಣಾಂಕ)(a_ಹೊರಹೋಗು - pBt->pTmpSpace);
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
    Pager *pSrcPager = pSrc->pBt->pPager;
    u8 *p_Pgno_ಹೊರಹೋಗು = 0;
    Pgno ovfl_ಒಳ = 0;
    Db_ಪುಟ *p_ಪುಟ_ಒಳ = 0;
    Mem_ಪುಟ *p_ಪುಟ_ಹೊರಹೋಗು = 0;
    u32 n_ಹೊರಹೋಗು;                     /* Size of output buffer aOut[] */

    n_ಹೊರಹೋಗು = btree_Payload_ಗಮ್ಯ_Local(pDest->p_ಪುಟ, pSrc->info.nPayload);
    pBt->n_Preformat_ಗಾತ್ರ = (ಪೂರ್ಣಾಂಕ)n_ಹೊರಹೋಗು + (ಪೂರ್ಣಾಂಕ)(a_ಹೊರಹೋಗು - pBt->pTmpSpace);
    ಆದರೆ( n_ಹೊರಹೋಗು<pSrc->info.nPayload ){
      p_Pgno_ಹೊರಹೋಗು = &a_ಹೊರಹೋಗು[n_ಹೊರಹೋಗು];
      pBt->n_Preformat_ಗಾತ್ರ += 4;
    }
 
    ಆದರೆ( nRem>n_ಒಳ ){
      ಆದರೆ( a_ಒಳ+n_ಒಳ+4>pSrc->p_ಪುಟ->a_ದತ್ತಾಂಶ_ಕೊನೆ ){
        ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(pSrc->p_ಪುಟ);
      }
      ovfl_ಒಳ = get4byte(&pSrc->info.pPayload[n_ಒಳ]);
    }
 
    ಮಾಡು {
      nRem -= n_ಹೊರಹೋಗು;
      ಮಾಡು{
        ಪ್ರತಿಜ್ಞಾ( n_ಹೊರಹೋಗು>0 );
        ಆದರೆ( n_ಒಳ>0 ){
          ಪೂರ್ಣಾಂಕ nCopy = MIN(n_ಹೊರಹೋಗು, n_ಒಳ);
          ಸ್ಮರಣೆ_ನಕಲು(a_ಹೊರಹೋಗು, a_ಒಳ, nCopy);
          n_ಹೊರಹೋಗು -= nCopy;
          n_ಒಳ -= nCopy;
          a_ಹೊರಹೋಗು += nCopy;
          a_ಒಳ += nCopy;
        }
        ಆದರೆ( n_ಹೊರಹೋಗು>0 ){
          sqlite3PagerUnref(p_ಪುಟ_ಒಳ);
          p_ಪುಟ_ಒಳ = 0;
          rc = sqlite3PagerGet(pSrcPager, ovfl_ಒಳ, &p_ಪುಟ_ಒಳ, PAGER_GET_READONLY);
          ಆದರೆ( rc==SQLITE_OK ){
            a_ಒಳ = (ಸ್ಥಿರಾಂಕ u8*)sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_ಪುಟ_ಒಳ);
            ovfl_ಒಳ = get4byte(a_ಒಳ);
            a_ಒಳ += 4;
            n_ಒಳ = pSrc->pBt->usable_ಗಾತ್ರ - 4;
          }
        }
      }ಆಗ( rc==SQLITE_OK && n_ಹೊರಹೋಗು>0 );
 
      ಆದರೆ( rc==SQLITE_OK && nRem>0 && ALWAYS(p_Pgno_ಹೊರಹೋಗು) ){
        Pgno pgnoNew = 0;  /* Prevent harmless static-analyzer warning */
        Mem_ಪುಟ *pNew = 0;
        rc = allocate_Btree_ಪುಟ(pBt, &pNew, &pgnoNew, 0, 0);
        put4byte(p_Pgno_ಹೊರಹೋಗು, pgnoNew);
        ಆದರೆ( ISAUTOVACUUM(pBt) && p_ಪುಟ_ಹೊರಹೋಗು ){
          ptrmapPut(pBt, pgnoNew, PTRMAP_OVERFLOW2, p_ಪುಟ_ಹೊರಹೋಗು->pgno, &rc);
        }
        release_ಪುಟ(p_ಪುಟ_ಹೊರಹೋಗು);
        p_ಪುಟ_ಹೊರಹೋಗು = pNew;
        ಆದರೆ( p_ಪುಟ_ಹೊರಹೋಗು ){
          p_Pgno_ಹೊರಹೋಗು = p_ಪುಟ_ಹೊರಹೋಗು->a_ದತ್ತಾಂಶ;
          put4byte(p_Pgno_ಹೊರಹೋಗು, 0);
          a_ಹೊರಹೋಗು = &p_Pgno_ಹೊರಹೋಗು[4];
          n_ಹೊರಹೋಗು = MIN(pBt->usable_ಗಾತ್ರ - 4, nRem);
        }
      }
    }ಆಗ( nRem>0 && rc==SQLITE_OK );
 
    release_ಪುಟ(p_ಪುಟ_ಹೊರಹೋಗು);
    sqlite3PagerUnref(p_ಪುಟ_ಒಳ);
    ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3BtreeDelete(BtCursor *pCur, u8 flags){
  Btree *p = pCur->pBtree;
  BtShared *pBt = p->pBt;             
  ಪೂರ್ಣಾಂಕ rc;                    /* Return code */
  Mem_ಪುಟ *p_ಪುಟ;            /* Page to delete cell from */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pCell;      /* Pointer to cell to delete */
  ಪೂರ್ಣಾಂಕ iCellIdx;              /* Index of cell to delete */
  ಪೂರ್ಣಾಂಕ i_Cell_ಆಳ;            /* Depth of node containing pCell */
  CellInfo info;             /* Size of the cell being deleted */
  u8 bPreserve;              /* Keep cursor valid.  2 for CURSOR_SKIPNEXT */

  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCur) );
  ಪ್ರತಿಜ್ಞಾ( pBt->ಒಳ_Transaction==TRANS_WRITE );
  ಪ್ರತಿಜ್ಞಾ( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  ಪ್ರತಿಜ್ಞಾ( pCur->curFlags & BTCF_WriteFlag );
  ಪ್ರತಿಜ್ಞಾ( has_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(p, pCur->pgno_ಬೇರು, pCur->p_ಕೀಲಿ_Info!=0, 2) );
  ಪ್ರತಿಜ್ಞಾ( !hasReadConflicts(p, pCur->pgno_ಬೇರು) );
  ಪ್ರತಿಜ್ಞಾ( (flags & ~(BTREE_SAVEPOSITION | BTREE_AUXDELETE))==0 );
  ಆದರೆ( pCur->eState!=CURSOR_VALID ){
    ಆದರೆ( pCur->eState>=CURSOR_REQUIRESEEK ){
      rc = btreeRestoreCursorPosition(pCur);
      ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || CORRUPT_DB || pCur->eState==CURSOR_VALID );
      ಆದರೆ( rc || pCur->eState!=CURSOR_VALID ) ಹಿಂತಿರುಗಿ rc;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_PGNO(pCur->pgno_ಬೇರು);
    }
  }
  ಪ್ರತಿಜ್ಞಾ( pCur->eState==CURSOR_VALID );

  i_Cell_ಆಳ = pCur->i_ಪುಟ;
  iCellIdx = pCur->ix;
  p_ಪುಟ = pCur->p_ಪುಟ;
  ಆದರೆ( p_ಪುಟ->nCell<=iCellIdx ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
  }
  pCell = ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, iCellIdx);
  ಆದರೆ( p_ಪುಟ->nFree<0 && btreeComputeFreeSpace(p_ಪುಟ) ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
  }
  ಆದರೆ( pCell<&p_ಪುಟ->aCellIdx[p_ಪುಟ->nCell] ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
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
  ಆದರೆ( bPreserve ){
    ಆದರೆ( !p_ಪುಟ->ಎಲೆ
     || (p_ಪುಟ->nFree+p_ಪುಟ->x_Cell_ಗಾತ್ರ(p_ಪುಟ,pCell)+2) >
                                                   (ಪೂರ್ಣಾಂಕ)(pBt->usable_ಗಾತ್ರ*2/3)
     || p_ಪುಟ->nCell==1  /* See dbfuzz001.test for a test case */
    ){
      /* A b-tree rebalance will be required after deleting this entry.
      ** Save the cursor key.  */
      rc = save_Cursor_ಕೀಲಿ(pCur);
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    }ಇಲ್ಲದಿದ್ದರೆ{
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
  ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
    rc = sqlite3_Btree_ಹಿಂದಿನ(pCur, 0);
    ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_ಮುಗಿದ );
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
  }

  /* Save the positions of any other cursors open on this table before
  ** making any modifications.  */
  ಆದರೆ( pCur->curFlags & BTCF_ಅಪವರ್ತ್ಯ ){
    rc = saveAllCursors(pBt, pCur->pgno_ಬೇರು, pCur);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
  }

  /* If this is a delete operation to remove a row from a table b-tree,
  ** invalidate any incrblob cursors open on the row being deleted.  */
  ಆದರೆ( pCur->p_ಕೀಲಿ_Info==0 && p->hasIncrblobCur ){
    invalidateIncrblobCursors(p, pCur->pgno_ಬೇರು, pCur->info.n_ಕೀಲಿ, 0);
  }

  /* Make the page containing the entry to be deleted writable. Then free any
  ** overflow pages associated with the entry and finally remove the cell
  ** itself from within the page.  */
  rc = sqlite3PagerWrite(p_ಪುಟ->p_Db_ಪುಟ);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
  BTREE_CLEAR_CELL(rc, p_ಪುಟ, pCell, info);
  dropCell(p_ಪುಟ, iCellIdx, info.n_ಗಾತ್ರ, &rc);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;

  /* If the cell deleted was not located on a leaf page, then the cursor
  ** is currently pointing to the largest entry in the sub-tree headed
  ** by the child-page of the cell that was just deleted from an internal
  ** node. The cell from the leaf node needs to be moved to the internal
  ** node to replace the deleted cell.  */
  ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
    Mem_ಪುಟ *p_ಎಲೆ = pCur->p_ಪುಟ;
    ಪೂರ್ಣಾಂಕ nCell;
    Pgno n;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pTmp;

    ಆದರೆ( p_ಎಲೆ->nFree<0 ){
      rc = btreeComputeFreeSpace(p_ಎಲೆ);
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
    }
    ಆದರೆ( i_Cell_ಆಳ<pCur->i_ಪುಟ-1 ){
      n = pCur->ap_ಪುಟ[i_Cell_ಆಳ+1]->pgno;
    }ಇಲ್ಲದಿದ್ದರೆ{
      n = pCur->p_ಪುಟ->pgno;
    }
    pCell = ಕಂಡುಹಿಡಿ_Cell(p_ಎಲೆ, p_ಎಲೆ->nCell-1);
    ಆದರೆ( pCell<&p_ಎಲೆ->a_ದತ್ತಾಂಶ[4] ) ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_ಪುಟ(p_ಎಲೆ);
    nCell = p_ಎಲೆ->x_Cell_ಗಾತ್ರ(p_ಎಲೆ, pCell);
    ಪ್ರತಿಜ್ಞಾ( MX_CELL_ಗಾತ್ರ(pBt) >= nCell );
    pTmp = pBt->pTmpSpace;
    ಪ್ರತಿಜ್ಞಾ( pTmp!=0 );
    rc = sqlite3PagerWrite(p_ಎಲೆ->p_Db_ಪುಟ);
    ಆದರೆ( rc==SQLITE_OK ){
      rc = ಸೇರಿಸು_Cell(p_ಪುಟ, iCellIdx, pCell-4, nCell+4-(pCell[0]==2), pTmp, n);
    }
    dropCell(p_ಎಲೆ, p_ಎಲೆ->nCell-1, nCell, &rc);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
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
  ಪ್ರತಿಜ್ಞಾ( pCur->p_ಪುಟ->n_ಅತಿಪ್ರವಾಹ==0 );
  ಪ್ರತಿಜ್ಞಾ( pCur->p_ಪುಟ->nFree>=0 );
  ಆದರೆ( pCur->p_ಪುಟ->nFree*3<=(ಪೂರ್ಣಾಂಕ)pCur->pBt->usable_ಗಾತ್ರ*2 ){
    /* Optimization: If the free space is less than 2/3rds of the page,
    ** then balance() will always be a no-op.  No need to invoke it. */
    rc = SQLITE_OK;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = balance(pCur);
  }
  ಆದರೆ( rc==SQLITE_OK && pCur->i_ಪುಟ>i_Cell_ಆಳ ){
    release_ಪುಟ_Not_Null(pCur->p_ಪುಟ);
    pCur->i_ಪುಟ--;
    ಆಗ( pCur->i_ಪುಟ>i_Cell_ಆಳ ){
      release_ಪುಟ(pCur->ap_ಪುಟ[pCur->i_ಪುಟ--]);
    }
    pCur->p_ಪುಟ = pCur->ap_ಪುಟ[pCur->i_ಪುಟ];
    rc = balance(pCur);
  }

  ಆದರೆ( rc==SQLITE_OK ){
    ಆದರೆ( bPreserve>1 ){
      ಪ್ರತಿಜ್ಞಾ( (pCur->i_ಪುಟ==i_Cell_ಆಳ || CORRUPT_DB) );
      ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ==pCur->p_ಪುಟ || CORRUPT_DB );
      ಪ್ರತಿಜ್ಞಾ( (p_ಪುಟ->nCell>0 || CORRUPT_DB) && iCellIdx<=p_ಪುಟ->nCell );
      pCur->eState = CURSOR_SKIPNEXT;
      ಆದರೆ( iCellIdx>=p_ಪುಟ->nCell ){
        pCur->skip_ಮುಂದಿನ = -1;
        pCur->ix = p_ಪುಟ->nCell-1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        pCur->skip_ಮುಂದಿನ = 1;
      }
      /* The pCur->info structure is currently populated with the details
      ** of the cell that was just removed. But pCur->ix now points to a
      ** different cell in the b-tree. So zero info.nSize here so that if
      ** saveCursorPosition() is called on this cursor it does not trust
      ** the contents of pCur->info.  */
      pCur->info.n_ಗಾತ್ರ = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = move_ಗಮ್ಯ_ಬೇರು(pCur);
      ಆದರೆ( bPreserve ){
        btree_Release_All_Cursor_ಪುಟಗಳು(pCur);
        pCur->eState = CURSOR_REQUIRESEEK;
      }
      ಆದರೆ( rc==SQLITE_ಖಾಲಿಗಳು ) rc = SQLITE_OK;
    }
  }
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btree_Create_ಕೋಷ್ಟಕ(Btree *p, Pgno *pi_ಕೋಷ್ಟಕ, ಪೂರ್ಣಾಂಕ createTabFlags){
  BtShared *pBt = p->pBt;
  Mem_ಪುಟ *p_ಬೇರು;
  Pgno pgno_ಬೇರು;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ ptfFlags;          /* Page-type flags for the root page of new table */

  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(p) );
  ಪ್ರತಿಜ್ಞಾ( pBt->ಒಳ_Transaction==TRANS_WRITE );
  ಪ್ರತಿಜ್ಞಾ( (pBt->btsFlags & BTS_READ_ONLY)==0 );

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_AUTOVACUUM
  rc = allocate_Btree_ಪುಟ(pBt, &p_ಬೇರು, &pgno_ಬೇರು, 1, 0);
  ಆದರೆ( rc ){
    ಹಿಂತಿರುಗಿ rc;
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ( pBt->autoVacuum ){
    Pgno pgnoMove;      /* Move a page here to make room for the root-page */
    Mem_ಪುಟ *p_ಪುಟ_Move; /* The page to move to. */

    /* Creating a new table may probably require moving an existing database
    ** to make room for the new tables root page. In case this page turns
    ** out to be an overflow page, delete all overflow page-map caches
    ** held by open cursors.
    */
    invalidate_All_ಅತಿಪ್ರವಾಹ_Cache(pBt);

    /* Read the value of meta[3] from the database to determine where the
    ** root page of the new table should go. meta[3] is the largest root-page
    ** created so far, so the new root-page is (meta[3]+1).
    */
    sqlite3BtreeGetMeta(p, BTREE_ಅತಿದೊಡ್ಡ_ಬೇರು_ಪುಟ, &pgno_ಬೇರು);
    ಆದರೆ( pgno_ಬೇರು>btreePagecount(pBt) ){
      ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_PGNO(pgno_ಬೇರು);
    }
    pgno_ಬೇರು++;

    /* The new root-page may not be allocated on a pointer-map page, or the
    ** PENDING_BYTE page.
    */
    ಆಗ( pgno_ಬೇರು==PTRMAP_PAGENO(pBt, pgno_ಬೇರು) ||
        pgno_ಬೇರು==PENDING_BYTE_ಪುಟ(pBt) ){
      pgno_ಬೇರು++;
    }
    ಪ್ರತಿಜ್ಞಾ( pgno_ಬೇರು>=3 );

    /* Allocate a page. The page that currently resides at pgnoRoot will
    ** be moved to the allocated page (unless the allocated page happens
    ** to reside at pgnoRoot).
    */
    rc = allocate_Btree_ಪುಟ(pBt, &p_ಪುಟ_Move, &pgnoMove, pgno_ಬೇರು, BTALLOC_EXACT);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹಿಂತಿರುಗಿ rc;
    }

    ಆದರೆ( pgnoMove!=pgno_ಬೇರು ){
      /* pgnoRoot is the page that will be used for the root-page of
      ** the new table (assuming an error did not occur). But we were
      ** allocated pgnoMove. If required (i.e. if it was not allocated
      ** by extending the file), the current page at position pgnoMove
      ** is already journaled.
      */
      u8 eType = 0;
      Pgno i_Ptr_ಪುಟ = 0;

      /* Save the positions of any open cursors. This is required in
      ** case they are holding a reference to an xFetch reference
      ** corresponding to page pgnoRoot.  */
      rc = saveAllCursors(pBt, 0, 0);
      release_ಪುಟ(p_ಪುಟ_Move);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ rc;
      }

      /* Move the page currently at pgnoRoot to pgnoMove. */
      rc = btree_Get_ಪುಟ(pBt, pgno_ಬೇರು, &p_ಬೇರು, 0);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ rc;
      }
      rc = ptrmapGet(pBt, pgno_ಬೇರು, &eType, &i_Ptr_ಪುಟ);
      ಆದರೆ( eType==PTRMAP_ROOTPAGE || eType==PTRMAP_FREEPAGE ){
        rc = SQLITE_CORRUPT_PGNO(pgno_ಬೇರು);
      }
      ಆದರೆ( rc!=SQLITE_OK ){
        release_ಪುಟ(p_ಬೇರು);
        ಹಿಂತಿರುಗಿ rc;
      }
      ಪ್ರತಿಜ್ಞಾ( eType!=PTRMAP_ROOTPAGE );
      ಪ್ರತಿಜ್ಞಾ( eType!=PTRMAP_FREEPAGE );
      rc = relocate_ಪುಟ(pBt, p_ಬೇರು, eType, i_Ptr_ಪುಟ, pgnoMove, 0);
      release_ಪುಟ(p_ಬೇರು);

      /* Obtain the page at pgnoRoot */
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ rc;
      }
      rc = btree_Get_ಪುಟ(pBt, pgno_ಬೇರು, &p_ಬೇರು, 0);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ rc;
      }
      rc = sqlite3PagerWrite(p_ಬೇರು->p_Db_ಪುಟ);
      ಆದರೆ( rc!=SQLITE_OK ){
        release_ಪುಟ(p_ಬೇರು);
        ಹಿಂತಿರುಗಿ rc;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಬೇರು = p_ಪುಟ_Move;
    }

    /* Update the pointer-map and meta-data with the new root-page number. */
    ptrmapPut(pBt, pgno_ಬೇರು, PTRMAP_ROOTPAGE, 0, &rc);
    ಆದರೆ( rc ){
      release_ಪುಟ(p_ಬೇರು);
      ಹಿಂತಿರುಗಿ rc;
    }

    /* When the new root page was allocated, page 1 was made writable in
    ** order either to increase the database filesize, or to decrement the
    ** freelist count.  Hence, the sqlite3BtreeUpdateMeta() call cannot fail.
    */
    ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(pBt->pPage1->p_Db_ಪುಟ) );
    rc = sqlite3BtreeUpdateMeta(p, 4, pgno_ಬೇರು);
    ಆದರೆ( NEVER(rc) ){
      release_ಪುಟ(p_ಬೇರು);
      ಹಿಂತಿರುಗಿ rc;
    }

  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = allocate_Btree_ಪುಟ(pBt, &p_ಬೇರು, &pgno_ಬೇರು, 1, 0);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( sqlite3PagerIswriteable(p_ಬೇರು->p_Db_ಪುಟ) );
  ಆದರೆ( createTabFlags & BTREE_INTKEY ){
    ptfFlags = PTF_INTKEY | PTF_LEAFDATA | PTF_ಎಲೆ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ptfFlags = PTF_ZERODATA | PTF_ಎಲೆ;
  }
  zero_ಪುಟ(p_ಬೇರು, ptfFlags);
  sqlite3PagerUnref(p_ಬೇರು->p_Db_ಪುಟ);
  ಪ್ರತಿಜ್ಞಾ( (pBt->openFlags & BTREE_SINGLE)==0 || pgno_ಬೇರು==2 );
  *pi_ಕೋಷ್ಟಕ = pgno_ಬೇರು;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Create_ಕೋಷ್ಟಕ(Btree *p, Pgno *pi_ಕೋಷ್ಟಕ, ಪೂರ್ಣಾಂಕ flags){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3BtreeEnter(p);
  rc = btree_Create_ಕೋಷ್ಟಕ(p, pi_ಕೋಷ್ಟಕ, flags);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ clear_Database_ಪುಟ(
  BtShared *pBt,           /* The BTree that contains the table */
  Pgno pgno,               /* Page number to clear */
  ಪೂರ್ಣಾಂಕ free_ಪುಟ_Flag,        /* bit 0: Deallocate page.  Bits 1-31: depth */
  i64 *pnChange            /* Add number of Cells freed to this counter */
){
  Mem_ಪುಟ *p_ಪುಟ;
  ಪೂರ್ಣಾಂಕ rc;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pCell;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ hdr;
  CellInfo info;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pBt->mutex) );
  ಆದರೆ( pgno>btreePagecount(pBt) ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_PGNO(pgno);
  }
  ಆದರೆ( (free_ಪುಟ_Flag>>1) > BTCURSOR_MAX_ಆಳ ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_PGNO(pgno);
  }
  rc = get_And_Init_ಪುಟ(pBt, pgno, &p_ಪುಟ, 0);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
  ಆದರೆ( (pBt->openFlags & BTREE_SINGLE)==0
   && sqlite3_Pager_ಪುಟ_Refcount(p_ಪುಟ->p_Db_ಪುಟ) != (1 + (pgno==1))
  ){
    rc = SQLITE_CORRUPT_ಪುಟ(p_ಪುಟ);
    ಹೋಗು cleardatabasepage_ಹೊರಹೋಗು;
  }
  hdr = p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ;
  ಗಾಗಿ(i=0; i<p_ಪುಟ->nCell; i++){
    pCell = ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, i);
    ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
      rc = clear_Database_ಪುಟ(pBt, get4byte(pCell),
                             (free_ಪುಟ_Flag+2)|1, pnChange);
      ಆದರೆ( rc ) ಹೋಗು cleardatabasepage_ಹೊರಹೋಗು;
    }
    BTREE_CLEAR_CELL(rc, p_ಪುಟ, pCell, info);
    ಆದರೆ( rc ) ಹೋಗು cleardatabasepage_ಹೊರಹೋಗು;
  }
  ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
    rc = clear_Database_ಪುಟ(pBt, get4byte(&p_ಪುಟ->a_ದತ್ತಾಂಶ[hdr+8]), 
                           (free_ಪುಟ_Flag+2)|1, pnChange);
    ಆದರೆ( rc ) ಹೋಗು cleardatabasepage_ಹೊರಹೋಗು;
    ಆದರೆ( p_ಪುಟ->int_ಕೀಲಿ ) pnChange = 0;
  }
  ಆದರೆ( pnChange ){
    testcase( !p_ಪುಟ->int_ಕೀಲಿ );
    *pnChange += p_ಪುಟ->nCell;
  }
  ಆದರೆ( (free_ಪುಟ_Flag&1)!=0 ){
    free_ಪುಟ(p_ಪುಟ, &rc);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (rc = sqlite3PagerWrite(p_ಪುಟ->p_Db_ಪುಟ))==0 ){
    zero_ಪುಟ(p_ಪುಟ, p_ಪುಟ->a_ದತ್ತಾಂಶ[hdr] | PTF_ಎಲೆ);
  }

cleardatabasepage_ಹೊರಹೋಗು:
  release_ಪುಟ(p_ಪುಟ);
  ಹಿಂತಿರುಗಿ rc;
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
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Clear_ಕೋಷ್ಟಕ(Btree *p, ಪೂರ್ಣಾಂಕ i_ಕೋಷ್ಟಕ, i64 *pnChange){
  ಪೂರ್ಣಾಂಕ rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);
  ಪ್ರತಿಜ್ಞಾ( p->ಒಳ_Trans==TRANS_WRITE );

  rc = saveAllCursors(pBt, (Pgno)i_ಕೋಷ್ಟಕ, 0);

  ಆದರೆ( SQLITE_OK==rc ){
    /* Invalidate all incrblob cursors open on table iTable (assuming iTable
    ** is the root of a table b-tree - if it is not, the following call is
    ** a no-op).  */
    ಆದರೆ( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, (Pgno)i_ಕೋಷ್ಟಕ, 0, 1);
    }
    rc = clear_Database_ಪುಟ(pBt, (Pgno)i_ಕೋಷ್ಟಕ, 0, pnChange);
  }
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Delete all information from the single table that pCur is open on.
**
** This routine only work for pCur on an ephemeral table.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Clear_ಕೋಷ್ಟಕ_ದ_Cursor(BtCursor *pCur){
  ಹಿಂತಿರುಗಿ sqlite3_Btree_Clear_ಕೋಷ್ಟಕ(pCur->pBtree, pCur->pgno_ಬೇರು, 0);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btree_Drop_ಕೋಷ್ಟಕ(Btree *p, Pgno i_ಕೋಷ್ಟಕ, ಪೂರ್ಣಾಂಕ *piMoved){
  ಪೂರ್ಣಾಂಕ rc;
  Mem_ಪುಟ *p_ಪುಟ = 0;
  BtShared *pBt = p->pBt;

  ಪ್ರತಿಜ್ಞಾ( sqlite3BtreeHoldsMutex(p) );
  ಪ್ರತಿಜ್ಞಾ( p->ಒಳ_Trans==TRANS_WRITE );
  ಪ್ರತಿಜ್ಞಾ( i_ಕೋಷ್ಟಕ>=2 );
  ಆದರೆ( i_ಕೋಷ್ಟಕ>btreePagecount(pBt) ){
    ಹಿಂತಿರುಗಿ SQLITE_CORRUPT_PGNO(i_ಕೋಷ್ಟಕ);
  }

  rc = sqlite3_Btree_Clear_ಕೋಷ್ಟಕ(p, i_ಕೋಷ್ಟಕ, 0);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
  rc = btree_Get_ಪುಟ(pBt, (Pgno)i_ಕೋಷ್ಟಕ, &p_ಪುಟ, 0);
  ಆದರೆ( NEVER(rc) ){
    release_ಪುಟ(p_ಪುಟ);
    ಹಿಂತಿರುಗಿ rc;
  }

  *piMoved = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_AUTOVACUUM
  free_ಪುಟ(p_ಪುಟ, &rc);
  release_ಪುಟ(p_ಪುಟ);
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ( pBt->autoVacuum ){
    Pgno max_ಬೇರು_Pgno;
    sqlite3BtreeGetMeta(p, BTREE_ಅತಿದೊಡ್ಡ_ಬೇರು_ಪುಟ, &max_ಬೇರು_Pgno);

    ಆದರೆ( i_ಕೋಷ್ಟಕ==max_ಬೇರು_Pgno ){
      /* If the table being dropped is the table with the largest root-page
      ** number in the database, put the root page on the free list.
      */
      free_ಪುಟ(p_ಪುಟ, &rc);
      release_ಪುಟ(p_ಪುಟ);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ rc;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* The table being dropped does not have the largest root-page
      ** number in the database. So move the page that does into the
      ** gap left by the deleted root-page.
      */
      Mem_ಪುಟ *pMove;
      release_ಪುಟ(p_ಪುಟ);
      rc = btree_Get_ಪುಟ(pBt, max_ಬೇರು_Pgno, &pMove, 0);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ rc;
      }
      rc = relocate_ಪುಟ(pBt, pMove, PTRMAP_ROOTPAGE, 0, i_ಕೋಷ್ಟಕ, 0);
      release_ಪುಟ(pMove);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ rc;
      }
      pMove = 0;
      rc = btree_Get_ಪುಟ(pBt, max_ಬೇರು_Pgno, &pMove, 0);
      ಆದರೆ( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pMove->p_Db_ಪುಟ);
      }
      free_ಪುಟ(pMove, &rc);
      release_ಪುಟ(pMove);
      ಆದರೆ( rc!=SQLITE_OK ){
        ಹಿಂತಿರುಗಿ rc;
      }
      *piMoved = max_ಬೇರು_Pgno;
    }

    /* Set the new 'max-root-page' value in the database header. This
    ** is the old value less one, less one more if that happens to
    ** be a root-page number, less one again if that is the
    ** PENDING_BYTE_PAGE.
    */
    max_ಬೇರು_Pgno--;
    ಆಗ( max_ಬೇರು_Pgno==PENDING_BYTE_ಪುಟ(pBt)
           || PTRMAP_ISPAGE(pBt, max_ಬೇರು_Pgno) ){
      max_ಬೇರು_Pgno--;
    }
    ಪ್ರತಿಜ್ಞಾ( max_ಬೇರು_Pgno!=PENDING_BYTE_ಪುಟ(pBt) );

    rc = sqlite3BtreeUpdateMeta(p, 4, max_ಬೇರು_Pgno);
  }ಇಲ್ಲದಿದ್ದರೆ{
    free_ಪುಟ(p_ಪುಟ, &rc);
    release_ಪುಟ(p_ಪುಟ);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ rc; 
}
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Drop_ಕೋಷ್ಟಕ(Btree *p, ಪೂರ್ಣಾಂಕ i_ಕೋಷ್ಟಕ, ಪೂರ್ಣಾಂಕ *piMoved){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3BtreeEnter(p);
  rc = btree_Drop_ಕೋಷ್ಟಕ(p, i_ಕೋಷ್ಟಕ, piMoved);
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
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
ಶೂನ್ಯ sqlite3BtreeGetMeta(Btree *p, ಪೂರ್ಣಾಂಕ idx, u32 *pMeta){
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  ಪ್ರತಿಜ್ಞಾ( p->ಒಳ_Trans>TRANS_NONE );
  ಪ್ರತಿಜ್ಞಾ( SQLITE_OK==query_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(p, SCHEMA_ಬೇರು, READ_ಬೀಗ) );
  ಪ್ರತಿಜ್ಞಾ( pBt->pPage1 );
  ಪ್ರತಿಜ್ಞಾ( idx>=0 && idx<=15 );

  ಆದರೆ( idx==BTREE_ದತ್ತಾಂಶ_ಆವೃತ್ತಿ ){
    *pMeta = sqlite3_Pager_ದತ್ತಾಂಶ_ಆವೃತ್ತಿ(pBt->pPager) + p->i_BData_ಆವೃತ್ತಿ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    *pMeta = get4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[36 + idx*4]);
  }

  /* If auto-vacuum is disabled in this build and this is an auto-vacuum
  ** database, mark the database as read-only.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_AUTOVACUUM
  ಆದರೆ( idx==BTREE_ಅತಿದೊಡ್ಡ_ಬೇರು_ಪುಟ && *pMeta>0 ){
    pBt->btsFlags |= BTS_READ_ONLY;
  }
#ಆದರೆ_ಅಂತ್ಯ

  sqlite3BtreeLeave(p);
}

/*
** Write meta-information back into the database.  Meta[0] is
** read-only and may not be written.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeUpdateMeta(Btree *p, ಪೂರ್ಣಾಂಕ idx, u32 iMeta){
  BtShared *pBt = p->pBt;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pP1;
  ಪೂರ್ಣಾಂಕ rc;
  ಪ್ರತಿಜ್ಞಾ( idx>=1 && idx<=15 );
  sqlite3BtreeEnter(p);
  ಪ್ರತಿಜ್ಞಾ( p->ಒಳ_Trans==TRANS_WRITE );
  ಪ್ರತಿಜ್ಞಾ( pBt->pPage1!=0 );
  pP1 = pBt->pPage1->a_ದತ್ತಾಂಶ;
  rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ಪುಟ);
  ಆದರೆ( rc==SQLITE_OK ){
    put4byte(&pP1[36 + idx*4], iMeta);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
    ಆದರೆ( idx==BTREE_INCR_VACUUM ){
      ಪ್ರತಿಜ್ಞಾ( pBt->autoVacuum || iMeta==0 );
      ಪ್ರತಿಜ್ಞಾ( iMeta==0 || iMeta==1 );
      pBt->incrVacuum = (u8)iMeta;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** The first argument, pCur, is a cursor opened on some b-tree. Count the
** number of entries in the b-tree and write the result to *pnEntry.
**
** SQLITE_OK is returned if the operation is successfully executed.
** Otherwise, if an error is encountered (i.e. an IO error or database
** corruption) an SQLite error code is returned.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಎಣಿಕೆ(sqlite3 *db, BtCursor *pCur, i64 *pn_ನಮೂದು){
  i64 n_ನಮೂದು = 0;                      /* Value to return in *pnEntry */
  ಪೂರ್ಣಾಂಕ rc;                              /* Return code */

  rc = move_ಗಮ್ಯ_ಬೇರು(pCur);
  ಆದರೆ( rc==SQLITE_ಖಾಲಿಗಳು ){
    *pn_ನಮೂದು = 0;
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }

  /* Unless an error occurs, the following loop runs one iteration for each
  ** page in the B-Tree structure (not including overflow pages).
  */
  ಆಗ( rc==SQLITE_OK && !Atomic_ತುಂಬು(&db->u1.isInterrupted) ){
    ಪೂರ್ಣಾಂಕ iIdx;                          /* Index of child node in parent */
    Mem_ಪುಟ *p_ಪುಟ;                    /* Current page of the b-tree */

    /* If this is a leaf page or the tree is not an int-key tree, then
    ** this page contains countable entries. Increment the entry counter
    ** accordingly.
    */
    p_ಪುಟ = pCur->p_ಪುಟ;
    ಆದರೆ( p_ಪುಟ->ಎಲೆ || !p_ಪುಟ->int_ಕೀಲಿ ){
      n_ನಮೂದು += p_ಪುಟ->nCell;
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
    ಆದರೆ( p_ಪುಟ->ಎಲೆ ){
      ಮಾಡು {
        ಆದರೆ( pCur->i_ಪುಟ==0 ){
          /* All pages of the b-tree have been visited. Return successfully. */
          *pn_ನಮೂದು = n_ನಮೂದು;
          ಹಿಂತಿರುಗಿ move_ಗಮ್ಯ_ಬೇರು(pCur);
        }
        move_ಗಮ್ಯ_ಪೋಷಕ(pCur);
      }ಆಗ ( pCur->ix>=pCur->p_ಪುಟ->nCell );

      pCur->ix++;
      p_ಪುಟ = pCur->p_ಪುಟ;
    }

    /* Descend to the child node of the cell that the cursor currently
    ** points at. This is the right-child if (iIdx==pPage->nCell).
    */
    iIdx = pCur->ix;
    ಆದರೆ( iIdx==p_ಪುಟ->nCell ){
      rc = move_ಗಮ್ಯ_ಮಗು(pCur, get4byte(&p_ಪುಟ->a_ದತ್ತಾಂಶ[p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ+8]));
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = move_ಗಮ್ಯ_ಮಗು(pCur, get4byte(ಕಂಡುಹಿಡಿ_Cell(p_ಪುಟ, iIdx)));
    }
  }

  /* An error has occurred. Return an error code. */
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return the pager associated with a BTree.  This routine is used for
** testing and debugging only.
*/
Pager *sqlite3BtreePager(Btree *p){
  ಹಿಂತಿರುಗಿ p->pBt->pPager;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INTEGRITY_CHECK
/*
** Record an OOM error during integrity_check
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkOom(IntegrityCk *pCheck){
  pCheck->rc = SQLITE_NOMEM;
  pCheck->mxErr = 0;  /* Causes integrity_check processing to stop */
  ಆದರೆ( pCheck->nErr==0 ) pCheck->nErr++;
}

/*
** Invoke the progress handler, if appropriate.  Also check for an
** interrupt.
*/
ಸ್ಥಿರ ಶೂನ್ಯ check_ಪ್ರಗತಿ(IntegrityCk *pCheck){
  sqlite3 *db = pCheck->db;
  ಆದರೆ( Atomic_ತುಂಬು(&db->u1.isInterrupted) ){
    pCheck->rc = SQLITE_ಅಡಚಣೆ;
    pCheck->nErr++;
    pCheck->mxErr = 0;
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ಪ್ರಗತಿ_CALLBACK
  ಆದರೆ( db->x_ಪ್ರಗತಿ ){
    ಪ್ರತಿಜ್ಞಾ( db->n_ಪ್ರಗತಿ_Ops>0 );
    pCheck->n_ಹೆಜ್ಜೆ++;
    ಆದರೆ( (pCheck->n_ಹೆಜ್ಜೆ % db->n_ಪ್ರಗತಿ_Ops)==0
     && db->x_ಪ್ರಗತಿ(db->p_ಪ್ರಗತಿ_Arg)
    ){
      pCheck->rc = SQLITE_ಅಡಚಣೆ;
      pCheck->nErr++;
      pCheck->mxErr = 0;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Append a message to the error message string.
*/
ಸ್ಥಿರ ಶೂನ್ಯ check_Append_ಸಂದೇಶ(
  IntegrityCk *pCheck,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat,
  ...
){
  va_list ap;
  check_ಪ್ರಗತಿ(pCheck);
  ಆದರೆ( !pCheck->mxErr ) ಹಿಂತಿರುಗಿ;
  pCheck->mxErr--;
  pCheck->nErr++;
  va_start(ap, zFormat);
  ಆದರೆ( pCheck->err_ಸಂದೇಶ.nChar ){
    sqlite3_str_append(&pCheck->err_ಸಂದೇಶ, "\n", 1);
  }
  ಆದರೆ( pCheck->zPfx ){
    sqlite3_str_appendf(&pCheck->err_ಸಂದೇಶ, pCheck->zPfx,
                        pCheck->v0, pCheck->v1, pCheck->v2);
  }
  sqlite3_str_vappendf(&pCheck->err_ಸಂದೇಶ, zFormat, ap);
  va_end(ap);
  ಆದರೆ( pCheck->err_ಸಂದೇಶ.accError==SQLITE_NOMEM ){
    checkOom(pCheck);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_INTEGRITY_CHECK */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INTEGRITY_CHECK

/*
** Return non-zero if the bit in the IntegrityCk.aPgRef[] array that
** corresponds to page iPg is already set.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_ಪುಟ_Referenced(IntegrityCk *pCheck, Pgno iPg){
  ಪ್ರತಿಜ್ಞಾ( pCheck->aPgRef!=0 );
  ಪ್ರತಿಜ್ಞಾ( iPg<=pCheck->n_Ck_ಪುಟ && ಗಾತ್ರ(pCheck->aPgRef[0])==1 );
  ಹಿಂತಿರುಗಿ (pCheck->aPgRef[iPg/8] & (1 << (iPg & 0x07)));
}

/*
** Set the bit in the IntegrityCk.aPgRef[] array that corresponds to page iPg.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_ಪುಟ_Referenced(IntegrityCk *pCheck, Pgno iPg){
  ಪ್ರತಿಜ್ಞಾ( pCheck->aPgRef!=0 );
  ಪ್ರತಿಜ್ಞಾ( iPg<=pCheck->n_Ck_ಪುಟ && ಗಾತ್ರ(pCheck->aPgRef[0])==1 );
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkRef(IntegrityCk *pCheck, Pgno i_ಪುಟ){
  ಆದರೆ( i_ಪುಟ>pCheck->n_Ck_ಪುಟ || i_ಪುಟ==0 ){
    check_Append_ಸಂದೇಶ(pCheck, "invalid page number %u", i_ಪುಟ);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಆದರೆ( get_ಪುಟ_Referenced(pCheck, i_ಪುಟ) ){
    check_Append_ಸಂದೇಶ(pCheck, "2nd reference to page %u", i_ಪುಟ);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಗಣ_ಪುಟ_Referenced(pCheck, i_ಪುಟ);
  ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
/*
** Check that the entry in the pointer-map for page iChild maps to
** page iParent, pointer type ptrType. If not, append an error message
** to pCheck.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkPtrmap(
  IntegrityCk *pCheck,   /* Integrity check context */
  Pgno i_ಮಗು,           /* Child page number */
  u8 eType,              /* Expected pointer map type */
  Pgno i_ಪೋಷಕ           /* Expected pointer map parent page number */
){
  ಪೂರ್ಣಾಂಕ rc;
  u8 ePtrmapType;
  Pgno i_Ptrmap_ಪೋಷಕ;

  rc = ptrmapGet(pCheck->pBt, i_ಮಗು, &ePtrmapType, &i_Ptrmap_ಪೋಷಕ);
  ಆದರೆ( rc!=SQLITE_OK ){
    ಆದರೆ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ) checkOom(pCheck);
    check_Append_ಸಂದೇಶ(pCheck, "Failed to read ptrmap key=%u", i_ಮಗು);
    ಹಿಂತಿರುಗಿ;
  }

  ಆದರೆ( ePtrmapType!=eType || i_Ptrmap_ಪೋಷಕ!=i_ಪೋಷಕ ){
    check_Append_ಸಂದೇಶ(pCheck,
      "Bad ptr map entry key=%u expected=(%u,%u) got=(%u,%u)",
      i_ಮಗು, eType, i_ಪೋಷಕ, ePtrmapType, i_Ptrmap_ಪೋಷಕ);
  }
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Check the integrity of the freelist or of an overflow page list.
** Verify that the number of pages on the list is N.
*/
ಸ್ಥಿರ ಶೂನ್ಯ checkList(
  IntegrityCk *pCheck,  /* Integrity checking context */
  ಪೂರ್ಣಾಂಕ isFreeList,       /* True for a freelist.  False for overflow page list */
  Pgno i_ಪುಟ,           /* Page number for first page in the list */
  u32 N                 /* Expected number of pages in the list */
){
  ಪೂರ್ಣಾಂಕ i;
  u32 expected = N;
  ಪೂರ್ಣಾಂಕ n_Err_At_ಆರಂಭ = pCheck->nErr;
  ಆಗ( i_ಪುಟ!=0 && pCheck->mxErr ){
    Db_ಪುಟ *p_Ovfl_ಪುಟ;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p_Ovfl_ದತ್ತಾಂಶ;
    ಆದರೆ( checkRef(pCheck, i_ಪುಟ) ) ನಿಲ್ಲಿಸು;
    N--;
    ಆದರೆ( sqlite3PagerGet(pCheck->pPager, (Pgno)i_ಪುಟ, &p_Ovfl_ಪುಟ, 0) ){
      check_Append_ಸಂದೇಶ(pCheck, "failed to get page %u", i_ಪುಟ);
      ನಿಲ್ಲಿಸು;
    }
    p_Ovfl_ದತ್ತಾಂಶ = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_Ovfl_ಪುಟ);
    ಆದರೆ( isFreeList ){
      u32 n = (u32)get4byte(&p_Ovfl_ದತ್ತಾಂಶ[4]);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
      ಆದರೆ( pCheck->pBt->autoVacuum ){
        checkPtrmap(pCheck, i_ಪುಟ, PTRMAP_FREEPAGE, 0);
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಆದರೆ( n>pCheck->pBt->usable_ಗಾತ್ರ/4-2 ){
        check_Append_ಸಂದೇಶ(pCheck,
           "freelist leaf count too big on page %u", i_ಪುಟ);
        N--;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಗಾಗಿ(i=0; i<(ಪೂರ್ಣಾಂಕ)n; i++){
          Pgno i_Free_ಪುಟ = get4byte(&p_Ovfl_ದತ್ತಾಂಶ[8+i*4]);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
          ಆದರೆ( pCheck->pBt->autoVacuum ){
            checkPtrmap(pCheck, i_Free_ಪುಟ, PTRMAP_FREEPAGE, 0);
          }
#ಆದರೆ_ಅಂತ್ಯ
          checkRef(pCheck, i_Free_ಪುಟ);
        }
        N -= n;
      }
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
    ಇಲ್ಲದಿದ್ದರೆ{
      /* If this database supports auto-vacuum and iPage is not the last
      ** page in this overflow list, check that the pointer-map entry for
      ** the following page matches iPage.
      */
      ಆದರೆ( pCheck->pBt->autoVacuum && N>0 ){
        i = get4byte(p_Ovfl_ದತ್ತಾಂಶ);
        checkPtrmap(pCheck, i, PTRMAP_OVERFLOW2, i_ಪುಟ);
      }
    }
#ಆದರೆ_ಅಂತ್ಯ
    i_ಪುಟ = get4byte(p_Ovfl_ದತ್ತಾಂಶ);
    sqlite3PagerUnref(p_Ovfl_ಪುಟ);
  }
  ಆದರೆ( N && n_Err_At_ಆರಂಭ==pCheck->nErr ){
    check_Append_ಸಂದೇಶ(pCheck,
      "%s is %u but should be %u",
      isFreeList ? "size" : "overflow list length",
      expected-N, expected);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_INTEGRITY_CHECK */

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
ಸ್ಥಿರ ಶೂನ್ಯ btree_ರಾಶಿ_ಸೇರಿಸು(u32 *a_ರಾಶಿ, u32 x){
  u32 j, i;
  ಪ್ರತಿಜ್ಞಾ( a_ರಾಶಿ!=0 );
  i = ++a_ರಾಶಿ[0];
  a_ರಾಶಿ[i] = x;
  ಆಗ( (j = i/2)>0 && a_ರಾಶಿ[j]>a_ರಾಶಿ[i] ){
    x = a_ರಾಶಿ[j];
    a_ರಾಶಿ[j] = a_ರಾಶಿ[i];
    a_ರಾಶಿ[i] = x;
    i = j;
  }
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ btree_ರಾಶಿ_Pull(u32 *a_ರಾಶಿ, u32 *p_ಹೊರಹೋಗು){
  u32 j, i, x;
  ಆದರೆ( (x = a_ರಾಶಿ[0])==0 ) ಹಿಂತಿರುಗಿ 0;
  *p_ಹೊರಹೋಗು = a_ರಾಶಿ[1];
  a_ರಾಶಿ[1] = a_ರಾಶಿ[x];
  a_ರಾಶಿ[x] = 0xffffffff;
  a_ರಾಶಿ[0]--;
  i = 1;
  ಆಗ( (j = i*2)<=a_ರಾಶಿ[0] ){
    ಆದರೆ( a_ರಾಶಿ[j]>a_ರಾಶಿ[j+1] ) j++;
    ಆದರೆ( a_ರಾಶಿ[i]<a_ರಾಶಿ[j] ) ನಿಲ್ಲಿಸು;
    x = a_ರಾಶಿ[i];
    a_ರಾಶಿ[i] = a_ರಾಶಿ[j];
    a_ರಾಶಿ[j] = x;
    i = j;
  }
  ಹಿಂತಿರುಗಿ 1; 
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INTEGRITY_CHECK
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_ವೃಕ್ಷ_ಪುಟ(
  IntegrityCk *pCheck,  /* Context for the sanity check */
  Pgno i_ಪುಟ,           /* Page number of the page to check */
  i64 *pi_Min_ಕೀಲಿ,        /* Write minimum integer primary key here */
  i64 max_ಕೀಲಿ            /* Error if integer primary key greater than this */
){
  Mem_ಪುಟ *p_ಪುಟ = 0;      /* The page being analyzed */
  ಪೂರ್ಣಾಂಕ i;                   /* Loop counter */
  ಪೂರ್ಣಾಂಕ rc;                  /* Result code from subroutine call */
  ಪೂರ್ಣಾಂಕ ಆಳ = -1, d2;      /* Depth of a subtree */
  ಪೂರ್ಣಾಂಕ pgno;                /* Page number */
  ಪೂರ್ಣಾಂಕ nFrag;               /* Number of fragmented bytes on the page */
  ಪೂರ್ಣಾಂಕ hdr;                 /* Offset to the page header */
  ಪೂರ್ಣಾಂಕ cell_ಆರಂಭ;           /* Offset to the start of the cell pointer array */
  ಪೂರ್ಣಾಂಕ nCell;               /* Number of cells */
  ಪೂರ್ಣಾಂಕ doCoverageCheck = 1; /* True if cell coverage checking should be done */
  ಪೂರ್ಣಾಂಕ ಕೀಲಿ_Can_Be_Equal = 1;   /* True if IPK can be equal to maxKey
                           ** False if IPK must be strictly less than maxKey */
  u8 *ದತ್ತಾಂಶ;                /* Page content */
  u8 *pCell;               /* Cell content */
  u8 *pCellIdx;            /* Next element of the cell pointer array */
  BtShared *pBt;           /* The BtShared object that owns pPage */
  u32 pc;                  /* Address of a cell */
  u32 usable_ಗಾತ್ರ;          /* Usable size of the page */
  u32 content_ಸ್ಥಾನಾಂತರ;       /* Offset to the start of the cell content area */
  u32 *ರಾಶಿ = 0;           /* Min-heap used for checking cell coverage */
  u32 x, prev = 0;         /* Next and previous entry on the min-heap */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *saved_zPfx = pCheck->zPfx;
  ಪೂರ್ಣಾಂಕ saved_v1 = pCheck->v1;
  ಪೂರ್ಣಾಂಕ saved_v2 = pCheck->v2;
  u8 savedIsInit = 0;

  /* Check that the page exists
  */
  check_ಪ್ರಗತಿ(pCheck);
  ಆದರೆ( pCheck->mxErr==0 ) ಹಿಂತಿರುಗಿ 0;
  pBt = pCheck->pBt;
  usable_ಗಾತ್ರ = pBt->usable_ಗಾತ್ರ;
  ಆದರೆ( i_ಪುಟ==0 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( checkRef(pCheck, i_ಪುಟ) ) ಹಿಂತಿರುಗಿ 0;
  pCheck->zPfx = "Tree %u page %u: ";
  pCheck->v1 = i_ಪುಟ;
  pCheck->nAbove++;
  ಆದರೆ( (rc = btree_Get_ಪುಟ(pBt, i_ಪುಟ, &p_ಪುಟ, 0))!=0 ){
    check_Append_ಸಂದೇಶ(pCheck,
       "unable to get the page. error code=%d", rc);
    ಆದರೆ( rc==SQLITE_IOERR_NOMEM ) pCheck->rc = SQLITE_NOMEM;
    ಹೋಗು ಕೊನೆ_ದ_check;
  }
  ಆದರೆ( pCheck->nAbove > BTCURSOR_MAX_ಆಳ ){
    check_Append_ಸಂದೇಶ(pCheck,"btree depth exceeds %d",BTCURSOR_MAX_ಆಳ);
    ಹೋಗು ಕೊನೆ_ದ_check;
  }

  /* Clear MemPage.isInit to make sure the corruption detection code in
  ** btreeInitPage() is executed.  */
  savedIsInit = p_ಪುಟ->isInit;
  p_ಪುಟ->isInit = 0;
  ಆದರೆ( (rc = btree_Init_ಪುಟ(p_ಪುಟ))!=0 ){
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_CORRUPT );  /* The only possible error from InitPage */
    check_Append_ಸಂದೇಶ(pCheck,
                   "btreeInitPage() returns error code %d", rc);
    ಹೋಗು ಕೊನೆ_ದ_check;
  }
  ಆದರೆ( (rc = btreeComputeFreeSpace(p_ಪುಟ))!=0 ){
    ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_CORRUPT );
    check_Append_ಸಂದೇಶ(pCheck, "free space corruption", rc);
    ಹೋಗು ಕೊನೆ_ದ_check;
  }
  ದತ್ತಾಂಶ = p_ಪುಟ->a_ದತ್ತಾಂಶ;
  hdr = p_ಪುಟ->hdr_ಸ್ಥಾನಾಂತರ;

  /* Set up for cell analysis */
  pCheck->zPfx = "Tree %u page %u cell %u: ";
  content_ಸ್ಥಾನಾಂತರ = get2byteNotZero(&ದತ್ತಾಂಶ[hdr+5]);
  ಪ್ರತಿಜ್ಞಾ( content_ಸ್ಥಾನಾಂತರ<=usable_ಗಾತ್ರ );  /* Enforced by btreeInitPage() */

  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  nCell = get2byte(&ದತ್ತಾಂಶ[hdr+3]);
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->nCell==nCell );
  ಆದರೆ( p_ಪುಟ->ಎಲೆ || p_ಪುಟ->int_ಕೀಲಿ==0 ){
    pCheck->n_ಅಡ್ಡಸಾಲು += nCell;
  }

  /* EVIDENCE-OF: R-23882-45353 The cell pointer array of a b-tree page
  ** immediately follows the b-tree page header. */
  cell_ಆರಂಭ = hdr + 12 - 4*p_ಪುಟ->ಎಲೆ;
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->aCellIdx==&ದತ್ತಾಂಶ[cell_ಆರಂಭ] );
  pCellIdx = &ದತ್ತಾಂಶ[cell_ಆರಂಭ + 2*(nCell-1)];

  ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
    /* Analyze the right-child page of internal pages */
    pgno = get4byte(&ದತ್ತಾಂಶ[hdr+8]);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
    ಆದರೆ( pBt->autoVacuum ){
      pCheck->zPfx = "Tree %u page %u right child: ";
      checkPtrmap(pCheck, pgno, PTRMAP_BTREE, i_ಪುಟ);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆಳ = check_ವೃಕ್ಷ_ಪುಟ(pCheck, pgno, &max_ಕೀಲಿ, max_ಕೀಲಿ);
    ಕೀಲಿ_Can_Be_Equal = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* For leaf pages, the coverage check will occur in the same loop
    ** as the other cell checks, so initialize the heap.  */
    ರಾಶಿ = pCheck->ರಾಶಿ;
    ರಾಶಿ[0] = 0;
  }

  /* EVIDENCE-OF: R-02776-14802 The cell pointer array consists of K 2-byte
  ** integer offsets to the cell contents. */
  ಗಾಗಿ(i=nCell-1; i>=0 && pCheck->mxErr; i--){
    CellInfo info;

    /* Check cell size */
    pCheck->v2 = i;
    ಪ್ರತಿಜ್ಞಾ( pCellIdx==&ದತ್ತಾಂಶ[cell_ಆರಂಭ + i*2] );
    pc = get2byteAligned(pCellIdx);
    pCellIdx -= 2;
    ಆದರೆ( pc<content_ಸ್ಥಾನಾಂತರ || pc>usable_ಗಾತ್ರ-4 ){
      check_Append_ಸಂದೇಶ(pCheck, "Offset %u out of range %u..%u",
                             pc, content_ಸ್ಥಾನಾಂತರ, usable_ಗಾತ್ರ-4);
      doCoverageCheck = 0;
      ಮುಂದುವರಿ;
    }
    pCell = &ದತ್ತಾಂಶ[pc];
    p_ಪುಟ->xParseCell(p_ಪುಟ, pCell, &info);
    ಆದರೆ( pc+info.n_ಗಾತ್ರ>usable_ಗಾತ್ರ ){
      check_Append_ಸಂದೇಶ(pCheck, "Extends off end of page");
      doCoverageCheck = 0;
      ಮುಂದುವರಿ;
    }
    ಆದರೆ( info.nPayload && info.pPayload[0]<2 ){
      check_Append_ಸಂದೇಶ(pCheck, "Bad cell header size");
      doCoverageCheck = 0;
      ಮುಂದುವರಿ;
    }

    /* Check for integer primary key out of range */
    ಆದರೆ( p_ಪುಟ->int_ಕೀಲಿ ){
      ಆದರೆ( ಕೀಲಿ_Can_Be_Equal ? (info.n_ಕೀಲಿ > max_ಕೀಲಿ) : (info.n_ಕೀಲಿ >= max_ಕೀಲಿ) ){
        check_Append_ಸಂದೇಶ(pCheck, "Rowid %lld out of order", info.n_ಕೀಲಿ);
      }
      max_ಕೀಲಿ = info.n_ಕೀಲಿ;
      ಕೀಲಿ_Can_Be_Equal = 0;     /* Only the first key on the page may ==maxKey */
    }

    /* Check the content overflow list */
    ಆದರೆ( info.nPayload>info.nLocal ){
      u32 n_ಪುಟ;       /* Number of pages on the overflow chain */
      Pgno pgnoOvfl;   /* First page of the overflow chain */
      ಪ್ರತಿಜ್ಞಾ( pc + info.n_ಗಾತ್ರ - 4 <= usable_ಗಾತ್ರ );
      n_ಪುಟ = (info.nPayload - info.nLocal + usable_ಗಾತ್ರ - 5)/(usable_ಗಾತ್ರ - 4);
      pgnoOvfl = get4byte(&pCell[info.n_ಗಾತ್ರ - 4]);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
      ಆದರೆ( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgnoOvfl, PTRMAP_OVERFLOW1, i_ಪುಟ);
      }
#ಆದರೆ_ಅಂತ್ಯ
      checkList(pCheck, 0, pgnoOvfl, n_ಪುಟ);
    }

    ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
      /* Check sanity of left child page for internal pages */
      pgno = get4byte(pCell);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
      ಆದರೆ( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgno, PTRMAP_BTREE, i_ಪುಟ);
      }
#ಆದರೆ_ಅಂತ್ಯ
      d2 = check_ವೃಕ್ಷ_ಪುಟ(pCheck, pgno, &max_ಕೀಲಿ, max_ಕೀಲಿ);
      ಕೀಲಿ_Can_Be_Equal = 0;
      ಆದರೆ( d2!=ಆಳ ){
        check_Append_ಸಂದೇಶ(pCheck, "Child page depth differs");
        ಆಳ = d2;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Populate the coverage-checking heap for leaf pages */
      ಪ್ರತಿಜ್ಞಾ( ರಾಶಿ!=0 && ರಾಶಿ[0] < pCheck->mx_ರಾಶಿ );
      btree_ರಾಶಿ_ಸೇರಿಸು(ರಾಶಿ, (pc<<16)|(pc+info.n_ಗಾತ್ರ-1));
    }
  }
  *pi_Min_ಕೀಲಿ = max_ಕೀಲಿ;

  /* Check for complete coverage of the page
  */
  pCheck->zPfx = 0;
  ಆದರೆ( doCoverageCheck && pCheck->mxErr>0 ){
    /* For leaf pages, the min-heap has already been initialized and the
    ** cells have already been inserted.  But for internal pages, that has
    ** not yet been done, so do it now */
    ಆದರೆ( !p_ಪುಟ->ಎಲೆ ){
      ರಾಶಿ = pCheck->ರಾಶಿ;
      ರಾಶಿ[0] = 0;
      ಗಾಗಿ(i=nCell-1; i>=0; i--){
        u32 ಗಾತ್ರ_2;
        pc = get2byteAligned(&ದತ್ತಾಂಶ[cell_ಆರಂಭ+i*2]);
        ಗಾತ್ರ_2 = p_ಪುಟ->x_Cell_ಗಾತ್ರ(p_ಪುಟ, &ದತ್ತಾಂಶ[pc]);
        ಪ್ರತಿಜ್ಞಾ( ರಾಶಿ!=0 && ರಾಶಿ[0] < pCheck->mx_ರಾಶಿ );
        btree_ರಾಶಿ_ಸೇರಿಸು(ರಾಶಿ, (pc<<16)|(pc+ಗಾತ್ರ_2-1));
      }
    }
    ಪ್ರತಿಜ್ಞಾ( ರಾಶಿ!=0 );
    /* Add the freeblocks to the min-heap
    **
    ** EVIDENCE-OF: R-20690-50594 The second field of the b-tree page header
    ** is the offset of the first freeblock, or zero if there are no
    ** freeblocks on the page.
    */
    i = get2byte(&ದತ್ತಾಂಶ[hdr+1]);
    ಆಗ( i>0 ){
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2, j;
      ಪ್ರತಿಜ್ಞಾ( (u32)i<=usable_ಗಾತ್ರ-4 ); /* Enforced by btreeComputeFreeSpace() */
      ಗಾತ್ರ_2 = get2byte(&ದತ್ತಾಂಶ[i+2]);
      ಪ್ರತಿಜ್ಞಾ( (u32)(i+ಗಾತ್ರ_2)<=usable_ಗಾತ್ರ ); /* due to btreeComputeFreeSpace() */
      ಪ್ರತಿಜ್ಞಾ( ರಾಶಿ!=0 && ರಾಶಿ[0] < pCheck->mx_ರಾಶಿ );
      btree_ರಾಶಿ_ಸೇರಿಸು(ರಾಶಿ, (((u32)i)<<16)|(i+ಗಾತ್ರ_2-1));
      /* EVIDENCE-OF: R-58208-19414 The first 2 bytes of a freeblock are a
      ** big-endian integer which is the offset in the b-tree page of the next
      ** freeblock in the chain, or zero if the freeblock is the last on the
      ** chain. */
      j = get2byte(&ದತ್ತಾಂಶ[i]);
      /* EVIDENCE-OF: R-06866-39125 Freeblocks are always connected in order of
      ** increasing offset. */
      ಪ್ರತಿಜ್ಞಾ( j==0 || j>i+ಗಾತ್ರ_2 );     /* Enforced by btreeComputeFreeSpace() */
      ಪ್ರತಿಜ್ಞಾ( (u32)j<=usable_ಗಾತ್ರ-4 ); /* Enforced by btreeComputeFreeSpace() */
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
    prev = content_ಸ್ಥಾನಾಂತರ - 1;   /* Implied first min-heap entry */
    ಆಗ( btree_ರಾಶಿ_Pull(ರಾಶಿ,&x) ){
      ಆದರೆ( (prev&0xffff)>=(x>>16) ){
        check_Append_ಸಂದೇಶ(pCheck,
          "Multiple uses for byte %u of page %u", x>>16, i_ಪುಟ);
        ನಿಲ್ಲಿಸು;
      }ಇಲ್ಲದಿದ್ದರೆ{
        nFrag += (x>>16) - (prev&0xffff) - 1;
        prev = x;
      }
    }
    nFrag += usable_ಗಾತ್ರ - (prev&0xffff) - 1;
    /* EVIDENCE-OF: R-43263-13491 The total number of bytes in all fragments
    ** is stored in the fifth field of the b-tree page header.
    ** EVIDENCE-OF: R-07161-27322 The one-byte integer at offset 7 gives the
    ** number of fragmented free bytes within the cell content area.
    */
    ಆದರೆ( ರಾಶಿ[0]==0 && nFrag!=ದತ್ತಾಂಶ[hdr+7] ){
      check_Append_ಸಂದೇಶ(pCheck,
          "Fragmentation of %u bytes reported as %u on page %u",
          nFrag, ದತ್ತಾಂಶ[hdr+7], i_ಪುಟ);
    }
  }

ಕೊನೆ_ದ_check:
  ಆದರೆ( !doCoverageCheck ) p_ಪುಟ->isInit = savedIsInit;
  release_ಪುಟ(p_ಪುಟ);
  pCheck->zPfx = saved_zPfx;
  pCheck->v1 = saved_v1;
  pCheck->v2 = saved_v2;
  pCheck->nAbove--;
  ಹಿಂತಿರುಗಿ ಆಳ+1;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_INTEGRITY_CHECK */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INTEGRITY_CHECK
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
ಪೂರ್ಣಾಂಕ sqlite3BtreeIntegrityCheck(
  sqlite3 *db,  /* Database connection that is running the check */
  Btree *p,     /* The btree to be checked */
  Pgno *a_ಬೇರು,  /* An array of root pages numbers for individual trees */
  Mem *aCnt,    /* Memory cells to write counts for each tree to */
  ಪೂರ್ಣಾಂಕ n_ಬೇರು,    /* Number of entries in aRoot[] */
  ಪೂರ್ಣಾಂಕ mxErr,    /* Stop reporting errors after this many */
  ಪೂರ್ಣಾಂಕ *pnErr,   /* OUT: Write number of errors seen to this variable */
  ಅಕ್ಷರ **pz_ಹೊರಹೋಗು  /* OUT: Write the error message string here */
){
  Pgno i;
  IntegrityCk sCheck;
  BtShared *pBt = p->pBt;
  u64 savedDbFlags = pBt->db->flags;
  ಅಕ್ಷರ zErr[100];
  ಪೂರ್ಣಾಂಕ bPartial = 0;            /* True if not checking all btrees */
  ಪೂರ್ಣಾಂಕ bCkFreelist = 1;         /* True to scan the freelist */
  VVA_ONLY( ಪೂರ್ಣಾಂಕ nRef );

  ಪ್ರತಿಜ್ಞಾ( n_ಬೇರು>0 );
  ಪ್ರತಿಜ್ಞಾ( aCnt!=0 );

  /* aRoot[0]==0 means this is a partial check */
  ಆದರೆ( a_ಬೇರು[0]==0 ){
    ಪ್ರತಿಜ್ಞಾ( n_ಬೇರು>1 );
    bPartial = 1;
    ಆದರೆ( a_ಬೇರು[1]!=1 ) bCkFreelist = 0;
  }

  sqlite3BtreeEnter(p);
  ಪ್ರತಿಜ್ಞಾ( p->ಒಳ_Trans>TRANS_NONE && pBt->ಒಳ_Transaction>TRANS_NONE );
  VVA_ONLY( nRef = sqlite3PagerRefcount(pBt->pPager) );
  ಪ್ರತಿಜ್ಞಾ( nRef>=0 );
  memset(&sCheck, 0, ಗಾತ್ರ(sCheck));
  sCheck.db = db;
  sCheck.pBt = pBt;
  sCheck.pPager = pBt->pPager;
  sCheck.n_Ck_ಪುಟ = btreePagecount(sCheck.pBt);
  sCheck.mxErr = mxErr;
  sqlite3StrAccumInit(&sCheck.err_ಸಂದೇಶ, 0, zErr, ಗಾತ್ರ(zErr), SQLITE_MAX_ಉದ್ದಳತೆ);
  sCheck.err_ಸಂದೇಶ.printfFlags = SQLITE_PRINTF_ಆಂತರಿಕ;
  ಆದರೆ( sCheck.n_Ck_ಪುಟ==0 ){
    ಹೋಗು integrity_ck_cleanup;
  }

  sCheck.aPgRef = sqlite3MallocZero((sCheck.n_Ck_ಪುಟ / 8)+ 1);
  ಆದರೆ( !sCheck.aPgRef ){
    checkOom(&sCheck);
    ಹೋಗು integrity_ck_cleanup;
  }
  sCheck.ರಾಶಿ = (u32*)sqlite3Malloc( pBt->ಪುಟ_ಗಾತ್ರ*2 );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  sCheck.mx_ರಾಶಿ = pBt->ಪುಟ_ಗಾತ್ರ/2 - 1;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( sCheck.ರಾಶಿ==0 ){
    checkOom(&sCheck);
    ಹೋಗು integrity_ck_cleanup;
  }

  i = PENDING_BYTE_ಪುಟ(pBt);
  ಆದರೆ( i<=sCheck.n_Ck_ಪುಟ ) ಗಣ_ಪುಟ_Referenced(&sCheck, i);

  /* Check the integrity of the freelist
  */
  ಆದರೆ( bCkFreelist ){
    sCheck.zPfx = "Freelist: ";
    checkList(&sCheck, 1, get4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[32]),
              get4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[36]));
    sCheck.zPfx = 0;
  }

  /* Check all the tables.
  */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
  ಆದರೆ( !bPartial ){
    ಆದರೆ( pBt->autoVacuum ){
      Pgno mx = 0;
      Pgno mx_ಒಳ_Hdr;
      ಗಾಗಿ(i=0; (ಪೂರ್ಣಾಂಕ)i<n_ಬೇರು; i++) ಆದರೆ( mx<a_ಬೇರು[i] ) mx = a_ಬೇರು[i];
      mx_ಒಳ_Hdr = get4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[52]);
      ಆದರೆ( mx!=mx_ಒಳ_Hdr ){
        check_Append_ಸಂದೇಶ(&sCheck,
          "max rootpage (%u) disagrees with header (%u)",
          mx, mx_ಒಳ_Hdr
        );
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( get4byte(&pBt->pPage1->a_ದತ್ತಾಂಶ[64])!=0 ){
      check_Append_ಸಂದೇಶ(&sCheck,
        "incremental_vacuum enabled with a max rootpage of zero"
      );
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  testcase( pBt->db->flags & SQLITE_Cell_ಗಾತ್ರ_Ck );
  pBt->db->flags &= ~(u64)SQLITE_Cell_ಗಾತ್ರ_Ck;
  ಗಾಗಿ(i=0; (ಪೂರ್ಣಾಂಕ)i<n_ಬೇರು && sCheck.mxErr; i++){
    sCheck.n_ಅಡ್ಡಸಾಲು = 0;
    ಆದರೆ( a_ಬೇರು[i] ){
      i64 not_ಬಳಸಿದ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTOVACUUM
      ಆದರೆ( pBt->autoVacuum && a_ಬೇರು[i]>1 && !bPartial ){
        checkPtrmap(&sCheck, a_ಬೇರು[i], PTRMAP_ROOTPAGE, 0);
      }
#ಆದರೆ_ಅಂತ್ಯ
      sCheck.v0 = a_ಬೇರು[i];
      ಪ್ರತಿಜ್ಞಾ( sCheck.nAbove==0 );
      check_ವೃಕ್ಷ_ಪುಟ(&sCheck, a_ಬೇರು[i], &not_ಬಳಸಿದ, ಅತಿದೊಡ್ಡ_INT64);
      ಪ್ರತಿಜ್ಞಾ( sCheck.nAbove==0 );
    }
    sqlite3_Mem_ಗಣ_Array_Int64(aCnt, i, sCheck.n_ಅಡ್ಡಸಾಲು);
  }
  pBt->db->flags = savedDbFlags;

  /* Make sure every page in the file is referenced
  */
  ಆದರೆ( !bPartial ){
    ಗಾಗಿ(i=1; i<=sCheck.n_Ck_ಪುಟ && sCheck.mxErr; i++){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_OMIT_AUTOVACUUM
      ಆದರೆ( get_ಪುಟ_Referenced(&sCheck, i)==0 ){
        check_Append_ಸಂದೇಶ(&sCheck, "Page %u: never used", i);
      }
#ಇಲ್ಲದಿದ್ದರೆ
      /* If the database supports auto-vacuum, make sure no tables contain
      ** references to pointer-map pages.
      */
      ಆದರೆ( get_ಪುಟ_Referenced(&sCheck, i)==0 &&
         (PTRMAP_PAGENO(pBt, i)!=i || !pBt->autoVacuum) ){
        check_Append_ಸಂದೇಶ(&sCheck, "Page %u: never used", i);
      }
      ಆದರೆ( get_ಪುಟ_Referenced(&sCheck, i)!=0 &&
         (PTRMAP_PAGENO(pBt, i)==i && pBt->autoVacuum) ){
        check_Append_ಸಂದೇಶ(&sCheck, "Page %u: pointer map referenced", i);
      }
#ಆದರೆ_ಅಂತ್ಯ
    }
  }

  /* Clean  up and report errors.
  */
integrity_ck_cleanup:
  sqlite3_free(sCheck.ರಾಶಿ);
  sqlite3_free(sCheck.aPgRef);
  *pnErr = sCheck.nErr;
  ಆದರೆ( sCheck.nErr==0 ){
    sqlite3_str_reset(&sCheck.err_ಸಂದೇಶ);
    *pz_ಹೊರಹೋಗು = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    *pz_ಹೊರಹೋಗು = sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&sCheck.err_ಸಂದೇಶ);
  }
  /* Make sure this analysis did not leave any unref() pages. */
  ಪ್ರತಿಜ್ಞಾ( nRef==sqlite3PagerRefcount(pBt->pPager) );
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ sCheck.rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_INTEGRITY_CHECK */

/*
** Return the full pathname of the underlying database file.  Return
** an empty string if the database is in-memory or a TEMP database.
**
** The pager filename is invariant as long as the pager is
** open so it is safe to access without the BtShared mutex.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3BtreeGetFilename(Btree *p){
  ಪ್ರತಿಜ್ಞಾ( p->pBt->pPager!=0 );
  ಹಿಂತಿರುಗಿ sqlite3PagerFilename(p->pBt->pPager, 1);
}

/*
** Return the pathname of the journal file for this database. The return
** value of this routine is the same regardless of whether the journal file
** has been created or not.
**
** The pager journal filename is invariant as long as the pager is
** open so it is safe to access without the BtShared mutex.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3BtreeGetJournalname(Btree *p){
  ಪ್ರತಿಜ್ಞಾ( p->pBt->pPager!=0 );
  ಹಿಂತಿರುಗಿ sqlite3PagerJournalname(p->pBt->pPager);
}

/*
** Return one of SQLITE_TXN_NONE, SQLITE_TXN_READ, or SQLITE_TXN_WRITE
** to describe the current transaction state of Btree p.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeTxnState(Btree *p){
  ಪ್ರತಿಜ್ಞಾ( p==0 || sqlite3_mutex_held(p->db->mutex) );
  ಹಿಂತಿರುಗಿ p ? p->ಒಳ_Trans : 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
/*
** Run a checkpoint on the Btree passed as the first argument.
**
** Return SQLITE_LOCKED if this or any other connection has an open
** transaction on the shared-cache the argument Btree is connected to.
**
** Parameter eMode is one of SQLITE_CHECKPOINT_PASSIVE, FULL or RESTART.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeCheckpoint(Btree *p, ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ, ಪೂರ್ಣಾಂಕ *pnLog, ಪೂರ್ಣಾಂಕ *pnCkpt){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( p ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
    ಆದರೆ( pBt->ಒಳ_Transaction!=TRANS_NONE ){
      rc = SQLITE_LOCKED;
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = sqlite3PagerCheckpoint(pBt->pPager, p->db, e_ಬಹುಲಕ, pnLog, pnCkpt);
    }
    sqlite3BtreeLeave(p);
  }
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Return true if there is currently a backup running on Btree p.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Is_ಒಳ_Backup(Btree *p){
  ಪ್ರತಿಜ್ಞಾ( p );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->db->mutex) );
  ಹಿಂತಿರುಗಿ p->nBackup!=0;
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
ಶೂನ್ಯ *sqlite3BtreeSchema(Btree *p, ಪೂರ್ಣಾಂಕ n_ಬೈಟುಗಳು, ಶೂನ್ಯ(*xFree)(ಶೂನ್ಯ *)){
  BtShared *pBt = p->pBt;
  ಪ್ರತಿಜ್ಞಾ( n_ಬೈಟುಗಳು==0 || n_ಬೈಟುಗಳು==ಗಾತ್ರ(Schema) );
  sqlite3BtreeEnter(p);
  ಆದರೆ( !pBt->pSchema && n_ಬೈಟುಗಳು ){
    pBt->pSchema = sqlite3DbMallocZero(0, n_ಬೈಟುಗಳು);
    pBt->xFreeSchema = xFree;
  }
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ pBt->pSchema;
}

/*
** Return SQLITE_LOCKED_SHAREDCACHE if another user of the same shared
** btree as the argument handle holds an exclusive lock on the
** sqlite_schema table. Otherwise SQLITE_OK.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeSchemaLocked(Btree *p){
  ಪೂರ್ಣಾಂಕ rc;
  ಬಳಸದ_PARAMETER(p);  /* only used in DEBUG builds */
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  rc = query_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(p, SCHEMA_ಬೇರು, READ_ಬೀಗ);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || rc==SQLITE_LOCKED_SHAREDCACHE );
  sqlite3BtreeLeave(p);
  ಹಿಂತಿರುಗಿ rc;
}


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
/*
** Obtain a lock on the table whose root page is iTab.  The
** lock is a write lock if isWritelock is true or a read lock
** if it is false.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಬೀಗ_ಕೋಷ್ಟಕ(Btree *p, ಪೂರ್ಣಾಂಕ iTab, u8 is_Write_ಬೀಗ){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪ್ರತಿಜ್ಞಾ( p->ಒಳ_Trans!=TRANS_NONE );
  ಆದರೆ( p->sharable ){
    u8 ಬೀಗ_Type = READ_ಬೀಗ + is_Write_ಬೀಗ;
    ಪ್ರತಿಜ್ಞಾ( READ_ಬೀಗ+1==WRITE_ಬೀಗ );
    ಪ್ರತಿಜ್ಞಾ( is_Write_ಬೀಗ==0 || is_Write_ಬೀಗ==1 );

    sqlite3BtreeEnter(p);
    rc = query_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(p, iTab, ಬೀಗ_Type);
    ಆದರೆ( rc==SQLITE_OK ){
      rc = ಗಣ_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(p, iTab, ಬೀಗ_Type);
    }
    sqlite3BtreeLeave(p);
  }
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB
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
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Put_ದತ್ತಾಂಶ(BtCursor *pCsr, u32 ಸ್ಥಾನಾಂತರ, u32 amt, ಶೂನ್ಯ *z){
  ಪೂರ್ಣಾಂಕ rc;
  ಪ್ರತಿಜ್ಞಾ( cursorOwnsBtShared(pCsr) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCsr->pBtree->db->mutex) );
  ಪ್ರತಿಜ್ಞಾ( pCsr->curFlags & BTCF_Incrblob );

  rc = restoreCursorPosition(pCsr);
  ಆದರೆ( rc!=SQLITE_OK ){
    ಹಿಂತಿರುಗಿ rc;
  }
  ಪ್ರತಿಜ್ಞಾ( pCsr->eState!=CURSOR_REQUIRESEEK );
  ಆದರೆ( pCsr->eState!=CURSOR_VALID ){
    ಹಿಂತಿರುಗಿ SQLITE_ABORT;
  }

  /* Save the positions of all other cursors open on this table. This is
  ** required in case any of them are holding references to an xFetch
  ** version of the b-tree page modified by the accessPayload call below.
  **
  ** Note that pCsr must be open on a INTKEY table and saveCursorPosition()
  ** and hence saveAllCursors() cannot fail on a BTREE_INTKEY table, hence
  ** saveAllCursors can only return SQLITE_OK.
  */
  VVA_ONLY(rc =) saveAllCursors(pCsr->pBt, pCsr->pgno_ಬೇರು, pCsr);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );

  /* Check some assumptions:
  **   (a) the cursor is open for writing,
  **   (b) there is a read/write transaction open,
  **   (c) the connection holds a write-lock on the table (if required),
  **   (d) there are no conflicting read-locks, and
  **   (e) the cursor points at a valid row of an intKey table.
  */
  ಆದರೆ( (pCsr->curFlags & BTCF_WriteFlag)==0 ){
    ಹಿಂತಿರುಗಿ SQLITE_READONLY;
  }
  ಪ್ರತಿಜ್ಞಾ( (pCsr->pBt->btsFlags & BTS_READ_ONLY)==0
              && pCsr->pBt->ಒಳ_Transaction==TRANS_WRITE );
  ಪ್ರತಿಜ್ಞಾ( has_Shared_Cache_ಕೋಷ್ಟಕ_ಬೀಗ(pCsr->pBtree, pCsr->pgno_ಬೇರು, 0, 2) );
  ಪ್ರತಿಜ್ಞಾ( !hasReadConflicts(pCsr->pBtree, pCsr->pgno_ಬೇರು) );
  ಪ್ರತಿಜ್ಞಾ( pCsr->p_ಪುಟ->int_ಕೀಲಿ );

  ಹಿಂತಿರುಗಿ accessPayload(pCsr, ಸ್ಥಾನಾಂತರ, amt, (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *)z, 1);
}

/*
** Mark this cursor as an incremental blob cursor.
*/
ಶೂನ್ಯ sqlite3BtreeIncrblobCursor(BtCursor *pCur){
  pCur->curFlags |= BTCF_Incrblob;
  pCur->pBtree->hasIncrblobCur = 1;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Set both the "read version" (single byte at byte offset 18) and
** "write version" (single byte at byte offset 19) fields in the database
** header to iVersion.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_ಆವೃತ್ತಿ(Btree *pBtree, ಪೂರ್ಣಾಂಕ i_ಆವೃತ್ತಿ){
  BtShared *pBt = pBtree->pBt;
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code */

  ಪ್ರತಿಜ್ಞಾ( i_ಆವೃತ್ತಿ==1 || i_ಆವೃತ್ತಿ==2 );

  /* If setting the version fields to 1, do not automatically open the
  ** WAL connection, even if the version fields are currently set to 2.
  */
  pBt->btsFlags &= ~BTS_NO_WAL;
  ಆದರೆ( i_ಆವೃತ್ತಿ==1 ) pBt->btsFlags |= BTS_NO_WAL;

  rc = sqlite3BtreeBeginTrans(pBtree, 0, 0);
  ಆದರೆ( rc==SQLITE_OK ){
    u8 *a_ದತ್ತಾಂಶ = pBt->pPage1->a_ದತ್ತಾಂಶ;
    ಆದರೆ( a_ದತ್ತಾಂಶ[18]!=(u8)i_ಆವೃತ್ತಿ || a_ದತ್ತಾಂಶ[19]!=(u8)i_ಆವೃತ್ತಿ ){
      rc = sqlite3BtreeBeginTrans(pBtree, 2, 0);
      ಆದರೆ( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ಪುಟ);
        ಆದರೆ( rc==SQLITE_OK ){
          a_ದತ್ತಾಂಶ[18] = (u8)i_ಆವೃತ್ತಿ;
          a_ದತ್ತಾಂಶ[19] = (u8)i_ಆವೃತ್ತಿ;
        }
      }
    }
  }

  pBt->btsFlags &= ~BTS_NO_WAL;
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return true if the cursor has a hint specified.  This routine is
** only used from within assert() statements
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeCursorHasHint(BtCursor *pCsr, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ){
  ಹಿಂತಿರುಗಿ (pCsr->hints & ಮುಖವಾಡ)!=0;
}

/*
** Return true if the given Btree is read-only.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeIsReadonly(Btree *p){
  ಹಿಂತಿರುಗಿ (p->pBt->btsFlags & BTS_READ_ONLY)!=0;
}

/*
** Return the size of the header added to each page by this module.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ_Btree(ಶೂನ್ಯ){ ಹಿಂತಿರುಗಿ ROUND8(ಗಾತ್ರ(Mem_ಪುಟ)); }

/*
** If no transaction is active and the database is not a temp-db, clear
** the in-memory pager cache.
*/
ಶೂನ್ಯ sqlite3BtreeClearCache(Btree *p){
  BtShared *pBt = p->pBt;
  ಆದರೆ( pBt->ಒಳ_Transaction==TRANS_NONE ){
    sqlite3PagerClearCache(pBt->pPager);
  }
}

#ಆದರೆ !defined(SQLITE_OMIT_SHARED_CACHE)
/*
** Return true if the Btree passed as the only argument is sharable.
*/
ಪೂರ್ಣಾಂಕ sqlite3BtreeSharable(Btree *p){
  ಹಿಂತಿರುಗಿ p->sharable;
}

/*
** Return the number of connections to the BtShared object accessed by
** the Btree handle passed as the only argument. For private caches
** this is always 1. For shared caches it may be 1 or greater.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Connection_ಎಣಿಕೆ(Btree *p){
  testcase( p->sharable );
  ಹಿಂತಿರುಗಿ p->pBt->nRef;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Re-enable GCC compiler warnings that were suppressed at the top
** of this source file to prevent annoying false-positives.
*/
#ಆದರೆ defined(__GNUC__) && __GNUC__>=11
# pragma GCC diagnostic ತೆಗೆ
#ಆದರೆ_ಅಂತ್ಯ
