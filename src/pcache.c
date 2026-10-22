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
#అన్తర్భావ "sqliteInt.h"

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
నిర్మాణం PCache {
  PgHdr *pDirty, *p_Dirty_తోక;         /* List of dirty pages in LRU order */
  PgHdr *pSynced;                     /* Last synced page in dirty page list */
  i64 n_Ref_కూడిక;                        /* Sum of ref counts over all pages */
  పూర్ణాంకం szCache;                        /* Configured cache size */
  పూర్ణాంకం szSpill;                        /* Size before spilling occurs */
  పూర్ణాంకం sz_పుట;                         /* Size of every page in this cache */
  పూర్ణాంకం szExtra;                        /* Size of extra space for each page */
  u8 bPurgeable;                      /* True if pages are on backing store */
  u8 eCreate;                         /* eCreate value for for xFetch() */
  పూర్ణాంకం (*xStress)(శూన్య*,PgHdr*);       /* Call to try make a page clean */
  శూన్య *pStress;                      /* Argument to xStress */
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
#ఒకవేళ defined(SQLITE_DEBUG) && 0
  పూర్ణాంకం sqlite3PcacheTrace = 2;       /* 0: off  1: simple  2: cache dumps */
  పూర్ణాంకం sqlite3PcacheMxDump = 9999;   /* Max cache entries for pcacheDump() */
# define pcacheTrace(X) ఒకవేళ(sqlite3PcacheTrace){sqlite3DebugPrintf X;}
  స్థిర శూన్య pcache_పుట_Trace(పూర్ణాంకం i, sqlite3_pcache_పుట *p_క్రింది){
    PgHdr *pPg;
    గుర్తులేని అక్షరం *a;
    పూర్ణాంకం j;
    ఒకవేళ( p_క్రింది==0 ){
      రాయి("%3d: NULL\n", i);
    }లేకపోతే{
      pPg = (PgHdr*)p_క్రింది->pExtra;
      రాయి("%3d: nRef %2lld flgs %02x data ", i, pPg->nRef, pPg->flags);
      a = (గుర్తులేని అక్షరం *)p_క్రింది->pBuf;
      కొరకు(j=0; j<12; j++) రాయి("%02x", a[j]);
      రాయి(" ptr %p\n", pPg);
    }
  }
  స్థిర శూన్య pcacheDump(PCache *pCache){
    పూర్ణాంకం N;
    పూర్ణాంకం i;
    sqlite3_pcache_పుట *p_క్రింది;
  
    ఒకవేళ( sqlite3PcacheTrace<2 ) తిరిగి;
    ఒకవేళ( pCache->pCache==0 ) తిరిగి;
    N = sqlite3PcachePagecount(pCache);
    ఒకవేళ( N>sqlite3PcacheMxDump ) N = sqlite3PcacheMxDump;
    కొరకు(i=1; i<=N; i++){
       p_క్రింది = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, i, 0);
       pcache_పుట_Trace(i, p_క్రింది);
       ఒకవేళ( p_క్రింది && ((PgHdr*)p_క్రింది)->p_పుట==0 ){
         sqlite3GlobalConfig.pcache2.xUnpin(pCache->pCache, p_క్రింది, 0);
       }
    }
  }
#లేకపోతే
# define pcacheTrace(X)
# define pcache_పుట_Trace(PGNO, X)
# define pcacheDump(X)
#అయితే_ముగింపు

/*
** Return 1 if pPg is on the dirty list for pCache.  Return 0 if not.
** This routine runs inside of assert() statements only.
*/
#ఒకవేళ defined(SQLITE_ENABLE_EXPENSIVE_ASSERT)
స్థిర పూర్ణాంకం పుట_On_Dirty_List(PCache *pCache, PgHdr *pPg){
  PgHdr *p;
  కొరకు(p=pCache->pDirty; p; p=p->p_Dirty_తదుపరి){
    ఒకవేళ( p==pPg ) తిరిగి 1;
  }
  తిరిగి 0;
}
స్థిర పూర్ణాంకం పుట_Not_On_Dirty_List(PCache *pCache, PgHdr *pPg){
  PgHdr *p;
  కొరకు(p=pCache->pDirty; p; p=p->p_Dirty_తదుపరి){
    ఒకవేళ( p==pPg ) తిరిగి 0;
  }
  తిరిగి 1;
}
#లేకపోతే
# define పుట_On_Dirty_List(A,B)    1
# define పుట_Not_On_Dirty_List(A,B) 1
#అయితే_ముగింపు

