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
# pragma GCC diagnostic ڈالو
# pragma GCC diagnostic ignored "-Wstringop-overread"
# pragma GCC diagnostic ignored "-Wstringop-overflow"
#جے_انت

/*
** The header string that appears at the beginning of every
** SQLite database.
*/
ساكن ثابت حرف z_Magic_سرخی[] = SQLITE_FILE_سرخی;

/*
** Set this global variable to 1 to enable tracing using the TRACE
** macro.
*/
#إذا 0
صحيح sqlite3BtreeTrace=1;  /* True to enable tracing */
# define TRACE(X)  إذا(sqlite3BtreeTrace){لکھو X;fflush(stdout);}
#وإلا
# define TRACE(X)
#جے_انت

/*
** Extract a 2-byte big-endian integer from an array of unsigned bytes.
** But if the value is zero, make it 65536.
**
** This routine is used to extract the "offset to cell content area" value
** from the header of a btree page.  If the page size is 65536 and the page
** is empty, the offset should be 65536, but the 2-byte value stores zero.
** This routine makes the necessary adjustment to 65536.
*/
#تعریف_کرو get2byteNotZero(X)  (((((صحيح)get2byte(X))-1)&0xffff)+1)

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
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
#تعریف_کرو IfNotOmitAV(اظہاریہ_2) (اظہاریہ_2)
#وإلا
#تعریف_کرو IfNotOmitAV(اظہاریہ_2) 0
#جے_انت

#جے_بنا_تعریف SQLITE_OMIT_SHARED_CACHE
/*
** A list of BtShared objects that are eligible for participation
** in shared cache.  This variable has file scope during normal builds,
** but the test harness needs to access it so we make it global for
** test builds.
**
** Access to this variable is protected by SQLITE_MUTEX_STATIC_MAIN.
*/
#جے_تعریف_شدہ SQLITE_TEST
BtShared *SQLITE_WSD sqlite3SharedCacheList = 0;
#وإلا
ساكن BtShared *SQLITE_WSD sqlite3SharedCacheList = 0;
#جے_انت
#جے_انت /* SQLITE_OMIT_SHARED_CACHE */

#جے_بنا_تعریف SQLITE_OMIT_SHARED_CACHE
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
#جے_انت



#جے_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
  /*
  ** The functions querySharedCacheTableLock(), setSharedCacheTableLock(),
  ** and clearAllSharedCacheTableLocks()
  ** manipulate entries in the BtShared.pLock linked list used to store
  ** shared-cache table level locks. If the library is compiled with the
  ** shared-cache feature disabled, then there is only ever one user
  ** of each BtShared structure and so this locking is not necessary.
  ** So define the lock related functions as no-ops.
  */
  #تعریف_کرو query_Shared_Cache_جدول_جندرا(a,b,c) SQLITE_OK
  #تعریف_کرو سیٹ_Shared_Cache_جدول_جندرا(a,b,c) SQLITE_OK
  #تعریف_کرو clear_All_Shared_Cache_جدول_Locks(a)
  #تعریف_کرو downgrade_All_Shared_Cache_جدول_Locks(a)
  #تعریف_کرو has_Shared_Cache_جدول_جندرا(a,b,c,d) 1
  #تعریف_کرو hasReadConflicts(a, b) 0
#جے_انت

#جے_تعریف_شدہ SQLITE_DEBUG
/*
** Return and reset the seek counter for a Btree object.
*/
sqlite3_uint64 sqlite3_Btree_Seek_گنتی(Btree *pBt){
  u64 n =  pBt->nSeek;
  pBt->nSeek = 0;
  أرجع n;
}
#جے_انت

/*
** Implementation of the SQLITE_CORRUPT_PAGE() macro. Takes a single
** (MemPage*) as an argument. The (MemPage*) must not be NULL.
**
** If SQLITE_DEBUG is not defined, then this macro is equivalent to
** SQLITE_CORRUPT_BKPT. Or, if SQLITE_DEBUG is set, then the log message
** normally produced as a side-effect of SQLITE_CORRUPT_BKPT is augmented
** with the page number and filename associated with the (MemPage*).
*/
#جے_تعریف_شدہ SQLITE_DEBUG
صحيح corrupt_ورقہ_Error(صحيح lineno, Mem_ورقہ *p){
  حرف *z_پیغام;
  sqlite3BeginBenignMalloc();
  z_پیغام = sqlite3_mprintf("database corruption page %u of %s",
             p->pgno, sqlite3PagerFilename(p->pBt->pPager, 0)
  );
  sqlite3_سرا_Benign_Malloc();
  إذا( z_پیغام ){
    sqlite3ReportError(SQLITE_CORRUPT, lineno, z_پیغام);
  }
  sqlite3_free(z_پیغام);
  أرجع SQLITE_CORRUPT_BKPT;
}
# define SQLITE_CORRUPT_ورقہ(p_Mem_ورقہ) corrupt_ورقہ_Error(__LINE__, p_Mem_ورقہ)
#وإلا
# define SQLITE_CORRUPT_ورقہ(p_Mem_ورقہ) SQLITE_CORRUPT_PGNO(p_Mem_ورقہ->pgno)
#جے_انت

/* Default value for SHARED_LOCK_TRACE macro if shared-cache is disabled
** or if the lock tracking is disabled.  This is always the value for
** release builds.
*/
#تعریف_کرو SHARED_جندرا_TRACE(X,پیغام,TAB,TYPE)  /*no-op*/

#جے_بنا_تعریف SQLITE_OMIT_SHARED_CACHE

#إذا 0
/*  ^----  Change to 1 and recompile to enable shared-lock tracing
**         for debugging purposes.
**
** Print all shared-cache locks on a BtShared.  Debugging use only.
*/
ساكن فراغ shared_جندرا_Trace(
  BtShared *pBt,
  ثابت حرف *z_پیغام,
  صحيح i_جڑ,
  صحيح e_جندرا_Type
){
  Bt_جندرا *p_جندرا;
  إذا( i_جڑ>0 ){
    لکھو("%s-%p %u%s:", z_پیغام, pBt, i_جڑ, e_جندرا_Type==READ_جندرا?"R":"W");
  }وإلا{
    لکھو("%s-%p:", z_پیغام, pBt);
  }
  لكل(p_جندرا=pBt->p_جندرا; p_جندرا; p_جندرا=p_جندرا->p_اگلا){
    لکھو(" %p/%u%s", p_جندرا->pBtree, p_جندرا->i_جدول,
           p_جندرا->e_جندرا==READ_جندرا ? "R" : "W");
    طالما( p_جندرا->p_اگلا && p_جندرا->pBtree==p_جندرا->p_اگلا->pBtree ){
      p_جندرا = p_جندرا->p_اگلا;
      لکھو(",%u%s", p_جندرا->i_جدول, p_جندرا->e_جندرا==READ_جندرا ? "R" : "W");
    }
  }
  لکھو("\n");
  fflush(stdout);
}
#تعریف_رد SHARED_جندرا_TRACE
#تعریف_کرو SHARED_جندرا_TRACE(X,پیغام,TAB,TYPE)  shared_جندرا_Trace(X,پیغام,TAB,TYPE)
#جے_انت /* Shared-lock tracing */

#جے_تعریف_شدہ SQLITE_DEBUG
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
ساكن صحيح has_Shared_Cache_جدول_جندرا(
  Btree *pBtree,         /* Handle that must hold lock */
  Pgno i_جڑ,            /* Root page of b-tree */
  صحيح is_اشاریہ,           /* True if iRoot is the root of an index b-tree */
  صحيح e_جندرا_Type          /* Required lock type (READ_LOCK or WRITE_LOCK) */
){
  Schema *pSchema = (Schema *)pBtree->pBt->pSchema;
  Pgno iTab = 0;
  Bt_جندرا *p_جندرا;

  /* If this database is not shareable, or if the client is reading
  ** and has the read-uncommitted flag set, then no lock is required.
  ** Return true immediately.
  */
  إذا( (pBtree->sharable==0)
   || (e_جندرا_Type==READ_جندرا && (pBtree->db->flags & SQLITE_ReadUncommit))
  ){
    أرجع 1;
  }

  /* If the client is reading  or writing an index and the schema is
  ** not loaded, then it is too difficult to actually check to see if
  ** the correct locks are held.  So do not bother - just return true.
  ** This case does not come up very often anyhow.
  */
  إذا( is_اشاریہ && (!pSchema || (pSchema->schemaFlags&DB_SchemaLoaded)==0) ){
    أرجع 1;
  }

  /* Figure out the root-page that the lock should be held on. For table
  ** b-trees, this is just the root page of the b-tree being read or
  ** written. For index b-trees, it is the root page of the associated
  ** table.  */
  إذا( is_اشاریہ ){
    HashElem *p;
    صحيح b_تکیا = 0;
    لكل(p=sqlite_Hash_پہلا(&pSchema->idxHash); p; p=sqlite_Hash_اگلا(p)){
      اشاریہ *pIdx = (اشاریہ *)sqlite_Hash_انکڑے(p);
      إذا( pIdx->tnum==i_جڑ ){
        إذا( b_تکیا ){
          /* Two or more indexes share the same root page.  There must
          ** be imposter tables.  So just return true.  The assert is not
          ** useful in that case. */
          أرجع 1;
        }
        iTab = pIdx->p_جدول->tnum;
        b_تکیا = 1;
      }
    }
  }وإلا{
    iTab = i_جڑ;
  }

  SHARED_جندرا_TRACE(pBtree->pBt,"hasLock",i_جڑ,e_جندرا_Type);

  /* Search for the required lock. Either a write-lock on root-page iTab, a
  ** write-lock on the schema table, or (if the client is reading) a
  ** read-lock on iTab will suffice. Return 1 if any of these are found.  */
  لكل(p_جندرا=pBtree->pBt->p_جندرا; p_جندرا; p_جندرا=p_جندرا->p_اگلا){
    إذا( p_جندرا->pBtree==pBtree
     && (p_جندرا->i_جدول==iTab || (p_جندرا->e_جندرا==WRITE_جندرا && p_جندرا->i_جدول==1))
     && p_جندرا->e_جندرا>=e_جندرا_Type
    ){
      أرجع 1;
    }
  }

  /* Failed to find the required lock. */
  أرجع 0;
}
#جے_انت /* SQLITE_DEBUG */

#جے_تعریف_شدہ SQLITE_DEBUG
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
ساكن صحيح hasReadConflicts(Btree *pBtree, Pgno i_جڑ){
  BtCursor *p;
  لكل(p=pBtree->pBt->pCursor; p; p=p->p_اگلا){
    إذا( p->pgno_جڑ==i_جڑ
     && p->pBtree!=pBtree
     && 0==(p->pBtree->db->flags & SQLITE_ReadUncommit)
    ){
      أرجع 1;
    }
  }
  أرجع 0;
}
#جے_انت    /* #ifdef SQLITE_DEBUG */

/*
** Query to see if Btree handle p may obtain a lock of type eLock
** (READ_LOCK or WRITE_LOCK) on the table with root-page iTab. Return
** SQLITE_OK if the lock may be obtained (by calling
** setSharedCacheTableLock()), or SQLITE_LOCKED if not.
*/
ساكن صحيح query_Shared_Cache_جدول_جندرا(Btree *p, Pgno iTab, u8 e_جندرا){
  BtShared *pBt = p->pBt;
  Bt_جندرا *pIter;

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( e_جندرا==READ_جندرا || e_جندرا==WRITE_جندرا );
  أكد( p->db!=0 );
  أكد( !(p->db->flags&SQLITE_ReadUncommit)||e_جندرا==WRITE_جندرا||iTab==1 );
 
  /* If requesting a write-lock, then the Btree must have an open write
  ** transaction on this file. And, obviously, for this to be so there
  ** must be an open write transaction on the file itself.
  */
  أكد( e_جندرا==READ_جندرا || (p==pBt->pWriter && p->وچ_Trans==TRANS_WRITE) );
  أكد( e_جندرا==READ_جندرا || pBt->وچ_Transaction==TRANS_WRITE );
 
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

  لكل(pIter=pBt->p_جندرا; pIter; pIter=pIter->p_اگلا){
    /* The condition (pIter->eLock!=eLock) in the following if(...)
    ** statement is a simplification of:
    **
    **   (eLock==WRITE_LOCK || pIter->eLock==WRITE_LOCK)
    **
    ** since we know that if eLock==WRITE_LOCK, then no other connection
    ** may hold a WRITE_LOCK on any table in this file (since there can
    ** only be a single writer).
    */
    أكد( pIter->e_جندرا==READ_جندرا || pIter->e_جندرا==WRITE_جندرا );
    أكد( e_جندرا==READ_جندرا || pIter->pBtree==p || pIter->e_جندرا==READ_جندرا);
    إذا( pIter->pBtree!=p && pIter->i_جدول==iTab && pIter->e_جندرا!=e_جندرا ){
      sqlite3ConnectionBlocked(p->db, pIter->pBtree->db);
      إذا( e_جندرا==WRITE_جندرا ){
        أكد( p==pBt->pWriter );
        pBt->btsFlags |= BTS_PENDING;
      }
      أرجع SQLITE_LOCKED_SHAREDCACHE;
    }
  }
  أرجع SQLITE_OK;
}
#جے_انت /* !SQLITE_OMIT_SHARED_CACHE */

#جے_بنا_تعریف SQLITE_OMIT_SHARED_CACHE
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
ساكن صحيح سیٹ_Shared_Cache_جدول_جندرا(Btree *p, Pgno i_جدول, u8 e_جندرا){
  BtShared *pBt = p->pBt;
  Bt_جندرا *p_جندرا = 0;
  Bt_جندرا *pIter;

  SHARED_جندرا_TRACE(pBt,"setLock", i_جدول, e_جندرا);

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( e_جندرا==READ_جندرا || e_جندرا==WRITE_جندرا );
  أكد( p->db!=0 );

  /* A connection with the read-uncommitted flag set will never try to
  ** obtain a read-lock using this function. The only read-lock obtained
  ** by a connection in read-uncommitted mode is on the sqlite_schema
  ** table, and that lock is obtained in BtreeBeginTrans().  */
  أكد( 0==(p->db->flags&SQLITE_ReadUncommit) || e_جندرا==WRITE_جندرا );

  /* This function should only be called on a sharable b-tree after it
  ** has been determined that no other b-tree holds a conflicting lock.  */
  أكد( p->sharable );
  أكد( SQLITE_OK==query_Shared_Cache_جدول_جندرا(p, i_جدول, e_جندرا) );

  /* First search the list for an existing lock on this table. */
  لكل(pIter=pBt->p_جندرا; pIter; pIter=pIter->p_اگلا){
    إذا( pIter->i_جدول==i_جدول && pIter->pBtree==p ){
      p_جندرا = pIter;
      اكسر;
    }
  }

  /* If the above search did not find a BtLock struct associating Btree p
  ** with table iTable, allocate one and link it into the list.
  */
  إذا( !p_جندرا ){
    p_جندرا = (Bt_جندرا *)sqlite3MallocZero(حجم(Bt_جندرا));
    إذا( !p_جندرا ){
      أرجع SQLITE_NOMEM_BKPT;
    }
    p_جندرا->i_جدول = i_جدول;
    p_جندرا->pBtree = p;
    p_جندرا->p_اگلا = pBt->p_جندرا;
    pBt->p_جندرا = p_جندرا;
  }

  /* Set the BtLock.eLock variable to the maximum of the current lock
  ** and the requested lock. This means if a write-lock was already held
  ** and a read-lock requested, we don't incorrectly downgrade the lock.
  */
  أكد( WRITE_جندرا>READ_جندرا );
  إذا( e_جندرا>p_جندرا->e_جندرا ){
    p_جندرا->e_جندرا = e_جندرا;
  }

  أرجع SQLITE_OK;
}
#جے_انت /* !SQLITE_OMIT_SHARED_CACHE */

#جے_بنا_تعریف SQLITE_OMIT_SHARED_CACHE
/*
** Release all the table locks (locks obtained via calls to
** the setSharedCacheTableLock() procedure) held by Btree object p.
**
** This function assumes that Btree p has an open read or write
** transaction. If it does not, then the BTS_PENDING flag
** may be incorrectly cleared.
*/
ساكن فراغ clear_All_Shared_Cache_جدول_Locks(Btree *p){
  BtShared *pBt = p->pBt;
  Bt_جندرا **ppIter = &pBt->p_جندرا;

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( p->sharable || 0==*ppIter );
  أكد( p->وچ_Trans>0 );

  SHARED_جندرا_TRACE(pBt, "clearAllLocks", 0, 0);

  طالما( *ppIter ){
    Bt_جندرا *p_جندرا = *ppIter;
    أكد( (pBt->btsFlags & BTS_EXCLUSIVE)==0 || pBt->pWriter==p_جندرا->pBtree );
    أكد( p_جندرا->pBtree->وچ_Trans>=p_جندرا->e_جندرا );
    إذا( p_جندرا->pBtree==p ){
      *ppIter = p_جندرا->p_اگلا;
      أكد( p_جندرا->i_جدول!=1 || p_جندرا==&p->جندرا );
      إذا( p_جندرا->i_جدول!=1 ){
        sqlite3_free(p_جندرا);
      }
    }وإلا{
      ppIter = &p_جندرا->p_اگلا;
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
ساكن فراغ downgrade_All_Shared_Cache_جدول_Locks(Btree *p){
  BtShared *pBt = p->pBt;

  SHARED_جندرا_TRACE(pBt, "downgradeLocks", 0, 0);

  إذا( pBt->pWriter==p ){
    Bt_جندرا *p_جندرا;
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(BTS_EXCLUSIVE|BTS_PENDING);
    لكل(p_جندرا=pBt->p_جندرا; p_جندرا; p_جندرا=p_جندرا->p_اگلا){
      أكد( p_جندرا->e_جندرا==READ_جندرا || p_جندرا->pBtree==p );
      p_جندرا->e_جندرا = READ_جندرا;
    }
  }
}

#جے_انت /* SQLITE_OMIT_SHARED_CACHE */

ساكن فراغ release_ورقہ(Mem_ورقہ *p_ورقہ);         /* Forward reference */
ساكن فراغ release_ورقہ_One(Mem_ورقہ *p_ورقہ);      /* Forward reference */
ساكن فراغ release_ورقہ_Not_Null(Mem_ورقہ *p_ورقہ);  /* Forward reference */

/*
***** This routine is used inside of assert() only ****
**
** Verify that the cursor holds the mutex on its BtShared
*/
#جے_تعریف_شدہ SQLITE_DEBUG
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
#جے_انت

/*
** Invalidate the overflow cache of the cursor passed as the first argument.
** on the shared btree structure pBt.
*/
#تعریف_کرو invalidate_حد_توں_ودھ_Cache(pCur) (pCur->curFlags &= ~BTCF_ValidOvfl)

/*
** Invalidate the overflow page-list cache for all cursors opened
** on the shared btree structure pBt.
*/
ساكن فراغ invalidate_All_حد_توں_ودھ_Cache(BtShared *pBt){
  BtCursor *p;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  لكل(p=pBt->pCursor; p; p=p->p_اگلا){
    invalidate_حد_توں_ودھ_Cache(p);
  }
}

#جے_بنا_تعریف SQLITE_OMIT_INCRBLOB
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
  Pgno pgno_جڑ,          /* The table that might be changing */
  i64 i_صف,               /* The rowid that might be changing */
  صحيح is_Clear_جدول        /* True if all rows are being deleted */
){
  BtCursor *p;
  أكد( pBtree->hasIncrblobCur );
  أكد( sqlite3BtreeHoldsMutex(pBtree) );
  pBtree->hasIncrblobCur = 0;
  لكل(p=pBtree->pBt->pCursor; p; p=p->p_اگلا){
    إذا( (p->curFlags & BTCF_Incrblob)!=0 ){
      pBtree->hasIncrblobCur = 1;
      إذا( p->pgno_جڑ==pgno_جڑ && (is_Clear_جدول || p->info.n_کلید==i_صف) ){
        p->eState = CURSOR_INVALID;
      }
    }
  }
}

#وإلا
  /* Stub function when INCRBLOB is omitted */
  #تعریف_کرو invalidateIncrblobCursors(w,x,y,z)
#جے_انت /* SQLITE_OMIT_INCRBLOB */

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
ساكن صحيح btree_سیٹ_Has_Content(BtShared *pBt, Pgno pgno){
  صحيح rc = SQLITE_OK;
  إذا( !pBt->pHasContent ){
    أكد( pgno<=pBt->n_ورقہ );
    pBt->pHasContent = sqlite3BitvecCreate(pBt->n_ورقہ);
    إذا( !pBt->pHasContent ){
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  إذا( rc==SQLITE_OK && pgno<=sqlite3_Bitvec_ناپ(pBt->pHasContent) ){
    rc = sqlite3_Bitvec_سیٹ(pBt->pHasContent, pgno);
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
  أرجع p && (pgno>sqlite3_Bitvec_ناپ(p) || sqlite3BitvecTestNotNull(p, pgno));
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
ساكن فراغ btree_Release_All_Cursor_ورقے(BtCursor *pCur){
  صحيح i;
  إذا( pCur->i_ورقہ>=0 ){
    لكل(i=0; i<pCur->i_ورقہ; i++){
      release_ورقہ_Not_Null(pCur->ap_ورقہ[i]);
    }
    release_ورقہ_Not_Null(pCur->p_ورقہ);
    pCur->i_ورقہ = -1;
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
ساكن صحيح save_Cursor_کلید(BtCursor *pCur){
  صحيح rc = SQLITE_OK;
  أكد( CURSOR_VALID==pCur->eState );
  أكد( 0==pCur->p_کلید );
  أكد( cursorHoldsMutex(pCur) );

  إذا( pCur->cur_Int_کلید ){
    /* Only the rowid is required for a table btree */
    pCur->n_کلید = sqlite3_Btree_Integer_کلید(pCur);
  }وإلا{
    /* For an index btree, save the complete key content. It is possible
    ** that the current key is corrupt. In that case, it is possible that
    ** the sqlite3VdbeRecordUnpack() function may overread the buffer by
    ** up to the size of 1 varint plus 1 8-byte value when the cursor
    ** position is restored. Hence the 17 bytes of padding allocated
    ** below. */
    فراغ *p_کلید;
    pCur->n_کلید = sqlite3_Btree_Payload_ناپ(pCur);
    p_کلید = sqlite3Malloc( ((i64)pCur->n_کلید) + 9 + 8 );
    إذا( p_کلید ){
      rc = sqlite3BtreePayload(pCur, 0, (صحيح)pCur->n_کلید, p_کلید);
      إذا( rc==SQLITE_OK ){
        memset(((u8*)p_کلید)+pCur->n_کلید, 0, 9+8);
        pCur->p_کلید = p_کلید;
      }وإلا{
        sqlite3_free(p_کلید);
      }
    }وإلا{
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  أكد( !pCur->cur_Int_کلید || !pCur->p_کلید );
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
  أكد( 0==pCur->p_کلید );
  أكد( cursorHoldsMutex(pCur) );

  إذا( pCur->curFlags & BTCF_Pinned ){
    أرجع SQLITE_CONSTRAINT_PINNED;
  }
  إذا( pCur->eState==CURSOR_SKIPNEXT ){
    pCur->eState = CURSOR_VALID;
  }وإلا{
    pCur->skip_اگلا = 0;
  }

  rc = save_Cursor_کلید(pCur);
  إذا( rc==SQLITE_OK ){
    btree_Release_All_Cursor_ورقے(pCur);
    pCur->eState = CURSOR_REQUIRESEEK;
  }

  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl|BTCF_At_آخری);
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
ساكن صحيح saveAllCursors(BtShared *pBt, Pgno i_جڑ, BtCursor *pExcept){
  BtCursor *p;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( pExcept==0 || pExcept->pBt==pBt );
  لكل(p=pBt->pCursor; p; p=p->p_اگلا){
    إذا( p!=pExcept && (0==i_جڑ || p->pgno_جڑ==i_جڑ) ) اكسر;
  }
  إذا( p ) أرجع saveCursorsOnList(p, i_جڑ, pExcept);
  إذا( pExcept ) pExcept->curFlags &= ~BTCF_مضرب;
  أرجع SQLITE_OK;
}

/* This helper routine to saveAllCursors does the actual work of saving
** the cursors if and when a cursor is found that actually requires saving.
** The common case is that no cursors need to be saved, so this routine is
** broken out from its caller to avoid unnecessary stack pointer movement.
*/
ساكن صحيح SQLITE_NOINLINE saveCursorsOnList(
  BtCursor *p,         /* The first cursor that needs saving */
  Pgno i_جڑ,          /* Only save cursor with this iRoot. Save all if zero */
  BtCursor *pExcept    /* Do not save this cursor */
){
  افعل{
    إذا( p!=pExcept && (0==i_جڑ || p->pgno_جڑ==i_جڑ) ){
      إذا( p->eState==CURSOR_VALID || p->eState==CURSOR_SKIPNEXT ){
        صحيح rc = saveCursorPosition(p);
        إذا( SQLITE_OK!=rc ){
          أرجع rc;
        }
      }وإلا{
        testcase( p->i_ورقہ>=0 );
        btree_Release_All_Cursor_ورقے(p);
      }
    }
    p = p->p_اگلا;
  }طالما( p );
  أرجع SQLITE_OK;
}

/*
** Clear the current cursor position.
*/
فراغ sqlite3BtreeClearCursor(BtCursor *pCur){
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
ساكن صحيح btreeMoveto(
  BtCursor *pCur,     /* Cursor open on the btree to be searched */
  ثابت فراغ *p_کلید,   /* Packed key if the btree is an index */
  i64 n_کلید,           /* Integer key for tables.  Size of pKey for indices */
  صحيح bias,           /* Bias search to the high end */
  صحيح *pRes           /* Write search results here */
){
  صحيح rc;                    /* Status code */
  Unpacked_ریکارڈ *p_Idx_کلید;   /* Unpacked index key */

  إذا( p_کلید ){
    کلید_Info *p_کلید_Info = pCur->p_کلید_Info;
    أكد( n_کلید==(i64)(صحيح)n_کلید );
    p_Idx_کلید = sqlite3_Vdbe_Alloc_Unpacked_ریکارڈ(p_کلید_Info);
    إذا( p_Idx_کلید==0 ) أرجع SQLITE_NOMEM_BKPT;
    sqlite3_Vdbe_ریکارڈ_Unpack((صحيح)n_کلید, p_کلید, p_Idx_کلید);
    إذا( p_Idx_کلید->n_میدان==0 || p_Idx_کلید->n_میدان>p_کلید_Info->n_All_میدان ){
      rc = SQLITE_CORRUPT_BKPT;
    }وإلا{
      rc = sqlite3_Btree_اشاریہ_Moveto(pCur, p_Idx_کلید, pRes);
    }
    sqlite3DbFree(pCur->p_کلید_Info->db, p_Idx_کلید);
  }وإلا{
    p_Idx_کلید = 0;
    rc = sqlite3_Btree_جدول_Moveto(pCur, n_کلید, bias, pRes);
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
  صحيح skip_اگلا = 0;
  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState>=CURSOR_REQUIRESEEK );
  إذا( pCur->eState==CURSOR_FAULT ){
    أرجع pCur->skip_اگلا;
  }
  pCur->eState = CURSOR_INVALID;
  إذا( sqlite3FaultSim(410) ){
    rc = SQLITE_IOERR;
  }وإلا{
    rc = btreeMoveto(pCur, pCur->p_کلید, pCur->n_کلید, 0, &skip_اگلا);
  }
  إذا( rc==SQLITE_OK ){
    sqlite3_free(pCur->p_کلید);
    pCur->p_کلید = 0;
    أكد( pCur->eState==CURSOR_VALID || pCur->eState==CURSOR_INVALID );
    إذا( skip_اگلا ) pCur->skip_اگلا = skip_اگلا;
    إذا( pCur->skip_اگلا && pCur->eState==CURSOR_VALID ){
      pCur->eState = CURSOR_SKIPNEXT;
    }
  }
  أرجع rc;
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
صحيح sqlite3BtreeCursorRestore(BtCursor *pCur, صحيح *p_Different_صف){
  صحيح rc;

  أكد( pCur!=0 );
  أكد( pCur->eState!=CURSOR_VALID );
  rc = restoreCursorPosition(pCur);
  إذا( rc ){
    *p_Different_صف = 1;
    أرجع rc;
  }
  إذا( pCur->eState!=CURSOR_VALID ){
    *p_Different_صف = 1;
  }وإلا{
    *p_Different_صف = 0;
  }
  أرجع SQLITE_OK;
}

#جے_تعریف_شدہ SQLITE_ENABLE_CURSOR_HINTS
/*
** Provide hints to the cursor.  The particular hint given (and the type
** and number of the varargs parameters) is determined by the eHintType
** parameter.  See the definitions of the BTREE_HINT_* macros for details.
*/
فراغ sqlite3BtreeCursorHint(BtCursor *pCur, صحيح eHintType, ...){
  /* Used only by system that substitute their own storage engine */
#جے_تعریف_شدہ SQLITE_DEBUG
  va_list ap;
  va_start(ap, eHintType);
  إذا( eHintType==BTREE_HINT_RANGE ){
    اظہاریہ *p_اظہاریہ;
    Walker w;
    memset(&w, 0, حجم(w));
    w.x_اظہاریہ_Callback = sqlite3_Cursor_Range_Hint_اظہاریہ_Check;
    p_اظہاریہ = va_arg(ap, اظہاریہ*);
    w.u.aMem = va_arg(ap, Mem*);
    أكد( p_اظہاریہ!=0 );
    أكد( w.u.aMem!=0 );
    sqlite3_Walk_اظہاریہ(&w, p_اظہاریہ);
  }وإلا إذا( ALWAYS(eHintType==BTREE_HINT_TABLECURSOR) ){
    BtCursor *pCsr = va_arg(ap, BtCursor*);
    أكد( pCur->p_Cursor_Hint_جدول_Cursor==0 
         || pCur->p_Cursor_Hint_جدول_Cursor==pCsr
    );
    أكد( pCsr->p_کلید_Info==0 || CORRUPT_DB );
    pCur->p_Cursor_Hint_جدول_Cursor = pCsr;
  }
  va_end(ap);
#جے_انت /* SQLITE_DEBUG */
}
#جے_انت /* SQLITE_ENABLE_CURSOR_HINTS */

#إذا defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** Return the pointer configured via the BTREE_HINT_TABLECURSOR hint on
** cursor pCsr. This is used from OP_DeferredSeek to assert() that the
** index cursor has been correctly configured with the table cursor.
*/
BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor *pCsr){
  أرجع pCsr->p_Cursor_Hint_جدول_Cursor;
}
#جے_انت

/*
** Provide flag hints to the cursor.
*/
فراغ sqlite3BtreeCursorHintFlags(BtCursor *pCur, بنانشانی x){
  أكد( x==BTREE_SEEK_EQ || x==BTREE_BULKLOAD || x==0 );
  pCur->hints = (u8)x;
}


#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
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
  صحيح n_ورقے_فی_Map_ورقہ;
  Pgno iPtrMap, ret;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  إذا( pgno<2 ) أرجع 0;
  n_ورقے_فی_Map_ورقہ = (pBt->usable_ناپ/5)+1;
  iPtrMap = (pgno-2)/n_ورقے_فی_Map_ورقہ;
  ret = (iPtrMap*n_ورقے_فی_Map_ورقہ) + 2;
  إذا( ret==PENDING_BYTE_ورقہ(pBt) ){
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
ساكن فراغ ptrmapPut(BtShared *pBt, Pgno کلید, u8 eType, Pgno ماپا, صحيح *pRC){
  Db_ورقہ *p_Db_ورقہ;  /* The pointer map page */
  u8 *pPtrmap;      /* The pointer map data */
  Pgno iPtrmap;     /* The pointer map page number */
  صحيح وتھ;       /* Offset in pointer map page */
  صحيح rc;           /* Return code from subfunctions */

  إذا( *pRC ) أرجع;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  /* The super-journal page number must never be used as a pointer map page */
  أكد( 0==PTRMAP_ISPAGE(pBt, PENDING_BYTE_ورقہ(pBt)) );

  أكد( pBt->autoVacuum );
  إذا( کلید==0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    أرجع;
  }
  iPtrmap = PTRMAP_PAGENO(pBt, کلید);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &p_Db_ورقہ, 0);
  إذا( rc!=SQLITE_OK ){
    *pRC = rc;
    أرجع;
  }
  إذا( ((حرف*)sqlite3PagerGetExtra(p_Db_ورقہ))[0]!=0 ){
    /* The first byte of the extra data is the MemPage.isInit byte.
    ** If that byte is set, it means this page is also being used
    ** as a btree page. */
    *pRC = SQLITE_CORRUPT_BKPT;
    اذهب ptrmap_exit;
  }
  وتھ = PTRMAP_PTROFFSET(iPtrmap, کلید);
  إذا( وتھ<0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    اذهب ptrmap_exit;
  }
  أكد( وتھ <= (صحيح)pBt->usable_ناپ-5 );
  pPtrmap = (u8 *)sqlite3_Pager_Get_انکڑے(p_Db_ورقہ);

  إذا( eType!=pPtrmap[وتھ] || get4byte(&pPtrmap[وتھ+1])!=ماپا ){
    TRACE(("PTRMAP_UPDATE: %u->(%u,%u)\n", کلید, eType, ماپا));
    *pRC= rc = sqlite3PagerWrite(p_Db_ورقہ);
    إذا( rc==SQLITE_OK ){
      pPtrmap[وتھ] = eType;
      put4byte(&pPtrmap[وتھ+1], ماپا);
    }
  }

ptrmap_exit:
  sqlite3PagerUnref(p_Db_ورقہ);
}

/*
** Read an entry from the pointer map.
**
** This routine retrieves the pointer map entry for page 'key', writing
** the type and parent page number to *pEType and *pPgno respectively.
** An error code is returned if something goes wrong, otherwise SQLITE_OK.
*/
ساكن صحيح ptrmapGet(BtShared *pBt, Pgno کلید, u8 *pEType, Pgno *pPgno){
  Db_ورقہ *p_Db_ورقہ;   /* The pointer map page */
  صحيح iPtrmap;       /* Pointer map page index */
  u8 *pPtrmap;       /* Pointer map page data */
  صحيح وتھ;        /* Offset of entry in pointer map */
  صحيح rc;

  أكد( sqlite3_mutex_held(pBt->mutex) );

  iPtrmap = PTRMAP_PAGENO(pBt, کلید);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &p_Db_ورقہ, 0);
  إذا( rc!=0 ){
    أرجع rc;
  }
  pPtrmap = (u8 *)sqlite3_Pager_Get_انکڑے(p_Db_ورقہ);

  وتھ = PTRMAP_PTROFFSET(iPtrmap, کلید);
  إذا( وتھ<0 ){
    sqlite3PagerUnref(p_Db_ورقہ);
    أرجع SQLITE_CORRUPT_BKPT;
  }
  أكد( وتھ <= (صحيح)pBt->usable_ناپ-5 );
  أكد( pEType!=0 );
  *pEType = pPtrmap[وتھ];
  إذا( pPgno ) *pPgno = get4byte(&pPtrmap[وتھ+1]);

  sqlite3PagerUnref(p_Db_ورقہ);
  إذا( *pEType<1 || *pEType>5 ) أرجع SQLITE_CORRUPT_PGNO(iPtrmap);
  أرجع SQLITE_OK;
}

#وإلا /* if defined SQLITE_OMIT_AUTOVACUUM */
  #تعریف_کرو ptrmapPut(w,x,y,z,rc)
  #تعریف_کرو ptrmapGet(w,x,y,z) SQLITE_OK
  #تعریف_کرو ptrmapPutOvflPtr(x, y, z, rc)
#جے_انت

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
#تعریف_کرو لبھو_Cell(P,I) \
  ((P)->a_انکڑے + ((P)->نقاب_ورقہ & get2byteAligned(&(P)->aCellIdx[2*(I)])))
#تعریف_کرو لبھو_Cell_Past_Ptr(P,I) \
  ((P)->a_انکڑے_Ofst + ((P)->نقاب_ورقہ & get2byteAligned(&(P)->aCellIdx[2*(I)])))


/*
** This is common tail processing for btreeParseCellPtr() and
** btreeParseCellPtrIndex() for the case when the cell does not fit entirely
** on a single B-tree page.  Make necessary adjustments to the CellInfo
** structure.
*/
ساكن SQLITE_NOINLINE فراغ btree_Parse_Cell_Adjust_ناپ_For_حد_توں_ودھ(
  Mem_ورقہ *p_ورقہ,         /* Page containing the cell */
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

  minLocal = p_ورقہ->minLocal;
  maxLocal = p_ورقہ->maxLocal;
  surplus = minLocal + (pInfo->nPayload - minLocal)%(p_ورقہ->pBt->usable_ناپ-4);
  testcase( surplus==maxLocal );
  testcase( surplus==maxLocal+1 );
  إذا( surplus <= maxLocal ){
    pInfo->nLocal = (u16)surplus;
  }وإلا{
    pInfo->nLocal = (u16)minLocal;
  }
  pInfo->n_ناپ = (u16)(&pInfo->pPayload[pInfo->nLocal] - pCell) + 4;
}

/*
** Given a record with nPayload bytes of payload stored within btree
** page pPage, return the number of bytes of payload stored locally.
*/
ساكن صحيح btree_Payload_منزل_Local(Mem_ورقہ *p_ورقہ, i64 nPayload){
  صحيح maxLocal;  /* Maximum amount of payload held locally */
  maxLocal = p_ورقہ->maxLocal;
  أكد( nPayload>=0 );
  إذا( nPayload<=maxLocal ){
    أرجع (صحيح)nPayload;
  }وإلا{
    صحيح minLocal;  /* Minimum amount of payload held locally */
    صحيح surplus;   /* Overflow payload available for local storage */
    minLocal = p_ورقہ->minLocal;
    surplus = (صحيح)(minLocal +(nPayload - minLocal)%(p_ورقہ->pBt->usable_ناپ-4));
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
  Mem_ورقہ *p_ورقہ,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  أكد( p_ورقہ->پتی==0 );
  أكد( p_ورقہ->بچہ_Ptr_ناپ==4 );
#جے_بنا_تعریف SQLITE_DEBUG
  بن_ورتیا_PARAMETER(p_ورقہ);
#جے_انت
  pInfo->n_ناپ = 4 + sqlite3GetVarint(&pCell[4], (u64*)&pInfo->n_کلید);
  pInfo->nPayload = 0;
  pInfo->nLocal = 0;
  pInfo->pPayload = 0;
  أرجع;
}
ساكن فراغ btreeParseCellPtr(
  Mem_ورقہ *p_ورقہ,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u64 nPayload;           /* Number of bytes of cell payload */
  u64 i_کلید;               /* Extracted Key value */

  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  أكد( p_ورقہ->پتی==0 || p_ورقہ->پتی==1 );
  أكد( p_ورقہ->int_کلید_پتی );
  أكد( p_ورقہ->بچہ_Ptr_ناپ==0 );
  pIter = pCell;

  /* The next block of code is equivalent to:
  **
  **     pIter += getVarint32(pIter, nPayload);
  **
  ** The code is inlined to avoid a function call.
  */
  nPayload = *pIter;
  إذا( nPayload>=0x80 ){
    u8 *p_سرا = &pIter[8];
    nPayload &= 0x7f;
    افعل{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }طالما( (*pIter)>=0x80 && pIter<p_سرا );
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
  إذا( i_کلید>=0x80 ){
    u8 x;
    i_کلید = (i_کلید<<7) ^ (x = *++pIter);
    إذا( x>=0x80 ){
      i_کلید = (i_کلید<<7) ^ (x = *++pIter);
      إذا( x>=0x80 ){
        i_کلید = (i_کلید<<7) ^ 0x10204000 ^ (x = *++pIter);
        إذا( x>=0x80 ){
          i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++pIter);
          إذا( x>=0x80 ){
            i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++pIter);
            إذا( x>=0x80 ){
              i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++pIter);
              إذا( x>=0x80 ){
                i_کلید = (i_کلید<<7) ^ 0x4000 ^ (x = *++pIter);
                إذا( x>=0x80 ){
                  i_کلید = (i_کلید<<8) ^ 0x8000 ^ (*++pIter);
                }
              }
            }
          }
        }
      }وإلا{
        i_کلید ^= 0x204000;
      }
    }وإلا{
      i_کلید ^= 0x4000;
    }
  }
  pIter++;

  pInfo->n_کلید = *(i64*)&i_کلید;
  pInfo->nPayload = (u32)nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==p_ورقہ->maxLocal );
  testcase( nPayload==(u32)p_ورقہ->maxLocal+1 );
  أكد( p_ورقہ->maxLocal <= BT_MAX_LOCAL );
  إذا( nPayload<=p_ورقہ->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->n_ناپ = (u16)nPayload + (u16)(pIter - pCell);
    إذا( pInfo->n_ناپ<4 ) pInfo->n_ناپ = 4;
    pInfo->nLocal = (u16)nPayload;
  }وإلا{
    btree_Parse_Cell_Adjust_ناپ_For_حد_توں_ودھ(p_ورقہ, pCell, pInfo);
  }
}
ساكن فراغ btree_Parse_Cell_Ptr_اشاریہ(
  Mem_ورقہ *p_ورقہ,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u32 nPayload;           /* Number of bytes of cell payload */

  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  أكد( p_ورقہ->پتی==0 || p_ورقہ->پتی==1 );
  أكد( p_ورقہ->int_کلید_پتی==0 );
  pIter = pCell + p_ورقہ->بچہ_Ptr_ناپ;
  nPayload = *pIter;
  إذا( nPayload>=0x80 ){
    u8 *p_سرا = &pIter[8];
    nPayload &= 0x7f;
    افعل{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }طالما( *(pIter)>=0x80 && pIter<p_سرا );
  }
  pIter++;
  pInfo->n_کلید = nPayload;
  pInfo->nPayload = nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==p_ورقہ->maxLocal );
  testcase( nPayload==(u32)p_ورقہ->maxLocal+1 );
  أكد( nPayload>=0 );
  أكد( p_ورقہ->maxLocal <= BT_MAX_LOCAL );
  إذا( nPayload<=p_ورقہ->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->n_ناپ = (u16)nPayload + (u16)(pIter - pCell);
    إذا( pInfo->n_ناپ<4 ) pInfo->n_ناپ = 4;
    pInfo->nLocal = (u16)nPayload;
  }وإلا{
    btree_Parse_Cell_Adjust_ناپ_For_حد_توں_ودھ(p_ورقہ, pCell, pInfo);
  }
}
ساكن فراغ btreeParseCell(
  Mem_ورقہ *p_ورقہ,         /* Page containing the cell */
  صحيح iCell,              /* The cell index.  First cell is 0 */
  CellInfo *pInfo         /* Fill in this structure */
){
  p_ورقہ->xParseCell(p_ورقہ, لبھو_Cell(p_ورقہ, iCell), pInfo);
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
ساكن u16 cell_ناپ_Ptr(Mem_ورقہ *p_ورقہ, u8 *pCell){
  u8 *pIter = pCell + 4;                   /* For looping over bytes of pCell */
  u8 *p_سرا;                                /* End mark for a varint */
  u32 n_ناپ;                               /* Size value to return */

#جے_تعریف_شدہ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_ورقہ->xParseCell(p_ورقہ, pCell, &debuginfo);
#جے_انت

  أكد( p_ورقہ->بچہ_Ptr_ناپ==4 );
  n_ناپ = *pIter;
  إذا( n_ناپ>=0x80 ){
    p_سرا = &pIter[8];
    n_ناپ &= 0x7f;
    افعل{
      n_ناپ = (n_ناپ<<7) | (*++pIter & 0x7f);
    }طالما( *(pIter)>=0x80 && pIter<p_سرا );
  }
  pIter++;
  testcase( n_ناپ==p_ورقہ->maxLocal );
  testcase( n_ناپ==(u32)p_ورقہ->maxLocal+1 );
  إذا( n_ناپ<=p_ورقہ->maxLocal ){
    n_ناپ += (u32)(pIter - pCell);
    أكد( n_ناپ>4 );
  }وإلا{
    صحيح minLocal = p_ورقہ->minLocal;
    n_ناپ = minLocal + (n_ناپ - minLocal) % (p_ورقہ->pBt->usable_ناپ - 4);
    testcase( n_ناپ==p_ورقہ->maxLocal );
    testcase( n_ناپ==(u32)p_ورقہ->maxLocal+1 );
    إذا( n_ناپ>p_ورقہ->maxLocal ){
      n_ناپ = minLocal;
    }
    n_ناپ += 4 + (u16)(pIter - pCell);
  }
  أكد( n_ناپ==debuginfo.n_ناپ || CORRUPT_DB );
  أرجع (u16)n_ناپ;
}
ساكن u16 cell_ناپ_Ptr_Idx_پتی(Mem_ورقہ *p_ورقہ, u8 *pCell){
  u8 *pIter = pCell;                       /* For looping over bytes of pCell */
  u8 *p_سرا;                                /* End mark for a varint */
  u32 n_ناپ;                               /* Size value to return */

#جے_تعریف_شدہ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_ورقہ->xParseCell(p_ورقہ, pCell, &debuginfo);
#جے_انت

  أكد( p_ورقہ->بچہ_Ptr_ناپ==0 );
  n_ناپ = *pIter;
  إذا( n_ناپ>=0x80 ){
    p_سرا = &pIter[8];
    n_ناپ &= 0x7f;
    افعل{
      n_ناپ = (n_ناپ<<7) | (*++pIter & 0x7f);
    }طالما( *(pIter)>=0x80 && pIter<p_سرا );
  }
  pIter++;
  testcase( n_ناپ==p_ورقہ->maxLocal );
  testcase( n_ناپ==(u32)p_ورقہ->maxLocal+1 );
  إذا( n_ناپ<=p_ورقہ->maxLocal ){
    n_ناپ += (u32)(pIter - pCell);
    إذا( n_ناپ<4 ) n_ناپ = 4;
  }وإلا{
    صحيح minLocal = p_ورقہ->minLocal;
    n_ناپ = minLocal + (n_ناپ - minLocal) % (p_ورقہ->pBt->usable_ناپ - 4);
    testcase( n_ناپ==p_ورقہ->maxLocal );
    testcase( n_ناپ==(u32)p_ورقہ->maxLocal+1 );
    إذا( n_ناپ>p_ورقہ->maxLocal ){
      n_ناپ = minLocal;
    }
    n_ناپ += 4 + (u16)(pIter - pCell);
  }
  أكد( n_ناپ==debuginfo.n_ناپ || CORRUPT_DB );
  أرجع (u16)n_ناپ;
}
ساكن u16 cell_ناپ_Ptr_No_Payload(Mem_ورقہ *p_ورقہ, u8 *pCell){
  u8 *pIter = pCell + 4; /* For looping over bytes of pCell */
  u8 *p_سرا;              /* End mark for a varint */

#جے_تعریف_شدہ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_ورقہ->xParseCell(p_ورقہ, pCell, &debuginfo);
#وإلا
  بن_ورتیا_PARAMETER(p_ورقہ);
#جے_انت

  أكد( p_ورقہ->بچہ_Ptr_ناپ==4 );
  p_سرا = pIter + 9;
  طالما( (*pIter++)&0x80 && pIter<p_سرا );
  أكد( debuginfo.n_ناپ==(u16)(pIter - pCell) || CORRUPT_DB );
  أرجع (u16)(pIter - pCell);
}
ساكن u16 cell_ناپ_Ptr_جدول_پتی(Mem_ورقہ *p_ورقہ, u8 *pCell){
  u8 *pIter = pCell;   /* For looping over bytes of pCell */
  u8 *p_سرا;            /* End mark for a varint */
  u32 n_ناپ;           /* Size value to return */

#جے_تعریف_شدہ SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_ورقہ->xParseCell(p_ورقہ, pCell, &debuginfo);
#جے_انت

  n_ناپ = *pIter;
  إذا( n_ناپ>=0x80 ){
    p_سرا = &pIter[8];
    n_ناپ &= 0x7f;
    افعل{
      n_ناپ = (n_ناپ<<7) | (*++pIter & 0x7f);
    }طالما( *(pIter)>=0x80 && pIter<p_سرا );
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
  testcase( n_ناپ==p_ورقہ->maxLocal );
  testcase( n_ناپ==(u32)p_ورقہ->maxLocal+1 );
  إذا( n_ناپ<=p_ورقہ->maxLocal ){
    n_ناپ += (u32)(pIter - pCell);
    إذا( n_ناپ<4 ) n_ناپ = 4;
  }وإلا{
    صحيح minLocal = p_ورقہ->minLocal;
    n_ناپ = minLocal + (n_ناپ - minLocal) % (p_ورقہ->pBt->usable_ناپ - 4);
    testcase( n_ناپ==p_ورقہ->maxLocal );
    testcase( n_ناپ==(u32)p_ورقہ->maxLocal+1 );
    إذا( n_ناپ>p_ورقہ->maxLocal ){
      n_ناپ = minLocal;
    }
    n_ناپ += 4 + (u16)(pIter - pCell);
  }
  أكد( n_ناپ==debuginfo.n_ناپ || CORRUPT_DB );
  أرجع (u16)n_ناپ;
}


#جے_تعریف_شدہ SQLITE_DEBUG
/* This variation on cellSizePtr() is used inside of assert() statements
** only. */
ساكن u16 cell_ناپ(Mem_ورقہ *p_ورقہ, صحيح iCell){
  أرجع p_ورقہ->x_Cell_ناپ(p_ورقہ, لبھو_Cell(p_ورقہ, iCell));
}
#جے_انت

#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
/*
** The cell pCell is currently part of page pSrc but will ultimately be part
** of pPage.  (pSrc and pPage are often the same.)  If pCell contains a
** pointer to an overflow page, insert an entry into the pointer-map for
** the overflow page that will be valid after pCell has been moved to pPage.
*/
ساكن فراغ ptrmapPutOvflPtr(Mem_ورقہ *p_ورقہ, Mem_ورقہ *pSrc, u8 *pCell,صحيح *pRC){
  CellInfo info;
  إذا( *pRC ) أرجع;
  أكد( pCell!=0 );
  p_ورقہ->xParseCell(p_ورقہ, pCell, &info);
  إذا( info.nLocal<info.nPayload ){
    Pgno ovfl;
    إذا( SQLITE_حد_توں_ودھ(pSrc->a_انکڑے_سرا, pCell, pCell+info.nLocal) ){
      testcase( pSrc!=p_ورقہ );
      *pRC = SQLITE_CORRUPT_BKPT;
      أرجع;
    }
    ovfl = get4byte(&pCell[info.n_ناپ-4]);
    ptrmapPut(p_ورقہ->pBt, ovfl, PTRMAP_OVERFLOW1, p_ورقہ->pgno, pRC);
  }
}
#جے_انت


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
ساكن صحيح defragment_ورقہ(Mem_ورقہ *p_ورقہ, صحيح nMaxFrag){
  صحيح i;                     /* Loop counter */
  صحيح pc;                    /* Address of the i-th cell */
  صحيح hdr;                   /* Offset to the page header */
  صحيح ناپ;                  /* Size of a cell */
  صحيح usable_ناپ;            /* Number of usable bytes on a page */
  صحيح cell_وتھ;            /* Offset to the cell pointer array */
  صحيح cbrk;                  /* Offset to the cell content area */
  صحيح nCell;                 /* Number of cells on the page */
  بنانشانی حرف *انکڑے;       /* The page data */
  بنانشانی حرف *عارضی;       /* Temp area for cell content */
  بنانشانی حرف *src;        /* Source of content */
  صحيح i_Cell_پہلا;            /* First allowable cell index */
  صحيح i_Cell_آخری;             /* Last possible cell index */
  صحيح i_Cell_شروع;            /* First cell offset in input */

  أكد( sqlite3PagerIswriteable(p_ورقہ->p_Db_ورقہ) );
  أكد( p_ورقہ->pBt!=0 );
  أكد( p_ورقہ->pBt->usable_ناپ <= SQLITE_MAX_ورقہ_ناپ );
  أكد( p_ورقہ->n_حد_توں_ودھ==0 );
  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  انکڑے = p_ورقہ->a_انکڑے;
  hdr = p_ورقہ->hdr_وتھ;
  cell_وتھ = p_ورقہ->cell_وتھ;
  nCell = p_ورقہ->nCell;
  أكد( nCell==get2byte(&انکڑے[hdr+3]) || CORRUPT_DB );
  i_Cell_پہلا = cell_وتھ + 2*nCell;
  usable_ناپ = p_ورقہ->pBt->usable_ناپ;

  /* This block handles pages with two or fewer free blocks and nMaxFrag
  ** or fewer fragmented bytes. In this case it is faster to move the
  ** two (or one) blocks of cells using memmove() and add the required
  ** offsets to each pointer in the cell-pointer array than it is to
  ** reconstruct the entire page.  */
  إذا( (صحيح)انکڑے[hdr+7]<=nMaxFrag ){
    صحيح iFree = get2byte(&انکڑے[hdr+1]);
    إذا( iFree>usable_ناپ-4 ) أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    إذا( iFree ){
      صحيح iFree2 = get2byte(&انکڑے[iFree]);
      إذا( iFree2>usable_ناپ-4 ) أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
      إذا( 0==iFree2 || (انکڑے[iFree2]==0 && انکڑے[iFree2+1]==0) ){
        u8 *p_سرا = &انکڑے[cell_وتھ + nCell*2];
        u8 *pAddr;
        صحيح sz2 = 0;
        صحيح sz = get2byte(&انکڑے[iFree+2]);
        صحيح چوٹی = get2byte(&انکڑے[hdr+5]);
        إذا( چوٹی>=iFree ){
          أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
        }
        إذا( iFree2 ){
          إذا( iFree+sz>iFree2 ) أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
          sz2 = get2byte(&انکڑے[iFree2+2]);
          إذا( iFree2+sz2 > usable_ناپ ) أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
          memmove(&انکڑے[iFree+sz+sz2], &انکڑے[iFree+sz], iFree2-(iFree+sz));
          sz += sz2;
        }وإلا إذا( iFree+sz>usable_ناپ ){
          أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
        }

        cbrk = چوٹی+sz;
        أكد( cbrk+(iFree-چوٹی) <= usable_ناپ );
        memmove(&انکڑے[cbrk], &انکڑے[چوٹی], iFree-چوٹی);
        لكل(pAddr=&انکڑے[cell_وتھ]; pAddr<p_سرا; pAddr+=2){
          pc = get2byte(pAddr);
          إذا( pc<iFree ){ put2byte(pAddr, pc+sz); }
          وإلا إذا( pc<iFree2 ){ put2byte(pAddr, pc+sz2); }
        }
        اذهب defragment_باہرلا;
      }
    }
  }

  cbrk = usable_ناپ;
  i_Cell_آخری = usable_ناپ - 4;
  i_Cell_شروع = get2byte(&انکڑے[hdr+5]);
  إذا( nCell>0 ){
    عارضی = sqlite3_Pager_عارضی_Space(p_ورقہ->pBt->pPager);
    حافظہ_نقل(عارضی, انکڑے, usable_ناپ);
    src = عارضی;
    لكل(i=0; i<nCell; i++){
      u8 *pAddr;     /* The i-th cell pointer */
      pAddr = &انکڑے[cell_وتھ + i*2];
      pc = get2byte(pAddr);
      testcase( pc==i_Cell_پہلا );
      testcase( pc==i_Cell_آخری );
      /* These conditions have already been verified in btreeInitPage()
      ** if PRAGMA cell_size_check=ON.
      */
      إذا( pc>i_Cell_آخری ){
        أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
      }
      أكد( pc>=0 && pc<=i_Cell_آخری );
      ناپ = p_ورقہ->x_Cell_ناپ(p_ورقہ, &src[pc]);
      cbrk -= ناپ;
      إذا( cbrk<i_Cell_شروع || pc+ناپ>usable_ناپ ){
        أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
      }
      أكد( cbrk+ناپ<=usable_ناپ && cbrk>=i_Cell_شروع );
      testcase( cbrk+ناپ==usable_ناپ );
      testcase( pc+ناپ==usable_ناپ );
      put2byte(pAddr, cbrk);
      حافظہ_نقل(&انکڑے[cbrk], &src[pc], ناپ);
    }
  }
  انکڑے[hdr+7] = 0;

defragment_باہرلا:
  أكد( p_ورقہ->nFree>=0 );
  إذا( انکڑے[hdr+7]+cbrk-i_Cell_پہلا!=p_ورقہ->nFree ){
    أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
  }
  أكد( cbrk>=i_Cell_پہلا );
  put2byte(&انکڑے[hdr+5], cbrk);
  انکڑے[hdr+1] = 0;
  انکڑے[hdr+2] = 0;
  memset(&انکڑے[i_Cell_پہلا], 0, cbrk-i_Cell_پہلا);
  أكد( sqlite3PagerIswriteable(p_ورقہ->p_Db_ورقہ) );
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
ساكن u8 *ورقہ_لبھو_تھاں(Mem_ورقہ *pPg, صحيح nByte, صحيح *pRc){
  ثابت صحيح hdr = pPg->hdr_وتھ;            /* Offset to page header */
  u8 * ثابت a_انکڑے = pPg->a_انکڑے;             /* Page data */
  صحيح iAddr = hdr + 1;                       /* Address of ptr to pc */
  u8 *pTmp = &a_انکڑے[iAddr];                  /* Temporary ptr into aData[] */
  صحيح pc = get2byte(pTmp);                   /* Address of a free slot */
  صحيح x;                                     /* Excess size of the slot */
  صحيح maxPC = pPg->pBt->usable_ناپ - nByte;  /* Max address for a usable slot */
  صحيح ناپ;                                  /* Size of the free slot */

  أكد( pc>0 );
  طالما( pc<=maxPC ){
    /* EVIDENCE-OF: R-22710-53328 The third and fourth bytes of each
    ** freeblock form a big-endian integer which is the size of the freeblock
    ** in bytes, including the 4-byte header. */
    pTmp = &a_انکڑے[pc+2];
    ناپ = get2byte(pTmp);
    إذا( (x = ناپ - nByte)>=0 ){
      testcase( x==4 );
      testcase( x==3 );
      إذا( x<4 ){
        /* EVIDENCE-OF: R-11498-58022 In a well-formed b-tree page, the total
        ** number of bytes in fragments may not exceed 60. */
        إذا( a_انکڑے[hdr+7]>57 ) أرجع 0;

        /* Remove the slot from the free-list. Update the number of
        ** fragmented bytes within the page. */
        حافظہ_نقل(&a_انکڑے[iAddr], &a_انکڑے[pc], 2);
        a_انکڑے[hdr+7] += (u8)x;
        أرجع &a_انکڑے[pc];
      }وإلا إذا( x+pc > maxPC ){
        /* This slot extends off the end of the usable part of the page */
        *pRc = SQLITE_CORRUPT_ورقہ(pPg);
        أرجع 0;
      }وإلا{
        /* The slot remains on the free-list. Reduce its size to account
        ** for the portion used by the new allocation. */
        put2byte(&a_انکڑے[pc+2], x);
      }
      أرجع &a_انکڑے[pc + x];
    }
    iAddr = pc;
    pTmp = &a_انکڑے[pc];
    pc = get2byte(pTmp);
    إذا( pc<=iAddr ){
      إذا( pc ){
        /* The next slot in the chain comes before the current slot */
        *pRc = SQLITE_CORRUPT_ورقہ(pPg);
      }
      أرجع 0;
    }
  }
  إذا( pc>maxPC+nByte-4 ){
    /* The free slot chain extends off the end of the page */
    *pRc = SQLITE_CORRUPT_ورقہ(pPg);
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
ساكن SQLITE_INLINE صحيح allocateSpace(Mem_ورقہ *p_ورقہ, صحيح nByte, صحيح *pIdx){
  ثابت صحيح hdr = p_ورقہ->hdr_وتھ;    /* Local cache of pPage->hdrOffset */
  u8 * ثابت انکڑے = p_ورقہ->a_انکڑے;      /* Local cache of pPage->aData */
  صحيح چوٹی;                             /* First byte of cell content area */
  صحيح rc = SQLITE_OK;                  /* Integer return code */
  u8 *pTmp;                            /* Temp ptr into data[] */
  صحيح gap;        /* First byte of gap between cell pointers and cell content */
 
  أكد( sqlite3PagerIswriteable(p_ورقہ->p_Db_ورقہ) );
  أكد( p_ورقہ->pBt );
  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  أكد( nByte>=0 );  /* Minimum cell size is 4 */
  أكد( p_ورقہ->nFree>=nByte );
  أكد( p_ورقہ->n_حد_توں_ودھ==0 );
  أكد( nByte < (صحيح)(p_ورقہ->pBt->usable_ناپ-8) );

  أكد( p_ورقہ->cell_وتھ == hdr + 12 - 4*p_ورقہ->پتی );
  gap = p_ورقہ->cell_وتھ + 2*p_ورقہ->nCell;
  أكد( gap<=65536 );
  /* EVIDENCE-OF: R-29356-02391 If the database uses a 65536-byte page size
  ** and the reserved space is zero (the usual value for reserved space)
  ** then the cell content offset of an empty page wants to be 65536.
  ** However, that integer is too large to be stored in a 2-byte unsigned
  ** integer, so a value of 0 is used in its place. */
  pTmp = &انکڑے[hdr+5];
  چوٹی = get2byte(pTmp);
  إذا( gap>چوٹی ){
    إذا( چوٹی==0 && p_ورقہ->pBt->usable_ناپ==65536 ){
      چوٹی = 65536;
    }وإلا{
      أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    }
  }وإلا إذا( چوٹی>(صحيح)p_ورقہ->pBt->usable_ناپ ){
    أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
  }

  /* If there is enough space between gap and top for one more cell pointer,
  ** and if the freelist is not empty, then search the
  ** freelist looking for a slot big enough to satisfy the request.
  */
  testcase( gap+2==چوٹی );
  testcase( gap+1==چوٹی );
  testcase( gap==چوٹی );
  إذا( (انکڑے[hdr+2] || انکڑے[hdr+1]) && gap+2<=چوٹی ){
    u8 *pSpace = ورقہ_لبھو_تھاں(p_ورقہ, nByte, &rc);
    إذا( pSpace ){
      صحيح g2;
      أكد( pSpace+nByte<=انکڑے+p_ورقہ->pBt->usable_ناپ );
      *pIdx = g2 = (صحيح)(pSpace-انکڑے);
      إذا( g2<=gap ){
        أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
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
  testcase( gap+2+nByte==چوٹی );
  إذا( gap+2+nByte>چوٹی ){
    أكد( p_ورقہ->nCell>0 || CORRUPT_DB );
    أكد( p_ورقہ->nFree>=0 );
    rc = defragment_ورقہ(p_ورقہ, MIN(4, p_ورقہ->nFree - (2+nByte)));
    إذا( rc ) أرجع rc;
    چوٹی = get2byteNotZero(&انکڑے[hdr+5]);
    أكد( gap+2+nByte<=چوٹی );
  }


  /* Allocate memory from the gap in between the cell pointer array
  ** and the cell content area.  The btreeComputeFreeSpace() call has already
  ** validated the freelist.  Given that the freelist is valid, there
  ** is no way that the allocation can extend off the end of the page.
  ** The assert() below verifies the previous sentence.
  */
  چوٹی -= nByte;
  put2byte(&انکڑے[hdr+5], چوٹی);
  أكد( چوٹی+nByte <= (صحيح)p_ورقہ->pBt->usable_ناپ );
  *pIdx = چوٹی;
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
ساكن صحيح freeSpace(Mem_ورقہ *p_ورقہ, صحيح i_شروع, صحيح i_ناپ){
  صحيح iPtr;                             /* Address of ptr to next freeblock */
  صحيح iFreeBlk;                         /* Address of the next freeblock */
  u8 hdr;                               /* Page header size.  0 or 100 */
  صحيح nFrag = 0;                        /* Reduction in fragmentation */
  صحيح i_Orig_ناپ = i_ناپ;                /* Original value of iSize */
  صحيح x;                                /* Offset to cell content area */
  صحيح i_سرا = i_شروع + i_ناپ;            /* First byte past the iStart buffer */
  بنانشانی حرف *انکڑے = p_ورقہ->a_انکڑے;   /* Page content */
  u8 *pTmp;                             /* Temporary ptr into data[] */

  أكد( p_ورقہ->pBt!=0 );
  أكد( sqlite3PagerIswriteable(p_ورقہ->p_Db_ورقہ) );
  أكد( CORRUPT_DB || i_شروع>=p_ورقہ->hdr_وتھ+6+p_ورقہ->بچہ_Ptr_ناپ );
  أكد( CORRUPT_DB || i_سرا <= (صحيح)p_ورقہ->pBt->usable_ناپ );
  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  أكد( i_ناپ>=4 );   /* Minimum cell size is 4 */
  أكد( CORRUPT_DB || i_شروع<=(صحيح)p_ورقہ->pBt->usable_ناپ-4 );

  /* The list of freeblocks must be in ascending order.  Find the
  ** spot on the list where iStart should be inserted.
  */
  hdr = p_ورقہ->hdr_وتھ;
  iPtr = hdr + 1;
  إذا( انکڑے[iPtr+1]==0 && انکڑے[iPtr]==0 ){
    iFreeBlk = 0;  /* Shortcut for the case when the freelist is empty */
  }وإلا{
    طالما( (iFreeBlk = get2byte(&انکڑے[iPtr]))<i_شروع ){
      إذا( iFreeBlk<=iPtr ){
        إذا( iFreeBlk==0 ) اكسر; /* TH3: corrupt082.100 */
        أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
      }
      iPtr = iFreeBlk;
    }
    إذا( iFreeBlk>(صحيح)p_ورقہ->pBt->usable_ناپ-4 ){ /* TH3: corrupt081.100 */
      أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    }
    أكد( iFreeBlk>iPtr || iFreeBlk==0 || CORRUPT_DB );
 
    /* At this point:
    **    iFreeBlk:   First freeblock after iStart, or zero if none
    **    iPtr:       The address of a pointer to iFreeBlk
    **
    ** Check to see if iFreeBlk should be coalesced onto the end of iStart.
    */
    إذا( iFreeBlk && i_سرا+3>=iFreeBlk ){
      nFrag = iFreeBlk - i_سرا;
      إذا( i_سرا>iFreeBlk ) أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
      i_سرا = iFreeBlk + get2byte(&انکڑے[iFreeBlk+2]);
      إذا( i_سرا > (صحيح)p_ورقہ->pBt->usable_ناپ ){
        أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
      }
      i_ناپ = i_سرا - i_شروع;
      iFreeBlk = get2byte(&انکڑے[iFreeBlk]);
    }
 
    /* If iPtr is another freeblock (that is, if iPtr is not the freelist
    ** pointer in the page header) then check to see if iStart should be
    ** coalesced onto the end of iPtr.
    */
    إذا( iPtr>hdr+1 ){
      صحيح i_Ptr_سرا = iPtr + get2byte(&انکڑے[iPtr+2]);
      إذا( i_Ptr_سرا+3>=i_شروع ){
        إذا( i_Ptr_سرا>i_شروع ) أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
        nFrag += i_شروع - i_Ptr_سرا;
        i_ناپ = i_سرا - iPtr;
        i_شروع = iPtr;
      }
    }
    إذا( nFrag>انکڑے[hdr+7] ) أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    انکڑے[hdr+7] -= (u8)nFrag;
  }
  pTmp = &انکڑے[hdr+5];
  x = get2byte(pTmp);
  إذا( p_ورقہ->pBt->btsFlags & BTS_FAST_SECURE ){
    /* Overwrite deleted information with zeros when the secure_delete
    ** option is enabled */
    memset(&انکڑے[i_شروع], 0, i_ناپ);
  }
  إذا( i_شروع<=x ){
    /* The new freeblock is at the beginning of the cell content area,
    ** so just extend the cell content area rather than create another
    ** freelist entry */
    إذا( i_شروع<x ) أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    إذا( iPtr!=hdr+1 ) أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    put2byte(&انکڑے[hdr+1], iFreeBlk);
    put2byte(&انکڑے[hdr+5], i_سرا);
  }وإلا{
    /* Insert the new freeblock into the freelist */
    put2byte(&انکڑے[iPtr], i_شروع);
    put2byte(&انکڑے[i_شروع], iFreeBlk);
    أكد( i_ناپ>=0 && i_ناپ<=0xffff );
    put2byte(&انکڑے[i_شروع+2], (u16)i_ناپ);
  }
  p_ورقہ->nFree += i_Orig_ناپ;
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
ساكن صحيح decodeFlags(Mem_ورقہ *p_ورقہ, صحيح flagByte){
  BtShared *pBt;     /* A copy of pPage->pBt */

  أكد( p_ورقہ->hdr_وتھ==(p_ورقہ->pgno==1 ? 100 : 0) );
  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  pBt = p_ورقہ->pBt;
  p_ورقہ->max1bytePayload = pBt->max1bytePayload;
  إذا( flagByte>=(PTF_ZERODATA | PTF_پتی) ){
    p_ورقہ->بچہ_Ptr_ناپ = 0;
    p_ورقہ->پتی = 1;
    إذا( flagByte==(PTF_LEAFDATA | PTF_INTKEY | PTF_پتی) ){
      p_ورقہ->int_کلید_پتی = 1;
      p_ورقہ->x_Cell_ناپ = cell_ناپ_Ptr_جدول_پتی;
      p_ورقہ->xParseCell = btreeParseCellPtr;
      p_ورقہ->int_کلید = 1;
      p_ورقہ->maxLocal = pBt->max_پتی;
      p_ورقہ->minLocal = pBt->min_پتی;
    }وإلا إذا( flagByte==(PTF_ZERODATA | PTF_پتی) ){
      p_ورقہ->int_کلید = 0;
      p_ورقہ->int_کلید_پتی = 0;
      p_ورقہ->x_Cell_ناپ = cell_ناپ_Ptr_Idx_پتی;
      p_ورقہ->xParseCell = btree_Parse_Cell_Ptr_اشاریہ;
      p_ورقہ->maxLocal = pBt->maxLocal;
      p_ورقہ->minLocal = pBt->minLocal;
    }وإلا{
      p_ورقہ->int_کلید = 0;
      p_ورقہ->int_کلید_پتی = 0;
      p_ورقہ->x_Cell_ناپ = cell_ناپ_Ptr_Idx_پتی;
      p_ورقہ->xParseCell = btree_Parse_Cell_Ptr_اشاریہ;
      أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    }
  }وإلا{
    p_ورقہ->بچہ_Ptr_ناپ = 4;
    p_ورقہ->پتی = 0;
    إذا( flagByte==(PTF_ZERODATA) ){
      p_ورقہ->int_کلید = 0;
      p_ورقہ->int_کلید_پتی = 0;
      p_ورقہ->x_Cell_ناپ = cell_ناپ_Ptr;
      p_ورقہ->xParseCell = btree_Parse_Cell_Ptr_اشاریہ;
      p_ورقہ->maxLocal = pBt->maxLocal;
      p_ورقہ->minLocal = pBt->minLocal;
    }وإلا إذا( flagByte==(PTF_LEAFDATA | PTF_INTKEY) ){
      p_ورقہ->int_کلید_پتی = 0;
      p_ورقہ->x_Cell_ناپ = cell_ناپ_Ptr_No_Payload;
      p_ورقہ->xParseCell = btreeParseCellPtrNoPayload;
      p_ورقہ->int_کلید = 1;
      p_ورقہ->maxLocal = pBt->max_پتی;
      p_ورقہ->minLocal = pBt->min_پتی;
    }وإلا{
      p_ورقہ->int_کلید = 0;
      p_ورقہ->int_کلید_پتی = 0;
      p_ورقہ->x_Cell_ناپ = cell_ناپ_Ptr;
      p_ورقہ->xParseCell = btree_Parse_Cell_Ptr_اشاریہ;
      أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    }
  }
  أرجع SQLITE_OK;
}

/*
** Compute the amount of freespace on the page.  In other words, fill
** in the pPage->nFree field.
*/
ساكن صحيح btreeComputeFreeSpace(Mem_ورقہ *p_ورقہ){
  صحيح pc;            /* Address of a freeblock within pPage->aData[] */
  u8 hdr;            /* Offset to beginning of page header */
  u8 *انکڑے;          /* Equal to pPage->aData */
  صحيح usable_ناپ;    /* Amount of usable space on each page */
  صحيح nFree;         /* Number of unused bytes on the page */
  صحيح چوٹی;           /* First byte of the cell content area */
  صحيح i_Cell_پہلا;    /* First allowable cell or freeblock offset */
  صحيح i_Cell_آخری;     /* Last possible cell or freeblock offset */

  أكد( p_ورقہ->pBt!=0 );
  أكد( p_ورقہ->pBt->db!=0 );
  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  أكد( p_ورقہ->pgno==sqlite3PagerPagenumber(p_ورقہ->p_Db_ورقہ) );
  أكد( p_ورقہ == sqlite3PagerGetExtra(p_ورقہ->p_Db_ورقہ) );
  أكد( p_ورقہ->a_انکڑے == sqlite3_Pager_Get_انکڑے(p_ورقہ->p_Db_ورقہ) );
  أكد( p_ورقہ->isInit==1 );
  أكد( p_ورقہ->nFree<0 );

  usable_ناپ = p_ورقہ->pBt->usable_ناپ;
  hdr = p_ورقہ->hdr_وتھ;
  انکڑے = p_ورقہ->a_انکڑے;
  /* EVIDENCE-OF: R-58015-48175 The two-byte integer at offset 5 designates
  ** the start of the cell content area. A zero value for this integer is
  ** interpreted as 65536. */
  چوٹی = get2byteNotZero(&انکڑے[hdr+5]);
  i_Cell_پہلا = hdr + 8 + p_ورقہ->بچہ_Ptr_ناپ + 2*p_ورقہ->nCell;
  i_Cell_آخری = usable_ناپ - 4;

  /* Compute the total free space on the page
  ** EVIDENCE-OF: R-23588-34450 The two-byte integer at offset 1 gives the
  ** start of the first freeblock on the page, or is zero if there are no
  ** freeblocks. */
  pc = get2byte(&انکڑے[hdr+1]);
  nFree = انکڑے[hdr+7] + چوٹی;  /* Init nFree to non-freeblock free space */
  إذا( pc>0 ){
    u32 اگلا, ناپ;
    إذا( pc<چوٹی ){
      /* EVIDENCE-OF: R-55530-52930 In a well-formed b-tree page, there will
      ** always be at least one cell before the first freeblock.
      */
      أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    }
    طالما( 1 ){
      إذا( pc>i_Cell_آخری ){
        /* Freeblock off the end of the page */
        أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
      }
      اگلا = get2byte(&انکڑے[pc]);
      ناپ = get2byte(&انکڑے[pc+2]);
      إذا( ناپ<4 && sqlite3FaultSim(422)==SQLITE_OK ){
        /* Minimum freeblock size is 4.  Enable fault-sim 422 to disable this
        ** check to reach interesting error states.  However, disabling this
        ** check can cause assertion faults due to min-heap overflow.  All
        ** fault-sims are for testing use only, but this one especially so. */
        أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
      }
      nFree = nFree + ناپ;
      إذا( اگلا<pc+ناپ+4 ) اكسر;
      pc = اگلا;
    }
    إذا( اگلا>0 ){
      /* Freeblock not in ascending order */
      أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    }
    إذا( pc+ناپ>(بنانشانی صحيح)usable_ناپ ){
      /* Last freeblock extends past page end */
      أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    }
  }

  /* At this point, nFree contains the sum of the offset to the start
  ** of the cell-content area plus the number of free bytes within
  ** the cell-content area. If this is greater than the usable-size
  ** of the page, then the page must be corrupted. This check also
  ** serves to verify that the offset to the start of the cell-content
  ** area, according to the page header, lies within the page.
  */
  إذا( nFree>usable_ناپ || nFree<i_Cell_پہلا ){
    أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
  }
  p_ورقہ->nFree = (u16)(nFree - i_Cell_پہلا);
  أرجع SQLITE_OK;
}

/*
** Do additional sanity check after btreeInitPage() if
** PRAGMA cell_size_check=ON
*/
ساكن SQLITE_NOINLINE صحيح btree_Cell_ناپ_Check(Mem_ورقہ *p_ورقہ){
  صحيح i_Cell_پہلا;    /* First allowable cell or freeblock offset */
  صحيح i_Cell_آخری;     /* Last possible cell or freeblock offset */
  صحيح i;             /* Index into the cell pointer array */
  صحيح sz;            /* Size of a cell */
  صحيح pc;            /* Address of a freeblock within pPage->aData[] */
  u8 *انکڑے;          /* Equal to pPage->aData */
  صحيح usable_ناپ;    /* Maximum usable space on the page */
  صحيح cell_وتھ;    /* Start of cell content area */

  i_Cell_پہلا = p_ورقہ->cell_وتھ + 2*p_ورقہ->nCell;
  usable_ناپ = p_ورقہ->pBt->usable_ناپ;
  i_Cell_آخری = usable_ناپ - 4;
  انکڑے = p_ورقہ->a_انکڑے;
  cell_وتھ = p_ورقہ->cell_وتھ;
  إذا( !p_ورقہ->پتی ) i_Cell_آخری--;
  لكل(i=0; i<p_ورقہ->nCell; i++){
    pc = get2byteAligned(&انکڑے[cell_وتھ+i*2]);
    testcase( pc==i_Cell_پہلا );
    testcase( pc==i_Cell_آخری );
    إذا( pc<i_Cell_پہلا || pc>i_Cell_آخری ){
      أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    }
    sz = p_ورقہ->x_Cell_ناپ(p_ورقہ, &انکڑے[pc]);
    testcase( pc+sz==usable_ناپ );
    إذا( pc+sz>usable_ناپ ){
      أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
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
ساكن صحيح btree_Init_ورقہ(Mem_ورقہ *p_ورقہ){
  u8 *انکڑے;          /* Equal to pPage->aData */
  BtShared *pBt;        /* The main btree structure */

  أكد( p_ورقہ->pBt!=0 );
  أكد( p_ورقہ->pBt->db!=0 );
  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  أكد( p_ورقہ->pgno==sqlite3PagerPagenumber(p_ورقہ->p_Db_ورقہ) );
  أكد( p_ورقہ == sqlite3PagerGetExtra(p_ورقہ->p_Db_ورقہ) );
  أكد( p_ورقہ->a_انکڑے == sqlite3_Pager_Get_انکڑے(p_ورقہ->p_Db_ورقہ) );
  أكد( p_ورقہ->isInit==0 );

  pBt = p_ورقہ->pBt;
  انکڑے = p_ورقہ->a_انکڑے + p_ورقہ->hdr_وتھ;
  /* EVIDENCE-OF: R-28594-02890 The one-byte flag at offset 0 indicating
  ** the b-tree page type. */
  إذا( decodeFlags(p_ورقہ, انکڑے[0]) ){
    أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
  }
  أكد( pBt->ورقہ_ناپ>=512 && pBt->ورقہ_ناپ<=65536 );
  p_ورقہ->نقاب_ورقہ = (u16)(pBt->ورقہ_ناپ - 1);
  p_ورقہ->n_حد_توں_ودھ = 0;
  p_ورقہ->cell_وتھ = (u16)(p_ورقہ->hdr_وتھ + 8 + p_ورقہ->بچہ_Ptr_ناپ);
  p_ورقہ->aCellIdx = انکڑے + p_ورقہ->بچہ_Ptr_ناپ + 8;
  p_ورقہ->a_انکڑے_سرا = p_ورقہ->a_انکڑے + pBt->ورقہ_ناپ;
  p_ورقہ->a_انکڑے_Ofst = p_ورقہ->a_انکڑے + p_ورقہ->بچہ_Ptr_ناپ;
  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  p_ورقہ->nCell = get2byte(&انکڑے[3]);
  إذا( p_ورقہ->nCell>MX_CELL(pBt) ){
    /* To many cells for a single page.  The page must be corrupt */
    أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
  }
  testcase( p_ورقہ->nCell==MX_CELL(pBt) );
  /* EVIDENCE-OF: R-24089-57979 If a page contains no cells (which is only
  ** possible for a root page of a table that contains no rows) then the
  ** offset to the cell content area will equal the page size minus the
  ** bytes of reserved space. */
  أكد( p_ورقہ->nCell>0
       || get2byteNotZero(&انکڑے[5])==(صحيح)pBt->usable_ناپ
       || CORRUPT_DB );
  p_ورقہ->nFree = -1;  /* Indicate that this value is yet uncomputed */
  p_ورقہ->isInit = 1;
  إذا( pBt->db->flags & SQLITE_Cell_ناپ_Ck ){
    أرجع btree_Cell_ناپ_Check(p_ورقہ);
  }
  أرجع SQLITE_OK;
}

/*
** Set up a raw page so that it looks like a database page holding
** no entries.
*/
ساكن فراغ zero_ورقہ(Mem_ورقہ *p_ورقہ, صحيح flags){
  بنانشانی حرف *انکڑے = p_ورقہ->a_انکڑے;
  BtShared *pBt = p_ورقہ->pBt;
  صحيح hdr = p_ورقہ->hdr_وتھ;
  صحيح پہلا;

  أكد( sqlite3PagerPagenumber(p_ورقہ->p_Db_ورقہ)==p_ورقہ->pgno || CORRUPT_DB );
  أكد( sqlite3PagerGetExtra(p_ورقہ->p_Db_ورقہ) == (فراغ*)p_ورقہ );
  أكد( sqlite3_Pager_Get_انکڑے(p_ورقہ->p_Db_ورقہ) == انکڑے );
  أكد( sqlite3PagerIswriteable(p_ورقہ->p_Db_ورقہ) );
  أكد( sqlite3_mutex_held(pBt->mutex) );
  إذا( pBt->btsFlags & BTS_FAST_SECURE ){
    memset(&انکڑے[hdr], 0, pBt->usable_ناپ - hdr);
  }
  انکڑے[hdr] = (حرف)flags;
  پہلا = hdr + ((flags&PTF_پتی)==0 ? 12 : 8);
  memset(&انکڑے[hdr+1], 0, 4);
  انکڑے[hdr+7] = 0;
  put2byte(&انکڑے[hdr+5], pBt->usable_ناپ);
  p_ورقہ->nFree = (u16)(pBt->usable_ناپ - پہلا);
  decodeFlags(p_ورقہ, flags);
  p_ورقہ->cell_وتھ = (u16)پہلا;
  p_ورقہ->a_انکڑے_سرا = &انکڑے[pBt->ورقہ_ناپ];
  p_ورقہ->aCellIdx = &انکڑے[پہلا];
  p_ورقہ->a_انکڑے_Ofst = &انکڑے[p_ورقہ->بچہ_Ptr_ناپ];
  p_ورقہ->n_حد_توں_ودھ = 0;
  أكد( pBt->ورقہ_ناپ>=512 && pBt->ورقہ_ناپ<=65536 );
  p_ورقہ->نقاب_ورقہ = (u16)(pBt->ورقہ_ناپ - 1);
  p_ورقہ->nCell = 0;
  p_ورقہ->isInit = 1;
}


/*
** Convert a DbPage obtained from the pager into a MemPage used by
** the btree layer.
*/
ساكن Mem_ورقہ *btree_ورقہ_From_Db_ورقہ(Db_ورقہ *p_Db_ورقہ, Pgno pgno, BtShared *pBt){
  Mem_ورقہ *p_ورقہ = (Mem_ورقہ*)sqlite3PagerGetExtra(p_Db_ورقہ);
  إذا( pgno!=p_ورقہ->pgno ){
    p_ورقہ->a_انکڑے = sqlite3_Pager_Get_انکڑے(p_Db_ورقہ);
    p_ورقہ->p_Db_ورقہ = p_Db_ورقہ;
    p_ورقہ->pBt = pBt;
    p_ورقہ->pgno = pgno;
    p_ورقہ->hdr_وتھ = pgno==1 ? 100 : 0;
  }
  أكد( p_ورقہ->a_انکڑے==sqlite3_Pager_Get_انکڑے(p_Db_ورقہ) );
  أرجع p_ورقہ;
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
ساكن صحيح btree_Get_ورقہ(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  Mem_ورقہ **pp_ورقہ,    /* Return the page in this parameter */
  صحيح flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  صحيح rc;
  Db_ورقہ *p_Db_ورقہ;

  أكد( flags==0 || flags==PAGER_GET_NOCONTENT || flags==PAGER_GET_READONLY );
  أكد( sqlite3_mutex_held(pBt->mutex) );
  rc = sqlite3PagerGet(pBt->pPager, pgno, (Db_ورقہ**)&p_Db_ورقہ, flags);
  إذا( rc ) أرجع rc;
  *pp_ورقہ = btree_ورقہ_From_Db_ورقہ(p_Db_ورقہ, pgno, pBt);
  أرجع SQLITE_OK;
}

/*
** Retrieve a page from the pager cache. If the requested page is not
** already in the pager cache return NULL. Initialize the MemPage.pBt and
** MemPage.aData elements if needed.
*/
ساكن Mem_ورقہ *btree_ورقہ_Lookup(BtShared *pBt, Pgno pgno){
  Db_ورقہ *p_Db_ورقہ;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  p_Db_ورقہ = sqlite3PagerLookup(pBt->pPager, pgno);
  إذا( p_Db_ورقہ ){
    أرجع btree_ورقہ_From_Db_ورقہ(p_Db_ورقہ, pgno, pBt);
  }
  أرجع 0;
}

/*
** Return the size of the database file in pages. If there is any kind of
** error, return ((unsigned int)-1).
*/
ساكن Pgno btreePagecount(BtShared *pBt){
  أرجع pBt->n_ورقہ;
}
Pgno sqlite3_Btree_آخری_ورقہ(Btree *p){
  أكد( sqlite3BtreeHoldsMutex(p) );
  أرجع btreePagecount(p->pBt);
}

/*
** Get a page from the pager and initialize it.
*/
ساكن صحيح get_And_Init_ورقہ(
  BtShared *pBt,                  /* The database file */
  Pgno pgno,                      /* Number of the page to get */
  Mem_ورقہ **pp_ورقہ,               /* Write the page pointer here */
  صحيح bReadOnly                   /* True for a read-only page */
){
  صحيح rc;
  Db_ورقہ *p_Db_ورقہ;
  Mem_ورقہ *p_ورقہ;
  أكد( sqlite3_mutex_held(pBt->mutex) );

  إذا( pgno>btreePagecount(pBt) ){
    *pp_ورقہ = 0;
    أرجع SQLITE_CORRUPT_BKPT;
  }
  rc = sqlite3PagerGet(pBt->pPager, pgno, (Db_ورقہ**)&p_Db_ورقہ, bReadOnly);
  إذا( rc ){
    *pp_ورقہ = 0;
    أرجع rc;
  }
  p_ورقہ = (Mem_ورقہ*)sqlite3PagerGetExtra(p_Db_ورقہ);
  إذا( p_ورقہ->isInit==0 ){
    btree_ورقہ_From_Db_ورقہ(p_Db_ورقہ, pgno, pBt);
    rc = btree_Init_ورقہ(p_ورقہ);
    إذا( rc!=SQLITE_OK ){
      release_ورقہ(p_ورقہ);
      *pp_ورقہ = 0;
      أرجع rc;
    }
  }
  أكد( p_ورقہ->pgno==pgno || CORRUPT_DB );
  أكد( p_ورقہ->a_انکڑے==sqlite3_Pager_Get_انکڑے(p_Db_ورقہ) );
  *pp_ورقہ = p_ورقہ;
  أرجع SQLITE_OK;
}

/*
** Release a MemPage.  This should be called once for each prior
** call to btreeGetPage.
**
** Page1 is a special case and must be released using releasePageOne().
*/
ساكن فراغ release_ورقہ_Not_Null(Mem_ورقہ *p_ورقہ){
  أكد( p_ورقہ->a_انکڑے );
  أكد( p_ورقہ->pBt );
  أكد( p_ورقہ->p_Db_ورقہ!=0 );
  أكد( sqlite3PagerGetExtra(p_ورقہ->p_Db_ورقہ) == (فراغ*)p_ورقہ );
  أكد( sqlite3_Pager_Get_انکڑے(p_ورقہ->p_Db_ورقہ)==p_ورقہ->a_انکڑے );
  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  sqlite3PagerUnrefNotNull(p_ورقہ->p_Db_ورقہ);
}
ساكن فراغ release_ورقہ(Mem_ورقہ *p_ورقہ){
  إذا( p_ورقہ ) release_ورقہ_Not_Null(p_ورقہ);
}
ساكن فراغ release_ورقہ_One(Mem_ورقہ *p_ورقہ){
  أكد( p_ورقہ!=0 );
  أكد( p_ورقہ->a_انکڑے );
  أكد( p_ورقہ->pBt );
  أكد( p_ورقہ->p_Db_ورقہ!=0 );
  أكد( sqlite3PagerGetExtra(p_ورقہ->p_Db_ورقہ) == (فراغ*)p_ورقہ );
  أكد( sqlite3_Pager_Get_انکڑے(p_ورقہ->p_Db_ورقہ)==p_ورقہ->a_انکڑے );
  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  sqlite3_Pager_Unref_ورقہ_One(p_ورقہ->p_Db_ورقہ);
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
ساكن صحيح btree_Get_بن_ورتیا_ورقہ(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  Mem_ورقہ **pp_ورقہ,    /* Return the page in this parameter */
  صحيح flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  صحيح rc = btree_Get_ورقہ(pBt, pgno, pp_ورقہ, flags);
  إذا( rc==SQLITE_OK ){
    إذا( sqlite3_Pager_ورقہ_Refcount((*pp_ورقہ)->p_Db_ورقہ)>1 ){
      release_ورقہ(*pp_ورقہ);
      *pp_ورقہ = 0;
      أرجع SQLITE_CORRUPT_BKPT;
    }
    (*pp_ورقہ)->isInit = 0;
  }وإلا{
    *pp_ورقہ = 0;
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
ساكن فراغ ورقہ_Reinit(Db_ورقہ *p_انکڑے){
  Mem_ورقہ *p_ورقہ;
  p_ورقہ = (Mem_ورقہ *)sqlite3PagerGetExtra(p_انکڑے);
  أكد( sqlite3_Pager_ورقہ_Refcount(p_انکڑے)>0 );
  إذا( p_ورقہ->isInit ){
    أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
    p_ورقہ->isInit = 0;
    إذا( sqlite3_Pager_ورقہ_Refcount(p_انکڑے)>1 ){
      /* pPage might not be a btree page;  it might be an overflow page
      ** or ptrmap page or a free page.  In those cases, the following
      ** call to btreeInitPage() will likely return SQLITE_CORRUPT.
      ** But no harm is done by this.  And it is very important that
      ** btreeInitPage() be called on every btree page so we make
      ** the call for every page that comes in for re-initializing. */
      btree_Init_ورقہ(p_ورقہ);
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
  بنانشانی حرف z_Db_سرخی[100];  /* Database header content */

  /* True if opening an ephemeral, temporary database */
  ثابت صحيح is_عارضی_Db = zFilename==0 || zFilename[0]==0;

  /* Set the variable isMemdb to true for an in-memory database, or
  ** false for a file-based database.
  */
#جے_تعریف_شدہ SQLITE_OMIT_MEMORYDB
  ثابت صحيح isMemdb = 0;
#وإلا
  ثابت صحيح isMemdb = (zFilename && strcmp(zFilename, ":memory:")==0)
                       || (is_عارضی_Db && sqlite3_عارضی_وچ_حافظہ(db))
                       || (vfsFlags & SQLITE_OPEN_حافظہ)!=0;
#جے_انت

  أكد( db!=0 );
  أكد( pVfs!=0 );
  أكد( sqlite3_mutex_held(db->mutex) );
  أكد( (flags&0xff)==flags );   /* flags fit in 8 bits */

  /* Only a BTREE_SINGLE database can be BTREE_UNORDERED */
  أكد( (flags & BTREE_UNORDERED)==0 || (flags & BTREE_SINGLE)!=0 );

  /* A BTREE_SINGLE database is always a temporary and/or ephemeral */
  أكد( (flags & BTREE_SINGLE)==0 || is_عارضی_Db );

  إذا( isMemdb ){
    flags |= BTREE_حافظہ;
  }
  إذا( (vfsFlags & SQLITE_OPEN_MAIN_DB)!=0 && (isMemdb || is_عارضی_Db) ){
    vfsFlags = (vfsFlags & ~SQLITE_OPEN_MAIN_DB) | SQLITE_OPEN_عارضی_DB;
  }
  p = sqlite3MallocZero(حجم(Btree));
  إذا( !p ){
    أرجع SQLITE_NOMEM_BKPT;
  }
  p->وچ_Trans = TRANS_NONE;
  p->db = db;
#جے_بنا_تعریف SQLITE_OMIT_SHARED_CACHE
  p->جندرا.pBtree = p;
  p->جندرا.i_جدول = 1;
#جے_انت

#إذا !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
  /*
  ** If this Btree is a candidate for shared cache, try to find an
  ** existing BtShared object that we can share with
  */
  إذا( is_عارضی_Db==0 && (isMemdb==0 || (vfsFlags&SQLITE_OPEN_URI)!=0) ){
    إذا( vfsFlags & SQLITE_OPEN_SHAREDCACHE ){
      صحيح nFilename = sqlite3Strlen30(zFilename)+1;
      صحيح n_بھریا_Pathname = pVfs->mxPathname+1;
      حرف *z_بھریا_Pathname = sqlite3Malloc(MAX(n_بھریا_Pathname,nFilename));
      MUTEX_LOGIC( sqlite3_mutex *mutexShared; )

      p->sharable = 1;
      إذا( !z_بھریا_Pathname ){
        sqlite3_free(p);
        أرجع SQLITE_NOMEM_BKPT;
      }
      إذا( isMemdb ){
        حافظہ_نقل(z_بھریا_Pathname, zFilename, nFilename);
      }وإلا{
        rc = sqlite3_Os_بھریا_Pathname(pVfs, zFilename,
                                   n_بھریا_Pathname, z_بھریا_Pathname);
        إذا( rc ){
          إذا( rc==SQLITE_OK_SYMLINK ){
            rc = SQLITE_OK;
          }وإلا{
            sqlite3_free(z_بھریا_Pathname);
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
#جے_انت
      لكل(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->p_اگلا){
        أكد( pBt->nRef>0 );
        إذا( 0==strcmp(z_بھریا_Pathname, sqlite3PagerFilename(pBt->pPager, 0))
                 && sqlite3PagerVfs(pBt->pPager)==pVfs ){
          صحيح iDb;
          لكل(iDb=db->nDb-1; iDb>=0; iDb--){
            Btree *pExisting = db->aDb[iDb].pBt;
            إذا( pExisting && pExisting->pBt==pBt ){
              sqlite3_mutex_leave(mutexShared);
              sqlite3_mutex_leave(mutexOpen);
              sqlite3_free(z_بھریا_Pathname);
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
      sqlite3_free(z_بھریا_Pathname);
    }
#جے_تعریف_شدہ SQLITE_DEBUG
    وإلا{
      /* In debug mode, we mark all persistent databases as sharable
      ** even when they are not.  This exercises the locking code and
      ** gives more opportunity for asserts(sqlite3_mutex_held())
      ** statements to find locking problems.
      */
      p->sharable = 1;
    }
#جے_انت
  }
#جے_انت
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
    memset(&z_Db_سرخی[16], 0, 8);
 
    pBt = sqlite3MallocZero( حجم(*pBt) );
    إذا( pBt==0 ){
      rc = SQLITE_NOMEM_BKPT;
      اذهب btree_open_باہرلا;
    }
    rc = sqlite3PagerOpen(pVfs, &pBt->pPager, zFilename,
                          حجم(Mem_ورقہ), flags, vfsFlags, ورقہ_Reinit);
    إذا( rc==SQLITE_OK ){
      sqlite3_Pager_سیٹ_Mmap_حد_بندی(pBt->pPager, db->szMmap);
      rc = sqlite3PagerReadFileheader(pBt->pPager,حجم(z_Db_سرخی),z_Db_سرخی);
    }
    إذا( rc!=SQLITE_OK ){
      اذهب btree_open_باہرلا;
    }
    pBt->openFlags = (u8)flags;
    pBt->db = db;
    sqlite3_Pager_سیٹ_Busy_Handler(pBt->pPager, btreeInvokeBusyHandler, pBt);
    p->pBt = pBt;
 
    pBt->pCursor = 0;
    pBt->pPage1 = 0;
    إذا( sqlite3PagerIsreadonly(pBt->pPager) ) pBt->btsFlags |= BTS_READ_ONLY;
#إذا defined(SQLITE_SECURE_DELETE)
    pBt->btsFlags |= BTS_SECURE_DELETE;
#وإلاإذا defined(SQLITE_FAST_SECURE_DELETE)
    pBt->btsFlags |= BTS_OVERWRITE;
#جے_انت
    /* EVIDENCE-OF: R-51873-39618 The page size for a database file is
    ** determined by the 2-byte integer located at an offset of 16 bytes from
    ** the beginning of the database file. */
    pBt->ورقہ_ناپ = (z_Db_سرخی[16]<<8) | (z_Db_سرخی[17]<<16);
    إذا( pBt->ورقہ_ناپ<512 || pBt->ورقہ_ناپ>SQLITE_MAX_ورقہ_ناپ
         || ((pBt->ورقہ_ناپ-1)&pBt->ورقہ_ناپ)!=0 ){
      pBt->ورقہ_ناپ = 0;
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
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
#جے_انت
      nReserve = 0;
    }وإلا{
      /* EVIDENCE-OF: R-37497-42412 The size of the reserved region is
      ** determined by the one-byte unsigned integer found at an offset of 20
      ** into the database file header. */
      nReserve = z_Db_سرخی[20];
      pBt->btsFlags |= BTS_PAGESIZE_FIXED;
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
      pBt->autoVacuum = (get4byte(&z_Db_سرخی[36 + 4*4])?1:0);
      pBt->incrVacuum = (get4byte(&z_Db_سرخی[36 + 7*4])?1:0);
#جے_انت
    }
    rc = sqlite3_Pager_سیٹ_Pagesize(pBt->pPager, &pBt->ورقہ_ناپ, nReserve);
    إذا( rc ) اذهب btree_open_باہرلا;
    pBt->usable_ناپ = pBt->ورقہ_ناپ - nReserve;
    أكد( (pBt->ورقہ_ناپ & 7)==0 );  /* 8-byte alignment of pageSize */
  
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
          اذهب btree_open_باہرلا;
        }
      }
      sqlite3_mutex_enter(mutexShared);
      pBt->p_اگلا = GLOBAL(BtShared*,sqlite3SharedCacheList);
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt;
      sqlite3_mutex_leave(mutexShared);
    }
#جے_انت
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
          p->p_اگلا = pSib;
          p->pPrev = 0;
          pSib->pPrev = p;
        }وإلا{
          طالما( pSib->p_اگلا && (uptr)pSib->p_اگلا->pBt<(uptr)p->pBt ){
            pSib = pSib->p_اگلا;
          }
          p->p_اگلا = pSib->p_اگلا;
          p->pPrev = pSib;
          إذا( p->p_اگلا ){
            p->p_اگلا->pPrev = p;
          }
          pSib->p_اگلا = p;
        }
        اكسر;
      }
    }
  }
#جے_انت
  *ppBtree = p;

btree_open_باہرلا:
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
      sqlite3_Btree_سیٹ_Cache_ناپ(p, SQLITE_DEFAULT_CACHE_ناپ);
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
  أكد( rc!=SQLITE_OK || sqlite3_Btree_Connection_گنتی(*ppBtree)>0 );
  أرجع rc;
}

/*
** Decrement the BtShared.nRef counter.  When it reaches zero,
** remove the BtShared structure from the sharing list.  Return
** true if the BtShared.nRef counter reaches zero and return
** false if it is still positive.
*/
ساكن صحيح removeFromSharingList(BtShared *pBt){
#جے_بنا_تعریف SQLITE_OMIT_SHARED_CACHE
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )
  BtShared *pList;
  صحيح removed = 0;

  أكد( sqlite3_mutex_notheld(pBt->mutex) );
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  pBt->nRef--;
  إذا( pBt->nRef<=0 ){
    إذا( GLOBAL(BtShared*,sqlite3SharedCacheList)==pBt ){
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt->p_اگلا;
    }وإلا{
      pList = GLOBAL(BtShared*,sqlite3SharedCacheList);
      طالما( ALWAYS(pList) && pList->p_اگلا!=pBt ){
        pList=pList->p_اگلا;
      }
      إذا( ALWAYS(pList) ){
        pList->p_اگلا = pBt->p_اگلا;
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
  بن_ورتیا_PARAMETER( pBt );
  أرجع 1;
#جے_انت
}

/*
** Make sure pBt->pTmpSpace points to an allocation of
** MX_CELL_SIZE(pBt) bytes with a 4-byte prefix for a left-child
** pointer.
*/
ساكن SQLITE_NOINLINE صحيح allocate_عارضی_Space(BtShared *pBt){
  أكد( pBt!=0 );
  أكد( pBt->pTmpSpace==0 );
  /* This routine is called only by btreeCursor() when allocating the
  ** first write cursor for the BtShared object */
  أكد( pBt->pCursor!=0 && (pBt->pCursor->curFlags & BTCF_WriteFlag)!=0 );
  pBt->pTmpSpace = sqlite3_ورقہ_Malloc( pBt->ورقہ_ناپ );
  إذا( pBt->pTmpSpace==0 ){
    BtCursor *pCur = pBt->pCursor;
    pBt->pCursor = pCur->p_اگلا;  /* Unlink the cursor */
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
ساكن فراغ free_عارضی_Space(BtShared *pBt){
  إذا( pBt->pTmpSpace ){
    pBt->pTmpSpace -= 4;
    sqlite3_ورقہ_Free(pBt->pTmpSpace);
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
#جے_تعریف_شدہ SQLITE_DEBUG
  {
    BtCursor *pCur = pBt->pCursor;
    طالما( pCur ){
      BtCursor *pTmp = pCur;
      pCur = pCur->p_اگلا;
      أكد( pTmp->pBtree!=p );

    }
  }
#جے_انت

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
  أكد( p->want_منزل_جندرا==0 && p->locked==0 );
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
    free_عارضی_Space(pBt);
    sqlite3_free(pBt);
  }

#جے_بنا_تعریف SQLITE_OMIT_SHARED_CACHE
  أكد( p->want_منزل_جندرا==0 );
  أكد( p->locked==0 );
  إذا( p->pPrev ) p->pPrev->p_اگلا = p->p_اگلا;
  إذا( p->p_اگلا ) p->p_اگلا->pPrev = p->pPrev;
#جے_انت

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
صحيح sqlite3_Btree_سیٹ_Cache_ناپ(Btree *p, صحيح mx_ورقہ){
  BtShared *pBt = p->pBt;
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_سیٹ_Cachesize(pBt->pPager, mx_ورقہ);
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
صحيح sqlite3_Btree_سیٹ_Spill_ناپ(Btree *p, صحيح mx_ورقہ){
  BtShared *pBt = p->pBt;
  صحيح res;
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  res = sqlite3_Pager_سیٹ_Spillsize(pBt->pPager, mx_ورقہ);
  sqlite3BtreeLeave(p);
  أرجع res;
}

#إذا SQLITE_MAX_MMAP_ناپ>0
/*
** Change the limit on the amount of the database file that may be
** memory mapped.
*/
صحيح sqlite3_Btree_سیٹ_Mmap_حد_بندی(Btree *p, sqlite3_int64 szMmap){
  BtShared *pBt = p->pBt;
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_سیٹ_Mmap_حد_بندی(pBt->pPager, szMmap);
  sqlite3BtreeLeave(p);
  أرجع SQLITE_OK;
}
#جے_انت /* SQLITE_MAX_MMAP_SIZE>0 */

/*
** Change the way data is synced to disk in order to increase or decrease
** how well the database resists damage due to OS crashes and power
** failures.  Level 1 is the same as asynchronous (no syncs() occur and
** there is a high probability of damage)  Level 2 is the default.  There
** is a very low but non-zero probability of damage.  Level 3 reduces the
** probability of damage to near zero but with a write performance reduction.
*/
#جے_بنا_تعریف SQLITE_OMIT_PAGER_PRAGMAS
صحيح sqlite3_Btree_سیٹ_Pager_Flags(
  Btree *p,              /* The btree to set the safety level on */
  بنانشانی pgFlags       /* Various PAGER_* flags */
){
  BtShared *pBt = p->pBt;
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_سیٹ_Flags(pBt->pPager, pgFlags);
  sqlite3BtreeLeave(p);
  أرجع SQLITE_OK;
}
#جے_انت

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
صحيح sqlite3_Btree_سیٹ_ورقہ_ناپ(Btree *p, صحيح ورقہ_ناپ, صحيح nReserve, صحيح iFix){
  صحيح rc = SQLITE_OK;
  صحيح x;
  BtShared *pBt = p->pBt;
  أكد( nReserve>=0 && nReserve<=255 );
  sqlite3BtreeEnter(p);
  pBt->nReserveWanted = (u8)nReserve;
  x = pBt->ورقہ_ناپ - pBt->usable_ناپ;
  إذا( x==nReserve && (ورقہ_ناپ==0 || (u32)ورقہ_ناپ==pBt->ورقہ_ناپ) ){
    sqlite3BtreeLeave(p);
    أرجع SQLITE_OK;
  }
  إذا( nReserve<x ) nReserve = x;
  إذا( pBt->btsFlags & BTS_PAGESIZE_FIXED ){
    sqlite3BtreeLeave(p);
    أرجع SQLITE_READONLY;
  }
  أكد( nReserve>=0 && nReserve<=255 );
  إذا( ورقہ_ناپ>=512 && ورقہ_ناپ<=SQLITE_MAX_ورقہ_ناپ &&
        ((ورقہ_ناپ-1)&ورقہ_ناپ)==0 ){
    أكد( (ورقہ_ناپ & 7)==0 );
    أكد( !pBt->pCursor );
    إذا( nReserve>32 && ورقہ_ناپ==512 ) ورقہ_ناپ = 1024;
    pBt->ورقہ_ناپ = (u32)ورقہ_ناپ;
    free_عارضی_Space(pBt);
  }
  rc = sqlite3_Pager_سیٹ_Pagesize(pBt->pPager, &pBt->ورقہ_ناپ, nReserve);
  pBt->usable_ناپ = pBt->ورقہ_ناپ - (u16)nReserve;
  إذا( iFix ) pBt->btsFlags |= BTS_PAGESIZE_FIXED;
  sqlite3BtreeLeave(p);
  أرجع rc;
}

/*
** Return the currently defined page size
*/
صحيح sqlite3_Btree_Get_ورقہ_ناپ(Btree *p){
  أرجع p->pBt->ورقہ_ناپ;
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
  n = p->pBt->ورقہ_ناپ - p->pBt->usable_ناپ;
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
Pgno sqlite3_Btree_Max_ورقہ_گنتی(Btree *p, Pgno mx_ورقہ){
  Pgno n;
  sqlite3BtreeEnter(p);
  n = sqlite3_Pager_Max_ورقہ_گنتی(p->pBt->pPager, mx_ورقہ);
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
صحيح sqlite3_Btree_سیٹ_Auto_Vacuum(Btree *p, صحيح autoVacuum){
#جے_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
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
#جے_انت
}

/*
** Return the value of the 'auto-vacuum' property. If auto-vacuum is
** enabled 1 is returned. Otherwise 0.
*/
صحيح sqlite3BtreeGetAutoVacuum(Btree *p){
#جے_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  أرجع BTREE_AUTOVACUUM_NONE;
#وإلا
  صحيح rc;
  sqlite3BtreeEnter(p);
  rc = (
    (!p->pBt->autoVacuum)?BTREE_AUTOVACUUM_NONE:
    (!p->pBt->incrVacuum)?BTREE_AUTOVACUUM_بھریا:
    BTREE_AUTOVACUUM_INCR
  );
  sqlite3BtreeLeave(p);
  أرجع rc;
#جے_انت
}

/*
** If the user has not set the safety-level for this database connection
** using "PRAGMA synchronous", and if the safety-level is not already
** set to the value passed to this function as the second parameter,
** set it so.
*/
#إذا SQLITE_DEFAULT_SYNCHRONOUS!=SQLITE_DEFAULT_WAL_SYNCHRONOUS \
    && !defined(SQLITE_OMIT_WAL)
ساكن فراغ سیٹ_Default_Sync_Flag(BtShared *pBt, u8 safety_level){
  sqlite3 *db;
  Db *pDb;
  إذا( (db=pBt->db)!=0 && (pDb=db->aDb)!=0 ){
    طالما( pDb->pBt==0 || pDb->pBt->pBt!=pBt ){ pDb++; }
    إذا( pDb->b_Sync_سیٹ==0
     && pDb->safety_level!=safety_level
     && pDb!=&db->aDb[1]
    ){
      pDb->safety_level = safety_level;
      sqlite3_Pager_سیٹ_Flags(pBt->pPager,
          pDb->safety_level | (db->flags & PAGER_FLAGS_نقاب));
    }
  }
}
#وإلا
# define سیٹ_Default_Sync_Flag(pBt,safety_level)
#جے_انت

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
ساكن صحيح جندرا_Btree(BtShared *pBt){
  صحيح rc;              /* Result code from subfunctions */
  Mem_ورقہ *pPage1;     /* Page 1 of the database file */
  u32 n_ورقہ;           /* Number of pages in the database */
  u32 n_ورقہ_File = 0;   /* Number of pages in the database file */

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( pBt->pPage1==0 );
  rc = sqlite3_Pager_Shared_جندرا(pBt->pPager);
  إذا( rc!=SQLITE_OK ) أرجع rc;
  rc = btree_Get_ورقہ(pBt, 1, &pPage1, 0);
  إذا( rc!=SQLITE_OK ) أرجع rc;

  /* Do some checking to help insure the file we opened really is
  ** a valid database file.
  */
  n_ورقہ = get4byte(28+(u8*)pPage1->a_انکڑے);
  sqlite3PagerPagecount(pBt->pPager, (صحيح*)&n_ورقہ_File);
  إذا( n_ورقہ==0 || memcmp(24+(u8*)pPage1->a_انکڑے, 92+(u8*)pPage1->a_انکڑے,4)!=0 ){
    n_ورقہ = n_ورقہ_File;
  }
  إذا( (pBt->db->flags & SQLITE_ResetDatabase)!=0 ){
    n_ورقہ = 0;
  }
  إذا( n_ورقہ>0 ){
    u32 ورقہ_ناپ;
    u32 usable_ناپ;
    u8 *page1 = pPage1->a_انکڑے;
    rc = SQLITE_NOTADB;
    /* EVIDENCE-OF: R-43737-39999 Every valid SQLite database file begins
    ** with the following 16 bytes (in hex): 53 51 4c 69 74 65 20 66 6f 72 6d
    ** 61 74 20 33 00. */
    إذا( memcmp(page1, z_Magic_سرخی, 16)!=0 ){
      اذهب page1_init_failed;
    }

#جے_تعریف_شدہ SQLITE_OMIT_WAL
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
        سیٹ_Default_Sync_Flag(pBt, SQLITE_DEFAULT_WAL_SYNCHRONOUS+1);
        إذا( isOpen==0 ){
          release_ورقہ_One(pPage1);
          أرجع SQLITE_OK;
        }
      }
      rc = SQLITE_NOTADB;
    }وإلا{
      سیٹ_Default_Sync_Flag(pBt, SQLITE_DEFAULT_SYNCHRONOUS+1);
    }
#جے_انت

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
    ورقہ_ناپ = (page1[16]<<8) | (page1[17]<<16);
    /* EVIDENCE-OF: R-25008-21688 The size of a page is a power of two
    ** between 512 and 65536 inclusive. */
    إذا( ((ورقہ_ناپ-1)&ورقہ_ناپ)!=0
     || ورقہ_ناپ>SQLITE_MAX_ورقہ_ناپ
     || ورقہ_ناپ<=256
    ){
      اذهب page1_init_failed;
    }
    أكد( (ورقہ_ناپ & 7)==0 );
    /* EVIDENCE-OF: R-59310-51205 The "reserved space" size in the 1-byte
    ** integer at offset 20 is the number of bytes of space at the end of
    ** each page to reserve for extensions.
    **
    ** EVIDENCE-OF: R-37497-42412 The size of the reserved region is
    ** determined by the one-byte unsigned integer found at an offset of 20
    ** into the database file header. */
    usable_ناپ = ورقہ_ناپ - page1[20];
    إذا( (u32)ورقہ_ناپ!=pBt->ورقہ_ناپ ){
      /* After reading the first page of the database assuming a page size
      ** of BtShared.pageSize, we have discovered that the page-size is
      ** actually pageSize. Unlock the database, leave pBt->pPage1 at
      ** zero and return SQLITE_OK. The caller will call this function
      ** again with the correct page-size.
      */
      release_ورقہ_One(pPage1);
      pBt->usable_ناپ = usable_ناپ;
      pBt->ورقہ_ناپ = ورقہ_ناپ;
      pBt->btsFlags |= BTS_PAGESIZE_FIXED;
      free_عارضی_Space(pBt);
      rc = sqlite3_Pager_سیٹ_Pagesize(pBt->pPager, &pBt->ورقہ_ناپ,
                                   ورقہ_ناپ-usable_ناپ);
      أرجع rc;
    }
    إذا( n_ورقہ>n_ورقہ_File ){
      إذا( sqlite3WritableSchema(pBt->db)==0 ){
        rc = SQLITE_CORRUPT_BKPT;
        اذهب page1_init_failed;
      }وإلا{
        n_ورقہ = n_ورقہ_File;
      }
    }
    /* EVIDENCE-OF: R-28312-64704 However, the usable size is not allowed to
    ** be less than 480. In other words, if the page size is 512, then the
    ** reserved space size cannot exceed 32. */
    إذا( usable_ناپ<480 ){
      اذهب page1_init_failed;
    }
    pBt->btsFlags |= BTS_PAGESIZE_FIXED;
    pBt->ورقہ_ناپ = ورقہ_ناپ;
    pBt->usable_ناپ = usable_ناپ;
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
    pBt->autoVacuum = (get4byte(&page1[36 + 4*4])?1:0);
    pBt->incrVacuum = (get4byte(&page1[36 + 7*4])?1:0);
#جے_انت
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
  pBt->maxLocal = (u16)((pBt->usable_ناپ-12)*64/255 - 23);
  pBt->minLocal = (u16)((pBt->usable_ناپ-12)*32/255 - 23);
  pBt->max_پتی = (u16)(pBt->usable_ناپ - 35);
  pBt->min_پتی = (u16)((pBt->usable_ناپ-12)*32/255 - 23);
  إذا( pBt->maxLocal>127 ){
    pBt->max1bytePayload = 127;
  }وإلا{
    pBt->max1bytePayload = (u8)pBt->maxLocal;
  }
  أكد( pBt->max_پتی + 23 <= MX_CELL_ناپ(pBt) );
  pBt->pPage1 = pPage1;
  pBt->n_ورقہ = n_ورقہ;
  أرجع SQLITE_OK;

page1_init_failed:
  release_ورقہ_One(pPage1);
  pBt->pPage1 = 0;
  أرجع rc;
}

#جے_بنا_تعریف NDEBUG
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
ساكن صحيح گنتی_Valid_Cursors(BtShared *pBt, صحيح wrOnly){
  BtCursor *pCur;
  صحيح r = 0;
  لكل(pCur=pBt->pCursor; pCur; pCur=pCur->p_اگلا){
    إذا( (wrOnly==0 || (pCur->curFlags & BTCF_WriteFlag)!=0)
     && pCur->eState!=CURSOR_FAULT ) r++;
  }
  أرجع r;
}
#جے_انت

/*
** If there are no outstanding cursors and we are not in the middle
** of a transaction but there is a read lock on the database, then
** this routine unrefs the first page of the database file which
** has the effect of releasing the read lock.
**
** If there is a transaction in progress, this routine is a no-op.
*/
ساكن فراغ unlock_Btree_If_بن_ورتیا(BtShared *pBt){
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( گنتی_Valid_Cursors(pBt,0)==0 || pBt->وچ_Transaction>TRANS_NONE );
  إذا( pBt->وچ_Transaction==TRANS_NONE && pBt->pPage1!=0 ){
    Mem_ورقہ *pPage1 = pBt->pPage1;
    أكد( pPage1->a_انکڑے );
    أكد( sqlite3PagerRefcount(pBt->pPager)==1 );
    pBt->pPage1 = 0;
    release_ورقہ_One(pPage1);
  }
}

/*
** If pBt points to an empty file then convert that empty file
** into a new empty database by initializing the first page of
** the database.
*/
ساكن صحيح newDatabase(BtShared *pBt){
  Mem_ورقہ *pP1;
  بنانشانی حرف *انکڑے;
  صحيح rc;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  إذا( pBt->n_ورقہ>0 ){
    أرجع SQLITE_OK;
  }
  pP1 = pBt->pPage1;
  أكد( pP1!=0 );
  انکڑے = pP1->a_انکڑے;
  rc = sqlite3PagerWrite(pP1->p_Db_ورقہ);
  إذا( rc ) أرجع rc;
  حافظہ_نقل(انکڑے, z_Magic_سرخی, حجم(z_Magic_سرخی));
  أكد( حجم(z_Magic_سرخی)==16 );
  انکڑے[16] = (u8)((pBt->ورقہ_ناپ>>8)&0xff);
  انکڑے[17] = (u8)((pBt->ورقہ_ناپ>>16)&0xff);
  انکڑے[18] = 1;
  انکڑے[19] = 1;
  أكد( pBt->usable_ناپ<=pBt->ورقہ_ناپ && pBt->usable_ناپ+255>=pBt->ورقہ_ناپ);
  انکڑے[20] = (u8)(pBt->ورقہ_ناپ - pBt->usable_ناپ);
  انکڑے[21] = 64;
  انکڑے[22] = 32;
  انکڑے[23] = 32;
  memset(&انکڑے[24], 0, 100-24);
  zero_ورقہ(pP1, PTF_INTKEY|PTF_پتی|PTF_LEAFDATA );
  pBt->btsFlags |= BTS_PAGESIZE_FIXED;
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
  أكد( pBt->autoVacuum==1 || pBt->autoVacuum==0 );
  أكد( pBt->incrVacuum==1 || pBt->incrVacuum==0 );
  put4byte(&انکڑے[36 + 4*4], pBt->autoVacuum);
  put4byte(&انکڑے[36 + 7*4], pBt->incrVacuum);
#جے_انت
  pBt->n_ورقہ = 1;
  انکڑے[31] = 1;
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
  p->pBt->n_ورقہ = 0;
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
  صحيح *p_Schema_نسخہ       /* Put schema version number here, if not NULL */
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
  إذا( p->وچ_Trans==TRANS_WRITE || (p->وچ_Trans==TRANS_READ && !wrflag) ){
    اذهب trans_begun;
  }
  أكد( pBt->وچ_Transaction==TRANS_WRITE || IfNotOmitAV(pBt->bDoTruncate)==0 );

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

#جے_بنا_تعریف SQLITE_OMIT_SHARED_CACHE
  {
    sqlite3 *p_بلاک = 0;
    /* If another database handle has already opened a write transaction
    ** on this shared-btree structure and a second write transaction is
    ** requested, return SQLITE_LOCKED.
    */
    إذا( (wrflag && pBt->وچ_Transaction==TRANS_WRITE)
     || (pBt->btsFlags & BTS_PENDING)!=0
    ){
      p_بلاک = pBt->pWriter->db;
    }وإلا إذا( wrflag>1 ){
      Bt_جندرا *pIter;
      لكل(pIter=pBt->p_جندرا; pIter; pIter=pIter->p_اگلا){
        إذا( pIter->pBtree!=p ){
          p_بلاک = pIter->pBtree->db;
          اكسر;
        }
      }
    }
    إذا( p_بلاک ){
      sqlite3ConnectionBlocked(p->db, p_بلاک);
      rc = SQLITE_LOCKED_SHAREDCACHE;
      اذهب trans_begun;
    }
  }
#جے_انت

  /* Any read-only or read-write transaction implies a read-lock on
  ** page 1. So if some other shared-cache client already has a write-lock
  ** on page 1, the transaction cannot be opened. */
  rc = query_Shared_Cache_جدول_جندرا(p, SCHEMA_جڑ, READ_جندرا);
  إذا( SQLITE_OK!=rc ) اذهب trans_begun;

  pBt->btsFlags &= ~BTS_INITIALLY_خالی_تھاواں;
  إذا( pBt->n_ورقہ==0 ) pBt->btsFlags |= BTS_INITIALLY_خالی_تھاواں;
  افعل {
    sqlite3PagerWalDb(pPager, p->db);

#جے_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
    /* If transitioning from no transaction directly to a write transaction,
    ** block for the WRITER lock first if possible. */
    إذا( pBt->pPage1==0 && wrflag ){
      أكد( pBt->وچ_Transaction==TRANS_NONE );
      rc = sqlite3_Pager_Wal_Write_جندرا(pPager, 1);
      إذا( rc!=SQLITE_BUSY && rc!=SQLITE_OK ) اكسر;
    }
#جے_انت

#جے_تعریف_شدہ SQLITE_EXPERIMENTAL_PRAGMA_20251114
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
#جے_انت

    /* Call lockBtree() until either pBt->pPage1 is populated or
    ** lockBtree() returns something other than SQLITE_OK. lockBtree()
    ** may return SQLITE_OK but leave pBt->pPage1 set to 0 if after
    ** reading page 1 it discovers that the page-size of the database
    ** file is not pBt->pageSize. In this case lockBtree() will update
    ** pBt->pageSize to the page-size of the file on disk.
    */
    طالما( pBt->pPage1==0 && SQLITE_OK==(rc = جندرا_Btree(pBt)) );

    إذا( rc==SQLITE_OK && wrflag ){
      إذا( (pBt->btsFlags & BTS_READ_ONLY)!=0 ){
        rc = SQLITE_READONLY;
      }وإلا{
        rc = sqlite3PagerBegin(pPager, wrflag>1, sqlite3_عارضی_وچ_حافظہ(p->db));
        إذا( rc==SQLITE_OK ){
          rc = newDatabase(pBt);
        }وإلا إذا( rc==SQLITE_BUSY_SNAPSHOT && pBt->وچ_Transaction==TRANS_NONE ){
          /* if there was no transaction opened when this function was
          ** called and SQLITE_BUSY_SNAPSHOT is returned, change the error
          ** code to SQLITE_BUSY. */
          rc = SQLITE_BUSY;
        }
      }
    }
 
    إذا( rc!=SQLITE_OK ){
      (فراغ)sqlite3_Pager_Wal_Write_جندرا(pPager, 0);
      unlock_Btree_If_بن_ورتیا(pBt);
    }
#إذا defined(SQLITE_ENABLE_SETLK_TIMEOUT)
    إذا( rc==SQLITE_BUSY_TIMEOUT ){
      /* If a blocking lock timed out, break out of the loop here so that
      ** the busy-handler is not invoked.  */
      اكسر;
    }
#جے_انت
  }طالما( (rc&0xFF)==SQLITE_BUSY && pBt->وچ_Transaction==TRANS_NONE &&
          btreeInvokeBusyHandler(pBt) );
  sqlite3PagerWalDb(pPager, 0);
#جے_تعریف_شدہ SQLITE_ENABLE_SETLK_TIMEOUT
  إذا( rc==SQLITE_BUSY_TIMEOUT ) rc = SQLITE_BUSY;
#جے_انت

  إذا( rc==SQLITE_OK ){
    إذا( p->وچ_Trans==TRANS_NONE ){
      pBt->nTransaction++;
#جے_بنا_تعریف SQLITE_OMIT_SHARED_CACHE
      إذا( p->sharable ){
        أكد( p->جندرا.pBtree==p && p->جندرا.i_جدول==1 );
        p->جندرا.e_جندرا = READ_جندرا;
        p->جندرا.p_اگلا = pBt->p_جندرا;
        pBt->p_جندرا = &p->جندرا;
      }
#جے_انت
    }
    p->وچ_Trans = (wrflag?TRANS_WRITE:TRANS_READ);
    إذا( p->وچ_Trans>pBt->وچ_Transaction ){
      pBt->وچ_Transaction = p->وچ_Trans;
    }
    إذا( wrflag ){
      Mem_ورقہ *pPage1 = pBt->pPage1;
#جے_بنا_تعریف SQLITE_OMIT_SHARED_CACHE
      أكد( !pBt->pWriter );
      pBt->pWriter = p;
      pBt->btsFlags &= ~BTS_EXCLUSIVE;
      إذا( wrflag>1 ) pBt->btsFlags |= BTS_EXCLUSIVE;
#جے_انت

      /* If the db-size header field is incorrect (as it may be if an old
      ** client has been writing the database file), update it now. Doing
      ** this sooner rather than later means the database size can safely
      ** re-read the database size from page 1 if a savepoint or transaction
      ** rollback occurs within the transaction.
      */
      إذا( pBt->n_ورقہ!=get4byte(&pPage1->a_انکڑے[28]) ){
        rc = sqlite3PagerWrite(pPage1->p_Db_ورقہ);
        إذا( rc==SQLITE_OK ){
          put4byte(&pPage1->a_انکڑے[28], pBt->n_ورقہ);
        }
      }
    }
  }

trans_begun:
  إذا( rc==SQLITE_OK ){
    إذا( p_Schema_نسخہ ){
      *p_Schema_نسخہ = get4byte(&pBt->pPage1->a_انکڑے[40]);
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
صحيح sqlite3BtreeBeginTrans(Btree *p, صحيح wrflag, صحيح *p_Schema_نسخہ){
  BtShared *pBt;
  إذا( p->sharable
   || p->وچ_Trans==TRANS_NONE
   || (p->وچ_Trans==TRANS_READ && wrflag!=0)
  ){
    أرجع btreeBeginTrans(p,wrflag,p_Schema_نسخہ);
  }
  pBt = p->pBt;
  إذا( p_Schema_نسخہ ){
    *p_Schema_نسخہ = get4byte(&pBt->pPage1->a_انکڑے[40]);
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

#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM

/*
** Set the pointer-map entries for all children of page pPage. Also, if
** pPage contains cells that point to overflow pages, set the pointer
** map entries for the overflow pages as well.
*/
ساكن صحيح سیٹ_بچہ_Ptrmaps(Mem_ورقہ *p_ورقہ){
  صحيح i;                             /* Counter variable */
  صحيح nCell;                         /* Number of cells in page pPage */
  صحيح rc;                            /* Return code */
  BtShared *pBt = p_ورقہ->pBt;
  Pgno pgno = p_ورقہ->pgno;

  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  rc = p_ورقہ->isInit ? SQLITE_OK : btree_Init_ورقہ(p_ورقہ);
  إذا( rc!=SQLITE_OK ) أرجع rc;
  nCell = p_ورقہ->nCell;

  لكل(i=0; i<nCell; i++){
    u8 *pCell = لبھو_Cell(p_ورقہ, i);

    ptrmapPutOvflPtr(p_ورقہ, p_ورقہ, pCell, &rc);

    إذا( !p_ورقہ->پتی ){
      Pgno بچہ_Pgno = get4byte(pCell);
      ptrmapPut(pBt, بچہ_Pgno, PTRMAP_BTREE, pgno, &rc);
    }
  }

  إذا( !p_ورقہ->پتی ){
    Pgno بچہ_Pgno = get4byte(&p_ورقہ->a_انکڑے[p_ورقہ->hdr_وتھ+8]);
    ptrmapPut(pBt, بچہ_Pgno, PTRMAP_BTREE, pgno, &rc);
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
ساكن صحيح modify_ورقہ_اشارہ_گر(Mem_ورقہ *p_ورقہ, Pgno iFrom, Pgno i_منزل, u8 eType){
  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  أكد( sqlite3PagerIswriteable(p_ورقہ->p_Db_ورقہ) );
  إذا( eType==PTRMAP_OVERFLOW2 ){
    /* The pointer is always the first 4 bytes of the page in this case.  */
    إذا( get4byte(p_ورقہ->a_انکڑے)!=iFrom ){
      أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    }
    put4byte(p_ورقہ->a_انکڑے, i_منزل);
  }وإلا{
    صحيح i;
    صحيح nCell;
    صحيح rc;

    rc = p_ورقہ->isInit ? SQLITE_OK : btree_Init_ورقہ(p_ورقہ);
    إذا( rc ) أرجع rc;
    nCell = p_ورقہ->nCell;

    لكل(i=0; i<nCell; i++){
      u8 *pCell = لبھو_Cell(p_ورقہ, i);
      إذا( eType==PTRMAP_OVERFLOW1 ){
        CellInfo info;
        p_ورقہ->xParseCell(p_ورقہ, pCell, &info);
        إذا( info.nLocal<info.nPayload ){
          إذا( pCell+info.n_ناپ > p_ورقہ->a_انکڑے+p_ورقہ->pBt->usable_ناپ ){
            أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
          }
          إذا( iFrom==get4byte(pCell+info.n_ناپ-4) ){
            put4byte(pCell+info.n_ناپ-4, i_منزل);
            اكسر;
          }
        }
      }وإلا{
        إذا( pCell+4 > p_ورقہ->a_انکڑے+p_ورقہ->pBt->usable_ناپ ){
          أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
        }
        إذا( get4byte(pCell)==iFrom ){
          put4byte(pCell, i_منزل);
          اكسر;
        }
      }
    }
 
    إذا( i==nCell ){
      إذا( eType!=PTRMAP_BTREE ||
          get4byte(&p_ورقہ->a_انکڑے[p_ورقہ->hdr_وتھ+8])!=iFrom ){
        أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
      }
      put4byte(&p_ورقہ->a_انکڑے[p_ورقہ->hdr_وتھ+8], i_منزل);
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
ساكن صحيح relocate_ورقہ(
  BtShared *pBt,           /* Btree */
  Mem_ورقہ *p_Db_ورقہ,        /* Open page to move */
  u8 eType,                /* Pointer map 'type' entry for pDbPage */
  Pgno i_Ptr_ورقہ,           /* Pointer map 'page-no' entry for pDbPage */
  Pgno i_Free_ورقہ,          /* The location to move pDbPage to */
  صحيح isCommit             /* isCommit flag passed to sqlite3PagerMovepage */
){
  Mem_ورقہ *p_Ptr_ورقہ;   /* The page that contains a pointer to pDbPage */
  Pgno i_Db_ورقہ = p_Db_ورقہ->pgno;
  Pager *pPager = pBt->pPager;
  صحيح rc;

  أكد( eType==PTRMAP_OVERFLOW2 || eType==PTRMAP_OVERFLOW1 ||
      eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE );
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( p_Db_ورقہ->pBt==pBt );
  إذا( i_Db_ورقہ<3 ) أرجع SQLITE_CORRUPT_BKPT;

  /* Move page iDbPage from its current location to page number iFreePage */
  TRACE(("AUTOVACUUM: Moving %u to free page %u (ptr page %u type %u)\n",
      i_Db_ورقہ, i_Free_ورقہ, i_Ptr_ورقہ, eType));
  rc = sqlite3PagerMovepage(pPager, p_Db_ورقہ->p_Db_ورقہ, i_Free_ورقہ, isCommit);
  إذا( rc!=SQLITE_OK ){
    أرجع rc;
  }
  p_Db_ورقہ->pgno = i_Free_ورقہ;

  /* If pDbPage was a btree-page, then it may have child pages and/or cells
  ** that point to overflow pages. The pointer map entries for all these
  ** pages need to be changed.
  **
  ** If pDbPage is an overflow page, then the first 4 bytes may store a
  ** pointer to a subsequent overflow page. If this is the case, then
  ** the pointer map needs to be updated for the subsequent overflow page.
  */
  إذا( eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE ){
    rc = سیٹ_بچہ_Ptrmaps(p_Db_ورقہ);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
  }وإلا{
    Pgno اگلا_Ovfl = get4byte(p_Db_ورقہ->a_انکڑے);
    إذا( اگلا_Ovfl!=0 ){
      ptrmapPut(pBt, اگلا_Ovfl, PTRMAP_OVERFLOW2, i_Free_ورقہ, &rc);
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
    rc = btree_Get_ورقہ(pBt, i_Ptr_ورقہ, &p_Ptr_ورقہ, 0);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
    rc = sqlite3PagerWrite(p_Ptr_ورقہ->p_Db_ورقہ);
    إذا( rc!=SQLITE_OK ){
      release_ورقہ(p_Ptr_ورقہ);
      أرجع rc;
    }
    rc = modify_ورقہ_اشارہ_گر(p_Ptr_ورقہ, i_Db_ورقہ, i_Free_ورقہ, eType);
    release_ورقہ(p_Ptr_ورقہ);
    إذا( rc==SQLITE_OK ){
      ptrmapPut(pBt, i_Free_ورقہ, eType, i_Ptr_ورقہ, &rc);
    }
  }
  أرجع rc;
}

/* Forward declaration required by incrVacuumStep(). */
ساكن صحيح allocate_Btree_ورقہ(BtShared *, Mem_ورقہ **, Pgno *, Pgno, u8);

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
ساكن صحيح incr_Vacuum_قدم(BtShared *pBt, Pgno nFin, Pgno i_آخری_Pg, صحيح bCommit){
  Pgno nFreeList;           /* Number of pages still on the free-list */
  صحيح rc;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( i_آخری_Pg>nFin );

  إذا( !PTRMAP_ISPAGE(pBt, i_آخری_Pg) && i_آخری_Pg!=PENDING_BYTE_ورقہ(pBt) ){
    u8 eType;
    Pgno i_Ptr_ورقہ;

    nFreeList = get4byte(&pBt->pPage1->a_انکڑے[36]);
    إذا( nFreeList==0 ){
      أرجع SQLITE_پورا;
    }

    rc = ptrmapGet(pBt, i_آخری_Pg, &eType, &i_Ptr_ورقہ);
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
        Mem_ورقہ *pFreePg;
        rc = allocate_Btree_ورقہ(pBt, &pFreePg, &iFreePg, i_آخری_Pg, BTALLOC_EXACT);
        إذا( rc!=SQLITE_OK ){
          أرجع rc;
        }
        أكد( iFreePg==i_آخری_Pg );
        release_ورقہ(pFreePg);
      }
    } وإلا {
      Pgno iFreePg;             /* Index of free page to move pLastPg to */
      Mem_ورقہ *p_آخری_Pg;
      u8 e_منوال = BTALLOC_ANY;   /* Mode parameter for allocateBtreePage() */
      Pgno iNear = 0;           /* nearby parameter for allocateBtreePage() */

      rc = btree_Get_ورقہ(pBt, i_آخری_Pg, &p_آخری_Pg, 0);
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
        e_منوال = BTALLOC_LE;
        iNear = nFin;
      }
      افعل {
        Mem_ورقہ *pFreePg;
        Pgno db_ناپ = btreePagecount(pBt);
        rc = allocate_Btree_ورقہ(pBt, &pFreePg, &iFreePg, iNear, e_منوال);
        إذا( rc!=SQLITE_OK ){
          release_ورقہ(p_آخری_Pg);
          أرجع rc;
        }
        release_ورقہ(pFreePg);
        إذا( iFreePg>db_ناپ ){
          release_ورقہ(p_آخری_Pg);
          أرجع SQLITE_CORRUPT_BKPT;
        }
      }طالما( bCommit && iFreePg>nFin );
      أكد( iFreePg<i_آخری_Pg );
     
      rc = relocate_ورقہ(pBt, p_آخری_Pg, eType, i_Ptr_ورقہ, iFreePg, bCommit);
      release_ورقہ(p_آخری_Pg);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
    }
  }

  إذا( bCommit==0 ){
    افعل {
      i_آخری_Pg--;
    }طالما( i_آخری_Pg==PENDING_BYTE_ورقہ(pBt) || PTRMAP_ISPAGE(pBt, i_آخری_Pg) );
    pBt->bDoTruncate = 1;
    pBt->n_ورقہ = i_آخری_Pg;
  }
  أرجع SQLITE_OK;
}

/*
** The database opened by the first argument is an auto-vacuum database
** nOrig pages in size containing nFree free pages. Return the expected
** size of the database in pages following an auto-vacuum operation.
*/
ساكن Pgno final_Db_ناپ(BtShared *pBt, Pgno nOrig, Pgno nFree){
  صحيح n_اندراج;                     /* Number of entries on one ptrmap page */
  Pgno nPtrmap;                   /* Number of PtrMap pages to be freed */
  Pgno nFin;                      /* Return value */

  n_اندراج = pBt->usable_ناپ/5;
  nPtrmap = (nFree-nOrig+PTRMAP_PAGENO(pBt, nOrig)+n_اندراج)/n_اندراج;
  nFin = nOrig - nFree - nPtrmap;
  إذا( nOrig>PENDING_BYTE_ورقہ(pBt) && nFin<PENDING_BYTE_ورقہ(pBt) ){
    nFin--;
  }
  طالما( PTRMAP_ISPAGE(pBt, nFin) || nFin==PENDING_BYTE_ورقہ(pBt) ){
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
  أكد( pBt->وچ_Transaction==TRANS_WRITE && p->وچ_Trans==TRANS_WRITE );
  إذا( !pBt->autoVacuum ){
    rc = SQLITE_پورا;
  }وإلا{
    Pgno nOrig = btreePagecount(pBt);
    Pgno nFree = get4byte(&pBt->pPage1->a_انکڑے[36]);
    Pgno nFin = final_Db_ناپ(pBt, nOrig, nFree);

    إذا( nOrig<nFin || nFree>=nOrig ){
      rc = SQLITE_CORRUPT_BKPT;
    }وإلا إذا( nFree>0 ){
      rc = saveAllCursors(pBt, 0, 0);
      إذا( rc==SQLITE_OK ){
        invalidate_All_حد_توں_ودھ_Cache(pBt);
        rc = incr_Vacuum_قدم(pBt, nFin, nOrig, 0);
      }
      إذا( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ورقہ);
        put4byte(&pBt->pPage1->a_انکڑے[28], pBt->n_ورقہ);
      }
    }وإلا{
      rc = SQLITE_پورا;
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
  invalidate_All_حد_توں_ودھ_Cache(pBt);
  أكد(pBt->autoVacuum);
  إذا( !pBt->incrVacuum ){
    Pgno nFin;         /* Number of pages in database after autovacuuming */
    Pgno nFree;        /* Number of pages on the freelist initially */
    Pgno nVac;         /* Number of pages to vacuum */
    Pgno iFree;        /* The next page to be freed */
    Pgno nOrig;        /* Database size before freeing */

    nOrig = btreePagecount(pBt);
    إذا( PTRMAP_ISPAGE(pBt, nOrig) || nOrig==PENDING_BYTE_ورقہ(pBt) ){
      /* It is not possible to create a database for which the final page
      ** is either a pointer-map page or the pending-byte page. If one
      ** is encountered, this indicates corruption.
      */
      أرجع SQLITE_CORRUPT_BKPT;
    }

    nFree = get4byte(&pBt->pPage1->a_انکڑے[36]);
    db = p->db;
    إذا( db->x_Autovac_ورقے ){
      صحيح iDb;
      لكل(iDb=0; ALWAYS(iDb<db->nDb); iDb++){
        إذا( db->aDb[iDb].pBt==p ) اكسر;
      }
      nVac = db->x_Autovac_ورقے(
        db->p_Autovac_ورقے_Arg,
        db->aDb[iDb].zDbSName,
        nOrig,
        nFree,
        pBt->ورقہ_ناپ
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
    nFin = final_Db_ناپ(pBt, nOrig, nVac);
    إذا( nFin>nOrig ) أرجع SQLITE_CORRUPT_BKPT;
    إذا( nFin<nOrig ){
      rc = saveAllCursors(pBt, 0, 0);
    }
    لكل(iFree=nOrig; iFree>nFin && rc==SQLITE_OK; iFree--){
      rc = incr_Vacuum_قدم(pBt, nFin, iFree, nVac==nFree);
    }
    إذا( (rc==SQLITE_پورا || rc==SQLITE_OK) && nFree>0 ){
      rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ورقہ);
      إذا( nVac==nFree ){
        put4byte(&pBt->pPage1->a_انکڑے[32], 0);
        put4byte(&pBt->pPage1->a_انکڑے[36], 0);
      }
      put4byte(&pBt->pPage1->a_انکڑے[28], nFin);
      pBt->bDoTruncate = 1;
      pBt->n_ورقہ = nFin;
    }
    إذا( rc!=SQLITE_OK ){
      sqlite3PagerRollback(pPager);
    }
  }

  أكد( nRef>=sqlite3PagerRefcount(pPager) );
  أرجع rc;
}

#وإلا /* ifndef SQLITE_OMIT_AUTOVACUUM */
# define سیٹ_بچہ_Ptrmaps(x) SQLITE_OK
#جے_انت

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
  إذا( p->وچ_Trans==TRANS_WRITE ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
    إذا( pBt->autoVacuum ){
      rc = autoVacuumCommit(p);
      إذا( rc!=SQLITE_OK ){
        sqlite3BtreeLeave(p);
        أرجع rc;
      }
    }
    إذا( pBt->bDoTruncate ){
      sqlite3PagerTruncateImage(pBt->pPager, pBt->n_ورقہ);
    }
#جے_انت
    rc = sqlite3PagerCommitPhaseOne(pBt->pPager, zSuperJrnl, 0);
    sqlite3BtreeLeave(p);
  }
  أرجع rc;
}

/*
** This function is called from both BtreeCommitPhaseTwo() and BtreeRollback()
** at the conclusion of a transaction.
*/
ساكن فراغ btree_سرا_Transaction(Btree *p){
  BtShared *pBt = p->pBt;
  sqlite3 *db = p->db;
  أكد( sqlite3BtreeHoldsMutex(p) );

#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
  pBt->bDoTruncate = 0;
#جے_انت
  إذا( p->وچ_Trans>TRANS_NONE && db->nVdbeRead>1 ){
    /* If there are other active statements that belong to this database
    ** handle, downgrade to a read-only transaction. The other statements
    ** may still be reading from the database.  */
    downgrade_All_Shared_Cache_جدول_Locks(p);
    p->وچ_Trans = TRANS_READ;
  }وإلا{
    /* If the handle had any kind of transaction open, decrement the
    ** transaction count of the shared btree. If the transaction count
    ** reaches 0, set the shared state to TRANS_NONE. The unlockBtreeIfUnused()
    ** call below will unlock the pager.  */
    إذا( p->وچ_Trans!=TRANS_NONE ){
      clear_All_Shared_Cache_جدول_Locks(p);
      pBt->nTransaction--;
      إذا( 0==pBt->nTransaction ){
        pBt->وچ_Transaction = TRANS_NONE;
      }
    }

    /* Set the current transaction state to TRANS_NONE and unlock the
    ** pager if this call closed the only read or write transaction.  */
    p->وچ_Trans = TRANS_NONE;
    unlock_Btree_If_بن_ورتیا(pBt);
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

  إذا( p->وچ_Trans==TRANS_NONE ) أرجع SQLITE_OK;
  sqlite3BtreeEnter(p);
  btreeIntegrity(p);

  /* If the handle has a write-transaction open, commit the shared-btrees
  ** transaction and set the shared state to TRANS_READ.
  */
  إذا( p->وچ_Trans==TRANS_WRITE ){
    صحيح rc;
    BtShared *pBt = p->pBt;
    أكد( pBt->وچ_Transaction==TRANS_WRITE );
    أكد( pBt->nTransaction>0 );
    rc = sqlite3PagerCommitPhaseTwo(pBt->pPager);
    إذا( rc!=SQLITE_OK && bCleanup==0 ){
      sqlite3BtreeLeave(p);
      أرجع rc;
    }
    p->i_BData_نسخہ--;  /* Compensate for pPager->iDataVersion++; */
    pBt->وچ_Transaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btree_سرا_Transaction(p);
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
صحيح sqlite3BtreeTripAllCursors(Btree *pBtree, صحيح err_کوڈ, صحيح writeOnly){
  BtCursor *p;
  صحيح rc = SQLITE_OK;

  أكد( (writeOnly==0 || writeOnly==1) && BTCF_WriteFlag==1 );
  إذا( pBtree ){
    sqlite3BtreeEnter(pBtree);
    لكل(p=pBtree->pBt->pCursor; p; p=p->p_اگلا){
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
        p->skip_اگلا = err_کوڈ;
      }
      btree_Release_All_Cursor_ورقے(p);
    }
    sqlite3BtreeLeave(pBtree);
  }
  أرجع rc;
}

/*
** Set the pBt->nPage field correctly, according to the current
** state of the database.  Assume pBt->pPage1 is valid.
*/
ساكن فراغ btree_سیٹ_NPage(BtShared *pBt, Mem_ورقہ *pPage1){
  صحيح n_ورقہ = get4byte(&pPage1->a_انکڑے[28]);
  testcase( n_ورقہ==0 );
  إذا( n_ورقہ==0 ) sqlite3PagerPagecount(pBt->pPager, &n_ورقہ);
  testcase( pBt->n_ورقہ!=(u32)n_ورقہ );
  pBt->n_ورقہ = n_ورقہ;
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
صحيح sqlite3BtreeRollback(Btree *p, صحيح trip_کوڈ, صحيح writeOnly){
  صحيح rc;
  BtShared *pBt = p->pBt;
  Mem_ورقہ *pPage1;

  أكد( writeOnly==1 || writeOnly==0 );
  أكد( trip_کوڈ==SQLITE_ABORT_ROLLBACK || trip_کوڈ==SQLITE_OK );
  sqlite3BtreeEnter(p);
  إذا( trip_کوڈ==SQLITE_OK ){
    rc = trip_کوڈ = saveAllCursors(pBt, 0, 0);
    إذا( rc ) writeOnly = 0;
  }وإلا{
    rc = SQLITE_OK;
  }
  إذا( trip_کوڈ ){
    صحيح rc2 = sqlite3BtreeTripAllCursors(p, trip_کوڈ, writeOnly);
    أكد( rc==SQLITE_OK || (writeOnly==0 && rc2==SQLITE_OK) );
    إذا( rc2!=SQLITE_OK ) rc = rc2;
  }
  btreeIntegrity(p);

  إذا( p->وچ_Trans==TRANS_WRITE ){
    صحيح rc2;

    أكد( TRANS_WRITE==pBt->وچ_Transaction );
    rc2 = sqlite3PagerRollback(pBt->pPager);
    إذا( rc2!=SQLITE_OK ){
      rc = rc2;
    }

    /* The rollback may have destroyed the pPage1->aData value.  So
    ** call btreeGetPage() on page 1 again to make
    ** sure pPage1->aData is set correctly. */
    إذا( btree_Get_ورقہ(pBt, 1, &pPage1, 0)==SQLITE_OK ){
      btree_سیٹ_NPage(pBt, pPage1);
      release_ورقہ_One(pPage1);
    }
    أكد( گنتی_Valid_Cursors(pBt, 1)==0 );
    pBt->وچ_Transaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btree_سرا_Transaction(p);
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
  أكد( p->وچ_Trans==TRANS_WRITE );
  أكد( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  أكد( iStatement>0 );
  أكد( iStatement>p->db->nSavepoint );
  أكد( pBt->وچ_Transaction==TRANS_WRITE );
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
  إذا( p && p->وچ_Trans==TRANS_WRITE ){
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
      إذا( iSavepoint<0 && (pBt->btsFlags & BTS_INITIALLY_خالی_تھاواں)!=0 ){
        pBt->n_ورقہ = 0;
      }
      rc = newDatabase(pBt);
      btree_سیٹ_NPage(pBt, pBt->pPage1);

      /* pBt->nPage might be zero if the database was corrupt when
      ** the transaction was started. Otherwise, it must be at least 1.  */
      أكد( CORRUPT_DB || pBt->n_ورقہ>0 );
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
  Pgno i_جدول,                           /* Root page of table to open */
  صحيح wrFlag,                            /* 1 to write. 0 read-only */
  بنية کلید_Info *p_کلید_Info,              /* First arg to comparison function */
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
  أكد( has_Shared_Cache_جدول_جندرا(p, i_جدول, p_کلید_Info!=0, (wrFlag?2:1))
          || i_جدول<1 );
  أكد( wrFlag==0 || !hasReadConflicts(p, i_جدول) );

  /* Assert that the caller has opened the required transaction. */
  أكد( p->وچ_Trans>TRANS_NONE );
  أكد( wrFlag==0 || p->وچ_Trans==TRANS_WRITE );
  أكد( pBt->pPage1 && pBt->pPage1->a_انکڑے );
  أكد( wrFlag==0 || (pBt->btsFlags & BTS_READ_ONLY)==0 );

  إذا( i_جدول<=1 ){
    إذا( i_جدول<1 ){
      أرجع SQLITE_CORRUPT_BKPT;
    }وإلا إذا( btreePagecount(pBt)==0 ){
      أكد( wrFlag==0 );
      i_جدول = 0;
    }
  }

  /* Now that no other errors can occur, finish filling in the BtCursor
  ** variables and link the cursor into the BtShared list.  */
  pCur->pgno_جڑ = i_جدول;
  pCur->i_ورقہ = -1;
  pCur->p_کلید_Info = p_کلید_Info;
  pCur->pBtree = p;
  pCur->pBt = pBt;
  pCur->curFlags = 0;
  /* If there are two or more cursors on the same btree, then all such
  ** cursors *must* have the BTCF_Multiple flag set. */
  لكل(pX=pBt->pCursor; pX; pX=pX->p_اگلا){
    إذا( pX->pgno_جڑ==i_جدول ){
      pX->curFlags |= BTCF_مضرب;
      pCur->curFlags = BTCF_مضرب;
    }
  }
  pCur->eState = CURSOR_INVALID;
  pCur->p_اگلا = pBt->pCursor;
  pBt->pCursor = pCur;
  إذا( wrFlag ){
    pCur->curFlags |= BTCF_WriteFlag;
    pCur->curPagerFlags = 0;
    إذا( pBt->pTmpSpace==0 ) أرجع allocate_عارضی_Space(pBt);
  }وإلا{
    pCur->curPagerFlags = PAGER_GET_READONLY;
  }
  أرجع SQLITE_OK;
}
ساكن صحيح btree_Cursor_With_جندرا(
  Btree *p,                              /* The btree */
  Pgno i_جدول,                           /* Root page of table to open */
  صحيح wrFlag,                            /* 1 to write. 0 read-only */
  بنية کلید_Info *p_کلید_Info,              /* First arg to comparison function */
  BtCursor *pCur                         /* Space for new cursor */
){
  صحيح rc;
  sqlite3BtreeEnter(p);
  rc = btreeCursor(p, i_جدول, wrFlag, p_کلید_Info, pCur);
  sqlite3BtreeLeave(p);
  أرجع rc;
}
صحيح sqlite3BtreeCursor(
  Btree *p,                                   /* The btree */
  Pgno i_جدول,                                /* Root page of table to open */
  صحيح wrFlag,                                 /* 1 to write. 0 read-only */
  بنية کلید_Info *p_کلید_Info,                   /* First arg to xCompare() */
  BtCursor *pCur                              /* Write new cursor here */
){
  إذا( p->sharable ){
    أرجع btree_Cursor_With_جندرا(p, i_جدول, wrFlag, p_کلید_Info, pCur);
  }وإلا{
    أرجع btreeCursor(p, i_جدول, wrFlag, p_کلید_Info, pCur);
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
صحيح sqlite3_Btree_Cursor_ناپ(فراغ){
  أرجع ROUND8(حجم(BtCursor));
}

#جے_تعریف_شدہ SQLITE_DEBUG
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
  إذا( pCur->p_اگلا!=0 ) أرجع 0;
  إذا( pCur->pBtree!=pBtree ) أرجع 0;
  أرجع 1;
}
#جے_انت

/*
** Initialize memory that will be converted into a BtCursor object.
**
** The simple approach here would be to memset() the entire object
** to zero.  But it turns out that the apPage[] and aiIdx[] arrays
** do not need to be zeroed and they are large, so we can save a lot
** of run-time by skipping the initialization of those elements.
*/
فراغ sqlite3BtreeCursorZero(BtCursor *p){
  memset(p, 0, offsetof(BtCursor, BTCURSOR_پہلا_UNINIT));
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
      pBt->pCursor = pCur->p_اگلا;
    }وإلا{
      BtCursor *pPrev = pBt->pCursor;
      افعل{
        إذا( pPrev->p_اگلا==pCur ){
          pPrev->p_اگلا = pCur->p_اگلا;
          اكسر;
        }
        pPrev = pPrev->p_اگلا;
      }طالما( ALWAYS(pPrev) );
    }
    btree_Release_All_Cursor_ورقے(pCur);
    unlock_Btree_If_بن_ورتیا(pBt);
    sqlite3_free(pCur->a_حد_توں_ودھ);
    sqlite3_free(pCur->p_کلید);
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
#جے_بنا_تعریف NDEBUG
  ساكن صحيح cellInfoEqual(CellInfo *a, CellInfo *b){
    إذا( a->n_کلید!=b->n_کلید ) أرجع 0;
    إذا( a->pPayload!=b->pPayload ) أرجع 0;
    إذا( a->nPayload!=b->nPayload ) أرجع 0;
    إذا( a->nLocal!=b->nLocal ) أرجع 0;
    إذا( a->n_ناپ!=b->n_ناپ ) أرجع 0;
    أرجع 1;
  }
  ساكن فراغ assertCellInfo(BtCursor *pCur){
    CellInfo info;
    memset(&info, 0, حجم(info));
    btreeParseCell(pCur->p_ورقہ, pCur->ix, &info);
    أكد( CORRUPT_DB || cellInfoEqual(&info, &pCur->info) );
  }
#وإلا
  #تعریف_کرو assertCellInfo(x)
#جے_انت
ساكن SQLITE_NOINLINE فراغ getCellInfo(BtCursor *pCur){
  إذا( pCur->info.n_ناپ==0 ){
    pCur->curFlags |= BTCF_ValidNKey;
    btreeParseCell(pCur->p_ورقہ,pCur->ix,&pCur->info);
  }وإلا{
    assertCellInfo(pCur);
  }
}

#جے_بنا_تعریف NDEBUG  /* The next routine used only within assert() statements */
/*
** Return true if the given BtCursor is valid.  A valid cursor is one
** that is currently pointing to a row in a (non-empty) table.
** This is a verification routine is used only within assert() statements.
*/
صحيح sqlite3BtreeCursorIsValid(BtCursor *pCur){
  أرجع pCur && pCur->eState==CURSOR_VALID;
}
#جے_انت /* NDEBUG */
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
i64 sqlite3_Btree_Integer_کلید(BtCursor *pCur){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->cur_Int_کلید );
  getCellInfo(pCur);
  أرجع pCur->info.n_کلید;
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
i64 sqlite3_Btree_وتھ(BtCursor *pCur){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  getCellInfo(pCur);
  أرجع (i64)pCur->pBt->ورقہ_ناپ*((i64)pCur->p_ورقہ->pgno - 1) +
         (i64)(pCur->info.pPayload - pCur->p_ورقہ->a_انکڑے);
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
u32 sqlite3_Btree_Payload_ناپ(BtCursor *pCur){
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
sqlite3_int64 sqlite3_Btree_Max_ریکارڈ_ناپ(BtCursor *pCur){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أرجع pCur->pBt->ورقہ_ناپ * (sqlite3_int64)pCur->pBt->n_ورقہ;
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
ساكن صحيح get_حد_توں_ودھ_ورقہ(
  BtShared *pBt,               /* The database file */
  Pgno ovfl,                   /* Current overflow page number */
  Mem_ورقہ **pp_ورقہ,            /* OUT: MemPage handle (may be NULL) */
  Pgno *p_Pgno_اگلا              /* OUT: Next overflow page number */
){
  Pgno اگلا = 0;
  Mem_ورقہ *p_ورقہ = 0;
  صحيح rc = SQLITE_OK;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد(p_Pgno_اگلا);

#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
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

    طالما( PTRMAP_ISPAGE(pBt, iGuess) || iGuess==PENDING_BYTE_ورقہ(pBt) ){
      iGuess++;
    }

    إذا( iGuess<=btreePagecount(pBt) ){
      rc = ptrmapGet(pBt, iGuess, &eType, &pgno);
      إذا( rc==SQLITE_OK && eType==PTRMAP_OVERFLOW2 && pgno==ovfl ){
        اگلا = iGuess;
        rc = SQLITE_پورا;
      }
    }
  }
#جے_انت

  أكد( اگلا==0 || rc==SQLITE_پورا );
  إذا( rc==SQLITE_OK ){
    rc = btree_Get_ورقہ(pBt, ovfl, &p_ورقہ, (pp_ورقہ==0) ? PAGER_GET_READONLY : 0);
    أكد( rc==SQLITE_OK || p_ورقہ==0 );
    إذا( rc==SQLITE_OK ){
      اگلا = get4byte(p_ورقہ->a_انکڑے);
    }
  }

  *p_Pgno_اگلا = اگلا;
  إذا( pp_ورقہ ){
    *pp_ورقہ = p_ورقہ;
  }وإلا{
    release_ورقہ(p_ورقہ);
  }
  أرجع (rc==SQLITE_پورا ? SQLITE_OK : rc);
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
  Db_ورقہ *p_Db_ورقہ           /* Page containing pPayload */
){
  إذا( eOp ){
    /* Copy data from buffer to page (a write operation) */
    صحيح rc = sqlite3PagerWrite(p_Db_ورقہ);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
    حافظہ_نقل(pPayload, pBuf, nByte);
  }وإلا{
    /* Copy data from page to buffer (a read operation) */
    حافظہ_نقل(pBuf, pPayload, nByte);
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
  u32 وتھ,          /* Begin reading this far into payload */
  u32 amt,             /* Read this many bytes */
  بنانشانی حرف *pBuf, /* Write the bytes into this buffer */
  صحيح eOp              /* zero to read. non-zero to write. */
){
  بنانشانی حرف *aPayload;
  صحيح rc = SQLITE_OK;
  صحيح iIdx = 0;
  Mem_ورقہ *p_ورقہ = pCur->p_ورقہ;               /* Btree page of current entry */
  BtShared *pBt = pCur->pBt;                  /* Btree this cursor belongs to */
#جے_تعریف_شدہ SQLITE_سدھا_حد_توں_ودھ_READ
  بنانشانی حرف * ثابت p_Buf_شروع = pBuf;     /* Start of original out buffer */
#جے_انت

  أكد( p_ورقہ );
  أكد( eOp==0 || eOp==1 );
  أكد( pCur->eState==CURSOR_VALID );
  إذا( pCur->ix>=p_ورقہ->nCell ){
    أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
  }
  أكد( cursorHoldsMutex(pCur) );

  getCellInfo(pCur);
  aPayload = pCur->info.pPayload;
  أكد( (u64)وتھ+(u64)amt <= (u64)pCur->info.nPayload );

  أكد( aPayload > p_ورقہ->a_انکڑے );
  إذا( (uptr)(aPayload - p_ورقہ->a_انکڑے) > (pBt->usable_ناپ - pCur->info.nLocal) ){
    /* Trying to read or write past the end of the data is an error.  The
    ** conditional above is really:
    **    &aPayload[pCur->info.nLocal] > &pPage->aData[pBt->usableSize]
    ** but is recast into its current form to avoid integer overflow problems
    */
    أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
  }

  /* Check if data must be read/written to/from the btree page itself. */
  إذا( وتھ<pCur->info.nLocal ){
    صحيح a = amt;
    إذا( a+وتھ>pCur->info.nLocal ){
      a = pCur->info.nLocal - وتھ;
    }
    rc = copyPayload(&aPayload[وتھ], pBuf, a, eOp, p_ورقہ->p_Db_ورقہ);
    وتھ = 0;
    pBuf += a;
    amt -= a;
  }وإلا{
    وتھ -= pCur->info.nLocal;
  }


  إذا( rc==SQLITE_OK && amt>0 ){
    ثابت u32 ovfl_ناپ = pBt->usable_ناپ - 4;  /* Bytes content per ovfl page */
    Pgno اگلا_ورقہ;

    اگلا_ورقہ = get4byte(&aPayload[pCur->info.nLocal]);

    /* If the BtCursor.aOverflow[] has not been allocated, allocate it now.
    **
    ** The aOverflow[] array is sized at one entry for each overflow page
    ** in the overflow chain. The page number of the first overflow page is
    ** stored in aOverflow[0], etc. A value of 0 in the aOverflow[] array
    ** means "not yet known" (the cache is lazily populated).
    */
    إذا( (pCur->curFlags & BTCF_ValidOvfl)==0 ){
      i64 nOvfl = pCur->info.nPayload;
      testcase( nOvfl - pCur->info.nLocal + ovfl_ناپ - 1 > 0xffffffffU );
      nOvfl = (nOvfl - pCur->info.nLocal + ovfl_ناپ-1)/ovfl_ناپ;
      إذا( pCur->a_حد_توں_ودھ==0
       || nOvfl*(صحيح)حجم(Pgno) > sqlite3_Malloc_ناپ(pCur->a_حد_توں_ودھ)
      ){
        Pgno *aNew;
        إذا( sqlite3FaultSim(413) ){
          aNew = 0;
        }وإلا{
          aNew = (Pgno*)sqlite3Realloc(pCur->a_حد_توں_ودھ, nOvfl*2*حجم(Pgno));
        }
        إذا( aNew==0 ){
          أرجع SQLITE_NOMEM_BKPT;
        }وإلا{
          pCur->a_حد_توں_ودھ = aNew;
        }
      }
      memset(pCur->a_حد_توں_ودھ, 0, nOvfl*حجم(Pgno));
      pCur->curFlags |= BTCF_ValidOvfl;
    }وإلا{
      /* Sanity check the validity of the overflow page cache */
      أكد( pCur->a_حد_توں_ودھ[0]==اگلا_ورقہ
           || pCur->a_حد_توں_ودھ[0]==0
           || CORRUPT_DB );
      أكد( pCur->a_حد_توں_ودھ[0]!=0 || pCur->a_حد_توں_ودھ[وتھ/ovfl_ناپ]==0 );

      /* If the overflow page-list cache has been allocated and the
      ** entry for the first required overflow page is valid, skip
      ** directly to it.
      */
      إذا( pCur->a_حد_توں_ودھ[وتھ/ovfl_ناپ] ){
        iIdx = (وتھ/ovfl_ناپ);
        اگلا_ورقہ = pCur->a_حد_توں_ودھ[iIdx];
        وتھ = (وتھ%ovfl_ناپ);
      }
    }

    أكد( rc==SQLITE_OK && amt>0 );
    طالما( اگلا_ورقہ ){
      /* If required, populate the overflow page-list cache. */
      إذا( اگلا_ورقہ > pBt->n_ورقہ ) أرجع SQLITE_CORRUPT_BKPT;
      أكد( pCur->a_حد_توں_ودھ[iIdx]==0
              || pCur->a_حد_توں_ودھ[iIdx]==اگلا_ورقہ
              || CORRUPT_DB );
      pCur->a_حد_توں_ودھ[iIdx] = اگلا_ورقہ;

      إذا( وتھ>=ovfl_ناپ ){
        /* The only reason to read this page is to obtain the page
        ** number for the next page in the overflow chain. The page
        ** data is not required. So first try to lookup the overflow
        ** page-list cache, if any, then fall back to the getOverflowPage()
        ** function.
        */
        أكد( pCur->curFlags & BTCF_ValidOvfl );
        أكد( pCur->pBtree->db==pBt->db );
        إذا( pCur->a_حد_توں_ودھ[iIdx+1] ){
          اگلا_ورقہ = pCur->a_حد_توں_ودھ[iIdx+1];
        }وإلا{
          rc = get_حد_توں_ودھ_ورقہ(pBt, اگلا_ورقہ, 0, &اگلا_ورقہ);
        }
        وتھ -= ovfl_ناپ;
      }وإلا{
        /* Need to read this page properly. It contains some of the
        ** range of data that is being read (eOp==0) or written (eOp!=0).
        */
        صحيح a = amt;
        إذا( a + وتھ > ovfl_ناپ ){
          a = ovfl_ناپ - وتھ;
        }

#جے_تعریف_شدہ SQLITE_سدھا_حد_توں_ودھ_READ
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
         && وتھ==0                                          /* (2) */
         && sqlite3_Pager_سدھا_Read_Ok(pBt->pPager, اگلا_ورقہ)    /* (3,4,5) */
         && &pBuf[-4]>=p_Buf_شروع                               /* (6) */
        ){
          sqlite3_file *fd = sqlite3PagerFile(pBt->pPager);
          u8 aSave[4];
          u8 *aWrite = &pBuf[-4];
          أكد( aWrite>=p_Buf_شروع );                         /* due to (6) */
          حافظہ_نقل(aSave, aWrite, 4);
          rc = sqlite3OsRead(fd, aWrite, a+4, (i64)pBt->ورقہ_ناپ*(اگلا_ورقہ-1));
          اگلا_ورقہ = get4byte(aWrite);
          حافظہ_نقل(aWrite, aSave, 4);
        }وإلا
#جے_انت

        {
          Db_ورقہ *p_Db_ورقہ;
          rc = sqlite3PagerGet(pBt->pPager, اگلا_ورقہ, &p_Db_ورقہ,
              (eOp==0 ? PAGER_GET_READONLY : 0)
          );
          إذا( rc==SQLITE_OK ){
            إذا( eOp!=0
             && (sqlite3_Pager_ورقہ_Refcount(p_Db_ورقہ)!=1
                 || NEVER(((Mem_ورقہ*)sqlite3PagerGetExtra(p_Db_ورقہ))->isInit))
             && sqlite3FaultSim(411)==SQLITE_OK
            ){
              sqlite3PagerUnref(p_Db_ورقہ);
              أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
            }
            aPayload = sqlite3_Pager_Get_انکڑے(p_Db_ورقہ);
            اگلا_ورقہ = get4byte(aPayload);
            rc = copyPayload(&aPayload[وتھ+4], pBuf, a, eOp, p_Db_ورقہ);
            sqlite3PagerUnref(p_Db_ورقہ);
            وتھ = 0;
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
    أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
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
صحيح sqlite3BtreePayload(BtCursor *pCur, u32 وتھ, u32 amt, فراغ *pBuf){
  أكد( cursorHoldsMutex(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->i_ورقہ>=0 && pCur->p_ورقہ );
  أرجع accessPayload(pCur, وتھ, amt, (بنانشانی حرف*)pBuf, 0);
}

/*
** This variant of sqlite3BtreePayload() works even if the cursor has not
** in the CURSOR_VALID state.  It is only used by the sqlite3_blob_read()
** interface.
*/
#جے_بنا_تعریف SQLITE_OMIT_INCRBLOB
ساكن SQLITE_NOINLINE صحيح accessPayloadChecked(
  BtCursor *pCur,
  u32 وتھ,
  u32 amt,
  فراغ *pBuf
){
  صحيح rc;
  إذا ( pCur->eState==CURSOR_INVALID ){
    أرجع SQLITE_ABORT;
  }
  أكد( cursorOwnsBtShared(pCur) );
  rc = btreeRestoreCursorPosition(pCur);
  أرجع rc ? rc : accessPayload(pCur, وتھ, amt, pBuf, 0);
}
صحيح sqlite3BtreePayloadChecked(BtCursor *pCur, u32 وتھ, u32 amt, فراغ *pBuf){
  إذا( pCur->eState==CURSOR_VALID ){
    أكد( cursorOwnsBtShared(pCur) );
    أرجع accessPayload(pCur, وتھ, amt, pBuf, 0);
  }وإلا{
    أرجع accessPayloadChecked(pCur, وتھ, amt, pBuf);
  }
}
#جے_انت /* SQLITE_OMIT_INCRBLOB */

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
  أكد( pCur!=0 && pCur->i_ورقہ>=0 && pCur->p_ورقہ);
  أكد( pCur->eState==CURSOR_VALID );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->ix<pCur->p_ورقہ->nCell || CORRUPT_DB );
  أكد( pCur->info.n_ناپ>0 );
  أكد( pCur->info.pPayload>pCur->p_ورقہ->a_انکڑے || CORRUPT_DB );
  أكد( pCur->info.pPayload<pCur->p_ورقہ->a_انکڑے_سرا ||CORRUPT_DB);
  amt = pCur->info.nLocal;
  إذا( amt>(صحيح)(pCur->p_ورقہ->a_انکڑے_سرا - pCur->info.pPayload) ){
    /* There is too little space on the page for the expected amount
    ** of local content. Database must be corrupt. */
    أكد( CORRUPT_DB );
    amt = MAX(0, (صحيح)(pCur->p_ورقہ->a_انکڑے_سرا - pCur->info.pPayload));
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
ساكن صحيح move_منزل_بچہ(BtCursor *pCur, u32 newPgno){
  صحيح rc;
  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->i_ورقہ<BTCURSOR_MAX_ڈونگھائی );
  أكد( pCur->i_ورقہ>=0 );
  إذا( pCur->i_ورقہ>=(BTCURSOR_MAX_ڈونگھائی-1) ){
    أرجع SQLITE_CORRUPT_BKPT;
  }
  pCur->info.n_ناپ = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  pCur->aiIdx[pCur->i_ورقہ] = pCur->ix;
  pCur->ap_ورقہ[pCur->i_ورقہ] = pCur->p_ورقہ;
  pCur->ix = 0;
  pCur->i_ورقہ++;
  rc = get_And_Init_ورقہ(pCur->pBt, newPgno, &pCur->p_ورقہ, pCur->curPagerFlags);
  أكد( pCur->p_ورقہ!=0 || rc!=SQLITE_OK );
  إذا( rc==SQLITE_OK
   && (pCur->p_ورقہ->nCell<1 || pCur->p_ورقہ->int_کلید!=pCur->cur_Int_کلید)
  ){
    release_ورقہ(pCur->p_ورقہ);
    rc = SQLITE_CORRUPT_PGNO(newPgno);
  }
  إذا( rc ){
    pCur->p_ورقہ = pCur->ap_ورقہ[--pCur->i_ورقہ];
  }
  أرجع rc;
}

#جے_تعریف_شدہ SQLITE_DEBUG
/*
** Page pParent is an internal (non-leaf) tree page. This function
** asserts that page number iChild is the left-child if the iIdx'th
** cell in page pParent. Or, if iIdx is equal to the total number of
** cells in pParent, that page number iChild is the right-child of
** the page.
*/
ساكن فراغ assert_ماپا_اشاریہ(Mem_ورقہ *p_ماپا, صحيح iIdx, Pgno i_بچہ){
  إذا( CORRUPT_DB ) أرجع;  /* The conditions tested below might not be true
                            ** in a corrupt database */
  أكد( iIdx<=p_ماپا->nCell );
  إذا( iIdx==p_ماپا->nCell ){
    أكد( get4byte(&p_ماپا->a_انکڑے[p_ماپا->hdr_وتھ+8])==i_بچہ );
  }وإلا{
    أكد( get4byte(لبھو_Cell(p_ماپا, iIdx))==i_بچہ );
  }
}
#وإلا
#  define assert_ماپا_اشاریہ(x,y,z)
#جے_انت

/*
** Move the cursor up to the parent page.
**
** pCur->idx is set to the cell index that contains the pointer
** to the page we are coming from.  If we are coming from the
** right-most child page then pCur->idx is set to one more than
** the largest cell index.
*/
ساكن فراغ move_منزل_ماپا(BtCursor *pCur){
  Mem_ورقہ *p_پتی;
  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->i_ورقہ>0 );
  أكد( pCur->p_ورقہ );
  assert_ماپا_اشاریہ(
    pCur->ap_ورقہ[pCur->i_ورقہ-1],
    pCur->aiIdx[pCur->i_ورقہ-1],
    pCur->p_ورقہ->pgno
  );
  testcase( pCur->aiIdx[pCur->i_ورقہ-1] > pCur->ap_ورقہ[pCur->i_ورقہ-1]->nCell );
  pCur->info.n_ناپ = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  pCur->ix = pCur->aiIdx[pCur->i_ورقہ-1];
  p_پتی = pCur->p_ورقہ;
  pCur->p_ورقہ = pCur->ap_ورقہ[--pCur->i_ورقہ];
  release_ورقہ_Not_Null(p_پتی);
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
ساكن صحيح move_منزل_جڑ(BtCursor *pCur){
  Mem_ورقہ *p_جڑ;
  صحيح rc = SQLITE_OK;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( CURSOR_INVALID < CURSOR_REQUIRESEEK );
  أكد( CURSOR_VALID   < CURSOR_REQUIRESEEK );
  أكد( CURSOR_FAULT   > CURSOR_REQUIRESEEK );
  أكد( pCur->eState < CURSOR_REQUIRESEEK || pCur->i_ورقہ<0 );
  أكد( pCur->pgno_جڑ>0 || pCur->i_ورقہ<0 );

  إذا( pCur->i_ورقہ>=0 ){
    إذا( pCur->i_ورقہ ){
      release_ورقہ_Not_Null(pCur->p_ورقہ);
      طالما( --pCur->i_ورقہ ){
        release_ورقہ_Not_Null(pCur->ap_ورقہ[pCur->i_ورقہ]);
      }
      p_جڑ = pCur->p_ورقہ = pCur->ap_ورقہ[0];
      اذهب skip_init;
    }
  }وإلا إذا( pCur->pgno_جڑ==0 ){
    pCur->eState = CURSOR_INVALID;
    أرجع SQLITE_خالی_تھاواں;
  }وإلا{
    أكد( pCur->i_ورقہ==(-1) );
    إذا( pCur->eState>=CURSOR_REQUIRESEEK ){
      إذا( pCur->eState==CURSOR_FAULT ){
        أكد( pCur->skip_اگلا!=SQLITE_OK );
        أرجع pCur->skip_اگلا;
      }
      sqlite3BtreeClearCursor(pCur);
    }
    rc = get_And_Init_ورقہ(pCur->pBt, pCur->pgno_جڑ, &pCur->p_ورقہ,
                        pCur->curPagerFlags);
    إذا( rc!=SQLITE_OK ){
      pCur->eState = CURSOR_INVALID;
      أرجع rc;
    }
    pCur->i_ورقہ = 0;
    pCur->cur_Int_کلید = pCur->p_ورقہ->int_کلید;
  }
  p_جڑ = pCur->p_ورقہ;
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
  إذا( p_جڑ->isInit==0 || (pCur->p_کلید_Info==0)!=p_جڑ->int_کلید ){
    أرجع SQLITE_CORRUPT_ورقہ(pCur->p_ورقہ);
  }

skip_init: 
  pCur->ix = 0;
  pCur->info.n_ناپ = 0;
  pCur->curFlags &= ~(BTCF_At_آخری|BTCF_ValidNKey|BTCF_ValidOvfl);

  إذا( p_جڑ->nCell>0 ){
    pCur->eState = CURSOR_VALID;
  }وإلا إذا( !p_جڑ->پتی ){
    Pgno subpage;
    إذا( p_جڑ->pgno!=1 ) أرجع SQLITE_CORRUPT_BKPT;
    subpage = get4byte(&p_جڑ->a_انکڑے[p_جڑ->hdr_وتھ+8]);
    pCur->eState = CURSOR_VALID;
    rc = move_منزل_بچہ(pCur, subpage);
  }وإلا{
    pCur->eState = CURSOR_INVALID;
    rc = SQLITE_خالی_تھاواں;
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
ساكن صحيح move_منزل_Leftmost(BtCursor *pCur){
  Pgno pgno;
  صحيح rc = SQLITE_OK;
  Mem_ورقہ *p_ورقہ;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  طالما( rc==SQLITE_OK && !(p_ورقہ = pCur->p_ورقہ)->پتی ){
    أكد( pCur->ix<p_ورقہ->nCell );
    pgno = get4byte(لبھو_Cell(p_ورقہ, pCur->ix));
    rc = move_منزل_بچہ(pCur, pgno);
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
ساكن صحيح move_منزل_Rightmost(BtCursor *pCur){
  Pgno pgno;
  صحيح rc = SQLITE_OK;
  Mem_ورقہ *p_ورقہ = 0;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( pCur->eState==CURSOR_VALID );
  طالما( !(p_ورقہ = pCur->p_ورقہ)->پتی ){
    pgno = get4byte(&p_ورقہ->a_انکڑے[p_ورقہ->hdr_وتھ+8]);
    pCur->ix = p_ورقہ->nCell;
    rc = move_منزل_بچہ(pCur, pgno);
    إذا( rc ) أرجع rc;
  }
  pCur->ix = p_ورقہ->nCell-1;
  أكد( pCur->info.n_ناپ==0 );
  أكد( (pCur->curFlags & BTCF_ValidNKey)==0 );
  أرجع SQLITE_OK;
}

/* Move the cursor to the first entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
صحيح sqlite3_Btree_پہلا(BtCursor *pCur, صحيح *pRes){
  صحيح rc;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  rc = move_منزل_جڑ(pCur);
  إذا( rc==SQLITE_OK ){
    أكد( pCur->p_ورقہ->nCell>0 );
    *pRes = 0;
    rc = move_منزل_Leftmost(pCur);
  }وإلا إذا( rc==SQLITE_خالی_تھاواں ){
    أكد( pCur->pgno_جڑ==0 || (pCur->p_ورقہ!=0 && pCur->p_ورقہ->nCell==0) );
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
صحيح sqlite3_Btree_Is_خالی_تھاواں(BtCursor *pCur, صحيح *pRes){
  صحيح rc;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  إذا( NEVER(pCur->eState==CURSOR_VALID) ){
    *pRes = 0;
    أرجع SQLITE_OK;
  }
  rc = move_منزل_جڑ(pCur);
  إذا( rc==SQLITE_خالی_تھاواں ){
    *pRes = 1;
    rc = SQLITE_OK;
  }وإلا{
    *pRes = 0;
  }
  أرجع rc;
}

#جے_تعریف_شدہ SQLITE_DEBUG
/* The cursors is CURSOR_VALID and has BTCF_AtLast set.  Verify that
** this flags are true for a consistent database.
**
** This routine is is called from within assert() statements only.
** It is an internal verification routine and does not appear in production
** builds.
*/
ساكن صحيح cursor_Is_At_آخری_اندراج(BtCursor *pCur){
  صحيح ii;
  لكل(ii=0; ii<pCur->i_ورقہ; ii++){
    إذا( pCur->aiIdx[ii]!=pCur->ap_ورقہ[ii]->nCell ) أرجع 0;
  }
  أرجع pCur->ix==pCur->p_ورقہ->nCell-1 && pCur->p_ورقہ->پتی!=0;
}
#جے_انت

/* Move the cursor to the last entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
ساكن SQLITE_NOINLINE صحيح btree_آخری(BtCursor *pCur, صحيح *pRes){
  صحيح rc = move_منزل_جڑ(pCur);
  إذا( rc==SQLITE_OK ){
    أكد( pCur->eState==CURSOR_VALID );
    *pRes = 0;
    rc = move_منزل_Rightmost(pCur);
    إذا( rc==SQLITE_OK ){
      pCur->curFlags |= BTCF_At_آخری;
    }وإلا{
      pCur->curFlags &= ~BTCF_At_آخری;
    }
  }وإلا إذا( rc==SQLITE_خالی_تھاواں ){
    أكد( pCur->pgno_جڑ==0 || pCur->p_ورقہ->nCell==0 );
    *pRes = 1;
    rc = SQLITE_OK;
  }
  أرجع rc;
}
صحيح sqlite3_Btree_آخری(BtCursor *pCur, صحيح *pRes){
  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* If the cursor already points to the last entry, this is a no-op. */
  إذا( CURSOR_VALID==pCur->eState && (pCur->curFlags & BTCF_At_آخری)!=0 ){
    أكد( cursor_Is_At_آخری_اندراج(pCur) || CORRUPT_DB );
    *pRes = 0;
    أرجع SQLITE_OK;
  }
  أرجع btree_آخری(pCur, pRes);
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
صحيح sqlite3_Btree_جدول_Moveto(
  BtCursor *pCur,          /* The cursor to be moved */
  i64 int_کلید,              /* The table key */
  صحيح bias_دایاں,           /* If true, bias the search to the high end */
  صحيح *pRes                /* Write search results here */
){
  صحيح rc;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  أكد( pRes );
  أكد( pCur->p_کلید_Info==0 );
  أكد( pCur->eState!=CURSOR_VALID || pCur->cur_Int_کلید!=0 );

  /* If the cursor is already positioned at the point we are trying
  ** to move to, then just return without doing any work */
  إذا( pCur->eState==CURSOR_VALID && (pCur->curFlags & BTCF_ValidNKey)!=0 ){
    إذا( pCur->info.n_کلید==int_کلید ){
      *pRes = 0;
      أرجع SQLITE_OK;
    }
    إذا( pCur->info.n_کلید<int_کلید ){
      إذا( (pCur->curFlags & BTCF_At_آخری)!=0 ){
        أكد( cursor_Is_At_آخری_اندراج(pCur) || CORRUPT_DB );
        *pRes = -1;
        أرجع SQLITE_OK;
      }
      /* If the requested key is one more than the previous key, then
      ** try to get there using sqlite3BtreeNext() rather than a full
      ** binary search.  This is an optimization only.  The correct answer
      ** is still obtained without this case, only a little more slowly. */
      إذا( pCur->info.n_کلید+1==int_کلید ){
        *pRes = 0;
        rc = sqlite3_Btree_اگلا(pCur, 0);
        إذا( rc==SQLITE_OK ){
          getCellInfo(pCur);
          إذا( pCur->info.n_کلید==int_کلید ){
            أرجع SQLITE_OK;
          }
        }وإلا إذا( rc!=SQLITE_پورا ){
          أرجع rc;
        }
      }
    }
  }

#جے_تعریف_شدہ SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#جے_انت

  rc = move_منزل_جڑ(pCur);
  إذا( rc ){
    إذا( rc==SQLITE_خالی_تھاواں ){
      أكد( pCur->pgno_جڑ==0 || pCur->p_ورقہ->nCell==0 );
      *pRes = -1;
      أرجع SQLITE_OK;
    }
    أرجع rc;
  }
  أكد( pCur->p_ورقہ );
  أكد( pCur->p_ورقہ->isInit );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->p_ورقہ->nCell > 0 );
  أكد( pCur->i_ورقہ==0 || pCur->ap_ورقہ[0]->int_کلید==pCur->cur_Int_کلید );
  أكد( pCur->cur_Int_کلید );

  لكل(;;){
    صحيح lwr, upr, idx, c;
    Pgno chldPg;
    Mem_ورقہ *p_ورقہ = pCur->p_ورقہ;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    أكد( p_ورقہ->nCell>0 );
    أكد( p_ورقہ->int_کلید );
    lwr = 0;
    upr = p_ورقہ->nCell-1;
    أكد( bias_دایاں==0 || bias_دایاں==1 );
    idx = upr>>(1-bias_دایاں); /* idx = biasRight ? upr : (lwr+upr)/2; */
    لكل(;;){
      i64 n_Cell_کلید;
      pCell = لبھو_Cell_Past_Ptr(p_ورقہ, idx);
      إذا( p_ورقہ->int_کلید_پتی ){
        طالما( 0x80 <= *(pCell++) ){
          إذا( pCell>=p_ورقہ->a_انکڑے_سرا ){
            أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
          }
        }
      }
      n_Cell_کلید = sqlite3_Varint_قدر(pCell);
      إذا( n_Cell_کلید<int_کلید ){
        lwr = idx+1;
        إذا( lwr>upr ){ c = -1; اكسر; }
      }وإلا إذا( n_Cell_کلید>int_کلید ){
        upr = idx-1;
        إذا( lwr>upr ){ c = +1; اكسر; }
      }وإلا{
        أكد( n_Cell_کلید==int_کلید );
        pCur->ix = (u16)idx;
        إذا( !p_ورقہ->پتی ){
          lwr = idx;
          اذهب moveto_جدول_اگلا_layer;
        }وإلا{
          pCur->curFlags |= BTCF_ValidNKey;
          pCur->info.n_کلید = n_Cell_کلید;
          pCur->info.n_ناپ = 0;
          *pRes = 0;
          أرجع SQLITE_OK;
        }
      }
      أكد( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2; */
    }
    أكد( lwr==upr+1 || !p_ورقہ->پتی );
    أكد( p_ورقہ->isInit );
    إذا( p_ورقہ->پتی ){
      أكد( pCur->ix<pCur->p_ورقہ->nCell );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      اذهب moveto_جدول_انت;
    }
moveto_جدول_اگلا_layer:
    إذا( lwr>=p_ورقہ->nCell ){
      chldPg = get4byte(&p_ورقہ->a_انکڑے[p_ورقہ->hdr_وتھ+8]);
    }وإلا{
      chldPg = get4byte(لبھو_Cell(p_ورقہ, lwr));
    }
    pCur->ix = (u16)lwr;
    rc = move_منزل_بچہ(pCur, chldPg);
    إذا( rc ) اكسر;
  }
moveto_جدول_انت:
  pCur->info.n_ناپ = 0;
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
ساكن صحيح اشاریہ_Cell_Compare(
  Mem_ورقہ *p_ورقہ,
  صحيح idx,
  Unpacked_ریکارڈ *p_Idx_کلید,
  ریکارڈ_Compare x_ریکارڈ_Compare
){
  صحيح c;
  صحيح nCell;  /* Size of the pCell cell in bytes */
  u8 *pCell = لبھو_Cell_Past_Ptr(p_ورقہ, idx);

  nCell = pCell[0];
  إذا( nCell<=p_ورقہ->max1bytePayload ){
    /* This branch runs if the record-size field of the cell is a
    ** single byte varint and the record fits entirely on the main
    ** b-tree page.  */
    إذا( pCell + nCell >= p_ورقہ->a_انکڑے_سرا ) أرجع 99;
    c = x_ریکارڈ_Compare(nCell, (فراغ*)&pCell[1], p_Idx_کلید);
  }وإلا إذا( !(pCell[1] & 0x80)
    && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=p_ورقہ->maxLocal
  ){
    /* The record-size field is a 2 byte varint and the record
    ** fits entirely on the main b-tree page.  */
    إذا( pCell + nCell >= p_ورقہ->a_انکڑے_سرا ) أرجع 99;
    c = x_ریکارڈ_Compare(nCell, (فراغ*)&pCell[2], p_Idx_کلید);
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
ساكن صحيح cursor_On_آخری_ورقہ(BtCursor *pCur){
  صحيح i;
  أكد( pCur->eState==CURSOR_VALID );
  لكل(i=0; i<pCur->i_ورقہ; i++){
    Mem_ورقہ *p_ورقہ = pCur->ap_ورقہ[i];
    إذا( pCur->aiIdx[i]<p_ورقہ->nCell ) أرجع 0;
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
صحيح sqlite3_Btree_اشاریہ_Moveto(
  BtCursor *pCur,          /* The cursor to be moved */
  Unpacked_ریکارڈ *p_Idx_کلید, /* Unpacked index key */
  صحيح *pRes                /* Write search results here */
){
  صحيح rc;
  ریکارڈ_Compare x_ریکارڈ_Compare;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  أكد( pRes );
  أكد( pCur->p_کلید_Info!=0 );

#جے_تعریف_شدہ SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#جے_انت

  x_ریکارڈ_Compare = sqlite3_Vdbe_لبھو_Compare(p_Idx_کلید);
  p_Idx_کلید->err_کوڈ = 0;
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
  إذا( pCur->eState==CURSOR_VALID
   && pCur->p_ورقہ->پتی
   && cursor_On_آخری_ورقہ(pCur)
  ){
    صحيح c;
    إذا( pCur->ix==pCur->p_ورقہ->nCell-1
     && (c = اشاریہ_Cell_Compare(pCur->p_ورقہ,pCur->ix,p_Idx_کلید,x_ریکارڈ_Compare))<=0
     && p_Idx_کلید->err_کوڈ==SQLITE_OK
    ){
      *pRes = c;
      أرجع SQLITE_OK;  /* Cursor already pointing at the correct spot */
    }
    إذا( pCur->i_ورقہ>0
     && اشاریہ_Cell_Compare(pCur->p_ورقہ, 0, p_Idx_کلید, x_ریکارڈ_Compare)<=0
     && p_Idx_کلید->err_کوڈ==SQLITE_OK
    ){
      pCur->curFlags &= ~(BTCF_ValidOvfl|BTCF_At_آخری);
      إذا( !pCur->p_ورقہ->isInit ){
        أرجع SQLITE_CORRUPT_BKPT;
      }
      اذهب bypass_moveto_جڑ;  /* Start search on the current page */
    }
    p_Idx_کلید->err_کوڈ = SQLITE_OK;
  }

  rc = move_منزل_جڑ(pCur);
  إذا( rc ){
    إذا( rc==SQLITE_خالی_تھاواں ){
      أكد( pCur->pgno_جڑ==0 || pCur->p_ورقہ->nCell==0 );
      *pRes = -1;
      أرجع SQLITE_OK;
    }
    أرجع rc;
  }

bypass_moveto_جڑ:
  أكد( pCur->p_ورقہ );
  أكد( pCur->p_ورقہ->isInit );
  أكد( pCur->eState==CURSOR_VALID );
  أكد( pCur->p_ورقہ->nCell > 0 );
  أكد( pCur->cur_Int_کلید==0 );
  أكد( p_Idx_کلید!=0 );
  لكل(;;){
    صحيح lwr, upr, idx, c;
    Pgno chldPg;
    Mem_ورقہ *p_ورقہ = pCur->p_ورقہ;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    أكد( p_ورقہ->nCell>0 );
    أكد( p_ورقہ->int_کلید==0 );
    lwr = 0;
    upr = p_ورقہ->nCell-1;
    idx = upr>>1; /* idx = (lwr+upr)/2; */
    لكل(;;){
      صحيح nCell;  /* Size of the pCell cell in bytes */
      pCell = لبھو_Cell_Past_Ptr(p_ورقہ, idx);

      /* The maximum supported page-size is 65536 bytes. This means that
      ** the maximum number of record bytes stored on an index B-Tree
      ** page is less than 16384 bytes and may be stored as a 2-byte
      ** varint. This information is used to attempt to avoid parsing
      ** the entire cell by checking for the cases where the record is
      ** stored entirely within the b-tree page by inspecting the first
      ** 2 bytes of the cell.
      */
      nCell = pCell[0];
      إذا( nCell<=p_ورقہ->max1bytePayload ){
        /* This branch runs if the record-size field of the cell is a
        ** single byte varint and the record fits entirely on the main
        ** b-tree page.  */
        إذا( pCell + nCell >= p_ورقہ->a_انکڑے_سرا ){
          rc = SQLITE_CORRUPT_ورقہ(p_ورقہ);
          اذهب moveto_اشاریہ_انت;
        }
        c = x_ریکارڈ_Compare(nCell, (فراغ*)&pCell[1], p_Idx_کلید);
      }وإلا إذا( !(pCell[1] & 0x80)
        && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=p_ورقہ->maxLocal
        && pCell + nCell < p_ورقہ->a_انکڑے_سرا
      ){
        /* The record-size field is a 2 byte varint and the record
        ** fits entirely on the main b-tree page.  */
        c = x_ریکارڈ_Compare(nCell, (فراغ*)&pCell[2], p_Idx_کلید);
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
        فراغ *p_Cell_کلید;
        u8 * ثابت pCellBody = pCell - p_ورقہ->بچہ_Ptr_ناپ;
        ثابت صحيح nOverrun = 18;  /* Size of the overrun padding */
        p_ورقہ->xParseCell(p_ورقہ, pCellBody, &pCur->info);
        nCell = (صحيح)pCur->info.n_کلید;
        testcase( nCell<0 );   /* True if key size is 2^32 or more */
        testcase( nCell==0 );  /* Invalid key size:  0x80 0x80 0x00 */
        testcase( nCell==1 );  /* Invalid key size:  0x80 0x80 0x01 */
        testcase( nCell==2 );  /* Minimum legal index key size */
        إذا( nCell<2 || nCell/pCur->pBt->usable_ناپ>pCur->pBt->n_ورقہ ){
          rc = SQLITE_CORRUPT_ورقہ(p_ورقہ);
          اذهب moveto_اشاریہ_انت;
        }
        p_Cell_کلید = sqlite3Malloc( (u64)nCell+(u64)nOverrun );
        إذا( p_Cell_کلید==0 ){
          rc = SQLITE_NOMEM_BKPT;
          اذهب moveto_اشاریہ_انت;
        }
        pCur->ix = (u16)idx;
        rc = accessPayload(pCur, 0, nCell, (بنانشانی حرف*)p_Cell_کلید, 0);
        memset(((u8*)p_Cell_کلید)+nCell,0,nOverrun); /* Fix uninit warnings */
        pCur->curFlags &= ~BTCF_ValidOvfl;
        إذا( rc ){
          sqlite3_free(p_Cell_کلید);
          اذهب moveto_اشاریہ_انت;
        }
        c = sqlite3_Vdbe_ریکارڈ_Compare(nCell, p_Cell_کلید, p_Idx_کلید);
        sqlite3_free(p_Cell_کلید);
      }
      أكد(
          (p_Idx_کلید->err_کوڈ!=SQLITE_CORRUPT || c==0)
       && (p_Idx_کلید->err_کوڈ!=SQLITE_NOMEM || pCur->pBtree->db->mallocFailed)
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
        إذا( p_Idx_کلید->err_کوڈ ) rc = SQLITE_CORRUPT_BKPT;
        اذهب moveto_اشاریہ_انت;
      }
      إذا( lwr>upr ) اكسر;
      أكد( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2 */
    }
    أكد( lwr==upr+1 || (p_ورقہ->int_کلید && !p_ورقہ->پتی) );
    أكد( p_ورقہ->isInit );
    إذا( p_ورقہ->پتی ){
      أكد( pCur->ix<pCur->p_ورقہ->nCell || CORRUPT_DB );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      اذهب moveto_اشاریہ_انت;
    }
    إذا( lwr>=p_ورقہ->nCell ){
      chldPg = get4byte(&p_ورقہ->a_انکڑے[p_ورقہ->hdr_وتھ+8]);
    }وإلا{
      chldPg = get4byte(لبھو_Cell(p_ورقہ, lwr));
    }

    /* This block is similar to an in-lined version of:
    **
    **    pCur->ix = (u16)lwr;
    **    rc = moveToChild(pCur, chldPg);
    **    if( rc ) break;
    */
    pCur->info.n_ناپ = 0;
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
    إذا( pCur->i_ورقہ>=(BTCURSOR_MAX_ڈونگھائی-1) ){
      أرجع SQLITE_CORRUPT_BKPT;
    }
    pCur->aiIdx[pCur->i_ورقہ] = (u16)lwr;
    pCur->ap_ورقہ[pCur->i_ورقہ] = pCur->p_ورقہ;
    pCur->ix = 0;
    pCur->i_ورقہ++;
    rc = get_And_Init_ورقہ(pCur->pBt, chldPg, &pCur->p_ورقہ, pCur->curPagerFlags);
    إذا( rc==SQLITE_OK
     && (pCur->p_ورقہ->nCell<1 || pCur->p_ورقہ->int_کلید!=pCur->cur_Int_کلید)
    ){
      release_ورقہ(pCur->p_ورقہ);
      rc = SQLITE_CORRUPT_PGNO(chldPg);
    }
    إذا( rc ){
      pCur->p_ورقہ = pCur->ap_ورقہ[--pCur->i_ورقہ];
      اكسر;
    }
    /*
    ***** End of in-lined moveToChild() call */
 }
moveto_اشاریہ_انت:
  pCur->info.n_ناپ = 0;
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
i64 sqlite3_Btree_صف_گنتی_Est(BtCursor *pCur){
  i64 n;
  u8 i;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* Currently this interface is only called by the OP_IfSizeBetween
  ** opcode and the OP_Count opcode with P3=1.  In either case,
  ** the cursor will always be valid unless the btree is empty. */
  إذا( pCur->eState!=CURSOR_VALID ) أرجع 0;
  إذا( NEVER(pCur->p_ورقہ->پتی==0) ) أرجع -1;

  n = pCur->p_ورقہ->nCell;
  لكل(i=0; i<pCur->i_ورقہ; i++){
    n *= pCur->ap_ورقہ[i]->nCell+1;
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
ساكن SQLITE_NOINLINE صحيح btree_اگلا(BtCursor *pCur){
  صحيح rc;
  صحيح idx;
  Mem_ورقہ *p_ورقہ;

  أكد( cursorOwnsBtShared(pCur) );
  إذا( pCur->eState!=CURSOR_VALID ){
    أكد( (pCur->curFlags & BTCF_ValidOvfl)==0 );
    rc = restoreCursorPosition(pCur);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
    إذا( CURSOR_INVALID==pCur->eState ){
      أرجع SQLITE_پورا;
    }
    إذا( pCur->eState==CURSOR_SKIPNEXT ){
      pCur->eState = CURSOR_VALID;
      إذا( pCur->skip_اگلا>0 ) أرجع SQLITE_OK;
    }
  }

  p_ورقہ = pCur->p_ورقہ;
  idx = ++pCur->ix;
  إذا( sqlite3FaultSim(412) ) p_ورقہ->isInit = 0;
  إذا( !p_ورقہ->isInit ){
    أرجع SQLITE_CORRUPT_BKPT;
  }

  إذا( idx>=p_ورقہ->nCell ){
    إذا( !p_ورقہ->پتی ){
      rc = move_منزل_بچہ(pCur, get4byte(&p_ورقہ->a_انکڑے[p_ورقہ->hdr_وتھ+8]));
      إذا( rc ) أرجع rc;
      أرجع move_منزل_Leftmost(pCur);
    }
    افعل{
      إذا( pCur->i_ورقہ==0 ){
        pCur->eState = CURSOR_INVALID;
        أرجع SQLITE_پورا;
      }
      move_منزل_ماپا(pCur);
      p_ورقہ = pCur->p_ورقہ;
    }طالما( pCur->ix>=p_ورقہ->nCell );
    إذا( p_ورقہ->int_کلید ){
      أرجع sqlite3_Btree_اگلا(pCur, 0);
    }وإلا{
      أرجع SQLITE_OK;
    }
  }
  إذا( p_ورقہ->پتی ){
    أرجع SQLITE_OK;
  }وإلا{
    أرجع move_منزل_Leftmost(pCur);
  }
}
صحيح sqlite3_Btree_اگلا(BtCursor *pCur, صحيح flags){
  Mem_ورقہ *p_ورقہ;
  بن_ورتیا_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  أكد( cursorOwnsBtShared(pCur) );
  أكد( flags==0 || flags==1 );
  pCur->info.n_ناپ = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  إذا( pCur->eState!=CURSOR_VALID ) أرجع btree_اگلا(pCur);
  p_ورقہ = pCur->p_ورقہ;
  إذا( (++pCur->ix)>=p_ورقہ->nCell ){
    pCur->ix--;
    أرجع btree_اگلا(pCur);
  }
  إذا( p_ورقہ->پتی ){
    أرجع SQLITE_OK;
  }وإلا{
    أرجع move_منزل_Leftmost(pCur);
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
ساكن SQLITE_NOINLINE صحيح btree_پچھلا(BtCursor *pCur){
  صحيح rc;
  Mem_ورقہ *p_ورقہ;

  أكد( cursorOwnsBtShared(pCur) );
  أكد( (pCur->curFlags & (BTCF_At_آخری|BTCF_ValidOvfl|BTCF_ValidNKey))==0 );
  أكد( pCur->info.n_ناپ==0 );
  إذا( pCur->eState!=CURSOR_VALID ){
    rc = restoreCursorPosition(pCur);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
    إذا( CURSOR_INVALID==pCur->eState ){
      أرجع SQLITE_پورا;
    }
    إذا( CURSOR_SKIPNEXT==pCur->eState ){
      pCur->eState = CURSOR_VALID;
      إذا( pCur->skip_اگلا<0 ) أرجع SQLITE_OK;
    }
  }

  p_ورقہ = pCur->p_ورقہ;
  إذا( sqlite3FaultSim(412) ) p_ورقہ->isInit = 0;
  إذا( !p_ورقہ->isInit ){
    أرجع SQLITE_CORRUPT_BKPT;
  }
  إذا( !p_ورقہ->پتی ){
    صحيح idx = pCur->ix;
    rc = move_منزل_بچہ(pCur, get4byte(لبھو_Cell(p_ورقہ, idx)));
    إذا( rc ) أرجع rc;
    rc = move_منزل_Rightmost(pCur);
  }وإلا{
    طالما( pCur->ix==0 ){
      إذا( pCur->i_ورقہ==0 ){
        pCur->eState = CURSOR_INVALID;
        أرجع SQLITE_پورا;
      }
      move_منزل_ماپا(pCur);
    }
    أكد( pCur->info.n_ناپ==0 );
    أكد( (pCur->curFlags & (BTCF_ValidOvfl))==0 );

    pCur->ix--;
    p_ورقہ = pCur->p_ورقہ;
    إذا( p_ورقہ->int_کلید && !p_ورقہ->پتی ){
      rc = sqlite3_Btree_پچھلا(pCur, 0);
    }وإلا{
      rc = SQLITE_OK;
    }
  }
  أرجع rc;
}
صحيح sqlite3_Btree_پچھلا(BtCursor *pCur, صحيح flags){
  أكد( cursorOwnsBtShared(pCur) );
  أكد( flags==0 || flags==1 );
  بن_ورتیا_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  pCur->curFlags &= ~(BTCF_At_آخری|BTCF_ValidOvfl|BTCF_ValidNKey);
  pCur->info.n_ناپ = 0;
  إذا( pCur->eState!=CURSOR_VALID
   || pCur->ix==0
   || pCur->p_ورقہ->پتی==0
  ){
    أرجع btree_پچھلا(pCur);
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
ساكن صحيح allocate_Btree_ورقہ(
  BtShared *pBt,         /* The btree */
  Mem_ورقہ **pp_ورقہ,      /* Store pointer to the allocated page here */
  Pgno *pPgno,           /* Store the page number here */
  Pgno nearby,           /* Search for a page near this one */
  u8 e_منوال               /* BTALLOC_EXACT, BTALLOC_LT, or BTALLOC_ANY */
){
  Mem_ورقہ *pPage1;
  صحيح rc;
  u32 n;     /* Number of pages on the freelist */
  u32 k;     /* Number of leaves on the trunk of the freelist */
  Mem_ورقہ *pTrunk = 0;
  Mem_ورقہ *pPrevTrunk = 0;
  Pgno mx_ورقہ;     /* Total size of the database file */

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( e_منوال==BTALLOC_ANY || (nearby>0 && IfNotOmitAV(pBt->autoVacuum)) );
  pPage1 = pBt->pPage1;
  mx_ورقہ = btreePagecount(pBt);
  /* EVIDENCE-OF: R-21003-45125 The 4-byte big-endian integer at offset 36
  ** stores the total number of pages on the freelist. */
  n = get4byte(&pPage1->a_انکڑے[36]);
  testcase( n==mx_ورقہ-1 );
  إذا( n>=mx_ورقہ ){
    أرجع SQLITE_CORRUPT_BKPT;
  }
  إذا( n>0 ){
    /* There are pages on the freelist.  Reuse one of those pages. */
    Pgno iTrunk;
    u8 تلاش_List = 0; /* If the free-list must be searched for 'nearby' */
    u32 n_تلاش = 0;   /* Count of the number of search attempts */
   
    /* If eMode==BTALLOC_EXACT and a query of the pointer-map
    ** shows that the page 'nearby' is somewhere on the free-list, then
    ** the entire-list will be searched for that page.
    */
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
    إذا( e_منوال==BTALLOC_EXACT ){
      إذا( nearby<=mx_ورقہ ){
        u8 eType;
        أكد( nearby>0 );
        أكد( pBt->autoVacuum );
        rc = ptrmapGet(pBt, nearby, &eType, 0);
        إذا( rc ) أرجع rc;
        إذا( eType==PTRMAP_FREEPAGE ){
          تلاش_List = 1;
        }
      }
    }وإلا إذا( e_منوال==BTALLOC_LE ){
      تلاش_List = 1;
    }
#جے_انت

    /* Decrement the free-list count by 1. Set iTrunk to the index of the
    ** first free-list trunk page. iPrevTrunk is initially 1.
    */
    rc = sqlite3PagerWrite(pPage1->p_Db_ورقہ);
    إذا( rc ) أرجع rc;
    put4byte(&pPage1->a_انکڑے[36], n-1);

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
        iTrunk = get4byte(&pPrevTrunk->a_انکڑے[0]);
      }وإلا{
        /* EVIDENCE-OF: R-59841-13798 The 4-byte big-endian integer at offset 32
        ** stores the page number of the first page of the freelist, or zero if
        ** the freelist is empty. */
        iTrunk = get4byte(&pPage1->a_انکڑے[32]);
      }
      testcase( iTrunk==mx_ورقہ );
      إذا( iTrunk>mx_ورقہ || n_تلاش++ > n ){
        rc = SQLITE_CORRUPT_PGNO(pPrevTrunk ? pPrevTrunk->pgno : 1);
      }وإلا{
        rc = btree_Get_بن_ورتیا_ورقہ(pBt, iTrunk, &pTrunk, 0);
      }
      إذا( rc ){
        pTrunk = 0;
        اذهب سرا_allocate_ورقہ;
      }
      أكد( pTrunk!=0 );
      أكد( pTrunk->a_انکڑے!=0 );
      /* EVIDENCE-OF: R-13523-04394 The second integer on a freelist trunk page
      ** is the number of leaf page pointers to follow. */
      k = get4byte(&pTrunk->a_انکڑے[4]);
      إذا( k==0 && !تلاش_List ){
        /* The trunk has no leaves and the list is not being searched.
        ** So extract the trunk page itself and use it as the newly
        ** allocated page */
        أكد( pPrevTrunk==0 );
        rc = sqlite3PagerWrite(pTrunk->p_Db_ورقہ);
        إذا( rc ){
          اذهب سرا_allocate_ورقہ;
        }
        *pPgno = iTrunk;
        حافظہ_نقل(&pPage1->a_انکڑے[32], &pTrunk->a_انکڑے[0], 4);
        *pp_ورقہ = pTrunk;
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
      }وإلا إذا( k>(u32)(pBt->usable_ناپ/4 - 2) ){
        /* Value of k is out of range.  Database corruption */
        rc = SQLITE_CORRUPT_PGNO(iTrunk);
        اذهب سرا_allocate_ورقہ;
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
      }وإلا إذا( تلاش_List
            && (nearby==iTrunk || (iTrunk<nearby && e_منوال==BTALLOC_LE))
      ){
        /* The list is being searched and this trunk page is the page
        ** to allocate, regardless of whether it has leaves.
        */
        *pPgno = iTrunk;
        *pp_ورقہ = pTrunk;
        تلاش_List = 0;
        rc = sqlite3PagerWrite(pTrunk->p_Db_ورقہ);
        إذا( rc ){
          اذهب سرا_allocate_ورقہ;
        }
        إذا( k==0 ){
          إذا( !pPrevTrunk ){
            حافظہ_نقل(&pPage1->a_انکڑے[32], &pTrunk->a_انکڑے[0], 4);
          }وإلا{
            rc = sqlite3PagerWrite(pPrevTrunk->p_Db_ورقہ);
            إذا( rc!=SQLITE_OK ){
              اذهب سرا_allocate_ورقہ;
            }
            حافظہ_نقل(&pPrevTrunk->a_انکڑے[0], &pTrunk->a_انکڑے[0], 4);
          }
        }وإلا{
          /* The trunk page is required by the caller but it contains
          ** pointers to free-list leaves. The first leaf becomes a trunk
          ** page in this case.
          */
          Mem_ورقہ *pNewTrunk;
          Pgno iNewTrunk = get4byte(&pTrunk->a_انکڑے[8]);
          إذا( iNewTrunk>mx_ورقہ ){
            rc = SQLITE_CORRUPT_PGNO(iTrunk);
            اذهب سرا_allocate_ورقہ;
          }
          testcase( iNewTrunk==mx_ورقہ );
          rc = btree_Get_بن_ورتیا_ورقہ(pBt, iNewTrunk, &pNewTrunk, 0);
          إذا( rc!=SQLITE_OK ){
            اذهب سرا_allocate_ورقہ;
          }
          rc = sqlite3PagerWrite(pNewTrunk->p_Db_ورقہ);
          إذا( rc!=SQLITE_OK ){
            release_ورقہ(pNewTrunk);
            اذهب سرا_allocate_ورقہ;
          }
          حافظہ_نقل(&pNewTrunk->a_انکڑے[0], &pTrunk->a_انکڑے[0], 4);
          put4byte(&pNewTrunk->a_انکڑے[4], k-1);
          حافظہ_نقل(&pNewTrunk->a_انکڑے[8], &pTrunk->a_انکڑے[12], (k-1)*4);
          release_ورقہ(pNewTrunk);
          إذا( !pPrevTrunk ){
            أكد( sqlite3PagerIswriteable(pPage1->p_Db_ورقہ) );
            put4byte(&pPage1->a_انکڑے[32], iNewTrunk);
          }وإلا{
            rc = sqlite3PagerWrite(pPrevTrunk->p_Db_ورقہ);
            إذا( rc ){
              اذهب سرا_allocate_ورقہ;
            }
            put4byte(&pPrevTrunk->a_انکڑے[0], iNewTrunk);
          }
        }
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
#جے_انت
      }وإلا إذا( k>0 ){
        /* Extract a leaf from the trunk */
        u32 closest;
        Pgno i_ورقہ;
        بنانشانی حرف *a_انکڑے = pTrunk->a_انکڑے;
        إذا( nearby>0 ){
          u32 i;
          closest = 0;
          إذا( e_منوال==BTALLOC_LE ){
            لكل(i=0; i<k; i++){
              i_ورقہ = get4byte(&a_انکڑے[8+i*4]);
              إذا( i_ورقہ<=nearby ){
                closest = i;
                اكسر;
              }
            }
          }وإلا{
            صحيح dist;
            dist = sqlite3AbsInt32(get4byte(&a_انکڑے[8]) - nearby);
            لكل(i=1; i<k; i++){
              صحيح d2 = sqlite3AbsInt32(get4byte(&a_انکڑے[8+i*4]) - nearby);
              إذا( d2<dist ){
                closest = i;
                dist = d2;
              }
            }
          }
        }وإلا{
          closest = 0;
        }

        i_ورقہ = get4byte(&a_انکڑے[8+closest*4]);
        testcase( i_ورقہ==mx_ورقہ );
        إذا( i_ورقہ>mx_ورقہ || i_ورقہ<2 ){
          rc = SQLITE_CORRUPT_PGNO(iTrunk);
          اذهب سرا_allocate_ورقہ;
        }
        testcase( i_ورقہ==mx_ورقہ );
        إذا( !تلاش_List
         || (i_ورقہ==nearby || (i_ورقہ<nearby && e_منوال==BTALLOC_LE))
        ){
          صحيح noContent;
          *pPgno = i_ورقہ;
          TRACE(("ALLOCATE: %u was leaf %u of %u on trunk %u"
                 ": %u more free pages\n",
                 *pPgno, closest+1, k, pTrunk->pgno, n-1));
          rc = sqlite3PagerWrite(pTrunk->p_Db_ورقہ);
          إذا( rc ) اذهب سرا_allocate_ورقہ;
          إذا( closest<k-1 ){
            حافظہ_نقل(&a_انکڑے[8+closest*4], &a_انکڑے[4+k*4], 4);
          }
          put4byte(&a_انکڑے[4], k-1);
          noContent = !btreeGetHasContent(pBt, *pPgno)? PAGER_GET_NOCONTENT : 0;
          rc = btree_Get_بن_ورتیا_ورقہ(pBt, *pPgno, pp_ورقہ, noContent);
          إذا( rc==SQLITE_OK ){
            rc = sqlite3PagerWrite((*pp_ورقہ)->p_Db_ورقہ);
            إذا( rc!=SQLITE_OK ){
              release_ورقہ(*pp_ورقہ);
              *pp_ورقہ = 0;
            }
          }
          تلاش_List = 0;
        }
      }
      release_ورقہ(pPrevTrunk);
      pPrevTrunk = 0;
    }طالما( تلاش_List );
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

    rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ورقہ);
    إذا( rc ) أرجع rc;
    pBt->n_ورقہ++;
    إذا( pBt->n_ورقہ==PENDING_BYTE_ورقہ(pBt) ) pBt->n_ورقہ++;

#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
    إذا( pBt->autoVacuum && PTRMAP_ISPAGE(pBt, pBt->n_ورقہ) ){
      /* If *pPgno refers to a pointer-map page, allocate two new pages
      ** at the end of the file instead of one. The first allocated page
      ** becomes a new pointer-map page, the second is used by the caller.
      */
      Mem_ورقہ *pPg = 0;
      TRACE(("ALLOCATE: %u from end of file (pointer-map page)\n", pBt->n_ورقہ));
      أكد( pBt->n_ورقہ!=PENDING_BYTE_ورقہ(pBt) );
      rc = btree_Get_بن_ورتیا_ورقہ(pBt, pBt->n_ورقہ, &pPg, bNoContent);
      إذا( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pPg->p_Db_ورقہ);
        release_ورقہ(pPg);
      }
      إذا( rc ) أرجع rc;
      pBt->n_ورقہ++;
      إذا( pBt->n_ورقہ==PENDING_BYTE_ورقہ(pBt) ){ pBt->n_ورقہ++; }
    }
#جے_انت
    put4byte(28 + (u8*)pBt->pPage1->a_انکڑے, pBt->n_ورقہ);
    *pPgno = pBt->n_ورقہ;

    أكد( *pPgno!=PENDING_BYTE_ورقہ(pBt) );
    rc = btree_Get_بن_ورتیا_ورقہ(pBt, *pPgno, pp_ورقہ, bNoContent);
    إذا( rc ) أرجع rc;
    rc = sqlite3PagerWrite((*pp_ورقہ)->p_Db_ورقہ);
    إذا( rc!=SQLITE_OK ){
      release_ورقہ(*pp_ورقہ);
      *pp_ورقہ = 0;
    }
    TRACE(("ALLOCATE: %u from end of file\n", *pPgno));
  }

  أكد( CORRUPT_DB || *pPgno!=PENDING_BYTE_ورقہ(pBt) );

سرا_allocate_ورقہ:
  release_ورقہ(pTrunk);
  release_ورقہ(pPrevTrunk);
  أكد( rc!=SQLITE_OK || sqlite3_Pager_ورقہ_Refcount((*pp_ورقہ)->p_Db_ورقہ)<=1 );
  أكد( rc!=SQLITE_OK || (*pp_ورقہ)->isInit==0 );
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
ساكن صحيح freePage2(BtShared *pBt, Mem_ورقہ *p_Mem_ورقہ, Pgno i_ورقہ){
  Mem_ورقہ *pTrunk = 0;                /* Free-list trunk page */
  Pgno iTrunk = 0;                    /* Page number of free-list trunk page */
  Mem_ورقہ *pPage1 = pBt->pPage1;      /* Local reference to page 1 */
  Mem_ورقہ *p_ورقہ;                     /* Page being freed. May be NULL. */
  صحيح rc;                             /* Return Code */
  u32 nFree;                          /* Initial number of pages on free-list */

  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( CORRUPT_DB || i_ورقہ>1 );
  أكد( !p_Mem_ورقہ || p_Mem_ورقہ->pgno==i_ورقہ );

  إذا( i_ورقہ<2 || i_ورقہ>pBt->n_ورقہ ){
    أرجع SQLITE_CORRUPT_BKPT;
  }
  إذا( p_Mem_ورقہ ){
    p_ورقہ = p_Mem_ورقہ;
    sqlite3PagerRef(p_ورقہ->p_Db_ورقہ);
  }وإلا{
    p_ورقہ = btree_ورقہ_Lookup(pBt, i_ورقہ);
  }

  /* Increment the free page count on pPage1 */
  rc = sqlite3PagerWrite(pPage1->p_Db_ورقہ);
  إذا( rc ) اذهب freepage_باہرلا;
  nFree = get4byte(&pPage1->a_انکڑے[36]);
  put4byte(&pPage1->a_انکڑے[36], nFree+1);

  إذا( pBt->btsFlags & BTS_SECURE_DELETE ){
    /* If the secure_delete option is enabled, then
    ** always fully overwrite deleted information with zeros.
    */
    إذا( (!p_ورقہ && ((rc = btree_Get_ورقہ(pBt, i_ورقہ, &p_ورقہ, 0))!=0) )
     ||            ((rc = sqlite3PagerWrite(p_ورقہ->p_Db_ورقہ))!=0)
    ){
      اذهب freepage_باہرلا;
    }
    memset(p_ورقہ->a_انکڑے, 0, p_ورقہ->pBt->ورقہ_ناپ);
  }

  /* If the database supports auto-vacuum, write an entry in the pointer-map
  ** to indicate that the page is free.
  */
  إذا( ISAUTOVACUUM(pBt) ){
    ptrmapPut(pBt, i_ورقہ, PTRMAP_FREEPAGE, 0, &rc);
    إذا( rc ) اذهب freepage_باہرلا;
  }

  /* Now manipulate the actual database free-list structure. There are two
  ** possibilities. If the free-list is currently empty, or if the first
  ** trunk page in the free-list is full, then this page will become a
  ** new free-list trunk page. Otherwise, it will become a leaf of the
  ** first trunk page in the current free-list. This block tests if it
  ** is possible to add the page as a new free-list leaf.
  */
  إذا( nFree!=0 ){
    u32 n_پتی;                /* Initial number of leaf cells on trunk page */

    iTrunk = get4byte(&pPage1->a_انکڑے[32]);
    إذا( iTrunk>btreePagecount(pBt) ){
      rc = SQLITE_CORRUPT_BKPT;
      اذهب freepage_باہرلا;
    }
    rc = btree_Get_ورقہ(pBt, iTrunk, &pTrunk, 0);
    إذا( rc!=SQLITE_OK ){
      اذهب freepage_باہرلا;
    }

    n_پتی = get4byte(&pTrunk->a_انکڑے[4]);
    أكد( pBt->usable_ناپ>32 );
    إذا( n_پتی > (u32)pBt->usable_ناپ/4 - 2 ){
      rc = SQLITE_CORRUPT_BKPT;
      اذهب freepage_باہرلا;
    }
    إذا( n_پتی < (u32)pBt->usable_ناپ/4 - 8 ){
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
      rc = sqlite3PagerWrite(pTrunk->p_Db_ورقہ);
      إذا( rc==SQLITE_OK ){
        put4byte(&pTrunk->a_انکڑے[4], n_پتی+1);
        put4byte(&pTrunk->a_انکڑے[8+n_پتی*4], i_ورقہ);
        إذا( p_ورقہ && (pBt->btsFlags & BTS_SECURE_DELETE)==0 ){
          sqlite3PagerDontWrite(p_ورقہ->p_Db_ورقہ);
        }
        rc = btree_سیٹ_Has_Content(pBt, i_ورقہ);
      }
      TRACE(("FREE-PAGE: %u leaf on trunk page %u\n",p_ورقہ->pgno,pTrunk->pgno));
      اذهب freepage_باہرلا;
    }
  }

  /* If control flows to this point, then it was not possible to add the
  ** the page being freed as a leaf page of the first trunk in the free-list.
  ** Possibly because the free-list is empty, or possibly because the
  ** first trunk in the free-list is full. Either way, the page being freed
  ** will become the new first trunk page in the free-list.
  */
  إذا( p_ورقہ==0 && SQLITE_OK!=(rc = btree_Get_ورقہ(pBt, i_ورقہ, &p_ورقہ, 0)) ){
    اذهب freepage_باہرلا;
  }
  rc = sqlite3PagerWrite(p_ورقہ->p_Db_ورقہ);
  إذا( rc!=SQLITE_OK ){
    اذهب freepage_باہرلا;
  }
  put4byte(p_ورقہ->a_انکڑے, iTrunk);
  put4byte(&p_ورقہ->a_انکڑے[4], 0);
  put4byte(&pPage1->a_انکڑے[32], i_ورقہ);
  TRACE(("FREE-PAGE: %u new trunk page replacing %u\n", p_ورقہ->pgno, iTrunk));

freepage_باہرلا:
  إذا( p_ورقہ ){
    p_ورقہ->isInit = 0;
  }
  release_ورقہ(p_ورقہ);
  release_ورقہ(pTrunk);
  أرجع rc;
}
ساكن فراغ free_ورقہ(Mem_ورقہ *p_ورقہ, صحيح *pRC){
  إذا( (*pRC)==SQLITE_OK ){
    *pRC = freePage2(p_ورقہ->pBt, p_ورقہ, p_ورقہ->pgno);
  }
}

/*
** Free the overflow pages associated with the given Cell.
*/
ساكن SQLITE_NOINLINE صحيح clear_Cell_حد_توں_ودھ(
  Mem_ورقہ *p_ورقہ,          /* The page that contains the Cell */
  بنانشانی حرف *pCell,    /* First byte of the Cell */
  CellInfo *pInfo          /* Size information about the cell */
){
  BtShared *pBt;
  Pgno ovflPgno;
  صحيح rc;
  صحيح nOvfl;
  u32 ovfl_ورقہ_ناپ;

  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  أكد( pInfo->nLocal!=pInfo->nPayload );
  testcase( pCell + pInfo->n_ناپ == p_ورقہ->a_انکڑے_سرا );
  testcase( pCell + (pInfo->n_ناپ-1) == p_ورقہ->a_انکڑے_سرا );
  إذا( pCell + pInfo->n_ناپ > p_ورقہ->a_انکڑے_سرا ){
    /* Cell extends past end of page */
    أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
  }
  ovflPgno = get4byte(pCell + pInfo->n_ناپ - 4);
  pBt = p_ورقہ->pBt;
  أكد( pBt->usable_ناپ > 4 );
  ovfl_ورقہ_ناپ = pBt->usable_ناپ - 4;
  nOvfl = (pInfo->nPayload - pInfo->nLocal + ovfl_ورقہ_ناپ - 1)/ovfl_ورقہ_ناپ;
  أكد( nOvfl>0 ||
    (CORRUPT_DB && (pInfo->nPayload + ovfl_ورقہ_ناپ)<ovfl_ورقہ_ناپ)
  );
  طالما( nOvfl-- ){
    Pgno i_اگلا = 0;
    Mem_ورقہ *pOvfl = 0;
    إذا( ovflPgno<2 || ovflPgno>btreePagecount(pBt) ){
      /* 0 is not a legal page number and page 1 cannot be an
      ** overflow page. Therefore if ovflPgno<2 or past the end of the
      ** file the database must be corrupt. */
      أرجع SQLITE_CORRUPT_BKPT;
    }
    إذا( nOvfl ){
      rc = get_حد_توں_ودھ_ورقہ(pBt, ovflPgno, &pOvfl, &i_اگلا);
      إذا( rc ) أرجع rc;
    }

    إذا( ( pOvfl || ((pOvfl = btree_ورقہ_Lookup(pBt, ovflPgno))!=0) )
     && sqlite3_Pager_ورقہ_Refcount(pOvfl->p_Db_ورقہ)!=1
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
      sqlite3PagerUnref(pOvfl->p_Db_ورقہ);
    }
    إذا( rc ) أرجع rc;
    ovflPgno = i_اگلا;
  }
  أرجع SQLITE_OK;
}

/* Call xParseCell to compute the size of a cell.  If the cell contains
** overflow, then invoke cellClearOverflow to clear out that overflow.
** Store the result code (SQLITE_OK or some error code) in rc.
**
** Implemented as macro to force inlining for performance.
*/
#تعریف_کرو BTREE_CLEAR_CELL(rc, p_ورقہ, pCell, sInfo)   \
  p_ورقہ->xParseCell(p_ورقہ, pCell, &sInfo);          \
  إذا( sInfo.nLocal!=sInfo.nPayload ){               \
    rc = clear_Cell_حد_توں_ودھ(p_ورقہ, pCell, &sInfo);   \
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
ساكن صحيح fill_وچ_Cell(
  Mem_ورقہ *p_ورقہ,                /* The page that contains the cell */
  بنانشانی حرف *pCell,          /* Complete text of the cell */
  ثابت BtreePayload *pX,        /* Payload with which to construct the cell */
  صحيح *pn_ناپ                    /* Write cell size here */
){
  صحيح nPayload;
  ثابت u8 *pSrc;
  صحيح nSrc, n, rc, mn;
  صحيح space_بایاں;
  Mem_ورقہ *p_منزل_Release;
  بنانشانی حرف *p_پہلوں;
  بنانشانی حرف *pPayload;
  BtShared *pBt;
  Pgno pgnoOvfl;
  صحيح n_سرخی;

  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );

  /* pPage is not necessarily writeable since pCell might be auxiliary
  ** buffer space that is separate from the pPage buffer area */
  أكد( pCell<p_ورقہ->a_انکڑے || pCell>=&p_ورقہ->a_انکڑے[p_ورقہ->pBt->ورقہ_ناپ]
            || sqlite3PagerIswriteable(p_ورقہ->p_Db_ورقہ) );

  /* Fill in the header. */
  n_سرخی = p_ورقہ->بچہ_Ptr_ناپ;
  إذا( p_ورقہ->int_کلید ){
    nPayload = pX->n_انکڑے + pX->nZero;
    pSrc = pX->p_انکڑے;
    nSrc = pX->n_انکڑے;
    أكد( p_ورقہ->int_کلید_پتی ); /* fillInCell() only called for leaves */
    n_سرخی += putVarint32(&pCell[n_سرخی], nPayload);
    n_سرخی += sqlite3PutVarint(&pCell[n_سرخی], *(u64*)&pX->n_کلید);
  }وإلا{
    أكد( pX->n_کلید<=0x7fffffff && pX->p_کلید!=0 );
    nSrc = nPayload = (صحيح)pX->n_کلید;
    pSrc = pX->p_کلید;
    n_سرخی += putVarint32(&pCell[n_سرخی], nPayload);
  }
 
  /* Fill in the payload */
  pPayload = &pCell[n_سرخی];
  إذا( nPayload<=p_ورقہ->maxLocal ){
    /* This is the common case where everything fits on the btree page
    ** and no overflow pages are required. */
    n = n_سرخی + nPayload;
    testcase( n==3 );
    testcase( n==4 );
    إذا( n<4 ){
      n = 4;
      pPayload[nPayload] = 0;
    }
    *pn_ناپ = n;
    أكد( nSrc<=nPayload );
    testcase( nSrc<nPayload );
    حافظہ_نقل(pPayload, pSrc, nSrc);
    memset(pPayload+nSrc, 0, nPayload-nSrc);
    أرجع SQLITE_OK;
  }

  /* If we reach this point, it means that some of the content will need
  ** to spill onto overflow pages.
  */
  mn = p_ورقہ->minLocal;
  n = mn + (nPayload - mn) % (p_ورقہ->pBt->usable_ناپ - 4);
  testcase( n==p_ورقہ->maxLocal );
  testcase( n==p_ورقہ->maxLocal+1 );
  إذا( n > p_ورقہ->maxLocal ) n = mn;
  space_بایاں = n;
  *pn_ناپ = n + n_سرخی + 4;
  p_پہلوں = &pCell[n_سرخی+n];
  p_منزل_Release = 0;
  pgnoOvfl = 0;
  pBt = p_ورقہ->pBt;

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
#جے_تعریف_شدہ SQLITE_DEBUG
  {
    CellInfo info;
    p_ورقہ->xParseCell(p_ورقہ, pCell, &info);
    أكد( n_سرخی==(صحيح)(info.pPayload - pCell) );
    أكد( info.n_کلید==pX->n_کلید );
    أكد( *pn_ناپ == info.n_ناپ );
    أكد( space_بایاں == info.nLocal );
  }
#جے_انت

  /* Write the payload into the local Cell and any extra into overflow pages */
  طالما( 1 ){
    n = nPayload;
    إذا( n>space_بایاں ) n = space_بایاں;

    /* If pToRelease is not zero than pPayload points into the data area
    ** of pToRelease.  Make sure pToRelease is still writeable. */
    أكد( p_منزل_Release==0 || sqlite3PagerIswriteable(p_منزل_Release->p_Db_ورقہ) );

    /* If pPayload is part of the data area of pPage, then make sure pPage
    ** is still writeable */
    أكد( pPayload<p_ورقہ->a_انکڑے || pPayload>=&p_ورقہ->a_انکڑے[pBt->ورقہ_ناپ]
            || sqlite3PagerIswriteable(p_ورقہ->p_Db_ورقہ) );

    إذا( nSrc>=n ){
      حافظہ_نقل(pPayload, pSrc, n);
    }وإلا إذا( nSrc>0 ){
      n = nSrc;
      حافظہ_نقل(pPayload, pSrc, n);
    }وإلا{
      memset(pPayload, 0, n);
    }
    nPayload -= n;
    إذا( nPayload<=0 ) اكسر;
    pPayload += n;
    pSrc += n;
    nSrc -= n;
    space_بایاں -= n;
    إذا( space_بایاں==0 ){
      Mem_ورقہ *pOvfl = 0;
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
      Pgno pgnoPtrmap = pgnoOvfl; /* Overflow page pointer-map entry page */
      إذا( pBt->autoVacuum ){
        افعل{
          pgnoOvfl++;
        } طالما(
          PTRMAP_ISPAGE(pBt, pgnoOvfl) || pgnoOvfl==PENDING_BYTE_ورقہ(pBt)
        );
      }
#جے_انت
      rc = allocate_Btree_ورقہ(pBt, &pOvfl, &pgnoOvfl, pgnoOvfl, 0);
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
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
          release_ورقہ(pOvfl);
        }
      }
#جے_انت
      إذا( rc ){
        release_ورقہ(p_منزل_Release);
        أرجع rc;
      }

      /* If pToRelease is not zero than pPrior points into the data area
      ** of pToRelease.  Make sure pToRelease is still writeable. */
      أكد( p_منزل_Release==0 || sqlite3PagerIswriteable(p_منزل_Release->p_Db_ورقہ) );

      /* If pPrior is part of the data area of pPage, then make sure pPage
      ** is still writeable */
      أكد( p_پہلوں<p_ورقہ->a_انکڑے || p_پہلوں>=&p_ورقہ->a_انکڑے[pBt->ورقہ_ناپ]
            || sqlite3PagerIswriteable(p_ورقہ->p_Db_ورقہ) );

      put4byte(p_پہلوں, pgnoOvfl);
      release_ورقہ(p_منزل_Release);
      p_منزل_Release = pOvfl;
      p_پہلوں = pOvfl->a_انکڑے;
      put4byte(p_پہلوں, 0);
      pPayload = &pOvfl->a_انکڑے[4];
      space_بایاں = pBt->usable_ناپ - 4;
    }
  }
  release_ورقہ(p_منزل_Release);
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
ساكن فراغ dropCell(Mem_ورقہ *p_ورقہ, صحيح idx, صحيح sz, صحيح *pRC){
  u32 pc;         /* Offset to cell content of cell being deleted */
  u8 *انکڑے;       /* pPage->aData */
  u8 *ptr;        /* Used to move bytes around within data[] */
  صحيح rc;         /* The return code */
  صحيح hdr;        /* Beginning of the header.  0 most pages.  100 page 1 */

  إذا( *pRC ) أرجع;
  أكد( idx>=0 );
  أكد( idx<p_ورقہ->nCell );
  أكد( CORRUPT_DB || sz==cell_ناپ(p_ورقہ, idx) );
  أكد( sqlite3PagerIswriteable(p_ورقہ->p_Db_ورقہ) );
  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  أكد( p_ورقہ->nFree>=0 );
  انکڑے = p_ورقہ->a_انکڑے;
  ptr = &p_ورقہ->aCellIdx[2*idx];
  أكد( p_ورقہ->pBt->usable_ناپ > (u32)(ptr-انکڑے) );
  pc = get2byte(ptr);
  hdr = p_ورقہ->hdr_وتھ;
  testcase( pc==(u32)get2byte(&انکڑے[hdr+5]) );
  testcase( pc+sz==p_ورقہ->pBt->usable_ناپ );
  إذا( pc+sz > p_ورقہ->pBt->usable_ناپ ){
    *pRC = SQLITE_CORRUPT_BKPT;
    أرجع;
  }
  rc = freeSpace(p_ورقہ, pc, sz);
  إذا( rc ){
    *pRC = rc;
    أرجع;
  }
  p_ورقہ->nCell--;
  إذا( p_ورقہ->nCell==0 ){
    memset(&انکڑے[hdr+1], 0, 4);
    انکڑے[hdr+7] = 0;
    put2byte(&انکڑے[hdr+5], p_ورقہ->pBt->usable_ناپ);
    p_ورقہ->nFree = p_ورقہ->pBt->usable_ناپ - p_ورقہ->hdr_وتھ
                       - p_ورقہ->بچہ_Ptr_ناپ - 8;
  }وإلا{
    memmove(ptr, ptr+2, 2*(p_ورقہ->nCell - idx));
    put2byte(&انکڑے[hdr+3], p_ورقہ->nCell);
    p_ورقہ->nFree += 2;
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
ساكن صحيح داخل_کرو_Cell(
  Mem_ورقہ *p_ورقہ,   /* Page into which we are copying */
  صحيح i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  صحيح sz,           /* Bytes of content in pCell */
  u8 *p_عارضی,        /* Temp storage space for pCell, if needed */
  Pgno i_بچہ       /* If non-zero, replace first 4 bytes with this value */
){
  صحيح idx = 0;      /* Where to write new cell content in data[] */
  صحيح j;            /* Loop counter */
  u8 *انکڑے;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  أكد( i>=0 && i<=p_ورقہ->nCell+p_ورقہ->n_حد_توں_ودھ );
  أكد( MX_CELL(p_ورقہ->pBt)<=10921 );
  أكد( p_ورقہ->nCell<=MX_CELL(p_ورقہ->pBt) || CORRUPT_DB );
  أكد( p_ورقہ->n_حد_توں_ودھ<=Array_ناپ(p_ورقہ->apOvfl) );
  أكد( Array_ناپ(p_ورقہ->apOvfl)==Array_ناپ(p_ورقہ->aiOvfl) );
  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  أكد( sz==p_ورقہ->x_Cell_ناپ(p_ورقہ, pCell) || CORRUPT_DB );
  أكد( p_ورقہ->nFree>=0 );
  أكد( i_بچہ>0 );
  إذا( p_ورقہ->n_حد_توں_ودھ || sz+2>p_ورقہ->nFree ){
    إذا( p_عارضی ){
      حافظہ_نقل(p_عارضی, pCell, sz);
      pCell = p_عارضی;
    }
    put4byte(pCell, i_بچہ);
    j = p_ورقہ->n_حد_توں_ودھ++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    أكد( j < Array_ناپ(p_ورقہ->apOvfl)-1 );
    p_ورقہ->apOvfl[j] = pCell;
    p_ورقہ->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    أكد( j==0 || p_ورقہ->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    أكد( j==0 || i==p_ورقہ->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }وإلا{
    صحيح rc = sqlite3PagerWrite(p_ورقہ->p_Db_ورقہ);
    إذا( NEVER(rc!=SQLITE_OK) ){
      أرجع rc;
    }
    أكد( sqlite3PagerIswriteable(p_ورقہ->p_Db_ورقہ) );
    انکڑے = p_ورقہ->a_انکڑے;
    أكد( &انکڑے[p_ورقہ->cell_وتھ]==p_ورقہ->aCellIdx );
    rc = allocateSpace(p_ورقہ, sz, &idx);
    إذا( rc ){ أرجع rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    أكد( idx >= 0 );
    أكد( idx >= p_ورقہ->cell_وتھ+2*p_ورقہ->nCell+2 || CORRUPT_DB );
    أكد( idx+sz <= (صحيح)p_ورقہ->pBt->usable_ناپ );
    p_ورقہ->nFree -= (u16)(2 + sz);
    /* In a corrupt database where an entry in the cell index section of
    ** a btree page has a value of 3 or less, the pCell value might point
    ** as many as 4 bytes in front of the start of the aData buffer for
    ** the source page.  Make sure this does not cause problems by not
    ** reading the first 4 bytes */
    حافظہ_نقل(&انکڑے[idx+4], pCell+4, sz-4);
    put4byte(&انکڑے[idx], i_بچہ);
    pIns = p_ورقہ->aCellIdx + i*2;
    إذا( i<p_ورقہ->nCell ) memmove(pIns+2, pIns, 2*(p_ورقہ->nCell - i));
    put2byte(pIns, idx);
    p_ورقہ->nCell++;
    /* increment the cell count */
    إذا( (++انکڑے[p_ورقہ->hdr_وتھ+4])==0 ) انکڑے[p_ورقہ->hdr_وتھ+3]++;
    أكد( get2byte(&انکڑے[p_ورقہ->hdr_وتھ+3])==p_ورقہ->nCell || CORRUPT_DB );
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
    إذا( p_ورقہ->pBt->autoVacuum ){
      صحيح rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(p_ورقہ, p_ورقہ, pCell, &rc2);
      إذا( rc2 ) أرجع rc2;
    }
#جے_انت
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
ساكن صحيح داخل_کرو_Cell_Fast(
  Mem_ورقہ *p_ورقہ,   /* Page into which we are copying */
  صحيح i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  صحيح sz            /* Bytes of content in pCell */
){
  صحيح idx = 0;      /* Where to write new cell content in data[] */
  صحيح j;            /* Loop counter */
  u8 *انکڑے;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  أكد( i>=0 && i<=p_ورقہ->nCell+p_ورقہ->n_حد_توں_ودھ );
  أكد( MX_CELL(p_ورقہ->pBt)<=10921 );
  أكد( p_ورقہ->nCell<=MX_CELL(p_ورقہ->pBt) || CORRUPT_DB );
  أكد( p_ورقہ->n_حد_توں_ودھ<=Array_ناپ(p_ورقہ->apOvfl) );
  أكد( Array_ناپ(p_ورقہ->apOvfl)==Array_ناپ(p_ورقہ->aiOvfl) );
  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  أكد( sz==p_ورقہ->x_Cell_ناپ(p_ورقہ, pCell) || CORRUPT_DB );
  أكد( p_ورقہ->nFree>=0 );
  أكد( p_ورقہ->n_حد_توں_ودھ==0 );
  إذا( sz+2>p_ورقہ->nFree ){
    j = p_ورقہ->n_حد_توں_ودھ++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    أكد( j < Array_ناپ(p_ورقہ->apOvfl)-1 );
    p_ورقہ->apOvfl[j] = pCell;
    p_ورقہ->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    أكد( j==0 || p_ورقہ->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    أكد( j==0 || i==p_ورقہ->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }وإلا{
    صحيح rc = sqlite3PagerWrite(p_ورقہ->p_Db_ورقہ);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
    أكد( sqlite3PagerIswriteable(p_ورقہ->p_Db_ورقہ) );
    انکڑے = p_ورقہ->a_انکڑے;
    أكد( &انکڑے[p_ورقہ->cell_وتھ]==p_ورقہ->aCellIdx );
    rc = allocateSpace(p_ورقہ, sz, &idx);
    إذا( rc ){ أرجع rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    أكد( idx >= 0 );
    أكد( idx >= p_ورقہ->cell_وتھ+2*p_ورقہ->nCell+2 || CORRUPT_DB );
    أكد( idx+sz <= (صحيح)p_ورقہ->pBt->usable_ناپ );
    p_ورقہ->nFree -= (u16)(2 + sz);
    حافظہ_نقل(&انکڑے[idx], pCell, sz);
    pIns = p_ورقہ->aCellIdx + i*2;
    إذا( i<p_ورقہ->nCell ) memmove(pIns+2, pIns, 2*(p_ورقہ->nCell - i));
    put2byte(pIns, idx);
    p_ورقہ->nCell++;
    /* increment the cell count */
    إذا( (++انکڑے[p_ورقہ->hdr_وتھ+4])==0 ) انکڑے[p_ورقہ->hdr_وتھ+3]++;
    أكد( get2byte(&انکڑے[p_ورقہ->hdr_وتھ+3])==p_ورقہ->nCell || CORRUPT_DB );
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
    إذا( p_ورقہ->pBt->autoVacuum ){
      صحيح rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(p_ورقہ, p_ورقہ, pCell, &rc2);
      إذا( rc2 ) أرجع rc2;
    }
#جے_انت
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
تعريفنوع بنية CellArray CellArray;
بنية CellArray {
  صحيح nCell;              /* Number of cells in apCell[] */
  Mem_ورقہ *pRef;          /* Reference page */
  u8 **apCell;            /* All cells begin balanced */
  u16 *szCell;            /* Local size of all cells in apCell[] */
  u8 *ap_سرا[NB*2];        /* MemPage.aDataEnd values */
  صحيح ixNx[NB*2];         /* Index of at which we move to the next apEnd[] */
};

/*
** Make sure the cell sizes at idx, idx+1, ..., idx+N-1 have been
** computed.
*/
ساكن فراغ populateCellCache(CellArray *p, صحيح idx, صحيح N){
  Mem_ورقہ *pRef = p->pRef;
  u16 *szCell = p->szCell;
  أكد( idx>=0 && idx+N<=p->nCell );
  طالما( N>0 ){
    أكد( p->apCell[idx]!=0 );
    إذا( szCell[idx]==0 ){
      szCell[idx] = pRef->x_Cell_ناپ(pRef, p->apCell[idx]);
    }وإلا{
      أكد( CORRUPT_DB ||
              szCell[idx]==pRef->x_Cell_ناپ(pRef, p->apCell[idx]) );
    }
    idx++;
    N--;
  }
}

/*
** Return the size of the Nth element of the cell array
*/
ساكن SQLITE_NOINLINE u16 compute_Cell_ناپ(CellArray *p, صحيح N){
  أكد( N>=0 && N<p->nCell );
  أكد( p->szCell[N]==0 );
  p->szCell[N] = p->pRef->x_Cell_ناپ(p->pRef, p->apCell[N]);
  أرجع p->szCell[N];
}
ساكن u16 cached_Cell_ناپ(CellArray *p, صحيح N){
  أكد( N>=0 && N<p->nCell );
  إذا( p->szCell[N] ) أرجع p->szCell[N];
  أرجع compute_Cell_ناپ(p, N);
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
ساكن صحيح rebuild_ورقہ(
  CellArray *pCArray,             /* Content to be added to page pPg */
  صحيح i_پہلا,                     /* First cell in pCArray to use */
  صحيح nCell,                      /* Final number of cells on page */
  Mem_ورقہ *pPg                    /* The page to be reconstructed */
){
  ثابت صحيح hdr = pPg->hdr_وتھ;          /* Offset of header on pPg */
  u8 * ثابت a_انکڑے = pPg->a_انکڑے;           /* Pointer to data for pPg */
  ثابت صحيح usable_ناپ = pPg->pBt->usable_ناپ;
  u8 * ثابت p_سرا = &a_انکڑے[usable_ناپ];
  صحيح i = i_پہلا;                 /* Which cell to copy from pCArray*/
  u32 j;                          /* Start of cell content area */
  صحيح i_سرا = i+nCell;             /* Loop terminator */
  u8 *pCellptr = pPg->aCellIdx;
  u8 *pTmp = sqlite3_Pager_عارضی_Space(pPg->pBt->pPager);
  u8 *p_انکڑے;
  صحيح k;                          /* Current slot in pCArray->apEnd[] */
  u8 *p_Src_سرا;                    /* Current pCArray->apEnd[k] value */

  أكد( nCell>0 );
  أكد( i<i_سرا );
  j = get2byte(&a_انکڑے[hdr+5]);
  إذا( j>(u32)usable_ناپ ){ j = 0; }
  حافظہ_نقل(&pTmp[j], &a_انکڑے[j], usable_ناپ - j);

  أكد( pCArray->ixNx[NB*2-1]>i );
  لكل(k=0; pCArray->ixNx[k]<=i; k++){}
  p_Src_سرا = pCArray->ap_سرا[k];

  p_انکڑے = p_سرا;
  طالما( 1/*exit by break*/ ){
    u8 *pCell = pCArray->apCell[i];
    u16 sz = pCArray->szCell[i];
    أكد( sz>0 );
    إذا( SQLITE_WITHIN(pCell,a_انکڑے+j,p_سرا) ){
      إذا( ((uptr)(pCell+sz))>(uptr)p_سرا ) أرجع SQLITE_CORRUPT_BKPT;
      pCell = &pTmp[pCell - a_انکڑے];
    }وإلا إذا( (uptr)(pCell+sz)>(uptr)p_Src_سرا
           && (uptr)(pCell)<(uptr)p_Src_سرا
    ){
      أرجع SQLITE_CORRUPT_BKPT;
    }

    p_انکڑے -= sz;
    put2byte(pCellptr, (p_انکڑے - a_انکڑے));
    pCellptr += 2;
    إذا( p_انکڑے < pCellptr ) أرجع SQLITE_CORRUPT_BKPT;
    memmove(p_انکڑے, pCell, sz);
    أكد( sz==pPg->x_Cell_ناپ(pPg, pCell) || CORRUPT_DB );
    i++;
    إذا( i>=i_سرا ) اكسر;
    إذا( pCArray->ixNx[k]<=i ){
      k++;
      p_Src_سرا = pCArray->ap_سرا[k];
    }
  }

  /* The pPg->nFree field is now set incorrectly. The caller will fix it. */
  أكد( nCell < 10922 );
  pPg->nCell = (u16)nCell;
  pPg->n_حد_توں_ودھ = 0;

  put2byte(&a_انکڑے[hdr+1], 0);
  put2byte(&a_انکڑے[hdr+3], pPg->nCell);
  put2byte(&a_انکڑے[hdr+5], p_انکڑے - a_انکڑے);
  a_انکڑے[hdr+7] = 0x00;
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
ساكن صحيح ورقہ_داخل_کرو_Array(
  Mem_ورقہ *pPg,                   /* Page to add cells to */
  u8 *pBegin,                     /* End of cell-pointer array */
  u8 **pp_انکڑے,                    /* IN/OUT: Page content-area pointer */
  u8 *pCellptr,                   /* Pointer to cell-pointer area */
  صحيح i_پہلا,                     /* Index of first cell to add */
  صحيح nCell,                      /* Number of cells to add to pPg */
  CellArray *pCArray              /* Array of cells */
){
  صحيح i = i_پہلا;                 /* Loop counter - cell index to insert */
  u8 *a_انکڑے = pPg->a_انکڑے;         /* Complete page */
  u8 *p_انکڑے = *pp_انکڑے;            /* Content area.  A subset of aData[] */
  صحيح i_سرا = i_پہلا + nCell;      /* End of loop. One past last cell to ins */
  صحيح k;                          /* Current slot in pCArray->apEnd[] */
  u8 *p_سرا;                       /* Maximum extent of cell data */
  أكد( CORRUPT_DB || pPg->hdr_وتھ==0 );    /* Never called on page 1 */
  إذا( i_سرا<=i_پہلا ) أرجع 0;
  أكد( pCArray->ixNx[NB*2-1]>i );
  لكل(k=0; pCArray->ixNx[k]<=i ; k++){}
  p_سرا = pCArray->ap_سرا[k];
  طالما( 1 /*Exit by break*/ ){
    صحيح sz, rc;
    u8 *p_تھاں;
    أكد( pCArray->szCell[i]!=0 );
    sz = pCArray->szCell[i];
    إذا( (a_انکڑے[1]==0 && a_انکڑے[2]==0) || (p_تھاں = ورقہ_لبھو_تھاں(pPg,sz,&rc))==0 ){
      إذا( (p_انکڑے - pBegin)<sz ) أرجع 1;
      p_انکڑے -= sz;
      p_تھاں = p_انکڑے;
    }
    /* pSlot and pCArray->apCell[i] will never overlap on a well-formed
    ** database.  But they might for a corrupt database.  Hence use memmove()
    ** since memcpy() sends SIGABORT with overlapping buffers on OpenBSD */
    أكد( (p_تھاں+sz)<=pCArray->apCell[i]
         || p_تھاں>=(pCArray->apCell[i]+sz)
         || CORRUPT_DB );
    إذا( (uptr)(pCArray->apCell[i]+sz)>(uptr)p_سرا
     && (uptr)(pCArray->apCell[i])<(uptr)p_سرا
    ){
      أكد( CORRUPT_DB );
      (فراغ)SQLITE_CORRUPT_BKPT;
      أرجع 1;
    }
    memmove(p_تھاں, pCArray->apCell[i], sz);
    put2byte(pCellptr, (p_تھاں - a_انکڑے));
    pCellptr += 2;
    i++;
    إذا( i>=i_سرا ) اكسر;
    إذا( pCArray->ixNx[k]<=i ){
      k++;
      p_سرا = pCArray->ap_سرا[k];
    }
  }
  *pp_انکڑے = p_انکڑے;
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
ساكن صحيح ورقہ_Free_Array(
  Mem_ورقہ *pPg,                   /* Page to edit */
  صحيح i_پہلا,                     /* First cell to delete */
  صحيح nCell,                      /* Cells to delete */
  CellArray *pCArray              /* Array of cells */
){
  u8 * ثابت a_انکڑے = pPg->a_انکڑے;
  u8 * ثابت p_سرا = &a_انکڑے[pPg->pBt->usable_ناپ];
  u8 * ثابت p_شروع = &a_انکڑے[pPg->hdr_وتھ + 8 + pPg->بچہ_Ptr_ناپ];
  صحيح nRet = 0;
  صحيح i, j;
  صحيح i_سرا = i_پہلا + nCell;
  صحيح nFree = 0;
  صحيح aOfst[10];
  صحيح aAfter[10];

  لكل(i=i_پہلا; i<i_سرا; i++){
    u8 *pCell = pCArray->apCell[i];
    إذا( SQLITE_WITHIN(pCell, p_شروع, p_سرا) ){
      صحيح sz;
      صحيح iAfter;
      صحيح iOfst;
      /* No need to use cachedCellSize() here.  The sizes of all cells that
      ** are to be freed have already been computing while deciding which
      ** cells need freeing */
      sz = pCArray->szCell[i];  أكد( sz>0 );
      iOfst = (u16)(pCell - a_انکڑے);
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
        إذا( &a_انکڑے[iAfter]>p_سرا ) أرجع 0;
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
ساكن صحيح edit_ورقہ(
  Mem_ورقہ *pPg,                   /* Edit this page */
  صحيح iOld,                       /* Index of first cell currently on page */
  صحيح iNew,                       /* Index of new first cell on page */
  صحيح nNew,                       /* Final number of cells on page */
  CellArray *pCArray              /* Array of cells and sizes */
){
  u8 * ثابت a_انکڑے = pPg->a_انکڑے;
  ثابت صحيح hdr = pPg->hdr_وتھ;
  u8 *pBegin = &pPg->aCellIdx[nNew * 2];
  صحيح nCell = pPg->nCell;       /* Cells stored on pPg */
  u8 *p_انکڑے;
  u8 *pCellptr;
  صحيح i;
  صحيح i_Old_سرا = iOld + pPg->nCell + pPg->n_حد_توں_ودھ;
  صحيح i_New_سرا = iNew + nNew;

#جے_تعریف_شدہ SQLITE_DEBUG
  u8 *pTmp = sqlite3_Pager_عارضی_Space(pPg->pBt->pPager);
  حافظہ_نقل(pTmp, a_انکڑے, pPg->pBt->usable_ناپ);
#جے_انت

  /* Remove cells from the start and end of the page */
  أكد( nCell>=0 );
  إذا( iOld<iNew ){
    صحيح n_کھسکاؤ = ورقہ_Free_Array(pPg, iOld, iNew-iOld, pCArray);
    إذا( NEVER(n_کھسکاؤ>nCell) ) أرجع SQLITE_CORRUPT_BKPT;
    memmove(pPg->aCellIdx, &pPg->aCellIdx[n_کھسکاؤ*2], nCell*2);
    nCell -= n_کھسکاؤ;
  }
  إذا( i_New_سرا < i_Old_سرا ){
    صحيح n_پوچھ = ورقہ_Free_Array(pPg, i_New_سرا, i_Old_سرا - i_New_سرا, pCArray);
    أكد( nCell>=n_پوچھ );
    nCell -= n_پوچھ;
  }

  p_انکڑے = &a_انکڑے[get2byte(&a_انکڑے[hdr+5])];
  إذا( p_انکڑے<pBegin ) اذهب editpage_fail;
  إذا( NEVER(p_انکڑے>pPg->a_انکڑے_سرا) ) اذهب editpage_fail;

  /* Add cells to the start of the page */
  إذا( iNew<iOld ){
    صحيح n_جوڑو = MIN(nNew,iOld-iNew);
    أكد( (iOld-iNew)<nNew || nCell==0 || CORRUPT_DB );
    أكد( n_جوڑو>=0 );
    pCellptr = pPg->aCellIdx;
    memmove(&pCellptr[n_جوڑو*2], pCellptr, nCell*2);
    إذا( ورقہ_داخل_کرو_Array(
          pPg, pBegin, &p_انکڑے, pCellptr,
          iNew, n_جوڑو, pCArray
    ) ) اذهب editpage_fail;
    nCell += n_جوڑو;
  }

  /* Add any overflow cells */
  لكل(i=0; i<pPg->n_حد_توں_ودھ; i++){
    صحيح iCell = (iOld + pPg->aiOvfl[i]) - iNew;
    إذا( iCell>=0 && iCell<nNew ){
      pCellptr = &pPg->aCellIdx[iCell * 2];
      إذا( nCell>iCell ){
        memmove(&pCellptr[2], pCellptr, (nCell - iCell) * 2);
      }
      nCell++;
      cached_Cell_ناپ(pCArray, iCell+iNew);
      إذا( ورقہ_داخل_کرو_Array(
            pPg, pBegin, &p_انکڑے, pCellptr,
            iCell+iNew, 1, pCArray
      ) ) اذهب editpage_fail;
    }
  }

  /* Append cells to the end of the page */
  أكد( nCell>=0 );
  pCellptr = &pPg->aCellIdx[nCell*2];
  إذا( ورقہ_داخل_کرو_Array(
        pPg, pBegin, &p_انکڑے, pCellptr,
        iNew+nCell, nNew-nCell, pCArray
      )
  ){
    اذهب editpage_fail;
  }

  أكد( nNew < 10922 );
  pPg->nCell = (u16)nNew;
  pPg->n_حد_توں_ودھ = 0;

  put2byte(&a_انکڑے[hdr+3], pPg->nCell);
  put2byte(&a_انکڑے[hdr+5], p_انکڑے - a_انکڑے);

#جے_تعریف_شدہ SQLITE_DEBUG
  لكل(i=0; i<nNew && !CORRUPT_DB; i++){
    u8 *pCell = pCArray->apCell[i+iNew];
    صحيح iOff = get2byteAligned(&pPg->aCellIdx[i*2]);
    إذا( SQLITE_WITHIN(pCell, a_انکڑے, &a_انکڑے[pPg->pBt->usable_ناپ]) ){
      pCell = &pTmp[pCell - a_انکڑے];
    }
    أكد( 0==memcmp(pCell, &a_انکڑے[iOff],
            pCArray->pRef->x_Cell_ناپ(pCArray->pRef, pCArray->apCell[i+iNew])) );
  }
#جے_انت

  أرجع SQLITE_OK;
 editpage_fail:
  /* Unable to edit this page. Rebuild it from scratch instead. */
  إذا( nNew<1 ) أرجع SQLITE_CORRUPT_BKPT;
  populateCellCache(pCArray, iNew, nNew);
  أرجع rebuild_ورقہ(pCArray, iNew, nNew, pPg);
}


#جے_بنا_تعریف SQLITE_OMIT_QUICKBALANCE
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
ساكن صحيح balance_تیز(Mem_ورقہ *p_ماپا, Mem_ورقہ *p_ورقہ, u8 *pSpace){
  BtShared *ثابت pBt = p_ورقہ->pBt;    /* B-Tree Database */
  Mem_ورقہ *pNew;                       /* Newly allocated page */
  صحيح rc;                              /* Return Code */
  Pgno pgnoNew;                        /* Page number of pNew */

  أكد( sqlite3_mutex_held(p_ورقہ->pBt->mutex) );
  أكد( sqlite3PagerIswriteable(p_ماپا->p_Db_ورقہ) );
  أكد( p_ورقہ->n_حد_توں_ودھ==1 );
 
  إذا( p_ورقہ->nCell==0 ) أرجع SQLITE_CORRUPT_BKPT;  /* dbfuzz001.test */
  أكد( p_ورقہ->nFree>=0 );
  أكد( p_ماپا->nFree>=0 );

  /* Allocate a new page. This page will become the right-sibling of
  ** pPage. Make the parent page writable, so that the new divider cell
  ** may be inserted. If both these operations are successful, proceed.
  */
  rc = allocate_Btree_ورقہ(pBt, &pNew, &pgnoNew, 0, 0);

  إذا( rc==SQLITE_OK ){

    u8 *p_باہرلا = &pSpace[4];
    u8 *pCell = p_ورقہ->apOvfl[0];
    u16 szCell = p_ورقہ->x_Cell_ناپ(p_ورقہ, pCell);
    u8 *pStop;
    CellArray b;

    أكد( sqlite3PagerIswriteable(pNew->p_Db_ورقہ) );
    أكد( CORRUPT_DB || p_ورقہ->a_انکڑے[0]==(PTF_INTKEY|PTF_LEAFDATA|PTF_پتی) );
    zero_ورقہ(pNew, PTF_INTKEY|PTF_LEAFDATA|PTF_پتی);
    b.nCell = 1;
    b.pRef = p_ورقہ;
    b.apCell = &pCell;
    b.szCell = &szCell;
    b.ap_سرا[0] = p_ورقہ->a_انکڑے_سرا;
    b.ixNx[0] = 2;
    b.ixNx[NB*2-1] = 0x7fffffff;
    rc = rebuild_ورقہ(&b, 0, 1, pNew);
    إذا( NEVER(rc) ){
      release_ورقہ(pNew);
      أرجع rc;
    }
    pNew->nFree = pBt->usable_ناپ - pNew->cell_وتھ - 2 - szCell;

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
      ptrmapPut(pBt, pgnoNew, PTRMAP_BTREE, p_ماپا->pgno, &rc);
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
    pCell = لبھو_Cell(p_ورقہ, p_ورقہ->nCell-1);
    pStop = &pCell[9];
    طالما( (*(pCell++)&0x80) && pCell<pStop );
    pStop = &pCell[9];
    طالما( ((*(p_باہرلا++) = *(pCell++))&0x80) && pCell<pStop );

    /* Insert the new divider cell into pParent. */
    إذا( rc==SQLITE_OK ){
      rc = داخل_کرو_Cell(p_ماپا, p_ماپا->nCell, pSpace, (صحيح)(p_باہرلا-pSpace),
                      0, p_ورقہ->pgno);
    }

    /* Set the right-child pointer of pParent to point to the new page. */
    put4byte(&p_ماپا->a_انکڑے[p_ماپا->hdr_وتھ+8], pgnoNew);
 
    /* Release the reference to the new page. */
    release_ورقہ(pNew);
  }

  أرجع rc;
}
#جے_انت /* SQLITE_OMIT_QUICKBALANCE */

#إذا 0
/*
** This function does not contribute anything to the operation of SQLite.
** it is sometimes activated temporarily while debugging code responsible
** for setting pointer-map entries.
*/
ساكن صحيح ptrmap_Check_ورقے(Mem_ورقہ **ap_ورقہ, صحيح n_ورقہ){
  صحيح i, j;
  لكل(i=0; i<n_ورقہ; i++){
    Pgno n;
    u8 e;
    Mem_ورقہ *p_ورقہ = ap_ورقہ[i];
    BtShared *pBt = p_ورقہ->pBt;
    أكد( p_ورقہ->isInit );

    لكل(j=0; j<p_ورقہ->nCell; j++){
      CellInfo info;
      u8 *z;
    
      z = لبھو_Cell(p_ورقہ, j);
      p_ورقہ->xParseCell(p_ورقہ, z, &info);
      إذا( info.nLocal<info.nPayload ){
        Pgno ovfl = get4byte(&z[info.n_ناپ-4]);
        ptrmapGet(pBt, ovfl, &e, &n);
        أكد( n==p_ورقہ->pgno && e==PTRMAP_OVERFLOW1 );
      }
      إذا( !p_ورقہ->پتی ){
        Pgno بچہ = get4byte(z);
        ptrmapGet(pBt, بچہ, &e, &n);
        أكد( n==p_ورقہ->pgno && e==PTRMAP_BTREE );
      }
    }
    إذا( !p_ورقہ->پتی ){
      Pgno بچہ = get4byte(&p_ورقہ->a_انکڑے[p_ورقہ->hdr_وتھ+8]);
      ptrmapGet(pBt, بچہ, &e, &n);
      أكد( n==p_ورقہ->pgno && e==PTRMAP_BTREE );
    }
  }
  أرجع 1;
}
#جے_انت

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
ساكن فراغ copy_گرہ_Content(Mem_ورقہ *pFrom, Mem_ورقہ *p_منزل, صحيح *pRC){
  إذا( (*pRC)==SQLITE_OK ){
    BtShared * ثابت pBt = pFrom->pBt;
    u8 * ثابت aFrom = pFrom->a_انکڑے;
    u8 * ثابت a_منزل = p_منزل->a_انکڑے;
    صحيح ثابت iFromHdr = pFrom->hdr_وتھ;
    صحيح ثابت i_منزل_Hdr = ((p_منزل->pgno==1) ? 100 : 0);
    صحيح rc;
    صحيح i_انکڑے;
 
 
    أكد( pFrom->isInit );
    أكد( pFrom->nFree>=i_منزل_Hdr );
    أكد( get2byte(&aFrom[iFromHdr+5]) <= (صحيح)pBt->usable_ناپ );
 
    /* Copy the b-tree node content from page pFrom to page pTo. */
    i_انکڑے = get2byte(&aFrom[iFromHdr+5]);
    حافظہ_نقل(&a_منزل[i_انکڑے], &aFrom[i_انکڑے], pBt->usable_ناپ-i_انکڑے);
    حافظہ_نقل(&a_منزل[i_منزل_Hdr], &aFrom[iFromHdr], pFrom->cell_وتھ + 2*pFrom->nCell);
 
    /* Reinitialize page pTo so that the contents of the MemPage structure
    ** match the new data. The initialization of pTo can actually fail under
    ** fairly obscure circumstances, even though it is a copy of initialized
    ** page pFrom.
    */
    p_منزل->isInit = 0;
    rc = btree_Init_ورقہ(p_منزل);
    إذا( rc==SQLITE_OK ) rc = btreeComputeFreeSpace(p_منزل);
    إذا( rc!=SQLITE_OK ){
      *pRC = rc;
      أرجع;
    }
 
    /* If this is an auto-vacuum database, update the pointer-map entries
    ** for any b-tree or overflow pages that pTo now contains the pointers to.
    */
    إذا( ISAUTOVACUUM(pBt) ){
      *pRC = سیٹ_بچہ_Ptrmaps(p_منزل);
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
  Mem_ورقہ *p_ماپا,               /* Parent page of siblings being balanced */
  صحيح i_ماپا_Idx,                 /* Index of "the page" in pParent */
  u8 *aOvflSpace,                 /* page-size bytes of space for parent ovfl */
  صحيح is_جڑ,                     /* True if pParent is a root-page */
  صحيح bBulk                       /* True if this call is part of a bulk load */
){
  BtShared *pBt;               /* The whole database */
  صحيح nMaxCells = 0;           /* Allocated size of apCell, szCell, aFrom. */
  صحيح nNew = 0;                /* Number of pages in apNew[] */
  صحيح nOld;                    /* Number of pages in apOld[] */
  صحيح i, j, k;                 /* Loop counters */
  صحيح nxDiv;                   /* Next divider slot in pParent->aCell[] */
  صحيح rc = SQLITE_OK;          /* The return code */
  u16 پتی_Correction;          /* 4 if pPage is a leaf.  0 if not */
  صحيح پتی_انکڑے;                /* True if pPage is a leaf of a LEAFDATA tree */
  صحيح usableSpace;             /* Bytes in pPage beyond the header */
  صحيح ورقہ_Flags;               /* Value of pPage->aData[0] */
  صحيح iSpace1 = 0;             /* First unused byte of aSpace1[] */
  صحيح iOvflSpace = 0;          /* First unused byte of aOvflSpace[] */
  u64 szScratch;               /* Size of scratch memory requested */
  Mem_ورقہ *apOld[NB];          /* pPage and up to two siblings */
  Mem_ورقہ *apNew[NB+2];        /* pPage and up to NB siblings after balancing */
  u8 *p_دایاں;                  /* Location in parent of right-sibling pointer */
  u8 *apDiv[NB-1];             /* Divider cells in pParent */
  صحيح cntNew[NB+2];            /* Index in b.paCell[] of cell after i-th page */
  صحيح cntOld[NB+2];            /* Old index in b.apCell[] */
  صحيح szNew[NB+2];             /* Combined size of cells placed on i-th page */
  u8 *aSpace1;                 /* Space for copies of dividers cells */
  Pgno pgno;                   /* Temp var to store a page number in */
  u8 ab_پورا[NB+2];             /* True after i'th new page is populated */
  Pgno aPgno[NB+2];            /* Page numbers of new pages before shuffling */
  CellArray b;                 /* Parsed information on cells being balanced */

  memset(ab_پورا, 0, حجم(ab_پورا));
  أكد( حجم(b) - حجم(b.ixNx) == offsetof(CellArray,ixNx) );
  memset(&b, 0, حجم(b)-حجم(b.ixNx[0]));
  b.ixNx[NB*2-1] = 0x7fffffff;
  pBt = p_ماپا->pBt;
  أكد( sqlite3_mutex_held(pBt->mutex) );
  أكد( sqlite3PagerIswriteable(p_ماپا->p_Db_ورقہ) );

  /* At this point pParent may have at most one overflow cell. And if
  ** this overflow cell is present, it must be the cell with
  ** index iParentIdx. This scenario comes about when this function
  ** is called (indirectly) from sqlite3BtreeDelete().
  */
  أكد( p_ماپا->n_حد_توں_ودھ==0 || p_ماپا->n_حد_توں_ودھ==1 );
  أكد( p_ماپا->n_حد_توں_ودھ==0 || p_ماپا->aiOvfl[0]==i_ماپا_Idx );

  إذا( !aOvflSpace ){
    أرجع SQLITE_NOMEM_BKPT;
  }
  أكد( p_ماپا->nFree>=0 );

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
  i = p_ماپا->n_حد_توں_ودھ + p_ماپا->nCell;
  إذا( i<2 ){
    nxDiv = 0;
  }وإلا{
    أكد( bBulk==0 || bBulk==1 );
    إذا( i_ماپا_Idx==0 ){                
      nxDiv = 0;
    }وإلا إذا( i_ماپا_Idx==i ){
      nxDiv = i-2+bBulk;
    }وإلا{
      nxDiv = i_ماپا_Idx-1;
    }
    i = 2-bBulk;
  }
  nOld = i+1;
  إذا( (i+nxDiv-p_ماپا->n_حد_توں_ودھ)==p_ماپا->nCell ){
    p_دایاں = &p_ماپا->a_انکڑے[p_ماپا->hdr_وتھ+8];
  }وإلا{
    p_دایاں = لبھو_Cell(p_ماپا, i+nxDiv-p_ماپا->n_حد_توں_ودھ);
  }
  pgno = get4byte(p_دایاں);
  طالما( 1 ){
    إذا( rc==SQLITE_OK ){
      rc = get_And_Init_ورقہ(pBt, pgno, &apOld[i], 0);
    }
    إذا( rc ){
      memset(apOld, 0, (i+1)*حجم(Mem_ورقہ*));
      اذهب balance_cleanup;
    }
    إذا( apOld[i]->nFree<0 ){
      rc = btreeComputeFreeSpace(apOld[i]);
      إذا( rc ){
        memset(apOld, 0, (i)*حجم(Mem_ورقہ*));
        اذهب balance_cleanup;
      }
    }
    nMaxCells += apOld[i]->nCell + Array_ناپ(p_ماپا->apOvfl);
    إذا( (i--)==0 ) اكسر;

    إذا( p_ماپا->n_حد_توں_ودھ && i+nxDiv==p_ماپا->aiOvfl[0] ){
      apDiv[i] = p_ماپا->apOvfl[0];
      pgno = get4byte(apDiv[i]);
      szNew[i] = p_ماپا->x_Cell_ناپ(p_ماپا, apDiv[i]);
      p_ماپا->n_حد_توں_ودھ = 0;
    }وإلا{
      apDiv[i] = لبھو_Cell(p_ماپا, i+nxDiv-p_ماپا->n_حد_توں_ودھ);
      pgno = get4byte(apDiv[i]);
      szNew[i] = p_ماپا->x_Cell_ناپ(p_ماپا, apDiv[i]);

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
        iOff = SQLITE_PTR_منزل_INT(apDiv[i]) - SQLITE_PTR_منزل_INT(p_ماپا->a_انکڑے);
        إذا( (iOff+szNew[i])<=(صحيح)pBt->usable_ناپ ){
          حافظہ_نقل(&aOvflSpace[iOff], apDiv[i], szNew[i]);
          apDiv[i] = &aOvflSpace[apDiv[i]-p_ماپا->a_انکڑے];
        }
      }
      dropCell(p_ماپا, i+nxDiv-p_ماپا->n_حد_توں_ودھ, szNew[i], &rc);
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
     + pBt->ورقہ_ناپ;                              /* aSpace1 */

  أكد( szScratch<=7*(صحيح)pBt->ورقہ_ناپ );
  b.apCell = sqlite3_ڈھیر_Alloc_Raw(0, szScratch );
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
  پتی_Correction = b.pRef->پتی*4;
  پتی_انکڑے = b.pRef->int_کلید_پتی;
  لكل(i=0; i<nOld; i++){
    Mem_ورقہ *pOld = apOld[i];
    صحيح حد_بندی = pOld->nCell;
    u8 *a_انکڑے = pOld->a_انکڑے;
    u16 نقاب_ورقہ = pOld->نقاب_ورقہ;
    u8 *piCell = a_انکڑے + pOld->cell_وتھ;
    u8 *pi_سرا;
    u8 **ppCell;
    VVA_ONLY( صحيح n_Cell_At_شروع = b.nCell; )

    /* Verify that all sibling pages are of the same "type" (table-leaf,
    ** table-interior, index-leaf, or index-interior).
    */
    إذا( pOld->a_انکڑے[0]!=apOld[0]->a_انکڑے[0] ){
      rc = SQLITE_CORRUPT_ورقہ(pOld);
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
    memset(&b.szCell[b.nCell], 0, حجم(b.szCell[0])*(حد_بندی+pOld->n_حد_توں_ودھ));
    إذا( pOld->n_حد_توں_ودھ>0 ){
      إذا( NEVER(حد_بندی<pOld->aiOvfl[0]) ){
        rc = SQLITE_CORRUPT_ورقہ(pOld);
        اذهب balance_cleanup;
      }
      حد_بندی = pOld->aiOvfl[0];
      لكل(j=0; j<حد_بندی; j++){
        b.apCell[b.nCell+j] = a_انکڑے + (نقاب_ورقہ & get2byteAligned(piCell));
        piCell += 2;
      }
      b.nCell += حد_بندی;
      لكل(k=0; k<pOld->n_حد_توں_ودھ; k++){
        أكد( k==0 || pOld->aiOvfl[k-1]+1==pOld->aiOvfl[k] );/* NOTE 1 */
        b.apCell[b.nCell] = pOld->apOvfl[k];
        b.nCell++;
      }
    }
    pi_سرا = a_انکڑے + pOld->cell_وتھ + 2*pOld->nCell;
    /* Collect cell pointers before updating the cell count. */
    ppCell = &b.apCell[b.nCell];
    طالما( piCell<pi_سرا ){
      أكد( ppCell<&b.apCell[nMaxCells] );
      *ppCell++ = a_انکڑے + (نقاب_ورقہ & get2byteAligned(piCell));
      piCell += 2;
    }
    b.nCell = (صحيح)(ppCell - b.apCell);
    أكد( (b.nCell-n_Cell_At_شروع)==(pOld->nCell+pOld->n_حد_توں_ودھ) );

    cntOld[i] = b.nCell;
    إذا( i<nOld-1 && !پتی_انکڑے){
      u16 sz = (u16)szNew[i];
      u8 *p_عارضی;
      أكد( b.nCell<nMaxCells );
      b.szCell[b.nCell] = sz;
      p_عارضی = &aSpace1[iSpace1];
      iSpace1 += sz;
      أكد( sz<=pBt->maxLocal+23 );
      أكد( iSpace1 <= (صحيح)pBt->ورقہ_ناپ );
      حافظہ_نقل(p_عارضی, apDiv[i], sz);
      b.apCell[b.nCell] = p_عارضی+پتی_Correction;
      أكد( پتی_Correction==0 || پتی_Correction==4 );
      b.szCell[b.nCell] = b.szCell[b.nCell] - پتی_Correction;
      إذا( !pOld->پتی ){
        أكد( پتی_Correction==0 );
        أكد( pOld->hdr_وتھ==0 || CORRUPT_DB );
        /* The right pointer of the child page pOld becomes the left
        ** pointer of the divider cell */
        حافظہ_نقل(b.apCell[b.nCell], &pOld->a_انکڑے[8], 4);
      }وإلا{
        أكد( پتی_Correction==4 );
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
  usableSpace = pBt->usable_ناپ - 12 + پتی_Correction;
  لكل(i=k=0; i<nOld; i++, k++){
    Mem_ورقہ *p = apOld[i];
    b.ap_سرا[k] = p->a_انکڑے_سرا;
    b.ixNx[k] = cntOld[i];
    إذا( k && b.ixNx[k]==b.ixNx[k-1] ){
      k--;  /* Omit b.ixNx[] entry for child pages with no cells */
    }
    إذا( !پتی_انکڑے ){
      k++;
      b.ap_سرا[k] = p_ماپا->a_انکڑے_سرا;
      b.ixNx[k] = cntOld[i]+1;
    }
    أكد( p->nFree>=0 );
    szNew[i] = usableSpace - p->nFree;
    لكل(j=0; j<p->n_حد_توں_ودھ; j++){
      szNew[i] += 2 + p->x_Cell_ناپ(p, p->apOvfl[j]);
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
      sz = 2 + cached_Cell_ناپ(&b, cntNew[i]-1);
      szNew[i] -= sz;
      إذا( !پتی_انکڑے ){
        إذا( cntNew[i]<b.nCell ){
          sz = 2 + cached_Cell_ناپ(&b, cntNew[i]);
        }وإلا{
          sz = 0;
        }
      }
      szNew[i+1] += sz;
      cntNew[i]--;
    }
    طالما( cntNew[i]<b.nCell ){
      sz = 2 + cached_Cell_ناپ(&b, cntNew[i]);
      إذا( szNew[i]+sz>usableSpace ) اكسر;
      szNew[i] += sz;
      cntNew[i]++;
      إذا( !پتی_انکڑے ){
        إذا( cntNew[i]<b.nCell ){
          sz = 2 + cached_Cell_ناپ(&b, cntNew[i]);
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
    صحيح sz_دایاں = szNew[i];  /* Size of sibling on the right */
    صحيح sz_بایاں = szNew[i-1]; /* Size of sibling on the left */
    صحيح r;              /* Index of right-most cell in left sibling */
    صحيح d;              /* Index of first cell to the left of right sibling */

    r = cntNew[i-1] - 1;
    d = r + 1 - پتی_انکڑے;
    (فراغ)cached_Cell_ناپ(&b, d);
    افعل{
      صحيح szR, szD;
      أكد( d<nMaxCells );
      أكد( r<nMaxCells );
      szR = cached_Cell_ناپ(&b, r);
      szD = b.szCell[d];
      إذا( sz_دایاں!=0
       && (bBulk || sz_دایاں+szD+2 > sz_بایاں-(szR+(i==k-1?0:2)))){
        اكسر;
      }
      sz_دایاں += szD + 2;
      sz_بایاں -= szR + 2;
      cntNew[i-1] = r;
      r--;
      d--;
    }طالما( r>=0 );
    szNew[i] = sz_دایاں;
    szNew[i-1] = sz_بایاں;
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
  أكد( cntNew[0]>0 || (p_ماپا->pgno==1 && p_ماپا->nCell==0) || CORRUPT_DB);
  TRACE(("BALANCE: old: %u(nc=%u) %u(nc=%u) %u(nc=%u)\n",
    apOld[0]->pgno, apOld[0]->nCell,
    nOld>=2 ? apOld[1]->pgno : 0, nOld>=2 ? apOld[1]->nCell : 0,
    nOld>=3 ? apOld[2]->pgno : 0, nOld>=3 ? apOld[2]->nCell : 0
  ));

  /*
  ** Allocate k new pages.  Reuse old pages where possible.
  */
  ورقہ_Flags = apOld[0]->a_انکڑے[0];
  لكل(i=0; i<k; i++){
    Mem_ورقہ *pNew;
    إذا( i<nOld ){
      pNew = apNew[i] = apOld[i];
      apOld[i] = 0;
      rc = sqlite3PagerWrite(pNew->p_Db_ورقہ);
      nNew++;
      إذا( sqlite3_Pager_ورقہ_Refcount(pNew->p_Db_ورقہ)!=1+(i==(i_ماپا_Idx-nxDiv))
       && rc==SQLITE_OK
      ){
        rc = SQLITE_CORRUPT_BKPT;
      }
      إذا( rc ) اذهب balance_cleanup;
    }وإلا{
      أكد( i>0 );
      rc = allocate_Btree_ورقہ(pBt, &pNew, &pgno, (bBulk ? 1 : pgno), 0);
      إذا( rc ) اذهب balance_cleanup;
      zero_ورقہ(pNew, ورقہ_Flags);
      apNew[i] = pNew;
      nNew++;
      cntOld[i] = b.nCell;

      /* Set the pointer-map entry for the new sibling page. */
      إذا( ISAUTOVACUUM(pBt) ){
        ptrmapPut(pBt, pNew->pgno, PTRMAP_BTREE, p_ماپا->pgno, &rc);
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
    أكد( apNew[i]->p_Db_ورقہ->flags & PGHDR_WRITEABLE );
    أكد( apNew[i]->p_Db_ورقہ->flags & PGHDR_DIRTY );
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
      Pgno pgno_عارضی = (PENDING_BYTE/pBt->ورقہ_ناپ)+1;
      u16 fgA = apNew[i]->p_Db_ورقہ->flags;
      u16 fgB = apNew[iB]->p_Db_ورقہ->flags;
      sqlite3PagerRekey(apNew[i]->p_Db_ورقہ, pgno_عارضی, fgB);
      sqlite3PagerRekey(apNew[iB]->p_Db_ورقہ, pgnoA, fgA);
      sqlite3PagerRekey(apNew[i]->p_Db_ورقہ, pgnoB, fgB);
      apNew[i]->pgno = pgnoB;
      apNew[iB]->pgno = pgnoA;
    }
  }

  TRACE(("BALANCE: new: %u(%u nc=%u) %u(%u nc=%u) %u(%u nc=%u) "
         "%u(%u nc=%u) %u(%u nc=%u)\n",
    apNew[0]->pgno, szNew[0], cntNew[0],
    nNew>=2 ? apNew[1]->pgno : 0, nNew>=2 ? szNew[1] : 0,
    nNew>=2 ? cntNew[1] - cntNew[0] - !پتی_انکڑے : 0,
    nNew>=3 ? apNew[2]->pgno : 0, nNew>=3 ? szNew[2] : 0,
    nNew>=3 ? cntNew[2] - cntNew[1] - !پتی_انکڑے : 0,
    nNew>=4 ? apNew[3]->pgno : 0, nNew>=4 ? szNew[3] : 0,
    nNew>=4 ? cntNew[3] - cntNew[2] - !پتی_انکڑے : 0,
    nNew>=5 ? apNew[4]->pgno : 0, nNew>=5 ? szNew[4] : 0,
    nNew>=5 ? cntNew[4] - cntNew[3] - !پتی_انکڑے : 0
  ));

  أكد( sqlite3PagerIswriteable(p_ماپا->p_Db_ورقہ) );
  أكد( nNew>=1 && nNew<=Array_ناپ(apNew) );
  أكد( apNew[nNew-1]!=0 );
  put4byte(p_دایاں, apNew[nNew-1]->pgno);

  /* If the sibling pages are not leaves, ensure that the right-child pointer
  ** of the right-most new sibling page is set to the value that was
  ** originally in the same field of the right-most old sibling page. */
  إذا( (ورقہ_Flags & PTF_پتی)==0 && nOld!=nNew ){
    Mem_ورقہ *pOld;
    إذا( nNew>nOld ){
      pOld = apNew[nOld-1];
    }وإلا{
      pOld = apOld[nOld-1];
    }
    حافظہ_نقل(&apNew[nNew-1]->a_انکڑے[8], &pOld->a_انکڑے[8], 4);
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
    Mem_ورقہ *pOld;
    Mem_ورقہ *pNew = pOld = apNew[0];
    صحيح cnt_Old_اگلا = pNew->nCell + pNew->n_حد_توں_ودھ;
    صحيح iNew = 0;
    صحيح iOld = 0;

    لكل(i=0; i<b.nCell; i++){
      u8 *pCell = b.apCell[i];
      طالما( i==cnt_Old_اگلا ){
        iOld++;
        أكد( iOld<nNew || iOld<nOld );
        أكد( iOld>=0 && iOld<NB );
        pOld = iOld<nNew ? apNew[iOld] : apOld[iOld];
        cnt_Old_اگلا += pOld->nCell + pOld->n_حد_توں_ودھ + !پتی_انکڑے;
      }
      إذا( i==cntNew[iNew] ){
        pNew = apNew[++iNew];
        إذا( !پتی_انکڑے ) استمر;
      }

      /* Cell pCell is destined for new sibling page pNew. Originally, it
      ** was either part of sibling page iOld (possibly an overflow cell),
      ** or else the divider cell to the left of sibling page iOld. So,
      ** if sibling page iOld had the same page number as pNew, and if
      ** pCell really was a part of sibling page iOld (not a divider or
      ** overflow cell), we can skip updating the pointer map entries.  */
      إذا( iOld>=nNew
       || pNew->pgno!=aPgno[iOld]
       || !SQLITE_WITHIN(pCell,pOld->a_انکڑے,pOld->a_انکڑے_سرا)
      ){
        إذا( !پتی_Correction ){
          ptrmapPut(pBt, get4byte(pCell), PTRMAP_BTREE, pNew->pgno, &rc);
        }
        إذا( cached_Cell_ناپ(&b,i)>pNew->minLocal ){
          ptrmapPutOvflPtr(pNew, pOld, pCell, &rc);
        }
        إذا( rc ) اذهب balance_cleanup;
      }
    }
  }

  /* Insert new divider cells into pParent. */
  لكل(i=0; i<nNew-1; i++){
    u8 *pCell;
    u8 *p_عارضی;
    صحيح sz;
    u8 *p_Src_سرا;
    Mem_ورقہ *pNew = apNew[i];
    j = cntNew[i];

    أكد( j<nMaxCells );
    أكد( b.apCell[j]!=0 );
    pCell = b.apCell[j];
    sz = b.szCell[j] + پتی_Correction;
    p_عارضی = &aOvflSpace[iOvflSpace];
    إذا( !pNew->پتی ){
      حافظہ_نقل(&pNew->a_انکڑے[8], pCell, 4);
    }وإلا إذا( پتی_انکڑے ){
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
        أكد(پتی_Correction==4);
        sz = p_ماپا->x_Cell_ناپ(p_ماپا, pCell);
      }
    }
    iOvflSpace += sz;
    أكد( sz<=pBt->maxLocal+23 );
    أكد( iOvflSpace <= (صحيح)pBt->ورقہ_ناپ );
    أكد( b.ixNx[NB*2-1]>j );
    لكل(k=0; b.ixNx[k]<=j; k++){}
    p_Src_سرا = b.ap_سرا[k];
    إذا( SQLITE_حد_توں_ودھ(p_Src_سرا, pCell, pCell+sz) ){
      rc = SQLITE_CORRUPT_BKPT;
      اذهب balance_cleanup;
    }
    rc = داخل_کرو_Cell(p_ماپا, nxDiv+i, pCell, sz, p_عارضی, pNew->pgno);
    إذا( rc!=SQLITE_OK ) اذهب balance_cleanup;
    أكد( sqlite3PagerIswriteable(p_ماپا->p_Db_ورقہ) );
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
    أكد( iPg<Array_ناپ(cntOld) );
    إذا( ab_پورا[iPg] ) استمر;         /* Skip pages already processed */
    إذا( i>=0                            /* On the upwards pass, or... */
     || cntOld[iPg-1]>=cntNew[iPg-1]    /* Condition (1) is true */
    ){
      صحيح iNew;
      صحيح iOld;
      صحيح nNewCell;

      /* Verify condition (1):  If cells are moving left, update iPg
      ** only after iPg-1 has already been updated. */
      أكد( iPg==0 || cntOld[iPg-1]>=cntNew[iPg-1] || ab_پورا[iPg-1] );

      /* Verify condition (2):  If cells are moving right, update iPg
      ** only after iPg+1 has already been updated. */
      أكد( cntNew[iPg]>=cntOld[iPg] || ab_پورا[iPg+1] );

      إذا( iPg==0 ){
        iNew = iOld = 0;
        nNewCell = cntNew[0];
      }وإلا{
        iOld = iPg<nOld ? (cntOld[iPg-1] + !پتی_انکڑے) : b.nCell;
        iNew = cntNew[iPg-1] + !پتی_انکڑے;
        nNewCell = cntNew[iPg] - iNew;
      }

      rc = edit_ورقہ(apNew[iPg], iOld, iNew, nNewCell, &b);
      إذا( rc ) اذهب balance_cleanup;
      ab_پورا[iPg]++;
      apNew[iPg]->nFree = usableSpace-szNew[iPg];
      أكد( apNew[iPg]->n_حد_توں_ودھ==0 );
      أكد( apNew[iPg]->nCell==nNewCell );
    }
  }

  /* All pages have been processed exactly once */
  أكد( memcmp(ab_پورا, "\01\01\01\01\01", nNew)==0 );

  أكد( nOld>0 );
  أكد( nNew>0 );

  إذا( is_جڑ && p_ماپا->nCell==0 && p_ماپا->hdr_وتھ<=apNew[0]->nFree ){
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
    rc = defragment_ورقہ(apNew[0], -1);
    testcase( rc!=SQLITE_OK );
    أكد( apNew[0]->nFree ==
        (get2byteNotZero(&apNew[0]->a_انکڑے[5]) - apNew[0]->cell_وتھ
          - apNew[0]->nCell*2)
      || rc!=SQLITE_OK
    );
    copy_گرہ_Content(apNew[0], p_ماپا, &rc);
    free_ورقہ(apNew[0], &rc);
  }وإلا إذا( ISAUTOVACUUM(pBt) && !پتی_Correction ){
    /* Fix the pointer map entries associated with the right-child of each
    ** sibling page. All other pointer map entries have already been taken
    ** care of.  */
    لكل(i=0; i<nNew; i++){
      u32 کلید = get4byte(&apNew[i]->a_انکڑے[8]);
      ptrmapPut(pBt, کلید, PTRMAP_BTREE, apNew[i]->pgno, &rc);
    }
  }

  أكد( p_ماپا->isInit );
  TRACE(("BALANCE: finished: old=%u new=%u cells=%u\n",
          nOld, nNew, b.nCell));

  /* Free any old pages that were not reused as new pages.
  */
  لكل(i=nNew; i<nOld; i++){
    free_ورقہ(apOld[i], &rc);
  }

#إذا 0
  إذا( ISAUTOVACUUM(pBt) && rc==SQLITE_OK && apNew[0]->isInit ){
    /* The ptrmapCheckPages() contains assert() statements that verify that
    ** all pointer map pages are set correctly. This is helpful while
    ** debugging. This is usually disabled because a corrupt database may
    ** cause an assert() statement to fail.  */
    ptrmap_Check_ورقے(apNew, nNew);
    ptrmap_Check_ورقے(&p_ماپا, 1);
  }
#جے_انت

  /*
  ** Cleanup before returning.
  */
balance_cleanup:
  sqlite3_ڈھیر_Free(0, b.apCell);
  لكل(i=0; i<nOld; i++){
    release_ورقہ(apOld[i]);
  }
  لكل(i=0; i<nNew; i++){
    release_ورقہ(apNew[i]);
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
ساكن صحيح balance_deeper(Mem_ورقہ *p_جڑ, Mem_ورقہ **pp_بچہ){
  صحيح rc;                        /* Return value from subprocedures */
  Mem_ورقہ *p_بچہ = 0;           /* Pointer to a new child page */
  Pgno pgno_بچہ = 0;            /* Page number of the new child page */
  BtShared *pBt = p_جڑ->pBt;    /* The BTree */

  أكد( p_جڑ->n_حد_توں_ودھ>0 );
  أكد( sqlite3_mutex_held(pBt->mutex) );

  /* Make pRoot, the root page of the b-tree, writable. Allocate a new
  ** page that will become the new right-child of pPage. Copy the contents
  ** of the node stored on pRoot into the new child page.
  */
  rc = sqlite3PagerWrite(p_جڑ->p_Db_ورقہ);
  إذا( rc==SQLITE_OK ){
    rc = allocate_Btree_ورقہ(pBt,&p_بچہ,&pgno_بچہ,p_جڑ->pgno,0);
    copy_گرہ_Content(p_جڑ, p_بچہ, &rc);
    إذا( ISAUTOVACUUM(pBt) ){
      ptrmapPut(pBt, pgno_بچہ, PTRMAP_BTREE, p_جڑ->pgno, &rc);
    }
  }
  إذا( rc ){
    *pp_بچہ = 0;
    release_ورقہ(p_بچہ);
    أرجع rc;
  }
  أكد( sqlite3PagerIswriteable(p_بچہ->p_Db_ورقہ) );
  أكد( sqlite3PagerIswriteable(p_جڑ->p_Db_ورقہ) );
  أكد( p_بچہ->nCell==p_جڑ->nCell || CORRUPT_DB );

  TRACE(("BALANCE: copy root %u into %u\n", p_جڑ->pgno, p_بچہ->pgno));

  /* Copy the overflow cells from pRoot to pChild */
  حافظہ_نقل(p_بچہ->aiOvfl, p_جڑ->aiOvfl,
         p_جڑ->n_حد_توں_ودھ*حجم(p_جڑ->aiOvfl[0]));
  حافظہ_نقل(p_بچہ->apOvfl, p_جڑ->apOvfl,
         p_جڑ->n_حد_توں_ودھ*حجم(p_جڑ->apOvfl[0]));
  p_بچہ->n_حد_توں_ودھ = p_جڑ->n_حد_توں_ودھ;

  /* Zero the contents of pRoot. Then install pChild as the right-child. */
  zero_ورقہ(p_جڑ, p_بچہ->a_انکڑے[0] & ~PTF_پتی);
  put4byte(&p_جڑ->a_انکڑے[p_جڑ->hdr_وتھ+8], pgno_بچہ);

  *pp_بچہ = p_بچہ;
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
  لكل(pOther=pCur->pBt->pCursor; pOther; pOther=pOther->p_اگلا){
    إذا( pOther!=pCur
     && pOther->eState==CURSOR_VALID
     && pOther->p_ورقہ==pCur->p_ورقہ
    ){
      أرجع SQLITE_CORRUPT_ورقہ(pCur->p_ورقہ);
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
  u8 a_Balance_تیز_Space[13];
  u8 *pFree = 0;

  VVA_ONLY( صحيح balance_تیز_called = 0 );
  VVA_ONLY( صحيح balance_deeper_called = 0 );

  افعل {
    صحيح i_ورقہ;
    Mem_ورقہ *p_ورقہ = pCur->p_ورقہ;

    إذا( NEVER(p_ورقہ->nFree<0) && btreeComputeFreeSpace(p_ورقہ) ) اكسر;
    إذا( p_ورقہ->n_حد_توں_ودھ==0 && p_ورقہ->nFree*3<=(صحيح)pCur->pBt->usable_ناپ*2 ){
      /* No rebalance required as long as:
      **   (1) There are no overflow cells
      **   (2) The amount of free space on the page is less than 2/3rds of
      **       the total usable space on the page. */
      اكسر;
    }وإلا إذا( (i_ورقہ = pCur->i_ورقہ)==0 ){
      إذا( p_ورقہ->n_حد_توں_ودھ && (rc = anotherValidCursor(pCur))==SQLITE_OK ){
        /* The root page of the b-tree is overfull. In this case call the
        ** balance_deeper() function to create a new child for the root-page
        ** and copy the current contents of the root-page to it. The
        ** next iteration of the do-loop will balance the child page.
        */
        أكد( balance_deeper_called==0 );
        VVA_ONLY( balance_deeper_called++ );
        rc = balance_deeper(p_ورقہ, &pCur->ap_ورقہ[1]);
        إذا( rc==SQLITE_OK ){
          pCur->i_ورقہ = 1;
          pCur->ix = 0;
          pCur->aiIdx[0] = 0;
          pCur->ap_ورقہ[0] = p_ورقہ;
          pCur->p_ورقہ = pCur->ap_ورقہ[1];
          أكد( pCur->p_ورقہ->n_حد_توں_ودھ );
        }
      }وإلا{
        اكسر;
      }
    }وإلا إذا( sqlite3_Pager_ورقہ_Refcount(p_ورقہ->p_Db_ورقہ)>1 ){
      /* The page being written is not a root page, and there is currently
      ** more than one reference to it. This only happens if the page is one
      ** of its own ancestor pages. Corruption. */
      rc = SQLITE_CORRUPT_ورقہ(p_ورقہ);
    }وإلا{
      Mem_ورقہ * ثابت p_ماپا = pCur->ap_ورقہ[i_ورقہ-1];
      صحيح ثابت iIdx = pCur->aiIdx[i_ورقہ-1];

      rc = sqlite3PagerWrite(p_ماپا->p_Db_ورقہ);
      إذا( rc==SQLITE_OK && p_ماپا->nFree<0 ){
        rc = btreeComputeFreeSpace(p_ماپا);
      }
      إذا( rc==SQLITE_OK ){
#جے_بنا_تعریف SQLITE_OMIT_QUICKBALANCE
        إذا( p_ورقہ->int_کلید_پتی
         && p_ورقہ->n_حد_توں_ودھ==1
         && p_ورقہ->aiOvfl[0]==p_ورقہ->nCell
         && p_ماپا->pgno!=1
         && p_ماپا->nCell==iIdx
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
          rc = balance_تیز(p_ماپا, p_ورقہ, a_Balance_تیز_Space);
        }وإلا
#جے_انت
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
          u8 *pSpace = sqlite3_ورقہ_Malloc(pCur->pBt->ورقہ_ناپ);
          rc = balance_nonroot(p_ماپا, iIdx, pSpace, i_ورقہ==1,
                               pCur->hints&BTREE_BULKLOAD);
          إذا( pFree ){
            /* If pFree is not NULL, it points to the pSpace buffer used
            ** by a previous call to balance_nonroot(). Its contents are
            ** now stored either on real database pages or within the
            ** new pSpace buffer, so it may be safely freed here. */
            sqlite3_ورقہ_Free(pFree);
          }

          /* The pSpace buffer will be freed after the next call to
          ** balance_nonroot(), or just before this function returns, whichever
          ** comes first. */
          pFree = pSpace;
        }
      }

      p_ورقہ->n_حد_توں_ودھ = 0;

      /* The next iteration of the do-loop balances the parent page. */
      release_ورقہ(p_ورقہ);
      pCur->i_ورقہ--;
      أكد( pCur->i_ورقہ>=0 );
      pCur->p_ورقہ = pCur->ap_ورقہ[pCur->i_ورقہ];
    }
  }طالما( rc==SQLITE_OK );

  إذا( pFree ){
    sqlite3_ورقہ_Free(pFree);
  }
  أرجع rc;
}

/* Overwrite content from pX into pDest.  Only do the write if the
** content is different from what is already there.
*/
ساكن صحيح btreeOverwriteContent(
  Mem_ورقہ *p_ورقہ,           /* MemPage on which writing will occur */
  u8 *pDest,                /* Pointer to the place to start writing */
  ثابت BtreePayload *pX,   /* Source of data to write */
  صحيح i_وتھ,              /* Offset of first byte to write */
  صحيح iAmt                  /* Number of bytes to be written */
){
  صحيح n_انکڑے = pX->n_انکڑے - i_وتھ;
  إذا( n_انکڑے<=0 ){
    /* Overwriting with zeros */
    صحيح i;
    لكل(i=0; i<iAmt && pDest[i]==0; i++){}
    إذا( i<iAmt ){
      صحيح rc = sqlite3PagerWrite(p_ورقہ->p_Db_ورقہ);
      إذا( rc ) أرجع rc;
      memset(pDest + i, 0, iAmt - i);
    }
  }وإلا{
    إذا( n_انکڑے<iAmt ){
      /* Mixed read data and zeros at the end.  Make a recursive call
      ** to write the zeros then fall through to write the real data */
      صحيح rc = btreeOverwriteContent(p_ورقہ, pDest+n_انکڑے, pX, i_وتھ+n_انکڑے,
                                 iAmt-n_انکڑے);
      إذا( rc ) أرجع rc;
      iAmt = n_انکڑے;
    }
    إذا( memcmp(pDest, ((u8*)pX->p_انکڑے) + i_وتھ, iAmt)!=0 ){
      صحيح rc = sqlite3PagerWrite(p_ورقہ->p_Db_ورقہ);
      إذا( rc ) أرجع rc;
      /* In a corrupt database, it is possible for the source and destination
      ** buffers to overlap.  This is harmless since the database is already
      ** corrupt but it does cause valgrind and ASAN warnings.  So use
      ** memmove(). */
      memmove(pDest, ((u8*)pX->p_انکڑے) + i_وتھ, iAmt);
    }
  }
  أرجع SQLITE_OK;
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.  In this variant, pCur is pointing to an overflow
** cell.
*/
ساكن SQLITE_NOINLINE صحيح btree_Overwrite_حد_توں_ودھ_Cell(
  BtCursor *pCur,                     /* Cursor pointing to cell to overwrite */
  ثابت BtreePayload *pX              /* Content to write into the cell */
){
  صحيح i_وتھ;                        /* Next byte of pX->pData to write */
  صحيح n_کل = pX->n_انکڑے + pX->nZero; /* Total bytes of to write */
  صحيح rc;                             /* Return code */
  Mem_ورقہ *p_ورقہ = pCur->p_ورقہ;       /* Page being written */
  BtShared *pBt;                      /* Btree */
  Pgno ovflPgno;                      /* Next overflow page to write */
  u32 ovfl_ورقہ_ناپ;                   /* Size to write on overflow page */

  أكد( pCur->info.nLocal<n_کل );  /* pCur is an overflow cell */

  /* Overwrite the local portion first */
  rc = btreeOverwriteContent(p_ورقہ, pCur->info.pPayload, pX,
                             0, pCur->info.nLocal);
  إذا( rc ) أرجع rc;

  /* Now overwrite the overflow pages */
  i_وتھ = pCur->info.nLocal;
  أكد( n_کل>=0 );
  أكد( i_وتھ>=0 );
  ovflPgno = get4byte(pCur->info.pPayload + i_وتھ);
  pBt = p_ورقہ->pBt;
  ovfl_ورقہ_ناپ = pBt->usable_ناپ - 4;
  افعل{
    rc = btree_Get_ورقہ(pBt, ovflPgno, &p_ورقہ, 0);
    إذا( rc ) أرجع rc;
    إذا( sqlite3_Pager_ورقہ_Refcount(p_ورقہ->p_Db_ورقہ)!=1 || p_ورقہ->isInit ){
      rc = SQLITE_CORRUPT_ورقہ(p_ورقہ);
    }وإلا{
      إذا( i_وتھ+ovfl_ورقہ_ناپ<(u32)n_کل ){
        ovflPgno = get4byte(p_ورقہ->a_انکڑے);
      }وإلا{
        ovfl_ورقہ_ناپ = n_کل - i_وتھ;
      }
      rc = btreeOverwriteContent(p_ورقہ, p_ورقہ->a_انکڑے+4, pX,
                                 i_وتھ, ovfl_ورقہ_ناپ);
    }
    sqlite3PagerUnref(p_ورقہ->p_Db_ورقہ);
    إذا( rc ) أرجع rc;
    i_وتھ += ovfl_ورقہ_ناپ;
  }طالما( i_وتھ<n_کل );
  أرجع SQLITE_OK;   
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.
*/
ساكن صحيح btreeOverwriteCell(BtCursor *pCur, ثابت BtreePayload *pX){
  صحيح n_کل = pX->n_انکڑے + pX->nZero; /* Total bytes of to write */
  Mem_ورقہ *p_ورقہ = pCur->p_ورقہ;       /* Page being written */

  إذا( pCur->info.pPayload + pCur->info.nLocal > p_ورقہ->a_انکڑے_سرا
   || pCur->info.pPayload < p_ورقہ->a_انکڑے + p_ورقہ->cell_وتھ
  ){
    أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
  }
  إذا( pCur->info.nLocal==n_کل ){
    /* The entire cell is local */
    أرجع btreeOverwriteContent(p_ورقہ, pCur->info.pPayload, pX,
                                 0, pCur->info.nLocal);
  }وإلا{
    /* The cell contains overflow content */
    أرجع btree_Overwrite_حد_توں_ودھ_Cell(pCur, pX);
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
صحيح sqlite3_Btree_داخل_کرو(
  BtCursor *pCur,                /* Insert data into the table of this cursor */
  ثابت BtreePayload *pX,        /* Content of the row to be inserted */
  صحيح flags,                     /* True if this is likely an append */
  صحيح seek_نتیجہ                 /* Result of prior IndexMoveto() call */
){
  صحيح rc;
  صحيح loc = seek_نتیجہ;          /* -1: before desired location  +1: after */
  صحيح szNew = 0;
  صحيح idx;
  Mem_ورقہ *p_ورقہ;
  Btree *p = pCur->pBtree;
  بنانشانی حرف *oldCell;
  بنانشانی حرف *newCell = 0;

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
  إذا( pCur->curFlags & BTCF_مضرب ){
    rc = saveAllCursors(p->pBt, pCur->pgno_جڑ, pCur);
    إذا( rc ) أرجع rc;
    إذا( loc && pCur->i_ورقہ<0 ){
      /* This can only happen if the schema is corrupt such that there is more
      ** than one table or index with the same root page as used by the cursor.
      ** Which can only happen if the SQLITE_NoSchemaError flag was set when
      ** the schema was loaded. This cannot be asserted though, as a user might
      ** set the flag, load the schema, and then unset the flag.  */
      أرجع SQLITE_CORRUPT_PGNO(pCur->pgno_جڑ);
    }
  }

  /* Ensure that the cursor is not in the CURSOR_FAULT state and that it
  ** points to a valid cell.
  */
  إذا( pCur->eState>=CURSOR_REQUIRESEEK ){
    testcase( pCur->eState==CURSOR_REQUIRESEEK );
    testcase( pCur->eState==CURSOR_FAULT );
    rc = move_منزل_جڑ(pCur);
    إذا( rc && rc!=SQLITE_خالی_تھاواں ) أرجع rc;
  }

  أكد( cursorOwnsBtShared(pCur) );
  أكد( (pCur->curFlags & BTCF_WriteFlag)!=0
              && p->pBt->وچ_Transaction==TRANS_WRITE
              && (p->pBt->btsFlags & BTS_READ_ONLY)==0 );
  أكد( has_Shared_Cache_جدول_جندرا(p, pCur->pgno_جڑ, pCur->p_کلید_Info!=0, 2) );

  /* Assert that the caller has been consistent. If this cursor was opened
  ** expecting an index b-tree, then the caller should be inserting blob
  ** keys with no associated data. If the cursor was opened expecting an
  ** intkey table, the caller should be inserting integer keys with a
  ** blob of associated data.  */
  أكد( (flags & BTREE_PREFORMAT) || (pX->p_کلید==0)==(pCur->p_کلید_Info==0) );

  إذا( pCur->p_کلید_Info==0 ){
    أكد( pX->p_کلید==0 );
    /* If this is an insert into a table b-tree, invalidate any incrblob
    ** cursors open on the row being replaced */
    إذا( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, pCur->pgno_جڑ, pX->n_کلید, 0);
    }

    /* If BTREE_SAVEPOSITION is set, the cursor must already be pointing
    ** to a row with the same key as the new entry being inserted.
    */
#جے_تعریف_شدہ SQLITE_DEBUG
    إذا( flags & BTREE_SAVEPOSITION ){
      أكد( pCur->curFlags & BTCF_ValidNKey );
      أكد( pX->n_کلید==pCur->info.n_کلید );
      أكد( loc==0 );
    }
#جے_انت

    /* On the other hand, BTREE_SAVEPOSITION==0 does not imply
    ** that the cursor is not pointing to a row to be overwritten.
    ** So do a complete check.
    */
    إذا( (pCur->curFlags&BTCF_ValidNKey)!=0 && pX->n_کلید==pCur->info.n_کلید ){
      /* The cursor is pointing to the entry that is to be
      ** overwritten */
      أكد( pX->n_انکڑے>=0 && pX->nZero>=0 );
      إذا( pCur->info.n_ناپ!=0
       && pCur->info.nPayload==(u32)pX->n_انکڑے+pX->nZero
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
      rc = sqlite3_Btree_جدول_Moveto(pCur, pX->n_کلید,
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
        Unpacked_ریکارڈ r;
        r.p_کلید_Info = pCur->p_کلید_Info;
        r.aMem = pX->aMem;
        r.n_میدان = pX->nMem;
        r.default_rc = 0;
        r.eq_تکیا = 0;
        rc = sqlite3_Btree_اشاریہ_Moveto(pCur, &r, &loc);
      }وإلا{
        rc = btreeMoveto(pCur, pX->p_کلید, pX->n_کلید,
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
      إذا( pCur->info.n_کلید==pX->n_کلید ){
        BtreePayload x2;
        x2.p_انکڑے = pX->p_کلید;
        x2.n_انکڑے = (صحيح)pX->n_کلید;  أكد( pX->n_کلید<=0x7fffffff );
        x2.nZero = 0;
        أرجع btreeOverwriteCell(pCur, &x2);
      }
    }
  }
  أكد( pCur->eState==CURSOR_VALID
       || (pCur->eState==CURSOR_INVALID && loc) || CORRUPT_DB );

  p_ورقہ = pCur->p_ورقہ;
  أكد( p_ورقہ->int_کلید || pX->n_کلید>=0 || (flags & BTREE_PREFORMAT) );
  أكد( p_ورقہ->پتی || !p_ورقہ->int_کلید );
  إذا( p_ورقہ->nFree<0 ){
    إذا( NEVER(pCur->eState>CURSOR_INVALID) ){
     /* ^^^^^--- due to the moveToRoot() call above */
      rc = SQLITE_CORRUPT_ورقہ(p_ورقہ);
    }وإلا{
      rc = btreeComputeFreeSpace(p_ورقہ);
    }
    إذا( rc ) أرجع rc;
  }

  TRACE(("INSERT: table=%u nkey=%lld ndata=%u page=%u %s\n",
          pCur->pgno_جڑ, pX->n_کلید, pX->n_انکڑے, p_ورقہ->pgno,
          loc==0 ? "overwrite" : "new entry"));
  أكد( p_ورقہ->isInit || CORRUPT_DB );
  newCell = p->pBt->pTmpSpace;
  أكد( newCell!=0 );
  أكد( BTREE_PREFORMAT==OPFLAG_PREFORMAT );
  إذا( flags & BTREE_PREFORMAT ){
    rc = SQLITE_OK;
    szNew = p->pBt->n_Preformat_ناپ;
    إذا( szNew<4 ){
      szNew = 4;
      newCell[3] = 0;
    }
    إذا( ISAUTOVACUUM(p->pBt) && szNew>p_ورقہ->maxLocal ){
      CellInfo info;
      p_ورقہ->xParseCell(p_ورقہ, newCell, &info);
      إذا( info.nPayload!=info.nLocal ){
        Pgno ovfl = get4byte(&newCell[szNew-4]);
        ptrmapPut(p->pBt, ovfl, PTRMAP_OVERFLOW1, p_ورقہ->pgno, &rc);
        إذا( NEVER(rc) ) اذهب سرا_داخل_کرو;
      }
    }
  }وإلا{
    rc = fill_وچ_Cell(p_ورقہ, newCell, pX, &szNew);
    إذا( rc ) اذهب سرا_داخل_کرو;
  }
  أكد( szNew==p_ورقہ->x_Cell_ناپ(p_ورقہ, newCell) );
  أكد( szNew <= MX_CELL_ناپ(p->pBt) );
  idx = pCur->ix;
  pCur->info.n_ناپ = 0;
  إذا( loc==0 ){
    CellInfo info;
    أكد( idx>=0 );
    إذا( idx>=p_ورقہ->nCell ){
      أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
    }
    rc = sqlite3PagerWrite(p_ورقہ->p_Db_ورقہ);
    إذا( rc ){
      اذهب سرا_داخل_کرو;
    }
    oldCell = لبھو_Cell(p_ورقہ, idx);
    إذا( !p_ورقہ->پتی ){
      حافظہ_نقل(newCell, oldCell, 4);
    }
    BTREE_CLEAR_CELL(rc, p_ورقہ, oldCell, info);
    testcase( pCur->curFlags & BTCF_ValidOvfl );
    invalidate_حد_توں_ودھ_Cache(pCur);
    إذا( info.n_ناپ==szNew && info.nLocal==info.nPayload
     && (!ISAUTOVACUUM(p->pBt) || szNew<p_ورقہ->minLocal)
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
      إذا( oldCell < p_ورقہ->a_انکڑے+p_ورقہ->hdr_وتھ+10 ){
        أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
      }
      إذا( oldCell+szNew > p_ورقہ->a_انکڑے_سرا ){
        أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
      }
      حافظہ_نقل(oldCell, newCell, szNew);
      أرجع SQLITE_OK;
    }
    dropCell(p_ورقہ, idx, info.n_ناپ, &rc);
    إذا( rc ) اذهب سرا_داخل_کرو;
  }وإلا إذا( loc<0 && p_ورقہ->nCell>0 ){
    أكد( p_ورقہ->پتی );
    idx = ++pCur->ix;
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  }وإلا{
    أكد( p_ورقہ->پتی );
  }
  rc = داخل_کرو_Cell_Fast(p_ورقہ, idx, newCell, szNew);
  أكد( p_ورقہ->n_حد_توں_ودھ==0 || rc==SQLITE_OK );
  أكد( rc!=SQLITE_OK || p_ورقہ->nCell>0 || p_ورقہ->n_حد_توں_ودھ>0 );

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
  إذا( p_ورقہ->n_حد_توں_ودھ ){
    أكد( rc==SQLITE_OK );
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
    rc = balance(pCur);

    /* Must make sure nOverflow is reset to zero even if the balance()
    ** fails. Internal data structure corruption will result otherwise.
    ** Also, set the cursor state to invalid. This stops saveCursorPosition()
    ** from trying to save the current position of the cursor.  */
    pCur->p_ورقہ->n_حد_توں_ودھ = 0;
    pCur->eState = CURSOR_INVALID;
    إذا( (flags & BTREE_SAVEPOSITION) && rc==SQLITE_OK ){
      btree_Release_All_Cursor_ورقے(pCur);
      إذا( pCur->p_کلید_Info ){
        أكد( pCur->p_کلید==0 );
        pCur->p_کلید = sqlite3Malloc( pX->n_کلید );
        إذا( pCur->p_کلید==0 ){
          rc = SQLITE_NOMEM;
        }وإلا{
          حافظہ_نقل(pCur->p_کلید, pX->p_کلید, pX->n_کلید);
        }
      }
      pCur->eState = CURSOR_REQUIRESEEK;
      pCur->n_کلید = pX->n_کلید;
    }
  }
  أكد( pCur->i_ورقہ<0 || pCur->p_ورقہ->n_حد_توں_ودھ==0 );

سرا_داخل_کرو:
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
صحيح sqlite3_Btree_Transfer_صف(BtCursor *pDest, BtCursor *pSrc, i64 i_کلید){
  BtShared *pBt = pDest->pBt;
  u8 *a_باہرلا = pBt->pTmpSpace;    /* Pointer to next output buffer */
  ثابت u8 *a_وچ;                /* Pointer to next input buffer */
  u32 n_وچ;                      /* Size of input buffer aIn[] */
  u32 nRem;                     /* Bytes of data still to copy */

  getCellInfo(pSrc);
  إذا( pSrc->info.nPayload<0x80 ){
    *(a_باہرلا++) = (u8)pSrc->info.nPayload;
  }وإلا{
    a_باہرلا += sqlite3PutVarint(a_باہرلا, pSrc->info.nPayload);
  }
  إذا( pDest->p_کلید_Info==0 ) a_باہرلا += sqlite3PutVarint(a_باہرلا, i_کلید);
  n_وچ = pSrc->info.nLocal;
  a_وچ = pSrc->info.pPayload;
  إذا( a_وچ+n_وچ>pSrc->p_ورقہ->a_انکڑے_سرا ){
    أرجع SQLITE_CORRUPT_ورقہ(pSrc->p_ورقہ);
  }
  nRem = pSrc->info.nPayload;
  إذا( n_وچ==nRem && n_وچ<pDest->p_ورقہ->maxLocal ){
    حافظہ_نقل(a_باہرلا, a_وچ, n_وچ);
    pBt->n_Preformat_ناپ = n_وچ + (صحيح)(a_باہرلا - pBt->pTmpSpace);
    أرجع SQLITE_OK;
  }وإلا{
    صحيح rc = SQLITE_OK;
    Pager *pSrcPager = pSrc->pBt->pPager;
    u8 *p_Pgno_باہرلا = 0;
    Pgno ovfl_وچ = 0;
    Db_ورقہ *p_ورقہ_وچ = 0;
    Mem_ورقہ *p_ورقہ_باہرلا = 0;
    u32 n_باہرلا;                     /* Size of output buffer aOut[] */

    n_باہرلا = btree_Payload_منزل_Local(pDest->p_ورقہ, pSrc->info.nPayload);
    pBt->n_Preformat_ناپ = (صحيح)n_باہرلا + (صحيح)(a_باہرلا - pBt->pTmpSpace);
    إذا( n_باہرلا<pSrc->info.nPayload ){
      p_Pgno_باہرلا = &a_باہرلا[n_باہرلا];
      pBt->n_Preformat_ناپ += 4;
    }
 
    إذا( nRem>n_وچ ){
      إذا( a_وچ+n_وچ+4>pSrc->p_ورقہ->a_انکڑے_سرا ){
        أرجع SQLITE_CORRUPT_ورقہ(pSrc->p_ورقہ);
      }
      ovfl_وچ = get4byte(&pSrc->info.pPayload[n_وچ]);
    }
 
    افعل {
      nRem -= n_باہرلا;
      افعل{
        أكد( n_باہرلا>0 );
        إذا( n_وچ>0 ){
          صحيح nCopy = MIN(n_باہرلا, n_وچ);
          حافظہ_نقل(a_باہرلا, a_وچ, nCopy);
          n_باہرلا -= nCopy;
          n_وچ -= nCopy;
          a_باہرلا += nCopy;
          a_وچ += nCopy;
        }
        إذا( n_باہرلا>0 ){
          sqlite3PagerUnref(p_ورقہ_وچ);
          p_ورقہ_وچ = 0;
          rc = sqlite3PagerGet(pSrcPager, ovfl_وچ, &p_ورقہ_وچ, PAGER_GET_READONLY);
          إذا( rc==SQLITE_OK ){
            a_وچ = (ثابت u8*)sqlite3_Pager_Get_انکڑے(p_ورقہ_وچ);
            ovfl_وچ = get4byte(a_وچ);
            a_وچ += 4;
            n_وچ = pSrc->pBt->usable_ناپ - 4;
          }
        }
      }طالما( rc==SQLITE_OK && n_باہرلا>0 );
 
      إذا( rc==SQLITE_OK && nRem>0 && ALWAYS(p_Pgno_باہرلا) ){
        Pgno pgnoNew = 0;  /* Prevent harmless static-analyzer warning */
        Mem_ورقہ *pNew = 0;
        rc = allocate_Btree_ورقہ(pBt, &pNew, &pgnoNew, 0, 0);
        put4byte(p_Pgno_باہرلا, pgnoNew);
        إذا( ISAUTOVACUUM(pBt) && p_ورقہ_باہرلا ){
          ptrmapPut(pBt, pgnoNew, PTRMAP_OVERFLOW2, p_ورقہ_باہرلا->pgno, &rc);
        }
        release_ورقہ(p_ورقہ_باہرلا);
        p_ورقہ_باہرلا = pNew;
        إذا( p_ورقہ_باہرلا ){
          p_Pgno_باہرلا = p_ورقہ_باہرلا->a_انکڑے;
          put4byte(p_Pgno_باہرلا, 0);
          a_باہرلا = &p_Pgno_باہرلا[4];
          n_باہرلا = MIN(pBt->usable_ناپ - 4, nRem);
        }
      }
    }طالما( nRem>0 && rc==SQLITE_OK );
 
    release_ورقہ(p_ورقہ_باہرلا);
    sqlite3PagerUnref(p_ورقہ_وچ);
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
  Mem_ورقہ *p_ورقہ;            /* Page to delete cell from */
  بنانشانی حرف *pCell;      /* Pointer to cell to delete */
  صحيح iCellIdx;              /* Index of cell to delete */
  صحيح i_Cell_ڈونگھائی;            /* Depth of node containing pCell */
  CellInfo info;             /* Size of the cell being deleted */
  u8 bPreserve;              /* Keep cursor valid.  2 for CURSOR_SKIPNEXT */

  أكد( cursorOwnsBtShared(pCur) );
  أكد( pBt->وچ_Transaction==TRANS_WRITE );
  أكد( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  أكد( pCur->curFlags & BTCF_WriteFlag );
  أكد( has_Shared_Cache_جدول_جندرا(p, pCur->pgno_جڑ, pCur->p_کلید_Info!=0, 2) );
  أكد( !hasReadConflicts(p, pCur->pgno_جڑ) );
  أكد( (flags & ~(BTREE_SAVEPOSITION | BTREE_AUXDELETE))==0 );
  إذا( pCur->eState!=CURSOR_VALID ){
    إذا( pCur->eState>=CURSOR_REQUIRESEEK ){
      rc = btreeRestoreCursorPosition(pCur);
      أكد( rc!=SQLITE_OK || CORRUPT_DB || pCur->eState==CURSOR_VALID );
      إذا( rc || pCur->eState!=CURSOR_VALID ) أرجع rc;
    }وإلا{
      أرجع SQLITE_CORRUPT_PGNO(pCur->pgno_جڑ);
    }
  }
  أكد( pCur->eState==CURSOR_VALID );

  i_Cell_ڈونگھائی = pCur->i_ورقہ;
  iCellIdx = pCur->ix;
  p_ورقہ = pCur->p_ورقہ;
  إذا( p_ورقہ->nCell<=iCellIdx ){
    أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
  }
  pCell = لبھو_Cell(p_ورقہ, iCellIdx);
  إذا( p_ورقہ->nFree<0 && btreeComputeFreeSpace(p_ورقہ) ){
    أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
  }
  إذا( pCell<&p_ورقہ->aCellIdx[p_ورقہ->nCell] ){
    أرجع SQLITE_CORRUPT_ورقہ(p_ورقہ);
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
    إذا( !p_ورقہ->پتی
     || (p_ورقہ->nFree+p_ورقہ->x_Cell_ناپ(p_ورقہ,pCell)+2) >
                                                   (صحيح)(pBt->usable_ناپ*2/3)
     || p_ورقہ->nCell==1  /* See dbfuzz001.test for a test case */
    ){
      /* A b-tree rebalance will be required after deleting this entry.
      ** Save the cursor key.  */
      rc = save_Cursor_کلید(pCur);
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
  إذا( !p_ورقہ->پتی ){
    rc = sqlite3_Btree_پچھلا(pCur, 0);
    أكد( rc!=SQLITE_پورا );
    إذا( rc ) أرجع rc;
  }

  /* Save the positions of any other cursors open on this table before
  ** making any modifications.  */
  إذا( pCur->curFlags & BTCF_مضرب ){
    rc = saveAllCursors(pBt, pCur->pgno_جڑ, pCur);
    إذا( rc ) أرجع rc;
  }

  /* If this is a delete operation to remove a row from a table b-tree,
  ** invalidate any incrblob cursors open on the row being deleted.  */
  إذا( pCur->p_کلید_Info==0 && p->hasIncrblobCur ){
    invalidateIncrblobCursors(p, pCur->pgno_جڑ, pCur->info.n_کلید, 0);
  }

  /* Make the page containing the entry to be deleted writable. Then free any
  ** overflow pages associated with the entry and finally remove the cell
  ** itself from within the page.  */
  rc = sqlite3PagerWrite(p_ورقہ->p_Db_ورقہ);
  إذا( rc ) أرجع rc;
  BTREE_CLEAR_CELL(rc, p_ورقہ, pCell, info);
  dropCell(p_ورقہ, iCellIdx, info.n_ناپ, &rc);
  إذا( rc ) أرجع rc;

  /* If the cell deleted was not located on a leaf page, then the cursor
  ** is currently pointing to the largest entry in the sub-tree headed
  ** by the child-page of the cell that was just deleted from an internal
  ** node. The cell from the leaf node needs to be moved to the internal
  ** node to replace the deleted cell.  */
  إذا( !p_ورقہ->پتی ){
    Mem_ورقہ *p_پتی = pCur->p_ورقہ;
    صحيح nCell;
    Pgno n;
    بنانشانی حرف *pTmp;

    إذا( p_پتی->nFree<0 ){
      rc = btreeComputeFreeSpace(p_پتی);
      إذا( rc ) أرجع rc;
    }
    إذا( i_Cell_ڈونگھائی<pCur->i_ورقہ-1 ){
      n = pCur->ap_ورقہ[i_Cell_ڈونگھائی+1]->pgno;
    }وإلا{
      n = pCur->p_ورقہ->pgno;
    }
    pCell = لبھو_Cell(p_پتی, p_پتی->nCell-1);
    إذا( pCell<&p_پتی->a_انکڑے[4] ) أرجع SQLITE_CORRUPT_ورقہ(p_پتی);
    nCell = p_پتی->x_Cell_ناپ(p_پتی, pCell);
    أكد( MX_CELL_ناپ(pBt) >= nCell );
    pTmp = pBt->pTmpSpace;
    أكد( pTmp!=0 );
    rc = sqlite3PagerWrite(p_پتی->p_Db_ورقہ);
    إذا( rc==SQLITE_OK ){
      rc = داخل_کرو_Cell(p_ورقہ, iCellIdx, pCell-4, nCell+4-(pCell[0]==2), pTmp, n);
    }
    dropCell(p_پتی, p_پتی->nCell-1, nCell, &rc);
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
  أكد( pCur->p_ورقہ->n_حد_توں_ودھ==0 );
  أكد( pCur->p_ورقہ->nFree>=0 );
  إذا( pCur->p_ورقہ->nFree*3<=(صحيح)pCur->pBt->usable_ناپ*2 ){
    /* Optimization: If the free space is less than 2/3rds of the page,
    ** then balance() will always be a no-op.  No need to invoke it. */
    rc = SQLITE_OK;
  }وإلا{
    rc = balance(pCur);
  }
  إذا( rc==SQLITE_OK && pCur->i_ورقہ>i_Cell_ڈونگھائی ){
    release_ورقہ_Not_Null(pCur->p_ورقہ);
    pCur->i_ورقہ--;
    طالما( pCur->i_ورقہ>i_Cell_ڈونگھائی ){
      release_ورقہ(pCur->ap_ورقہ[pCur->i_ورقہ--]);
    }
    pCur->p_ورقہ = pCur->ap_ورقہ[pCur->i_ورقہ];
    rc = balance(pCur);
  }

  إذا( rc==SQLITE_OK ){
    إذا( bPreserve>1 ){
      أكد( (pCur->i_ورقہ==i_Cell_ڈونگھائی || CORRUPT_DB) );
      أكد( p_ورقہ==pCur->p_ورقہ || CORRUPT_DB );
      أكد( (p_ورقہ->nCell>0 || CORRUPT_DB) && iCellIdx<=p_ورقہ->nCell );
      pCur->eState = CURSOR_SKIPNEXT;
      إذا( iCellIdx>=p_ورقہ->nCell ){
        pCur->skip_اگلا = -1;
        pCur->ix = p_ورقہ->nCell-1;
      }وإلا{
        pCur->skip_اگلا = 1;
      }
      /* The pCur->info structure is currently populated with the details
      ** of the cell that was just removed. But pCur->ix now points to a
      ** different cell in the b-tree. So zero info.nSize here so that if
      ** saveCursorPosition() is called on this cursor it does not trust
      ** the contents of pCur->info.  */
      pCur->info.n_ناپ = 0;
    }وإلا{
      rc = move_منزل_جڑ(pCur);
      إذا( bPreserve ){
        btree_Release_All_Cursor_ورقے(pCur);
        pCur->eState = CURSOR_REQUIRESEEK;
      }
      إذا( rc==SQLITE_خالی_تھاواں ) rc = SQLITE_OK;
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
ساكن صحيح btree_Create_جدول(Btree *p, Pgno *pi_جدول, صحيح createTabFlags){
  BtShared *pBt = p->pBt;
  Mem_ورقہ *p_جڑ;
  Pgno pgno_جڑ;
  صحيح rc;
  صحيح ptfFlags;          /* Page-type flags for the root page of new table */

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( pBt->وچ_Transaction==TRANS_WRITE );
  أكد( (pBt->btsFlags & BTS_READ_ONLY)==0 );

#جے_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  rc = allocate_Btree_ورقہ(pBt, &p_جڑ, &pgno_جڑ, 1, 0);
  إذا( rc ){
    أرجع rc;
  }
#وإلا
  إذا( pBt->autoVacuum ){
    Pgno pgnoMove;      /* Move a page here to make room for the root-page */
    Mem_ورقہ *p_ورقہ_Move; /* The page to move to. */

    /* Creating a new table may probably require moving an existing database
    ** to make room for the new tables root page. In case this page turns
    ** out to be an overflow page, delete all overflow page-map caches
    ** held by open cursors.
    */
    invalidate_All_حد_توں_ودھ_Cache(pBt);

    /* Read the value of meta[3] from the database to determine where the
    ** root page of the new table should go. meta[3] is the largest root-page
    ** created so far, so the new root-page is (meta[3]+1).
    */
    sqlite3BtreeGetMeta(p, BTREE_سبھ_توں_وڈا_جڑ_ورقہ, &pgno_جڑ);
    إذا( pgno_جڑ>btreePagecount(pBt) ){
      أرجع SQLITE_CORRUPT_PGNO(pgno_جڑ);
    }
    pgno_جڑ++;

    /* The new root-page may not be allocated on a pointer-map page, or the
    ** PENDING_BYTE page.
    */
    طالما( pgno_جڑ==PTRMAP_PAGENO(pBt, pgno_جڑ) ||
        pgno_جڑ==PENDING_BYTE_ورقہ(pBt) ){
      pgno_جڑ++;
    }
    أكد( pgno_جڑ>=3 );

    /* Allocate a page. The page that currently resides at pgnoRoot will
    ** be moved to the allocated page (unless the allocated page happens
    ** to reside at pgnoRoot).
    */
    rc = allocate_Btree_ورقہ(pBt, &p_ورقہ_Move, &pgnoMove, pgno_جڑ, BTALLOC_EXACT);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }

    إذا( pgnoMove!=pgno_جڑ ){
      /* pgnoRoot is the page that will be used for the root-page of
      ** the new table (assuming an error did not occur). But we were
      ** allocated pgnoMove. If required (i.e. if it was not allocated
      ** by extending the file), the current page at position pgnoMove
      ** is already journaled.
      */
      u8 eType = 0;
      Pgno i_Ptr_ورقہ = 0;

      /* Save the positions of any open cursors. This is required in
      ** case they are holding a reference to an xFetch reference
      ** corresponding to page pgnoRoot.  */
      rc = saveAllCursors(pBt, 0, 0);
      release_ورقہ(p_ورقہ_Move);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }

      /* Move the page currently at pgnoRoot to pgnoMove. */
      rc = btree_Get_ورقہ(pBt, pgno_جڑ, &p_جڑ, 0);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
      rc = ptrmapGet(pBt, pgno_جڑ, &eType, &i_Ptr_ورقہ);
      إذا( eType==PTRMAP_ROOTPAGE || eType==PTRMAP_FREEPAGE ){
        rc = SQLITE_CORRUPT_PGNO(pgno_جڑ);
      }
      إذا( rc!=SQLITE_OK ){
        release_ورقہ(p_جڑ);
        أرجع rc;
      }
      أكد( eType!=PTRMAP_ROOTPAGE );
      أكد( eType!=PTRMAP_FREEPAGE );
      rc = relocate_ورقہ(pBt, p_جڑ, eType, i_Ptr_ورقہ, pgnoMove, 0);
      release_ورقہ(p_جڑ);

      /* Obtain the page at pgnoRoot */
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
      rc = btree_Get_ورقہ(pBt, pgno_جڑ, &p_جڑ, 0);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
      rc = sqlite3PagerWrite(p_جڑ->p_Db_ورقہ);
      إذا( rc!=SQLITE_OK ){
        release_ورقہ(p_جڑ);
        أرجع rc;
      }
    }وإلا{
      p_جڑ = p_ورقہ_Move;
    }

    /* Update the pointer-map and meta-data with the new root-page number. */
    ptrmapPut(pBt, pgno_جڑ, PTRMAP_ROOTPAGE, 0, &rc);
    إذا( rc ){
      release_ورقہ(p_جڑ);
      أرجع rc;
    }

    /* When the new root page was allocated, page 1 was made writable in
    ** order either to increase the database filesize, or to decrement the
    ** freelist count.  Hence, the sqlite3BtreeUpdateMeta() call cannot fail.
    */
    أكد( sqlite3PagerIswriteable(pBt->pPage1->p_Db_ورقہ) );
    rc = sqlite3BtreeUpdateMeta(p, 4, pgno_جڑ);
    إذا( NEVER(rc) ){
      release_ورقہ(p_جڑ);
      أرجع rc;
    }

  }وإلا{
    rc = allocate_Btree_ورقہ(pBt, &p_جڑ, &pgno_جڑ, 1, 0);
    إذا( rc ) أرجع rc;
  }
#جے_انت
  أكد( sqlite3PagerIswriteable(p_جڑ->p_Db_ورقہ) );
  إذا( createTabFlags & BTREE_INTKEY ){
    ptfFlags = PTF_INTKEY | PTF_LEAFDATA | PTF_پتی;
  }وإلا{
    ptfFlags = PTF_ZERODATA | PTF_پتی;
  }
  zero_ورقہ(p_جڑ, ptfFlags);
  sqlite3PagerUnref(p_جڑ->p_Db_ورقہ);
  أكد( (pBt->openFlags & BTREE_SINGLE)==0 || pgno_جڑ==2 );
  *pi_جدول = pgno_جڑ;
  أرجع SQLITE_OK;
}
صحيح sqlite3_Btree_Create_جدول(Btree *p, Pgno *pi_جدول, صحيح flags){
  صحيح rc;
  sqlite3BtreeEnter(p);
  rc = btree_Create_جدول(p, pi_جدول, flags);
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
ساكن صحيح clear_Database_ورقہ(
  BtShared *pBt,           /* The BTree that contains the table */
  Pgno pgno,               /* Page number to clear */
  صحيح free_ورقہ_Flag,        /* bit 0: Deallocate page.  Bits 1-31: depth */
  i64 *pnChange            /* Add number of Cells freed to this counter */
){
  Mem_ورقہ *p_ورقہ;
  صحيح rc;
  بنانشانی حرف *pCell;
  صحيح i;
  صحيح hdr;
  CellInfo info;

  أكد( sqlite3_mutex_held(pBt->mutex) );
  إذا( pgno>btreePagecount(pBt) ){
    أرجع SQLITE_CORRUPT_PGNO(pgno);
  }
  إذا( (free_ورقہ_Flag>>1) > BTCURSOR_MAX_ڈونگھائی ){
    أرجع SQLITE_CORRUPT_PGNO(pgno);
  }
  rc = get_And_Init_ورقہ(pBt, pgno, &p_ورقہ, 0);
  إذا( rc ) أرجع rc;
  إذا( (pBt->openFlags & BTREE_SINGLE)==0
   && sqlite3_Pager_ورقہ_Refcount(p_ورقہ->p_Db_ورقہ) != (1 + (pgno==1))
  ){
    rc = SQLITE_CORRUPT_ورقہ(p_ورقہ);
    اذهب cleardatabasepage_باہرلا;
  }
  hdr = p_ورقہ->hdr_وتھ;
  لكل(i=0; i<p_ورقہ->nCell; i++){
    pCell = لبھو_Cell(p_ورقہ, i);
    إذا( !p_ورقہ->پتی ){
      rc = clear_Database_ورقہ(pBt, get4byte(pCell),
                             (free_ورقہ_Flag+2)|1, pnChange);
      إذا( rc ) اذهب cleardatabasepage_باہرلا;
    }
    BTREE_CLEAR_CELL(rc, p_ورقہ, pCell, info);
    إذا( rc ) اذهب cleardatabasepage_باہرلا;
  }
  إذا( !p_ورقہ->پتی ){
    rc = clear_Database_ورقہ(pBt, get4byte(&p_ورقہ->a_انکڑے[hdr+8]), 
                           (free_ورقہ_Flag+2)|1, pnChange);
    إذا( rc ) اذهب cleardatabasepage_باہرلا;
    إذا( p_ورقہ->int_کلید ) pnChange = 0;
  }
  إذا( pnChange ){
    testcase( !p_ورقہ->int_کلید );
    *pnChange += p_ورقہ->nCell;
  }
  إذا( (free_ورقہ_Flag&1)!=0 ){
    free_ورقہ(p_ورقہ, &rc);
  }وإلا إذا( (rc = sqlite3PagerWrite(p_ورقہ->p_Db_ورقہ))==0 ){
    zero_ورقہ(p_ورقہ, p_ورقہ->a_انکڑے[hdr] | PTF_پتی);
  }

cleardatabasepage_باہرلا:
  release_ورقہ(p_ورقہ);
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
صحيح sqlite3_Btree_Clear_جدول(Btree *p, صحيح i_جدول, i64 *pnChange){
  صحيح rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);
  أكد( p->وچ_Trans==TRANS_WRITE );

  rc = saveAllCursors(pBt, (Pgno)i_جدول, 0);

  إذا( SQLITE_OK==rc ){
    /* Invalidate all incrblob cursors open on table iTable (assuming iTable
    ** is the root of a table b-tree - if it is not, the following call is
    ** a no-op).  */
    إذا( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, (Pgno)i_جدول, 0, 1);
    }
    rc = clear_Database_ورقہ(pBt, (Pgno)i_جدول, 0, pnChange);
  }
  sqlite3BtreeLeave(p);
  أرجع rc;
}

/*
** Delete all information from the single table that pCur is open on.
**
** This routine only work for pCur on an ephemeral table.
*/
صحيح sqlite3_Btree_Clear_جدول_دا_Cursor(BtCursor *pCur){
  أرجع sqlite3_Btree_Clear_جدول(pCur->pBtree, pCur->pgno_جڑ, 0);
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
ساكن صحيح btree_Drop_جدول(Btree *p, Pgno i_جدول, صحيح *piMoved){
  صحيح rc;
  Mem_ورقہ *p_ورقہ = 0;
  BtShared *pBt = p->pBt;

  أكد( sqlite3BtreeHoldsMutex(p) );
  أكد( p->وچ_Trans==TRANS_WRITE );
  أكد( i_جدول>=2 );
  إذا( i_جدول>btreePagecount(pBt) ){
    أرجع SQLITE_CORRUPT_PGNO(i_جدول);
  }

  rc = sqlite3_Btree_Clear_جدول(p, i_جدول, 0);
  إذا( rc ) أرجع rc;
  rc = btree_Get_ورقہ(pBt, (Pgno)i_جدول, &p_ورقہ, 0);
  إذا( NEVER(rc) ){
    release_ورقہ(p_ورقہ);
    أرجع rc;
  }

  *piMoved = 0;

#جے_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  free_ورقہ(p_ورقہ, &rc);
  release_ورقہ(p_ورقہ);
#وإلا
  إذا( pBt->autoVacuum ){
    Pgno max_جڑ_Pgno;
    sqlite3BtreeGetMeta(p, BTREE_سبھ_توں_وڈا_جڑ_ورقہ, &max_جڑ_Pgno);

    إذا( i_جدول==max_جڑ_Pgno ){
      /* If the table being dropped is the table with the largest root-page
      ** number in the database, put the root page on the free list.
      */
      free_ورقہ(p_ورقہ, &rc);
      release_ورقہ(p_ورقہ);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
    }وإلا{
      /* The table being dropped does not have the largest root-page
      ** number in the database. So move the page that does into the
      ** gap left by the deleted root-page.
      */
      Mem_ورقہ *pMove;
      release_ورقہ(p_ورقہ);
      rc = btree_Get_ورقہ(pBt, max_جڑ_Pgno, &pMove, 0);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
      rc = relocate_ورقہ(pBt, pMove, PTRMAP_ROOTPAGE, 0, i_جدول, 0);
      release_ورقہ(pMove);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
      pMove = 0;
      rc = btree_Get_ورقہ(pBt, max_جڑ_Pgno, &pMove, 0);
      إذا( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pMove->p_Db_ورقہ);
      }
      free_ورقہ(pMove, &rc);
      release_ورقہ(pMove);
      إذا( rc!=SQLITE_OK ){
        أرجع rc;
      }
      *piMoved = max_جڑ_Pgno;
    }

    /* Set the new 'max-root-page' value in the database header. This
    ** is the old value less one, less one more if that happens to
    ** be a root-page number, less one again if that is the
    ** PENDING_BYTE_PAGE.
    */
    max_جڑ_Pgno--;
    طالما( max_جڑ_Pgno==PENDING_BYTE_ورقہ(pBt)
           || PTRMAP_ISPAGE(pBt, max_جڑ_Pgno) ){
      max_جڑ_Pgno--;
    }
    أكد( max_جڑ_Pgno!=PENDING_BYTE_ورقہ(pBt) );

    rc = sqlite3BtreeUpdateMeta(p, 4, max_جڑ_Pgno);
  }وإلا{
    free_ورقہ(p_ورقہ, &rc);
    release_ورقہ(p_ورقہ);
  }
#جے_انت
  أرجع rc; 
}
صحيح sqlite3_Btree_Drop_جدول(Btree *p, صحيح i_جدول, صحيح *piMoved){
  صحيح rc;
  sqlite3BtreeEnter(p);
  rc = btree_Drop_جدول(p, i_جدول, piMoved);
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
  أكد( p->وچ_Trans>TRANS_NONE );
  أكد( SQLITE_OK==query_Shared_Cache_جدول_جندرا(p, SCHEMA_جڑ, READ_جندرا) );
  أكد( pBt->pPage1 );
  أكد( idx>=0 && idx<=15 );

  إذا( idx==BTREE_انکڑے_نسخہ ){
    *pMeta = sqlite3_Pager_انکڑے_نسخہ(pBt->pPager) + p->i_BData_نسخہ;
  }وإلا{
    *pMeta = get4byte(&pBt->pPage1->a_انکڑے[36 + idx*4]);
  }

  /* If auto-vacuum is disabled in this build and this is an auto-vacuum
  ** database, mark the database as read-only.  */
#جے_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
  إذا( idx==BTREE_سبھ_توں_وڈا_جڑ_ورقہ && *pMeta>0 ){
    pBt->btsFlags |= BTS_READ_ONLY;
  }
#جے_انت

  sqlite3BtreeLeave(p);
}

/*
** Write meta-information back into the database.  Meta[0] is
** read-only and may not be written.
*/
صحيح sqlite3BtreeUpdateMeta(Btree *p, صحيح idx, u32 iMeta){
  BtShared *pBt = p->pBt;
  بنانشانی حرف *pP1;
  صحيح rc;
  أكد( idx>=1 && idx<=15 );
  sqlite3BtreeEnter(p);
  أكد( p->وچ_Trans==TRANS_WRITE );
  أكد( pBt->pPage1!=0 );
  pP1 = pBt->pPage1->a_انکڑے;
  rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ورقہ);
  إذا( rc==SQLITE_OK ){
    put4byte(&pP1[36 + idx*4], iMeta);
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
    إذا( idx==BTREE_INCR_VACUUM ){
      أكد( pBt->autoVacuum || iMeta==0 );
      أكد( iMeta==0 || iMeta==1 );
      pBt->incrVacuum = (u8)iMeta;
    }
#جے_انت
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
صحيح sqlite3_Btree_گنتی(sqlite3 *db, BtCursor *pCur, i64 *pn_اندراج){
  i64 n_اندراج = 0;                      /* Value to return in *pnEntry */
  صحيح rc;                              /* Return code */

  rc = move_منزل_جڑ(pCur);
  إذا( rc==SQLITE_خالی_تھاواں ){
    *pn_اندراج = 0;
    أرجع SQLITE_OK;
  }

  /* Unless an error occurs, the following loop runs one iteration for each
  ** page in the B-Tree structure (not including overflow pages).
  */
  طالما( rc==SQLITE_OK && !Atomic_لدو(&db->u1.isInterrupted) ){
    صحيح iIdx;                          /* Index of child node in parent */
    Mem_ورقہ *p_ورقہ;                    /* Current page of the b-tree */

    /* If this is a leaf page or the tree is not an int-key tree, then
    ** this page contains countable entries. Increment the entry counter
    ** accordingly.
    */
    p_ورقہ = pCur->p_ورقہ;
    إذا( p_ورقہ->پتی || !p_ورقہ->int_کلید ){
      n_اندراج += p_ورقہ->nCell;
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
    إذا( p_ورقہ->پتی ){
      افعل {
        إذا( pCur->i_ورقہ==0 ){
          /* All pages of the b-tree have been visited. Return successfully. */
          *pn_اندراج = n_اندراج;
          أرجع move_منزل_جڑ(pCur);
        }
        move_منزل_ماپا(pCur);
      }طالما ( pCur->ix>=pCur->p_ورقہ->nCell );

      pCur->ix++;
      p_ورقہ = pCur->p_ورقہ;
    }

    /* Descend to the child node of the cell that the cursor currently
    ** points at. This is the right-child if (iIdx==pPage->nCell).
    */
    iIdx = pCur->ix;
    إذا( iIdx==p_ورقہ->nCell ){
      rc = move_منزل_بچہ(pCur, get4byte(&p_ورقہ->a_انکڑے[p_ورقہ->hdr_وتھ+8]));
    }وإلا{
      rc = move_منزل_بچہ(pCur, get4byte(لبھو_Cell(p_ورقہ, iIdx)));
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

#جے_بنا_تعریف SQLITE_OMIT_INTEGRITY_CHECK
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
ساكن فراغ check_ترقی(IntegrityCk *pCheck){
  sqlite3 *db = pCheck->db;
  إذا( Atomic_لدو(&db->u1.isInterrupted) ){
    pCheck->rc = SQLITE_رکاوٹ;
    pCheck->nErr++;
    pCheck->mxErr = 0;
  }
#جے_بنا_تعریف SQLITE_OMIT_ترقی_CALLBACK
  إذا( db->x_ترقی ){
    أكد( db->n_ترقی_Ops>0 );
    pCheck->n_قدم++;
    إذا( (pCheck->n_قدم % db->n_ترقی_Ops)==0
     && db->x_ترقی(db->p_ترقی_Arg)
    ){
      pCheck->rc = SQLITE_رکاوٹ;
      pCheck->nErr++;
      pCheck->mxErr = 0;
    }
  }
#جے_انت
}

/*
** Append a message to the error message string.
*/
ساكن فراغ check_Append_پیغام(
  IntegrityCk *pCheck,
  ثابت حرف *zFormat,
  ...
){
  va_list ap;
  check_ترقی(pCheck);
  إذا( !pCheck->mxErr ) أرجع;
  pCheck->mxErr--;
  pCheck->nErr++;
  va_start(ap, zFormat);
  إذا( pCheck->err_پیغام.nChar ){
    sqlite3_str_append(&pCheck->err_پیغام, "\n", 1);
  }
  إذا( pCheck->zPfx ){
    sqlite3_str_appendf(&pCheck->err_پیغام, pCheck->zPfx,
                        pCheck->v0, pCheck->v1, pCheck->v2);
  }
  sqlite3_str_vappendf(&pCheck->err_پیغام, zFormat, ap);
  va_end(ap);
  إذا( pCheck->err_پیغام.accError==SQLITE_NOMEM ){
    checkOom(pCheck);
  }
}
#جے_انت /* SQLITE_OMIT_INTEGRITY_CHECK */

#جے_بنا_تعریف SQLITE_OMIT_INTEGRITY_CHECK

/*
** Return non-zero if the bit in the IntegrityCk.aPgRef[] array that
** corresponds to page iPg is already set.
*/
ساكن صحيح get_ورقہ_Referenced(IntegrityCk *pCheck, Pgno iPg){
  أكد( pCheck->aPgRef!=0 );
  أكد( iPg<=pCheck->n_Ck_ورقہ && حجم(pCheck->aPgRef[0])==1 );
  أرجع (pCheck->aPgRef[iPg/8] & (1 << (iPg & 0x07)));
}

/*
** Set the bit in the IntegrityCk.aPgRef[] array that corresponds to page iPg.
*/
ساكن فراغ سیٹ_ورقہ_Referenced(IntegrityCk *pCheck, Pgno iPg){
  أكد( pCheck->aPgRef!=0 );
  أكد( iPg<=pCheck->n_Ck_ورقہ && حجم(pCheck->aPgRef[0])==1 );
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
ساكن صحيح checkRef(IntegrityCk *pCheck, Pgno i_ورقہ){
  إذا( i_ورقہ>pCheck->n_Ck_ورقہ || i_ورقہ==0 ){
    check_Append_پیغام(pCheck, "invalid page number %u", i_ورقہ);
    أرجع 1;
  }
  إذا( get_ورقہ_Referenced(pCheck, i_ورقہ) ){
    check_Append_پیغام(pCheck, "2nd reference to page %u", i_ورقہ);
    أرجع 1;
  }
  سیٹ_ورقہ_Referenced(pCheck, i_ورقہ);
  أرجع 0;
}

#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
/*
** Check that the entry in the pointer-map for page iChild maps to
** page iParent, pointer type ptrType. If not, append an error message
** to pCheck.
*/
ساكن فراغ checkPtrmap(
  IntegrityCk *pCheck,   /* Integrity check context */
  Pgno i_بچہ,           /* Child page number */
  u8 eType,              /* Expected pointer map type */
  Pgno i_ماپا           /* Expected pointer map parent page number */
){
  صحيح rc;
  u8 ePtrmapType;
  Pgno i_Ptrmap_ماپا;

  rc = ptrmapGet(pCheck->pBt, i_بچہ, &ePtrmapType, &i_Ptrmap_ماپا);
  إذا( rc!=SQLITE_OK ){
    إذا( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ) checkOom(pCheck);
    check_Append_پیغام(pCheck, "Failed to read ptrmap key=%u", i_بچہ);
    أرجع;
  }

  إذا( ePtrmapType!=eType || i_Ptrmap_ماپا!=i_ماپا ){
    check_Append_پیغام(pCheck,
      "Bad ptr map entry key=%u expected=(%u,%u) got=(%u,%u)",
      i_بچہ, eType, i_ماپا, ePtrmapType, i_Ptrmap_ماپا);
  }
}
#جے_انت

/*
** Check the integrity of the freelist or of an overflow page list.
** Verify that the number of pages on the list is N.
*/
ساكن فراغ checkList(
  IntegrityCk *pCheck,  /* Integrity checking context */
  صحيح isFreeList,       /* True for a freelist.  False for overflow page list */
  Pgno i_ورقہ,           /* Page number for first page in the list */
  u32 N                 /* Expected number of pages in the list */
){
  صحيح i;
  u32 expected = N;
  صحيح n_Err_At_شروع = pCheck->nErr;
  طالما( i_ورقہ!=0 && pCheck->mxErr ){
    Db_ورقہ *p_Ovfl_ورقہ;
    بنانشانی حرف *p_Ovfl_انکڑے;
    إذا( checkRef(pCheck, i_ورقہ) ) اكسر;
    N--;
    إذا( sqlite3PagerGet(pCheck->pPager, (Pgno)i_ورقہ, &p_Ovfl_ورقہ, 0) ){
      check_Append_پیغام(pCheck, "failed to get page %u", i_ورقہ);
      اكسر;
    }
    p_Ovfl_انکڑے = (بنانشانی حرف *)sqlite3_Pager_Get_انکڑے(p_Ovfl_ورقہ);
    إذا( isFreeList ){
      u32 n = (u32)get4byte(&p_Ovfl_انکڑے[4]);
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
      إذا( pCheck->pBt->autoVacuum ){
        checkPtrmap(pCheck, i_ورقہ, PTRMAP_FREEPAGE, 0);
      }
#جے_انت
      إذا( n>pCheck->pBt->usable_ناپ/4-2 ){
        check_Append_پیغام(pCheck,
           "freelist leaf count too big on page %u", i_ورقہ);
        N--;
      }وإلا{
        لكل(i=0; i<(صحيح)n; i++){
          Pgno i_Free_ورقہ = get4byte(&p_Ovfl_انکڑے[8+i*4]);
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
          إذا( pCheck->pBt->autoVacuum ){
            checkPtrmap(pCheck, i_Free_ورقہ, PTRMAP_FREEPAGE, 0);
          }
#جے_انت
          checkRef(pCheck, i_Free_ورقہ);
        }
        N -= n;
      }
    }
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
    وإلا{
      /* If this database supports auto-vacuum and iPage is not the last
      ** page in this overflow list, check that the pointer-map entry for
      ** the following page matches iPage.
      */
      إذا( pCheck->pBt->autoVacuum && N>0 ){
        i = get4byte(p_Ovfl_انکڑے);
        checkPtrmap(pCheck, i, PTRMAP_OVERFLOW2, i_ورقہ);
      }
    }
#جے_انت
    i_ورقہ = get4byte(p_Ovfl_انکڑے);
    sqlite3PagerUnref(p_Ovfl_ورقہ);
  }
  إذا( N && n_Err_At_شروع==pCheck->nErr ){
    check_Append_پیغام(pCheck,
      "%s is %u but should be %u",
      isFreeList ? "size" : "overflow list length",
      expected-N, expected);
  }
}
#جے_انت /* SQLITE_OMIT_INTEGRITY_CHECK */

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
ساكن فراغ btree_ڈھیر_داخل_کرو(u32 *a_ڈھیر, u32 x){
  u32 j, i;
  أكد( a_ڈھیر!=0 );
  i = ++a_ڈھیر[0];
  a_ڈھیر[i] = x;
  طالما( (j = i/2)>0 && a_ڈھیر[j]>a_ڈھیر[i] ){
    x = a_ڈھیر[j];
    a_ڈھیر[j] = a_ڈھیر[i];
    a_ڈھیر[i] = x;
    i = j;
  }
}
ساكن صحيح btree_ڈھیر_Pull(u32 *a_ڈھیر, u32 *p_باہرلا){
  u32 j, i, x;
  إذا( (x = a_ڈھیر[0])==0 ) أرجع 0;
  *p_باہرلا = a_ڈھیر[1];
  a_ڈھیر[1] = a_ڈھیر[x];
  a_ڈھیر[x] = 0xffffffff;
  a_ڈھیر[0]--;
  i = 1;
  طالما( (j = i*2)<=a_ڈھیر[0] ){
    إذا( a_ڈھیر[j]>a_ڈھیر[j+1] ) j++;
    إذا( a_ڈھیر[i]<a_ڈھیر[j] ) اكسر;
    x = a_ڈھیر[i];
    a_ڈھیر[i] = a_ڈھیر[j];
    a_ڈھیر[j] = x;
    i = j;
  }
  أرجع 1; 
}

#جے_بنا_تعریف SQLITE_OMIT_INTEGRITY_CHECK
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
ساكن صحيح check_درخت_ورقہ(
  IntegrityCk *pCheck,  /* Context for the sanity check */
  Pgno i_ورقہ,           /* Page number of the page to check */
  i64 *pi_Min_کلید,        /* Write minimum integer primary key here */
  i64 max_کلید            /* Error if integer primary key greater than this */
){
  Mem_ورقہ *p_ورقہ = 0;      /* The page being analyzed */
  صحيح i;                   /* Loop counter */
  صحيح rc;                  /* Result code from subroutine call */
  صحيح ڈونگھائی = -1, d2;      /* Depth of a subtree */
  صحيح pgno;                /* Page number */
  صحيح nFrag;               /* Number of fragmented bytes on the page */
  صحيح hdr;                 /* Offset to the page header */
  صحيح cell_شروع;           /* Offset to the start of the cell pointer array */
  صحيح nCell;               /* Number of cells */
  صحيح doCoverageCheck = 1; /* True if cell coverage checking should be done */
  صحيح کلید_Can_Be_Equal = 1;   /* True if IPK can be equal to maxKey
                           ** False if IPK must be strictly less than maxKey */
  u8 *انکڑے;                /* Page content */
  u8 *pCell;               /* Cell content */
  u8 *pCellIdx;            /* Next element of the cell pointer array */
  BtShared *pBt;           /* The BtShared object that owns pPage */
  u32 pc;                  /* Address of a cell */
  u32 usable_ناپ;          /* Usable size of the page */
  u32 content_وتھ;       /* Offset to the start of the cell content area */
  u32 *ڈھیر = 0;           /* Min-heap used for checking cell coverage */
  u32 x, prev = 0;         /* Next and previous entry on the min-heap */
  ثابت حرف *saved_zPfx = pCheck->zPfx;
  صحيح saved_v1 = pCheck->v1;
  صحيح saved_v2 = pCheck->v2;
  u8 savedIsInit = 0;

  /* Check that the page exists
  */
  check_ترقی(pCheck);
  إذا( pCheck->mxErr==0 ) أرجع 0;
  pBt = pCheck->pBt;
  usable_ناپ = pBt->usable_ناپ;
  إذا( i_ورقہ==0 ) أرجع 0;
  إذا( checkRef(pCheck, i_ورقہ) ) أرجع 0;
  pCheck->zPfx = "Tree %u page %u: ";
  pCheck->v1 = i_ورقہ;
  pCheck->nAbove++;
  إذا( (rc = btree_Get_ورقہ(pBt, i_ورقہ, &p_ورقہ, 0))!=0 ){
    check_Append_پیغام(pCheck,
       "unable to get the page. error code=%d", rc);
    إذا( rc==SQLITE_IOERR_NOMEM ) pCheck->rc = SQLITE_NOMEM;
    اذهب سرا_دا_check;
  }
  إذا( pCheck->nAbove > BTCURSOR_MAX_ڈونگھائی ){
    check_Append_پیغام(pCheck,"btree depth exceeds %d",BTCURSOR_MAX_ڈونگھائی);
    اذهب سرا_دا_check;
  }

  /* Clear MemPage.isInit to make sure the corruption detection code in
  ** btreeInitPage() is executed.  */
  savedIsInit = p_ورقہ->isInit;
  p_ورقہ->isInit = 0;
  إذا( (rc = btree_Init_ورقہ(p_ورقہ))!=0 ){
    أكد( rc==SQLITE_CORRUPT );  /* The only possible error from InitPage */
    check_Append_پیغام(pCheck,
                   "btreeInitPage() returns error code %d", rc);
    اذهب سرا_دا_check;
  }
  إذا( (rc = btreeComputeFreeSpace(p_ورقہ))!=0 ){
    أكد( rc==SQLITE_CORRUPT );
    check_Append_پیغام(pCheck, "free space corruption", rc);
    اذهب سرا_دا_check;
  }
  انکڑے = p_ورقہ->a_انکڑے;
  hdr = p_ورقہ->hdr_وتھ;

  /* Set up for cell analysis */
  pCheck->zPfx = "Tree %u page %u cell %u: ";
  content_وتھ = get2byteNotZero(&انکڑے[hdr+5]);
  أكد( content_وتھ<=usable_ناپ );  /* Enforced by btreeInitPage() */

  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  nCell = get2byte(&انکڑے[hdr+3]);
  أكد( p_ورقہ->nCell==nCell );
  إذا( p_ورقہ->پتی || p_ورقہ->int_کلید==0 ){
    pCheck->n_صف += nCell;
  }

  /* EVIDENCE-OF: R-23882-45353 The cell pointer array of a b-tree page
  ** immediately follows the b-tree page header. */
  cell_شروع = hdr + 12 - 4*p_ورقہ->پتی;
  أكد( p_ورقہ->aCellIdx==&انکڑے[cell_شروع] );
  pCellIdx = &انکڑے[cell_شروع + 2*(nCell-1)];

  إذا( !p_ورقہ->پتی ){
    /* Analyze the right-child page of internal pages */
    pgno = get4byte(&انکڑے[hdr+8]);
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
    إذا( pBt->autoVacuum ){
      pCheck->zPfx = "Tree %u page %u right child: ";
      checkPtrmap(pCheck, pgno, PTRMAP_BTREE, i_ورقہ);
    }
#جے_انت
    ڈونگھائی = check_درخت_ورقہ(pCheck, pgno, &max_کلید, max_کلید);
    کلید_Can_Be_Equal = 0;
  }وإلا{
    /* For leaf pages, the coverage check will occur in the same loop
    ** as the other cell checks, so initialize the heap.  */
    ڈھیر = pCheck->ڈھیر;
    ڈھیر[0] = 0;
  }

  /* EVIDENCE-OF: R-02776-14802 The cell pointer array consists of K 2-byte
  ** integer offsets to the cell contents. */
  لكل(i=nCell-1; i>=0 && pCheck->mxErr; i--){
    CellInfo info;

    /* Check cell size */
    pCheck->v2 = i;
    أكد( pCellIdx==&انکڑے[cell_شروع + i*2] );
    pc = get2byteAligned(pCellIdx);
    pCellIdx -= 2;
    إذا( pc<content_وتھ || pc>usable_ناپ-4 ){
      check_Append_پیغام(pCheck, "Offset %u out of range %u..%u",
                             pc, content_وتھ, usable_ناپ-4);
      doCoverageCheck = 0;
      استمر;
    }
    pCell = &انکڑے[pc];
    p_ورقہ->xParseCell(p_ورقہ, pCell, &info);
    إذا( pc+info.n_ناپ>usable_ناپ ){
      check_Append_پیغام(pCheck, "Extends off end of page");
      doCoverageCheck = 0;
      استمر;
    }
    إذا( info.nPayload && info.pPayload[0]<2 ){
      check_Append_پیغام(pCheck, "Bad cell header size");
      doCoverageCheck = 0;
      استمر;
    }

    /* Check for integer primary key out of range */
    إذا( p_ورقہ->int_کلید ){
      إذا( کلید_Can_Be_Equal ? (info.n_کلید > max_کلید) : (info.n_کلید >= max_کلید) ){
        check_Append_پیغام(pCheck, "Rowid %lld out of order", info.n_کلید);
      }
      max_کلید = info.n_کلید;
      کلید_Can_Be_Equal = 0;     /* Only the first key on the page may ==maxKey */
    }

    /* Check the content overflow list */
    إذا( info.nPayload>info.nLocal ){
      u32 n_ورقہ;       /* Number of pages on the overflow chain */
      Pgno pgnoOvfl;   /* First page of the overflow chain */
      أكد( pc + info.n_ناپ - 4 <= usable_ناپ );
      n_ورقہ = (info.nPayload - info.nLocal + usable_ناپ - 5)/(usable_ناپ - 4);
      pgnoOvfl = get4byte(&pCell[info.n_ناپ - 4]);
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
      إذا( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgnoOvfl, PTRMAP_OVERFLOW1, i_ورقہ);
      }
#جے_انت
      checkList(pCheck, 0, pgnoOvfl, n_ورقہ);
    }

    إذا( !p_ورقہ->پتی ){
      /* Check sanity of left child page for internal pages */
      pgno = get4byte(pCell);
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
      إذا( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgno, PTRMAP_BTREE, i_ورقہ);
      }
#جے_انت
      d2 = check_درخت_ورقہ(pCheck, pgno, &max_کلید, max_کلید);
      کلید_Can_Be_Equal = 0;
      إذا( d2!=ڈونگھائی ){
        check_Append_پیغام(pCheck, "Child page depth differs");
        ڈونگھائی = d2;
      }
    }وإلا{
      /* Populate the coverage-checking heap for leaf pages */
      أكد( ڈھیر!=0 && ڈھیر[0] < pCheck->mx_ڈھیر );
      btree_ڈھیر_داخل_کرو(ڈھیر, (pc<<16)|(pc+info.n_ناپ-1));
    }
  }
  *pi_Min_کلید = max_کلید;

  /* Check for complete coverage of the page
  */
  pCheck->zPfx = 0;
  إذا( doCoverageCheck && pCheck->mxErr>0 ){
    /* For leaf pages, the min-heap has already been initialized and the
    ** cells have already been inserted.  But for internal pages, that has
    ** not yet been done, so do it now */
    إذا( !p_ورقہ->پتی ){
      ڈھیر = pCheck->ڈھیر;
      ڈھیر[0] = 0;
      لكل(i=nCell-1; i>=0; i--){
        u32 ناپ;
        pc = get2byteAligned(&انکڑے[cell_شروع+i*2]);
        ناپ = p_ورقہ->x_Cell_ناپ(p_ورقہ, &انکڑے[pc]);
        أكد( ڈھیر!=0 && ڈھیر[0] < pCheck->mx_ڈھیر );
        btree_ڈھیر_داخل_کرو(ڈھیر, (pc<<16)|(pc+ناپ-1));
      }
    }
    أكد( ڈھیر!=0 );
    /* Add the freeblocks to the min-heap
    **
    ** EVIDENCE-OF: R-20690-50594 The second field of the b-tree page header
    ** is the offset of the first freeblock, or zero if there are no
    ** freeblocks on the page.
    */
    i = get2byte(&انکڑے[hdr+1]);
    طالما( i>0 ){
      صحيح ناپ, j;
      أكد( (u32)i<=usable_ناپ-4 ); /* Enforced by btreeComputeFreeSpace() */
      ناپ = get2byte(&انکڑے[i+2]);
      أكد( (u32)(i+ناپ)<=usable_ناپ ); /* due to btreeComputeFreeSpace() */
      أكد( ڈھیر!=0 && ڈھیر[0] < pCheck->mx_ڈھیر );
      btree_ڈھیر_داخل_کرو(ڈھیر, (((u32)i)<<16)|(i+ناپ-1));
      /* EVIDENCE-OF: R-58208-19414 The first 2 bytes of a freeblock are a
      ** big-endian integer which is the offset in the b-tree page of the next
      ** freeblock in the chain, or zero if the freeblock is the last on the
      ** chain. */
      j = get2byte(&انکڑے[i]);
      /* EVIDENCE-OF: R-06866-39125 Freeblocks are always connected in order of
      ** increasing offset. */
      أكد( j==0 || j>i+ناپ );     /* Enforced by btreeComputeFreeSpace() */
      أكد( (u32)j<=usable_ناپ-4 ); /* Enforced by btreeComputeFreeSpace() */
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
    prev = content_وتھ - 1;   /* Implied first min-heap entry */
    طالما( btree_ڈھیر_Pull(ڈھیر,&x) ){
      إذا( (prev&0xffff)>=(x>>16) ){
        check_Append_پیغام(pCheck,
          "Multiple uses for byte %u of page %u", x>>16, i_ورقہ);
        اكسر;
      }وإلا{
        nFrag += (x>>16) - (prev&0xffff) - 1;
        prev = x;
      }
    }
    nFrag += usable_ناپ - (prev&0xffff) - 1;
    /* EVIDENCE-OF: R-43263-13491 The total number of bytes in all fragments
    ** is stored in the fifth field of the b-tree page header.
    ** EVIDENCE-OF: R-07161-27322 The one-byte integer at offset 7 gives the
    ** number of fragmented free bytes within the cell content area.
    */
    إذا( ڈھیر[0]==0 && nFrag!=انکڑے[hdr+7] ){
      check_Append_پیغام(pCheck,
          "Fragmentation of %u bytes reported as %u on page %u",
          nFrag, انکڑے[hdr+7], i_ورقہ);
    }
  }

سرا_دا_check:
  إذا( !doCoverageCheck ) p_ورقہ->isInit = savedIsInit;
  release_ورقہ(p_ورقہ);
  pCheck->zPfx = saved_zPfx;
  pCheck->v1 = saved_v1;
  pCheck->v2 = saved_v2;
  pCheck->nAbove--;
  أرجع ڈونگھائی+1;
}
#جے_انت /* SQLITE_OMIT_INTEGRITY_CHECK */

#جے_بنا_تعریف SQLITE_OMIT_INTEGRITY_CHECK
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
  Pgno *a_جڑ,  /* An array of root pages numbers for individual trees */
  Mem *aCnt,    /* Memory cells to write counts for each tree to */
  صحيح n_جڑ,    /* Number of entries in aRoot[] */
  صحيح mxErr,    /* Stop reporting errors after this many */
  صحيح *pnErr,   /* OUT: Write number of errors seen to this variable */
  حرف **pz_باہرلا  /* OUT: Write the error message string here */
){
  Pgno i;
  IntegrityCk sCheck;
  BtShared *pBt = p->pBt;
  u64 savedDbFlags = pBt->db->flags;
  حرف zErr[100];
  صحيح bPartial = 0;            /* True if not checking all btrees */
  صحيح bCkFreelist = 1;         /* True to scan the freelist */
  VVA_ONLY( صحيح nRef );

  أكد( n_جڑ>0 );
  أكد( aCnt!=0 );

  /* aRoot[0]==0 means this is a partial check */
  إذا( a_جڑ[0]==0 ){
    أكد( n_جڑ>1 );
    bPartial = 1;
    إذا( a_جڑ[1]!=1 ) bCkFreelist = 0;
  }

  sqlite3BtreeEnter(p);
  أكد( p->وچ_Trans>TRANS_NONE && pBt->وچ_Transaction>TRANS_NONE );
  VVA_ONLY( nRef = sqlite3PagerRefcount(pBt->pPager) );
  أكد( nRef>=0 );
  memset(&sCheck, 0, حجم(sCheck));
  sCheck.db = db;
  sCheck.pBt = pBt;
  sCheck.pPager = pBt->pPager;
  sCheck.n_Ck_ورقہ = btreePagecount(sCheck.pBt);
  sCheck.mxErr = mxErr;
  sqlite3StrAccumInit(&sCheck.err_پیغام, 0, zErr, حجم(zErr), SQLITE_MAX_لمائی);
  sCheck.err_پیغام.printfFlags = SQLITE_PRINTF_اندرونی;
  إذا( sCheck.n_Ck_ورقہ==0 ){
    اذهب integrity_ck_cleanup;
  }

  sCheck.aPgRef = sqlite3MallocZero((sCheck.n_Ck_ورقہ / 8)+ 1);
  إذا( !sCheck.aPgRef ){
    checkOom(&sCheck);
    اذهب integrity_ck_cleanup;
  }
  sCheck.ڈھیر = (u32*)sqlite3Malloc( pBt->ورقہ_ناپ*2 );
#جے_تعریف_شدہ SQLITE_DEBUG
  sCheck.mx_ڈھیر = pBt->ورقہ_ناپ/2 - 1;
#جے_انت
  إذا( sCheck.ڈھیر==0 ){
    checkOom(&sCheck);
    اذهب integrity_ck_cleanup;
  }

  i = PENDING_BYTE_ورقہ(pBt);
  إذا( i<=sCheck.n_Ck_ورقہ ) سیٹ_ورقہ_Referenced(&sCheck, i);

  /* Check the integrity of the freelist
  */
  إذا( bCkFreelist ){
    sCheck.zPfx = "Freelist: ";
    checkList(&sCheck, 1, get4byte(&pBt->pPage1->a_انکڑے[32]),
              get4byte(&pBt->pPage1->a_انکڑے[36]));
    sCheck.zPfx = 0;
  }

  /* Check all the tables.
  */
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
  إذا( !bPartial ){
    إذا( pBt->autoVacuum ){
      Pgno mx = 0;
      Pgno mx_وچ_Hdr;
      لكل(i=0; (صحيح)i<n_جڑ; i++) إذا( mx<a_جڑ[i] ) mx = a_جڑ[i];
      mx_وچ_Hdr = get4byte(&pBt->pPage1->a_انکڑے[52]);
      إذا( mx!=mx_وچ_Hdr ){
        check_Append_پیغام(&sCheck,
          "max rootpage (%u) disagrees with header (%u)",
          mx, mx_وچ_Hdr
        );
      }
    }وإلا إذا( get4byte(&pBt->pPage1->a_انکڑے[64])!=0 ){
      check_Append_پیغام(&sCheck,
        "incremental_vacuum enabled with a max rootpage of zero"
      );
    }
  }
#جے_انت
  testcase( pBt->db->flags & SQLITE_Cell_ناپ_Ck );
  pBt->db->flags &= ~(u64)SQLITE_Cell_ناپ_Ck;
  لكل(i=0; (صحيح)i<n_جڑ && sCheck.mxErr; i++){
    sCheck.n_صف = 0;
    إذا( a_جڑ[i] ){
      i64 not_ورتیا;
#جے_بنا_تعریف SQLITE_OMIT_AUTOVACUUM
      إذا( pBt->autoVacuum && a_جڑ[i]>1 && !bPartial ){
        checkPtrmap(&sCheck, a_جڑ[i], PTRMAP_ROOTPAGE, 0);
      }
#جے_انت
      sCheck.v0 = a_جڑ[i];
      أكد( sCheck.nAbove==0 );
      check_درخت_ورقہ(&sCheck, a_جڑ[i], &not_ورتیا, سبھ_توں_وڈا_INT64);
      أكد( sCheck.nAbove==0 );
    }
    sqlite3_Mem_سیٹ_Array_Int64(aCnt, i, sCheck.n_صف);
  }
  pBt->db->flags = savedDbFlags;

  /* Make sure every page in the file is referenced
  */
  إذا( !bPartial ){
    لكل(i=1; i<=sCheck.n_Ck_ورقہ && sCheck.mxErr; i++){
#جے_تعریف_شدہ SQLITE_OMIT_AUTOVACUUM
      إذا( get_ورقہ_Referenced(&sCheck, i)==0 ){
        check_Append_پیغام(&sCheck, "Page %u: never used", i);
      }
#وإلا
      /* If the database supports auto-vacuum, make sure no tables contain
      ** references to pointer-map pages.
      */
      إذا( get_ورقہ_Referenced(&sCheck, i)==0 &&
         (PTRMAP_PAGENO(pBt, i)!=i || !pBt->autoVacuum) ){
        check_Append_پیغام(&sCheck, "Page %u: never used", i);
      }
      إذا( get_ورقہ_Referenced(&sCheck, i)!=0 &&
         (PTRMAP_PAGENO(pBt, i)==i && pBt->autoVacuum) ){
        check_Append_پیغام(&sCheck, "Page %u: pointer map referenced", i);
      }
#جے_انت
    }
  }

  /* Clean  up and report errors.
  */
integrity_ck_cleanup:
  sqlite3_free(sCheck.ڈھیر);
  sqlite3_free(sCheck.aPgRef);
  *pnErr = sCheck.nErr;
  إذا( sCheck.nErr==0 ){
    sqlite3_str_reset(&sCheck.err_پیغام);
    *pz_باہرلا = 0;
  }وإلا{
    *pz_باہرلا = sqlite3_Str_Accum_انت(&sCheck.err_پیغام);
  }
  /* Make sure this analysis did not leave any unref() pages. */
  أكد( nRef==sqlite3PagerRefcount(pBt->pPager) );
  sqlite3BtreeLeave(p);
  أرجع sCheck.rc;
}
#جے_انت /* SQLITE_OMIT_INTEGRITY_CHECK */

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
  أرجع p ? p->وچ_Trans : 0;
}

#جے_بنا_تعریف SQLITE_OMIT_WAL
/*
** Run a checkpoint on the Btree passed as the first argument.
**
** Return SQLITE_LOCKED if this or any other connection has an open
** transaction on the shared-cache the argument Btree is connected to.
**
** Parameter eMode is one of SQLITE_CHECKPOINT_PASSIVE, FULL or RESTART.
*/
صحيح sqlite3BtreeCheckpoint(Btree *p, صحيح e_منوال, صحيح *pnLog, صحيح *pnCkpt){
  صحيح rc = SQLITE_OK;
  إذا( p ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
    إذا( pBt->وچ_Transaction!=TRANS_NONE ){
      rc = SQLITE_LOCKED;
    }وإلا{
      rc = sqlite3PagerCheckpoint(pBt->pPager, p->db, e_منوال, pnLog, pnCkpt);
    }
    sqlite3BtreeLeave(p);
  }
  أرجع rc;
}
#جے_انت

/*
** Return true if there is currently a backup running on Btree p.
*/
صحيح sqlite3_Btree_Is_وچ_Backup(Btree *p){
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
فراغ *sqlite3BtreeSchema(Btree *p, صحيح n_بائٹاں, فراغ(*xFree)(فراغ *)){
  BtShared *pBt = p->pBt;
  أكد( n_بائٹاں==0 || n_بائٹاں==حجم(Schema) );
  sqlite3BtreeEnter(p);
  إذا( !pBt->pSchema && n_بائٹاں ){
    pBt->pSchema = sqlite3DbMallocZero(0, n_بائٹاں);
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
  بن_ورتیا_PARAMETER(p);  /* only used in DEBUG builds */
  أكد( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  rc = query_Shared_Cache_جدول_جندرا(p, SCHEMA_جڑ, READ_جندرا);
  أكد( rc==SQLITE_OK || rc==SQLITE_LOCKED_SHAREDCACHE );
  sqlite3BtreeLeave(p);
  أرجع rc;
}


#جے_بنا_تعریف SQLITE_OMIT_SHARED_CACHE
/*
** Obtain a lock on the table whose root page is iTab.  The
** lock is a write lock if isWritelock is true or a read lock
** if it is false.
*/
صحيح sqlite3_Btree_جندرا_جدول(Btree *p, صحيح iTab, u8 is_Write_جندرا){
  صحيح rc = SQLITE_OK;
  أكد( p->وچ_Trans!=TRANS_NONE );
  إذا( p->sharable ){
    u8 جندرا_Type = READ_جندرا + is_Write_جندرا;
    أكد( READ_جندرا+1==WRITE_جندرا );
    أكد( is_Write_جندرا==0 || is_Write_جندرا==1 );

    sqlite3BtreeEnter(p);
    rc = query_Shared_Cache_جدول_جندرا(p, iTab, جندرا_Type);
    إذا( rc==SQLITE_OK ){
      rc = سیٹ_Shared_Cache_جدول_جندرا(p, iTab, جندرا_Type);
    }
    sqlite3BtreeLeave(p);
  }
  أرجع rc;
}
#جے_انت

#جے_بنا_تعریف SQLITE_OMIT_INCRBLOB
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
صحيح sqlite3_Btree_Put_انکڑے(BtCursor *pCsr, u32 وتھ, u32 amt, فراغ *z){
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
  VVA_ONLY(rc =) saveAllCursors(pCsr->pBt, pCsr->pgno_جڑ, pCsr);
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
              && pCsr->pBt->وچ_Transaction==TRANS_WRITE );
  أكد( has_Shared_Cache_جدول_جندرا(pCsr->pBtree, pCsr->pgno_جڑ, 0, 2) );
  أكد( !hasReadConflicts(pCsr->pBtree, pCsr->pgno_جڑ) );
  أكد( pCsr->p_ورقہ->int_کلید );

  أرجع accessPayload(pCsr, وتھ, amt, (بنانشانی حرف *)z, 1);
}

/*
** Mark this cursor as an incremental blob cursor.
*/
فراغ sqlite3BtreeIncrblobCursor(BtCursor *pCur){
  pCur->curFlags |= BTCF_Incrblob;
  pCur->pBtree->hasIncrblobCur = 1;
}
#جے_انت

/*
** Set both the "read version" (single byte at byte offset 18) and
** "write version" (single byte at byte offset 19) fields in the database
** header to iVersion.
*/
صحيح sqlite3_Btree_سیٹ_نسخہ(Btree *pBtree, صحيح i_نسخہ){
  BtShared *pBt = pBtree->pBt;
  صحيح rc;                         /* Return code */

  أكد( i_نسخہ==1 || i_نسخہ==2 );

  /* If setting the version fields to 1, do not automatically open the
  ** WAL connection, even if the version fields are currently set to 2.
  */
  pBt->btsFlags &= ~BTS_NO_WAL;
  إذا( i_نسخہ==1 ) pBt->btsFlags |= BTS_NO_WAL;

  rc = sqlite3BtreeBeginTrans(pBtree, 0, 0);
  إذا( rc==SQLITE_OK ){
    u8 *a_انکڑے = pBt->pPage1->a_انکڑے;
    إذا( a_انکڑے[18]!=(u8)i_نسخہ || a_انکڑے[19]!=(u8)i_نسخہ ){
      rc = sqlite3BtreeBeginTrans(pBtree, 2, 0);
      إذا( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->p_Db_ورقہ);
        إذا( rc==SQLITE_OK ){
          a_انکڑے[18] = (u8)i_نسخہ;
          a_انکڑے[19] = (u8)i_نسخہ;
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
صحيح sqlite3BtreeCursorHasHint(BtCursor *pCsr, بنانشانی صحيح نقاب){
  أرجع (pCsr->hints & نقاب)!=0;
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
صحيح sqlite3_سرخی_ناپ_Btree(فراغ){ أرجع ROUND8(حجم(Mem_ورقہ)); }

/*
** If no transaction is active and the database is not a temp-db, clear
** the in-memory pager cache.
*/
فراغ sqlite3BtreeClearCache(Btree *p){
  BtShared *pBt = p->pBt;
  إذا( pBt->وچ_Transaction==TRANS_NONE ){
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
صحيح sqlite3_Btree_Connection_گنتی(Btree *p){
  testcase( p->sharable );
  أرجع p->pBt->nRef;
}
#جے_انت

/* Re-enable GCC compiler warnings that were suppressed at the top
** of this source file to prevent annoying false-positives.
*/
#إذا defined(__GNUC__) && __GNUC__>=11
# pragma GCC diagnostic نکالو
#جے_انت
