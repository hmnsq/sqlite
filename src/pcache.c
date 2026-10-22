/*
** 2008 August 05
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file implements that page cache.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** A complete page cache is an instance of this structure.  Every
** entry in the cache holds a single page of the database file.  The
** btree layer only operates on the cached copy of the database pages.
**
** A page cache entry is "clean" if it exactly matches what is currently
** on disk.  A page is "dirty" if it has been modified and needs to be
** persisted to disk.
**
** pDirty, pDirtyTail, pSynced:
**   All dirty pages are linked into the doubly linked list using
**   PgHdr.pDirtyNext and pDirtyPrev. The list is maintained in LRU order
**   such that p was added to the list more recently than p->pDirtyNext.
**   PCache.pDirty points to the first (newest) element in the list and
**   pDirtyTail to the last (oldest).
**
**   The PCache.pSynced variable is used to optimize searching for a dirty
**   page to eject from the cache mid-transaction. It is better to eject
**   a page that does not require a journal sync than one that does. 
**   Therefore, pSynced is maintained so that it *almost* always points
**   to either the oldest page in the pDirty/pDirtyTail list that has a
**   clear PGHDR_NEED_SYNC flag or to a page that is older than this one
**   (so that the right page to eject can be found by following pDirtyPrev
**   pointers).
*/
ਬਣਤਰ PCache {
  PgHdr *pDirty, *p_Dirty_ਪੂਛ;         /* List of dirty pages in LRU order */
  PgHdr *pSynced;                     /* Last synced page in dirty page list */
  i64 n_Ref_ਜੋੜ;                        /* Sum of ref counts over all pages */
  ਪੂਰਨ szCache;                        /* Configured cache size */
  ਪੂਰਨ szSpill;                        /* Size before spilling occurs */
  ਪੂਰਨ sz_ਪੰਨਾ;                         /* Size of every page in this cache */
  ਪੂਰਨ szExtra;                        /* Size of extra space for each page */
  u8 bPurgeable;                      /* True if pages are on backing store */
  u8 eCreate;                         /* eCreate value for for xFetch() */
  ਪੂਰਨ (*xStress)(ਖਾਲੀ*,PgHdr*);       /* Call to try make a page clean */
  ਖਾਲੀ *pStress;                      /* Argument to xStress */
  sqlite3_pcache *pCache;             /* Pluggable cache module */
};

/********************************** Test and Debug Logic **********************/
/*
** Debug tracing macros.  Enable by by changing the "0" to "1" and
** recompiling.
**
** When sqlite3PcacheTrace is 1, single line trace messages are issued.
** When sqlite3PcacheTrace is 2, a dump of the pcache showing all cache entries
** is displayed for many operations, resulting in a lot of output.
*/
#ਜੇ defined(SQLITE_DEBUG) && 0
  ਪੂਰਨ sqlite3PcacheTrace = 2;       /* 0: off  1: simple  2: cache dumps */
  ਪੂਰਨ sqlite3PcacheMxDump = 9999;   /* Max cache entries for pcacheDump() */
# define pcacheTrace(X) ਜੇ(sqlite3PcacheTrace){sqlite3DebugPrintf X;}
  ਸਥਿਰ ਖਾਲੀ pcache_ਪੰਨਾ_Trace(ਪੂਰਨ i, sqlite3_pcache_ਪੰਨਾ *p_ਹੇਠਲਾ){
    PgHdr *pPg;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a;
    ਪੂਰਨ j;
    ਜੇ( p_ਹੇਠਲਾ==0 ){
      ਲਿਖੋ("%3d: NULL\n", i);
    }ਨਹੀਂਤਾਂ{
      pPg = (PgHdr*)p_ਹੇਠਲਾ->pExtra;
      ਲਿਖੋ("%3d: nRef %2lld flgs %02x data ", i, pPg->nRef, pPg->flags);
      a = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)p_ਹੇਠਲਾ->pBuf;
      ਲਈ(j=0; j<12; j++) ਲਿਖੋ("%02x", a[j]);
      ਲਿਖੋ(" ptr %p\n", pPg);
    }
  }
  ਸਥਿਰ ਖਾਲੀ pcacheDump(PCache *pCache){
    ਪੂਰਨ N;
    ਪੂਰਨ i;
    sqlite3_pcache_ਪੰਨਾ *p_ਹੇਠਲਾ;
  
    ਜੇ( sqlite3PcacheTrace<2 ) ਵਾਪਸ;
    ਜੇ( pCache->pCache==0 ) ਵਾਪਸ;
    N = sqlite3PcachePagecount(pCache);
    ਜੇ( N>sqlite3PcacheMxDump ) N = sqlite3PcacheMxDump;
    ਲਈ(i=1; i<=N; i++){
       p_ਹੇਠਲਾ = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, i, 0);
       pcache_ਪੰਨਾ_Trace(i, p_ਹੇਠਲਾ);
       ਜੇ( p_ਹੇਠਲਾ && ((PgHdr*)p_ਹੇਠਲਾ)->p_ਪੰਨਾ==0 ){
         sqlite3GlobalConfig.pcache2.xUnpin(pCache->pCache, p_ਹੇਠਲਾ, 0);
       }
    }
  }
#ਨਹੀਂਤਾਂ
# define pcacheTrace(X)
# define pcache_ਪੰਨਾ_Trace(PGNO, X)
# define pcacheDump(X)
#ਜੇ_ਅੰਤ