/*
** Check invariants on a PgHdr entry.  Return true if everything is OK.
** Return false if any invariant is violated.
**
** This routine is for use inside of assert() statements only.  For
** example:
**
**          assert( sqlite3PcachePageSanity(pPg) );
*/
#నిర్వచితమైతే SQLITE_DEBUG
పూర్ణాంకం sqlite3_Pcache_పుట_Sanity(PgHdr *pPg){
  PCache *pCache;
  ప్రతిజ్ఞా( pPg!=0 );
  ప్రతిజ్ఞా( pPg->pgno>0 || pPg->pPager==0 );    /* Page number is 1 or more */
  pCache = pPg->pCache;
  ప్రతిజ్ఞా( pCache!=0 );      /* Every page has an associated PCache */
  ఒకవేళ( pPg->flags & PGHDR_CLEAN ){
    ప్రతిజ్ఞా( (pPg->flags & PGHDR_DIRTY)==0 );/* Cannot be both CLEAN and DIRTY */
    ప్రతిజ్ఞా( పుట_Not_On_Dirty_List(pCache, pPg) );/* CLEAN pages not on dirtylist */
  }లేకపోతే{
    ప్రతిజ్ఞా( (pPg->flags & PGHDR_DIRTY)!=0 );/* If not CLEAN must be DIRTY */
    ప్రతిజ్ఞా( pPg->p_Dirty_తదుపరి==0 || pPg->p_Dirty_తదుపరి->pDirtyPrev==pPg );
    ప్రతిజ్ఞా( pPg->pDirtyPrev==0 || pPg->pDirtyPrev->p_Dirty_తదుపరి==pPg );
    ప్రతిజ్ఞా( pPg->pDirtyPrev!=0 || pCache->pDirty==pPg );
    ప్రతిజ్ఞా( పుట_On_Dirty_List(pCache, pPg) );
  }
  /* WRITEABLE pages must also be DIRTY */
  ఒకవేళ( pPg->flags & PGHDR_WRITEABLE ){
    ప్రతిజ్ఞా( pPg->flags & PGHDR_DIRTY );     /* WRITEABLE implies DIRTY */
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
  తిరిగి 1;
}
#అయితే_ముగింపు /* SQLITE_DEBUG */


/********************************** Linked List Management ********************/

/* Allowed values for second argument to pcacheManageDirtyList() */
#నిర్వచించు PCACHE_DIRTYLIST_REMOVE   1    /* Remove pPage from dirty list */
#నిర్వచించు PCACHE_DIRTYLIST_కూడు      2    /* Add pPage to the dirty list */
#నిర్వచించు PCACHE_DIRTYLIST_FRONT    3    /* Move pPage to the front of the list */

/*
** Manage pPage's participation on the dirty list.  Bits of the addRemove
** argument determines what operation to do.  The 0x01 bit means first
** remove pPage from the dirty list.  The 0x02 means add pPage back to
** the dirty list.  Doing both moves pPage to the front of the dirty list.
*/
స్థిర శూన్య pcacheManageDirtyList(PgHdr *p_పుట, u8 కూడు_Remove){
  PCache *p = p_పుట->pCache;

  pcacheTrace(("%p.DIRTYLIST.%s %d\n", p,
                కూడు_Remove==1 ? "REMOVE" : కూడు_Remove==2 ? "ADD" : "FRONT",
                p_పుట->pgno));
  ఒకవేళ( కూడు_Remove & PCACHE_DIRTYLIST_REMOVE ){
    ప్రతిజ్ఞా( p_పుట->p_Dirty_తదుపరి || p_పుట==p->p_Dirty_తోక );
    ప్రతిజ్ఞా( p_పుట->pDirtyPrev || p_పుట==p->pDirty );
  
    /* Update the PCache1.pSynced variable if necessary. */
    ఒకవేళ( p->pSynced==p_పుట ){
      p->pSynced = p_పుట->pDirtyPrev;
    }
  
    ఒకవేళ( p_పుట->p_Dirty_తదుపరి ){
      p_పుట->p_Dirty_తదుపరి->pDirtyPrev = p_పుట->pDirtyPrev;
    }లేకపోతే{
      ప్రతిజ్ఞా( p_పుట==p->p_Dirty_తోక );
      p->p_Dirty_తోక = p_పుట->pDirtyPrev;
    }
    ఒకవేళ( p_పుట->pDirtyPrev ){
      p_పుట->pDirtyPrev->p_Dirty_తదుపరి = p_పుట->p_Dirty_తదుపరి;
    }లేకపోతే{
      /* If there are now no dirty pages in the cache, set eCreate to 2. 
      ** This is an optimization that allows sqlite3PcacheFetch() to skip
      ** searching for a dirty page to eject from the cache when it might
      ** otherwise have to.  */
      ప్రతిజ్ఞా( p_పుట==p->pDirty );
      p->pDirty = p_పుట->p_Dirty_తదుపరి;
      ప్రతిజ్ఞా( p->bPurgeable || p->eCreate==2 );
      ఒకవేళ( p->pDirty==0 ){         /*OPTIMIZATION-IF-TRUE*/
        ప్రతిజ్ఞా( p->bPurgeable==0 || p->eCreate==1 );
        p->eCreate = 2;
      }
    }
  }
  ఒకవేళ( కూడు_Remove & PCACHE_DIRTYLIST_కూడు ){
    p_పుట->pDirtyPrev = 0;
    p_పుట->p_Dirty_తదుపరి = p->pDirty;
    ఒకవేళ( p_పుట->p_Dirty_తదుపరి ){
      ప్రతిజ్ఞా( p_పుట->p_Dirty_తదుపరి->pDirtyPrev==0 );
      p_పుట->p_Dirty_తదుపరి->pDirtyPrev = p_పుట;
    }లేకపోతే{
      p->p_Dirty_తోక = p_పుట;
      ఒకవేళ( p->bPurgeable ){
        ప్రతిజ్ఞా( p->eCreate==2 );
        p->eCreate = 1;
      }
    }
    p->pDirty = p_పుట;

    /* If pSynced is NULL and this page has a clear NEED_SYNC flag, set
    ** pSynced to point to it. Checking the NEED_SYNC flag is an 
    ** optimization, as if pSynced points to a page with the NEED_SYNC
    ** flag set sqlite3PcacheFetchStress() searches through all newer 
    ** entries of the dirty-list for a page with NEED_SYNC clear anyway.  */
    ఒకవేళ( !p->pSynced 
     && 0==(p_పుట->flags&PGHDR_NEED_SYNC)   /*OPTIMIZATION-IF-FALSE*/
    ){
      p->pSynced = p_పుట;
    }
  }
  pcacheDump(p);
}

/*
** Wrapper around the pluggable caches xUnpin method. If the cache is
** being used for an in-memory database, this function is a no-op.
*/
స్థిర శూన్య pcacheUnpin(PgHdr *p){
  ఒకవేళ( p->pCache->bPurgeable ){
    pcacheTrace(("%p.UNPIN %d\n", p->pCache, p->pgno));
    sqlite3GlobalConfig.pcache2.xUnpin(p->pCache->pCache, p->p_పుట, 0);
    pcacheDump(p->pCache);
  }
}

/*
** Compute the number of pages of cache requested.   p->szCache is the
** cache size requested by the "PRAGMA cache_size" statement.
*/
స్థిర పూర్ణాంకం సంఖ్య_యొక్క_Cache_పుటలు(PCache *p){
  ఒకవేళ( p->szCache>=0 ){
    /* IMPLEMENTATION-OF: R-42059-47211 If the argument N is positive then the
    ** suggested cache size is set to N. */
    తిరిగి p->szCache;
  }లేకపోతే{
    i64 n;
    /* IMPLEMENTATION-OF: R-59858-46238 If the argument N is negative, then the
    ** number of cache pages is adjusted to be a number of pages that would
    ** use approximately abs(N*1024) bytes of memory based on the current
    ** page size. */
    n = ((-1024*(i64)p->szCache)/(p->sz_పుట+p->szExtra));
    ఒకవేళ( n>1000000000 ) n = 1000000000;
    తిరిగి (పూర్ణాంకం)n;
  }
}

/*************************************************** General Interfaces ******
**
** Initialize and shutdown the page cache subsystem. Neither of these 
** functions are threadsafe.
*/
పూర్ణాంకం sqlite3PcacheInitialize(శూన్య){
  ఒకవేళ( sqlite3GlobalConfig.pcache2.xInit==0 ){
    /* IMPLEMENTATION-OF: R-26801-64137 If the xInit() method is NULL, then the
    ** built-in default page cache is used instead of the application defined
    ** page cache. */
    sqlite3_PCache_సమితి_Default();
    ప్రతిజ్ఞా( sqlite3GlobalConfig.pcache2.xInit!=0 );
  }
  తిరిగి sqlite3GlobalConfig.pcache2.xInit(sqlite3GlobalConfig.pcache2.pArg);
}
శూన్య sqlite3PcacheShutdown(శూన్య){
  ఒకవేళ( sqlite3GlobalConfig.pcache2.xShutdown ){
    /* IMPLEMENTATION-OF: R-26000-56589 The xShutdown() method may be NULL. */
    sqlite3GlobalConfig.pcache2.xShutdown(sqlite3GlobalConfig.pcache2.pArg);
  }
}

/*
** Return the size in bytes of a PCache object.
*/
పూర్ణాంకం sqlite3_Pcache_కొలత(శూన్య){ తిరిగి పరిమాణం(PCache); }

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
పూర్ణాంకం sqlite3PcacheOpen(
  పూర్ణాంకం sz_పుట,                  /* Size of every page */
  పూర్ణాంకం szExtra,                 /* Extra space associated with each page */
  పూర్ణాంకం bPurgeable,              /* True if pages are on backing store */
  పూర్ణాంకం (*xStress)(శూన్య*,PgHdr*),/* Call to try to make pages clean */
  శూన్య *pStress,               /* Argument to xStress */
  PCache *p                    /* Preallocated space for the PCache */
){
  memset(p, 0, పరిమాణం(PCache));
  p->sz_పుట = 1;
  p->szExtra = szExtra;
  ప్రతిజ్ఞా( szExtra>=8 );  /* First 8 bytes will be zeroed */
  p->bPurgeable = bPurgeable;
  p->eCreate = 2;
  p->xStress = xStress;
  p->pStress = pStress;
  p->szCache = 100;
  p->szSpill = 1;
  pcacheTrace(("%p.OPEN szPage %d bPurgeable %d\n",p,sz_పుట,bPurgeable));
  తిరిగి sqlite3_Pcache_సమితి_పుట_కొలత(p, sz_పుట);
}

/*
** Change the page size for PCache object. The caller must ensure that there
** are no outstanding page references when this function is called.
*/
పూర్ణాంకం sqlite3_Pcache_సమితి_పుట_కొలత(PCache *pCache, పూర్ణాంకం sz_పుట){
  ప్రతిజ్ఞా( pCache->n_Ref_కూడిక==0 && pCache->pDirty==0 );
  ఒకవేళ( pCache->sz_పుట ){
    sqlite3_pcache *pNew;
    pNew = sqlite3GlobalConfig.pcache2.xCreate(
                sz_పుట, pCache->szExtra + ROUND8(పరిమాణం(PgHdr)),
                pCache->bPurgeable
    );
    ఒకవేళ( pNew==0 ) తిరిగి SQLITE_NOMEM_BKPT;
    sqlite3GlobalConfig.pcache2.xCachesize(pNew, సంఖ్య_యొక్క_Cache_పుటలు(pCache));
    ఒకవేళ( pCache->pCache ){
      sqlite3GlobalConfig.pcache2.xDestroy(pCache->pCache);
    }
    pCache->pCache = pNew;
    pCache->sz_పుట = sz_పుట;
    pcacheTrace(("%p.PAGESIZE %d\n",pCache,sz_పుట));
  }
  తిరిగి SQLITE_OK;
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
sqlite3_pcache_పుట *sqlite3PcacheFetch(
  PCache *pCache,       /* Obtain the page from this cache */
  Pgno pgno,            /* Page number to obtain */
  పూర్ణాంకం createFlag        /* If true, create page if it does not exist already */
){
  పూర్ణాంకం eCreate;
  sqlite3_pcache_పుట *pRes;

  ప్రతిజ్ఞా( pCache!=0 );
  ప్రతిజ్ఞా( pCache->pCache!=0 );
  ప్రతిజ్ఞా( createFlag==3 || createFlag==0 );
  ప్రతిజ్ఞా( pCache->eCreate==((pCache->bPurgeable && pCache->pDirty) ? 1 : 2) );

  /* eCreate defines what to do if the page does not exist.
  **    0     Do not allocate a new page.  (createFlag==0)
  **    1     Allocate a new page if doing so is inexpensive.
  **          (createFlag==1 AND bPurgeable AND pDirty)
  **    2     Allocate a new page even it doing so is difficult.
  **          (createFlag==1 AND !(bPurgeable AND pDirty)
  */
  eCreate = createFlag & pCache->eCreate;
  ప్రతిజ్ఞా( eCreate==0 || eCreate==1 || eCreate==2 );
  ప్రతిజ్ఞా( createFlag==0 || pCache->eCreate==eCreate );
  ప్రతిజ్ఞా( createFlag==0 || eCreate==1+(!pCache->bPurgeable||!pCache->pDirty) );
  pRes = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, pgno, eCreate);
  pcacheTrace(("%p.FETCH %d%s (result: %p) ",pCache,pgno,
               createFlag?" create":"",pRes));
  pcache_పుట_Trace(pgno, pRes);
  తిరిగి pRes;
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
పూర్ణాంకం sqlite3PcacheFetchStress(
  PCache *pCache,                 /* Obtain the page from this cache */
  Pgno pgno,                      /* Page number to obtain */
  sqlite3_pcache_పుట **pp_పుట    /* Write result here */
){
  PgHdr *pPg;
  ఒకవేళ( pCache->eCreate==2 ) తిరిగి 0;

  ఒకవేళ( sqlite3PcachePagecount(pCache)>pCache->szSpill ){
    /* Find a dirty page to write-out and recycle. First try to find a 
    ** page that does not require a journal-sync (one with PGHDR_NEED_SYNC
    ** cleared), but if that is not possible settle for any other 
    ** unreferenced dirty page.
    **
    ** If the LRU page in the dirty list that has a clear PGHDR_NEED_SYNC
    ** flag is currently referenced, then the following may leave pSynced
    ** set incorrectly (pointing to other than the LRU page with NEED_SYNC
    ** cleared). This is Ok, as pSynced is just an optimization.  */
    కొరకు(pPg=pCache->pSynced; 
        pPg && (pPg->nRef || (pPg->flags&PGHDR_NEED_SYNC)); 
        pPg=pPg->pDirtyPrev
    );
    pCache->pSynced = pPg;
    ఒకవేళ( !pPg ){
      కొరకు(pPg=pCache->p_Dirty_తోక; pPg && pPg->nRef; pPg=pPg->pDirtyPrev);
    }
    ఒకవేళ( pPg ){
      పూర్ణాంకం rc;
#నిర్వచితమైతే SQLITE_LOG_CACHE_SPILL
      sqlite3_log(SQLITE_నిండిన, 
                  "spill page %d making room for %d - cache used: %d/%d",
                  pPg->pgno, pgno,
                  sqlite3GlobalConfig.pcache2.xPagecount(pCache->pCache),
                సంఖ్య_యొక్క_Cache_పుటలు(pCache));
#అయితే_ముగింపు
      pcacheTrace(("%p.SPILL %d\n",pCache,pPg->pgno));
      rc = pCache->xStress(pCache->pStress, pPg);
      pcacheDump(pCache);
      ఒకవేళ( rc!=SQLITE_OK && rc!=SQLITE_BUSY ){
        తిరిగి rc;
      }
    }
  }
  *pp_పుట = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, pgno, 2);
  తిరిగి *pp_పుట==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
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
స్థిర SQLITE_NOINLINE PgHdr *pcache_Fetch_ముగింపు_With_Init(
  PCache *pCache,             /* Obtain the page from this cache */
  Pgno pgno,                  /* Page number obtained */
  sqlite3_pcache_పుట *p_పుట  /* Page obtained by prior PcacheFetch() call */
){
  PgHdr *pPgHdr;
  ప్రతిజ్ఞా( p_పుట!=0 );
  pPgHdr = (PgHdr*)p_పుట->pExtra;
  ప్రతిజ్ఞా( pPgHdr->p_పుట==0 );
  memset(&pPgHdr->pDirty, 0, పరిమాణం(PgHdr) - offsetof(PgHdr,pDirty));
  pPgHdr->p_పుట = p_పుట;
  pPgHdr->p_దత్తాంశం = p_పుట->pBuf;
  pPgHdr->pExtra = (శూన్య *)&pPgHdr[1];
  memset(pPgHdr->pExtra, 0, 8);
  ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT( pPgHdr->pExtra ) );
  pPgHdr->pCache = pCache;
  pPgHdr->pgno = pgno;
  pPgHdr->flags = PGHDR_CLEAN;
  తిరిగి sqlite3_Pcache_Fetch_ముగింపు(pCache,pgno,p_పుట);
}

/*
** This routine converts the sqlite3_pcache_page object returned by
** sqlite3PcacheFetch() into an initialized PgHdr object.  This routine
** must be called after sqlite3PcacheFetch() in order to get a usable
** result.
*/
PgHdr *sqlite3_Pcache_Fetch_ముగింపు(
  PCache *pCache,             /* Obtain the page from this cache */
  Pgno pgno,                  /* Page number obtained */
  sqlite3_pcache_పుట *p_పుట  /* Page obtained by prior PcacheFetch() call */
){
  PgHdr *pPgHdr;

  ప్రతిజ్ఞా( p_పుట!=0 );
  pPgHdr = (PgHdr *)p_పుట->pExtra;

  ఒకవేళ( !pPgHdr->p_పుట ){
    తిరిగి pcache_Fetch_ముగింపు_With_Init(pCache, pgno, p_పుట);
  }
  pCache->n_Ref_కూడిక++;
  pPgHdr->nRef++;
  ప్రతిజ్ఞా( sqlite3_Pcache_పుట_Sanity(pPgHdr) );
  తిరిగి pPgHdr;
}

/*
** Decrement the reference count on a page. If the page is clean and the
** reference count drops to 0, then it is made eligible for recycling.
*/
శూన్య SQLITE_NOINLINE sqlite3PcacheRelease(PgHdr *p){
  ప్రతిజ్ఞా( p->nRef>0 );
  p->pCache->n_Ref_కూడిక--;
  ఒకవేళ( (--p->nRef)==0 ){
    ఒకవేళ( p->flags&PGHDR_CLEAN ){
      pcacheUnpin(p);
    }లేకపోతే{
      pcacheManageDirtyList(p, PCACHE_DIRTYLIST_FRONT);
      ప్రతిజ్ఞా( sqlite3_Pcache_పుట_Sanity(p) );
    }
  }
}

/*
** Increase the reference count of a supplied page by 1.
*/
శూన్య sqlite3PcacheRef(PgHdr *p){
  ప్రతిజ్ఞా(p->nRef>0);
  ప్రతిజ్ఞా( sqlite3_Pcache_పుట_Sanity(p) );
  p->nRef++;
  p->pCache->n_Ref_కూడిక++;
}

/*
** Drop a page from the cache. There must be exactly one reference to the
** page. This function deletes that reference, so after it returns the
** page pointed to by p is invalid.
*/
శూన్య sqlite3PcacheDrop(PgHdr *p){
  ప్రతిజ్ఞా( p->nRef==1 );
  ప్రతిజ్ఞా( sqlite3_Pcache_పుట_Sanity(p) );
  ఒకవేళ( p->flags&PGHDR_DIRTY ){
    pcacheManageDirtyList(p, PCACHE_DIRTYLIST_REMOVE);
  }
  p->pCache->n_Ref_కూడిక--;
  sqlite3GlobalConfig.pcache2.xUnpin(p->pCache->pCache, p->p_పుట, 1);
}

/*
** Make sure the page is marked as dirty. If it isn't dirty already,
** make it so.
*/
శూన్య sqlite3PcacheMakeDirty(PgHdr *p){
  ప్రతిజ్ఞా( p->nRef>0 );
  ప్రతిజ్ఞా( sqlite3_Pcache_పుట_Sanity(p) );
  ఒకవేళ( p->flags & (PGHDR_CLEAN|PGHDR_DONT_WRITE) ){    /*OPTIMIZATION-IF-FALSE*/
    p->flags &= ~PGHDR_DONT_WRITE;
    ఒకవేళ( p->flags & PGHDR_CLEAN ){
      p->flags ^= (PGHDR_DIRTY|PGHDR_CLEAN);
      pcacheTrace(("%p.DIRTY %d\n",p->pCache,p->pgno));
      ప్రతిజ్ఞా( (p->flags & (PGHDR_DIRTY|PGHDR_CLEAN))==PGHDR_DIRTY );
      pcacheManageDirtyList(p, PCACHE_DIRTYLIST_కూడు);
      ప్రతిజ్ఞా( sqlite3_Pcache_పుట_Sanity(p) );
    }
    ప్రతిజ్ఞా( sqlite3_Pcache_పుట_Sanity(p) );
  }
}

/*
** Make sure the page is marked as clean. If it isn't clean already,
** make it so.
*/
శూన్య sqlite3PcacheMakeClean(PgHdr *p){
  ప్రతిజ్ఞా( sqlite3_Pcache_పుట_Sanity(p) );
  ప్రతిజ్ఞా( (p->flags & PGHDR_DIRTY)!=0 );
  ప్రతిజ్ఞా( (p->flags & PGHDR_CLEAN)==0 );
  pcacheManageDirtyList(p, PCACHE_DIRTYLIST_REMOVE);
  p->flags &= ~(PGHDR_DIRTY|PGHDR_NEED_SYNC|PGHDR_WRITEABLE);
  p->flags |= PGHDR_CLEAN;
  pcacheTrace(("%p.CLEAN %d\n",p->pCache,p->pgno));
  ప్రతిజ్ఞా( sqlite3_Pcache_పుట_Sanity(p) );
  ఒకవేళ( p->nRef==0 ){
    pcacheUnpin(p);
  }
}

/*
** Make every page in the cache clean.
*/
శూన్య sqlite3PcacheCleanAll(PCache *pCache){
  PgHdr *p;
  pcacheTrace(("%p.CLEAN-ALL\n",pCache));
  అయితే( (p = pCache->pDirty)!=0 ){
    sqlite3PcacheMakeClean(p);
  }
}

/*
** Clear the PGHDR_NEED_SYNC and PGHDR_WRITEABLE flag from all dirty pages.
*/
శూన్య sqlite3PcacheClearWritable(PCache *pCache){
  PgHdr *p;
  pcacheTrace(("%p.CLEAR-WRITEABLE\n",pCache));
  కొరకు(p=pCache->pDirty; p; p=p->p_Dirty_తదుపరి){
    p->flags &= ~(PGHDR_NEED_SYNC|PGHDR_WRITEABLE);
  }
  pCache->pSynced = pCache->p_Dirty_తోక;
}

/*
** Clear the PGHDR_NEED_SYNC flag from all dirty pages.
*/
శూన్య sqlite3PcacheClearSyncFlags(PCache *pCache){
  PgHdr *p;
  కొరకు(p=pCache->pDirty; p; p=p->p_Dirty_తదుపరి){
    p->flags &= ~PGHDR_NEED_SYNC;
  }
  pCache->pSynced = pCache->p_Dirty_తోక;
}

/*
** Change the page number of page p to newPgno. 
*/
శూన్య sqlite3PcacheMove(PgHdr *p, Pgno newPgno){
  PCache *pCache = p->pCache;
  sqlite3_pcache_పుట *pOther;
  ప్రతిజ్ఞా( p->nRef>0 );
  ప్రతిజ్ఞా( newPgno>0 );
  ప్రతిజ్ఞా( sqlite3_Pcache_పుట_Sanity(p) );
  pcacheTrace(("%p.MOVE %d -> %d\n",pCache,p->pgno,newPgno));
  pOther = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, newPgno, 0);
  ఒకవేళ( pOther ){
    PgHdr *pXPage = (PgHdr*)pOther->pExtra;
    ప్రతిజ్ఞా( pXPage->nRef==0 );
    pXPage->nRef++;
    pCache->n_Ref_కూడిక++;
    sqlite3PcacheDrop(pXPage);
  }
  sqlite3GlobalConfig.pcache2.xRekey(pCache->pCache, p->p_పుట, p->pgno,newPgno);
  p->pgno = newPgno;
  ఒకవేళ( (p->flags&PGHDR_DIRTY) && (p->flags&PGHDR_NEED_SYNC) ){
    pcacheManageDirtyList(p, PCACHE_DIRTYLIST_FRONT);
    ప్రతిజ్ఞా( sqlite3_Pcache_పుట_Sanity(p) );
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
శూన్య sqlite3PcacheTruncate(PCache *pCache, Pgno pgno){
  ఒకవేళ( pCache->pCache ){
    PgHdr *p;
    PgHdr *p_తదుపరి;
    pcacheTrace(("%p.TRUNCATE %d\n",pCache,pgno));
    కొరకు(p=pCache->pDirty; p; p=p_తదుపరి){
      p_తదుపరి = p->p_Dirty_తదుపరి;
      /* This routine never gets call with a positive pgno except right
      ** after sqlite3PcacheCleanAll().  So if there are dirty pages,
      ** it must be that pgno==0.
      */
      ప్రతిజ్ఞా( p->pgno>0 );
      ఒకవేళ( p->pgno>pgno ){
        ప్రతిజ్ఞా( p->flags&PGHDR_DIRTY );
        sqlite3PcacheMakeClean(p);
      }
    }
    ఒకవేళ( pgno==0 && pCache->n_Ref_కూడిక ){
      sqlite3_pcache_పుట *pPage1;
      pPage1 = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache,1,0);
      ఒకవేళ( ALWAYS(pPage1) ){  /* Page 1 is always available in cache, because
                             ** pCache->nRefSum>0 */
        memset(pPage1->pBuf, 0, pCache->sz_పుట);
        pgno = 1;
      }
    }
    sqlite3GlobalConfig.pcache2.xTruncate(pCache->pCache, pgno+1);
  }
}

/*
** Close a cache.
*/
శూన్య sqlite3PcacheClose(PCache *pCache){
  ప్రతిజ్ఞా( pCache->pCache!=0 );
  pcacheTrace(("%p.CLOSE\n",pCache));
  sqlite3GlobalConfig.pcache2.xDestroy(pCache->pCache);
}

/* 
** Discard the contents of the cache.
*/
శూన్య sqlite3PcacheClear(PCache *pCache){
  sqlite3PcacheTruncate(pCache, 0);
}

/*
** Merge two lists of pages connected by pDirty and in pgno order.
** Do not bother fixing the pDirtyPrev pointers.
*/
స్థిర PgHdr *pcache_విలీనం_Dirty_List(PgHdr *pA, PgHdr *pB){
  PgHdr ఫలితం, *p_తోక;
  p_తోక = &ఫలితం;
  ప్రతిజ్ఞా( pA!=0 && pB!=0 );
  కొరకు(;;){
    ఒకవేళ( pA->pgno<pB->pgno ){
      p_తోక->pDirty = pA;
      p_తోక = pA;
      pA = pA->pDirty;
      ఒకవేళ( pA==0 ){
        p_తోక->pDirty = pB;
        ఆపు;
      }
    }లేకపోతే{
      p_తోక->pDirty = pB;
      p_తోక = pB;
      pB = pB->pDirty;
      ఒకవేళ( pB==0 ){
        p_తోక->pDirty = pA;
        ఆపు;
      }
    }
  }
  తిరిగి ఫలితం.pDirty;
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
#నిర్వచించు N_క్రమీకరించు_పాత్ర  32
స్థిర PgHdr *pcache_క్రమీకరించు_Dirty_List(PgHdr *p_లోపల){
  PgHdr *a[N_క్రమీకరించు_పాత్ర], *p;
  పూర్ణాంకం i;
  memset(a, 0, పరిమాణం(a));
  అయితే( p_లోపల ){
    p = p_లోపల;
    p_లోపల = p->pDirty;
    p->pDirty = 0;
    కొరకు(i=0; ALWAYS(i<N_క్రమీకరించు_పాత్ర-1); i++){
      ఒకవేళ( a[i]==0 ){
        a[i] = p;
        ఆపు;
      }లేకపోతే{
        p = pcache_విలీనం_Dirty_List(a[i], p);
        a[i] = 0;
      }
    }
    ఒకవేళ( NEVER(i==N_క్రమీకరించు_పాత్ర-1) ){
      /* To get here, there need to be 2^(N_SORT_BUCKET) elements in
      ** the input list.  But that is impossible.
      */
      a[i] = pcache_విలీనం_Dirty_List(a[i], p);
    }
  }
  p = a[0];
  కొరకు(i=1; i<N_క్రమీకరించు_పాత్ర; i++){
    ఒకవేళ( a[i]==0 ) కొనసాగు;
    p = p ? pcache_విలీనం_Dirty_List(p, a[i]) : a[i];
  }
  తిరిగి p;
}

/*
** Return a list of all dirty pages in the cache, sorted by page number.
*/
PgHdr *sqlite3PcacheDirtyList(PCache *pCache){
  PgHdr *p;
  కొరకు(p=pCache->pDirty; p; p=p->p_Dirty_తదుపరి){
    p->pDirty = p->p_Dirty_తదుపరి;
  }
  తిరిగి pcache_క్రమీకరించు_Dirty_List(pCache->pDirty);
}

/* 
** Return the total number of references to all pages held by the cache.
**
** This is not the total number of pages referenced, but the sum of the
** reference count for all pages.
*/
i64 sqlite3_Pcache_Ref_లెక్క(PCache *pCache){
  తిరిగి pCache->n_Ref_కూడిక;
}

/*
** Return the number of references to the page supplied as an argument.
*/
i64 sqlite3_Pcache_పుట_Refcount(PgHdr *p){
  తిరిగి p->nRef;
}

/* 
** Return the total number of pages in the cache.
*/
పూర్ణాంకం sqlite3PcachePagecount(PCache *pCache){
  ప్రతిజ్ఞా( pCache->pCache!=0 );
  తిరిగి sqlite3GlobalConfig.pcache2.xPagecount(pCache->pCache);
}

#నిర్వచితమైతే SQLITE_TEST
/*
** Get the suggested cache-size value.
*/
పూర్ణాంకం sqlite3PcacheGetCachesize(PCache *pCache){
  తిరిగి సంఖ్య_యొక్క_Cache_పుటలు(pCache);
}
#అయితే_ముగింపు

/*
** Set the suggested cache-size value.
*/
శూన్య sqlite3_Pcache_సమితి_Cachesize(PCache *pCache, పూర్ణాంకం mx_పుట){
  ప్రతిజ్ఞా( pCache->pCache!=0 );
  pCache->szCache = mx_పుట;
  sqlite3GlobalConfig.pcache2.xCachesize(pCache->pCache,
                                         సంఖ్య_యొక్క_Cache_పుటలు(pCache));
}

/*
** Set the suggested cache-spill value.  Make no changes if if the
** argument is zero.  Return the effective cache-spill size, which will
** be the larger of the szSpill and szCache.
*/
పూర్ణాంకం sqlite3_Pcache_సమితి_Spillsize(PCache *p, పూర్ణాంకం mx_పుట){
  పూర్ణాంకం res;
  ప్రతిజ్ఞా( p->pCache!=0 );
  ఒకవేళ( mx_పుట ){
    ఒకవేళ( mx_పుట<0 ){
      mx_పుట = (పూర్ణాంకం)((-1024*(i64)mx_పుట)/(p->sz_పుట+p->szExtra));
    }
    p->szSpill = mx_పుట;
  }
  res = సంఖ్య_యొక్క_Cache_పుటలు(p);
  ఒకవేళ( res<p->szSpill ) res = p->szSpill; 
  తిరిగి res;
}

/*
** Free up as much memory as possible from the page cache.
*/
శూన్య sqlite3PcacheShrink(PCache *pCache){
  ప్రతిజ్ఞా( pCache->pCache!=0 );
  sqlite3GlobalConfig.pcache2.xShrink(pCache->pCache);
}

/*
** Return the size of the header added by this middleware layer
** in the page-cache hierarchy.
*/
పూర్ణాంకం sqlite3_శీర్షిక_కొలత_Pcache(శూన్య){ తిరిగి ROUND8(పరిమాణం(PgHdr)); }

/*
** Return the number of dirty pages currently in the cache, as a percentage
** of the configured cache size.
*/
పూర్ణాంకం sqlite3PCachePercentDirty(PCache *pCache){
  PgHdr *pDirty;
  పూర్ణాంకం nDirty = 0;
  పూర్ణాంకం nCache = సంఖ్య_యొక్క_Cache_పుటలు(pCache);
  కొరకు(pDirty=pCache->pDirty; pDirty; pDirty=pDirty->p_Dirty_తదుపరి) nDirty++;
  తిరిగి nCache ? (పూర్ణాంకం)(((i64)nDirty * 100) / nCache) : 0;
}

#నిర్వచితమైతే SQLITE_ప్రత్యక్ష_అధికప్రవాహం_READ
/* 
** Return true if there are one or more dirty pages in the cache. Else false.
*/
పూర్ణాంకం sqlite3PCacheIsDirty(PCache *pCache){
  తిరిగి (pCache->pDirty!=0);
}
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_CHECK_పుటలు) || defined(SQLITE_DEBUG)
/*
** For all dirty pages currently in the cache, invoke the specified
** callback. This is only used if the SQLITE_CHECK_PAGES macro is
** defined.
*/
శూన్య sqlite3PcacheIterateDirty(PCache *pCache, శూన్య (*xIter)(PgHdr *)){
  PgHdr *pDirty;
  కొరకు(pDirty=pCache->pDirty; pDirty; pDirty=pDirty->p_Dirty_తదుపరి){
    xIter(pDirty);
  }
}
#అయితే_ముగింపు
