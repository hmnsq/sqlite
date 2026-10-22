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
#అన్తర్భావ "btreeInt.h"

/*
** Suppress false-positive compiler warnings from GCC.  Warnings are
** re-enabled at the bottom of this source file.
*/
#ఒకవేళ defined(__GNUC__) && __GNUC__>=11
# pragma GCC diagnostic నెట్టు
# pragma GCC diagnostic ignored "-Wstringop-overread"
# pragma GCC diagnostic ignored "-Wstringop-overflow"
#అయితే_ముగింపు

/*
** The header string that appears at the beginning of every
** SQLite database.
*/
స్థిర స్థిరరాశి అక్షరం z_Magic_శీర్షిక[] = SQLITE_FILE_శీర్షిక;

/*
** Set this global variable to 1 to enable tracing using the TRACE
** macro.
*/
#ఒకవేళ 0
పూర్ణాంకం sqlite3BtreeTrace=1;  /* True to enable tracing */
# define TRACE(X)  ఒకవేళ(sqlite3BtreeTrace){రాయి X;fflush(stdout);}
#లేకపోతే
# define TRACE(X)
#అయితే_ముగింపు

/*
** Extract a 2-byte big-endian integer from an array of unsigned bytes.
** But if the value is zero, make it 65536.
**
** This routine is used to extract the "offset to cell content area" value
** from the header of a btree page.  If the page size is 65536 and the page
** is empty, the offset should be 65536, but the 2-byte value stores zero.
** This routine makes the necessary adjustment to 65536.
*/
#నిర్వచించు get2byteNotZero(X)  (((((పూర్ణాంకం)get2byte(X))-1)&0xffff)+1)

/*
** Values passed as the 5th argument to allocateBtreePage()
*/
#నిర్వచించు BTALLOC_ANY   0           /* Allocate any page */
#నిర్వచించు BTALLOC_EXACT 1           /* Allocate exact page if possible */
#నిర్వచించు BTALLOC_LE    2           /* Allocate any page <= the parameter */

/*
** Macro IfNotOmitAV(x) returns (x) if SQLITE_OMIT_AUTOVACUUM is not
** defined, or 0 if it is. For example:
**
**   bIncrVacuum = IfNotOmitAV(pBtShared->incrVacuum);
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
#నిర్వచించు IfNotOmitAV(సమాసం_2) (సమాసం_2)
#లేకపోతే
#నిర్వచించు IfNotOmitAV(సమాసం_2) 0
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
/*
** A list of BtShared objects that are eligible for participation
** in shared cache.  This variable has file scope during normal builds,
** but the test harness needs to access it so we make it global for
** test builds.
**
** Access to this variable is protected by SQLITE_MUTEX_STATIC_MAIN.
*/
#నిర్వచితమైతే SQLITE_TEST
BtShared *SQLITE_WSD sqlite3SharedCacheList = 0;
#లేకపోతే
స్థిర BtShared *SQLITE_WSD sqlite3SharedCacheList = 0;
#అయితే_ముగింపు
#అయితే_ముగింపు /* SQLITE_OMIT_SHARED_CACHE */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
/*
** Enable or disable the shared pager and schema features.
**
** This routine has no effect on existing database connections.
** The shared cache setting effects only future calls to
** sqlite3_open(), sqlite3_open16(), or sqlite3_open_v2().
*/
పూర్ణాంకం sqlite3_enable_shared_cache(పూర్ణాంకం enable){
  sqlite3GlobalConfig.sharedCacheEnabled = enable;
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు



#నిర్వచితమైతే SQLITE_OMIT_SHARED_CACHE
  /*
  ** The functions querySharedCacheTableLock(), setSharedCacheTableLock(),
  ** and clearAllSharedCacheTableLocks()
  ** manipulate entries in the BtShared.pLock linked list used to store
  ** shared-cache table level locks. If the library is compiled with the
  ** shared-cache feature disabled, then there is only ever one user
  ** of each BtShared structure and so this locking is not necessary.
  ** So define the lock related functions as no-ops.
  */
  #నిర్వచించు query_Shared_Cache_పట్టిక_తాళం(a,b,c) SQLITE_OK
  #నిర్వచించు సమితి_Shared_Cache_పట్టిక_తాళం(a,b,c) SQLITE_OK
  #నిర్వచించు clear_All_Shared_Cache_పట్టిక_Locks(a)
  #నిర్వచించు downgrade_All_Shared_Cache_పట్టిక_Locks(a)
  #నిర్వచించు has_Shared_Cache_పట్టిక_తాళం(a,b,c,d) 1
  #నిర్వచించు hasReadConflicts(a, b) 0
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Return and reset the seek counter for a Btree object.
*/
sqlite3_uint64 sqlite3_Btree_Seek_లెక్క(Btree *pBt){
  u64 n =  pBt->nSeek;
  pBt->nSeek = 0;
  తిరిగి n;
}
#అయితే_ముగింపు

/*
** Implementation of the SQLITE_CORRUPT_PAGE() macro. Takes a single
** (MemPage*) as an argument. The (MemPage*) must not be NULL.
**
** If SQLITE_DEBUG is not defined, then this macro is equivalent to
** SQLITE_CORRUPT_BKPT. Or, if SQLITE_DEBUG is set, then the log message
** normally produced as a side-effect of SQLITE_CORRUPT_BKPT is augmented
** with the page number and filename associated with the (MemPage*).
*/
#నిర్వచితమైతే SQLITE_DEBUG
పూర్ణాంకం corrupt_పుట_Error(పూర్ణాంకం lineno, Mem_పుట *p){
  అక్షరం *z_సందేశం;
  sqlite3BeginBenignMalloc();
  z_సందేశం = sqlite3_mprintf("database corruption page %u of %s",
             p->pgno, sqlite3PagerFilename(p->pBt->pPager, 0)
  );
  sqlite3_చివర_Benign_Malloc();
  ఒకవేళ( z_సందేశం ){
    sqlite3ReportError(SQLITE_CORRUPT, lineno, z_సందేశం);
  }
  sqlite3_free(z_సందేశం);
  తిరిగి SQLITE_CORRUPT_BKPT;
}
# define SQLITE_CORRUPT_పుట(p_Mem_పుట) corrupt_పుట_Error(__LINE__, p_Mem_పుట)
#లేకపోతే
# define SQLITE_CORRUPT_పుట(p_Mem_పుట) SQLITE_CORRUPT_PGNO(p_Mem_పుట->pgno)
#అయితే_ముగింపు

/* Default value for SHARED_LOCK_TRACE macro if shared-cache is disabled
** or if the lock tracking is disabled.  This is always the value for
** release builds.
*/
#నిర్వచించు SHARED_తాళం_TRACE(X,సందేశం,TAB,TYPE)  /*no-op*/

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE

#ఒకవేళ 0
/*  ^----  Change to 1 and recompile to enable shared-lock tracing
**         for debugging purposes.
**
** Print all shared-cache locks on a BtShared.  Debugging use only.
*/
స్థిర శూన్య shared_తాళం_Trace(
  BtShared *pBt,
  స్థిరరాశి అక్షరం *z_సందేశం,
  పూర్ణాంకం i_మూలం,
  పూర్ణాంకం e_తాళం_Type
){
  Bt_తాళం *p_తాళం;
  ఒకవేళ( i_మూలం>0 ){
    రాయి("%s-%p %u%s:", z_సందేశం, pBt, i_మూలం, e_తాళం_Type==READ_తాళం?"R":"W");
  }లేకపోతే{
    రాయి("%s-%p:", z_సందేశం, pBt);
  }
  కొరకు(p_తాళం=pBt->p_తాళం; p_తాళం; p_తాళం=p_తాళం->p_తదుపరి){
    రాయి(" %p/%u%s", p_తాళం->pBtree, p_తాళం->i_పట్టిక,
           p_తాళం->e_తాళం==READ_తాళం ? "R" : "W");
    అయితే( p_తాళం->p_తదుపరి && p_తాళం->pBtree==p_తాళం->p_తదుపరి->pBtree ){
      p_తాళం = p_తాళం->p_తదుపరి;
      రాయి(",%u%s", p_తాళం->i_పట్టిక, p_తాళం->e_తాళం==READ_తాళం ? "R" : "W");
    }
  }
  రాయి("\n");
  fflush(stdout);
}
#నిర్వచనం_రద్దు SHARED_తాళం_TRACE
#నిర్వచించు SHARED_తాళం_TRACE(X,సందేశం,TAB,TYPE)  shared_తాళం_Trace(X,సందేశం,TAB,TYPE)
#అయితే_ముగింపు /* Shared-lock tracing */

#నిర్వచితమైతే SQLITE_DEBUG
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
స్థిర పూర్ణాంకం has_Shared_Cache_పట్టిక_తాళం(
  Btree *pBtree,         /* Handle that must hold lock */
  Pgno i_మూలం,            /* Root page of b-tree */
  పూర్ణాంకం is_సూచీ,           /* True if iRoot is the root of an index b-tree */
  పూర్ణాంకం e_తాళం_Type          /* Required lock type (READ_LOCK or WRITE_LOCK) */
){
  Schema *pSchema = (Schema *)pBtree->pBt->pSchema;
  Pgno iTab = 0;
  Bt_తాళం *p_తాళం;

  /* If this database is not shareable, or if the client is reading
  ** and has the read-uncommitted flag set, then no lock is required.
  ** Return true immediately.
  */
  ఒకవేళ( (pBtree->sharable==0)
   || (e_తాళం_Type==READ_తాళం && (pBtree->db->flags & SQLITE_ReadUncommit))
  ){
    తిరిగి 1;
  }

  /* If the client is reading  or writing an index and the schema is
  ** not loaded, then it is too difficult to actually check to see if
  ** the correct locks are held.  So do not bother - just return true.
  ** This case does not come up very often anyhow.
  */
  ఒకవేళ( is_సూచీ && (!pSchema || (pSchema->schemaFlags&DB_SchemaLoaded)==0) ){
    తిరిగి 1;
  }

  /* Figure out the root-page that the lock should be held on. For table
  ** b-trees, this is just the root page of the b-tree being read or
  ** written. For index b-trees, it is the root page of the associated
  ** table.  */
  ఒకవేళ( is_సూచీ ){
    HashElem *p;
    పూర్ణాంకం b_చూసిన = 0;
    కొరకు(p=sqlite_Hash_మొదటి(&pSchema->idxHash); p; p=sqlite_Hash_తదుపరి(p)){
      సూచీ *pIdx = (సూచీ *)sqlite_Hash_దత్తాంశం(p);
      ఒకవేళ( pIdx->tnum==i_మూలం ){
        ఒకవేళ( b_చూసిన ){
          /* Two or more indexes share the same root page.  There must
          ** be imposter tables.  So just return true.  The assert is not
          ** useful in that case. */
          తిరిగి 1;
        }
        iTab = pIdx->p_పట్టిక->tnum;
        b_చూసిన = 1;
      }
    }
  }లేకపోతే{
    iTab = i_మూలం;
  }

  SHARED_తాళం_TRACE(pBtree->pBt,"hasLock",i_మూలం,e_తాళం_Type);

  /* Search for the required lock. Either a write-lock on root-page iTab, a
  ** write-lock on the schema table, or (if the client is reading) a
  ** read-lock on iTab will suffice. Return 1 if any of these are found.  */
  కొరకు(p_తాళం=pBtree->pBt->p_తాళం; p_తాళం; p_తాళం=p_తాళం->p_తదుపరి){
    ఒకవేళ( p_తాళం->pBtree==pBtree
     && (p_తాళం->i_పట్టిక==iTab || (p_తాళం->e_తాళం==WRITE_తాళం && p_తాళం->i_పట్టిక==1))
     && p_తాళం->e_తాళం>=e_తాళం_Type
    ){
      తిరిగి 1;
    }
  }

  /* Failed to find the required lock. */
  తిరిగి 0;
}
#అయితే_ముగింపు /* SQLITE_DEBUG */

#నిర్వచితమైతే SQLITE_DEBUG
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
స్థిర పూర్ణాంకం hasReadConflicts(Btree *pBtree, Pgno i_మూలం){
  BtCursor *p;
  కొరకు(p=pBtree->pBt->pCursor; p; p=p->p_తదుపరి){
    ఒకవేళ( p->pgno_మూలం==i_మూలం
     && p->pBtree!=pBtree
     && 0==(p->pBtree->db->flags & SQLITE_ReadUncommit)
    ){
      తిరిగి 1;
    }
  }
  తిరిగి 0;
}
#అయితే_ముగింపు    /* #ifdef SQLITE_DEBUG */

/*
** Query to see if Btree handle p may obtain a lock of type eLock
** (READ_LOCK or WRITE_LOCK) on the table with root-page iTab. Return
** SQLITE_OK if the lock may be obtained (by calling
** setSharedCacheTableLock()), or SQLITE_LOCKED if not.
*/
స్థిర పూర్ణాంకం query_Shared_Cache_పట్టిక_తాళం(Btree *p, Pgno iTab, u8 e_తాళం){
  BtShared *pBt = p->pBt;
  Bt_తాళం *pIter;

  ప్రతిజ్ఞా( sqlite3BtreeHoldsMutex(p) );
  ప్రతిజ్ఞా( e_తాళం==READ_తాళం || e_తాళం==WRITE_తాళం );
  ప్రతిజ్ఞా( p->db!=0 );
  ప్రతిజ్ఞా( !(p->db->flags&SQLITE_ReadUncommit)||e_తాళం==WRITE_తాళం||iTab==1 );
 
  /* If requesting a write-lock, then the Btree must have an open write
  ** transaction on this file. And, obviously, for this to be so there
  ** must be an open write transaction on the file itself.
  */
  ప్రతిజ్ఞా( e_తాళం==READ_తాళం || (p==pBt->pWriter && p->లోపల_Trans==TRANS_WRITE) );
  ప్రతిజ్ఞా( e_తాళం==READ_తాళం || pBt->లోపల_Transaction==TRANS_WRITE );
 
  /* This routine is a no-op if the shared-cache is not enabled */
  ఒకవేళ( !p->sharable ){
    తిరిగి SQLITE_OK;
  }

  /* If some other connection is holding an exclusive lock, the
  ** requested lock may not be obtained.
  */
  ఒకవేళ( pBt->pWriter!=p && (pBt->btsFlags & BTS_EXCLUSIVE)!=0 ){
    sqlite3ConnectionBlocked(p->db, pBt->pWriter->db);
    తిరిగి SQLITE_LOCKED_SHAREDCACHE;
  }

  కొరకు(pIter=pBt->p_తాళం; pIter; pIter=pIter->p_తదుపరి){
    /* The condition (pIter->eLock!=eLock) in the following if(...)
    ** statement is a simplification of:
    **
    **   (eLock==WRITE_LOCK || pIter->eLock==WRITE_LOCK)
    **
    ** since we know that if eLock==WRITE_LOCK, then no other connection
    ** may hold a WRITE_LOCK on any table in this file (since there can
    ** only be a single writer).
    */
    ప్రతిజ్ఞా( pIter->e_తాళం==READ_తాళం || pIter->e_తాళం==WRITE_తాళం );
    ప్రతిజ్ఞా( e_తాళం==READ_తాళం || pIter->pBtree==p || pIter->e_తాళం==READ_తాళం);
    ఒకవేళ( pIter->pBtree!=p && pIter->i_పట్టిక==iTab && pIter->e_తాళం!=e_తాళం ){
      sqlite3ConnectionBlocked(p->db, pIter->pBtree->db);
      ఒకవేళ( e_తాళం==WRITE_తాళం ){
        ప్రతిజ్ఞా( p==pBt->pWriter );
        pBt->btsFlags |= BTS_PENDING;
      }
      తిరిగి SQLITE_LOCKED_SHAREDCACHE;
    }
  }
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* !SQLITE_OMIT_SHARED_CACHE */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
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
స్థిర పూర్ణాంకం సమితి_Shared_Cache_పట్టిక_తాళం(Btree *p, Pgno i_పట్టిక, u8 e_తాళం){
  BtShared *pBt = p->pBt;
  Bt_తాళం *p_తాళం = 0;
  Bt_తాళం *pIter;

  SHARED_తాళం_TRACE(pBt,"setLock", i_పట్టిక, e_తాళం);

  ప్రతిజ్ఞా( sqlite3BtreeHoldsMutex(p) );
  ప్రతిజ్ఞా( e_తాళం==READ_తాళం || e_తాళం==WRITE_తాళం );
  ప్రతిజ్ఞా( p->db!=0 );

  /* A connection with the read-uncommitted flag set will never try to
  ** obtain a read-lock using this function. The only read-lock obtained
  ** by a connection in read-uncommitted mode is on the sqlite_schema
  ** table, and that lock is obtained in BtreeBeginTrans().  */
  ప్రతిజ్ఞా( 0==(p->db->flags&SQLITE_ReadUncommit) || e_తాళం==WRITE_తాళం );

  /* This function should only be called on a sharable b-tree after it
  ** has been determined that no other b-tree holds a conflicting lock.  */
  ప్రతిజ్ఞా( p->sharable );
  ప్రతిజ్ఞా( SQLITE_OK==query_Shared_Cache_పట్టిక_తాళం(p, i_పట్టిక, e_తాళం) );

  /* First search the list for an existing lock on this table. */
  కొరకు(pIter=pBt->p_తాళం; pIter; pIter=pIter->p_తదుపరి){
    ఒకవేళ( pIter->i_పట్టిక==i_పట్టిక && pIter->pBtree==p ){
      p_తాళం = pIter;
      ఆపు;
    }
  }

  /* If the above search did not find a BtLock struct associating Btree p
  ** with table iTable, allocate one and link it into the list.
  */
  ఒకవేళ( !p_తాళం ){
    p_తాళం = (Bt_తాళం *)sqlite3MallocZero(పరిమాణం(Bt_తాళం));
    ఒకవేళ( !p_తాళం ){
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    p_తాళం->i_పట్టిక = i_పట్టిక;
    p_తాళం->pBtree = p;
    p_తాళం->p_తదుపరి = pBt->p_తాళం;
    pBt->p_తాళం = p_తాళం;
  }

  /* Set the BtLock.eLock variable to the maximum of the current lock
  ** and the requested lock. This means if a write-lock was already held
  ** and a read-lock requested, we don't incorrectly downgrade the lock.
  */
  ప్రతిజ్ఞా( WRITE_తాళం>READ_తాళం );
  ఒకవేళ( e_తాళం>p_తాళం->e_తాళం ){
    p_తాళం->e_తాళం = e_తాళం;
  }

  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* !SQLITE_OMIT_SHARED_CACHE */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
/*
** Release all the table locks (locks obtained via calls to
** the setSharedCacheTableLock() procedure) held by Btree object p.
**
** This function assumes that Btree p has an open read or write
** transaction. If it does not, then the BTS_PENDING flag
** may be incorrectly cleared.
*/
స్థిర శూన్య clear_All_Shared_Cache_పట్టిక_Locks(Btree *p){
  BtShared *pBt = p->pBt;
  Bt_తాళం **ppIter = &pBt->p_తాళం;

  ప్రతిజ్ఞా( sqlite3BtreeHoldsMutex(p) );
  ప్రతిజ్ఞా( p->sharable || 0==*ppIter );
  ప్రతిజ్ఞా( p->లోపల_Trans>0 );

  SHARED_తాళం_TRACE(pBt, "clearAllLocks", 0, 0);

  అయితే( *ppIter ){
    Bt_తాళం *p_తాళం = *ppIter;
    ప్రతిజ్ఞా( (pBt->btsFlags & BTS_EXCLUSIVE)==0 || pBt->pWriter==p_తాళం->pBtree );
    ప్రతిజ్ఞా( p_తాళం->pBtree->లోపల_Trans>=p_తాళం->e_తాళం );
    ఒకవేళ( p_తాళం->pBtree==p ){
      *ppIter = p_తాళం->p_తదుపరి;
      ప్రతిజ్ఞా( p_తాళం->i_పట్టిక!=1 || p_తాళం==&p->తాళం );
      ఒకవేళ( p_తాళం->i_పట్టిక!=1 ){
        sqlite3_free(p_తాళం);
      }
    }లేకపోతే{
      ppIter = &p_తాళం->p_తదుపరి;
    }
  }

  ప్రతిజ్ఞా( (pBt->btsFlags & BTS_PENDING)==0 || pBt->pWriter );
  ఒకవేళ( pBt->pWriter==p ){
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(BTS_EXCLUSIVE|BTS_PENDING);
  }లేకపోతే ఒకవేళ( pBt->nTransaction==2 ){
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
స్థిర శూన్య downgrade_All_Shared_Cache_పట్టిక_Locks(Btree *p){
  BtShared *pBt = p->pBt;

  SHARED_తాళం_TRACE(pBt, "downgradeLocks", 0, 0);

  ఒకవేళ( pBt->pWriter==p ){
    Bt_తాళం *p_తాళం;
    pBt->pWriter = 0;
    pBt->btsFlags &= ~(BTS_EXCLUSIVE|BTS_PENDING);
    కొరకు(p_తాళం=pBt->p_తాళం; p_తాళం; p_తాళం=p_తాళం->p_తదుపరి){
      ప్రతిజ్ఞా( p_తాళం->e_తాళం==READ_తాళం || p_తాళం->pBtree==p );
      p_తాళం->e_తాళం = READ_తాళం;
    }
  }
}

#అయితే_ముగింపు /* SQLITE_OMIT_SHARED_CACHE */

స్థిర శూన్య release_పుట(Mem_పుట *p_పుట);         /* Forward reference */
స్థిర శూన్య release_పుట_One(Mem_పుట *p_పుట);      /* Forward reference */
స్థిర శూన్య release_పుట_Not_Null(Mem_పుట *p_పుట);  /* Forward reference */

/*
***** This routine is used inside of assert() only ****
**
** Verify that the cursor holds the mutex on its BtShared
*/
#నిర్వచితమైతే SQLITE_DEBUG
స్థిర పూర్ణాంకం cursorHoldsMutex(BtCursor *p){
  తిరిగి sqlite3_mutex_held(p->pBt->mutex);
}

/* Verify that the cursor and the BtShared agree about what is the current
** database connetion. This is important in shared-cache mode. If the database
** connection pointers get out-of-sync, it is possible for routines like
** btreeInitPage() to reference an stale connection pointer that references a
** a connection that has already closed.  This routine is used inside assert()
** statements only and for the purpose of double-checking that the btree code
** does keep the database connection pointers up-to-date.
*/
స్థిర పూర్ణాంకం cursorOwnsBtShared(BtCursor *p){
  ప్రతిజ్ఞా( cursorHoldsMutex(p) );
  తిరిగి (p->pBtree->db==p->pBt->db);
}
#అయితే_ముగింపు

/*
** Invalidate the overflow cache of the cursor passed as the first argument.
** on the shared btree structure pBt.
*/
#నిర్వచించు invalidate_అధికప్రవాహం_Cache(pCur) (pCur->curFlags &= ~BTCF_ValidOvfl)

/*
** Invalidate the overflow page-list cache for all cursors opened
** on the shared btree structure pBt.
*/
స్థిర శూన్య invalidate_All_అధికప్రవాహం_Cache(BtShared *pBt){
  BtCursor *p;
  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  కొరకు(p=pBt->pCursor; p; p=p->p_తదుపరి){
    invalidate_అధికప్రవాహం_Cache(p);
  }
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB
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
స్థిర శూన్య invalidateIncrblobCursors(
  Btree *pBtree,          /* The database file to check */
  Pgno pgno_మూలం,          /* The table that might be changing */
  i64 i_అడ్డువరుస,               /* The rowid that might be changing */
  పూర్ణాంకం is_Clear_పట్టిక        /* True if all rows are being deleted */
){
  BtCursor *p;
  ప్రతిజ్ఞా( pBtree->hasIncrblobCur );
  ప్రతిజ్ఞా( sqlite3BtreeHoldsMutex(pBtree) );
  pBtree->hasIncrblobCur = 0;
  కొరకు(p=pBtree->pBt->pCursor; p; p=p->p_తదుపరి){
    ఒకవేళ( (p->curFlags & BTCF_Incrblob)!=0 ){
      pBtree->hasIncrblobCur = 1;
      ఒకవేళ( p->pgno_మూలం==pgno_మూలం && (is_Clear_పట్టిక || p->info.n_కీలకం==i_అడ్డువరుస) ){
        p->eState = CURSOR_INVALID;
      }
    }
  }
}

#లేకపోతే
  /* Stub function when INCRBLOB is omitted */
  #నిర్వచించు invalidateIncrblobCursors(w,x,y,z)
#అయితే_ముగింపు /* SQLITE_OMIT_INCRBLOB */

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
స్థిర పూర్ణాంకం btree_సమితి_Has_Content(BtShared *pBt, Pgno pgno){
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( !pBt->pHasContent ){
    ప్రతిజ్ఞా( pgno<=pBt->n_పుట );
    pBt->pHasContent = sqlite3BitvecCreate(pBt->n_పుట);
    ఒకవేళ( !pBt->pHasContent ){
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  ఒకవేళ( rc==SQLITE_OK && pgno<=sqlite3_Bitvec_కొలత(pBt->pHasContent) ){
    rc = sqlite3_Bitvec_సమితి(pBt->pHasContent, pgno);
  }
  తిరిగి rc;
}

/*
** Query the BtShared.pHasContent vector.
**
** This function is called when a free-list leaf page is removed from the
** free-list for reuse. It returns false if it is safe to retrieve the
** page from the pager layer with the 'no-content' flag set. True otherwise.
*/
స్థిర పూర్ణాంకం btreeGetHasContent(BtShared *pBt, Pgno pgno){
  Bitvec *p = pBt->pHasContent;
  తిరిగి p && (pgno>sqlite3_Bitvec_కొలత(p) || sqlite3BitvecTestNotNull(p, pgno));
}

/*
** Clear (destroy) the BtShared.pHasContent bitvec. This should be
** invoked at the conclusion of each write-transaction.
*/
స్థిర శూన్య btreeClearHasContent(BtShared *pBt){
  sqlite3BitvecDestroy(pBt->pHasContent);
  pBt->pHasContent = 0;
}

/*
** Release all of the apPage[] pages for a cursor.
*/
స్థిర శూన్య btree_Release_All_Cursor_పుటలు(BtCursor *pCur){
  పూర్ణాంకం i;
  ఒకవేళ( pCur->i_పుట>=0 ){
    కొరకు(i=0; i<pCur->i_పుట; i++){
      release_పుట_Not_Null(pCur->ap_పుట[i]);
    }
    release_పుట_Not_Null(pCur->p_పుట);
    pCur->i_పుట = -1;
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
స్థిర పూర్ణాంకం save_Cursor_కీలకం(BtCursor *pCur){
  పూర్ణాంకం rc = SQLITE_OK;
  ప్రతిజ్ఞా( CURSOR_VALID==pCur->eState );
  ప్రతిజ్ఞా( 0==pCur->p_కీలకం );
  ప్రతిజ్ఞా( cursorHoldsMutex(pCur) );

  ఒకవేళ( pCur->cur_Int_కీలకం ){
    /* Only the rowid is required for a table btree */
    pCur->n_కీలకం = sqlite3_Btree_Integer_కీలకం(pCur);
  }లేకపోతే{
    /* For an index btree, save the complete key content. It is possible
    ** that the current key is corrupt. In that case, it is possible that
    ** the sqlite3VdbeRecordUnpack() function may overread the buffer by
    ** up to the size of 1 varint plus 1 8-byte value when the cursor
    ** position is restored. Hence the 17 bytes of padding allocated
    ** below. */
    శూన్య *p_కీలకం;
    pCur->n_కీలకం = sqlite3_Btree_Payload_కొలత(pCur);
    p_కీలకం = sqlite3Malloc( ((i64)pCur->n_కీలకం) + 9 + 8 );
    ఒకవేళ( p_కీలకం ){
      rc = sqlite3BtreePayload(pCur, 0, (పూర్ణాంకం)pCur->n_కీలకం, p_కీలకం);
      ఒకవేళ( rc==SQLITE_OK ){
        memset(((u8*)p_కీలకం)+pCur->n_కీలకం, 0, 9+8);
        pCur->p_కీలకం = p_కీలకం;
      }లేకపోతే{
        sqlite3_free(p_కీలకం);
      }
    }లేకపోతే{
      rc = SQLITE_NOMEM_BKPT;
    }
  }
  ప్రతిజ్ఞా( !pCur->cur_Int_కీలకం || !pCur->p_కీలకం );
  తిరిగి rc;
}

/*
** Save the current cursor position in the variables BtCursor.nKey
** and BtCursor.pKey. The cursor's state is set to CURSOR_REQUIRESEEK.
**
** The caller must ensure that the cursor is valid (has eState==CURSOR_VALID)
** prior to calling this routine. 
*/
స్థిర పూర్ణాంకం saveCursorPosition(BtCursor *pCur){
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( CURSOR_VALID==pCur->eState || CURSOR_SKIPNEXT==pCur->eState );
  ప్రతిజ్ఞా( 0==pCur->p_కీలకం );
  ప్రతిజ్ఞా( cursorHoldsMutex(pCur) );

  ఒకవేళ( pCur->curFlags & BTCF_Pinned ){
    తిరిగి SQLITE_CONSTRAINT_PINNED;
  }
  ఒకవేళ( pCur->eState==CURSOR_SKIPNEXT ){
    pCur->eState = CURSOR_VALID;
  }లేకపోతే{
    pCur->skip_తదుపరి = 0;
  }

  rc = save_Cursor_కీలకం(pCur);
  ఒకవేళ( rc==SQLITE_OK ){
    btree_Release_All_Cursor_పుటలు(pCur);
    pCur->eState = CURSOR_REQUIRESEEK;
  }

  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl|BTCF_At_చివరి);
  తిరిగి rc;
}

/* Forward reference */
స్థిర పూర్ణాంకం SQLITE_NOINLINE saveCursorsOnList(BtCursor*,Pgno,BtCursor*);

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
స్థిర పూర్ణాంకం saveAllCursors(BtShared *pBt, Pgno i_మూలం, BtCursor *pExcept){
  BtCursor *p;
  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ప్రతిజ్ఞా( pExcept==0 || pExcept->pBt==pBt );
  కొరకు(p=pBt->pCursor; p; p=p->p_తదుపరి){
    ఒకవేళ( p!=pExcept && (0==i_మూలం || p->pgno_మూలం==i_మూలం) ) ఆపు;
  }
  ఒకవేళ( p ) తిరిగి saveCursorsOnList(p, i_మూలం, pExcept);
  ఒకవేళ( pExcept ) pExcept->curFlags &= ~BTCF_గుణిజం;
  తిరిగి SQLITE_OK;
}

/* This helper routine to saveAllCursors does the actual work of saving
** the cursors if and when a cursor is found that actually requires saving.
** The common case is that no cursors need to be saved, so this routine is
** broken out from its caller to avoid unnecessary stack pointer movement.
*/
స్థిర పూర్ణాంకం SQLITE_NOINLINE saveCursorsOnList(
  BtCursor *p,         /* The first cursor that needs saving */
  Pgno i_మూలం,          /* Only save cursor with this iRoot. Save all if zero */
  BtCursor *pExcept    /* Do not save this cursor */
){
  చేయి{
    ఒకవేళ( p!=pExcept && (0==i_మూలం || p->pgno_మూలం==i_మూలం) ){
      ఒకవేళ( p->eState==CURSOR_VALID || p->eState==CURSOR_SKIPNEXT ){
        పూర్ణాంకం rc = saveCursorPosition(p);
        ఒకవేళ( SQLITE_OK!=rc ){
          తిరిగి rc;
        }
      }లేకపోతే{
        testcase( p->i_పుట>=0 );
        btree_Release_All_Cursor_పుటలు(p);
      }
    }
    p = p->p_తదుపరి;
  }అయితే( p );
  తిరిగి SQLITE_OK;
}

/*
** Clear the current cursor position.
*/
శూన్య sqlite3BtreeClearCursor(BtCursor *pCur){
  ప్రతిజ్ఞా( cursorHoldsMutex(pCur) );
  sqlite3_free(pCur->p_కీలకం);
  pCur->p_కీలకం = 0;
  pCur->eState = CURSOR_INVALID;
}

/*
** In this version of BtreeMoveto, pKey is a packed index record
** such as is generated by the OP_MakeRecord opcode.  Unpack the
** record and then call sqlite3BtreeIndexMoveto() to do the work.
*/
స్థిర పూర్ణాంకం btreeMoveto(
  BtCursor *pCur,     /* Cursor open on the btree to be searched */
  స్థిరరాశి శూన్య *p_కీలకం,   /* Packed key if the btree is an index */
  i64 n_కీలకం,           /* Integer key for tables.  Size of pKey for indices */
  పూర్ణాంకం bias,           /* Bias search to the high end */
  పూర్ణాంకం *pRes           /* Write search results here */
){
  పూర్ణాంకం rc;                    /* Status code */
  Unpacked_రికార్డు *p_Idx_కీలకం;   /* Unpacked index key */

  ఒకవేళ( p_కీలకం ){
    కీలకం_Info *p_కీలకం_Info = pCur->p_కీలకం_Info;
    ప్రతిజ్ఞా( n_కీలకం==(i64)(పూర్ణాంకం)n_కీలకం );
    p_Idx_కీలకం = sqlite3_Vdbe_Alloc_Unpacked_రికార్డు(p_కీలకం_Info);
    ఒకవేళ( p_Idx_కీలకం==0 ) తిరిగి SQLITE_NOMEM_BKPT;
    sqlite3_Vdbe_రికార్డు_Unpack((పూర్ణాంకం)n_కీలకం, p_కీలకం, p_Idx_కీలకం);
    ఒకవేళ( p_Idx_కీలకం->n_క్షేత్రం==0 || p_Idx_కీలకం->n_క్షేత్రం>p_కీలకం_Info->n_All_క్షేత్రం ){
      rc = SQLITE_CORRUPT_BKPT;
    }లేకపోతే{
      rc = sqlite3_Btree_సూచీ_Moveto(pCur, p_Idx_కీలకం, pRes);
    }
    sqlite3DbFree(pCur->p_కీలకం_Info->db, p_Idx_కీలకం);
  }లేకపోతే{
    p_Idx_కీలకం = 0;
    rc = sqlite3_Btree_పట్టిక_Moveto(pCur, n_కీలకం, bias, pRes);
  }
  తిరిగి rc;
}

/*
** Restore the cursor to the position it was in (or as close to as possible)
** when saveCursorPosition() was called. Note that this call deletes the
** saved position info stored by saveCursorPosition(), so there can be
** at most one effective restoreCursorPosition() call after each
** saveCursorPosition().
*/
స్థిర పూర్ణాంకం btreeRestoreCursorPosition(BtCursor *pCur){
  పూర్ణాంకం rc;
  పూర్ణాంకం skip_తదుపరి = 0;
  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( pCur->eState>=CURSOR_REQUIRESEEK );
  ఒకవేళ( pCur->eState==CURSOR_FAULT ){
    తిరిగి pCur->skip_తదుపరి;
  }
  pCur->eState = CURSOR_INVALID;
  ఒకవేళ( sqlite3FaultSim(410) ){
    rc = SQLITE_IOERR;
  }లేకపోతే{
    rc = btreeMoveto(pCur, pCur->p_కీలకం, pCur->n_కీలకం, 0, &skip_తదుపరి);
  }
  ఒకవేళ( rc==SQLITE_OK ){
    sqlite3_free(pCur->p_కీలకం);
    pCur->p_కీలకం = 0;
    ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID || pCur->eState==CURSOR_INVALID );
    ఒకవేళ( skip_తదుపరి ) pCur->skip_తదుపరి = skip_తదుపరి;
    ఒకవేళ( pCur->skip_తదుపరి && pCur->eState==CURSOR_VALID ){
      pCur->eState = CURSOR_SKIPNEXT;
    }
  }
  తిరిగి rc;
}

#నిర్వచించు restoreCursorPosition(p) \
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
పూర్ణాంకం sqlite3BtreeCursorHasMoved(BtCursor *pCur){
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(pCur)
       || pCur==sqlite3BtreeFakeValidCursor() );
  ప్రతిజ్ఞా( offsetof(BtCursor, eState)==0 );
  ప్రతిజ్ఞా( పరిమాణం(pCur->eState)==1 );
  తిరిగి CURSOR_VALID != *(u8*)pCur;
}

/*
** Return a pointer to a fake BtCursor object that will always answer
** false to the sqlite3BtreeCursorHasMoved() routine above.  The fake
** cursor returned must not be used with any other Btree interface.
*/
BtCursor *sqlite3BtreeFakeValidCursor(శూన్య){
  స్థిర u8 fakeCursor = CURSOR_VALID;
  ప్రతిజ్ఞా( offsetof(BtCursor, eState)==0 );
  తిరిగి (BtCursor*)&fakeCursor;
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
పూర్ణాంకం sqlite3BtreeCursorRestore(BtCursor *pCur, పూర్ణాంకం *p_Different_అడ్డువరుస){
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( pCur!=0 );
  ప్రతిజ్ఞా( pCur->eState!=CURSOR_VALID );
  rc = restoreCursorPosition(pCur);
  ఒకవేళ( rc ){
    *p_Different_అడ్డువరుస = 1;
    తిరిగి rc;
  }
  ఒకవేళ( pCur->eState!=CURSOR_VALID ){
    *p_Different_అడ్డువరుస = 1;
  }లేకపోతే{
    *p_Different_అడ్డువరుస = 0;
  }
  తిరిగి SQLITE_OK;
}

#నిర్వచితమైతే SQLITE_ENABLE_CURSOR_HINTS
/*
** Provide hints to the cursor.  The particular hint given (and the type
** and number of the varargs parameters) is determined by the eHintType
** parameter.  See the definitions of the BTREE_HINT_* macros for details.
*/
శూన్య sqlite3BtreeCursorHint(BtCursor *pCur, పూర్ణాంకం eHintType, ...){
  /* Used only by system that substitute their own storage engine */
#నిర్వచితమైతే SQLITE_DEBUG
  va_list ap;
  va_start(ap, eHintType);
  ఒకవేళ( eHintType==BTREE_HINT_RANGE ){
    సమాసం *p_సమాసం;
    Walker w;
    memset(&w, 0, పరిమాణం(w));
    w.x_సమాసం_Callback = sqlite3_Cursor_Range_Hint_సమాసం_Check;
    p_సమాసం = va_arg(ap, సమాసం*);
    w.u.aMem = va_arg(ap, Mem*);
    ప్రతిజ్ఞా( p_సమాసం!=0 );
    ప్రతిజ్ఞా( w.u.aMem!=0 );
    sqlite3_Walk_సమాసం(&w, p_సమాసం);
  }లేకపోతే ఒకవేళ( ALWAYS(eHintType==BTREE_HINT_TABLECURSOR) ){
    BtCursor *pCsr = va_arg(ap, BtCursor*);
    ప్రతిజ్ఞా( pCur->p_Cursor_Hint_పట్టిక_Cursor==0 
         || pCur->p_Cursor_Hint_పట్టిక_Cursor==pCsr
    );
    ప్రతిజ్ఞా( pCsr->p_కీలకం_Info==0 || CORRUPT_DB );
    pCur->p_Cursor_Hint_పట్టిక_Cursor = pCsr;
  }
  va_end(ap);
#అయితే_ముగింపు /* SQLITE_DEBUG */
}
#అయితే_ముగింపు /* SQLITE_ENABLE_CURSOR_HINTS */

#ఒకవేళ defined(SQLITE_ENABLE_CURSOR_HINTS) && defined(SQLITE_DEBUG)
/*
** Return the pointer configured via the BTREE_HINT_TABLECURSOR hint on
** cursor pCsr. This is used from OP_DeferredSeek to assert() that the
** index cursor has been correctly configured with the table cursor.
*/
BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor *pCsr){
  తిరిగి pCsr->p_Cursor_Hint_పట్టిక_Cursor;
}
#అయితే_ముగింపు

/*
** Provide flag hints to the cursor.
*/
శూన్య sqlite3BtreeCursorHintFlags(BtCursor *pCur, గుర్తులేని x){
  ప్రతిజ్ఞా( x==BTREE_SEEK_EQ || x==BTREE_BULKLOAD || x==0 );
  pCur->hints = (u8)x;
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
/*
** Given a page number of a regular database page, return the page
** number for the pointer-map page that contains the entry for the
** input page number.
**
** Return 0 (not a valid page) for pgno==1 since there is
** no pointer map associated with page 1.  The integrity_check logic
** requires that ptrmapPageno(*,1)!=1.
*/
స్థిర Pgno ptrmapPageno(BtShared *pBt, Pgno pgno){
  పూర్ణాంకం n_పుటలు_చొప్పున_Map_పుట;
  Pgno iPtrMap, ret;
  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ఒకవేళ( pgno<2 ) తిరిగి 0;
  n_పుటలు_చొప్పున_Map_పుట = (pBt->usable_కొలత/5)+1;
  iPtrMap = (pgno-2)/n_పుటలు_చొప్పున_Map_పుట;
  ret = (iPtrMap*n_పుటలు_చొప్పున_Map_పుట) + 2;
  ఒకవేళ( ret==PENDING_BYTE_పుట(pBt) ){
    ret++;
  }
  తిరిగి ret;
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
స్థిర శూన్య ptrmapPut(BtShared *pBt, Pgno కీలకం, u8 eType, Pgno మాతృ, పూర్ణాంకం *pRC){
  Db_పుట *p_Db_పుట;  /* The pointer map page */
  u8 *pPtrmap;      /* The pointer map data */
  Pgno iPtrmap;     /* The pointer map page number */
  పూర్ణాంకం స్థానభ్రంశం;       /* Offset in pointer map page */
  పూర్ణాంకం rc;           /* Return code from subfunctions */

  ఒకవేళ( *pRC ) తిరిగి;

  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  /* The super-journal page number must never be used as a pointer map page */
  ప్రతిజ్ఞా( 0==PTRMAP_ISPAGE(pBt, PENDING_BYTE_పుట(pBt)) );

  ప్రతిజ్ఞా( pBt->autoVacuum );
  ఒకవేళ( కీలకం==0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    తిరిగి;
  }
  iPtrmap = PTRMAP_PAGENO(pBt, కీలకం);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &p_Db_పుట, 0);
  ఒకవేళ( rc!=SQLITE_OK ){
    *pRC = rc;
    తిరిగి;
  }
  ఒకవేళ( ((అక్షరం*)sqlite3PagerGetExtra(p_Db_పుట))[0]!=0 ){
    /* The first byte of the extra data is the MemPage.isInit byte.
    ** If that byte is set, it means this page is also being used
    ** as a btree page. */
    *pRC = SQLITE_CORRUPT_BKPT;
    వెళ్ళు ptrmap_exit;
  }
  స్థానభ్రంశం = PTRMAP_PTROFFSET(iPtrmap, కీలకం);
  ఒకవేళ( స్థానభ్రంశం<0 ){
    *pRC = SQLITE_CORRUPT_BKPT;
    వెళ్ళు ptrmap_exit;
  }
  ప్రతిజ్ఞా( స్థానభ్రంశం <= (పూర్ణాంకం)pBt->usable_కొలత-5 );
  pPtrmap = (u8 *)sqlite3_Pager_Get_దత్తాంశం(p_Db_పుట);

  ఒకవేళ( eType!=pPtrmap[స్థానభ్రంశం] || get4byte(&pPtrmap[స్థానభ్రంశం+1])!=మాతృ ){
    TRACE(("PTRMAP_UPDATE: %u->(%u,%u)\n", కీలకం, eType, మాతృ));
    *pRC= rc = sqlite3PagerWrite(p_Db_పుట);
    ఒకవేళ( rc==SQLITE_OK ){
      pPtrmap[స్థానభ్రంశం] = eType;
      put4byte(&pPtrmap[స్థానభ్రంశం+1], మాతృ);
    }
  }

ptrmap_exit:
  sqlite3PagerUnref(p_Db_పుట);
}

/*
** Read an entry from the pointer map.
**
** This routine retrieves the pointer map entry for page 'key', writing
** the type and parent page number to *pEType and *pPgno respectively.
** An error code is returned if something goes wrong, otherwise SQLITE_OK.
*/
స్థిర పూర్ణాంకం ptrmapGet(BtShared *pBt, Pgno కీలకం, u8 *pEType, Pgno *pPgno){
  Db_పుట *p_Db_పుట;   /* The pointer map page */
  పూర్ణాంకం iPtrmap;       /* Pointer map page index */
  u8 *pPtrmap;       /* Pointer map page data */
  పూర్ణాంకం స్థానభ్రంశం;        /* Offset of entry in pointer map */
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );

  iPtrmap = PTRMAP_PAGENO(pBt, కీలకం);
  rc = sqlite3PagerGet(pBt->pPager, iPtrmap, &p_Db_పుట, 0);
  ఒకవేళ( rc!=0 ){
    తిరిగి rc;
  }
  pPtrmap = (u8 *)sqlite3_Pager_Get_దత్తాంశం(p_Db_పుట);

  స్థానభ్రంశం = PTRMAP_PTROFFSET(iPtrmap, కీలకం);
  ఒకవేళ( స్థానభ్రంశం<0 ){
    sqlite3PagerUnref(p_Db_పుట);
    తిరిగి SQLITE_CORRUPT_BKPT;
  }
  ప్రతిజ్ఞా( స్థానభ్రంశం <= (పూర్ణాంకం)pBt->usable_కొలత-5 );
  ప్రతిజ్ఞా( pEType!=0 );
  *pEType = pPtrmap[స్థానభ్రంశం];
  ఒకవేళ( pPgno ) *pPgno = get4byte(&pPtrmap[స్థానభ్రంశం+1]);

  sqlite3PagerUnref(p_Db_పుట);
  ఒకవేళ( *pEType<1 || *pEType>5 ) తిరిగి SQLITE_CORRUPT_PGNO(iPtrmap);
  తిరిగి SQLITE_OK;
}

#లేకపోతే /* if defined SQLITE_OMIT_AUTOVACUUM */
  #నిర్వచించు ptrmapPut(w,x,y,z,rc)
  #నిర్వచించు ptrmapGet(w,x,y,z) SQLITE_OK
  #నిర్వచించు ptrmapPutOvflPtr(x, y, z, rc)
#అయితే_ముగింపు

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
#నిర్వచించు కనుగొను_Cell(P,I) \
  ((P)->a_దత్తాంశం + ((P)->ముసుగు_పుట & get2byteAligned(&(P)->aCellIdx[2*(I)])))
#నిర్వచించు కనుగొను_Cell_Past_Ptr(P,I) \
  ((P)->a_దత్తాంశం_Ofst + ((P)->ముసుగు_పుట & get2byteAligned(&(P)->aCellIdx[2*(I)])))


/*
** This is common tail processing for btreeParseCellPtr() and
** btreeParseCellPtrIndex() for the case when the cell does not fit entirely
** on a single B-tree page.  Make necessary adjustments to the CellInfo
** structure.
*/
స్థిర SQLITE_NOINLINE శూన్య btree_Parse_Cell_Adjust_కొలత_For_అధికప్రవాహం(
  Mem_పుట *p_పుట,         /* Page containing the cell */
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
  పూర్ణాంకం minLocal;  /* Minimum amount of payload held locally */
  పూర్ణాంకం maxLocal;  /* Maximum amount of payload held locally */
  పూర్ణాంకం surplus;   /* Overflow payload available for local storage */

  minLocal = p_పుట->minLocal;
  maxLocal = p_పుట->maxLocal;
  surplus = minLocal + (pInfo->nPayload - minLocal)%(p_పుట->pBt->usable_కొలత-4);
  testcase( surplus==maxLocal );
  testcase( surplus==maxLocal+1 );
  ఒకవేళ( surplus <= maxLocal ){
    pInfo->nLocal = (u16)surplus;
  }లేకపోతే{
    pInfo->nLocal = (u16)minLocal;
  }
  pInfo->n_కొలత = (u16)(&pInfo->pPayload[pInfo->nLocal] - pCell) + 4;
}

/*
** Given a record with nPayload bytes of payload stored within btree
** page pPage, return the number of bytes of payload stored locally.
*/
స్థిర పూర్ణాంకం btree_Payload_గమ్యం_Local(Mem_పుట *p_పుట, i64 nPayload){
  పూర్ణాంకం maxLocal;  /* Maximum amount of payload held locally */
  maxLocal = p_పుట->maxLocal;
  ప్రతిజ్ఞా( nPayload>=0 );
  ఒకవేళ( nPayload<=maxLocal ){
    తిరిగి (పూర్ణాంకం)nPayload;
  }లేకపోతే{
    పూర్ణాంకం minLocal;  /* Minimum amount of payload held locally */
    పూర్ణాంకం surplus;   /* Overflow payload available for local storage */
    minLocal = p_పుట->minLocal;
    surplus = (పూర్ణాంకం)(minLocal +(nPayload - minLocal)%(p_పుట->pBt->usable_కొలత-4));
    తిరిగి (surplus <= maxLocal) ? surplus : minLocal;
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
స్థిర శూన్య btreeParseCellPtrNoPayload(
  Mem_పుట *p_పుట,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  ప్రతిజ్ఞా( p_పుట->ఆకు==0 );
  ప్రతిజ్ఞా( p_పుట->బిడ్డ_Ptr_కొలత==4 );
#నిర్వచితం_కాకపోతే SQLITE_DEBUG
  వాడని_PARAMETER(p_పుట);
#అయితే_ముగింపు
  pInfo->n_కొలత = 4 + sqlite3GetVarint(&pCell[4], (u64*)&pInfo->n_కీలకం);
  pInfo->nPayload = 0;
  pInfo->nLocal = 0;
  pInfo->pPayload = 0;
  తిరిగి;
}
స్థిర శూన్య btreeParseCellPtr(
  Mem_పుట *p_పుట,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u64 nPayload;           /* Number of bytes of cell payload */
  u64 i_కీలకం;               /* Extracted Key value */

  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  ప్రతిజ్ఞా( p_పుట->ఆకు==0 || p_పుట->ఆకు==1 );
  ప్రతిజ్ఞా( p_పుట->int_కీలకం_ఆకు );
  ప్రతిజ్ఞా( p_పుట->బిడ్డ_Ptr_కొలత==0 );
  pIter = pCell;

  /* The next block of code is equivalent to:
  **
  **     pIter += getVarint32(pIter, nPayload);
  **
  ** The code is inlined to avoid a function call.
  */
  nPayload = *pIter;
  ఒకవేళ( nPayload>=0x80 ){
    u8 *p_చివర = &pIter[8];
    nPayload &= 0x7f;
    చేయి{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }అయితే( (*pIter)>=0x80 && pIter<p_చివర );
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
  i_కీలకం = *pIter;
  ఒకవేళ( i_కీలకం>=0x80 ){
    u8 x;
    i_కీలకం = (i_కీలకం<<7) ^ (x = *++pIter);
    ఒకవేళ( x>=0x80 ){
      i_కీలకం = (i_కీలకం<<7) ^ (x = *++pIter);
      ఒకవేళ( x>=0x80 ){
        i_కీలకం = (i_కీలకం<<7) ^ 0x10204000 ^ (x = *++pIter);
        ఒకవేళ( x>=0x80 ){
          i_కీలకం = (i_కీలకం<<7) ^ 0x4000 ^ (x = *++pIter);
          ఒకవేళ( x>=0x80 ){
            i_కీలకం = (i_కీలకం<<7) ^ 0x4000 ^ (x = *++pIter);
            ఒకవేళ( x>=0x80 ){
              i_కీలకం = (i_కీలకం<<7) ^ 0x4000 ^ (x = *++pIter);
              ఒకవేళ( x>=0x80 ){
                i_కీలకం = (i_కీలకం<<7) ^ 0x4000 ^ (x = *++pIter);
                ఒకవేళ( x>=0x80 ){
                  i_కీలకం = (i_కీలకం<<8) ^ 0x8000 ^ (*++pIter);
                }
              }
            }
          }
        }
      }లేకపోతే{
        i_కీలకం ^= 0x204000;
      }
    }లేకపోతే{
      i_కీలకం ^= 0x4000;
    }
  }
  pIter++;

  pInfo->n_కీలకం = *(i64*)&i_కీలకం;
  pInfo->nPayload = (u32)nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==p_పుట->maxLocal );
  testcase( nPayload==(u32)p_పుట->maxLocal+1 );
  ప్రతిజ్ఞా( p_పుట->maxLocal <= BT_MAX_LOCAL );
  ఒకవేళ( nPayload<=p_పుట->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->n_కొలత = (u16)nPayload + (u16)(pIter - pCell);
    ఒకవేళ( pInfo->n_కొలత<4 ) pInfo->n_కొలత = 4;
    pInfo->nLocal = (u16)nPayload;
  }లేకపోతే{
    btree_Parse_Cell_Adjust_కొలత_For_అధికప్రవాహం(p_పుట, pCell, pInfo);
  }
}
స్థిర శూన్య btree_Parse_Cell_Ptr_సూచీ(
  Mem_పుట *p_పుట,         /* Page containing the cell */
  u8 *pCell,              /* Pointer to the cell text. */
  CellInfo *pInfo         /* Fill in this structure */
){
  u8 *pIter;              /* For scanning through pCell */
  u32 nPayload;           /* Number of bytes of cell payload */

  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  ప్రతిజ్ఞా( p_పుట->ఆకు==0 || p_పుట->ఆకు==1 );
  ప్రతిజ్ఞా( p_పుట->int_కీలకం_ఆకు==0 );
  pIter = pCell + p_పుట->బిడ్డ_Ptr_కొలత;
  nPayload = *pIter;
  ఒకవేళ( nPayload>=0x80 ){
    u8 *p_చివర = &pIter[8];
    nPayload &= 0x7f;
    చేయి{
      nPayload = (nPayload<<7) | (*++pIter & 0x7f);
    }అయితే( *(pIter)>=0x80 && pIter<p_చివర );
  }
  pIter++;
  pInfo->n_కీలకం = nPayload;
  pInfo->nPayload = nPayload;
  pInfo->pPayload = pIter;
  testcase( nPayload==p_పుట->maxLocal );
  testcase( nPayload==(u32)p_పుట->maxLocal+1 );
  ప్రతిజ్ఞా( nPayload>=0 );
  ప్రతిజ్ఞా( p_పుట->maxLocal <= BT_MAX_LOCAL );
  ఒకవేళ( nPayload<=p_పుట->maxLocal ){
    /* This is the (easy) common case where the entire payload fits
    ** on the local page.  No overflow is required.
    */
    pInfo->n_కొలత = (u16)nPayload + (u16)(pIter - pCell);
    ఒకవేళ( pInfo->n_కొలత<4 ) pInfo->n_కొలత = 4;
    pInfo->nLocal = (u16)nPayload;
  }లేకపోతే{
    btree_Parse_Cell_Adjust_కొలత_For_అధికప్రవాహం(p_పుట, pCell, pInfo);
  }
}
స్థిర శూన్య btreeParseCell(
  Mem_పుట *p_పుట,         /* Page containing the cell */
  పూర్ణాంకం iCell,              /* The cell index.  First cell is 0 */
  CellInfo *pInfo         /* Fill in this structure */
){
  p_పుట->xParseCell(p_పుట, కనుగొను_Cell(p_పుట, iCell), pInfo);
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
స్థిర u16 cell_కొలత_Ptr(Mem_పుట *p_పుట, u8 *pCell){
  u8 *pIter = pCell + 4;                   /* For looping over bytes of pCell */
  u8 *p_చివర;                                /* End mark for a varint */
  u32 n_కొలత;                               /* Size value to return */

#నిర్వచితమైతే SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_పుట->xParseCell(p_పుట, pCell, &debuginfo);
#అయితే_ముగింపు

  ప్రతిజ్ఞా( p_పుట->బిడ్డ_Ptr_కొలత==4 );
  n_కొలత = *pIter;
  ఒకవేళ( n_కొలత>=0x80 ){
    p_చివర = &pIter[8];
    n_కొలత &= 0x7f;
    చేయి{
      n_కొలత = (n_కొలత<<7) | (*++pIter & 0x7f);
    }అయితే( *(pIter)>=0x80 && pIter<p_చివర );
  }
  pIter++;
  testcase( n_కొలత==p_పుట->maxLocal );
  testcase( n_కొలత==(u32)p_పుట->maxLocal+1 );
  ఒకవేళ( n_కొలత<=p_పుట->maxLocal ){
    n_కొలత += (u32)(pIter - pCell);
    ప్రతిజ్ఞా( n_కొలత>4 );
  }లేకపోతే{
    పూర్ణాంకం minLocal = p_పుట->minLocal;
    n_కొలత = minLocal + (n_కొలత - minLocal) % (p_పుట->pBt->usable_కొలత - 4);
    testcase( n_కొలత==p_పుట->maxLocal );
    testcase( n_కొలత==(u32)p_పుట->maxLocal+1 );
    ఒకవేళ( n_కొలత>p_పుట->maxLocal ){
      n_కొలత = minLocal;
    }
    n_కొలత += 4 + (u16)(pIter - pCell);
  }
  ప్రతిజ్ఞా( n_కొలత==debuginfo.n_కొలత || CORRUPT_DB );
  తిరిగి (u16)n_కొలత;
}
స్థిర u16 cell_కొలత_Ptr_Idx_ఆకు(Mem_పుట *p_పుట, u8 *pCell){
  u8 *pIter = pCell;                       /* For looping over bytes of pCell */
  u8 *p_చివర;                                /* End mark for a varint */
  u32 n_కొలత;                               /* Size value to return */

#నిర్వచితమైతే SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_పుట->xParseCell(p_పుట, pCell, &debuginfo);
#అయితే_ముగింపు

  ప్రతిజ్ఞా( p_పుట->బిడ్డ_Ptr_కొలత==0 );
  n_కొలత = *pIter;
  ఒకవేళ( n_కొలత>=0x80 ){
    p_చివర = &pIter[8];
    n_కొలత &= 0x7f;
    చేయి{
      n_కొలత = (n_కొలత<<7) | (*++pIter & 0x7f);
    }అయితే( *(pIter)>=0x80 && pIter<p_చివర );
  }
  pIter++;
  testcase( n_కొలత==p_పుట->maxLocal );
  testcase( n_కొలత==(u32)p_పుట->maxLocal+1 );
  ఒకవేళ( n_కొలత<=p_పుట->maxLocal ){
    n_కొలత += (u32)(pIter - pCell);
    ఒకవేళ( n_కొలత<4 ) n_కొలత = 4;
  }లేకపోతే{
    పూర్ణాంకం minLocal = p_పుట->minLocal;
    n_కొలత = minLocal + (n_కొలత - minLocal) % (p_పుట->pBt->usable_కొలత - 4);
    testcase( n_కొలత==p_పుట->maxLocal );
    testcase( n_కొలత==(u32)p_పుట->maxLocal+1 );
    ఒకవేళ( n_కొలత>p_పుట->maxLocal ){
      n_కొలత = minLocal;
    }
    n_కొలత += 4 + (u16)(pIter - pCell);
  }
  ప్రతిజ్ఞా( n_కొలత==debuginfo.n_కొలత || CORRUPT_DB );
  తిరిగి (u16)n_కొలత;
}
స్థిర u16 cell_కొలత_Ptr_No_Payload(Mem_పుట *p_పుట, u8 *pCell){
  u8 *pIter = pCell + 4; /* For looping over bytes of pCell */
  u8 *p_చివర;              /* End mark for a varint */

#నిర్వచితమైతే SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_పుట->xParseCell(p_పుట, pCell, &debuginfo);
#లేకపోతే
  వాడని_PARAMETER(p_పుట);
#అయితే_ముగింపు

  ప్రతిజ్ఞా( p_పుట->బిడ్డ_Ptr_కొలత==4 );
  p_చివర = pIter + 9;
  అయితే( (*pIter++)&0x80 && pIter<p_చివర );
  ప్రతిజ్ఞా( debuginfo.n_కొలత==(u16)(pIter - pCell) || CORRUPT_DB );
  తిరిగి (u16)(pIter - pCell);
}
స్థిర u16 cell_కొలత_Ptr_పట్టిక_ఆకు(Mem_పుట *p_పుట, u8 *pCell){
  u8 *pIter = pCell;   /* For looping over bytes of pCell */
  u8 *p_చివర;            /* End mark for a varint */
  u32 n_కొలత;           /* Size value to return */

#నిర్వచితమైతే SQLITE_DEBUG
  /* The value returned by this function should always be the same as
  ** the (CellInfo.nSize) value found by doing a full parse of the
  ** cell. If SQLITE_DEBUG is defined, an assert() at the bottom of
  ** this function verifies that this invariant is not violated. */
  CellInfo debuginfo;
  p_పుట->xParseCell(p_పుట, pCell, &debuginfo);
#అయితే_ముగింపు

  n_కొలత = *pIter;
  ఒకవేళ( n_కొలత>=0x80 ){
    p_చివర = &pIter[8];
    n_కొలత &= 0x7f;
    చేయి{
      n_కొలత = (n_కొలత<<7) | (*++pIter & 0x7f);
    }అయితే( *(pIter)>=0x80 && pIter<p_చివర );
  }
  pIter++;
  /* pIter now points at the 64-bit integer key value, a variable length
  ** integer. The following block moves pIter to point at the first byte
  ** past the end of the key value. */
  ఒకవేళ( (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80
   && (*pIter++)&0x80 ){ pIter++; }
  testcase( n_కొలత==p_పుట->maxLocal );
  testcase( n_కొలత==(u32)p_పుట->maxLocal+1 );
  ఒకవేళ( n_కొలత<=p_పుట->maxLocal ){
    n_కొలత += (u32)(pIter - pCell);
    ఒకవేళ( n_కొలత<4 ) n_కొలత = 4;
  }లేకపోతే{
    పూర్ణాంకం minLocal = p_పుట->minLocal;
    n_కొలత = minLocal + (n_కొలత - minLocal) % (p_పుట->pBt->usable_కొలత - 4);
    testcase( n_కొలత==p_పుట->maxLocal );
    testcase( n_కొలత==(u32)p_పుట->maxLocal+1 );
    ఒకవేళ( n_కొలత>p_పుట->maxLocal ){
      n_కొలత = minLocal;
    }
    n_కొలత += 4 + (u16)(pIter - pCell);
  }
  ప్రతిజ్ఞా( n_కొలత==debuginfo.n_కొలత || CORRUPT_DB );
  తిరిగి (u16)n_కొలత;
}


#నిర్వచితమైతే SQLITE_DEBUG
/* This variation on cellSizePtr() is used inside of assert() statements
** only. */
స్థిర u16 cell_కొలత(Mem_పుట *p_పుట, పూర్ణాంకం iCell){
  తిరిగి p_పుట->x_Cell_కొలత(p_పుట, కనుగొను_Cell(p_పుట, iCell));
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
/*
** The cell pCell is currently part of page pSrc but will ultimately be part
** of pPage.  (pSrc and pPage are often the same.)  If pCell contains a
** pointer to an overflow page, insert an entry into the pointer-map for
** the overflow page that will be valid after pCell has been moved to pPage.
*/
స్థిర శూన్య ptrmapPutOvflPtr(Mem_పుట *p_పుట, Mem_పుట *pSrc, u8 *pCell,పూర్ణాంకం *pRC){
  CellInfo info;
  ఒకవేళ( *pRC ) తిరిగి;
  ప్రతిజ్ఞా( pCell!=0 );
  p_పుట->xParseCell(p_పుట, pCell, &info);
  ఒకవేళ( info.nLocal<info.nPayload ){
    Pgno ovfl;
    ఒకవేళ( SQLITE_అధికప్రవాహం(pSrc->a_దత్తాంశం_చివర, pCell, pCell+info.nLocal) ){
      testcase( pSrc!=p_పుట );
      *pRC = SQLITE_CORRUPT_BKPT;
      తిరిగి;
    }
    ovfl = get4byte(&pCell[info.n_కొలత-4]);
    ptrmapPut(p_పుట->pBt, ovfl, PTRMAP_OVERFLOW1, p_పుట->pgno, pRC);
  }
}
#అయితే_ముగింపు


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
స్థిర పూర్ణాంకం defragment_పుట(Mem_పుట *p_పుట, పూర్ణాంకం nMaxFrag){
  పూర్ణాంకం i;                     /* Loop counter */
  పూర్ణాంకం pc;                    /* Address of the i-th cell */
  పూర్ణాంకం hdr;                   /* Offset to the page header */
  పూర్ణాంకం కొలత;                  /* Size of a cell */
  పూర్ణాంకం usable_కొలత;            /* Number of usable bytes on a page */
  పూర్ణాంకం cell_స్థానభ్రంశం;            /* Offset to the cell pointer array */
  పూర్ణాంకం cbrk;                  /* Offset to the cell content area */
  పూర్ణాంకం nCell;                 /* Number of cells on the page */
  గుర్తులేని అక్షరం *దత్తాంశం;       /* The page data */
  గుర్తులేని అక్షరం *తాత్కాలిక;       /* Temp area for cell content */
  గుర్తులేని అక్షరం *src;        /* Source of content */
  పూర్ణాంకం i_Cell_మొదటి;            /* First allowable cell index */
  పూర్ణాంకం i_Cell_చివరి;             /* Last possible cell index */
  పూర్ణాంకం i_Cell_ప్రారంభం;            /* First cell offset in input */

  ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_పుట->p_Db_పుట) );
  ప్రతిజ్ఞా( p_పుట->pBt!=0 );
  ప్రతిజ్ఞా( p_పుట->pBt->usable_కొలత <= SQLITE_MAX_పుట_కొలత );
  ప్రతిజ్ఞా( p_పుట->n_అధికప్రవాహం==0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  దత్తాంశం = p_పుట->a_దత్తాంశం;
  hdr = p_పుట->hdr_స్థానభ్రంశం;
  cell_స్థానభ్రంశం = p_పుట->cell_స్థానభ్రంశం;
  nCell = p_పుట->nCell;
  ప్రతిజ్ఞా( nCell==get2byte(&దత్తాంశం[hdr+3]) || CORRUPT_DB );
  i_Cell_మొదటి = cell_స్థానభ్రంశం + 2*nCell;
  usable_కొలత = p_పుట->pBt->usable_కొలత;

  /* This block handles pages with two or fewer free blocks and nMaxFrag
  ** or fewer fragmented bytes. In this case it is faster to move the
  ** two (or one) blocks of cells using memmove() and add the required
  ** offsets to each pointer in the cell-pointer array than it is to
  ** reconstruct the entire page.  */
  ఒకవేళ( (పూర్ణాంకం)దత్తాంశం[hdr+7]<=nMaxFrag ){
    పూర్ణాంకం iFree = get2byte(&దత్తాంశం[hdr+1]);
    ఒకవేళ( iFree>usable_కొలత-4 ) తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    ఒకవేళ( iFree ){
      పూర్ణాంకం iFree2 = get2byte(&దత్తాంశం[iFree]);
      ఒకవేళ( iFree2>usable_కొలత-4 ) తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
      ఒకవేళ( 0==iFree2 || (దత్తాంశం[iFree2]==0 && దత్తాంశం[iFree2+1]==0) ){
        u8 *p_చివర = &దత్తాంశం[cell_స్థానభ్రంశం + nCell*2];
        u8 *pAddr;
        పూర్ణాంకం sz2 = 0;
        పూర్ణాంకం sz = get2byte(&దత్తాంశం[iFree+2]);
        పూర్ణాంకం శిఖరం = get2byte(&దత్తాంశం[hdr+5]);
        ఒకవేళ( శిఖరం>=iFree ){
          తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
        }
        ఒకవేళ( iFree2 ){
          ఒకవేళ( iFree+sz>iFree2 ) తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
          sz2 = get2byte(&దత్తాంశం[iFree2+2]);
          ఒకవేళ( iFree2+sz2 > usable_కొలత ) తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
          memmove(&దత్తాంశం[iFree+sz+sz2], &దత్తాంశం[iFree+sz], iFree2-(iFree+sz));
          sz += sz2;
        }లేకపోతే ఒకవేళ( iFree+sz>usable_కొలత ){
          తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
        }

        cbrk = శిఖరం+sz;
        ప్రతిజ్ఞా( cbrk+(iFree-శిఖరం) <= usable_కొలత );
        memmove(&దత్తాంశం[cbrk], &దత్తాంశం[శిఖరం], iFree-శిఖరం);
        కొరకు(pAddr=&దత్తాంశం[cell_స్థానభ్రంశం]; pAddr<p_చివర; pAddr+=2){
          pc = get2byte(pAddr);
          ఒకవేళ( pc<iFree ){ put2byte(pAddr, pc+sz); }
          లేకపోతే ఒకవేళ( pc<iFree2 ){ put2byte(pAddr, pc+sz2); }
        }
        వెళ్ళు defragment_బహిర్గతం;
      }
    }
  }

  cbrk = usable_కొలత;
  i_Cell_చివరి = usable_కొలత - 4;
  i_Cell_ప్రారంభం = get2byte(&దత్తాంశం[hdr+5]);
  ఒకవేళ( nCell>0 ){
    తాత్కాలిక = sqlite3_Pager_తాత్కాలిక_Space(p_పుట->pBt->pPager);
    జ్ఞాపకం_నకలు(తాత్కాలిక, దత్తాంశం, usable_కొలత);
    src = తాత్కాలిక;
    కొరకు(i=0; i<nCell; i++){
      u8 *pAddr;     /* The i-th cell pointer */
      pAddr = &దత్తాంశం[cell_స్థానభ్రంశం + i*2];
      pc = get2byte(pAddr);
      testcase( pc==i_Cell_మొదటి );
      testcase( pc==i_Cell_చివరి );
      /* These conditions have already been verified in btreeInitPage()
      ** if PRAGMA cell_size_check=ON.
      */
      ఒకవేళ( pc>i_Cell_చివరి ){
        తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
      }
      ప్రతిజ్ఞా( pc>=0 && pc<=i_Cell_చివరి );
      కొలత = p_పుట->x_Cell_కొలత(p_పుట, &src[pc]);
      cbrk -= కొలత;
      ఒకవేళ( cbrk<i_Cell_ప్రారంభం || pc+కొలత>usable_కొలత ){
        తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
      }
      ప్రతిజ్ఞా( cbrk+కొలత<=usable_కొలత && cbrk>=i_Cell_ప్రారంభం );
      testcase( cbrk+కొలత==usable_కొలత );
      testcase( pc+కొలత==usable_కొలత );
      put2byte(pAddr, cbrk);
      జ్ఞాపకం_నకలు(&దత్తాంశం[cbrk], &src[pc], కొలత);
    }
  }
  దత్తాంశం[hdr+7] = 0;

defragment_బహిర్గతం:
  ప్రతిజ్ఞా( p_పుట->nFree>=0 );
  ఒకవేళ( దత్తాంశం[hdr+7]+cbrk-i_Cell_మొదటి!=p_పుట->nFree ){
    తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
  }
  ప్రతిజ్ఞా( cbrk>=i_Cell_మొదటి );
  put2byte(&దత్తాంశం[hdr+5], cbrk);
  దత్తాంశం[hdr+1] = 0;
  దత్తాంశం[hdr+2] = 0;
  memset(&దత్తాంశం[i_Cell_మొదటి], 0, cbrk-i_Cell_మొదటి);
  ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_పుట->p_Db_పుట) );
  తిరిగి SQLITE_OK;
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
స్థిర u8 *పుట_కనుగొను_స్థానం(Mem_పుట *pPg, పూర్ణాంకం nByte, పూర్ణాంకం *pRc){
  స్థిరరాశి పూర్ణాంకం hdr = pPg->hdr_స్థానభ్రంశం;            /* Offset to page header */
  u8 * స్థిరరాశి a_దత్తాంశం = pPg->a_దత్తాంశం;             /* Page data */
  పూర్ణాంకం iAddr = hdr + 1;                       /* Address of ptr to pc */
  u8 *pTmp = &a_దత్తాంశం[iAddr];                  /* Temporary ptr into aData[] */
  పూర్ణాంకం pc = get2byte(pTmp);                   /* Address of a free slot */
  పూర్ణాంకం x;                                     /* Excess size of the slot */
  పూర్ణాంకం maxPC = pPg->pBt->usable_కొలత - nByte;  /* Max address for a usable slot */
  పూర్ణాంకం కొలత;                                  /* Size of the free slot */

  ప్రతిజ్ఞా( pc>0 );
  అయితే( pc<=maxPC ){
    /* EVIDENCE-OF: R-22710-53328 The third and fourth bytes of each
    ** freeblock form a big-endian integer which is the size of the freeblock
    ** in bytes, including the 4-byte header. */
    pTmp = &a_దత్తాంశం[pc+2];
    కొలత = get2byte(pTmp);
    ఒకవేళ( (x = కొలత - nByte)>=0 ){
      testcase( x==4 );
      testcase( x==3 );
      ఒకవేళ( x<4 ){
        /* EVIDENCE-OF: R-11498-58022 In a well-formed b-tree page, the total
        ** number of bytes in fragments may not exceed 60. */
        ఒకవేళ( a_దత్తాంశం[hdr+7]>57 ) తిరిగి 0;

        /* Remove the slot from the free-list. Update the number of
        ** fragmented bytes within the page. */
        జ్ఞాపకం_నకలు(&a_దత్తాంశం[iAddr], &a_దత్తాంశం[pc], 2);
        a_దత్తాంశం[hdr+7] += (u8)x;
        తిరిగి &a_దత్తాంశం[pc];
      }లేకపోతే ఒకవేళ( x+pc > maxPC ){
        /* This slot extends off the end of the usable part of the page */
        *pRc = SQLITE_CORRUPT_పుట(pPg);
        తిరిగి 0;
      }లేకపోతే{
        /* The slot remains on the free-list. Reduce its size to account
        ** for the portion used by the new allocation. */
        put2byte(&a_దత్తాంశం[pc+2], x);
      }
      తిరిగి &a_దత్తాంశం[pc + x];
    }
    iAddr = pc;
    pTmp = &a_దత్తాంశం[pc];
    pc = get2byte(pTmp);
    ఒకవేళ( pc<=iAddr ){
      ఒకవేళ( pc ){
        /* The next slot in the chain comes before the current slot */
        *pRc = SQLITE_CORRUPT_పుట(pPg);
      }
      తిరిగి 0;
    }
  }
  ఒకవేళ( pc>maxPC+nByte-4 ){
    /* The free slot chain extends off the end of the page */
    *pRc = SQLITE_CORRUPT_పుట(pPg);
  }
  తిరిగి 0;
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
స్థిర SQLITE_INLINE పూర్ణాంకం allocateSpace(Mem_పుట *p_పుట, పూర్ణాంకం nByte, పూర్ణాంకం *pIdx){
  స్థిరరాశి పూర్ణాంకం hdr = p_పుట->hdr_స్థానభ్రంశం;    /* Local cache of pPage->hdrOffset */
  u8 * స్థిరరాశి దత్తాంశం = p_పుట->a_దత్తాంశం;      /* Local cache of pPage->aData */
  పూర్ణాంకం శిఖరం;                             /* First byte of cell content area */
  పూర్ణాంకం rc = SQLITE_OK;                  /* Integer return code */
  u8 *pTmp;                            /* Temp ptr into data[] */
  పూర్ణాంకం gap;        /* First byte of gap between cell pointers and cell content */
 
  ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_పుట->p_Db_పుట) );
  ప్రతిజ్ఞా( p_పుట->pBt );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  ప్రతిజ్ఞా( nByte>=0 );  /* Minimum cell size is 4 */
  ప్రతిజ్ఞా( p_పుట->nFree>=nByte );
  ప్రతిజ్ఞా( p_పుట->n_అధికప్రవాహం==0 );
  ప్రతిజ్ఞా( nByte < (పూర్ణాంకం)(p_పుట->pBt->usable_కొలత-8) );

  ప్రతిజ్ఞా( p_పుట->cell_స్థానభ్రంశం == hdr + 12 - 4*p_పుట->ఆకు );
  gap = p_పుట->cell_స్థానభ్రంశం + 2*p_పుట->nCell;
  ప్రతిజ్ఞా( gap<=65536 );
  /* EVIDENCE-OF: R-29356-02391 If the database uses a 65536-byte page size
  ** and the reserved space is zero (the usual value for reserved space)
  ** then the cell content offset of an empty page wants to be 65536.
  ** However, that integer is too large to be stored in a 2-byte unsigned
  ** integer, so a value of 0 is used in its place. */
  pTmp = &దత్తాంశం[hdr+5];
  శిఖరం = get2byte(pTmp);
  ఒకవేళ( gap>శిఖరం ){
    ఒకవేళ( శిఖరం==0 && p_పుట->pBt->usable_కొలత==65536 ){
      శిఖరం = 65536;
    }లేకపోతే{
      తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    }
  }లేకపోతే ఒకవేళ( శిఖరం>(పూర్ణాంకం)p_పుట->pBt->usable_కొలత ){
    తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
  }

  /* If there is enough space between gap and top for one more cell pointer,
  ** and if the freelist is not empty, then search the
  ** freelist looking for a slot big enough to satisfy the request.
  */
  testcase( gap+2==శిఖరం );
  testcase( gap+1==శిఖరం );
  testcase( gap==శిఖరం );
  ఒకవేళ( (దత్తాంశం[hdr+2] || దత్తాంశం[hdr+1]) && gap+2<=శిఖరం ){
    u8 *pSpace = పుట_కనుగొను_స్థానం(p_పుట, nByte, &rc);
    ఒకవేళ( pSpace ){
      పూర్ణాంకం g2;
      ప్రతిజ్ఞా( pSpace+nByte<=దత్తాంశం+p_పుట->pBt->usable_కొలత );
      *pIdx = g2 = (పూర్ణాంకం)(pSpace-దత్తాంశం);
      ఒకవేళ( g2<=gap ){
        తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
      }లేకపోతే{
        తిరిగి SQLITE_OK;
      }
    }లేకపోతే ఒకవేళ( rc ){
      తిరిగి rc;
    }
  }

  /* The request could not be fulfilled using a freelist slot.  Check
  ** to see if defragmentation is necessary.
  */
  testcase( gap+2+nByte==శిఖరం );
  ఒకవేళ( gap+2+nByte>శిఖరం ){
    ప్రతిజ్ఞా( p_పుట->nCell>0 || CORRUPT_DB );
    ప్రతిజ్ఞా( p_పుట->nFree>=0 );
    rc = defragment_పుట(p_పుట, MIN(4, p_పుట->nFree - (2+nByte)));
    ఒకవేళ( rc ) తిరిగి rc;
    శిఖరం = get2byteNotZero(&దత్తాంశం[hdr+5]);
    ప్రతిజ్ఞా( gap+2+nByte<=శిఖరం );
  }


  /* Allocate memory from the gap in between the cell pointer array
  ** and the cell content area.  The btreeComputeFreeSpace() call has already
  ** validated the freelist.  Given that the freelist is valid, there
  ** is no way that the allocation can extend off the end of the page.
  ** The assert() below verifies the previous sentence.
  */
  శిఖరం -= nByte;
  put2byte(&దత్తాంశం[hdr+5], శిఖరం);
  ప్రతిజ్ఞా( శిఖరం+nByte <= (పూర్ణాంకం)p_పుట->pBt->usable_కొలత );
  *pIdx = శిఖరం;
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం freeSpace(Mem_పుట *p_పుట, పూర్ణాంకం i_ప్రారంభం, పూర్ణాంకం i_కొలత){
  పూర్ణాంకం iPtr;                             /* Address of ptr to next freeblock */
  పూర్ణాంకం iFreeBlk;                         /* Address of the next freeblock */
  u8 hdr;                               /* Page header size.  0 or 100 */
  పూర్ణాంకం nFrag = 0;                        /* Reduction in fragmentation */
  పూర్ణాంకం i_Orig_కొలత = i_కొలత;                /* Original value of iSize */
  పూర్ణాంకం x;                                /* Offset to cell content area */
  పూర్ణాంకం i_చివర = i_ప్రారంభం + i_కొలత;            /* First byte past the iStart buffer */
  గుర్తులేని అక్షరం *దత్తాంశం = p_పుట->a_దత్తాంశం;   /* Page content */
  u8 *pTmp;                             /* Temporary ptr into data[] */

  ప్రతిజ్ఞా( p_పుట->pBt!=0 );
  ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_పుట->p_Db_పుట) );
  ప్రతిజ్ఞా( CORRUPT_DB || i_ప్రారంభం>=p_పుట->hdr_స్థానభ్రంశం+6+p_పుట->బిడ్డ_Ptr_కొలత );
  ప్రతిజ్ఞా( CORRUPT_DB || i_చివర <= (పూర్ణాంకం)p_పుట->pBt->usable_కొలత );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  ప్రతిజ్ఞా( i_కొలత>=4 );   /* Minimum cell size is 4 */
  ప్రతిజ్ఞా( CORRUPT_DB || i_ప్రారంభం<=(పూర్ణాంకం)p_పుట->pBt->usable_కొలత-4 );

  /* The list of freeblocks must be in ascending order.  Find the
  ** spot on the list where iStart should be inserted.
  */
  hdr = p_పుట->hdr_స్థానభ్రంశం;
  iPtr = hdr + 1;
  ఒకవేళ( దత్తాంశం[iPtr+1]==0 && దత్తాంశం[iPtr]==0 ){
    iFreeBlk = 0;  /* Shortcut for the case when the freelist is empty */
  }లేకపోతే{
    అయితే( (iFreeBlk = get2byte(&దత్తాంశం[iPtr]))<i_ప్రారంభం ){
      ఒకవేళ( iFreeBlk<=iPtr ){
        ఒకవేళ( iFreeBlk==0 ) ఆపు; /* TH3: corrupt082.100 */
        తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
      }
      iPtr = iFreeBlk;
    }
    ఒకవేళ( iFreeBlk>(పూర్ణాంకం)p_పుట->pBt->usable_కొలత-4 ){ /* TH3: corrupt081.100 */
      తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    }
    ప్రతిజ్ఞా( iFreeBlk>iPtr || iFreeBlk==0 || CORRUPT_DB );
 
    /* At this point:
    **    iFreeBlk:   First freeblock after iStart, or zero if none
    **    iPtr:       The address of a pointer to iFreeBlk
    **
    ** Check to see if iFreeBlk should be coalesced onto the end of iStart.
    */
    ఒకవేళ( iFreeBlk && i_చివర+3>=iFreeBlk ){
      nFrag = iFreeBlk - i_చివర;
      ఒకవేళ( i_చివర>iFreeBlk ) తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
      i_చివర = iFreeBlk + get2byte(&దత్తాంశం[iFreeBlk+2]);
      ఒకవేళ( i_చివర > (పూర్ణాంకం)p_పుట->pBt->usable_కొలత ){
        తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
      }
      i_కొలత = i_చివర - i_ప్రారంభం;
      iFreeBlk = get2byte(&దత్తాంశం[iFreeBlk]);
    }
 
    /* If iPtr is another freeblock (that is, if iPtr is not the freelist
    ** pointer in the page header) then check to see if iStart should be
    ** coalesced onto the end of iPtr.
    */
    ఒకవేళ( iPtr>hdr+1 ){
      పూర్ణాంకం i_Ptr_చివర = iPtr + get2byte(&దత్తాంశం[iPtr+2]);
      ఒకవేళ( i_Ptr_చివర+3>=i_ప్రారంభం ){
        ఒకవేళ( i_Ptr_చివర>i_ప్రారంభం ) తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
        nFrag += i_ప్రారంభం - i_Ptr_చివర;
        i_కొలత = i_చివర - iPtr;
        i_ప్రారంభం = iPtr;
      }
    }
    ఒకవేళ( nFrag>దత్తాంశం[hdr+7] ) తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    దత్తాంశం[hdr+7] -= (u8)nFrag;
  }
  pTmp = &దత్తాంశం[hdr+5];
  x = get2byte(pTmp);
  ఒకవేళ( p_పుట->pBt->btsFlags & BTS_FAST_SECURE ){
    /* Overwrite deleted information with zeros when the secure_delete
    ** option is enabled */
    memset(&దత్తాంశం[i_ప్రారంభం], 0, i_కొలత);
  }
  ఒకవేళ( i_ప్రారంభం<=x ){
    /* The new freeblock is at the beginning of the cell content area,
    ** so just extend the cell content area rather than create another
    ** freelist entry */
    ఒకవేళ( i_ప్రారంభం<x ) తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    ఒకవేళ( iPtr!=hdr+1 ) తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    put2byte(&దత్తాంశం[hdr+1], iFreeBlk);
    put2byte(&దత్తాంశం[hdr+5], i_చివర);
  }లేకపోతే{
    /* Insert the new freeblock into the freelist */
    put2byte(&దత్తాంశం[iPtr], i_ప్రారంభం);
    put2byte(&దత్తాంశం[i_ప్రారంభం], iFreeBlk);
    ప్రతిజ్ఞా( i_కొలత>=0 && i_కొలత<=0xffff );
    put2byte(&దత్తాంశం[i_ప్రారంభం+2], (u16)i_కొలత);
  }
  p_పుట->nFree += i_Orig_కొలత;
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం decodeFlags(Mem_పుట *p_పుట, పూర్ణాంకం flagByte){
  BtShared *pBt;     /* A copy of pPage->pBt */

  ప్రతిజ్ఞా( p_పుట->hdr_స్థానభ్రంశం==(p_పుట->pgno==1 ? 100 : 0) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  pBt = p_పుట->pBt;
  p_పుట->max1bytePayload = pBt->max1bytePayload;
  ఒకవేళ( flagByte>=(PTF_ZERODATA | PTF_ఆకు) ){
    p_పుట->బిడ్డ_Ptr_కొలత = 0;
    p_పుట->ఆకు = 1;
    ఒకవేళ( flagByte==(PTF_LEAFDATA | PTF_INTKEY | PTF_ఆకు) ){
      p_పుట->int_కీలకం_ఆకు = 1;
      p_పుట->x_Cell_కొలత = cell_కొలత_Ptr_పట్టిక_ఆకు;
      p_పుట->xParseCell = btreeParseCellPtr;
      p_పుట->int_కీలకం = 1;
      p_పుట->maxLocal = pBt->max_ఆకు;
      p_పుట->minLocal = pBt->min_ఆకు;
    }లేకపోతే ఒకవేళ( flagByte==(PTF_ZERODATA | PTF_ఆకు) ){
      p_పుట->int_కీలకం = 0;
      p_పుట->int_కీలకం_ఆకు = 0;
      p_పుట->x_Cell_కొలత = cell_కొలత_Ptr_Idx_ఆకు;
      p_పుట->xParseCell = btree_Parse_Cell_Ptr_సూచీ;
      p_పుట->maxLocal = pBt->maxLocal;
      p_పుట->minLocal = pBt->minLocal;
    }లేకపోతే{
      p_పుట->int_కీలకం = 0;
      p_పుట->int_కీలకం_ఆకు = 0;
      p_పుట->x_Cell_కొలత = cell_కొలత_Ptr_Idx_ఆకు;
      p_పుట->xParseCell = btree_Parse_Cell_Ptr_సూచీ;
      తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    }
  }లేకపోతే{
    p_పుట->బిడ్డ_Ptr_కొలత = 4;
    p_పుట->ఆకు = 0;
    ఒకవేళ( flagByte==(PTF_ZERODATA) ){
      p_పుట->int_కీలకం = 0;
      p_పుట->int_కీలకం_ఆకు = 0;
      p_పుట->x_Cell_కొలత = cell_కొలత_Ptr;
      p_పుట->xParseCell = btree_Parse_Cell_Ptr_సూచీ;
      p_పుట->maxLocal = pBt->maxLocal;
      p_పుట->minLocal = pBt->minLocal;
    }లేకపోతే ఒకవేళ( flagByte==(PTF_LEAFDATA | PTF_INTKEY) ){
      p_పుట->int_కీలకం_ఆకు = 0;
      p_పుట->x_Cell_కొలత = cell_కొలత_Ptr_No_Payload;
      p_పుట->xParseCell = btreeParseCellPtrNoPayload;
      p_పుట->int_కీలకం = 1;
      p_పుట->maxLocal = pBt->max_ఆకు;
      p_పుట->minLocal = pBt->min_ఆకు;
    }లేకపోతే{
      p_పుట->int_కీలకం = 0;
      p_పుట->int_కీలకం_ఆకు = 0;
      p_పుట->x_Cell_కొలత = cell_కొలత_Ptr;
      p_పుట->xParseCell = btree_Parse_Cell_Ptr_సూచీ;
      తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    }
  }
  తిరిగి SQLITE_OK;
}

/*
** Compute the amount of freespace on the page.  In other words, fill
** in the pPage->nFree field.
*/
స్థిర పూర్ణాంకం btreeComputeFreeSpace(Mem_పుట *p_పుట){
  పూర్ణాంకం pc;            /* Address of a freeblock within pPage->aData[] */
  u8 hdr;            /* Offset to beginning of page header */
  u8 *దత్తాంశం;          /* Equal to pPage->aData */
  పూర్ణాంకం usable_కొలత;    /* Amount of usable space on each page */
  పూర్ణాంకం nFree;         /* Number of unused bytes on the page */
  పూర్ణాంకం శిఖరం;           /* First byte of the cell content area */
  పూర్ణాంకం i_Cell_మొదటి;    /* First allowable cell or freeblock offset */
  పూర్ణాంకం i_Cell_చివరి;     /* Last possible cell or freeblock offset */

  ప్రతిజ్ఞా( p_పుట->pBt!=0 );
  ప్రతిజ్ఞా( p_పుట->pBt->db!=0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  ప్రతిజ్ఞా( p_పుట->pgno==sqlite3PagerPagenumber(p_పుట->p_Db_పుట) );
  ప్రతిజ్ఞా( p_పుట == sqlite3PagerGetExtra(p_పుట->p_Db_పుట) );
  ప్రతిజ్ఞా( p_పుట->a_దత్తాంశం == sqlite3_Pager_Get_దత్తాంశం(p_పుట->p_Db_పుట) );
  ప్రతిజ్ఞా( p_పుట->isInit==1 );
  ప్రతిజ్ఞా( p_పుట->nFree<0 );

  usable_కొలత = p_పుట->pBt->usable_కొలత;
  hdr = p_పుట->hdr_స్థానభ్రంశం;
  దత్తాంశం = p_పుట->a_దత్తాంశం;
  /* EVIDENCE-OF: R-58015-48175 The two-byte integer at offset 5 designates
  ** the start of the cell content area. A zero value for this integer is
  ** interpreted as 65536. */
  శిఖరం = get2byteNotZero(&దత్తాంశం[hdr+5]);
  i_Cell_మొదటి = hdr + 8 + p_పుట->బిడ్డ_Ptr_కొలత + 2*p_పుట->nCell;
  i_Cell_చివరి = usable_కొలత - 4;

  /* Compute the total free space on the page
  ** EVIDENCE-OF: R-23588-34450 The two-byte integer at offset 1 gives the
  ** start of the first freeblock on the page, or is zero if there are no
  ** freeblocks. */
  pc = get2byte(&దత్తాంశం[hdr+1]);
  nFree = దత్తాంశం[hdr+7] + శిఖరం;  /* Init nFree to non-freeblock free space */
  ఒకవేళ( pc>0 ){
    u32 తదుపరి, కొలత;
    ఒకవేళ( pc<శిఖరం ){
      /* EVIDENCE-OF: R-55530-52930 In a well-formed b-tree page, there will
      ** always be at least one cell before the first freeblock.
      */
      తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    }
    అయితే( 1 ){
      ఒకవేళ( pc>i_Cell_చివరి ){
        /* Freeblock off the end of the page */
        తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
      }
      తదుపరి = get2byte(&దత్తాంశం[pc]);
      కొలత = get2byte(&దత్తాంశం[pc+2]);
      ఒకవేళ( కొలత<4 && sqlite3FaultSim(422)==SQLITE_OK ){
        /* Minimum freeblock size is 4.  Enable fault-sim 422 to disable this
        ** check to reach interesting error states.  However, disabling this
        ** check can cause assertion faults due to min-heap overflow.  All
        ** fault-sims are for testing use only, but this one especially so. */
        తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
      }
      nFree = nFree + కొలత;
      ఒకవేళ( తదుపరి<pc+కొలత+4 ) ఆపు;
      pc = తదుపరి;
    }
    ఒకవేళ( తదుపరి>0 ){
      /* Freeblock not in ascending order */
      తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    }
    ఒకవేళ( pc+కొలత>(గుర్తులేని పూర్ణాంకం)usable_కొలత ){
      /* Last freeblock extends past page end */
      తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    }
  }

  /* At this point, nFree contains the sum of the offset to the start
  ** of the cell-content area plus the number of free bytes within
  ** the cell-content area. If this is greater than the usable-size
  ** of the page, then the page must be corrupted. This check also
  ** serves to verify that the offset to the start of the cell-content
  ** area, according to the page header, lies within the page.
  */
  ఒకవేళ( nFree>usable_కొలత || nFree<i_Cell_మొదటి ){
    తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
  }
  p_పుట->nFree = (u16)(nFree - i_Cell_మొదటి);
  తిరిగి SQLITE_OK;
}

/*
** Do additional sanity check after btreeInitPage() if
** PRAGMA cell_size_check=ON
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం btree_Cell_కొలత_Check(Mem_పుట *p_పుట){
  పూర్ణాంకం i_Cell_మొదటి;    /* First allowable cell or freeblock offset */
  పూర్ణాంకం i_Cell_చివరి;     /* Last possible cell or freeblock offset */
  పూర్ణాంకం i;             /* Index into the cell pointer array */
  పూర్ణాంకం sz;            /* Size of a cell */
  పూర్ణాంకం pc;            /* Address of a freeblock within pPage->aData[] */
  u8 *దత్తాంశం;          /* Equal to pPage->aData */
  పూర్ణాంకం usable_కొలత;    /* Maximum usable space on the page */
  పూర్ణాంకం cell_స్థానభ్రంశం;    /* Start of cell content area */

  i_Cell_మొదటి = p_పుట->cell_స్థానభ్రంశం + 2*p_పుట->nCell;
  usable_కొలత = p_పుట->pBt->usable_కొలత;
  i_Cell_చివరి = usable_కొలత - 4;
  దత్తాంశం = p_పుట->a_దత్తాంశం;
  cell_స్థానభ్రంశం = p_పుట->cell_స్థానభ్రంశం;
  ఒకవేళ( !p_పుట->ఆకు ) i_Cell_చివరి--;
  కొరకు(i=0; i<p_పుట->nCell; i++){
    pc = get2byteAligned(&దత్తాంశం[cell_స్థానభ్రంశం+i*2]);
    testcase( pc==i_Cell_మొదటి );
    testcase( pc==i_Cell_చివరి );
    ఒకవేళ( pc<i_Cell_మొదటి || pc>i_Cell_చివరి ){
      తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    }
    sz = p_పుట->x_Cell_కొలత(p_పుట, &దత్తాంశం[pc]);
    testcase( pc+sz==usable_కొలత );
    ఒకవేళ( pc+sz>usable_కొలత ){
      తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    }
  }
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం btree_Init_పుట(Mem_పుట *p_పుట){
  u8 *దత్తాంశం;          /* Equal to pPage->aData */
  BtShared *pBt;        /* The main btree structure */

  ప్రతిజ్ఞా( p_పుట->pBt!=0 );
  ప్రతిజ్ఞా( p_పుట->pBt->db!=0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  ప్రతిజ్ఞా( p_పుట->pgno==sqlite3PagerPagenumber(p_పుట->p_Db_పుట) );
  ప్రతిజ్ఞా( p_పుట == sqlite3PagerGetExtra(p_పుట->p_Db_పుట) );
  ప్రతిజ్ఞా( p_పుట->a_దత్తాంశం == sqlite3_Pager_Get_దత్తాంశం(p_పుట->p_Db_పుట) );
  ప్రతిజ్ఞా( p_పుట->isInit==0 );

  pBt = p_పుట->pBt;
  దత్తాంశం = p_పుట->a_దత్తాంశం + p_పుట->hdr_స్థానభ్రంశం;
  /* EVIDENCE-OF: R-28594-02890 The one-byte flag at offset 0 indicating
  ** the b-tree page type. */
  ఒకవేళ( decodeFlags(p_పుట, దత్తాంశం[0]) ){
    తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
  }
  ప్రతిజ్ఞా( pBt->పుట_కొలత>=512 && pBt->పుట_కొలత<=65536 );
  p_పుట->ముసుగు_పుట = (u16)(pBt->పుట_కొలత - 1);
  p_పుట->n_అధికప్రవాహం = 0;
  p_పుట->cell_స్థానభ్రంశం = (u16)(p_పుట->hdr_స్థానభ్రంశం + 8 + p_పుట->బిడ్డ_Ptr_కొలత);
  p_పుట->aCellIdx = దత్తాంశం + p_పుట->బిడ్డ_Ptr_కొలత + 8;
  p_పుట->a_దత్తాంశం_చివర = p_పుట->a_దత్తాంశం + pBt->పుట_కొలత;
  p_పుట->a_దత్తాంశం_Ofst = p_పుట->a_దత్తాంశం + p_పుట->బిడ్డ_Ptr_కొలత;
  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  p_పుట->nCell = get2byte(&దత్తాంశం[3]);
  ఒకవేళ( p_పుట->nCell>MX_CELL(pBt) ){
    /* To many cells for a single page.  The page must be corrupt */
    తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
  }
  testcase( p_పుట->nCell==MX_CELL(pBt) );
  /* EVIDENCE-OF: R-24089-57979 If a page contains no cells (which is only
  ** possible for a root page of a table that contains no rows) then the
  ** offset to the cell content area will equal the page size minus the
  ** bytes of reserved space. */
  ప్రతిజ్ఞా( p_పుట->nCell>0
       || get2byteNotZero(&దత్తాంశం[5])==(పూర్ణాంకం)pBt->usable_కొలత
       || CORRUPT_DB );
  p_పుట->nFree = -1;  /* Indicate that this value is yet uncomputed */
  p_పుట->isInit = 1;
  ఒకవేళ( pBt->db->flags & SQLITE_Cell_కొలత_Ck ){
    తిరిగి btree_Cell_కొలత_Check(p_పుట);
  }
  తిరిగి SQLITE_OK;
}

/*
** Set up a raw page so that it looks like a database page holding
** no entries.
*/
స్థిర శూన్య zero_పుట(Mem_పుట *p_పుట, పూర్ణాంకం flags){
  గుర్తులేని అక్షరం *దత్తాంశం = p_పుట->a_దత్తాంశం;
  BtShared *pBt = p_పుట->pBt;
  పూర్ణాంకం hdr = p_పుట->hdr_స్థానభ్రంశం;
  పూర్ణాంకం మొదటి;

  ప్రతిజ్ఞా( sqlite3PagerPagenumber(p_పుట->p_Db_పుట)==p_పుట->pgno || CORRUPT_DB );
  ప్రతిజ్ఞా( sqlite3PagerGetExtra(p_పుట->p_Db_పుట) == (శూన్య*)p_పుట );
  ప్రతిజ్ఞా( sqlite3_Pager_Get_దత్తాంశం(p_పుట->p_Db_పుట) == దత్తాంశం );
  ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_పుట->p_Db_పుట) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ఒకవేళ( pBt->btsFlags & BTS_FAST_SECURE ){
    memset(&దత్తాంశం[hdr], 0, pBt->usable_కొలత - hdr);
  }
  దత్తాంశం[hdr] = (అక్షరం)flags;
  మొదటి = hdr + ((flags&PTF_ఆకు)==0 ? 12 : 8);
  memset(&దత్తాంశం[hdr+1], 0, 4);
  దత్తాంశం[hdr+7] = 0;
  put2byte(&దత్తాంశం[hdr+5], pBt->usable_కొలత);
  p_పుట->nFree = (u16)(pBt->usable_కొలత - మొదటి);
  decodeFlags(p_పుట, flags);
  p_పుట->cell_స్థానభ్రంశం = (u16)మొదటి;
  p_పుట->a_దత్తాంశం_చివర = &దత్తాంశం[pBt->పుట_కొలత];
  p_పుట->aCellIdx = &దత్తాంశం[మొదటి];
  p_పుట->a_దత్తాంశం_Ofst = &దత్తాంశం[p_పుట->బిడ్డ_Ptr_కొలత];
  p_పుట->n_అధికప్రవాహం = 0;
  ప్రతిజ్ఞా( pBt->పుట_కొలత>=512 && pBt->పుట_కొలత<=65536 );
  p_పుట->ముసుగు_పుట = (u16)(pBt->పుట_కొలత - 1);
  p_పుట->nCell = 0;
  p_పుట->isInit = 1;
}


/*
** Convert a DbPage obtained from the pager into a MemPage used by
** the btree layer.
*/
స్థిర Mem_పుట *btree_పుట_From_Db_పుట(Db_పుట *p_Db_పుట, Pgno pgno, BtShared *pBt){
  Mem_పుట *p_పుట = (Mem_పుట*)sqlite3PagerGetExtra(p_Db_పుట);
  ఒకవేళ( pgno!=p_పుట->pgno ){
    p_పుట->a_దత్తాంశం = sqlite3_Pager_Get_దత్తాంశం(p_Db_పుట);
    p_పుట->p_Db_పుట = p_Db_పుట;
    p_పుట->pBt = pBt;
    p_పుట->pgno = pgno;
    p_పుట->hdr_స్థానభ్రంశం = pgno==1 ? 100 : 0;
  }
  ప్రతిజ్ఞా( p_పుట->a_దత్తాంశం==sqlite3_Pager_Get_దత్తాంశం(p_Db_పుట) );
  తిరిగి p_పుట;
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
స్థిర పూర్ణాంకం btree_Get_పుట(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  Mem_పుట **pp_పుట,    /* Return the page in this parameter */
  పూర్ణాంకం flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  పూర్ణాంకం rc;
  Db_పుట *p_Db_పుట;

  ప్రతిజ్ఞా( flags==0 || flags==PAGER_GET_NOCONTENT || flags==PAGER_GET_READONLY );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  rc = sqlite3PagerGet(pBt->pPager, pgno, (Db_పుట**)&p_Db_పుట, flags);
  ఒకవేళ( rc ) తిరిగి rc;
  *pp_పుట = btree_పుట_From_Db_పుట(p_Db_పుట, pgno, pBt);
  తిరిగి SQLITE_OK;
}

/*
** Retrieve a page from the pager cache. If the requested page is not
** already in the pager cache return NULL. Initialize the MemPage.pBt and
** MemPage.aData elements if needed.
*/
స్థిర Mem_పుట *btree_పుట_Lookup(BtShared *pBt, Pgno pgno){
  Db_పుట *p_Db_పుట;
  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  p_Db_పుట = sqlite3PagerLookup(pBt->pPager, pgno);
  ఒకవేళ( p_Db_పుట ){
    తిరిగి btree_పుట_From_Db_పుట(p_Db_పుట, pgno, pBt);
  }
  తిరిగి 0;
}

/*
** Return the size of the database file in pages. If there is any kind of
** error, return ((unsigned int)-1).
*/
స్థిర Pgno btreePagecount(BtShared *pBt){
  తిరిగి pBt->n_పుట;
}
Pgno sqlite3_Btree_చివరి_పుట(Btree *p){
  ప్రతిజ్ఞా( sqlite3BtreeHoldsMutex(p) );
  తిరిగి btreePagecount(p->pBt);
}

/*
** Get a page from the pager and initialize it.
*/
స్థిర పూర్ణాంకం get_And_Init_పుట(
  BtShared *pBt,                  /* The database file */
  Pgno pgno,                      /* Number of the page to get */
  Mem_పుట **pp_పుట,               /* Write the page pointer here */
  పూర్ణాంకం bReadOnly                   /* True for a read-only page */
){
  పూర్ణాంకం rc;
  Db_పుట *p_Db_పుట;
  Mem_పుట *p_పుట;
  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );

  ఒకవేళ( pgno>btreePagecount(pBt) ){
    *pp_పుట = 0;
    తిరిగి SQLITE_CORRUPT_BKPT;
  }
  rc = sqlite3PagerGet(pBt->pPager, pgno, (Db_పుట**)&p_Db_పుట, bReadOnly);
  ఒకవేళ( rc ){
    *pp_పుట = 0;
    తిరిగి rc;
  }
  p_పుట = (Mem_పుట*)sqlite3PagerGetExtra(p_Db_పుట);
  ఒకవేళ( p_పుట->isInit==0 ){
    btree_పుట_From_Db_పుట(p_Db_పుట, pgno, pBt);
    rc = btree_Init_పుట(p_పుట);
    ఒకవేళ( rc!=SQLITE_OK ){
      release_పుట(p_పుట);
      *pp_పుట = 0;
      తిరిగి rc;
    }
  }
  ప్రతిజ్ఞా( p_పుట->pgno==pgno || CORRUPT_DB );
  ప్రతిజ్ఞా( p_పుట->a_దత్తాంశం==sqlite3_Pager_Get_దత్తాంశం(p_Db_పుట) );
  *pp_పుట = p_పుట;
  తిరిగి SQLITE_OK;
}

/*
** Release a MemPage.  This should be called once for each prior
** call to btreeGetPage.
**
** Page1 is a special case and must be released using releasePageOne().
*/
స్థిర శూన్య release_పుట_Not_Null(Mem_పుట *p_పుట){
  ప్రతిజ్ఞా( p_పుట->a_దత్తాంశం );
  ప్రతిజ్ఞా( p_పుట->pBt );
  ప్రతిజ్ఞా( p_పుట->p_Db_పుట!=0 );
  ప్రతిజ్ఞా( sqlite3PagerGetExtra(p_పుట->p_Db_పుట) == (శూన్య*)p_పుట );
  ప్రతిజ్ఞా( sqlite3_Pager_Get_దత్తాంశం(p_పుట->p_Db_పుట)==p_పుట->a_దత్తాంశం );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  sqlite3PagerUnrefNotNull(p_పుట->p_Db_పుట);
}
స్థిర శూన్య release_పుట(Mem_పుట *p_పుట){
  ఒకవేళ( p_పుట ) release_పుట_Not_Null(p_పుట);
}
స్థిర శూన్య release_పుట_One(Mem_పుట *p_పుట){
  ప్రతిజ్ఞా( p_పుట!=0 );
  ప్రతిజ్ఞా( p_పుట->a_దత్తాంశం );
  ప్రతిజ్ఞా( p_పుట->pBt );
  ప్రతిజ్ఞా( p_పుట->p_Db_పుట!=0 );
  ప్రతిజ్ఞా( sqlite3PagerGetExtra(p_పుట->p_Db_పుట) == (శూన్య*)p_పుట );
  ప్రతిజ్ఞా( sqlite3_Pager_Get_దత్తాంశం(p_పుట->p_Db_పుట)==p_పుట->a_దత్తాంశం );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  sqlite3_Pager_Unref_పుట_One(p_పుట->p_Db_పుట);
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
స్థిర పూర్ణాంకం btree_Get_వాడని_పుట(
  BtShared *pBt,       /* The btree */
  Pgno pgno,           /* Number of the page to fetch */
  Mem_పుట **pp_పుట,    /* Return the page in this parameter */
  పూర్ణాంకం flags            /* PAGER_GET_NOCONTENT or PAGER_GET_READONLY */
){
  పూర్ణాంకం rc = btree_Get_పుట(pBt, pgno, pp_పుట, flags);
  ఒకవేళ( rc==SQLITE_OK ){
    ఒకవేళ( sqlite3_Pager_పుట_Refcount((*pp_పుట)->p_Db_పుట)>1 ){
      release_పుట(*pp_పుట);
      *pp_పుట = 0;
      తిరిగి SQLITE_CORRUPT_BKPT;
    }
    (*pp_పుట)->isInit = 0;
  }లేకపోతే{
    *pp_పుట = 0;
  }
  తిరిగి rc;
}


/*
** During a rollback, when the pager reloads information into the cache
** so that the cache is restored to its original state at the start of
** the transaction, for each page restored this routine is called.
**
** This routine needs to reset the extra data section at the end of the
** page to agree with the restored data.
*/
స్థిర శూన్య పుట_Reinit(Db_పుట *p_దత్తాంశం){
  Mem_పుట *p_పుట;
  p_పుట = (Mem_పుట *)sqlite3PagerGetExtra(p_దత్తాంశం);
  ప్రతిజ్ఞా( sqlite3_Pager_పుట_Refcount(p_దత్తాంశం)>0 );
  ఒకవేళ( p_పుట->isInit ){
    ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
    p_పుట->isInit = 0;
    ఒకవేళ( sqlite3_Pager_పుట_Refcount(p_దత్తాంశం)>1 ){
      /* pPage might not be a btree page;  it might be an overflow page
      ** or ptrmap page or a free page.  In those cases, the following
      ** call to btreeInitPage() will likely return SQLITE_CORRUPT.
      ** But no harm is done by this.  And it is very important that
      ** btreeInitPage() be called on every btree page so we make
      ** the call for every page that comes in for re-initializing. */
      btree_Init_పుట(p_పుట);
    }
  }
}

/*
** Invoke the busy handler for a btree.
*/
స్థిర పూర్ణాంకం btreeInvokeBusyHandler(శూన్య *pArg){
  BtShared *pBt = (BtShared*)pArg;
  ప్రతిజ్ఞా( pBt->db );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->db->mutex) );
  తిరిగి sqlite3InvokeBusyHandler(&pBt->db->busyHandler);
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
పూర్ణాంకం sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,      /* VFS to use for this b-tree */
  స్థిరరాశి అక్షరం *zFilename,  /* Name of the file containing the BTree database */
  sqlite3 *db,            /* Associated database handle */
  Btree **ppBtree,        /* Pointer to new Btree object written here */
  పూర్ణాంకం flags,              /* Options */
  పూర్ణాంకం vfsFlags            /* Flags passed through to sqlite3_vfs.xOpen() */
){
  BtShared *pBt = 0;             /* Shared part of btree structure */
  Btree *p;                      /* Handle to return */
  sqlite3_mutex *mutexOpen = 0;  /* Prevents a race condition. Ticket #3537 */
  పూర్ణాంకం rc = SQLITE_OK;            /* Result code from this function */
  u8 nReserve;                   /* Byte of unused space on each page */
  గుర్తులేని అక్షరం z_Db_శీర్షిక[100];  /* Database header content */

  /* True if opening an ephemeral, temporary database */
  స్థిరరాశి పూర్ణాంకం is_తాత్కాలిక_Db = zFilename==0 || zFilename[0]==0;

  /* Set the variable isMemdb to true for an in-memory database, or
  ** false for a file-based database.
  */
#నిర్వచితమైతే SQLITE_OMIT_MEMORYDB
  స్థిరరాశి పూర్ణాంకం isMemdb = 0;
#లేకపోతే
  స్థిరరాశి పూర్ణాంకం isMemdb = (zFilename && strcmp(zFilename, ":memory:")==0)
                       || (is_తాత్కాలిక_Db && sqlite3_తాత్కాలిక_లోపల_జ్ఞాపకం(db))
                       || (vfsFlags & SQLITE_OPEN_జ్ఞాపకం)!=0;
#అయితే_ముగింపు

  ప్రతిజ్ఞా( db!=0 );
  ప్రతిజ్ఞా( pVfs!=0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(db->mutex) );
  ప్రతిజ్ఞా( (flags&0xff)==flags );   /* flags fit in 8 bits */

  /* Only a BTREE_SINGLE database can be BTREE_UNORDERED */
  ప్రతిజ్ఞా( (flags & BTREE_UNORDERED)==0 || (flags & BTREE_SINGLE)!=0 );

  /* A BTREE_SINGLE database is always a temporary and/or ephemeral */
  ప్రతిజ్ఞా( (flags & BTREE_SINGLE)==0 || is_తాత్కాలిక_Db );

  ఒకవేళ( isMemdb ){
    flags |= BTREE_జ్ఞాపకం;
  }
  ఒకవేళ( (vfsFlags & SQLITE_OPEN_MAIN_DB)!=0 && (isMemdb || is_తాత్కాలిక_Db) ){
    vfsFlags = (vfsFlags & ~SQLITE_OPEN_MAIN_DB) | SQLITE_OPEN_తాత్కాలిక_DB;
  }
  p = sqlite3MallocZero(పరిమాణం(Btree));
  ఒకవేళ( !p ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  p->లోపల_Trans = TRANS_NONE;
  p->db = db;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
  p->తాళం.pBtree = p;
  p->తాళం.i_పట్టిక = 1;
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
  /*
  ** If this Btree is a candidate for shared cache, try to find an
  ** existing BtShared object that we can share with
  */
  ఒకవేళ( is_తాత్కాలిక_Db==0 && (isMemdb==0 || (vfsFlags&SQLITE_OPEN_URI)!=0) ){
    ఒకవేళ( vfsFlags & SQLITE_OPEN_SHAREDCACHE ){
      పూర్ణాంకం nFilename = sqlite3Strlen30(zFilename)+1;
      పూర్ణాంకం n_నిండిన_Pathname = pVfs->mxPathname+1;
      అక్షరం *z_నిండిన_Pathname = sqlite3Malloc(MAX(n_నిండిన_Pathname,nFilename));
      MUTEX_LOGIC( sqlite3_mutex *mutexShared; )

      p->sharable = 1;
      ఒకవేళ( !z_నిండిన_Pathname ){
        sqlite3_free(p);
        తిరిగి SQLITE_NOMEM_BKPT;
      }
      ఒకవేళ( isMemdb ){
        జ్ఞాపకం_నకలు(z_నిండిన_Pathname, zFilename, nFilename);
      }లేకపోతే{
        rc = sqlite3_Os_నిండిన_Pathname(pVfs, zFilename,
                                   n_నిండిన_Pathname, z_నిండిన_Pathname);
        ఒకవేళ( rc ){
          ఒకవేళ( rc==SQLITE_OK_SYMLINK ){
            rc = SQLITE_OK;
          }లేకపోతే{
            sqlite3_free(z_నిండిన_Pathname);
            sqlite3_free(p);
            తిరిగి rc;
          }
        }
      }
#ఒకవేళ SQLITE_THREADSAFE
      mutexOpen = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_OPEN);
      sqlite3_mutex_enter(mutexOpen);
      mutexShared = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);
      sqlite3_mutex_enter(mutexShared);
#అయితే_ముగింపు
      కొరకు(pBt=GLOBAL(BtShared*,sqlite3SharedCacheList); pBt; pBt=pBt->p_తదుపరి){
        ప్రతిజ్ఞా( pBt->nRef>0 );
        ఒకవేళ( 0==strcmp(z_నిండిన_Pathname, sqlite3PagerFilename(pBt->pPager, 0))
                 && sqlite3PagerVfs(pBt->pPager)==pVfs ){
          పూర్ణాంకం iDb;
          కొరకు(iDb=db->nDb-1; iDb>=0; iDb--){
            Btree *pExisting = db->aDb[iDb].pBt;
            ఒకవేళ( pExisting && pExisting->pBt==pBt ){
              sqlite3_mutex_leave(mutexShared);
              sqlite3_mutex_leave(mutexOpen);
              sqlite3_free(z_నిండిన_Pathname);
              sqlite3_free(p);
              తిరిగి SQLITE_CONSTRAINT;
            }
          }
          p->pBt = pBt;
          pBt->nRef++;
          ఆపు;
        }
      }
      sqlite3_mutex_leave(mutexShared);
      sqlite3_free(z_నిండిన_Pathname);
    }
#నిర్వచితమైతే SQLITE_DEBUG
    లేకపోతే{
      /* In debug mode, we mark all persistent databases as sharable
      ** even when they are not.  This exercises the locking code and
      ** gives more opportunity for asserts(sqlite3_mutex_held())
      ** statements to find locking problems.
      */
      p->sharable = 1;
    }
#అయితే_ముగింపు
  }
#అయితే_ముగింపు
  ఒకవేళ( pBt==0 ){
    /*
    ** The following asserts make sure that structures used by the btree are
    ** the right size.  This is to guard against size changes that result
    ** when compiling on a different architecture.
    */
    ప్రతిజ్ఞా( పరిమాణం(i64)==8 );
    ప్రతిజ్ఞా( పరిమాణం(u64)==8 );
    ప్రతిజ్ఞా( పరిమాణం(u32)==4 );
    ప్రతిజ్ఞా( పరిమాణం(u16)==2 );
    ప్రతిజ్ఞా( పరిమాణం(Pgno)==4 );

    /* Suppress false-positive compiler warning from PVS-Studio */
    memset(&z_Db_శీర్షిక[16], 0, 8);
 
    pBt = sqlite3MallocZero( పరిమాణం(*pBt) );
    ఒకవేళ( pBt==0 ){
      rc = SQLITE_NOMEM_BKPT;
      వెళ్ళు btree_open_బహిర్గతం;
    }
    rc = sqlite3PagerOpen(pVfs, &pBt->pPager, zFilename,
                          పరిమాణం(Mem_పుట), flags, vfsFlags, పుట_Reinit);
    ఒకవేళ( rc==SQLITE_OK ){
      sqlite3_Pager_సమితి_Mmap_హద్దు(pBt->pPager, db->szMmap);
      rc = sqlite3PagerReadFileheader(pBt->pPager,పరిమాణం(z_Db_శీర్షిక),z_Db_శీర్షిక);
    }
    ఒకవేళ( rc!=SQLITE_OK ){
      వెళ్ళు btree_open_బహిర్గతం;
    }
    pBt->openFlags = (u8)flags;
    pBt->db = db;
    sqlite3_Pager_సమితి_Busy_Handler(pBt->pPager, btreeInvokeBusyHandler, pBt);
    p->pBt = pBt;
 
    pBt->pCursor = 0;
    pBt->pPage1 = 0;
    ఒకవేళ( sqlite3PagerIsreadonly(pBt->pPager) ) pBt->btsFlags |= BTS_READ_ONLY;
#ఒకవేళ defined(SQLITE_SECURE_DELETE)
    pBt->btsFlags |= BTS_SECURE_DELETE;
#లేదావేళ defined(SQLITE_FAST_SECURE_DELETE)
    pBt->btsFlags |= BTS_OVERWRITE;
#అయితే_ముగింపు
    /* EVIDENCE-OF: R-51873-39618 The page size for a database file is
    ** determined by the 2-byte integer located at an offset of 16 bytes from
    ** the beginning of the database file. */
    pBt->పుట_కొలత = (z_Db_శీర్షిక[16]<<8) | (z_Db_శీర్షిక[17]<<16);
    ఒకవేళ( pBt->పుట_కొలత<512 || pBt->పుట_కొలత>SQLITE_MAX_పుట_కొలత
         || ((pBt->పుట_కొలత-1)&pBt->పుట_కొలత)!=0 ){
      pBt->పుట_కొలత = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
      /* If the magic name ":memory:" will create an in-memory database, then
      ** leave the autoVacuum mode at 0 (do not auto-vacuum), even if
      ** SQLITE_DEFAULT_AUTOVACUUM is true. On the other hand, if
      ** SQLITE_OMIT_MEMORYDB has been defined, then ":memory:" is just a
      ** regular file-name. In this case the auto-vacuum applies as per normal.
      */
      ఒకవేళ( zFilename && !isMemdb ){
        pBt->autoVacuum = (SQLITE_DEFAULT_AUTOVACUUM ? 1 : 0);
        pBt->incrVacuum = (SQLITE_DEFAULT_AUTOVACUUM==2 ? 1 : 0);
      }
#అయితే_ముగింపు
      nReserve = 0;
    }లేకపోతే{
      /* EVIDENCE-OF: R-37497-42412 The size of the reserved region is
      ** determined by the one-byte unsigned integer found at an offset of 20
      ** into the database file header. */
      nReserve = z_Db_శీర్షిక[20];
      pBt->btsFlags |= BTS_PAGESIZE_FIXED;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
      pBt->autoVacuum = (get4byte(&z_Db_శీర్షిక[36 + 4*4])?1:0);
      pBt->incrVacuum = (get4byte(&z_Db_శీర్షిక[36 + 7*4])?1:0);
#అయితే_ముగింపు
    }
    rc = sqlite3_Pager_సమితి_Pagesize(pBt->pPager, &pBt->పుట_కొలత, nReserve);
    ఒకవేళ( rc ) వెళ్ళు btree_open_బహిర్గతం;
    pBt->usable_కొలత = pBt->పుట_కొలత - nReserve;
    ప్రతిజ్ఞా( (pBt->పుట_కొలత & 7)==0 );  /* 8-byte alignment of pageSize */
  
#ఒకవేళ !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
    /* Add the new BtShared object to the linked list sharable BtShareds.
    */
    pBt->nRef = 1;
    ఒకవేళ( p->sharable ){
      MUTEX_LOGIC( sqlite3_mutex *mutexShared; )
      MUTEX_LOGIC( mutexShared = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN);)
      ఒకవేళ( SQLITE_THREADSAFE && sqlite3GlobalConfig.bCoreMutex ){
        pBt->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_FAST);
        ఒకవేళ( pBt->mutex==0 ){
          rc = SQLITE_NOMEM_BKPT;
          వెళ్ళు btree_open_బహిర్గతం;
        }
      }
      sqlite3_mutex_enter(mutexShared);
      pBt->p_తదుపరి = GLOBAL(BtShared*,sqlite3SharedCacheList);
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt;
      sqlite3_mutex_leave(mutexShared);
    }
#అయితే_ముగింపు
  }

#ఒకవేళ !defined(SQLITE_OMIT_SHARED_CACHE) && !defined(SQLITE_OMIT_DISKIO)
  /* If the new Btree uses a sharable pBtShared, then link the new
  ** Btree into the list of all sharable Btrees for the same connection.
  ** The list is kept in ascending order by pBt address.
  */
  ఒకవేళ( p->sharable ){
    పూర్ణాంకం i;
    Btree *pSib;
    కొరకు(i=0; i<db->nDb; i++){
      ఒకవేళ( (pSib = db->aDb[i].pBt)!=0 && pSib->sharable ){
        అయితే( pSib->pPrev ){ pSib = pSib->pPrev; }
        ఒకవేళ( (uptr)p->pBt<(uptr)pSib->pBt ){
          p->p_తదుపరి = pSib;
          p->pPrev = 0;
          pSib->pPrev = p;
        }లేకపోతే{
          అయితే( pSib->p_తదుపరి && (uptr)pSib->p_తదుపరి->pBt<(uptr)p->pBt ){
            pSib = pSib->p_తదుపరి;
          }
          p->p_తదుపరి = pSib->p_తదుపరి;
          p->pPrev = pSib;
          ఒకవేళ( p->p_తదుపరి ){
            p->p_తదుపరి->pPrev = p;
          }
          pSib->p_తదుపరి = p;
        }
        ఆపు;
      }
    }
  }
#అయితే_ముగింపు
  *ppBtree = p;

btree_open_బహిర్గతం:
  ఒకవేళ( rc!=SQLITE_OK ){
    ఒకవేళ( pBt && pBt->pPager ){
      sqlite3PagerClose(pBt->pPager, 0);
    }
    sqlite3_free(pBt);
    sqlite3_free(p);
    *ppBtree = 0;
  }లేకపోతే{
    sqlite3_file *pFile;

    /* If the B-Tree was successfully opened, set the pager-cache size to the
    ** default value. Except, when opening on an existing shared pager-cache,
    ** do not change the pager-cache size.
    */
    ఒకవేళ( sqlite3BtreeSchema(p, 0, 0)==0 ){
      sqlite3_Btree_సమితి_Cache_కొలత(p, SQLITE_DEFAULT_CACHE_కొలత);
    }

    pFile = sqlite3PagerFile(pBt->pPager);
    ఒకవేళ( pFile->pMethods ){
      sqlite3OsFileControlHint(pFile, SQLITE_FCNTL_PDB, (శూన్య*)&pBt->db);
    }
  }
  ఒకవేళ( mutexOpen ){
    ప్రతిజ్ఞా( sqlite3_mutex_held(mutexOpen) );
    sqlite3_mutex_leave(mutexOpen);
  }
  ప్రతిజ్ఞా( rc!=SQLITE_OK || sqlite3_Btree_Connection_లెక్క(*ppBtree)>0 );
  తిరిగి rc;
}

/*
** Decrement the BtShared.nRef counter.  When it reaches zero,
** remove the BtShared structure from the sharing list.  Return
** true if the BtShared.nRef counter reaches zero and return
** false if it is still positive.
*/
స్థిర పూర్ణాంకం removeFromSharingList(BtShared *pBt){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
  MUTEX_LOGIC( sqlite3_mutex *pMainMtx; )
  BtShared *pList;
  పూర్ణాంకం removed = 0;

  ప్రతిజ్ఞా( sqlite3_mutex_notheld(pBt->mutex) );
  MUTEX_LOGIC( pMainMtx = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MAIN); )
  sqlite3_mutex_enter(pMainMtx);
  pBt->nRef--;
  ఒకవేళ( pBt->nRef<=0 ){
    ఒకవేళ( GLOBAL(BtShared*,sqlite3SharedCacheList)==pBt ){
      GLOBAL(BtShared*,sqlite3SharedCacheList) = pBt->p_తదుపరి;
    }లేకపోతే{
      pList = GLOBAL(BtShared*,sqlite3SharedCacheList);
      అయితే( ALWAYS(pList) && pList->p_తదుపరి!=pBt ){
        pList=pList->p_తదుపరి;
      }
      ఒకవేళ( ALWAYS(pList) ){
        pList->p_తదుపరి = pBt->p_తదుపరి;
      }
    }
    ఒకవేళ( SQLITE_THREADSAFE ){
      sqlite3_mutex_free(pBt->mutex);
    }
    removed = 1;
  }
  sqlite3_mutex_leave(pMainMtx);
  తిరిగి removed;
#లేకపోతే
  వాడని_PARAMETER( pBt );
  తిరిగి 1;
#అయితే_ముగింపు
}

/*
** Make sure pBt->pTmpSpace points to an allocation of
** MX_CELL_SIZE(pBt) bytes with a 4-byte prefix for a left-child
** pointer.
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం allocate_తాత్కాలిక_Space(BtShared *pBt){
  ప్రతిజ్ఞా( pBt!=0 );
  ప్రతిజ్ఞా( pBt->pTmpSpace==0 );
  /* This routine is called only by btreeCursor() when allocating the
  ** first write cursor for the BtShared object */
  ప్రతిజ్ఞా( pBt->pCursor!=0 && (pBt->pCursor->curFlags & BTCF_WriteFlag)!=0 );
  pBt->pTmpSpace = sqlite3_పుట_Malloc( pBt->పుట_కొలత );
  ఒకవేళ( pBt->pTmpSpace==0 ){
    BtCursor *pCur = pBt->pCursor;
    pBt->pCursor = pCur->p_తదుపరి;  /* Unlink the cursor */
    memset(pCur, 0, పరిమాణం(*pCur));
    తిరిగి SQLITE_NOMEM_BKPT;
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
  తిరిగి SQLITE_OK;
}

/*
** Free the pBt->pTmpSpace allocation
*/
స్థిర శూన్య free_తాత్కాలిక_Space(BtShared *pBt){
  ఒకవేళ( pBt->pTmpSpace ){
    pBt->pTmpSpace -= 4;
    sqlite3_పుట_Free(pBt->pTmpSpace);
    pBt->pTmpSpace = 0;
  }
}

/*
** Close an open database and invalidate all cursors.
*/
పూర్ణాంకం sqlite3BtreeClose(Btree *p){
  BtShared *pBt = p->pBt;

  /* Close all cursors opened via this handle.  */
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);

  /* Verify that no other cursors have this Btree open */
#నిర్వచితమైతే SQLITE_DEBUG
  {
    BtCursor *pCur = pBt->pCursor;
    అయితే( pCur ){
      BtCursor *pTmp = pCur;
      pCur = pCur->p_తదుపరి;
      ప్రతిజ్ఞా( pTmp->pBtree!=p );

    }
  }
#అయితే_ముగింపు

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
  ప్రతిజ్ఞా( p->want_గమ్యం_తాళం==0 && p->locked==0 );
  ఒకవేళ( !p->sharable || removeFromSharingList(pBt) ){
    /* The pBt is no longer on the sharing list, so we can access
    ** it without having to hold the mutex.
    **
    ** Clean out and delete the BtShared object.
    */
    ప్రతిజ్ఞా( !pBt->pCursor );
    sqlite3PagerClose(pBt->pPager, p->db);
    ఒకవేళ( pBt->xFreeSchema && pBt->pSchema ){
      pBt->xFreeSchema(pBt->pSchema);
    }
    sqlite3DbFree(0, pBt->pSchema);
    free_తాత్కాలిక_Space(pBt);
    sqlite3_free(pBt);
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
  ప్రతిజ్ఞా( p->want_గమ్యం_తాళం==0 );
  ప్రతిజ్ఞా( p->locked==0 );
  ఒకవేళ( p->pPrev ) p->pPrev->p_తదుపరి = p->p_తదుపరి;
  ఒకవేళ( p->p_తదుపరి ) p->p_తదుపరి->pPrev = p->pPrev;
#అయితే_ముగింపు

  sqlite3_free(p);
  తిరిగి SQLITE_OK;
}

/*
** Change the "soft" limit on the number of pages in the cache.
** Unused and unmodified pages will be recycled when the number of
** pages in the cache exceeds this soft limit.  But the size of the
** cache is allowed to grow larger than this limit if it contains
** dirty pages or pages still in active use.
*/
పూర్ణాంకం sqlite3_Btree_సమితి_Cache_కొలత(Btree *p, పూర్ణాంకం mx_పుట){
  BtShared *pBt = p->pBt;
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_సమితి_Cachesize(pBt->pPager, mx_పుట);
  sqlite3BtreeLeave(p);
  తిరిగి SQLITE_OK;
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
పూర్ణాంకం sqlite3_Btree_సమితి_Spill_కొలత(Btree *p, పూర్ణాంకం mx_పుట){
  BtShared *pBt = p->pBt;
  పూర్ణాంకం res;
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  res = sqlite3_Pager_సమితి_Spillsize(pBt->pPager, mx_పుట);
  sqlite3BtreeLeave(p);
  తిరిగి res;
}

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
/*
** Change the limit on the amount of the database file that may be
** memory mapped.
*/
పూర్ణాంకం sqlite3_Btree_సమితి_Mmap_హద్దు(Btree *p, sqlite3_int64 szMmap){
  BtShared *pBt = p->pBt;
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_సమితి_Mmap_హద్దు(pBt->pPager, szMmap);
  sqlite3BtreeLeave(p);
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు /* SQLITE_MAX_MMAP_SIZE>0 */

/*
** Change the way data is synced to disk in order to increase or decrease
** how well the database resists damage due to OS crashes and power
** failures.  Level 1 is the same as asynchronous (no syncs() occur and
** there is a high probability of damage)  Level 2 is the default.  There
** is a very low but non-zero probability of damage.  Level 3 reduces the
** probability of damage to near zero but with a write performance reduction.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_PAGER_PRAGMAS
పూర్ణాంకం sqlite3_Btree_సమితి_Pager_Flags(
  Btree *p,              /* The btree to set the safety level on */
  గుర్తులేని pgFlags       /* Various PAGER_* flags */
){
  BtShared *pBt = p->pBt;
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  sqlite3_Pager_సమితి_Flags(pBt->pPager, pgFlags);
  sqlite3BtreeLeave(p);
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు

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
పూర్ణాంకం sqlite3_Btree_సమితి_పుట_కొలత(Btree *p, పూర్ణాంకం పుట_కొలత, పూర్ణాంకం nReserve, పూర్ణాంకం iFix){
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం x;
  BtShared *pBt = p->pBt;
  ప్రతిజ్ఞా( nReserve>=0 && nReserve<=255 );
  sqlite3BtreeEnter(p);
  pBt->nReserveWanted = (u8)nReserve;
  x = pBt->పుట_కొలత - pBt->usable_కొలత;
  ఒకవేళ( x==nReserve && (పుట_కొలత==0 || (u32)పుట_కొలత==pBt->పుట_కొలత) ){
    sqlite3BtreeLeave(p);
    తిరిగి SQLITE_OK;
  }
  ఒకవేళ( nReserve<x ) nReserve = x;
  ఒకవేళ( pBt->btsFlags & BTS_PAGESIZE_FIXED ){
    sqlite3BtreeLeave(p);
    తిరిగి SQLITE_READONLY;
  }
  ప్రతిజ్ఞా( nReserve>=0 && nReserve<=255 );
  ఒకవేళ( పుట_కొలత>=512 && పుట_కొలత<=SQLITE_MAX_పుట_కొలత &&
        ((పుట_కొలత-1)&పుట_కొలత)==0 ){
    ప్రతిజ్ఞా( (పుట_కొలత & 7)==0 );
    ప్రతిజ్ఞా( !pBt->pCursor );
    ఒకవేళ( nReserve>32 && పుట_కొలత==512 ) పుట_కొలత = 1024;
    pBt->పుట_కొలత = (u32)పుట_కొలత;
    free_తాత్కాలిక_Space(pBt);
  }
  rc = sqlite3_Pager_సమితి_Pagesize(pBt->pPager, &pBt->పుట_కొలత, nReserve);
  pBt->usable_కొలత = pBt->పుట_కొలత - (u16)nReserve;
  ఒకవేళ( iFix ) pBt->btsFlags |= BTS_PAGESIZE_FIXED;
  sqlite3BtreeLeave(p);
  తిరిగి rc;
}

/*
** Return the currently defined page size
*/
పూర్ణాంకం sqlite3_Btree_Get_పుట_కొలత(Btree *p){
  తిరిగి p->pBt->పుట_కొలత;
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
పూర్ణాంకం sqlite3BtreeGetReserveNoMutex(Btree *p){
  పూర్ణాంకం n;
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->pBt->mutex) );
  n = p->pBt->పుట_కొలత - p->pBt->usable_కొలత;
  తిరిగి n;
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
పూర్ణాంకం sqlite3BtreeGetRequestedReserve(Btree *p){
  పూర్ణాంకం n1, n2;
  sqlite3BtreeEnter(p);
  n1 = (పూర్ణాంకం)p->pBt->nReserveWanted;
  n2 = sqlite3BtreeGetReserveNoMutex(p);
  sqlite3BtreeLeave(p);
  తిరిగి n1>n2 ? n1 : n2;
}


/*
** Set the maximum page count for a database if mxPage is positive.
** No changes are made if mxPage is 0 or negative.
** Regardless of the value of mxPage, return the maximum page count.
*/
Pgno sqlite3_Btree_Max_పుట_లెక్క(Btree *p, Pgno mx_పుట){
  Pgno n;
  sqlite3BtreeEnter(p);
  n = sqlite3_Pager_Max_పుట_లెక్క(p->pBt->pPager, mx_పుట);
  sqlite3BtreeLeave(p);
  తిరిగి n;
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
పూర్ణాంకం sqlite3BtreeSecureDelete(Btree *p, పూర్ణాంకం newFlag){
  పూర్ణాంకం b;
  ఒకవేళ( p==0 ) తిరిగి 0;
  sqlite3BtreeEnter(p);
  ప్రతిజ్ఞా( BTS_OVERWRITE==BTS_SECURE_DELETE*2 );
  ప్రతిజ్ఞా( BTS_FAST_SECURE==(BTS_OVERWRITE|BTS_SECURE_DELETE) );
  ఒకవేళ( newFlag>=0 ){
    p->pBt->btsFlags &= ~BTS_FAST_SECURE;
    p->pBt->btsFlags |= (u16)(BTS_SECURE_DELETE*newFlag);
  }
  b = (p->pBt->btsFlags & BTS_FAST_SECURE)/BTS_SECURE_DELETE;
  sqlite3BtreeLeave(p);
  తిరిగి b;
}

/*
** Change the 'auto-vacuum' property of the database. If the 'autoVacuum'
** parameter is non-zero, then auto-vacuum mode is enabled. If zero, it
** is disabled. The default value for the auto-vacuum property is
** determined by the SQLITE_DEFAULT_AUTOVACUUM macro.
*/
పూర్ణాంకం sqlite3_Btree_సమితి_Auto_Vacuum(Btree *p, పూర్ణాంకం autoVacuum){
#నిర్వచితమైతే SQLITE_OMIT_AUTOVACUUM
  తిరిగి SQLITE_READONLY;
#లేకపోతే
  BtShared *pBt = p->pBt;
  పూర్ణాంకం rc = SQLITE_OK;
  u8 av = (u8)autoVacuum;

  sqlite3BtreeEnter(p);
  ఒకవేళ( (pBt->btsFlags & BTS_PAGESIZE_FIXED)!=0 && (av ?1:0)!=pBt->autoVacuum ){
    rc = SQLITE_READONLY;
  }లేకపోతే{
    pBt->autoVacuum = av ?1:0;
    pBt->incrVacuum = av==2 ?1:0;
  }
  sqlite3BtreeLeave(p);
  తిరిగి rc;
#అయితే_ముగింపు
}

/*
** Return the value of the 'auto-vacuum' property. If auto-vacuum is
** enabled 1 is returned. Otherwise 0.
*/
పూర్ణాంకం sqlite3BtreeGetAutoVacuum(Btree *p){
#నిర్వచితమైతే SQLITE_OMIT_AUTOVACUUM
  తిరిగి BTREE_AUTOVACUUM_NONE;
#లేకపోతే
  పూర్ణాంకం rc;
  sqlite3BtreeEnter(p);
  rc = (
    (!p->pBt->autoVacuum)?BTREE_AUTOVACUUM_NONE:
    (!p->pBt->incrVacuum)?BTREE_AUTOVACUUM_నిండిన:
    BTREE_AUTOVACUUM_INCR
  );
  sqlite3BtreeLeave(p);
  తిరిగి rc;
#అయితే_ముగింపు
}

/*
** If the user has not set the safety-level for this database connection
** using "PRAGMA synchronous", and if the safety-level is not already
** set to the value passed to this function as the second parameter,
** set it so.
*/
#ఒకవేళ SQLITE_DEFAULT_SYNCHRONOUS!=SQLITE_DEFAULT_WAL_SYNCHRONOUS \
    && !defined(SQLITE_OMIT_WAL)
స్థిర శూన్య సమితి_Default_Sync_Flag(BtShared *pBt, u8 safety_level){
  sqlite3 *db;
  Db *pDb;
  ఒకవేళ( (db=pBt->db)!=0 && (pDb=db->aDb)!=0 ){
    అయితే( pDb->pBt==0 || pDb->pBt->pBt!=pBt ){ pDb++; }
    ఒకవేళ( pDb->b_Sync_సమితి==0
     && pDb->safety_level!=safety_level
     && pDb!=&db->aDb[1]
    ){
      pDb->safety_level = safety_level;
      sqlite3_Pager_సమితి_Flags(pBt->pPager,
          pDb->safety_level | (db->flags & PAGER_FLAGS_ముసుగు));
    }
  }
}
#లేకపోతే
# define సమితి_Default_Sync_Flag(pBt,safety_level)
#అయితే_ముగింపు

/* Forward declaration */
స్థిర పూర్ణాంకం newDatabase(BtShared*);


/*
** Get a reference to pPage1 of the database file.  This will
** also acquire a readlock on that file.
**
** SQLITE_OK is returned on success.  If the file is not a
** well-formed database file, then SQLITE_CORRUPT is returned.
** SQLITE_BUSY is returned if the database is locked.  SQLITE_NOMEM
** is returned if we run out of memory.
*/
స్థిర పూర్ణాంకం తాళం_Btree(BtShared *pBt){
  పూర్ణాంకం rc;              /* Result code from subfunctions */
  Mem_పుట *pPage1;     /* Page 1 of the database file */
  u32 n_పుట;           /* Number of pages in the database */
  u32 n_పుట_File = 0;   /* Number of pages in the database file */

  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ప్రతిజ్ఞా( pBt->pPage1==0 );
  rc = sqlite3_Pager_Shared_తాళం(pBt->pPager);
  ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
  rc = btree_Get_పుట(pBt, 1, &pPage1, 0);
  ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;

  /* Do some checking to help insure the file we opened really is
  ** a valid database file.
  */
  n_పుట = get4byte(28+(u8*)pPage1->a_దత్తాంశం);
  sqlite3PagerPagecount(pBt->pPager, (పూర్ణాంకం*)&n_పుట_File);
  ఒకవేళ( n_పుట==0 || memcmp(24+(u8*)pPage1->a_దత్తాంశం, 92+(u8*)pPage1->a_దత్తాంశం,4)!=0 ){
    n_పుట = n_పుట_File;
  }
  ఒకవేళ( (pBt->db->flags & SQLITE_ResetDatabase)!=0 ){
    n_పుట = 0;
  }
  ఒకవేళ( n_పుట>0 ){
    u32 పుట_కొలత;
    u32 usable_కొలత;
    u8 *page1 = pPage1->a_దత్తాంశం;
    rc = SQLITE_NOTADB;
    /* EVIDENCE-OF: R-43737-39999 Every valid SQLite database file begins
    ** with the following 16 bytes (in hex): 53 51 4c 69 74 65 20 66 6f 72 6d
    ** 61 74 20 33 00. */
    ఒకవేళ( memcmp(page1, z_Magic_శీర్షిక, 16)!=0 ){
      వెళ్ళు page1_init_failed;
    }

#నిర్వచితమైతే SQLITE_OMIT_WAL
    ఒకవేళ( page1[18]>1 ){
      pBt->btsFlags |= BTS_READ_ONLY;
    }
    ఒకవేళ( page1[19]>1 ){
      వెళ్ళు page1_init_failed;
    }
#లేకపోతే
    ఒకవేళ( page1[18]>2 ){
      pBt->btsFlags |= BTS_READ_ONLY;
    }
    ఒకవేళ( page1[19]>2 ){
      వెళ్ళు page1_init_failed;
    }

    /* If the read version is set to 2, this database should be accessed
    ** in WAL mode. If the log is not already open, open it now. Then
    ** return SQLITE_OK and return without populating BtShared.pPage1.
    ** The caller detects this and calls this function again. This is
    ** required as the version of page 1 currently in the page1 buffer
    ** may not be the latest version - there may be a newer one in the log
    ** file.
    */
    ఒకవేళ( page1[19]==2 && (pBt->btsFlags & BTS_NO_WAL)==0 ){
      పూర్ణాంకం isOpen = 0;
      rc = sqlite3PagerOpenWal(pBt->pPager, &isOpen);
      ఒకవేళ( rc!=SQLITE_OK ){
        వెళ్ళు page1_init_failed;
      }లేకపోతే{
        సమితి_Default_Sync_Flag(pBt, SQLITE_DEFAULT_WAL_SYNCHRONOUS+1);
        ఒకవేళ( isOpen==0 ){
          release_పుట_One(pPage1);
          తిరిగి SQLITE_OK;
        }
      }
      rc = SQLITE_NOTADB;
    }లేకపోతే{
      సమితి_Default_Sync_Flag(pBt, SQLITE_DEFAULT_SYNCHRONOUS+1);
    }
#అయితే_ముగింపు

    /* EVIDENCE-OF: R-15465-20813 The maximum and minimum embedded payload
    ** fractions and the leaf payload fraction values must be 64, 32, and 32.
    **
    ** The original design allowed these amounts to vary, but as of
    ** version 3.6.0, we require them to be fixed.
    */
    ఒకవేళ( memcmp(&page1[21], "\100\040\040",3)!=0 ){
      వెళ్ళు page1_init_failed;
    }
    /* EVIDENCE-OF: R-51873-39618 The page size for a database file is
    ** determined by the 2-byte integer located at an offset of 16 bytes from
    ** the beginning of the database file. */
    పుట_కొలత = (page1[16]<<8) | (page1[17]<<16);
    /* EVIDENCE-OF: R-25008-21688 The size of a page is a power of two
    ** between 512 and 65536 inclusive. */
    ఒకవేళ( ((పుట_కొలత-1)&పుట_కొలత)!=0
     || పుట_కొలత>SQLITE_MAX_పుట_కొలత
     || పుట_కొలత<=256
    ){
      వెళ్ళు page1_init_failed;
    }
    ప్రతిజ్ఞా( (పుట_కొలత & 7)==0 );
    /* EVIDENCE-OF: R-59310-51205 The "reserved space" size in the 1-byte
    ** integer at offset 20 is the number of bytes of space at the end of
    ** each page to reserve for extensions.
    **
    ** EVIDENCE-OF: R-37497-42412 The size of the reserved region is
    ** determined by the one-byte unsigned integer found at an offset of 20
    ** into the database file header. */
    usable_కొలత = పుట_కొలత - page1[20];
    ఒకవేళ( (u32)పుట_కొలత!=pBt->పుట_కొలత ){
      /* After reading the first page of the database assuming a page size
      ** of BtShared.pageSize, we have discovered that the page-size is
      ** actually pageSize. Unlock the database, leave pBt->pPage1 at
      ** zero and return SQLITE_OK. The caller will call this function
      ** again with the correct page-size.
      */
      release_పుట_One(pPage1);
      pBt->usable_కొలత = usable_కొలత;
      pBt->పుట_కొలత = పుట_కొలత;
      pBt->btsFlags |= BTS_PAGESIZE_FIXED;
      free_తాత్కాలిక_Space(pBt);
      rc = sqlite3_Pager_సమితి_Pagesize(pBt->pPager, &pBt->పుట_కొలత,
                                   పుట_కొలత-usable_కొలత);
      తిరిగి rc;
    }
    ఒకవేళ( n_పుట>n_పుట_File ){
      ఒకవేళ( sqlite3WritableSchema(pBt->db)==0 ){
        rc = SQLITE_CORRUPT_BKPT;
        వెళ్ళు page1_init_failed;
      }లేకపోతే{
        n_పుట = n_పుట_File;
      }
    }
    /* EVIDENCE-OF: R-28312-64704 However, the usable size is not allowed to
    ** be less than 480. In other words, if the page size is 512, then the
    ** reserved space size cannot exceed 32. */
    ఒకవేళ( usable_కొలత<480 ){
      వెళ్ళు page1_init_failed;
    }
    pBt->btsFlags |= BTS_PAGESIZE_FIXED;
    pBt->పుట_కొలత = పుట_కొలత;
    pBt->usable_కొలత = usable_కొలత;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
    pBt->autoVacuum = (get4byte(&page1[36 + 4*4])?1:0);
    pBt->incrVacuum = (get4byte(&page1[36 + 7*4])?1:0);
#అయితే_ముగింపు
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
  pBt->maxLocal = (u16)((pBt->usable_కొలత-12)*64/255 - 23);
  pBt->minLocal = (u16)((pBt->usable_కొలత-12)*32/255 - 23);
  pBt->max_ఆకు = (u16)(pBt->usable_కొలత - 35);
  pBt->min_ఆకు = (u16)((pBt->usable_కొలత-12)*32/255 - 23);
  ఒకవేళ( pBt->maxLocal>127 ){
    pBt->max1bytePayload = 127;
  }లేకపోతే{
    pBt->max1bytePayload = (u8)pBt->maxLocal;
  }
  ప్రతిజ్ఞా( pBt->max_ఆకు + 23 <= MX_CELL_కొలత(pBt) );
  pBt->pPage1 = pPage1;
  pBt->n_పుట = n_పుట;
  తిరిగి SQLITE_OK;

page1_init_failed:
  release_పుట_One(pPage1);
  pBt->pPage1 = 0;
  తిరిగి rc;
}

#నిర్వచితం_కాకపోతే NDEBUG
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
స్థిర పూర్ణాంకం లెక్క_Valid_Cursors(BtShared *pBt, పూర్ణాంకం wrOnly){
  BtCursor *pCur;
  పూర్ణాంకం r = 0;
  కొరకు(pCur=pBt->pCursor; pCur; pCur=pCur->p_తదుపరి){
    ఒకవేళ( (wrOnly==0 || (pCur->curFlags & BTCF_WriteFlag)!=0)
     && pCur->eState!=CURSOR_FAULT ) r++;
  }
  తిరిగి r;
}
#అయితే_ముగింపు

/*
** If there are no outstanding cursors and we are not in the middle
** of a transaction but there is a read lock on the database, then
** this routine unrefs the first page of the database file which
** has the effect of releasing the read lock.
**
** If there is a transaction in progress, this routine is a no-op.
*/
స్థిర శూన్య unlock_Btree_If_వాడని(BtShared *pBt){
  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ప్రతిజ్ఞా( లెక్క_Valid_Cursors(pBt,0)==0 || pBt->లోపల_Transaction>TRANS_NONE );
  ఒకవేళ( pBt->లోపల_Transaction==TRANS_NONE && pBt->pPage1!=0 ){
    Mem_పుట *pPage1 = pBt->pPage1;
    ప్రతిజ్ఞా( pPage1->a_దత్తాంశం );
    ప్రతిజ్ఞా( sqlite3PagerRefcount(pBt->pPager)==1 );
    pBt->pPage1 = 0;
    release_పుట_One(pPage1);
  }
}

/*
** If pBt points to an empty file then convert that empty file
** into a new empty database by initializing the first page of
** the database.
*/
స్థిర పూర్ణాంకం newDatabase(BtShared *pBt){
  Mem_పుట *pP1;
  గుర్తులేని అక్షరం *దత్తాంశం;
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ఒకవేళ( pBt->n_పుట>0 ){
    తిరిగి SQLITE_OK;
  }
  pP1 = pBt->pPage1;
  ప్రతిజ్ఞా( pP1!=0 );
  దత్తాంశం = pP1->a_దత్తాంశం;
  rc = sqlite3PagerWrite(pP1->p_Db_పుట);
  ఒకవేళ( rc ) తిరిగి rc;
  జ్ఞాపకం_నకలు(దత్తాంశం, z_Magic_శీర్షిక, పరిమాణం(z_Magic_శీర్షిక));
  ప్రతిజ్ఞా( పరిమాణం(z_Magic_శీర్షిక)==16 );
  దత్తాంశం[16] = (u8)((pBt->పుట_కొలత>>8)&0xff);
  దత్తాంశం[17] = (u8)((pBt->పుట_కొలత>>16)&0xff);
  దత్తాంశం[18] = 1;
  దత్తాంశం[19] = 1;
  ప్రతిజ్ఞా( pBt->usable_కొలత<=pBt->పుట_కొలత && pBt->usable_కొలత+255>=pBt->పుట_కొలత);
  దత్తాంశం[20] = (u8)(pBt->పుట_కొలత - pBt->usable_కొలత);
  దత్తాంశం[21] = 64;
  దత్తాంశం[22] = 32;
  దత్తాంశం[23] = 32;
  memset(&దత్తాంశం[24], 0, 100-24);
  zero_పుట(pP1, PTF_INTKEY|PTF_ఆకు|PTF_LEAFDATA );
  pBt->btsFlags |= BTS_PAGESIZE_FIXED;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
  ప్రతిజ్ఞా( pBt->autoVacuum==1 || pBt->autoVacuum==0 );
  ప్రతిజ్ఞా( pBt->incrVacuum==1 || pBt->incrVacuum==0 );
  put4byte(&దత్తాంశం[36 + 4*4], pBt->autoVacuum);
  put4byte(&దత్తాంశం[36 + 7*4], pBt->incrVacuum);
#అయితే_ముగింపు
  pBt->n_పుట = 1;
  దత్తాంశం[31] = 1;
  తిరిగి SQLITE_OK;
}

/*
** Initialize the first page of the database file (creating a database
** consisting of a single page and no schema objects). Return SQLITE_OK
** if successful, or an SQLite error code otherwise.
*/
పూర్ణాంకం sqlite3BtreeNewDb(Btree *p){
  పూర్ణాంకం rc;
  sqlite3BtreeEnter(p);
  p->pBt->n_పుట = 0;
  rc = newDatabase(p->pBt);
  sqlite3BtreeLeave(p);
  తిరిగి rc;
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం btreeBeginTrans(
  Btree *p,                 /* The btree in which to start the transaction */
  పూర్ణాంకం wrflag,               /* True to start a write transaction */
  పూర్ణాంకం *p_Schema_సంస్కరణ       /* Put schema version number here, if not NULL */
){
  BtShared *pBt = p->pBt;
  Pager *pPager = pBt->pPager;
  పూర్ణాంకం rc = SQLITE_OK;

  sqlite3BtreeEnter(p);
  btreeIntegrity(p);

  /* If the btree is already in a write-transaction, or it
  ** is already in a read-transaction and a read-transaction
  ** is requested, this is a no-op.
  */
  ఒకవేళ( p->లోపల_Trans==TRANS_WRITE || (p->లోపల_Trans==TRANS_READ && !wrflag) ){
    వెళ్ళు trans_begun;
  }
  ప్రతిజ్ఞా( pBt->లోపల_Transaction==TRANS_WRITE || IfNotOmitAV(pBt->bDoTruncate)==0 );

  ఒకవేళ( (p->db->flags & SQLITE_ResetDatabase)
   && sqlite3PagerIsreadonly(pPager)==0
  ){
    pBt->btsFlags &= ~BTS_READ_ONLY;
  }

  /* Write transactions are not possible on a read-only database */
  ఒకవేళ( (pBt->btsFlags & BTS_READ_ONLY)!=0 && wrflag ){
    rc = SQLITE_READONLY;
    వెళ్ళు trans_begun;
  }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
  {
    sqlite3 *p_ఖండం = 0;
    /* If another database handle has already opened a write transaction
    ** on this shared-btree structure and a second write transaction is
    ** requested, return SQLITE_LOCKED.
    */
    ఒకవేళ( (wrflag && pBt->లోపల_Transaction==TRANS_WRITE)
     || (pBt->btsFlags & BTS_PENDING)!=0
    ){
      p_ఖండం = pBt->pWriter->db;
    }లేకపోతే ఒకవేళ( wrflag>1 ){
      Bt_తాళం *pIter;
      కొరకు(pIter=pBt->p_తాళం; pIter; pIter=pIter->p_తదుపరి){
        ఒకవేళ( pIter->pBtree!=p ){
          p_ఖండం = pIter->pBtree->db;
          ఆపు;
        }
      }
    }
    ఒకవేళ( p_ఖండం ){
      sqlite3ConnectionBlocked(p->db, p_ఖండం);
      rc = SQLITE_LOCKED_SHAREDCACHE;
      వెళ్ళు trans_begun;
    }
  }
#అయితే_ముగింపు

  /* Any read-only or read-write transaction implies a read-lock on
  ** page 1. So if some other shared-cache client already has a write-lock
  ** on page 1, the transaction cannot be opened. */
  rc = query_Shared_Cache_పట్టిక_తాళం(p, SCHEMA_మూలం, READ_తాళం);
  ఒకవేళ( SQLITE_OK!=rc ) వెళ్ళు trans_begun;

  pBt->btsFlags &= ~BTS_INITIALLY_ఖాళీలు;
  ఒకవేళ( pBt->n_పుట==0 ) pBt->btsFlags |= BTS_INITIALLY_ఖాళీలు;
  చేయి {
    sqlite3PagerWalDb(pPager, p->db);

#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
    /* If transitioning from no transaction directly to a write transaction,
    ** block for the WRITER lock first if possible. */
    ఒకవేళ( pBt->pPage1==0 && wrflag ){
      ప్రతిజ్ఞా( pBt->లోపల_Transaction==TRANS_NONE );
      rc = sqlite3_Pager_Wal_Write_తాళం(pPager, 1);
      ఒకవేళ( rc!=SQLITE_BUSY && rc!=SQLITE_OK ) ఆపు;
    }
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_EXPERIMENTAL_PRAGMA_20251114
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
    ఒకవేళ( pBt->pPage1==0 && wrflag ){
      sqlite3_file *fd = sqlite3PagerFile(pPager);
      అక్షరం *aFcntl[3] = {0,0,0};
      aFcntl[1] = "experimental_pragma_20251114";
      ప్రతిజ్ఞా( wrflag==1 || wrflag==2 );
      aFcntl[2] = (wrflag==1 ? "1" : "2");
      sqlite3OsFileControlHint(fd, SQLITE_FCNTL_PRAGMA, (శూన్య*)aFcntl);
      sqlite3_free(aFcntl[0]);
    }
#అయితే_ముగింపు

    /* Call lockBtree() until either pBt->pPage1 is populated or
    ** lockBtree() returns something other than SQLITE_OK. lockBtree()
    ** may return SQLITE_OK but leave pBt->pPage1 set to 0 if after
    ** reading page 1 it discovers that the page-size of the database
    ** file is not pBt->pageSize. In this case lockBtree() will update
    ** pBt->pageSize to the page-size of the file on disk.
    */
    అయితే( pBt->pPage1==0 && SQLITE_OK==(rc = తాళం_Btree(pBt)) );

    ఒకవేళ( rc==SQLITE_OK && wrflag ){
      ఒకవేళ( (pBt->btsFlags & BTS_READ_ONLY)!=0 ){
        rc = SQLITE_READONLY;
      }లేకపోతే{
        rc = sqlite3PagerBegin(pPager, wrflag>1, sqlite3_తాత్కాలిక_లోపల_జ్ఞాపకం(p->db));
        ఒకవేళ( rc==SQLITE_OK ){
          rc = newDatabase(pBt);
        }లేకపోతే ఒకవేళ( rc==SQLITE_BUSY_SNAPSHOT && pBt->లోపల_Transaction==TRANS_NONE ){
          /* if there was no transaction opened when this function was
          ** called and SQLITE_BUSY_SNAPSHOT is returned, change the error
          ** code to SQLITE_BUSY. */
          rc = SQLITE_BUSY;
        }
      }
    }
 
    ఒకవేళ( rc!=SQLITE_OK ){
      (శూన్య)sqlite3_Pager_Wal_Write_తాళం(pPager, 0);
      unlock_Btree_If_వాడని(pBt);
    }
#ఒకవేళ defined(SQLITE_ENABLE_SETLK_TIMEOUT)
    ఒకవేళ( rc==SQLITE_BUSY_TIMEOUT ){
      /* If a blocking lock timed out, break out of the loop here so that
      ** the busy-handler is not invoked.  */
      ఆపు;
    }
#అయితే_ముగింపు
  }అయితే( (rc&0xFF)==SQLITE_BUSY && pBt->లోపల_Transaction==TRANS_NONE &&
          btreeInvokeBusyHandler(pBt) );
  sqlite3PagerWalDb(pPager, 0);
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  ఒకవేళ( rc==SQLITE_BUSY_TIMEOUT ) rc = SQLITE_BUSY;
#అయితే_ముగింపు

  ఒకవేళ( rc==SQLITE_OK ){
    ఒకవేళ( p->లోపల_Trans==TRANS_NONE ){
      pBt->nTransaction++;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
      ఒకవేళ( p->sharable ){
        ప్రతిజ్ఞా( p->తాళం.pBtree==p && p->తాళం.i_పట్టిక==1 );
        p->తాళం.e_తాళం = READ_తాళం;
        p->తాళం.p_తదుపరి = pBt->p_తాళం;
        pBt->p_తాళం = &p->తాళం;
      }
#అయితే_ముగింపు
    }
    p->లోపల_Trans = (wrflag?TRANS_WRITE:TRANS_READ);
    ఒకవేళ( p->లోపల_Trans>pBt->లోపల_Transaction ){
      pBt->లోపల_Transaction = p->లోపల_Trans;
    }
    ఒకవేళ( wrflag ){
      Mem_పుట *pPage1 = pBt->pPage1;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
      ప్రతిజ్ఞా( !pBt->pWriter );
      pBt->pWriter = p;
      pBt->btsFlags &= ~BTS_EXCLUSIVE;
      ఒకవేళ( wrflag>1 ) pBt->btsFlags |= BTS_EXCLUSIVE;
#అయితే_ముగింపు

      /* If the db-size header field is incorrect (as it may be if an old
      ** client has been writing the database file), update it now. Doing
      ** this sooner rather than later means the database size can safely
      ** re-read the database size from page 1 if a savepoint or transaction
      ** rollback occurs within the transaction.
      */
      ఒకవేళ( pBt->n_పుట!=get4byte(&pPage1->a_దత్తాంశం[28]) ){
        rc = sqlite3PagerWrite(pPage1->p_Db_పుట);
        ఒకవేళ( rc==SQLITE_OK ){
          put4byte(&pPage1->a_దత్తాంశం[28], pBt->n_పుట);
        }
      }
    }
  }

trans_begun:
  ఒకవేళ( rc==SQLITE_OK ){
    ఒకవేళ( p_Schema_సంస్కరణ ){
      *p_Schema_సంస్కరణ = get4byte(&pBt->pPage1->a_దత్తాంశం[40]);
    }
    ఒకవేళ( wrflag ){
      /* This call makes sure that the pager has the correct number of
      ** open savepoints. If the second parameter is greater than 0 and
      ** the sub-journal is not already open, then it will be opened here.
      */
      rc = sqlite3PagerOpenSavepoint(pPager, p->db->nSavepoint);
    }
  }

  btreeIntegrity(p);
  sqlite3BtreeLeave(p);
  తిరిగి rc;
}
పూర్ణాంకం sqlite3BtreeBeginTrans(Btree *p, పూర్ణాంకం wrflag, పూర్ణాంకం *p_Schema_సంస్కరణ){
  BtShared *pBt;
  ఒకవేళ( p->sharable
   || p->లోపల_Trans==TRANS_NONE
   || (p->లోపల_Trans==TRANS_READ && wrflag!=0)
  ){
    తిరిగి btreeBeginTrans(p,wrflag,p_Schema_సంస్కరణ);
  }
  pBt = p->pBt;
  ఒకవేళ( p_Schema_సంస్కరణ ){
    *p_Schema_సంస్కరణ = get4byte(&pBt->pPage1->a_దత్తాంశం[40]);
  }
  ఒకవేళ( wrflag ){
    /* This call makes sure that the pager has the correct number of
    ** open savepoints. If the second parameter is greater than 0 and
    ** the sub-journal is not already open, then it will be opened here.
    */
    తిరిగి sqlite3PagerOpenSavepoint(pBt->pPager, p->db->nSavepoint);
  }లేకపోతే{
    తిరిగి SQLITE_OK;
  }
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM

/*
** Set the pointer-map entries for all children of page pPage. Also, if
** pPage contains cells that point to overflow pages, set the pointer
** map entries for the overflow pages as well.
*/
స్థిర పూర్ణాంకం సమితి_బిడ్డ_Ptrmaps(Mem_పుట *p_పుట){
  పూర్ణాంకం i;                             /* Counter variable */
  పూర్ణాంకం nCell;                         /* Number of cells in page pPage */
  పూర్ణాంకం rc;                            /* Return code */
  BtShared *pBt = p_పుట->pBt;
  Pgno pgno = p_పుట->pgno;

  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  rc = p_పుట->isInit ? SQLITE_OK : btree_Init_పుట(p_పుట);
  ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
  nCell = p_పుట->nCell;

  కొరకు(i=0; i<nCell; i++){
    u8 *pCell = కనుగొను_Cell(p_పుట, i);

    ptrmapPutOvflPtr(p_పుట, p_పుట, pCell, &rc);

    ఒకవేళ( !p_పుట->ఆకు ){
      Pgno బిడ్డ_Pgno = get4byte(pCell);
      ptrmapPut(pBt, బిడ్డ_Pgno, PTRMAP_BTREE, pgno, &rc);
    }
  }

  ఒకవేళ( !p_పుట->ఆకు ){
    Pgno బిడ్డ_Pgno = get4byte(&p_పుట->a_దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+8]);
    ptrmapPut(pBt, బిడ్డ_Pgno, PTRMAP_BTREE, pgno, &rc);
  }

  తిరిగి rc;
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
స్థిర పూర్ణాంకం modify_పుట_సూచిక(Mem_పుట *p_పుట, Pgno iFrom, Pgno i_గమ్యం, u8 eType){
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_పుట->p_Db_పుట) );
  ఒకవేళ( eType==PTRMAP_OVERFLOW2 ){
    /* The pointer is always the first 4 bytes of the page in this case.  */
    ఒకవేళ( get4byte(p_పుట->a_దత్తాంశం)!=iFrom ){
      తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    }
    put4byte(p_పుట->a_దత్తాంశం, i_గమ్యం);
  }లేకపోతే{
    పూర్ణాంకం i;
    పూర్ణాంకం nCell;
    పూర్ణాంకం rc;

    rc = p_పుట->isInit ? SQLITE_OK : btree_Init_పుట(p_పుట);
    ఒకవేళ( rc ) తిరిగి rc;
    nCell = p_పుట->nCell;

    కొరకు(i=0; i<nCell; i++){
      u8 *pCell = కనుగొను_Cell(p_పుట, i);
      ఒకవేళ( eType==PTRMAP_OVERFLOW1 ){
        CellInfo info;
        p_పుట->xParseCell(p_పుట, pCell, &info);
        ఒకవేళ( info.nLocal<info.nPayload ){
          ఒకవేళ( pCell+info.n_కొలత > p_పుట->a_దత్తాంశం+p_పుట->pBt->usable_కొలత ){
            తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
          }
          ఒకవేళ( iFrom==get4byte(pCell+info.n_కొలత-4) ){
            put4byte(pCell+info.n_కొలత-4, i_గమ్యం);
            ఆపు;
          }
        }
      }లేకపోతే{
        ఒకవేళ( pCell+4 > p_పుట->a_దత్తాంశం+p_పుట->pBt->usable_కొలత ){
          తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
        }
        ఒకవేళ( get4byte(pCell)==iFrom ){
          put4byte(pCell, i_గమ్యం);
          ఆపు;
        }
      }
    }
 
    ఒకవేళ( i==nCell ){
      ఒకవేళ( eType!=PTRMAP_BTREE ||
          get4byte(&p_పుట->a_దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+8])!=iFrom ){
        తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
      }
      put4byte(&p_పుట->a_దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+8], i_గమ్యం);
    }
  }
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం relocate_పుట(
  BtShared *pBt,           /* Btree */
  Mem_పుట *p_Db_పుట,        /* Open page to move */
  u8 eType,                /* Pointer map 'type' entry for pDbPage */
  Pgno i_Ptr_పుట,           /* Pointer map 'page-no' entry for pDbPage */
  Pgno i_Free_పుట,          /* The location to move pDbPage to */
  పూర్ణాంకం isCommit             /* isCommit flag passed to sqlite3PagerMovepage */
){
  Mem_పుట *p_Ptr_పుట;   /* The page that contains a pointer to pDbPage */
  Pgno i_Db_పుట = p_Db_పుట->pgno;
  Pager *pPager = pBt->pPager;
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( eType==PTRMAP_OVERFLOW2 || eType==PTRMAP_OVERFLOW1 ||
      eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ప్రతిజ్ఞా( p_Db_పుట->pBt==pBt );
  ఒకవేళ( i_Db_పుట<3 ) తిరిగి SQLITE_CORRUPT_BKPT;

  /* Move page iDbPage from its current location to page number iFreePage */
  TRACE(("AUTOVACUUM: Moving %u to free page %u (ptr page %u type %u)\n",
      i_Db_పుట, i_Free_పుట, i_Ptr_పుట, eType));
  rc = sqlite3PagerMovepage(pPager, p_Db_పుట->p_Db_పుట, i_Free_పుట, isCommit);
  ఒకవేళ( rc!=SQLITE_OK ){
    తిరిగి rc;
  }
  p_Db_పుట->pgno = i_Free_పుట;

  /* If pDbPage was a btree-page, then it may have child pages and/or cells
  ** that point to overflow pages. The pointer map entries for all these
  ** pages need to be changed.
  **
  ** If pDbPage is an overflow page, then the first 4 bytes may store a
  ** pointer to a subsequent overflow page. If this is the case, then
  ** the pointer map needs to be updated for the subsequent overflow page.
  */
  ఒకవేళ( eType==PTRMAP_BTREE || eType==PTRMAP_ROOTPAGE ){
    rc = సమితి_బిడ్డ_Ptrmaps(p_Db_పుట);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
  }లేకపోతే{
    Pgno తదుపరి_Ovfl = get4byte(p_Db_పుట->a_దత్తాంశం);
    ఒకవేళ( తదుపరి_Ovfl!=0 ){
      ptrmapPut(pBt, తదుపరి_Ovfl, PTRMAP_OVERFLOW2, i_Free_పుట, &rc);
      ఒకవేళ( rc!=SQLITE_OK ){
        తిరిగి rc;
      }
    }
  }

  /* Fix the database pointer on page iPtrPage that pointed at iDbPage so
  ** that it points at iFreePage. Also fix the pointer map entry for
  ** iPtrPage.
  */
  ఒకవేళ( eType!=PTRMAP_ROOTPAGE ){
    rc = btree_Get_పుట(pBt, i_Ptr_పుట, &p_Ptr_పుట, 0);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
    rc = sqlite3PagerWrite(p_Ptr_పుట->p_Db_పుట);
    ఒకవేళ( rc!=SQLITE_OK ){
      release_పుట(p_Ptr_పుట);
      తిరిగి rc;
    }
    rc = modify_పుట_సూచిక(p_Ptr_పుట, i_Db_పుట, i_Free_పుట, eType);
    release_పుట(p_Ptr_పుట);
    ఒకవేళ( rc==SQLITE_OK ){
      ptrmapPut(pBt, i_Free_పుట, eType, i_Ptr_పుట, &rc);
    }
  }
  తిరిగి rc;
}

/* Forward declaration required by incrVacuumStep(). */
స్థిర పూర్ణాంకం allocate_Btree_పుట(BtShared *, Mem_పుట **, Pgno *, Pgno, u8);

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
స్థిర పూర్ణాంకం incr_Vacuum_అడుగు(BtShared *pBt, Pgno nFin, Pgno i_చివరి_Pg, పూర్ణాంకం bCommit){
  Pgno nFreeList;           /* Number of pages still on the free-list */
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ప్రతిజ్ఞా( i_చివరి_Pg>nFin );

  ఒకవేళ( !PTRMAP_ISPAGE(pBt, i_చివరి_Pg) && i_చివరి_Pg!=PENDING_BYTE_పుట(pBt) ){
    u8 eType;
    Pgno i_Ptr_పుట;

    nFreeList = get4byte(&pBt->pPage1->a_దత్తాంశం[36]);
    ఒకవేళ( nFreeList==0 ){
      తిరిగి SQLITE_పూర్తి;
    }

    rc = ptrmapGet(pBt, i_చివరి_Pg, &eType, &i_Ptr_పుట);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
    ఒకవేళ( eType==PTRMAP_ROOTPAGE ){
      తిరిగి SQLITE_CORRUPT_BKPT;
    }

    ఒకవేళ( eType==PTRMAP_FREEPAGE ){
      ఒకవేళ( bCommit==0 ){
        /* Remove the page from the files free-list. This is not required
        ** if bCommit is non-zero. In that case, the free-list will be
        ** truncated to zero after this function returns, so it doesn't
        ** matter if it still contains some garbage entries.
        */
        Pgno iFreePg;
        Mem_పుట *pFreePg;
        rc = allocate_Btree_పుట(pBt, &pFreePg, &iFreePg, i_చివరి_Pg, BTALLOC_EXACT);
        ఒకవేళ( rc!=SQLITE_OK ){
          తిరిగి rc;
        }
        ప్రతిజ్ఞా( iFreePg==i_చివరి_Pg );
        release_పుట(pFreePg);
      }
    } లేకపోతే {
      Pgno iFreePg;             /* Index of free page to move pLastPg to */
      Mem_పుట *p_చివరి_Pg;
      u8 e_బహుళకం = BTALLOC_ANY;   /* Mode parameter for allocateBtreePage() */
      Pgno iNear = 0;           /* nearby parameter for allocateBtreePage() */

      rc = btree_Get_పుట(pBt, i_చివరి_Pg, &p_చివరి_Pg, 0);
      ఒకవేళ( rc!=SQLITE_OK ){
        తిరిగి rc;
      }

      /* If bCommit is zero, this loop runs exactly once and page pLastPg
      ** is swapped with the first free page pulled off the free list.
      **
      ** On the other hand, if bCommit is greater than zero, then keep
      ** looping until a free-page located within the first nFin pages
      ** of the file is found.
      */
      ఒకవేళ( bCommit==0 ){
        e_బహుళకం = BTALLOC_LE;
        iNear = nFin;
      }
      చేయి {
        Mem_పుట *pFreePg;
        Pgno db_కొలత = btreePagecount(pBt);
        rc = allocate_Btree_పుట(pBt, &pFreePg, &iFreePg, iNear, e_బహుళకం);
        ఒకవేళ( rc!=SQLITE_OK ){
          release_పుట(p_చివరి_Pg);
          తిరిగి rc;
        }
        release_పుట(pFreePg);
        ఒకవేళ( iFreePg>db_కొలత ){
          release_పుట(p_చివరి_Pg);
          తిరిగి SQLITE_CORRUPT_BKPT;
        }
      }అయితే( bCommit && iFreePg>nFin );
      ప్రతిజ్ఞా( iFreePg<i_చివరి_Pg );
     
      rc = relocate_పుట(pBt, p_చివరి_Pg, eType, i_Ptr_పుట, iFreePg, bCommit);
      release_పుట(p_చివరి_Pg);
      ఒకవేళ( rc!=SQLITE_OK ){
        తిరిగి rc;
      }
    }
  }

  ఒకవేళ( bCommit==0 ){
    చేయి {
      i_చివరి_Pg--;
    }అయితే( i_చివరి_Pg==PENDING_BYTE_పుట(pBt) || PTRMAP_ISPAGE(pBt, i_చివరి_Pg) );
    pBt->bDoTruncate = 1;
    pBt->n_పుట = i_చివరి_Pg;
  }
  తిరిగి SQLITE_OK;
}

/*
** The database opened by the first argument is an auto-vacuum database
** nOrig pages in size containing nFree free pages. Return the expected
** size of the database in pages following an auto-vacuum operation.
*/
స్థిర Pgno final_Db_కొలత(BtShared *pBt, Pgno nOrig, Pgno nFree){
  పూర్ణాంకం n_నమోదు;                     /* Number of entries on one ptrmap page */
  Pgno nPtrmap;                   /* Number of PtrMap pages to be freed */
  Pgno nFin;                      /* Return value */

  n_నమోదు = pBt->usable_కొలత/5;
  nPtrmap = (nFree-nOrig+PTRMAP_PAGENO(pBt, nOrig)+n_నమోదు)/n_నమోదు;
  nFin = nOrig - nFree - nPtrmap;
  ఒకవేళ( nOrig>PENDING_BYTE_పుట(pBt) && nFin<PENDING_BYTE_పుట(pBt) ){
    nFin--;
  }
  అయితే( PTRMAP_ISPAGE(pBt, nFin) || nFin==PENDING_BYTE_పుట(pBt) ){
    nFin--;
  }

  తిరిగి nFin;
}

/*
** A write-transaction must be opened before calling this function.
** It performs a single unit of work towards an incremental vacuum.
**
** If the incremental vacuum is finished after this function has run,
** SQLITE_DONE is returned. If it is not finished, but no error occurred,
** SQLITE_OK is returned. Otherwise an SQLite error code.
*/
పూర్ణాంకం sqlite3BtreeIncrVacuum(Btree *p){
  పూర్ణాంకం rc;
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  ప్రతిజ్ఞా( pBt->లోపల_Transaction==TRANS_WRITE && p->లోపల_Trans==TRANS_WRITE );
  ఒకవేళ( !pBt->autoVacuum ){
    rc = SQLITE_పూర్తి;
  }లేకపోతే{
    Pgno nOrig = btreePagecount(pBt);
    Pgno nFree = get4byte(&pBt->pPage1->a_దత్తాంశం[36]);
    Pgno nFin = final_Db_కొలత(pBt, nOrig, nFree);

    ఒకవేళ( nOrig<nFin || nFree>=nOrig ){
      rc = SQLITE_CORRUPT_BKPT;
    }లేకపోతే ఒకవేళ( nFree>0 ){
      rc = saveAllCursors(pBt, 0, 0);
      ఒకవేళ( rc==SQLITE_OK ){
        invalidate_All_అధికప్రవాహం_Cache(pBt);
        rc = incr_Vacuum_అడుగు(pBt, nFin, nOrig, 0);
      }
      ఒకవేళ( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->p_Db_పుట);
        put4byte(&pBt->pPage1->a_దత్తాంశం[28], pBt->n_పుట);
      }
    }లేకపోతే{
      rc = SQLITE_పూర్తి;
    }
  }
  sqlite3BtreeLeave(p);
  తిరిగి rc;
}

/*
** This routine is called prior to sqlite3PagerCommit when a transaction
** is committed for an auto-vacuum database.
*/
స్థిర పూర్ణాంకం autoVacuumCommit(Btree *p){
  పూర్ణాంకం rc = SQLITE_OK;
  Pager *pPager;
  BtShared *pBt;
  sqlite3 *db;
  VVA_ONLY( పూర్ణాంకం nRef );

  ప్రతిజ్ఞా( p!=0 );
  pBt = p->pBt; 
  pPager = pBt->pPager;
  VVA_ONLY( nRef = sqlite3PagerRefcount(pPager); )

  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  invalidate_All_అధికప్రవాహం_Cache(pBt);
  ప్రతిజ్ఞా(pBt->autoVacuum);
  ఒకవేళ( !pBt->incrVacuum ){
    Pgno nFin;         /* Number of pages in database after autovacuuming */
    Pgno nFree;        /* Number of pages on the freelist initially */
    Pgno nVac;         /* Number of pages to vacuum */
    Pgno iFree;        /* The next page to be freed */
    Pgno nOrig;        /* Database size before freeing */

    nOrig = btreePagecount(pBt);
    ఒకవేళ( PTRMAP_ISPAGE(pBt, nOrig) || nOrig==PENDING_BYTE_పుట(pBt) ){
      /* It is not possible to create a database for which the final page
      ** is either a pointer-map page or the pending-byte page. If one
      ** is encountered, this indicates corruption.
      */
      తిరిగి SQLITE_CORRUPT_BKPT;
    }

    nFree = get4byte(&pBt->pPage1->a_దత్తాంశం[36]);
    db = p->db;
    ఒకవేళ( db->x_Autovac_పుటలు ){
      పూర్ణాంకం iDb;
      కొరకు(iDb=0; ALWAYS(iDb<db->nDb); iDb++){
        ఒకవేళ( db->aDb[iDb].pBt==p ) ఆపు;
      }
      nVac = db->x_Autovac_పుటలు(
        db->p_Autovac_పుటలు_Arg,
        db->aDb[iDb].zDbSName,
        nOrig,
        nFree,
        pBt->పుట_కొలత
      );
      ఒకవేళ( nVac>nFree ){
        nVac = nFree;
      }
      ఒకవేళ( nVac==0 ){
        తిరిగి SQLITE_OK;
      }
    }లేకపోతే{
      nVac = nFree;
    }
    nFin = final_Db_కొలత(pBt, nOrig, nVac);
    ఒకవేళ( nFin>nOrig ) తిరిగి SQLITE_CORRUPT_BKPT;
    ఒకవేళ( nFin<nOrig ){
      rc = saveAllCursors(pBt, 0, 0);
    }
    కొరకు(iFree=nOrig; iFree>nFin && rc==SQLITE_OK; iFree--){
      rc = incr_Vacuum_అడుగు(pBt, nFin, iFree, nVac==nFree);
    }
    ఒకవేళ( (rc==SQLITE_పూర్తి || rc==SQLITE_OK) && nFree>0 ){
      rc = sqlite3PagerWrite(pBt->pPage1->p_Db_పుట);
      ఒకవేళ( nVac==nFree ){
        put4byte(&pBt->pPage1->a_దత్తాంశం[32], 0);
        put4byte(&pBt->pPage1->a_దత్తాంశం[36], 0);
      }
      put4byte(&pBt->pPage1->a_దత్తాంశం[28], nFin);
      pBt->bDoTruncate = 1;
      pBt->n_పుట = nFin;
    }
    ఒకవేళ( rc!=SQLITE_OK ){
      sqlite3PagerRollback(pPager);
    }
  }

  ప్రతిజ్ఞా( nRef>=sqlite3PagerRefcount(pPager) );
  తిరిగి rc;
}

#లేకపోతే /* ifndef SQLITE_OMIT_AUTOVACUUM */
# define సమితి_బిడ్డ_Ptrmaps(x) SQLITE_OK
#అయితే_ముగింపు

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
పూర్ణాంకం sqlite3BtreeCommitPhaseOne(Btree *p, స్థిరరాశి అక్షరం *zSuperJrnl){
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( p->లోపల_Trans==TRANS_WRITE ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
    ఒకవేళ( pBt->autoVacuum ){
      rc = autoVacuumCommit(p);
      ఒకవేళ( rc!=SQLITE_OK ){
        sqlite3BtreeLeave(p);
        తిరిగి rc;
      }
    }
    ఒకవేళ( pBt->bDoTruncate ){
      sqlite3PagerTruncateImage(pBt->pPager, pBt->n_పుట);
    }
#అయితే_ముగింపు
    rc = sqlite3PagerCommitPhaseOne(pBt->pPager, zSuperJrnl, 0);
    sqlite3BtreeLeave(p);
  }
  తిరిగి rc;
}

/*
** This function is called from both BtreeCommitPhaseTwo() and BtreeRollback()
** at the conclusion of a transaction.
*/
స్థిర శూన్య btree_చివర_Transaction(Btree *p){
  BtShared *pBt = p->pBt;
  sqlite3 *db = p->db;
  ప్రతిజ్ఞా( sqlite3BtreeHoldsMutex(p) );

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
  pBt->bDoTruncate = 0;
#అయితే_ముగింపు
  ఒకవేళ( p->లోపల_Trans>TRANS_NONE && db->nVdbeRead>1 ){
    /* If there are other active statements that belong to this database
    ** handle, downgrade to a read-only transaction. The other statements
    ** may still be reading from the database.  */
    downgrade_All_Shared_Cache_పట్టిక_Locks(p);
    p->లోపల_Trans = TRANS_READ;
  }లేకపోతే{
    /* If the handle had any kind of transaction open, decrement the
    ** transaction count of the shared btree. If the transaction count
    ** reaches 0, set the shared state to TRANS_NONE. The unlockBtreeIfUnused()
    ** call below will unlock the pager.  */
    ఒకవేళ( p->లోపల_Trans!=TRANS_NONE ){
      clear_All_Shared_Cache_పట్టిక_Locks(p);
      pBt->nTransaction--;
      ఒకవేళ( 0==pBt->nTransaction ){
        pBt->లోపల_Transaction = TRANS_NONE;
      }
    }

    /* Set the current transaction state to TRANS_NONE and unlock the
    ** pager if this call closed the only read or write transaction.  */
    p->లోపల_Trans = TRANS_NONE;
    unlock_Btree_If_వాడని(pBt);
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
పూర్ణాంకం sqlite3BtreeCommitPhaseTwo(Btree *p, పూర్ణాంకం bCleanup){

  ఒకవేళ( p->లోపల_Trans==TRANS_NONE ) తిరిగి SQLITE_OK;
  sqlite3BtreeEnter(p);
  btreeIntegrity(p);

  /* If the handle has a write-transaction open, commit the shared-btrees
  ** transaction and set the shared state to TRANS_READ.
  */
  ఒకవేళ( p->లోపల_Trans==TRANS_WRITE ){
    పూర్ణాంకం rc;
    BtShared *pBt = p->pBt;
    ప్రతిజ్ఞా( pBt->లోపల_Transaction==TRANS_WRITE );
    ప్రతిజ్ఞా( pBt->nTransaction>0 );
    rc = sqlite3PagerCommitPhaseTwo(pBt->pPager);
    ఒకవేళ( rc!=SQLITE_OK && bCleanup==0 ){
      sqlite3BtreeLeave(p);
      తిరిగి rc;
    }
    p->i_BData_సంస్కరణ--;  /* Compensate for pPager->iDataVersion++; */
    pBt->లోపల_Transaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btree_చివర_Transaction(p);
  sqlite3BtreeLeave(p);
  తిరిగి SQLITE_OK;
}

/*
** Do both phases of a commit.
*/
పూర్ణాంకం sqlite3BtreeCommit(Btree *p){
  పూర్ణాంకం rc;
  sqlite3BtreeEnter(p);
  rc = sqlite3BtreeCommitPhaseOne(p, 0);
  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3BtreeCommitPhaseTwo(p, 0);
  }
  sqlite3BtreeLeave(p);
  తిరిగి rc;
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
పూర్ణాంకం sqlite3BtreeTripAllCursors(Btree *pBtree, పూర్ణాంకం err_సంకేతం, పూర్ణాంకం writeOnly){
  BtCursor *p;
  పూర్ణాంకం rc = SQLITE_OK;

  ప్రతిజ్ఞా( (writeOnly==0 || writeOnly==1) && BTCF_WriteFlag==1 );
  ఒకవేళ( pBtree ){
    sqlite3BtreeEnter(pBtree);
    కొరకు(p=pBtree->pBt->pCursor; p; p=p->p_తదుపరి){
      ఒకవేళ( writeOnly && (p->curFlags & BTCF_WriteFlag)==0 ){
        ఒకవేళ( p->eState==CURSOR_VALID || p->eState==CURSOR_SKIPNEXT ){
          rc = saveCursorPosition(p);
          ఒకవేళ( rc!=SQLITE_OK ){
            (శూన్య)sqlite3BtreeTripAllCursors(pBtree, rc, 0);
            ఆపు;
          }
        }
      }లేకపోతే{
        sqlite3BtreeClearCursor(p);
        p->eState = CURSOR_FAULT;
        p->skip_తదుపరి = err_సంకేతం;
      }
      btree_Release_All_Cursor_పుటలు(p);
    }
    sqlite3BtreeLeave(pBtree);
  }
  తిరిగి rc;
}

/*
** Set the pBt->nPage field correctly, according to the current
** state of the database.  Assume pBt->pPage1 is valid.
*/
స్థిర శూన్య btree_సమితి_NPage(BtShared *pBt, Mem_పుట *pPage1){
  పూర్ణాంకం n_పుట = get4byte(&pPage1->a_దత్తాంశం[28]);
  testcase( n_పుట==0 );
  ఒకవేళ( n_పుట==0 ) sqlite3PagerPagecount(pBt->pPager, &n_పుట);
  testcase( pBt->n_పుట!=(u32)n_పుట );
  pBt->n_పుట = n_పుట;
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
పూర్ణాంకం sqlite3BtreeRollback(Btree *p, పూర్ణాంకం trip_సంకేతం, పూర్ణాంకం writeOnly){
  పూర్ణాంకం rc;
  BtShared *pBt = p->pBt;
  Mem_పుట *pPage1;

  ప్రతిజ్ఞా( writeOnly==1 || writeOnly==0 );
  ప్రతిజ్ఞా( trip_సంకేతం==SQLITE_ABORT_ROLLBACK || trip_సంకేతం==SQLITE_OK );
  sqlite3BtreeEnter(p);
  ఒకవేళ( trip_సంకేతం==SQLITE_OK ){
    rc = trip_సంకేతం = saveAllCursors(pBt, 0, 0);
    ఒకవేళ( rc ) writeOnly = 0;
  }లేకపోతే{
    rc = SQLITE_OK;
  }
  ఒకవేళ( trip_సంకేతం ){
    పూర్ణాంకం rc2 = sqlite3BtreeTripAllCursors(p, trip_సంకేతం, writeOnly);
    ప్రతిజ్ఞా( rc==SQLITE_OK || (writeOnly==0 && rc2==SQLITE_OK) );
    ఒకవేళ( rc2!=SQLITE_OK ) rc = rc2;
  }
  btreeIntegrity(p);

  ఒకవేళ( p->లోపల_Trans==TRANS_WRITE ){
    పూర్ణాంకం rc2;

    ప్రతిజ్ఞా( TRANS_WRITE==pBt->లోపల_Transaction );
    rc2 = sqlite3PagerRollback(pBt->pPager);
    ఒకవేళ( rc2!=SQLITE_OK ){
      rc = rc2;
    }

    /* The rollback may have destroyed the pPage1->aData value.  So
    ** call btreeGetPage() on page 1 again to make
    ** sure pPage1->aData is set correctly. */
    ఒకవేళ( btree_Get_పుట(pBt, 1, &pPage1, 0)==SQLITE_OK ){
      btree_సమితి_NPage(pBt, pPage1);
      release_పుట_One(pPage1);
    }
    ప్రతిజ్ఞా( లెక్క_Valid_Cursors(pBt, 1)==0 );
    pBt->లోపల_Transaction = TRANS_READ;
    btreeClearHasContent(pBt);
  }

  btree_చివర_Transaction(p);
  sqlite3BtreeLeave(p);
  తిరిగి rc;
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
పూర్ణాంకం sqlite3BtreeBeginStmt(Btree *p, పూర్ణాంకం iStatement){
  పూర్ణాంకం rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);
  ప్రతిజ్ఞా( p->లోపల_Trans==TRANS_WRITE );
  ప్రతిజ్ఞా( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  ప్రతిజ్ఞా( iStatement>0 );
  ప్రతిజ్ఞా( iStatement>p->db->nSavepoint );
  ప్రతిజ్ఞా( pBt->లోపల_Transaction==TRANS_WRITE );
  /* At the pager level, a statement transaction is a savepoint with
  ** an index greater than all savepoints created explicitly using
  ** SQL statements. It is illegal to open, release or rollback any
  ** such savepoints while the statement transaction savepoint is active.
  */
  rc = sqlite3PagerOpenSavepoint(pBt->pPager, iStatement);
  sqlite3BtreeLeave(p);
  తిరిగి rc;
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
పూర్ణాంకం sqlite3BtreeSavepoint(Btree *p, పూర్ణాంకం op, పూర్ణాంకం iSavepoint){
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( p && p->లోపల_Trans==TRANS_WRITE ){
    BtShared *pBt = p->pBt;
    ప్రతిజ్ఞా( op==SAVEPOINT_RELEASE || op==SAVEPOINT_ROLLBACK );
    ప్రతిజ్ఞా( iSavepoint>=0 || (iSavepoint==-1 && op==SAVEPOINT_ROLLBACK) );
    sqlite3BtreeEnter(p);
    ఒకవేళ( op==SAVEPOINT_ROLLBACK ){
      rc = saveAllCursors(pBt, 0, 0);
    }
    ఒకవేళ( rc==SQLITE_OK ){
      rc = sqlite3PagerSavepoint(pBt->pPager, op, iSavepoint);
    }
    ఒకవేళ( rc==SQLITE_OK ){
      ఒకవేళ( iSavepoint<0 && (pBt->btsFlags & BTS_INITIALLY_ఖాళీలు)!=0 ){
        pBt->n_పుట = 0;
      }
      rc = newDatabase(pBt);
      btree_సమితి_NPage(pBt, pBt->pPage1);

      /* pBt->nPage might be zero if the database was corrupt when
      ** the transaction was started. Otherwise, it must be at least 1.  */
      ప్రతిజ్ఞా( CORRUPT_DB || pBt->n_పుట>0 );
    }
    sqlite3BtreeLeave(p);
  }
  తిరిగి rc;
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
స్థిర పూర్ణాంకం btreeCursor(
  Btree *p,                              /* The btree */
  Pgno i_పట్టిక,                           /* Root page of table to open */
  పూర్ణాంకం wrFlag,                            /* 1 to write. 0 read-only */
  నిర్మాణం కీలకం_Info *p_కీలకం_Info,              /* First arg to comparison function */
  BtCursor *pCur                         /* Space for new cursor */
){
  BtShared *pBt = p->pBt;                /* Shared b-tree handle */
  BtCursor *pX;                          /* Looping over other all cursors */

  ప్రతిజ్ఞా( sqlite3BtreeHoldsMutex(p) );
  ప్రతిజ్ఞా( wrFlag==0
       || wrFlag==BTREE_WRCSR
       || wrFlag==(BTREE_WRCSR|BTREE_FORDELETE)
  );

  /* The following assert statements verify that if this is a sharable
  ** b-tree database, the connection is holding the required table locks,
  ** and that no other connection has any open cursor that conflicts with
  ** this lock.  The iTable<1 term disables the check for corrupt schemas. */
  ప్రతిజ్ఞా( has_Shared_Cache_పట్టిక_తాళం(p, i_పట్టిక, p_కీలకం_Info!=0, (wrFlag?2:1))
          || i_పట్టిక<1 );
  ప్రతిజ్ఞా( wrFlag==0 || !hasReadConflicts(p, i_పట్టిక) );

  /* Assert that the caller has opened the required transaction. */
  ప్రతిజ్ఞా( p->లోపల_Trans>TRANS_NONE );
  ప్రతిజ్ఞా( wrFlag==0 || p->లోపల_Trans==TRANS_WRITE );
  ప్రతిజ్ఞా( pBt->pPage1 && pBt->pPage1->a_దత్తాంశం );
  ప్రతిజ్ఞా( wrFlag==0 || (pBt->btsFlags & BTS_READ_ONLY)==0 );

  ఒకవేళ( i_పట్టిక<=1 ){
    ఒకవేళ( i_పట్టిక<1 ){
      తిరిగి SQLITE_CORRUPT_BKPT;
    }లేకపోతే ఒకవేళ( btreePagecount(pBt)==0 ){
      ప్రతిజ్ఞా( wrFlag==0 );
      i_పట్టిక = 0;
    }
  }

  /* Now that no other errors can occur, finish filling in the BtCursor
  ** variables and link the cursor into the BtShared list.  */
  pCur->pgno_మూలం = i_పట్టిక;
  pCur->i_పుట = -1;
  pCur->p_కీలకం_Info = p_కీలకం_Info;
  pCur->pBtree = p;
  pCur->pBt = pBt;
  pCur->curFlags = 0;
  /* If there are two or more cursors on the same btree, then all such
  ** cursors *must* have the BTCF_Multiple flag set. */
  కొరకు(pX=pBt->pCursor; pX; pX=pX->p_తదుపరి){
    ఒకవేళ( pX->pgno_మూలం==i_పట్టిక ){
      pX->curFlags |= BTCF_గుణిజం;
      pCur->curFlags = BTCF_గుణిజం;
    }
  }
  pCur->eState = CURSOR_INVALID;
  pCur->p_తదుపరి = pBt->pCursor;
  pBt->pCursor = pCur;
  ఒకవేళ( wrFlag ){
    pCur->curFlags |= BTCF_WriteFlag;
    pCur->curPagerFlags = 0;
    ఒకవేళ( pBt->pTmpSpace==0 ) తిరిగి allocate_తాత్కాలిక_Space(pBt);
  }లేకపోతే{
    pCur->curPagerFlags = PAGER_GET_READONLY;
  }
  తిరిగి SQLITE_OK;
}
స్థిర పూర్ణాంకం btree_Cursor_With_తాళం(
  Btree *p,                              /* The btree */
  Pgno i_పట్టిక,                           /* Root page of table to open */
  పూర్ణాంకం wrFlag,                            /* 1 to write. 0 read-only */
  నిర్మాణం కీలకం_Info *p_కీలకం_Info,              /* First arg to comparison function */
  BtCursor *pCur                         /* Space for new cursor */
){
  పూర్ణాంకం rc;
  sqlite3BtreeEnter(p);
  rc = btreeCursor(p, i_పట్టిక, wrFlag, p_కీలకం_Info, pCur);
  sqlite3BtreeLeave(p);
  తిరిగి rc;
}
పూర్ణాంకం sqlite3BtreeCursor(
  Btree *p,                                   /* The btree */
  Pgno i_పట్టిక,                                /* Root page of table to open */
  పూర్ణాంకం wrFlag,                                 /* 1 to write. 0 read-only */
  నిర్మాణం కీలకం_Info *p_కీలకం_Info,                   /* First arg to xCompare() */
  BtCursor *pCur                              /* Write new cursor here */
){
  ఒకవేళ( p->sharable ){
    తిరిగి btree_Cursor_With_తాళం(p, i_పట్టిక, wrFlag, p_కీలకం_Info, pCur);
  }లేకపోతే{
    తిరిగి btreeCursor(p, i_పట్టిక, wrFlag, p_కీలకం_Info, pCur);
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
పూర్ణాంకం sqlite3_Btree_Cursor_కొలత(శూన్య){
  తిరిగి ROUND8(పరిమాణం(BtCursor));
}

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Return true if and only if the Btree object will be automatically
** closed with the BtCursor closes.  This is used within assert() statements
** only.
*/
పూర్ణాంకం sqlite3BtreeClosesWithCursor(
  Btree *pBtree,       /* the btree object */
  BtCursor *pCur       /* Corresponding cursor */
){
  BtShared *pBt = pBtree->pBt;
  ఒకవేళ( (pBt->openFlags & BTREE_SINGLE)==0 ) తిరిగి 0;
  ఒకవేళ( pBt->pCursor!=pCur ) తిరిగి 0;
  ఒకవేళ( pCur->p_తదుపరి!=0 ) తిరిగి 0;
  ఒకవేళ( pCur->pBtree!=pBtree ) తిరిగి 0;
  తిరిగి 1;
}
#అయితే_ముగింపు

/*
** Initialize memory that will be converted into a BtCursor object.
**
** The simple approach here would be to memset() the entire object
** to zero.  But it turns out that the apPage[] and aiIdx[] arrays
** do not need to be zeroed and they are large, so we can save a lot
** of run-time by skipping the initialization of those elements.
*/
శూన్య sqlite3BtreeCursorZero(BtCursor *p){
  memset(p, 0, offsetof(BtCursor, BTCURSOR_మొదటి_UNINIT));
}

/*
** Close a cursor.  The read lock on the database file is released
** when the last cursor is closed.
*/
పూర్ణాంకం sqlite3BtreeCloseCursor(BtCursor *pCur){
  Btree *pBtree = pCur->pBtree;
  ఒకవేళ( pBtree ){
    BtShared *pBt = pCur->pBt;
    sqlite3BtreeEnter(pBtree);
    ప్రతిజ్ఞా( pBt->pCursor!=0 );
    ఒకవేళ( pBt->pCursor==pCur ){
      pBt->pCursor = pCur->p_తదుపరి;
    }లేకపోతే{
      BtCursor *pPrev = pBt->pCursor;
      చేయి{
        ఒకవేళ( pPrev->p_తదుపరి==pCur ){
          pPrev->p_తదుపరి = pCur->p_తదుపరి;
          ఆపు;
        }
        pPrev = pPrev->p_తదుపరి;
      }అయితే( ALWAYS(pPrev) );
    }
    btree_Release_All_Cursor_పుటలు(pCur);
    unlock_Btree_If_వాడని(pBt);
    sqlite3_free(pCur->a_అధికప్రవాహం);
    sqlite3_free(pCur->p_కీలకం);
    ఒకవేళ( (pBt->openFlags & BTREE_SINGLE) && pBt->pCursor==0 ){
      /* Since the BtShared is not sharable, there is no need to
      ** worry about the missing sqlite3BtreeLeave() call here.  */
      ప్రతిజ్ఞా( pBtree->sharable==0 );
      sqlite3BtreeClose(pBtree);
    }లేకపోతే{
      sqlite3BtreeLeave(pBtree);
    }
    pCur->pBtree = 0;
  }
  తిరిగి SQLITE_OK;
}

/*
** Make sure the BtCursor* given in the argument has a valid
** BtCursor.info structure.  If it is not already valid, call
** btreeParseCell() to fill it in.
**
** BtCursor.info is a cache of the information in the current cell.
** Using this cache reduces the number of calls to btreeParseCell().
*/
#నిర్వచితం_కాకపోతే NDEBUG
  స్థిర పూర్ణాంకం cellInfoEqual(CellInfo *a, CellInfo *b){
    ఒకవేళ( a->n_కీలకం!=b->n_కీలకం ) తిరిగి 0;
    ఒకవేళ( a->pPayload!=b->pPayload ) తిరిగి 0;
    ఒకవేళ( a->nPayload!=b->nPayload ) తిరిగి 0;
    ఒకవేళ( a->nLocal!=b->nLocal ) తిరిగి 0;
    ఒకవేళ( a->n_కొలత!=b->n_కొలత ) తిరిగి 0;
    తిరిగి 1;
  }
  స్థిర శూన్య assertCellInfo(BtCursor *pCur){
    CellInfo info;
    memset(&info, 0, పరిమాణం(info));
    btreeParseCell(pCur->p_పుట, pCur->ix, &info);
    ప్రతిజ్ఞా( CORRUPT_DB || cellInfoEqual(&info, &pCur->info) );
  }
#లేకపోతే
  #నిర్వచించు assertCellInfo(x)
#అయితే_ముగింపు
స్థిర SQLITE_NOINLINE శూన్య getCellInfo(BtCursor *pCur){
  ఒకవేళ( pCur->info.n_కొలత==0 ){
    pCur->curFlags |= BTCF_ValidNKey;
    btreeParseCell(pCur->p_పుట,pCur->ix,&pCur->info);
  }లేకపోతే{
    assertCellInfo(pCur);
  }
}

#నిర్వచితం_కాకపోతే NDEBUG  /* The next routine used only within assert() statements */
/*
** Return true if the given BtCursor is valid.  A valid cursor is one
** that is currently pointing to a row in a (non-empty) table.
** This is a verification routine is used only within assert() statements.
*/
పూర్ణాంకం sqlite3BtreeCursorIsValid(BtCursor *pCur){
  తిరిగి pCur && pCur->eState==CURSOR_VALID;
}
#అయితే_ముగింపు /* NDEBUG */
పూర్ణాంకం sqlite3BtreeCursorIsValidNN(BtCursor *pCur){
  ప్రతిజ్ఞా( pCur!=0 );
  తిరిగి pCur->eState==CURSOR_VALID;
}

/*
** Return the value of the integer key or "rowid" for a table btree.
** This routine is only valid for a cursor that is pointing into a
** ordinary table btree.  If the cursor points to an index btree or
** is invalid, the result of this routine is undefined.
*/
i64 sqlite3_Btree_Integer_కీలకం(BtCursor *pCur){
  ప్రతిజ్ఞా( cursorHoldsMutex(pCur) );
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  ప్రతిజ్ఞా( pCur->cur_Int_కీలకం );
  getCellInfo(pCur);
  తిరిగి pCur->info.n_కీలకం;
}

/*
** Pin or unpin a cursor.
*/
శూన్య sqlite3BtreeCursorPin(BtCursor *pCur){
  ప్రతిజ్ఞా( (pCur->curFlags & BTCF_Pinned)==0 );
  pCur->curFlags |= BTCF_Pinned;
}
శూన్య sqlite3BtreeCursorUnpin(BtCursor *pCur){
  ప్రతిజ్ఞా( (pCur->curFlags & BTCF_Pinned)!=0 );
  pCur->curFlags &= ~BTCF_Pinned;
}

/*
** Return the offset into the database file for the start of the
** payload to which the cursor is pointing.
*/
i64 sqlite3_Btree_స్థానభ్రంశం(BtCursor *pCur){
  ప్రతిజ్ఞా( cursorHoldsMutex(pCur) );
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  getCellInfo(pCur);
  తిరిగి (i64)pCur->pBt->పుట_కొలత*((i64)pCur->p_పుట->pgno - 1) +
         (i64)(pCur->info.pPayload - pCur->p_పుట->a_దత్తాంశం);
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
u32 sqlite3_Btree_Payload_కొలత(BtCursor *pCur){
  ప్రతిజ్ఞా( cursorHoldsMutex(pCur) );
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  getCellInfo(pCur);
  తిరిగి pCur->info.nPayload;
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
sqlite3_int64 sqlite3_Btree_Max_రికార్డు_కొలత(BtCursor *pCur){
  ప్రతిజ్ఞా( cursorHoldsMutex(pCur) );
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  తిరిగి pCur->pBt->పుట_కొలత * (sqlite3_int64)pCur->pBt->n_పుట;
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
స్థిర పూర్ణాంకం get_అధికప్రవాహం_పుట(
  BtShared *pBt,               /* The database file */
  Pgno ovfl,                   /* Current overflow page number */
  Mem_పుట **pp_పుట,            /* OUT: MemPage handle (may be NULL) */
  Pgno *p_Pgno_తదుపరి              /* OUT: Next overflow page number */
){
  Pgno తదుపరి = 0;
  Mem_పుట *p_పుట = 0;
  పూర్ణాంకం rc = SQLITE_OK;

  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ప్రతిజ్ఞా(p_Pgno_తదుపరి);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
  /* Try to find the next page in the overflow list using the
  ** autovacuum pointer-map pages. Guess that the next page in
  ** the overflow list is page number (ovfl+1). If that guess turns
  ** out to be wrong, fall back to loading the data of page
  ** number ovfl to determine the next page number.
  */
  ఒకవేళ( pBt->autoVacuum ){
    Pgno pgno;
    Pgno iGuess = ovfl+1;
    u8 eType;

    అయితే( PTRMAP_ISPAGE(pBt, iGuess) || iGuess==PENDING_BYTE_పుట(pBt) ){
      iGuess++;
    }

    ఒకవేళ( iGuess<=btreePagecount(pBt) ){
      rc = ptrmapGet(pBt, iGuess, &eType, &pgno);
      ఒకవేళ( rc==SQLITE_OK && eType==PTRMAP_OVERFLOW2 && pgno==ovfl ){
        తదుపరి = iGuess;
        rc = SQLITE_పూర్తి;
      }
    }
  }
#అయితే_ముగింపు

  ప్రతిజ్ఞా( తదుపరి==0 || rc==SQLITE_పూర్తి );
  ఒకవేళ( rc==SQLITE_OK ){
    rc = btree_Get_పుట(pBt, ovfl, &p_పుట, (pp_పుట==0) ? PAGER_GET_READONLY : 0);
    ప్రతిజ్ఞా( rc==SQLITE_OK || p_పుట==0 );
    ఒకవేళ( rc==SQLITE_OK ){
      తదుపరి = get4byte(p_పుట->a_దత్తాంశం);
    }
  }

  *p_Pgno_తదుపరి = తదుపరి;
  ఒకవేళ( pp_పుట ){
    *pp_పుట = p_పుట;
  }లేకపోతే{
    release_పుట(p_పుట);
  }
  తిరిగి (rc==SQLITE_పూర్తి ? SQLITE_OK : rc);
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
స్థిర పూర్ణాంకం copyPayload(
  శూన్య *pPayload,           /* Pointer to page data */
  శూన్య *pBuf,               /* Pointer to buffer */
  పూర్ణాంకం nByte,                /* Number of bytes to copy */
  పూర్ణాంకం eOp,                  /* 0 -> copy from page, 1 -> copy to page */
  Db_పుట *p_Db_పుట           /* Page containing pPayload */
){
  ఒకవేళ( eOp ){
    /* Copy data from buffer to page (a write operation) */
    పూర్ణాంకం rc = sqlite3PagerWrite(p_Db_పుట);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
    జ్ఞాపకం_నకలు(pPayload, pBuf, nByte);
  }లేకపోతే{
    /* Copy data from page to buffer (a read operation) */
    జ్ఞాపకం_నకలు(pBuf, pPayload, nByte);
  }
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం accessPayload(
  BtCursor *pCur,      /* Cursor pointing to entry to read from */
  u32 స్థానభ్రంశం,          /* Begin reading this far into payload */
  u32 amt,             /* Read this many bytes */
  గుర్తులేని అక్షరం *pBuf, /* Write the bytes into this buffer */
  పూర్ణాంకం eOp              /* zero to read. non-zero to write. */
){
  గుర్తులేని అక్షరం *aPayload;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం iIdx = 0;
  Mem_పుట *p_పుట = pCur->p_పుట;               /* Btree page of current entry */
  BtShared *pBt = pCur->pBt;                  /* Btree this cursor belongs to */
#నిర్వచితమైతే SQLITE_ప్రత్యక్ష_అధికప్రవాహం_READ
  గుర్తులేని అక్షరం * స్థిరరాశి p_Buf_ప్రారంభం = pBuf;     /* Start of original out buffer */
#అయితే_ముగింపు

  ప్రతిజ్ఞా( p_పుట );
  ప్రతిజ్ఞా( eOp==0 || eOp==1 );
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  ఒకవేళ( pCur->ix>=p_పుట->nCell ){
    తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
  }
  ప్రతిజ్ఞా( cursorHoldsMutex(pCur) );

  getCellInfo(pCur);
  aPayload = pCur->info.pPayload;
  ప్రతిజ్ఞా( (u64)స్థానభ్రంశం+(u64)amt <= (u64)pCur->info.nPayload );

  ప్రతిజ్ఞా( aPayload > p_పుట->a_దత్తాంశం );
  ఒకవేళ( (uptr)(aPayload - p_పుట->a_దత్తాంశం) > (pBt->usable_కొలత - pCur->info.nLocal) ){
    /* Trying to read or write past the end of the data is an error.  The
    ** conditional above is really:
    **    &aPayload[pCur->info.nLocal] > &pPage->aData[pBt->usableSize]
    ** but is recast into its current form to avoid integer overflow problems
    */
    తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
  }

  /* Check if data must be read/written to/from the btree page itself. */
  ఒకవేళ( స్థానభ్రంశం<pCur->info.nLocal ){
    పూర్ణాంకం a = amt;
    ఒకవేళ( a+స్థానభ్రంశం>pCur->info.nLocal ){
      a = pCur->info.nLocal - స్థానభ్రంశం;
    }
    rc = copyPayload(&aPayload[స్థానభ్రంశం], pBuf, a, eOp, p_పుట->p_Db_పుట);
    స్థానభ్రంశం = 0;
    pBuf += a;
    amt -= a;
  }లేకపోతే{
    స్థానభ్రంశం -= pCur->info.nLocal;
  }


  ఒకవేళ( rc==SQLITE_OK && amt>0 ){
    స్థిరరాశి u32 ovfl_కొలత = pBt->usable_కొలత - 4;  /* Bytes content per ovfl page */
    Pgno తదుపరి_పుట;

    తదుపరి_పుట = get4byte(&aPayload[pCur->info.nLocal]);

    /* If the BtCursor.aOverflow[] has not been allocated, allocate it now.
    **
    ** The aOverflow[] array is sized at one entry for each overflow page
    ** in the overflow chain. The page number of the first overflow page is
    ** stored in aOverflow[0], etc. A value of 0 in the aOverflow[] array
    ** means "not yet known" (the cache is lazily populated).
    */
    ఒకవేళ( (pCur->curFlags & BTCF_ValidOvfl)==0 ){
      i64 nOvfl = pCur->info.nPayload;
      testcase( nOvfl - pCur->info.nLocal + ovfl_కొలత - 1 > 0xffffffffU );
      nOvfl = (nOvfl - pCur->info.nLocal + ovfl_కొలత-1)/ovfl_కొలత;
      ఒకవేళ( pCur->a_అధికప్రవాహం==0
       || nOvfl*(పూర్ణాంకం)పరిమాణం(Pgno) > sqlite3_Malloc_కొలత(pCur->a_అధికప్రవాహం)
      ){
        Pgno *aNew;
        ఒకవేళ( sqlite3FaultSim(413) ){
          aNew = 0;
        }లేకపోతే{
          aNew = (Pgno*)sqlite3Realloc(pCur->a_అధికప్రవాహం, nOvfl*2*పరిమాణం(Pgno));
        }
        ఒకవేళ( aNew==0 ){
          తిరిగి SQLITE_NOMEM_BKPT;
        }లేకపోతే{
          pCur->a_అధికప్రవాహం = aNew;
        }
      }
      memset(pCur->a_అధికప్రవాహం, 0, nOvfl*పరిమాణం(Pgno));
      pCur->curFlags |= BTCF_ValidOvfl;
    }లేకపోతే{
      /* Sanity check the validity of the overflow page cache */
      ప్రతిజ్ఞా( pCur->a_అధికప్రవాహం[0]==తదుపరి_పుట
           || pCur->a_అధికప్రవాహం[0]==0
           || CORRUPT_DB );
      ప్రతిజ్ఞా( pCur->a_అధికప్రవాహం[0]!=0 || pCur->a_అధికప్రవాహం[స్థానభ్రంశం/ovfl_కొలత]==0 );

      /* If the overflow page-list cache has been allocated and the
      ** entry for the first required overflow page is valid, skip
      ** directly to it.
      */
      ఒకవేళ( pCur->a_అధికప్రవాహం[స్థానభ్రంశం/ovfl_కొలత] ){
        iIdx = (స్థానభ్రంశం/ovfl_కొలత);
        తదుపరి_పుట = pCur->a_అధికప్రవాహం[iIdx];
        స్థానభ్రంశం = (స్థానభ్రంశం%ovfl_కొలత);
      }
    }

    ప్రతిజ్ఞా( rc==SQLITE_OK && amt>0 );
    అయితే( తదుపరి_పుట ){
      /* If required, populate the overflow page-list cache. */
      ఒకవేళ( తదుపరి_పుట > pBt->n_పుట ) తిరిగి SQLITE_CORRUPT_BKPT;
      ప్రతిజ్ఞా( pCur->a_అధికప్రవాహం[iIdx]==0
              || pCur->a_అధికప్రవాహం[iIdx]==తదుపరి_పుట
              || CORRUPT_DB );
      pCur->a_అధికప్రవాహం[iIdx] = తదుపరి_పుట;

      ఒకవేళ( స్థానభ్రంశం>=ovfl_కొలత ){
        /* The only reason to read this page is to obtain the page
        ** number for the next page in the overflow chain. The page
        ** data is not required. So first try to lookup the overflow
        ** page-list cache, if any, then fall back to the getOverflowPage()
        ** function.
        */
        ప్రతిజ్ఞా( pCur->curFlags & BTCF_ValidOvfl );
        ప్రతిజ్ఞా( pCur->pBtree->db==pBt->db );
        ఒకవేళ( pCur->a_అధికప్రవాహం[iIdx+1] ){
          తదుపరి_పుట = pCur->a_అధికప్రవాహం[iIdx+1];
        }లేకపోతే{
          rc = get_అధికప్రవాహం_పుట(pBt, తదుపరి_పుట, 0, &తదుపరి_పుట);
        }
        స్థానభ్రంశం -= ovfl_కొలత;
      }లేకపోతే{
        /* Need to read this page properly. It contains some of the
        ** range of data that is being read (eOp==0) or written (eOp!=0).
        */
        పూర్ణాంకం a = amt;
        ఒకవేళ( a + స్థానభ్రంశం > ovfl_కొలత ){
          a = ovfl_కొలత - స్థానభ్రంశం;
        }

#నిర్వచితమైతే SQLITE_ప్రత్యక్ష_అధికప్రవాహం_READ
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
        ఒకవేళ( eOp==0                                             /* (1) */
         && స్థానభ్రంశం==0                                          /* (2) */
         && sqlite3_Pager_ప్రత్యక్ష_Read_Ok(pBt->pPager, తదుపరి_పుట)    /* (3,4,5) */
         && &pBuf[-4]>=p_Buf_ప్రారంభం                               /* (6) */
        ){
          sqlite3_file *fd = sqlite3PagerFile(pBt->pPager);
          u8 aSave[4];
          u8 *aWrite = &pBuf[-4];
          ప్రతిజ్ఞా( aWrite>=p_Buf_ప్రారంభం );                         /* due to (6) */
          జ్ఞాపకం_నకలు(aSave, aWrite, 4);
          rc = sqlite3OsRead(fd, aWrite, a+4, (i64)pBt->పుట_కొలత*(తదుపరి_పుట-1));
          తదుపరి_పుట = get4byte(aWrite);
          జ్ఞాపకం_నకలు(aWrite, aSave, 4);
        }లేకపోతే
#అయితే_ముగింపు

        {
          Db_పుట *p_Db_పుట;
          rc = sqlite3PagerGet(pBt->pPager, తదుపరి_పుట, &p_Db_పుట,
              (eOp==0 ? PAGER_GET_READONLY : 0)
          );
          ఒకవేళ( rc==SQLITE_OK ){
            ఒకవేళ( eOp!=0
             && (sqlite3_Pager_పుట_Refcount(p_Db_పుట)!=1
                 || NEVER(((Mem_పుట*)sqlite3PagerGetExtra(p_Db_పుట))->isInit))
             && sqlite3FaultSim(411)==SQLITE_OK
            ){
              sqlite3PagerUnref(p_Db_పుట);
              తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
            }
            aPayload = sqlite3_Pager_Get_దత్తాంశం(p_Db_పుట);
            తదుపరి_పుట = get4byte(aPayload);
            rc = copyPayload(&aPayload[స్థానభ్రంశం+4], pBuf, a, eOp, p_Db_పుట);
            sqlite3PagerUnref(p_Db_పుట);
            స్థానభ్రంశం = 0;
          }
        }
        amt -= a;
        ఒకవేళ( amt==0 ) తిరిగి rc;
        pBuf += a;
      }
      ఒకవేళ( rc ) ఆపు;
      iIdx++;
    }
  }

  ఒకవేళ( rc==SQLITE_OK && amt>0 ){
    /* Overflow chain ends prematurely */
    తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
  }
  తిరిగి rc;
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
పూర్ణాంకం sqlite3BtreePayload(BtCursor *pCur, u32 స్థానభ్రంశం, u32 amt, శూన్య *pBuf){
  ప్రతిజ్ఞా( cursorHoldsMutex(pCur) );
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  ప్రతిజ్ఞా( pCur->i_పుట>=0 && pCur->p_పుట );
  తిరిగి accessPayload(pCur, స్థానభ్రంశం, amt, (గుర్తులేని అక్షరం*)pBuf, 0);
}

/*
** This variant of sqlite3BtreePayload() works even if the cursor has not
** in the CURSOR_VALID state.  It is only used by the sqlite3_blob_read()
** interface.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB
స్థిర SQLITE_NOINLINE పూర్ణాంకం accessPayloadChecked(
  BtCursor *pCur,
  u32 స్థానభ్రంశం,
  u32 amt,
  శూన్య *pBuf
){
  పూర్ణాంకం rc;
  ఒకవేళ ( pCur->eState==CURSOR_INVALID ){
    తిరిగి SQLITE_ABORT;
  }
  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  rc = btreeRestoreCursorPosition(pCur);
  తిరిగి rc ? rc : accessPayload(pCur, స్థానభ్రంశం, amt, pBuf, 0);
}
పూర్ణాంకం sqlite3BtreePayloadChecked(BtCursor *pCur, u32 స్థానభ్రంశం, u32 amt, శూన్య *pBuf){
  ఒకవేళ( pCur->eState==CURSOR_VALID ){
    ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
    తిరిగి accessPayload(pCur, స్థానభ్రంశం, amt, pBuf, 0);
  }లేకపోతే{
    తిరిగి accessPayloadChecked(pCur, స్థానభ్రంశం, amt, pBuf);
  }
}
#అయితే_ముగింపు /* SQLITE_OMIT_INCRBLOB */

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
స్థిర స్థిరరాశి శూన్య *fetchPayload(
  BtCursor *pCur,      /* Cursor pointing to entry to read from */
  u32 *pAmt            /* Write the number of available bytes here */
){
  పూర్ణాంకం amt;
  ప్రతిజ్ఞా( pCur!=0 && pCur->i_పుట>=0 && pCur->p_పుట);
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( pCur->ix<pCur->p_పుట->nCell || CORRUPT_DB );
  ప్రతిజ్ఞా( pCur->info.n_కొలత>0 );
  ప్రతిజ్ఞా( pCur->info.pPayload>pCur->p_పుట->a_దత్తాంశం || CORRUPT_DB );
  ప్రతిజ్ఞా( pCur->info.pPayload<pCur->p_పుట->a_దత్తాంశం_చివర ||CORRUPT_DB);
  amt = pCur->info.nLocal;
  ఒకవేళ( amt>(పూర్ణాంకం)(pCur->p_పుట->a_దత్తాంశం_చివర - pCur->info.pPayload) ){
    /* There is too little space on the page for the expected amount
    ** of local content. Database must be corrupt. */
    ప్రతిజ్ఞా( CORRUPT_DB );
    amt = MAX(0, (పూర్ణాంకం)(pCur->p_పుట->a_దత్తాంశం_చివర - pCur->info.pPayload));
  }
  *pAmt = (u32)amt;
  తిరిగి (శూన్య*)pCur->info.pPayload;
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
స్థిరరాశి శూన్య *sqlite3BtreePayloadFetch(BtCursor *pCur, u32 *pAmt){
  తిరిగి fetchPayload(pCur, pAmt);
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
స్థిర పూర్ణాంకం move_గమ్యం_బిడ్డ(BtCursor *pCur, u32 newPgno){
  పూర్ణాంకం rc;
  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  ప్రతిజ్ఞా( pCur->i_పుట<BTCURSOR_MAX_లోతు );
  ప్రతిజ్ఞా( pCur->i_పుట>=0 );
  ఒకవేళ( pCur->i_పుట>=(BTCURSOR_MAX_లోతు-1) ){
    తిరిగి SQLITE_CORRUPT_BKPT;
  }
  pCur->info.n_కొలత = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  pCur->aiIdx[pCur->i_పుట] = pCur->ix;
  pCur->ap_పుట[pCur->i_పుట] = pCur->p_పుట;
  pCur->ix = 0;
  pCur->i_పుట++;
  rc = get_And_Init_పుట(pCur->pBt, newPgno, &pCur->p_పుట, pCur->curPagerFlags);
  ప్రతిజ్ఞా( pCur->p_పుట!=0 || rc!=SQLITE_OK );
  ఒకవేళ( rc==SQLITE_OK
   && (pCur->p_పుట->nCell<1 || pCur->p_పుట->int_కీలకం!=pCur->cur_Int_కీలకం)
  ){
    release_పుట(pCur->p_పుట);
    rc = SQLITE_CORRUPT_PGNO(newPgno);
  }
  ఒకవేళ( rc ){
    pCur->p_పుట = pCur->ap_పుట[--pCur->i_పుట];
  }
  తిరిగి rc;
}

#నిర్వచితమైతే SQLITE_DEBUG
/*
** Page pParent is an internal (non-leaf) tree page. This function
** asserts that page number iChild is the left-child if the iIdx'th
** cell in page pParent. Or, if iIdx is equal to the total number of
** cells in pParent, that page number iChild is the right-child of
** the page.
*/
స్థిర శూన్య assert_మాతృ_సూచీ(Mem_పుట *p_మాతృ, పూర్ణాంకం iIdx, Pgno i_బిడ్డ){
  ఒకవేళ( CORRUPT_DB ) తిరిగి;  /* The conditions tested below might not be true
                            ** in a corrupt database */
  ప్రతిజ్ఞా( iIdx<=p_మాతృ->nCell );
  ఒకవేళ( iIdx==p_మాతృ->nCell ){
    ప్రతిజ్ఞా( get4byte(&p_మాతృ->a_దత్తాంశం[p_మాతృ->hdr_స్థానభ్రంశం+8])==i_బిడ్డ );
  }లేకపోతే{
    ప్రతిజ్ఞా( get4byte(కనుగొను_Cell(p_మాతృ, iIdx))==i_బిడ్డ );
  }
}
#లేకపోతే
#  define assert_మాతృ_సూచీ(x,y,z)
#అయితే_ముగింపు

/*
** Move the cursor up to the parent page.
**
** pCur->idx is set to the cell index that contains the pointer
** to the page we are coming from.  If we are coming from the
** right-most child page then pCur->idx is set to one more than
** the largest cell index.
*/
స్థిర శూన్య move_గమ్యం_మాతృ(BtCursor *pCur){
  Mem_పుట *p_ఆకు;
  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  ప్రతిజ్ఞా( pCur->i_పుట>0 );
  ప్రతిజ్ఞా( pCur->p_పుట );
  assert_మాతృ_సూచీ(
    pCur->ap_పుట[pCur->i_పుట-1],
    pCur->aiIdx[pCur->i_పుట-1],
    pCur->p_పుట->pgno
  );
  testcase( pCur->aiIdx[pCur->i_పుట-1] > pCur->ap_పుట[pCur->i_పుట-1]->nCell );
  pCur->info.n_కొలత = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  pCur->ix = pCur->aiIdx[pCur->i_పుట-1];
  p_ఆకు = pCur->p_పుట;
  pCur->p_పుట = pCur->ap_పుట[--pCur->i_పుట];
  release_పుట_Not_Null(p_ఆకు);
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
స్థిర పూర్ణాంకం move_గమ్యం_మూలం(BtCursor *pCur){
  Mem_పుట *p_మూలం;
  పూర్ణాంకం rc = SQLITE_OK;

  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( CURSOR_INVALID < CURSOR_REQUIRESEEK );
  ప్రతిజ్ఞా( CURSOR_VALID   < CURSOR_REQUIRESEEK );
  ప్రతిజ్ఞా( CURSOR_FAULT   > CURSOR_REQUIRESEEK );
  ప్రతిజ్ఞా( pCur->eState < CURSOR_REQUIRESEEK || pCur->i_పుట<0 );
  ప్రతిజ్ఞా( pCur->pgno_మూలం>0 || pCur->i_పుట<0 );

  ఒకవేళ( pCur->i_పుట>=0 ){
    ఒకవేళ( pCur->i_పుట ){
      release_పుట_Not_Null(pCur->p_పుట);
      అయితే( --pCur->i_పుట ){
        release_పుట_Not_Null(pCur->ap_పుట[pCur->i_పుట]);
      }
      p_మూలం = pCur->p_పుట = pCur->ap_పుట[0];
      వెళ్ళు skip_init;
    }
  }లేకపోతే ఒకవేళ( pCur->pgno_మూలం==0 ){
    pCur->eState = CURSOR_INVALID;
    తిరిగి SQLITE_ఖాళీలు;
  }లేకపోతే{
    ప్రతిజ్ఞా( pCur->i_పుట==(-1) );
    ఒకవేళ( pCur->eState>=CURSOR_REQUIRESEEK ){
      ఒకవేళ( pCur->eState==CURSOR_FAULT ){
        ప్రతిజ్ఞా( pCur->skip_తదుపరి!=SQLITE_OK );
        తిరిగి pCur->skip_తదుపరి;
      }
      sqlite3BtreeClearCursor(pCur);
    }
    rc = get_And_Init_పుట(pCur->pBt, pCur->pgno_మూలం, &pCur->p_పుట,
                        pCur->curPagerFlags);
    ఒకవేళ( rc!=SQLITE_OK ){
      pCur->eState = CURSOR_INVALID;
      తిరిగి rc;
    }
    pCur->i_పుట = 0;
    pCur->cur_Int_కీలకం = pCur->p_పుట->int_కీలకం;
  }
  p_మూలం = pCur->p_పుట;
  ప్రతిజ్ఞా( p_మూలం->pgno==pCur->pgno_మూలం || CORRUPT_DB );

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
  ప్రతిజ్ఞా( p_మూలం->int_కీలకం==1 || p_మూలం->int_కీలకం==0 );
  ఒకవేళ( p_మూలం->isInit==0 || (pCur->p_కీలకం_Info==0)!=p_మూలం->int_కీలకం ){
    తిరిగి SQLITE_CORRUPT_పుట(pCur->p_పుట);
  }

skip_init: 
  pCur->ix = 0;
  pCur->info.n_కొలత = 0;
  pCur->curFlags &= ~(BTCF_At_చివరి|BTCF_ValidNKey|BTCF_ValidOvfl);

  ఒకవేళ( p_మూలం->nCell>0 ){
    pCur->eState = CURSOR_VALID;
  }లేకపోతే ఒకవేళ( !p_మూలం->ఆకు ){
    Pgno subpage;
    ఒకవేళ( p_మూలం->pgno!=1 ) తిరిగి SQLITE_CORRUPT_BKPT;
    subpage = get4byte(&p_మూలం->a_దత్తాంశం[p_మూలం->hdr_స్థానభ్రంశం+8]);
    pCur->eState = CURSOR_VALID;
    rc = move_గమ్యం_బిడ్డ(pCur, subpage);
  }లేకపోతే{
    pCur->eState = CURSOR_INVALID;
    rc = SQLITE_ఖాళీలు;
  }
  తిరిగి rc;
}

/*
** Move the cursor down to the left-most leaf entry beneath the
** entry to which it is currently pointing.
**
** The left-most leaf is the one with the smallest key - the first
** in ascending order.
*/
స్థిర పూర్ణాంకం move_గమ్యం_Leftmost(BtCursor *pCur){
  Pgno pgno;
  పూర్ణాంకం rc = SQLITE_OK;
  Mem_పుట *p_పుట;

  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  అయితే( rc==SQLITE_OK && !(p_పుట = pCur->p_పుట)->ఆకు ){
    ప్రతిజ్ఞా( pCur->ix<p_పుట->nCell );
    pgno = get4byte(కనుగొను_Cell(p_పుట, pCur->ix));
    rc = move_గమ్యం_బిడ్డ(pCur, pgno);
  }
  తిరిగి rc;
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
స్థిర పూర్ణాంకం move_గమ్యం_Rightmost(BtCursor *pCur){
  Pgno pgno;
  పూర్ణాంకం rc = SQLITE_OK;
  Mem_పుట *p_పుట = 0;

  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  అయితే( !(p_పుట = pCur->p_పుట)->ఆకు ){
    pgno = get4byte(&p_పుట->a_దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+8]);
    pCur->ix = p_పుట->nCell;
    rc = move_గమ్యం_బిడ్డ(pCur, pgno);
    ఒకవేళ( rc ) తిరిగి rc;
  }
  pCur->ix = p_పుట->nCell-1;
  ప్రతిజ్ఞా( pCur->info.n_కొలత==0 );
  ప్రతిజ్ఞా( (pCur->curFlags & BTCF_ValidNKey)==0 );
  తిరిగి SQLITE_OK;
}

/* Move the cursor to the first entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
పూర్ణాంకం sqlite3_Btree_మొదటి(BtCursor *pCur, పూర్ణాంకం *pRes){
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  rc = move_గమ్యం_మూలం(pCur);
  ఒకవేళ( rc==SQLITE_OK ){
    ప్రతిజ్ఞా( pCur->p_పుట->nCell>0 );
    *pRes = 0;
    rc = move_గమ్యం_Leftmost(pCur);
  }లేకపోతే ఒకవేళ( rc==SQLITE_ఖాళీలు ){
    ప్రతిజ్ఞా( pCur->pgno_మూలం==0 || (pCur->p_పుట!=0 && pCur->p_పుట->nCell==0) );
    *pRes = 1;
    rc = SQLITE_OK;
  }
  తిరిగి rc;
}

/* Set *pRes to 1 (true) if the BTree pointed to by cursor pCur contains zero
** rows of content.  Set *pRes to 0 (false) if the table contains content.
** Return SQLITE_OK on success or some error code (ex: SQLITE_NOMEM) if
** something goes wrong.
*/
పూర్ణాంకం sqlite3_Btree_Is_ఖాళీలు(BtCursor *pCur, పూర్ణాంకం *pRes){
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ఒకవేళ( NEVER(pCur->eState==CURSOR_VALID) ){
    *pRes = 0;
    తిరిగి SQLITE_OK;
  }
  rc = move_గమ్యం_మూలం(pCur);
  ఒకవేళ( rc==SQLITE_ఖాళీలు ){
    *pRes = 1;
    rc = SQLITE_OK;
  }లేకపోతే{
    *pRes = 0;
  }
  తిరిగి rc;
}

#నిర్వచితమైతే SQLITE_DEBUG
/* The cursors is CURSOR_VALID and has BTCF_AtLast set.  Verify that
** this flags are true for a consistent database.
**
** This routine is is called from within assert() statements only.
** It is an internal verification routine and does not appear in production
** builds.
*/
స్థిర పూర్ణాంకం cursor_Is_At_చివరి_నమోదు(BtCursor *pCur){
  పూర్ణాంకం ii;
  కొరకు(ii=0; ii<pCur->i_పుట; ii++){
    ఒకవేళ( pCur->aiIdx[ii]!=pCur->ap_పుట[ii]->nCell ) తిరిగి 0;
  }
  తిరిగి pCur->ix==pCur->p_పుట->nCell-1 && pCur->p_పుట->ఆకు!=0;
}
#అయితే_ముగింపు

/* Move the cursor to the last entry in the table.  Return SQLITE_OK
** on success.  Set *pRes to 0 if the cursor actually points to something
** or set *pRes to 1 if the table is empty.
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం btree_చివరి(BtCursor *pCur, పూర్ణాంకం *pRes){
  పూర్ణాంకం rc = move_గమ్యం_మూలం(pCur);
  ఒకవేళ( rc==SQLITE_OK ){
    ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
    *pRes = 0;
    rc = move_గమ్యం_Rightmost(pCur);
    ఒకవేళ( rc==SQLITE_OK ){
      pCur->curFlags |= BTCF_At_చివరి;
    }లేకపోతే{
      pCur->curFlags &= ~BTCF_At_చివరి;
    }
  }లేకపోతే ఒకవేళ( rc==SQLITE_ఖాళీలు ){
    ప్రతిజ్ఞా( pCur->pgno_మూలం==0 || pCur->p_పుట->nCell==0 );
    *pRes = 1;
    rc = SQLITE_OK;
  }
  తిరిగి rc;
}
పూర్ణాంకం sqlite3_Btree_చివరి(BtCursor *pCur, పూర్ణాంకం *pRes){
  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* If the cursor already points to the last entry, this is a no-op. */
  ఒకవేళ( CURSOR_VALID==pCur->eState && (pCur->curFlags & BTCF_At_చివరి)!=0 ){
    ప్రతిజ్ఞా( cursor_Is_At_చివరి_నమోదు(pCur) || CORRUPT_DB );
    *pRes = 0;
    తిరిగి SQLITE_OK;
  }
  తిరిగి btree_చివరి(pCur, pRes);
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
పూర్ణాంకం sqlite3_Btree_పట్టిక_Moveto(
  BtCursor *pCur,          /* The cursor to be moved */
  i64 int_కీలకం,              /* The table key */
  పూర్ణాంకం bias_కుడి,           /* If true, bias the search to the high end */
  పూర్ణాంకం *pRes                /* Write search results here */
){
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ప్రతిజ్ఞా( pRes );
  ప్రతిజ్ఞా( pCur->p_కీలకం_Info==0 );
  ప్రతిజ్ఞా( pCur->eState!=CURSOR_VALID || pCur->cur_Int_కీలకం!=0 );

  /* If the cursor is already positioned at the point we are trying
  ** to move to, then just return without doing any work */
  ఒకవేళ( pCur->eState==CURSOR_VALID && (pCur->curFlags & BTCF_ValidNKey)!=0 ){
    ఒకవేళ( pCur->info.n_కీలకం==int_కీలకం ){
      *pRes = 0;
      తిరిగి SQLITE_OK;
    }
    ఒకవేళ( pCur->info.n_కీలకం<int_కీలకం ){
      ఒకవేళ( (pCur->curFlags & BTCF_At_చివరి)!=0 ){
        ప్రతిజ్ఞా( cursor_Is_At_చివరి_నమోదు(pCur) || CORRUPT_DB );
        *pRes = -1;
        తిరిగి SQLITE_OK;
      }
      /* If the requested key is one more than the previous key, then
      ** try to get there using sqlite3BtreeNext() rather than a full
      ** binary search.  This is an optimization only.  The correct answer
      ** is still obtained without this case, only a little more slowly. */
      ఒకవేళ( pCur->info.n_కీలకం+1==int_కీలకం ){
        *pRes = 0;
        rc = sqlite3_Btree_తదుపరి(pCur, 0);
        ఒకవేళ( rc==SQLITE_OK ){
          getCellInfo(pCur);
          ఒకవేళ( pCur->info.n_కీలకం==int_కీలకం ){
            తిరిగి SQLITE_OK;
          }
        }లేకపోతే ఒకవేళ( rc!=SQLITE_పూర్తి ){
          తిరిగి rc;
        }
      }
    }
  }

#నిర్వచితమైతే SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#అయితే_ముగింపు

  rc = move_గమ్యం_మూలం(pCur);
  ఒకవేళ( rc ){
    ఒకవేళ( rc==SQLITE_ఖాళీలు ){
      ప్రతిజ్ఞా( pCur->pgno_మూలం==0 || pCur->p_పుట->nCell==0 );
      *pRes = -1;
      తిరిగి SQLITE_OK;
    }
    తిరిగి rc;
  }
  ప్రతిజ్ఞా( pCur->p_పుట );
  ప్రతిజ్ఞా( pCur->p_పుట->isInit );
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  ప్రతిజ్ఞా( pCur->p_పుట->nCell > 0 );
  ప్రతిజ్ఞా( pCur->i_పుట==0 || pCur->ap_పుట[0]->int_కీలకం==pCur->cur_Int_కీలకం );
  ప్రతిజ్ఞా( pCur->cur_Int_కీలకం );

  కొరకు(;;){
    పూర్ణాంకం lwr, upr, idx, c;
    Pgno chldPg;
    Mem_పుట *p_పుట = pCur->p_పుట;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    ప్రతిజ్ఞా( p_పుట->nCell>0 );
    ప్రతిజ్ఞా( p_పుట->int_కీలకం );
    lwr = 0;
    upr = p_పుట->nCell-1;
    ప్రతిజ్ఞా( bias_కుడి==0 || bias_కుడి==1 );
    idx = upr>>(1-bias_కుడి); /* idx = biasRight ? upr : (lwr+upr)/2; */
    కొరకు(;;){
      i64 n_Cell_కీలకం;
      pCell = కనుగొను_Cell_Past_Ptr(p_పుట, idx);
      ఒకవేళ( p_పుట->int_కీలకం_ఆకు ){
        అయితే( 0x80 <= *(pCell++) ){
          ఒకవేళ( pCell>=p_పుట->a_దత్తాంశం_చివర ){
            తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
          }
        }
      }
      n_Cell_కీలకం = sqlite3_Varint_విలువ(pCell);
      ఒకవేళ( n_Cell_కీలకం<int_కీలకం ){
        lwr = idx+1;
        ఒకవేళ( lwr>upr ){ c = -1; ఆపు; }
      }లేకపోతే ఒకవేళ( n_Cell_కీలకం>int_కీలకం ){
        upr = idx-1;
        ఒకవేళ( lwr>upr ){ c = +1; ఆపు; }
      }లేకపోతే{
        ప్రతిజ్ఞా( n_Cell_కీలకం==int_కీలకం );
        pCur->ix = (u16)idx;
        ఒకవేళ( !p_పుట->ఆకు ){
          lwr = idx;
          వెళ్ళు moveto_పట్టిక_తదుపరి_layer;
        }లేకపోతే{
          pCur->curFlags |= BTCF_ValidNKey;
          pCur->info.n_కీలకం = n_Cell_కీలకం;
          pCur->info.n_కొలత = 0;
          *pRes = 0;
          తిరిగి SQLITE_OK;
        }
      }
      ప్రతిజ్ఞా( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2; */
    }
    ప్రతిజ్ఞా( lwr==upr+1 || !p_పుట->ఆకు );
    ప్రతిజ్ఞా( p_పుట->isInit );
    ఒకవేళ( p_పుట->ఆకు ){
      ప్రతిజ్ఞా( pCur->ix<pCur->p_పుట->nCell );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      వెళ్ళు moveto_పట్టిక_ముగింపు;
    }
moveto_పట్టిక_తదుపరి_layer:
    ఒకవేళ( lwr>=p_పుట->nCell ){
      chldPg = get4byte(&p_పుట->a_దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+8]);
    }లేకపోతే{
      chldPg = get4byte(కనుగొను_Cell(p_పుట, lwr));
    }
    pCur->ix = (u16)lwr;
    rc = move_గమ్యం_బిడ్డ(pCur, chldPg);
    ఒకవేళ( rc ) ఆపు;
  }
moveto_పట్టిక_ముగింపు:
  pCur->info.n_కొలత = 0;
  ప్రతిజ్ఞా( (pCur->curFlags & BTCF_ValidOvfl)==0 );
  తిరిగి rc;
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
స్థిర పూర్ణాంకం సూచీ_Cell_Compare(
  Mem_పుట *p_పుట,
  పూర్ణాంకం idx,
  Unpacked_రికార్డు *p_Idx_కీలకం,
  రికార్డు_Compare x_రికార్డు_Compare
){
  పూర్ణాంకం c;
  పూర్ణాంకం nCell;  /* Size of the pCell cell in bytes */
  u8 *pCell = కనుగొను_Cell_Past_Ptr(p_పుట, idx);

  nCell = pCell[0];
  ఒకవేళ( nCell<=p_పుట->max1bytePayload ){
    /* This branch runs if the record-size field of the cell is a
    ** single byte varint and the record fits entirely on the main
    ** b-tree page.  */
    ఒకవేళ( pCell + nCell >= p_పుట->a_దత్తాంశం_చివర ) తిరిగి 99;
    c = x_రికార్డు_Compare(nCell, (శూన్య*)&pCell[1], p_Idx_కీలకం);
  }లేకపోతే ఒకవేళ( !(pCell[1] & 0x80)
    && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=p_పుట->maxLocal
  ){
    /* The record-size field is a 2 byte varint and the record
    ** fits entirely on the main b-tree page.  */
    ఒకవేళ( pCell + nCell >= p_పుట->a_దత్తాంశం_చివర ) తిరిగి 99;
    c = x_రికార్డు_Compare(nCell, (శూన్య*)&pCell[2], p_Idx_కీలకం);
  }లేకపోతే{
    /* If the record extends into overflow pages, do not attempt
    ** the optimization. */
    c = 99;
  }
  తిరిగి c;
}

/*
** Return true (non-zero) if pCur is current pointing to the last
** page of a table.
*/
స్థిర పూర్ణాంకం cursor_On_చివరి_పుట(BtCursor *pCur){
  పూర్ణాంకం i;
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  కొరకు(i=0; i<pCur->i_పుట; i++){
    Mem_పుట *p_పుట = pCur->ap_పుట[i];
    ఒకవేళ( pCur->aiIdx[i]<p_పుట->nCell ) తిరిగి 0;
  }
  తిరిగి 1;
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
పూర్ణాంకం sqlite3_Btree_సూచీ_Moveto(
  BtCursor *pCur,          /* The cursor to be moved */
  Unpacked_రికార్డు *p_Idx_కీలకం, /* Unpacked index key */
  పూర్ణాంకం *pRes                /* Write search results here */
){
  పూర్ణాంకం rc;
  రికార్డు_Compare x_రికార్డు_Compare;

  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCur->pBtree->db->mutex) );
  ప్రతిజ్ఞా( pRes );
  ప్రతిజ్ఞా( pCur->p_కీలకం_Info!=0 );

#నిర్వచితమైతే SQLITE_DEBUG
  pCur->pBtree->nSeek++;   /* Performance measurement during testing */
#అయితే_ముగింపు

  x_రికార్డు_Compare = sqlite3_Vdbe_కనుగొను_Compare(p_Idx_కీలకం);
  p_Idx_కీలకం->err_సంకేతం = 0;
  ప్రతిజ్ఞా( p_Idx_కీలకం->default_rc==1
       || p_Idx_కీలకం->default_rc==0
       || p_Idx_కీలకం->default_rc==-1
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
  ఒకవేళ( pCur->eState==CURSOR_VALID
   && pCur->p_పుట->ఆకు
   && cursor_On_చివరి_పుట(pCur)
  ){
    పూర్ణాంకం c;
    ఒకవేళ( pCur->ix==pCur->p_పుట->nCell-1
     && (c = సూచీ_Cell_Compare(pCur->p_పుట,pCur->ix,p_Idx_కీలకం,x_రికార్డు_Compare))<=0
     && p_Idx_కీలకం->err_సంకేతం==SQLITE_OK
    ){
      *pRes = c;
      తిరిగి SQLITE_OK;  /* Cursor already pointing at the correct spot */
    }
    ఒకవేళ( pCur->i_పుట>0
     && సూచీ_Cell_Compare(pCur->p_పుట, 0, p_Idx_కీలకం, x_రికార్డు_Compare)<=0
     && p_Idx_కీలకం->err_సంకేతం==SQLITE_OK
    ){
      pCur->curFlags &= ~(BTCF_ValidOvfl|BTCF_At_చివరి);
      ఒకవేళ( !pCur->p_పుట->isInit ){
        తిరిగి SQLITE_CORRUPT_BKPT;
      }
      వెళ్ళు bypass_moveto_మూలం;  /* Start search on the current page */
    }
    p_Idx_కీలకం->err_సంకేతం = SQLITE_OK;
  }

  rc = move_గమ్యం_మూలం(pCur);
  ఒకవేళ( rc ){
    ఒకవేళ( rc==SQLITE_ఖాళీలు ){
      ప్రతిజ్ఞా( pCur->pgno_మూలం==0 || pCur->p_పుట->nCell==0 );
      *pRes = -1;
      తిరిగి SQLITE_OK;
    }
    తిరిగి rc;
  }

bypass_moveto_మూలం:
  ప్రతిజ్ఞా( pCur->p_పుట );
  ప్రతిజ్ఞా( pCur->p_పుట->isInit );
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );
  ప్రతిజ్ఞా( pCur->p_పుట->nCell > 0 );
  ప్రతిజ్ఞా( pCur->cur_Int_కీలకం==0 );
  ప్రతిజ్ఞా( p_Idx_కీలకం!=0 );
  కొరకు(;;){
    పూర్ణాంకం lwr, upr, idx, c;
    Pgno chldPg;
    Mem_పుట *p_పుట = pCur->p_పుట;
    u8 *pCell;                          /* Pointer to current cell in pPage */

    /* pPage->nCell must be greater than zero. If this is the root-page
    ** the cursor would have been INVALID above and this for(;;) loop
    ** not run. If this is not the root-page, then the moveToChild() routine
    ** would have already detected db corruption. Similarly, pPage must
    ** be the right kind (index or table) of b-tree page. Otherwise
    ** a moveToChild() or moveToRoot() call would have detected corruption.  */
    ప్రతిజ్ఞా( p_పుట->nCell>0 );
    ప్రతిజ్ఞా( p_పుట->int_కీలకం==0 );
    lwr = 0;
    upr = p_పుట->nCell-1;
    idx = upr>>1; /* idx = (lwr+upr)/2; */
    కొరకు(;;){
      పూర్ణాంకం nCell;  /* Size of the pCell cell in bytes */
      pCell = కనుగొను_Cell_Past_Ptr(p_పుట, idx);

      /* The maximum supported page-size is 65536 bytes. This means that
      ** the maximum number of record bytes stored on an index B-Tree
      ** page is less than 16384 bytes and may be stored as a 2-byte
      ** varint. This information is used to attempt to avoid parsing
      ** the entire cell by checking for the cases where the record is
      ** stored entirely within the b-tree page by inspecting the first
      ** 2 bytes of the cell.
      */
      nCell = pCell[0];
      ఒకవేళ( nCell<=p_పుట->max1bytePayload ){
        /* This branch runs if the record-size field of the cell is a
        ** single byte varint and the record fits entirely on the main
        ** b-tree page.  */
        ఒకవేళ( pCell + nCell >= p_పుట->a_దత్తాంశం_చివర ){
          rc = SQLITE_CORRUPT_పుట(p_పుట);
          వెళ్ళు moveto_సూచీ_ముగింపు;
        }
        c = x_రికార్డు_Compare(nCell, (శూన్య*)&pCell[1], p_Idx_కీలకం);
      }లేకపోతే ఒకవేళ( !(pCell[1] & 0x80)
        && (nCell = ((nCell&0x7f)<<7) + pCell[1])<=p_పుట->maxLocal
        && pCell + nCell < p_పుట->a_దత్తాంశం_చివర
      ){
        /* The record-size field is a 2 byte varint and the record
        ** fits entirely on the main b-tree page.  */
        c = x_రికార్డు_Compare(nCell, (శూన్య*)&pCell[2], p_Idx_కీలకం);
      }లేకపోతే{
        /* The record flows over onto one or more overflow pages. In
        ** this case the whole cell needs to be parsed, a buffer allocated
        ** and accessPayload() used to retrieve the record into the
        ** buffer before VdbeRecordCompare() can be called.
        **
        ** If the record is corrupt, the xRecordCompare routine may read
        ** up to two varints past the end of the buffer. An extra 18
        ** bytes of padding is allocated at the end of the buffer in
        ** case this happens.  */
        శూన్య *p_Cell_కీలకం;
        u8 * స్థిరరాశి pCellBody = pCell - p_పుట->బిడ్డ_Ptr_కొలత;
        స్థిరరాశి పూర్ణాంకం nOverrun = 18;  /* Size of the overrun padding */
        p_పుట->xParseCell(p_పుట, pCellBody, &pCur->info);
        nCell = (పూర్ణాంకం)pCur->info.n_కీలకం;
        testcase( nCell<0 );   /* True if key size is 2^32 or more */
        testcase( nCell==0 );  /* Invalid key size:  0x80 0x80 0x00 */
        testcase( nCell==1 );  /* Invalid key size:  0x80 0x80 0x01 */
        testcase( nCell==2 );  /* Minimum legal index key size */
        ఒకవేళ( nCell<2 || nCell/pCur->pBt->usable_కొలత>pCur->pBt->n_పుట ){
          rc = SQLITE_CORRUPT_పుట(p_పుట);
          వెళ్ళు moveto_సూచీ_ముగింపు;
        }
        p_Cell_కీలకం = sqlite3Malloc( (u64)nCell+(u64)nOverrun );
        ఒకవేళ( p_Cell_కీలకం==0 ){
          rc = SQLITE_NOMEM_BKPT;
          వెళ్ళు moveto_సూచీ_ముగింపు;
        }
        pCur->ix = (u16)idx;
        rc = accessPayload(pCur, 0, nCell, (గుర్తులేని అక్షరం*)p_Cell_కీలకం, 0);
        memset(((u8*)p_Cell_కీలకం)+nCell,0,nOverrun); /* Fix uninit warnings */
        pCur->curFlags &= ~BTCF_ValidOvfl;
        ఒకవేళ( rc ){
          sqlite3_free(p_Cell_కీలకం);
          వెళ్ళు moveto_సూచీ_ముగింపు;
        }
        c = sqlite3_Vdbe_రికార్డు_Compare(nCell, p_Cell_కీలకం, p_Idx_కీలకం);
        sqlite3_free(p_Cell_కీలకం);
      }
      ప్రతిజ్ఞా(
          (p_Idx_కీలకం->err_సంకేతం!=SQLITE_CORRUPT || c==0)
       && (p_Idx_కీలకం->err_సంకేతం!=SQLITE_NOMEM || pCur->pBtree->db->mallocFailed)
      );
      ఒకవేళ( c<0 ){
        lwr = idx+1;
      }లేకపోతే ఒకవేళ( c>0 ){
        upr = idx-1;
      }లేకపోతే{
        ప్రతిజ్ఞా( c==0 );
        *pRes = 0;
        rc = SQLITE_OK;
        pCur->ix = (u16)idx;
        ఒకవేళ( p_Idx_కీలకం->err_సంకేతం ) rc = SQLITE_CORRUPT_BKPT;
        వెళ్ళు moveto_సూచీ_ముగింపు;
      }
      ఒకవేళ( lwr>upr ) ఆపు;
      ప్రతిజ్ఞా( lwr+upr>=0 );
      idx = (lwr+upr)>>1;  /* idx = (lwr+upr)/2 */
    }
    ప్రతిజ్ఞా( lwr==upr+1 || (p_పుట->int_కీలకం && !p_పుట->ఆకు) );
    ప్రతిజ్ఞా( p_పుట->isInit );
    ఒకవేళ( p_పుట->ఆకు ){
      ప్రతిజ్ఞా( pCur->ix<pCur->p_పుట->nCell || CORRUPT_DB );
      pCur->ix = (u16)idx;
      *pRes = c;
      rc = SQLITE_OK;
      వెళ్ళు moveto_సూచీ_ముగింపు;
    }
    ఒకవేళ( lwr>=p_పుట->nCell ){
      chldPg = get4byte(&p_పుట->a_దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+8]);
    }లేకపోతే{
      chldPg = get4byte(కనుగొను_Cell(p_పుట, lwr));
    }

    /* This block is similar to an in-lined version of:
    **
    **    pCur->ix = (u16)lwr;
    **    rc = moveToChild(pCur, chldPg);
    **    if( rc ) break;
    */
    pCur->info.n_కొలత = 0;
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
    ఒకవేళ( pCur->i_పుట>=(BTCURSOR_MAX_లోతు-1) ){
      తిరిగి SQLITE_CORRUPT_BKPT;
    }
    pCur->aiIdx[pCur->i_పుట] = (u16)lwr;
    pCur->ap_పుట[pCur->i_పుట] = pCur->p_పుట;
    pCur->ix = 0;
    pCur->i_పుట++;
    rc = get_And_Init_పుట(pCur->pBt, chldPg, &pCur->p_పుట, pCur->curPagerFlags);
    ఒకవేళ( rc==SQLITE_OK
     && (pCur->p_పుట->nCell<1 || pCur->p_పుట->int_కీలకం!=pCur->cur_Int_కీలకం)
    ){
      release_పుట(pCur->p_పుట);
      rc = SQLITE_CORRUPT_PGNO(chldPg);
    }
    ఒకవేళ( rc ){
      pCur->p_పుట = pCur->ap_పుట[--pCur->i_పుట];
      ఆపు;
    }
    /*
    ***** End of in-lined moveToChild() call */
 }
moveto_సూచీ_ముగింపు:
  pCur->info.n_కొలత = 0;
  ప్రతిజ్ఞా( (pCur->curFlags & BTCF_ValidOvfl)==0 );
  తిరిగి rc;
}


/*
** Return TRUE if the cursor is not pointing at an entry of the table.
**
** TRUE will be returned after a call to sqlite3BtreeNext() moves
** past the last entry in the table or sqlite3BtreePrev() moves past
** the first entry.  TRUE is also returned if the table is empty.
*/
పూర్ణాంకం sqlite3BtreeEof(BtCursor *pCur){
  /* TODO: What if the cursor is in CURSOR_REQUIRESEEK but all table entries
  ** have been deleted? This API will need to change to return an error code
  ** as well as the boolean result value.
  */
  తిరిగి (CURSOR_VALID!=pCur->eState);
}

/*
** Return an estimate for the number of rows in the table that pCur is
** pointing to.  Return a negative number if no estimate is currently
** available.
*/
i64 sqlite3_Btree_అడ్డువరుస_లెక్క_Est(BtCursor *pCur){
  i64 n;
  u8 i;

  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCur->pBtree->db->mutex) );

  /* Currently this interface is only called by the OP_IfSizeBetween
  ** opcode and the OP_Count opcode with P3=1.  In either case,
  ** the cursor will always be valid unless the btree is empty. */
  ఒకవేళ( pCur->eState!=CURSOR_VALID ) తిరిగి 0;
  ఒకవేళ( NEVER(pCur->p_పుట->ఆకు==0) ) తిరిగి -1;

  n = pCur->p_పుట->nCell;
  కొరకు(i=0; i<pCur->i_పుట; i++){
    n *= pCur->ap_పుట[i]->nCell+1;
  }
  తిరిగి n;
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం btree_తదుపరి(BtCursor *pCur){
  పూర్ణాంకం rc;
  పూర్ణాంకం idx;
  Mem_పుట *p_పుట;

  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ఒకవేళ( pCur->eState!=CURSOR_VALID ){
    ప్రతిజ్ఞా( (pCur->curFlags & BTCF_ValidOvfl)==0 );
    rc = restoreCursorPosition(pCur);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
    ఒకవేళ( CURSOR_INVALID==pCur->eState ){
      తిరిగి SQLITE_పూర్తి;
    }
    ఒకవేళ( pCur->eState==CURSOR_SKIPNEXT ){
      pCur->eState = CURSOR_VALID;
      ఒకవేళ( pCur->skip_తదుపరి>0 ) తిరిగి SQLITE_OK;
    }
  }

  p_పుట = pCur->p_పుట;
  idx = ++pCur->ix;
  ఒకవేళ( sqlite3FaultSim(412) ) p_పుట->isInit = 0;
  ఒకవేళ( !p_పుట->isInit ){
    తిరిగి SQLITE_CORRUPT_BKPT;
  }

  ఒకవేళ( idx>=p_పుట->nCell ){
    ఒకవేళ( !p_పుట->ఆకు ){
      rc = move_గమ్యం_బిడ్డ(pCur, get4byte(&p_పుట->a_దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+8]));
      ఒకవేళ( rc ) తిరిగి rc;
      తిరిగి move_గమ్యం_Leftmost(pCur);
    }
    చేయి{
      ఒకవేళ( pCur->i_పుట==0 ){
        pCur->eState = CURSOR_INVALID;
        తిరిగి SQLITE_పూర్తి;
      }
      move_గమ్యం_మాతృ(pCur);
      p_పుట = pCur->p_పుట;
    }అయితే( pCur->ix>=p_పుట->nCell );
    ఒకవేళ( p_పుట->int_కీలకం ){
      తిరిగి sqlite3_Btree_తదుపరి(pCur, 0);
    }లేకపోతే{
      తిరిగి SQLITE_OK;
    }
  }
  ఒకవేళ( p_పుట->ఆకు ){
    తిరిగి SQLITE_OK;
  }లేకపోతే{
    తిరిగి move_గమ్యం_Leftmost(pCur);
  }
}
పూర్ణాంకం sqlite3_Btree_తదుపరి(BtCursor *pCur, పూర్ణాంకం flags){
  Mem_పుట *p_పుట;
  వాడని_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( flags==0 || flags==1 );
  pCur->info.n_కొలత = 0;
  pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  ఒకవేళ( pCur->eState!=CURSOR_VALID ) తిరిగి btree_తదుపరి(pCur);
  p_పుట = pCur->p_పుట;
  ఒకవేళ( (++pCur->ix)>=p_పుట->nCell ){
    pCur->ix--;
    తిరిగి btree_తదుపరి(pCur);
  }
  ఒకవేళ( p_పుట->ఆకు ){
    తిరిగి SQLITE_OK;
  }లేకపోతే{
    తిరిగి move_గమ్యం_Leftmost(pCur);
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం btree_మునుపటి(BtCursor *pCur){
  పూర్ణాంకం rc;
  Mem_పుట *p_పుట;

  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( (pCur->curFlags & (BTCF_At_చివరి|BTCF_ValidOvfl|BTCF_ValidNKey))==0 );
  ప్రతిజ్ఞా( pCur->info.n_కొలత==0 );
  ఒకవేళ( pCur->eState!=CURSOR_VALID ){
    rc = restoreCursorPosition(pCur);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
    ఒకవేళ( CURSOR_INVALID==pCur->eState ){
      తిరిగి SQLITE_పూర్తి;
    }
    ఒకవేళ( CURSOR_SKIPNEXT==pCur->eState ){
      pCur->eState = CURSOR_VALID;
      ఒకవేళ( pCur->skip_తదుపరి<0 ) తిరిగి SQLITE_OK;
    }
  }

  p_పుట = pCur->p_పుట;
  ఒకవేళ( sqlite3FaultSim(412) ) p_పుట->isInit = 0;
  ఒకవేళ( !p_పుట->isInit ){
    తిరిగి SQLITE_CORRUPT_BKPT;
  }
  ఒకవేళ( !p_పుట->ఆకు ){
    పూర్ణాంకం idx = pCur->ix;
    rc = move_గమ్యం_బిడ్డ(pCur, get4byte(కనుగొను_Cell(p_పుట, idx)));
    ఒకవేళ( rc ) తిరిగి rc;
    rc = move_గమ్యం_Rightmost(pCur);
  }లేకపోతే{
    అయితే( pCur->ix==0 ){
      ఒకవేళ( pCur->i_పుట==0 ){
        pCur->eState = CURSOR_INVALID;
        తిరిగి SQLITE_పూర్తి;
      }
      move_గమ్యం_మాతృ(pCur);
    }
    ప్రతిజ్ఞా( pCur->info.n_కొలత==0 );
    ప్రతిజ్ఞా( (pCur->curFlags & (BTCF_ValidOvfl))==0 );

    pCur->ix--;
    p_పుట = pCur->p_పుట;
    ఒకవేళ( p_పుట->int_కీలకం && !p_పుట->ఆకు ){
      rc = sqlite3_Btree_మునుపటి(pCur, 0);
    }లేకపోతే{
      rc = SQLITE_OK;
    }
  }
  తిరిగి rc;
}
పూర్ణాంకం sqlite3_Btree_మునుపటి(BtCursor *pCur, పూర్ణాంకం flags){
  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( flags==0 || flags==1 );
  వాడని_PARAMETER( flags );  /* Used in COMDB2 but not native SQLite */
  pCur->curFlags &= ~(BTCF_At_చివరి|BTCF_ValidOvfl|BTCF_ValidNKey);
  pCur->info.n_కొలత = 0;
  ఒకవేళ( pCur->eState!=CURSOR_VALID
   || pCur->ix==0
   || pCur->p_పుట->ఆకు==0
  ){
    తిరిగి btree_మునుపటి(pCur);
  }
  pCur->ix--;
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం allocate_Btree_పుట(
  BtShared *pBt,         /* The btree */
  Mem_పుట **pp_పుట,      /* Store pointer to the allocated page here */
  Pgno *pPgno,           /* Store the page number here */
  Pgno nearby,           /* Search for a page near this one */
  u8 e_బహుళకం               /* BTALLOC_EXACT, BTALLOC_LT, or BTALLOC_ANY */
){
  Mem_పుట *pPage1;
  పూర్ణాంకం rc;
  u32 n;     /* Number of pages on the freelist */
  u32 k;     /* Number of leaves on the trunk of the freelist */
  Mem_పుట *pTrunk = 0;
  Mem_పుట *pPrevTrunk = 0;
  Pgno mx_పుట;     /* Total size of the database file */

  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ప్రతిజ్ఞా( e_బహుళకం==BTALLOC_ANY || (nearby>0 && IfNotOmitAV(pBt->autoVacuum)) );
  pPage1 = pBt->pPage1;
  mx_పుట = btreePagecount(pBt);
  /* EVIDENCE-OF: R-21003-45125 The 4-byte big-endian integer at offset 36
  ** stores the total number of pages on the freelist. */
  n = get4byte(&pPage1->a_దత్తాంశం[36]);
  testcase( n==mx_పుట-1 );
  ఒకవేళ( n>=mx_పుట ){
    తిరిగి SQLITE_CORRUPT_BKPT;
  }
  ఒకవేళ( n>0 ){
    /* There are pages on the freelist.  Reuse one of those pages. */
    Pgno iTrunk;
    u8 శోధన_List = 0; /* If the free-list must be searched for 'nearby' */
    u32 n_శోధన = 0;   /* Count of the number of search attempts */
   
    /* If eMode==BTALLOC_EXACT and a query of the pointer-map
    ** shows that the page 'nearby' is somewhere on the free-list, then
    ** the entire-list will be searched for that page.
    */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
    ఒకవేళ( e_బహుళకం==BTALLOC_EXACT ){
      ఒకవేళ( nearby<=mx_పుట ){
        u8 eType;
        ప్రతిజ్ఞా( nearby>0 );
        ప్రతిజ్ఞా( pBt->autoVacuum );
        rc = ptrmapGet(pBt, nearby, &eType, 0);
        ఒకవేళ( rc ) తిరిగి rc;
        ఒకవేళ( eType==PTRMAP_FREEPAGE ){
          శోధన_List = 1;
        }
      }
    }లేకపోతే ఒకవేళ( e_బహుళకం==BTALLOC_LE ){
      శోధన_List = 1;
    }
#అయితే_ముగింపు

    /* Decrement the free-list count by 1. Set iTrunk to the index of the
    ** first free-list trunk page. iPrevTrunk is initially 1.
    */
    rc = sqlite3PagerWrite(pPage1->p_Db_పుట);
    ఒకవేళ( rc ) తిరిగి rc;
    put4byte(&pPage1->a_దత్తాంశం[36], n-1);

    /* The code within this loop is run only once if the 'searchList' variable
    ** is not true. Otherwise, it runs once for each trunk-page on the
    ** free-list until the page 'nearby' is located (eMode==BTALLOC_EXACT)
    ** or until a page less than 'nearby' is located (eMode==BTALLOC_LT)
    */
    చేయి {
      pPrevTrunk = pTrunk;
      ఒకవేళ( pPrevTrunk ){
        /* EVIDENCE-OF: R-01506-11053 The first integer on a freelist trunk page
        ** is the page number of the next freelist trunk page in the list or
        ** zero if this is the last freelist trunk page. */
        iTrunk = get4byte(&pPrevTrunk->a_దత్తాంశం[0]);
      }లేకపోతే{
        /* EVIDENCE-OF: R-59841-13798 The 4-byte big-endian integer at offset 32
        ** stores the page number of the first page of the freelist, or zero if
        ** the freelist is empty. */
        iTrunk = get4byte(&pPage1->a_దత్తాంశం[32]);
      }
      testcase( iTrunk==mx_పుట );
      ఒకవేళ( iTrunk>mx_పుట || n_శోధన++ > n ){
        rc = SQLITE_CORRUPT_PGNO(pPrevTrunk ? pPrevTrunk->pgno : 1);
      }లేకపోతే{
        rc = btree_Get_వాడని_పుట(pBt, iTrunk, &pTrunk, 0);
      }
      ఒకవేళ( rc ){
        pTrunk = 0;
        వెళ్ళు చివర_allocate_పుట;
      }
      ప్రతిజ్ఞా( pTrunk!=0 );
      ప్రతిజ్ఞా( pTrunk->a_దత్తాంశం!=0 );
      /* EVIDENCE-OF: R-13523-04394 The second integer on a freelist trunk page
      ** is the number of leaf page pointers to follow. */
      k = get4byte(&pTrunk->a_దత్తాంశం[4]);
      ఒకవేళ( k==0 && !శోధన_List ){
        /* The trunk has no leaves and the list is not being searched.
        ** So extract the trunk page itself and use it as the newly
        ** allocated page */
        ప్రతిజ్ఞా( pPrevTrunk==0 );
        rc = sqlite3PagerWrite(pTrunk->p_Db_పుట);
        ఒకవేళ( rc ){
          వెళ్ళు చివర_allocate_పుట;
        }
        *pPgno = iTrunk;
        జ్ఞాపకం_నకలు(&pPage1->a_దత్తాంశం[32], &pTrunk->a_దత్తాంశం[0], 4);
        *pp_పుట = pTrunk;
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
      }లేకపోతే ఒకవేళ( k>(u32)(pBt->usable_కొలత/4 - 2) ){
        /* Value of k is out of range.  Database corruption */
        rc = SQLITE_CORRUPT_PGNO(iTrunk);
        వెళ్ళు చివర_allocate_పుట;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
      }లేకపోతే ఒకవేళ( శోధన_List
            && (nearby==iTrunk || (iTrunk<nearby && e_బహుళకం==BTALLOC_LE))
      ){
        /* The list is being searched and this trunk page is the page
        ** to allocate, regardless of whether it has leaves.
        */
        *pPgno = iTrunk;
        *pp_పుట = pTrunk;
        శోధన_List = 0;
        rc = sqlite3PagerWrite(pTrunk->p_Db_పుట);
        ఒకవేళ( rc ){
          వెళ్ళు చివర_allocate_పుట;
        }
        ఒకవేళ( k==0 ){
          ఒకవేళ( !pPrevTrunk ){
            జ్ఞాపకం_నకలు(&pPage1->a_దత్తాంశం[32], &pTrunk->a_దత్తాంశం[0], 4);
          }లేకపోతే{
            rc = sqlite3PagerWrite(pPrevTrunk->p_Db_పుట);
            ఒకవేళ( rc!=SQLITE_OK ){
              వెళ్ళు చివర_allocate_పుట;
            }
            జ్ఞాపకం_నకలు(&pPrevTrunk->a_దత్తాంశం[0], &pTrunk->a_దత్తాంశం[0], 4);
          }
        }లేకపోతే{
          /* The trunk page is required by the caller but it contains
          ** pointers to free-list leaves. The first leaf becomes a trunk
          ** page in this case.
          */
          Mem_పుట *pNewTrunk;
          Pgno iNewTrunk = get4byte(&pTrunk->a_దత్తాంశం[8]);
          ఒకవేళ( iNewTrunk>mx_పుట ){
            rc = SQLITE_CORRUPT_PGNO(iTrunk);
            వెళ్ళు చివర_allocate_పుట;
          }
          testcase( iNewTrunk==mx_పుట );
          rc = btree_Get_వాడని_పుట(pBt, iNewTrunk, &pNewTrunk, 0);
          ఒకవేళ( rc!=SQLITE_OK ){
            వెళ్ళు చివర_allocate_పుట;
          }
          rc = sqlite3PagerWrite(pNewTrunk->p_Db_పుట);
          ఒకవేళ( rc!=SQLITE_OK ){
            release_పుట(pNewTrunk);
            వెళ్ళు చివర_allocate_పుట;
          }
          జ్ఞాపకం_నకలు(&pNewTrunk->a_దత్తాంశం[0], &pTrunk->a_దత్తాంశం[0], 4);
          put4byte(&pNewTrunk->a_దత్తాంశం[4], k-1);
          జ్ఞాపకం_నకలు(&pNewTrunk->a_దత్తాంశం[8], &pTrunk->a_దత్తాంశం[12], (k-1)*4);
          release_పుట(pNewTrunk);
          ఒకవేళ( !pPrevTrunk ){
            ప్రతిజ్ఞా( sqlite3PagerIswriteable(pPage1->p_Db_పుట) );
            put4byte(&pPage1->a_దత్తాంశం[32], iNewTrunk);
          }లేకపోతే{
            rc = sqlite3PagerWrite(pPrevTrunk->p_Db_పుట);
            ఒకవేళ( rc ){
              వెళ్ళు చివర_allocate_పుట;
            }
            put4byte(&pPrevTrunk->a_దత్తాంశం[0], iNewTrunk);
          }
        }
        pTrunk = 0;
        TRACE(("ALLOCATE: %u trunk - %u free pages left\n", *pPgno, n-1));
#అయితే_ముగింపు
      }లేకపోతే ఒకవేళ( k>0 ){
        /* Extract a leaf from the trunk */
        u32 closest;
        Pgno i_పుట;
        గుర్తులేని అక్షరం *a_దత్తాంశం = pTrunk->a_దత్తాంశం;
        ఒకవేళ( nearby>0 ){
          u32 i;
          closest = 0;
          ఒకవేళ( e_బహుళకం==BTALLOC_LE ){
            కొరకు(i=0; i<k; i++){
              i_పుట = get4byte(&a_దత్తాంశం[8+i*4]);
              ఒకవేళ( i_పుట<=nearby ){
                closest = i;
                ఆపు;
              }
            }
          }లేకపోతే{
            పూర్ణాంకం dist;
            dist = sqlite3AbsInt32(get4byte(&a_దత్తాంశం[8]) - nearby);
            కొరకు(i=1; i<k; i++){
              పూర్ణాంకం d2 = sqlite3AbsInt32(get4byte(&a_దత్తాంశం[8+i*4]) - nearby);
              ఒకవేళ( d2<dist ){
                closest = i;
                dist = d2;
              }
            }
          }
        }లేకపోతే{
          closest = 0;
        }

        i_పుట = get4byte(&a_దత్తాంశం[8+closest*4]);
        testcase( i_పుట==mx_పుట );
        ఒకవేళ( i_పుట>mx_పుట || i_పుట<2 ){
          rc = SQLITE_CORRUPT_PGNO(iTrunk);
          వెళ్ళు చివర_allocate_పుట;
        }
        testcase( i_పుట==mx_పుట );
        ఒకవేళ( !శోధన_List
         || (i_పుట==nearby || (i_పుట<nearby && e_బహుళకం==BTALLOC_LE))
        ){
          పూర్ణాంకం noContent;
          *pPgno = i_పుట;
          TRACE(("ALLOCATE: %u was leaf %u of %u on trunk %u"
                 ": %u more free pages\n",
                 *pPgno, closest+1, k, pTrunk->pgno, n-1));
          rc = sqlite3PagerWrite(pTrunk->p_Db_పుట);
          ఒకవేళ( rc ) వెళ్ళు చివర_allocate_పుట;
          ఒకవేళ( closest<k-1 ){
            జ్ఞాపకం_నకలు(&a_దత్తాంశం[8+closest*4], &a_దత్తాంశం[4+k*4], 4);
          }
          put4byte(&a_దత్తాంశం[4], k-1);
          noContent = !btreeGetHasContent(pBt, *pPgno)? PAGER_GET_NOCONTENT : 0;
          rc = btree_Get_వాడని_పుట(pBt, *pPgno, pp_పుట, noContent);
          ఒకవేళ( rc==SQLITE_OK ){
            rc = sqlite3PagerWrite((*pp_పుట)->p_Db_పుట);
            ఒకవేళ( rc!=SQLITE_OK ){
              release_పుట(*pp_పుట);
              *pp_పుట = 0;
            }
          }
          శోధన_List = 0;
        }
      }
      release_పుట(pPrevTrunk);
      pPrevTrunk = 0;
    }అయితే( శోధన_List );
  }లేకపోతే{
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
    పూర్ణాంకం bNoContent = (0==IfNotOmitAV(pBt->bDoTruncate))? PAGER_GET_NOCONTENT:0;

    rc = sqlite3PagerWrite(pBt->pPage1->p_Db_పుట);
    ఒకవేళ( rc ) తిరిగి rc;
    pBt->n_పుట++;
    ఒకవేళ( pBt->n_పుట==PENDING_BYTE_పుట(pBt) ) pBt->n_పుట++;

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
    ఒకవేళ( pBt->autoVacuum && PTRMAP_ISPAGE(pBt, pBt->n_పుట) ){
      /* If *pPgno refers to a pointer-map page, allocate two new pages
      ** at the end of the file instead of one. The first allocated page
      ** becomes a new pointer-map page, the second is used by the caller.
      */
      Mem_పుట *pPg = 0;
      TRACE(("ALLOCATE: %u from end of file (pointer-map page)\n", pBt->n_పుట));
      ప్రతిజ్ఞా( pBt->n_పుట!=PENDING_BYTE_పుట(pBt) );
      rc = btree_Get_వాడని_పుట(pBt, pBt->n_పుట, &pPg, bNoContent);
      ఒకవేళ( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pPg->p_Db_పుట);
        release_పుట(pPg);
      }
      ఒకవేళ( rc ) తిరిగి rc;
      pBt->n_పుట++;
      ఒకవేళ( pBt->n_పుట==PENDING_BYTE_పుట(pBt) ){ pBt->n_పుట++; }
    }
#అయితే_ముగింపు
    put4byte(28 + (u8*)pBt->pPage1->a_దత్తాంశం, pBt->n_పుట);
    *pPgno = pBt->n_పుట;

    ప్రతిజ్ఞా( *pPgno!=PENDING_BYTE_పుట(pBt) );
    rc = btree_Get_వాడని_పుట(pBt, *pPgno, pp_పుట, bNoContent);
    ఒకవేళ( rc ) తిరిగి rc;
    rc = sqlite3PagerWrite((*pp_పుట)->p_Db_పుట);
    ఒకవేళ( rc!=SQLITE_OK ){
      release_పుట(*pp_పుట);
      *pp_పుట = 0;
    }
    TRACE(("ALLOCATE: %u from end of file\n", *pPgno));
  }

  ప్రతిజ్ఞా( CORRUPT_DB || *pPgno!=PENDING_BYTE_పుట(pBt) );

చివర_allocate_పుట:
  release_పుట(pTrunk);
  release_పుట(pPrevTrunk);
  ప్రతిజ్ఞా( rc!=SQLITE_OK || sqlite3_Pager_పుట_Refcount((*pp_పుట)->p_Db_పుట)<=1 );
  ప్రతిజ్ఞా( rc!=SQLITE_OK || (*pp_పుట)->isInit==0 );
  తిరిగి rc;
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
స్థిర పూర్ణాంకం freePage2(BtShared *pBt, Mem_పుట *p_Mem_పుట, Pgno i_పుట){
  Mem_పుట *pTrunk = 0;                /* Free-list trunk page */
  Pgno iTrunk = 0;                    /* Page number of free-list trunk page */
  Mem_పుట *pPage1 = pBt->pPage1;      /* Local reference to page 1 */
  Mem_పుట *p_పుట;                     /* Page being freed. May be NULL. */
  పూర్ణాంకం rc;                             /* Return Code */
  u32 nFree;                          /* Initial number of pages on free-list */

  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ప్రతిజ్ఞా( CORRUPT_DB || i_పుట>1 );
  ప్రతిజ్ఞా( !p_Mem_పుట || p_Mem_పుట->pgno==i_పుట );

  ఒకవేళ( i_పుట<2 || i_పుట>pBt->n_పుట ){
    తిరిగి SQLITE_CORRUPT_BKPT;
  }
  ఒకవేళ( p_Mem_పుట ){
    p_పుట = p_Mem_పుట;
    sqlite3PagerRef(p_పుట->p_Db_పుట);
  }లేకపోతే{
    p_పుట = btree_పుట_Lookup(pBt, i_పుట);
  }

  /* Increment the free page count on pPage1 */
  rc = sqlite3PagerWrite(pPage1->p_Db_పుట);
  ఒకవేళ( rc ) వెళ్ళు freepage_బహిర్గతం;
  nFree = get4byte(&pPage1->a_దత్తాంశం[36]);
  put4byte(&pPage1->a_దత్తాంశం[36], nFree+1);

  ఒకవేళ( pBt->btsFlags & BTS_SECURE_DELETE ){
    /* If the secure_delete option is enabled, then
    ** always fully overwrite deleted information with zeros.
    */
    ఒకవేళ( (!p_పుట && ((rc = btree_Get_పుట(pBt, i_పుట, &p_పుట, 0))!=0) )
     ||            ((rc = sqlite3PagerWrite(p_పుట->p_Db_పుట))!=0)
    ){
      వెళ్ళు freepage_బహిర్గతం;
    }
    memset(p_పుట->a_దత్తాంశం, 0, p_పుట->pBt->పుట_కొలత);
  }

  /* If the database supports auto-vacuum, write an entry in the pointer-map
  ** to indicate that the page is free.
  */
  ఒకవేళ( ISAUTOVACUUM(pBt) ){
    ptrmapPut(pBt, i_పుట, PTRMAP_FREEPAGE, 0, &rc);
    ఒకవేళ( rc ) వెళ్ళు freepage_బహిర్గతం;
  }

  /* Now manipulate the actual database free-list structure. There are two
  ** possibilities. If the free-list is currently empty, or if the first
  ** trunk page in the free-list is full, then this page will become a
  ** new free-list trunk page. Otherwise, it will become a leaf of the
  ** first trunk page in the current free-list. This block tests if it
  ** is possible to add the page as a new free-list leaf.
  */
  ఒకవేళ( nFree!=0 ){
    u32 n_ఆకు;                /* Initial number of leaf cells on trunk page */

    iTrunk = get4byte(&pPage1->a_దత్తాంశం[32]);
    ఒకవేళ( iTrunk>btreePagecount(pBt) ){
      rc = SQLITE_CORRUPT_BKPT;
      వెళ్ళు freepage_బహిర్గతం;
    }
    rc = btree_Get_పుట(pBt, iTrunk, &pTrunk, 0);
    ఒకవేళ( rc!=SQLITE_OK ){
      వెళ్ళు freepage_బహిర్గతం;
    }

    n_ఆకు = get4byte(&pTrunk->a_దత్తాంశం[4]);
    ప్రతిజ్ఞా( pBt->usable_కొలత>32 );
    ఒకవేళ( n_ఆకు > (u32)pBt->usable_కొలత/4 - 2 ){
      rc = SQLITE_CORRUPT_BKPT;
      వెళ్ళు freepage_బహిర్గతం;
    }
    ఒకవేళ( n_ఆకు < (u32)pBt->usable_కొలత/4 - 8 ){
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
      rc = sqlite3PagerWrite(pTrunk->p_Db_పుట);
      ఒకవేళ( rc==SQLITE_OK ){
        put4byte(&pTrunk->a_దత్తాంశం[4], n_ఆకు+1);
        put4byte(&pTrunk->a_దత్తాంశం[8+n_ఆకు*4], i_పుట);
        ఒకవేళ( p_పుట && (pBt->btsFlags & BTS_SECURE_DELETE)==0 ){
          sqlite3PagerDontWrite(p_పుట->p_Db_పుట);
        }
        rc = btree_సమితి_Has_Content(pBt, i_పుట);
      }
      TRACE(("FREE-PAGE: %u leaf on trunk page %u\n",p_పుట->pgno,pTrunk->pgno));
      వెళ్ళు freepage_బహిర్గతం;
    }
  }

  /* If control flows to this point, then it was not possible to add the
  ** the page being freed as a leaf page of the first trunk in the free-list.
  ** Possibly because the free-list is empty, or possibly because the
  ** first trunk in the free-list is full. Either way, the page being freed
  ** will become the new first trunk page in the free-list.
  */
  ఒకవేళ( p_పుట==0 && SQLITE_OK!=(rc = btree_Get_పుట(pBt, i_పుట, &p_పుట, 0)) ){
    వెళ్ళు freepage_బహిర్గతం;
  }
  rc = sqlite3PagerWrite(p_పుట->p_Db_పుట);
  ఒకవేళ( rc!=SQLITE_OK ){
    వెళ్ళు freepage_బహిర్గతం;
  }
  put4byte(p_పుట->a_దత్తాంశం, iTrunk);
  put4byte(&p_పుట->a_దత్తాంశం[4], 0);
  put4byte(&pPage1->a_దత్తాంశం[32], i_పుట);
  TRACE(("FREE-PAGE: %u new trunk page replacing %u\n", p_పుట->pgno, iTrunk));

freepage_బహిర్గతం:
  ఒకవేళ( p_పుట ){
    p_పుట->isInit = 0;
  }
  release_పుట(p_పుట);
  release_పుట(pTrunk);
  తిరిగి rc;
}
స్థిర శూన్య free_పుట(Mem_పుట *p_పుట, పూర్ణాంకం *pRC){
  ఒకవేళ( (*pRC)==SQLITE_OK ){
    *pRC = freePage2(p_పుట->pBt, p_పుట, p_పుట->pgno);
  }
}

/*
** Free the overflow pages associated with the given Cell.
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం clear_Cell_అధికప్రవాహం(
  Mem_పుట *p_పుట,          /* The page that contains the Cell */
  గుర్తులేని అక్షరం *pCell,    /* First byte of the Cell */
  CellInfo *pInfo          /* Size information about the cell */
){
  BtShared *pBt;
  Pgno ovflPgno;
  పూర్ణాంకం rc;
  పూర్ణాంకం nOvfl;
  u32 ovfl_పుట_కొలత;

  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  ప్రతిజ్ఞా( pInfo->nLocal!=pInfo->nPayload );
  testcase( pCell + pInfo->n_కొలత == p_పుట->a_దత్తాంశం_చివర );
  testcase( pCell + (pInfo->n_కొలత-1) == p_పుట->a_దత్తాంశం_చివర );
  ఒకవేళ( pCell + pInfo->n_కొలత > p_పుట->a_దత్తాంశం_చివర ){
    /* Cell extends past end of page */
    తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
  }
  ovflPgno = get4byte(pCell + pInfo->n_కొలత - 4);
  pBt = p_పుట->pBt;
  ప్రతిజ్ఞా( pBt->usable_కొలత > 4 );
  ovfl_పుట_కొలత = pBt->usable_కొలత - 4;
  nOvfl = (pInfo->nPayload - pInfo->nLocal + ovfl_పుట_కొలత - 1)/ovfl_పుట_కొలత;
  ప్రతిజ్ఞా( nOvfl>0 ||
    (CORRUPT_DB && (pInfo->nPayload + ovfl_పుట_కొలత)<ovfl_పుట_కొలత)
  );
  అయితే( nOvfl-- ){
    Pgno i_తదుపరి = 0;
    Mem_పుట *pOvfl = 0;
    ఒకవేళ( ovflPgno<2 || ovflPgno>btreePagecount(pBt) ){
      /* 0 is not a legal page number and page 1 cannot be an
      ** overflow page. Therefore if ovflPgno<2 or past the end of the
      ** file the database must be corrupt. */
      తిరిగి SQLITE_CORRUPT_BKPT;
    }
    ఒకవేళ( nOvfl ){
      rc = get_అధికప్రవాహం_పుట(pBt, ovflPgno, &pOvfl, &i_తదుపరి);
      ఒకవేళ( rc ) తిరిగి rc;
    }

    ఒకవేళ( ( pOvfl || ((pOvfl = btree_పుట_Lookup(pBt, ovflPgno))!=0) )
     && sqlite3_Pager_పుట_Refcount(pOvfl->p_Db_పుట)!=1
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
    }లేకపోతే{
      rc = freePage2(pBt, pOvfl, ovflPgno);
    }

    ఒకవేళ( pOvfl ){
      sqlite3PagerUnref(pOvfl->p_Db_పుట);
    }
    ఒకవేళ( rc ) తిరిగి rc;
    ovflPgno = i_తదుపరి;
  }
  తిరిగి SQLITE_OK;
}

/* Call xParseCell to compute the size of a cell.  If the cell contains
** overflow, then invoke cellClearOverflow to clear out that overflow.
** Store the result code (SQLITE_OK or some error code) in rc.
**
** Implemented as macro to force inlining for performance.
*/
#నిర్వచించు BTREE_CLEAR_CELL(rc, p_పుట, pCell, sInfo)   \
  p_పుట->xParseCell(p_పుట, pCell, &sInfo);          \
  ఒకవేళ( sInfo.nLocal!=sInfo.nPayload ){               \
    rc = clear_Cell_అధికప్రవాహం(p_పుట, pCell, &sInfo);   \
  }లేకపోతే{                                            \
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
స్థిర పూర్ణాంకం fill_లోపల_Cell(
  Mem_పుట *p_పుట,                /* The page that contains the cell */
  గుర్తులేని అక్షరం *pCell,          /* Complete text of the cell */
  స్థిరరాశి BtreePayload *pX,        /* Payload with which to construct the cell */
  పూర్ణాంకం *pn_కొలత                    /* Write cell size here */
){
  పూర్ణాంకం nPayload;
  స్థిరరాశి u8 *pSrc;
  పూర్ణాంకం nSrc, n, rc, mn;
  పూర్ణాంకం space_ఎడమ;
  Mem_పుట *p_గమ్యం_Release;
  గుర్తులేని అక్షరం *p_పూర్వ;
  గుర్తులేని అక్షరం *pPayload;
  BtShared *pBt;
  Pgno pgnoOvfl;
  పూర్ణాంకం n_శీర్షిక;

  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );

  /* pPage is not necessarily writeable since pCell might be auxiliary
  ** buffer space that is separate from the pPage buffer area */
  ప్రతిజ్ఞా( pCell<p_పుట->a_దత్తాంశం || pCell>=&p_పుట->a_దత్తాంశం[p_పుట->pBt->పుట_కొలత]
            || sqlite3PagerIswriteable(p_పుట->p_Db_పుట) );

  /* Fill in the header. */
  n_శీర్షిక = p_పుట->బిడ్డ_Ptr_కొలత;
  ఒకవేళ( p_పుట->int_కీలకం ){
    nPayload = pX->n_దత్తాంశం + pX->nZero;
    pSrc = pX->p_దత్తాంశం;
    nSrc = pX->n_దత్తాంశం;
    ప్రతిజ్ఞా( p_పుట->int_కీలకం_ఆకు ); /* fillInCell() only called for leaves */
    n_శీర్షిక += putVarint32(&pCell[n_శీర్షిక], nPayload);
    n_శీర్షిక += sqlite3PutVarint(&pCell[n_శీర్షిక], *(u64*)&pX->n_కీలకం);
  }లేకపోతే{
    ప్రతిజ్ఞా( pX->n_కీలకం<=0x7fffffff && pX->p_కీలకం!=0 );
    nSrc = nPayload = (పూర్ణాంకం)pX->n_కీలకం;
    pSrc = pX->p_కీలకం;
    n_శీర్షిక += putVarint32(&pCell[n_శీర్షిక], nPayload);
  }
 
  /* Fill in the payload */
  pPayload = &pCell[n_శీర్షిక];
  ఒకవేళ( nPayload<=p_పుట->maxLocal ){
    /* This is the common case where everything fits on the btree page
    ** and no overflow pages are required. */
    n = n_శీర్షిక + nPayload;
    testcase( n==3 );
    testcase( n==4 );
    ఒకవేళ( n<4 ){
      n = 4;
      pPayload[nPayload] = 0;
    }
    *pn_కొలత = n;
    ప్రతిజ్ఞా( nSrc<=nPayload );
    testcase( nSrc<nPayload );
    జ్ఞాపకం_నకలు(pPayload, pSrc, nSrc);
    memset(pPayload+nSrc, 0, nPayload-nSrc);
    తిరిగి SQLITE_OK;
  }

  /* If we reach this point, it means that some of the content will need
  ** to spill onto overflow pages.
  */
  mn = p_పుట->minLocal;
  n = mn + (nPayload - mn) % (p_పుట->pBt->usable_కొలత - 4);
  testcase( n==p_పుట->maxLocal );
  testcase( n==p_పుట->maxLocal+1 );
  ఒకవేళ( n > p_పుట->maxLocal ) n = mn;
  space_ఎడమ = n;
  *pn_కొలత = n + n_శీర్షిక + 4;
  p_పూర్వ = &pCell[n_శీర్షిక+n];
  p_గమ్యం_Release = 0;
  pgnoOvfl = 0;
  pBt = p_పుట->pBt;

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
#నిర్వచితమైతే SQLITE_DEBUG
  {
    CellInfo info;
    p_పుట->xParseCell(p_పుట, pCell, &info);
    ప్రతిజ్ఞా( n_శీర్షిక==(పూర్ణాంకం)(info.pPayload - pCell) );
    ప్రతిజ్ఞా( info.n_కీలకం==pX->n_కీలకం );
    ప్రతిజ్ఞా( *pn_కొలత == info.n_కొలత );
    ప్రతిజ్ఞా( space_ఎడమ == info.nLocal );
  }
#అయితే_ముగింపు

  /* Write the payload into the local Cell and any extra into overflow pages */
  అయితే( 1 ){
    n = nPayload;
    ఒకవేళ( n>space_ఎడమ ) n = space_ఎడమ;

    /* If pToRelease is not zero than pPayload points into the data area
    ** of pToRelease.  Make sure pToRelease is still writeable. */
    ప్రతిజ్ఞా( p_గమ్యం_Release==0 || sqlite3PagerIswriteable(p_గమ్యం_Release->p_Db_పుట) );

    /* If pPayload is part of the data area of pPage, then make sure pPage
    ** is still writeable */
    ప్రతిజ్ఞా( pPayload<p_పుట->a_దత్తాంశం || pPayload>=&p_పుట->a_దత్తాంశం[pBt->పుట_కొలత]
            || sqlite3PagerIswriteable(p_పుట->p_Db_పుట) );

    ఒకవేళ( nSrc>=n ){
      జ్ఞాపకం_నకలు(pPayload, pSrc, n);
    }లేకపోతే ఒకవేళ( nSrc>0 ){
      n = nSrc;
      జ్ఞాపకం_నకలు(pPayload, pSrc, n);
    }లేకపోతే{
      memset(pPayload, 0, n);
    }
    nPayload -= n;
    ఒకవేళ( nPayload<=0 ) ఆపు;
    pPayload += n;
    pSrc += n;
    nSrc -= n;
    space_ఎడమ -= n;
    ఒకవేళ( space_ఎడమ==0 ){
      Mem_పుట *pOvfl = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
      Pgno pgnoPtrmap = pgnoOvfl; /* Overflow page pointer-map entry page */
      ఒకవేళ( pBt->autoVacuum ){
        చేయి{
          pgnoOvfl++;
        } అయితే(
          PTRMAP_ISPAGE(pBt, pgnoOvfl) || pgnoOvfl==PENDING_BYTE_పుట(pBt)
        );
      }
#అయితే_ముగింపు
      rc = allocate_Btree_పుట(pBt, &pOvfl, &pgnoOvfl, pgnoOvfl, 0);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
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
      ఒకవేళ( pBt->autoVacuum && rc==SQLITE_OK ){
        u8 eType = (pgnoPtrmap?PTRMAP_OVERFLOW2:PTRMAP_OVERFLOW1);
        ptrmapPut(pBt, pgnoOvfl, eType, pgnoPtrmap, &rc);
        ఒకవేళ( rc ){
          release_పుట(pOvfl);
        }
      }
#అయితే_ముగింపు
      ఒకవేళ( rc ){
        release_పుట(p_గమ్యం_Release);
        తిరిగి rc;
      }

      /* If pToRelease is not zero than pPrior points into the data area
      ** of pToRelease.  Make sure pToRelease is still writeable. */
      ప్రతిజ్ఞా( p_గమ్యం_Release==0 || sqlite3PagerIswriteable(p_గమ్యం_Release->p_Db_పుట) );

      /* If pPrior is part of the data area of pPage, then make sure pPage
      ** is still writeable */
      ప్రతిజ్ఞా( p_పూర్వ<p_పుట->a_దత్తాంశం || p_పూర్వ>=&p_పుట->a_దత్తాంశం[pBt->పుట_కొలత]
            || sqlite3PagerIswriteable(p_పుట->p_Db_పుట) );

      put4byte(p_పూర్వ, pgnoOvfl);
      release_పుట(p_గమ్యం_Release);
      p_గమ్యం_Release = pOvfl;
      p_పూర్వ = pOvfl->a_దత్తాంశం;
      put4byte(p_పూర్వ, 0);
      pPayload = &pOvfl->a_దత్తాంశం[4];
      space_ఎడమ = pBt->usable_కొలత - 4;
    }
  }
  release_పుట(p_గమ్యం_Release);
  తిరిగి SQLITE_OK;
}

/*
** Remove the i-th cell from pPage.  This routine effects pPage only.
** The cell content is not freed or deallocated.  It is assumed that
** the cell content has been copied someplace else.  This routine just
** removes the reference to the cell from pPage.
**
** "sz" must be the number of bytes in the cell.
*/
స్థిర శూన్య dropCell(Mem_పుట *p_పుట, పూర్ణాంకం idx, పూర్ణాంకం sz, పూర్ణాంకం *pRC){
  u32 pc;         /* Offset to cell content of cell being deleted */
  u8 *దత్తాంశం;       /* pPage->aData */
  u8 *ptr;        /* Used to move bytes around within data[] */
  పూర్ణాంకం rc;         /* The return code */
  పూర్ణాంకం hdr;        /* Beginning of the header.  0 most pages.  100 page 1 */

  ఒకవేళ( *pRC ) తిరిగి;
  ప్రతిజ్ఞా( idx>=0 );
  ప్రతిజ్ఞా( idx<p_పుట->nCell );
  ప్రతిజ్ఞా( CORRUPT_DB || sz==cell_కొలత(p_పుట, idx) );
  ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_పుట->p_Db_పుట) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  ప్రతిజ్ఞా( p_పుట->nFree>=0 );
  దత్తాంశం = p_పుట->a_దత్తాంశం;
  ptr = &p_పుట->aCellIdx[2*idx];
  ప్రతిజ్ఞా( p_పుట->pBt->usable_కొలత > (u32)(ptr-దత్తాంశం) );
  pc = get2byte(ptr);
  hdr = p_పుట->hdr_స్థానభ్రంశం;
  testcase( pc==(u32)get2byte(&దత్తాంశం[hdr+5]) );
  testcase( pc+sz==p_పుట->pBt->usable_కొలత );
  ఒకవేళ( pc+sz > p_పుట->pBt->usable_కొలత ){
    *pRC = SQLITE_CORRUPT_BKPT;
    తిరిగి;
  }
  rc = freeSpace(p_పుట, pc, sz);
  ఒకవేళ( rc ){
    *pRC = rc;
    తిరిగి;
  }
  p_పుట->nCell--;
  ఒకవేళ( p_పుట->nCell==0 ){
    memset(&దత్తాంశం[hdr+1], 0, 4);
    దత్తాంశం[hdr+7] = 0;
    put2byte(&దత్తాంశం[hdr+5], p_పుట->pBt->usable_కొలత);
    p_పుట->nFree = p_పుట->pBt->usable_కొలత - p_పుట->hdr_స్థానభ్రంశం
                       - p_పుట->బిడ్డ_Ptr_కొలత - 8;
  }లేకపోతే{
    memmove(ptr, ptr+2, 2*(p_పుట->nCell - idx));
    put2byte(&దత్తాంశం[hdr+3], p_పుట->nCell);
    p_పుట->nFree += 2;
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
స్థిర పూర్ణాంకం చొప్పించు_Cell(
  Mem_పుట *p_పుట,   /* Page into which we are copying */
  పూర్ణాంకం i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  పూర్ణాంకం sz,           /* Bytes of content in pCell */
  u8 *p_తాత్కాలిక,        /* Temp storage space for pCell, if needed */
  Pgno i_బిడ్డ       /* If non-zero, replace first 4 bytes with this value */
){
  పూర్ణాంకం idx = 0;      /* Where to write new cell content in data[] */
  పూర్ణాంకం j;            /* Loop counter */
  u8 *దత్తాంశం;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  ప్రతిజ్ఞా( i>=0 && i<=p_పుట->nCell+p_పుట->n_అధికప్రవాహం );
  ప్రతిజ్ఞా( MX_CELL(p_పుట->pBt)<=10921 );
  ప్రతిజ్ఞా( p_పుట->nCell<=MX_CELL(p_పుట->pBt) || CORRUPT_DB );
  ప్రతిజ్ఞా( p_పుట->n_అధికప్రవాహం<=Array_కొలత(p_పుట->apOvfl) );
  ప్రతిజ్ఞా( Array_కొలత(p_పుట->apOvfl)==Array_కొలత(p_పుట->aiOvfl) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  ప్రతిజ్ఞా( sz==p_పుట->x_Cell_కొలత(p_పుట, pCell) || CORRUPT_DB );
  ప్రతిజ్ఞా( p_పుట->nFree>=0 );
  ప్రతిజ్ఞా( i_బిడ్డ>0 );
  ఒకవేళ( p_పుట->n_అధికప్రవాహం || sz+2>p_పుట->nFree ){
    ఒకవేళ( p_తాత్కాలిక ){
      జ్ఞాపకం_నకలు(p_తాత్కాలిక, pCell, sz);
      pCell = p_తాత్కాలిక;
    }
    put4byte(pCell, i_బిడ్డ);
    j = p_పుట->n_అధికప్రవాహం++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    ప్రతిజ్ఞా( j < Array_కొలత(p_పుట->apOvfl)-1 );
    p_పుట->apOvfl[j] = pCell;
    p_పుట->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    ప్రతిజ్ఞా( j==0 || p_పుట->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    ప్రతిజ్ఞా( j==0 || i==p_పుట->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }లేకపోతే{
    పూర్ణాంకం rc = sqlite3PagerWrite(p_పుట->p_Db_పుట);
    ఒకవేళ( NEVER(rc!=SQLITE_OK) ){
      తిరిగి rc;
    }
    ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_పుట->p_Db_పుట) );
    దత్తాంశం = p_పుట->a_దత్తాంశం;
    ప్రతిజ్ఞా( &దత్తాంశం[p_పుట->cell_స్థానభ్రంశం]==p_పుట->aCellIdx );
    rc = allocateSpace(p_పుట, sz, &idx);
    ఒకవేళ( rc ){ తిరిగి rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    ప్రతిజ్ఞా( idx >= 0 );
    ప్రతిజ్ఞా( idx >= p_పుట->cell_స్థానభ్రంశం+2*p_పుట->nCell+2 || CORRUPT_DB );
    ప్రతిజ్ఞా( idx+sz <= (పూర్ణాంకం)p_పుట->pBt->usable_కొలత );
    p_పుట->nFree -= (u16)(2 + sz);
    /* In a corrupt database where an entry in the cell index section of
    ** a btree page has a value of 3 or less, the pCell value might point
    ** as many as 4 bytes in front of the start of the aData buffer for
    ** the source page.  Make sure this does not cause problems by not
    ** reading the first 4 bytes */
    జ్ఞాపకం_నకలు(&దత్తాంశం[idx+4], pCell+4, sz-4);
    put4byte(&దత్తాంశం[idx], i_బిడ్డ);
    pIns = p_పుట->aCellIdx + i*2;
    ఒకవేళ( i<p_పుట->nCell ) memmove(pIns+2, pIns, 2*(p_పుట->nCell - i));
    put2byte(pIns, idx);
    p_పుట->nCell++;
    /* increment the cell count */
    ఒకవేళ( (++దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+4])==0 ) దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+3]++;
    ప్రతిజ్ఞా( get2byte(&దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+3])==p_పుట->nCell || CORRUPT_DB );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
    ఒకవేళ( p_పుట->pBt->autoVacuum ){
      పూర్ణాంకం rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(p_పుట, p_పుట, pCell, &rc2);
      ఒకవేళ( rc2 ) తిరిగి rc2;
    }
#అయితే_ముగింపు
  }
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం చొప్పించు_Cell_Fast(
  Mem_పుట *p_పుట,   /* Page into which we are copying */
  పూర్ణాంకం i,            /* New cell becomes the i-th cell of the page */
  u8 *pCell,        /* Content of the new cell */
  పూర్ణాంకం sz            /* Bytes of content in pCell */
){
  పూర్ణాంకం idx = 0;      /* Where to write new cell content in data[] */
  పూర్ణాంకం j;            /* Loop counter */
  u8 *దత్తాంశం;         /* The content of the whole page */
  u8 *pIns;         /* The point in pPage->aCellIdx[] where no cell inserted */

  ప్రతిజ్ఞా( i>=0 && i<=p_పుట->nCell+p_పుట->n_అధికప్రవాహం );
  ప్రతిజ్ఞా( MX_CELL(p_పుట->pBt)<=10921 );
  ప్రతిజ్ఞా( p_పుట->nCell<=MX_CELL(p_పుట->pBt) || CORRUPT_DB );
  ప్రతిజ్ఞా( p_పుట->n_అధికప్రవాహం<=Array_కొలత(p_పుట->apOvfl) );
  ప్రతిజ్ఞా( Array_కొలత(p_పుట->apOvfl)==Array_కొలత(p_పుట->aiOvfl) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  ప్రతిజ్ఞా( sz==p_పుట->x_Cell_కొలత(p_పుట, pCell) || CORRUPT_DB );
  ప్రతిజ్ఞా( p_పుట->nFree>=0 );
  ప్రతిజ్ఞా( p_పుట->n_అధికప్రవాహం==0 );
  ఒకవేళ( sz+2>p_పుట->nFree ){
    j = p_పుట->n_అధికప్రవాహం++;
    /* Comparison against ArraySize-1 since we hold back one extra slot
    ** as a contingency.  In other words, never need more than 3 overflow
    ** slots but 4 are allocated, just to be safe. */
    ప్రతిజ్ఞా( j < Array_కొలత(p_పుట->apOvfl)-1 );
    p_పుట->apOvfl[j] = pCell;
    p_పుట->aiOvfl[j] = (u16)i;

    /* When multiple overflows occur, they are always sequential and in
    ** sorted order.  This invariants arise because multiple overflows can
    ** only occur when inserting divider cells into the parent page during
    ** balancing, and the dividers are adjacent and sorted.
    */
    ప్రతిజ్ఞా( j==0 || p_పుట->aiOvfl[j-1]<(u16)i ); /* Overflows in sorted order */
    ప్రతిజ్ఞా( j==0 || i==p_పుట->aiOvfl[j-1]+1 );   /* Overflows are sequential */
  }లేకపోతే{
    పూర్ణాంకం rc = sqlite3PagerWrite(p_పుట->p_Db_పుట);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
    ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_పుట->p_Db_పుట) );
    దత్తాంశం = p_పుట->a_దత్తాంశం;
    ప్రతిజ్ఞా( &దత్తాంశం[p_పుట->cell_స్థానభ్రంశం]==p_పుట->aCellIdx );
    rc = allocateSpace(p_పుట, sz, &idx);
    ఒకవేళ( rc ){ తిరిగి rc; }
    /* The allocateSpace() routine guarantees the following properties
    ** if it returns successfully */
    ప్రతిజ్ఞా( idx >= 0 );
    ప్రతిజ్ఞా( idx >= p_పుట->cell_స్థానభ్రంశం+2*p_పుట->nCell+2 || CORRUPT_DB );
    ప్రతిజ్ఞా( idx+sz <= (పూర్ణాంకం)p_పుట->pBt->usable_కొలత );
    p_పుట->nFree -= (u16)(2 + sz);
    జ్ఞాపకం_నకలు(&దత్తాంశం[idx], pCell, sz);
    pIns = p_పుట->aCellIdx + i*2;
    ఒకవేళ( i<p_పుట->nCell ) memmove(pIns+2, pIns, 2*(p_పుట->nCell - i));
    put2byte(pIns, idx);
    p_పుట->nCell++;
    /* increment the cell count */
    ఒకవేళ( (++దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+4])==0 ) దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+3]++;
    ప్రతిజ్ఞా( get2byte(&దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+3])==p_పుట->nCell || CORRUPT_DB );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
    ఒకవేళ( p_పుట->pBt->autoVacuum ){
      పూర్ణాంకం rc2 = SQLITE_OK;
      /* The cell may contain a pointer to an overflow page. If so, write
      ** the entry for the overflow page into the pointer map.
      */
      ptrmapPutOvflPtr(p_పుట, p_పుట, pCell, &rc2);
      ఒకవేళ( rc2 ) తిరిగి rc2;
    }
#అయితే_ముగింపు
  }
  తిరిగి SQLITE_OK;
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
#నిర్వచించు NN 1             /* Number of neighbors on either side of pPage */
#నిర్వచించు NB 3             /* (NN*2+1): Total pages involved in the balance */

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
రకనిర్వచనం నిర్మాణం CellArray CellArray;
నిర్మాణం CellArray {
  పూర్ణాంకం nCell;              /* Number of cells in apCell[] */
  Mem_పుట *pRef;          /* Reference page */
  u8 **apCell;            /* All cells begin balanced */
  u16 *szCell;            /* Local size of all cells in apCell[] */
  u8 *ap_చివర[NB*2];        /* MemPage.aDataEnd values */
  పూర్ణాంకం ixNx[NB*2];         /* Index of at which we move to the next apEnd[] */
};

/*
** Make sure the cell sizes at idx, idx+1, ..., idx+N-1 have been
** computed.
*/
స్థిర శూన్య populateCellCache(CellArray *p, పూర్ణాంకం idx, పూర్ణాంకం N){
  Mem_పుట *pRef = p->pRef;
  u16 *szCell = p->szCell;
  ప్రతిజ్ఞా( idx>=0 && idx+N<=p->nCell );
  అయితే( N>0 ){
    ప్రతిజ్ఞా( p->apCell[idx]!=0 );
    ఒకవేళ( szCell[idx]==0 ){
      szCell[idx] = pRef->x_Cell_కొలత(pRef, p->apCell[idx]);
    }లేకపోతే{
      ప్రతిజ్ఞా( CORRUPT_DB ||
              szCell[idx]==pRef->x_Cell_కొలత(pRef, p->apCell[idx]) );
    }
    idx++;
    N--;
  }
}

/*
** Return the size of the Nth element of the cell array
*/
స్థిర SQLITE_NOINLINE u16 compute_Cell_కొలత(CellArray *p, పూర్ణాంకం N){
  ప్రతిజ్ఞా( N>=0 && N<p->nCell );
  ప్రతిజ్ఞా( p->szCell[N]==0 );
  p->szCell[N] = p->pRef->x_Cell_కొలత(p->pRef, p->apCell[N]);
  తిరిగి p->szCell[N];
}
స్థిర u16 cached_Cell_కొలత(CellArray *p, పూర్ణాంకం N){
  ప్రతిజ్ఞా( N>=0 && N<p->nCell );
  ఒకవేళ( p->szCell[N] ) తిరిగి p->szCell[N];
  తిరిగి compute_Cell_కొలత(p, N);
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
స్థిర పూర్ణాంకం rebuild_పుట(
  CellArray *pCArray,             /* Content to be added to page pPg */
  పూర్ణాంకం i_మొదటి,                     /* First cell in pCArray to use */
  పూర్ణాంకం nCell,                      /* Final number of cells on page */
  Mem_పుట *pPg                    /* The page to be reconstructed */
){
  స్థిరరాశి పూర్ణాంకం hdr = pPg->hdr_స్థానభ్రంశం;          /* Offset of header on pPg */
  u8 * స్థిరరాశి a_దత్తాంశం = pPg->a_దత్తాంశం;           /* Pointer to data for pPg */
  స్థిరరాశి పూర్ణాంకం usable_కొలత = pPg->pBt->usable_కొలత;
  u8 * స్థిరరాశి p_చివర = &a_దత్తాంశం[usable_కొలత];
  పూర్ణాంకం i = i_మొదటి;                 /* Which cell to copy from pCArray*/
  u32 j;                          /* Start of cell content area */
  పూర్ణాంకం i_చివర = i+nCell;             /* Loop terminator */
  u8 *pCellptr = pPg->aCellIdx;
  u8 *pTmp = sqlite3_Pager_తాత్కాలిక_Space(pPg->pBt->pPager);
  u8 *p_దత్తాంశం;
  పూర్ణాంకం k;                          /* Current slot in pCArray->apEnd[] */
  u8 *p_Src_చివర;                    /* Current pCArray->apEnd[k] value */

  ప్రతిజ్ఞా( nCell>0 );
  ప్రతిజ్ఞా( i<i_చివర );
  j = get2byte(&a_దత్తాంశం[hdr+5]);
  ఒకవేళ( j>(u32)usable_కొలత ){ j = 0; }
  జ్ఞాపకం_నకలు(&pTmp[j], &a_దత్తాంశం[j], usable_కొలత - j);

  ప్రతిజ్ఞా( pCArray->ixNx[NB*2-1]>i );
  కొరకు(k=0; pCArray->ixNx[k]<=i; k++){}
  p_Src_చివర = pCArray->ap_చివర[k];

  p_దత్తాంశం = p_చివర;
  అయితే( 1/*exit by break*/ ){
    u8 *pCell = pCArray->apCell[i];
    u16 sz = pCArray->szCell[i];
    ప్రతిజ్ఞా( sz>0 );
    ఒకవేళ( SQLITE_WITHIN(pCell,a_దత్తాంశం+j,p_చివర) ){
      ఒకవేళ( ((uptr)(pCell+sz))>(uptr)p_చివర ) తిరిగి SQLITE_CORRUPT_BKPT;
      pCell = &pTmp[pCell - a_దత్తాంశం];
    }లేకపోతే ఒకవేళ( (uptr)(pCell+sz)>(uptr)p_Src_చివర
           && (uptr)(pCell)<(uptr)p_Src_చివర
    ){
      తిరిగి SQLITE_CORRUPT_BKPT;
    }

    p_దత్తాంశం -= sz;
    put2byte(pCellptr, (p_దత్తాంశం - a_దత్తాంశం));
    pCellptr += 2;
    ఒకవేళ( p_దత్తాంశం < pCellptr ) తిరిగి SQLITE_CORRUPT_BKPT;
    memmove(p_దత్తాంశం, pCell, sz);
    ప్రతిజ్ఞా( sz==pPg->x_Cell_కొలత(pPg, pCell) || CORRUPT_DB );
    i++;
    ఒకవేళ( i>=i_చివర ) ఆపు;
    ఒకవేళ( pCArray->ixNx[k]<=i ){
      k++;
      p_Src_చివర = pCArray->ap_చివర[k];
    }
  }

  /* The pPg->nFree field is now set incorrectly. The caller will fix it. */
  ప్రతిజ్ఞా( nCell < 10922 );
  pPg->nCell = (u16)nCell;
  pPg->n_అధికప్రవాహం = 0;

  put2byte(&a_దత్తాంశం[hdr+1], 0);
  put2byte(&a_దత్తాంశం[hdr+3], pPg->nCell);
  put2byte(&a_దత్తాంశం[hdr+5], p_దత్తాంశం - a_దత్తాంశం);
  a_దత్తాంశం[hdr+7] = 0x00;
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం పుట_చొప్పించు_Array(
  Mem_పుట *pPg,                   /* Page to add cells to */
  u8 *pBegin,                     /* End of cell-pointer array */
  u8 **pp_దత్తాంశం,                    /* IN/OUT: Page content-area pointer */
  u8 *pCellptr,                   /* Pointer to cell-pointer area */
  పూర్ణాంకం i_మొదటి,                     /* Index of first cell to add */
  పూర్ణాంకం nCell,                      /* Number of cells to add to pPg */
  CellArray *pCArray              /* Array of cells */
){
  పూర్ణాంకం i = i_మొదటి;                 /* Loop counter - cell index to insert */
  u8 *a_దత్తాంశం = pPg->a_దత్తాంశం;         /* Complete page */
  u8 *p_దత్తాంశం = *pp_దత్తాంశం;            /* Content area.  A subset of aData[] */
  పూర్ణాంకం i_చివర = i_మొదటి + nCell;      /* End of loop. One past last cell to ins */
  పూర్ణాంకం k;                          /* Current slot in pCArray->apEnd[] */
  u8 *p_చివర;                       /* Maximum extent of cell data */
  ప్రతిజ్ఞా( CORRUPT_DB || pPg->hdr_స్థానభ్రంశం==0 );    /* Never called on page 1 */
  ఒకవేళ( i_చివర<=i_మొదటి ) తిరిగి 0;
  ప్రతిజ్ఞా( pCArray->ixNx[NB*2-1]>i );
  కొరకు(k=0; pCArray->ixNx[k]<=i ; k++){}
  p_చివర = pCArray->ap_చివర[k];
  అయితే( 1 /*Exit by break*/ ){
    పూర్ణాంకం sz, rc;
    u8 *p_స్థానం;
    ప్రతిజ్ఞా( pCArray->szCell[i]!=0 );
    sz = pCArray->szCell[i];
    ఒకవేళ( (a_దత్తాంశం[1]==0 && a_దత్తాంశం[2]==0) || (p_స్థానం = పుట_కనుగొను_స్థానం(pPg,sz,&rc))==0 ){
      ఒకవేళ( (p_దత్తాంశం - pBegin)<sz ) తిరిగి 1;
      p_దత్తాంశం -= sz;
      p_స్థానం = p_దత్తాంశం;
    }
    /* pSlot and pCArray->apCell[i] will never overlap on a well-formed
    ** database.  But they might for a corrupt database.  Hence use memmove()
    ** since memcpy() sends SIGABORT with overlapping buffers on OpenBSD */
    ప్రతిజ్ఞా( (p_స్థానం+sz)<=pCArray->apCell[i]
         || p_స్థానం>=(pCArray->apCell[i]+sz)
         || CORRUPT_DB );
    ఒకవేళ( (uptr)(pCArray->apCell[i]+sz)>(uptr)p_చివర
     && (uptr)(pCArray->apCell[i])<(uptr)p_చివర
    ){
      ప్రతిజ్ఞా( CORRUPT_DB );
      (శూన్య)SQLITE_CORRUPT_BKPT;
      తిరిగి 1;
    }
    memmove(p_స్థానం, pCArray->apCell[i], sz);
    put2byte(pCellptr, (p_స్థానం - a_దత్తాంశం));
    pCellptr += 2;
    i++;
    ఒకవేళ( i>=i_చివర ) ఆపు;
    ఒకవేళ( pCArray->ixNx[k]<=i ){
      k++;
      p_చివర = pCArray->ap_చివర[k];
    }
  }
  *pp_దత్తాంశం = p_దత్తాంశం;
  తిరిగి 0;
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
స్థిర పూర్ణాంకం పుట_Free_Array(
  Mem_పుట *pPg,                   /* Page to edit */
  పూర్ణాంకం i_మొదటి,                     /* First cell to delete */
  పూర్ణాంకం nCell,                      /* Cells to delete */
  CellArray *pCArray              /* Array of cells */
){
  u8 * స్థిరరాశి a_దత్తాంశం = pPg->a_దత్తాంశం;
  u8 * స్థిరరాశి p_చివర = &a_దత్తాంశం[pPg->pBt->usable_కొలత];
  u8 * స్థిరరాశి p_ప్రారంభం = &a_దత్తాంశం[pPg->hdr_స్థానభ్రంశం + 8 + pPg->బిడ్డ_Ptr_కొలత];
  పూర్ణాంకం nRet = 0;
  పూర్ణాంకం i, j;
  పూర్ణాంకం i_చివర = i_మొదటి + nCell;
  పూర్ణాంకం nFree = 0;
  పూర్ణాంకం aOfst[10];
  పూర్ణాంకం aAfter[10];

  కొరకు(i=i_మొదటి; i<i_చివర; i++){
    u8 *pCell = pCArray->apCell[i];
    ఒకవేళ( SQLITE_WITHIN(pCell, p_ప్రారంభం, p_చివర) ){
      పూర్ణాంకం sz;
      పూర్ణాంకం iAfter;
      పూర్ణాంకం iOfst;
      /* No need to use cachedCellSize() here.  The sizes of all cells that
      ** are to be freed have already been computing while deciding which
      ** cells need freeing */
      sz = pCArray->szCell[i];  ప్రతిజ్ఞా( sz>0 );
      iOfst = (u16)(pCell - a_దత్తాంశం);
      iAfter = iOfst+sz;
      కొరకు(j=0; j<nFree; j++){
        ఒకవేళ( aOfst[j]==iAfter ){
          aOfst[j] = iOfst;
          ఆపు;
        }లేకపోతే ఒకవేళ( aAfter[j]==iOfst ){
          aAfter[j] = iAfter;
          ఆపు;
        }
      }
      ఒకవేళ( j>=nFree ){
        ఒకవేళ( nFree>=(పూర్ణాంకం)(పరిమాణం(aOfst)/పరిమాణం(aOfst[0])) ){
          కొరకు(j=0; j<nFree; j++){
            freeSpace(pPg, aOfst[j], aAfter[j]-aOfst[j]);
          }
          nFree = 0;
        }
        aOfst[nFree] = iOfst;
        aAfter[nFree] = iAfter;
        ఒకవేళ( &a_దత్తాంశం[iAfter]>p_చివర ) తిరిగి 0;
        nFree++;
      }
      nRet++;
    }
  }
  కొరకు(j=0; j<nFree; j++){
    freeSpace(pPg, aOfst[j], aAfter[j]-aOfst[j]);
  }
  తిరిగి nRet;
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
స్థిర పూర్ణాంకం edit_పుట(
  Mem_పుట *pPg,                   /* Edit this page */
  పూర్ణాంకం iOld,                       /* Index of first cell currently on page */
  పూర్ణాంకం iNew,                       /* Index of new first cell on page */
  పూర్ణాంకం nNew,                       /* Final number of cells on page */
  CellArray *pCArray              /* Array of cells and sizes */
){
  u8 * స్థిరరాశి a_దత్తాంశం = pPg->a_దత్తాంశం;
  స్థిరరాశి పూర్ణాంకం hdr = pPg->hdr_స్థానభ్రంశం;
  u8 *pBegin = &pPg->aCellIdx[nNew * 2];
  పూర్ణాంకం nCell = pPg->nCell;       /* Cells stored on pPg */
  u8 *p_దత్తాంశం;
  u8 *pCellptr;
  పూర్ణాంకం i;
  పూర్ణాంకం i_Old_చివర = iOld + pPg->nCell + pPg->n_అధికప్రవాహం;
  పూర్ణాంకం i_New_చివర = iNew + nNew;

#నిర్వచితమైతే SQLITE_DEBUG
  u8 *pTmp = sqlite3_Pager_తాత్కాలిక_Space(pPg->pBt->pPager);
  జ్ఞాపకం_నకలు(pTmp, a_దత్తాంశం, pPg->pBt->usable_కొలత);
#అయితే_ముగింపు

  /* Remove cells from the start and end of the page */
  ప్రతిజ్ఞా( nCell>=0 );
  ఒకవేళ( iOld<iNew ){
    పూర్ణాంకం n_జరుపు = పుట_Free_Array(pPg, iOld, iNew-iOld, pCArray);
    ఒకవేళ( NEVER(n_జరుపు>nCell) ) తిరిగి SQLITE_CORRUPT_BKPT;
    memmove(pPg->aCellIdx, &pPg->aCellIdx[n_జరుపు*2], nCell*2);
    nCell -= n_జరుపు;
  }
  ఒకవేళ( i_New_చివర < i_Old_చివర ){
    పూర్ణాంకం n_తోక = పుట_Free_Array(pPg, i_New_చివర, i_Old_చివర - i_New_చివర, pCArray);
    ప్రతిజ్ఞా( nCell>=n_తోక );
    nCell -= n_తోక;
  }

  p_దత్తాంశం = &a_దత్తాంశం[get2byte(&a_దత్తాంశం[hdr+5])];
  ఒకవేళ( p_దత్తాంశం<pBegin ) వెళ్ళు editpage_fail;
  ఒకవేళ( NEVER(p_దత్తాంశం>pPg->a_దత్తాంశం_చివర) ) వెళ్ళు editpage_fail;

  /* Add cells to the start of the page */
  ఒకవేళ( iNew<iOld ){
    పూర్ణాంకం n_కూడు = MIN(nNew,iOld-iNew);
    ప్రతిజ్ఞా( (iOld-iNew)<nNew || nCell==0 || CORRUPT_DB );
    ప్రతిజ్ఞా( n_కూడు>=0 );
    pCellptr = pPg->aCellIdx;
    memmove(&pCellptr[n_కూడు*2], pCellptr, nCell*2);
    ఒకవేళ( పుట_చొప్పించు_Array(
          pPg, pBegin, &p_దత్తాంశం, pCellptr,
          iNew, n_కూడు, pCArray
    ) ) వెళ్ళు editpage_fail;
    nCell += n_కూడు;
  }

  /* Add any overflow cells */
  కొరకు(i=0; i<pPg->n_అధికప్రవాహం; i++){
    పూర్ణాంకం iCell = (iOld + pPg->aiOvfl[i]) - iNew;
    ఒకవేళ( iCell>=0 && iCell<nNew ){
      pCellptr = &pPg->aCellIdx[iCell * 2];
      ఒకవేళ( nCell>iCell ){
        memmove(&pCellptr[2], pCellptr, (nCell - iCell) * 2);
      }
      nCell++;
      cached_Cell_కొలత(pCArray, iCell+iNew);
      ఒకవేళ( పుట_చొప్పించు_Array(
            pPg, pBegin, &p_దత్తాంశం, pCellptr,
            iCell+iNew, 1, pCArray
      ) ) వెళ్ళు editpage_fail;
    }
  }

  /* Append cells to the end of the page */
  ప్రతిజ్ఞా( nCell>=0 );
  pCellptr = &pPg->aCellIdx[nCell*2];
  ఒకవేళ( పుట_చొప్పించు_Array(
        pPg, pBegin, &p_దత్తాంశం, pCellptr,
        iNew+nCell, nNew-nCell, pCArray
      )
  ){
    వెళ్ళు editpage_fail;
  }

  ప్రతిజ్ఞా( nNew < 10922 );
  pPg->nCell = (u16)nNew;
  pPg->n_అధికప్రవాహం = 0;

  put2byte(&a_దత్తాంశం[hdr+3], pPg->nCell);
  put2byte(&a_దత్తాంశం[hdr+5], p_దత్తాంశం - a_దత్తాంశం);

#నిర్వచితమైతే SQLITE_DEBUG
  కొరకు(i=0; i<nNew && !CORRUPT_DB; i++){
    u8 *pCell = pCArray->apCell[i+iNew];
    పూర్ణాంకం iOff = get2byteAligned(&pPg->aCellIdx[i*2]);
    ఒకవేళ( SQLITE_WITHIN(pCell, a_దత్తాంశం, &a_దత్తాంశం[pPg->pBt->usable_కొలత]) ){
      pCell = &pTmp[pCell - a_దత్తాంశం];
    }
    ప్రతిజ్ఞా( 0==memcmp(pCell, &a_దత్తాంశం[iOff],
            pCArray->pRef->x_Cell_కొలత(pCArray->pRef, pCArray->apCell[i+iNew])) );
  }
#అయితే_ముగింపు

  తిరిగి SQLITE_OK;
 editpage_fail:
  /* Unable to edit this page. Rebuild it from scratch instead. */
  ఒకవేళ( nNew<1 ) తిరిగి SQLITE_CORRUPT_BKPT;
  populateCellCache(pCArray, iNew, nNew);
  తిరిగి rebuild_పుట(pCArray, iNew, nNew, pPg);
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_QUICKBALANCE
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
స్థిర పూర్ణాంకం balance_శీఘ్ర(Mem_పుట *p_మాతృ, Mem_పుట *p_పుట, u8 *pSpace){
  BtShared *స్థిరరాశి pBt = p_పుట->pBt;    /* B-Tree Database */
  Mem_పుట *pNew;                       /* Newly allocated page */
  పూర్ణాంకం rc;                              /* Return Code */
  Pgno pgnoNew;                        /* Page number of pNew */

  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pBt->mutex) );
  ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_మాతృ->p_Db_పుట) );
  ప్రతిజ్ఞా( p_పుట->n_అధికప్రవాహం==1 );
 
  ఒకవేళ( p_పుట->nCell==0 ) తిరిగి SQLITE_CORRUPT_BKPT;  /* dbfuzz001.test */
  ప్రతిజ్ఞా( p_పుట->nFree>=0 );
  ప్రతిజ్ఞా( p_మాతృ->nFree>=0 );

  /* Allocate a new page. This page will become the right-sibling of
  ** pPage. Make the parent page writable, so that the new divider cell
  ** may be inserted. If both these operations are successful, proceed.
  */
  rc = allocate_Btree_పుట(pBt, &pNew, &pgnoNew, 0, 0);

  ఒకవేళ( rc==SQLITE_OK ){

    u8 *p_బహిర్గతం = &pSpace[4];
    u8 *pCell = p_పుట->apOvfl[0];
    u16 szCell = p_పుట->x_Cell_కొలత(p_పుట, pCell);
    u8 *pStop;
    CellArray b;

    ప్రతిజ్ఞా( sqlite3PagerIswriteable(pNew->p_Db_పుట) );
    ప్రతిజ్ఞా( CORRUPT_DB || p_పుట->a_దత్తాంశం[0]==(PTF_INTKEY|PTF_LEAFDATA|PTF_ఆకు) );
    zero_పుట(pNew, PTF_INTKEY|PTF_LEAFDATA|PTF_ఆకు);
    b.nCell = 1;
    b.pRef = p_పుట;
    b.apCell = &pCell;
    b.szCell = &szCell;
    b.ap_చివర[0] = p_పుట->a_దత్తాంశం_చివర;
    b.ixNx[0] = 2;
    b.ixNx[NB*2-1] = 0x7fffffff;
    rc = rebuild_పుట(&b, 0, 1, pNew);
    ఒకవేళ( NEVER(rc) ){
      release_పుట(pNew);
      తిరిగి rc;
    }
    pNew->nFree = pBt->usable_కొలత - pNew->cell_స్థానభ్రంశం - 2 - szCell;

    /* If this is an auto-vacuum database, update the pointer map
    ** with entries for the new page, and any pointer from the
    ** cell on the page to an overflow page. If either of these
    ** operations fails, the return code is set, but the contents
    ** of the parent page are still manipulated by the code below.
    ** That is Ok, at this point the parent page is guaranteed to
    ** be marked as dirty. Returning an error code will cause a
    ** rollback, undoing any changes made to the parent page.
    */
    ఒకవేళ( ISAUTOVACUUM(pBt) ){
      ptrmapPut(pBt, pgnoNew, PTRMAP_BTREE, p_మాతృ->pgno, &rc);
      ఒకవేళ( szCell>pNew->minLocal ){
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
    pCell = కనుగొను_Cell(p_పుట, p_పుట->nCell-1);
    pStop = &pCell[9];
    అయితే( (*(pCell++)&0x80) && pCell<pStop );
    pStop = &pCell[9];
    అయితే( ((*(p_బహిర్గతం++) = *(pCell++))&0x80) && pCell<pStop );

    /* Insert the new divider cell into pParent. */
    ఒకవేళ( rc==SQLITE_OK ){
      rc = చొప్పించు_Cell(p_మాతృ, p_మాతృ->nCell, pSpace, (పూర్ణాంకం)(p_బహిర్గతం-pSpace),
                      0, p_పుట->pgno);
    }

    /* Set the right-child pointer of pParent to point to the new page. */
    put4byte(&p_మాతృ->a_దత్తాంశం[p_మాతృ->hdr_స్థానభ్రంశం+8], pgnoNew);
 
    /* Release the reference to the new page. */
    release_పుట(pNew);
  }

  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_QUICKBALANCE */

#ఒకవేళ 0
/*
** This function does not contribute anything to the operation of SQLite.
** it is sometimes activated temporarily while debugging code responsible
** for setting pointer-map entries.
*/
స్థిర పూర్ణాంకం ptrmap_Check_పుటలు(Mem_పుట **ap_పుట, పూర్ణాంకం n_పుట){
  పూర్ణాంకం i, j;
  కొరకు(i=0; i<n_పుట; i++){
    Pgno n;
    u8 e;
    Mem_పుట *p_పుట = ap_పుట[i];
    BtShared *pBt = p_పుట->pBt;
    ప్రతిజ్ఞా( p_పుట->isInit );

    కొరకు(j=0; j<p_పుట->nCell; j++){
      CellInfo info;
      u8 *z;
    
      z = కనుగొను_Cell(p_పుట, j);
      p_పుట->xParseCell(p_పుట, z, &info);
      ఒకవేళ( info.nLocal<info.nPayload ){
        Pgno ovfl = get4byte(&z[info.n_కొలత-4]);
        ptrmapGet(pBt, ovfl, &e, &n);
        ప్రతిజ్ఞా( n==p_పుట->pgno && e==PTRMAP_OVERFLOW1 );
      }
      ఒకవేళ( !p_పుట->ఆకు ){
        Pgno బిడ్డ = get4byte(z);
        ptrmapGet(pBt, బిడ్డ, &e, &n);
        ప్రతిజ్ఞా( n==p_పుట->pgno && e==PTRMAP_BTREE );
      }
    }
    ఒకవేళ( !p_పుట->ఆకు ){
      Pgno బిడ్డ = get4byte(&p_పుట->a_దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+8]);
      ptrmapGet(pBt, బిడ్డ, &e, &n);
      ప్రతిజ్ఞా( n==p_పుట->pgno && e==PTRMAP_BTREE );
    }
  }
  తిరిగి 1;
}
#అయితే_ముగింపు

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
స్థిర శూన్య copy_కణుపు_Content(Mem_పుట *pFrom, Mem_పుట *p_గమ్యం, పూర్ణాంకం *pRC){
  ఒకవేళ( (*pRC)==SQLITE_OK ){
    BtShared * స్థిరరాశి pBt = pFrom->pBt;
    u8 * స్థిరరాశి aFrom = pFrom->a_దత్తాంశం;
    u8 * స్థిరరాశి a_గమ్యం = p_గమ్యం->a_దత్తాంశం;
    పూర్ణాంకం స్థిరరాశి iFromHdr = pFrom->hdr_స్థానభ్రంశం;
    పూర్ణాంకం స్థిరరాశి i_గమ్యం_Hdr = ((p_గమ్యం->pgno==1) ? 100 : 0);
    పూర్ణాంకం rc;
    పూర్ణాంకం i_దత్తాంశం;
 
 
    ప్రతిజ్ఞా( pFrom->isInit );
    ప్రతిజ్ఞా( pFrom->nFree>=i_గమ్యం_Hdr );
    ప్రతిజ్ఞా( get2byte(&aFrom[iFromHdr+5]) <= (పూర్ణాంకం)pBt->usable_కొలత );
 
    /* Copy the b-tree node content from page pFrom to page pTo. */
    i_దత్తాంశం = get2byte(&aFrom[iFromHdr+5]);
    జ్ఞాపకం_నకలు(&a_గమ్యం[i_దత్తాంశం], &aFrom[i_దత్తాంశం], pBt->usable_కొలత-i_దత్తాంశం);
    జ్ఞాపకం_నకలు(&a_గమ్యం[i_గమ్యం_Hdr], &aFrom[iFromHdr], pFrom->cell_స్థానభ్రంశం + 2*pFrom->nCell);
 
    /* Reinitialize page pTo so that the contents of the MemPage structure
    ** match the new data. The initialization of pTo can actually fail under
    ** fairly obscure circumstances, even though it is a copy of initialized
    ** page pFrom.
    */
    p_గమ్యం->isInit = 0;
    rc = btree_Init_పుట(p_గమ్యం);
    ఒకవేళ( rc==SQLITE_OK ) rc = btreeComputeFreeSpace(p_గమ్యం);
    ఒకవేళ( rc!=SQLITE_OK ){
      *pRC = rc;
      తిరిగి;
    }
 
    /* If this is an auto-vacuum database, update the pointer-map entries
    ** for any b-tree or overflow pages that pTo now contains the pointers to.
    */
    ఒకవేళ( ISAUTOVACUUM(pBt) ){
      *pRC = సమితి_బిడ్డ_Ptrmaps(p_గమ్యం);
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
స్థిర పూర్ణాంకం balance_nonroot(
  Mem_పుట *p_మాతృ,               /* Parent page of siblings being balanced */
  పూర్ణాంకం i_మాతృ_Idx,                 /* Index of "the page" in pParent */
  u8 *aOvflSpace,                 /* page-size bytes of space for parent ovfl */
  పూర్ణాంకం is_మూలం,                     /* True if pParent is a root-page */
  పూర్ణాంకం bBulk                       /* True if this call is part of a bulk load */
){
  BtShared *pBt;               /* The whole database */
  పూర్ణాంకం nMaxCells = 0;           /* Allocated size of apCell, szCell, aFrom. */
  పూర్ణాంకం nNew = 0;                /* Number of pages in apNew[] */
  పూర్ణాంకం nOld;                    /* Number of pages in apOld[] */
  పూర్ణాంకం i, j, k;                 /* Loop counters */
  పూర్ణాంకం nxDiv;                   /* Next divider slot in pParent->aCell[] */
  పూర్ణాంకం rc = SQLITE_OK;          /* The return code */
  u16 ఆకు_Correction;          /* 4 if pPage is a leaf.  0 if not */
  పూర్ణాంకం ఆకు_దత్తాంశం;                /* True if pPage is a leaf of a LEAFDATA tree */
  పూర్ణాంకం usableSpace;             /* Bytes in pPage beyond the header */
  పూర్ణాంకం పుట_Flags;               /* Value of pPage->aData[0] */
  పూర్ణాంకం iSpace1 = 0;             /* First unused byte of aSpace1[] */
  పూర్ణాంకం iOvflSpace = 0;          /* First unused byte of aOvflSpace[] */
  u64 szScratch;               /* Size of scratch memory requested */
  Mem_పుట *apOld[NB];          /* pPage and up to two siblings */
  Mem_పుట *apNew[NB+2];        /* pPage and up to NB siblings after balancing */
  u8 *p_కుడి;                  /* Location in parent of right-sibling pointer */
  u8 *apDiv[NB-1];             /* Divider cells in pParent */
  పూర్ణాంకం cntNew[NB+2];            /* Index in b.paCell[] of cell after i-th page */
  పూర్ణాంకం cntOld[NB+2];            /* Old index in b.apCell[] */
  పూర్ణాంకం szNew[NB+2];             /* Combined size of cells placed on i-th page */
  u8 *aSpace1;                 /* Space for copies of dividers cells */
  Pgno pgno;                   /* Temp var to store a page number in */
  u8 ab_పూర్తి[NB+2];             /* True after i'th new page is populated */
  Pgno aPgno[NB+2];            /* Page numbers of new pages before shuffling */
  CellArray b;                 /* Parsed information on cells being balanced */

  memset(ab_పూర్తి, 0, పరిమాణం(ab_పూర్తి));
  ప్రతిజ్ఞా( పరిమాణం(b) - పరిమాణం(b.ixNx) == offsetof(CellArray,ixNx) );
  memset(&b, 0, పరిమాణం(b)-పరిమాణం(b.ixNx[0]));
  b.ixNx[NB*2-1] = 0x7fffffff;
  pBt = p_మాతృ->pBt;
  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_మాతృ->p_Db_పుట) );

  /* At this point pParent may have at most one overflow cell. And if
  ** this overflow cell is present, it must be the cell with
  ** index iParentIdx. This scenario comes about when this function
  ** is called (indirectly) from sqlite3BtreeDelete().
  */
  ప్రతిజ్ఞా( p_మాతృ->n_అధికప్రవాహం==0 || p_మాతృ->n_అధికప్రవాహం==1 );
  ప్రతిజ్ఞా( p_మాతృ->n_అధికప్రవాహం==0 || p_మాతృ->aiOvfl[0]==i_మాతృ_Idx );

  ఒకవేళ( !aOvflSpace ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  ప్రతిజ్ఞా( p_మాతృ->nFree>=0 );

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
  i = p_మాతృ->n_అధికప్రవాహం + p_మాతృ->nCell;
  ఒకవేళ( i<2 ){
    nxDiv = 0;
  }లేకపోతే{
    ప్రతిజ్ఞా( bBulk==0 || bBulk==1 );
    ఒకవేళ( i_మాతృ_Idx==0 ){                
      nxDiv = 0;
    }లేకపోతే ఒకవేళ( i_మాతృ_Idx==i ){
      nxDiv = i-2+bBulk;
    }లేకపోతే{
      nxDiv = i_మాతృ_Idx-1;
    }
    i = 2-bBulk;
  }
  nOld = i+1;
  ఒకవేళ( (i+nxDiv-p_మాతృ->n_అధికప్రవాహం)==p_మాతృ->nCell ){
    p_కుడి = &p_మాతృ->a_దత్తాంశం[p_మాతృ->hdr_స్థానభ్రంశం+8];
  }లేకపోతే{
    p_కుడి = కనుగొను_Cell(p_మాతృ, i+nxDiv-p_మాతృ->n_అధికప్రవాహం);
  }
  pgno = get4byte(p_కుడి);
  అయితే( 1 ){
    ఒకవేళ( rc==SQLITE_OK ){
      rc = get_And_Init_పుట(pBt, pgno, &apOld[i], 0);
    }
    ఒకవేళ( rc ){
      memset(apOld, 0, (i+1)*పరిమాణం(Mem_పుట*));
      వెళ్ళు balance_cleanup;
    }
    ఒకవేళ( apOld[i]->nFree<0 ){
      rc = btreeComputeFreeSpace(apOld[i]);
      ఒకవేళ( rc ){
        memset(apOld, 0, (i)*పరిమాణం(Mem_పుట*));
        వెళ్ళు balance_cleanup;
      }
    }
    nMaxCells += apOld[i]->nCell + Array_కొలత(p_మాతృ->apOvfl);
    ఒకవేళ( (i--)==0 ) ఆపు;

    ఒకవేళ( p_మాతృ->n_అధికప్రవాహం && i+nxDiv==p_మాతృ->aiOvfl[0] ){
      apDiv[i] = p_మాతృ->apOvfl[0];
      pgno = get4byte(apDiv[i]);
      szNew[i] = p_మాతృ->x_Cell_కొలత(p_మాతృ, apDiv[i]);
      p_మాతృ->n_అధికప్రవాహం = 0;
    }లేకపోతే{
      apDiv[i] = కనుగొను_Cell(p_మాతృ, i+nxDiv-p_మాతృ->n_అధికప్రవాహం);
      pgno = get4byte(apDiv[i]);
      szNew[i] = p_మాతృ->x_Cell_కొలత(p_మాతృ, apDiv[i]);

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
      ఒకవేళ( pBt->btsFlags & BTS_FAST_SECURE ){
        పూర్ణాంకం iOff;

        /* If the following if() condition is not true, the db is corrupted.
        ** The call to dropCell() below will detect this.  */
        iOff = SQLITE_PTR_గమ్యం_INT(apDiv[i]) - SQLITE_PTR_గమ్యం_INT(p_మాతృ->a_దత్తాంశం);
        ఒకవేళ( (iOff+szNew[i])<=(పూర్ణాంకం)pBt->usable_కొలత ){
          జ్ఞాపకం_నకలు(&aOvflSpace[iOff], apDiv[i], szNew[i]);
          apDiv[i] = &aOvflSpace[apDiv[i]-p_మాతృ->a_దత్తాంశం];
        }
      }
      dropCell(p_మాతృ, i+nxDiv-p_మాతృ->n_అధికప్రవాహం, szNew[i], &rc);
    }
  }

  /* Make nMaxCells a multiple of 4 in order to preserve 8-byte
  ** alignment */
  nMaxCells = (nMaxCells + 3)&~3;

  /*
  ** Allocate space for memory structures
  */
  szScratch =
       nMaxCells*పరిమాణం(u8*)                       /* b.apCell */
     + nMaxCells*పరిమాణం(u16)                       /* b.szCell */
     + pBt->పుట_కొలత;                              /* aSpace1 */

  ప్రతిజ్ఞా( szScratch<=7*(పూర్ణాంకం)pBt->పుట_కొలత );
  b.apCell = sqlite3_దొంతర_Alloc_Raw(0, szScratch );
  ఒకవేళ( b.apCell==0 ){
    rc = SQLITE_NOMEM_BKPT;
    వెళ్ళు balance_cleanup;
  }
  b.szCell = (u16*)&b.apCell[nMaxCells];
  aSpace1 = (u8*)&b.szCell[nMaxCells];
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT(aSpace1) );

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
  ఆకు_Correction = b.pRef->ఆకు*4;
  ఆకు_దత్తాంశం = b.pRef->int_కీలకం_ఆకు;
  కొరకు(i=0; i<nOld; i++){
    Mem_పుట *pOld = apOld[i];
    పూర్ణాంకం హద్దు = pOld->nCell;
    u8 *a_దత్తాంశం = pOld->a_దత్తాంశం;
    u16 ముసుగు_పుట = pOld->ముసుగు_పుట;
    u8 *piCell = a_దత్తాంశం + pOld->cell_స్థానభ్రంశం;
    u8 *pi_చివర;
    u8 **ppCell;
    VVA_ONLY( పూర్ణాంకం n_Cell_At_ప్రారంభం = b.nCell; )

    /* Verify that all sibling pages are of the same "type" (table-leaf,
    ** table-interior, index-leaf, or index-interior).
    */
    ఒకవేళ( pOld->a_దత్తాంశం[0]!=apOld[0]->a_దత్తాంశం[0] ){
      rc = SQLITE_CORRUPT_పుట(pOld);
      వెళ్ళు balance_cleanup;
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
    memset(&b.szCell[b.nCell], 0, పరిమాణం(b.szCell[0])*(హద్దు+pOld->n_అధికప్రవాహం));
    ఒకవేళ( pOld->n_అధికప్రవాహం>0 ){
      ఒకవేళ( NEVER(హద్దు<pOld->aiOvfl[0]) ){
        rc = SQLITE_CORRUPT_పుట(pOld);
        వెళ్ళు balance_cleanup;
      }
      హద్దు = pOld->aiOvfl[0];
      కొరకు(j=0; j<హద్దు; j++){
        b.apCell[b.nCell+j] = a_దత్తాంశం + (ముసుగు_పుట & get2byteAligned(piCell));
        piCell += 2;
      }
      b.nCell += హద్దు;
      కొరకు(k=0; k<pOld->n_అధికప్రవాహం; k++){
        ప్రతిజ్ఞా( k==0 || pOld->aiOvfl[k-1]+1==pOld->aiOvfl[k] );/* NOTE 1 */
        b.apCell[b.nCell] = pOld->apOvfl[k];
        b.nCell++;
      }
    }
    pi_చివర = a_దత్తాంశం + pOld->cell_స్థానభ్రంశం + 2*pOld->nCell;
    /* Collect cell pointers before updating the cell count. */
    ppCell = &b.apCell[b.nCell];
    అయితే( piCell<pi_చివర ){
      ప్రతిజ్ఞా( ppCell<&b.apCell[nMaxCells] );
      *ppCell++ = a_దత్తాంశం + (ముసుగు_పుట & get2byteAligned(piCell));
      piCell += 2;
    }
    b.nCell = (పూర్ణాంకం)(ppCell - b.apCell);
    ప్రతిజ్ఞా( (b.nCell-n_Cell_At_ప్రారంభం)==(pOld->nCell+pOld->n_అధికప్రవాహం) );

    cntOld[i] = b.nCell;
    ఒకవేళ( i<nOld-1 && !ఆకు_దత్తాంశం){
      u16 sz = (u16)szNew[i];
      u8 *p_తాత్కాలిక;
      ప్రతిజ్ఞా( b.nCell<nMaxCells );
      b.szCell[b.nCell] = sz;
      p_తాత్కాలిక = &aSpace1[iSpace1];
      iSpace1 += sz;
      ప్రతిజ్ఞా( sz<=pBt->maxLocal+23 );
      ప్రతిజ్ఞా( iSpace1 <= (పూర్ణాంకం)pBt->పుట_కొలత );
      జ్ఞాపకం_నకలు(p_తాత్కాలిక, apDiv[i], sz);
      b.apCell[b.nCell] = p_తాత్కాలిక+ఆకు_Correction;
      ప్రతిజ్ఞా( ఆకు_Correction==0 || ఆకు_Correction==4 );
      b.szCell[b.nCell] = b.szCell[b.nCell] - ఆకు_Correction;
      ఒకవేళ( !pOld->ఆకు ){
        ప్రతిజ్ఞా( ఆకు_Correction==0 );
        ప్రతిజ్ఞా( pOld->hdr_స్థానభ్రంశం==0 || CORRUPT_DB );
        /* The right pointer of the child page pOld becomes the left
        ** pointer of the divider cell */
        జ్ఞాపకం_నకలు(b.apCell[b.nCell], &pOld->a_దత్తాంశం[8], 4);
      }లేకపోతే{
        ప్రతిజ్ఞా( ఆకు_Correction==4 );
        అయితే( b.szCell[b.nCell]<4 ){
          /* Do not allow any cells smaller than 4 bytes. If a smaller cell
          ** does exist, pad it with 0x00 bytes. */
          ప్రతిజ్ఞా( b.szCell[b.nCell]==3 || CORRUPT_DB );
          ప్రతిజ్ఞా( b.apCell[b.nCell]==&aSpace1[iSpace1-3] || CORRUPT_DB );
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
  usableSpace = pBt->usable_కొలత - 12 + ఆకు_Correction;
  కొరకు(i=k=0; i<nOld; i++, k++){
    Mem_పుట *p = apOld[i];
    b.ap_చివర[k] = p->a_దత్తాంశం_చివర;
    b.ixNx[k] = cntOld[i];
    ఒకవేళ( k && b.ixNx[k]==b.ixNx[k-1] ){
      k--;  /* Omit b.ixNx[] entry for child pages with no cells */
    }
    ఒకవేళ( !ఆకు_దత్తాంశం ){
      k++;
      b.ap_చివర[k] = p_మాతృ->a_దత్తాంశం_చివర;
      b.ixNx[k] = cntOld[i]+1;
    }
    ప్రతిజ్ఞా( p->nFree>=0 );
    szNew[i] = usableSpace - p->nFree;
    కొరకు(j=0; j<p->n_అధికప్రవాహం; j++){
      szNew[i] += 2 + p->x_Cell_కొలత(p, p->apOvfl[j]);
    }
    cntNew[i] = cntOld[i];
  }
  k = nOld;
  కొరకు(i=0; i<k; i++){
    పూర్ణాంకం sz;
    అయితే( szNew[i]>usableSpace ){
      ఒకవేళ( i+1>=k ){
        k = i+2;
        ఒకవేళ( k>NB+2 ){ rc = SQLITE_CORRUPT_BKPT; వెళ్ళు balance_cleanup; }
        szNew[k-1] = 0;
        cntNew[k-1] = b.nCell;
      }
      sz = 2 + cached_Cell_కొలత(&b, cntNew[i]-1);
      szNew[i] -= sz;
      ఒకవేళ( !ఆకు_దత్తాంశం ){
        ఒకవేళ( cntNew[i]<b.nCell ){
          sz = 2 + cached_Cell_కొలత(&b, cntNew[i]);
        }లేకపోతే{
          sz = 0;
        }
      }
      szNew[i+1] += sz;
      cntNew[i]--;
    }
    అయితే( cntNew[i]<b.nCell ){
      sz = 2 + cached_Cell_కొలత(&b, cntNew[i]);
      ఒకవేళ( szNew[i]+sz>usableSpace ) ఆపు;
      szNew[i] += sz;
      cntNew[i]++;
      ఒకవేళ( !ఆకు_దత్తాంశం ){
        ఒకవేళ( cntNew[i]<b.nCell ){
          sz = 2 + cached_Cell_కొలత(&b, cntNew[i]);
        }లేకపోతే{
          sz = 0;
        }
      }
      szNew[i+1] -= sz;
    }
    ఒకవేళ( cntNew[i]>=b.nCell ){
      k = i+1;
    }లేకపోతే ఒకవేళ( cntNew[i] <= (i>0 ? cntNew[i-1] : 0) ){
      rc = SQLITE_CORRUPT_BKPT;
      వెళ్ళు balance_cleanup;
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
  కొరకు(i=k-1; i>0; i--){
    పూర్ణాంకం sz_కుడి = szNew[i];  /* Size of sibling on the right */
    పూర్ణాంకం sz_ఎడమ = szNew[i-1]; /* Size of sibling on the left */
    పూర్ణాంకం r;              /* Index of right-most cell in left sibling */
    పూర్ణాంకం d;              /* Index of first cell to the left of right sibling */

    r = cntNew[i-1] - 1;
    d = r + 1 - ఆకు_దత్తాంశం;
    (శూన్య)cached_Cell_కొలత(&b, d);
    చేయి{
      పూర్ణాంకం szR, szD;
      ప్రతిజ్ఞా( d<nMaxCells );
      ప్రతిజ్ఞా( r<nMaxCells );
      szR = cached_Cell_కొలత(&b, r);
      szD = b.szCell[d];
      ఒకవేళ( sz_కుడి!=0
       && (bBulk || sz_కుడి+szD+2 > sz_ఎడమ-(szR+(i==k-1?0:2)))){
        ఆపు;
      }
      sz_కుడి += szD + 2;
      sz_ఎడమ -= szR + 2;
      cntNew[i-1] = r;
      r--;
      d--;
    }అయితే( r>=0 );
    szNew[i] = sz_కుడి;
    szNew[i-1] = sz_ఎడమ;
    ఒకవేళ( cntNew[i-1] <= (i>1 ? cntNew[i-2] : 0) ){
      rc = SQLITE_CORRUPT_BKPT;
      వెళ్ళు balance_cleanup;
    }
  }

  /* Sanity check:  For a non-corrupt database file one of the following
  ** must be true:
  **    (1) We found one or more cells (cntNew[0])>0), or
  **    (2) pPage is a virtual root page.  A virtual root page is when
  **        the real root page is page 1 and we are the only child of
  **        that page.
  */
  ప్రతిజ్ఞా( cntNew[0]>0 || (p_మాతృ->pgno==1 && p_మాతృ->nCell==0) || CORRUPT_DB);
  TRACE(("BALANCE: old: %u(nc=%u) %u(nc=%u) %u(nc=%u)\n",
    apOld[0]->pgno, apOld[0]->nCell,
    nOld>=2 ? apOld[1]->pgno : 0, nOld>=2 ? apOld[1]->nCell : 0,
    nOld>=3 ? apOld[2]->pgno : 0, nOld>=3 ? apOld[2]->nCell : 0
  ));

  /*
  ** Allocate k new pages.  Reuse old pages where possible.
  */
  పుట_Flags = apOld[0]->a_దత్తాంశం[0];
  కొరకు(i=0; i<k; i++){
    Mem_పుట *pNew;
    ఒకవేళ( i<nOld ){
      pNew = apNew[i] = apOld[i];
      apOld[i] = 0;
      rc = sqlite3PagerWrite(pNew->p_Db_పుట);
      nNew++;
      ఒకవేళ( sqlite3_Pager_పుట_Refcount(pNew->p_Db_పుట)!=1+(i==(i_మాతృ_Idx-nxDiv))
       && rc==SQLITE_OK
      ){
        rc = SQLITE_CORRUPT_BKPT;
      }
      ఒకవేళ( rc ) వెళ్ళు balance_cleanup;
    }లేకపోతే{
      ప్రతిజ్ఞా( i>0 );
      rc = allocate_Btree_పుట(pBt, &pNew, &pgno, (bBulk ? 1 : pgno), 0);
      ఒకవేళ( rc ) వెళ్ళు balance_cleanup;
      zero_పుట(pNew, పుట_Flags);
      apNew[i] = pNew;
      nNew++;
      cntOld[i] = b.nCell;

      /* Set the pointer-map entry for the new sibling page. */
      ఒకవేళ( ISAUTOVACUUM(pBt) ){
        ptrmapPut(pBt, pNew->pgno, PTRMAP_BTREE, p_మాతృ->pgno, &rc);
        ఒకవేళ( rc!=SQLITE_OK ){
          వెళ్ళు balance_cleanup;
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
  కొరకు(i=0; i<nNew; i++){
    aPgno[i] = apNew[i]->pgno;
    ప్రతిజ్ఞా( apNew[i]->p_Db_పుట->flags & PGHDR_WRITEABLE );
    ప్రతిజ్ఞా( apNew[i]->p_Db_పుట->flags & PGHDR_DIRTY );
  }
  కొరకు(i=0; i<nNew-1; i++){
    పూర్ణాంకం iB = i;
    కొరకు(j=i+1; j<nNew; j++){
      ఒకవేళ( apNew[j]->pgno < apNew[iB]->pgno ) iB = j;
    }

    /* If apNew[i] has a page number that is bigger than any of the
    ** subsequence apNew[i] entries, then swap apNew[i] with the subsequent
    ** entry that has the smallest page number (which we know to be
    ** entry apNew[iB]).
    */
    ఒకవేళ( iB!=i ){
      Pgno pgnoA = apNew[i]->pgno;
      Pgno pgnoB = apNew[iB]->pgno;
      Pgno pgno_తాత్కాలిక = (PENDING_BYTE/pBt->పుట_కొలత)+1;
      u16 fgA = apNew[i]->p_Db_పుట->flags;
      u16 fgB = apNew[iB]->p_Db_పుట->flags;
      sqlite3PagerRekey(apNew[i]->p_Db_పుట, pgno_తాత్కాలిక, fgB);
      sqlite3PagerRekey(apNew[iB]->p_Db_పుట, pgnoA, fgA);
      sqlite3PagerRekey(apNew[i]->p_Db_పుట, pgnoB, fgB);
      apNew[i]->pgno = pgnoB;
      apNew[iB]->pgno = pgnoA;
    }
  }

  TRACE(("BALANCE: new: %u(%u nc=%u) %u(%u nc=%u) %u(%u nc=%u) "
         "%u(%u nc=%u) %u(%u nc=%u)\n",
    apNew[0]->pgno, szNew[0], cntNew[0],
    nNew>=2 ? apNew[1]->pgno : 0, nNew>=2 ? szNew[1] : 0,
    nNew>=2 ? cntNew[1] - cntNew[0] - !ఆకు_దత్తాంశం : 0,
    nNew>=3 ? apNew[2]->pgno : 0, nNew>=3 ? szNew[2] : 0,
    nNew>=3 ? cntNew[2] - cntNew[1] - !ఆకు_దత్తాంశం : 0,
    nNew>=4 ? apNew[3]->pgno : 0, nNew>=4 ? szNew[3] : 0,
    nNew>=4 ? cntNew[3] - cntNew[2] - !ఆకు_దత్తాంశం : 0,
    nNew>=5 ? apNew[4]->pgno : 0, nNew>=5 ? szNew[4] : 0,
    nNew>=5 ? cntNew[4] - cntNew[3] - !ఆకు_దత్తాంశం : 0
  ));

  ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_మాతృ->p_Db_పుట) );
  ప్రతిజ్ఞా( nNew>=1 && nNew<=Array_కొలత(apNew) );
  ప్రతిజ్ఞా( apNew[nNew-1]!=0 );
  put4byte(p_కుడి, apNew[nNew-1]->pgno);

  /* If the sibling pages are not leaves, ensure that the right-child pointer
  ** of the right-most new sibling page is set to the value that was
  ** originally in the same field of the right-most old sibling page. */
  ఒకవేళ( (పుట_Flags & PTF_ఆకు)==0 && nOld!=nNew ){
    Mem_పుట *pOld;
    ఒకవేళ( nNew>nOld ){
      pOld = apNew[nOld-1];
    }లేకపోతే{
      pOld = apOld[nOld-1];
    }
    జ్ఞాపకం_నకలు(&apNew[nNew-1]->a_దత్తాంశం[8], &pOld->a_దత్తాంశం[8], 4);
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
  ఒకవేళ( ISAUTOVACUUM(pBt) ){
    Mem_పుట *pOld;
    Mem_పుట *pNew = pOld = apNew[0];
    పూర్ణాంకం cnt_Old_తదుపరి = pNew->nCell + pNew->n_అధికప్రవాహం;
    పూర్ణాంకం iNew = 0;
    పూర్ణాంకం iOld = 0;

    కొరకు(i=0; i<b.nCell; i++){
      u8 *pCell = b.apCell[i];
      అయితే( i==cnt_Old_తదుపరి ){
        iOld++;
        ప్రతిజ్ఞా( iOld<nNew || iOld<nOld );
        ప్రతిజ్ఞా( iOld>=0 && iOld<NB );
        pOld = iOld<nNew ? apNew[iOld] : apOld[iOld];
        cnt_Old_తదుపరి += pOld->nCell + pOld->n_అధికప్రవాహం + !ఆకు_దత్తాంశం;
      }
      ఒకవేళ( i==cntNew[iNew] ){
        pNew = apNew[++iNew];
        ఒకవేళ( !ఆకు_దత్తాంశం ) కొనసాగు;
      }

      /* Cell pCell is destined for new sibling page pNew. Originally, it
      ** was either part of sibling page iOld (possibly an overflow cell),
      ** or else the divider cell to the left of sibling page iOld. So,
      ** if sibling page iOld had the same page number as pNew, and if
      ** pCell really was a part of sibling page iOld (not a divider or
      ** overflow cell), we can skip updating the pointer map entries.  */
      ఒకవేళ( iOld>=nNew
       || pNew->pgno!=aPgno[iOld]
       || !SQLITE_WITHIN(pCell,pOld->a_దత్తాంశం,pOld->a_దత్తాంశం_చివర)
      ){
        ఒకవేళ( !ఆకు_Correction ){
          ptrmapPut(pBt, get4byte(pCell), PTRMAP_BTREE, pNew->pgno, &rc);
        }
        ఒకవేళ( cached_Cell_కొలత(&b,i)>pNew->minLocal ){
          ptrmapPutOvflPtr(pNew, pOld, pCell, &rc);
        }
        ఒకవేళ( rc ) వెళ్ళు balance_cleanup;
      }
    }
  }

  /* Insert new divider cells into pParent. */
  కొరకు(i=0; i<nNew-1; i++){
    u8 *pCell;
    u8 *p_తాత్కాలిక;
    పూర్ణాంకం sz;
    u8 *p_Src_చివర;
    Mem_పుట *pNew = apNew[i];
    j = cntNew[i];

    ప్రతిజ్ఞా( j<nMaxCells );
    ప్రతిజ్ఞా( b.apCell[j]!=0 );
    pCell = b.apCell[j];
    sz = b.szCell[j] + ఆకు_Correction;
    p_తాత్కాలిక = &aOvflSpace[iOvflSpace];
    ఒకవేళ( !pNew->ఆకు ){
      జ్ఞాపకం_నకలు(&pNew->a_దత్తాంశం[8], pCell, 4);
    }లేకపోతే ఒకవేళ( ఆకు_దత్తాంశం ){
      /* If the tree is a leaf-data tree, and the siblings are leaves,
      ** then there is no divider cell in b.apCell[]. Instead, the divider
      ** cell consists of the integer key for the right-most cell of
      ** the sibling-page assembled above only.
      */
      CellInfo info;
      j--;
      pNew->xParseCell(pNew, b.apCell[j], &info);
      pCell = p_తాత్కాలిక;
      sz = 4 + sqlite3PutVarint(&pCell[4], info.n_కీలకం);
      p_తాత్కాలిక = 0;
    }లేకపోతే{
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
      ఒకవేళ( b.szCell[j]==4 ){
        ప్రతిజ్ఞా(ఆకు_Correction==4);
        sz = p_మాతృ->x_Cell_కొలత(p_మాతృ, pCell);
      }
    }
    iOvflSpace += sz;
    ప్రతిజ్ఞా( sz<=pBt->maxLocal+23 );
    ప్రతిజ్ఞా( iOvflSpace <= (పూర్ణాంకం)pBt->పుట_కొలత );
    ప్రతిజ్ఞా( b.ixNx[NB*2-1]>j );
    కొరకు(k=0; b.ixNx[k]<=j; k++){}
    p_Src_చివర = b.ap_చివర[k];
    ఒకవేళ( SQLITE_అధికప్రవాహం(p_Src_చివర, pCell, pCell+sz) ){
      rc = SQLITE_CORRUPT_BKPT;
      వెళ్ళు balance_cleanup;
    }
    rc = చొప్పించు_Cell(p_మాతృ, nxDiv+i, pCell, sz, p_తాత్కాలిక, pNew->pgno);
    ఒకవేళ( rc!=SQLITE_OK ) వెళ్ళు balance_cleanup;
    ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_మాతృ->p_Db_పుట) );
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
  కొరకు(i=1-nNew; i<nNew; i++){
    పూర్ణాంకం iPg = i<0 ? -i : i;
    ప్రతిజ్ఞా( iPg>=0 && iPg<nNew );
    ప్రతిజ్ఞా( iPg>=1 || i>=0 );
    ప్రతిజ్ఞా( iPg<Array_కొలత(cntOld) );
    ఒకవేళ( ab_పూర్తి[iPg] ) కొనసాగు;         /* Skip pages already processed */
    ఒకవేళ( i>=0                            /* On the upwards pass, or... */
     || cntOld[iPg-1]>=cntNew[iPg-1]    /* Condition (1) is true */
    ){
      పూర్ణాంకం iNew;
      పూర్ణాంకం iOld;
      పూర్ణాంకం nNewCell;

      /* Verify condition (1):  If cells are moving left, update iPg
      ** only after iPg-1 has already been updated. */
      ప్రతిజ్ఞా( iPg==0 || cntOld[iPg-1]>=cntNew[iPg-1] || ab_పూర్తి[iPg-1] );

      /* Verify condition (2):  If cells are moving right, update iPg
      ** only after iPg+1 has already been updated. */
      ప్రతిజ్ఞా( cntNew[iPg]>=cntOld[iPg] || ab_పూర్తి[iPg+1] );

      ఒకవేళ( iPg==0 ){
        iNew = iOld = 0;
        nNewCell = cntNew[0];
      }లేకపోతే{
        iOld = iPg<nOld ? (cntOld[iPg-1] + !ఆకు_దత్తాంశం) : b.nCell;
        iNew = cntNew[iPg-1] + !ఆకు_దత్తాంశం;
        nNewCell = cntNew[iPg] - iNew;
      }

      rc = edit_పుట(apNew[iPg], iOld, iNew, nNewCell, &b);
      ఒకవేళ( rc ) వెళ్ళు balance_cleanup;
      ab_పూర్తి[iPg]++;
      apNew[iPg]->nFree = usableSpace-szNew[iPg];
      ప్రతిజ్ఞా( apNew[iPg]->n_అధికప్రవాహం==0 );
      ప్రతిజ్ఞా( apNew[iPg]->nCell==nNewCell );
    }
  }

  /* All pages have been processed exactly once */
  ప్రతిజ్ఞా( memcmp(ab_పూర్తి, "\01\01\01\01\01", nNew)==0 );

  ప్రతిజ్ఞా( nOld>0 );
  ప్రతిజ్ఞా( nNew>0 );

  ఒకవేళ( is_మూలం && p_మాతృ->nCell==0 && p_మాతృ->hdr_స్థానభ్రంశం<=apNew[0]->nFree ){
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
    ప్రతిజ్ఞా( nNew==1 || CORRUPT_DB );
    rc = defragment_పుట(apNew[0], -1);
    testcase( rc!=SQLITE_OK );
    ప్రతిజ్ఞా( apNew[0]->nFree ==
        (get2byteNotZero(&apNew[0]->a_దత్తాంశం[5]) - apNew[0]->cell_స్థానభ్రంశం
          - apNew[0]->nCell*2)
      || rc!=SQLITE_OK
    );
    copy_కణుపు_Content(apNew[0], p_మాతృ, &rc);
    free_పుట(apNew[0], &rc);
  }లేకపోతే ఒకవేళ( ISAUTOVACUUM(pBt) && !ఆకు_Correction ){
    /* Fix the pointer map entries associated with the right-child of each
    ** sibling page. All other pointer map entries have already been taken
    ** care of.  */
    కొరకు(i=0; i<nNew; i++){
      u32 కీలకం = get4byte(&apNew[i]->a_దత్తాంశం[8]);
      ptrmapPut(pBt, కీలకం, PTRMAP_BTREE, apNew[i]->pgno, &rc);
    }
  }

  ప్రతిజ్ఞా( p_మాతృ->isInit );
  TRACE(("BALANCE: finished: old=%u new=%u cells=%u\n",
          nOld, nNew, b.nCell));

  /* Free any old pages that were not reused as new pages.
  */
  కొరకు(i=nNew; i<nOld; i++){
    free_పుట(apOld[i], &rc);
  }

#ఒకవేళ 0
  ఒకవేళ( ISAUTOVACUUM(pBt) && rc==SQLITE_OK && apNew[0]->isInit ){
    /* The ptrmapCheckPages() contains assert() statements that verify that
    ** all pointer map pages are set correctly. This is helpful while
    ** debugging. This is usually disabled because a corrupt database may
    ** cause an assert() statement to fail.  */
    ptrmap_Check_పుటలు(apNew, nNew);
    ptrmap_Check_పుటలు(&p_మాతృ, 1);
  }
#అయితే_ముగింపు

  /*
  ** Cleanup before returning.
  */
balance_cleanup:
  sqlite3_దొంతర_Free(0, b.apCell);
  కొరకు(i=0; i<nOld; i++){
    release_పుట(apOld[i]);
  }
  కొరకు(i=0; i<nNew; i++){
    release_పుట(apNew[i]);
  }

  తిరిగి rc;
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
స్థిర పూర్ణాంకం balance_deeper(Mem_పుట *p_మూలం, Mem_పుట **pp_బిడ్డ){
  పూర్ణాంకం rc;                        /* Return value from subprocedures */
  Mem_పుట *p_బిడ్డ = 0;           /* Pointer to a new child page */
  Pgno pgno_బిడ్డ = 0;            /* Page number of the new child page */
  BtShared *pBt = p_మూలం->pBt;    /* The BTree */

  ప్రతిజ్ఞా( p_మూలం->n_అధికప్రవాహం>0 );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );

  /* Make pRoot, the root page of the b-tree, writable. Allocate a new
  ** page that will become the new right-child of pPage. Copy the contents
  ** of the node stored on pRoot into the new child page.
  */
  rc = sqlite3PagerWrite(p_మూలం->p_Db_పుట);
  ఒకవేళ( rc==SQLITE_OK ){
    rc = allocate_Btree_పుట(pBt,&p_బిడ్డ,&pgno_బిడ్డ,p_మూలం->pgno,0);
    copy_కణుపు_Content(p_మూలం, p_బిడ్డ, &rc);
    ఒకవేళ( ISAUTOVACUUM(pBt) ){
      ptrmapPut(pBt, pgno_బిడ్డ, PTRMAP_BTREE, p_మూలం->pgno, &rc);
    }
  }
  ఒకవేళ( rc ){
    *pp_బిడ్డ = 0;
    release_పుట(p_బిడ్డ);
    తిరిగి rc;
  }
  ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_బిడ్డ->p_Db_పుట) );
  ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_మూలం->p_Db_పుట) );
  ప్రతిజ్ఞా( p_బిడ్డ->nCell==p_మూలం->nCell || CORRUPT_DB );

  TRACE(("BALANCE: copy root %u into %u\n", p_మూలం->pgno, p_బిడ్డ->pgno));

  /* Copy the overflow cells from pRoot to pChild */
  జ్ఞాపకం_నకలు(p_బిడ్డ->aiOvfl, p_మూలం->aiOvfl,
         p_మూలం->n_అధికప్రవాహం*పరిమాణం(p_మూలం->aiOvfl[0]));
  జ్ఞాపకం_నకలు(p_బిడ్డ->apOvfl, p_మూలం->apOvfl,
         p_మూలం->n_అధికప్రవాహం*పరిమాణం(p_మూలం->apOvfl[0]));
  p_బిడ్డ->n_అధికప్రవాహం = p_మూలం->n_అధికప్రవాహం;

  /* Zero the contents of pRoot. Then install pChild as the right-child. */
  zero_పుట(p_మూలం, p_బిడ్డ->a_దత్తాంశం[0] & ~PTF_ఆకు);
  put4byte(&p_మూలం->a_దత్తాంశం[p_మూలం->hdr_స్థానభ్రంశం+8], pgno_బిడ్డ);

  *pp_బిడ్డ = p_బిడ్డ;
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం anotherValidCursor(BtCursor *pCur){
  BtCursor *pOther;
  కొరకు(pOther=pCur->pBt->pCursor; pOther; pOther=pOther->p_తదుపరి){
    ఒకవేళ( pOther!=pCur
     && pOther->eState==CURSOR_VALID
     && pOther->p_పుట==pCur->p_పుట
    ){
      తిరిగి SQLITE_CORRUPT_పుట(pCur->p_పుట);
    }
  }
  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం balance(BtCursor *pCur){
  పూర్ణాంకం rc = SQLITE_OK;
  u8 a_Balance_శీఘ్ర_Space[13];
  u8 *pFree = 0;

  VVA_ONLY( పూర్ణాంకం balance_శీఘ్ర_called = 0 );
  VVA_ONLY( పూర్ణాంకం balance_deeper_called = 0 );

  చేయి {
    పూర్ణాంకం i_పుట;
    Mem_పుట *p_పుట = pCur->p_పుట;

    ఒకవేళ( NEVER(p_పుట->nFree<0) && btreeComputeFreeSpace(p_పుట) ) ఆపు;
    ఒకవేళ( p_పుట->n_అధికప్రవాహం==0 && p_పుట->nFree*3<=(పూర్ణాంకం)pCur->pBt->usable_కొలత*2 ){
      /* No rebalance required as long as:
      **   (1) There are no overflow cells
      **   (2) The amount of free space on the page is less than 2/3rds of
      **       the total usable space on the page. */
      ఆపు;
    }లేకపోతే ఒకవేళ( (i_పుట = pCur->i_పుట)==0 ){
      ఒకవేళ( p_పుట->n_అధికప్రవాహం && (rc = anotherValidCursor(pCur))==SQLITE_OK ){
        /* The root page of the b-tree is overfull. In this case call the
        ** balance_deeper() function to create a new child for the root-page
        ** and copy the current contents of the root-page to it. The
        ** next iteration of the do-loop will balance the child page.
        */
        ప్రతిజ్ఞా( balance_deeper_called==0 );
        VVA_ONLY( balance_deeper_called++ );
        rc = balance_deeper(p_పుట, &pCur->ap_పుట[1]);
        ఒకవేళ( rc==SQLITE_OK ){
          pCur->i_పుట = 1;
          pCur->ix = 0;
          pCur->aiIdx[0] = 0;
          pCur->ap_పుట[0] = p_పుట;
          pCur->p_పుట = pCur->ap_పుట[1];
          ప్రతిజ్ఞా( pCur->p_పుట->n_అధికప్రవాహం );
        }
      }లేకపోతే{
        ఆపు;
      }
    }లేకపోతే ఒకవేళ( sqlite3_Pager_పుట_Refcount(p_పుట->p_Db_పుట)>1 ){
      /* The page being written is not a root page, and there is currently
      ** more than one reference to it. This only happens if the page is one
      ** of its own ancestor pages. Corruption. */
      rc = SQLITE_CORRUPT_పుట(p_పుట);
    }లేకపోతే{
      Mem_పుట * స్థిరరాశి p_మాతృ = pCur->ap_పుట[i_పుట-1];
      పూర్ణాంకం స్థిరరాశి iIdx = pCur->aiIdx[i_పుట-1];

      rc = sqlite3PagerWrite(p_మాతృ->p_Db_పుట);
      ఒకవేళ( rc==SQLITE_OK && p_మాతృ->nFree<0 ){
        rc = btreeComputeFreeSpace(p_మాతృ);
      }
      ఒకవేళ( rc==SQLITE_OK ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_QUICKBALANCE
        ఒకవేళ( p_పుట->int_కీలకం_ఆకు
         && p_పుట->n_అధికప్రవాహం==1
         && p_పుట->aiOvfl[0]==p_పుట->nCell
         && p_మాతృ->pgno!=1
         && p_మాతృ->nCell==iIdx
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
          ప్రతిజ్ఞా( balance_శీఘ్ర_called==0 );
          VVA_ONLY( balance_శీఘ్ర_called++ );
          rc = balance_శీఘ్ర(p_మాతృ, p_పుట, a_Balance_శీఘ్ర_Space);
        }లేకపోతే
#అయితే_ముగింపు
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
          u8 *pSpace = sqlite3_పుట_Malloc(pCur->pBt->పుట_కొలత);
          rc = balance_nonroot(p_మాతృ, iIdx, pSpace, i_పుట==1,
                               pCur->hints&BTREE_BULKLOAD);
          ఒకవేళ( pFree ){
            /* If pFree is not NULL, it points to the pSpace buffer used
            ** by a previous call to balance_nonroot(). Its contents are
            ** now stored either on real database pages or within the
            ** new pSpace buffer, so it may be safely freed here. */
            sqlite3_పుట_Free(pFree);
          }

          /* The pSpace buffer will be freed after the next call to
          ** balance_nonroot(), or just before this function returns, whichever
          ** comes first. */
          pFree = pSpace;
        }
      }

      p_పుట->n_అధికప్రవాహం = 0;

      /* The next iteration of the do-loop balances the parent page. */
      release_పుట(p_పుట);
      pCur->i_పుట--;
      ప్రతిజ్ఞా( pCur->i_పుట>=0 );
      pCur->p_పుట = pCur->ap_పుట[pCur->i_పుట];
    }
  }అయితే( rc==SQLITE_OK );

  ఒకవేళ( pFree ){
    sqlite3_పుట_Free(pFree);
  }
  తిరిగి rc;
}

/* Overwrite content from pX into pDest.  Only do the write if the
** content is different from what is already there.
*/
స్థిర పూర్ణాంకం btreeOverwriteContent(
  Mem_పుట *p_పుట,           /* MemPage on which writing will occur */
  u8 *pDest,                /* Pointer to the place to start writing */
  స్థిరరాశి BtreePayload *pX,   /* Source of data to write */
  పూర్ణాంకం i_స్థానభ్రంశం,              /* Offset of first byte to write */
  పూర్ణాంకం iAmt                  /* Number of bytes to be written */
){
  పూర్ణాంకం n_దత్తాంశం = pX->n_దత్తాంశం - i_స్థానభ్రంశం;
  ఒకవేళ( n_దత్తాంశం<=0 ){
    /* Overwriting with zeros */
    పూర్ణాంకం i;
    కొరకు(i=0; i<iAmt && pDest[i]==0; i++){}
    ఒకవేళ( i<iAmt ){
      పూర్ణాంకం rc = sqlite3PagerWrite(p_పుట->p_Db_పుట);
      ఒకవేళ( rc ) తిరిగి rc;
      memset(pDest + i, 0, iAmt - i);
    }
  }లేకపోతే{
    ఒకవేళ( n_దత్తాంశం<iAmt ){
      /* Mixed read data and zeros at the end.  Make a recursive call
      ** to write the zeros then fall through to write the real data */
      పూర్ణాంకం rc = btreeOverwriteContent(p_పుట, pDest+n_దత్తాంశం, pX, i_స్థానభ్రంశం+n_దత్తాంశం,
                                 iAmt-n_దత్తాంశం);
      ఒకవేళ( rc ) తిరిగి rc;
      iAmt = n_దత్తాంశం;
    }
    ఒకవేళ( memcmp(pDest, ((u8*)pX->p_దత్తాంశం) + i_స్థానభ్రంశం, iAmt)!=0 ){
      పూర్ణాంకం rc = sqlite3PagerWrite(p_పుట->p_Db_పుట);
      ఒకవేళ( rc ) తిరిగి rc;
      /* In a corrupt database, it is possible for the source and destination
      ** buffers to overlap.  This is harmless since the database is already
      ** corrupt but it does cause valgrind and ASAN warnings.  So use
      ** memmove(). */
      memmove(pDest, ((u8*)pX->p_దత్తాంశం) + i_స్థానభ్రంశం, iAmt);
    }
  }
  తిరిగి SQLITE_OK;
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.  In this variant, pCur is pointing to an overflow
** cell.
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం btree_Overwrite_అధికప్రవాహం_Cell(
  BtCursor *pCur,                     /* Cursor pointing to cell to overwrite */
  స్థిరరాశి BtreePayload *pX              /* Content to write into the cell */
){
  పూర్ణాంకం i_స్థానభ్రంశం;                        /* Next byte of pX->pData to write */
  పూర్ణాంకం n_మొత్తం = pX->n_దత్తాంశం + pX->nZero; /* Total bytes of to write */
  పూర్ణాంకం rc;                             /* Return code */
  Mem_పుట *p_పుట = pCur->p_పుట;       /* Page being written */
  BtShared *pBt;                      /* Btree */
  Pgno ovflPgno;                      /* Next overflow page to write */
  u32 ovfl_పుట_కొలత;                   /* Size to write on overflow page */

  ప్రతిజ్ఞా( pCur->info.nLocal<n_మొత్తం );  /* pCur is an overflow cell */

  /* Overwrite the local portion first */
  rc = btreeOverwriteContent(p_పుట, pCur->info.pPayload, pX,
                             0, pCur->info.nLocal);
  ఒకవేళ( rc ) తిరిగి rc;

  /* Now overwrite the overflow pages */
  i_స్థానభ్రంశం = pCur->info.nLocal;
  ప్రతిజ్ఞా( n_మొత్తం>=0 );
  ప్రతిజ్ఞా( i_స్థానభ్రంశం>=0 );
  ovflPgno = get4byte(pCur->info.pPayload + i_స్థానభ్రంశం);
  pBt = p_పుట->pBt;
  ovfl_పుట_కొలత = pBt->usable_కొలత - 4;
  చేయి{
    rc = btree_Get_పుట(pBt, ovflPgno, &p_పుట, 0);
    ఒకవేళ( rc ) తిరిగి rc;
    ఒకవేళ( sqlite3_Pager_పుట_Refcount(p_పుట->p_Db_పుట)!=1 || p_పుట->isInit ){
      rc = SQLITE_CORRUPT_పుట(p_పుట);
    }లేకపోతే{
      ఒకవేళ( i_స్థానభ్రంశం+ovfl_పుట_కొలత<(u32)n_మొత్తం ){
        ovflPgno = get4byte(p_పుట->a_దత్తాంశం);
      }లేకపోతే{
        ovfl_పుట_కొలత = n_మొత్తం - i_స్థానభ్రంశం;
      }
      rc = btreeOverwriteContent(p_పుట, p_పుట->a_దత్తాంశం+4, pX,
                                 i_స్థానభ్రంశం, ovfl_పుట_కొలత);
    }
    sqlite3PagerUnref(p_పుట->p_Db_పుట);
    ఒకవేళ( rc ) తిరిగి rc;
    i_స్థానభ్రంశం += ovfl_పుట_కొలత;
  }అయితే( i_స్థానభ్రంశం<n_మొత్తం );
  తిరిగి SQLITE_OK;   
}

/*
** Overwrite the cell that cursor pCur is pointing to with fresh content
** contained in pX.
*/
స్థిర పూర్ణాంకం btreeOverwriteCell(BtCursor *pCur, స్థిరరాశి BtreePayload *pX){
  పూర్ణాంకం n_మొత్తం = pX->n_దత్తాంశం + pX->nZero; /* Total bytes of to write */
  Mem_పుట *p_పుట = pCur->p_పుట;       /* Page being written */

  ఒకవేళ( pCur->info.pPayload + pCur->info.nLocal > p_పుట->a_దత్తాంశం_చివర
   || pCur->info.pPayload < p_పుట->a_దత్తాంశం + p_పుట->cell_స్థానభ్రంశం
  ){
    తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
  }
  ఒకవేళ( pCur->info.nLocal==n_మొత్తం ){
    /* The entire cell is local */
    తిరిగి btreeOverwriteContent(p_పుట, pCur->info.pPayload, pX,
                                 0, pCur->info.nLocal);
  }లేకపోతే{
    /* The cell contains overflow content */
    తిరిగి btree_Overwrite_అధికప్రవాహం_Cell(pCur, pX);
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
పూర్ణాంకం sqlite3_Btree_చొప్పించు(
  BtCursor *pCur,                /* Insert data into the table of this cursor */
  స్థిరరాశి BtreePayload *pX,        /* Content of the row to be inserted */
  పూర్ణాంకం flags,                     /* True if this is likely an append */
  పూర్ణాంకం seek_ఫలితం                 /* Result of prior IndexMoveto() call */
){
  పూర్ణాంకం rc;
  పూర్ణాంకం loc = seek_ఫలితం;          /* -1: before desired location  +1: after */
  పూర్ణాంకం szNew = 0;
  పూర్ణాంకం idx;
  Mem_పుట *p_పుట;
  Btree *p = pCur->pBtree;
  గుర్తులేని అక్షరం *oldCell;
  గుర్తులేని అక్షరం *newCell = 0;

  ప్రతిజ్ఞా( (flags & (BTREE_SAVEPOSITION|BTREE_APPEND|BTREE_PREFORMAT))==flags );
  ప్రతిజ్ఞా( (flags & BTREE_PREFORMAT)==0 || seek_ఫలితం || pCur->p_కీలకం_Info==0 );

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
  ఒకవేళ( pCur->curFlags & BTCF_గుణిజం ){
    rc = saveAllCursors(p->pBt, pCur->pgno_మూలం, pCur);
    ఒకవేళ( rc ) తిరిగి rc;
    ఒకవేళ( loc && pCur->i_పుట<0 ){
      /* This can only happen if the schema is corrupt such that there is more
      ** than one table or index with the same root page as used by the cursor.
      ** Which can only happen if the SQLITE_NoSchemaError flag was set when
      ** the schema was loaded. This cannot be asserted though, as a user might
      ** set the flag, load the schema, and then unset the flag.  */
      తిరిగి SQLITE_CORRUPT_PGNO(pCur->pgno_మూలం);
    }
  }

  /* Ensure that the cursor is not in the CURSOR_FAULT state and that it
  ** points to a valid cell.
  */
  ఒకవేళ( pCur->eState>=CURSOR_REQUIRESEEK ){
    testcase( pCur->eState==CURSOR_REQUIRESEEK );
    testcase( pCur->eState==CURSOR_FAULT );
    rc = move_గమ్యం_మూలం(pCur);
    ఒకవేళ( rc && rc!=SQLITE_ఖాళీలు ) తిరిగి rc;
  }

  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( (pCur->curFlags & BTCF_WriteFlag)!=0
              && p->pBt->లోపల_Transaction==TRANS_WRITE
              && (p->pBt->btsFlags & BTS_READ_ONLY)==0 );
  ప్రతిజ్ఞా( has_Shared_Cache_పట్టిక_తాళం(p, pCur->pgno_మూలం, pCur->p_కీలకం_Info!=0, 2) );

  /* Assert that the caller has been consistent. If this cursor was opened
  ** expecting an index b-tree, then the caller should be inserting blob
  ** keys with no associated data. If the cursor was opened expecting an
  ** intkey table, the caller should be inserting integer keys with a
  ** blob of associated data.  */
  ప్రతిజ్ఞా( (flags & BTREE_PREFORMAT) || (pX->p_కీలకం==0)==(pCur->p_కీలకం_Info==0) );

  ఒకవేళ( pCur->p_కీలకం_Info==0 ){
    ప్రతిజ్ఞా( pX->p_కీలకం==0 );
    /* If this is an insert into a table b-tree, invalidate any incrblob
    ** cursors open on the row being replaced */
    ఒకవేళ( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, pCur->pgno_మూలం, pX->n_కీలకం, 0);
    }

    /* If BTREE_SAVEPOSITION is set, the cursor must already be pointing
    ** to a row with the same key as the new entry being inserted.
    */
#నిర్వచితమైతే SQLITE_DEBUG
    ఒకవేళ( flags & BTREE_SAVEPOSITION ){
      ప్రతిజ్ఞా( pCur->curFlags & BTCF_ValidNKey );
      ప్రతిజ్ఞా( pX->n_కీలకం==pCur->info.n_కీలకం );
      ప్రతిజ్ఞా( loc==0 );
    }
#అయితే_ముగింపు

    /* On the other hand, BTREE_SAVEPOSITION==0 does not imply
    ** that the cursor is not pointing to a row to be overwritten.
    ** So do a complete check.
    */
    ఒకవేళ( (pCur->curFlags&BTCF_ValidNKey)!=0 && pX->n_కీలకం==pCur->info.n_కీలకం ){
      /* The cursor is pointing to the entry that is to be
      ** overwritten */
      ప్రతిజ్ఞా( pX->n_దత్తాంశం>=0 && pX->nZero>=0 );
      ఒకవేళ( pCur->info.n_కొలత!=0
       && pCur->info.nPayload==(u32)pX->n_దత్తాంశం+pX->nZero
      ){
        /* New entry is the same size as the old.  Do an overwrite */
        తిరిగి btreeOverwriteCell(pCur, pX);
      }
      ప్రతిజ్ఞా( loc==0 );
    }లేకపోతే ఒకవేళ( loc==0 ){
      /* The cursor is *not* pointing to the cell to be overwritten, nor
      ** to an adjacent cell.  Move the cursor so that it is pointing either
      ** to the cell to be overwritten or an adjacent cell.
      */
      rc = sqlite3_Btree_పట్టిక_Moveto(pCur, pX->n_కీలకం,
               (flags & BTREE_APPEND)!=0, &loc);
      ఒకవేళ( rc ) తిరిగి rc;
    }
  }లేకపోతే{
    /* This is an index or a WITHOUT ROWID table */

    /* If BTREE_SAVEPOSITION is set, the cursor must already be pointing
    ** to a row with the same key as the new entry being inserted.
    */
    ప్రతిజ్ఞా( (flags & BTREE_SAVEPOSITION)==0 || loc==0 );

    /* If the cursor is not already pointing either to the cell to be
    ** overwritten, or if a new cell is being inserted, if the cursor is
    ** not pointing to an immediately adjacent cell, then move the cursor
    ** so that it does.
    */
    ఒకవేళ( loc==0 && (flags & BTREE_SAVEPOSITION)==0 ){
      ఒకవేళ( pX->nMem ){
        Unpacked_రికార్డు r;
        r.p_కీలకం_Info = pCur->p_కీలకం_Info;
        r.aMem = pX->aMem;
        r.n_క్షేత్రం = pX->nMem;
        r.default_rc = 0;
        r.eq_చూసిన = 0;
        rc = sqlite3_Btree_సూచీ_Moveto(pCur, &r, &loc);
      }లేకపోతే{
        rc = btreeMoveto(pCur, pX->p_కీలకం, pX->n_కీలకం,
                    (flags & BTREE_APPEND)!=0, &loc);
      }
      ఒకవేళ( rc ) తిరిగి rc;
    }

    /* If the cursor is currently pointing to an entry to be overwritten
    ** and the new content is the same as as the old, then use the
    ** overwrite optimization.
    */
    ఒకవేళ( loc==0 ){
      getCellInfo(pCur);
      ఒకవేళ( pCur->info.n_కీలకం==pX->n_కీలకం ){
        BtreePayload x2;
        x2.p_దత్తాంశం = pX->p_కీలకం;
        x2.n_దత్తాంశం = (పూర్ణాంకం)pX->n_కీలకం;  ప్రతిజ్ఞా( pX->n_కీలకం<=0x7fffffff );
        x2.nZero = 0;
        తిరిగి btreeOverwriteCell(pCur, &x2);
      }
    }
  }
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID
       || (pCur->eState==CURSOR_INVALID && loc) || CORRUPT_DB );

  p_పుట = pCur->p_పుట;
  ప్రతిజ్ఞా( p_పుట->int_కీలకం || pX->n_కీలకం>=0 || (flags & BTREE_PREFORMAT) );
  ప్రతిజ్ఞా( p_పుట->ఆకు || !p_పుట->int_కీలకం );
  ఒకవేళ( p_పుట->nFree<0 ){
    ఒకవేళ( NEVER(pCur->eState>CURSOR_INVALID) ){
     /* ^^^^^--- due to the moveToRoot() call above */
      rc = SQLITE_CORRUPT_పుట(p_పుట);
    }లేకపోతే{
      rc = btreeComputeFreeSpace(p_పుట);
    }
    ఒకవేళ( rc ) తిరిగి rc;
  }

  TRACE(("INSERT: table=%u nkey=%lld ndata=%u page=%u %s\n",
          pCur->pgno_మూలం, pX->n_కీలకం, pX->n_దత్తాంశం, p_పుట->pgno,
          loc==0 ? "overwrite" : "new entry"));
  ప్రతిజ్ఞా( p_పుట->isInit || CORRUPT_DB );
  newCell = p->pBt->pTmpSpace;
  ప్రతిజ్ఞా( newCell!=0 );
  ప్రతిజ్ఞా( BTREE_PREFORMAT==OPFLAG_PREFORMAT );
  ఒకవేళ( flags & BTREE_PREFORMAT ){
    rc = SQLITE_OK;
    szNew = p->pBt->n_Preformat_కొలత;
    ఒకవేళ( szNew<4 ){
      szNew = 4;
      newCell[3] = 0;
    }
    ఒకవేళ( ISAUTOVACUUM(p->pBt) && szNew>p_పుట->maxLocal ){
      CellInfo info;
      p_పుట->xParseCell(p_పుట, newCell, &info);
      ఒకవేళ( info.nPayload!=info.nLocal ){
        Pgno ovfl = get4byte(&newCell[szNew-4]);
        ptrmapPut(p->pBt, ovfl, PTRMAP_OVERFLOW1, p_పుట->pgno, &rc);
        ఒకవేళ( NEVER(rc) ) వెళ్ళు చివర_చొప్పించు;
      }
    }
  }లేకపోతే{
    rc = fill_లోపల_Cell(p_పుట, newCell, pX, &szNew);
    ఒకవేళ( rc ) వెళ్ళు చివర_చొప్పించు;
  }
  ప్రతిజ్ఞా( szNew==p_పుట->x_Cell_కొలత(p_పుట, newCell) );
  ప్రతిజ్ఞా( szNew <= MX_CELL_కొలత(p->pBt) );
  idx = pCur->ix;
  pCur->info.n_కొలత = 0;
  ఒకవేళ( loc==0 ){
    CellInfo info;
    ప్రతిజ్ఞా( idx>=0 );
    ఒకవేళ( idx>=p_పుట->nCell ){
      తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
    }
    rc = sqlite3PagerWrite(p_పుట->p_Db_పుట);
    ఒకవేళ( rc ){
      వెళ్ళు చివర_చొప్పించు;
    }
    oldCell = కనుగొను_Cell(p_పుట, idx);
    ఒకవేళ( !p_పుట->ఆకు ){
      జ్ఞాపకం_నకలు(newCell, oldCell, 4);
    }
    BTREE_CLEAR_CELL(rc, p_పుట, oldCell, info);
    testcase( pCur->curFlags & BTCF_ValidOvfl );
    invalidate_అధికప్రవాహం_Cache(pCur);
    ఒకవేళ( info.n_కొలత==szNew && info.nLocal==info.nPayload
     && (!ISAUTOVACUUM(p->pBt) || szNew<p_పుట->minLocal)
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
      ప్రతిజ్ఞా( rc==SQLITE_OK ); /* clearCell never fails when nLocal==nPayload */
      ఒకవేళ( oldCell < p_పుట->a_దత్తాంశం+p_పుట->hdr_స్థానభ్రంశం+10 ){
        తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
      }
      ఒకవేళ( oldCell+szNew > p_పుట->a_దత్తాంశం_చివర ){
        తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
      }
      జ్ఞాపకం_నకలు(oldCell, newCell, szNew);
      తిరిగి SQLITE_OK;
    }
    dropCell(p_పుట, idx, info.n_కొలత, &rc);
    ఒకవేళ( rc ) వెళ్ళు చివర_చొప్పించు;
  }లేకపోతే ఒకవేళ( loc<0 && p_పుట->nCell>0 ){
    ప్రతిజ్ఞా( p_పుట->ఆకు );
    idx = ++pCur->ix;
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
  }లేకపోతే{
    ప్రతిజ్ఞా( p_పుట->ఆకు );
  }
  rc = చొప్పించు_Cell_Fast(p_పుట, idx, newCell, szNew);
  ప్రతిజ్ఞా( p_పుట->n_అధికప్రవాహం==0 || rc==SQLITE_OK );
  ప్రతిజ్ఞా( rc!=SQLITE_OK || p_పుట->nCell>0 || p_పుట->n_అధికప్రవాహం>0 );

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
  ఒకవేళ( p_పుట->n_అధికప్రవాహం ){
    ప్రతిజ్ఞా( rc==SQLITE_OK );
    pCur->curFlags &= ~(BTCF_ValidNKey|BTCF_ValidOvfl);
    rc = balance(pCur);

    /* Must make sure nOverflow is reset to zero even if the balance()
    ** fails. Internal data structure corruption will result otherwise.
    ** Also, set the cursor state to invalid. This stops saveCursorPosition()
    ** from trying to save the current position of the cursor.  */
    pCur->p_పుట->n_అధికప్రవాహం = 0;
    pCur->eState = CURSOR_INVALID;
    ఒకవేళ( (flags & BTREE_SAVEPOSITION) && rc==SQLITE_OK ){
      btree_Release_All_Cursor_పుటలు(pCur);
      ఒకవేళ( pCur->p_కీలకం_Info ){
        ప్రతిజ్ఞా( pCur->p_కీలకం==0 );
        pCur->p_కీలకం = sqlite3Malloc( pX->n_కీలకం );
        ఒకవేళ( pCur->p_కీలకం==0 ){
          rc = SQLITE_NOMEM;
        }లేకపోతే{
          జ్ఞాపకం_నకలు(pCur->p_కీలకం, pX->p_కీలకం, pX->n_కీలకం);
        }
      }
      pCur->eState = CURSOR_REQUIRESEEK;
      pCur->n_కీలకం = pX->n_కీలకం;
    }
  }
  ప్రతిజ్ఞా( pCur->i_పుట<0 || pCur->p_పుట->n_అధికప్రవాహం==0 );

చివర_చొప్పించు:
  తిరిగి rc;
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
పూర్ణాంకం sqlite3_Btree_Transfer_అడ్డువరుస(BtCursor *pDest, BtCursor *pSrc, i64 i_కీలకం){
  BtShared *pBt = pDest->pBt;
  u8 *a_బహిర్గతం = pBt->pTmpSpace;    /* Pointer to next output buffer */
  స్థిరరాశి u8 *a_లోపల;                /* Pointer to next input buffer */
  u32 n_లోపల;                      /* Size of input buffer aIn[] */
  u32 nRem;                     /* Bytes of data still to copy */

  getCellInfo(pSrc);
  ఒకవేళ( pSrc->info.nPayload<0x80 ){
    *(a_బహిర్గతం++) = (u8)pSrc->info.nPayload;
  }లేకపోతే{
    a_బహిర్గతం += sqlite3PutVarint(a_బహిర్గతం, pSrc->info.nPayload);
  }
  ఒకవేళ( pDest->p_కీలకం_Info==0 ) a_బహిర్గతం += sqlite3PutVarint(a_బహిర్గతం, i_కీలకం);
  n_లోపల = pSrc->info.nLocal;
  a_లోపల = pSrc->info.pPayload;
  ఒకవేళ( a_లోపల+n_లోపల>pSrc->p_పుట->a_దత్తాంశం_చివర ){
    తిరిగి SQLITE_CORRUPT_పుట(pSrc->p_పుట);
  }
  nRem = pSrc->info.nPayload;
  ఒకవేళ( n_లోపల==nRem && n_లోపల<pDest->p_పుట->maxLocal ){
    జ్ఞాపకం_నకలు(a_బహిర్గతం, a_లోపల, n_లోపల);
    pBt->n_Preformat_కొలత = n_లోపల + (పూర్ణాంకం)(a_బహిర్గతం - pBt->pTmpSpace);
    తిరిగి SQLITE_OK;
  }లేకపోతే{
    పూర్ణాంకం rc = SQLITE_OK;
    Pager *pSrcPager = pSrc->pBt->pPager;
    u8 *p_Pgno_బహిర్గతం = 0;
    Pgno ovfl_లోపల = 0;
    Db_పుట *p_పుట_లోపల = 0;
    Mem_పుట *p_పుట_బహిర్గతం = 0;
    u32 n_బహిర్గతం;                     /* Size of output buffer aOut[] */

    n_బహిర్గతం = btree_Payload_గమ్యం_Local(pDest->p_పుట, pSrc->info.nPayload);
    pBt->n_Preformat_కొలత = (పూర్ణాంకం)n_బహిర్గతం + (పూర్ణాంకం)(a_బహిర్గతం - pBt->pTmpSpace);
    ఒకవేళ( n_బహిర్గతం<pSrc->info.nPayload ){
      p_Pgno_బహిర్గతం = &a_బహిర్గతం[n_బహిర్గతం];
      pBt->n_Preformat_కొలత += 4;
    }
 
    ఒకవేళ( nRem>n_లోపల ){
      ఒకవేళ( a_లోపల+n_లోపల+4>pSrc->p_పుట->a_దత్తాంశం_చివర ){
        తిరిగి SQLITE_CORRUPT_పుట(pSrc->p_పుట);
      }
      ovfl_లోపల = get4byte(&pSrc->info.pPayload[n_లోపల]);
    }
 
    చేయి {
      nRem -= n_బహిర్గతం;
      చేయి{
        ప్రతిజ్ఞా( n_బహిర్గతం>0 );
        ఒకవేళ( n_లోపల>0 ){
          పూర్ణాంకం nCopy = MIN(n_బహిర్గతం, n_లోపల);
          జ్ఞాపకం_నకలు(a_బహిర్గతం, a_లోపల, nCopy);
          n_బహిర్గతం -= nCopy;
          n_లోపల -= nCopy;
          a_బహిర్గతం += nCopy;
          a_లోపల += nCopy;
        }
        ఒకవేళ( n_బహిర్గతం>0 ){
          sqlite3PagerUnref(p_పుట_లోపల);
          p_పుట_లోపల = 0;
          rc = sqlite3PagerGet(pSrcPager, ovfl_లోపల, &p_పుట_లోపల, PAGER_GET_READONLY);
          ఒకవేళ( rc==SQLITE_OK ){
            a_లోపల = (స్థిరరాశి u8*)sqlite3_Pager_Get_దత్తాంశం(p_పుట_లోపల);
            ovfl_లోపల = get4byte(a_లోపల);
            a_లోపల += 4;
            n_లోపల = pSrc->pBt->usable_కొలత - 4;
          }
        }
      }అయితే( rc==SQLITE_OK && n_బహిర్గతం>0 );
 
      ఒకవేళ( rc==SQLITE_OK && nRem>0 && ALWAYS(p_Pgno_బహిర్గతం) ){
        Pgno pgnoNew = 0;  /* Prevent harmless static-analyzer warning */
        Mem_పుట *pNew = 0;
        rc = allocate_Btree_పుట(pBt, &pNew, &pgnoNew, 0, 0);
        put4byte(p_Pgno_బహిర్గతం, pgnoNew);
        ఒకవేళ( ISAUTOVACUUM(pBt) && p_పుట_బహిర్గతం ){
          ptrmapPut(pBt, pgnoNew, PTRMAP_OVERFLOW2, p_పుట_బహిర్గతం->pgno, &rc);
        }
        release_పుట(p_పుట_బహిర్గతం);
        p_పుట_బహిర్గతం = pNew;
        ఒకవేళ( p_పుట_బహిర్గతం ){
          p_Pgno_బహిర్గతం = p_పుట_బహిర్గతం->a_దత్తాంశం;
          put4byte(p_Pgno_బహిర్గతం, 0);
          a_బహిర్గతం = &p_Pgno_బహిర్గతం[4];
          n_బహిర్గతం = MIN(pBt->usable_కొలత - 4, nRem);
        }
      }
    }అయితే( nRem>0 && rc==SQLITE_OK );
 
    release_పుట(p_పుట_బహిర్గతం);
    sqlite3PagerUnref(p_పుట_లోపల);
    తిరిగి rc;
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
పూర్ణాంకం sqlite3BtreeDelete(BtCursor *pCur, u8 flags){
  Btree *p = pCur->pBtree;
  BtShared *pBt = p->pBt;             
  పూర్ణాంకం rc;                    /* Return code */
  Mem_పుట *p_పుట;            /* Page to delete cell from */
  గుర్తులేని అక్షరం *pCell;      /* Pointer to cell to delete */
  పూర్ణాంకం iCellIdx;              /* Index of cell to delete */
  పూర్ణాంకం i_Cell_లోతు;            /* Depth of node containing pCell */
  CellInfo info;             /* Size of the cell being deleted */
  u8 bPreserve;              /* Keep cursor valid.  2 for CURSOR_SKIPNEXT */

  ప్రతిజ్ఞా( cursorOwnsBtShared(pCur) );
  ప్రతిజ్ఞా( pBt->లోపల_Transaction==TRANS_WRITE );
  ప్రతిజ్ఞా( (pBt->btsFlags & BTS_READ_ONLY)==0 );
  ప్రతిజ్ఞా( pCur->curFlags & BTCF_WriteFlag );
  ప్రతిజ్ఞా( has_Shared_Cache_పట్టిక_తాళం(p, pCur->pgno_మూలం, pCur->p_కీలకం_Info!=0, 2) );
  ప్రతిజ్ఞా( !hasReadConflicts(p, pCur->pgno_మూలం) );
  ప్రతిజ్ఞా( (flags & ~(BTREE_SAVEPOSITION | BTREE_AUXDELETE))==0 );
  ఒకవేళ( pCur->eState!=CURSOR_VALID ){
    ఒకవేళ( pCur->eState>=CURSOR_REQUIRESEEK ){
      rc = btreeRestoreCursorPosition(pCur);
      ప్రతిజ్ఞా( rc!=SQLITE_OK || CORRUPT_DB || pCur->eState==CURSOR_VALID );
      ఒకవేళ( rc || pCur->eState!=CURSOR_VALID ) తిరిగి rc;
    }లేకపోతే{
      తిరిగి SQLITE_CORRUPT_PGNO(pCur->pgno_మూలం);
    }
  }
  ప్రతిజ్ఞా( pCur->eState==CURSOR_VALID );

  i_Cell_లోతు = pCur->i_పుట;
  iCellIdx = pCur->ix;
  p_పుట = pCur->p_పుట;
  ఒకవేళ( p_పుట->nCell<=iCellIdx ){
    తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
  }
  pCell = కనుగొను_Cell(p_పుట, iCellIdx);
  ఒకవేళ( p_పుట->nFree<0 && btreeComputeFreeSpace(p_పుట) ){
    తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
  }
  ఒకవేళ( pCell<&p_పుట->aCellIdx[p_పుట->nCell] ){
    తిరిగి SQLITE_CORRUPT_పుట(p_పుట);
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
  ఒకవేళ( bPreserve ){
    ఒకవేళ( !p_పుట->ఆకు
     || (p_పుట->nFree+p_పుట->x_Cell_కొలత(p_పుట,pCell)+2) >
                                                   (పూర్ణాంకం)(pBt->usable_కొలత*2/3)
     || p_పుట->nCell==1  /* See dbfuzz001.test for a test case */
    ){
      /* A b-tree rebalance will be required after deleting this entry.
      ** Save the cursor key.  */
      rc = save_Cursor_కీలకం(pCur);
      ఒకవేళ( rc ) తిరిగి rc;
    }లేకపోతే{
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
  ఒకవేళ( !p_పుట->ఆకు ){
    rc = sqlite3_Btree_మునుపటి(pCur, 0);
    ప్రతిజ్ఞా( rc!=SQLITE_పూర్తి );
    ఒకవేళ( rc ) తిరిగి rc;
  }

  /* Save the positions of any other cursors open on this table before
  ** making any modifications.  */
  ఒకవేళ( pCur->curFlags & BTCF_గుణిజం ){
    rc = saveAllCursors(pBt, pCur->pgno_మూలం, pCur);
    ఒకవేళ( rc ) తిరిగి rc;
  }

  /* If this is a delete operation to remove a row from a table b-tree,
  ** invalidate any incrblob cursors open on the row being deleted.  */
  ఒకవేళ( pCur->p_కీలకం_Info==0 && p->hasIncrblobCur ){
    invalidateIncrblobCursors(p, pCur->pgno_మూలం, pCur->info.n_కీలకం, 0);
  }

  /* Make the page containing the entry to be deleted writable. Then free any
  ** overflow pages associated with the entry and finally remove the cell
  ** itself from within the page.  */
  rc = sqlite3PagerWrite(p_పుట->p_Db_పుట);
  ఒకవేళ( rc ) తిరిగి rc;
  BTREE_CLEAR_CELL(rc, p_పుట, pCell, info);
  dropCell(p_పుట, iCellIdx, info.n_కొలత, &rc);
  ఒకవేళ( rc ) తిరిగి rc;

  /* If the cell deleted was not located on a leaf page, then the cursor
  ** is currently pointing to the largest entry in the sub-tree headed
  ** by the child-page of the cell that was just deleted from an internal
  ** node. The cell from the leaf node needs to be moved to the internal
  ** node to replace the deleted cell.  */
  ఒకవేళ( !p_పుట->ఆకు ){
    Mem_పుట *p_ఆకు = pCur->p_పుట;
    పూర్ణాంకం nCell;
    Pgno n;
    గుర్తులేని అక్షరం *pTmp;

    ఒకవేళ( p_ఆకు->nFree<0 ){
      rc = btreeComputeFreeSpace(p_ఆకు);
      ఒకవేళ( rc ) తిరిగి rc;
    }
    ఒకవేళ( i_Cell_లోతు<pCur->i_పుట-1 ){
      n = pCur->ap_పుట[i_Cell_లోతు+1]->pgno;
    }లేకపోతే{
      n = pCur->p_పుట->pgno;
    }
    pCell = కనుగొను_Cell(p_ఆకు, p_ఆకు->nCell-1);
    ఒకవేళ( pCell<&p_ఆకు->a_దత్తాంశం[4] ) తిరిగి SQLITE_CORRUPT_పుట(p_ఆకు);
    nCell = p_ఆకు->x_Cell_కొలత(p_ఆకు, pCell);
    ప్రతిజ్ఞా( MX_CELL_కొలత(pBt) >= nCell );
    pTmp = pBt->pTmpSpace;
    ప్రతిజ్ఞా( pTmp!=0 );
    rc = sqlite3PagerWrite(p_ఆకు->p_Db_పుట);
    ఒకవేళ( rc==SQLITE_OK ){
      rc = చొప్పించు_Cell(p_పుట, iCellIdx, pCell-4, nCell+4-(pCell[0]==2), pTmp, n);
    }
    dropCell(p_ఆకు, p_ఆకు->nCell-1, nCell, &rc);
    ఒకవేళ( rc ) తిరిగి rc;
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
  ప్రతిజ్ఞా( pCur->p_పుట->n_అధికప్రవాహం==0 );
  ప్రతిజ్ఞా( pCur->p_పుట->nFree>=0 );
  ఒకవేళ( pCur->p_పుట->nFree*3<=(పూర్ణాంకం)pCur->pBt->usable_కొలత*2 ){
    /* Optimization: If the free space is less than 2/3rds of the page,
    ** then balance() will always be a no-op.  No need to invoke it. */
    rc = SQLITE_OK;
  }లేకపోతే{
    rc = balance(pCur);
  }
  ఒకవేళ( rc==SQLITE_OK && pCur->i_పుట>i_Cell_లోతు ){
    release_పుట_Not_Null(pCur->p_పుట);
    pCur->i_పుట--;
    అయితే( pCur->i_పుట>i_Cell_లోతు ){
      release_పుట(pCur->ap_పుట[pCur->i_పుట--]);
    }
    pCur->p_పుట = pCur->ap_పుట[pCur->i_పుట];
    rc = balance(pCur);
  }

  ఒకవేళ( rc==SQLITE_OK ){
    ఒకవేళ( bPreserve>1 ){
      ప్రతిజ్ఞా( (pCur->i_పుట==i_Cell_లోతు || CORRUPT_DB) );
      ప్రతిజ్ఞా( p_పుట==pCur->p_పుట || CORRUPT_DB );
      ప్రతిజ్ఞా( (p_పుట->nCell>0 || CORRUPT_DB) && iCellIdx<=p_పుట->nCell );
      pCur->eState = CURSOR_SKIPNEXT;
      ఒకవేళ( iCellIdx>=p_పుట->nCell ){
        pCur->skip_తదుపరి = -1;
        pCur->ix = p_పుట->nCell-1;
      }లేకపోతే{
        pCur->skip_తదుపరి = 1;
      }
      /* The pCur->info structure is currently populated with the details
      ** of the cell that was just removed. But pCur->ix now points to a
      ** different cell in the b-tree. So zero info.nSize here so that if
      ** saveCursorPosition() is called on this cursor it does not trust
      ** the contents of pCur->info.  */
      pCur->info.n_కొలత = 0;
    }లేకపోతే{
      rc = move_గమ్యం_మూలం(pCur);
      ఒకవేళ( bPreserve ){
        btree_Release_All_Cursor_పుటలు(pCur);
        pCur->eState = CURSOR_REQUIRESEEK;
      }
      ఒకవేళ( rc==SQLITE_ఖాళీలు ) rc = SQLITE_OK;
    }
  }
  తిరిగి rc;
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
స్థిర పూర్ణాంకం btree_Create_పట్టిక(Btree *p, Pgno *pi_పట్టిక, పూర్ణాంకం createTabFlags){
  BtShared *pBt = p->pBt;
  Mem_పుట *p_మూలం;
  Pgno pgno_మూలం;
  పూర్ణాంకం rc;
  పూర్ణాంకం ptfFlags;          /* Page-type flags for the root page of new table */

  ప్రతిజ్ఞా( sqlite3BtreeHoldsMutex(p) );
  ప్రతిజ్ఞా( pBt->లోపల_Transaction==TRANS_WRITE );
  ప్రతిజ్ఞా( (pBt->btsFlags & BTS_READ_ONLY)==0 );

#నిర్వచితమైతే SQLITE_OMIT_AUTOVACUUM
  rc = allocate_Btree_పుట(pBt, &p_మూలం, &pgno_మూలం, 1, 0);
  ఒకవేళ( rc ){
    తిరిగి rc;
  }
#లేకపోతే
  ఒకవేళ( pBt->autoVacuum ){
    Pgno pgnoMove;      /* Move a page here to make room for the root-page */
    Mem_పుట *p_పుట_Move; /* The page to move to. */

    /* Creating a new table may probably require moving an existing database
    ** to make room for the new tables root page. In case this page turns
    ** out to be an overflow page, delete all overflow page-map caches
    ** held by open cursors.
    */
    invalidate_All_అధికప్రవాహం_Cache(pBt);

    /* Read the value of meta[3] from the database to determine where the
    ** root page of the new table should go. meta[3] is the largest root-page
    ** created so far, so the new root-page is (meta[3]+1).
    */
    sqlite3BtreeGetMeta(p, BTREE_అతిపెద్ద_మూలం_పుట, &pgno_మూలం);
    ఒకవేళ( pgno_మూలం>btreePagecount(pBt) ){
      తిరిగి SQLITE_CORRUPT_PGNO(pgno_మూలం);
    }
    pgno_మూలం++;

    /* The new root-page may not be allocated on a pointer-map page, or the
    ** PENDING_BYTE page.
    */
    అయితే( pgno_మూలం==PTRMAP_PAGENO(pBt, pgno_మూలం) ||
        pgno_మూలం==PENDING_BYTE_పుట(pBt) ){
      pgno_మూలం++;
    }
    ప్రతిజ్ఞా( pgno_మూలం>=3 );

    /* Allocate a page. The page that currently resides at pgnoRoot will
    ** be moved to the allocated page (unless the allocated page happens
    ** to reside at pgnoRoot).
    */
    rc = allocate_Btree_పుట(pBt, &p_పుట_Move, &pgnoMove, pgno_మూలం, BTALLOC_EXACT);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }

    ఒకవేళ( pgnoMove!=pgno_మూలం ){
      /* pgnoRoot is the page that will be used for the root-page of
      ** the new table (assuming an error did not occur). But we were
      ** allocated pgnoMove. If required (i.e. if it was not allocated
      ** by extending the file), the current page at position pgnoMove
      ** is already journaled.
      */
      u8 eType = 0;
      Pgno i_Ptr_పుట = 0;

      /* Save the positions of any open cursors. This is required in
      ** case they are holding a reference to an xFetch reference
      ** corresponding to page pgnoRoot.  */
      rc = saveAllCursors(pBt, 0, 0);
      release_పుట(p_పుట_Move);
      ఒకవేళ( rc!=SQLITE_OK ){
        తిరిగి rc;
      }

      /* Move the page currently at pgnoRoot to pgnoMove. */
      rc = btree_Get_పుట(pBt, pgno_మూలం, &p_మూలం, 0);
      ఒకవేళ( rc!=SQLITE_OK ){
        తిరిగి rc;
      }
      rc = ptrmapGet(pBt, pgno_మూలం, &eType, &i_Ptr_పుట);
      ఒకవేళ( eType==PTRMAP_ROOTPAGE || eType==PTRMAP_FREEPAGE ){
        rc = SQLITE_CORRUPT_PGNO(pgno_మూలం);
      }
      ఒకవేళ( rc!=SQLITE_OK ){
        release_పుట(p_మూలం);
        తిరిగి rc;
      }
      ప్రతిజ్ఞా( eType!=PTRMAP_ROOTPAGE );
      ప్రతిజ్ఞా( eType!=PTRMAP_FREEPAGE );
      rc = relocate_పుట(pBt, p_మూలం, eType, i_Ptr_పుట, pgnoMove, 0);
      release_పుట(p_మూలం);

      /* Obtain the page at pgnoRoot */
      ఒకవేళ( rc!=SQLITE_OK ){
        తిరిగి rc;
      }
      rc = btree_Get_పుట(pBt, pgno_మూలం, &p_మూలం, 0);
      ఒకవేళ( rc!=SQLITE_OK ){
        తిరిగి rc;
      }
      rc = sqlite3PagerWrite(p_మూలం->p_Db_పుట);
      ఒకవేళ( rc!=SQLITE_OK ){
        release_పుట(p_మూలం);
        తిరిగి rc;
      }
    }లేకపోతే{
      p_మూలం = p_పుట_Move;
    }

    /* Update the pointer-map and meta-data with the new root-page number. */
    ptrmapPut(pBt, pgno_మూలం, PTRMAP_ROOTPAGE, 0, &rc);
    ఒకవేళ( rc ){
      release_పుట(p_మూలం);
      తిరిగి rc;
    }

    /* When the new root page was allocated, page 1 was made writable in
    ** order either to increase the database filesize, or to decrement the
    ** freelist count.  Hence, the sqlite3BtreeUpdateMeta() call cannot fail.
    */
    ప్రతిజ్ఞా( sqlite3PagerIswriteable(pBt->pPage1->p_Db_పుట) );
    rc = sqlite3BtreeUpdateMeta(p, 4, pgno_మూలం);
    ఒకవేళ( NEVER(rc) ){
      release_పుట(p_మూలం);
      తిరిగి rc;
    }

  }లేకపోతే{
    rc = allocate_Btree_పుట(pBt, &p_మూలం, &pgno_మూలం, 1, 0);
    ఒకవేళ( rc ) తిరిగి rc;
  }
#అయితే_ముగింపు
  ప్రతిజ్ఞా( sqlite3PagerIswriteable(p_మూలం->p_Db_పుట) );
  ఒకవేళ( createTabFlags & BTREE_INTKEY ){
    ptfFlags = PTF_INTKEY | PTF_LEAFDATA | PTF_ఆకు;
  }లేకపోతే{
    ptfFlags = PTF_ZERODATA | PTF_ఆకు;
  }
  zero_పుట(p_మూలం, ptfFlags);
  sqlite3PagerUnref(p_మూలం->p_Db_పుట);
  ప్రతిజ్ఞా( (pBt->openFlags & BTREE_SINGLE)==0 || pgno_మూలం==2 );
  *pi_పట్టిక = pgno_మూలం;
  తిరిగి SQLITE_OK;
}
పూర్ణాంకం sqlite3_Btree_Create_పట్టిక(Btree *p, Pgno *pi_పట్టిక, పూర్ణాంకం flags){
  పూర్ణాంకం rc;
  sqlite3BtreeEnter(p);
  rc = btree_Create_పట్టిక(p, pi_పట్టిక, flags);
  sqlite3BtreeLeave(p);
  తిరిగి rc;
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
స్థిర పూర్ణాంకం clear_Database_పుట(
  BtShared *pBt,           /* The BTree that contains the table */
  Pgno pgno,               /* Page number to clear */
  పూర్ణాంకం free_పుట_Flag,        /* bit 0: Deallocate page.  Bits 1-31: depth */
  i64 *pnChange            /* Add number of Cells freed to this counter */
){
  Mem_పుట *p_పుట;
  పూర్ణాంకం rc;
  గుర్తులేని అక్షరం *pCell;
  పూర్ణాంకం i;
  పూర్ణాంకం hdr;
  CellInfo info;

  ప్రతిజ్ఞా( sqlite3_mutex_held(pBt->mutex) );
  ఒకవేళ( pgno>btreePagecount(pBt) ){
    తిరిగి SQLITE_CORRUPT_PGNO(pgno);
  }
  ఒకవేళ( (free_పుట_Flag>>1) > BTCURSOR_MAX_లోతు ){
    తిరిగి SQLITE_CORRUPT_PGNO(pgno);
  }
  rc = get_And_Init_పుట(pBt, pgno, &p_పుట, 0);
  ఒకవేళ( rc ) తిరిగి rc;
  ఒకవేళ( (pBt->openFlags & BTREE_SINGLE)==0
   && sqlite3_Pager_పుట_Refcount(p_పుట->p_Db_పుట) != (1 + (pgno==1))
  ){
    rc = SQLITE_CORRUPT_పుట(p_పుట);
    వెళ్ళు cleardatabasepage_బహిర్గతం;
  }
  hdr = p_పుట->hdr_స్థానభ్రంశం;
  కొరకు(i=0; i<p_పుట->nCell; i++){
    pCell = కనుగొను_Cell(p_పుట, i);
    ఒకవేళ( !p_పుట->ఆకు ){
      rc = clear_Database_పుట(pBt, get4byte(pCell),
                             (free_పుట_Flag+2)|1, pnChange);
      ఒకవేళ( rc ) వెళ్ళు cleardatabasepage_బహిర్గతం;
    }
    BTREE_CLEAR_CELL(rc, p_పుట, pCell, info);
    ఒకవేళ( rc ) వెళ్ళు cleardatabasepage_బహిర్గతం;
  }
  ఒకవేళ( !p_పుట->ఆకు ){
    rc = clear_Database_పుట(pBt, get4byte(&p_పుట->a_దత్తాంశం[hdr+8]), 
                           (free_పుట_Flag+2)|1, pnChange);
    ఒకవేళ( rc ) వెళ్ళు cleardatabasepage_బహిర్గతం;
    ఒకవేళ( p_పుట->int_కీలకం ) pnChange = 0;
  }
  ఒకవేళ( pnChange ){
    testcase( !p_పుట->int_కీలకం );
    *pnChange += p_పుట->nCell;
  }
  ఒకవేళ( (free_పుట_Flag&1)!=0 ){
    free_పుట(p_పుట, &rc);
  }లేకపోతే ఒకవేళ( (rc = sqlite3PagerWrite(p_పుట->p_Db_పుట))==0 ){
    zero_పుట(p_పుట, p_పుట->a_దత్తాంశం[hdr] | PTF_ఆకు);
  }

cleardatabasepage_బహిర్గతం:
  release_పుట(p_పుట);
  తిరిగి rc;
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
పూర్ణాంకం sqlite3_Btree_Clear_పట్టిక(Btree *p, పూర్ణాంకం i_పట్టిక, i64 *pnChange){
  పూర్ణాంకం rc;
  BtShared *pBt = p->pBt;
  sqlite3BtreeEnter(p);
  ప్రతిజ్ఞా( p->లోపల_Trans==TRANS_WRITE );

  rc = saveAllCursors(pBt, (Pgno)i_పట్టిక, 0);

  ఒకవేళ( SQLITE_OK==rc ){
    /* Invalidate all incrblob cursors open on table iTable (assuming iTable
    ** is the root of a table b-tree - if it is not, the following call is
    ** a no-op).  */
    ఒకవేళ( p->hasIncrblobCur ){
      invalidateIncrblobCursors(p, (Pgno)i_పట్టిక, 0, 1);
    }
    rc = clear_Database_పుట(pBt, (Pgno)i_పట్టిక, 0, pnChange);
  }
  sqlite3BtreeLeave(p);
  తిరిగి rc;
}

/*
** Delete all information from the single table that pCur is open on.
**
** This routine only work for pCur on an ephemeral table.
*/
పూర్ణాంకం sqlite3_Btree_Clear_పట్టిక_యొక్క_Cursor(BtCursor *pCur){
  తిరిగి sqlite3_Btree_Clear_పట్టిక(pCur->pBtree, pCur->pgno_మూలం, 0);
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
స్థిర పూర్ణాంకం btree_Drop_పట్టిక(Btree *p, Pgno i_పట్టిక, పూర్ణాంకం *piMoved){
  పూర్ణాంకం rc;
  Mem_పుట *p_పుట = 0;
  BtShared *pBt = p->pBt;

  ప్రతిజ్ఞా( sqlite3BtreeHoldsMutex(p) );
  ప్రతిజ్ఞా( p->లోపల_Trans==TRANS_WRITE );
  ప్రతిజ్ఞా( i_పట్టిక>=2 );
  ఒకవేళ( i_పట్టిక>btreePagecount(pBt) ){
    తిరిగి SQLITE_CORRUPT_PGNO(i_పట్టిక);
  }

  rc = sqlite3_Btree_Clear_పట్టిక(p, i_పట్టిక, 0);
  ఒకవేళ( rc ) తిరిగి rc;
  rc = btree_Get_పుట(pBt, (Pgno)i_పట్టిక, &p_పుట, 0);
  ఒకవేళ( NEVER(rc) ){
    release_పుట(p_పుట);
    తిరిగి rc;
  }

  *piMoved = 0;

#నిర్వచితమైతే SQLITE_OMIT_AUTOVACUUM
  free_పుట(p_పుట, &rc);
  release_పుట(p_పుట);
#లేకపోతే
  ఒకవేళ( pBt->autoVacuum ){
    Pgno max_మూలం_Pgno;
    sqlite3BtreeGetMeta(p, BTREE_అతిపెద్ద_మూలం_పుట, &max_మూలం_Pgno);

    ఒకవేళ( i_పట్టిక==max_మూలం_Pgno ){
      /* If the table being dropped is the table with the largest root-page
      ** number in the database, put the root page on the free list.
      */
      free_పుట(p_పుట, &rc);
      release_పుట(p_పుట);
      ఒకవేళ( rc!=SQLITE_OK ){
        తిరిగి rc;
      }
    }లేకపోతే{
      /* The table being dropped does not have the largest root-page
      ** number in the database. So move the page that does into the
      ** gap left by the deleted root-page.
      */
      Mem_పుట *pMove;
      release_పుట(p_పుట);
      rc = btree_Get_పుట(pBt, max_మూలం_Pgno, &pMove, 0);
      ఒకవేళ( rc!=SQLITE_OK ){
        తిరిగి rc;
      }
      rc = relocate_పుట(pBt, pMove, PTRMAP_ROOTPAGE, 0, i_పట్టిక, 0);
      release_పుట(pMove);
      ఒకవేళ( rc!=SQLITE_OK ){
        తిరిగి rc;
      }
      pMove = 0;
      rc = btree_Get_పుట(pBt, max_మూలం_Pgno, &pMove, 0);
      ఒకవేళ( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pMove->p_Db_పుట);
      }
      free_పుట(pMove, &rc);
      release_పుట(pMove);
      ఒకవేళ( rc!=SQLITE_OK ){
        తిరిగి rc;
      }
      *piMoved = max_మూలం_Pgno;
    }

    /* Set the new 'max-root-page' value in the database header. This
    ** is the old value less one, less one more if that happens to
    ** be a root-page number, less one again if that is the
    ** PENDING_BYTE_PAGE.
    */
    max_మూలం_Pgno--;
    అయితే( max_మూలం_Pgno==PENDING_BYTE_పుట(pBt)
           || PTRMAP_ISPAGE(pBt, max_మూలం_Pgno) ){
      max_మూలం_Pgno--;
    }
    ప్రతిజ్ఞా( max_మూలం_Pgno!=PENDING_BYTE_పుట(pBt) );

    rc = sqlite3BtreeUpdateMeta(p, 4, max_మూలం_Pgno);
  }లేకపోతే{
    free_పుట(p_పుట, &rc);
    release_పుట(p_పుట);
  }
#అయితే_ముగింపు
  తిరిగి rc; 
}
పూర్ణాంకం sqlite3_Btree_Drop_పట్టిక(Btree *p, పూర్ణాంకం i_పట్టిక, పూర్ణాంకం *piMoved){
  పూర్ణాంకం rc;
  sqlite3BtreeEnter(p);
  rc = btree_Drop_పట్టిక(p, i_పట్టిక, piMoved);
  sqlite3BtreeLeave(p);
  తిరిగి rc;
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
శూన్య sqlite3BtreeGetMeta(Btree *p, పూర్ణాంకం idx, u32 *pMeta){
  BtShared *pBt = p->pBt;

  sqlite3BtreeEnter(p);
  ప్రతిజ్ఞా( p->లోపల_Trans>TRANS_NONE );
  ప్రతిజ్ఞా( SQLITE_OK==query_Shared_Cache_పట్టిక_తాళం(p, SCHEMA_మూలం, READ_తాళం) );
  ప్రతిజ్ఞా( pBt->pPage1 );
  ప్రతిజ్ఞా( idx>=0 && idx<=15 );

  ఒకవేళ( idx==BTREE_దత్తాంశం_సంస్కరణ ){
    *pMeta = sqlite3_Pager_దత్తాంశం_సంస్కరణ(pBt->pPager) + p->i_BData_సంస్కరణ;
  }లేకపోతే{
    *pMeta = get4byte(&pBt->pPage1->a_దత్తాంశం[36 + idx*4]);
  }

  /* If auto-vacuum is disabled in this build and this is an auto-vacuum
  ** database, mark the database as read-only.  */
#నిర్వచితమైతే SQLITE_OMIT_AUTOVACUUM
  ఒకవేళ( idx==BTREE_అతిపెద్ద_మూలం_పుట && *pMeta>0 ){
    pBt->btsFlags |= BTS_READ_ONLY;
  }
#అయితే_ముగింపు

  sqlite3BtreeLeave(p);
}

/*
** Write meta-information back into the database.  Meta[0] is
** read-only and may not be written.
*/
పూర్ణాంకం sqlite3BtreeUpdateMeta(Btree *p, పూర్ణాంకం idx, u32 iMeta){
  BtShared *pBt = p->pBt;
  గుర్తులేని అక్షరం *pP1;
  పూర్ణాంకం rc;
  ప్రతిజ్ఞా( idx>=1 && idx<=15 );
  sqlite3BtreeEnter(p);
  ప్రతిజ్ఞా( p->లోపల_Trans==TRANS_WRITE );
  ప్రతిజ్ఞా( pBt->pPage1!=0 );
  pP1 = pBt->pPage1->a_దత్తాంశం;
  rc = sqlite3PagerWrite(pBt->pPage1->p_Db_పుట);
  ఒకవేళ( rc==SQLITE_OK ){
    put4byte(&pP1[36 + idx*4], iMeta);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
    ఒకవేళ( idx==BTREE_INCR_VACUUM ){
      ప్రతిజ్ఞా( pBt->autoVacuum || iMeta==0 );
      ప్రతిజ్ఞా( iMeta==0 || iMeta==1 );
      pBt->incrVacuum = (u8)iMeta;
    }
#అయితే_ముగింపు
  }
  sqlite3BtreeLeave(p);
  తిరిగి rc;
}

/*
** The first argument, pCur, is a cursor opened on some b-tree. Count the
** number of entries in the b-tree and write the result to *pnEntry.
**
** SQLITE_OK is returned if the operation is successfully executed.
** Otherwise, if an error is encountered (i.e. an IO error or database
** corruption) an SQLite error code is returned.
*/
పూర్ణాంకం sqlite3_Btree_లెక్క(sqlite3 *db, BtCursor *pCur, i64 *pn_నమోదు){
  i64 n_నమోదు = 0;                      /* Value to return in *pnEntry */
  పూర్ణాంకం rc;                              /* Return code */

  rc = move_గమ్యం_మూలం(pCur);
  ఒకవేళ( rc==SQLITE_ఖాళీలు ){
    *pn_నమోదు = 0;
    తిరిగి SQLITE_OK;
  }

  /* Unless an error occurs, the following loop runs one iteration for each
  ** page in the B-Tree structure (not including overflow pages).
  */
  అయితే( rc==SQLITE_OK && !Atomic_ఎక్కించు(&db->u1.isInterrupted) ){
    పూర్ణాంకం iIdx;                          /* Index of child node in parent */
    Mem_పుట *p_పుట;                    /* Current page of the b-tree */

    /* If this is a leaf page or the tree is not an int-key tree, then
    ** this page contains countable entries. Increment the entry counter
    ** accordingly.
    */
    p_పుట = pCur->p_పుట;
    ఒకవేళ( p_పుట->ఆకు || !p_పుట->int_కీలకం ){
      n_నమోదు += p_పుట->nCell;
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
    ఒకవేళ( p_పుట->ఆకు ){
      చేయి {
        ఒకవేళ( pCur->i_పుట==0 ){
          /* All pages of the b-tree have been visited. Return successfully. */
          *pn_నమోదు = n_నమోదు;
          తిరిగి move_గమ్యం_మూలం(pCur);
        }
        move_గమ్యం_మాతృ(pCur);
      }అయితే ( pCur->ix>=pCur->p_పుట->nCell );

      pCur->ix++;
      p_పుట = pCur->p_పుట;
    }

    /* Descend to the child node of the cell that the cursor currently
    ** points at. This is the right-child if (iIdx==pPage->nCell).
    */
    iIdx = pCur->ix;
    ఒకవేళ( iIdx==p_పుట->nCell ){
      rc = move_గమ్యం_బిడ్డ(pCur, get4byte(&p_పుట->a_దత్తాంశం[p_పుట->hdr_స్థానభ్రంశం+8]));
    }లేకపోతే{
      rc = move_గమ్యం_బిడ్డ(pCur, get4byte(కనుగొను_Cell(p_పుట, iIdx)));
    }
  }

  /* An error has occurred. Return an error code. */
  తిరిగి rc;
}

/*
** Return the pager associated with a BTree.  This routine is used for
** testing and debugging only.
*/
Pager *sqlite3BtreePager(Btree *p){
  తిరిగి p->pBt->pPager;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_INTEGRITY_CHECK
/*
** Record an OOM error during integrity_check
*/
స్థిర శూన్య checkOom(IntegrityCk *pCheck){
  pCheck->rc = SQLITE_NOMEM;
  pCheck->mxErr = 0;  /* Causes integrity_check processing to stop */
  ఒకవేళ( pCheck->nErr==0 ) pCheck->nErr++;
}

/*
** Invoke the progress handler, if appropriate.  Also check for an
** interrupt.
*/
స్థిర శూన్య check_పురోగతి(IntegrityCk *pCheck){
  sqlite3 *db = pCheck->db;
  ఒకవేళ( Atomic_ఎక్కించు(&db->u1.isInterrupted) ){
    pCheck->rc = SQLITE_అంతరాయం;
    pCheck->nErr++;
    pCheck->mxErr = 0;
  }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_పురోగతి_CALLBACK
  ఒకవేళ( db->x_పురోగతి ){
    ప్రతిజ్ఞా( db->n_పురోగతి_Ops>0 );
    pCheck->n_అడుగు++;
    ఒకవేళ( (pCheck->n_అడుగు % db->n_పురోగతి_Ops)==0
     && db->x_పురోగతి(db->p_పురోగతి_Arg)
    ){
      pCheck->rc = SQLITE_అంతరాయం;
      pCheck->nErr++;
      pCheck->mxErr = 0;
    }
  }
#అయితే_ముగింపు
}

/*
** Append a message to the error message string.
*/
స్థిర శూన్య check_Append_సందేశం(
  IntegrityCk *pCheck,
  స్థిరరాశి అక్షరం *zFormat,
  ...
){
  va_list ap;
  check_పురోగతి(pCheck);
  ఒకవేళ( !pCheck->mxErr ) తిరిగి;
  pCheck->mxErr--;
  pCheck->nErr++;
  va_start(ap, zFormat);
  ఒకవేళ( pCheck->err_సందేశం.nChar ){
    sqlite3_str_append(&pCheck->err_సందేశం, "\n", 1);
  }
  ఒకవేళ( pCheck->zPfx ){
    sqlite3_str_appendf(&pCheck->err_సందేశం, pCheck->zPfx,
                        pCheck->v0, pCheck->v1, pCheck->v2);
  }
  sqlite3_str_vappendf(&pCheck->err_సందేశం, zFormat, ap);
  va_end(ap);
  ఒకవేళ( pCheck->err_సందేశం.accError==SQLITE_NOMEM ){
    checkOom(pCheck);
  }
}
#అయితే_ముగింపు /* SQLITE_OMIT_INTEGRITY_CHECK */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_INTEGRITY_CHECK

/*
** Return non-zero if the bit in the IntegrityCk.aPgRef[] array that
** corresponds to page iPg is already set.
*/
స్థిర పూర్ణాంకం get_పుట_Referenced(IntegrityCk *pCheck, Pgno iPg){
  ప్రతిజ్ఞా( pCheck->aPgRef!=0 );
  ప్రతిజ్ఞా( iPg<=pCheck->n_Ck_పుట && పరిమాణం(pCheck->aPgRef[0])==1 );
  తిరిగి (pCheck->aPgRef[iPg/8] & (1 << (iPg & 0x07)));
}

/*
** Set the bit in the IntegrityCk.aPgRef[] array that corresponds to page iPg.
*/
స్థిర శూన్య సమితి_పుట_Referenced(IntegrityCk *pCheck, Pgno iPg){
  ప్రతిజ్ఞా( pCheck->aPgRef!=0 );
  ప్రతిజ్ఞా( iPg<=pCheck->n_Ck_పుట && పరిమాణం(pCheck->aPgRef[0])==1 );
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
స్థిర పూర్ణాంకం checkRef(IntegrityCk *pCheck, Pgno i_పుట){
  ఒకవేళ( i_పుట>pCheck->n_Ck_పుట || i_పుట==0 ){
    check_Append_సందేశం(pCheck, "invalid page number %u", i_పుట);
    తిరిగి 1;
  }
  ఒకవేళ( get_పుట_Referenced(pCheck, i_పుట) ){
    check_Append_సందేశం(pCheck, "2nd reference to page %u", i_పుట);
    తిరిగి 1;
  }
  సమితి_పుట_Referenced(pCheck, i_పుట);
  తిరిగి 0;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
/*
** Check that the entry in the pointer-map for page iChild maps to
** page iParent, pointer type ptrType. If not, append an error message
** to pCheck.
*/
స్థిర శూన్య checkPtrmap(
  IntegrityCk *pCheck,   /* Integrity check context */
  Pgno i_బిడ్డ,           /* Child page number */
  u8 eType,              /* Expected pointer map type */
  Pgno i_మాతృ           /* Expected pointer map parent page number */
){
  పూర్ణాంకం rc;
  u8 ePtrmapType;
  Pgno i_Ptrmap_మాతృ;

  rc = ptrmapGet(pCheck->pBt, i_బిడ్డ, &ePtrmapType, &i_Ptrmap_మాతృ);
  ఒకవేళ( rc!=SQLITE_OK ){
    ఒకవేళ( rc==SQLITE_NOMEM || rc==SQLITE_IOERR_NOMEM ) checkOom(pCheck);
    check_Append_సందేశం(pCheck, "Failed to read ptrmap key=%u", i_బిడ్డ);
    తిరిగి;
  }

  ఒకవేళ( ePtrmapType!=eType || i_Ptrmap_మాతృ!=i_మాతృ ){
    check_Append_సందేశం(pCheck,
      "Bad ptr map entry key=%u expected=(%u,%u) got=(%u,%u)",
      i_బిడ్డ, eType, i_మాతృ, ePtrmapType, i_Ptrmap_మాతృ);
  }
}
#అయితే_ముగింపు

/*
** Check the integrity of the freelist or of an overflow page list.
** Verify that the number of pages on the list is N.
*/
స్థిర శూన్య checkList(
  IntegrityCk *pCheck,  /* Integrity checking context */
  పూర్ణాంకం isFreeList,       /* True for a freelist.  False for overflow page list */
  Pgno i_పుట,           /* Page number for first page in the list */
  u32 N                 /* Expected number of pages in the list */
){
  పూర్ణాంకం i;
  u32 expected = N;
  పూర్ణాంకం n_Err_At_ప్రారంభం = pCheck->nErr;
  అయితే( i_పుట!=0 && pCheck->mxErr ){
    Db_పుట *p_Ovfl_పుట;
    గుర్తులేని అక్షరం *p_Ovfl_దత్తాంశం;
    ఒకవేళ( checkRef(pCheck, i_పుట) ) ఆపు;
    N--;
    ఒకవేళ( sqlite3PagerGet(pCheck->pPager, (Pgno)i_పుట, &p_Ovfl_పుట, 0) ){
      check_Append_సందేశం(pCheck, "failed to get page %u", i_పుట);
      ఆపు;
    }
    p_Ovfl_దత్తాంశం = (గుర్తులేని అక్షరం *)sqlite3_Pager_Get_దత్తాంశం(p_Ovfl_పుట);
    ఒకవేళ( isFreeList ){
      u32 n = (u32)get4byte(&p_Ovfl_దత్తాంశం[4]);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
      ఒకవేళ( pCheck->pBt->autoVacuum ){
        checkPtrmap(pCheck, i_పుట, PTRMAP_FREEPAGE, 0);
      }
#అయితే_ముగింపు
      ఒకవేళ( n>pCheck->pBt->usable_కొలత/4-2 ){
        check_Append_సందేశం(pCheck,
           "freelist leaf count too big on page %u", i_పుట);
        N--;
      }లేకపోతే{
        కొరకు(i=0; i<(పూర్ణాంకం)n; i++){
          Pgno i_Free_పుట = get4byte(&p_Ovfl_దత్తాంశం[8+i*4]);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
          ఒకవేళ( pCheck->pBt->autoVacuum ){
            checkPtrmap(pCheck, i_Free_పుట, PTRMAP_FREEPAGE, 0);
          }
#అయితే_ముగింపు
          checkRef(pCheck, i_Free_పుట);
        }
        N -= n;
      }
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
    లేకపోతే{
      /* If this database supports auto-vacuum and iPage is not the last
      ** page in this overflow list, check that the pointer-map entry for
      ** the following page matches iPage.
      */
      ఒకవేళ( pCheck->pBt->autoVacuum && N>0 ){
        i = get4byte(p_Ovfl_దత్తాంశం);
        checkPtrmap(pCheck, i, PTRMAP_OVERFLOW2, i_పుట);
      }
    }
#అయితే_ముగింపు
    i_పుట = get4byte(p_Ovfl_దత్తాంశం);
    sqlite3PagerUnref(p_Ovfl_పుట);
  }
  ఒకవేళ( N && n_Err_At_ప్రారంభం==pCheck->nErr ){
    check_Append_సందేశం(pCheck,
      "%s is %u but should be %u",
      isFreeList ? "size" : "overflow list length",
      expected-N, expected);
  }
}
#అయితే_ముగింపు /* SQLITE_OMIT_INTEGRITY_CHECK */

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
స్థిర శూన్య btree_కుప్ప_చొప్పించు(u32 *a_కుప్ప, u32 x){
  u32 j, i;
  ప్రతిజ్ఞా( a_కుప్ప!=0 );
  i = ++a_కుప్ప[0];
  a_కుప్ప[i] = x;
  అయితే( (j = i/2)>0 && a_కుప్ప[j]>a_కుప్ప[i] ){
    x = a_కుప్ప[j];
    a_కుప్ప[j] = a_కుప్ప[i];
    a_కుప్ప[i] = x;
    i = j;
  }
}
స్థిర పూర్ణాంకం btree_కుప్ప_Pull(u32 *a_కుప్ప, u32 *p_బహిర్గతం){
  u32 j, i, x;
  ఒకవేళ( (x = a_కుప్ప[0])==0 ) తిరిగి 0;
  *p_బహిర్గతం = a_కుప్ప[1];
  a_కుప్ప[1] = a_కుప్ప[x];
  a_కుప్ప[x] = 0xffffffff;
  a_కుప్ప[0]--;
  i = 1;
  అయితే( (j = i*2)<=a_కుప్ప[0] ){
    ఒకవేళ( a_కుప్ప[j]>a_కుప్ప[j+1] ) j++;
    ఒకవేళ( a_కుప్ప[i]<a_కుప్ప[j] ) ఆపు;
    x = a_కుప్ప[i];
    a_కుప్ప[i] = a_కుప్ప[j];
    a_కుప్ప[j] = x;
    i = j;
  }
  తిరిగి 1; 
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_INTEGRITY_CHECK
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
స్థిర పూర్ణాంకం check_వృక్షం_పుట(
  IntegrityCk *pCheck,  /* Context for the sanity check */
  Pgno i_పుట,           /* Page number of the page to check */
  i64 *pi_Min_కీలకం,        /* Write minimum integer primary key here */
  i64 max_కీలకం            /* Error if integer primary key greater than this */
){
  Mem_పుట *p_పుట = 0;      /* The page being analyzed */
  పూర్ణాంకం i;                   /* Loop counter */
  పూర్ణాంకం rc;                  /* Result code from subroutine call */
  పూర్ణాంకం లోతు = -1, d2;      /* Depth of a subtree */
  పూర్ణాంకం pgno;                /* Page number */
  పూర్ణాంకం nFrag;               /* Number of fragmented bytes on the page */
  పూర్ణాంకం hdr;                 /* Offset to the page header */
  పూర్ణాంకం cell_ప్రారంభం;           /* Offset to the start of the cell pointer array */
  పూర్ణాంకం nCell;               /* Number of cells */
  పూర్ణాంకం doCoverageCheck = 1; /* True if cell coverage checking should be done */
  పూర్ణాంకం కీలకం_Can_Be_Equal = 1;   /* True if IPK can be equal to maxKey
                           ** False if IPK must be strictly less than maxKey */
  u8 *దత్తాంశం;                /* Page content */
  u8 *pCell;               /* Cell content */
  u8 *pCellIdx;            /* Next element of the cell pointer array */
  BtShared *pBt;           /* The BtShared object that owns pPage */
  u32 pc;                  /* Address of a cell */
  u32 usable_కొలత;          /* Usable size of the page */
  u32 content_స్థానభ్రంశం;       /* Offset to the start of the cell content area */
  u32 *కుప్ప = 0;           /* Min-heap used for checking cell coverage */
  u32 x, prev = 0;         /* Next and previous entry on the min-heap */
  స్థిరరాశి అక్షరం *saved_zPfx = pCheck->zPfx;
  పూర్ణాంకం saved_v1 = pCheck->v1;
  పూర్ణాంకం saved_v2 = pCheck->v2;
  u8 savedIsInit = 0;

  /* Check that the page exists
  */
  check_పురోగతి(pCheck);
  ఒకవేళ( pCheck->mxErr==0 ) తిరిగి 0;
  pBt = pCheck->pBt;
  usable_కొలత = pBt->usable_కొలత;
  ఒకవేళ( i_పుట==0 ) తిరిగి 0;
  ఒకవేళ( checkRef(pCheck, i_పుట) ) తిరిగి 0;
  pCheck->zPfx = "Tree %u page %u: ";
  pCheck->v1 = i_పుట;
  pCheck->nAbove++;
  ఒకవేళ( (rc = btree_Get_పుట(pBt, i_పుట, &p_పుట, 0))!=0 ){
    check_Append_సందేశం(pCheck,
       "unable to get the page. error code=%d", rc);
    ఒకవేళ( rc==SQLITE_IOERR_NOMEM ) pCheck->rc = SQLITE_NOMEM;
    వెళ్ళు చివర_యొక్క_check;
  }
  ఒకవేళ( pCheck->nAbove > BTCURSOR_MAX_లోతు ){
    check_Append_సందేశం(pCheck,"btree depth exceeds %d",BTCURSOR_MAX_లోతు);
    వెళ్ళు చివర_యొక్క_check;
  }

  /* Clear MemPage.isInit to make sure the corruption detection code in
  ** btreeInitPage() is executed.  */
  savedIsInit = p_పుట->isInit;
  p_పుట->isInit = 0;
  ఒకవేళ( (rc = btree_Init_పుట(p_పుట))!=0 ){
    ప్రతిజ్ఞా( rc==SQLITE_CORRUPT );  /* The only possible error from InitPage */
    check_Append_సందేశం(pCheck,
                   "btreeInitPage() returns error code %d", rc);
    వెళ్ళు చివర_యొక్క_check;
  }
  ఒకవేళ( (rc = btreeComputeFreeSpace(p_పుట))!=0 ){
    ప్రతిజ్ఞా( rc==SQLITE_CORRUPT );
    check_Append_సందేశం(pCheck, "free space corruption", rc);
    వెళ్ళు చివర_యొక్క_check;
  }
  దత్తాంశం = p_పుట->a_దత్తాంశం;
  hdr = p_పుట->hdr_స్థానభ్రంశం;

  /* Set up for cell analysis */
  pCheck->zPfx = "Tree %u page %u cell %u: ";
  content_స్థానభ్రంశం = get2byteNotZero(&దత్తాంశం[hdr+5]);
  ప్రతిజ్ఞా( content_స్థానభ్రంశం<=usable_కొలత );  /* Enforced by btreeInitPage() */

  /* EVIDENCE-OF: R-37002-32774 The two-byte integer at offset 3 gives the
  ** number of cells on the page. */
  nCell = get2byte(&దత్తాంశం[hdr+3]);
  ప్రతిజ్ఞా( p_పుట->nCell==nCell );
  ఒకవేళ( p_పుట->ఆకు || p_పుట->int_కీలకం==0 ){
    pCheck->n_అడ్డువరుస += nCell;
  }

  /* EVIDENCE-OF: R-23882-45353 The cell pointer array of a b-tree page
  ** immediately follows the b-tree page header. */
  cell_ప్రారంభం = hdr + 12 - 4*p_పుట->ఆకు;
  ప్రతిజ్ఞా( p_పుట->aCellIdx==&దత్తాంశం[cell_ప్రారంభం] );
  pCellIdx = &దత్తాంశం[cell_ప్రారంభం + 2*(nCell-1)];

  ఒకవేళ( !p_పుట->ఆకు ){
    /* Analyze the right-child page of internal pages */
    pgno = get4byte(&దత్తాంశం[hdr+8]);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
    ఒకవేళ( pBt->autoVacuum ){
      pCheck->zPfx = "Tree %u page %u right child: ";
      checkPtrmap(pCheck, pgno, PTRMAP_BTREE, i_పుట);
    }
#అయితే_ముగింపు
    లోతు = check_వృక్షం_పుట(pCheck, pgno, &max_కీలకం, max_కీలకం);
    కీలకం_Can_Be_Equal = 0;
  }లేకపోతే{
    /* For leaf pages, the coverage check will occur in the same loop
    ** as the other cell checks, so initialize the heap.  */
    కుప్ప = pCheck->కుప్ప;
    కుప్ప[0] = 0;
  }

  /* EVIDENCE-OF: R-02776-14802 The cell pointer array consists of K 2-byte
  ** integer offsets to the cell contents. */
  కొరకు(i=nCell-1; i>=0 && pCheck->mxErr; i--){
    CellInfo info;

    /* Check cell size */
    pCheck->v2 = i;
    ప్రతిజ్ఞా( pCellIdx==&దత్తాంశం[cell_ప్రారంభం + i*2] );
    pc = get2byteAligned(pCellIdx);
    pCellIdx -= 2;
    ఒకవేళ( pc<content_స్థానభ్రంశం || pc>usable_కొలత-4 ){
      check_Append_సందేశం(pCheck, "Offset %u out of range %u..%u",
                             pc, content_స్థానభ్రంశం, usable_కొలత-4);
      doCoverageCheck = 0;
      కొనసాగు;
    }
    pCell = &దత్తాంశం[pc];
    p_పుట->xParseCell(p_పుట, pCell, &info);
    ఒకవేళ( pc+info.n_కొలత>usable_కొలత ){
      check_Append_సందేశం(pCheck, "Extends off end of page");
      doCoverageCheck = 0;
      కొనసాగు;
    }
    ఒకవేళ( info.nPayload && info.pPayload[0]<2 ){
      check_Append_సందేశం(pCheck, "Bad cell header size");
      doCoverageCheck = 0;
      కొనసాగు;
    }

    /* Check for integer primary key out of range */
    ఒకవేళ( p_పుట->int_కీలకం ){
      ఒకవేళ( కీలకం_Can_Be_Equal ? (info.n_కీలకం > max_కీలకం) : (info.n_కీలకం >= max_కీలకం) ){
        check_Append_సందేశం(pCheck, "Rowid %lld out of order", info.n_కీలకం);
      }
      max_కీలకం = info.n_కీలకం;
      కీలకం_Can_Be_Equal = 0;     /* Only the first key on the page may ==maxKey */
    }

    /* Check the content overflow list */
    ఒకవేళ( info.nPayload>info.nLocal ){
      u32 n_పుట;       /* Number of pages on the overflow chain */
      Pgno pgnoOvfl;   /* First page of the overflow chain */
      ప్రతిజ్ఞా( pc + info.n_కొలత - 4 <= usable_కొలత );
      n_పుట = (info.nPayload - info.nLocal + usable_కొలత - 5)/(usable_కొలత - 4);
      pgnoOvfl = get4byte(&pCell[info.n_కొలత - 4]);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
      ఒకవేళ( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgnoOvfl, PTRMAP_OVERFLOW1, i_పుట);
      }
#అయితే_ముగింపు
      checkList(pCheck, 0, pgnoOvfl, n_పుట);
    }

    ఒకవేళ( !p_పుట->ఆకు ){
      /* Check sanity of left child page for internal pages */
      pgno = get4byte(pCell);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
      ఒకవేళ( pBt->autoVacuum ){
        checkPtrmap(pCheck, pgno, PTRMAP_BTREE, i_పుట);
      }
#అయితే_ముగింపు
      d2 = check_వృక్షం_పుట(pCheck, pgno, &max_కీలకం, max_కీలకం);
      కీలకం_Can_Be_Equal = 0;
      ఒకవేళ( d2!=లోతు ){
        check_Append_సందేశం(pCheck, "Child page depth differs");
        లోతు = d2;
      }
    }లేకపోతే{
      /* Populate the coverage-checking heap for leaf pages */
      ప్రతిజ్ఞా( కుప్ప!=0 && కుప్ప[0] < pCheck->mx_కుప్ప );
      btree_కుప్ప_చొప్పించు(కుప్ప, (pc<<16)|(pc+info.n_కొలత-1));
    }
  }
  *pi_Min_కీలకం = max_కీలకం;

  /* Check for complete coverage of the page
  */
  pCheck->zPfx = 0;
  ఒకవేళ( doCoverageCheck && pCheck->mxErr>0 ){
    /* For leaf pages, the min-heap has already been initialized and the
    ** cells have already been inserted.  But for internal pages, that has
    ** not yet been done, so do it now */
    ఒకవేళ( !p_పుట->ఆకు ){
      కుప్ప = pCheck->కుప్ప;
      కుప్ప[0] = 0;
      కొరకు(i=nCell-1; i>=0; i--){
        u32 కొలత;
        pc = get2byteAligned(&దత్తాంశం[cell_ప్రారంభం+i*2]);
        కొలత = p_పుట->x_Cell_కొలత(p_పుట, &దత్తాంశం[pc]);
        ప్రతిజ్ఞా( కుప్ప!=0 && కుప్ప[0] < pCheck->mx_కుప్ప );
        btree_కుప్ప_చొప్పించు(కుప్ప, (pc<<16)|(pc+కొలత-1));
      }
    }
    ప్రతిజ్ఞా( కుప్ప!=0 );
    /* Add the freeblocks to the min-heap
    **
    ** EVIDENCE-OF: R-20690-50594 The second field of the b-tree page header
    ** is the offset of the first freeblock, or zero if there are no
    ** freeblocks on the page.
    */
    i = get2byte(&దత్తాంశం[hdr+1]);
    అయితే( i>0 ){
      పూర్ణాంకం కొలత, j;
      ప్రతిజ్ఞా( (u32)i<=usable_కొలత-4 ); /* Enforced by btreeComputeFreeSpace() */
      కొలత = get2byte(&దత్తాంశం[i+2]);
      ప్రతిజ్ఞా( (u32)(i+కొలత)<=usable_కొలత ); /* due to btreeComputeFreeSpace() */
      ప్రతిజ్ఞా( కుప్ప!=0 && కుప్ప[0] < pCheck->mx_కుప్ప );
      btree_కుప్ప_చొప్పించు(కుప్ప, (((u32)i)<<16)|(i+కొలత-1));
      /* EVIDENCE-OF: R-58208-19414 The first 2 bytes of a freeblock are a
      ** big-endian integer which is the offset in the b-tree page of the next
      ** freeblock in the chain, or zero if the freeblock is the last on the
      ** chain. */
      j = get2byte(&దత్తాంశం[i]);
      /* EVIDENCE-OF: R-06866-39125 Freeblocks are always connected in order of
      ** increasing offset. */
      ప్రతిజ్ఞా( j==0 || j>i+కొలత );     /* Enforced by btreeComputeFreeSpace() */
      ప్రతిజ్ఞా( (u32)j<=usable_కొలత-4 ); /* Enforced by btreeComputeFreeSpace() */
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
    prev = content_స్థానభ్రంశం - 1;   /* Implied first min-heap entry */
    అయితే( btree_కుప్ప_Pull(కుప్ప,&x) ){
      ఒకవేళ( (prev&0xffff)>=(x>>16) ){
        check_Append_సందేశం(pCheck,
          "Multiple uses for byte %u of page %u", x>>16, i_పుట);
        ఆపు;
      }లేకపోతే{
        nFrag += (x>>16) - (prev&0xffff) - 1;
        prev = x;
      }
    }
    nFrag += usable_కొలత - (prev&0xffff) - 1;
    /* EVIDENCE-OF: R-43263-13491 The total number of bytes in all fragments
    ** is stored in the fifth field of the b-tree page header.
    ** EVIDENCE-OF: R-07161-27322 The one-byte integer at offset 7 gives the
    ** number of fragmented free bytes within the cell content area.
    */
    ఒకవేళ( కుప్ప[0]==0 && nFrag!=దత్తాంశం[hdr+7] ){
      check_Append_సందేశం(pCheck,
          "Fragmentation of %u bytes reported as %u on page %u",
          nFrag, దత్తాంశం[hdr+7], i_పుట);
    }
  }

చివర_యొక్క_check:
  ఒకవేళ( !doCoverageCheck ) p_పుట->isInit = savedIsInit;
  release_పుట(p_పుట);
  pCheck->zPfx = saved_zPfx;
  pCheck->v1 = saved_v1;
  pCheck->v2 = saved_v2;
  pCheck->nAbove--;
  తిరిగి లోతు+1;
}
#అయితే_ముగింపు /* SQLITE_OMIT_INTEGRITY_CHECK */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_INTEGRITY_CHECK
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
పూర్ణాంకం sqlite3BtreeIntegrityCheck(
  sqlite3 *db,  /* Database connection that is running the check */
  Btree *p,     /* The btree to be checked */
  Pgno *a_మూలం,  /* An array of root pages numbers for individual trees */
  Mem *aCnt,    /* Memory cells to write counts for each tree to */
  పూర్ణాంకం n_మూలం,    /* Number of entries in aRoot[] */
  పూర్ణాంకం mxErr,    /* Stop reporting errors after this many */
  పూర్ణాంకం *pnErr,   /* OUT: Write number of errors seen to this variable */
  అక్షరం **pz_బహిర్గతం  /* OUT: Write the error message string here */
){
  Pgno i;
  IntegrityCk sCheck;
  BtShared *pBt = p->pBt;
  u64 savedDbFlags = pBt->db->flags;
  అక్షరం zErr[100];
  పూర్ణాంకం bPartial = 0;            /* True if not checking all btrees */
  పూర్ణాంకం bCkFreelist = 1;         /* True to scan the freelist */
  VVA_ONLY( పూర్ణాంకం nRef );

  ప్రతిజ్ఞా( n_మూలం>0 );
  ప్రతిజ్ఞా( aCnt!=0 );

  /* aRoot[0]==0 means this is a partial check */
  ఒకవేళ( a_మూలం[0]==0 ){
    ప్రతిజ్ఞా( n_మూలం>1 );
    bPartial = 1;
    ఒకవేళ( a_మూలం[1]!=1 ) bCkFreelist = 0;
  }

  sqlite3BtreeEnter(p);
  ప్రతిజ్ఞా( p->లోపల_Trans>TRANS_NONE && pBt->లోపల_Transaction>TRANS_NONE );
  VVA_ONLY( nRef = sqlite3PagerRefcount(pBt->pPager) );
  ప్రతిజ్ఞా( nRef>=0 );
  memset(&sCheck, 0, పరిమాణం(sCheck));
  sCheck.db = db;
  sCheck.pBt = pBt;
  sCheck.pPager = pBt->pPager;
  sCheck.n_Ck_పుట = btreePagecount(sCheck.pBt);
  sCheck.mxErr = mxErr;
  sqlite3StrAccumInit(&sCheck.err_సందేశం, 0, zErr, పరిమాణం(zErr), SQLITE_MAX_నిడివి);
  sCheck.err_సందేశం.printfFlags = SQLITE_PRINTF_అంతర్గత;
  ఒకవేళ( sCheck.n_Ck_పుట==0 ){
    వెళ్ళు integrity_ck_cleanup;
  }

  sCheck.aPgRef = sqlite3MallocZero((sCheck.n_Ck_పుట / 8)+ 1);
  ఒకవేళ( !sCheck.aPgRef ){
    checkOom(&sCheck);
    వెళ్ళు integrity_ck_cleanup;
  }
  sCheck.కుప్ప = (u32*)sqlite3Malloc( pBt->పుట_కొలత*2 );
#నిర్వచితమైతే SQLITE_DEBUG
  sCheck.mx_కుప్ప = pBt->పుట_కొలత/2 - 1;
#అయితే_ముగింపు
  ఒకవేళ( sCheck.కుప్ప==0 ){
    checkOom(&sCheck);
    వెళ్ళు integrity_ck_cleanup;
  }

  i = PENDING_BYTE_పుట(pBt);
  ఒకవేళ( i<=sCheck.n_Ck_పుట ) సమితి_పుట_Referenced(&sCheck, i);

  /* Check the integrity of the freelist
  */
  ఒకవేళ( bCkFreelist ){
    sCheck.zPfx = "Freelist: ";
    checkList(&sCheck, 1, get4byte(&pBt->pPage1->a_దత్తాంశం[32]),
              get4byte(&pBt->pPage1->a_దత్తాంశం[36]));
    sCheck.zPfx = 0;
  }

  /* Check all the tables.
  */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
  ఒకవేళ( !bPartial ){
    ఒకవేళ( pBt->autoVacuum ){
      Pgno mx = 0;
      Pgno mx_లోపల_Hdr;
      కొరకు(i=0; (పూర్ణాంకం)i<n_మూలం; i++) ఒకవేళ( mx<a_మూలం[i] ) mx = a_మూలం[i];
      mx_లోపల_Hdr = get4byte(&pBt->pPage1->a_దత్తాంశం[52]);
      ఒకవేళ( mx!=mx_లోపల_Hdr ){
        check_Append_సందేశం(&sCheck,
          "max rootpage (%u) disagrees with header (%u)",
          mx, mx_లోపల_Hdr
        );
      }
    }లేకపోతే ఒకవేళ( get4byte(&pBt->pPage1->a_దత్తాంశం[64])!=0 ){
      check_Append_సందేశం(&sCheck,
        "incremental_vacuum enabled with a max rootpage of zero"
      );
    }
  }
#అయితే_ముగింపు
  testcase( pBt->db->flags & SQLITE_Cell_కొలత_Ck );
  pBt->db->flags &= ~(u64)SQLITE_Cell_కొలత_Ck;
  కొరకు(i=0; (పూర్ణాంకం)i<n_మూలం && sCheck.mxErr; i++){
    sCheck.n_అడ్డువరుస = 0;
    ఒకవేళ( a_మూలం[i] ){
      i64 not_వాడిన;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTOVACUUM
      ఒకవేళ( pBt->autoVacuum && a_మూలం[i]>1 && !bPartial ){
        checkPtrmap(&sCheck, a_మూలం[i], PTRMAP_ROOTPAGE, 0);
      }
#అయితే_ముగింపు
      sCheck.v0 = a_మూలం[i];
      ప్రతిజ్ఞా( sCheck.nAbove==0 );
      check_వృక్షం_పుట(&sCheck, a_మూలం[i], &not_వాడిన, అతిపెద్ద_INT64);
      ప్రతిజ్ఞా( sCheck.nAbove==0 );
    }
    sqlite3_Mem_సమితి_Array_Int64(aCnt, i, sCheck.n_అడ్డువరుస);
  }
  pBt->db->flags = savedDbFlags;

  /* Make sure every page in the file is referenced
  */
  ఒకవేళ( !bPartial ){
    కొరకు(i=1; i<=sCheck.n_Ck_పుట && sCheck.mxErr; i++){
#నిర్వచితమైతే SQLITE_OMIT_AUTOVACUUM
      ఒకవేళ( get_పుట_Referenced(&sCheck, i)==0 ){
        check_Append_సందేశం(&sCheck, "Page %u: never used", i);
      }
#లేకపోతే
      /* If the database supports auto-vacuum, make sure no tables contain
      ** references to pointer-map pages.
      */
      ఒకవేళ( get_పుట_Referenced(&sCheck, i)==0 &&
         (PTRMAP_PAGENO(pBt, i)!=i || !pBt->autoVacuum) ){
        check_Append_సందేశం(&sCheck, "Page %u: never used", i);
      }
      ఒకవేళ( get_పుట_Referenced(&sCheck, i)!=0 &&
         (PTRMAP_PAGENO(pBt, i)==i && pBt->autoVacuum) ){
        check_Append_సందేశం(&sCheck, "Page %u: pointer map referenced", i);
      }
#అయితే_ముగింపు
    }
  }

  /* Clean  up and report errors.
  */
integrity_ck_cleanup:
  sqlite3_free(sCheck.కుప్ప);
  sqlite3_free(sCheck.aPgRef);
  *pnErr = sCheck.nErr;
  ఒకవేళ( sCheck.nErr==0 ){
    sqlite3_str_reset(&sCheck.err_సందేశం);
    *pz_బహిర్గతం = 0;
  }లేకపోతే{
    *pz_బహిర్గతం = sqlite3_Str_Accum_ముగింపు(&sCheck.err_సందేశం);
  }
  /* Make sure this analysis did not leave any unref() pages. */
  ప్రతిజ్ఞా( nRef==sqlite3PagerRefcount(pBt->pPager) );
  sqlite3BtreeLeave(p);
  తిరిగి sCheck.rc;
}
#అయితే_ముగింపు /* SQLITE_OMIT_INTEGRITY_CHECK */

/*
** Return the full pathname of the underlying database file.  Return
** an empty string if the database is in-memory or a TEMP database.
**
** The pager filename is invariant as long as the pager is
** open so it is safe to access without the BtShared mutex.
*/
స్థిరరాశి అక్షరం *sqlite3BtreeGetFilename(Btree *p){
  ప్రతిజ్ఞా( p->pBt->pPager!=0 );
  తిరిగి sqlite3PagerFilename(p->pBt->pPager, 1);
}

/*
** Return the pathname of the journal file for this database. The return
** value of this routine is the same regardless of whether the journal file
** has been created or not.
**
** The pager journal filename is invariant as long as the pager is
** open so it is safe to access without the BtShared mutex.
*/
స్థిరరాశి అక్షరం *sqlite3BtreeGetJournalname(Btree *p){
  ప్రతిజ్ఞా( p->pBt->pPager!=0 );
  తిరిగి sqlite3PagerJournalname(p->pBt->pPager);
}

/*
** Return one of SQLITE_TXN_NONE, SQLITE_TXN_READ, or SQLITE_TXN_WRITE
** to describe the current transaction state of Btree p.
*/
పూర్ణాంకం sqlite3BtreeTxnState(Btree *p){
  ప్రతిజ్ఞా( p==0 || sqlite3_mutex_held(p->db->mutex) );
  తిరిగి p ? p->లోపల_Trans : 0;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
/*
** Run a checkpoint on the Btree passed as the first argument.
**
** Return SQLITE_LOCKED if this or any other connection has an open
** transaction on the shared-cache the argument Btree is connected to.
**
** Parameter eMode is one of SQLITE_CHECKPOINT_PASSIVE, FULL or RESTART.
*/
పూర్ణాంకం sqlite3BtreeCheckpoint(Btree *p, పూర్ణాంకం e_బహుళకం, పూర్ణాంకం *pnLog, పూర్ణాంకం *pnCkpt){
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( p ){
    BtShared *pBt = p->pBt;
    sqlite3BtreeEnter(p);
    ఒకవేళ( pBt->లోపల_Transaction!=TRANS_NONE ){
      rc = SQLITE_LOCKED;
    }లేకపోతే{
      rc = sqlite3PagerCheckpoint(pBt->pPager, p->db, e_బహుళకం, pnLog, pnCkpt);
    }
    sqlite3BtreeLeave(p);
  }
  తిరిగి rc;
}
#అయితే_ముగింపు

/*
** Return true if there is currently a backup running on Btree p.
*/
పూర్ణాంకం sqlite3_Btree_Is_లోపల_Backup(Btree *p){
  ప్రతిజ్ఞా( p );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->db->mutex) );
  తిరిగి p->nBackup!=0;
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
శూన్య *sqlite3BtreeSchema(Btree *p, పూర్ణాంకం n_బైట్లు, శూన్య(*xFree)(శూన్య *)){
  BtShared *pBt = p->pBt;
  ప్రతిజ్ఞా( n_బైట్లు==0 || n_బైట్లు==పరిమాణం(Schema) );
  sqlite3BtreeEnter(p);
  ఒకవేళ( !pBt->pSchema && n_బైట్లు ){
    pBt->pSchema = sqlite3DbMallocZero(0, n_బైట్లు);
    pBt->xFreeSchema = xFree;
  }
  sqlite3BtreeLeave(p);
  తిరిగి pBt->pSchema;
}

/*
** Return SQLITE_LOCKED_SHAREDCACHE if another user of the same shared
** btree as the argument handle holds an exclusive lock on the
** sqlite_schema table. Otherwise SQLITE_OK.
*/
పూర్ణాంకం sqlite3BtreeSchemaLocked(Btree *p){
  పూర్ణాంకం rc;
  వాడని_PARAMETER(p);  /* only used in DEBUG builds */
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->db->mutex) );
  sqlite3BtreeEnter(p);
  rc = query_Shared_Cache_పట్టిక_తాళం(p, SCHEMA_మూలం, READ_తాళం);
  ప్రతిజ్ఞా( rc==SQLITE_OK || rc==SQLITE_LOCKED_SHAREDCACHE );
  sqlite3BtreeLeave(p);
  తిరిగి rc;
}


#నిర్వచితం_కాకపోతే SQLITE_OMIT_SHARED_CACHE
/*
** Obtain a lock on the table whose root page is iTab.  The
** lock is a write lock if isWritelock is true or a read lock
** if it is false.
*/
పూర్ణాంకం sqlite3_Btree_తాళం_పట్టిక(Btree *p, పూర్ణాంకం iTab, u8 is_Write_తాళం){
  పూర్ణాంకం rc = SQLITE_OK;
  ప్రతిజ్ఞా( p->లోపల_Trans!=TRANS_NONE );
  ఒకవేళ( p->sharable ){
    u8 తాళం_Type = READ_తాళం + is_Write_తాళం;
    ప్రతిజ్ఞా( READ_తాళం+1==WRITE_తాళం );
    ప్రతిజ్ఞా( is_Write_తాళం==0 || is_Write_తాళం==1 );

    sqlite3BtreeEnter(p);
    rc = query_Shared_Cache_పట్టిక_తాళం(p, iTab, తాళం_Type);
    ఒకవేళ( rc==SQLITE_OK ){
      rc = సమితి_Shared_Cache_పట్టిక_తాళం(p, iTab, తాళం_Type);
    }
    sqlite3BtreeLeave(p);
  }
  తిరిగి rc;
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే SQLITE_OMIT_INCRBLOB
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
పూర్ణాంకం sqlite3_Btree_Put_దత్తాంశం(BtCursor *pCsr, u32 స్థానభ్రంశం, u32 amt, శూన్య *z){
  పూర్ణాంకం rc;
  ప్రతిజ్ఞా( cursorOwnsBtShared(pCsr) );
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCsr->pBtree->db->mutex) );
  ప్రతిజ్ఞా( pCsr->curFlags & BTCF_Incrblob );

  rc = restoreCursorPosition(pCsr);
  ఒకవేళ( rc!=SQLITE_OK ){
    తిరిగి rc;
  }
  ప్రతిజ్ఞా( pCsr->eState!=CURSOR_REQUIRESEEK );
  ఒకవేళ( pCsr->eState!=CURSOR_VALID ){
    తిరిగి SQLITE_ABORT;
  }

  /* Save the positions of all other cursors open on this table. This is
  ** required in case any of them are holding references to an xFetch
  ** version of the b-tree page modified by the accessPayload call below.
  **
  ** Note that pCsr must be open on a INTKEY table and saveCursorPosition()
  ** and hence saveAllCursors() cannot fail on a BTREE_INTKEY table, hence
  ** saveAllCursors can only return SQLITE_OK.
  */
  VVA_ONLY(rc =) saveAllCursors(pCsr->pBt, pCsr->pgno_మూలం, pCsr);
  ప్రతిజ్ఞా( rc==SQLITE_OK );

  /* Check some assumptions:
  **   (a) the cursor is open for writing,
  **   (b) there is a read/write transaction open,
  **   (c) the connection holds a write-lock on the table (if required),
  **   (d) there are no conflicting read-locks, and
  **   (e) the cursor points at a valid row of an intKey table.
  */
  ఒకవేళ( (pCsr->curFlags & BTCF_WriteFlag)==0 ){
    తిరిగి SQLITE_READONLY;
  }
  ప్రతిజ్ఞా( (pCsr->pBt->btsFlags & BTS_READ_ONLY)==0
              && pCsr->pBt->లోపల_Transaction==TRANS_WRITE );
  ప్రతిజ్ఞా( has_Shared_Cache_పట్టిక_తాళం(pCsr->pBtree, pCsr->pgno_మూలం, 0, 2) );
  ప్రతిజ్ఞా( !hasReadConflicts(pCsr->pBtree, pCsr->pgno_మూలం) );
  ప్రతిజ్ఞా( pCsr->p_పుట->int_కీలకం );

  తిరిగి accessPayload(pCsr, స్థానభ్రంశం, amt, (గుర్తులేని అక్షరం *)z, 1);
}

/*
** Mark this cursor as an incremental blob cursor.
*/
శూన్య sqlite3BtreeIncrblobCursor(BtCursor *pCur){
  pCur->curFlags |= BTCF_Incrblob;
  pCur->pBtree->hasIncrblobCur = 1;
}
#అయితే_ముగింపు

/*
** Set both the "read version" (single byte at byte offset 18) and
** "write version" (single byte at byte offset 19) fields in the database
** header to iVersion.
*/
పూర్ణాంకం sqlite3_Btree_సమితి_సంస్కరణ(Btree *pBtree, పూర్ణాంకం i_సంస్కరణ){
  BtShared *pBt = pBtree->pBt;
  పూర్ణాంకం rc;                         /* Return code */

  ప్రతిజ్ఞా( i_సంస్కరణ==1 || i_సంస్కరణ==2 );

  /* If setting the version fields to 1, do not automatically open the
  ** WAL connection, even if the version fields are currently set to 2.
  */
  pBt->btsFlags &= ~BTS_NO_WAL;
  ఒకవేళ( i_సంస్కరణ==1 ) pBt->btsFlags |= BTS_NO_WAL;

  rc = sqlite3BtreeBeginTrans(pBtree, 0, 0);
  ఒకవేళ( rc==SQLITE_OK ){
    u8 *a_దత్తాంశం = pBt->pPage1->a_దత్తాంశం;
    ఒకవేళ( a_దత్తాంశం[18]!=(u8)i_సంస్కరణ || a_దత్తాంశం[19]!=(u8)i_సంస్కరణ ){
      rc = sqlite3BtreeBeginTrans(pBtree, 2, 0);
      ఒకవేళ( rc==SQLITE_OK ){
        rc = sqlite3PagerWrite(pBt->pPage1->p_Db_పుట);
        ఒకవేళ( rc==SQLITE_OK ){
          a_దత్తాంశం[18] = (u8)i_సంస్కరణ;
          a_దత్తాంశం[19] = (u8)i_సంస్కరణ;
        }
      }
    }
  }

  pBt->btsFlags &= ~BTS_NO_WAL;
  తిరిగి rc;
}

/*
** Return true if the cursor has a hint specified.  This routine is
** only used from within assert() statements
*/
పూర్ణాంకం sqlite3BtreeCursorHasHint(BtCursor *pCsr, గుర్తులేని పూర్ణాంకం ముసుగు){
  తిరిగి (pCsr->hints & ముసుగు)!=0;
}

/*
** Return true if the given Btree is read-only.
*/
పూర్ణాంకం sqlite3BtreeIsReadonly(Btree *p){
  తిరిగి (p->pBt->btsFlags & BTS_READ_ONLY)!=0;
}

/*
** Return the size of the header added to each page by this module.
*/
పూర్ణాంకం sqlite3_శీర్షిక_కొలత_Btree(శూన్య){ తిరిగి ROUND8(పరిమాణం(Mem_పుట)); }

/*
** If no transaction is active and the database is not a temp-db, clear
** the in-memory pager cache.
*/
శూన్య sqlite3BtreeClearCache(Btree *p){
  BtShared *pBt = p->pBt;
  ఒకవేళ( pBt->లోపల_Transaction==TRANS_NONE ){
    sqlite3PagerClearCache(pBt->pPager);
  }
}

#ఒకవేళ !defined(SQLITE_OMIT_SHARED_CACHE)
/*
** Return true if the Btree passed as the only argument is sharable.
*/
పూర్ణాంకం sqlite3BtreeSharable(Btree *p){
  తిరిగి p->sharable;
}

/*
** Return the number of connections to the BtShared object accessed by
** the Btree handle passed as the only argument. For private caches
** this is always 1. For shared caches it may be 1 or greater.
*/
పూర్ణాంకం sqlite3_Btree_Connection_లెక్క(Btree *p){
  testcase( p->sharable );
  తిరిగి p->pBt->nRef;
}
#అయితే_ముగింపు

/* Re-enable GCC compiler warnings that were suppressed at the top
** of this source file to prevent annoying false-positives.
*/
#ఒకవేళ defined(__GNUC__) && __GNUC__>=11
# pragma GCC diagnostic తీయి
#అయితే_ముగింపు