/*
** Return 1 if pPg is on the dirty list for pCache.  Return 0 if not.
** This routine runs inside of assert() statements only.
*/
#ਜੇ defined(SQLITE_ENABLE_EXPENSIVE_ASSERT)
ਸਥਿਰ ਪੂਰਨ ਪੰਨਾ_On_Dirty_List(PCache *pCache, PgHdr *pPg){
  PgHdr *p;
  ਲਈ(p=pCache->pDirty; p; p=p->p_Dirty_ਅਗਲਾ){
    ਜੇ( p==pPg ) ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}
ਸਥਿਰ ਪੂਰਨ ਪੰਨਾ_Not_On_Dirty_List(PCache *pCache, PgHdr *pPg){
  PgHdr *p;
  ਲਈ(p=pCache->pDirty; p; p=p->p_Dirty_ਅਗਲਾ){
    ਜੇ( p==pPg ) ਵਾਪਸ 0;
  }
  ਵਾਪਸ 1;
}
#ਨਹੀਂਤਾਂ
# define ਪੰਨਾ_On_Dirty_List(A,B)    1
# define ਪੰਨਾ_Not_On_Dirty_List(A,B) 1
#ਜੇ_ਅੰਤ

/*
** Check invariants on a PgHdr entry.  Return true if everything is OK.
** Return false if any invariant is violated.
**
** This routine is for use inside of assert() statements only.  For
** example:
**
**          assert( sqlite3PcachePageSanity(pPg) );
*/
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਪੂਰਨ sqlite3_Pcache_ਪੰਨਾ_Sanity(PgHdr *pPg){
  PCache *pCache;
  ਪ੍ਰਤਿਜ੍ਞਾ( pPg!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pPg->pgno>0 || pPg->pPager==0 );    /* Page number is 1 or more */
  pCache = pPg->pCache;
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache!=0 );      /* Every page has an associated PCache */
  ਜੇ( pPg->flags & PGHDR_CLEAN ){
    ਪ੍ਰਤਿਜ੍ਞਾ( (pPg->flags & PGHDR_DIRTY)==0 );/* Cannot be both CLEAN and DIRTY */
    ਪ੍ਰਤਿਜ੍ਞਾ( ਪੰਨਾ_Not_On_Dirty_List(pCache, pPg) );/* CLEAN pages not on dirtylist */
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( (pPg->flags & PGHDR_DIRTY)!=0 );/* If not CLEAN must be DIRTY */
    ਪ੍ਰਤਿਜ੍ਞਾ( pPg->p_Dirty_ਅਗਲਾ==0 || pPg->p_Dirty_ਅਗਲਾ->pDirtyPrev==pPg );
    ਪ੍ਰਤਿਜ੍ਞਾ( pPg->pDirtyPrev==0 || pPg->pDirtyPrev->p_Dirty_ਅਗਲਾ==pPg );
    ਪ੍ਰਤਿਜ੍ਞਾ( pPg->pDirtyPrev!=0 || pCache->pDirty==pPg );
    ਪ੍ਰਤਿਜ੍ਞਾ( ਪੰਨਾ_On_Dirty_List(pCache, pPg) );
  }
  /* WRITEABLE pages must also be DIRTY */
  ਜੇ( pPg->flags & PGHDR_WRITEABLE ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pPg->flags & PGHDR_DIRTY );     /* WRITEABLE implies DIRTY */
  }
  /* NEED_SYNC can be set independently of WRITEABLE.  This can happen,
  ** for example, when using the sqlite3PagerDontWrite() optimization:
  **    (1)  Page X is journalled, and gets WRITEABLE and NEED_SEEK.
  **    (2)  Page X moved to freelist, WRITEABLE is cleared
  **    (3)  Page X reused, WRITEABLE is set again
  ** If NEED_SYNC had been cleared in step 2, then it would not be reset
  ** in step 3, and page might be written into the database without first
  ** syncing the rollback journal, which might cause corruption on a power
  ** loss.
  **
  ** Another example is when the database page size is smaller than the
  ** disk sector size.  When any page of a sector is journalled, all pages
  ** in that sector are marked NEED_SYNC even if they are still CLEAN, just
  ** in case they are later modified, since all pages in the same sector
  ** must be journalled and synced before any of those pages can be safely
  ** written.
  */
  ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */


/********************************** Linked List Management ********************/

/* Allowed values for second argument to pcacheManageDirtyList() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCACHE_DIRTYLIST_REMOVE   1    /* Remove pPage from dirty list */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCACHE_DIRTYLIST_ਜੋੜੋ      2    /* Add pPage to the dirty list */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PCACHE_DIRTYLIST_FRONT    3    /* Move pPage to the front of the list */

