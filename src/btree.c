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
#شامل "btreeInt.h"

/*
** Suppress false-positive compiler warnings from GCC.  Warnings are
** re-enabled at the bottom of this source file.
*/
#اگر defined(__GNUC__) && __GNUC__>=11
# pragma GCC diagnostic ڈالو
# pragma GCC diagnostic ignored "-Wstringop-overread"
# pragma GCC diagnostic ignored "-Wstringop-overflow"
#اگر_ختم

/*
** The header string that appears at the beginning of every
** SQLite database.
*/
ساکن ثابت حرف z_Magic_سرخی[] = SQLITE_FILE_سرخی;

/*
** Set this global variable to 1 to enable tracing using the TRACE
** macro.
*/
#اگر 0
صحیح sqlite3BtreeTrace=1;  /* True to enable tracing */
# define TRACE(X)  اگر(sqlite3BtreeTrace){لکھو X;fflush(stdout);}
#ورنہ
# define TRACE(X)
#اگر_ختم

/*
** Extract a 2-byte big-endian integer from an array of unsigned bytes.
** But if the value is zero, make it 65536.
**
** This routine is used to extract the "offset to cell content area" value
** from the header of a btree page.  If the page size is 65536 and the page
** is empty, the offset should be 65536, but the 2-byte value stores zero.
** This routine makes the necessary adjustment to 65536.
*/
#تعریف_کرو get2byteNotZero(X)  (((((صحیح)get2byte(X))-1)&0xffff)+1)

/*
** Values passed as the 5th argument to allocateBtreePage()
*/
#تعریف_کرو BTALLOC_ANY   0           /* Allocate any page */
#تعریف_کرو BTALLOC_EXACT 1           /* Allocate exact page if possible */
#تعریف_کرو BTALLOC_LE    2           /* Allocate any page <= the parameter */

/*
** Macro IfNotOmitAV(x) returns (x) if SQLITE_OMIT_AUTOVACUUM is not
** defined, or 0 if it is. For example:
**
**   bIncrVacuum = IfNotOmitAV(pBtShared->incrVacuum);
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
#تعریف_کرو IfNotOmitAV(اظہاریہ_2) (اظہاریہ_2)
#ورنہ
#تعریف_کرو IfNotOmitAV(اظہاریہ_2) 0
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
/*
** A list of BtShared objects that are eligible for participation
** in shared cache.  This variable has file scope during normal builds,
** but the test harness needs to access it so we make it global for
** test builds.
**
** Access to this variable is protected by SQLITE_MUTEX_STATIC_MAIN.
*/
#اگر_تعریف_شدہ SQLITE_TEST
BtShared *SQLITE_WSD sqlite3SharedCacheList = 0;
#ورنہ
ساکن BtShared *SQLITE_WSD sqlite3SharedCacheList = 0;
#اگر_ختم
#اگر_ختم /* SQLITE_OMIT_SHARED_CACHE */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
/*
** Enable or disable the shared pager and schema features.
**
** This routine has no effect on existing database connections.
** The shared cache setting effects only future calls to
** sqlite3_open(), sqlite3_open16(), or sqlite3_open_v2().
*/
صحیح sqlite3_enable_shared_cache(صحیح enable){
  sqlite3GlobalConfig.sharedCacheEnabled = enable;
  واپس SQLITE_OK;
}
#اگر_ختم



#اگر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
  /*
  ** The functions querySharedCacheTableLock(), setSharedCacheTableLock(),
  ** and clearAllSharedCacheTableLocks()
  ** manipulate entries in the BtShared.pLock linked list used to store
  ** shared-cache table level locks. If the library is compiled with the
  ** shared-cache feature disabled, then there is only ever one user
  ** of each BtShared structure and so this locking is not necessary.
  ** So define the lock related functions as no-ops.
  */
  #تعریف_کرو query_Shared_Cache_جدول_تالا(a,b,c) SQLITE_OK
  #تعریف_کرو سیٹ_Shared_Cache_جدول_تالا(a,b,c) SQLITE_OK
  #تعریف_کرو clear_All_Shared_Cache_جدول_Locks(a)
  #تعریف_کرو downgrade_All_Shared_Cache_جدول_Locks(a)
  #تعریف_کرو has_Shared_Cache_جدول_تالا(a,b,c,d) 1
  #تعریف_کرو hasReadConflicts(a, b) 0
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** Return and reset the seek counter for a Btree object.
*/
sqlite3_uint64 sqlite3_Btree_Seek_گنتی(Btree *pBt){
  u64 n =  pBt->nSeek;
  pBt->nSeek = 0;
  واپس n;
}
#اگر_ختم

/*
** Implementation of the SQLITE_CORRUPT_PAGE() macro. Takes a single
** (MemPage*) as an argument. The (MemPage*) must not be NULL.
**
** If SQLITE_DEBUG is not defined, then this macro is equivalent to
** SQLITE_CORRUPT_BKPT. Or, if SQLITE_DEBUG is set, then the log message
** normally produced as a side-effect of SQLITE_CORRUPT_BKPT is augmented
** with the page number and filename associated with the (MemPage*).
*/
#اگر_تعریف_شدہ SQLITE_DEBUG
صحیح corrupt_صفحہ_Error(صحیح lineno, Mem_صفحہ *p){
  حرف *z_پیغام;
  sqlite3BeginBenignMalloc();
  z_پیغام = sqlite3_mprintf("database corruption page %u of %s",
             p->pgno, sqlite3PagerFilename(p->pBt->pPager, 0)
  );
  sqlite3_انتہا_Benign_Malloc();
  اگر( z_پیغام ){
    sqlite3ReportError(SQLITE_CORRUPT, lineno, z_پیغام);
  }
  sqlite3_free(z_پیغام);
  واپس SQLITE_CORRUPT_BKPT;
}
# define SQLITE_CORRUPT_صفحہ(p_Mem_صفحہ) corrupt_صفحہ_Error(__LINE__, p_Mem_صفحہ)
#ورنہ
# define SQLITE_CORRUPT_صفحہ(p_Mem_صفحہ) SQLITE_CORRUPT_PGNO(p_Mem_صفحہ->pgno)
#اگر_ختم

/* Default value for SHARED_LOCK_TRACE macro if shared-cache is disabled
** or if the lock tracking is disabled.  This is always the value for
** release builds.
*/
#تعریف_کرو SHARED_تالا_TRACE(X,پیغام,TAB,TYPE)  /*no-op*/

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE

#اگر 0
/*  ^----  Change to 1 and recompile to enable shared-lock tracing
**         for debugging purposes.
**
** Print all shared-cache locks on a BtShared.  Debugging use only.
*/
ساکن خالی shared_تالا_Trace(
  BtShared *pBt,
  ثابت حرف *z_پیغام,
  صحیح i_جڑ,
  صحیح e_تالا_Type
){
  Bt_تالا *p_تالا;
  اگر( i_جڑ>0 ){
    لکھو("%s-%p %u%s:", z_پیغام, pBt, i_جڑ, e_تالا_Type==READ_تالا?"R":"W");
  }ورنہ{
    لکھو("%s-%p:", z_پیغام, pBt);
  }
  برائے(p_تالا=pBt->p_تالا; p_تالا; p_تالا=p_تالا->p_اگلا){
    لکھو(" %p/%u%s", p_تالا->pBtree, p_تالا->i_جدول,
           p_تالا->e_تالا==READ_تالا ? "R" : "W");
    جب( p_تالا->p_اگلا && p_تالا->pBtree==p_تالا->p_اگلا->pBtree ){
      p_تالا = p_تالا->p_اگلا;
      لکھو(",%u%s", p_تالا->i_جدول, p_تالا->e_تالا==READ_تالا ? "R" : "W");
    }
  }
  لکھو("\n");
  fflush(stdout);
}
#تعریف_منسوخ SHARED_تالا_TRACE
#تعریف_کرو SHARED_تالا_TRACE(X,پیغام,TAB,TYPE)  shared_تالا_Trace(X,پیغام,TAB,TYPE)
#اگر_ختم /* Shared-lock tracing */

#اگر_تعریف_شدہ SQLITE_DEBUG
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
ساکن صحیح has_Shared_Cache_جدول_تالا(
  Btree *pBtree,         /* Handle that must hold lock */
  Pgno i_جڑ,            /* Root page of b-tree */
  صحیح is_اشاریہ,           /* True if iRoot is the root of an index b-tree */
  صحیح e_تالا_Type          /* Required lock type (READ_LOCK or WRITE_LOCK) */
){
  Schema *pSchema = (Schema *)pBtree->pBt->pSchema;
  Pgno iTab = 0;
  Bt_تالا *p_تالا;

  /* If this database is not shareable, or if the client is reading
  ** and has the read-uncommitted flag set, then no lock is required.
  ** Return true immediately.
  */
  اگر( (pBtree->sharable==0)
   || (e_تالا_Type==READ_تالا && (pBtree->db->flags & SQLITE_ReadUncommit))
  ){
    واپس 1;
  }

  /* If the client is reading  or writing an index and the schema is
  ** not loaded, then it is too difficult to actually check to see if
  ** the correct locks are held.  So do not bother - just return true.
  ** This case does not come up very often anyhow.
  */
  اگر( is_اشاریہ && (!pSchema || (pSchema->schemaFlags&DB_SchemaLoaded)==0) ){
    واپس 1;
  }

  /* Figure out the root-page that the lock should be held on. For table
  ** b-trees, this is just the root page of the b-tree being read or
  ** written. For index b-trees, it is the root page of the associated
  ** table.  */
  اگر( is_اشاریہ ){
    HashElem *p;
    صحیح b_نظر_آیا = 0;
    برائے(p=sqlite_Hash_پہلا(&pSchema->idxHash); p; p=sqlite_Hash_اگلا(p)){
      اشاریہ *pIdx = (اشاریہ *)sqlite_Hash_ڈیٹا(p);
      اگر( pIdx->tnum==i_جڑ ){
        اگر( b_نظر_آیا ){
          /* Two or more indexes share the same root page.  There must
          ** be imposter tables.  So just return true.  The assert is not
          ** useful in that case. */
          واپس 1;
        }
        iTab = pIdx->p_جدول->tnum;
        b_نظر_آیا = 1;
      }
    }
  }ورنہ{
    iTab = i_جڑ;
  }

  SHARED_تالا_TRACE(pBtree->pBt,"hasLock",i_جڑ,e_تالا_Type);

  /* Search for the required lock. Either a write-lock on root-page iTab, a
  ** write-lock on the schema table, or (if the client is reading) a
  ** read-lock on iTab will suffice. Return 1 if any of these are found.  */
  برائے(p_تالا=pBtree->pBt->p_تالا; p_تالا; p_تالا=p_تالا->p_اگلا){
    اگر( p_تالا->pBtree==pBtree
     && (p_تالا->i_جدول==iTab || (p_تالا->e_تالا==WRITE_تالا && p_تالا->i_جدول==1))
     && p_تالا->e_تالا>=e_تالا_Type
    ){
      واپس 1;
    }
  }

  /* Failed to find the required lock. */
  واپس 0;
}
#اگر_ختم /* SQLITE_DEBUG */

#اگر_تعریف_شدہ SQLITE_DEBUG
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
ساکن صحیح hasReadConflicts(Btree *pBtree, Pgno i_جڑ){
  BtCursor *p;
  برائے(p=pBtree->pBt->pCursor; p; p=p->p_اگلا){
    اگر( p->pgno_جڑ==i_جڑ
     && p->pBtree!=pBtree
     && 0==(p->pBtree->db->flags & SQLITE_ReadUncommit)
    ){
      واپس 1;
    }
  }
  واپس 0;
}
#اگر_ختم    /* #ifdef SQLITE_DEBUG */

/*
** Query to see if Btree handle p may obtain a lock of type eLock
** (READ_LOCK or WRITE_LOCK) on the table with root-page iTab. Return
** SQLITE_OK if the lock may be obtained (by calling
** setSharedCacheTableLock()), or SQLITE_LOCKED if not.
*/
ساکن صحیح query_Shared_Cache_جدول_تالا(Btree *p, Pgno iTab, u8 e_تالا){
  BtShared *pBt = p->pBt;
  Bt_تالا *pIter;

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( e_تالا==READ_تالا || e_تالا==WRITE_تالا );
  أكد( p->db!=0 );
  أكد( !(p->db->flags&SQLITE_ReadUncommit)||e_تالا==WRITE_تالا||iTab==1 );
 
  /* If requesting a write-lock, then the Btree must have an open write
  ** transaction on this file. And, obviously, for this to be so there
  ** must be an open write transaction on the file itself.
  */
  أكد( e_تالا==READ_تالا || (p==pBt->pWriter && p->اندر_Trans==TRANS_WRITE) );
  أكد( e_تالا==READ_تالا || pBt->اندر_Transaction==TRANS_WRITE );
 
  /* This routine is a no-op if the shared-cache is not enabled */
  اگر( !p->sharable ){
    واپس SQLITE_OK;
  }

  /* If some other connection is holding an exclusive lock, the
  ** requested lock may not be obtained.
  */
  اگر( pBt->pWriter!=p && (pBt->btsFlags & BTS_EXCLUSIVE)!=0 ){
    sqlite3ConnectionBlocked(p->db, pBt->pWriter->db);
    واپس SQLITE_LOCKED_SHAREDCACHE;
  }

  برائے(pIter=pBt->p_تالا; pIter; pIter=pIter->p_اگلا){
    /* The condition (pIter->eLock!=eLock) in the following if(...)
    ** statement is a simplification of:
    **
    **   (eLock==WRITE_LOCK || pIter->eLock==WRITE_LOCK)
    **
    ** since we know that if eLock==WRITE_LOCK, then no other connection
    ** may hold a WRITE_LOCK on any table in this file (since there can
    ** only be a single writer).
    */
    أكد( pIter->e_تالا==READ_تالا || pIter->e_تالا==WRITE_تالا );
    أكد( e_تالا==READ_تالا || pIter->pBtree==p || pIter->e_تالا==READ_تالا);
    اگر( pIter->pBtree!=p && pIter->i_جدول==iTab && pIter->e_تالا!=e_تالا ){
      sqlite3ConnectionBlocked(p->db, pIter->pBtree->db);
      اگر( e_تالا==WRITE_تالا ){
        أكد( p==pBt->pWriter );
        pBt->btsFlags |= BTS_PENDING;
      }
      واپس SQLITE_LOCKED_SHAREDCACHE;
    }
  }
  واپس SQLITE_OK;
}
#اگر_ختم /* !SQLITE_OMIT_SHARED_CACHE */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
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
ساکن صحیح سیٹ_Shared_Cache_جدول_تالا(Btree *p, Pgno i_جدول, u8 e_تالا){
  BtShared *pBt = p->pBt;
  Bt_تالا *p_تالا = 0;
  Bt_تالا *pIter;

  SHARED_تالا_TRACE(pBt,"setLock", i_جدول, e_تالا);

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( e_تالا==READ_تالا || e_تالا==WRITE_تالا );
  أكد( p->db!=0 );

  /* A connection with the read-uncommitted flag set will never try to
  ** obtain a read-lock using this function. The only read-lock obtained
  ** by a connection in read-uncommitted mode is on the sqlite_schema
  ** table, and that lock is obtained in BtreeBeginTrans().  */
  أكد( 0==(p->db->flags&SQLITE_ReadUncommit) || e_تالا==WRITE_تالا );

  /* This function should only be called on a sharable b-tree after it
  ** has been determined that no other b-tree holds a conflicting lock.  */
  أكد( p->sharable );
  أكد( SQLITE_OK==query_Shared_Cache_جدول_تالا(p, i_جدول, e_تالا) );

  /* First search the list for an existing lock on this table. */
  برائے(pIter=pBt->p_تالا; pIter; pIter=pIter->p_اگلا){
    اگر( pIter->i_جدول==i_جدول && pIter->pBtree==p ){
      p_تالا = pIter;
      توڑو;
    }
  }

  /* If the above search did not find a BtLock struct associating Btree p
  ** with table iTable, allocate one and link it into the list.
  */
  اگر( !p_تالا ){
    p_تالا = (Bt_تالا *)sqlite3MallocZero(حجم(Bt_تالا));
    اگر( !p_تالا ){
      واپس SQLITE_NOMEM_BKPT;
    }
    p_تالا->i_جدول = i_جدول;
    p_تالا->pBtree = p;
    p_تالا->p_اگلا = pBt->p_تالا;
    pBt->p_تالا = p_تالا;
  }

  /* Set the BtLock.eLock variable to the maximum of the current lock
  ** and the requested lock. This means if a write-lock was already held
  ** and a read-lock requested, we don't incorrectly downgrade the lock.
  */
  أكد( WRITE_تالا>READ_تالا );
  اگر( e_تالا>p_تالا->e_تالا ){
    p_تالا->e_تالا = e_تالا;
  }

  واپس SQLITE_OK;
}
#اگر_ختم /* !SQLITE_OMIT_SHARED_CACHE */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
/*
** Release all the table locks (locks obtained via calls to
** the setSharedCacheTableLock() procedure) held by Btree object p.
**
** This function assumes that Btree p has an open read or write
** transaction. If it does not, then the BTS_PENDING flag
** may be incorrectly cleared.
*/
ساکن خالی clear_All_Shared_Cache_جدول_Locks(Btree *p){
  BtShared *pBt = p->pBt;
  Bt_تالا **ppIter = &pBt->p_تالا;

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( p->sharable || 0==*ppIter );
  أكد( p->اندر_Trans>0 );

  SHARED_تالا_TRACE(pBt, "clearAllLocks", 0, 0);

  جب( *ppIter ){
    Bt_تالا *p_تالا = *ppIter;
    أكد( (pBt->btsFlags & BTS_EXCLUSIVE)==0 || pBt->pWriter==p_تالا->pBtree );
    أكد( p_تالا->pBtree->اندر_Trans>=p_تالا->e_تالا );
    اگر( p_تالا->pBtree==p ){
      *ppIter = p_تالا->p_اگلا;
      أكد( p_تالا->i_جدول!=1 || p_تالا==&p->تالا );
      اگر( p_تالا->i_جدول!=1 ){
        sqlite3_free(p_تالا);
      }
    }ورنہ{
      ppIter = &p_تالا->p_اگلا;
    }
  }

  أكد( (pBt->btsFlags & BTS_PENDING)==0 || pBt->pWriter );
  اگر( pBt->pWriter==p ){
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(BTS_EXCLUSIVE|BTS_PENDING);
  }ورنہ اگر( pBt->nTransaction==2 ){
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
ساکن خالی downgrade_All_Shared_Cache_جدول_Locks(Btree *p){
  BtShared *pBt = p->pBt;

  SHARED_تالا_TRACE(pBt, "downgradeLocks", 0, 0);

  اگر( pBt->pWriter==p ){
    Bt_تالا *p_تالا;
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(BTS_EXCLUSIVE|BTS_PENDING);
    برائے(p_تالا=pBt->p_تالا; p_تالا; p_تالا=p_تالا->p_اگلا){
      أكد( p_تالا->e_تالا==READ_تالا || p_تالا->pBtree==p );
      p_تالا->e_تالا = READ_تالا;
    }
  }
}

#اگر_ختم /* SQLITE_OMIT_SHARED_CACHE */

ساکن خالی release_صفحہ(Mem_صفحہ *p_صفحہ);         /* Forward reference */
ساکن خالی release_صفحہ_One(Mem_صفحہ *p_صفحہ);      /* Forward reference */
ساکن خالی release_صفحہ_Not_Null(Mem_صفحہ *p_صفحہ);  /* Forward reference */

/*
***** This routine is used inside of assert() only ****
**
** Verify that the cursor holds the mutex on its BtShared
*/
#اگر_تعریف_شدہ SQLITE_DEBUG
ساکن صحیح cursorHoldsMutex(BtCursor *p){
  واپس sqlite3_mutex_held(p->pBt->mutex);
}

/* Verify that the cursor and the BtShared agree about what is the current
** database connetion. This is important in shared-cache mode. If the database
** connection pointers get out-of-sync, it is possible for routines like
** btreeInitPage() to reference an stale connection pointer that references a
** a connection that has already closed.  This routine is used inside assert()
** statements only and for the purpose of double-checking that the btree code
** does keep the database connection pointers up-to-date.
*/
ساکن صحیح cursorOwnsBtShared(BtCursor *p){
  أكد( cursorHoldsMutex(p) );
  واپس (p->pBtree->db==p->pBt->db);
}
#اگر_ختم

/*
** Invalidate the overflow cache of the cursor passed as the first argument.
** on the shared btree structure pBt.
*/
#تعریف_کرو invalidate_حد_سے_تجاوز_Cache(pCur) (pCur->curFlags &= ~BTCF_ValidOvfl)

/*
** Invalidate the overflow page-list cache for all cursors opened
** on the shared btree structure pBt.
*/
ساکن خالی invalidate_All_حد_سے_تجاوز_Cache(BtShared *pBt){
  BtCursor *p;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  برائے(p=pBt->pCursor; p; p=p->p_اگلا){
    invalidate_حد_سے_تجاوز_Cache(p);
  }
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
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
ساکن خالی invalidateIncrblobCursors(
  Btree *pBtree,          /* The database file to check */
  Pgno pgno_جڑ,          /* The table that might be changing */
  i64 i_صف,               /* The rowid that might be changing */
  صحیح is_Clear_جدول        /* True if all rows are being deleted */
){
  BtCursor *p;
  أكد( pBtree->hasIncrblobCur );
  أكد( sqlite3BtreeHoldsMutex(pBtree) );
  pBtree->hasIncrblobCur = 0;
  برائے(p=pBtree->pBt->pCursor; p; p=p->p_اگلا){
    اگر( (p->curFlags & BTCF_Incrblob)!=0 ){
      pBtree->hasIncrblobCur = 1;
      اگر( p->pgno_جڑ==pgno_جڑ && (is_Clear_جدول || p->info.n_کلید==i_صف) ){
        p->eState = CURSOR_INVALID;
      }
    }
  }
}

#ورنہ
  /* Stub function when INCRBLOB is omitted */
  #تعریف_کرو invalidateIncrblobCursors(w,x,y,z)
#اگر_ختم /* SQLITE_OMIT_INCRBLOB */

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
ساکن صحیح btree_سیٹ_Has_Content(BtShared *pBt, Pgno pgno){
  صحیح rc = SQLITE_OK;
  اگر( !pBt->pHasContent ){
    أكد( pgno<=pBt->n_صفحہ );
    pBt->pHasContent = sqlite3BitvecCreate(pBt->n_صفحہ);
    اگر( !pBt->pHasContent ){
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  اگر( rc==SQLITE_OK && pgno<=sqlite3_Bitvec_سائز(pBt->pHasContent) ){
    rc = sqlite3_Bitvec_سیٹ(pBt->pHasContent, pgno);
  }
  واپس rc;
}

/*
** Query the BtShared.pHasContent vector.
**
** This function is called when a free-list leaf page is removed from the
** free-list for reuse. It returns false if it is safe to retrieve the
** page from the pager layer with the 'no-content' flag set. True otherwise.
*/
ساکن صحیح btreeGetHasContent(BtShared *pBt, Pgno pgno){
  Bitvec *p = pBt->pHasContent;
  واپس p && (pgno>sqlite3_Bitvec_سائز(p) || sqlite3BitvecTestNotNull(p, pgno));
}

/*
** Clear (destroy) the BtShared.pHasContent bitvec. This should be
** invoked at the conclusion of each write-transaction.
*/
ساکن خالی btreeClearHasContent(BtShared *pBt){
  sqlite3BitvecDestroy(pBt->pHasContent);
  pBt->pHasContent = 0;
}

/*
** Release all of the apPage[] pages for a cursor.
*/
ساکن خالی btree_Release_All_Cursor_صفحات(BtCursor *pCur){
  صحیح i;
  اگر( pCur->i_صفحہ>=0 ){
    برائے(i=0; i<pCur->i_صفحہ; i++){
      release_صفحہ_Not_Null(pCur->ap_صفحہ[i]);
    }
    release_صفحہ_Not_Null(pCur->p_صفحہ);
    pCur->i_صفحہ = -1;
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
ساکن صحیح save_Cursor_کلید(BtCursor *pCur){
  صحیح rc = SQLITE_OK;
  أكد( CURSOR_VALID==pCur->eState );
  أكد( 0==pCur->p_کلید );
  أكد( cursorHoldsMutex(pCur) );

  اگر( pCur->cur_Int_کلید ){
    /* Only the rowid is required for a table btree */
    pCur->n_کلید = sqlite3_Btree_Integer_کلید(pCur);
  }ورنہ{
    /* For an index btree, save the complete key content. It is possible
    ** that the current key is corrupt. In that case, it is possible that
    ** the sqlite3VdbeRecordUnpack() function may overread the buffer by
    ** up to the size of 1 varint plus 1 8-byte value when the cursor
    ** position is restored. Hence the 17 bytes of padding allocated
    ** below. */
    خالی *p_کلید;
    pCur->n_کلید = sqlite3_Btree_Payload_سائز(pCur);
    p_کلید = sqlite3Malloc( ((i64)pCur->n_کلید) + 9 + 8 );
    اگر( p_کلید ){
      rc = sqlite3BtreePayload(pCur, 0, (صحیح)pCur->n_کلید, p_کلید);
      اگر( rc==SQLITE_OK ){
        memset(((u8*)p_کلید)+pCur->n_کلید, 0, 9+8);
        pCur->p_کلید = p_کلید;
      }ورنہ{
        sqlite3_free(p_کلید);
      }
    }ورنہ{
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  أكد( !pCur->cur_Int_کلید || !pCur->p_کلید );
  واپس rc;
}

/*
** Save the current cursor position in the variables BtCursor.nKey
** and BtCursor.pKey. The cursor's state is set to CURSOR_REQUIRESEEK.
**
** The caller must ensure that the cursor is valid (has eState==CURSOR_VALID)
** prior to calling this routine. 
*/
ساکن صحیح saveCursorPosition(BtCursor *pCur){
  صحیح rc;

  أكد( CURSOR_VALID==pCur->eState || CURSOR_SKIPNEXT==pCur->eState );
  أكد( 0==pCur->p_کلید );
  أكد( cursorHoldsMutex(pCur) );

  اگر( pCur->curFlags & BTCF_Pinned ){
    واپس SQLITE_CONSTRAINT_PINNED;
  }
  اگر( pCur->eState==CURSOR_SKIPNEXT ){
    pCur->eState = CURSOR_VALID;
  }ورنہ{
    pCur->skip_اگلا = 0;
  }

  rc = save_Cursor_کلید(pCur);
  اگر( rc==SQLITE_OK ){
    btree_Release_All_Cursor_صفحات(pCur);
    pCur->eState = CURSOR_REQUIRESEEK;
  }

  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl|BTCF_At_آخری);
  واپس rc;
}

/* Forward reference */
ساکن صحیح SQLITE_NOINLINE saveCursorsOnList(BtCursor*,Pgno,BtCursor*);

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
ساکن صحیح saveAllCursors(BtShared *pBt, Pgno i_جڑ, BtCursor *pExcept){
  BtCursor *p;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( pExcept==0 || pExcept->pBt==pBt );
  برائے(p=pBt->pCursor; p; p=p->p_اگلا){
    اگر( p!=pExcept && (0==i_جڑ || p->pgno_جڑ==i_جڑ) ) توڑو;
  }
  اگر( p ) واپس saveCursorsOnList(p, i_جڑ, pExcept);
  اگر( pExcept ) pExcept->curFlags &= ~BTCF_مضرب;
  واپس SQLITE_OK;
}

/* This helper routine to saveAllCursors does the actual work of saving
** the cursors if and when a cursor is found that actually requires saving.
** The common case is that no cursors need to be saved, so this routine is
** broken out from its caller to avoid unnecessary stack pointer movement.
*/
ساکن صحیح SQLITE_NOINLINE saveCursorsOnList(
  BtCursor *p,         /* The first cursor that needs saving */
  Pgno i_جڑ,          /* Only save cursor with this iRoot. Save all if zero */
  BtCursor *pExcept    /* Do not save this cursor */
){
  کرو{
    اگر( p!=pExcept && (0==i_جڑ || p->pgno_جڑ==i_جڑ) ){
      اگر( p->eState==CURSOR_VALID || p->eState==CURSOR_SKIPNEXT ){
        صحیح rc = saveCursorPosition(p);
        اگر( SQLITE_OK!=rc ){
          واپس rc;
        }
      }ورنہ{
        testcase( p->i_صفحہ>=0 );
        btree_Release_All_Cursor_صفحات(p);
      }
    }
    p = p->p_اگلا;
  }جب( p );
  واپس SQLITE_OK;
}

/*
** Clear the current cursor position.
*/
خالی sqlite3BtreeClearCursor(BtCursor *pCur){
  أكد( cursorHoldsMutex(pCur) );
  sqlite3_free(pCur->p_کلید);
  pCur->p_کلید = 0;
  pCur->eState = CURSOR_INVALID;
}

/*
** In this version of BtreeMoveto, pKey is a packed index record
** such as is generated by the OP_MakeRecord opcode.  Unpack the
** record and then call sqlite3BtreeIndexMoveto() to do the work.
*/
ساکن صحیح btreeMoveto(
  BtCursor *pCur,     /* Cursor open on the btree to be searched */
  ثابت خالی *p_کلید,   /* Packed key if the btree is an index */
  i64 n_کلید,           /* Integer key for tables.  Size of pKey for indices */
  صحیح bias,           /* Bias search to the high end */
  صحیح *pRes           /* Write search results here */
){
  صحیح rc;                    /* Status code */
  Unpacked_ریکارڈ *p_Idx_کلید;   /* Unpacked index key */

  اگر( p_کلید ){
    کلید_Info *p_کلید_Info = pCur->p_کلید_Info;
    أكد( n_کلید==(i64)(صحیح)n_کلید );
    p_Idx_کلید = sqlite3_Vdbe_Alloc_Unpacked_ریکارڈ(p_کلید_Info);
    اگر( p_Idx_کلید==0 ) واپس SQLITE_NOMEM_BKPT;
    sqlite3_Vdbe_ریکارڈ_Unpack((صحیح)n_کلید, p_کلید, p_Idx_کلید);
    اگر( p_Idx_کلید->n_میدان==0 || p_Idx_کلید->n_میدان>p_کلید_Info->n_All_میدان ){
      rc = SQLITE_CORRUPT_BKPT;
    }ورنہ{
      rc = sqlite3_Btree_اشاریہ_Moveto(pCur, p_Idx_کلید, pRes);
    }
    sqlite3DbFree(pCur->p_کلید_Info->db, p_Idx_کلید);
  }ورنہ{
    p_Idx_کلید = 0;
    rc = sqlite3_Btree_جدول_Moveto(pCur, n_کلید, bias, pRes);
  }
  واپس rc;
}

/*
** Restore the cursor to the position it was in (or as close to as possible)
** when saveCursorPosition() was called. Note that this call deletes the
** saved position info stored by saveCursorPosition(), so there can be
** at most one effective restoreCursorPosition() call after each
** saveCursorPosition().
*/
ساکن صحیح btreeRestoreCursorPosition(BtCursor *pCur){
  صحیح rc;
  صحیح skip_اگلا = 0;
  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState>=CURSOR_REQUIRESEEK );
  اگر( pCur->eState==CURSOR_FAULT ){
    واپس pCur->skip_اگلا;
  }
  pCur->eState = CURSOR_INVALID;
  اگر( sqlite3FaultSim(410) ){
    rc = SQLITE_IOERR;
  }ورنہ{
    rc = btreeMoveto(pCur, pCur->p_کلید, pCur->n_کلید, 0, &skip_اگلا);
  }
  اگر( rc==SQLITE_OK ){
    sqlite3_free(pCur->p_کلید);
    pCur->p_کلید = 0;
    أكد( pCur->eState==CURSOR_VALID || pCur->eState==CURSOR_INVALID );
    اگر( skip_اگلا ) pCur->skip_اگلا = skip_اگلا;
    اگر( pCur->skip_اگلا && pCur->eState==CURSOR_VALID ){
      pCur->eState = CURSOR_SKIPNEXT;
    }
  }
  واپس rc;
}

#تعریف_کرو restoreCursorPosition(p) \
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
صحیح sqlite3BtreeCursorHasMoved(BtCursor *pCur){
  أكد( EIGHT_BYTE_ALIGNMENT(pCur)
       || pCur==sqlite3BtreeFakeValidCursor() );
  أكد( offsetof(BtCursor, eState)==0 );
  أكد( حجم(pCur->eState)==1 );
  واپس CURSOR_VALID != *(u8*)pCur;
}

/*
** Return a pointer to a fake BtCursor object that will always answer
** false to the sqlite3BtreeCursorHasMoved() routine above.  The fake
** cursor returned must not be used with any other Btree interface.
*/
BtCursor *sqlite3BtreeFakeValidCursor(خالی){
  ساکن u8 fakeCursor = CURSOR_VALID;
  أكد( offsetof(BtCursor, eState)==0 );
  واپس (BtCursor*)&fakeCursor;
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
صحیح sqlite3BtreeCursorRestore(BtCursor *pCur, صحیح *p_Different_صف){
  صحیح rc;

  أكد( pCur!=0 );
  أكد( pCur->eState!=CURSOR_VALID );
  rc = restoreCursorPosition(pCur);
  اگر( rc ){
    *p_Different_صف = 1;
    واپس rc;
  }
  اگر( pCur->eState!=CURSOR_VALID ){
    *p_Different_صف = 1;
  }ورنہ{
    *p_Different_صف = 0;
  }
  واپس SQLITE_OK;
}

#اگر_تعریف_شدہ SQLITE_ENABLE_CURSOR_HINTS
/*
** Provide hints to the cursor.  The particular hint given (and the type
** and number of the varargs parameters) is determined by the eHintType
** parameter.  See the definitions of the BTREE_HINT_* macros for details.
*/
خالی sqlite3BtreeCursorHint(BtCursor *pCur, صحیح eHintType, ...){
  /* Used only by system that substitute their own storage engine */
#اگر_تعریف_شدہ SQLITE_DEBUG
  va_list ap;
  va_start(ap, eHintType);
  اگر( eHintType==BTREE_HINT_RANGE ){
    اظہاریہ *p_اظہاریہ;
    Walker w;
    memset(&w, 0, حجم(w));
    w.x_اظہاریہ_Callback = sqlite3_Cursor_Range_Hint_اظہاریہ_Check;
    p_اظہاریہ = va_arg(ap, اظہاریہ*);
    w.u.aMem = va_arg(ap, Mem*);
    أكد( p_اظہاریہ!=0 );
    أكد( w.u.aMem!=0 );
    sqlite3_Walk_اظہاریہ(&w, p_اظہاریہ);
  }ورنہ اگر( ALWAYS(eHintType==BTREE_HINT_TABLECURSOR) ){
    BtCursor *pCsr = va_arg(ap, BtCursor*);
    أكد( pCur->p_Cursor_Hint_جدول_Cursor==0 
         || pCur->p_Cursor_Hint_جدول_Cursor==pCsr
    );
    أكد( pCsr->p_کلید_Info==0 || CORRUPT_DB );
    pCur->p_Cursor_Hint_جدول_Cursor = pCsr;
  }
  va_end(ap);
#اگر_ختم /* SQLITE_DEBUG */
}
#اگر_ختم /* SQLITE_ENABLE_CURSOR_HINTS */

#اگر defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** Return the pointer configured via the BTREE_HINT_TABLECURSOR hint on
** cursor pCsr. This is used from OP_DeferredSeek to assert() that the
** index cursor has been correctly configured with the table cursor.
*/
BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor *pCsr){
  واپس pCsr->p_Cursor_Hint_جدول_Cursor;
}
#اگر_ختم

/*
** Provide flag hints to the cursor.
*/
خالی sqlite3BtreeCursorHintFlags(BtCursor *pCur, غیرعلامتی x){
  أكد( x==BTREE_SEEK_EQ || x==BTREE_BULKLOAD || x==0 );
  pCur->hints = (u8)x;
}


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
/*
** Given a page number of a regular database page, return the page
** number for the pointer-map page that contains the entry for the
** input page number.
**
** Return 0 (not a valid page) for pgno==1 since there is
** no pointer map associated with page 1.  The integrity_check logic
** requires that ptrmapPageno(*,1)!=1.
*/
ساکن Pgno ptrmapPageno(BtShared *pBt, Pgno pgno){
  صحیح n_صفحات_فی_Map_صفحہ;
  Pgno iPtrMap, ret;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  اگر( pgno<2 ) واپس 0;
  n_صفحات_فی_Map_صفحہ = (pBt->usable_سائز/5)+1;
  iPtrMap = (pgno-2)/n_صفحات_فی_Map_صفحہ;
  ret = (iPtrMap*n_صفحات_فی_Map_صفحہ) + 2;
  اگر( ret==PENDING_BYTE_صفحہ(pBt) ){
    ret++;
  }
  واپس ret;
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
ساکن خالی ptrmapPut(BtShared *pBt, Pgno کلید, u8 eType, Pgno والد, صحیح *pRC){
  Db_صفحہ *p_Db_صفحہ;  /* The pointer map page */
  u8 *pPtrmap;      /* The pointer map data */
  Pgno iPtrmap;     /* The pointer map page number */
  صحیح انحراف;       /* Offset in pointer map page */
  صحیح rc;           /* Return code from subfunctions */

  اگر( *pRC ) واپس;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  /* The super-journal page number must never be used as a pointer map page */
  أكد( 0==PTRMAP_ISPAGE(pBt, PENDING_BYTE_صفحہ(pBt)) );

  أكد( pBt->autoVacuum );
  اگر( کلید==0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    واپس;
  }
  iPtrmap = PTRMAP_PAGENO(pBt, کلید);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &p_Db_صفحہ, 0);
  اگر( rc!=SQLITE_OK ){
    *pRC = rc;
    واپس;
  }
  اگر( ((حرف*)sqlite3PagerGetExtra(p_Db_صفحہ))[0]!=0 ){
    /* The first byte of the extra data is the MemPage.isInit byte.
    ** If that byte is set, it means this page is also being used
    ** as a btree page. */
    *pRC = SQLITE_CORRUPT_BKPT;
    جاؤ ptrmap_exit;
  }
  انحراف = PTRMAP_PTROFFSET(iPtrmap, کلید);
  اگر( انحراف<0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    جاؤ ptrmap_exit;
  }
  أكد( انحراف <= (صحیح)pBt->usable_سائز-5 );
  pPtrmap = (u8 *)sqlite3_Pager_Get_ڈیٹا(p_Db_صفحہ);

  اگر( eType!=pPtrmap[انحراف] || get4byte(&pPtrmap[انحراف+1])!=والد ){
    TRACE(("PTRMAP_UPDATE: %u->(%u,%u)\n", کلید, eType, والد));
    *pRC= rc = sqlite3PagerWrite(p_Db_صفحہ);
    اگر( rc==SQLITE_OK ){
      pPtrmap[انحراف] = eType;
      put4byte(&pPtrmap[انحراف+1], والد);
    }
  }

ptrmap_exit:
  sqlite3PagerUnref(p_Db_صفحہ);
}

/*
** Read an entry from the pointer map.
**
** This routine retrieves the pointer map entry for page 'key', writing
** the type and parent page number to *pEType and *pPgno respectively.
** An error code is returned if something goes wrong, otherwise SQLITE_OK.
*/
ساکن صحیح ptrmapGet(BtShared *pBt, Pgno کلید, u8 *pEType, Pgno *pPgno){
  Db_صفحہ *p_Db_صفحہ;   /* The pointer map page */
  صحیح iPtrmap;       /* Pointer map page index */
  u8 *pPtrmap;       /* Pointer map page data */
  صحیح انحراف;        /* Offset of entry in pointer map */
  صحیح rc;

  أكد( sqlite3_mutex_held(pBt->mutex) );

  iPtrmap = PTRMAP_PAGENO(pBt, کلید);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &p_Db_صفحہ, 0);
  اگر( rc!=0 ){
    واپس rc;
  }
  pPtrmap = (u8 *)sqlite3_Pager_Get_ڈیٹا(p_Db_صفحہ);

  انحراف = PTRMAP_PTROFFSET(iPtrmap, کلید);
  اگر( انحراف<0 ){
    sqlite3PagerUnref(p_Db_صفحہ);
    واپس SQLITE_CORRUPT_BKPT;
  }
  أكد( انحراف <= (صحیح)pBt->usable_سائز-5 );
  أكد( pEType!=0 );
  *pEType = pPtrmap[انحراف];
  اگر( pPgno ) *pPgno = get4byte(&pPtrmap[انحراف+1]);

  sqlite3PagerUnref(p_Db_صفحہ);
  اگر( *pEType<1 || *pEType>5 ) واپس SQLITE_CORRUPT_PGNO(iPtrmap);
  واپس SQLITE_OK;
}

#ورنہ /* if defined SQLITE_OMIT_AUTOVACUUM */
  #تعریف_کرو ptrmapPut(w,x,y,z,rc)
  #تعریف_کرو ptrmapGet(w,x,y,z) SQLITE_OK
  #تعریف_کرو ptrmapPutOvflPtr(x, y, z, rc)
#اگر_ختم

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
#تعریف_کرو ڈھونڈو_Cell(P,I) \
  ((P)->a_ڈیٹا + ((P)->نقاب_صفحہ & get2byteAligned(&(P)->aCellIdx[2*(I)])))
#تعریف_کرو ڈھونڈو_Cell_Past_Ptr(P,I) \
  ((P)->a_ڈیٹا_Ofst + ((P)->نقاب_صفحہ & get2byteAligned(&(P)->aCellIdx[2*(I)])))


/*
** This is common tail processing for btreeParseCellPtr() and
** btreeParseCellPtrIndex() for the case when the cell does not fit entirely
** on a single B-tree page.  Make necessary adjustments to the CellInfo
** structure.
*/
ساکن SQLITE_NOINLINE خالی btree_Parse_Cell_Adjust_سائز_For_حد_سے_تجاوز(
  Mem_صفحہ *p_صفحہ,         /* Page containing the cell */
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
  صحیح minLocal;  /* Minimum amount of payload held locally */
  صحیح maxLocal;  /* Maximum amount of payload held locally */
  صحیح surplus;   /* Overflow payload available for local storage */

  minLocal = p_صفحہ->minLocal;
  maxLocal = p_صفحہ->maxLocal;
  surplus = minLocal + (pInfo->nPayload - minLocal)%(p_صفحہ->pBt->usable_سائز-4);
  testcase( surplus==maxLocal );
  testcase( surplus==maxLocal+1 );
  اگر( surplus <= maxLocal ){
    pInfo->nLocal = (u16)surplus;
  }ورنہ{
    pInfo->nLocal = (u16)minLocal;
  }
  pInfo->n_سائز = (u16)(&pInfo->pPayload[pInfo->nLocal] - pCell) + 4;
}

/*
** Given a record with nPayload bytes of payload stored within btree
** page pPage, return the number of bytes of payload stored locally.
*/
ساکن صحیح btree_Payload_مقصد_Local(Mem_صفحہ *p_صفحہ, i64 nPayload){
  صحیح maxLocal;  /* Maximum amount of payload held locally */
  maxLocal = p_صفحہ->maxLocal;
  أكد( nPayload>=0 );
  اگر( nPayload<=maxLocal ){
    واپس (صحیح)nPayload;
  }ورنہ{
    صحیح minLocal;  /* Minimum amount of payload held locally */
    صحیح surplus;   /* Overflow payload available for local storage */
    minLocal = p_صفحہ->minLocal;
    surplus = (صحیح)(minLocal +(nPayload - minLocal)%(p_صفحہ->pBt->usable_سائز-4));
    واپس (surplus <= maxLocal) ? surplus : minLocal;
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
ساکن خالی btreeParseCellPtrNoPayload(
  Mem_صفحہ *p_صفحہ,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  أكد( p_صفحہ->برگ==0 );
  أكد( p_صفحہ->بچہ_Ptr_سائز==4 );
#اگر_غیر_تعریف_شدہ SQLITE_DEBUG
  غیر_مستعمل_PARAMETER(p_صفحہ);
#اگر_ختم
  pInfo->n_سائز = 4 + sqlite3GetVarint(&pCell[4], (u64*)&pInfo->n_کلید);
  pInfo->nPayload = 0;
  pInfo->nLocal = 0;
  pInfo->pPayload = 0;
  واپس;
}
ساکن خالی btreeParseCellPtr(
  Mem_صفحہ *p_صفحہ,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u64 nPayload;           /* Number of bytes of cell payload */
  u64 i_کلید;               /* Extracted Key value */

  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  أكد( p_صفحہ->برگ==0 || p_صفحہ->برگ==1 );
  أكد( p_صفحہ->int_کلید_برگ );
  أكد( p_صفحہ->بچہ_Ptr_سائز==0 );
  pIter = pCell;

  /* The next block of code is equivalent to:
  **
  **     pIter += getVarint32(pIter, nPayload);
  **
  ** The code is inlined to avoid a function call.
  */
  nPayload = *pIter;
  اگر( nPayload>=0x80 ){
    u8 *p_انتہا = &pIter[8];
    nPayload &= 0x7f;
    کرو{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }جب( (*pIter)>=0x80 && pIter<p_انتہا );
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
  i_کلید = *pIter;
  اگر( i_کلید>=0x80 ){
    u8 x;
    i_کلید = (i_کلید<<7) ^ (x = *++pIter);
    اگر( x>=0x80 ){
      i_کلید = (i_کلید<<7) ^ (x = *++pIter);
      اگر( x>=0x80 ){
        i_کلید = (i_کلید<<7) ^ 0x10204000 ^ (x = *++pIter);
        اگر( x>=0x80 ){
          i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++pIter);
          اگر( x>=0x80 ){
            i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++pIter);
            اگر( x>=0x80 ){
              i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++pIter);
              اگر( x>=0x80 ){
                i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++pIter);
                اگر( x>=0x80 ){
                  i_کلید = (i_کلید<<8) ^ 0x8000 ^ (*++pIter);
                }
              }
            }
          }
        }
      }ورنہ{
        i_کلید ^= 0x204000;
      }
    }ورنہ{
      i_کلید ^= 0x4000;
    }
  }
  pIter++;

  pInfo->n_کلید = *(i64*)&i_کلید;
  pInfo->nPayload = (u32)nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==p_صفحہ->maxLocal );
  testcase( nPayload==(u32)p_صفحہ->maxLocal+1 );
  أكد( p_صفحہ->maxLocal <= BT_MAX_LOCAL );
  اگر( nPayload<=p_صفحہ->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->n_سائز = (u16)nPayload + (u16)(pIter - pCell);
    اگر( pInfo->n_سائز<4 ) pInfo->n_سائز = 4;
    pInfo->nLocal = (u16)nPayload;
  }ورنہ{
    btree_Parse_Cell_Adjust_سائز_For_حد_سے_تجاوز(p_صفحہ, pCell, pInfo);
  }
}
ساکن خالی btree_Parse_Cell_Ptr_اشاریہ(
  Mem_صفحہ *p_صفحہ,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u32 nPayload;           /* Number of bytes of cell payload */

  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  أكد( p_صفحہ->برگ==0 || p_صفحہ->برگ==1 );
  أكد( p_صفحہ->int_کلید_برگ==0 );
  pIter = pCell + p_صفحہ->بچہ_Ptr_سائز;
  nPayload = *pIter;
  اگر( nPayload>=0x80 ){
    u8 *p_انتہا = &pIter[8];
    nPayload &= 0x7f;
    کرو{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }جب( *(pIter)>=0x80 && pIter<p_انتہا );
  }
  pIter++;
  pInfo->n_کلید = nPayload;
  pInfo->nPayload = nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==p_صفحہ->maxLocal );
  testcase( nPayload==(u32)p_صفحہ->maxLocal+1 );
  أكد( nPayload>=0 );
  أكد( p_صفحہ->maxLocal <= BT_MAX_LOCAL );
  اگر( nPayload<=p_صفحہ->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->n_سائز = (u16)nPayload + (u16)(pIter - pCell);
    اگر( pInfo->n_سائز<4 ) pInfo->n_سائز = 4;
    pInfo->nLocal = (u16)nPayload;
  }ورنہ{
    btree_Parse_Cell_Adjust_سائز_For_حد_سے_تجاوز(p_صفحہ, pCell, pInfo);
  }
}
ساکن خالی btreeParseCell(
  Mem_صفحہ *p_صفحہ,         /* Page containing the cell */
  صحیح iCell,              /* The cell index.  First cell is 0 */
  CellInfo *pInfo         /* Fill in this structure */
){
  p_صفحہ->xParseCell(p_صفحہ, ڈھونڈو_Cell(p_صفحہ, iCell), pInfo);
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
ساکن u16 cell_سائز_Ptr(Mem_صفحہ *p_صفحہ, u8 *pCell){
  u8 *pIter = pCell + 4;                   /* For looping over bytes of pCell */
  u8 *p_انتہا;                                /* End mark for a varint */
  u32 n_سائز;                               /* Size value to return */

#اگر_تعریف_شدہ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_صفحہ->xParseCell(p_صفحہ, pCell, &debuginfo);
#اگر_ختم

  أكد( p_صفحہ->بچہ_Ptr_سائز==4 );
  n_سائز = *pIter;
  اگر( n_سائز>=0x80 ){
    p_انتہا = &pIter[8];
    n_سائز &= 0x7f;
    کرو{
      n_سائز = (n_سائز<<7) | (*++pIter & 0x7f);
    }جب( *(pIter)>=0x80 && pIter<p_انتہا );
  }
  pIter++;
  testcase( n_سائز==p_صفحہ->maxLocal );
  testcase( n_سائز==(u32)p_صفحہ->maxLocal+1 );
  اگر( n_سائز<=p_صفحہ->maxLocal ){
    n_سائز += (u32)(pIter - pCell);
    أكد( n_سائز>4 );
  }ورنہ{
    صحیح minLocal = p_صفحہ->minLocal;
    n_سائز = minLocal + (n_سائز - minLocal) % (p_صفحہ->pBt->usable_سائز - 4);
    testcase( n_سائز==p_صفحہ->maxLocal );
    testcase( n_سائز==(u32)p_صفحہ->maxLocal+1 );
    اگر( n_سائز>p_صفحہ->maxLocal ){
      n_سائز = minLocal;
    }
    n_سائز += 4 + (u16)(pIter - pCell);
  }
  أكد( n_سائز==debuginfo.n_سائز || CORRUPT_DB );
  واپس (u16)n_سائز;
}
ساکن u16 cell_سائز_Ptr_Idx_برگ(Mem_صفحہ *p_صفحہ, u8 *pCell){
  u8 *pIter = pCell;                       /* For looping over bytes of pCell */
  u8 *p_انتہا;                                /* End mark for a varint */
  u32 n_سائز;                               /* Size value to return */

#اگر_تعریف_شدہ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_صفحہ->xParseCell(p_صفحہ, pCell, &debuginfo);
#اگر_ختم

  أكد( p_صفحہ->بچہ_Ptr_سائز==0 );
  n_سائز = *pIter;
  اگر( n_سائز>=0x80 ){
    p_انتہا = &pIter[8];
    n_سائز &= 0x7f;
    کرو{
      n_سائز = (n_سائز<<7) | (*++pIter & 0x7f);
    }جب( *(pIter)>=0x80 && pIter<p_انتہا );
  }
  pIter++;
  testcase( n_سائز==p_صفحہ->maxLocal );
  testcase( n_سائز==(u32)p_صفحہ->maxLocal+1 );
  اگر( n_سائز<=p_صفحہ->maxLocal ){
    n_سائز += (u32)(pIter - pCell);
    اگر( n_سائز<4 ) n_سائز = 4;
  }ورنہ{
    صحیح minLocal = p_صفحہ->minLocal;
    n_سائز = minLocal + (n_سائز - minLocal) % (p_صفحہ->pBt->usable_سائز - 4);
    testcase( n_سائز==p_صفحہ->maxLocal );
    testcase( n_سائز==(u32)p_صفحہ->maxLocal+1 );
    اگر( n_سائز>p_صفحہ->maxLocal ){
      n_سائز = minLocal;
    }
    n_سائز += 4 + (u16)(pIter - pCell);
  }
  أكد( n_سائز==debuginfo.n_سائز || CORRUPT_DB );
  واپس (u16)n_سائز;
}
ساکن u16 cell_سائز_Ptr_No_Payload(Mem_صفحہ *p_صفحہ, u8 *pCell){
  u8 *pIter = pCell + 4; /* For looping over bytes of pCell */
  u8 *p_انتہا;              /* End mark for a varint */

#اگر_تعریف_شدہ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_صفحہ->xParseCell(p_صفحہ, pCell, &debuginfo);
#ورنہ
  غیر_مستعمل_PARAMETER(p_صفحہ);
#اگر_ختم

  أكد( p_صفحہ->بچہ_Ptr_سائز==4 );
  p_انتہا = pIter + 9;
  جب( (*pIter++)&0x80 && pIter<p_انتہا );
  أكد( debuginfo.n_سائز==(u16)(pIter - pCell) || CORRUPT_DB );
  واپس (u16)(pIter - pCell);
}
ساکن u16 cell_سائز_Ptr_جدول_برگ(Mem_صفحہ *p_صفحہ, u8 *pCell){
  u8 *pIter = pCell;   /* For looping over bytes of pCell */
  u8 *p_انتہا;            /* End mark for a varint */
  u32 n_سائز;           /* Size value to return */

#اگر_تعریف_شدہ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_صفحہ->xParseCell(p_صفحہ, pCell, &debuginfo);
#اگر_ختم

  n_سائز = *pIter;
  اگر( n_سائز>=0x80 ){
    p_انتہا = &pIter[8];
    n_سائز &= 0x7f;
    کرو{
      n_سائز = (n_سائز<<7) | (*++pIter & 0x7f);
    }جب( *(pIter)>=0x80 && pIter<p_انتہا );
  }
  pIter++;
  /* pIter now points at the 64-bit integer key value, a variable length
  ** integer. The following block moves pIter to point at the first byte
  ** past the end of the key value. */
  اگر( (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80 ){ pIter++; }
  testcase( n_سائز==p_صفحہ->maxLocal );
  testcase( n_سائز==(u32)p_صفحہ->maxLocal+1 );
  اگر( n_سائز<=p_صفحہ->maxLocal ){
    n_سائز += (u32)(pIter - pCell);
    اگر( n_سائز<4 ) n_سائز = 4;
  }ورنہ{
    صحیح minLocal = p_صفحہ->minLocal;
    n_سائز = minLocal + (n_سائز - minLocal) % (p_صفحہ->pBt->usable_سائز - 4);
    testcase( n_سائز==p_صفحہ->maxLocal );
    testcase( n_سائز==(u32)p_صفحہ->maxLocal+1 );
    اگر( n_سائز>p_صفحہ->maxLocal ){
      n_سائز = minLocal;
    }
    n_سائز += 4 + (u16)(pIter - pCell);
  }
  أكد( n_سائز==debuginfo.n_سائز || CORRUPT_DB );
  واپس (u16)n_سائز;
}


#اگر_تعریف_شدہ SQLITE_DEBUG
/* This variation on cellSizePtr() is used inside of assert() statements
** only. */
ساکن u16 cell_سائز(Mem_صفحہ *p_صفحہ, صحیح iCell){
  واپس p_صفحہ->x_Cell_سائز(p_صفحہ, ڈھونڈو_Cell(p_صفحہ, iCell));
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
/*
** The cell pCell is currently part of page pSrc but will ultimately be part
** of pPage.  (pSrc and pPage are often the same.)  If pCell contains a
** pointer to an overflow page, insert an entry into the pointer-map for
** the overflow page that will be valid after pCell has been moved to pPage.
*/
ساکن خالی ptrmapPutOvflPtr(Mem_صفحہ *p_صفحہ, Mem_صفحہ *pSrc, u8 *pCell,صحیح *pRC){
  CellInfo info;
  اگر( *pRC ) واپس;
  أكد( pCell!=0 );
  p_صفحہ->xParseCell(p_صفحہ, pCell, &info);
  اگر( info.nLocal<info.nPayload ){
    Pgno ovfl;
    اگر( SQLITE_حد_سے_تجاوز(pSrc->a_ڈیٹا_انتہا, pCell, pCell+info.nLocal) ){
      testcase( pSrc!=p_صفحہ );
      *pRC = SQLITE_CORRUPT_BKPT;
      واپس;
    }
    ovfl = get4byte(&pCell[info.n_سائز-4]);
    ptrmapPut(p_صفحہ->pBt, ovfl, PTRMAP_OVERFLOW1, p_صفحہ->pgno, pRC);
  }
}
#اگر_ختم


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
ساکن صحیح defragment_صفحہ(Mem_صفحہ *p_صفحہ, صحیح nMaxFrag){
  صحیح i;                     /* Loop counter */
  صحیح pc;                    /* Address of the i-th cell */
  صحیح hdr;                   /* Offset to the page header */
  صحیح سائز;                  /* Size of a cell */
  صحیح usable_سائز;            /* Number of usable bytes on a page */
  صحیح cell_انحراف;            /* Offset to the cell pointer array */
  صحیح cbrk;                  /* Offset to the cell content area */
  صحیح nCell;                 /* Number of cells on the page */
  غیرعلامتی حرف *ڈیٹا;       /* The page data */
  غیرعلامتی حرف *عارضی;       /* Temp area for cell content */
  غیرعلامتی حرف *src;        /* Source of content */
  صحیح i_Cell_پہلا;            /* First allowable cell index */
  صحیح i_Cell_آخری;             /* Last possible cell index */
  صحیح i_Cell_آغاز;            /* First cell offset in input */

  أكد( sqlite3PagerIswriteable(p_صفحہ->p_Db_صفحہ) );
  أكد( p_صفحہ->pBt!=0 );
  أكد( p_صفحہ->pBt->usable_سائز <= SQLITE_MAX_صفحہ_سائز );
  أكد( p_صفحہ->n_حد_سے_تجاوز==0 );
  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  ڈیٹا = p_صفحہ->a_ڈیٹا;
  hdr = p_صفحہ->hdr_انحراف;
  cell_انحراف = p_صفحہ->cell_انحراف;
  nCell = p_صفحہ->nCell;
  أكد( nCell==get2byte(&ڈیٹا[hdr+3]) || CORRUPT_DB );
  i_Cell_پہلا = cell_انحراف + 2*nCell;
  usable_سائز = p_صفحہ->pBt->usable_سائز;

  /* This block handles pages with two or fewer free blocks and nMaxFrag
  ** or fewer fragmented bytes. In this case it is faster to move the
  ** two (or one) blocks of cells using memmove() and add the required
  ** offsets to each pointer in the cell-pointer array than it is to
  ** reconstruct the entire page.  */
  اگر( (صحیح)ڈیٹا[hdr+7]<=nMaxFrag ){
    صحیح iFree = get2byte(&ڈیٹا[hdr+1]);
    اگر( iFree>usable_سائز-4 ) واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    اگر( iFree ){
      صحیح iFree2 = get2byte(&ڈیٹا[iFree]);
      اگر( iFree2>usable_سائز-4 ) واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
      اگر( 0==iFree2 || (ڈیٹا[iFree2]==0 && ڈیٹا[iFree2+1]==0) ){
        u8 *p_انتہا = &ڈیٹا[cell_انحراف + nCell*2];
        u8 *pAddr;
        صحیح sz2 = 0;
        صحیح sz = get2byte(&ڈیٹا[iFree+2]);
        صحیح چوٹی = get2byte(&ڈیٹا[hdr+5]);
        اگر( چوٹی>=iFree ){
          واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
        }
        اگر( iFree2 ){
          اگر( iFree+sz>iFree2 ) واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
          sz2 = get2byte(&ڈیٹا[iFree2+2]);
          اگر( iFree2+sz2 > usable_سائز ) واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
          memmove(&ڈیٹا[iFree+sz+sz2], &ڈیٹا[iFree+sz], iFree2-(iFree+sz));
          sz += sz2;
        }ورنہ اگر( iFree+sz>usable_سائز ){
          واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
        }

        cbrk = چوٹی+sz;
        أكد( cbrk+(iFree-چوٹی) <= usable_سائز );
        memmove(&ڈیٹا[cbrk], &ڈیٹا[چوٹی], iFree-چوٹی);
        برائے(pAddr=&ڈیٹا[cell_انحراف]; pAddr<p_انتہا; pAddr+=2){
          pc = get2byte(pAddr);
          اگر( pc<iFree ){ put2byte(pAddr, pc+sz); }
          ورنہ اگر( pc<iFree2 ){ put2byte(pAddr, pc+sz2); }
        }
        جاؤ defragment_برآمد;
      }
    }
  }

  cbrk = usable_سائز;
  i_Cell_آخری = usable_سائز - 4;
  i_Cell_آغاز = get2byte(&ڈیٹا[hdr+5]);
  اگر( nCell>0 ){
    عارضی = sqlite3_Pager_عارضی_Space(p_صفحہ->pBt->pPager);
    حافظہ_نقل(عارضی, ڈیٹا, usable_سائز);
    src = عارضی;
    برائے(i=0; i<nCell; i++){
      u8 *pAddr;     /* The i-th cell pointer */
      pAddr = &ڈیٹا[cell_انحراف + i*2];
      pc = get2byte(pAddr);
      testcase( pc==i_Cell_پہلا );
      testcase( pc==i_Cell_آخری );
      /* These conditions have already been verified in btreeInitPage()
      ** if PRAGMA cell_size_check=ON.
      */
      اگر( pc>i_Cell_آخری ){
        واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
      }
      أكد( pc>=0 && pc<=i_Cell_آخری );
      سائز = p_صفحہ->x_Cell_سائز(p_صفحہ, &src[pc]);
      cbrk -= سائز;
      اگر( cbrk<i_Cell_آغاز || pc+سائز>usable_سائز ){
        واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
      }
      أكد( cbrk+سائز<=usable_سائز && cbrk>=i_Cell_آغاز );
      testcase( cbrk+سائز==usable_سائز );
      testcase( pc+سائز==usable_سائز );
      put2byte(pAddr, cbrk);
      حافظہ_نقل(&ڈیٹا[cbrk], &src[pc], سائز);
    }
  }
  ڈیٹا[hdr+7] = 0;

defragment_برآمد:
  أكد( p_صفحہ->nFree>=0 );
  اگر( ڈیٹا[hdr+7]+cbrk-i_Cell_پہلا!=p_صفحہ->nFree ){
    واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
  }
  أكد( cbrk>=i_Cell_پہلا );
  put2byte(&ڈیٹا[hdr+5], cbrk);
  ڈیٹا[hdr+1] = 0;
  ڈیٹا[hdr+2] = 0;
  memset(&ڈیٹا[i_Cell_پہلا], 0, cbrk-i_Cell_پہلا);
  أكد( sqlite3PagerIswriteable(p_صفحہ->p_Db_صفحہ) );
  واپس SQLITE_OK;
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
ساکن u8 *صفحہ_ڈھونڈو_خانہ(Mem_صفحہ *pPg, صحیح nByte, صحیح *pRc){
  ثابت صحیح hdr = pPg->hdr_انحراف;            /* Offset to page header */
  u8 * ثابت a_ڈیٹا = pPg->a_ڈیٹا;             /* Page data */
  صحیح iAddr = hdr + 1;                       /* Address of ptr to pc */
  u8 *pTmp = &a_ڈیٹا[iAddr];                  /* Temporary ptr into aData[] */
  صحیح pc = get2byte(pTmp);                   /* Address of a free slot */
  صحیح x;                                     /* Excess size of the slot */
  صحیح maxPC = pPg->pBt->usable_سائز - nByte;  /* Max address for a usable slot */
  صحیح سائز;                                  /* Size of the free slot */

  أكد( pc>0 );
  جب( pc<=maxPC ){
    /* EVIDENCE-OF: R-22710-53328 The third and fourth bytes of each
    ** freeblock form a big-endian integer which is the size of the freeblock
    ** in bytes, including the 4-byte header. */
    pTmp = &a_ڈیٹا[pc+2];
    سائز = get2byte(pTmp);
    اگر( (x = سائز - nByte)>=0 ){
      testcase( x==4 );
      testcase( x==3 );
      اگر( x<4 ){
        /* EVIDENCE-OF: R-11498-58022 In a well-formed b-tree page, the total
        ** number of bytes in fragments may not exceed 60. */
        اگر( a_ڈیٹا[hdr+7]>57 ) واپس 0;

        /* Remove the slot from the free-list. Update the number of
        ** fragmented bytes within the page. */
        حافظہ_نقل(&a_ڈیٹا[iAddr], &a_ڈیٹا[pc], 2);
        a_ڈیٹا[hdr+7] += (u8)x;
        واپس &a_ڈیٹا[pc];
      }ورنہ اگر( x+pc > maxPC ){
        /* This slot extends off the end of the usable part of the page */
        *pRc = SQLITE_CORRUPT_صفحہ(pPg);
        واپس 0;
      }ورنہ{
        /* The slot remains on the free-list. Reduce its size to account
        ** for the portion used by the new allocation. */
        put2byte(&a_ڈیٹا[pc+2], x);
      }
      واپس &a_ڈیٹا[pc + x];
    }
    iAddr = pc;
    pTmp = &a_ڈیٹا[pc];
    pc = get2byte(pTmp);
    اگر( pc<=iAddr ){
      اگر( pc ){
        /* The next slot in the chain comes before the current slot */
        *pRc = SQLITE_CORRUPT_صفحہ(pPg);
      }
      واپس 0;
    }
  }
  اگر( pc>maxPC+nByte-4 ){
    /* The free slot chain extends off the end of the page */
    *pRc = SQLITE_CORRUPT_صفحہ(pPg);
  }
  واپس 0;
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
ساکن SQLITE_INLINE صحیح allocateSpace(Mem_صفحہ *p_صفحہ, صحیح nByte, صحیح *pIdx){
  ثابت صحیح hdr = p_صفحہ->hdr_انحراف;    /* Local cache of pPage->hdrOffset */
  u8 * ثابت ڈیٹا = p_صفحہ->a_ڈیٹا;      /* Local cache of pPage->aData */
  صحیح چوٹی;                             /* First byte of cell content area */
  صحیح rc = SQLITE_OK;                  /* Integer return code */
  u8 *pTmp;                            /* Temp ptr into data[] */
  صحیح gap;        /* First byte of gap between cell pointers and cell content */
 
  أكد( sqlite3PagerIswriteable(p_صفحہ->p_Db_صفحہ) );
  أكد( p_صفحہ->pBt );
  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  أكد( nByte>=0 );  /* Minimum cell size is 4 */
  أكد( p_صفحہ->nFree>=nByte );
  أكد( p_صفحہ->n_حد_سے_تجاوز==0 );
  أكد( nByte < (صحیح)(p_صفحہ->pBt->usable_سائز-8) );

  أكد( p_صفحہ->cell_انحراف == hdr + 12 - 4*p_صفحہ->برگ );
  gap = p_صفحہ->cell_انحراف + 2*p_صفحہ->nCell;
  أكد( gap<=65536 );
  /* EVIDENCE-OF: R-29356-02391 If the database uses a 65536-byte page size
  ** and the reserved space is zero (the usual value for reserved space)
  ** then the cell content offset of an empty page wants to be 65536.
  ** However, that integer is too large to be stored in a 2-byte unsigned
  ** integer, so a value of 0 is used in its place. */
  pTmp = &ڈیٹا[hdr+5];
  چوٹی = get2byte(pTmp);
  اگر( gap>چوٹی ){
    اگر( چوٹی==0 && p_صفحہ->pBt->usable_سائز==65536 ){
      چوٹی = 65536;
    }ورنہ{
      واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }
  }ورنہ اگر( چوٹی>(صحیح)p_صفحہ->pBt->usable_سائز ){
    واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
  }

  /* If there is enough space between gap and top for one more cell pointer,
  ** and if the freelist is not empty, then search the
  ** freelist looking for a slot big enough to satisfy the request.
  */
  testcase( gap+2==چوٹی );
  testcase( gap+1==چوٹی );
  testcase( gap==چوٹی );
  اگر( (ڈیٹا[hdr+2] || ڈیٹا[hdr+1]) && gap+2<=چوٹی ){
    u8 *pSpace = صفحہ_ڈھونڈو_خانہ(p_صفحہ, nByte, &rc);
    اگر( pSpace ){
      صحیح g2;
      أكد( pSpace+nByte<=ڈیٹا+p_صفحہ->pBt->usable_سائز );
      *pIdx = g2 = (صحیح)(pSpace-ڈیٹا);
      اگر( g2<=gap ){
        واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
      }ورنہ{
        واپس SQLITE_OK;
      }
    }ورنہ اگر( rc ){
      واپس rc;
    }
  }

  /* The request could not be fulfilled using a freelist slot.  Check
  ** to see if defragmentation is necessary.
  */
  testcase( gap+2+nByte==چوٹی );
  اگر( gap+2+nByte>چوٹی ){
    أكد( p_صفحہ->nCell>0 || CORRUPT_DB );
    أكد( p_صفحہ->nFree>=0 );
    rc = defragment_صفحہ(p_صفحہ, MIN(4, p_صفحہ->nFree - (2+nByte)));
    اگر( rc ) واپس rc;
    چوٹی = get2byteNotZero(&ڈیٹا[hdr+5]);
    أكد( gap+2+nByte<=چوٹی );
  }


  /* Allocate memory from the gap in between the cell pointer array
  ** and the cell content area.  The btreeComputeFreeSpace() call has already
  ** validated the freelist.  Given that the freelist is valid, there
  ** is no way that the allocation can extend off the end of the page.
  ** The assert() below verifies the previous sentence.
  */
  چوٹی -= nByte;
  put2byte(&ڈیٹا[hdr+5], چوٹی);
  أكد( چوٹی+nByte <= (صحیح)p_صفحہ->pBt->usable_سائز );
  *pIdx = چوٹی;
  واپس SQLITE_OK;
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
ساکن صحیح freeSpace(Mem_صفحہ *p_صفحہ, صحیح i_آغاز, صحیح i_سائز){
  صحیح iPtr;                             /* Address of ptr to next freeblock */
  صحیح iFreeBlk;                         /* Address of the next freeblock */
  u8 hdr;                               /* Page header size.  0 or 100 */
  صحیح nFrag = 0;                        /* Reduction in fragmentation */
  صحیح i_Orig_سائز = i_سائز;                /* Original value of iSize */
  صحیح x;                                /* Offset to cell content area */
  صحیح i_انتہا = i_آغاز + i_سائز;            /* First byte past the iStart buffer */
  غیرعلامتی حرف *ڈیٹا = p_صفحہ->a_ڈیٹا;   /* Page content */
  u8 *pTmp;                             /* Temporary ptr into data[] */

  أكد( p_صفحہ->pBt!=0 );
  أكد( sqlite3PagerIswriteable(p_صفحہ->p_Db_صفحہ) );
  أكد( CORRUPT_DB || i_آغاز>=p_صفحہ->hdr_انحراف+6+p_صفحہ->بچہ_Ptr_سائز );
  أكد( CORRUPT_DB || i_انتہا <= (صحیح)p_صفحہ->pBt->usable_سائز );
  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  أكد( i_سائز>=4 );   /* Minimum cell size is 4 */
  أكد( CORRUPT_DB || i_آغاز<=(صحیح)p_صفحہ->pBt->usable_سائز-4 );

  /* The list of freeblocks must be in ascending order.  Find the
  ** spot on the list where iStart should be inserted.
  */
  hdr = p_صفحہ->hdr_انحراف;
  iPtr = hdr + 1;
  اگر( ڈیٹا[iPtr+1]==0 && ڈیٹا[iPtr]==0 ){
    iFreeBlk = 0;  /* Shortcut for the case when the freelist is empty */
  }ورنہ{
    جب( (iFreeBlk = get2byte(&ڈیٹا[iPtr]))<i_آغاز ){
      اگر( iFreeBlk<=iPtr ){
        اگر( iFreeBlk==0 ) توڑو; /* TH3: corrupt082.100 */
        واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
      }
      iPtr = iFreeBlk;
    }
    اگر( iFreeBlk>(صحیح)p_صفحہ->pBt->usable_سائز-4 ){ /* TH3: corrupt081.100 */
      واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }
    أكد( iFreeBlk>iPtr || iFreeBlk==0 || CORRUPT_DB );
 
    /* At this point:
    **    iFreeBlk:   First freeblock after iStart, or zero if none
    **    iPtr:       The address of a pointer to iFreeBlk
    **
    ** Check to see if iFreeBlk should be coalesced onto the end of iStart.
    */
    اگر( iFreeBlk && i_انتہا+3>=iFreeBlk ){
      nFrag = iFreeBlk - i_انتہا;
      اگر( i_انتہا>iFreeBlk ) واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
      i_انتہا = iFreeBlk + get2byte(&ڈیٹا[iFreeBlk+2]);
      اگر( i_انتہا > (صحیح)p_صفحہ->pBt->usable_سائز ){
        واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
      }
      i_سائز = i_انتہا - i_آغاز;
      iFreeBlk = get2byte(&ڈیٹا[iFreeBlk]);
    }
 
    /* If iPtr is another freeblock (that is, if iPtr is not the freelist
    ** pointer in the page header) then check to see if iStart should be
    ** coalesced onto the end of iPtr.
    */
    اگر( iPtr>hdr+1 ){
      صحیح i_Ptr_انتہا = iPtr + get2byte(&ڈیٹا[iPtr+2]);
      اگر( i_Ptr_انتہا+3>=i_آغاز ){
        اگر( i_Ptr_انتہا>i_آغاز ) واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
        nFrag += i_آغاز - i_Ptr_انتہا;
        i_سائز = i_انتہا - iPtr;
        i_آغاز = iPtr;
      }
    }
    اگر( nFrag>ڈیٹا[hdr+7] ) واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    ڈیٹا[hdr+7] -= (u8)nFrag;
  }
  pTmp = &ڈیٹا[hdr+5];
  x = get2byte(pTmp);
  اگر( p_صفحہ->pBt->btsFlags & BTS_FAST_SECURE ){
    /* Overwrite deleted information with zeros when the secure_delete
    ** option is enabled */
    memset(&ڈیٹا[i_آغاز], 0, i_سائز);
  }
  اگر( i_آغاز<=x ){
    /* The new freeblock is at the beginning of the cell content area,
    ** so just extend the cell content area rather than create another
    ** freelist entry */
    اگر( i_آغاز<x ) واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    اگر( iPtr!=hdr+1 ) واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    put2byte(&ڈیٹا[hdr+1], iFreeBlk);
    put2byte(&ڈیٹا[hdr+5], i_انتہا);
  }ورنہ{
    /* Insert the new freeblock into the freelist */
    put2byte(&ڈیٹا[iPtr], i_آغاز);
    put2byte(&ڈیٹا[i_آغاز], iFreeBlk);
    أكد( i_سائز>=0 && i_سائز<=0xffff );
    put2byte(&ڈیٹا[i_آغاز+2], (u16)i_سائز);
  }
  p_صفحہ->nFree += i_Orig_سائز;
  واپس SQLITE_OK;
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
ساکن صحیح decodeFlags(Mem_صفحہ *p_صفحہ, صحیح flagByte){
  BtShared *pBt;     /* A copy of pPage->pBt */

  أكد( p_صفحہ->hdr_انحراف==(p_صفحہ->pgno==1 ? 100 : 0) );
  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  pBt = p_صفحہ->pBt;
  p_صفحہ->max1bytePayload = pBt->max1bytePayload;
  اگر( flagByte>=(PTF_ZERODATA | PTF_برگ) ){
    p_صفحہ->بچہ_Ptr_سائز = 0;
    p_صفحہ->برگ = 1;
    اگر( flagByte==(PTF_LEAFDATA | PTF_INTKEY | PTF_برگ) ){
      p_صفحہ->int_کلید_برگ = 1;
      p_صفحہ->x_Cell_سائز = cell_سائز_Ptr_جدول_برگ;
      p_صفحہ->xParseCell = btreeParseCellPtr;
      p_صفحہ->int_کلید = 1;
      p_صفحہ->maxLocal = pBt->max_برگ;
      p_صفحہ->minLocal = pBt->min_برگ;
    }ورنہ اگر( flagByte==(PTF_ZERODATA | PTF_برگ) ){
      p_صفحہ->int_کلید = 0;
      p_صفحہ->int_کلید_برگ = 0;
      p_صفحہ->x_Cell_سائز = cell_سائز_Ptr_Idx_برگ;
      p_صفحہ->xParseCell = btree_Parse_Cell_Ptr_اشاریہ;
      p_صفحہ->maxLocal = pBt->maxLocal;
      p_صفحہ->minLocal = pBt->minLocal;
    }ورنہ{
      p_صفحہ->int_کلید = 0;
      p_صفحہ->int_کلید_برگ = 0;
      p_صفحہ->x_Cell_سائز = cell_سائز_Ptr_Idx_برگ;
      p_صفحہ->xParseCell = btree_Parse_Cell_Ptr_اشاریہ;
      واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }
  }ورنہ{
    p_صفحہ->بچہ_Ptr_سائز = 4;
    p_صفحہ->برگ = 0;
    اگر( flagByte==(PTF_ZERODATA) ){
      p_صفحہ->int_کلید = 0;
      p_صفحہ->int_کلید_برگ = 0;
      p_صفحہ->x_Cell_سائز = cell_سائز_Ptr;
      p_صفحہ->xParseCell = btree_Parse_Cell_Ptr_اشاریہ;
      p_صفحہ->maxLocal = pBt->maxLocal;
      p_صفحہ->minLocal = pBt->minLocal;
    }ورنہ اگر( flagByte==(PTF_LEAFDATA | PTF_INTKEY) ){
      p_صفحہ->int_کلید_برگ = 0;
      p_صفحہ->x_Cell_سائز = cell_سائز_Ptr_No_Payload;
      p_صفحہ->xParseCell = btreeParseCellPtrNoPayload;
      p_صفحہ->int_کلید = 1;
      p_صفحہ->maxLocal = pBt->max_برگ;
      p_صفحہ->minLocal = pBt->min_برگ;
    }ورنہ{
      p_صفحہ->int_کلید = 0;
      p_صفحہ->int_کلید_برگ = 0;
      p_صفحہ->x_Cell_سائز = cell_سائز_Ptr;
      p_صفحہ->xParseCell = btree_Parse_Cell_Ptr_اشاریہ;
      واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }
  }
  واپس SQLITE_OK;
}

/*
** Compute the amount of freespace on the page.  In other words, fill
** in the pPage->nFree field.
*/
ساکن صحیح btreeComputeFreeSpace(Mem_صفحہ *p_صفحہ){
  صحیح pc;            /* Address of a freeblock within pPage->aData[] */
  u8 hdr;            /* Offset to beginning of page header */
  u8 *ڈیٹا;          /* Equal to pPage->aData */
  صحیح usable_سائز;    /* Amount of usable space on each page */
  صحیح nFree;         /* Number of unused bytes on the page */
  صحیح چوٹی;           /* First byte of the cell content area */
  صحیح i_Cell_پہلا;    /* First allowable cell or freeblock offset */
  صحیح i_Cell_آخری;     /* Last possible cell or freeblock offset */

  أكد( p_صفحہ->pBt!=0 );
  أكد( p_صفحہ->pBt->db!=0 );
  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  أكد( p_صفحہ->pgno==sqlite3PagerPagenumber(p_صفحہ->p_Db_صفحہ) );
  أكد( p_صفحہ == sqlite3PagerGetExtra(p_صفحہ->p_Db_صفحہ) );
  أكد( p_صفحہ->a_ڈیٹا == sqlite3_Pager_Get_ڈیٹا(p_صفحہ->p_Db_صفحہ) );
  أكد( p_صفحہ->isInit==1 );
  أكد( p_صفحہ->nFree<0 );

  usable_سائز = p_صفحہ->pBt->usable_سائز;
  hdr = p_صفحہ->hdr_انحراف;
  ڈیٹا = p_صفحہ->a_ڈیٹا;
  /* EVIDENCE-OF: R-58015-48175 The two-byte integer at offset 5 designates
  ** the start of the cell content area. A zero value for this integer is
  ** interpreted as 65536. */
  چوٹی = get2byteNotZero(&ڈیٹا[hdr+5]);
  i_Cell_پہلا = hdr + 8 + p_صفحہ->بچہ_Ptr_سائز + 2*p_صفحہ->nCell;
  i_Cell_آخری = usable_سائز - 4;

  /* Compute the total free space on the page
  ** EVIDENCE-OF: R-23588-34450 The two-byte integer at offset 1 gives the
  ** start of the first freeblock on the page, or is zero if there are no
  ** freeblocks. */
  pc = get2byte(&ڈیٹا[hdr+1]);
  nFree = ڈیٹا[hdr+7] + چوٹی;  /* Init nFree to non-freeblock free space */
  اگر( pc>0 ){
    u32 اگلا, سائز;
    اگر( pc<چوٹی ){
      /* EVIDENCE-OF: R-55530-52930 In a well-formed b-tree page, there will
      ** always be at least one cell before the first freeblock.
      */
      واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }
    جب( 1 ){
      اگر( pc>i_Cell_آخری ){
        /* Freeblock off the end of the page */
        واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
      }
      اگلا = get2byte(&ڈیٹا[pc]);
      سائز = get2byte(&ڈیٹا[pc+2]);
      اگر( سائز<4 && sqlite3FaultSim(422)==SQLITE_OK ){
        /* Minimum freeblock size is 4.  Enable fault-sim 422 to disable this
        ** check to reach interesting error states.  However, disabling this
        ** check can cause assertion faults due to min-heap overflow.  All
        ** fault-sims are for testing use only, but this one especially so. */
        واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
      }
      nFree = nFree + سائز;
      اگر( اگلا<pc+سائز+4 ) توڑو;
      pc = اگلا;
    }
    اگر( اگلا>0 ){
      /* Freeblock not in ascending order */
      واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }
    اگر( pc+سائز>(غیرعلامتی صحیح)usable_سائز ){
      /* Last freeblock extends past page end */
      واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }
  }

  /* At this point, nFree contains the sum of the offset to the start
  ** of the cell-content area plus the number of free bytes within
  ** the cell-content area. If this is greater than the usable-size
  ** of the page, then the page must be corrupted. This check also
  ** serves to verify that the offset to the start of the cell-content
  ** area, according to the page header, lies within the page.
  */
  اگر( nFree>usable_سائز || nFree<i_Cell_پہلا ){
    واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
  }
  p_صفحہ->nFree = (u16)(nFree - i_Cell_پہلا);
  واپس SQLITE_OK;
}

/*
** Do additional sanity check after btreeInitPage() if
** PRAGMA cell_size_check=ON
*/
ساکن SQLITE_NOINLINE صحیح btree_Cell_سائز_Check(Mem_صفحہ *p_صفحہ){
  صحیح i_Cell_پہلا;    /* First allowable cell or freeblock offset */
  صحیح i_Cell_آخری;     /* Last possible cell or freeblock offset */
  صحیح i;             /* Index into the cell pointer array */
  صحیح sz;            /* Size of a cell */
  صحیح pc;            /* Address of a freeblock within pPage->aData[] */
  u8 *ڈیٹا;          /* Equal to pPage->aData */
  صحیح usable_سائز;    /* Maximum usable space on the page */
  صحیح cell_انحراف;    /* Start of cell content area */

  i_Cell_پہلا = p_صفحہ->cell_انحراف + 2*p_صفحہ->nCell;
  usable_سائز = p_صفحہ->pBt->usable_سائز;
  i_Cell_آخری = usable_سائز - 4;
  ڈیٹا = p_صفحہ->a_ڈیٹا;
  cell_انحراف = p_صفحہ->cell_انحراف;
  اگر( !p_صفحہ->برگ ) i_Cell_آخری--;
  برائے(i=0; i<p_صفحہ->nCell; i++){
    pc = get2byteAligned(&ڈیٹا[cell_انحراف+i*2]);
    testcase( pc==i_Cell_پہلا );
    testcase( pc==i_Cell_آخری );
    اگر( pc<i_Cell_پہلا || pc>i_Cell_آخری ){
      واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }
    sz = p_صفحہ->x_Cell_سائز(p_صفحہ, &ڈیٹا[pc]);
    testcase( pc+sz==usable_سائز );
    اگر( pc+sz>usable_سائز ){
      واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }
  }
  واپس SQLITE_OK;
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
ساکن صحیح btree_Init_صفحہ(Mem_صفحہ *p_صفحہ){
  u8 *ڈیٹا;          /* Equal to pPage->aData */
  BtShared *pBt;        /* The main btree structure */

  أكد( p_صفحہ->pBt!=0 );
  أكد( p_صفحہ->pBt->db!=0 );
  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  أكد( p_صفحہ->pgno==sqlite3PagerPagenumber(p_صفحہ->p_Db_صفحہ) );
  أكد( p_صفحہ == sqlite3PagerGetExtra(p_صفحہ->p_Db_صفحہ) );
  أكد( p_صفحہ->a_ڈیٹا == sqlite3_Pager_Get_ڈیٹا(p_صفحہ->p_Db_صفحہ) );
  أكد( p_صفحہ->isInit==0 );

  pBt = p_صفحہ->pBt;
  ڈیٹا = p_صفحہ->a_ڈیٹا + p_صفحہ->hdr_انحراف;
  /* EVIDENCE-OF: R-28594-02890 The one-byte flag at offset 0 indicating
  ** the b-tree page type. */
  اگر( decodeFlags(p_صفحہ, ڈیٹا[0]) ){
    واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
  }
  أكد( pBt->صفحہ_سائز>=512 && pBt->صفحہ_سائز<=65536 );
  p_صفحہ->نقاب_صفحہ = (u16)(pBt->صفحہ_سائز - 1);
  p_صفحہ->n_حد_سے_تجاوز = 0;
  p_صفحہ->cell_انحراف = (u16)(p_صفحہ->hdr_انحراف + 8 + p_صفحہ->بچہ_Ptr_سائز);
  p_صفحہ->aCellIdx = ڈیٹا + p_صفحہ->بچہ_Ptr_سائز + 8;
  p_صفحہ->a_ڈیٹا_انتہا = p_صفحہ->a_ڈیٹا + pBt->صفحہ_سائز;
  p_صفحہ->a_ڈیٹا_Ofst = p_صفحہ->a_ڈیٹا + p_صفحہ->بچہ_Ptr_سائز;
  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  p_صفحہ->nCell = get2byte(&ڈیٹا[3]);
  اگر( p_صفحہ->nCell>MX_CELL(pBt) ){
    /* To many cells for a single page.  The page must be corrupt */
    واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
  }
  testcase( p_صفحہ->nCell==MX_CELL(pBt) );
  /* EVIDENCE-OF: R-24089-57979 If a page contains no cells (which is only
  ** possible for a root page of a table that contains no rows) then the
  ** offset to the cell content area will equal the page size minus the
  ** bytes of reserved space. */
  أكد( p_صفحہ->nCell>0
       || get2byteNotZero(&ڈیٹا[5])==(صحیح)pBt->usable_سائز
       || CORRUPT_DB );
  p_صفحہ->nFree = -1;  /* Indicate that this value is yet uncomputed */
  p_صفحہ->isInit = 1;
  اگر( pBt->db->flags & SQLITE_Cell_سائز_Ck ){
    واپس btree_Cell_سائز_Check(p_صفحہ);
  }
  واپس SQLITE_OK;
}

/*
** Set up a raw page so that it looks like a database page holding
** no entries.
*/
ساکن خالی zero_صفحہ(Mem_صفحہ *p_صفحہ, صحیح flags){
  غیرعلامتی حرف *ڈیٹا = p_صفحہ->a_ڈیٹا;
  BtShared *pBt = p_صفحہ->pBt;
  صحیح hdr = p_صفحہ->hdr_انحراف;
  صحیح پہلا;

  أكد( sqlite3PagerPagenumber(p_صفحہ->p_Db_صفحہ)==p_صفحہ->pgno || CORRUPT_DB );
  أكد( sqlite3PagerGetExtra(p_صفحہ->p_Db_صفحہ) == (خالی*)p_صفحہ );
  أكد( sqlite3_Pager_Get_ڈیٹا(p_صفحہ->p_Db_صفحہ) == ڈیٹا );
  أكد( sqlite3PagerIswriteable(p_صفحہ->p_Db_صفحہ) );
  أكد( sqlite3_mutex_held(pBt->mutex) );
  اگر( pBt->btsFlags & BTS_FAST_SECURE ){
    memset(&ڈیٹا[hdr], 0, pBt->usable_سائز - hdr);
  }
  ڈیٹا[hdr] = (حرف)flags;
  پہلا = hdr + ((flags&PTF_برگ)==0 ? 12 : 8);
  memset(&ڈیٹا[hdr+1], 0, 4);
  ڈیٹا[hdr+7] = 0;
  put2byte(&ڈیٹا[hdr+5], pBt->usable_سائز);
  p_صفحہ->nFree = (u16)(pBt->usable_سائز - پہلا);
  decodeFlags(p_صفحہ, flags);
  p_صفحہ->cell_انحراف = (u16)پہلا;
  p_صفحہ->a_ڈیٹا_انتہا = &ڈیٹا[pBt->صفحہ_سائز];
  p_صفحہ->aCellIdx = &ڈیٹا[پہلا];
  p_صفحہ->a_ڈیٹا_Ofst = &ڈیٹا[p_صفحہ->بچہ_Ptr_سائز];
  p_صفحہ->n_حد_سے_تجاوز = 0;
  أكد( pBt->صفحہ_سائز>=512 && pBt->صفحہ_سائز<=65536 );
  p_صفحہ->نقاب_صفحہ = (u16)(pBt->صفحہ_سائز - 1);
  p_صفحہ->nCell = 0;
  p_صفحہ->isInit = 1;
}


/*
** Convert a DbPage obtained from the pager into a MemPage used by
** the btree layer.
*/
ساکن Mem_صفحہ *btree_صفحہ_From_Db_صفحہ(Db_صفحہ *p_Db_صفحہ, Pgno pgno, BtShared *pBt){
  Mem_صفحہ *p_صفحہ = (Mem_صفحہ*)sqlite3PagerGetExtra(p_Db_صفحہ);
  اگر( pgno!=p_صفحہ->pgno ){
    p_صفحہ->a_ڈیٹا = sqlite3_Pager_Get_ڈیٹا(p_Db_صفحہ);
    p_صفحہ->p_Db_صفحہ = p_Db_صفحہ;
    p_صفحہ->pBt = pBt;
    p_صفحہ->pgno = pgno;
    p_صفحہ->hdr_انحراف = pgno==1 ? 100 : 0;
  }
  أكد( p_صفحہ->a_ڈیٹا==sqlite3_Pager_Get_ڈیٹا(p_Db_صفحہ) );
  واپس p_صفحہ;
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
ساکن صحیح btree_Get_صفحہ(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  Mem_صفحہ **pp_صفحہ,    /* Return the page in this parameter */
  صحیح flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  صحیح rc;
  Db_صفحہ *p_Db_صفحہ;

  أكد( flags==0 || flags==PAGER_GET_NOCONTENT || flags==PAGER_GET_READONLY );
  أكد( sqlite3_mutex_held(pBt->mutex) );
  rc = sqlite3PagerGet(pBt->pPager, pgno, (Db_صفحہ**)&p_Db_صفحہ, flags);
  اگر( rc ) واپس rc;
  *pp_صفحہ = btree_صفحہ_From_Db_صفحہ(p_Db_صفحہ, pgno, pBt);
  واپس SQLITE_OK;
}

/*
** Retrieve a page from the pager cache. If the requested page is not
** already in the pager cache return NULL. Initialize the MemPage.pBt and
** MemPage.aData elements if needed.
*/
ساکن Mem_صفحہ *btree_صفحہ_Lookup(BtShared *pBt, Pgno pgno){
  Db_صفحہ *p_Db_صفحہ;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  p_Db_صفحہ = sqlite3PagerLookup(pBt->pPager, pgno);
  اگر( p_Db_صفحہ ){
    واپس btree_صفحہ_From_Db_صفحہ(p_Db_صفحہ, pgno, pBt);
  }
  واپس 0;
}

/*
** Return the size of the database file in pages. If there is any kind of
** error, return ((unsigned int)-1).
*/
ساکن Pgno btreePagecount(BtShared *pBt){
  واپس pBt->n_صفحہ;
}
Pgno sqlite3_Btree_آخری_صفحہ(Btree *p){
  أكد( sqlite3BtreeHoldsMutex(p) );
  واپس btreePagecount(p->pBt);
}

/*
** Get a page from the pager and initialize it.
*/
ساکن صحیح get_And_Init_صفحہ(
  BtShared *pBt,                  /* The database file */
  Pgno pgno,                      /* Number of the page to get */
  Mem_صفحہ **pp_صفحہ,               /* Write the page pointer here */
  صحیح bReadOnly                   /* True for a read-only page */
){
  صحیح rc;
  Db_صفحہ *p_Db_صفحہ;
  Mem_صفحہ *p_صفحہ;
  أكد( sqlite3_mutex_held(pBt->mutex) );

  اگر( pgno>btreePagecount(pBt) ){
    *pp_صفحہ = 0;
    واپس SQLITE_CORRUPT_BKPT;
  }
  rc = sqlite3PagerGet(pBt->pPager, pgno, (Db_صفحہ**)&p_Db_صفحہ, bReadOnly);
  اگر( rc ){
    *pp_صفحہ = 0;
    واپس rc;
  }
  p_صفحہ = (Mem_صفحہ*)sqlite3PagerGetExtra(p_Db_صفحہ);
  اگر( p_صفحہ->isInit==0 ){
    btree_صفحہ_From_Db_صفحہ(p_Db_صفحہ, pgno, pBt);
    rc = btree_Init_صفحہ(p_صفحہ);
    اگر( rc!=SQLITE_OK ){
      release_صفحہ(p_صفحہ);
      *pp_صفحہ = 0;
      واپس rc;
    }
  }
  أكد( p_صفحہ->pgno==pgno || CORRUPT_DB );
  أكد( p_صفحہ->a_ڈیٹا==sqlite3_Pager_Get_ڈیٹا(p_Db_صفحہ) );
  *pp_صفحہ = p_صفحہ;
  واپس SQLITE_OK;
}

/*
** Release a MemPage.  This should be called once for each prior
** call to btreeGetPage.
**
** Page1 is a special case and must be released using releasePageOne().
*/
ساکن خالی release_صفحہ_Not_Null(Mem_صفحہ *p_صفحہ){
  أكد( p_صفحہ->a_ڈیٹا );
  أكد( p_صفحہ->pBt );
  أكد( p_صفحہ->p_Db_صفحہ!=0 );
  أكد( sqlite3PagerGetExtra(p_صفحہ->p_Db_صفحہ) == (خالی*)p_صفحہ );
  أكد( sqlite3_Pager_Get_ڈیٹا(p_صفحہ->p_Db_صفحہ)==p_صفحہ->a_ڈیٹا );
  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  sqlite3PagerUnrefNotNull(p_صفحہ->p_Db_صفحہ);
}
ساکن خالی release_صفحہ(Mem_صفحہ *p_صفحہ){
  اگر( p_صفحہ ) release_صفحہ_Not_Null(p_صفحہ);
}
ساکن خالی release_صفحہ_One(Mem_صفحہ *p_صفحہ){
  أكد( p_صفحہ!=0 );
  أكد( p_صفحہ->a_ڈیٹا );
  أكد( p_صفحہ->pBt );
  أكد( p_صفحہ->p_Db_صفحہ!=0 );
  أكد( sqlite3PagerGetExtra(p_صفحہ->p_Db_صفحہ) == (خالی*)p_صفحہ );
  أكد( sqlite3_Pager_Get_ڈیٹا(p_صفحہ->p_Db_صفحہ)==p_صفحہ->a_ڈیٹا );
  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  sqlite3_Pager_Unref_صفحہ_One(p_صفحہ->p_Db_صفحہ);
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
ساکن صحیح btree_Get_غیر_مستعمل_صفحہ(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  Mem_صفحہ **pp_صفحہ,    /* Return the page in this parameter */
  صحیح flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  صحیح rc = btree_Get_صفحہ(pBt, pgno, pp_صفحہ, flags);
  اگر( rc==SQLITE_OK ){
    اگر( sqlite3_Pager_صفحہ_Refcount((*pp_صفحہ)->p_Db_صفحہ)>1 ){
      release_صفحہ(*pp_صفحہ);
      *pp_صفحہ = 0;
      واپس SQLITE_CORRUPT_BKPT;
    }
    (*pp_صفحہ)->isInit = 0;
  }ورنہ{
    *pp_صفحہ = 0;
  }
  واپس rc;
}


/*
** During a rollback, when the pager reloads information into the cache
** so that the cache is restored to its original state at the start of
** the transaction, for each page restored this routine is called.
**
** This routine needs to reset the extra data section at the end of the
** page to agree with the restored data.
*/
ساکن خالی صفحہ_Reinit(Db_صفحہ *p_ڈیٹا){
  Mem_صفحہ *p_صفحہ;
  p_صفحہ = (Mem_صفحہ *)sqlite3PagerGetExtra(p_ڈیٹا);
  أكد( sqlite3_Pager_صفحہ_Refcount(p_ڈیٹا)>0 );
  اگر( p_صفحہ->isInit ){
    أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
    p_صفحہ->isInit = 0;
    اگر( sqlite3_Pager_صفحہ_Refcount(p_ڈیٹا)>1 ){
      /* pPage might not be a btree page;  it might be an overflow page
      ** or ptrmap page or a free page.  In those cases, the following
      ** call to btreeInitPage() will likely return SQLITE_CORRUPT.
      ** But no harm is done by this.  And it is very important that
      ** btreeInitPage() be called on every btree page so we make
      ** the call for every page that comes in for re-initializing. */
      btree_Init_صفحہ(p_صفحہ);
    }
  }
}

/*
** Invoke the busy handler for a btree.
*/
ساکن صحیح btreeInvokeBusyHandler(خالی *pArg){
  BtShared *pBt = (BtShared*)pArg;
  أكد( pBt->db );
  أكد( sqlite3_mutex_held(pBt->db->mutex) );
  واپس sqlite3InvokeBusyHandler(&pBt->db->busyHandler);
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
صحیح sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,      /* VFS to use for this b-tree */
  ثابت حرف *zFilename,  /* Name of the file containing the BTree database */
  sqlite3 *db,            /* Associated database handle */
  Btree **ppBtree,        /* Pointer to new Btree object written here */
  صحیح flags,              /* Options */
  صحیح vfsFlags            /* Flags passed through to sqlite3_vfs.xOpen() */
){
  BtShared *pBt = 0;             /* Shared part of btree structure */
  Btree *p;                      /* Handle to return */
  sqlite3_mutex *mutexOpen = 0;  /* Prevents a race condition. Ticket #3537 */
  صحیح rc = SQLITE_OK;            /* Result code from this function */
  u8 nReserve;                   /* Byte of unused space on each page */
  غیرعلامتی حرف z_Db_سرخی[100];  /* Database header content */

  /* True if opening an ephemeral, temporary database */
  ثابت صحیح is_عارضی_Db = zFilename==0 || zFilename[0]==0;

  /* Set the variable isMemdb to true for an in-memory database, or
  ** false for a file-based database.
  */
#اگر_تعریف_شدہ SQLITE_OMIT_MEMORYDB
  ثابت صحیح isMemdb = 0;
#ورنہ
  ثابت صحیح isMemdb = (zFilename && strcmp(zFilename, ":memory:")==0)
                       || (is_عارضی_Db && sqlite3_عارضی_اندر_حافظہ(db))
                       || (vfsFlags & SQLITE_OPEN_حافظہ)!=0;
#اگر_ختم

  أكد( db!=0 );
  أكد( pVfs!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( (flags&0xff)==flags );   /* flags fit in 8 bits */

  /* Only a BTREE_SINGLE database can be BTREE_UNORDERED */
  أكد( (flags & BTREE_UNORDERED)==0 || (flags & BTREE_SINGLE)!=0 );

  /* A BTREE_SINGLE database is always a temporary and/or ephemeral */
  أكد( (flags & BTREE_SINGLE)==0 || is_عارضی_Db );

  اگر( isMemdb ){
    flags |= BTREE_حافظہ;
  }
  اگر( (vfsFlags & SQLITE_OPEN_MAIN_DB)!=0 && (isMemdb || is_عارضی_Db) ){
    vfsFlags = (vfsFlags & ~SQLITE_OPEN_MAIN_DB) | SQLITE_OPEN_عارضی_DB;
  }
  p = sqlite3MallocZero(حجم(Btree));
  اگر( !p ){
    واپس SQLITE_NOMEM_BKPT;
  }
  p->اندر_Trans = TRANS_NONE;
  p->db = db;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
  p->تالا.pBtree = p;
  p->تالا.i_جدول = 1;
#اگر_ختم

#اگر !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
  /*
  ** If this Btree is a candidate for shared cache, try to find an
  ** existing BtShared object that we can share with
  */
  اگر( is_عارضی_Db==0 && (isMemdb==0 || (vfsFlags&SQLITE_OPEN_URI)!=0) ){
    اگر( vfsFlags & SQLITE_OPEN_SHAREDCACHE ){
      صحیح nFilename = sqlite3Strlen30(zFilename)+1;
      صحیح n_بھرا_Pathname = pVfs->mxPathname+1;
      حرف *z_بھرا_Pathname = sqlite3Malloc(MAX(n_بھرا_Pathname,nFilename));
      MUTEX_LOGIC( sqlite3_mutex *mutexShared; )

      p->sharable = 1;
      اگر( !z_بھرا_Pathname ){
        sqlite3_free(p);
        واپس SQLITE_NOMEM_BKPT;
      }
      اگر( isMemdb ){
        حافظہ_نقل(z_بھرا_Pathname, zFilename, nFilename);
      }ورنہ{
        rc = sqlite3_Os_بھرا_Pathname(pVfs, zFilename,
                                   n_بھرا_Pathname, z_بھرا_Pathname);
        اگر( rc ){
          اگر( rc==SQLITE_OK_SYMLINK ){
            rc = SQLITE_OK;
          }ورنہ{
            sqlite3_free(z_بھرا_Pathname);
            sqlite3_free(p);
            واپس rc;
          }
        }
      }
#اگر SQLITE_THREADSAFE
      mutexOpen = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_OPEN);
      sqlite3_mutex_enter(mutexOpen);
      mutexShared = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
      sqlite3_mutex_enter(mutexShared);
#اگر_ختم
      برائے(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->p_اگلا){
        أكد( pBt->nRef>0 );
        اگر( 0==strcmp(z_بھرا_Pathname, sqlite3PagerFilename(pBt->pPager, 0))
                 && sqlite3PagerVfs(pBt->pPager)==pVfs ){
          صحیح iDb;
          برائے(iDb=db->nDb-1; iDb>=0; iDb--){
            Btree *pExisting = db->aDb[iDb].pBt;
            اگر( pExisting && pExisting->pBt==pBt ){
              sqlite3_mutex_leave(mutexShared);
              sqlite3_mutex_leave(mutexOpen);
              sqlite3_free(z_بھرا_Pathname);
              sqlite3_free(p);
              واپس SQLITE_CONSTRAINT;
            }
          }
          p->pBt = pBt;
          pBt->nRef++;
          توڑو;
        }
      }
      sqlite3_mutex_leave(mutexShared);
      sqlite3_free(z_بھرا_Pathname);
    }
#اگر_تعریف_شدہ SQLITE_DEBUG
    ورنہ{
      /* In debug mode, we mark all persistent databases as sharable
      ** even when they are not.  This exercises the locking code and
      ** gives more opportunity for asserts(sqlite3_mutex_held())
      ** statements to find locking problems.
      */
      p->sharable = 1;
    }
#اگر_ختم
  }
#اگر_ختم
  اگر( pBt==0 ){
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
    memset(&z_Db_سرخی[16], 0, 8);
 
    pBt = sqlite3MallocZero( حجم(*pBt) );
    اگر( pBt==0 ){
      rc = SQLITE_NOMEM_BKPT;
      جاؤ btree_open_برآمد;
    }
    rc = sqlite3PagerOpen(pVfs, &pBt->pPager, zFilename,
                          حجم(Mem_صفحہ), flags, vfsFlags, صفحہ_Reinit);
    اگر( rc==SQLITE_OK ){
      sqlite3_Pager_سیٹ_Mmap_حد_بندی(pBt->pPager, db->szMmap);
      rc = sqlite3PagerReadFileheader(pBt->pPager,حجم(z_Db_سرخی),z_Db_سرخی);
    }
    اگر( rc!=SQLITE_OK ){
      جاؤ btree_open_برآمد;
    }
    pBt->openFlags = (u8)flags;
    pBt->db = db;
    sqlite3_Pager_سیٹ_Busy_Handler(pBt->pPager, btreeInvokeBusyHandler, pBt);
    p->pBt = pBt;
 
    pBt->pCursor = 0;
    pBt->pPage1 = 0;
    اگر( sqlite3PagerIsreadonly(pBt->pPager) ) pBt->btsFlags |= BTS_READ_ONLY;
#اگر defined(SQLITE_SECURE_DELETE)
    pBt->btsFlags |= BTS_SECURE_DELETE;
#وگرنہ_اگر defined(SQLITE_FAST_SECURE_DELETE)
    pBt->btsFlags |= BTS_OVERWRITE;
#اگر_ختم
    /* EVIDENCE-OF: R-51873-39618 The page size for a database file is
    ** determined by the 2-byte integer located at an offset of 16 bytes from
    ** the beginning of the database file. */
    pBt->صفحہ_سائز = (z_Db_سرخی[16]<<8) | (z_Db_سرخی[17]<<16);
    اگر( pBt->صفحہ_سائز<512 || pBt->صفحہ_سائز>SQLITE_MAX_صفحہ_سائز
         || ((pBt->صفحہ_سائز-1)&pBt->صفحہ_سائز)!=0 ){
      pBt->صفحہ_سائز = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
      /* If the magic name ":memory:" will create an in-memory database, then
      ** leave the autoVacuum mode at 0 (do not auto-vacuum), even if
      ** SQLITE_DEFAULT_AUTOVACUUM is true. On the other hand, if
      ** SQLITE_OMIT_MEMORYDB has been defined, then ":memory:" is just a
      ** regular file-name. In this case the auto-vacuum applies as per normal.
      */
      اگر( zFilename && !isMemdb ){
        pBt->autoVacuum = (SQLITE_DEFAULT_AUTOVACUUM ? 1 : 0);
        pBt->incrVacuum = (SQLITE_DEFAULT_AUTOVACUUM==2 ? 1 : 0);
      }
#اگر_ختم
      nReserve = 0;
    }ورنہ{
      /* EVIDENCE-OF: R-37497-42412 The size of the reserved region is
      ** determined by the one-byte unsigned integer found at an offset of 20
      ** into the database file header. */
      nReserve = z_Db_سرخی[20];
      pBt->btsFlags |= BTS_PAGESIZE_FIXED;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
      pBt->autoVacuum = (get4byte(&z_Db_سرخی[36 + 4*4])?1:0);
      pBt->incrVacuum = (get4byte(&z_Db_سرخی[36 + 7*4])?1:0);
#اگر_ختم
    }
    rc = sqlite3_Pager_سیٹ_Pagesize(pBt->pPager, &pBt->صفحہ_سائز, nReserve);
    اگر( rc ) جاؤ btree_open_برآمد;
    pBt->usable_سائز = pBt->صفحہ_سائز - nReserve;
    أكد( (pBt->صفحہ_سائز & 7)==0 );  /* 8-byte alignment of pageSize */
  
#اگر !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
    /* Add the new BtShared object to the linked list sharable BtShareds.
    */
    pBt->nRef = 1;
    اگر( p->sharable ){
      MUTEX_LOGIC( sqlite3_mutex *mutexShared; )
      MUTEX_LOGIC( mutexShared = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);)
      اگر( SQLITE_THREADSAFE && sqlite3GlobalConfig.bCoreMutex ){
        pBt->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_FAST);
        اگر( pBt->mutex==0 ){
          rc = SQLITE_NOMEM_BKPT;
          جاؤ btree_open_برآمد;
        }
      }
      sqlite3_mutex_enter(mutexShared);
      pBt->p_اگلا = GLOBAL(BtShared*,sqlite3SharedCacheList);
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt;
      sqlite3_mutex_leave(mutexShared);
    }
#اگر_ختم
  }

#اگر !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
  /* If the new Btree uses a sharable pBtShared, then link the new
  ** Btree into the list of all sharable Btrees for the same connection.
  ** The list is kept in ascending order by pBt address.
  */
  اگر( p->sharable ){
    صحیح i;
    Btree *pSib;
    برائے(i=0; i<db->nDb; i++){
      اگر( (pSib = db->aDb[i].pBt)!=0 && pSib->sharable ){
        جب( pSib->p_پچھلا ){ pSib = pSib->p_پچھلا; }
        اگر( (uptr)p->pBt<(uptr)pSib->pBt ){
          p->p_اگلا = pSib;
          p->p_پچھلا = 0;
          pSib->p_پچھلا = p;
        }ورنہ{
          جب( pSib->p_اگلا && (uptr)pSib->p_اگلا->pBt<(uptr)p->pBt ){
            pSib = pSib->p_اگلا;
          }
          p->p_اگلا = pSib->p_اگلا;
          p->p_پچھلا = pSib;
          اگر( p->p_اگلا ){
            p->p_اگلا->p_پچھلا = p;
          }
          pSib->p_اگلا = p;
        }
        توڑو;
      }
    }
  }
#اگر_ختم
  *ppBtree = p;

btree_open_برآمد:
  اگر( rc!=SQLITE_OK ){
    اگر( pBt && pBt->pPager ){
      sqlite3PagerClose(pBt->pPager, 0);
    }
    sqlite3_free(pBt);
    sqlite3_free(p);
    *ppBtree = 0;
  }ورنہ{
    sqlite3_file *pFile;

    /* If the B-Tree was successfully opened, set the pager-cache size to the
    ** default value. Except, when opening on an existing shared pager-cache,
    ** do not change the pager-cache size.
    */
    اگر( sqlite3BtreeSchema(p, 0, 0)==0 ){
      sqlite3_Btree_سیٹ_Cache_سائز(p, SQLITE_DEFAULT_CACHE_سائز);
    }

    pFile = sqlite3PagerFile(pBt->pPager);
    اگر( pFile->pMethods ){
      sqlite3OsFileControlHint(pFile, SQLITE_FCNTL_PDB, (خالی*)&pBt->db);
    }
  }
  اگر( mutexOpen ){
    أكد( sqlite3_mutex_held(mutexOpen) );
    sqlite3_mutex_leave(mutexOpen);
  }
  أكد( rc!=SQLITE_OK || sqlite3_Btree_Connection_گنتی(*ppBtree)>0 );
  واپس rc;
}

/*
** Decrement the BtShared.nRef counter.  When it reaches zero,
** remove the BtShared structure from the sharing list.  Return
** true if the BtShared.nRef counter reaches zero and return
** false if it is still positive.
*/
ساکن صحیح removeFromSharingList(BtShared *pBt){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )
  BtShared *pList;
  صحیح removed = 0;

  أكد( sqlite3_mutex_notheld(pBt->mutex) );
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  pBt->nRef--;
  اگر( pBt->nRef<=0 ){
    اگر( GLOBAL(BtShared*,sqlite3SharedCacheList)==pBt ){
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt->p_اگلا;
    }ورنہ{
      pList = GLOBAL(BtShared*,sqlite3SharedCacheList);
      جب( ALWAYS(pList) && pList->p_اگلا!=pBt ){
        pList=pList->p_اگلا;
      }
      اگر( ALWAYS(pList) ){
        pList->p_اگلا = pBt->p_اگلا;
      }
    }
    اگر( SQLITE_THREADSAFE ){
      sqlite3_mutex_free(pBt->mutex);
    }
    removed = 1;
  }
  sqlite3_mutex_leave(pMainMtx);
  واپس removed;
#ورنہ
  غیر_مستعمل_PARAMETER( pBt );
  واپس 1;
#اگر_ختم
}

/*
** Make sure pBt->pTmpSpace points to an allocation of
** MX_CELL_SIZE(pBt) bytes with a 4-byte prefix for a left-child
** pointer.
*/
ساکن SQLITE_NOINLINE صحیح allocate_عارضی_Space(BtShared *pBt){
  أكد( pBt!=0 );
  أكد( pBt->pTmpSpace==0 );
  /* This routine is called only by btreeCursor() when allocating the
  ** first write cursor for the BtShared object */
  أكد( pBt->pCursor!=0 && (pBt->pCursor->curFlags & BTCF_WriteFlag)!=0 );
  pBt->pTmpSpace = sqlite3_صفحہ_Malloc( pBt->صفحہ_سائز );
  اگر( pBt->pTmpSpace==0 ){
    BtCursor *pCur = pBt->pCursor;
    pBt->pCursor = pCur->p_اگلا;  /* Unlink the cursor */
    memset(pCur, 0, حجم(*pCur));
    واپس SQLITE_NOMEM_BKPT;
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
  واپس SQLITE_OK;
}

/*
** Free the pBt->pTmpSpace allocation
*/
ساکن خالی free_عارضی_Space(BtShared *pBt){
  اگر( pBt->pTmpSpace ){
    pBt->pTmpSpace -= 4;
    sqlite3_صفحہ_Free(pBt->pTmpSpace);
    pBt->pTmpSpace = 0;
  }
}

/*
** Close an open database and invalidate all cursors.
*/
صحیح sqlite3BtreeClose(Btree *p){
  BtShared *pBt = p->pBt;

  /* Close all cursors opened via this handle.  */
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);

  /* Verify that no other cursors have this Btree open */
#اگر_تعریف_شدہ SQLITE_DEBUG
  {
    BtCursor *pCur = pBt->pCursor;
    جب( pCur ){
      BtCursor *pTmp = pCur;
      pCur = pCur->p_اگلا;
      أكد( pTmp->pBtree!=p );

    }
  }
#اگر_ختم

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
  أكد( p->want_مقصد_تالا==0 && p->locked==0 );
  اگر( !p->sharable || removeFromSharingList(pBt) ){
    /* The pBt is no longer on the sharing list, so we can access
    ** it without having to hold the mutex.
    **
    ** Clean out and delete the BtShared object.
    */
    أكد( !pBt->pCursor );
    sqlite3PagerClose(pBt->pPager, p->db);
    اگر( pBt->xFreeSchema && pBt->pSchema ){
      pBt->xFreeSchema(pBt->pSchema);
    }
    sqlite3DbFree(0, pBt->pSchema);
    free_عارضی_Space(pBt);
    sqlite3_free(pBt);
  }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
  أكد( p->want_مقصد_تالا==0 );
  أكد( p->locked==0 );
  اگر( p->p_پچھلا ) p->p_پچھلا->p_اگلا = p->p_اگلا;
  اگر( p->p_اگلا ) p->p_اگلا->p_پچھلا = p->p_پچھلا;
#اگر_ختم

  sqlite3_free(p);
  واپس SQLITE_OK;
}

/*
** Change the "soft" limit on the number of pages in the cache.
** Unused and unmodified pages will be recycled when the number of
** pages in the cache exceeds this soft limit.  But the size of the
** cache is allowed to grow larger than this limit if it contains
** dirty pages or pages still in active use.
*/
صحیح sqlite3_Btree_سیٹ_Cache_سائز(Btree *p, صحیح mx_صفحہ){
  BtShared *pBt = p->pBt;
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_سیٹ_Cachesize(pBt->pPager, mx_صفحہ);
  sqlite3BtreeLeave(p);
  واپس SQLITE_OK;
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
صحیح sqlite3_Btree_سیٹ_Spill_سائز(Btree *p, صحیح mx_صفحہ){
  BtShared *pBt = p->pBt;
  صحیح res;
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  res = sqlite3_Pager_سیٹ_Spillsize(pBt->pPager, mx_صفحہ);
  sqlite3BtreeLeave(p);
  واپس res;
}

#اگر SQLITE_MAX_MMAP_سائز>0
/*
** Change the limit on the amount of the database file that may be
** memory mapped.
*/
صحیح sqlite3_Btree_سیٹ_Mmap_حد_بندی(Btree *p, sqlite3_int64 szMmap){
  BtShared *pBt = p->pBt;
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_سیٹ_Mmap_حد_بندی(pBt->pPager, szMmap);
  sqlite3BtreeLeave(p);
  واپس SQLITE_OK;
}
#اگر_ختم /* SQLITE_MAX_MMAP_SIZE>0 */

/*
** Change the way data is synced to disk in order to increase or decrease
** how well the database resists damage due to OS crashes and power
** failures.  Level 1 is the same as asynchronous (no syncs() occur and
** there is a high probability of damage)  Level 2 is the default.  There
** is a very low but non-zero probability of damage.  Level 3 reduces the
** probability of damage to near zero but with a write performance reduction.
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_PAGER_PRAGMAS
صحیح sqlite3_Btree_سیٹ_Pager_Flags(
  Btree *p,              /* The btree to set the safety level on */
  غیرعلامتی pgFlags       /* Various PAGER_* flags */
){
  BtShared *pBt = p->pBt;
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_سیٹ_Flags(pBt->pPager, pgFlags);
  sqlite3BtreeLeave(p);
  واپس SQLITE_OK;
}
#اگر_ختم

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
صحیح sqlite3_Btree_سیٹ_صفحہ_سائز(Btree *p, صحیح صفحہ_سائز, صحیح nReserve, صحیح iFix){
  صحیح rc = SQLITE_OK;
  صحیح x;
  BtShared *pBt = p->pBt;
  أكد( nReserve>=0 && nReserve<=255 );
  sqlite3BtreeEnter(p);
  pBt->nReserveWanted = (u8)nReserve;
  x = pBt->صفحہ_سائز - pBt->usable_سائز;
  اگر( x==nReserve && (صفحہ_سائز==0 || (u32)صفحہ_سائز==pBt->صفحہ_سائز) ){
    sqlite3BtreeLeave(p);
    واپس SQLITE_OK;
  }
  اگر( nReserve<x ) nReserve = x;
  اگر( pBt->btsFlags & BTS_PAGESIZE_FIXED ){
    sqlite3BtreeLeave(p);
    واپس SQLITE_READONLY;
  }
  أكد( nReserve>=0 && nReserve<=255 );
  اگر( صفحہ_سائز>=512 && صفحہ_سائز<=SQLITE_MAX_صفحہ_سائز &&
        ((صفحہ_سائز-1)&صفحہ_سائز)==0 ){
    أكد( (صفحہ_سائز & 7)==0 );
    أكد( !pBt->pCursor );
    اگر( nReserve>32 && صفحہ_سائز==512 ) صفحہ_سائز = 1024;
    pBt->صفحہ_سائز = (u32)صفحہ_سائز;
    free_عارضی_Space(pBt);
  }
  rc = sqlite3_Pager_سیٹ_Pagesize(pBt->pPager, &pBt->صفحہ_سائز, nReserve);
  pBt->usable_سائز = pBt->صفحہ_سائز - (u16)nReserve;
  اگر( iFix ) pBt->btsFlags |= BTS_PAGESIZE_FIXED;
  sqlite3BtreeLeave(p);
  واپس rc;
}

/*
** Return the currently defined page size
*/
صحیح sqlite3_Btree_Get_صفحہ_سائز(Btree *p){
  واپس p->pBt->صفحہ_سائز;
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
صحیح sqlite3BtreeGetReserveNoMutex(Btree *p){
  صحیح n;
  أكد( sqlite3_mutex_held(p->pBt->mutex) );
  n = p->pBt->صفحہ_سائز - p->pBt->usable_سائز;
  واپس n;
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
صحیح sqlite3BtreeGetRequestedReserve(Btree *p){
  صحیح n1, n2;
  sqlite3BtreeEnter(p);
  n1 = (صحیح)p->pBt->nReserveWanted;
  n2 = sqlite3BtreeGetReserveNoMutex(p);
  sqlite3BtreeLeave(p);
  واپس n1>n2 ? n1 : n2;
}


/*
** Set the maximum page count for a database if mxPage is positive.
** No changes are made if mxPage is 0 or negative.
** Regardless of the value of mxPage, return the maximum page count.
*/
Pgno sqlite3_Btree_Max_صفحہ_گنتی(Btree *p, Pgno mx_صفحہ){
  Pgno n;
  sqlite3BtreeEnter(p);
  n = sqlite3_Pager_Max_صفحہ_گنتی(p->pBt->pPager, mx_صفحہ);
  sqlite3BtreeLeave(p);
  واپس n;
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
صحیح sqlite3BtreeSecureDelete(Btree *p, صحیح newFlag){
  صحیح b;
  اگر( p==0 ) واپس 0;
  sqlite3BtreeEnter(p);
  أكد( BTS_OVERWRITE==BTS_SECURE_DELETE*2 );
  أكد( BTS_FAST_SECURE==(BTS_OVERWRITE|BTS_SECURE_DELETE) );
  اگر( newFlag>=0 ){
    p->pBt->btsFlags &= ~BTS_FAST_SECURE;
    p->pBt->btsFlags |= (u16)(BTS_SECURE_DELETE*newFlag);
  }
  b = (p->pBt->btsFlags & BTS_FAST_SECURE)/BTS_SECURE_DELETE;
  sqlite3BtreeLeave(p);
  واپس b;
}

/*
** Change the 'auto-vacuum' property of the database. If the 'autoVacuum'
** parameter is non-zero, then auto-vacuum mode is enabled. If zero, it
** is disabled. The default value for the auto-vacuum property is
** determined by the SQLITE_DEFAULT_AUTOVACUUM macro.
*/
صحیح sqlite3_Btree_سیٹ_Auto_Vacuum(Btree *p, صحیح autoVacuum){
#اگر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  واپس SQLITE_READONLY;
#ورنہ
  BtShared *pBt = p->pBt;
  صحیح rc = SQLITE_OK;
  u8 av = (u8)autoVacuum;

  sqlite3BtreeEnter(p);
  اگر( (pBt->btsFlags & BTS_PAGESIZE_FIXED)!=0 && (av ?1:0)!=pBt->autoVacuum ){
    rc = SQLITE_READONLY;
  }ورنہ{
    pBt->autoVacuum = av ?1:0;
    pBt->incrVacuum = av==2 ?1:0;
  }
  sqlite3BtreeLeave(p);
  واپس rc;
#اگر_ختم
}

/*
** Return the value of the 'auto-vacuum' property. If auto-vacuum is
** enabled 1 is returned. Otherwise 0.
*/
صحیح sqlite3BtreeGetAutoVacuum(Btree *p){
#اگر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  واپس BTREE_AUTOVACUUM_NONE;
#ورنہ
  صحیح rc;
  sqlite3BtreeEnter(p);
  rc = (
    (!p->pBt->autoVacuum)?BTREE_AUTOVACUUM_NONE:
    (!p->pBt->incrVacuum)?BTREE_AUTOVACUUM_بھرا:
    BTREE_AUTOVACUUM_INCR
  );
  sqlite3BtreeLeave(p);
  واپس rc;
#اگر_ختم
}

/*
** If the user has not set the safety-level for this database connection
** using "PRAGMA synchronous", and if the safety-level is not already
** set to the value passed to this function as the second parameter,
** set it so.
*/
#اگر SQLITE_DEFAULT_SYNCHRONOUS!=SQLITE_DEFAULT_WAL_SYNCHRONOUS \
    && !defined(SQLITE_OMIT_WAL)
ساکن خالی سیٹ_Default_Sync_Flag(BtShared *pBt, u8 safety_level){
  sqlite3 *db;
  Db *pDb;
  اگر( (db=pBt->db)!=0 && (pDb=db->aDb)!=0 ){
    جب( pDb->pBt==0 || pDb->pBt->pBt!=pBt ){ pDb++; }
    اگر( pDb->b_Sync_سیٹ==0
     && pDb->safety_level!=safety_level
     && pDb!=&db->aDb[1]
    ){
      pDb->safety_level = safety_level;
      sqlite3_Pager_سیٹ_Flags(pBt->pPager,
          pDb->safety_level | (db->flags & PAGER_FLAGS_نقاب));
    }
  }
}
#ورنہ
# define سیٹ_Default_Sync_Flag(pBt,safety_level)
#اگر_ختم

/* Forward declaration */
ساکن صحیح newDatabase(BtShared*);


/*
** Get a reference to pPage1 of the database file.  This will
** also acquire a readlock on that file.
**
** SQLITE_OK is returned on success.  If the file is not a
** well-formed database file, then SQLITE_CORRUPT is returned.
** SQLITE_BUSY is returned if the database is locked.  SQLITE_NOMEM
** is returned if we run out of memory.
*/
ساکن صحیح تالا_Btree(BtShared *pBt){
  صحیح rc;              /* Result code from subfunctions */
  Mem_صفحہ *pPage1;     /* Page 1 of the database file */
  u32 n_صفحہ;           /* Number of pages in the database */
  u32 n_صفحہ_File = 0;   /* Number of pages in the database file */

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( pBt->pPage1==0 );
  rc = sqlite3_Pager_Shared_تالا(pBt->pPager);
  اگر( rc!=SQLITE_OK ) واپس rc;
  rc = btree_Get_صفحہ(pBt, 1, &pPage1, 0);
  اگر( rc!=SQLITE_OK ) واپس rc;

  /* Do some checking to help insure the file we opened really is
  ** a valid database file.
  */
  n_صفحہ = get4byte(28+(u8*)pPage1->a_ڈیٹا);
  sqlite3PagerPagecount(pBt->pPager, (صحیح*)&n_صفحہ_File);
  اگر( n_صفحہ==0 || memcmp(24+(u8*)pPage1->a_ڈیٹا, 92+(u8*)pPage1->a_ڈیٹا,4)!=0 ){
    n_صفحہ = n_صفحہ_File;
  }
  اگر( (pBt->db->flags & SQLITE_ResetDatabase)!=0 ){
    n_صفحہ = 0;
  }
  اگر( n_صفحہ>0 ){
    u32 صفحہ_سائز;
    u32 usable_سائز;
    u8 *page1 = pPage1->a_ڈیٹا;
    rc = SQLITE_NOTADB;
    /* EVIDENCE-OF: R-43737-39999 Every valid SQLite database file begins
    ** with the following 16 bytes (in hex): 53 51 4c 69 74 65 20 66 6f 72 6d
    ** 61 74 20 33 00. */
    اگر( memcmp(page1, z_Magic_سرخی, 16)!=0 ){
      جاؤ page1_init_failed;
    }

#اگر_تعریف_شدہ SQLITE_OMIT_WAL
    اگر( page1[18]>1 ){
      pBt->btsFlags |= BTS_READ_ONLY;
    }
    اگر( page1[19]>1 ){
      جاؤ page1_init_failed;
    }
#ورنہ
    اگر( page1[18]>2 ){
      pBt->btsFlags |= BTS_READ_ONLY;
    }
    اگر( page1[19]>2 ){
      جاؤ page1_init_failed;
    }

    /* If the read version is set to 2, this database should be accessed
    ** in WAL mode. If the log is not already open, open it now. Then
    ** return SQLITE_OK and return without populating BtShared.pPage1.
    ** The caller detects this and calls this function again. This is
    ** required as the version of page 1 currently in the page1 buffer
    ** may not be the latest version - there may be a newer one in the log
    ** file.
    */
    اگر( page1[19]==2 && (pBt->btsFlags & BTS_NO_WAL)==0 ){
      صحیح isOpen = 0;
      rc = sqlite3PagerOpenWal(pBt->pPager, &isOpen);
      اگر( rc!=SQLITE_OK ){
        جاؤ page1_init_failed;
      }ورنہ{
        سیٹ_Default_Sync_Flag(pBt, SQLITE_DEFAULT_WAL_SYNCHRONOUS+1);
        اگر( isOpen==0 ){
          release_صفحہ_One(pPage1);
          واپس SQLITE_OK;
        }
      }
      rc = SQLITE_NOTADB;
    }ورنہ{
      سیٹ_Default_Sync_Flag(pBt, SQLITE_DEFAULT_SYNCHRONOUS+1);
    }
#اگر_ختم

    /* EVIDENCE-OF: R-15465-20813 The maximum and minimum embedded payload
    ** fractions and the leaf payload fraction values must be 64, 32, and 32.
    **
    ** The original design allowed these amounts to vary, but as of
    ** version 3.6.0, we require them to be fixed.
    */
    اگر( memcmp(&page1[21], "\100\040\040",3)!=0 ){
      جاؤ page1_init_failed;
    }
    /* EVIDENCE-OF: R-51873-39618 The page size for a database file is
    ** determined by the 2-byte integer located at an offset of 16 bytes from
    ** the beginning of the database file. */
    صفحہ_سائز = (page1[16]<<8) | (page1[17]<<16);
    /* EVIDENCE-OF: R-25008-21688 The size of a page is a power of two
    ** between 512 and 65536 inclusive. */
    اگر( ((صفحہ_سائز-1)&صفحہ_سائز)!=0
     || صفحہ_سائز>SQLITE_MAX_صفحہ_سائز
     || صفحہ_سائز<=256
    ){
      جاؤ page1_init_failed;
    }
    أكد( (صفحہ_سائز & 7)==0 );
    /* EVIDENCE-OF: R-59310-51205 The "reserved space" size in the 1-byte
    ** integer at offset 20 is the number of bytes of space at the end of
    ** each page to reserve for extensions.
    **
    ** EVIDENCE-OF: R-37497-42412 The size of the reserved region is
    ** determined by the one-byte unsigned integer found at an offset of 20
    ** into the database file header. */
    usable_سائز = صفحہ_سائز - page1[20];
    اگر( (u32)صفحہ_سائز!=pBt->صفحہ_سائز ){
      /* After reading the first page of the database assuming a page size
      ** of BtShared.pageSize, we have discovered that the page-size is
      ** actually pageSize. Unlock the database, leave pBt->pPage1 at
      ** zero and return SQLITE_OK. The caller will call this function
      ** again with the correct page-size.
      */
      release_صفحہ_One(pPage1);
      pBt->usable_سائز = usable_سائز;
      pBt->صفحہ_سائز = صفحہ_سائز;
      pBt->btsFlags |= BTS_PAGESIZE_FIXED;
      free_عارضی_Space(pBt);
      rc = sqlite3_Pager_سیٹ_Pagesize(pBt->pPager, &pBt->صفحہ_سائز,
                                   صفحہ_سائز-usable_سائز);
      واپس rc;
    }
    اگر( n_صفحہ>n_صفحہ_File ){
      اگر( sqlite3WritableSchema(pBt->db)==0 ){
        rc = SQLITE_CORRUPT_BKPT;
        جاؤ page1_init_failed;
      }ورنہ{
        n_صفحہ = n_صفحہ_File;
      }
    }
    /* EVIDENCE-OF: R-28312-64704 However, the usable size is not allowed to
    ** be less than 480. In other words, if the page size is 512, then the
    ** reserved space size cannot exceed 32. */
    اگر( usable_سائز<480 ){
      جاؤ page1_init_failed;
    }
    pBt->btsFlags |= BTS_PAGESIZE_FIXED;
    pBt->صفحہ_سائز = صفحہ_سائز;
    pBt->usable_سائز = usable_سائز;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
    pBt->autoVacuum = (get4byte(&page1[36 + 4*4])?1:0);
    pBt->incrVacuum = (get4byte(&page1[36 + 7*4])?1:0);
#اگر_ختم
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
  pBt->maxLocal = (u16)((pBt->usable_سائز-12)*64/255 - 23);
  pBt->minLocal = (u16)((pBt->usable_سائز-12)*32/255 - 23);
  pBt->max_برگ = (u16)(pBt->usable_سائز - 35);
  pBt->min_برگ = (u16)((pBt->usable_سائز-12)*32/255 - 23);
  اگر( pBt->maxLocal>127 ){
    pBt->max1bytePayload = 127;
  }ورنہ{
    pBt->max1bytePayload = (u8)pBt->maxLocal;
  }
  أكد( pBt->max_برگ + 23 <= MX_CELL_سائز(pBt) );
  pBt->pPage1 = pPage1;
  pBt->n_صفحہ = n_صفحہ;
  واپس SQLITE_OK;

page1_init_failed:
  release_صفحہ_One(pPage1);
  pBt->pPage1 = 0;
  واپس rc;
}

#اگر_غیر_تعریف_شدہ NDEBUG
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
ساکن صحیح گنتی_Valid_Cursors(BtShared *pBt, صحیح wrOnly){
  BtCursor *pCur;
  صحیح r = 0;
  برائے(pCur=pBt->pCursor; pCur; pCur=pCur->p_اگلا){
    اگر( (wrOnly==0 || (pCur->curFlags & BTCF_WriteFlag)!=0)
     && pCur->eState!=CURSOR_FAULT ) r++;
  }
  واپس r;
}
#اگر_ختم

/*
** If there are no outstanding cursors and we are not in the middle
** of a transaction but there is a read lock on the database, then
** this routine unrefs the first page of the database file which
** has the effect of releasing the read lock.
**
** If there is a transaction in progress, this routine is a no-op.
*/
ساکن خالی unlock_Btree_If_غیر_مستعمل(BtShared *pBt){
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( گنتی_Valid_Cursors(pBt,0)==0 || pBt->اندر_Transaction>TRANS_NONE );
  اگر( pBt->اندر_Transaction==TRANS_NONE && pBt->pPage1!=0 ){
    Mem_صفحہ *pPage1 = pBt->pPage1;
    أكد( pPage1->a_ڈیٹا );
    أكد( sqlite3PagerRefcount(pBt->pPager)==1 );
    pBt->pPage1 = 0;
    release_صفحہ_One(pPage1);
  }
}

/*
** If pBt points to an empty file then convert that empty file
** into a new empty database by initializing the first page of
** the database.
*/
ساکن صحیح newDatabase(BtShared *pBt){
  Mem_صفحہ *pP1;
  غیرعلامتی حرف *ڈیٹا;
  صحیح rc;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  اگر( pBt->n_صفحہ>0 ){
    واپس SQLITE_OK;
  }
  pP1 = pBt->pPage1;
  أكد( pP1!=0 );
  ڈیٹا = pP1->a_ڈیٹا;
  rc = sqlite3PagerWrite(pP1->p_Db_صفحہ);
  اگر( rc ) واپس rc;
  حافظہ_نقل(ڈیٹا, z_Magic_سرخی, حجم(z_Magic_سرخی));
  أكد( حجم(z_Magic_سرخی)==16 );
  ڈیٹا[16] = (u8)((pBt->صفحہ_سائز>>8)&0xff);
  ڈیٹا[17] = (u8)((pBt->صفحہ_سائز>>16)&0xff);
  ڈیٹا[18] = 1;
  ڈیٹا[19] = 1;
  أكد( pBt->usable_سائز<=pBt->صفحہ_سائز && pBt->usable_سائز+255>=pBt->صفحہ_سائز);
  ڈیٹا[20] = (u8)(pBt->صفحہ_سائز - pBt->usable_سائز);
  ڈیٹا[21] = 64;
  ڈیٹا[22] = 32;
  ڈیٹا[23] = 32;
  memset(&ڈیٹا[24], 0, 100-24);
  zero_صفحہ(pP1, PTF_INTKEY|PTF_برگ|PTF_LEAFDATA );
  pBt->btsFlags |= BTS_PAGESIZE_FIXED;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  أكد( pBt->autoVacuum==1 || pBt->autoVacuum==0 );
  أكد( pBt->incrVacuum==1 || pBt->incrVacuum==0 );
  put4byte(&ڈیٹا[36 + 4*4], pBt->autoVacuum);
  put4byte(&ڈیٹا[36 + 7*4], pBt->incrVacuum);
#اگر_ختم
  pBt->n_صفحہ = 1;
  ڈیٹا[31] = 1;
  واپس SQLITE_OK;
}

/*
** Initialize the first page of the database file (creating a database
** consisting of a single page and no schema objects). Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
صحیح sqlite3BtreeNewDb(Btree *p){
  صحیح rc;
  sqlite3BtreeEnter(p);
  p->pBt->n_صفحہ = 0;
  rc = newDatabase(p->pBt);
  sqlite3BtreeLeave(p);
  واپس rc;
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
ساکن SQLITE_NOINLINE صحیح btreeBeginTrans(
  Btree *p,                 /* The btree in which to start the transaction */
  صحیح wrflag,               /* True to start a write transaction */
  صحیح *p_Schema_نسخہ       /* Put schema version number here, if not NULL */
){
  BtShared *pBt = p->pBt;
  Pager *pPager = pBt->pPager;
  صحیح rc = SQLITE_OK;

  sqlite3BtreeEnter(p);
  btreeIntegrity(p);

  /* If the btree is already in a write-transaction, or it
  ** is already in a read-transaction and a read-transaction
  ** is requested, this is a no-op.
  */
  اگر( p->اندر_Trans==TRANS_WRITE || (p->اندر_Trans==TRANS_READ && !wrflag) ){
    جاؤ trans_begun;
  }
  أكد( pBt->اندر_Transaction==TRANS_WRITE || IfNotOmitAV(pBt->bDoTruncate)==0 );

  اگر( (p->db->flags & SQLITE_ResetDatabase)
   && sqlite3PagerIsreadonly(pPager)==0
  ){
    pBt->btsFlags &= ~BTS_READ_ONLY;
  }

  /* Write transactions are not possible on a read-only database */
  اگر( (pBt->btsFlags & BTS_READ_ONLY)!=0 && wrflag ){
    rc = SQLITE_READONLY;
    جاؤ trans_begun;
  }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
  {
    sqlite3 *p_بلاک = 0;
    /* If another database handle has already opened a write transaction
    ** on this shared-btree structure and a second write transaction is
    ** requested, return SQLITE_LOCKED.
    */
    اگر( (wrflag && pBt->اندر_Transaction==TRANS_WRITE)
     || (pBt->btsFlags & BTS_PENDING)!=0
    ){
      p_بلاک = pBt->pWriter->db;
    }ورنہ اگر( wrflag>1 ){
      Bt_تالا *pIter;
      برائے(pIter=pBt->p_تالا; pIter; pIter=pIter->p_اگلا){
        اگر( pIter->pBtree!=p ){
          p_بلاک = pIter->pBtree->db;
          توڑو;
        }
      }
    }
    اگر( p_بلاک ){
      sqlite3ConnectionBlocked(p->db, p_بلاک);
      rc = SQLITE_LOCKED_SHAREDCACHE;
      جاؤ trans_begun;
    }
  }
#اگر_ختم

  /* Any read-only or read-write transaction implies a read-lock on
  ** page 1. So if some other shared-cache client already has a write-lock
  ** on page 1, the transaction cannot be opened. */
  rc = query_Shared_Cache_جدول_تالا(p, SCHEMA_جڑ, READ_تالا);
  اگر( SQLITE_OK!=rc ) جاؤ trans_begun;

  pBt->btsFlags &= ~BTS_INITIALLY_خالی_خانے;
  اگر( pBt->n_صفحہ==0 ) pBt->btsFlags |= BTS_INITIALLY_خالی_خانے;
  کرو {
    sqlite3PagerWalDb(pPager, p->db);

#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
    /* If transitioning from no transaction directly to a write transaction,
    ** block for the WRITER lock first if possible. */
    اگر( pBt->pPage1==0 && wrflag ){
      أكد( pBt->اندر_Transaction==TRANS_NONE );
      rc = sqlite3_Pager_Wal_Write_تالا(pPager, 1);
      اگر( rc!=SQLITE_BUSY && rc!=SQLITE_OK ) توڑو;
    }
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_EXPERIMENTAL_PRAGMA_20251114
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
    اگر( pBt->pPage1==0 && wrflag ){
      sqlite3_file *fd = sqlite3PagerFile(pPager);
      حرف *aFcntl[3] = {0,0,0};
      aFcntl[1] = "experimental_pragma_20251114";
      أكد( wrflag==1 || wrflag==2 );
      aFcntl[2] = (wrflag==1 ? "1" : "2");
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_PRAGMA, (خالی*)aFcntl);
      sqlite3_free(aFcntl[0]);
    }
#اگر_ختم

    /* Call lockBtree() until either pBt->pPage1 is populated or
    ** lockBtree() returns something other than SQLITE_OK. lockBtree()
    ** may return SQLITE_OK but leave pBt->pPage1 set to 0 if after
    ** reading page 1 it discovers that the page-size of the database
    ** file is not pBt->pageSize. In this case lockBtree() will update
    ** pBt->pageSize to the page-size of the file on disk.
    */
    جب( pBt->pPage1==0 && SQLITE_OK==(rc = تالا_Btree(pBt)) );

    اگر( rc==SQLITE_OK && wrflag ){
      اگر( (pBt->btsFlags & BTS_READ_ONLY)!=0 ){
        rc = SQLITE_READONLY;
      }ورنہ{
        rc = sqlite3PagerBegin(pPager, wrflag>1, sqlite3_عارضی_اندر_حافظہ(p->db));
        اگر( rc==SQLITE_OK ){
          rc = newDatabase(pBt);
        }ورنہ اگر( rc==SQLITE_BUSY_SNAPSHOT && pBt->اندر_Transaction==TRANS_NONE ){
          /* if there was no transaction opened when this function was
          ** called and SQLITE_BUSY_SNAPSHOT is returned, change the error
          ** code to SQLITE_BUSY. */
          rc = SQLITE_BUSY;
        }
      }
    }
 
    اگر( rc!=SQLITE_OK ){
      (خالی)sqlite3_Pager_Wal_Write_تالا(pPager, 0);
      unlock_Btree_If_غیر_مستعمل(pBt);
    }
#اگر defined(SQLITE_ENABLE_SETLK_TIMEOUT)
    اگر( rc==SQLITE_BUSY_TIMEOUT ){
      /* If a blocking lock timed out, break out of the loop here so that
      ** the busy-handler is not invoked.  */
      توڑو;
    }
#اگر_ختم
  }جب( (rc&0xFF)==SQLITE_BUSY && pBt->اندر_Transaction==TRANS_NONE &&
          btreeInvokeBusyHandler(pBt) );
  sqlite3PagerWalDb(pPager, 0);
#اگر_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
  اگر( rc==SQLITE_BUSY_TIMEOUT ) rc = SQLITE_BUSY;
#اگر_ختم

  اگر( rc==SQLITE_OK ){
    اگر( p->اندر_Trans==TRANS_NONE ){
      pBt->nTransaction++;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
      اگر( p->sharable ){
        أكد( p->تالا.pBtree==p && p->تالا.i_جدول==1 );
        p->تالا.e_تالا = READ_تالا;
        p->تالا.p_اگلا = pBt->p_تالا;
        pBt->p_تالا = &p->تالا;
      }
#اگر_ختم
    }
    p->اندر_Trans = (wrflag?TRANS_WRITE:TRANS_READ);
    اگر( p->اندر_Trans>pBt->اندر_Transaction ){
      pBt->اندر_Transaction = p->اندر_Trans;
    }
    اگر( wrflag ){
      Mem_صفحہ *pPage1 = pBt->pPage1;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
      أكد( !pBt->pWriter );
      pBt->pWriter = p;
      pBt->btsFlags &= ~BTS_EXCLUSIVE;
      اگر( wrflag>1 ) pBt->btsFlags |= BTS_EXCLUSIVE;
#اگر_ختم

      /* If the db-size header field is incorrect (as it may be if an old
      ** client has been writing the database file), update it now. Doing
      ** this sooner rather than later means the database size can safely
      ** re-read the database size from page 1 if a savepoint or transaction
      ** rollback occurs within the transaction.
      */
      اگر( pBt->n_صفحہ!=get4byte(&pPage1->a_ڈیٹا[28]) ){
        rc = sqlite3PagerWrite(pPage1->p_Db_صفحہ);
        اگر( rc==SQLITE_OK ){
          put4byte(&pPage1->a_ڈیٹا[28], pBt->n_صفحہ);
        }
      }
    }
  }

trans_begun:
  اگر( rc==SQLITE_OK ){
    اگر( p_Schema_نسخہ ){
      *p_Schema_نسخہ = get4byte(&pBt->pPage1->a_ڈیٹا[40]);
    }
    اگر( wrflag ){
      /* This call makes sure that the pager has the correct number of
      ** open savepoints. If the second parameter is greater than 0 and
      ** the sub-journal is not already open, then it will be opened here.
      */
      rc = sqlite3PagerOpenSavepoint(pPager, p->db->nSavepoint);
    }
  }

  btreeIntegrity(p);
  sqlite3BtreeLeave(p);
  واپس rc;
}
صحیح sqlite3BtreeBeginTrans(Btree *p, صحیح wrflag, صحیح *p_Schema_نسخہ){
  BtShared *pBt;
  اگر( p->sharable
   || p->اندر_Trans==TRANS_NONE
   || (p->اندر_Trans==TRANS_READ && wrflag!=0)
  ){
    واپس btreeBeginTrans(p,wrflag,p_Schema_نسخہ);
  }
  pBt = p->pBt;
  اگر( p_Schema_نسخہ ){
    *p_Schema_نسخہ = get4byte(&pBt->pPage1->a_ڈیٹا[40]);
  }
  اگر( wrflag ){
    /* This call makes sure that the pager has the correct number of
    ** open savepoints. If the second parameter is greater than 0 and
    ** the sub-journal is not already open, then it will be opened here.
    */
    واپس sqlite3PagerOpenSavepoint(pBt->pPager, p->db->nSavepoint);
  }ورنہ{
    واپس SQLITE_OK;
  }
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM

/*
** Set the pointer-map entries for all children of page pPage. Also, if
** pPage contains cells that point to overflow pages, set the pointer
** map entries for the overflow pages as well.
*/
ساکن صحیح سیٹ_بچہ_Ptrmaps(Mem_صفحہ *p_صفحہ){
  صحیح i;                             /* Counter variable */
  صحیح nCell;                         /* Number of cells in page pPage */
  صحیح rc;                            /* Return code */
  BtShared *pBt = p_صفحہ->pBt;
  Pgno pgno = p_صفحہ->pgno;

  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  rc = p_صفحہ->isInit ? SQLITE_OK : btree_Init_صفحہ(p_صفحہ);
  اگر( rc!=SQLITE_OK ) واپس rc;
  nCell = p_صفحہ->nCell;

  برائے(i=0; i<nCell; i++){
    u8 *pCell = ڈھونڈو_Cell(p_صفحہ, i);

    ptrmapPutOvflPtr(p_صفحہ, p_صفحہ, pCell, &rc);

    اگر( !p_صفحہ->برگ ){
      Pgno بچہ_Pgno = get4byte(pCell);
      ptrmapPut(pBt, بچہ_Pgno, PTRMAP_BTREE, pgno, &rc);
    }
  }

  اگر( !p_صفحہ->برگ ){
    Pgno بچہ_Pgno = get4byte(&p_صفحہ->a_ڈیٹا[p_صفحہ->hdr_انحراف+8]);
    ptrmapPut(pBt, بچہ_Pgno, PTRMAP_BTREE, pgno, &rc);
  }

  واپس rc;
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
ساکن صحیح modify_صفحہ_اشارہ_گر(Mem_صفحہ *p_صفحہ, Pgno iFrom, Pgno i_مقصد, u8 eType){
  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  أكد( sqlite3PagerIswriteable(p_صفحہ->p_Db_صفحہ) );
  اگر( eType==PTRMAP_OVERFLOW2 ){
    /* The pointer is always the first 4 bytes of the page in this case.  */
    اگر( get4byte(p_صفحہ->a_ڈیٹا)!=iFrom ){
      واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }
    put4byte(p_صفحہ->a_ڈیٹا, i_مقصد);
  }ورنہ{
    صحیح i;
    صحیح nCell;
    صحیح rc;

    rc = p_صفحہ->isInit ? SQLITE_OK : btree_Init_صفحہ(p_صفحہ);
    اگر( rc ) واپس rc;
    nCell = p_صفحہ->nCell;

    برائے(i=0; i<nCell; i++){
      u8 *pCell = ڈھونڈو_Cell(p_صفحہ, i);
      اگر( eType==PTRMAP_OVERFLOW1 ){
        CellInfo info;
        p_صفحہ->xParseCell(p_صفحہ, pCell, &info);
        اگر( info.nLocal<info.nPayload ){
          اگر( pCell+info.n_سائز > p_صفحہ->a_ڈیٹا+p_صفحہ->pBt->usable_سائز ){
            واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
          }
          اگر( iFrom==get4byte(pCell+info.n_سائز-4) ){
            put4byte(pCell+info.n_سائز-4, i_مقصد);
            توڑو;
          }
        }
      }ورنہ{
        اگر( pCell+4 > p_صفحہ->a_ڈیٹا+p_صفحہ->pBt->usable_سائز ){
          واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
        }
        اگر( get4byte(pCell)==iFrom ){
          put4byte(pCell, i_مقصد);
          توڑو;
        }
      }
    }
 
    اگر( i==nCell ){
      اگر( eType!=PTRMAP_BTREE ||
          get4byte(&p_صفحہ->a_ڈیٹا[p_صفحہ->hdr_انحراف+8])!=iFrom ){
        واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
      }
      put4byte(&p_صفحہ->a_ڈیٹا[p_صفحہ->hdr_انحراف+8], i_مقصد);
    }
  }
  واپس SQLITE_OK;
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
ساکن صحیح relocate_صفحہ(
  BtShared *pBt,           /* Btree */
  Mem_صفحہ *p_Db_صفحہ,        /* Open page to move */
  u8 eType,                /* Pointer map 'type' entry for pDbPage */
  Pgno i_Ptr_صفحہ,           /* Pointer map 'page-no' entry for pDbPage */
  Pgno i_Free_صفحہ,          /* The location to move pDbPage to */
  صحیح isCommit             /* isCommit flag passed to sqlite3PagerMovepage */
){
  Mem_صفحہ *p_Ptr_صفحہ;   /* The page that contains a pointer to pDbPage */
  Pgno i_Db_صفحہ = p_Db_صفحہ->pgno;
  Pager *pPager = pBt->pPager;
  صحیح rc;

  أكد( eType==PTRMAP_OVERFLOW2 || eType==PTRMAP_OVERFLOW1 ||
      eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE );
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( p_Db_صفحہ->pBt==pBt );
  اگر( i_Db_صفحہ<3 ) واپس SQLITE_CORRUPT_BKPT;

  /* Move page iDbPage from its current location to page number iFreePage */
  TRACE(("AUTOVACUUM: Moving %u to free page %u (ptr page %u type %u)\n",
      i_Db_صفحہ, i_Free_صفحہ, i_Ptr_صفحہ, eType));
  rc = sqlite3PagerMovepage(pPager, p_Db_صفحہ->p_Db_صفحہ, i_Free_صفحہ, isCommit);
  اگر( rc!=SQLITE_OK ){
    واپس rc;
  }
  p_Db_صفحہ->pgno = i_Free_صفحہ;

  /* If pDbPage was a btree-page, then it may have child pages and/or cells
  ** that point to overflow pages. The pointer map entries for all these
  ** pages need to be changed.
  **
  ** If pDbPage is an overflow page, then the first 4 bytes may store a
  ** pointer to a subsequent overflow page. If this is the case, then
  ** the pointer map needs to be updated for the subsequent overflow page.
  */
  اگر( eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE ){
    rc = سیٹ_بچہ_Ptrmaps(p_Db_صفحہ);
    اگر( rc!=SQLITE_OK ){
      واپس rc;
    }
  }ورنہ{
    Pgno اگلا_Ovfl = get4byte(p_Db_صفحہ->a_ڈیٹا);
    اگر( اگلا_Ovfl!=0 ){
      ptrmapPut(pBt, اگلا_Ovfl, PTRMAP_OVERFLOW2, i_Free_صفحہ, &rc);
      اگر( rc!=SQLITE_OK ){
        واپس rc;
      }
    }
  }

  /* Fix the database pointer on page iPtrPage that pointed at iDbPage so
  ** that it points at iFreePage. Also fix the pointer map entry for
  ** iPtrPage.
  */
  اگر( eType!=PTRMAP_ROOTPAGE ){
    rc = btree_Get_صفحہ(pBt, i_Ptr_صفحہ, &p_Ptr_صفحہ, 0);
    اگر( rc!=SQLITE_OK ){
      واپس rc;
    }
    rc = sqlite3PagerWrite(p_Ptr_صفحہ->p_Db_صفحہ);
    اگر( rc!=SQLITE_OK ){
      release_صفحہ(p_Ptr_صفحہ);
      واپس rc;
    }
    rc = modify_صفحہ_اشارہ_گر(p_Ptr_صفحہ, i_Db_صفحہ, i_Free_صفحہ, eType);
    release_صفحہ(p_Ptr_صفحہ);
    اگر( rc==SQLITE_OK ){
      ptrmapPut(pBt, i_Free_صفحہ, eType, i_Ptr_صفحہ, &rc);
    }
  }
  واپس rc;
}

/* Forward declaration required by incrVacuumStep(). */
ساکن صحیح allocate_Btree_صفحہ(BtShared *, Mem_صفحہ **, Pgno *, Pgno, u8);

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
ساکن صحیح incr_Vacuum_قدم(BtShared *pBt, Pgno nFin, Pgno i_آخری_Pg, صحیح bCommit){
  Pgno nFreeList;           /* Number of pages still on the free-list */
  صحیح rc;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( i_آخری_Pg>nFin );

  اگر( !PTRMAP_ISPAGE(pBt, i_آخری_Pg) && i_آخری_Pg!=PENDING_BYTE_صفحہ(pBt) ){
    u8 eType;
    Pgno i_Ptr_صفحہ;

    nFreeList = get4byte(&pBt->pPage1->a_ڈیٹا[36]);
    اگر( nFreeList==0 ){
      واپس SQLITE_مکمل;
    }

    rc = ptrmapGet(pBt, i_آخری_Pg, &eType, &i_Ptr_صفحہ);
    اگر( rc!=SQLITE_OK ){
      واپس rc;
    }
    اگر( eType==PTRMAP_ROOTPAGE ){
      واپس SQLITE_CORRUPT_BKPT;
    }

    اگر( eType==PTRMAP_FREEPAGE ){
      اگر( bCommit==0 ){
        /* Remove the page from the files free-list. This is not required
        ** if bCommit is non-zero. In that case, the free-list will be
        ** truncated to zero after this function returns, so it doesn't
        ** matter if it still contains some garbage entries.
        */
        Pgno iFreePg;
        Mem_صفحہ *pFreePg;
        rc = allocate_Btree_صفحہ(pBt, &pFreePg, &iFreePg, i_آخری_Pg, BTALLOC_EXACT);
        اگر( rc!=SQLITE_OK ){
          واپس rc;
        }
        أكد( iFreePg==i_آخری_Pg );
        release_صفحہ(pFreePg);
      }
    } ورنہ {
      Pgno iFreePg;             /* Index of free page to move pLastPg to */
      Mem_صفحہ *p_آخری_Pg;
      u8 e_منوال = BTALLOC_ANY;   /* Mode parameter for allocateBtreePage() */
      Pgno iNear = 0;           /* nearby parameter for allocateBtreePage() */

      rc = btree_Get_صفحہ(pBt, i_آخری_Pg, &p_آخری_Pg, 0);
      اگر( rc!=SQLITE_OK ){
        واپس rc;
      }

      /* If bCommit is zero, this loop runs exactly once and page pLastPg
      ** is swapped with the first free page pulled off the free list.
      **
      ** On the other hand, if bCommit is greater than zero, then keep
      ** looping until a free-page located within the first nFin pages
      ** of the file is found.
      */
      اگر( bCommit==0 ){
        e_منوال = BTALLOC_LE;
        iNear = nFin;
      }
      کرو {
        Mem_صفحہ *pFreePg;
        Pgno db_سائز = btreePagecount(pBt);
        rc = allocate_Btree_صفحہ(pBt, &pFreePg, &iFreePg, iNear, e_منوال);
        اگر( rc!=SQLITE_OK ){
          release_صفحہ(p_آخری_Pg);
          واپس rc;
        }
        release_صفحہ(pFreePg);
        اگر( iFreePg>db_سائز ){
          release_صفحہ(p_آخری_Pg);
          واپس SQLITE_CORRUPT_BKPT;
        }
      }جب( bCommit && iFreePg>nFin );
      أكد( iFreePg<i_آخری_Pg );
     
      rc = relocate_صفحہ(pBt, p_آخری_Pg, eType, i_Ptr_صفحہ, iFreePg, bCommit);
      release_صفحہ(p_آخری_Pg);
      اگر( rc!=SQLITE_OK ){
        واپس rc;
      }
    }
  }

  اگر( bCommit==0 ){
    کرو {
      i_آخری_Pg--;
    }جب( i_آخری_Pg==PENDING_BYTE_صفحہ(pBt) || PTRMAP_ISPAGE(pBt, i_آخری_Pg) );
    pBt->bDoTruncate = 1;
    pBt->n_صفحہ = i_آخری_Pg;
  }
  واپس SQLITE_OK;
}

/*
** The database opened by the first argument is an auto-vacuum database
** nOrig pages in size containing nFree free pages. Return the expected
** size of the database in pages following an auto-vacuum operation.
*/
ساکن Pgno final_Db_سائز(BtShared *pBt, Pgno nOrig, Pgno nFree){
  صحیح n_اندراج;                     /* Number of entries on one ptrmap page */
  Pgno nPtrmap;                   /* Number of PtrMap pages to be freed */
  Pgno nFin;                      /* Return value */

  n_اندراج = pBt->usable_سائز/5;
  nPtrmap = (nFree-nOrig+PTRMAP_PAGENO(pBt, nOrig)+n_اندراج)/n_اندراج;
  nFin = nOrig - nFree - nPtrmap;
  اگر( nOrig>PENDING_BYTE_صفحہ(pBt) && nFin<PENDING_BYTE_صفحہ(pBt) ){
    nFin--;
  }
  جب( PTRMAP_ISPAGE(pBt, nFin) || nFin==PENDING_BYTE_صفحہ(pBt) ){
    nFin--;
  }

  واپس nFin;
}

/*
** A write-transaction must be opened before calling this function.
** It performs a single unit of work towards an incremental vacuum.
**
** If the incremental vacuum is finished after this function has run,
** SQLITE_DONE is returned. If it is not finished, but no error occurred,
** SQLITE_OK is returned. Otherwise an SQLite error code.
*/
صحیح sqlite3BtreeIncrVacuum(Btree *p){
  صحیح rc;
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  أكد( pBt->اندر_Transaction==TRANS_WRITE && p->اندر_Trans==TRANS_WRITE );
  اگر( !pBt->autoVacuum ){
    rc = SQLITE_مکمل;
  }ورنہ{
    Pgno nOrig = btreePagecount(pBt);
    Pgno nFree = get4byte(&pBt->pPage1->a_ڈیٹا[36]);
    Pgno nFin = final_Db_سائز(pBt, nOrig, nFree);

    اگر( nOrig<nFin || nFree>=nOrig ){
      rc = SQLITE_CORRUPT_BKPT;
    }ورنہ اگر( nFree>0 ){
      rc = saveAllCursors(pBt, 0, 0);
      اگر( rc==SQLITE_OK ){
        invalidate_All_حد_سے_تجاوز_Cache(pBt);
        rc = incr_Vacuum_قدم(pBt, nFin, nOrig, 0);
      }
      اگر( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->p_Db_صفحہ);
        put4byte(&pBt->pPage1->a_ڈیٹا[28], pBt->n_صفحہ);
      }
    }ورنہ{
      rc = SQLITE_مکمل;
    }
  }
  sqlite3BtreeLeave(p);
  واپس rc;
}

/*
** This routine is called prior to sqlite3PagerCommit when a transaction
** is committed for an auto-vacuum database.
*/
ساکن صحیح autoVacuumCommit(Btree *p){
  صحیح rc = SQLITE_OK;
  Pager *pPager;
  BtShared *pBt;
  sqlite3 *db;
  VVA_ONLY( صحیح nRef );

  أكد( p!=0 );
  pBt = p->pBt; 
  pPager = pBt->pPager;
  VVA_ONLY( nRef = sqlite3PagerRefcount(pPager); )

  أكد( sqlite3_mutex_held(pBt->mutex) );
  invalidate_All_حد_سے_تجاوز_Cache(pBt);
  أكد(pBt->autoVacuum);
  اگر( !pBt->incrVacuum ){
    Pgno nFin;         /* Number of pages in database after autovacuuming */
    Pgno nFree;        /* Number of pages on the freelist initially */
    Pgno nVac;         /* Number of pages to vacuum */
    Pgno iFree;        /* The next page to be freed */
    Pgno nOrig;        /* Database size before freeing */

    nOrig = btreePagecount(pBt);
    اگر( PTRMAP_ISPAGE(pBt, nOrig) || nOrig==PENDING_BYTE_صفحہ(pBt) ){
      /* It is not possible to create a database for which the final page
      ** is either a pointer-map page or the pending-byte page. If one
      ** is encountered, this indicates corruption.
      */
      واپس SQLITE_CORRUPT_BKPT;
    }

    nFree = get4byte(&pBt->pPage1->a_ڈیٹا[36]);
    db = p->db;
    اگر( db->x_Autovac_صفحات ){
      صحیح iDb;
      برائے(iDb=0; ALWAYS(iDb<db->nDb); iDb++){
        اگر( db->aDb[iDb].pBt==p ) توڑو;
      }
      nVac = db->x_Autovac_صفحات(
        db->p_Autovac_صفحات_Arg,
        db->aDb[iDb].zDbSName,
        nOrig,
        nFree,
        pBt->صفحہ_سائز
      );
      اگر( nVac>nFree ){
        nVac = nFree;
      }
      اگر( nVac==0 ){
        واپس SQLITE_OK;
      }
    }ورنہ{
      nVac = nFree;
    }
    nFin = final_Db_سائز(pBt, nOrig, nVac);
    اگر( nFin>nOrig ) واپس SQLITE_CORRUPT_BKPT;
    اگر( nFin<nOrig ){
      rc = saveAllCursors(pBt, 0, 0);
    }
    برائے(iFree=nOrig; iFree>nFin && rc==SQLITE_OK; iFree--){
      rc = incr_Vacuum_قدم(pBt, nFin, iFree, nVac==nFree);
    }
    اگر( (rc==SQLITE_مکمل || rc==SQLITE_OK) && nFree>0 ){
      rc = sqlite3PagerWrite(pBt->pPage1->p_Db_صفحہ);
      اگر( nVac==nFree ){
        put4byte(&pBt->pPage1->a_ڈیٹا[32], 0);
        put4byte(&pBt->pPage1->a_ڈیٹا[36], 0);
      }
      put4byte(&pBt->pPage1->a_ڈیٹا[28], nFin);
      pBt->bDoTruncate = 1;
      pBt->n_صفحہ = nFin;
    }
    اگر( rc!=SQLITE_OK ){
      sqlite3PagerRollback(pPager);
    }
  }

  أكد( nRef>=sqlite3PagerRefcount(pPager) );
  واپس rc;
}

#ورنہ /* ifndef SQLITE_OMIT_AUTOVACUUM */
# define سیٹ_بچہ_Ptrmaps(x) SQLITE_OK
#اگر_ختم

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
صحیح sqlite3BtreeCommitPhaseOne(Btree *p, ثابت حرف *zSuperJrnl){
  صحیح rc = SQLITE_OK;
  اگر( p->اندر_Trans==TRANS_WRITE ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
    اگر( pBt->autoVacuum ){
      rc = autoVacuumCommit(p);
      اگر( rc!=SQLITE_OK ){
        sqlite3BtreeLeave(p);
        واپس rc;
      }
    }
    اگر( pBt->bDoTruncate ){
      sqlite3PagerTruncateImage(pBt->pPager, pBt->n_صفحہ);
    }
#اگر_ختم
    rc = sqlite3PagerCommitPhaseOne(pBt->pPager, zSuperJrnl, 0);
    sqlite3BtreeLeave(p);
  }
  واپس rc;
}

/*
** This function is called from both BtreeCommitPhaseTwo() and BtreeRollback()
** at the conclusion of a transaction.
*/
ساکن خالی btree_انتہا_Transaction(Btree *p){
  BtShared *pBt = p->pBt;
  sqlite3 *db = p->db;
  أكد( sqlite3BtreeHoldsMutex(p) );

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  pBt->bDoTruncate = 0;
#اگر_ختم
  اگر( p->اندر_Trans>TRANS_NONE && db->nVdbeRead>1 ){
    /* If there are other active statements that belong to this database
    ** handle, downgrade to a read-only transaction. The other statements
    ** may still be reading from the database.  */
    downgrade_All_Shared_Cache_جدول_Locks(p);
    p->اندر_Trans = TRANS_READ;
  }ورنہ{
    /* If the handle had any kind of transaction open, decrement the
    ** transaction count of the shared btree. If the transaction count
    ** reaches 0, set the shared state to TRANS_NONE. The unlockBtreeIfUnused()
    ** call below will unlock the pager.  */
    اگر( p->اندر_Trans!=TRANS_NONE ){
      clear_All_Shared_Cache_جدول_Locks(p);
      pBt->nTransaction--;
      اگر( 0==pBt->nTransaction ){
        pBt->اندر_Transaction = TRANS_NONE;
      }
    }

    /* Set the current transaction state to TRANS_NONE and unlock the
    ** pager if this call closed the only read or write transaction.  */
    p->اندر_Trans = TRANS_NONE;
    unlock_Btree_If_غیر_مستعمل(pBt);
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
صحیح sqlite3BtreeCommitPhaseTwo(Btree *p, صحیح bCleanup){

  اگر( p->اندر_Trans==TRANS_NONE ) واپس SQLITE_OK;
  sqlite3BtreeEnter(p);
  btreeIntegrity(p);

  /* If the handle has a write-transaction open, commit the shared-btrees
  ** transaction and set the shared state to TRANS_READ.
  */
  اگر( p->اندر_Trans==TRANS_WRITE ){
    صحیح rc;
    BtShared *pBt = p->pBt;
    أكد( pBt->اندر_Transaction==TRANS_WRITE );
    أكد( pBt->nTransaction>0 );
    rc = sqlite3PagerCommitPhaseTwo(pBt->pPager);
    اگر( rc!=SQLITE_OK && bCleanup==0 ){
      sqlite3BtreeLeave(p);
      واپس rc;
    }
    p->i_BData_نسخہ--;  /* Compensate for pPager->iDataVersion++; */
    pBt->اندر_Transaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btree_انتہا_Transaction(p);
  sqlite3BtreeLeave(p);
  واپس SQLITE_OK;
}

/*
** Do both phases of a commit.
*/
صحیح sqlite3BtreeCommit(Btree *p){
  صحیح rc;
  sqlite3BtreeEnter(p);
  rc = sqlite3BtreeCommitPhaseOne(p, 0);
  اگر( rc==SQLITE_OK ){
    rc = sqlite3BtreeCommitPhaseTwo(p, 0);
  }
  sqlite3BtreeLeave(p);
  واپس rc;
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
صحیح sqlite3BtreeTripAllCursors(Btree *pBtree, صحیح err_رمز, صحیح writeOnly){
  BtCursor *p;
  صحیح rc = SQLITE_OK;

  أكد( (writeOnly==0 || writeOnly==1) && BTCF_WriteFlag==1 );
  اگر( pBtree ){
    sqlite3BtreeEnter(pBtree);
    برائے(p=pBtree->pBt->pCursor; p; p=p->p_اگلا){
      اگر( writeOnly && (p->curFlags & BTCF_WriteFlag)==0 ){
        اگر( p->eState==CURSOR_VALID || p->eState==CURSOR_SKIPNEXT ){
          rc = saveCursorPosition(p);
          اگر( rc!=SQLITE_OK ){
            (خالی)sqlite3BtreeTripAllCursors(pBtree, rc, 0);
            توڑو;
          }
        }
      }ورنہ{
        sqlite3BtreeClearCursor(p);
        p->eState = CURSOR_FAULT;
        p->skip_اگلا = err_رمز;
      }
      btree_Release_All_Cursor_صفحات(p);
    }
    sqlite3BtreeLeave(pBtree);
  }
  واپس rc;
}

/*
** Set the pBt->nPage field correctly, according to the current
** state of the database.  Assume pBt->pPage1 is valid.
*/
ساکن خالی btree_سیٹ_NPage(BtShared *pBt, Mem_صفحہ *pPage1){
  صحیح n_صفحہ = get4byte(&pPage1->a_ڈیٹا[28]);
  testcase( n_صفحہ==0 );
  اگر( n_صفحہ==0 ) sqlite3PagerPagecount(pBt->pPager, &n_صفحہ);
  testcase( pBt->n_صفحہ!=(u32)n_صفحہ );
  pBt->n_صفحہ = n_صفحہ;
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
صحیح sqlite3BtreeRollback(Btree *p, صحیح trip_رمز, صحیح writeOnly){
  صحیح rc;
  BtShared *pBt = p->pBt;
  Mem_صفحہ *pPage1;

  أكد( writeOnly==1 || writeOnly==0 );
  أكد( trip_رمز==SQLITE_ABORT_ROLLBACK || trip_رمز==SQLITE_OK );
  sqlite3BtreeEnter(p);
  اگر( trip_رمز==SQLITE_OK ){
    rc = trip_رمز = saveAllCursors(pBt, 0, 0);
    اگر( rc ) writeOnly = 0;
  }ورنہ{
    rc = SQLITE_OK;
  }
  اگر( trip_رمز ){
    صحیح rc2 = sqlite3BtreeTripAllCursors(p, trip_رمز, writeOnly);
    أكد( rc==SQLITE_OK || (writeOnly==0 && rc2==SQLITE_OK) );
    اگر( rc2!=SQLITE_OK ) rc = rc2;
  }
  btreeIntegrity(p);

  اگر( p->اندر_Trans==TRANS_WRITE ){
    صحیح rc2;

    أكد( TRANS_WRITE==pBt->اندر_Transaction );
    rc2 = sqlite3PagerRollback(pBt->pPager);
    اگر( rc2!=SQLITE_OK ){
      rc = rc2;
    }

    /* The rollback may have destroyed the pPage1->aData value.  So
    ** call btreeGetPage() on page 1 again to make
    ** sure pPage1->aData is set correctly. */
    اگر( btree_Get_صفحہ(pBt, 1, &pPage1, 0)==SQLITE_OK ){
      btree_سیٹ_NPage(pBt, pPage1);
      release_صفحہ_One(pPage1);
    }
    أكد( گنتی_Valid_Cursors(pBt, 1)==0 );
    pBt->اندر_Transaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btree_انتہا_Transaction(p);
  sqlite3BtreeLeave(p);
  واپس rc;
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
صحیح sqlite3BtreeBeginStmt(Btree *p, صحیح iStatement){
  صحیح rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);
  أكد( p->اندر_Trans==TRANS_WRITE );
  أكد( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  أكد( iStatement>0 );
  أكد( iStatement>p->db->nSavepoint );
  أكد( pBt->اندر_Transaction==TRANS_WRITE );
  /* At the pager level, a statement transaction is a savepoint with
  ** an index greater than all savepoints created explicitly using
  ** SQL statements. It is illegal to open, release or rollback any
  ** such savepoints while the statement transaction savepoint is active.
  */
  rc = sqlite3PagerOpenSavepoint(pBt->pPager, iStatement);
  sqlite3BtreeLeave(p);
  واپس rc;
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
صحیح sqlite3BtreeSavepoint(Btree *p, صحیح op, صحیح iSavepoint){
  صحیح rc = SQLITE_OK;
  اگر( p && p->اندر_Trans==TRANS_WRITE ){
    BtShared *pBt = p->pBt;
    أكد( op==SAVEPOINT_RELEASE || op==SAVEPOINT_ROLLBACK );
    أكد( iSavepoint>=0 || (iSavepoint==-1 && op==SAVEPOINT_ROLLBACK) );
    sqlite3BtreeEnter(p);
    اگر( op==SAVEPOINT_ROLLBACK ){
      rc = saveAllCursors(pBt, 0, 0);
    }
    اگر( rc==SQLITE_OK ){
      rc = sqlite3PagerSavepoint(pBt->pPager, op, iSavepoint);
    }
    اگر( rc==SQLITE_OK ){
      اگر( iSavepoint<0 && (pBt->btsFlags & BTS_INITIALLY_خالی_خانے)!=0 ){
        pBt->n_صفحہ = 0;
      }
      rc = newDatabase(pBt);
      btree_سیٹ_NPage(pBt, pBt->pPage1);

      /* pBt->nPage might be zero if the database was corrupt when
      ** the transaction was started. Otherwise, it must be at least 1.  */
      أكد( CORRUPT_DB || pBt->n_صفحہ>0 );
    }
    sqlite3BtreeLeave(p);
  }
  واپس rc;
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
ساکن صحیح btreeCursor(
  Btree *p,                              /* The btree */
  Pgno i_جدول,                           /* Root page of table to open */
  صحیح wrFlag,                            /* 1 to write. 0 read-only */
  ساخت کلید_Info *p_کلید_Info,              /* First arg to comparison function */
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
  أكد( has_Shared_Cache_جدول_تالا(p, i_جدول, p_کلید_Info!=0, (wrFlag?2:1))
          || i_جدول<1 );
  أكد( wrFlag==0 || !hasReadConflicts(p, i_جدول) );

  /* Assert that the caller has opened the required transaction. */
  أكد( p->اندر_Trans>TRANS_NONE );
  أكد( wrFlag==0 || p->اندر_Trans==TRANS_WRITE );
  أكد( pBt->pPage1 && pBt->pPage1->a_ڈیٹا );
  أكد( wrFlag==0 || (pBt->btsFlags & BTS_READ_ONLY)==0 );

  اگر( i_جدول<=1 ){
    اگر( i_جدول<1 ){
      واپس SQLITE_CORRUPT_BKPT;
    }ورنہ اگر( btreePagecount(pBt)==0 ){
      أكد( wrFlag==0 );
      i_جدول = 0;
    }
  }

  /* Now that no other errors can occur, finish filling in the BtCursor
  ** variables and link the cursor into the BtShared list.  */
  pCur->pgno_جڑ = i_جدول;
  pCur->i_صفحہ = -1;
  pCur->p_کلید_Info = p_کلید_Info;
  pCur->pBtree = p;
  pCur->pBt = pBt;
  pCur->curFlags = 0;
  /* If there are two or more cursors on the same btree, then all such
  ** cursors *must* have the BTCF_Multiple flag set. */
  برائے(pX=pBt->pCursor; pX; pX=pX->p_اگلا){
    اگر( pX->pgno_جڑ==i_جدول ){
      pX->curFlags |= BTCF_مضرب;
      pCur->curFlags = BTCF_مضرب;
    }
  }
  pCur->eState = CURSOR_INVALID;
  pCur->p_اگلا = pBt->pCursor;
  pBt->pCursor = pCur;
  اگر( wrFlag ){
    pCur->curFlags |= BTCF_WriteFlag;
    pCur->curPagerFlags = 0;
    اگر( pBt->pTmpSpace==0 ) واپس allocate_عارضی_Space(pBt);
  }ورنہ{
    pCur->curPagerFlags = PAGER_GET_READONLY;
  }
  واپس SQLITE_OK;
}
ساکن صحیح btree_Cursor_With_تالا(
  Btree *p,                              /* The btree */
  Pgno i_جدول,                           /* Root page of table to open */
  صحیح wrFlag,                            /* 1 to write. 0 read-only */
  ساخت کلید_Info *p_کلید_Info,              /* First arg to comparison function */
  BtCursor *pCur                         /* Space for new cursor */
){
  صحیح rc;
  sqlite3BtreeEnter(p);
  rc = btreeCursor(p, i_جدول, wrFlag, p_کلید_Info, pCur);
  sqlite3BtreeLeave(p);
  واپس rc;
}
صحیح sqlite3BtreeCursor(
  Btree *p,                                   /* The btree */
  Pgno i_جدول,                                /* Root page of table to open */
  صحیح wrFlag,                                 /* 1 to write. 0 read-only */
  ساخت کلید_Info *p_کلید_Info,                   /* First arg to xCompare() */
  BtCursor *pCur                              /* Write new cursor here */
){
  اگر( p->sharable ){
    واپس btree_Cursor_With_تالا(p, i_جدول, wrFlag, p_کلید_Info, pCur);
  }ورنہ{
    واپس btreeCursor(p, i_جدول, wrFlag, p_کلید_Info, pCur);
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
صحیح sqlite3_Btree_Cursor_سائز(خالی){
  واپس ROUND8(حجم(BtCursor));
}

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** Return true if and only if the Btree object will be automatically
** closed with the BtCursor closes.  This is used within assert() statements
** only.
*/
صحیح sqlite3BtreeClosesWithCursor(
  Btree *pBtree,       /* the btree object */
  BtCursor *pCur       /* Corresponding cursor */
){
  BtShared *pBt = pBtree->pBt;
  اگر( (pBt->openFlags & BTREE_SINGLE)==0 ) واپس 0;
  اگر( pBt->pCursor!=pCur ) واپس 0;
  اگر( pCur->p_اگلا!=0 ) واپس 0;
  اگر( pCur->pBtree!=pBtree ) واپس 0;
  واپس 1;
}
#اگر_ختم

/*
** Initialize memory that will be converted into a BtCursor object.
**
** The simple approach here would be to memset() the entire object
** to zero.  But it turns out that the apPage[] and aiIdx[] arrays
** do not need to be zeroed and they are large, so we can save a lot
** of run-time by skipping the initialization of those elements.
*/
خالی sqlite3BtreeCursorZero(BtCursor *p){
  memset(p, 0, offsetof(BtCursor, BTCURSOR_پہلا_UNINIT));
}

/*
** Close a cursor.  The read lock on the database file is released
** when the last cursor is closed.
*/
صحیح sqlite3BtreeCloseCursor(BtCursor *pCur){
  Btree *pBtree = pCur->pBtree;
  اگر( pBtree ){
    BtShared *pBt = pCur->pBt;
    sqlite3BtreeEnter(pBtree);
    أكد( pBt->pCursor!=0 );
    اگر( pBt->pCursor==pCur ){
      pBt->pCursor = pCur->p_اگلا;
    }ورنہ{
      BtCursor *p_پچھلا = pBt->pCursor;
      کرو{
        اگر( p_پچھلا->p_اگلا==pCur ){
          p_پچھلا->p_اگلا = pCur->p_اگلا;
          توڑو;
        }
        p_پچھلا = p_پچھلا->p_اگلا;
      }جب( ALWAYS(p_پچھلا) );
    }
    btree_Release_All_Cursor_صفحات(pCur);
    unlock_Btree_If_غیر_مستعمل(pBt);
    sqlite3_free(pCur->a_حد_سے_تجاوز);
    sqlite3_free(pCur->p_کلید);
    اگر( (pBt->openFlags & BTREE_SINGLE) && pBt->pCursor==0 ){
      /* Since the BtShared is not sharable, there is no need to
      ** worry about the missing sqlite3BtreeLeave() call here.  */
      أكد( pBtree->sharable==0 );
      sqlite3BtreeClose(pBtree);
    }ورنہ{
      sqlite3BtreeLeave(pBtree);
    }
    pCur->pBtree = 0;
  }
  واپس SQLITE_OK;
}

/*
** Make sure the BtCursor* given in the argument has a valid
** BtCursor.info structure.  If it is not already valid, call
** btreeParseCell() to fill it in.
**
** BtCursor.info is a cache of the information in the current cell.
** Using this cache reduces the number of calls to btreeParseCell().
*/
#اگر_غیر_تعریف_شدہ NDEBUG
  ساکن صحیح cellInfoEqual(CellInfo *a, CellInfo *b){
    اگر( a->n_کلید!=b->n_کلید ) واپس 0;
    اگر( a->pPayload!=b->pPayload ) واپس 0;
    اگر( a->nPayload!=b->nPayload ) واپس 0;
    اگر( a->nLocal!=b->nLocal ) واپس 0;
    اگر( a->n_سائز!=b->n_سائز ) واپس 0;
    واپس 1;
  }
  ساکن خالی assertCellInfo(BtCursor *pCur){
    CellInfo info;
    memset(&info, 0, حجم(info));
    btreeParseCell(pCur->p_صفحہ, pCur->ix, &info);
    أكد( CORRUPT_DB || cellInfoEqual(&info, &pCur->info) );
  }
#ورنہ
  #تعریف_کرو assertCellInfo(x)
#اگر_ختم
ساکن SQLITE_NOINLINE خالی getCellInfo(BtCursor *pCur){
  اگر( pCur->info.n_سائز==0 ){
    pCur->curFlags |= BTCF_ValidNKey;
    btreeParseCell(pCur->p_صفحہ,pCur->ix,&pCur->info);
  }ورنہ{
    assertCellInfo(pCur);
  }
}

#اگر_غیر_تعریف_شدہ NDEBUG  /* The next routine used only within assert() statements */
/*
** Return true if the given BtCursor is valid.  A valid cursor is one
** that is currently pointing to a row in a (non-empty) table.
** This is a verification routine is used only within assert() statements.
*/
صحیح sqlite3BtreeCursorIsValid(BtCursor *pCur){
  واپس pCur && pCur->eState==CURSOR_VALID;
}
#اگر_ختم /* NDEBUG */
صحیح sqlite3BtreeCursorIsValidNN(BtCursor *pCur){
  أكد( pCur!=0 );
  واپس pCur->eState==CURSOR_VALID;
}

/*
** Return the value of the integer key or "rowid" for a table btree.
** This routine is only valid for a cursor that is pointing into a
** ordinary table btree.  If the cursor points to an index btree or
** is invalid, the result of this routine is undefined.
*/
i64 sqlite3_Btree_Integer_کلید(BtCursor *pCur){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->cur_Int_کلید );
  getCellInfo(pCur);
  واپس pCur->info.n_کلید;
}

/*
** Pin or unpin a cursor.
*/
خالی sqlite3BtreeCursorPin(BtCursor *pCur){
  أكد( (pCur->curFlags & BTCF_Pinned)==0 );
  pCur->curFlags |= BTCF_Pinned;
}
خالی sqlite3BtreeCursorUnpin(BtCursor *pCur){
  أكد( (pCur->curFlags & BTCF_Pinned)!=0 );
  pCur->curFlags &= ~BTCF_Pinned;
}

/*
** Return the offset into the database file for the start of the
** payload to which the cursor is pointing.
*/
i64 sqlite3_Btree_انحراف(BtCursor *pCur){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  getCellInfo(pCur);
  واپس (i64)pCur->pBt->صفحہ_سائز*((i64)pCur->p_صفحہ->pgno - 1) +
         (i64)(pCur->info.pPayload - pCur->p_صفحہ->a_ڈیٹا);
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
u32 sqlite3_Btree_Payload_سائز(BtCursor *pCur){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  getCellInfo(pCur);
  واپس pCur->info.nPayload;
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
sqlite3_int64 sqlite3_Btree_Max_ریکارڈ_سائز(BtCursor *pCur){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  واپس pCur->pBt->صفحہ_سائز * (sqlite3_int64)pCur->pBt->n_صفحہ;
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
ساکن صحیح get_حد_سے_تجاوز_صفحہ(
  BtShared *pBt,               /* The database file */
  Pgno ovfl,                   /* Current overflow page number */
  Mem_صفحہ **pp_صفحہ,            /* OUT: MemPage handle (may be NULL) */
  Pgno *p_Pgno_اگلا              /* OUT: Next overflow page number */
){
  Pgno اگلا = 0;
  Mem_صفحہ *p_صفحہ = 0;
  صحیح rc = SQLITE_OK;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد(p_Pgno_اگلا);

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  /* Try to find the next page in the overflow list using the
  ** autovacuum pointer-map pages. Guess that the next page in
  ** the overflow list is page number (ovfl+1). If that guess turns
  ** out to be wrong, fall back to loading the data of page
  ** number ovfl to determine the next page number.
  */
  اگر( pBt->autoVacuum ){
    Pgno pgno;
    Pgno iGuess = ovfl+1;
    u8 eType;

    جب( PTRMAP_ISPAGE(pBt, iGuess) || iGuess==PENDING_BYTE_صفحہ(pBt) ){
      iGuess++;
    }

    اگر( iGuess<=btreePagecount(pBt) ){
      rc = ptrmapGet(pBt, iGuess, &eType, &pgno);
      اگر( rc==SQLITE_OK && eType==PTRMAP_OVERFLOW2 && pgno==ovfl ){
        اگلا = iGuess;
        rc = SQLITE_مکمل;
      }
    }
  }
#اگر_ختم

  أكد( اگلا==0 || rc==SQLITE_مکمل );
  اگر( rc==SQLITE_OK ){
    rc = btree_Get_صفحہ(pBt, ovfl, &p_صفحہ, (pp_صفحہ==0) ? PAGER_GET_READONLY : 0);
    أكد( rc==SQLITE_OK || p_صفحہ==0 );
    اگر( rc==SQLITE_OK ){
      اگلا = get4byte(p_صفحہ->a_ڈیٹا);
    }
  }

  *p_Pgno_اگلا = اگلا;
  اگر( pp_صفحہ ){
    *pp_صفحہ = p_صفحہ;
  }ورنہ{
    release_صفحہ(p_صفحہ);
  }
  واپس (rc==SQLITE_مکمل ? SQLITE_OK : rc);
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
ساکن صحیح copyPayload(
  خالی *pPayload,           /* Pointer to page data */
  خالی *pBuf,               /* Pointer to buffer */
  صحیح nByte,                /* Number of bytes to copy */
  صحیح eOp,                  /* 0 -> copy from page, 1 -> copy to page */
  Db_صفحہ *p_Db_صفحہ           /* Page containing pPayload */
){
  اگر( eOp ){
    /* Copy data from buffer to page (a write operation) */
    صحیح rc = sqlite3PagerWrite(p_Db_صفحہ);
    اگر( rc!=SQLITE_OK ){
      واپس rc;
    }
    حافظہ_نقل(pPayload, pBuf, nByte);
  }ورنہ{
    /* Copy data from page to buffer (a read operation) */
    حافظہ_نقل(pBuf, pPayload, nByte);
  }
  واپس SQLITE_OK;
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
ساکن صحیح accessPayload(
  BtCursor *pCur,      /* Cursor pointing to entry to read from */
  u32 انحراف,          /* Begin reading this far into payload */
  u32 amt,             /* Read this many bytes */
  غیرعلامتی حرف *pBuf, /* Write the bytes into this buffer */
  صحیح eOp              /* zero to read. non-zero to write. */
){
  غیرعلامتی حرف *aPayload;
  صحیح rc = SQLITE_OK;
  صحیح iIdx = 0;
  Mem_صفحہ *p_صفحہ = pCur->p_صفحہ;               /* Btree page of current entry */
  BtShared *pBt = pCur->pBt;                  /* Btree this cursor belongs to */
#اگر_تعریف_شدہ SQLITE_براہ_راست_حد_سے_تجاوز_READ
  غیرعلامتی حرف * ثابت p_Buf_آغاز = pBuf;     /* Start of original out buffer */
#اگر_ختم

  أكد( p_صفحہ );
  أكد( eOp==0 || eOp==1 );
  أكد( pCur->eState==CURSOR_VALID );
  اگر( pCur->ix>=p_صفحہ->nCell ){
    واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
  }
  أكد( cursorHoldsMutex(pCur) );

  getCellInfo(pCur);
  aPayload = pCur->info.pPayload;
  أكد( (u64)انحراف+(u64)amt <= (u64)pCur->info.nPayload );

  أكد( aPayload > p_صفحہ->a_ڈیٹا );
  اگر( (uptr)(aPayload - p_صفحہ->a_ڈیٹا) > (pBt->usable_سائز - pCur->info.nLocal) ){
    /* Trying to read or write past the end of the data is an error.  The
    ** conditional above is really:
    **    &aPayload[pCur->info.nLocal] > &pPage->aData[pBt->usableSize]
    ** but is recast into its current form to avoid integer overflow problems
    */
    واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
  }

  /* Check if data must be read/written to/from the btree page itself. */
  اگر( انحراف<pCur->info.nLocal ){
    صحیح a = amt;
    اگر( a+انحراف>pCur->info.nLocal ){
      a = pCur->info.nLocal - انحراف;
    }
    rc = copyPayload(&aPayload[انحراف], pBuf, a, eOp, p_صفحہ->p_Db_صفحہ);
    انحراف = 0;
    pBuf += a;
    amt -= a;
  }ورنہ{
    انحراف -= pCur->info.nLocal;
  }


  اگر( rc==SQLITE_OK && amt>0 ){
    ثابت u32 ovfl_سائز = pBt->usable_سائز - 4;  /* Bytes content per ovfl page */
    Pgno اگلا_صفحہ;

    اگلا_صفحہ = get4byte(&aPayload[pCur->info.nLocal]);

    /* If the BtCursor.aOverflow[] has not been allocated, allocate it now.
    **
    ** The aOverflow[] array is sized at one entry for each overflow page
    ** in the overflow chain. The page number of the first overflow page is
    ** stored in aOverflow[0], etc. A value of 0 in the aOverflow[] array
    ** means "not yet known" (the cache is lazily populated).
    */
    اگر( (pCur->curFlags & BTCF_ValidOvfl)==0 ){
      i64 nOvfl = pCur->info.nPayload;
      testcase( nOvfl - pCur->info.nLocal + ovfl_سائز - 1 > 0xffffffffU );
      nOvfl = (nOvfl - pCur->info.nLocal + ovfl_سائز-1)/ovfl_سائز;
      اگر( pCur->a_حد_سے_تجاوز==0
       || nOvfl*(صحیح)حجم(Pgno) > sqlite3_Malloc_سائز(pCur->a_حد_سے_تجاوز)
      ){
        Pgno *aNew;
        اگر( sqlite3FaultSim(413) ){
          aNew = 0;
        }ورنہ{
          aNew = (Pgno*)sqlite3Realloc(pCur->a_حد_سے_تجاوز, nOvfl*2*حجم(Pgno));
        }
        اگر( aNew==0 ){
          واپس SQLITE_NOMEM_BKPT;
        }ورنہ{
          pCur->a_حد_سے_تجاوز = aNew;
        }
      }
      memset(pCur->a_حد_سے_تجاوز, 0, nOvfl*حجم(Pgno));
      pCur->curFlags |= BTCF_ValidOvfl;
    }ورنہ{
      /* Sanity check the validity of the overflow page cache */
      أكد( pCur->a_حد_سے_تجاوز[0]==اگلا_صفحہ
           || pCur->a_حد_سے_تجاوز[0]==0
           || CORRUPT_DB );
      أكد( pCur->a_حد_سے_تجاوز[0]!=0 || pCur->a_حد_سے_تجاوز[انحراف/ovfl_سائز]==0 );

      /* If the overflow page-list cache has been allocated and the
      ** entry for the first required overflow page is valid, skip
      ** directly to it.
      */
      اگر( pCur->a_حد_سے_تجاوز[انحراف/ovfl_سائز] ){
        iIdx = (انحراف/ovfl_سائز);
        اگلا_صفحہ = pCur->a_حد_سے_تجاوز[iIdx];
        انحراف = (انحراف%ovfl_سائز);
      }
    }

    أكد( rc==SQLITE_OK && amt>0 );
    جب( اگلا_صفحہ ){
      /* If required, populate the overflow page-list cache. */
      اگر( اگلا_صفحہ > pBt->n_صفحہ ) واپس SQLITE_CORRUPT_BKPT;
      أكد( pCur->a_حد_سے_تجاوز[iIdx]==0
              || pCur->a_حد_سے_تجاوز[iIdx]==اگلا_صفحہ
              || CORRUPT_DB );
      pCur->a_حد_سے_تجاوز[iIdx] = اگلا_صفحہ;

      اگر( انحراف>=ovfl_سائز ){
        /* The only reason to read this page is to obtain the page
        ** number for the next page in the overflow chain. The page
        ** data is not required. So first try to lookup the overflow
        ** page-list cache, if any, then fall back to the getOverflowPage()
        ** function.
        */
        أكد( pCur->curFlags & BTCF_ValidOvfl );
        أكد( pCur->pBtree->db==pBt->db );
        اگر( pCur->a_حد_سے_تجاوز[iIdx+1] ){
          اگلا_صفحہ = pCur->a_حد_سے_تجاوز[iIdx+1];
        }ورنہ{
          rc = get_حد_سے_تجاوز_صفحہ(pBt, اگلا_صفحہ, 0, &اگلا_صفحہ);
        }
        انحراف -= ovfl_سائز;
      }ورنہ{
        /* Need to read this page properly. It contains some of the
        ** range of data that is being read (eOp==0) or written (eOp!=0).
        */
        صحیح a = amt;
        اگر( a + انحراف > ovfl_سائز ){
          a = ovfl_سائز - انحراف;
        }

#اگر_تعریف_شدہ SQLITE_براہ_راست_حد_سے_تجاوز_READ
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
        اگر( eOp==0                                             /* (1) */
         && انحراف==0                                          /* (2) */
         && sqlite3_Pager_براہ_راست_Read_Ok(pBt->pPager, اگلا_صفحہ)    /* (3,4,5) */
         && &pBuf[-4]>=p_Buf_آغاز                               /* (6) */
        ){
          sqlite3_file *fd = sqlite3PagerFile(pBt->pPager);
          u8 aSave[4];
          u8 *aWrite = &pBuf[-4];
          أكد( aWrite>=p_Buf_آغاز );                         /* due to (6) */
          حافظہ_نقل(aSave, aWrite, 4);
          rc = sqlite3OsRead(fd, aWrite, a+4, (i64)pBt->صفحہ_سائز*(اگلا_صفحہ-1));
          اگلا_صفحہ = get4byte(aWrite);
          حافظہ_نقل(aWrite, aSave, 4);
        }ورنہ
#اگر_ختم

        {
          Db_صفحہ *p_Db_صفحہ;
          rc = sqlite3PagerGet(pBt->pPager, اگلا_صفحہ, &p_Db_صفحہ,
              (eOp==0 ? PAGER_GET_READONLY : 0)
          );
          اگر( rc==SQLITE_OK ){
            اگر( eOp!=0
             && (sqlite3_Pager_صفحہ_Refcount(p_Db_صفحہ)!=1
                 || NEVER(((Mem_صفحہ*)sqlite3PagerGetExtra(p_Db_صفحہ))->isInit))
             && sqlite3FaultSim(411)==SQLITE_OK
            ){
              sqlite3PagerUnref(p_Db_صفحہ);
              واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
            }
            aPayload = sqlite3_Pager_Get_ڈیٹا(p_Db_صفحہ);
            اگلا_صفحہ = get4byte(aPayload);
            rc = copyPayload(&aPayload[انحراف+4], pBuf, a, eOp, p_Db_صفحہ);
            sqlite3PagerUnref(p_Db_صفحہ);
            انحراف = 0;
          }
        }
        amt -= a;
        اگر( amt==0 ) واپس rc;
        pBuf += a;
      }
      اگر( rc ) توڑو;
      iIdx++;
    }
  }

  اگر( rc==SQLITE_OK && amt>0 ){
    /* Overflow chain ends prematurely */
    واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
  }
  واپس rc;
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
صحیح sqlite3BtreePayload(BtCursor *pCur, u32 انحراف, u32 amt, خالی *pBuf){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->i_صفحہ>=0 && pCur->p_صفحہ );
  واپس accessPayload(pCur, انحراف, amt, (غیرعلامتی حرف*)pBuf, 0);
}

/*
** This variant of sqlite3BtreePayload() works even if the cursor has not
** in the CURSOR_VALID state.  It is only used by the sqlite3_blob_read()
** interface.
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
ساکن SQLITE_NOINLINE صحیح accessPayloadChecked(
  BtCursor *pCur,
  u32 انحراف,
  u32 amt,
  خالی *pBuf
){
  صحیح rc;
  اگر ( pCur->eState==CURSOR_INVALID ){
    واپس SQLITE_ABORT;
  }
  أكد( cursorOwnsBtShared(pCur) );
  rc = btreeRestoreCursorPosition(pCur);
  واپس rc ? rc : accessPayload(pCur, انحراف, amt, pBuf, 0);
}
صحیح sqlite3BtreePayloadChecked(BtCursor *pCur, u32 انحراف, u32 amt, خالی *pBuf){
  اگر( pCur->eState==CURSOR_VALID ){
    أكد( cursorOwnsBtShared(pCur) );
    واپس accessPayload(pCur, انحراف, amt, pBuf, 0);
  }ورنہ{
    واپس accessPayloadChecked(pCur, انحراف, amt, pBuf);
  }
}
#اگر_ختم /* SQLITE_OMIT_INCRBLOB */

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
ساکن ثابت خالی *fetchPayload(
  BtCursor *pCur,      /* Cursor pointing to entry to read from */
  u32 *pAmt            /* Write the number of available bytes here */
){
  صحیح amt;
  أكد( pCur!=0 && pCur->i_صفحہ>=0 && pCur->p_صفحہ);
  أكد( pCur->eState==CURSOR_VALID );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->ix<pCur->p_صفحہ->nCell || CORRUPT_DB );
  أكد( pCur->info.n_سائز>0 );
  أكد( pCur->info.pPayload>pCur->p_صفحہ->a_ڈیٹا || CORRUPT_DB );
  أكد( pCur->info.pPayload<pCur->p_صفحہ->a_ڈیٹا_انتہا ||CORRUPT_DB);
  amt = pCur->info.nLocal;
  اگر( amt>(صحیح)(pCur->p_صفحہ->a_ڈیٹا_انتہا - pCur->info.pPayload) ){
    /* There is too little space on the page for the expected amount
    ** of local content. Database must be corrupt. */
    أكد( CORRUPT_DB );
    amt = MAX(0, (صحیح)(pCur->p_صفحہ->a_ڈیٹا_انتہا - pCur->info.pPayload));
  }
  *pAmt = (u32)amt;
  واپس (خالی*)pCur->info.pPayload;
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
ثابت خالی *sqlite3BtreePayloadFetch(BtCursor *pCur, u32 *pAmt){
  واپس fetchPayload(pCur, pAmt);
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
ساکن صحیح move_مقصد_بچہ(BtCursor *pCur, u32 newPgno){
  صحیح rc;
  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->i_صفحہ<BTCURSOR_MAX_گہرائی );
  أكد( pCur->i_صفحہ>=0 );
  اگر( pCur->i_صفحہ>=(BTCURSOR_MAX_گہرائی-1) ){
    واپس SQLITE_CORRUPT_BKPT;
  }
  pCur->info.n_سائز = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  pCur->aiIdx[pCur->i_صفحہ] = pCur->ix;
  pCur->ap_صفحہ[pCur->i_صفحہ] = pCur->p_صفحہ;
  pCur->ix = 0;
  pCur->i_صفحہ++;
  rc = get_And_Init_صفحہ(pCur->pBt, newPgno, &pCur->p_صفحہ, pCur->curPagerFlags);
  أكد( pCur->p_صفحہ!=0 || rc!=SQLITE_OK );
  اگر( rc==SQLITE_OK
   && (pCur->p_صفحہ->nCell<1 || pCur->p_صفحہ->int_کلید!=pCur->cur_Int_کلید)
  ){
    release_صفحہ(pCur->p_صفحہ);
    rc = SQLITE_CORRUPT_PGNO(newPgno);
  }
  اگر( rc ){
    pCur->p_صفحہ = pCur->ap_صفحہ[--pCur->i_صفحہ];
  }
  واپس rc;
}

#اگر_تعریف_شدہ SQLITE_DEBUG
/*
** Page pParent is an internal (non-leaf) tree page. This function
** asserts that page number iChild is the left-child if the iIdx'th
** cell in page pParent. Or, if iIdx is equal to the total number of
** cells in pParent, that page number iChild is the right-child of
** the page.
*/
ساکن خالی assert_والد_اشاریہ(Mem_صفحہ *p_والد, صحیح iIdx, Pgno i_بچہ){
  اگر( CORRUPT_DB ) واپس;  /* The conditions tested below might not be true
                            ** in a corrupt database */
  أكد( iIdx<=p_والد->nCell );
  اگر( iIdx==p_والد->nCell ){
    أكد( get4byte(&p_والد->a_ڈیٹا[p_والد->hdr_انحراف+8])==i_بچہ );
  }ورنہ{
    أكد( get4byte(ڈھونڈو_Cell(p_والد, iIdx))==i_بچہ );
  }
}
#ورنہ
#  define assert_والد_اشاریہ(x,y,z)
#اگر_ختم

/*
** Move the cursor up to the parent page.
**
** pCur->idx is set to the cell index that contains the pointer
** to the page we are coming from.  If we are coming from the
** right-most child page then pCur->idx is set to one more than
** the largest cell index.
*/
ساکن خالی move_مقصد_والد(BtCursor *pCur){
  Mem_صفحہ *p_برگ;
  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->i_صفحہ>0 );
  أكد( pCur->p_صفحہ );
  assert_والد_اشاریہ(
    pCur->ap_صفحہ[pCur->i_صفحہ-1],
    pCur->aiIdx[pCur->i_صفحہ-1],
    pCur->p_صفحہ->pgno
  );
  testcase( pCur->aiIdx[pCur->i_صفحہ-1] > pCur->ap_صفحہ[pCur->i_صفحہ-1]->nCell );
  pCur->info.n_سائز = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  pCur->ix = pCur->aiIdx[pCur->i_صفحہ-1];
  p_برگ = pCur->p_صفحہ;
  pCur->p_صفحہ = pCur->ap_صفحہ[--pCur->i_صفحہ];
  release_صفحہ_Not_Null(p_برگ);
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
ساکن صحیح move_مقصد_جڑ(BtCursor *pCur){
  Mem_صفحہ *p_جڑ;
  صحیح rc = SQLITE_OK;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( CURSOR_INVALID < CURSOR_REQUIRESEEK );
  أكد( CURSOR_VALID   < CURSOR_REQUIRESEEK );
  أكد( CURSOR_FAULT   > CURSOR_REQUIRESEEK );
  أكد( pCur->eState < CURSOR_REQUIRESEEK || pCur->i_صفحہ<0 );
  أكد( pCur->pgno_جڑ>0 || pCur->i_صفحہ<0 );

  اگر( pCur->i_صفحہ>=0 ){
    اگر( pCur->i_صفحہ ){
      release_صفحہ_Not_Null(pCur->p_صفحہ);
      جب( --pCur->i_صفحہ ){
        release_صفحہ_Not_Null(pCur->ap_صفحہ[pCur->i_صفحہ]);
      }
      p_جڑ = pCur->p_صفحہ = pCur->ap_صفحہ[0];
      جاؤ skip_init;
    }
  }ورنہ اگر( pCur->pgno_جڑ==0 ){
    pCur->eState = CURSOR_INVALID;
    واپس SQLITE_خالی_خانے;
  }ورنہ{
    أكد( pCur->i_صفحہ==(-1) );
    اگر( pCur->eState>=CURSOR_REQUIRESEEK ){
      اگر( pCur->eState==CURSOR_FAULT ){
        أكد( pCur->skip_اگلا!=SQLITE_OK );
        واپس pCur->skip_اگلا;
      }
      sqlite3BtreeClearCursor(pCur);
    }
    rc = get_And_Init_صفحہ(pCur->pBt, pCur->pgno_جڑ, &pCur->p_صفحہ,
                        pCur->curPagerFlags);
    اگر( rc!=SQLITE_OK ){
      pCur->eState = CURSOR_INVALID;
      واپس rc;
    }
    pCur->i_صفحہ = 0;
    pCur->cur_Int_کلید = pCur->p_صفحہ->int_کلید;
  }
  p_جڑ = pCur->p_صفحہ;
  أكد( p_جڑ->pgno==pCur->pgno_جڑ || CORRUPT_DB );

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
  أكد( p_جڑ->int_کلید==1 || p_جڑ->int_کلید==0 );
  اگر( p_جڑ->isInit==0 || (pCur->p_کلید_Info==0)!=p_جڑ->int_کلید ){
    واپس SQLITE_CORRUPT_صفحہ(pCur->p_صفحہ);
  }

skip_init: 
  pCur->ix = 0;
  pCur->info.n_سائز = 0;
  pCur->curFlags &= ~(BTCF_At_آخری|BTCF_ValidNKey|BTCF_ValidOvfl);

  اگر( p_جڑ->nCell>0 ){
    pCur->eState = CURSOR_VALID;
  }ورنہ اگر( !p_جڑ->برگ ){
    Pgno subpage;
    اگر( p_جڑ->pgno!=1 ) واپس SQLITE_CORRUPT_BKPT;
    subpage = get4byte(&p_جڑ->a_ڈیٹا[p_جڑ->hdr_انحراف+8]);
    pCur->eState = CURSOR_VALID;
    rc = move_مقصد_بچہ(pCur, subpage);
  }ورنہ{
    pCur->eState = CURSOR_INVALID;
    rc = SQLITE_خالی_خانے;
  }
  واپس rc;
}

/*
** Move the cursor down to the left-most leaf entry beneath the
** entry to which it is currently pointing.
**
** The left-most leaf is the one with the smallest key - the first
** in ascending order.
*/
ساکن صحیح move_مقصد_Leftmost(BtCursor *pCur){
  Pgno pgno;
  صحیح rc = SQLITE_OK;
  Mem_صفحہ *p_صفحہ;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  جب( rc==SQLITE_OK && !(p_صفحہ = pCur->p_صفحہ)->برگ ){
    أكد( pCur->ix<p_صفحہ->nCell );
    pgno = get4byte(ڈھونڈو_Cell(p_صفحہ, pCur->ix));
    rc = move_مقصد_بچہ(pCur, pgno);
  }
  واپس rc;
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
ساکن صحیح move_مقصد_Rightmost(BtCursor *pCur){
  Pgno pgno;
  صحیح rc = SQLITE_OK;
  Mem_صفحہ *p_صفحہ = 0;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  جب( !(p_صفحہ = pCur->p_صفحہ)->برگ ){
    pgno = get4byte(&p_صفحہ->a_ڈیٹا[p_صفحہ->hdr_انحراف+8]);
    pCur->ix = p_صفحہ->nCell;
    rc = move_مقصد_بچہ(pCur, pgno);
    اگر( rc ) واپس rc;
  }
  pCur->ix = p_صفحہ->nCell-1;
  أكد( pCur->info.n_سائز==0 );
  أكد( (pCur->curFlags & BTCF_ValidNKey)==0 );
  واپس SQLITE_OK;
}

/* Move the cursor to the first entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
صحیح sqlite3_Btree_پہلا(BtCursor *pCur, صحیح *pRes){
  صحیح rc;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  rc = move_مقصد_جڑ(pCur);
  اگر( rc==SQLITE_OK ){
    أكد( pCur->p_صفحہ->nCell>0 );
    *pRes = 0;
    rc = move_مقصد_Leftmost(pCur);
  }ورنہ اگر( rc==SQLITE_خالی_خانے ){
    أكد( pCur->pgno_جڑ==0 || (pCur->p_صفحہ!=0 && pCur->p_صفحہ->nCell==0) );
    *pRes = 1;
    rc = SQLITE_OK;
  }
  واپس rc;
}

/* Set *pRes to 1 (true) if the BTree pointed to by cursor pCur contains zero
** rows of content.  Set *pRes to 0 (false) if the table contains content.
** Return SQLITE_OK on success or some error code (ex: SQLITE_NOMEM) if
** something goes wrong.
*/
صحیح sqlite3_Btree_Is_خالی_خانے(BtCursor *pCur, صحیح *pRes){
  صحیح rc;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  اگر( NEVER(pCur->eState==CURSOR_VALID) ){
    *pRes = 0;
    واپس SQLITE_OK;
  }
  rc = move_مقصد_جڑ(pCur);
  اگر( rc==SQLITE_خالی_خانے ){
    *pRes = 1;
    rc = SQLITE_OK;
  }ورنہ{
    *pRes = 0;
  }
  واپس rc;
}

#اگر_تعریف_شدہ SQLITE_DEBUG
/* The cursors is CURSOR_VALID and has BTCF_AtLast set.  Verify that
** this flags are true for a consistent database.
**
** This routine is is called from within assert() statements only.
** It is an internal verification routine and does not appear in production
** builds.
*/
ساکن صحیح cursor_Is_At_آخری_اندراج(BtCursor *pCur){
  صحیح ii;
  برائے(ii=0; ii<pCur->i_صفحہ; ii++){
    اگر( pCur->aiIdx[ii]!=pCur->ap_صفحہ[ii]->nCell ) واپس 0;
  }
  واپس pCur->ix==pCur->p_صفحہ->nCell-1 && pCur->p_صفحہ->برگ!=0;
}
#اگر_ختم

/* Move the cursor to the last entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
ساکن SQLITE_NOINLINE صحیح btree_آخری(BtCursor *pCur, صحیح *pRes){
  صحیح rc = move_مقصد_جڑ(pCur);
  اگر( rc==SQLITE_OK ){
    أكد( pCur->eState==CURSOR_VALID );
    *pRes = 0;
    rc = move_مقصد_Rightmost(pCur);
    اگر( rc==SQLITE_OK ){
      pCur->curFlags |= BTCF_At_آخری;
    }ورنہ{
      pCur->curFlags &= ~BTCF_At_آخری;
    }
  }ورنہ اگر( rc==SQLITE_خالی_خانے ){
    أكد( pCur->pgno_جڑ==0 || pCur->p_صفحہ->nCell==0 );
    *pRes = 1;
    rc = SQLITE_OK;
  }
  واپس rc;
}
صحیح sqlite3_Btree_آخری(BtCursor *pCur, صحیح *pRes){
  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* If the cursor already points to the last entry, this is a no-op. */
  اگر( CURSOR_VALID==pCur->eState && (pCur->curFlags & BTCF_At_آخری)!=0 ){
    أكد( cursor_Is_At_آخری_اندراج(pCur) || CORRUPT_DB );
    *pRes = 0;
    واپس SQLITE_OK;
  }
  واپس btree_آخری(pCur, pRes);
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
صحیح sqlite3_Btree_جدول_Moveto(
  BtCursor *pCur,          /* The cursor to be moved */
  i64 int_کلید,              /* The table key */
  صحیح bias_دایاں,           /* If true, bias the search to the high end */
  صحیح *pRes                /* Write search results here */
){
  صحیح rc;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  أكد( pRes );
  أكد( pCur->p_کلید_Info==0 );
  أكد( pCur->eState!=CURSOR_VALID || pCur->cur_Int_کلید!=0 );

  /* If the cursor is already positioned at the point we are trying
  ** to move to, then just return without doing any work */
  اگر( pCur->eState==CURSOR_VALID && (pCur->curFlags & BTCF_ValidNKey)!=0 ){
    اگر( pCur->info.n_کلید==int_کلید ){
      *pRes = 0;
      واپس SQLITE_OK;
    }
    اگر( pCur->info.n_کلید<int_کلید ){
      اگر( (pCur->curFlags & BTCF_At_آخری)!=0 ){
        أكد( cursor_Is_At_آخری_اندراج(pCur) || CORRUPT_DB );
        *pRes = -1;
        واپس SQLITE_OK;
      }
      /* If the requested key is one more than the previous key, then
      ** try to get there using sqlite3BtreeNext() rather than a full
      ** binary search.  This is an optimization only.  The correct answer
      ** is still obtained without this case, only a little more slowly. */
      اگر( pCur->info.n_کلید+1==int_کلید ){
        *pRes = 0;
        rc = sqlite3_Btree_اگلا(pCur, 0);
        اگر( rc==SQLITE_OK ){
          getCellInfo(pCur);
          اگر( pCur->info.n_کلید==int_کلید ){
            واپس SQLITE_OK;
          }
        }ورنہ اگر( rc!=SQLITE_مکمل ){
          واپس rc;
        }
      }
    }
  }

#اگر_تعریف_شدہ SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#اگر_ختم

  rc = move_مقصد_جڑ(pCur);
  اگر( rc ){
    اگر( rc==SQLITE_خالی_خانے ){
      أكد( pCur->pgno_جڑ==0 || pCur->p_صفحہ->nCell==0 );
      *pRes = -1;
      واپس SQLITE_OK;
    }
    واپس rc;
  }
  أكد( pCur->p_صفحہ );
  أكد( pCur->p_صفحہ->isInit );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->p_صفحہ->nCell > 0 );
  أكد( pCur->i_صفحہ==0 || pCur->ap_صفحہ[0]->int_کلید==pCur->cur_Int_کلید );
  أكد( pCur->cur_Int_کلید );

  برائے(;;){
    صحیح lwr, upr, idx, c;
    Pgno chldPg;
    Mem_صفحہ *p_صفحہ = pCur->p_صفحہ;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    أكد( p_صفحہ->nCell>0 );
    أكد( p_صفحہ->int_کلید );
    lwr = 0;
    upr = p_صفحہ->nCell-1;
    أكد( bias_دایاں==0 || bias_دایاں==1 );
    idx = upr>>(1-bias_دایاں); /* idx = biasRight ? upr : (lwr+upr)/2; */
    برائے(;;){
      i64 n_Cell_کلید;
      pCell = ڈھونڈو_Cell_Past_Ptr(p_صفحہ, idx);
      اگر( p_صفحہ->int_کلید_برگ ){
        جب( 0x80 <= *(pCell++) ){
          اگر( pCell>=p_صفحہ->a_ڈیٹا_انتہا ){
            واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
          }
        }
      }
      n_Cell_کلید = sqlite3_Varint_قدر(pCell);
      اگر( n_Cell_کلید<int_کلید ){
        lwr = idx+1;
        اگر( lwr>upr ){ c = -1; توڑو; }
      }ورنہ اگر( n_Cell_کلید>int_کلید ){
        upr = idx-1;
        اگر( lwr>upr ){ c = +1; توڑو; }
      }ورنہ{
        أكد( n_Cell_کلید==int_کلید );
        pCur->ix = (u16)idx;
        اگر( !p_صفحہ->برگ ){
          lwr = idx;
          جاؤ moveto_جدول_اگلا_layer;
        }ورنہ{
          pCur->curFlags |= BTCF_ValidNKey;
          pCur->info.n_کلید = n_Cell_کلید;
          pCur->info.n_سائز = 0;
          *pRes = 0;
          واپس SQLITE_OK;
        }
      }
      أكد( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2; */
    }
    أكد( lwr==upr+1 || !p_صفحہ->برگ );
    أكد( p_صفحہ->isInit );
    اگر( p_صفحہ->برگ ){
      أكد( pCur->ix<pCur->p_صفحہ->nCell );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      جاؤ moveto_جدول_اختتام;
    }
moveto_جدول_اگلا_layer:
    اگر( lwr>=p_صفحہ->nCell ){
      chldPg = get4byte(&p_صفحہ->a_ڈیٹا[p_صفحہ->hdr_انحراف+8]);
    }ورنہ{
      chldPg = get4byte(ڈھونڈو_Cell(p_صفحہ, lwr));
    }
    pCur->ix = (u16)lwr;
    rc = move_مقصد_بچہ(pCur, chldPg);
    اگر( rc ) توڑو;
  }
moveto_جدول_اختتام:
  pCur->info.n_سائز = 0;
  أكد( (pCur->curFlags & BTCF_ValidOvfl)==0 );
  واپس rc;
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
ساکن صحیح اشاریہ_Cell_Compare(
  Mem_صفحہ *p_صفحہ,
  صحیح idx,
  Unpacked_ریکارڈ *p_Idx_کلید,
  ریکارڈ_Compare x_ریکارڈ_Compare
){
  صحیح c;
  صحیح nCell;  /* Size of the pCell cell in bytes */
  u8 *pCell = ڈھونڈو_Cell_Past_Ptr(p_صفحہ, idx);

  nCell = pCell[0];
  اگر( nCell<=p_صفحہ->max1bytePayload ){
    /* This branch runs if the record-size field of the cell is a
    ** single byte varint and the record fits entirely on the main
    ** b-tree page.  */
    اگر( pCell + nCell >= p_صفحہ->a_ڈیٹا_انتہا ) واپس 99;
    c = x_ریکارڈ_Compare(nCell, (خالی*)&pCell[1], p_Idx_کلید);
  }ورنہ اگر( !(pCell[1] & 0x80)
    && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=p_صفحہ->maxLocal
  ){
    /* The record-size field is a 2 byte varint and the record
    ** fits entirely on the main b-tree page.  */
    اگر( pCell + nCell >= p_صفحہ->a_ڈیٹا_انتہا ) واپس 99;
    c = x_ریکارڈ_Compare(nCell, (خالی*)&pCell[2], p_Idx_کلید);
  }ورنہ{
    /* If the record extends into overflow pages, do not attempt
    ** the optimization. */
    c = 99;
  }
  واپس c;
}

/*
** Return true (non-zero) if pCur is current pointing to the last
** page of a table.
*/
ساکن صحیح cursor_On_آخری_صفحہ(BtCursor *pCur){
  صحیح i;
  أكد( pCur->eState==CURSOR_VALID );
  برائے(i=0; i<pCur->i_صفحہ; i++){
    Mem_صفحہ *p_صفحہ = pCur->ap_صفحہ[i];
    اگر( pCur->aiIdx[i]<p_صفحہ->nCell ) واپس 0;
  }
  واپس 1;
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
صحیح sqlite3_Btree_اشاریہ_Moveto(
  BtCursor *pCur,          /* The cursor to be moved */
  Unpacked_ریکارڈ *p_Idx_کلید, /* Unpacked index key */
  صحیح *pRes                /* Write search results here */
){
  صحیح rc;
  ریکارڈ_Compare x_ریکارڈ_Compare;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  أكد( pRes );
  أكد( pCur->p_کلید_Info!=0 );

#اگر_تعریف_شدہ SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#اگر_ختم

  x_ریکارڈ_Compare = sqlite3_Vdbe_ڈھونڈو_Compare(p_Idx_کلید);
  p_Idx_کلید->err_رمز = 0;
  أكد( p_Idx_کلید->default_rc==1
       || p_Idx_کلید->default_rc==0
       || p_Idx_کلید->default_rc==-1
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
  اگر( pCur->eState==CURSOR_VALID
   && pCur->p_صفحہ->برگ
   && cursor_On_آخری_صفحہ(pCur)
  ){
    صحیح c;
    اگر( pCur->ix==pCur->p_صفحہ->nCell-1
     && (c = اشاریہ_Cell_Compare(pCur->p_صفحہ,pCur->ix,p_Idx_کلید,x_ریکارڈ_Compare))<=0
     && p_Idx_کلید->err_رمز==SQLITE_OK
    ){
      *pRes = c;
      واپس SQLITE_OK;  /* Cursor already pointing at the correct spot */
    }
    اگر( pCur->i_صفحہ>0
     && اشاریہ_Cell_Compare(pCur->p_صفحہ, 0, p_Idx_کلید, x_ریکارڈ_Compare)<=0
     && p_Idx_کلید->err_رمز==SQLITE_OK
    ){
      pCur->curFlags &= ~(BTCF_ValidOvfl|BTCF_At_آخری);
      اگر( !pCur->p_صفحہ->isInit ){
        واپس SQLITE_CORRUPT_BKPT;
      }
      جاؤ bypass_moveto_جڑ;  /* Start search on the current page */
    }
    p_Idx_کلید->err_رمز = SQLITE_OK;
  }

  rc = move_مقصد_جڑ(pCur);
  اگر( rc ){
    اگر( rc==SQLITE_خالی_خانے ){
      أكد( pCur->pgno_جڑ==0 || pCur->p_صفحہ->nCell==0 );
      *pRes = -1;
      واپس SQLITE_OK;
    }
    واپس rc;
  }

bypass_moveto_جڑ:
  أكد( pCur->p_صفحہ );
  أكد( pCur->p_صفحہ->isInit );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->p_صفحہ->nCell > 0 );
  أكد( pCur->cur_Int_کلید==0 );
  أكد( p_Idx_کلید!=0 );
  برائے(;;){
    صحیح lwr, upr, idx, c;
    Pgno chldPg;
    Mem_صفحہ *p_صفحہ = pCur->p_صفحہ;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    أكد( p_صفحہ->nCell>0 );
    أكد( p_صفحہ->int_کلید==0 );
    lwr = 0;
    upr = p_صفحہ->nCell-1;
    idx = upr>>1; /* idx = (lwr+upr)/2; */
    برائے(;;){
      صحیح nCell;  /* Size of the pCell cell in bytes */
      pCell = ڈھونڈو_Cell_Past_Ptr(p_صفحہ, idx);

      /* The maximum supported page-size is 65536 bytes. This means that
      ** the maximum number of record bytes stored on an index B-Tree
      ** page is less than 16384 bytes and may be stored as a 2-byte
      ** varint. This information is used to attempt to avoid parsing
      ** the entire cell by checking for the cases where the record is
      ** stored entirely within the b-tree page by inspecting the first
      ** 2 bytes of the cell.
      */
      nCell = pCell[0];
      اگر( nCell<=p_صفحہ->max1bytePayload ){
        /* This branch runs if the record-size field of the cell is a
        ** single byte varint and the record fits entirely on the main
        ** b-tree page.  */
        اگر( pCell + nCell >= p_صفحہ->a_ڈیٹا_انتہا ){
          rc = SQLITE_CORRUPT_صفحہ(p_صفحہ);
          جاؤ moveto_اشاریہ_اختتام;
        }
        c = x_ریکارڈ_Compare(nCell, (خالی*)&pCell[1], p_Idx_کلید);
      }ورنہ اگر( !(pCell[1] & 0x80)
        && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=p_صفحہ->maxLocal
        && pCell + nCell < p_صفحہ->a_ڈیٹا_انتہا
      ){
        /* The record-size field is a 2 byte varint and the record
        ** fits entirely on the main b-tree page.  */
        c = x_ریکارڈ_Compare(nCell, (خالی*)&pCell[2], p_Idx_کلید);
      }ورنہ{
        /* The record flows over onto one or more overflow pages. In
        ** this case the whole cell needs to be parsed, a buffer allocated
        ** and accessPayload() used to retrieve the record into the
        ** buffer before VdbeRecordCompare() can be called.
        **
        ** If the record is corrupt, the xRecordCompare routine may read
        ** up to two varints past the end of the buffer. An extra 18
        ** bytes of padding is allocated at the end of the buffer in
        ** case this happens.  */
        خالی *p_Cell_کلید;
        u8 * ثابت pCellBody = pCell - p_صفحہ->بچہ_Ptr_سائز;
        ثابت صحیح nOverrun = 18;  /* Size of the overrun padding */
        p_صفحہ->xParseCell(p_صفحہ, pCellBody, &pCur->info);
        nCell = (صحیح)pCur->info.n_کلید;
        testcase( nCell<0 );   /* True if key size is 2^32 or more */
        testcase( nCell==0 );  /* Invalid key size:  0x80 0x80 0x00 */
        testcase( nCell==1 );  /* Invalid key size:  0x80 0x80 0x01 */
        testcase( nCell==2 );  /* Minimum legal index key size */
        اگر( nCell<2 || nCell/pCur->pBt->usable_سائز>pCur->pBt->n_صفحہ ){
          rc = SQLITE_CORRUPT_صفحہ(p_صفحہ);
          جاؤ moveto_اشاریہ_اختتام;
        }
        p_Cell_کلید = sqlite3Malloc( (u64)nCell+(u64)nOverrun );
        اگر( p_Cell_کلید==0 ){
          rc = SQLITE_NOMEM_BKPT;
          جاؤ moveto_اشاریہ_اختتام;
        }
        pCur->ix = (u16)idx;
        rc = accessPayload(pCur, 0, nCell, (غیرعلامتی حرف*)p_Cell_کلید, 0);
        memset(((u8*)p_Cell_کلید)+nCell,0,nOverrun); /* Fix uninit warnings */
        pCur->curFlags &= ~BTCF_ValidOvfl;
        اگر( rc ){
          sqlite3_free(p_Cell_کلید);
          جاؤ moveto_اشاریہ_اختتام;
        }
        c = sqlite3_Vdbe_ریکارڈ_Compare(nCell, p_Cell_کلید, p_Idx_کلید);
        sqlite3_free(p_Cell_کلید);
      }
      أكد(
          (p_Idx_کلید->err_رمز!=SQLITE_CORRUPT || c==0)
       && (p_Idx_کلید->err_رمز!=SQLITE_NOMEM || pCur->pBtree->db->mallocFailed)
      );
      اگر( c<0 ){
        lwr = idx+1;
      }ورنہ اگر( c>0 ){
        upr = idx-1;
      }ورنہ{
        أكد( c==0 );
        *pRes = 0;
        rc = SQLITE_OK;
        pCur->ix = (u16)idx;
        اگر( p_Idx_کلید->err_رمز ) rc = SQLITE_CORRUPT_BKPT;
        جاؤ moveto_اشاریہ_اختتام;
      }
      اگر( lwr>upr ) توڑو;
      أكد( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2 */
    }
    أكد( lwr==upr+1 || (p_صفحہ->int_کلید && !p_صفحہ->برگ) );
    أكد( p_صفحہ->isInit );
    اگر( p_صفحہ->برگ ){
      أكد( pCur->ix<pCur->p_صفحہ->nCell || CORRUPT_DB );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      جاؤ moveto_اشاریہ_اختتام;
    }
    اگر( lwr>=p_صفحہ->nCell ){
      chldPg = get4byte(&p_صفحہ->a_ڈیٹا[p_صفحہ->hdr_انحراف+8]);
    }ورنہ{
      chldPg = get4byte(ڈھونڈو_Cell(p_صفحہ, lwr));
    }

    /* This block is similar to an in-lined version of:
    **
    **    pCur->ix = (u16)lwr;
    **    rc = moveToChild(pCur, chldPg);
    **    if( rc ) break;
    */
    pCur->info.n_سائز = 0;
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
    اگر( pCur->i_صفحہ>=(BTCURSOR_MAX_گہرائی-1) ){
      واپس SQLITE_CORRUPT_BKPT;
    }
    pCur->aiIdx[pCur->i_صفحہ] = (u16)lwr;
    pCur->ap_صفحہ[pCur->i_صفحہ] = pCur->p_صفحہ;
    pCur->ix = 0;
    pCur->i_صفحہ++;
    rc = get_And_Init_صفحہ(pCur->pBt, chldPg, &pCur->p_صفحہ, pCur->curPagerFlags);
    اگر( rc==SQLITE_OK
     && (pCur->p_صفحہ->nCell<1 || pCur->p_صفحہ->int_کلید!=pCur->cur_Int_کلید)
    ){
      release_صفحہ(pCur->p_صفحہ);
      rc = SQLITE_CORRUPT_PGNO(chldPg);
    }
    اگر( rc ){
      pCur->p_صفحہ = pCur->ap_صفحہ[--pCur->i_صفحہ];
      توڑو;
    }
    /*
    ***** End of in-lined moveToChild() call */
 }
moveto_اشاریہ_اختتام:
  pCur->info.n_سائز = 0;
  أكد( (pCur->curFlags & BTCF_ValidOvfl)==0 );
  واپس rc;
}


/*
** Return TRUE if the cursor is not pointing at an entry of the table.
**
** TRUE will be returned after a call to sqlite3BtreeNext() moves
** past the last entry in the table or sqlite3BtreePrev() moves past
** the first entry.  TRUE is also returned if the table is empty.
*/
صحیح sqlite3BtreeEof(BtCursor *pCur){
  /* TODO: What if the cursor is in CURSOR_REQUIRESEEK but all table entries
  ** have been deleted? This API will need to change to return an error code
  ** as well as the boolean result value.
  */
  واپس (CURSOR_VALID!=pCur->eState);
}

/*
** Return an estimate for the number of rows in the table that pCur is
** pointing to.  Return a negative number if no estimate is currently
** available.
*/
i64 sqlite3_Btree_صف_گنتی_Est(BtCursor *pCur){
  i64 n;
  u8 i;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* Currently this interface is only called by the OP_IfSizeBetween
  ** opcode and the OP_Count opcode with P3=1.  In either case,
  ** the cursor will always be valid unless the btree is empty. */
  اگر( pCur->eState!=CURSOR_VALID ) واپس 0;
  اگر( NEVER(pCur->p_صفحہ->برگ==0) ) واپس -1;

  n = pCur->p_صفحہ->nCell;
  برائے(i=0; i<pCur->i_صفحہ; i++){
    n *= pCur->ap_صفحہ[i]->nCell+1;
  }
  واپس n;
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
ساکن SQLITE_NOINLINE صحیح btree_اگلا(BtCursor *pCur){
  صحیح rc;
  صحیح idx;
  Mem_صفحہ *p_صفحہ;

  أكد( cursorOwnsBtShared(pCur) );
  اگر( pCur->eState!=CURSOR_VALID ){
    أكد( (pCur->curFlags & BTCF_ValidOvfl)==0 );
    rc = restoreCursorPosition(pCur);
    اگر( rc!=SQLITE_OK ){
      واپس rc;
    }
    اگر( CURSOR_INVALID==pCur->eState ){
      واپس SQLITE_مکمل;
    }
    اگر( pCur->eState==CURSOR_SKIPNEXT ){
      pCur->eState = CURSOR_VALID;
      اگر( pCur->skip_اگلا>0 ) واپس SQLITE_OK;
    }
  }

  p_صفحہ = pCur->p_صفحہ;
  idx = ++pCur->ix;
  اگر( sqlite3FaultSim(412) ) p_صفحہ->isInit = 0;
  اگر( !p_صفحہ->isInit ){
    واپس SQLITE_CORRUPT_BKPT;
  }

  اگر( idx>=p_صفحہ->nCell ){
    اگر( !p_صفحہ->برگ ){
      rc = move_مقصد_بچہ(pCur, get4byte(&p_صفحہ->a_ڈیٹا[p_صفحہ->hdr_انحراف+8]));
      اگر( rc ) واپس rc;
      واپس move_مقصد_Leftmost(pCur);
    }
    کرو{
      اگر( pCur->i_صفحہ==0 ){
        pCur->eState = CURSOR_INVALID;
        واپس SQLITE_مکمل;
      }
      move_مقصد_والد(pCur);
      p_صفحہ = pCur->p_صفحہ;
    }جب( pCur->ix>=p_صفحہ->nCell );
    اگر( p_صفحہ->int_کلید ){
      واپس sqlite3_Btree_اگلا(pCur, 0);
    }ورنہ{
      واپس SQLITE_OK;
    }
  }
  اگر( p_صفحہ->برگ ){
    واپس SQLITE_OK;
  }ورنہ{
    واپس move_مقصد_Leftmost(pCur);
  }
}
صحیح sqlite3_Btree_اگلا(BtCursor *pCur, صحیح flags){
  Mem_صفحہ *p_صفحہ;
  غیر_مستعمل_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  أكد( cursorOwnsBtShared(pCur) );
  أكد( flags==0 || flags==1 );
  pCur->info.n_سائز = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  اگر( pCur->eState!=CURSOR_VALID ) واپس btree_اگلا(pCur);
  p_صفحہ = pCur->p_صفحہ;
  اگر( (++pCur->ix)>=p_صفحہ->nCell ){
    pCur->ix--;
    واپس btree_اگلا(pCur);
  }
  اگر( p_صفحہ->برگ ){
    واپس SQLITE_OK;
  }ورنہ{
    واپس move_مقصد_Leftmost(pCur);
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
ساکن SQLITE_NOINLINE صحیح btree_پچھلا(BtCursor *pCur){
  صحیح rc;
  Mem_صفحہ *p_صفحہ;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( (pCur->curFlags & (BTCF_At_آخری|BTCF_ValidOvfl|BTCF_ValidNKey))==0 );
  أكد( pCur->info.n_سائز==0 );
  اگر( pCur->eState!=CURSOR_VALID ){
    rc = restoreCursorPosition(pCur);
    اگر( rc!=SQLITE_OK ){
      واپس rc;
    }
    اگر( CURSOR_INVALID==pCur->eState ){
      واپس SQLITE_مکمل;
    }
    اگر( CURSOR_SKIPNEXT==pCur->eState ){
      pCur->eState = CURSOR_VALID;
      اگر( pCur->skip_اگلا<0 ) واپس SQLITE_OK;
    }
  }

  p_صفحہ = pCur->p_صفحہ;
  اگر( sqlite3FaultSim(412) ) p_صفحہ->isInit = 0;
  اگر( !p_صفحہ->isInit ){
    واپس SQLITE_CORRUPT_BKPT;
  }
  اگر( !p_صفحہ->برگ ){
    صحیح idx = pCur->ix;
    rc = move_مقصد_بچہ(pCur, get4byte(ڈھونڈو_Cell(p_صفحہ, idx)));
    اگر( rc ) واپس rc;
    rc = move_مقصد_Rightmost(pCur);
  }ورنہ{
    جب( pCur->ix==0 ){
      اگر( pCur->i_صفحہ==0 ){
        pCur->eState = CURSOR_INVALID;
        واپس SQLITE_مکمل;
      }
      move_مقصد_والد(pCur);
    }
    أكد( pCur->info.n_سائز==0 );
    أكد( (pCur->curFlags & (BTCF_ValidOvfl))==0 );

    pCur->ix--;
    p_صفحہ = pCur->p_صفحہ;
    اگر( p_صفحہ->int_کلید && !p_صفحہ->برگ ){
      rc = sqlite3_Btree_پچھلا(pCur, 0);
    }ورنہ{
      rc = SQLITE_OK;
    }
  }
  واپس rc;
}
صحیح sqlite3_Btree_پچھلا(BtCursor *pCur, صحیح flags){
  أكد( cursorOwnsBtShared(pCur) );
  أكد( flags==0 || flags==1 );
  غیر_مستعمل_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  pCur->curFlags &= ~(BTCF_At_آخری|BTCF_ValidOvfl|BTCF_ValidNKey);
  pCur->info.n_سائز = 0;
  اگر( pCur->eState!=CURSOR_VALID
   || pCur->ix==0
   || pCur->p_صفحہ->برگ==0
  ){
    واپس btree_پچھلا(pCur);
  }
  pCur->ix--;
  واپس SQLITE_OK;
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
ساکن صحیح allocate_Btree_صفحہ(
  BtShared *pBt,         /* The btree */
  Mem_صفحہ **pp_صفحہ,      /* Store pointer to the allocated page here */
  Pgno *pPgno,           /* Store the page number here */
  Pgno nearby,           /* Search for a page near this one */
  u8 e_منوال               /* BTALLOC_EXACT, BTALLOC_LT, or BTALLOC_ANY */
){
  Mem_صفحہ *pPage1;
  صحیح rc;
  u32 n;     /* Number of pages on the freelist */
  u32 k;     /* Number of leaves on the trunk of the freelist */
  Mem_صفحہ *pTrunk = 0;
  Mem_صفحہ *p_پچھلا_Trunk = 0;
  Pgno mx_صفحہ;     /* Total size of the database file */

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( e_منوال==BTALLOC_ANY || (nearby>0 && IfNotOmitAV(pBt->autoVacuum)) );
  pPage1 = pBt->pPage1;
  mx_صفحہ = btreePagecount(pBt);
  /* EVIDENCE-OF: R-21003-45125 The 4-byte big-endian integer at offset 36
  ** stores the total number of pages on the freelist. */
  n = get4byte(&pPage1->a_ڈیٹا[36]);
  testcase( n==mx_صفحہ-1 );
  اگر( n>=mx_صفحہ ){
    واپس SQLITE_CORRUPT_BKPT;
  }
  اگر( n>0 ){
    /* There are pages on the freelist.  Reuse one of those pages. */
    Pgno iTrunk;
    u8 تلاش_List = 0; /* If the free-list must be searched for 'nearby' */
    u32 n_تلاش = 0;   /* Count of the number of search attempts */
   
    /* If eMode==BTALLOC_EXACT and a query of the pointer-map
    ** shows that the page 'nearby' is somewhere on the free-list, then
    ** the entire-list will be searched for that page.
    */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
    اگر( e_منوال==BTALLOC_EXACT ){
      اگر( nearby<=mx_صفحہ ){
        u8 eType;
        أكد( nearby>0 );
        أكد( pBt->autoVacuum );
        rc = ptrmapGet(pBt, nearby, &eType, 0);
        اگر( rc ) واپس rc;
        اگر( eType==PTRMAP_FREEPAGE ){
          تلاش_List = 1;
        }
      }
    }ورنہ اگر( e_منوال==BTALLOC_LE ){
      تلاش_List = 1;
    }
#اگر_ختم

    /* Decrement the free-list count by 1. Set iTrunk to the index of the
    ** first free-list trunk page. iPrevTrunk is initially 1.
    */
    rc = sqlite3PagerWrite(pPage1->p_Db_صفحہ);
    اگر( rc ) واپس rc;
    put4byte(&pPage1->a_ڈیٹا[36], n-1);

    /* The code within this loop is run only once if the 'searchList' variable
    ** is not true. Otherwise, it runs once for each trunk-page on the
    ** free-list until the page 'nearby' is located (eMode==BTALLOC_EXACT)
    ** or until a page less than 'nearby' is located (eMode==BTALLOC_LT)
    */
    کرو {
      p_پچھلا_Trunk = pTrunk;
      اگر( p_پچھلا_Trunk ){
        /* EVIDENCE-OF: R-01506-11053 The first integer on a freelist trunk page
        ** is the page number of the next freelist trunk page in the list or
        ** zero if this is the last freelist trunk page. */
        iTrunk = get4byte(&p_پچھلا_Trunk->a_ڈیٹا[0]);
      }ورنہ{
        /* EVIDENCE-OF: R-59841-13798 The 4-byte big-endian integer at offset 32
        ** stores the page number of the first page of the freelist, or zero if
        ** the freelist is empty. */
        iTrunk = get4byte(&pPage1->a_ڈیٹا[32]);
      }
      testcase( iTrunk==mx_صفحہ );
      اگر( iTrunk>mx_صفحہ || n_تلاش++ > n ){
        rc = SQLITE_CORRUPT_PGNO(p_پچھلا_Trunk ? p_پچھلا_Trunk->pgno : 1);
      }ورنہ{
        rc = btree_Get_غیر_مستعمل_صفحہ(pBt, iTrunk, &pTrunk, 0);
      }
      اگر( rc ){
        pTrunk = 0;
        جاؤ انتہا_allocate_صفحہ;
      }
      أكد( pTrunk!=0 );
      أكد( pTrunk->a_ڈیٹا!=0 );
      /* EVIDENCE-OF: R-13523-04394 The second integer on a freelist trunk page
      ** is the number of leaf page pointers to follow. */
      k = get4byte(&pTrunk->a_ڈیٹا[4]);
      اگر( k==0 && !تلاش_List ){
        /* The trunk has no leaves and the list is not being searched.
        ** So extract the trunk page itself and use it as the newly
        ** allocated page */
        أكد( p_پچھلا_Trunk==0 );
        rc = sqlite3PagerWrite(pTrunk->p_Db_صفحہ);
        اگر( rc ){
          جاؤ انتہا_allocate_صفحہ;
        }
        *pPgno = iTrunk;
        حافظہ_نقل(&pPage1->a_ڈیٹا[32], &pTrunk->a_ڈیٹا[0], 4);
        *pp_صفحہ = pTrunk;
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
      }ورنہ اگر( k>(u32)(pBt->usable_سائز/4 - 2) ){
        /* Value of k is out of range.  Database corruption */
        rc = SQLITE_CORRUPT_PGNO(iTrunk);
        جاؤ انتہا_allocate_صفحہ;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
      }ورنہ اگر( تلاش_List
            && (nearby==iTrunk || (iTrunk<nearby && e_منوال==BTALLOC_LE))
      ){
        /* The list is being searched and this trunk page is the page
        ** to allocate, regardless of whether it has leaves.
        */
        *pPgno = iTrunk;
        *pp_صفحہ = pTrunk;
        تلاش_List = 0;
        rc = sqlite3PagerWrite(pTrunk->p_Db_صفحہ);
        اگر( rc ){
          جاؤ انتہا_allocate_صفحہ;
        }
        اگر( k==0 ){
          اگر( !p_پچھلا_Trunk ){
            حافظہ_نقل(&pPage1->a_ڈیٹا[32], &pTrunk->a_ڈیٹا[0], 4);
          }ورنہ{
            rc = sqlite3PagerWrite(p_پچھلا_Trunk->p_Db_صفحہ);
            اگر( rc!=SQLITE_OK ){
              جاؤ انتہا_allocate_صفحہ;
            }
            حافظہ_نقل(&p_پچھلا_Trunk->a_ڈیٹا[0], &pTrunk->a_ڈیٹا[0], 4);
          }
        }ورنہ{
          /* The trunk page is required by the caller but it contains
          ** pointers to free-list leaves. The first leaf becomes a trunk
          ** page in this case.
          */
          Mem_صفحہ *pNewTrunk;
          Pgno iNewTrunk = get4byte(&pTrunk->a_ڈیٹا[8]);
          اگر( iNewTrunk>mx_صفحہ ){
            rc = SQLITE_CORRUPT_PGNO(iTrunk);
            جاؤ انتہا_allocate_صفحہ;
          }
          testcase( iNewTrunk==mx_صفحہ );
          rc = btree_Get_غیر_مستعمل_صفحہ(pBt, iNewTrunk, &pNewTrunk, 0);
          اگر( rc!=SQLITE_OK ){
            جاؤ انتہا_allocate_صفحہ;
          }
          rc = sqlite3PagerWrite(pNewTrunk->p_Db_صفحہ);
          اگر( rc!=SQLITE_OK ){
            release_صفحہ(pNewTrunk);
            جاؤ انتہا_allocate_صفحہ;
          }
          حافظہ_نقل(&pNewTrunk->a_ڈیٹا[0], &pTrunk->a_ڈیٹا[0], 4);
          put4byte(&pNewTrunk->a_ڈیٹا[4], k-1);
          حافظہ_نقل(&pNewTrunk->a_ڈیٹا[8], &pTrunk->a_ڈیٹا[12], (k-1)*4);
          release_صفحہ(pNewTrunk);
          اگر( !p_پچھلا_Trunk ){
            أكد( sqlite3PagerIswriteable(pPage1->p_Db_صفحہ) );
            put4byte(&pPage1->a_ڈیٹا[32], iNewTrunk);
          }ورنہ{
            rc = sqlite3PagerWrite(p_پچھلا_Trunk->p_Db_صفحہ);
            اگر( rc ){
              جاؤ انتہا_allocate_صفحہ;
            }
            put4byte(&p_پچھلا_Trunk->a_ڈیٹا[0], iNewTrunk);
          }
        }
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
#اگر_ختم
      }ورنہ اگر( k>0 ){
        /* Extract a leaf from the trunk */
        u32 closest;
        Pgno i_صفحہ;
        غیرعلامتی حرف *a_ڈیٹا = pTrunk->a_ڈیٹا;
        اگر( nearby>0 ){
          u32 i;
          closest = 0;
          اگر( e_منوال==BTALLOC_LE ){
            برائے(i=0; i<k; i++){
              i_صفحہ = get4byte(&a_ڈیٹا[8+i*4]);
              اگر( i_صفحہ<=nearby ){
                closest = i;
                توڑو;
              }
            }
          }ورنہ{
            صحیح dist;
            dist = sqlite3AbsInt32(get4byte(&a_ڈیٹا[8]) - nearby);
            برائے(i=1; i<k; i++){
              صحیح d2 = sqlite3AbsInt32(get4byte(&a_ڈیٹا[8+i*4]) - nearby);
              اگر( d2<dist ){
                closest = i;
                dist = d2;
              }
            }
          }
        }ورنہ{
          closest = 0;
        }

        i_صفحہ = get4byte(&a_ڈیٹا[8+closest*4]);
        testcase( i_صفحہ==mx_صفحہ );
        اگر( i_صفحہ>mx_صفحہ || i_صفحہ<2 ){
          rc = SQLITE_CORRUPT_PGNO(iTrunk);
          جاؤ انتہا_allocate_صفحہ;
        }
        testcase( i_صفحہ==mx_صفحہ );
        اگر( !تلاش_List
         || (i_صفحہ==nearby || (i_صفحہ<nearby && e_منوال==BTALLOC_LE))
        ){
          صحیح noContent;
          *pPgno = i_صفحہ;
          TRACE(("ALLOCATE: %u was leaf %u of %u on trunk %u"
                 ": %u more free pages\n",
                 *pPgno, closest+1, k, pTrunk->pgno, n-1));
          rc = sqlite3PagerWrite(pTrunk->p_Db_صفحہ);
          اگر( rc ) جاؤ انتہا_allocate_صفحہ;
          اگر( closest<k-1 ){
            حافظہ_نقل(&a_ڈیٹا[8+closest*4], &a_ڈیٹا[4+k*4], 4);
          }
          put4byte(&a_ڈیٹا[4], k-1);
          noContent = !btreeGetHasContent(pBt, *pPgno)? PAGER_GET_NOCONTENT : 0;
          rc = btree_Get_غیر_مستعمل_صفحہ(pBt, *pPgno, pp_صفحہ, noContent);
          اگر( rc==SQLITE_OK ){
            rc = sqlite3PagerWrite((*pp_صفحہ)->p_Db_صفحہ);
            اگر( rc!=SQLITE_OK ){
              release_صفحہ(*pp_صفحہ);
              *pp_صفحہ = 0;
            }
          }
          تلاش_List = 0;
        }
      }
      release_صفحہ(p_پچھلا_Trunk);
      p_پچھلا_Trunk = 0;
    }جب( تلاش_List );
  }ورنہ{
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
    صحیح bNoContent = (0==IfNotOmitAV(pBt->bDoTruncate))? PAGER_GET_NOCONTENT:0;

    rc = sqlite3PagerWrite(pBt->pPage1->p_Db_صفحہ);
    اگر( rc ) واپس rc;
    pBt->n_صفحہ++;
    اگر( pBt->n_صفحہ==PENDING_BYTE_صفحہ(pBt) ) pBt->n_صفحہ++;

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
    اگر( pBt->autoVacuum && PTRMAP_ISPAGE(pBt, pBt->n_صفحہ) ){
      /* If *pPgno refers to a pointer-map page, allocate two new pages
      ** at the end of the file instead of one. The first allocated page
      ** becomes a new pointer-map page, the second is used by the caller.
      */
      Mem_صفحہ *pPg = 0;
      TRACE(("ALLOCATE: %u from end of file (pointer-map page)\n", pBt->n_صفحہ));
      أكد( pBt->n_صفحہ!=PENDING_BYTE_صفحہ(pBt) );
      rc = btree_Get_غیر_مستعمل_صفحہ(pBt, pBt->n_صفحہ, &pPg, bNoContent);
      اگر( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pPg->p_Db_صفحہ);
        release_صفحہ(pPg);
      }
      اگر( rc ) واپس rc;
      pBt->n_صفحہ++;
      اگر( pBt->n_صفحہ==PENDING_BYTE_صفحہ(pBt) ){ pBt->n_صفحہ++; }
    }
#اگر_ختم
    put4byte(28 + (u8*)pBt->pPage1->a_ڈیٹا, pBt->n_صفحہ);
    *pPgno = pBt->n_صفحہ;

    أكد( *pPgno!=PENDING_BYTE_صفحہ(pBt) );
    rc = btree_Get_غیر_مستعمل_صفحہ(pBt, *pPgno, pp_صفحہ, bNoContent);
    اگر( rc ) واپس rc;
    rc = sqlite3PagerWrite((*pp_صفحہ)->p_Db_صفحہ);
    اگر( rc!=SQLITE_OK ){
      release_صفحہ(*pp_صفحہ);
      *pp_صفحہ = 0;
    }
    TRACE(("ALLOCATE: %u from end of file\n", *pPgno));
  }

  أكد( CORRUPT_DB || *pPgno!=PENDING_BYTE_صفحہ(pBt) );

انتہا_allocate_صفحہ:
  release_صفحہ(pTrunk);
  release_صفحہ(p_پچھلا_Trunk);
  أكد( rc!=SQLITE_OK || sqlite3_Pager_صفحہ_Refcount((*pp_صفحہ)->p_Db_صفحہ)<=1 );
  أكد( rc!=SQLITE_OK || (*pp_صفحہ)->isInit==0 );
  واپس rc;
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
ساکن صحیح freePage2(BtShared *pBt, Mem_صفحہ *p_Mem_صفحہ, Pgno i_صفحہ){
  Mem_صفحہ *pTrunk = 0;                /* Free-list trunk page */
  Pgno iTrunk = 0;                    /* Page number of free-list trunk page */
  Mem_صفحہ *pPage1 = pBt->pPage1;      /* Local reference to page 1 */
  Mem_صفحہ *p_صفحہ;                     /* Page being freed. May be NULL. */
  صحیح rc;                             /* Return Code */
  u32 nFree;                          /* Initial number of pages on free-list */

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( CORRUPT_DB || i_صفحہ>1 );
  أكد( !p_Mem_صفحہ || p_Mem_صفحہ->pgno==i_صفحہ );

  اگر( i_صفحہ<2 || i_صفحہ>pBt->n_صفحہ ){
    واپس SQLITE_CORRUPT_BKPT;
  }
  اگر( p_Mem_صفحہ ){
    p_صفحہ = p_Mem_صفحہ;
    sqlite3PagerRef(p_صفحہ->p_Db_صفحہ);
  }ورنہ{
    p_صفحہ = btree_صفحہ_Lookup(pBt, i_صفحہ);
  }

  /* Increment the free page count on pPage1 */
  rc = sqlite3PagerWrite(pPage1->p_Db_صفحہ);
  اگر( rc ) جاؤ freepage_برآمد;
  nFree = get4byte(&pPage1->a_ڈیٹا[36]);
  put4byte(&pPage1->a_ڈیٹا[36], nFree+1);

  اگر( pBt->btsFlags & BTS_SECURE_DELETE ){
    /* If the secure_delete option is enabled, then
    ** always fully overwrite deleted information with zeros.
    */
    اگر( (!p_صفحہ && ((rc = btree_Get_صفحہ(pBt, i_صفحہ, &p_صفحہ, 0))!=0) )
     ||            ((rc = sqlite3PagerWrite(p_صفحہ->p_Db_صفحہ))!=0)
    ){
      جاؤ freepage_برآمد;
    }
    memset(p_صفحہ->a_ڈیٹا, 0, p_صفحہ->pBt->صفحہ_سائز);
  }

  /* If the database supports auto-vacuum, write an entry in the pointer-map
  ** to indicate that the page is free.
  */
  اگر( ISAUTOVACUUM(pBt) ){
    ptrmapPut(pBt, i_صفحہ, PTRMAP_FREEPAGE, 0, &rc);
    اگر( rc ) جاؤ freepage_برآمد;
  }

  /* Now manipulate the actual database free-list structure. There are two
  ** possibilities. If the free-list is currently empty, or if the first
  ** trunk page in the free-list is full, then this page will become a
  ** new free-list trunk page. Otherwise, it will become a leaf of the
  ** first trunk page in the current free-list. This block tests if it
  ** is possible to add the page as a new free-list leaf.
  */
  اگر( nFree!=0 ){
    u32 n_برگ;                /* Initial number of leaf cells on trunk page */

    iTrunk = get4byte(&pPage1->a_ڈیٹا[32]);
    اگر( iTrunk>btreePagecount(pBt) ){
      rc = SQLITE_CORRUPT_BKPT;
      جاؤ freepage_برآمد;
    }
    rc = btree_Get_صفحہ(pBt, iTrunk, &pTrunk, 0);
    اگر( rc!=SQLITE_OK ){
      جاؤ freepage_برآمد;
    }

    n_برگ = get4byte(&pTrunk->a_ڈیٹا[4]);
    أكد( pBt->usable_سائز>32 );
    اگر( n_برگ > (u32)pBt->usable_سائز/4 - 2 ){
      rc = SQLITE_CORRUPT_BKPT;
      جاؤ freepage_برآمد;
    }
    اگر( n_برگ < (u32)pBt->usable_سائز/4 - 8 ){
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
      rc = sqlite3PagerWrite(pTrunk->p_Db_صفحہ);
      اگر( rc==SQLITE_OK ){
        put4byte(&pTrunk->a_ڈیٹا[4], n_برگ+1);
        put4byte(&pTrunk->a_ڈیٹا[8+n_برگ*4], i_صفحہ);
        اگر( p_صفحہ && (pBt->btsFlags & BTS_SECURE_DELETE)==0 ){
          sqlite3PagerDontWrite(p_صفحہ->p_Db_صفحہ);
        }
        rc = btree_سیٹ_Has_Content(pBt, i_صفحہ);
      }
      TRACE(("FREE-PAGE: %u leaf on trunk page %u\n",p_صفحہ->pgno,pTrunk->pgno));
      جاؤ freepage_برآمد;
    }
  }

  /* If control flows to this point, then it was not possible to add the
  ** the page being freed as a leaf page of the first trunk in the free-list.
  ** Possibly because the free-list is empty, or possibly because the
  ** first trunk in the free-list is full. Either way, the page being freed
  ** will become the new first trunk page in the free-list.
  */
  اگر( p_صفحہ==0 && SQLITE_OK!=(rc = btree_Get_صفحہ(pBt, i_صفحہ, &p_صفحہ, 0)) ){
    جاؤ freepage_برآمد;
  }
  rc = sqlite3PagerWrite(p_صفحہ->p_Db_صفحہ);
  اگر( rc!=SQLITE_OK ){
    جاؤ freepage_برآمد;
  }
  put4byte(p_صفحہ->a_ڈیٹا, iTrunk);
  put4byte(&p_صفحہ->a_ڈیٹا[4], 0);
  put4byte(&pPage1->a_ڈیٹا[32], i_صفحہ);
  TRACE(("FREE-PAGE: %u new trunk page replacing %u\n", p_صفحہ->pgno, iTrunk));

freepage_برآمد:
  اگر( p_صفحہ ){
    p_صفحہ->isInit = 0;
  }
  release_صفحہ(p_صفحہ);
  release_صفحہ(pTrunk);
  واپس rc;
}
ساکن خالی free_صفحہ(Mem_صفحہ *p_صفحہ, صحیح *pRC){
  اگر( (*pRC)==SQLITE_OK ){
    *pRC = freePage2(p_صفحہ->pBt, p_صفحہ, p_صفحہ->pgno);
  }
}

/*
** Free the overflow pages associated with the given Cell.
*/
ساکن SQLITE_NOINLINE صحیح clear_Cell_حد_سے_تجاوز(
  Mem_صفحہ *p_صفحہ,          /* The page that contains the Cell */
  غیرعلامتی حرف *pCell,    /* First byte of the Cell */
  CellInfo *pInfo          /* Size information about the cell */
){
  BtShared *pBt;
  Pgno ovflPgno;
  صحیح rc;
  صحیح nOvfl;
  u32 ovfl_صفحہ_سائز;

  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  أكد( pInfo->nLocal!=pInfo->nPayload );
  testcase( pCell + pInfo->n_سائز == p_صفحہ->a_ڈیٹا_انتہا );
  testcase( pCell + (pInfo->n_سائز-1) == p_صفحہ->a_ڈیٹا_انتہا );
  اگر( pCell + pInfo->n_سائز > p_صفحہ->a_ڈیٹا_انتہا ){
    /* Cell extends past end of page */
    واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
  }
  ovflPgno = get4byte(pCell + pInfo->n_سائز - 4);
  pBt = p_صفحہ->pBt;
  أكد( pBt->usable_سائز > 4 );
  ovfl_صفحہ_سائز = pBt->usable_سائز - 4;
  nOvfl = (pInfo->nPayload - pInfo->nLocal + ovfl_صفحہ_سائز - 1)/ovfl_صفحہ_سائز;
  أكد( nOvfl>0 ||
    (CORRUPT_DB && (pInfo->nPayload + ovfl_صفحہ_سائز)<ovfl_صفحہ_سائز)
  );
  جب( nOvfl-- ){
    Pgno i_اگلا = 0;
    Mem_صفحہ *pOvfl = 0;
    اگر( ovflPgno<2 || ovflPgno>btreePagecount(pBt) ){
      /* 0 is not a legal page number and page 1 cannot be an
      ** overflow page. Therefore if ovflPgno<2 or past the end of the
      ** file the database must be corrupt. */
      واپس SQLITE_CORRUPT_BKPT;
    }
    اگر( nOvfl ){
      rc = get_حد_سے_تجاوز_صفحہ(pBt, ovflPgno, &pOvfl, &i_اگلا);
      اگر( rc ) واپس rc;
    }

    اگر( ( pOvfl || ((pOvfl = btree_صفحہ_Lookup(pBt, ovflPgno))!=0) )
     && sqlite3_Pager_صفحہ_Refcount(pOvfl->p_Db_صفحہ)!=1
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
    }ورنہ{
      rc = freePage2(pBt, pOvfl, ovflPgno);
    }

    اگر( pOvfl ){
      sqlite3PagerUnref(pOvfl->p_Db_صفحہ);
    }
    اگر( rc ) واپس rc;
    ovflPgno = i_اگلا;
  }
  واپس SQLITE_OK;
}

/* Call xParseCell to compute the size of a cell.  If the cell contains
** overflow, then invoke cellClearOverflow to clear out that overflow.
** Store the result code (SQLITE_OK or some error code) in rc.
**
** Implemented as macro to force inlining for performance.
*/
#تعریف_کرو BTREE_CLEAR_CELL(rc, p_صفحہ, pCell, sInfo)   \
  p_صفحہ->xParseCell(p_صفحہ, pCell, &sInfo);          \
  اگر( sInfo.nLocal!=sInfo.nPayload ){               \
    rc = clear_Cell_حد_سے_تجاوز(p_صفحہ, pCell, &sInfo);   \
  }ورنہ{                                            \
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
ساکن صحیح fill_اندر_Cell(
  Mem_صفحہ *p_صفحہ,                /* The page that contains the cell */
  غیرعلامتی حرف *pCell,          /* Complete text of the cell */
  ثابت BtreePayload *pX,        /* Payload with which to construct the cell */
  صحیح *pn_سائز                    /* Write cell size here */
){
  صحیح nPayload;
  ثابت u8 *pSrc;
  صحیح nSrc, n, rc, mn;
  صحیح space_بایاں;
  Mem_صفحہ *p_مقصد_Release;
  غیرعلامتی حرف *p_پیشگی;
  غیرعلامتی حرف *pPayload;
  BtShared *pBt;
  Pgno pgnoOvfl;
  صحیح n_سرخی;

  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );

  /* pPage is not necessarily writeable since pCell might be auxiliary
  ** buffer space that is separate from the pPage buffer area */
  أكد( pCell<p_صفحہ->a_ڈیٹا || pCell>=&p_صفحہ->a_ڈیٹا[p_صفحہ->pBt->صفحہ_سائز]
            || sqlite3PagerIswriteable(p_صفحہ->p_Db_صفحہ) );

  /* Fill in the header. */
  n_سرخی = p_صفحہ->بچہ_Ptr_سائز;
  اگر( p_صفحہ->int_کلید ){
    nPayload = pX->n_ڈیٹا + pX->nZero;
    pSrc = pX->p_ڈیٹا;
    nSrc = pX->n_ڈیٹا;
    أكد( p_صفحہ->int_کلید_برگ ); /* fillInCell() only called for leaves */
    n_سرخی += putVarint32(&pCell[n_سرخی], nPayload);
    n_سرخی += sqlite3PutVarint(&pCell[n_سرخی], *(u64*)&pX->n_کلید);
  }ورنہ{
    أكد( pX->n_کلید<=0x7fffffff && pX->p_کلید!=0 );
    nSrc = nPayload = (صحیح)pX->n_کلید;
    pSrc = pX->p_کلید;
    n_سرخی += putVarint32(&pCell[n_سرخی], nPayload);
  }
 
  /* Fill in the payload */
  pPayload = &pCell[n_سرخی];
  اگر( nPayload<=p_صفحہ->maxLocal ){
    /* This is the common case where everything fits on the btree page
    ** and no overflow pages are required. */
    n = n_سرخی + nPayload;
    testcase( n==3 );
    testcase( n==4 );
    اگر( n<4 ){
      n = 4;
      pPayload[nPayload] = 0;
    }
    *pn_سائز = n;
    أكد( nSrc<=nPayload );
    testcase( nSrc<nPayload );
    حافظہ_نقل(pPayload, pSrc, nSrc);
    memset(pPayload+nSrc, 0, nPayload-nSrc);
    واپس SQLITE_OK;
  }

  /* If we reach this point, it means that some of the content will need
  ** to spill onto overflow pages.
  */
  mn = p_صفحہ->minLocal;
  n = mn + (nPayload - mn) % (p_صفحہ->pBt->usable_سائز - 4);
  testcase( n==p_صفحہ->maxLocal );
  testcase( n==p_صفحہ->maxLocal+1 );
  اگر( n > p_صفحہ->maxLocal ) n = mn;
  space_بایاں = n;
  *pn_سائز = n + n_سرخی + 4;
  p_پیشگی = &pCell[n_سرخی+n];
  p_مقصد_Release = 0;
  pgnoOvfl = 0;
  pBt = p_صفحہ->pBt;

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
#اگر_تعریف_شدہ SQLITE_DEBUG
  {
    CellInfo info;
    p_صفحہ->xParseCell(p_صفحہ, pCell, &info);
    أكد( n_سرخی==(صحیح)(info.pPayload - pCell) );
    أكد( info.n_کلید==pX->n_کلید );
    أكد( *pn_سائز == info.n_سائز );
    أكد( space_بایاں == info.nLocal );
  }
#اگر_ختم

  /* Write the payload into the local Cell and any extra into overflow pages */
  جب( 1 ){
    n = nPayload;
    اگر( n>space_بایاں ) n = space_بایاں;

    /* If pToRelease is not zero than pPayload points into the data area
    ** of pToRelease.  Make sure pToRelease is still writeable. */
    أكد( p_مقصد_Release==0 || sqlite3PagerIswriteable(p_مقصد_Release->p_Db_صفحہ) );

    /* If pPayload is part of the data area of pPage, then make sure pPage
    ** is still writeable */
    أكد( pPayload<p_صفحہ->a_ڈیٹا || pPayload>=&p_صفحہ->a_ڈیٹا[pBt->صفحہ_سائز]
            || sqlite3PagerIswriteable(p_صفحہ->p_Db_صفحہ) );

    اگر( nSrc>=n ){
      حافظہ_نقل(pPayload, pSrc, n);
    }ورنہ اگر( nSrc>0 ){
      n = nSrc;
      حافظہ_نقل(pPayload, pSrc, n);
    }ورنہ{
      memset(pPayload, 0, n);
    }
    nPayload -= n;
    اگر( nPayload<=0 ) توڑو;
    pPayload += n;
    pSrc += n;
    nSrc -= n;
    space_بایاں -= n;
    اگر( space_بایاں==0 ){
      Mem_صفحہ *pOvfl = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
      Pgno pgnoPtrmap = pgnoOvfl; /* Overflow page pointer-map entry page */
      اگر( pBt->autoVacuum ){
        کرو{
          pgnoOvfl++;
        } جب(
          PTRMAP_ISPAGE(pBt, pgnoOvfl) || pgnoOvfl==PENDING_BYTE_صفحہ(pBt)
        );
      }
#اگر_ختم
      rc = allocate_Btree_صفحہ(pBt, &pOvfl, &pgnoOvfl, pgnoOvfl, 0);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
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
      اگر( pBt->autoVacuum && rc==SQLITE_OK ){
        u8 eType = (pgnoPtrmap?PTRMAP_OVERFLOW2:PTRMAP_OVERFLOW1);
        ptrmapPut(pBt, pgnoOvfl, eType, pgnoPtrmap, &rc);
        اگر( rc ){
          release_صفحہ(pOvfl);
        }
      }
#اگر_ختم
      اگر( rc ){
        release_صفحہ(p_مقصد_Release);
        واپس rc;
      }

      /* If pToRelease is not zero than pPrior points into the data area
      ** of pToRelease.  Make sure pToRelease is still writeable. */
      أكد( p_مقصد_Release==0 || sqlite3PagerIswriteable(p_مقصد_Release->p_Db_صفحہ) );

      /* If pPrior is part of the data area of pPage, then make sure pPage
      ** is still writeable */
      أكد( p_پیشگی<p_صفحہ->a_ڈیٹا || p_پیشگی>=&p_صفحہ->a_ڈیٹا[pBt->صفحہ_سائز]
            || sqlite3PagerIswriteable(p_صفحہ->p_Db_صفحہ) );

      put4byte(p_پیشگی, pgnoOvfl);
      release_صفحہ(p_مقصد_Release);
      p_مقصد_Release = pOvfl;
      p_پیشگی = pOvfl->a_ڈیٹا;
      put4byte(p_پیشگی, 0);
      pPayload = &pOvfl->a_ڈیٹا[4];
      space_بایاں = pBt->usable_سائز - 4;
    }
  }
  release_صفحہ(p_مقصد_Release);
  واپس SQLITE_OK;
}

/*
** Remove the i-th cell from pPage.  This routine effects pPage only.
** The cell content is not freed or deallocated.  It is assumed that
** the cell content has been copied someplace else.  This routine just
** removes the reference to the cell from pPage.
**
** "sz" must be the number of bytes in the cell.
*/
ساکن خالی dropCell(Mem_صفحہ *p_صفحہ, صحیح idx, صحیح sz, صحیح *pRC){
  u32 pc;         /* Offset to cell content of cell being deleted */
  u8 *ڈیٹا;       /* pPage->aData */
  u8 *ptr;        /* Used to move bytes around within data[] */
  صحیح rc;         /* The return code */
  صحیح hdr;        /* Beginning of the header.  0 most pages.  100 page 1 */

  اگر( *pRC ) واپس;
  أكد( idx>=0 );
  أكد( idx<p_صفحہ->nCell );
  أكد( CORRUPT_DB || sz==cell_سائز(p_صفحہ, idx) );
  أكد( sqlite3PagerIswriteable(p_صفحہ->p_Db_صفحہ) );
  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  أكد( p_صفحہ->nFree>=0 );
  ڈیٹا = p_صفحہ->a_ڈیٹا;
  ptr = &p_صفحہ->aCellIdx[2*idx];
  أكد( p_صفحہ->pBt->usable_سائز > (u32)(ptr-ڈیٹا) );
  pc = get2byte(ptr);
  hdr = p_صفحہ->hdr_انحراف;
  testcase( pc==(u32)get2byte(&ڈیٹا[hdr+5]) );
  testcase( pc+sz==p_صفحہ->pBt->usable_سائز );
  اگر( pc+sz > p_صفحہ->pBt->usable_سائز ){
    *pRC = SQLITE_CORRUPT_BKPT;
    واپس;
  }
  rc = freeSpace(p_صفحہ, pc, sz);
  اگر( rc ){
    *pRC = rc;
    واپس;
  }
  p_صفحہ->nCell--;
  اگر( p_صفحہ->nCell==0 ){
    memset(&ڈیٹا[hdr+1], 0, 4);
    ڈیٹا[hdr+7] = 0;
    put2byte(&ڈیٹا[hdr+5], p_صفحہ->pBt->usable_سائز);
    p_صفحہ->nFree = p_صفحہ->pBt->usable_سائز - p_صفحہ->hdr_انحراف
                       - p_صفحہ->بچہ_Ptr_سائز - 8;
  }ورنہ{
    memmove(ptr, ptr+2, 2*(p_صفحہ->nCell - idx));
    put2byte(&ڈیٹا[hdr+3], p_صفحہ->nCell);
    p_صفحہ->nFree += 2;
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
ساکن صحیح داخل_کرو_Cell(
  Mem_صفحہ *p_صفحہ,   /* Page into which we are copying */
  صحیح i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  صحیح sz,           /* Bytes of content in pCell */
  u8 *p_عارضی,        /* Temp storage space for pCell, if needed */
  Pgno i_بچہ       /* If non-zero, replace first 4 bytes with this value */
){
  صحیح idx = 0;      /* Where to write new cell content in data[] */
  صحیح j;            /* Loop counter */
  u8 *ڈیٹا;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  أكد( i>=0 && i<=p_صفحہ->nCell+p_صفحہ->n_حد_سے_تجاوز );
  أكد( MX_CELL(p_صفحہ->pBt)<=10921 );
  أكد( p_صفحہ->nCell<=MX_CELL(p_صفحہ->pBt) || CORRUPT_DB );
  أكد( p_صفحہ->n_حد_سے_تجاوز<=Array_سائز(p_صفحہ->apOvfl) );
  أكد( Array_سائز(p_صفحہ->apOvfl)==Array_سائز(p_صفحہ->aiOvfl) );
  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  أكد( sz==p_صفحہ->x_Cell_سائز(p_صفحہ, pCell) || CORRUPT_DB );
  أكد( p_صفحہ->nFree>=0 );
  أكد( i_بچہ>0 );
  اگر( p_صفحہ->n_حد_سے_تجاوز || sz+2>p_صفحہ->nFree ){
    اگر( p_عارضی ){
      حافظہ_نقل(p_عارضی, pCell, sz);
      pCell = p_عارضی;
    }
    put4byte(pCell, i_بچہ);
    j = p_صفحہ->n_حد_سے_تجاوز++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    أكد( j < Array_سائز(p_صفحہ->apOvfl)-1 );
    p_صفحہ->apOvfl[j] = pCell;
    p_صفحہ->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    أكد( j==0 || p_صفحہ->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    أكد( j==0 || i==p_صفحہ->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }ورنہ{
    صحیح rc = sqlite3PagerWrite(p_صفحہ->p_Db_صفحہ);
    اگر( NEVER(rc!=SQLITE_OK) ){
      واپس rc;
    }
    أكد( sqlite3PagerIswriteable(p_صفحہ->p_Db_صفحہ) );
    ڈیٹا = p_صفحہ->a_ڈیٹا;
    أكد( &ڈیٹا[p_صفحہ->cell_انحراف]==p_صفحہ->aCellIdx );
    rc = allocateSpace(p_صفحہ, sz, &idx);
    اگر( rc ){ واپس rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    أكد( idx >= 0 );
    أكد( idx >= p_صفحہ->cell_انحراف+2*p_صفحہ->nCell+2 || CORRUPT_DB );
    أكد( idx+sz <= (صحیح)p_صفحہ->pBt->usable_سائز );
    p_صفحہ->nFree -= (u16)(2 + sz);
    /* In a corrupt database where an entry in the cell index section of
    ** a btree page has a value of 3 or less, the pCell value might point
    ** as many as 4 bytes in front of the start of the aData buffer for
    ** the source page.  Make sure this does not cause problems by not
    ** reading the first 4 bytes */
    حافظہ_نقل(&ڈیٹا[idx+4], pCell+4, sz-4);
    put4byte(&ڈیٹا[idx], i_بچہ);
    pIns = p_صفحہ->aCellIdx + i*2;
    اگر( i<p_صفحہ->nCell ) memmove(pIns+2, pIns, 2*(p_صفحہ->nCell - i));
    put2byte(pIns, idx);
    p_صفحہ->nCell++;
    /* increment the cell count */
    اگر( (++ڈیٹا[p_صفحہ->hdr_انحراف+4])==0 ) ڈیٹا[p_صفحہ->hdr_انحراف+3]++;
    أكد( get2byte(&ڈیٹا[p_صفحہ->hdr_انحراف+3])==p_صفحہ->nCell || CORRUPT_DB );
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
    اگر( p_صفحہ->pBt->autoVacuum ){
      صحیح rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(p_صفحہ, p_صفحہ, pCell, &rc2);
      اگر( rc2 ) واپس rc2;
    }
#اگر_ختم
  }
  واپس SQLITE_OK;
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
ساکن صحیح داخل_کرو_Cell_Fast(
  Mem_صفحہ *p_صفحہ,   /* Page into which we are copying */
  صحیح i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  صحیح sz            /* Bytes of content in pCell */
){
  صحیح idx = 0;      /* Where to write new cell content in data[] */
  صحیح j;            /* Loop counter */
  u8 *ڈیٹا;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  أكد( i>=0 && i<=p_صفحہ->nCell+p_صفحہ->n_حد_سے_تجاوز );
  أكد( MX_CELL(p_صفحہ->pBt)<=10921 );
  أكد( p_صفحہ->nCell<=MX_CELL(p_صفحہ->pBt) || CORRUPT_DB );
  أكد( p_صفحہ->n_حد_سے_تجاوز<=Array_سائز(p_صفحہ->apOvfl) );
  أكد( Array_سائز(p_صفحہ->apOvfl)==Array_سائز(p_صفحہ->aiOvfl) );
  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  أكد( sz==p_صفحہ->x_Cell_سائز(p_صفحہ, pCell) || CORRUPT_DB );
  أكد( p_صفحہ->nFree>=0 );
  أكد( p_صفحہ->n_حد_سے_تجاوز==0 );
  اگر( sz+2>p_صفحہ->nFree ){
    j = p_صفحہ->n_حد_سے_تجاوز++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    أكد( j < Array_سائز(p_صفحہ->apOvfl)-1 );
    p_صفحہ->apOvfl[j] = pCell;
    p_صفحہ->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    أكد( j==0 || p_صفحہ->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    أكد( j==0 || i==p_صفحہ->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }ورنہ{
    صحیح rc = sqlite3PagerWrite(p_صفحہ->p_Db_صفحہ);
    اگر( rc!=SQLITE_OK ){
      واپس rc;
    }
    أكد( sqlite3PagerIswriteable(p_صفحہ->p_Db_صفحہ) );
    ڈیٹا = p_صفحہ->a_ڈیٹا;
    أكد( &ڈیٹا[p_صفحہ->cell_انحراف]==p_صفحہ->aCellIdx );
    rc = allocateSpace(p_صفحہ, sz, &idx);
    اگر( rc ){ واپس rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    أكد( idx >= 0 );
    أكد( idx >= p_صفحہ->cell_انحراف+2*p_صفحہ->nCell+2 || CORRUPT_DB );
    أكد( idx+sz <= (صحیح)p_صفحہ->pBt->usable_سائز );
    p_صفحہ->nFree -= (u16)(2 + sz);
    حافظہ_نقل(&ڈیٹا[idx], pCell, sz);
    pIns = p_صفحہ->aCellIdx + i*2;
    اگر( i<p_صفحہ->nCell ) memmove(pIns+2, pIns, 2*(p_صفحہ->nCell - i));
    put2byte(pIns, idx);
    p_صفحہ->nCell++;
    /* increment the cell count */
    اگر( (++ڈیٹا[p_صفحہ->hdr_انحراف+4])==0 ) ڈیٹا[p_صفحہ->hdr_انحراف+3]++;
    أكد( get2byte(&ڈیٹا[p_صفحہ->hdr_انحراف+3])==p_صفحہ->nCell || CORRUPT_DB );
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
    اگر( p_صفحہ->pBt->autoVacuum ){
      صحیح rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(p_صفحہ, p_صفحہ, pCell, &rc2);
      اگر( rc2 ) واپس rc2;
    }
#اگر_ختم
  }
  واپس SQLITE_OK;
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
#تعریف_کرو NN 1             /* Number of neighbors on either side of pPage */
#تعریف_کرو NB 3             /* (NN*2+1): Total pages involved in the balance */

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
قسمتعریف ساخت CellArray CellArray;
ساخت CellArray {
  صحیح nCell;              /* Number of cells in apCell[] */
  Mem_صفحہ *pRef;          /* Reference page */
  u8 **apCell;            /* All cells begin balanced */
  u16 *szCell;            /* Local size of all cells in apCell[] */
  u8 *ap_انتہا[NB*2];        /* MemPage.aDataEnd values */
  صحیح ixNx[NB*2];         /* Index of at which we move to the next apEnd[] */
};

/*
** Make sure the cell sizes at idx, idx+1, ..., idx+N-1 have been
** computed.
*/
ساکن خالی populateCellCache(CellArray *p, صحیح idx, صحیح N){
  Mem_صفحہ *pRef = p->pRef;
  u16 *szCell = p->szCell;
  أكد( idx>=0 && idx+N<=p->nCell );
  جب( N>0 ){
    أكد( p->apCell[idx]!=0 );
    اگر( szCell[idx]==0 ){
      szCell[idx] = pRef->x_Cell_سائز(pRef, p->apCell[idx]);
    }ورنہ{
      أكد( CORRUPT_DB ||
              szCell[idx]==pRef->x_Cell_سائز(pRef, p->apCell[idx]) );
    }
    idx++;
    N--;
  }
}

/*
** Return the size of the Nth element of the cell array
*/
ساکن SQLITE_NOINLINE u16 compute_Cell_سائز(CellArray *p, صحیح N){
  أكد( N>=0 && N<p->nCell );
  أكد( p->szCell[N]==0 );
  p->szCell[N] = p->pRef->x_Cell_سائز(p->pRef, p->apCell[N]);
  واپس p->szCell[N];
}
ساکن u16 cached_Cell_سائز(CellArray *p, صحیح N){
  أكد( N>=0 && N<p->nCell );
  اگر( p->szCell[N] ) واپس p->szCell[N];
  واپس compute_Cell_سائز(p, N);
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
ساکن صحیح rebuild_صفحہ(
  CellArray *pCArray,             /* Content to be added to page pPg */
  صحیح i_پہلا,                     /* First cell in pCArray to use */
  صحیح nCell,                      /* Final number of cells on page */
  Mem_صفحہ *pPg                    /* The page to be reconstructed */
){
  ثابت صحیح hdr = pPg->hdr_انحراف;          /* Offset of header on pPg */
  u8 * ثابت a_ڈیٹا = pPg->a_ڈیٹا;           /* Pointer to data for pPg */
  ثابت صحیح usable_سائز = pPg->pBt->usable_سائز;
  u8 * ثابت p_انتہا = &a_ڈیٹا[usable_سائز];
  صحیح i = i_پہلا;                 /* Which cell to copy from pCArray*/
  u32 j;                          /* Start of cell content area */
  صحیح i_انتہا = i+nCell;             /* Loop terminator */
  u8 *pCellptr = pPg->aCellIdx;
  u8 *pTmp = sqlite3_Pager_عارضی_Space(pPg->pBt->pPager);
  u8 *p_ڈیٹا;
  صحیح k;                          /* Current slot in pCArray->apEnd[] */
  u8 *p_Src_انتہا;                    /* Current pCArray->apEnd[k] value */

  أكد( nCell>0 );
  أكد( i<i_انتہا );
  j = get2byte(&a_ڈیٹا[hdr+5]);
  اگر( j>(u32)usable_سائز ){ j = 0; }
  حافظہ_نقل(&pTmp[j], &a_ڈیٹا[j], usable_سائز - j);

  أكد( pCArray->ixNx[NB*2-1]>i );
  برائے(k=0; pCArray->ixNx[k]<=i; k++){}
  p_Src_انتہا = pCArray->ap_انتہا[k];

  p_ڈیٹا = p_انتہا;
  جب( 1/*exit by break*/ ){
    u8 *pCell = pCArray->apCell[i];
    u16 sz = pCArray->szCell[i];
    أكد( sz>0 );
    اگر( SQLITE_WITHIN(pCell,a_ڈیٹا+j,p_انتہا) ){
      اگر( ((uptr)(pCell+sz))>(uptr)p_انتہا ) واپس SQLITE_CORRUPT_BKPT;
      pCell = &pTmp[pCell - a_ڈیٹا];
    }ورنہ اگر( (uptr)(pCell+sz)>(uptr)p_Src_انتہا
           && (uptr)(pCell)<(uptr)p_Src_انتہا
    ){
      واپس SQLITE_CORRUPT_BKPT;
    }

    p_ڈیٹا -= sz;
    put2byte(pCellptr, (p_ڈیٹا - a_ڈیٹا));
    pCellptr += 2;
    اگر( p_ڈیٹا < pCellptr ) واپس SQLITE_CORRUPT_BKPT;
    memmove(p_ڈیٹا, pCell, sz);
    أكد( sz==pPg->x_Cell_سائز(pPg, pCell) || CORRUPT_DB );
    i++;
    اگر( i>=i_انتہا ) توڑو;
    اگر( pCArray->ixNx[k]<=i ){
      k++;
      p_Src_انتہا = pCArray->ap_انتہا[k];
    }
  }

  /* The pPg->nFree field is now set incorrectly. The caller will fix it. */
  أكد( nCell < 10922 );
  pPg->nCell = (u16)nCell;
  pPg->n_حد_سے_تجاوز = 0;

  put2byte(&a_ڈیٹا[hdr+1], 0);
  put2byte(&a_ڈیٹا[hdr+3], pPg->nCell);
  put2byte(&a_ڈیٹا[hdr+5], p_ڈیٹا - a_ڈیٹا);
  a_ڈیٹا[hdr+7] = 0x00;
  واپس SQLITE_OK;
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
ساکن صحیح صفحہ_داخل_کرو_Array(
  Mem_صفحہ *pPg,                   /* Page to add cells to */
  u8 *pBegin,                     /* End of cell-pointer array */
  u8 **pp_ڈیٹا,                    /* IN/OUT: Page content-area pointer */
  u8 *pCellptr,                   /* Pointer to cell-pointer area */
  صحیح i_پہلا,                     /* Index of first cell to add */
  صحیح nCell,                      /* Number of cells to add to pPg */
  CellArray *pCArray              /* Array of cells */
){
  صحیح i = i_پہلا;                 /* Loop counter - cell index to insert */
  u8 *a_ڈیٹا = pPg->a_ڈیٹا;         /* Complete page */
  u8 *p_ڈیٹا = *pp_ڈیٹا;            /* Content area.  A subset of aData[] */
  صحیح i_انتہا = i_پہلا + nCell;      /* End of loop. One past last cell to ins */
  صحیح k;                          /* Current slot in pCArray->apEnd[] */
  u8 *p_انتہا;                       /* Maximum extent of cell data */
  أكد( CORRUPT_DB || pPg->hdr_انحراف==0 );    /* Never called on page 1 */
  اگر( i_انتہا<=i_پہلا ) واپس 0;
  أكد( pCArray->ixNx[NB*2-1]>i );
  برائے(k=0; pCArray->ixNx[k]<=i ; k++){}
  p_انتہا = pCArray->ap_انتہا[k];
  جب( 1 /*Exit by break*/ ){
    صحیح sz, rc;
    u8 *p_خانہ;
    أكد( pCArray->szCell[i]!=0 );
    sz = pCArray->szCell[i];
    اگر( (a_ڈیٹا[1]==0 && a_ڈیٹا[2]==0) || (p_خانہ = صفحہ_ڈھونڈو_خانہ(pPg,sz,&rc))==0 ){
      اگر( (p_ڈیٹا - pBegin)<sz ) واپس 1;
      p_ڈیٹا -= sz;
      p_خانہ = p_ڈیٹا;
    }
    /* pSlot and pCArray->apCell[i] will never overlap on a well-formed
    ** database.  But they might for a corrupt database.  Hence use memmove()
    ** since memcpy() sends SIGABORT with overlapping buffers on OpenBSD */
    أكد( (p_خانہ+sz)<=pCArray->apCell[i]
         || p_خانہ>=(pCArray->apCell[i]+sz)
         || CORRUPT_DB );
    اگر( (uptr)(pCArray->apCell[i]+sz)>(uptr)p_انتہا
     && (uptr)(pCArray->apCell[i])<(uptr)p_انتہا
    ){
      أكد( CORRUPT_DB );
      (خالی)SQLITE_CORRUPT_BKPT;
      واپس 1;
    }
    memmove(p_خانہ, pCArray->apCell[i], sz);
    put2byte(pCellptr, (p_خانہ - a_ڈیٹا));
    pCellptr += 2;
    i++;
    اگر( i>=i_انتہا ) توڑو;
    اگر( pCArray->ixNx[k]<=i ){
      k++;
      p_انتہا = pCArray->ap_انتہا[k];
    }
  }
  *pp_ڈیٹا = p_ڈیٹا;
  واپس 0;
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
ساکن صحیح صفحہ_Free_Array(
  Mem_صفحہ *pPg,                   /* Page to edit */
  صحیح i_پہلا,                     /* First cell to delete */
  صحیح nCell,                      /* Cells to delete */
  CellArray *pCArray              /* Array of cells */
){
  u8 * ثابت a_ڈیٹا = pPg->a_ڈیٹا;
  u8 * ثابت p_انتہا = &a_ڈیٹا[pPg->pBt->usable_سائز];
  u8 * ثابت p_آغاز = &a_ڈیٹا[pPg->hdr_انحراف + 8 + pPg->بچہ_Ptr_سائز];
  صحیح nRet = 0;
  صحیح i, j;
  صحیح i_انتہا = i_پہلا + nCell;
  صحیح nFree = 0;
  صحیح aOfst[10];
  صحیح aAfter[10];

  برائے(i=i_پہلا; i<i_انتہا; i++){
    u8 *pCell = pCArray->apCell[i];
    اگر( SQLITE_WITHIN(pCell, p_آغاز, p_انتہا) ){
      صحیح sz;
      صحیح iAfter;
      صحیح iOfst;
      /* No need to use cachedCellSize() here.  The sizes of all cells that
      ** are to be freed have already been computing while deciding which
      ** cells need freeing */
      sz = pCArray->szCell[i];  أكد( sz>0 );
      iOfst = (u16)(pCell - a_ڈیٹا);
      iAfter = iOfst+sz;
      برائے(j=0; j<nFree; j++){
        اگر( aOfst[j]==iAfter ){
          aOfst[j] = iOfst;
          توڑو;
        }ورنہ اگر( aAfter[j]==iOfst ){
          aAfter[j] = iAfter;
          توڑو;
        }
      }
      اگر( j>=nFree ){
        اگر( nFree>=(صحیح)(حجم(aOfst)/حجم(aOfst[0])) ){
          برائے(j=0; j<nFree; j++){
            freeSpace(pPg, aOfst[j], aAfter[j]-aOfst[j]);
          }
          nFree = 0;
        }
        aOfst[nFree] = iOfst;
        aAfter[nFree] = iAfter;
        اگر( &a_ڈیٹا[iAfter]>p_انتہا ) واپس 0;
        nFree++;
      }
      nRet++;
    }
  }
  برائے(j=0; j<nFree; j++){
    freeSpace(pPg, aOfst[j], aAfter[j]-aOfst[j]);
  }
  واپس nRet;
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
ساکن صحیح edit_صفحہ(
  Mem_صفحہ *pPg,                   /* Edit this page */
  صحیح iOld,                       /* Index of first cell currently on page */
  صحیح iNew,                       /* Index of new first cell on page */
  صحیح nNew,                       /* Final number of cells on page */
  CellArray *pCArray              /* Array of cells and sizes */
){
  u8 * ثابت a_ڈیٹا = pPg->a_ڈیٹا;
  ثابت صحیح hdr = pPg->hdr_انحراف;
  u8 *pBegin = &pPg->aCellIdx[nNew * 2];
  صحیح nCell = pPg->nCell;       /* Cells stored on pPg */
  u8 *p_ڈیٹا;
  u8 *pCellptr;
  صحیح i;
  صحیح i_Old_انتہا = iOld + pPg->nCell + pPg->n_حد_سے_تجاوز;
  صحیح i_New_انتہا = iNew + nNew;

#اگر_تعریف_شدہ SQLITE_DEBUG
  u8 *pTmp = sqlite3_Pager_عارضی_Space(pPg->pBt->pPager);
  حافظہ_نقل(pTmp, a_ڈیٹا, pPg->pBt->usable_سائز);
#اگر_ختم

  /* Remove cells from the start and end of the page */
  أكد( nCell>=0 );
  اگر( iOld<iNew ){
    صحیح n_سرکاؤ = صفحہ_Free_Array(pPg, iOld, iNew-iOld, pCArray);
    اگر( NEVER(n_سرکاؤ>nCell) ) واپس SQLITE_CORRUPT_BKPT;
    memmove(pPg->aCellIdx, &pPg->aCellIdx[n_سرکاؤ*2], nCell*2);
    nCell -= n_سرکاؤ;
  }
  اگر( i_New_انتہا < i_Old_انتہا ){
    صحیح n_دم = صفحہ_Free_Array(pPg, i_New_انتہا, i_Old_انتہا - i_New_انتہا, pCArray);
    أكد( nCell>=n_دم );
    nCell -= n_دم;
  }

  p_ڈیٹا = &a_ڈیٹا[get2byte(&a_ڈیٹا[hdr+5])];
  اگر( p_ڈیٹا<pBegin ) جاؤ editpage_fail;
  اگر( NEVER(p_ڈیٹا>pPg->a_ڈیٹا_انتہا) ) جاؤ editpage_fail;

  /* Add cells to the start of the page */
  اگر( iNew<iOld ){
    صحیح n_جوڑو = MIN(nNew,iOld-iNew);
    أكد( (iOld-iNew)<nNew || nCell==0 || CORRUPT_DB );
    أكد( n_جوڑو>=0 );
    pCellptr = pPg->aCellIdx;
    memmove(&pCellptr[n_جوڑو*2], pCellptr, nCell*2);
    اگر( صفحہ_داخل_کرو_Array(
          pPg, pBegin, &p_ڈیٹا, pCellptr,
          iNew, n_جوڑو, pCArray
    ) ) جاؤ editpage_fail;
    nCell += n_جوڑو;
  }

  /* Add any overflow cells */
  برائے(i=0; i<pPg->n_حد_سے_تجاوز; i++){
    صحیح iCell = (iOld + pPg->aiOvfl[i]) - iNew;
    اگر( iCell>=0 && iCell<nNew ){
      pCellptr = &pPg->aCellIdx[iCell * 2];
      اگر( nCell>iCell ){
        memmove(&pCellptr[2], pCellptr, (nCell - iCell) * 2);
      }
      nCell++;
      cached_Cell_سائز(pCArray, iCell+iNew);
      اگر( صفحہ_داخل_کرو_Array(
            pPg, pBegin, &p_ڈیٹا, pCellptr,
            iCell+iNew, 1, pCArray
      ) ) جاؤ editpage_fail;
    }
  }

  /* Append cells to the end of the page */
  أكد( nCell>=0 );
  pCellptr = &pPg->aCellIdx[nCell*2];
  اگر( صفحہ_داخل_کرو_Array(
        pPg, pBegin, &p_ڈیٹا, pCellptr,
        iNew+nCell, nNew-nCell, pCArray
      )
  ){
    جاؤ editpage_fail;
  }

  أكد( nNew < 10922 );
  pPg->nCell = (u16)nNew;
  pPg->n_حد_سے_تجاوز = 0;

  put2byte(&a_ڈیٹا[hdr+3], pPg->nCell);
  put2byte(&a_ڈیٹا[hdr+5], p_ڈیٹا - a_ڈیٹا);

#اگر_تعریف_شدہ SQLITE_DEBUG
  برائے(i=0; i<nNew && !CORRUPT_DB; i++){
    u8 *pCell = pCArray->apCell[i+iNew];
    صحیح iOff = get2byteAligned(&pPg->aCellIdx[i*2]);
    اگر( SQLITE_WITHIN(pCell, a_ڈیٹا, &a_ڈیٹا[pPg->pBt->usable_سائز]) ){
      pCell = &pTmp[pCell - a_ڈیٹا];
    }
    أكد( 0==memcmp(pCell, &a_ڈیٹا[iOff],
            pCArray->pRef->x_Cell_سائز(pCArray->pRef, pCArray->apCell[i+iNew])) );
  }
#اگر_ختم

  واپس SQLITE_OK;
 editpage_fail:
  /* Unable to edit this page. Rebuild it from scratch instead. */
  اگر( nNew<1 ) واپس SQLITE_CORRUPT_BKPT;
  populateCellCache(pCArray, iNew, nNew);
  واپس rebuild_صفحہ(pCArray, iNew, nNew, pPg);
}


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_QUICKBALANCE
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
ساکن صحیح balance_تیز(Mem_صفحہ *p_والد, Mem_صفحہ *p_صفحہ, u8 *pSpace){
  BtShared *ثابت pBt = p_صفحہ->pBt;    /* B-Tree Database */
  Mem_صفحہ *pNew;                       /* Newly allocated page */
  صحیح rc;                              /* Return Code */
  Pgno pgnoNew;                        /* Page number of pNew */

  أكد( sqlite3_mutex_held(p_صفحہ->pBt->mutex) );
  أكد( sqlite3PagerIswriteable(p_والد->p_Db_صفحہ) );
  أكد( p_صفحہ->n_حد_سے_تجاوز==1 );
 
  اگر( p_صفحہ->nCell==0 ) واپس SQLITE_CORRUPT_BKPT;  /* dbfuzz001.test */
  أكد( p_صفحہ->nFree>=0 );
  أكد( p_والد->nFree>=0 );

  /* Allocate a new page. This page will become the right-sibling of
  ** pPage. Make the parent page writable, so that the new divider cell
  ** may be inserted. If both these operations are successful, proceed.
  */
  rc = allocate_Btree_صفحہ(pBt, &pNew, &pgnoNew, 0, 0);

  اگر( rc==SQLITE_OK ){

    u8 *p_برآمد = &pSpace[4];
    u8 *pCell = p_صفحہ->apOvfl[0];
    u16 szCell = p_صفحہ->x_Cell_سائز(p_صفحہ, pCell);
    u8 *pStop;
    CellArray b;

    أكد( sqlite3PagerIswriteable(pNew->p_Db_صفحہ) );
    أكد( CORRUPT_DB || p_صفحہ->a_ڈیٹا[0]==(PTF_INTKEY|PTF_LEAFDATA|PTF_برگ) );
    zero_صفحہ(pNew, PTF_INTKEY|PTF_LEAFDATA|PTF_برگ);
    b.nCell = 1;
    b.pRef = p_صفحہ;
    b.apCell = &pCell;
    b.szCell = &szCell;
    b.ap_انتہا[0] = p_صفحہ->a_ڈیٹا_انتہا;
    b.ixNx[0] = 2;
    b.ixNx[NB*2-1] = 0x7fffffff;
    rc = rebuild_صفحہ(&b, 0, 1, pNew);
    اگر( NEVER(rc) ){
      release_صفحہ(pNew);
      واپس rc;
    }
    pNew->nFree = pBt->usable_سائز - pNew->cell_انحراف - 2 - szCell;

    /* If this is an auto-vacuum database, update the pointer map
    ** with entries for the new page, and any pointer from the
    ** cell on the page to an overflow page. If either of these
    ** operations fails, the return code is set, but the contents
    ** of the parent page are still manipulated by the code below.
    ** That is Ok, at this point the parent page is guaranteed to
    ** be marked as dirty. Returning an error code will cause a
    ** rollback, undoing any changes made to the parent page.
    */
    اگر( ISAUTOVACUUM(pBt) ){
      ptrmapPut(pBt, pgnoNew, PTRMAP_BTREE, p_والد->pgno, &rc);
      اگر( szCell>pNew->minLocal ){
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
    pCell = ڈھونڈو_Cell(p_صفحہ, p_صفحہ->nCell-1);
    pStop = &pCell[9];
    جب( (*(pCell++)&0x80) && pCell<pStop );
    pStop = &pCell[9];
    جب( ((*(p_برآمد++) = *(pCell++))&0x80) && pCell<pStop );

    /* Insert the new divider cell into pParent. */
    اگر( rc==SQLITE_OK ){
      rc = داخل_کرو_Cell(p_والد, p_والد->nCell, pSpace, (صحیح)(p_برآمد-pSpace),
                      0, p_صفحہ->pgno);
    }

    /* Set the right-child pointer of pParent to point to the new page. */
    put4byte(&p_والد->a_ڈیٹا[p_والد->hdr_انحراف+8], pgnoNew);
 
    /* Release the reference to the new page. */
    release_صفحہ(pNew);
  }

  واپس rc;
}
#اگر_ختم /* SQLITE_OMIT_QUICKBALANCE */

#اگر 0
/*
** This function does not contribute anything to the operation of SQLite.
** it is sometimes activated temporarily while debugging code responsible
** for setting pointer-map entries.
*/
ساکن صحیح ptrmap_Check_صفحات(Mem_صفحہ **ap_صفحہ, صحیح n_صفحہ){
  صحیح i, j;
  برائے(i=0; i<n_صفحہ; i++){
    Pgno n;
    u8 e;
    Mem_صفحہ *p_صفحہ = ap_صفحہ[i];
    BtShared *pBt = p_صفحہ->pBt;
    أكد( p_صفحہ->isInit );

    برائے(j=0; j<p_صفحہ->nCell; j++){
      CellInfo info;
      u8 *z;
    
      z = ڈھونڈو_Cell(p_صفحہ, j);
      p_صفحہ->xParseCell(p_صفحہ, z, &info);
      اگر( info.nLocal<info.nPayload ){
        Pgno ovfl = get4byte(&z[info.n_سائز-4]);
        ptrmapGet(pBt, ovfl, &e, &n);
        أكد( n==p_صفحہ->pgno && e==PTRMAP_OVERFLOW1 );
      }
      اگر( !p_صفحہ->برگ ){
        Pgno بچہ = get4byte(z);
        ptrmapGet(pBt, بچہ, &e, &n);
        أكد( n==p_صفحہ->pgno && e==PTRMAP_BTREE );
      }
    }
    اگر( !p_صفحہ->برگ ){
      Pgno بچہ = get4byte(&p_صفحہ->a_ڈیٹا[p_صفحہ->hdr_انحراف+8]);
      ptrmapGet(pBt, بچہ, &e, &n);
      أكد( n==p_صفحہ->pgno && e==PTRMAP_BTREE );
    }
  }
  واپس 1;
}
#اگر_ختم

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
ساکن خالی copy_گرہ_Content(Mem_صفحہ *pFrom, Mem_صفحہ *p_مقصد, صحیح *pRC){
  اگر( (*pRC)==SQLITE_OK ){
    BtShared * ثابت pBt = pFrom->pBt;
    u8 * ثابت aFrom = pFrom->a_ڈیٹا;
    u8 * ثابت a_مقصد = p_مقصد->a_ڈیٹا;
    صحیح ثابت iFromHdr = pFrom->hdr_انحراف;
    صحیح ثابت i_مقصد_Hdr = ((p_مقصد->pgno==1) ? 100 : 0);
    صحیح rc;
    صحیح i_ڈیٹا;
 
 
    أكد( pFrom->isInit );
    أكد( pFrom->nFree>=i_مقصد_Hdr );
    أكد( get2byte(&aFrom[iFromHdr+5]) <= (صحیح)pBt->usable_سائز );
 
    /* Copy the b-tree node content from page pFrom to page pTo. */
    i_ڈیٹا = get2byte(&aFrom[iFromHdr+5]);
    حافظہ_نقل(&a_مقصد[i_ڈیٹا], &aFrom[i_ڈیٹا], pBt->usable_سائز-i_ڈیٹا);
    حافظہ_نقل(&a_مقصد[i_مقصد_Hdr], &aFrom[iFromHdr], pFrom->cell_انحراف + 2*pFrom->nCell);
 
    /* Reinitialize page pTo so that the contents of the MemPage structure
    ** match the new data. The initialization of pTo can actually fail under
    ** fairly obscure circumstances, even though it is a copy of initialized
    ** page pFrom.
    */
    p_مقصد->isInit = 0;
    rc = btree_Init_صفحہ(p_مقصد);
    اگر( rc==SQLITE_OK ) rc = btreeComputeFreeSpace(p_مقصد);
    اگر( rc!=SQLITE_OK ){
      *pRC = rc;
      واپس;
    }
 
    /* If this is an auto-vacuum database, update the pointer-map entries
    ** for any b-tree or overflow pages that pTo now contains the pointers to.
    */
    اگر( ISAUTOVACUUM(pBt) ){
      *pRC = سیٹ_بچہ_Ptrmaps(p_مقصد);
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
ساکن صحیح balance_nonroot(
  Mem_صفحہ *p_والد,               /* Parent page of siblings being balanced */
  صحیح i_والد_Idx,                 /* Index of "the page" in pParent */
  u8 *aOvflSpace,                 /* page-size bytes of space for parent ovfl */
  صحیح is_جڑ,                     /* True if pParent is a root-page */
  صحیح bBulk                       /* True if this call is part of a bulk load */
){
  BtShared *pBt;               /* The whole database */
  صحیح nMaxCells = 0;           /* Allocated size of apCell, szCell, aFrom. */
  صحیح nNew = 0;                /* Number of pages in apNew[] */
  صحیح nOld;                    /* Number of pages in apOld[] */
  صحیح i, j, k;                 /* Loop counters */
  صحیح nxDiv;                   /* Next divider slot in pParent->aCell[] */
  صحیح rc = SQLITE_OK;          /* The return code */
  u16 برگ_Correction;          /* 4 if pPage is a leaf.  0 if not */
  صحیح برگ_ڈیٹا;                /* True if pPage is a leaf of a LEAFDATA tree */
  صحیح usableSpace;             /* Bytes in pPage beyond the header */
  صحیح صفحہ_Flags;               /* Value of pPage->aData[0] */
  صحیح iSpace1 = 0;             /* First unused byte of aSpace1[] */
  صحیح iOvflSpace = 0;          /* First unused byte of aOvflSpace[] */
  u64 szScratch;               /* Size of scratch memory requested */
  Mem_صفحہ *apOld[NB];          /* pPage and up to two siblings */
  Mem_صفحہ *apNew[NB+2];        /* pPage and up to NB siblings after balancing */
  u8 *p_دایاں;                  /* Location in parent of right-sibling pointer */
  u8 *apDiv[NB-1];             /* Divider cells in pParent */
  صحیح cntNew[NB+2];            /* Index in b.paCell[] of cell after i-th page */
  صحیح cntOld[NB+2];            /* Old index in b.apCell[] */
  صحیح szNew[NB+2];             /* Combined size of cells placed on i-th page */
  u8 *aSpace1;                 /* Space for copies of dividers cells */
  Pgno pgno;                   /* Temp var to store a page number in */
  u8 ab_مکمل[NB+2];             /* True after i'th new page is populated */
  Pgno aPgno[NB+2];            /* Page numbers of new pages before shuffling */
  CellArray b;                 /* Parsed information on cells being balanced */

  memset(ab_مکمل, 0, حجم(ab_مکمل));
  أكد( حجم(b) - حجم(b.ixNx) == offsetof(CellArray,ixNx) );
  memset(&b, 0, حجم(b)-حجم(b.ixNx[0]));
  b.ixNx[NB*2-1] = 0x7fffffff;
  pBt = p_والد->pBt;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( sqlite3PagerIswriteable(p_والد->p_Db_صفحہ) );

  /* At this point pParent may have at most one overflow cell. And if
  ** this overflow cell is present, it must be the cell with
  ** index iParentIdx. This scenario comes about when this function
  ** is called (indirectly) from sqlite3BtreeDelete().
  */
  أكد( p_والد->n_حد_سے_تجاوز==0 || p_والد->n_حد_سے_تجاوز==1 );
  أكد( p_والد->n_حد_سے_تجاوز==0 || p_والد->aiOvfl[0]==i_والد_Idx );

  اگر( !aOvflSpace ){
    واپس SQLITE_NOMEM_BKPT;
  }
  أكد( p_والد->nFree>=0 );

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
  i = p_والد->n_حد_سے_تجاوز + p_والد->nCell;
  اگر( i<2 ){
    nxDiv = 0;
  }ورنہ{
    أكد( bBulk==0 || bBulk==1 );
    اگر( i_والد_Idx==0 ){                
      nxDiv = 0;
    }ورنہ اگر( i_والد_Idx==i ){
      nxDiv = i-2+bBulk;
    }ورنہ{
      nxDiv = i_والد_Idx-1;
    }
    i = 2-bBulk;
  }
  nOld = i+1;
  اگر( (i+nxDiv-p_والد->n_حد_سے_تجاوز)==p_والد->nCell ){
    p_دایاں = &p_والد->a_ڈیٹا[p_والد->hdr_انحراف+8];
  }ورنہ{
    p_دایاں = ڈھونڈو_Cell(p_والد, i+nxDiv-p_والد->n_حد_سے_تجاوز);
  }
  pgno = get4byte(p_دایاں);
  جب( 1 ){
    اگر( rc==SQLITE_OK ){
      rc = get_And_Init_صفحہ(pBt, pgno, &apOld[i], 0);
    }
    اگر( rc ){
      memset(apOld, 0, (i+1)*حجم(Mem_صفحہ*));
      جاؤ balance_cleanup;
    }
    اگر( apOld[i]->nFree<0 ){
      rc = btreeComputeFreeSpace(apOld[i]);
      اگر( rc ){
        memset(apOld, 0, (i)*حجم(Mem_صفحہ*));
        جاؤ balance_cleanup;
      }
    }
    nMaxCells += apOld[i]->nCell + Array_سائز(p_والد->apOvfl);
    اگر( (i--)==0 ) توڑو;

    اگر( p_والد->n_حد_سے_تجاوز && i+nxDiv==p_والد->aiOvfl[0] ){
      apDiv[i] = p_والد->apOvfl[0];
      pgno = get4byte(apDiv[i]);
      szNew[i] = p_والد->x_Cell_سائز(p_والد, apDiv[i]);
      p_والد->n_حد_سے_تجاوز = 0;
    }ورنہ{
      apDiv[i] = ڈھونڈو_Cell(p_والد, i+nxDiv-p_والد->n_حد_سے_تجاوز);
      pgno = get4byte(apDiv[i]);
      szNew[i] = p_والد->x_Cell_سائز(p_والد, apDiv[i]);

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
      اگر( pBt->btsFlags & BTS_FAST_SECURE ){
        صحیح iOff;

        /* If the following if() condition is not true, the db is corrupted.
        ** The call to dropCell() below will detect this.  */
        iOff = SQLITE_PTR_مقصد_INT(apDiv[i]) - SQLITE_PTR_مقصد_INT(p_والد->a_ڈیٹا);
        اگر( (iOff+szNew[i])<=(صحیح)pBt->usable_سائز ){
          حافظہ_نقل(&aOvflSpace[iOff], apDiv[i], szNew[i]);
          apDiv[i] = &aOvflSpace[apDiv[i]-p_والد->a_ڈیٹا];
        }
      }
      dropCell(p_والد, i+nxDiv-p_والد->n_حد_سے_تجاوز, szNew[i], &rc);
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
     + pBt->صفحہ_سائز;                              /* aSpace1 */

  أكد( szScratch<=7*(صحیح)pBt->صفحہ_سائز );
  b.apCell = sqlite3_ڈھیر_Alloc_Raw(0, szScratch );
  اگر( b.apCell==0 ){
    rc = SQLITE_NOMEM_BKPT;
    جاؤ balance_cleanup;
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
  برگ_Correction = b.pRef->برگ*4;
  برگ_ڈیٹا = b.pRef->int_کلید_برگ;
  برائے(i=0; i<nOld; i++){
    Mem_صفحہ *pOld = apOld[i];
    صحیح حد_بندی = pOld->nCell;
    u8 *a_ڈیٹا = pOld->a_ڈیٹا;
    u16 نقاب_صفحہ = pOld->نقاب_صفحہ;
    u8 *piCell = a_ڈیٹا + pOld->cell_انحراف;
    u8 *pi_انتہا;
    u8 **ppCell;
    VVA_ONLY( صحیح n_Cell_At_آغاز = b.nCell; )

    /* Verify that all sibling pages are of the same "type" (table-leaf,
    ** table-interior, index-leaf, or index-interior).
    */
    اگر( pOld->a_ڈیٹا[0]!=apOld[0]->a_ڈیٹا[0] ){
      rc = SQLITE_CORRUPT_صفحہ(pOld);
      جاؤ balance_cleanup;
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
    memset(&b.szCell[b.nCell], 0, حجم(b.szCell[0])*(حد_بندی+pOld->n_حد_سے_تجاوز));
    اگر( pOld->n_حد_سے_تجاوز>0 ){
      اگر( NEVER(حد_بندی<pOld->aiOvfl[0]) ){
        rc = SQLITE_CORRUPT_صفحہ(pOld);
        جاؤ balance_cleanup;
      }
      حد_بندی = pOld->aiOvfl[0];
      برائے(j=0; j<حد_بندی; j++){
        b.apCell[b.nCell+j] = a_ڈیٹا + (نقاب_صفحہ & get2byteAligned(piCell));
        piCell += 2;
      }
      b.nCell += حد_بندی;
      برائے(k=0; k<pOld->n_حد_سے_تجاوز; k++){
        أكد( k==0 || pOld->aiOvfl[k-1]+1==pOld->aiOvfl[k] );/* NOTE 1 */
        b.apCell[b.nCell] = pOld->apOvfl[k];
        b.nCell++;
      }
    }
    pi_انتہا = a_ڈیٹا + pOld->cell_انحراف + 2*pOld->nCell;
    /* Collect cell pointers before updating the cell count. */
    ppCell = &b.apCell[b.nCell];
    جب( piCell<pi_انتہا ){
      أكد( ppCell<&b.apCell[nMaxCells] );
      *ppCell++ = a_ڈیٹا + (نقاب_صفحہ & get2byteAligned(piCell));
      piCell += 2;
    }
    b.nCell = (صحیح)(ppCell - b.apCell);
    أكد( (b.nCell-n_Cell_At_آغاز)==(pOld->nCell+pOld->n_حد_سے_تجاوز) );

    cntOld[i] = b.nCell;
    اگر( i<nOld-1 && !برگ_ڈیٹا){
      u16 sz = (u16)szNew[i];
      u8 *p_عارضی;
      أكد( b.nCell<nMaxCells );
      b.szCell[b.nCell] = sz;
      p_عارضی = &aSpace1[iSpace1];
      iSpace1 += sz;
      أكد( sz<=pBt->maxLocal+23 );
      أكد( iSpace1 <= (صحیح)pBt->صفحہ_سائز );
      حافظہ_نقل(p_عارضی, apDiv[i], sz);
      b.apCell[b.nCell] = p_عارضی+برگ_Correction;
      أكد( برگ_Correction==0 || برگ_Correction==4 );
      b.szCell[b.nCell] = b.szCell[b.nCell] - برگ_Correction;
      اگر( !pOld->برگ ){
        أكد( برگ_Correction==0 );
        أكد( pOld->hdr_انحراف==0 || CORRUPT_DB );
        /* The right pointer of the child page pOld becomes the left
        ** pointer of the divider cell */
        حافظہ_نقل(b.apCell[b.nCell], &pOld->a_ڈیٹا[8], 4);
      }ورنہ{
        أكد( برگ_Correction==4 );
        جب( b.szCell[b.nCell]<4 ){
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
  usableSpace = pBt->usable_سائز - 12 + برگ_Correction;
  برائے(i=k=0; i<nOld; i++, k++){
    Mem_صفحہ *p = apOld[i];
    b.ap_انتہا[k] = p->a_ڈیٹا_انتہا;
    b.ixNx[k] = cntOld[i];
    اگر( k && b.ixNx[k]==b.ixNx[k-1] ){
      k--;  /* Omit b.ixNx[] entry for child pages with no cells */
    }
    اگر( !برگ_ڈیٹا ){
      k++;
      b.ap_انتہا[k] = p_والد->a_ڈیٹا_انتہا;
      b.ixNx[k] = cntOld[i]+1;
    }
    أكد( p->nFree>=0 );
    szNew[i] = usableSpace - p->nFree;
    برائے(j=0; j<p->n_حد_سے_تجاوز; j++){
      szNew[i] += 2 + p->x_Cell_سائز(p, p->apOvfl[j]);
    }
    cntNew[i] = cntOld[i];
  }
  k = nOld;
  برائے(i=0; i<k; i++){
    صحیح sz;
    جب( szNew[i]>usableSpace ){
      اگر( i+1>=k ){
        k = i+2;
        اگر( k>NB+2 ){ rc = SQLITE_CORRUPT_BKPT; جاؤ balance_cleanup; }
        szNew[k-1] = 0;
        cntNew[k-1] = b.nCell;
      }
      sz = 2 + cached_Cell_سائز(&b, cntNew[i]-1);
      szNew[i] -= sz;
      اگر( !برگ_ڈیٹا ){
        اگر( cntNew[i]<b.nCell ){
          sz = 2 + cached_Cell_سائز(&b, cntNew[i]);
        }ورنہ{
          sz = 0;
        }
      }
      szNew[i+1] += sz;
      cntNew[i]--;
    }
    جب( cntNew[i]<b.nCell ){
      sz = 2 + cached_Cell_سائز(&b, cntNew[i]);
      اگر( szNew[i]+sz>usableSpace ) توڑو;
      szNew[i] += sz;
      cntNew[i]++;
      اگر( !برگ_ڈیٹا ){
        اگر( cntNew[i]<b.nCell ){
          sz = 2 + cached_Cell_سائز(&b, cntNew[i]);
        }ورنہ{
          sz = 0;
        }
      }
      szNew[i+1] -= sz;
    }
    اگر( cntNew[i]>=b.nCell ){
      k = i+1;
    }ورنہ اگر( cntNew[i] <= (i>0 ? cntNew[i-1] : 0) ){
      rc = SQLITE_CORRUPT_BKPT;
      جاؤ balance_cleanup;
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
  برائے(i=k-1; i>0; i--){
    صحیح sz_دایاں = szNew[i];  /* Size of sibling on the right */
    صحیح sz_بایاں = szNew[i-1]; /* Size of sibling on the left */
    صحیح r;              /* Index of right-most cell in left sibling */
    صحیح d;              /* Index of first cell to the left of right sibling */

    r = cntNew[i-1] - 1;
    d = r + 1 - برگ_ڈیٹا;
    (خالی)cached_Cell_سائز(&b, d);
    کرو{
      صحیح szR, szD;
      أكد( d<nMaxCells );
      أكد( r<nMaxCells );
      szR = cached_Cell_سائز(&b, r);
      szD = b.szCell[d];
      اگر( sz_دایاں!=0
       && (bBulk || sz_دایاں+szD+2 > sz_بایاں-(szR+(i==k-1?0:2)))){
        توڑو;
      }
      sz_دایاں += szD + 2;
      sz_بایاں -= szR + 2;
      cntNew[i-1] = r;
      r--;
      d--;
    }جب( r>=0 );
    szNew[i] = sz_دایاں;
    szNew[i-1] = sz_بایاں;
    اگر( cntNew[i-1] <= (i>1 ? cntNew[i-2] : 0) ){
      rc = SQLITE_CORRUPT_BKPT;
      جاؤ balance_cleanup;
    }
  }

  /* Sanity check:  For a non-corrupt database file one of the following
  ** must be true:
  **    (1) We found one or more cells (cntNew[0])>0), or
  **    (2) pPage is a virtual root page.  A virtual root page is when
  **        the real root page is page 1 and we are the only child of
  **        that page.
  */
  أكد( cntNew[0]>0 || (p_والد->pgno==1 && p_والد->nCell==0) || CORRUPT_DB);
  TRACE(("BALANCE: old: %u(nc=%u) %u(nc=%u) %u(nc=%u)\n",
    apOld[0]->pgno, apOld[0]->nCell,
    nOld>=2 ? apOld[1]->pgno : 0, nOld>=2 ? apOld[1]->nCell : 0,
    nOld>=3 ? apOld[2]->pgno : 0, nOld>=3 ? apOld[2]->nCell : 0
  ));

  /*
  ** Allocate k new pages.  Reuse old pages where possible.
  */
  صفحہ_Flags = apOld[0]->a_ڈیٹا[0];
  برائے(i=0; i<k; i++){
    Mem_صفحہ *pNew;
    اگر( i<nOld ){
      pNew = apNew[i] = apOld[i];
      apOld[i] = 0;
      rc = sqlite3PagerWrite(pNew->p_Db_صفحہ);
      nNew++;
      اگر( sqlite3_Pager_صفحہ_Refcount(pNew->p_Db_صفحہ)!=1+(i==(i_والد_Idx-nxDiv))
       && rc==SQLITE_OK
      ){
        rc = SQLITE_CORRUPT_BKPT;
      }
      اگر( rc ) جاؤ balance_cleanup;
    }ورنہ{
      أكد( i>0 );
      rc = allocate_Btree_صفحہ(pBt, &pNew, &pgno, (bBulk ? 1 : pgno), 0);
      اگر( rc ) جاؤ balance_cleanup;
      zero_صفحہ(pNew, صفحہ_Flags);
      apNew[i] = pNew;
      nNew++;
      cntOld[i] = b.nCell;

      /* Set the pointer-map entry for the new sibling page. */
      اگر( ISAUTOVACUUM(pBt) ){
        ptrmapPut(pBt, pNew->pgno, PTRMAP_BTREE, p_والد->pgno, &rc);
        اگر( rc!=SQLITE_OK ){
          جاؤ balance_cleanup;
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
  برائے(i=0; i<nNew; i++){
    aPgno[i] = apNew[i]->pgno;
    أكد( apNew[i]->p_Db_صفحہ->flags & PGHDR_WRITEABLE );
    أكد( apNew[i]->p_Db_صفحہ->flags & PGHDR_DIRTY );
  }
  برائے(i=0; i<nNew-1; i++){
    صحیح iB = i;
    برائے(j=i+1; j<nNew; j++){
      اگر( apNew[j]->pgno < apNew[iB]->pgno ) iB = j;
    }

    /* If apNew[i] has a page number that is bigger than any of the
    ** subsequence apNew[i] entries, then swap apNew[i] with the subsequent
    ** entry that has the smallest page number (which we know to be
    ** entry apNew[iB]).
    */
    اگر( iB!=i ){
      Pgno pgnoA = apNew[i]->pgno;
      Pgno pgnoB = apNew[iB]->pgno;
      Pgno pgno_عارضی = (PENDING_BYTE/pBt->صفحہ_سائز)+1;
      u16 fgA = apNew[i]->p_Db_صفحہ->flags;
      u16 fgB = apNew[iB]->p_Db_صفحہ->flags;
      sqlite3PagerRekey(apNew[i]->p_Db_صفحہ, pgno_عارضی, fgB);
      sqlite3PagerRekey(apNew[iB]->p_Db_صفحہ, pgnoA, fgA);
      sqlite3PagerRekey(apNew[i]->p_Db_صفحہ, pgnoB, fgB);
      apNew[i]->pgno = pgnoB;
      apNew[iB]->pgno = pgnoA;
    }
  }

  TRACE(("BALANCE: new: %u(%u nc=%u) %u(%u nc=%u) %u(%u nc=%u) "
         "%u(%u nc=%u) %u(%u nc=%u)\n",
    apNew[0]->pgno, szNew[0], cntNew[0],
    nNew>=2 ? apNew[1]->pgno : 0, nNew>=2 ? szNew[1] : 0,
    nNew>=2 ? cntNew[1] - cntNew[0] - !برگ_ڈیٹا : 0,
    nNew>=3 ? apNew[2]->pgno : 0, nNew>=3 ? szNew[2] : 0,
    nNew>=3 ? cntNew[2] - cntNew[1] - !برگ_ڈیٹا : 0,
    nNew>=4 ? apNew[3]->pgno : 0, nNew>=4 ? szNew[3] : 0,
    nNew>=4 ? cntNew[3] - cntNew[2] - !برگ_ڈیٹا : 0,
    nNew>=5 ? apNew[4]->pgno : 0, nNew>=5 ? szNew[4] : 0,
    nNew>=5 ? cntNew[4] - cntNew[3] - !برگ_ڈیٹا : 0
  ));

  أكد( sqlite3PagerIswriteable(p_والد->p_Db_صفحہ) );
  أكد( nNew>=1 && nNew<=Array_سائز(apNew) );
  أكد( apNew[nNew-1]!=0 );
  put4byte(p_دایاں, apNew[nNew-1]->pgno);

  /* If the sibling pages are not leaves, ensure that the right-child pointer
  ** of the right-most new sibling page is set to the value that was
  ** originally in the same field of the right-most old sibling page. */
  اگر( (صفحہ_Flags & PTF_برگ)==0 && nOld!=nNew ){
    Mem_صفحہ *pOld;
    اگر( nNew>nOld ){
      pOld = apNew[nOld-1];
    }ورنہ{
      pOld = apOld[nOld-1];
    }
    حافظہ_نقل(&apNew[nNew-1]->a_ڈیٹا[8], &pOld->a_ڈیٹا[8], 4);
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
  اگر( ISAUTOVACUUM(pBt) ){
    Mem_صفحہ *pOld;
    Mem_صفحہ *pNew = pOld = apNew[0];
    صحیح cnt_Old_اگلا = pNew->nCell + pNew->n_حد_سے_تجاوز;
    صحیح iNew = 0;
    صحیح iOld = 0;

    برائے(i=0; i<b.nCell; i++){
      u8 *pCell = b.apCell[i];
      جب( i==cnt_Old_اگلا ){
        iOld++;
        أكد( iOld<nNew || iOld<nOld );
        أكد( iOld>=0 && iOld<NB );
        pOld = iOld<nNew ? apNew[iOld] : apOld[iOld];
        cnt_Old_اگلا += pOld->nCell + pOld->n_حد_سے_تجاوز + !برگ_ڈیٹا;
      }
      اگر( i==cntNew[iNew] ){
        pNew = apNew[++iNew];
        اگر( !برگ_ڈیٹا ) جاری;
      }

      /* Cell pCell is destined for new sibling page pNew. Originally, it
      ** was either part of sibling page iOld (possibly an overflow cell),
      ** or else the divider cell to the left of sibling page iOld. So,
      ** if sibling page iOld had the same page number as pNew, and if
      ** pCell really was a part of sibling page iOld (not a divider or
      ** overflow cell), we can skip updating the pointer map entries.  */
      اگر( iOld>=nNew
       || pNew->pgno!=aPgno[iOld]
       || !SQLITE_WITHIN(pCell,pOld->a_ڈیٹا,pOld->a_ڈیٹا_انتہا)
      ){
        اگر( !برگ_Correction ){
          ptrmapPut(pBt, get4byte(pCell), PTRMAP_BTREE, pNew->pgno, &rc);
        }
        اگر( cached_Cell_سائز(&b,i)>pNew->minLocal ){
          ptrmapPutOvflPtr(pNew, pOld, pCell, &rc);
        }
        اگر( rc ) جاؤ balance_cleanup;
      }
    }
  }

  /* Insert new divider cells into pParent. */
  برائے(i=0; i<nNew-1; i++){
    u8 *pCell;
    u8 *p_عارضی;
    صحیح sz;
    u8 *p_Src_انتہا;
    Mem_صفحہ *pNew = apNew[i];
    j = cntNew[i];

    أكد( j<nMaxCells );
    أكد( b.apCell[j]!=0 );
    pCell = b.apCell[j];
    sz = b.szCell[j] + برگ_Correction;
    p_عارضی = &aOvflSpace[iOvflSpace];
    اگر( !pNew->برگ ){
      حافظہ_نقل(&pNew->a_ڈیٹا[8], pCell, 4);
    }ورنہ اگر( برگ_ڈیٹا ){
      /* If the tree is a leaf-data tree, and the siblings are leaves,
      ** then there is no divider cell in b.apCell[]. Instead, the divider
      ** cell consists of the integer key for the right-most cell of
      ** the sibling-page assembled above only.
      */
      CellInfo info;
      j--;
      pNew->xParseCell(pNew, b.apCell[j], &info);
      pCell = p_عارضی;
      sz = 4 + sqlite3PutVarint(&pCell[4], info.n_کلید);
      p_عارضی = 0;
    }ورنہ{
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
      اگر( b.szCell[j]==4 ){
        أكد(برگ_Correction==4);
        sz = p_والد->x_Cell_سائز(p_والد, pCell);
      }
    }
    iOvflSpace += sz;
    أكد( sz<=pBt->maxLocal+23 );
    أكد( iOvflSpace <= (صحیح)pBt->صفحہ_سائز );
    أكد( b.ixNx[NB*2-1]>j );
    برائے(k=0; b.ixNx[k]<=j; k++){}
    p_Src_انتہا = b.ap_انتہا[k];
    اگر( SQLITE_حد_سے_تجاوز(p_Src_انتہا, pCell, pCell+sz) ){
      rc = SQLITE_CORRUPT_BKPT;
      جاؤ balance_cleanup;
    }
    rc = داخل_کرو_Cell(p_والد, nxDiv+i, pCell, sz, p_عارضی, pNew->pgno);
    اگر( rc!=SQLITE_OK ) جاؤ balance_cleanup;
    أكد( sqlite3PagerIswriteable(p_والد->p_Db_صفحہ) );
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
  برائے(i=1-nNew; i<nNew; i++){
    صحیح iPg = i<0 ? -i : i;
    أكد( iPg>=0 && iPg<nNew );
    أكد( iPg>=1 || i>=0 );
    أكد( iPg<Array_سائز(cntOld) );
    اگر( ab_مکمل[iPg] ) جاری;         /* Skip pages already processed */
    اگر( i>=0                            /* On the upwards pass, or... */
     || cntOld[iPg-1]>=cntNew[iPg-1]    /* Condition (1) is true */
    ){
      صحیح iNew;
      صحیح iOld;
      صحیح nNewCell;

      /* Verify condition (1):  If cells are moving left, update iPg
      ** only after iPg-1 has already been updated. */
      أكد( iPg==0 || cntOld[iPg-1]>=cntNew[iPg-1] || ab_مکمل[iPg-1] );

      /* Verify condition (2):  If cells are moving right, update iPg
      ** only after iPg+1 has already been updated. */
      أكد( cntNew[iPg]>=cntOld[iPg] || ab_مکمل[iPg+1] );

      اگر( iPg==0 ){
        iNew = iOld = 0;
        nNewCell = cntNew[0];
      }ورنہ{
        iOld = iPg<nOld ? (cntOld[iPg-1] + !برگ_ڈیٹا) : b.nCell;
        iNew = cntNew[iPg-1] + !برگ_ڈیٹا;
        nNewCell = cntNew[iPg] - iNew;
      }

      rc = edit_صفحہ(apNew[iPg], iOld, iNew, nNewCell, &b);
      اگر( rc ) جاؤ balance_cleanup;
      ab_مکمل[iPg]++;
      apNew[iPg]->nFree = usableSpace-szNew[iPg];
      أكد( apNew[iPg]->n_حد_سے_تجاوز==0 );
      أكد( apNew[iPg]->nCell==nNewCell );
    }
  }

  /* All pages have been processed exactly once */
  أكد( memcmp(ab_مکمل, "\01\01\01\01\01", nNew)==0 );

  أكد( nOld>0 );
  أكد( nNew>0 );

  اگر( is_جڑ && p_والد->nCell==0 && p_والد->hdr_انحراف<=apNew[0]->nFree ){
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
    rc = defragment_صفحہ(apNew[0], -1);
    testcase( rc!=SQLITE_OK );
    أكد( apNew[0]->nFree ==
        (get2byteNotZero(&apNew[0]->a_ڈیٹا[5]) - apNew[0]->cell_انحراف
          - apNew[0]->nCell*2)
      || rc!=SQLITE_OK
    );
    copy_گرہ_Content(apNew[0], p_والد, &rc);
    free_صفحہ(apNew[0], &rc);
  }ورنہ اگر( ISAUTOVACUUM(pBt) && !برگ_Correction ){
    /* Fix the pointer map entries associated with the right-child of each
    ** sibling page. All other pointer map entries have already been taken
    ** care of.  */
    برائے(i=0; i<nNew; i++){
      u32 کلید = get4byte(&apNew[i]->a_ڈیٹا[8]);
      ptrmapPut(pBt, کلید, PTRMAP_BTREE, apNew[i]->pgno, &rc);
    }
  }

  أكد( p_والد->isInit );
  TRACE(("BALANCE: finished: old=%u new=%u cells=%u\n",
          nOld, nNew, b.nCell));

  /* Free any old pages that were not reused as new pages.
  */
  برائے(i=nNew; i<nOld; i++){
    free_صفحہ(apOld[i], &rc);
  }

#اگر 0
  اگر( ISAUTOVACUUM(pBt) && rc==SQLITE_OK && apNew[0]->isInit ){
    /* The ptrmapCheckPages() contains assert() statements that verify that
    ** all pointer map pages are set correctly. This is helpful while
    ** debugging. This is usually disabled because a corrupt database may
    ** cause an assert() statement to fail.  */
    ptrmap_Check_صفحات(apNew, nNew);
    ptrmap_Check_صفحات(&p_والد, 1);
  }
#اگر_ختم

  /*
  ** Cleanup before returning.
  */
balance_cleanup:
  sqlite3_ڈھیر_Free(0, b.apCell);
  برائے(i=0; i<nOld; i++){
    release_صفحہ(apOld[i]);
  }
  برائے(i=0; i<nNew; i++){
    release_صفحہ(apNew[i]);
  }

  واپس rc;
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
ساکن صحیح balance_deeper(Mem_صفحہ *p_جڑ, Mem_صفحہ **pp_بچہ){
  صحیح rc;                        /* Return value from subprocedures */
  Mem_صفحہ *p_بچہ = 0;           /* Pointer to a new child page */
  Pgno pgno_بچہ = 0;            /* Page number of the new child page */
  BtShared *pBt = p_جڑ->pBt;    /* The BTree */

  أكد( p_جڑ->n_حد_سے_تجاوز>0 );
  أكد( sqlite3_mutex_held(pBt->mutex) );

  /* Make pRoot, the root page of the b-tree, writable. Allocate a new
  ** page that will become the new right-child of pPage. Copy the contents
  ** of the node stored on pRoot into the new child page.
  */
  rc = sqlite3PagerWrite(p_جڑ->p_Db_صفحہ);
  اگر( rc==SQLITE_OK ){
    rc = allocate_Btree_صفحہ(pBt,&p_بچہ,&pgno_بچہ,p_جڑ->pgno,0);
    copy_گرہ_Content(p_جڑ, p_بچہ, &rc);
    اگر( ISAUTOVACUUM(pBt) ){
      ptrmapPut(pBt, pgno_بچہ, PTRMAP_BTREE, p_جڑ->pgno, &rc);
    }
  }
  اگر( rc ){
    *pp_بچہ = 0;
    release_صفحہ(p_بچہ);
    واپس rc;
  }
  أكد( sqlite3PagerIswriteable(p_بچہ->p_Db_صفحہ) );
  أكد( sqlite3PagerIswriteable(p_جڑ->p_Db_صفحہ) );
  أكد( p_بچہ->nCell==p_جڑ->nCell || CORRUPT_DB );

  TRACE(("BALANCE: copy root %u into %u\n", p_جڑ->pgno, p_بچہ->pgno));

  /* Copy the overflow cells from pRoot to pChild */
  حافظہ_نقل(p_بچہ->aiOvfl, p_جڑ->aiOvfl,
         p_جڑ->n_حد_سے_تجاوز*حجم(p_جڑ->aiOvfl[0]));
  حافظہ_نقل(p_بچہ->apOvfl, p_جڑ->apOvfl,
         p_جڑ->n_حد_سے_تجاوز*حجم(p_جڑ->apOvfl[0]));
  p_بچہ->n_حد_سے_تجاوز = p_جڑ->n_حد_سے_تجاوز;

  /* Zero the contents of pRoot. Then install pChild as the right-child. */
  zero_صفحہ(p_جڑ, p_بچہ->a_ڈیٹا[0] & ~PTF_برگ);
  put4byte(&p_جڑ->a_ڈیٹا[p_جڑ->hdr_انحراف+8], pgno_بچہ);

  *pp_بچہ = p_بچہ;
  واپس SQLITE_OK;
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
ساکن صحیح anotherValidCursor(BtCursor *pCur){
  BtCursor *pOther;
  برائے(pOther=pCur->pBt->pCursor; pOther; pOther=pOther->p_اگلا){
    اگر( pOther!=pCur
     && pOther->eState==CURSOR_VALID
     && pOther->p_صفحہ==pCur->p_صفحہ
    ){
      واپس SQLITE_CORRUPT_صفحہ(pCur->p_صفحہ);
    }
  }
  واپس SQLITE_OK;
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
ساکن صحیح balance(BtCursor *pCur){
  صحیح rc = SQLITE_OK;
  u8 a_Balance_تیز_Space[13];
  u8 *pFree = 0;

  VVA_ONLY( صحیح balance_تیز_called = 0 );
  VVA_ONLY( صحیح balance_deeper_called = 0 );

  کرو {
    صحیح i_صفحہ;
    Mem_صفحہ *p_صفحہ = pCur->p_صفحہ;

    اگر( NEVER(p_صفحہ->nFree<0) && btreeComputeFreeSpace(p_صفحہ) ) توڑو;
    اگر( p_صفحہ->n_حد_سے_تجاوز==0 && p_صفحہ->nFree*3<=(صحیح)pCur->pBt->usable_سائز*2 ){
      /* No rebalance required as long as:
      **   (1) There are no overflow cells
      **   (2) The amount of free space on the page is less than 2/3rds of
      **       the total usable space on the page. */
      توڑو;
    }ورنہ اگر( (i_صفحہ = pCur->i_صفحہ)==0 ){
      اگر( p_صفحہ->n_حد_سے_تجاوز && (rc = anotherValidCursor(pCur))==SQLITE_OK ){
        /* The root page of the b-tree is overfull. In this case call the
        ** balance_deeper() function to create a new child for the root-page
        ** and copy the current contents of the root-page to it. The
        ** next iteration of the do-loop will balance the child page.
        */
        أكد( balance_deeper_called==0 );
        VVA_ONLY( balance_deeper_called++ );
        rc = balance_deeper(p_صفحہ, &pCur->ap_صفحہ[1]);
        اگر( rc==SQLITE_OK ){
          pCur->i_صفحہ = 1;
          pCur->ix = 0;
          pCur->aiIdx[0] = 0;
          pCur->ap_صفحہ[0] = p_صفحہ;
          pCur->p_صفحہ = pCur->ap_صفحہ[1];
          أكد( pCur->p_صفحہ->n_حد_سے_تجاوز );
        }
      }ورنہ{
        توڑو;
      }
    }ورنہ اگر( sqlite3_Pager_صفحہ_Refcount(p_صفحہ->p_Db_صفحہ)>1 ){
      /* The page being written is not a root page, and there is currently
      ** more than one reference to it. This only happens if the page is one
      ** of its own ancestor pages. Corruption. */
      rc = SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }ورنہ{
      Mem_صفحہ * ثابت p_والد = pCur->ap_صفحہ[i_صفحہ-1];
      صحیح ثابت iIdx = pCur->aiIdx[i_صفحہ-1];

      rc = sqlite3PagerWrite(p_والد->p_Db_صفحہ);
      اگر( rc==SQLITE_OK && p_والد->nFree<0 ){
        rc = btreeComputeFreeSpace(p_والد);
      }
      اگر( rc==SQLITE_OK ){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_QUICKBALANCE
        اگر( p_صفحہ->int_کلید_برگ
         && p_صفحہ->n_حد_سے_تجاوز==1
         && p_صفحہ->aiOvfl[0]==p_صفحہ->nCell
         && p_والد->pgno!=1
         && p_والد->nCell==iIdx
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
          أكد( balance_تیز_called==0 );
          VVA_ONLY( balance_تیز_called++ );
          rc = balance_تیز(p_والد, p_صفحہ, a_Balance_تیز_Space);
        }ورنہ
#اگر_ختم
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
          u8 *pSpace = sqlite3_صفحہ_Malloc(pCur->pBt->صفحہ_سائز);
          rc = balance_nonroot(p_والد, iIdx, pSpace, i_صفحہ==1,
                               pCur->hints&BTREE_BULKLOAD);
          اگر( pFree ){
            /* If pFree is not NULL, it points to the pSpace buffer used
            ** by a previous call to balance_nonroot(). Its contents are
            ** now stored either on real database pages or within the
            ** new pSpace buffer, so it may be safely freed here. */
            sqlite3_صفحہ_Free(pFree);
          }

          /* The pSpace buffer will be freed after the next call to
          ** balance_nonroot(), or just before this function returns, whichever
          ** comes first. */
          pFree = pSpace;
        }
      }

      p_صفحہ->n_حد_سے_تجاوز = 0;

      /* The next iteration of the do-loop balances the parent page. */
      release_صفحہ(p_صفحہ);
      pCur->i_صفحہ--;
      أكد( pCur->i_صفحہ>=0 );
      pCur->p_صفحہ = pCur->ap_صفحہ[pCur->i_صفحہ];
    }
  }جب( rc==SQLITE_OK );

  اگر( pFree ){
    sqlite3_صفحہ_Free(pFree);
  }
  واپس rc;
}

/* Overwrite content from pX into pDest.  Only do the write if the
** content is different from what is already there.
*/
ساکن صحیح btreeOverwriteContent(
  Mem_صفحہ *p_صفحہ,           /* MemPage on which writing will occur */
  u8 *pDest,                /* Pointer to the place to start writing */
  ثابت BtreePayload *pX,   /* Source of data to write */
  صحیح i_انحراف,              /* Offset of first byte to write */
  صحیح iAmt                  /* Number of bytes to be written */
){
  صحیح n_ڈیٹا = pX->n_ڈیٹا - i_انحراف;
  اگر( n_ڈیٹا<=0 ){
    /* Overwriting with zeros */
    صحیح i;
    برائے(i=0; i<iAmt && pDest[i]==0; i++){}
    اگر( i<iAmt ){
      صحیح rc = sqlite3PagerWrite(p_صفحہ->p_Db_صفحہ);
      اگر( rc ) واپس rc;
      memset(pDest + i, 0, iAmt - i);
    }
  }ورنہ{
    اگر( n_ڈیٹا<iAmt ){
      /* Mixed read data and zeros at the end.  Make a recursive call
      ** to write the zeros then fall through to write the real data */
      صحیح rc = btreeOverwriteContent(p_صفحہ, pDest+n_ڈیٹا, pX, i_انحراف+n_ڈیٹا,
                                 iAmt-n_ڈیٹا);
      اگر( rc ) واپس rc;
      iAmt = n_ڈیٹا;
    }
    اگر( memcmp(pDest, ((u8*)pX->p_ڈیٹا) + i_انحراف, iAmt)!=0 ){
      صحیح rc = sqlite3PagerWrite(p_صفحہ->p_Db_صفحہ);
      اگر( rc ) واپس rc;
      /* In a corrupt database, it is possible for the source and destination
      ** buffers to overlap.  This is harmless since the database is already
      ** corrupt but it does cause valgrind and ASAN warnings.  So use
      ** memmove(). */
      memmove(pDest, ((u8*)pX->p_ڈیٹا) + i_انحراف, iAmt);
    }
  }
  واپس SQLITE_OK;
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.  In this variant, pCur is pointing to an overflow
** cell.
*/
ساکن SQLITE_NOINLINE صحیح btree_Overwrite_حد_سے_تجاوز_Cell(
  BtCursor *pCur,                     /* Cursor pointing to cell to overwrite */
  ثابت BtreePayload *pX              /* Content to write into the cell */
){
  صحیح i_انحراف;                        /* Next byte of pX->pData to write */
  صحیح n_کل = pX->n_ڈیٹا + pX->nZero; /* Total bytes of to write */
  صحیح rc;                             /* Return code */
  Mem_صفحہ *p_صفحہ = pCur->p_صفحہ;       /* Page being written */
  BtShared *pBt;                      /* Btree */
  Pgno ovflPgno;                      /* Next overflow page to write */
  u32 ovfl_صفحہ_سائز;                   /* Size to write on overflow page */

  أكد( pCur->info.nLocal<n_کل );  /* pCur is an overflow cell */

  /* Overwrite the local portion first */
  rc = btreeOverwriteContent(p_صفحہ, pCur->info.pPayload, pX,
                             0, pCur->info.nLocal);
  اگر( rc ) واپس rc;

  /* Now overwrite the overflow pages */
  i_انحراف = pCur->info.nLocal;
  أكد( n_کل>=0 );
  أكد( i_انحراف>=0 );
  ovflPgno = get4byte(pCur->info.pPayload + i_انحراف);
  pBt = p_صفحہ->pBt;
  ovfl_صفحہ_سائز = pBt->usable_سائز - 4;
  کرو{
    rc = btree_Get_صفحہ(pBt, ovflPgno, &p_صفحہ, 0);
    اگر( rc ) واپس rc;
    اگر( sqlite3_Pager_صفحہ_Refcount(p_صفحہ->p_Db_صفحہ)!=1 || p_صفحہ->isInit ){
      rc = SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }ورنہ{
      اگر( i_انحراف+ovfl_صفحہ_سائز<(u32)n_کل ){
        ovflPgno = get4byte(p_صفحہ->a_ڈیٹا);
      }ورنہ{
        ovfl_صفحہ_سائز = n_کل - i_انحراف;
      }
      rc = btreeOverwriteContent(p_صفحہ, p_صفحہ->a_ڈیٹا+4, pX,
                                 i_انحراف, ovfl_صفحہ_سائز);
    }
    sqlite3PagerUnref(p_صفحہ->p_Db_صفحہ);
    اگر( rc ) واپس rc;
    i_انحراف += ovfl_صفحہ_سائز;
  }جب( i_انحراف<n_کل );
  واپس SQLITE_OK;   
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.
*/
ساکن صحیح btreeOverwriteCell(BtCursor *pCur, ثابت BtreePayload *pX){
  صحیح n_کل = pX->n_ڈیٹا + pX->nZero; /* Total bytes of to write */
  Mem_صفحہ *p_صفحہ = pCur->p_صفحہ;       /* Page being written */

  اگر( pCur->info.pPayload + pCur->info.nLocal > p_صفحہ->a_ڈیٹا_انتہا
   || pCur->info.pPayload < p_صفحہ->a_ڈیٹا + p_صفحہ->cell_انحراف
  ){
    واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
  }
  اگر( pCur->info.nLocal==n_کل ){
    /* The entire cell is local */
    واپس btreeOverwriteContent(p_صفحہ, pCur->info.pPayload, pX,
                                 0, pCur->info.nLocal);
  }ورنہ{
    /* The cell contains overflow content */
    واپس btree_Overwrite_حد_سے_تجاوز_Cell(pCur, pX);
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
صحیح sqlite3_Btree_داخل_کرو(
  BtCursor *pCur,                /* Insert data into the table of this cursor */
  ثابت BtreePayload *pX,        /* Content of the row to be inserted */
  صحیح flags,                     /* True if this is likely an append */
  صحیح seek_نتیجہ                 /* Result of prior IndexMoveto() call */
){
  صحیح rc;
  صحیح loc = seek_نتیجہ;          /* -1: before desired location  +1: after */
  صحیح szNew = 0;
  صحیح idx;
  Mem_صفحہ *p_صفحہ;
  Btree *p = pCur->pBtree;
  غیرعلامتی حرف *oldCell;
  غیرعلامتی حرف *newCell = 0;

  أكد( (flags & (BTREE_SAVEPOSITION|BTREE_APPEND|BTREE_PREFORMAT))==flags );
  أكد( (flags & BTREE_PREFORMAT)==0 || seek_نتیجہ || pCur->p_کلید_Info==0 );

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
  اگر( pCur->curFlags & BTCF_مضرب ){
    rc = saveAllCursors(p->pBt, pCur->pgno_جڑ, pCur);
    اگر( rc ) واپس rc;
    اگر( loc && pCur->i_صفحہ<0 ){
      /* This can only happen if the schema is corrupt such that there is more
      ** than one table or index with the same root page as used by the cursor.
      ** Which can only happen if the SQLITE_NoSchemaError flag was set when
      ** the schema was loaded. This cannot be asserted though, as a user might
      ** set the flag, load the schema, and then unset the flag.  */
      واپس SQLITE_CORRUPT_PGNO(pCur->pgno_جڑ);
    }
  }

  /* Ensure that the cursor is not in the CURSOR_FAULT state and that it
  ** points to a valid cell.
  */
  اگر( pCur->eState>=CURSOR_REQUIRESEEK ){
    testcase( pCur->eState==CURSOR_REQUIRESEEK );
    testcase( pCur->eState==CURSOR_FAULT );
    rc = move_مقصد_جڑ(pCur);
    اگر( rc && rc!=SQLITE_خالی_خانے ) واپس rc;
  }

  أكد( cursorOwnsBtShared(pCur) );
  أكد( (pCur->curFlags & BTCF_WriteFlag)!=0
              && p->pBt->اندر_Transaction==TRANS_WRITE
              && (p->pBt->btsFlags & BTS_READ_ONLY)==0 );
  أكد( has_Shared_Cache_جدول_تالا(p, pCur->pgno_جڑ, pCur->p_کلید_Info!=0, 2) );

  /* Assert that the caller has been consistent. If this cursor was opened
  ** expecting an index b-tree, then the caller should be inserting blob
  ** keys with no associated data. If the cursor was opened expecting an
  ** intkey table, the caller should be inserting integer keys with a
  ** blob of associated data.  */
  أكد( (flags & BTREE_PREFORMAT) || (pX->p_کلید==0)==(pCur->p_کلید_Info==0) );

  اگر( pCur->p_کلید_Info==0 ){
    أكد( pX->p_کلید==0 );
    /* If this is an insert into a table b-tree, invalidate any incrblob
    ** cursors open on the row being replaced */
    اگر( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, pCur->pgno_جڑ, pX->n_کلید, 0);
    }

    /* If BTREE_SAVEPOSITION is set, the cursor must already be pointing
    ** to a row with the same key as the new entry being inserted.
    */
#اگر_تعریف_شدہ SQLITE_DEBUG
    اگر( flags & BTREE_SAVEPOSITION ){
      أكد( pCur->curFlags & BTCF_ValidNKey );
      أكد( pX->n_کلید==pCur->info.n_کلید );
      أكد( loc==0 );
    }
#اگر_ختم

    /* On the other hand, BTREE_SAVEPOSITION==0 does not imply
    ** that the cursor is not pointing to a row to be overwritten.
    ** So do a complete check.
    */
    اگر( (pCur->curFlags&BTCF_ValidNKey)!=0 && pX->n_کلید==pCur->info.n_کلید ){
      /* The cursor is pointing to the entry that is to be
      ** overwritten */
      أكد( pX->n_ڈیٹا>=0 && pX->nZero>=0 );
      اگر( pCur->info.n_سائز!=0
       && pCur->info.nPayload==(u32)pX->n_ڈیٹا+pX->nZero
      ){
        /* New entry is the same size as the old.  Do an overwrite */
        واپس btreeOverwriteCell(pCur, pX);
      }
      أكد( loc==0 );
    }ورنہ اگر( loc==0 ){
      /* The cursor is *not* pointing to the cell to be overwritten, nor
      ** to an adjacent cell.  Move the cursor so that it is pointing either
      ** to the cell to be overwritten or an adjacent cell.
      */
      rc = sqlite3_Btree_جدول_Moveto(pCur, pX->n_کلید,
               (flags & BTREE_APPEND)!=0, &loc);
      اگر( rc ) واپس rc;
    }
  }ورنہ{
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
    اگر( loc==0 && (flags & BTREE_SAVEPOSITION)==0 ){
      اگر( pX->nMem ){
        Unpacked_ریکارڈ r;
        r.p_کلید_Info = pCur->p_کلید_Info;
        r.aMem = pX->aMem;
        r.n_میدان = pX->nMem;
        r.default_rc = 0;
        r.eq_نظر_آیا = 0;
        rc = sqlite3_Btree_اشاریہ_Moveto(pCur, &r, &loc);
      }ورنہ{
        rc = btreeMoveto(pCur, pX->p_کلید, pX->n_کلید,
                    (flags & BTREE_APPEND)!=0, &loc);
      }
      اگر( rc ) واپس rc;
    }

    /* If the cursor is currently pointing to an entry to be overwritten
    ** and the new content is the same as as the old, then use the
    ** overwrite optimization.
    */
    اگر( loc==0 ){
      getCellInfo(pCur);
      اگر( pCur->info.n_کلید==pX->n_کلید ){
        BtreePayload x2;
        x2.p_ڈیٹا = pX->p_کلید;
        x2.n_ڈیٹا = (صحیح)pX->n_کلید;  أكد( pX->n_کلید<=0x7fffffff );
        x2.nZero = 0;
        واپس btreeOverwriteCell(pCur, &x2);
      }
    }
  }
  أكد( pCur->eState==CURSOR_VALID
       || (pCur->eState==CURSOR_INVALID && loc) || CORRUPT_DB );

  p_صفحہ = pCur->p_صفحہ;
  أكد( p_صفحہ->int_کلید || pX->n_کلید>=0 || (flags & BTREE_PREFORMAT) );
  أكد( p_صفحہ->برگ || !p_صفحہ->int_کلید );
  اگر( p_صفحہ->nFree<0 ){
    اگر( NEVER(pCur->eState>CURSOR_INVALID) ){
     /* ^^^^^--- due to the moveToRoot() call above */
      rc = SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }ورنہ{
      rc = btreeComputeFreeSpace(p_صفحہ);
    }
    اگر( rc ) واپس rc;
  }

  TRACE(("INSERT: table=%u nkey=%lld ndata=%u page=%u %s\n",
          pCur->pgno_جڑ, pX->n_کلید, pX->n_ڈیٹا, p_صفحہ->pgno,
          loc==0 ? "overwrite" : "new entry"));
  أكد( p_صفحہ->isInit || CORRUPT_DB );
  newCell = p->pBt->pTmpSpace;
  أكد( newCell!=0 );
  أكد( BTREE_PREFORMAT==OPFLAG_PREFORMAT );
  اگر( flags & BTREE_PREFORMAT ){
    rc = SQLITE_OK;
    szNew = p->pBt->n_Preformat_سائز;
    اگر( szNew<4 ){
      szNew = 4;
      newCell[3] = 0;
    }
    اگر( ISAUTOVACUUM(p->pBt) && szNew>p_صفحہ->maxLocal ){
      CellInfo info;
      p_صفحہ->xParseCell(p_صفحہ, newCell, &info);
      اگر( info.nPayload!=info.nLocal ){
        Pgno ovfl = get4byte(&newCell[szNew-4]);
        ptrmapPut(p->pBt, ovfl, PTRMAP_OVERFLOW1, p_صفحہ->pgno, &rc);
        اگر( NEVER(rc) ) جاؤ انتہا_داخل_کرو;
      }
    }
  }ورنہ{
    rc = fill_اندر_Cell(p_صفحہ, newCell, pX, &szNew);
    اگر( rc ) جاؤ انتہا_داخل_کرو;
  }
  أكد( szNew==p_صفحہ->x_Cell_سائز(p_صفحہ, newCell) );
  أكد( szNew <= MX_CELL_سائز(p->pBt) );
  idx = pCur->ix;
  pCur->info.n_سائز = 0;
  اگر( loc==0 ){
    CellInfo info;
    أكد( idx>=0 );
    اگر( idx>=p_صفحہ->nCell ){
      واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
    }
    rc = sqlite3PagerWrite(p_صفحہ->p_Db_صفحہ);
    اگر( rc ){
      جاؤ انتہا_داخل_کرو;
    }
    oldCell = ڈھونڈو_Cell(p_صفحہ, idx);
    اگر( !p_صفحہ->برگ ){
      حافظہ_نقل(newCell, oldCell, 4);
    }
    BTREE_CLEAR_CELL(rc, p_صفحہ, oldCell, info);
    testcase( pCur->curFlags & BTCF_ValidOvfl );
    invalidate_حد_سے_تجاوز_Cache(pCur);
    اگر( info.n_سائز==szNew && info.nLocal==info.nPayload
     && (!ISAUTOVACUUM(p->pBt) || szNew<p_صفحہ->minLocal)
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
      اگر( oldCell < p_صفحہ->a_ڈیٹا+p_صفحہ->hdr_انحراف+10 ){
        واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
      }
      اگر( oldCell+szNew > p_صفحہ->a_ڈیٹا_انتہا ){
        واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
      }
      حافظہ_نقل(oldCell, newCell, szNew);
      واپس SQLITE_OK;
    }
    dropCell(p_صفحہ, idx, info.n_سائز, &rc);
    اگر( rc ) جاؤ انتہا_داخل_کرو;
  }ورنہ اگر( loc<0 && p_صفحہ->nCell>0 ){
    أكد( p_صفحہ->برگ );
    idx = ++pCur->ix;
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  }ورنہ{
    أكد( p_صفحہ->برگ );
  }
  rc = داخل_کرو_Cell_Fast(p_صفحہ, idx, newCell, szNew);
  أكد( p_صفحہ->n_حد_سے_تجاوز==0 || rc==SQLITE_OK );
  أكد( rc!=SQLITE_OK || p_صفحہ->nCell>0 || p_صفحہ->n_حد_سے_تجاوز>0 );

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
  اگر( p_صفحہ->n_حد_سے_تجاوز ){
    أكد( rc==SQLITE_OK );
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
    rc = balance(pCur);

    /* Must make sure nOverflow is reset to zero even if the balance()
    ** fails. Internal data structure corruption will result otherwise.
    ** Also, set the cursor state to invalid. This stops saveCursorPosition()
    ** from trying to save the current position of the cursor.  */
    pCur->p_صفحہ->n_حد_سے_تجاوز = 0;
    pCur->eState = CURSOR_INVALID;
    اگر( (flags & BTREE_SAVEPOSITION) && rc==SQLITE_OK ){
      btree_Release_All_Cursor_صفحات(pCur);
      اگر( pCur->p_کلید_Info ){
        أكد( pCur->p_کلید==0 );
        pCur->p_کلید = sqlite3Malloc( pX->n_کلید );
        اگر( pCur->p_کلید==0 ){
          rc = SQLITE_NOMEM;
        }ورنہ{
          حافظہ_نقل(pCur->p_کلید, pX->p_کلید, pX->n_کلید);
        }
      }
      pCur->eState = CURSOR_REQUIRESEEK;
      pCur->n_کلید = pX->n_کلید;
    }
  }
  أكد( pCur->i_صفحہ<0 || pCur->p_صفحہ->n_حد_سے_تجاوز==0 );

انتہا_داخل_کرو:
  واپس rc;
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
صحیح sqlite3_Btree_Transfer_صف(BtCursor *pDest, BtCursor *pSrc, i64 i_کلید){
  BtShared *pBt = pDest->pBt;
  u8 *a_برآمد = pBt->pTmpSpace;    /* Pointer to next output buffer */
  ثابت u8 *a_اندر;                /* Pointer to next input buffer */
  u32 n_اندر;                      /* Size of input buffer aIn[] */
  u32 nRem;                     /* Bytes of data still to copy */

  getCellInfo(pSrc);
  اگر( pSrc->info.nPayload<0x80 ){
    *(a_برآمد++) = (u8)pSrc->info.nPayload;
  }ورنہ{
    a_برآمد += sqlite3PutVarint(a_برآمد, pSrc->info.nPayload);
  }
  اگر( pDest->p_کلید_Info==0 ) a_برآمد += sqlite3PutVarint(a_برآمد, i_کلید);
  n_اندر = pSrc->info.nLocal;
  a_اندر = pSrc->info.pPayload;
  اگر( a_اندر+n_اندر>pSrc->p_صفحہ->a_ڈیٹا_انتہا ){
    واپس SQLITE_CORRUPT_صفحہ(pSrc->p_صفحہ);
  }
  nRem = pSrc->info.nPayload;
  اگر( n_اندر==nRem && n_اندر<pDest->p_صفحہ->maxLocal ){
    حافظہ_نقل(a_برآمد, a_اندر, n_اندر);
    pBt->n_Preformat_سائز = n_اندر + (صحیح)(a_برآمد - pBt->pTmpSpace);
    واپس SQLITE_OK;
  }ورنہ{
    صحیح rc = SQLITE_OK;
    Pager *pSrcPager = pSrc->pBt->pPager;
    u8 *p_Pgno_برآمد = 0;
    Pgno ovfl_اندر = 0;
    Db_صفحہ *p_صفحہ_اندر = 0;
    Mem_صفحہ *p_صفحہ_برآمد = 0;
    u32 n_برآمد;                     /* Size of output buffer aOut[] */

    n_برآمد = btree_Payload_مقصد_Local(pDest->p_صفحہ, pSrc->info.nPayload);
    pBt->n_Preformat_سائز = (صحیح)n_برآمد + (صحیح)(a_برآمد - pBt->pTmpSpace);
    اگر( n_برآمد<pSrc->info.nPayload ){
      p_Pgno_برآمد = &a_برآمد[n_برآمد];
      pBt->n_Preformat_سائز += 4;
    }
 
    اگر( nRem>n_اندر ){
      اگر( a_اندر+n_اندر+4>pSrc->p_صفحہ->a_ڈیٹا_انتہا ){
        واپس SQLITE_CORRUPT_صفحہ(pSrc->p_صفحہ);
      }
      ovfl_اندر = get4byte(&pSrc->info.pPayload[n_اندر]);
    }
 
    کرو {
      nRem -= n_برآمد;
      کرو{
        أكد( n_برآمد>0 );
        اگر( n_اندر>0 ){
          صحیح nCopy = MIN(n_برآمد, n_اندر);
          حافظہ_نقل(a_برآمد, a_اندر, nCopy);
          n_برآمد -= nCopy;
          n_اندر -= nCopy;
          a_برآمد += nCopy;
          a_اندر += nCopy;
        }
        اگر( n_برآمد>0 ){
          sqlite3PagerUnref(p_صفحہ_اندر);
          p_صفحہ_اندر = 0;
          rc = sqlite3PagerGet(pSrcPager, ovfl_اندر, &p_صفحہ_اندر, PAGER_GET_READONLY);
          اگر( rc==SQLITE_OK ){
            a_اندر = (ثابت u8*)sqlite3_Pager_Get_ڈیٹا(p_صفحہ_اندر);
            ovfl_اندر = get4byte(a_اندر);
            a_اندر += 4;
            n_اندر = pSrc->pBt->usable_سائز - 4;
          }
        }
      }جب( rc==SQLITE_OK && n_برآمد>0 );
 
      اگر( rc==SQLITE_OK && nRem>0 && ALWAYS(p_Pgno_برآمد) ){
        Pgno pgnoNew = 0;  /* Prevent harmless static-analyzer warning */
        Mem_صفحہ *pNew = 0;
        rc = allocate_Btree_صفحہ(pBt, &pNew, &pgnoNew, 0, 0);
        put4byte(p_Pgno_برآمد, pgnoNew);
        اگر( ISAUTOVACUUM(pBt) && p_صفحہ_برآمد ){
          ptrmapPut(pBt, pgnoNew, PTRMAP_OVERFLOW2, p_صفحہ_برآمد->pgno, &rc);
        }
        release_صفحہ(p_صفحہ_برآمد);
        p_صفحہ_برآمد = pNew;
        اگر( p_صفحہ_برآمد ){
          p_Pgno_برآمد = p_صفحہ_برآمد->a_ڈیٹا;
          put4byte(p_Pgno_برآمد, 0);
          a_برآمد = &p_Pgno_برآمد[4];
          n_برآمد = MIN(pBt->usable_سائز - 4, nRem);
        }
      }
    }جب( nRem>0 && rc==SQLITE_OK );
 
    release_صفحہ(p_صفحہ_برآمد);
    sqlite3PagerUnref(p_صفحہ_اندر);
    واپس rc;
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
صحیح sqlite3BtreeDelete(BtCursor *pCur, u8 flags){
  Btree *p = pCur->pBtree;
  BtShared *pBt = p->pBt;             
  صحیح rc;                    /* Return code */
  Mem_صفحہ *p_صفحہ;            /* Page to delete cell from */
  غیرعلامتی حرف *pCell;      /* Pointer to cell to delete */
  صحیح iCellIdx;              /* Index of cell to delete */
  صحیح i_Cell_گہرائی;            /* Depth of node containing pCell */
  CellInfo info;             /* Size of the cell being deleted */
  u8 bPreserve;              /* Keep cursor valid.  2 for CURSOR_SKIPNEXT */

  أكد( cursorOwnsBtShared(pCur) );
  أكد( pBt->اندر_Transaction==TRANS_WRITE );
  أكد( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  أكد( pCur->curFlags & BTCF_WriteFlag );
  أكد( has_Shared_Cache_جدول_تالا(p, pCur->pgno_جڑ, pCur->p_کلید_Info!=0, 2) );
  أكد( !hasReadConflicts(p, pCur->pgno_جڑ) );
  أكد( (flags & ~(BTREE_SAVEPOSITION | BTREE_AUXDELETE))==0 );
  اگر( pCur->eState!=CURSOR_VALID ){
    اگر( pCur->eState>=CURSOR_REQUIRESEEK ){
      rc = btreeRestoreCursorPosition(pCur);
      أكد( rc!=SQLITE_OK || CORRUPT_DB || pCur->eState==CURSOR_VALID );
      اگر( rc || pCur->eState!=CURSOR_VALID ) واپس rc;
    }ورنہ{
      واپس SQLITE_CORRUPT_PGNO(pCur->pgno_جڑ);
    }
  }
  أكد( pCur->eState==CURSOR_VALID );

  i_Cell_گہرائی = pCur->i_صفحہ;
  iCellIdx = pCur->ix;
  p_صفحہ = pCur->p_صفحہ;
  اگر( p_صفحہ->nCell<=iCellIdx ){
    واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
  }
  pCell = ڈھونڈو_Cell(p_صفحہ, iCellIdx);
  اگر( p_صفحہ->nFree<0 && btreeComputeFreeSpace(p_صفحہ) ){
    واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
  }
  اگر( pCell<&p_صفحہ->aCellIdx[p_صفحہ->nCell] ){
    واپس SQLITE_CORRUPT_صفحہ(p_صفحہ);
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
  اگر( bPreserve ){
    اگر( !p_صفحہ->برگ
     || (p_صفحہ->nFree+p_صفحہ->x_Cell_سائز(p_صفحہ,pCell)+2) >
                                                   (صحیح)(pBt->usable_سائز*2/3)
     || p_صفحہ->nCell==1  /* See dbfuzz001.test for a test case */
    ){
      /* A b-tree rebalance will be required after deleting this entry.
      ** Save the cursor key.  */
      rc = save_Cursor_کلید(pCur);
      اگر( rc ) واپس rc;
    }ورنہ{
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
  اگر( !p_صفحہ->برگ ){
    rc = sqlite3_Btree_پچھلا(pCur, 0);
    أكد( rc!=SQLITE_مکمل );
    اگر( rc ) واپس rc;
  }

  /* Save the positions of any other cursors open on this table before
  ** making any modifications.  */
  اگر( pCur->curFlags & BTCF_مضرب ){
    rc = saveAllCursors(pBt, pCur->pgno_جڑ, pCur);
    اگر( rc ) واپس rc;
  }

  /* If this is a delete operation to remove a row from a table b-tree,
  ** invalidate any incrblob cursors open on the row being deleted.  */
  اگر( pCur->p_کلید_Info==0 && p->hasIncrblobCur ){
    invalidateIncrblobCursors(p, pCur->pgno_جڑ, pCur->info.n_کلید, 0);
  }

  /* Make the page containing the entry to be deleted writable. Then free any
  ** overflow pages associated with the entry and finally remove the cell
  ** itself from within the page.  */
  rc = sqlite3PagerWrite(p_صفحہ->p_Db_صفحہ);
  اگر( rc ) واپس rc;
  BTREE_CLEAR_CELL(rc, p_صفحہ, pCell, info);
  dropCell(p_صفحہ, iCellIdx, info.n_سائز, &rc);
  اگر( rc ) واپس rc;

  /* If the cell deleted was not located on a leaf page, then the cursor
  ** is currently pointing to the largest entry in the sub-tree headed
  ** by the child-page of the cell that was just deleted from an internal
  ** node. The cell from the leaf node needs to be moved to the internal
  ** node to replace the deleted cell.  */
  اگر( !p_صفحہ->برگ ){
    Mem_صفحہ *p_برگ = pCur->p_صفحہ;
    صحیح nCell;
    Pgno n;
    غیرعلامتی حرف *pTmp;

    اگر( p_برگ->nFree<0 ){
      rc = btreeComputeFreeSpace(p_برگ);
      اگر( rc ) واپس rc;
    }
    اگر( i_Cell_گہرائی<pCur->i_صفحہ-1 ){
      n = pCur->ap_صفحہ[i_Cell_گہرائی+1]->pgno;
    }ورنہ{
      n = pCur->p_صفحہ->pgno;
    }
    pCell = ڈھونڈو_Cell(p_برگ, p_برگ->nCell-1);
    اگر( pCell<&p_برگ->a_ڈیٹا[4] ) واپس SQLITE_CORRUPT_صفحہ(p_برگ);
    nCell = p_برگ->x_Cell_سائز(p_برگ, pCell);
    أكد( MX_CELL_سائز(pBt) >= nCell );
    pTmp = pBt->pTmpSpace;
    أكد( pTmp!=0 );
    rc = sqlite3PagerWrite(p_برگ->p_Db_صفحہ);
    اگر( rc==SQLITE_OK ){
      rc = داخل_کرو_Cell(p_صفحہ, iCellIdx, pCell-4, nCell+4-(pCell[0]==2), pTmp, n);
    }
    dropCell(p_برگ, p_برگ->nCell-1, nCell, &rc);
    اگر( rc ) واپس rc;
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
  أكد( pCur->p_صفحہ->n_حد_سے_تجاوز==0 );
  أكد( pCur->p_صفحہ->nFree>=0 );
  اگر( pCur->p_صفحہ->nFree*3<=(صحیح)pCur->pBt->usable_سائز*2 ){
    /* Optimization: If the free space is less than 2/3rds of the page,
    ** then balance() will always be a no-op.  No need to invoke it. */
    rc = SQLITE_OK;
  }ورنہ{
    rc = balance(pCur);
  }
  اگر( rc==SQLITE_OK && pCur->i_صفحہ>i_Cell_گہرائی ){
    release_صفحہ_Not_Null(pCur->p_صفحہ);
    pCur->i_صفحہ--;
    جب( pCur->i_صفحہ>i_Cell_گہرائی ){
      release_صفحہ(pCur->ap_صفحہ[pCur->i_صفحہ--]);
    }
    pCur->p_صفحہ = pCur->ap_صفحہ[pCur->i_صفحہ];
    rc = balance(pCur);
  }

  اگر( rc==SQLITE_OK ){
    اگر( bPreserve>1 ){
      أكد( (pCur->i_صفحہ==i_Cell_گہرائی || CORRUPT_DB) );
      أكد( p_صفحہ==pCur->p_صفحہ || CORRUPT_DB );
      أكد( (p_صفحہ->nCell>0 || CORRUPT_DB) && iCellIdx<=p_صفحہ->nCell );
      pCur->eState = CURSOR_SKIPNEXT;
      اگر( iCellIdx>=p_صفحہ->nCell ){
        pCur->skip_اگلا = -1;
        pCur->ix = p_صفحہ->nCell-1;
      }ورنہ{
        pCur->skip_اگلا = 1;
      }
      /* The pCur->info structure is currently populated with the details
      ** of the cell that was just removed. But pCur->ix now points to a
      ** different cell in the b-tree. So zero info.nSize here so that if
      ** saveCursorPosition() is called on this cursor it does not trust
      ** the contents of pCur->info.  */
      pCur->info.n_سائز = 0;
    }ورنہ{
      rc = move_مقصد_جڑ(pCur);
      اگر( bPreserve ){
        btree_Release_All_Cursor_صفحات(pCur);
        pCur->eState = CURSOR_REQUIRESEEK;
      }
      اگر( rc==SQLITE_خالی_خانے ) rc = SQLITE_OK;
    }
  }
  واپس rc;
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
ساکن صحیح btree_Create_جدول(Btree *p, Pgno *pi_جدول, صحیح createTabFlags){
  BtShared *pBt = p->pBt;
  Mem_صفحہ *p_جڑ;
  Pgno pgno_جڑ;
  صحیح rc;
  صحیح ptfFlags;          /* Page-type flags for the root page of new table */

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( pBt->اندر_Transaction==TRANS_WRITE );
  أكد( (pBt->btsFlags & BTS_READ_ONLY)==0 );

#اگر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  rc = allocate_Btree_صفحہ(pBt, &p_جڑ, &pgno_جڑ, 1, 0);
  اگر( rc ){
    واپس rc;
  }
#ورنہ
  اگر( pBt->autoVacuum ){
    Pgno pgnoMove;      /* Move a page here to make room for the root-page */
    Mem_صفحہ *p_صفحہ_Move; /* The page to move to. */

    /* Creating a new table may probably require moving an existing database
    ** to make room for the new tables root page. In case this page turns
    ** out to be an overflow page, delete all overflow page-map caches
    ** held by open cursors.
    */
    invalidate_All_حد_سے_تجاوز_Cache(pBt);

    /* Read the value of meta[3] from the database to determine where the
    ** root page of the new table should go. meta[3] is the largest root-page
    ** created so far, so the new root-page is (meta[3]+1).
    */
    sqlite3BtreeGetMeta(p, BTREE_سب_سے_بڑا_جڑ_صفحہ, &pgno_جڑ);
    اگر( pgno_جڑ>btreePagecount(pBt) ){
      واپس SQLITE_CORRUPT_PGNO(pgno_جڑ);
    }
    pgno_جڑ++;

    /* The new root-page may not be allocated on a pointer-map page, or the
    ** PENDING_BYTE page.
    */
    جب( pgno_جڑ==PTRMAP_PAGENO(pBt, pgno_جڑ) ||
        pgno_جڑ==PENDING_BYTE_صفحہ(pBt) ){
      pgno_جڑ++;
    }
    أكد( pgno_جڑ>=3 );

    /* Allocate a page. The page that currently resides at pgnoRoot will
    ** be moved to the allocated page (unless the allocated page happens
    ** to reside at pgnoRoot).
    */
    rc = allocate_Btree_صفحہ(pBt, &p_صفحہ_Move, &pgnoMove, pgno_جڑ, BTALLOC_EXACT);
    اگر( rc!=SQLITE_OK ){
      واپس rc;
    }

    اگر( pgnoMove!=pgno_جڑ ){
      /* pgnoRoot is the page that will be used for the root-page of
      ** the new table (assuming an error did not occur). But we were
      ** allocated pgnoMove. If required (i.e. if it was not allocated
      ** by extending the file), the current page at position pgnoMove
      ** is already journaled.
      */
      u8 eType = 0;
      Pgno i_Ptr_صفحہ = 0;

      /* Save the positions of any open cursors. This is required in
      ** case they are holding a reference to an xFetch reference
      ** corresponding to page pgnoRoot.  */
      rc = saveAllCursors(pBt, 0, 0);
      release_صفحہ(p_صفحہ_Move);
      اگر( rc!=SQLITE_OK ){
        واپس rc;
      }

      /* Move the page currently at pgnoRoot to pgnoMove. */
      rc = btree_Get_صفحہ(pBt, pgno_جڑ, &p_جڑ, 0);
      اگر( rc!=SQLITE_OK ){
        واپس rc;
      }
      rc = ptrmapGet(pBt, pgno_جڑ, &eType, &i_Ptr_صفحہ);
      اگر( eType==PTRMAP_ROOTPAGE || eType==PTRMAP_FREEPAGE ){
        rc = SQLITE_CORRUPT_PGNO(pgno_جڑ);
      }
      اگر( rc!=SQLITE_OK ){
        release_صفحہ(p_جڑ);
        واپس rc;
      }
      أكد( eType!=PTRMAP_ROOTPAGE );
      أكد( eType!=PTRMAP_FREEPAGE );
      rc = relocate_صفحہ(pBt, p_جڑ, eType, i_Ptr_صفحہ, pgnoMove, 0);
      release_صفحہ(p_جڑ);

      /* Obtain the page at pgnoRoot */
      اگر( rc!=SQLITE_OK ){
        واپس rc;
      }
      rc = btree_Get_صفحہ(pBt, pgno_جڑ, &p_جڑ, 0);
      اگر( rc!=SQLITE_OK ){
        واپس rc;
      }
      rc = sqlite3PagerWrite(p_جڑ->p_Db_صفحہ);
      اگر( rc!=SQLITE_OK ){
        release_صفحہ(p_جڑ);
        واپس rc;
      }
    }ورنہ{
      p_جڑ = p_صفحہ_Move;
    }

    /* Update the pointer-map and meta-data with the new root-page number. */
    ptrmapPut(pBt, pgno_جڑ, PTRMAP_ROOTPAGE, 0, &rc);
    اگر( rc ){
      release_صفحہ(p_جڑ);
      واپس rc;
    }

    /* When the new root page was allocated, page 1 was made writable in
    ** order either to increase the database filesize, or to decrement the
    ** freelist count.  Hence, the sqlite3BtreeUpdateMeta() call cannot fail.
    */
    أكد( sqlite3PagerIswriteable(pBt->pPage1->p_Db_صفحہ) );
    rc = sqlite3BtreeUpdateMeta(p, 4, pgno_جڑ);
    اگر( NEVER(rc) ){
      release_صفحہ(p_جڑ);
      واپس rc;
    }

  }ورنہ{
    rc = allocate_Btree_صفحہ(pBt, &p_جڑ, &pgno_جڑ, 1, 0);
    اگر( rc ) واپس rc;
  }
#اگر_ختم
  أكد( sqlite3PagerIswriteable(p_جڑ->p_Db_صفحہ) );
  اگر( createTabFlags & BTREE_INTKEY ){
    ptfFlags = PTF_INTKEY | PTF_LEAFDATA | PTF_برگ;
  }ورنہ{
    ptfFlags = PTF_ZERODATA | PTF_برگ;
  }
  zero_صفحہ(p_جڑ, ptfFlags);
  sqlite3PagerUnref(p_جڑ->p_Db_صفحہ);
  أكد( (pBt->openFlags & BTREE_SINGLE)==0 || pgno_جڑ==2 );
  *pi_جدول = pgno_جڑ;
  واپس SQLITE_OK;
}
صحیح sqlite3_Btree_Create_جدول(Btree *p, Pgno *pi_جدول, صحیح flags){
  صحیح rc;
  sqlite3BtreeEnter(p);
  rc = btree_Create_جدول(p, pi_جدول, flags);
  sqlite3BtreeLeave(p);
  واپس rc;
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
ساکن صحیح clear_Database_صفحہ(
  BtShared *pBt,           /* The BTree that contains the table */
  Pgno pgno,               /* Page number to clear */
  صحیح free_صفحہ_Flag,        /* bit 0: Deallocate page.  Bits 1-31: depth */
  i64 *pnChange            /* Add number of Cells freed to this counter */
){
  Mem_صفحہ *p_صفحہ;
  صحیح rc;
  غیرعلامتی حرف *pCell;
  صحیح i;
  صحیح hdr;
  CellInfo info;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  اگر( pgno>btreePagecount(pBt) ){
    واپس SQLITE_CORRUPT_PGNO(pgno);
  }
  اگر( (free_صفحہ_Flag>>1) > BTCURSOR_MAX_گہرائی ){
    واپس SQLITE_CORRUPT_PGNO(pgno);
  }
  rc = get_And_Init_صفحہ(pBt, pgno, &p_صفحہ, 0);
  اگر( rc ) واپس rc;
  اگر( (pBt->openFlags & BTREE_SINGLE)==0
   && sqlite3_Pager_صفحہ_Refcount(p_صفحہ->p_Db_صفحہ) != (1 + (pgno==1))
  ){
    rc = SQLITE_CORRUPT_صفحہ(p_صفحہ);
    جاؤ cleardatabasepage_برآمد;
  }
  hdr = p_صفحہ->hdr_انحراف;
  برائے(i=0; i<p_صفحہ->nCell; i++){
    pCell = ڈھونڈو_Cell(p_صفحہ, i);
    اگر( !p_صفحہ->برگ ){
      rc = clear_Database_صفحہ(pBt, get4byte(pCell),
                             (free_صفحہ_Flag+2)|1, pnChange);
      اگر( rc ) جاؤ cleardatabasepage_برآمد;
    }
    BTREE_CLEAR_CELL(rc, p_صفحہ, pCell, info);
    اگر( rc ) جاؤ cleardatabasepage_برآمد;
  }
  اگر( !p_صفحہ->برگ ){
    rc = clear_Database_صفحہ(pBt, get4byte(&p_صفحہ->a_ڈیٹا[hdr+8]), 
                           (free_صفحہ_Flag+2)|1, pnChange);
    اگر( rc ) جاؤ cleardatabasepage_برآمد;
    اگر( p_صفحہ->int_کلید ) pnChange = 0;
  }
  اگر( pnChange ){
    testcase( !p_صفحہ->int_کلید );
    *pnChange += p_صفحہ->nCell;
  }
  اگر( (free_صفحہ_Flag&1)!=0 ){
    free_صفحہ(p_صفحہ, &rc);
  }ورنہ اگر( (rc = sqlite3PagerWrite(p_صفحہ->p_Db_صفحہ))==0 ){
    zero_صفحہ(p_صفحہ, p_صفحہ->a_ڈیٹا[hdr] | PTF_برگ);
  }

cleardatabasepage_برآمد:
  release_صفحہ(p_صفحہ);
  واپس rc;
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
صحیح sqlite3_Btree_Clear_جدول(Btree *p, صحیح i_جدول, i64 *pnChange){
  صحیح rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);
  أكد( p->اندر_Trans==TRANS_WRITE );

  rc = saveAllCursors(pBt, (Pgno)i_جدول, 0);

  اگر( SQLITE_OK==rc ){
    /* Invalidate all incrblob cursors open on table iTable (assuming iTable
    ** is the root of a table b-tree - if it is not, the following call is
    ** a no-op).  */
    اگر( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, (Pgno)i_جدول, 0, 1);
    }
    rc = clear_Database_صفحہ(pBt, (Pgno)i_جدول, 0, pnChange);
  }
  sqlite3BtreeLeave(p);
  واپس rc;
}

/*
** Delete all information from the single table that pCur is open on.
**
** This routine only work for pCur on an ephemeral table.
*/
صحیح sqlite3_Btree_Clear_جدول_کا_Cursor(BtCursor *pCur){
  واپس sqlite3_Btree_Clear_جدول(pCur->pBtree, pCur->pgno_جڑ, 0);
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
ساکن صحیح btree_Drop_جدول(Btree *p, Pgno i_جدول, صحیح *piMoved){
  صحیح rc;
  Mem_صفحہ *p_صفحہ = 0;
  BtShared *pBt = p->pBt;

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( p->اندر_Trans==TRANS_WRITE );
  أكد( i_جدول>=2 );
  اگر( i_جدول>btreePagecount(pBt) ){
    واپس SQLITE_CORRUPT_PGNO(i_جدول);
  }

  rc = sqlite3_Btree_Clear_جدول(p, i_جدول, 0);
  اگر( rc ) واپس rc;
  rc = btree_Get_صفحہ(pBt, (Pgno)i_جدول, &p_صفحہ, 0);
  اگر( NEVER(rc) ){
    release_صفحہ(p_صفحہ);
    واپس rc;
  }

  *piMoved = 0;

#اگر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  free_صفحہ(p_صفحہ, &rc);
  release_صفحہ(p_صفحہ);
#ورنہ
  اگر( pBt->autoVacuum ){
    Pgno max_جڑ_Pgno;
    sqlite3BtreeGetMeta(p, BTREE_سب_سے_بڑا_جڑ_صفحہ, &max_جڑ_Pgno);

    اگر( i_جدول==max_جڑ_Pgno ){
      /* If the table being dropped is the table with the largest root-page
      ** number in the database, put the root page on the free list.
      */
      free_صفحہ(p_صفحہ, &rc);
      release_صفحہ(p_صفحہ);
      اگر( rc!=SQLITE_OK ){
        واپس rc;
      }
    }ورنہ{
      /* The table being dropped does not have the largest root-page
      ** number in the database. So move the page that does into the
      ** gap left by the deleted root-page.
      */
      Mem_صفحہ *pMove;
      release_صفحہ(p_صفحہ);
      rc = btree_Get_صفحہ(pBt, max_جڑ_Pgno, &pMove, 0);
      اگر( rc!=SQLITE_OK ){
        واپس rc;
      }
      rc = relocate_صفحہ(pBt, pMove, PTRMAP_ROOTPAGE, 0, i_جدول, 0);
      release_صفحہ(pMove);
      اگر( rc!=SQLITE_OK ){
        واپس rc;
      }
      pMove = 0;
      rc = btree_Get_صفحہ(pBt, max_جڑ_Pgno, &pMove, 0);
      اگر( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pMove->p_Db_صفحہ);
      }
      free_صفحہ(pMove, &rc);
      release_صفحہ(pMove);
      اگر( rc!=SQLITE_OK ){
        واپس rc;
      }
      *piMoved = max_جڑ_Pgno;
    }

    /* Set the new 'max-root-page' value in the database header. This
    ** is the old value less one, less one more if that happens to
    ** be a root-page number, less one again if that is the
    ** PENDING_BYTE_PAGE.
    */
    max_جڑ_Pgno--;
    جب( max_جڑ_Pgno==PENDING_BYTE_صفحہ(pBt)
           || PTRMAP_ISPAGE(pBt, max_جڑ_Pgno) ){
      max_جڑ_Pgno--;
    }
    أكد( max_جڑ_Pgno!=PENDING_BYTE_صفحہ(pBt) );

    rc = sqlite3BtreeUpdateMeta(p, 4, max_جڑ_Pgno);
  }ورنہ{
    free_صفحہ(p_صفحہ, &rc);
    release_صفحہ(p_صفحہ);
  }
#اگر_ختم
  واپس rc; 
}
صحیح sqlite3_Btree_Drop_جدول(Btree *p, صحیح i_جدول, صحیح *piMoved){
  صحیح rc;
  sqlite3BtreeEnter(p);
  rc = btree_Drop_جدول(p, i_جدول, piMoved);
  sqlite3BtreeLeave(p);
  واپس rc;
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
خالی sqlite3BtreeGetMeta(Btree *p, صحیح idx, u32 *pMeta){
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  أكد( p->اندر_Trans>TRANS_NONE );
  أكد( SQLITE_OK==query_Shared_Cache_جدول_تالا(p, SCHEMA_جڑ, READ_تالا) );
  أكد( pBt->pPage1 );
  أكد( idx>=0 && idx<=15 );

  اگر( idx==BTREE_ڈیٹا_نسخہ ){
    *pMeta = sqlite3_Pager_ڈیٹا_نسخہ(pBt->pPager) + p->i_BData_نسخہ;
  }ورنہ{
    *pMeta = get4byte(&pBt->pPage1->a_ڈیٹا[36 + idx*4]);
  }

  /* If auto-vacuum is disabled in this build and this is an auto-vacuum
  ** database, mark the database as read-only.  */
#اگر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  اگر( idx==BTREE_سب_سے_بڑا_جڑ_صفحہ && *pMeta>0 ){
    pBt->btsFlags |= BTS_READ_ONLY;
  }
#اگر_ختم

  sqlite3BtreeLeave(p);
}

/*
** Write meta-information back into the database.  Meta[0] is
** read-only and may not be written.
*/
صحیح sqlite3BtreeUpdateMeta(Btree *p, صحیح idx, u32 iMeta){
  BtShared *pBt = p->pBt;
  غیرعلامتی حرف *pP1;
  صحیح rc;
  أكد( idx>=1 && idx<=15 );
  sqlite3BtreeEnter(p);
  أكد( p->اندر_Trans==TRANS_WRITE );
  أكد( pBt->pPage1!=0 );
  pP1 = pBt->pPage1->a_ڈیٹا;
  rc = sqlite3PagerWrite(pBt->pPage1->p_Db_صفحہ);
  اگر( rc==SQLITE_OK ){
    put4byte(&pP1[36 + idx*4], iMeta);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
    اگر( idx==BTREE_INCR_VACUUM ){
      أكد( pBt->autoVacuum || iMeta==0 );
      أكد( iMeta==0 || iMeta==1 );
      pBt->incrVacuum = (u8)iMeta;
    }
#اگر_ختم
  }
  sqlite3BtreeLeave(p);
  واپس rc;
}

/*
** The first argument, pCur, is a cursor opened on some b-tree. Count the
** number of entries in the b-tree and write the result to *pnEntry.
**
** SQLITE_OK is returned if the operation is successfully executed.
** Otherwise, if an error is encountered (i.e. an IO error or database
** corruption) an SQLite error code is returned.
*/
صحیح sqlite3_Btree_گنتی(sqlite3 *db, BtCursor *pCur, i64 *pn_اندراج){
  i64 n_اندراج = 0;                      /* Value to return in *pnEntry */
  صحیح rc;                              /* Return code */

  rc = move_مقصد_جڑ(pCur);
  اگر( rc==SQLITE_خالی_خانے ){
    *pn_اندراج = 0;
    واپس SQLITE_OK;
  }

  /* Unless an error occurs, the following loop runs one iteration for each
  ** page in the B-Tree structure (not including overflow pages).
  */
  جب( rc==SQLITE_OK && !Atomic_بھرو(&db->u1.isInterrupted) ){
    صحیح iIdx;                          /* Index of child node in parent */
    Mem_صفحہ *p_صفحہ;                    /* Current page of the b-tree */

    /* If this is a leaf page or the tree is not an int-key tree, then
    ** this page contains countable entries. Increment the entry counter
    ** accordingly.
    */
    p_صفحہ = pCur->p_صفحہ;
    اگر( p_صفحہ->برگ || !p_صفحہ->int_کلید ){
      n_اندراج += p_صفحہ->nCell;
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
    اگر( p_صفحہ->برگ ){
      کرو {
        اگر( pCur->i_صفحہ==0 ){
          /* All pages of the b-tree have been visited. Return successfully. */
          *pn_اندراج = n_اندراج;
          واپس move_مقصد_جڑ(pCur);
        }
        move_مقصد_والد(pCur);
      }جب ( pCur->ix>=pCur->p_صفحہ->nCell );

      pCur->ix++;
      p_صفحہ = pCur->p_صفحہ;
    }

    /* Descend to the child node of the cell that the cursor currently
    ** points at. This is the right-child if (iIdx==pPage->nCell).
    */
    iIdx = pCur->ix;
    اگر( iIdx==p_صفحہ->nCell ){
      rc = move_مقصد_بچہ(pCur, get4byte(&p_صفحہ->a_ڈیٹا[p_صفحہ->hdr_انحراف+8]));
    }ورنہ{
      rc = move_مقصد_بچہ(pCur, get4byte(ڈھونڈو_Cell(p_صفحہ, iIdx)));
    }
  }

  /* An error has occurred. Return an error code. */
  واپس rc;
}

/*
** Return the pager associated with a BTree.  This routine is used for
** testing and debugging only.
*/
Pager *sqlite3BtreePager(Btree *p){
  واپس p->pBt->pPager;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INTEGRITY_CHECK
/*
** Record an OOM error during integrity_check
*/
ساکن خالی checkOom(IntegrityCk *pCheck){
  pCheck->rc = SQLITE_NOMEM;
  pCheck->mxErr = 0;  /* Causes integrity_check processing to stop */
  اگر( pCheck->nErr==0 ) pCheck->nErr++;
}

/*
** Invoke the progress handler, if appropriate.  Also check for an
** interrupt.
*/
ساکن خالی check_پیش_رفت(IntegrityCk *pCheck){
  sqlite3 *db = pCheck->db;
  اگر( Atomic_بھرو(&db->u1.isInterrupted) ){
    pCheck->rc = SQLITE_مداخلت;
    pCheck->nErr++;
    pCheck->mxErr = 0;
  }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_پیش_رفت_CALLBACK
  اگر( db->x_پیش_رفت ){
    أكد( db->n_پیش_رفت_Ops>0 );
    pCheck->n_قدم++;
    اگر( (pCheck->n_قدم % db->n_پیش_رفت_Ops)==0
     && db->x_پیش_رفت(db->p_پیش_رفت_Arg)
    ){
      pCheck->rc = SQLITE_مداخلت;
      pCheck->nErr++;
      pCheck->mxErr = 0;
    }
  }
#اگر_ختم
}

/*
** Append a message to the error message string.
*/
ساکن خالی check_Append_پیغام(
  IntegrityCk *pCheck,
  ثابت حرف *zFormat,
  ...
){
  va_list ap;
  check_پیش_رفت(pCheck);
  اگر( !pCheck->mxErr ) واپس;
  pCheck->mxErr--;
  pCheck->nErr++;
  va_start(ap, zFormat);
  اگر( pCheck->err_پیغام.nChar ){
    sqlite3_str_append(&pCheck->err_پیغام, "\n", 1);
  }
  اگر( pCheck->zPfx ){
    sqlite3_str_appendf(&pCheck->err_پیغام, pCheck->zPfx,
                        pCheck->v0, pCheck->v1, pCheck->v2);
  }
  sqlite3_str_vappendf(&pCheck->err_پیغام, zFormat, ap);
  va_end(ap);
  اگر( pCheck->err_پیغام.accError==SQLITE_NOMEM ){
    checkOom(pCheck);
  }
}
#اگر_ختم /* SQLITE_OMIT_INTEGRITY_CHECK */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INTEGRITY_CHECK

/*
** Return non-zero if the bit in the IntegrityCk.aPgRef[] array that
** corresponds to page iPg is already set.
*/
ساکن صحیح get_صفحہ_Referenced(IntegrityCk *pCheck, Pgno iPg){
  أكد( pCheck->aPgRef!=0 );
  أكد( iPg<=pCheck->n_Ck_صفحہ && حجم(pCheck->aPgRef[0])==1 );
  واپس (pCheck->aPgRef[iPg/8] & (1 << (iPg & 0x07)));
}

/*
** Set the bit in the IntegrityCk.aPgRef[] array that corresponds to page iPg.
*/
ساکن خالی سیٹ_صفحہ_Referenced(IntegrityCk *pCheck, Pgno iPg){
  أكد( pCheck->aPgRef!=0 );
  أكد( iPg<=pCheck->n_Ck_صفحہ && حجم(pCheck->aPgRef[0])==1 );
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
ساکن صحیح checkRef(IntegrityCk *pCheck, Pgno i_صفحہ){
  اگر( i_صفحہ>pCheck->n_Ck_صفحہ || i_صفحہ==0 ){
    check_Append_پیغام(pCheck, "invalid page number %u", i_صفحہ);
    واپس 1;
  }
  اگر( get_صفحہ_Referenced(pCheck, i_صفحہ) ){
    check_Append_پیغام(pCheck, "2nd reference to page %u", i_صفحہ);
    واپس 1;
  }
  سیٹ_صفحہ_Referenced(pCheck, i_صفحہ);
  واپس 0;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
/*
** Check that the entry in the pointer-map for page iChild maps to
** page iParent, pointer type ptrType. If not, append an error message
** to pCheck.
*/
ساکن خالی checkPtrmap(
  IntegrityCk *pCheck,   /* Integrity check context */
  Pgno i_بچہ,           /* Child page number */
  u8 eType,              /* Expected pointer map type */
  Pgno i_والد           /* Expected pointer map parent page number */
){
  صحیح rc;
  u8 ePtrmapType;
  Pgno i_Ptrmap_والد;

  rc = ptrmapGet(pCheck->pBt, i_بچہ, &ePtrmapType, &i_Ptrmap_والد);
  اگر( rc!=SQLITE_OK ){
    اگر( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ) checkOom(pCheck);
    check_Append_پیغام(pCheck, "Failed to read ptrmap key=%u", i_بچہ);
    واپس;
  }

  اگر( ePtrmapType!=eType || i_Ptrmap_والد!=i_والد ){
    check_Append_پیغام(pCheck,
      "Bad ptr map entry key=%u expected=(%u,%u) got=(%u,%u)",
      i_بچہ, eType, i_والد, ePtrmapType, i_Ptrmap_والد);
  }
}
#اگر_ختم

/*
** Check the integrity of the freelist or of an overflow page list.
** Verify that the number of pages on the list is N.
*/
ساکن خالی checkList(
  IntegrityCk *pCheck,  /* Integrity checking context */
  صحیح isFreeList,       /* True for a freelist.  False for overflow page list */
  Pgno i_صفحہ,           /* Page number for first page in the list */
  u32 N                 /* Expected number of pages in the list */
){
  صحیح i;
  u32 expected = N;
  صحیح n_Err_At_آغاز = pCheck->nErr;
  جب( i_صفحہ!=0 && pCheck->mxErr ){
    Db_صفحہ *p_Ovfl_صفحہ;
    غیرعلامتی حرف *p_Ovfl_ڈیٹا;
    اگر( checkRef(pCheck, i_صفحہ) ) توڑو;
    N--;
    اگر( sqlite3PagerGet(pCheck->pPager, (Pgno)i_صفحہ, &p_Ovfl_صفحہ, 0) ){
      check_Append_پیغام(pCheck, "failed to get page %u", i_صفحہ);
      توڑو;
    }
    p_Ovfl_ڈیٹا = (غیرعلامتی حرف *)sqlite3_Pager_Get_ڈیٹا(p_Ovfl_صفحہ);
    اگر( isFreeList ){
      u32 n = (u32)get4byte(&p_Ovfl_ڈیٹا[4]);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
      اگر( pCheck->pBt->autoVacuum ){
        checkPtrmap(pCheck, i_صفحہ, PTRMAP_FREEPAGE, 0);
      }
#اگر_ختم
      اگر( n>pCheck->pBt->usable_سائز/4-2 ){
        check_Append_پیغام(pCheck,
           "freelist leaf count too big on page %u", i_صفحہ);
        N--;
      }ورنہ{
        برائے(i=0; i<(صحیح)n; i++){
          Pgno i_Free_صفحہ = get4byte(&p_Ovfl_ڈیٹا[8+i*4]);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
          اگر( pCheck->pBt->autoVacuum ){
            checkPtrmap(pCheck, i_Free_صفحہ, PTRMAP_FREEPAGE, 0);
          }
#اگر_ختم
          checkRef(pCheck, i_Free_صفحہ);
        }
        N -= n;
      }
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
    ورنہ{
      /* If this database supports auto-vacuum and iPage is not the last
      ** page in this overflow list, check that the pointer-map entry for
      ** the following page matches iPage.
      */
      اگر( pCheck->pBt->autoVacuum && N>0 ){
        i = get4byte(p_Ovfl_ڈیٹا);
        checkPtrmap(pCheck, i, PTRMAP_OVERFLOW2, i_صفحہ);
      }
    }
#اگر_ختم
    i_صفحہ = get4byte(p_Ovfl_ڈیٹا);
    sqlite3PagerUnref(p_Ovfl_صفحہ);
  }
  اگر( N && n_Err_At_آغاز==pCheck->nErr ){
    check_Append_پیغام(pCheck,
      "%s is %u but should be %u",
      isFreeList ? "size" : "overflow list length",
      expected-N, expected);
  }
}
#اگر_ختم /* SQLITE_OMIT_INTEGRITY_CHECK */

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
ساکن خالی btree_ڈھیر_داخل_کرو(u32 *a_ڈھیر, u32 x){
  u32 j, i;
  أكد( a_ڈھیر!=0 );
  i = ++a_ڈھیر[0];
  a_ڈھیر[i] = x;
  جب( (j = i/2)>0 && a_ڈھیر[j]>a_ڈھیر[i] ){
    x = a_ڈھیر[j];
    a_ڈھیر[j] = a_ڈھیر[i];
    a_ڈھیر[i] = x;
    i = j;
  }
}
ساکن صحیح btree_ڈھیر_Pull(u32 *a_ڈھیر, u32 *p_برآمد){
  u32 j, i, x;
  اگر( (x = a_ڈھیر[0])==0 ) واپس 0;
  *p_برآمد = a_ڈھیر[1];
  a_ڈھیر[1] = a_ڈھیر[x];
  a_ڈھیر[x] = 0xffffffff;
  a_ڈھیر[0]--;
  i = 1;
  جب( (j = i*2)<=a_ڈھیر[0] ){
    اگر( a_ڈھیر[j]>a_ڈھیر[j+1] ) j++;
    اگر( a_ڈھیر[i]<a_ڈھیر[j] ) توڑو;
    x = a_ڈھیر[i];
    a_ڈھیر[i] = a_ڈھیر[j];
    a_ڈھیر[j] = x;
    i = j;
  }
  واپس 1; 
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INTEGRITY_CHECK
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
ساکن صحیح check_درخت_صفحہ(
  IntegrityCk *pCheck,  /* Context for the sanity check */
  Pgno i_صفحہ,           /* Page number of the page to check */
  i64 *pi_Min_کلید,        /* Write minimum integer primary key here */
  i64 max_کلید            /* Error if integer primary key greater than this */
){
  Mem_صفحہ *p_صفحہ = 0;      /* The page being analyzed */
  صحیح i;                   /* Loop counter */
  صحیح rc;                  /* Result code from subroutine call */
  صحیح گہرائی = -1, d2;      /* Depth of a subtree */
  صحیح pgno;                /* Page number */
  صحیح nFrag;               /* Number of fragmented bytes on the page */
  صحیح hdr;                 /* Offset to the page header */
  صحیح cell_آغاز;           /* Offset to the start of the cell pointer array */
  صحیح nCell;               /* Number of cells */
  صحیح doCoverageCheck = 1; /* True if cell coverage checking should be done */
  صحیح کلید_Can_Be_Equal = 1;   /* True if IPK can be equal to maxKey
                           ** False if IPK must be strictly less than maxKey */
  u8 *ڈیٹا;                /* Page content */
  u8 *pCell;               /* Cell content */
  u8 *pCellIdx;            /* Next element of the cell pointer array */
  BtShared *pBt;           /* The BtShared object that owns pPage */
  u32 pc;                  /* Address of a cell */
  u32 usable_سائز;          /* Usable size of the page */
  u32 content_انحراف;       /* Offset to the start of the cell content area */
  u32 *ڈھیر = 0;           /* Min-heap used for checking cell coverage */
  u32 x, پچھلا = 0;         /* Next and previous entry on the min-heap */
  ثابت حرف *saved_zPfx = pCheck->zPfx;
  صحیح saved_v1 = pCheck->v1;
  صحیح saved_v2 = pCheck->v2;
  u8 savedIsInit = 0;

  /* Check that the page exists
  */
  check_پیش_رفت(pCheck);
  اگر( pCheck->mxErr==0 ) واپس 0;
  pBt = pCheck->pBt;
  usable_سائز = pBt->usable_سائز;
  اگر( i_صفحہ==0 ) واپس 0;
  اگر( checkRef(pCheck, i_صفحہ) ) واپس 0;
  pCheck->zPfx = "Tree %u page %u: ";
  pCheck->v1 = i_صفحہ;
  pCheck->nAbove++;
  اگر( (rc = btree_Get_صفحہ(pBt, i_صفحہ, &p_صفحہ, 0))!=0 ){
    check_Append_پیغام(pCheck,
       "unable to get the page. error code=%d", rc);
    اگر( rc==SQLITE_IOERR_NOMEM ) pCheck->rc = SQLITE_NOMEM;
    جاؤ انتہا_کا_check;
  }
  اگر( pCheck->nAbove > BTCURSOR_MAX_گہرائی ){
    check_Append_پیغام(pCheck,"btree depth exceeds %d",BTCURSOR_MAX_گہرائی);
    جاؤ انتہا_کا_check;
  }

  /* Clear MemPage.isInit to make sure the corruption detection code in
  ** btreeInitPage() is executed.  */
  savedIsInit = p_صفحہ->isInit;
  p_صفحہ->isInit = 0;
  اگر( (rc = btree_Init_صفحہ(p_صفحہ))!=0 ){
    أكد( rc==SQLITE_CORRUPT );  /* The only possible error from InitPage */
    check_Append_پیغام(pCheck,
                   "btreeInitPage() returns error code %d", rc);
    جاؤ انتہا_کا_check;
  }
  اگر( (rc = btreeComputeFreeSpace(p_صفحہ))!=0 ){
    أكد( rc==SQLITE_CORRUPT );
    check_Append_پیغام(pCheck, "free space corruption", rc);
    جاؤ انتہا_کا_check;
  }
  ڈیٹا = p_صفحہ->a_ڈیٹا;
  hdr = p_صفحہ->hdr_انحراف;

  /* Set up for cell analysis */
  pCheck->zPfx = "Tree %u page %u cell %u: ";
  content_انحراف = get2byteNotZero(&ڈیٹا[hdr+5]);
  أكد( content_انحراف<=usable_سائز );  /* Enforced by btreeInitPage() */

  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  nCell = get2byte(&ڈیٹا[hdr+3]);
  أكد( p_صفحہ->nCell==nCell );
  اگر( p_صفحہ->برگ || p_صفحہ->int_کلید==0 ){
    pCheck->n_صف += nCell;
  }

  /* EVIDENCE-OF: R-23882-45353 The cell pointer array of a b-tree page
  ** immediately follows the b-tree page header. */
  cell_آغاز = hdr + 12 - 4*p_صفحہ->برگ;
  أكد( p_صفحہ->aCellIdx==&ڈیٹا[cell_آغاز] );
  pCellIdx = &ڈیٹا[cell_آغاز + 2*(nCell-1)];

  اگر( !p_صفحہ->برگ ){
    /* Analyze the right-child page of internal pages */
    pgno = get4byte(&ڈیٹا[hdr+8]);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
    اگر( pBt->autoVacuum ){
      pCheck->zPfx = "Tree %u page %u right child: ";
      checkPtrmap(pCheck, pgno, PTRMAP_BTREE, i_صفحہ);
    }
#اگر_ختم
    گہرائی = check_درخت_صفحہ(pCheck, pgno, &max_کلید, max_کلید);
    کلید_Can_Be_Equal = 0;
  }ورنہ{
    /* For leaf pages, the coverage check will occur in the same loop
    ** as the other cell checks, so initialize the heap.  */
    ڈھیر = pCheck->ڈھیر;
    ڈھیر[0] = 0;
  }

  /* EVIDENCE-OF: R-02776-14802 The cell pointer array consists of K 2-byte
  ** integer offsets to the cell contents. */
  برائے(i=nCell-1; i>=0 && pCheck->mxErr; i--){
    CellInfo info;

    /* Check cell size */
    pCheck->v2 = i;
    أكد( pCellIdx==&ڈیٹا[cell_آغاز + i*2] );
    pc = get2byteAligned(pCellIdx);
    pCellIdx -= 2;
    اگر( pc<content_انحراف || pc>usable_سائز-4 ){
      check_Append_پیغام(pCheck, "Offset %u out of range %u..%u",
                             pc, content_انحراف, usable_سائز-4);
      doCoverageCheck = 0;
      جاری;
    }
    pCell = &ڈیٹا[pc];
    p_صفحہ->xParseCell(p_صفحہ, pCell, &info);
    اگر( pc+info.n_سائز>usable_سائز ){
      check_Append_پیغام(pCheck, "Extends off end of page");
      doCoverageCheck = 0;
      جاری;
    }
    اگر( info.nPayload && info.pPayload[0]<2 ){
      check_Append_پیغام(pCheck, "Bad cell header size");
      doCoverageCheck = 0;
      جاری;
    }

    /* Check for integer primary key out of range */
    اگر( p_صفحہ->int_کلید ){
      اگر( کلید_Can_Be_Equal ? (info.n_کلید > max_کلید) : (info.n_کلید >= max_کلید) ){
        check_Append_پیغام(pCheck, "Rowid %lld out of order", info.n_کلید);
      }
      max_کلید = info.n_کلید;
      کلید_Can_Be_Equal = 0;     /* Only the first key on the page may ==maxKey */
    }

    /* Check the content overflow list */
    اگر( info.nPayload>info.nLocal ){
      u32 n_صفحہ;       /* Number of pages on the overflow chain */
      Pgno pgnoOvfl;   /* First page of the overflow chain */
      أكد( pc + info.n_سائز - 4 <= usable_سائز );
      n_صفحہ = (info.nPayload - info.nLocal + usable_سائز - 5)/(usable_سائز - 4);
      pgnoOvfl = get4byte(&pCell[info.n_سائز - 4]);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
      اگر( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgnoOvfl, PTRMAP_OVERFLOW1, i_صفحہ);
      }
#اگر_ختم
      checkList(pCheck, 0, pgnoOvfl, n_صفحہ);
    }

    اگر( !p_صفحہ->برگ ){
      /* Check sanity of left child page for internal pages */
      pgno = get4byte(pCell);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
      اگر( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgno, PTRMAP_BTREE, i_صفحہ);
      }
#اگر_ختم
      d2 = check_درخت_صفحہ(pCheck, pgno, &max_کلید, max_کلید);
      کلید_Can_Be_Equal = 0;
      اگر( d2!=گہرائی ){
        check_Append_پیغام(pCheck, "Child page depth differs");
        گہرائی = d2;
      }
    }ورنہ{
      /* Populate the coverage-checking heap for leaf pages */
      أكد( ڈھیر!=0 && ڈھیر[0] < pCheck->mx_ڈھیر );
      btree_ڈھیر_داخل_کرو(ڈھیر, (pc<<16)|(pc+info.n_سائز-1));
    }
  }
  *pi_Min_کلید = max_کلید;

  /* Check for complete coverage of the page
  */
  pCheck->zPfx = 0;
  اگر( doCoverageCheck && pCheck->mxErr>0 ){
    /* For leaf pages, the min-heap has already been initialized and the
    ** cells have already been inserted.  But for internal pages, that has
    ** not yet been done, so do it now */
    اگر( !p_صفحہ->برگ ){
      ڈھیر = pCheck->ڈھیر;
      ڈھیر[0] = 0;
      برائے(i=nCell-1; i>=0; i--){
        u32 سائز;
        pc = get2byteAligned(&ڈیٹا[cell_آغاز+i*2]);
        سائز = p_صفحہ->x_Cell_سائز(p_صفحہ, &ڈیٹا[pc]);
        أكد( ڈھیر!=0 && ڈھیر[0] < pCheck->mx_ڈھیر );
        btree_ڈھیر_داخل_کرو(ڈھیر, (pc<<16)|(pc+سائز-1));
      }
    }
    أكد( ڈھیر!=0 );
    /* Add the freeblocks to the min-heap
    **
    ** EVIDENCE-OF: R-20690-50594 The second field of the b-tree page header
    ** is the offset of the first freeblock, or zero if there are no
    ** freeblocks on the page.
    */
    i = get2byte(&ڈیٹا[hdr+1]);
    جب( i>0 ){
      صحیح سائز, j;
      أكد( (u32)i<=usable_سائز-4 ); /* Enforced by btreeComputeFreeSpace() */
      سائز = get2byte(&ڈیٹا[i+2]);
      أكد( (u32)(i+سائز)<=usable_سائز ); /* due to btreeComputeFreeSpace() */
      أكد( ڈھیر!=0 && ڈھیر[0] < pCheck->mx_ڈھیر );
      btree_ڈھیر_داخل_کرو(ڈھیر, (((u32)i)<<16)|(i+سائز-1));
      /* EVIDENCE-OF: R-58208-19414 The first 2 bytes of a freeblock are a
      ** big-endian integer which is the offset in the b-tree page of the next
      ** freeblock in the chain, or zero if the freeblock is the last on the
      ** chain. */
      j = get2byte(&ڈیٹا[i]);
      /* EVIDENCE-OF: R-06866-39125 Freeblocks are always connected in order of
      ** increasing offset. */
      أكد( j==0 || j>i+سائز );     /* Enforced by btreeComputeFreeSpace() */
      أكد( (u32)j<=usable_سائز-4 ); /* Enforced by btreeComputeFreeSpace() */
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
    پچھلا = content_انحراف - 1;   /* Implied first min-heap entry */
    جب( btree_ڈھیر_Pull(ڈھیر,&x) ){
      اگر( (پچھلا&0xffff)>=(x>>16) ){
        check_Append_پیغام(pCheck,
          "Multiple uses for byte %u of page %u", x>>16, i_صفحہ);
        توڑو;
      }ورنہ{
        nFrag += (x>>16) - (پچھلا&0xffff) - 1;
        پچھلا = x;
      }
    }
    nFrag += usable_سائز - (پچھلا&0xffff) - 1;
    /* EVIDENCE-OF: R-43263-13491 The total number of bytes in all fragments
    ** is stored in the fifth field of the b-tree page header.
    ** EVIDENCE-OF: R-07161-27322 The one-byte integer at offset 7 gives the
    ** number of fragmented free bytes within the cell content area.
    */
    اگر( ڈھیر[0]==0 && nFrag!=ڈیٹا[hdr+7] ){
      check_Append_پیغام(pCheck,
          "Fragmentation of %u bytes reported as %u on page %u",
          nFrag, ڈیٹا[hdr+7], i_صفحہ);
    }
  }

انتہا_کا_check:
  اگر( !doCoverageCheck ) p_صفحہ->isInit = savedIsInit;
  release_صفحہ(p_صفحہ);
  pCheck->zPfx = saved_zPfx;
  pCheck->v1 = saved_v1;
  pCheck->v2 = saved_v2;
  pCheck->nAbove--;
  واپس گہرائی+1;
}
#اگر_ختم /* SQLITE_OMIT_INTEGRITY_CHECK */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INTEGRITY_CHECK
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
صحیح sqlite3BtreeIntegrityCheck(
  sqlite3 *db,  /* Database connection that is running the check */
  Btree *p,     /* The btree to be checked */
  Pgno *a_جڑ,  /* An array of root pages numbers for individual trees */
  Mem *aCnt,    /* Memory cells to write counts for each tree to */
  صحیح n_جڑ,    /* Number of entries in aRoot[] */
  صحیح mxErr,    /* Stop reporting errors after this many */
  صحیح *pnErr,   /* OUT: Write number of errors seen to this variable */
  حرف **pz_برآمد  /* OUT: Write the error message string here */
){
  Pgno i;
  IntegrityCk sCheck;
  BtShared *pBt = p->pBt;
  u64 savedDbFlags = pBt->db->flags;
  حرف zErr[100];
  صحیح bPartial = 0;            /* True if not checking all btrees */
  صحیح bCkFreelist = 1;         /* True to scan the freelist */
  VVA_ONLY( صحیح nRef );

  أكد( n_جڑ>0 );
  أكد( aCnt!=0 );

  /* aRoot[0]==0 means this is a partial check */
  اگر( a_جڑ[0]==0 ){
    أكد( n_جڑ>1 );
    bPartial = 1;
    اگر( a_جڑ[1]!=1 ) bCkFreelist = 0;
  }

  sqlite3BtreeEnter(p);
  أكد( p->اندر_Trans>TRANS_NONE && pBt->اندر_Transaction>TRANS_NONE );
  VVA_ONLY( nRef = sqlite3PagerRefcount(pBt->pPager) );
  أكد( nRef>=0 );
  memset(&sCheck, 0, حجم(sCheck));
  sCheck.db = db;
  sCheck.pBt = pBt;
  sCheck.pPager = pBt->pPager;
  sCheck.n_Ck_صفحہ = btreePagecount(sCheck.pBt);
  sCheck.mxErr = mxErr;
  sqlite3StrAccumInit(&sCheck.err_پیغام, 0, zErr, حجم(zErr), SQLITE_MAX_لمبائی);
  sCheck.err_پیغام.printfFlags = SQLITE_PRINTF_اندرونی;
  اگر( sCheck.n_Ck_صفحہ==0 ){
    جاؤ integrity_ck_cleanup;
  }

  sCheck.aPgRef = sqlite3MallocZero((sCheck.n_Ck_صفحہ / 8)+ 1);
  اگر( !sCheck.aPgRef ){
    checkOom(&sCheck);
    جاؤ integrity_ck_cleanup;
  }
  sCheck.ڈھیر = (u32*)sqlite3Malloc( pBt->صفحہ_سائز*2 );
#اگر_تعریف_شدہ SQLITE_DEBUG
  sCheck.mx_ڈھیر = pBt->صفحہ_سائز/2 - 1;
#اگر_ختم
  اگر( sCheck.ڈھیر==0 ){
    checkOom(&sCheck);
    جاؤ integrity_ck_cleanup;
  }

  i = PENDING_BYTE_صفحہ(pBt);
  اگر( i<=sCheck.n_Ck_صفحہ ) سیٹ_صفحہ_Referenced(&sCheck, i);

  /* Check the integrity of the freelist
  */
  اگر( bCkFreelist ){
    sCheck.zPfx = "Freelist: ";
    checkList(&sCheck, 1, get4byte(&pBt->pPage1->a_ڈیٹا[32]),
              get4byte(&pBt->pPage1->a_ڈیٹا[36]));
    sCheck.zPfx = 0;
  }

  /* Check all the tables.
  */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  اگر( !bPartial ){
    اگر( pBt->autoVacuum ){
      Pgno mx = 0;
      Pgno mx_اندر_Hdr;
      برائے(i=0; (صحیح)i<n_جڑ; i++) اگر( mx<a_جڑ[i] ) mx = a_جڑ[i];
      mx_اندر_Hdr = get4byte(&pBt->pPage1->a_ڈیٹا[52]);
      اگر( mx!=mx_اندر_Hdr ){
        check_Append_پیغام(&sCheck,
          "max rootpage (%u) disagrees with header (%u)",
          mx, mx_اندر_Hdr
        );
      }
    }ورنہ اگر( get4byte(&pBt->pPage1->a_ڈیٹا[64])!=0 ){
      check_Append_پیغام(&sCheck,
        "incremental_vacuum enabled with a max rootpage of zero"
      );
    }
  }
#اگر_ختم
  testcase( pBt->db->flags & SQLITE_Cell_سائز_Ck );
  pBt->db->flags &= ~(u64)SQLITE_Cell_سائز_Ck;
  برائے(i=0; (صحیح)i<n_جڑ && sCheck.mxErr; i++){
    sCheck.n_صف = 0;
    اگر( a_جڑ[i] ){
      i64 not_مستعمل;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
      اگر( pBt->autoVacuum && a_جڑ[i]>1 && !bPartial ){
        checkPtrmap(&sCheck, a_جڑ[i], PTRMAP_ROOTPAGE, 0);
      }
#اگر_ختم
      sCheck.v0 = a_جڑ[i];
      أكد( sCheck.nAbove==0 );
      check_درخت_صفحہ(&sCheck, a_جڑ[i], &not_مستعمل, سب_سے_بڑا_INT64);
      أكد( sCheck.nAbove==0 );
    }
    sqlite3_Mem_سیٹ_Array_Int64(aCnt, i, sCheck.n_صف);
  }
  pBt->db->flags = savedDbFlags;

  /* Make sure every page in the file is referenced
  */
  اگر( !bPartial ){
    برائے(i=1; i<=sCheck.n_Ck_صفحہ && sCheck.mxErr; i++){
#اگر_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
      اگر( get_صفحہ_Referenced(&sCheck, i)==0 ){
        check_Append_پیغام(&sCheck, "Page %u: never used", i);
      }
#ورنہ
      /* If the database supports auto-vacuum, make sure no tables contain
      ** references to pointer-map pages.
      */
      اگر( get_صفحہ_Referenced(&sCheck, i)==0 &&
         (PTRMAP_PAGENO(pBt, i)!=i || !pBt->autoVacuum) ){
        check_Append_پیغام(&sCheck, "Page %u: never used", i);
      }
      اگر( get_صفحہ_Referenced(&sCheck, i)!=0 &&
         (PTRMAP_PAGENO(pBt, i)==i && pBt->autoVacuum) ){
        check_Append_پیغام(&sCheck, "Page %u: pointer map referenced", i);
      }
#اگر_ختم
    }
  }

  /* Clean  up and report errors.
  */
integrity_ck_cleanup:
  sqlite3_free(sCheck.ڈھیر);
  sqlite3_free(sCheck.aPgRef);
  *pnErr = sCheck.nErr;
  اگر( sCheck.nErr==0 ){
    sqlite3_str_reset(&sCheck.err_پیغام);
    *pz_برآمد = 0;
  }ورنہ{
    *pz_برآمد = sqlite3_Str_Accum_اختتام(&sCheck.err_پیغام);
  }
  /* Make sure this analysis did not leave any unref() pages. */
  أكد( nRef==sqlite3PagerRefcount(pBt->pPager) );
  sqlite3BtreeLeave(p);
  واپس sCheck.rc;
}
#اگر_ختم /* SQLITE_OMIT_INTEGRITY_CHECK */

/*
** Return the full pathname of the underlying database file.  Return
** an empty string if the database is in-memory or a TEMP database.
**
** The pager filename is invariant as long as the pager is
** open so it is safe to access without the BtShared mutex.
*/
ثابت حرف *sqlite3BtreeGetFilename(Btree *p){
  أكد( p->pBt->pPager!=0 );
  واپس sqlite3PagerFilename(p->pBt->pPager, 1);
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
  واپس sqlite3PagerJournalname(p->pBt->pPager);
}

/*
** Return one of SQLITE_TXN_NONE, SQLITE_TXN_READ, or SQLITE_TXN_WRITE
** to describe the current transaction state of Btree p.
*/
صحیح sqlite3BtreeTxnState(Btree *p){
  أكد( p==0 || sqlite3_mutex_held(p->db->mutex) );
  واپس p ? p->اندر_Trans : 0;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
/*
** Run a checkpoint on the Btree passed as the first argument.
**
** Return SQLITE_LOCKED if this or any other connection has an open
** transaction on the shared-cache the argument Btree is connected to.
**
** Parameter eMode is one of SQLITE_CHECKPOINT_PASSIVE, FULL or RESTART.
*/
صحیح sqlite3BtreeCheckpoint(Btree *p, صحیح e_منوال, صحیح *pnLog, صحیح *pnCkpt){
  صحیح rc = SQLITE_OK;
  اگر( p ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
    اگر( pBt->اندر_Transaction!=TRANS_NONE ){
      rc = SQLITE_LOCKED;
    }ورنہ{
      rc = sqlite3PagerCheckpoint(pBt->pPager, p->db, e_منوال, pnLog, pnCkpt);
    }
    sqlite3BtreeLeave(p);
  }
  واپس rc;
}
#اگر_ختم

/*
** Return true if there is currently a backup running on Btree p.
*/
صحیح sqlite3_Btree_Is_اندر_Backup(Btree *p){
  أكد( p );
  أكد( sqlite3_mutex_held(p->db->mutex) );
  واپس p->nBackup!=0;
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
خالی *sqlite3BtreeSchema(Btree *p, صحیح n_بائٹیں, خالی(*xFree)(خالی *)){
  BtShared *pBt = p->pBt;
  أكد( n_بائٹیں==0 || n_بائٹیں==حجم(Schema) );
  sqlite3BtreeEnter(p);
  اگر( !pBt->pSchema && n_بائٹیں ){
    pBt->pSchema = sqlite3DbMallocZero(0, n_بائٹیں);
    pBt->xFreeSchema = xFree;
  }
  sqlite3BtreeLeave(p);
  واپس pBt->pSchema;
}

/*
** Return SQLITE_LOCKED_SHAREDCACHE if another user of the same shared
** btree as the argument handle holds an exclusive lock on the
** sqlite_schema table. Otherwise SQLITE_OK.
*/
صحیح sqlite3BtreeSchemaLocked(Btree *p){
  صحیح rc;
  غیر_مستعمل_PARAMETER(p);  /* only used in DEBUG builds */
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  rc = query_Shared_Cache_جدول_تالا(p, SCHEMA_جڑ, READ_تالا);
  أكد( rc==SQLITE_OK || rc==SQLITE_LOCKED_SHAREDCACHE );
  sqlite3BtreeLeave(p);
  واپس rc;
}


#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
/*
** Obtain a lock on the table whose root page is iTab.  The
** lock is a write lock if isWritelock is true or a read lock
** if it is false.
*/
صحیح sqlite3_Btree_تالا_جدول(Btree *p, صحیح iTab, u8 is_Write_تالا){
  صحیح rc = SQLITE_OK;
  أكد( p->اندر_Trans!=TRANS_NONE );
  اگر( p->sharable ){
    u8 تالا_Type = READ_تالا + is_Write_تالا;
    أكد( READ_تالا+1==WRITE_تالا );
    أكد( is_Write_تالا==0 || is_Write_تالا==1 );

    sqlite3BtreeEnter(p);
    rc = query_Shared_Cache_جدول_تالا(p, iTab, تالا_Type);
    اگر( rc==SQLITE_OK ){
      rc = سیٹ_Shared_Cache_جدول_تالا(p, iTab, تالا_Type);
    }
    sqlite3BtreeLeave(p);
  }
  واپس rc;
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
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
صحیح sqlite3_Btree_Put_ڈیٹا(BtCursor *pCsr, u32 انحراف, u32 amt, خالی *z){
  صحیح rc;
  أكد( cursorOwnsBtShared(pCsr) );
  أكد( sqlite3_mutex_held(pCsr->pBtree->db->mutex) );
  أكد( pCsr->curFlags & BTCF_Incrblob );

  rc = restoreCursorPosition(pCsr);
  اگر( rc!=SQLITE_OK ){
    واپس rc;
  }
  أكد( pCsr->eState!=CURSOR_REQUIRESEEK );
  اگر( pCsr->eState!=CURSOR_VALID ){
    واپس SQLITE_ABORT;
  }

  /* Save the positions of all other cursors open on this table. This is
  ** required in case any of them are holding references to an xFetch
  ** version of the b-tree page modified by the accessPayload call below.
  **
  ** Note that pCsr must be open on a INTKEY table and saveCursorPosition()
  ** and hence saveAllCursors() cannot fail on a BTREE_INTKEY table, hence
  ** saveAllCursors can only return SQLITE_OK.
  */
  VVA_ONLY(rc =) saveAllCursors(pCsr->pBt, pCsr->pgno_جڑ, pCsr);
  أكد( rc==SQLITE_OK );

  /* Check some assumptions:
  **   (a) the cursor is open for writing,
  **   (b) there is a read/write transaction open,
  **   (c) the connection holds a write-lock on the table (if required),
  **   (d) there are no conflicting read-locks, and
  **   (e) the cursor points at a valid row of an intKey table.
  */
  اگر( (pCsr->curFlags & BTCF_WriteFlag)==0 ){
    واپس SQLITE_READONLY;
  }
  أكد( (pCsr->pBt->btsFlags & BTS_READ_ONLY)==0
              && pCsr->pBt->اندر_Transaction==TRANS_WRITE );
  أكد( has_Shared_Cache_جدول_تالا(pCsr->pBtree, pCsr->pgno_جڑ, 0, 2) );
  أكد( !hasReadConflicts(pCsr->pBtree, pCsr->pgno_جڑ) );
  أكد( pCsr->p_صفحہ->int_کلید );

  واپس accessPayload(pCsr, انحراف, amt, (غیرعلامتی حرف *)z, 1);
}

/*
** Mark this cursor as an incremental blob cursor.
*/
خالی sqlite3BtreeIncrblobCursor(BtCursor *pCur){
  pCur->curFlags |= BTCF_Incrblob;
  pCur->pBtree->hasIncrblobCur = 1;
}
#اگر_ختم

/*
** Set both the "read version" (single byte at byte offset 18) and
** "write version" (single byte at byte offset 19) fields in the database
** header to iVersion.
*/
صحیح sqlite3_Btree_سیٹ_نسخہ(Btree *pBtree, صحیح i_نسخہ){
  BtShared *pBt = pBtree->pBt;
  صحیح rc;                         /* Return code */

  أكد( i_نسخہ==1 || i_نسخہ==2 );

  /* If setting the version fields to 1, do not automatically open the
  ** WAL connection, even if the version fields are currently set to 2.
  */
  pBt->btsFlags &= ~BTS_NO_WAL;
  اگر( i_نسخہ==1 ) pBt->btsFlags |= BTS_NO_WAL;

  rc = sqlite3BtreeBeginTrans(pBtree, 0, 0);
  اگر( rc==SQLITE_OK ){
    u8 *a_ڈیٹا = pBt->pPage1->a_ڈیٹا;
    اگر( a_ڈیٹا[18]!=(u8)i_نسخہ || a_ڈیٹا[19]!=(u8)i_نسخہ ){
      rc = sqlite3BtreeBeginTrans(pBtree, 2, 0);
      اگر( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->p_Db_صفحہ);
        اگر( rc==SQLITE_OK ){
          a_ڈیٹا[18] = (u8)i_نسخہ;
          a_ڈیٹا[19] = (u8)i_نسخہ;
        }
      }
    }
  }

  pBt->btsFlags &= ~BTS_NO_WAL;
  واپس rc;
}

/*
** Return true if the cursor has a hint specified.  This routine is
** only used from within assert() statements
*/
صحیح sqlite3BtreeCursorHasHint(BtCursor *pCsr, غیرعلامتی صحیح نقاب){
  واپس (pCsr->hints & نقاب)!=0;
}

/*
** Return true if the given Btree is read-only.
*/
صحیح sqlite3BtreeIsReadonly(Btree *p){
  واپس (p->pBt->btsFlags & BTS_READ_ONLY)!=0;
}

/*
** Return the size of the header added to each page by this module.
*/
صحیح sqlite3_سرخی_سائز_Btree(خالی){ واپس ROUND8(حجم(Mem_صفحہ)); }

/*
** If no transaction is active and the database is not a temp-db, clear
** the in-memory pager cache.
*/
خالی sqlite3BtreeClearCache(Btree *p){
  BtShared *pBt = p->pBt;
  اگر( pBt->اندر_Transaction==TRANS_NONE ){
    sqlite3PagerClearCache(pBt->pPager);
  }
}

#اگر !defined(SQLITE_OMIT_SHARED_CACHE)
/*
** Return true if the Btree passed as the only argument is sharable.
*/
صحیح sqlite3BtreeSharable(Btree *p){
  واپس p->sharable;
}

/*
** Return the number of connections to the BtShared object accessed by
** the Btree handle passed as the only argument. For private caches
** this is always 1. For shared caches it may be 1 or greater.
*/
صحیح sqlite3_Btree_Connection_گنتی(Btree *p){
  testcase( p->sharable );
  واپس p->pBt->nRef;
}
#اگر_ختم

/* Re-enable GCC compiler warnings that were suppressed at the top
** of this source file to prevent annoying false-positives.
*/
#اگر defined(__GNUC__) && __GNUC__>=11
# pragma GCC diagnostic نکالو
#اگر_ختم