/*
** Manage pPage's participation on the dirty list.  Bits of the addRemove
** argument determines what operation to do.  The 0x01 bit means first
** remove pPage from the dirty list.  The 0x02 means add pPage back to
** the dirty list.  Doing both moves pPage to the front of the dirty list.
*/
ਸਥਿਰ ਖਾਲੀ pcacheManageDirtyList(PgHdr *p_ਪੰਨਾ, u8 ਜੋੜੋ_Remove){
  PCache *p = p_ਪੰਨਾ->pCache;

  pcacheTrace(("%p.DIRTYLIST.%s %d\n", p,
                ਜੋੜੋ_Remove==1 ? "REMOVE" : ਜੋੜੋ_Remove==2 ? "ADD" : "FRONT",
                p_ਪੰਨਾ->pgno));
  ਜੇ( ਜੋੜੋ_Remove & PCACHE_DIRTYLIST_REMOVE ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->p_Dirty_ਅਗਲਾ || p_ਪੰਨਾ==p->p_Dirty_ਪੂਛ );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pDirtyPrev || p_ਪੰਨਾ==p->pDirty );
  
    /* Update the PCache1.pSynced variable if necessary. */
    ਜੇ( p->pSynced==p_ਪੰਨਾ ){
      p->pSynced = p_ਪੰਨਾ->pDirtyPrev;
    }
  
    ਜੇ( p_ਪੰਨਾ->p_Dirty_ਅਗਲਾ ){
      p_ਪੰਨਾ->p_Dirty_ਅਗਲਾ->pDirtyPrev = p_ਪੰਨਾ->pDirtyPrev;
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ==p->p_Dirty_ਪੂਛ );
      p->p_Dirty_ਪੂਛ = p_ਪੰਨਾ->pDirtyPrev;
    }
    ਜੇ( p_ਪੰਨਾ->pDirtyPrev ){
      p_ਪੰਨਾ->pDirtyPrev->p_Dirty_ਅਗਲਾ = p_ਪੰਨਾ->p_Dirty_ਅਗਲਾ;
    }ਨਹੀਂਤਾਂ{
      /* If there are now no dirty pages in the cache, set eCreate to 2. 
      ** This is an optimization that allows sqlite3PcacheFetch() to skip
      ** searching for a dirty page to eject from the cache when it might
      ** otherwise have to.  */
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ==p->pDirty );
      p->pDirty = p_ਪੰਨਾ->p_Dirty_ਅਗਲਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( p->bPurgeable || p->eCreate==2 );
      ਜੇ( p->pDirty==0 ){         /*OPTIMIZATION-IF-TRUE*/
        ਪ੍ਰਤਿਜ੍ਞਾ( p->bPurgeable==0 || p->eCreate==1 );
        p->eCreate = 2;
      }
    }
  }
  ਜੇ( ਜੋੜੋ_Remove & PCACHE_DIRTYLIST_ਜੋੜੋ ){
    p_ਪੰਨਾ->pDirtyPrev = 0;
    p_ਪੰਨਾ->p_Dirty_ਅਗਲਾ = p->pDirty;
    ਜੇ( p_ਪੰਨਾ->p_Dirty_ਅਗਲਾ ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->p_Dirty_ਅਗਲਾ->pDirtyPrev==0 );
      p_ਪੰਨਾ->p_Dirty_ਅਗਲਾ->pDirtyPrev = p_ਪੰਨਾ;
    }ਨਹੀਂਤਾਂ{
      p->p_Dirty_ਪੂਛ = p_ਪੰਨਾ;
      ਜੇ( p->bPurgeable ){
        ਪ੍ਰਤਿਜ੍ਞਾ( p->eCreate==2 );
        p->eCreate = 1;
      }
    }
    p->pDirty = p_ਪੰਨਾ;

    /* If pSynced is NULL and this page has a clear NEED_SYNC flag, set
    ** pSynced to point to it. Checking the NEED_SYNC flag is an 
    ** optimization, as if pSynced points to a page with the NEED_SYNC
    ** flag set sqlite3PcacheFetchStress() searches through all newer 
    ** entries of the dirty-list for a page with NEED_SYNC clear anyway.  */
    ਜੇ( !p->pSynced 
     && 0==(p_ਪੰਨਾ->flags&PGHDR_NEED_SYNC)   /*OPTIMIZATION-IF-FALSE*/
    ){
      p->pSynced = p_ਪੰਨਾ;
    }
  }
  pcacheDump(p);
}

/*
** Wrapper around the pluggable caches xUnpin method. If the cache is
** being used for an in-memory database, this function is a no-op.
*/
ਸਥਿਰ ਖਾਲੀ pcacheUnpin(PgHdr *p){
  ਜੇ( p->pCache->bPurgeable ){
    pcacheTrace(("%p.UNPIN %d\n", p->pCache, p->pgno));
    sqlite3GlobalConfig.pcache2.xUnpin(p->pCache->pCache, p->p_ਪੰਨਾ, 0);
    pcacheDump(p->pCache);
  }
}

/*
** Compute the number of pages of cache requested.   p->szCache is the
** cache size requested by the "PRAGMA cache_size" statement.
*/
ਸਥਿਰ ਪੂਰਨ ਸੰਖਿਆ_ਦਾ_Cache_ਪੰਨੇ(PCache *p){
  ਜੇ( p->szCache>=0 ){
    /* IMPLEMENTATION-OF: R-42059-47211 If the argument N is positive then the
    ** suggested cache size is set to N. */
    ਵਾਪਸ p->szCache;
  }ਨਹੀਂਤਾਂ{
    i64 n;
    /* IMPLEMENTATION-OF: R-59858-46238 If the argument N is negative, then the
    ** number of cache pages is adjusted to be a number of pages that would
    ** use approximately abs(N*1024) bytes of memory based on the current
    ** page size. */
    n = ((-1024*(i64)p->szCache)/(p->sz_ਪੰਨਾ+p->szExtra));
    ਜੇ( n>1000000000 ) n = 1000000000;
    ਵਾਪਸ (ਪੂਰਨ)n;
  }
}

/*************************************************** General Interfaces ******
**
** Initialize and shutdown the page cache subsystem. Neither of these 
** functions are threadsafe.
*/
ਪੂਰਨ sqlite3PcacheInitialize(ਖਾਲੀ){
  ਜੇ( sqlite3GlobalConfig.pcache2.xInit==0 ){
    /* IMPLEMENTATION-OF: R-26801-64137 If the xInit() method is NULL, then the
    ** built-in default page cache is used instead of the application defined
    ** page cache. */
    sqlite3_PCache_ਸੈੱਟ_Default();
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3GlobalConfig.pcache2.xInit!=0 );
  }
  ਵਾਪਸ sqlite3GlobalConfig.pcache2.xInit(sqlite3GlobalConfig.pcache2.pArg);
}
ਖਾਲੀ sqlite3PcacheShutdown(ਖਾਲੀ){
  ਜੇ( sqlite3GlobalConfig.pcache2.xShutdown ){
    /* IMPLEMENTATION-OF: R-26000-56589 The xShutdown() method may be NULL. */
    sqlite3GlobalConfig.pcache2.xShutdown(sqlite3GlobalConfig.pcache2.pArg);
  }
}

/*
** Return the size in bytes of a PCache object.
*/
ਪੂਰਨ sqlite3_Pcache_ਮਾਪ(ਖਾਲੀ){ ਵਾਪਸ ਆਕਾਰ(PCache); }

/*
** Create a new PCache object. Storage space to hold the object
** has already been allocated and is passed in as the p pointer. 
** The caller discovers how much space needs to be allocated by 
** calling sqlite3PcacheSize().
**
** szExtra is some extra space allocated for each page.  The first
** 8 bytes of the extra space will be zeroed as the page is allocated,
** but remaining content will be uninitialized.  Though it is opaque
** to this module, the extra space really ends up being the MemPage
** structure in the pager.
*/
ਪੂਰਨ sqlite3PcacheOpen(
  ਪੂਰਨ sz_ਪੰਨਾ,                  /* Size of every page */
  ਪੂਰਨ szExtra,                 /* Extra space associated with each page */
  ਪੂਰਨ bPurgeable,              /* True if pages are on backing store */
  ਪੂਰਨ (*xStress)(ਖਾਲੀ*,PgHdr*),/* Call to try to make pages clean */
  ਖਾਲੀ *pStress,               /* Argument to xStress */
  PCache *p                    /* Preallocated space for the PCache */
){
  memset(p, 0, ਆਕਾਰ(PCache));
  p->sz_ਪੰਨਾ = 1;
  p->szExtra = szExtra;
  ਪ੍ਰਤਿਜ੍ਞਾ( szExtra>=8 );  /* First 8 bytes will be zeroed */
  p->bPurgeable = bPurgeable;
  p->eCreate = 2;
  p->xStress = xStress;
  p->pStress = pStress;
  p->szCache = 100;
  p->szSpill = 1;
  pcacheTrace(("%p.OPEN szPage %d bPurgeable %d\n",p,sz_ਪੰਨਾ,bPurgeable));
  ਵਾਪਸ sqlite3_Pcache_ਸੈੱਟ_ਪੰਨਾ_ਮਾਪ(p, sz_ਪੰਨਾ);
}

/*
** Change the page size for PCache object. The caller must ensure that there
** are no outstanding page references when this function is called.
*/
ਪੂਰਨ sqlite3_Pcache_ਸੈੱਟ_ਪੰਨਾ_ਮਾਪ(PCache *pCache, ਪੂਰਨ sz_ਪੰਨਾ){
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->n_Ref_ਜੋੜ==0 && pCache->pDirty==0 );
  ਜੇ( pCache->sz_ਪੰਨਾ ){
    sqlite3_pcache *pNew;
    pNew = sqlite3GlobalConfig.pcache2.xCreate(
                sz_ਪੰਨਾ, pCache->szExtra + ROUND8(ਆਕਾਰ(PgHdr)),
                pCache->bPurgeable
    );
    ਜੇ( pNew==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
    sqlite3GlobalConfig.pcache2.xCachesize(pNew, ਸੰਖਿਆ_ਦਾ_Cache_ਪੰਨੇ(pCache));
    ਜੇ( pCache->pCache ){
      sqlite3GlobalConfig.pcache2.xDestroy(pCache->pCache);
    }
    pCache->pCache = pNew;
    pCache->sz_ਪੰਨਾ = sz_ਪੰਨਾ;
    pcacheTrace(("%p.PAGESIZE %d\n",pCache,sz_ਪੰਨਾ));
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Try to obtain a page from the cache.
**
** This routine returns a pointer to an sqlite3_pcache_page object if
** such an object is already in cache, or if a new one is created.
** This routine returns a NULL pointer if the object was not in cache
** and could not be created.
**
** The createFlags should be 0 to check for existing pages and should
** be 3 (not 1, but 3) to try to create a new page.
**
** If the createFlag is 0, then NULL is always returned if the page
** is not already in the cache.  If createFlag is 1, then a new page
** is created only if that can be done without spilling dirty pages
** and without exceeding the cache size limit.
**
** The caller needs to invoke sqlite3PcacheFetchFinish() to properly
** initialize the sqlite3_pcache_page object and convert it into a
** PgHdr object.  The sqlite3PcacheFetch() and sqlite3PcacheFetchFinish()
** routines are split this way for performance reasons. When separated
** they can both (usually) operate without having to push values to
** the stack on entry and pop them back off on exit, which saves a
** lot of pushing and popping.
*/
sqlite3_pcache_ਪੰਨਾ *sqlite3PcacheFetch(
  PCache *pCache,       /* Obtain the page from this cache */
  Pgno pgno,            /* Page number to obtain */
  ਪੂਰਨ createFlag        /* If true, create page if it does not exist already */
){
  ਪੂਰਨ eCreate;
  sqlite3_pcache_ਪੰਨਾ *pRes;

  ਪ੍ਰਤਿਜ੍ਞਾ( pCache!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->pCache!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( createFlag==3 || createFlag==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->eCreate==((pCache->bPurgeable && pCache->pDirty) ? 1 : 2) );

  /* eCreate defines what to do if the page does not exist.
  **    0     Do not allocate a new page.  (createFlag==0)
  **    1     Allocate a new page if doing so is inexpensive.
  **          (createFlag==1 AND bPurgeable AND pDirty)
  **    2     Allocate a new page even it doing so is difficult.
  **          (createFlag==1 AND !(bPurgeable AND pDirty)
  */
  eCreate = createFlag & pCache->eCreate;
  ਪ੍ਰਤਿਜ੍ਞਾ( eCreate==0 || eCreate==1 || eCreate==2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( createFlag==0 || pCache->eCreate==eCreate );
  ਪ੍ਰਤਿਜ੍ਞਾ( createFlag==0 || eCreate==1+(!pCache->bPurgeable||!pCache->pDirty) );
  pRes = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, pgno, eCreate);
  pcacheTrace(("%p.FETCH %d%s (result: %p) ",pCache,pgno,
               createFlag?" create":"",pRes));
  pcache_ਪੰਨਾ_Trace(pgno, pRes);
  ਵਾਪਸ pRes;
}

/*
** If the sqlite3PcacheFetch() routine is unable to allocate a new
** page because no clean pages are available for reuse and the cache
** size limit has been reached, then this routine can be invoked to 
** try harder to allocate a page.  This routine might invoke the stress
** callback to spill dirty pages to the journal.  It will then try to
** allocate the new page and will only fail to allocate a new page on
** an OOM error.
**
** This routine should be invoked only after sqlite3PcacheFetch() fails.
*/
ਪੂਰਨ sqlite3PcacheFetchStress(
  PCache *pCache,                 /* Obtain the page from this cache */
  Pgno pgno,                      /* Page number to obtain */
  sqlite3_pcache_ਪੰਨਾ **pp_ਪੰਨਾ    /* Write result here */
){
  PgHdr *pPg;
  ਜੇ( pCache->eCreate==2 ) ਵਾਪਸ 0;

  ਜੇ( sqlite3PcachePagecount(pCache)>pCache->szSpill ){
    /* Find a dirty page to write-out and recycle. First try to find a 
    ** page that does not require a journal-sync (one with PGHDR_NEED_SYNC
    ** cleared), but if that is not possible settle for any other 
    ** unreferenced dirty page.
    **
    ** If the LRU page in the dirty list that has a clear PGHDR_NEED_SYNC
    ** flag is currently referenced, then the following may leave pSynced
    ** set incorrectly (pointing to other than the LRU page with NEED_SYNC
    ** cleared). This is Ok, as pSynced is just an optimization.  */
    ਲਈ(pPg=pCache->pSynced; 
        pPg && (pPg->nRef || (pPg->flags&PGHDR_NEED_SYNC)); 
        pPg=pPg->pDirtyPrev
    );
    pCache->pSynced = pPg;
    ਜੇ( !pPg ){
      ਲਈ(pPg=pCache->p_Dirty_ਪੂਛ; pPg && pPg->nRef; pPg=pPg->pDirtyPrev);
    }
    ਜੇ( pPg ){
      ਪੂਰਨ rc;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_LOG_CACHE_SPILL
      sqlite3_log(SQLITE_ਭਰਿਆ, 
                  "spill page %d making room for %d - cache used: %d/%d",
                  pPg->pgno, pgno,
                  sqlite3GlobalConfig.pcache2.xPagecount(pCache->pCache),
                ਸੰਖਿਆ_ਦਾ_Cache_ਪੰਨੇ(pCache));
#ਜੇ_ਅੰਤ
      pcacheTrace(("%p.SPILL %d\n",pCache,pPg->pgno));
      rc = pCache->xStress(pCache->pStress, pPg);
      pcacheDump(pCache);
      ਜੇ( rc!=SQLITE_OK && rc!=SQLITE_BUSY ){
        ਵਾਪਸ rc;
      }
    }
  }
  *pp_ਪੰਨਾ = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, pgno, 2);
  ਵਾਪਸ *pp_ਪੰਨਾ==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
}

/*
** This is a helper routine for sqlite3PcacheFetchFinish()
**
** In the uncommon case where the page being fetched has not been
** initialized, this routine is invoked to do the initialization.
** This routine is broken out into a separate function since it
** requires extra stack manipulation that can be avoided in the common
** case.
*/
ਸਥਿਰ SQLITE_NOINLINE PgHdr *pcache_Fetch_ਅੰਤ_With_Init(
  PCache *pCache,             /* Obtain the page from this cache */
  Pgno pgno,                  /* Page number obtained */
  sqlite3_pcache_ਪੰਨਾ *p_ਪੰਨਾ  /* Page obtained by prior PcacheFetch() call */
){
  PgHdr *pPgHdr;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ!=0 );
  pPgHdr = (PgHdr*)p_ਪੰਨਾ->pExtra;
  ਪ੍ਰਤਿਜ੍ਞਾ( pPgHdr->p_ਪੰਨਾ==0 );
  memset(&pPgHdr->pDirty, 0, ਆਕਾਰ(PgHdr) - offsetof(PgHdr,pDirty));
  pPgHdr->p_ਪੰਨਾ = p_ਪੰਨਾ;
  pPgHdr->p_ਅੰਕੜੇ = p_ਪੰਨਾ->pBuf;
  pPgHdr->pExtra = (ਖਾਲੀ *)&pPgHdr[1];
  memset(pPgHdr->pExtra, 0, 8);
  ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT( pPgHdr->pExtra ) );
  pPgHdr->pCache = pCache;
  pPgHdr->pgno = pgno;
  pPgHdr->flags = PGHDR_CLEAN;
  ਵਾਪਸ sqlite3_Pcache_Fetch_ਅੰਤ(pCache,pgno,p_ਪੰਨਾ);
}

/*
** This routine converts the sqlite3_pcache_page object returned by
** sqlite3PcacheFetch() into an initialized PgHdr object.  This routine
** must be called after sqlite3PcacheFetch() in order to get a usable
** result.
*/
PgHdr *sqlite3_Pcache_Fetch_ਅੰਤ(
  PCache *pCache,             /* Obtain the page from this cache */
  Pgno pgno,                  /* Page number obtained */
  sqlite3_pcache_ਪੰਨਾ *p_ਪੰਨਾ  /* Page obtained by prior PcacheFetch() call */
){
  PgHdr *pPgHdr;

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ!=0 );
  pPgHdr = (PgHdr *)p_ਪੰਨਾ->pExtra;

  ਜੇ( !pPgHdr->p_ਪੰਨਾ ){
    ਵਾਪਸ pcache_Fetch_ਅੰਤ_With_Init(pCache, pgno, p_ਪੰਨਾ);
  }
  pCache->n_Ref_ਜੋੜ++;
  pPgHdr->nRef++;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pcache_ਪੰਨਾ_Sanity(pPgHdr) );
  ਵਾਪਸ pPgHdr;
}

/*
** Decrement the reference count on a page. If the page is clean and the
** reference count drops to 0, then it is made eligible for recycling.
*/
ਖਾਲੀ SQLITE_NOINLINE sqlite3PcacheRelease(PgHdr *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nRef>0 );
  p->pCache->n_Ref_ਜੋੜ--;
  ਜੇ( (--p->nRef)==0 ){
    ਜੇ( p->flags&PGHDR_CLEAN ){
      pcacheUnpin(p);
    }ਨਹੀਂਤਾਂ{
      pcacheManageDirtyList(p, PCACHE_DIRTYLIST_FRONT);
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pcache_ਪੰਨਾ_Sanity(p) );
    }
  }
}

/*
** Increase the reference count of a supplied page by 1.
*/
ਖਾਲੀ sqlite3PcacheRef(PgHdr *p){
  ਪ੍ਰਤਿਜ੍ਞਾ(p->nRef>0);
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pcache_ਪੰਨਾ_Sanity(p) );
  p->nRef++;
  p->pCache->n_Ref_ਜੋੜ++;
}

/*
** Drop a page from the cache. There must be exactly one reference to the
** page. This function deletes that reference, so after it returns the
** page pointed to by p is invalid.
*/
ਖਾਲੀ sqlite3PcacheDrop(PgHdr *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nRef==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pcache_ਪੰਨਾ_Sanity(p) );
  ਜੇ( p->flags&PGHDR_DIRTY ){
    pcacheManageDirtyList(p, PCACHE_DIRTYLIST_REMOVE);
  }
  p->pCache->n_Ref_ਜੋੜ--;
  sqlite3GlobalConfig.pcache2.xUnpin(p->pCache->pCache, p->p_ਪੰਨਾ, 1);
}

/*
** Make sure the page is marked as dirty. If it isn't dirty already,
** make it so.
*/
ਖਾਲੀ sqlite3PcacheMakeDirty(PgHdr *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nRef>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pcache_ਪੰਨਾ_Sanity(p) );
  ਜੇ( p->flags & (PGHDR_CLEAN|PGHDR_DONT_WRITE) ){    /*OPTIMIZATION-IF-FALSE*/
    p->flags &= ~PGHDR_DONT_WRITE;
    ਜੇ( p->flags & PGHDR_CLEAN ){
      p->flags ^= (PGHDR_DIRTY|PGHDR_CLEAN);
      pcacheTrace(("%p.DIRTY %d\n",p->pCache,p->pgno));
      ਪ੍ਰਤਿਜ੍ਞਾ( (p->flags & (PGHDR_DIRTY|PGHDR_CLEAN))==PGHDR_DIRTY );
      pcacheManageDirtyList(p, PCACHE_DIRTYLIST_ਜੋੜੋ);
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pcache_ਪੰਨਾ_Sanity(p) );
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pcache_ਪੰਨਾ_Sanity(p) );
  }
}

/*
** Make sure the page is marked as clean. If it isn't clean already,
** make it so.
*/
ਖਾਲੀ sqlite3PcacheMakeClean(PgHdr *p){
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pcache_ਪੰਨਾ_Sanity(p) );
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->flags & PGHDR_DIRTY)!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->flags & PGHDR_CLEAN)==0 );
  pcacheManageDirtyList(p, PCACHE_DIRTYLIST_REMOVE);
  p->flags &= ~(PGHDR_DIRTY|PGHDR_NEED_SYNC|PGHDR_WRITEABLE);
  p->flags |= PGHDR_CLEAN;
  pcacheTrace(("%p.CLEAN %d\n",p->pCache,p->pgno));
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pcache_ਪੰਨਾ_Sanity(p) );
  ਜੇ( p->nRef==0 ){
    pcacheUnpin(p);
  }
}

/*
** Make every page in the cache clean.
*/
ਖਾਲੀ sqlite3PcacheCleanAll(PCache *pCache){
  PgHdr *p;
  pcacheTrace(("%p.CLEAN-ALL\n",pCache));
  ਜਦੋਂ( (p = pCache->pDirty)!=0 ){
    sqlite3PcacheMakeClean(p);
  }
}

/*
** Clear the PGHDR_NEED_SYNC and PGHDR_WRITEABLE flag from all dirty pages.
*/
ਖਾਲੀ sqlite3PcacheClearWritable(PCache *pCache){
  PgHdr *p;
  pcacheTrace(("%p.CLEAR-WRITEABLE\n",pCache));
  ਲਈ(p=pCache->pDirty; p; p=p->p_Dirty_ਅਗਲਾ){
    p->flags &= ~(PGHDR_NEED_SYNC|PGHDR_WRITEABLE);
  }
  pCache->pSynced = pCache->p_Dirty_ਪੂਛ;
}

/*
** Clear the PGHDR_NEED_SYNC flag from all dirty pages.
*/
ਖਾਲੀ sqlite3PcacheClearSyncFlags(PCache *pCache){
  PgHdr *p;
  ਲਈ(p=pCache->pDirty; p; p=p->p_Dirty_ਅਗਲਾ){
    p->flags &= ~PGHDR_NEED_SYNC;
  }
  pCache->pSynced = pCache->p_Dirty_ਪੂਛ;
}

/*
** Change the page number of page p to newPgno. 
*/
ਖਾਲੀ sqlite3PcacheMove(PgHdr *p, Pgno newPgno){
  PCache *pCache = p->pCache;
  sqlite3_pcache_ਪੰਨਾ *pOther;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->nRef>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( newPgno>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pcache_ਪੰਨਾ_Sanity(p) );
  pcacheTrace(("%p.MOVE %d -> %d\n",pCache,p->pgno,newPgno));
  pOther = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, newPgno, 0);
  ਜੇ( pOther ){
    PgHdr *pXPage = (PgHdr*)pOther->pExtra;
    ਪ੍ਰਤਿਜ੍ਞਾ( pXPage->nRef==0 );
    pXPage->nRef++;
    pCache->n_Ref_ਜੋੜ++;
    sqlite3PcacheDrop(pXPage);
  }
  sqlite3GlobalConfig.pcache2.xRekey(pCache->pCache, p->p_ਪੰਨਾ, p->pgno,newPgno);
  p->pgno = newPgno;
  ਜੇ( (p->flags&PGHDR_DIRTY) && (p->flags&PGHDR_NEED_SYNC) ){
    pcacheManageDirtyList(p, PCACHE_DIRTYLIST_FRONT);
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Pcache_ਪੰਨਾ_Sanity(p) );
  }
}

/*
** Drop every cache entry whose page number is greater than "pgno". The
** caller must ensure that there are no outstanding references to any pages
** other than page 1 with a page number greater than pgno.
**
** If there is a reference to page 1 and the pgno parameter passed to this
** function is 0, then the data area associated with page 1 is zeroed, but
** the page object is not dropped.
*/
ਖਾਲੀ sqlite3PcacheTruncate(PCache *pCache, Pgno pgno){
  ਜੇ( pCache->pCache ){
    PgHdr *p;
    PgHdr *p_ਅਗਲਾ;
    pcacheTrace(("%p.TRUNCATE %d\n",pCache,pgno));
    ਲਈ(p=pCache->pDirty; p; p=p_ਅਗਲਾ){
      p_ਅਗਲਾ = p->p_Dirty_ਅਗਲਾ;
      /* This routine never gets call with a positive pgno except right
      ** after sqlite3PcacheCleanAll().  So if there are dirty pages,
      ** it must be that pgno==0.
      */
      ਪ੍ਰਤਿਜ੍ਞਾ( p->pgno>0 );
      ਜੇ( p->pgno>pgno ){
        ਪ੍ਰਤਿਜ੍ਞਾ( p->flags&PGHDR_DIRTY );
        sqlite3PcacheMakeClean(p);
      }
    }
    ਜੇ( pgno==0 && pCache->n_Ref_ਜੋੜ ){
      sqlite3_pcache_ਪੰਨਾ *pPage1;
      pPage1 = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache,1,0);
      ਜੇ( ALWAYS(pPage1) ){  /* Page 1 is always available in cache, because
                             ** pCache->nRefSum>0 */
        memset(pPage1->pBuf, 0, pCache->sz_ਪੰਨਾ);
        pgno = 1;
      }
    }
    sqlite3GlobalConfig.pcache2.xTruncate(pCache->pCache, pgno+1);
  }
}

/*
** Close a cache.
*/
ਖਾਲੀ sqlite3PcacheClose(PCache *pCache){
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->pCache!=0 );
  pcacheTrace(("%p.CLOSE\n",pCache));
  sqlite3GlobalConfig.pcache2.xDestroy(pCache->pCache);
}

/* 
** Discard the contents of the cache.
*/
ਖਾਲੀ sqlite3PcacheClear(PCache *pCache){
  sqlite3PcacheTruncate(pCache, 0);
}

/*
** Merge two lists of pages connected by pDirty and in pgno order.
** Do not bother fixing the pDirtyPrev pointers.
*/
ਸਥਿਰ PgHdr *pcache_ਮਿਲਾਓ_Dirty_List(PgHdr *pA, PgHdr *pB){
  PgHdr ਨਤੀਜਾ, *p_ਪੂਛ;
  p_ਪੂਛ = &ਨਤੀਜਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pA!=0 && pB!=0 );
  ਲਈ(;;){
    ਜੇ( pA->pgno<pB->pgno ){
      p_ਪੂਛ->pDirty = pA;
      p_ਪੂਛ = pA;
      pA = pA->pDirty;
      ਜੇ( pA==0 ){
        p_ਪੂਛ->pDirty = pB;
        ਰੋਕੋ;
      }
    }ਨਹੀਂਤਾਂ{
      p_ਪੂਛ->pDirty = pB;
      p_ਪੂਛ = pB;
      pB = pB->pDirty;
      ਜੇ( pB==0 ){
        p_ਪੂਛ->pDirty = pA;
        ਰੋਕੋ;
      }
    }
  }
  ਵਾਪਸ ਨਤੀਜਾ.pDirty;
}

/*
** Sort the list of pages in ascending order by pgno.  Pages are
** connected by pDirty pointers.  The pDirtyPrev pointers are
** corrupted by this sort.
**
** Since there cannot be more than 2^31 distinct pages in a database,
** there cannot be more than 31 buckets required by the merge sorter.
** One extra bucket is added to catch overflow in case something
** ever changes to make the previous sentence incorrect.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ N_ਛਾਂਟੋ_ਬਾਲਟੀ  32
ਸਥਿਰ PgHdr *pcache_ਛਾਂਟੋ_Dirty_List(PgHdr *p_ਵਿੱਚ){
  PgHdr *a[N_ਛਾਂਟੋ_ਬਾਲਟੀ], *p;
  ਪੂਰਨ i;
  memset(a, 0, ਆਕਾਰ(a));
  ਜਦੋਂ( p_ਵਿੱਚ ){
    p = p_ਵਿੱਚ;
    p_ਵਿੱਚ = p->pDirty;
    p->pDirty = 0;
    ਲਈ(i=0; ALWAYS(i<N_ਛਾਂਟੋ_ਬਾਲਟੀ-1); i++){
      ਜੇ( a[i]==0 ){
        a[i] = p;
        ਰੋਕੋ;
      }ਨਹੀਂਤਾਂ{
        p = pcache_ਮਿਲਾਓ_Dirty_List(a[i], p);
        a[i] = 0;
      }
    }
    ਜੇ( NEVER(i==N_ਛਾਂਟੋ_ਬਾਲਟੀ-1) ){
      /* To get here, there need to be 2^(N_SORT_BUCKET) elements in
      ** the input list.  But that is impossible.
      */
      a[i] = pcache_ਮਿਲਾਓ_Dirty_List(a[i], p);
    }
  }
  p = a[0];
  ਲਈ(i=1; i<N_ਛਾਂਟੋ_ਬਾਲਟੀ; i++){
    ਜੇ( a[i]==0 ) ਜਾਰੀ;
    p = p ? pcache_ਮਿਲਾਓ_Dirty_List(p, a[i]) : a[i];
  }
  ਵਾਪਸ p;
}

/*
** Return a list of all dirty pages in the cache, sorted by page number.
*/
PgHdr *sqlite3PcacheDirtyList(PCache *pCache){
  PgHdr *p;
  ਲਈ(p=pCache->pDirty; p; p=p->p_Dirty_ਅਗਲਾ){
    p->pDirty = p->p_Dirty_ਅਗਲਾ;
  }
  ਵਾਪਸ pcache_ਛਾਂਟੋ_Dirty_List(pCache->pDirty);
}

/* 
** Return the total number of references to all pages held by the cache.
**
** This is not the total number of pages referenced, but the sum of the
** reference count for all pages.
*/
i64 sqlite3_Pcache_Ref_ਗਿਣਤੀ(PCache *pCache){
  ਵਾਪਸ pCache->n_Ref_ਜੋੜ;
}

/*
** Return the number of references to the page supplied as an argument.
*/
i64 sqlite3_Pcache_ਪੰਨਾ_Refcount(PgHdr *p){
  ਵਾਪਸ p->nRef;
}

/* 
** Return the total number of pages in the cache.
*/
ਪੂਰਨ sqlite3PcachePagecount(PCache *pCache){
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->pCache!=0 );
  ਵਾਪਸ sqlite3GlobalConfig.pcache2.xPagecount(pCache->pCache);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
/*
** Get the suggested cache-size value.
*/
ਪੂਰਨ sqlite3PcacheGetCachesize(PCache *pCache){
  ਵਾਪਸ ਸੰਖਿਆ_ਦਾ_Cache_ਪੰਨੇ(pCache);
}
#ਜੇ_ਅੰਤ

/*
** Set the suggested cache-size value.
*/
ਖਾਲੀ sqlite3_Pcache_ਸੈੱਟ_Cachesize(PCache *pCache, ਪੂਰਨ mx_ਪੰਨਾ){
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->pCache!=0 );
  pCache->szCache = mx_ਪੰਨਾ;
  sqlite3GlobalConfig.pcache2.xCachesize(pCache->pCache,
                                         ਸੰਖਿਆ_ਦਾ_Cache_ਪੰਨੇ(pCache));
}

/*
** Set the suggested cache-spill value.  Make no changes if if the
** argument is zero.  Return the effective cache-spill size, which will
** be the larger of the szSpill and szCache.
*/
ਪੂਰਨ sqlite3_Pcache_ਸੈੱਟ_Spillsize(PCache *p, ਪੂਰਨ mx_ਪੰਨਾ){
  ਪੂਰਨ res;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pCache!=0 );
  ਜੇ( mx_ਪੰਨਾ ){
    ਜੇ( mx_ਪੰਨਾ<0 ){
      mx_ਪੰਨਾ = (ਪੂਰਨ)((-1024*(i64)mx_ਪੰਨਾ)/(p->sz_ਪੰਨਾ+p->szExtra));
    }
    p->szSpill = mx_ਪੰਨਾ;
  }
  res = ਸੰਖਿਆ_ਦਾ_Cache_ਪੰਨੇ(p);
  ਜੇ( res<p->szSpill ) res = p->szSpill; 
  ਵਾਪਸ res;
}

/*
** Free up as much memory as possible from the page cache.
*/
ਖਾਲੀ sqlite3PcacheShrink(PCache *pCache){
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->pCache!=0 );
  sqlite3GlobalConfig.pcache2.xShrink(pCache->pCache);
}

/*
** Return the size of the header added by this middleware layer
** in the page-cache hierarchy.
*/
ਪੂਰਨ sqlite3_ਸਿਰਲੇਖ_ਮਾਪ_Pcache(ਖਾਲੀ){ ਵਾਪਸ ROUND8(ਆਕਾਰ(PgHdr)); }

/*
** Return the number of dirty pages currently in the cache, as a percentage
** of the configured cache size.
*/
ਪੂਰਨ sqlite3PCachePercentDirty(PCache *pCache){
  PgHdr *pDirty;
  ਪੂਰਨ nDirty = 0;
  ਪੂਰਨ nCache = ਸੰਖਿਆ_ਦਾ_Cache_ਪੰਨੇ(pCache);
  ਲਈ(pDirty=pCache->pDirty; pDirty; pDirty=pDirty->p_Dirty_ਅਗਲਾ) nDirty++;
  ਵਾਪਸ nCache ? (ਪੂਰਨ)(((i64)nDirty * 100) / nCache) : 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ਸਿੱਧਾ_ਉਛਾਲ_READ
/* 
** Return true if there are one or more dirty pages in the cache. Else false.
*/
ਪੂਰਨ sqlite3PCacheIsDirty(PCache *pCache){
  ਵਾਪਸ (pCache->pDirty!=0);
}
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_CHECK_ਪੰਨੇ) || defined(SQLITE_DEBUG)
/*
** For all dirty pages currently in the cache, invoke the specified
** callback. This is only used if the SQLITE_CHECK_PAGES macro is
** defined.
*/
ਖਾਲੀ sqlite3PcacheIterateDirty(PCache *pCache, ਖਾਲੀ (*xIter)(PgHdr *)){
  PgHdr *pDirty;
  ਲਈ(pDirty=pCache->pDirty; pDirty; pDirty=pDirty->p_Dirty_ਅਗਲਾ){
    xIter(pDirty);
  }
}
#ਜੇ_ਅੰਤ
