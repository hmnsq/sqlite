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
#அந்தர்பாவ "sqliteInt.h"

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
கட்டமைப்பு PCache {
  PgHdr *pDirty, *pDirtyTail;         /* List of dirty pages in LRU order */
  PgHdr *pSynced;                     /* Last synced page in dirty page list */
  i64 nRefSum;                        /* Sum of ref counts over all pages */
  முழு szCache;                        /* Configured cache size */
  முழு szSpill;                        /* Size before spilling occurs */
  முழு szPage;                         /* Size of every page in this cache */
  முழு szExtra;                        /* Size of extra space for each page */
  u8 bPurgeable;                      /* True if pages are on backing store */
  u8 eCreate;                         /* eCreate value for for xFetch() */
  முழு (*xStress)(வெறுமை*,PgHdr*);       /* Call to try make a page clean */
  வெறுமை *pStress;                      /* Argument to xStress */
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
#எனில் defined(SQLITE_DEBUG) && 0
  முழு sqlite3PcacheTrace = 2;       /* 0: off  1: simple  2: cache dumps */
  முழு sqlite3PcacheMxDump = 9999;   /* Max cache entries for pcacheDump() */
# define pcacheTrace(X) எனில்(sqlite3PcacheTrace){sqlite3DebugPrintf X;}
  நிலையான வெறுமை pcachePageTrace(முழு i, sqlite3_pcache_page *pLower){
    PgHdr *pPg;
    unsigned எழுத்து *a;
    முழு j;
    எனில்( pLower==0 ){
      printf("%3d: NULL\n", i);
    }இல்லையெனில்{
      pPg = (PgHdr*)pLower->pExtra;
      printf("%3d: nRef %2lld flgs %02x data ", i, pPg->nRef, pPg->flags);
      a = (unsigned எழுத்து *)pLower->pBuf;
      க்கு(j=0; j<12; j++) printf("%02x", a[j]);
      printf(" ptr %p\n", pPg);
    }
  }
  நிலையான வெறுமை pcacheDump(PCache *pCache){
    முழு N;
    முழு i;
    sqlite3_pcache_page *pLower;
  
    எனில்( sqlite3PcacheTrace<2 ) திரும்பு;
    எனில்( pCache->pCache==0 ) திரும்பு;
    N = sqlite3PcachePagecount(pCache);
    எனில்( N>sqlite3PcacheMxDump ) N = sqlite3PcacheMxDump;
    க்கு(i=1; i<=N; i++){
       pLower = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, i, 0);
       pcachePageTrace(i, pLower);
       எனில்( pLower && ((PgHdr*)pLower)->pPage==0 ){
         sqlite3GlobalConfig.pcache2.xUnpin(pCache->pCache, pLower, 0);
       }
    }
  }
#இல்லையெனில்
# define pcacheTrace(X)
# define pcachePageTrace(PGNO, X)
# define pcacheDump(X)
#endif

/*
** Return 1 if pPg is on the dirty list for pCache.  Return 0 if not.
** This routine runs inside of assert() statements only.
*/
#எனில் defined(SQLITE_ENABLE_EXPENSIVE_ASSERT)
நிலையான முழு pageOnDirtyList(PCache *pCache, PgHdr *pPg){
  PgHdr *p;
  க்கு(p=pCache->pDirty; p; p=p->pDirtyNext){
    எனில்( p==pPg ) திரும்பு 1;
  }
  திரும்பு 0;
}
நிலையான முழு pageNotOnDirtyList(PCache *pCache, PgHdr *pPg){
  PgHdr *p;
  க்கு(p=pCache->pDirty; p; p=p->pDirtyNext){
    எனில்( p==pPg ) திரும்பு 0;
  }
  திரும்பு 1;
}
#இல்லையெனில்
# define pageOnDirtyList(A,B)    1
# define pageNotOnDirtyList(A,B) 1
#endif

/*
** Check invariants on a PgHdr entry.  Return true if everything is OK.
** Return false if any invariant is violated.
**
** This routine is for use inside of assert() statements only.  For
** example:
**
**          assert( sqlite3PcachePageSanity(pPg) );
*/
#ifdef SQLITE_DEBUG
முழு sqlite3PcachePageSanity(PgHdr *pPg){
  PCache *pCache;
  ப்ரதிஜ்ஞா( pPg!=0 );
  ப்ரதிஜ்ஞா( pPg->pgno>0 || pPg->pPager==0 );    /* Page number is 1 or more */
  pCache = pPg->pCache;
  ப்ரதிஜ்ஞா( pCache!=0 );      /* Every page has an associated PCache */
  எனில்( pPg->flags & PGHDR_CLEAN ){
    ப்ரதிஜ்ஞா( (pPg->flags & PGHDR_DIRTY)==0 );/* Cannot be both CLEAN and DIRTY */
    ப்ரதிஜ்ஞா( pageNotOnDirtyList(pCache, pPg) );/* CLEAN pages not on dirtylist */
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( (pPg->flags & PGHDR_DIRTY)!=0 );/* If not CLEAN must be DIRTY */
    ப்ரதிஜ்ஞா( pPg->pDirtyNext==0 || pPg->pDirtyNext->pDirtyPrev==pPg );
    ப்ரதிஜ்ஞா( pPg->pDirtyPrev==0 || pPg->pDirtyPrev->pDirtyNext==pPg );
    ப்ரதிஜ்ஞா( pPg->pDirtyPrev!=0 || pCache->pDirty==pPg );
    ப்ரதிஜ்ஞா( pageOnDirtyList(pCache, pPg) );
  }
  /* WRITEABLE pages must also be DIRTY */
  எனில்( pPg->flags & PGHDR_WRITEABLE ){
    ப்ரதிஜ்ஞா( pPg->flags & PGHDR_DIRTY );     /* WRITEABLE implies DIRTY */
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
  திரும்பு 1;
}
#endif /* SQLITE_DEBUG */


/********************************** Linked List Management ********************/

/* Allowed values for second argument to pcacheManageDirtyList() */
#define PCACHE_DIRTYLIST_REMOVE   1    /* Remove pPage from dirty list */
#define PCACHE_DIRTYLIST_ADD      2    /* Add pPage to the dirty list */
#define PCACHE_DIRTYLIST_FRONT    3    /* Move pPage to the front of the list */

/*
** Manage pPage's participation on the dirty list.  Bits of the addRemove
** argument determines what operation to do.  The 0x01 bit means first
** remove pPage from the dirty list.  The 0x02 means add pPage back to
** the dirty list.  Doing both moves pPage to the front of the dirty list.
*/
நிலையான வெறுமை pcacheManageDirtyList(PgHdr *pPage, u8 addRemove){
  PCache *p = pPage->pCache;

  pcacheTrace(("%p.DIRTYLIST.%s %d\n", p,
                addRemove==1 ? "REMOVE" : addRemove==2 ? "ADD" : "FRONT",
                pPage->pgno));
  எனில்( addRemove & PCACHE_DIRTYLIST_REMOVE ){
    ப்ரதிஜ்ஞா( pPage->pDirtyNext || pPage==p->pDirtyTail );
    ப்ரதிஜ்ஞா( pPage->pDirtyPrev || pPage==p->pDirty );
  
    /* Update the PCache1.pSynced variable if necessary. */
    எனில்( p->pSynced==pPage ){
      p->pSynced = pPage->pDirtyPrev;
    }
  
    எனில்( pPage->pDirtyNext ){
      pPage->pDirtyNext->pDirtyPrev = pPage->pDirtyPrev;
    }இல்லையெனில்{
      ப்ரதிஜ்ஞா( pPage==p->pDirtyTail );
      p->pDirtyTail = pPage->pDirtyPrev;
    }
    எனில்( pPage->pDirtyPrev ){
      pPage->pDirtyPrev->pDirtyNext = pPage->pDirtyNext;
    }இல்லையெனில்{
      /* If there are now no dirty pages in the cache, set eCreate to 2. 
      ** This is an optimization that allows sqlite3PcacheFetch() to skip
      ** searching for a dirty page to eject from the cache when it might
      ** otherwise have to.  */
      ப்ரதிஜ்ஞா( pPage==p->pDirty );
      p->pDirty = pPage->pDirtyNext;
      ப்ரதிஜ்ஞா( p->bPurgeable || p->eCreate==2 );
      எனில்( p->pDirty==0 ){         /*OPTIMIZATION-IF-TRUE*/
        ப்ரதிஜ்ஞா( p->bPurgeable==0 || p->eCreate==1 );
        p->eCreate = 2;
      }
    }
  }
  எனில்( addRemove & PCACHE_DIRTYLIST_ADD ){
    pPage->pDirtyPrev = 0;
    pPage->pDirtyNext = p->pDirty;
    எனில்( pPage->pDirtyNext ){
      ப்ரதிஜ்ஞா( pPage->pDirtyNext->pDirtyPrev==0 );
      pPage->pDirtyNext->pDirtyPrev = pPage;
    }இல்லையெனில்{
      p->pDirtyTail = pPage;
      எனில்( p->bPurgeable ){
        ப்ரதிஜ்ஞா( p->eCreate==2 );
        p->eCreate = 1;
      }
    }
    p->pDirty = pPage;

    /* If pSynced is NULL and this page has a clear NEED_SYNC flag, set
    ** pSynced to point to it. Checking the NEED_SYNC flag is an 
    ** optimization, as if pSynced points to a page with the NEED_SYNC
    ** flag set sqlite3PcacheFetchStress() searches through all newer 
    ** entries of the dirty-list for a page with NEED_SYNC clear anyway.  */
    எனில்( !p->pSynced 
     && 0==(pPage->flags&PGHDR_NEED_SYNC)   /*OPTIMIZATION-IF-FALSE*/
    ){
      p->pSynced = pPage;
    }
  }
  pcacheDump(p);
}

/*
** Wrapper around the pluggable caches xUnpin method. If the cache is
** being used for an in-memory database, this function is a no-op.
*/
நிலையான வெறுமை pcacheUnpin(PgHdr *p){
  எனில்( p->pCache->bPurgeable ){
    pcacheTrace(("%p.UNPIN %d\n", p->pCache, p->pgno));
    sqlite3GlobalConfig.pcache2.xUnpin(p->pCache->pCache, p->pPage, 0);
    pcacheDump(p->pCache);
  }
}

/*
** Compute the number of pages of cache requested.   p->szCache is the
** cache size requested by the "PRAGMA cache_size" statement.
*/
நிலையான முழு எண்_Of_Cache_Pages(PCache *p){
  எனில்( p->szCache>=0 ){
    /* IMPLEMENTATION-OF: R-42059-47211 If the argument N is positive then the
    ** suggested cache size is set to N. */
    திரும்பு p->szCache;
  }இல்லையெனில்{
    i64 n;
    /* IMPLEMENTATION-OF: R-59858-46238 If the argument N is negative, then the
    ** number of cache pages is adjusted to be a number of pages that would
    ** use approximately abs(N*1024) bytes of memory based on the current
    ** page size. */
    n = ((-1024*(i64)p->szCache)/(p->szPage+p->szExtra));
    எனில்( n>1000000000 ) n = 1000000000;
    திரும்பு (முழு)n;
  }
}

/*************************************************** General Interfaces ******
**
** Initialize and shutdown the page cache subsystem. Neither of these 
** functions are threadsafe.
*/
முழு sqlite3PcacheInitialize(வெறுமை){
  எனில்( sqlite3GlobalConfig.pcache2.xInit==0 ){
    /* IMPLEMENTATION-OF: R-26801-64137 If the xInit() method is NULL, then the
    ** built-in default page cache is used instead of the application defined
    ** page cache. */
    sqlite3PCacheSetDefault();
    ப்ரதிஜ்ஞா( sqlite3GlobalConfig.pcache2.xInit!=0 );
  }
  திரும்பு sqlite3GlobalConfig.pcache2.xInit(sqlite3GlobalConfig.pcache2.pArg);
}
வெறுமை sqlite3PcacheShutdown(வெறுமை){
  எனில்( sqlite3GlobalConfig.pcache2.xShutdown ){
    /* IMPLEMENTATION-OF: R-26000-56589 The xShutdown() method may be NULL. */
    sqlite3GlobalConfig.pcache2.xShutdown(sqlite3GlobalConfig.pcache2.pArg);
  }
}

/*
** Return the size in bytes of a PCache object.
*/
முழு sqlite3PcacheSize(வெறுமை){ திரும்பு அளவு(PCache); }

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
முழு sqlite3PcacheOpen(
  முழு szPage,                  /* Size of every page */
  முழு szExtra,                 /* Extra space associated with each page */
  முழு bPurgeable,              /* True if pages are on backing store */
  முழு (*xStress)(வெறுமை*,PgHdr*),/* Call to try to make pages clean */
  வெறுமை *pStress,               /* Argument to xStress */
  PCache *p                    /* Preallocated space for the PCache */
){
  memset(p, 0, அளவு(PCache));
  p->szPage = 1;
  p->szExtra = szExtra;
  ப்ரதிஜ்ஞா( szExtra>=8 );  /* First 8 bytes will be zeroed */
  p->bPurgeable = bPurgeable;
  p->eCreate = 2;
  p->xStress = xStress;
  p->pStress = pStress;
  p->szCache = 100;
  p->szSpill = 1;
  pcacheTrace(("%p.OPEN szPage %d bPurgeable %d\n",p,szPage,bPurgeable));
  திரும்பு sqlite3PcacheSetPageSize(p, szPage);
}

/*
** Change the page size for PCache object. The caller must ensure that there
** are no outstanding page references when this function is called.
*/
முழு sqlite3PcacheSetPageSize(PCache *pCache, முழு szPage){
  ப்ரதிஜ்ஞா( pCache->nRefSum==0 && pCache->pDirty==0 );
  எனில்( pCache->szPage ){
    sqlite3_pcache *pNew;
    pNew = sqlite3GlobalConfig.pcache2.xCreate(
                szPage, pCache->szExtra + ROUND8(அளவு(PgHdr)),
                pCache->bPurgeable
    );
    எனில்( pNew==0 ) திரும்பு SQLITE_NOMEM_BKPT;
    sqlite3GlobalConfig.pcache2.xCachesize(pNew, எண்_Of_Cache_Pages(pCache));
    எனில்( pCache->pCache ){
      sqlite3GlobalConfig.pcache2.xDestroy(pCache->pCache);
    }
    pCache->pCache = pNew;
    pCache->szPage = szPage;
    pcacheTrace(("%p.PAGESIZE %d\n",pCache,szPage));
  }
  திரும்பு SQLITE_OK;
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
sqlite3_pcache_page *sqlite3PcacheFetch(
  PCache *pCache,       /* Obtain the page from this cache */
  Pgno pgno,            /* Page number to obtain */
  முழு createFlag        /* If true, create page if it does not exist already */
){
  முழு eCreate;
  sqlite3_pcache_page *pRes;

  ப்ரதிஜ்ஞா( pCache!=0 );
  ப்ரதிஜ்ஞா( pCache->pCache!=0 );
  ப்ரதிஜ்ஞா( createFlag==3 || createFlag==0 );
  ப்ரதிஜ்ஞா( pCache->eCreate==((pCache->bPurgeable && pCache->pDirty) ? 1 : 2) );

  /* eCreate defines what to do if the page does not exist.
  **    0     Do not allocate a new page.  (createFlag==0)
  **    1     Allocate a new page if doing so is inexpensive.
  **          (createFlag==1 AND bPurgeable AND pDirty)
  **    2     Allocate a new page even it doing so is difficult.
  **          (createFlag==1 AND !(bPurgeable AND pDirty)
  */
  eCreate = createFlag & pCache->eCreate;
  ப்ரதிஜ்ஞா( eCreate==0 || eCreate==1 || eCreate==2 );
  ப்ரதிஜ்ஞா( createFlag==0 || pCache->eCreate==eCreate );
  ப்ரதிஜ்ஞா( createFlag==0 || eCreate==1+(!pCache->bPurgeable||!pCache->pDirty) );
  pRes = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, pgno, eCreate);
  pcacheTrace(("%p.FETCH %d%s (result: %p) ",pCache,pgno,
               createFlag?" create":"",pRes));
  pcachePageTrace(pgno, pRes);
  திரும்பு pRes;
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
முழு sqlite3PcacheFetchStress(
  PCache *pCache,                 /* Obtain the page from this cache */
  Pgno pgno,                      /* Page number to obtain */
  sqlite3_pcache_page **ppPage    /* Write result here */
){
  PgHdr *pPg;
  எனில்( pCache->eCreate==2 ) திரும்பு 0;

  எனில்( sqlite3PcachePagecount(pCache)>pCache->szSpill ){
    /* Find a dirty page to write-out and recycle. First try to find a 
    ** page that does not require a journal-sync (one with PGHDR_NEED_SYNC
    ** cleared), but if that is not possible settle for any other 
    ** unreferenced dirty page.
    **
    ** If the LRU page in the dirty list that has a clear PGHDR_NEED_SYNC
    ** flag is currently referenced, then the following may leave pSynced
    ** set incorrectly (pointing to other than the LRU page with NEED_SYNC
    ** cleared). This is Ok, as pSynced is just an optimization.  */
    க்கு(pPg=pCache->pSynced; 
        pPg && (pPg->nRef || (pPg->flags&PGHDR_NEED_SYNC)); 
        pPg=pPg->pDirtyPrev
    );
    pCache->pSynced = pPg;
    எனில்( !pPg ){
      க்கு(pPg=pCache->pDirtyTail; pPg && pPg->nRef; pPg=pPg->pDirtyPrev);
    }
    எனில்( pPg ){
      முழு rc;
#ifdef SQLITE_LOG_CACHE_SPILL
      sqlite3_log(SQLITE_FULL, 
                  "spill page %d making room for %d - cache used: %d/%d",
                  pPg->pgno, pgno,
                  sqlite3GlobalConfig.pcache2.xPagecount(pCache->pCache),
                எண்_Of_Cache_Pages(pCache));
#endif
      pcacheTrace(("%p.SPILL %d\n",pCache,pPg->pgno));
      rc = pCache->xStress(pCache->pStress, pPg);
      pcacheDump(pCache);
      எனில்( rc!=SQLITE_OK && rc!=SQLITE_BUSY ){
        திரும்பு rc;
      }
    }
  }
  *ppPage = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, pgno, 2);
  திரும்பு *ppPage==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
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
நிலையான SQLITE_NOINLINE PgHdr *pcacheFetchFinishWithInit(
  PCache *pCache,             /* Obtain the page from this cache */
  Pgno pgno,                  /* Page number obtained */
  sqlite3_pcache_page *pPage  /* Page obtained by prior PcacheFetch() call */
){
  PgHdr *pPgHdr;
  ப்ரதிஜ்ஞா( pPage!=0 );
  pPgHdr = (PgHdr*)pPage->pExtra;
  ப்ரதிஜ்ஞா( pPgHdr->pPage==0 );
  memset(&pPgHdr->pDirty, 0, அளவு(PgHdr) - offsetof(PgHdr,pDirty));
  pPgHdr->pPage = pPage;
  pPgHdr->pData = pPage->pBuf;
  pPgHdr->pExtra = (வெறுமை *)&pPgHdr[1];
  memset(pPgHdr->pExtra, 0, 8);
  ப்ரதிஜ்ஞா( EIGHT_BYTE_ALIGNMENT( pPgHdr->pExtra ) );
  pPgHdr->pCache = pCache;
  pPgHdr->pgno = pgno;
  pPgHdr->flags = PGHDR_CLEAN;
  திரும்பு sqlite3PcacheFetchFinish(pCache,pgno,pPage);
}

/*
** This routine converts the sqlite3_pcache_page object returned by
** sqlite3PcacheFetch() into an initialized PgHdr object.  This routine
** must be called after sqlite3PcacheFetch() in order to get a usable
** result.
*/
PgHdr *sqlite3PcacheFetchFinish(
  PCache *pCache,             /* Obtain the page from this cache */
  Pgno pgno,                  /* Page number obtained */
  sqlite3_pcache_page *pPage  /* Page obtained by prior PcacheFetch() call */
){
  PgHdr *pPgHdr;

  ப்ரதிஜ்ஞா( pPage!=0 );
  pPgHdr = (PgHdr *)pPage->pExtra;

  எனில்( !pPgHdr->pPage ){
    திரும்பு pcacheFetchFinishWithInit(pCache, pgno, pPage);
  }
  pCache->nRefSum++;
  pPgHdr->nRef++;
  ப்ரதிஜ்ஞா( sqlite3PcachePageSanity(pPgHdr) );
  திரும்பு pPgHdr;
}

/*
** Decrement the reference count on a page. If the page is clean and the
** reference count drops to 0, then it is made eligible for recycling.
*/
வெறுமை SQLITE_NOINLINE sqlite3PcacheRelease(PgHdr *p){
  ப்ரதிஜ்ஞா( p->nRef>0 );
  p->pCache->nRefSum--;
  எனில்( (--p->nRef)==0 ){
    எனில்( p->flags&PGHDR_CLEAN ){
      pcacheUnpin(p);
    }இல்லையெனில்{
      pcacheManageDirtyList(p, PCACHE_DIRTYLIST_FRONT);
      ப்ரதிஜ்ஞா( sqlite3PcachePageSanity(p) );
    }
  }
}

/*
** Increase the reference count of a supplied page by 1.
*/
வெறுமை sqlite3PcacheRef(PgHdr *p){
  ப்ரதிஜ்ஞா(p->nRef>0);
  ப்ரதிஜ்ஞா( sqlite3PcachePageSanity(p) );
  p->nRef++;
  p->pCache->nRefSum++;
}

/*
** Drop a page from the cache. There must be exactly one reference to the
** page. This function deletes that reference, so after it returns the
** page pointed to by p is invalid.
*/
வெறுமை sqlite3PcacheDrop(PgHdr *p){
  ப்ரதிஜ்ஞா( p->nRef==1 );
  ப்ரதிஜ்ஞா( sqlite3PcachePageSanity(p) );
  எனில்( p->flags&PGHDR_DIRTY ){
    pcacheManageDirtyList(p, PCACHE_DIRTYLIST_REMOVE);
  }
  p->pCache->nRefSum--;
  sqlite3GlobalConfig.pcache2.xUnpin(p->pCache->pCache, p->pPage, 1);
}

/*
** Make sure the page is marked as dirty. If it isn't dirty already,
** make it so.
*/
வெறுமை sqlite3PcacheMakeDirty(PgHdr *p){
  ப்ரதிஜ்ஞா( p->nRef>0 );
  ப்ரதிஜ்ஞா( sqlite3PcachePageSanity(p) );
  எனில்( p->flags & (PGHDR_CLEAN|PGHDR_DONT_WRITE) ){    /*OPTIMIZATION-IF-FALSE*/
    p->flags &= ~PGHDR_DONT_WRITE;
    எனில்( p->flags & PGHDR_CLEAN ){
      p->flags ^= (PGHDR_DIRTY|PGHDR_CLEAN);
      pcacheTrace(("%p.DIRTY %d\n",p->pCache,p->pgno));
      ப்ரதிஜ்ஞா( (p->flags & (PGHDR_DIRTY|PGHDR_CLEAN))==PGHDR_DIRTY );
      pcacheManageDirtyList(p, PCACHE_DIRTYLIST_ADD);
      ப்ரதிஜ்ஞா( sqlite3PcachePageSanity(p) );
    }
    ப்ரதிஜ்ஞா( sqlite3PcachePageSanity(p) );
  }
}

/*
** Make sure the page is marked as clean. If it isn't clean already,
** make it so.
*/
வெறுமை sqlite3PcacheMakeClean(PgHdr *p){
  ப்ரதிஜ்ஞா( sqlite3PcachePageSanity(p) );
  ப்ரதிஜ்ஞா( (p->flags & PGHDR_DIRTY)!=0 );
  ப்ரதிஜ்ஞா( (p->flags & PGHDR_CLEAN)==0 );
  pcacheManageDirtyList(p, PCACHE_DIRTYLIST_REMOVE);
  p->flags &= ~(PGHDR_DIRTY|PGHDR_NEED_SYNC|PGHDR_WRITEABLE);
  p->flags |= PGHDR_CLEAN;
  pcacheTrace(("%p.CLEAN %d\n",p->pCache,p->pgno));
  ப்ரதிஜ்ஞா( sqlite3PcachePageSanity(p) );
  எனில்( p->nRef==0 ){
    pcacheUnpin(p);
  }
}

/*
** Make every page in the cache clean.
*/
வெறுமை sqlite3PcacheCleanAll(PCache *pCache){
  PgHdr *p;
  pcacheTrace(("%p.CLEAN-ALL\n",pCache));
  போது( (p = pCache->pDirty)!=0 ){
    sqlite3PcacheMakeClean(p);
  }
}

/*
** Clear the PGHDR_NEED_SYNC and PGHDR_WRITEABLE flag from all dirty pages.
*/
வெறுமை sqlite3PcacheClearWritable(PCache *pCache){
  PgHdr *p;
  pcacheTrace(("%p.CLEAR-WRITEABLE\n",pCache));
  க்கு(p=pCache->pDirty; p; p=p->pDirtyNext){
    p->flags &= ~(PGHDR_NEED_SYNC|PGHDR_WRITEABLE);
  }
  pCache->pSynced = pCache->pDirtyTail;
}

/*
** Clear the PGHDR_NEED_SYNC flag from all dirty pages.
*/
வெறுமை sqlite3PcacheClearSyncFlags(PCache *pCache){
  PgHdr *p;
  க்கு(p=pCache->pDirty; p; p=p->pDirtyNext){
    p->flags &= ~PGHDR_NEED_SYNC;
  }
  pCache->pSynced = pCache->pDirtyTail;
}

/*
** Change the page number of page p to newPgno. 
*/
வெறுமை sqlite3PcacheMove(PgHdr *p, Pgno newPgno){
  PCache *pCache = p->pCache;
  sqlite3_pcache_page *pOther;
  ப்ரதிஜ்ஞா( p->nRef>0 );
  ப்ரதிஜ்ஞா( newPgno>0 );
  ப்ரதிஜ்ஞா( sqlite3PcachePageSanity(p) );
  pcacheTrace(("%p.MOVE %d -> %d\n",pCache,p->pgno,newPgno));
  pOther = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, newPgno, 0);
  எனில்( pOther ){
    PgHdr *pXPage = (PgHdr*)pOther->pExtra;
    ப்ரதிஜ்ஞா( pXPage->nRef==0 );
    pXPage->nRef++;
    pCache->nRefSum++;
    sqlite3PcacheDrop(pXPage);
  }
  sqlite3GlobalConfig.pcache2.xRekey(pCache->pCache, p->pPage, p->pgno,newPgno);
  p->pgno = newPgno;
  எனில்( (p->flags&PGHDR_DIRTY) && (p->flags&PGHDR_NEED_SYNC) ){
    pcacheManageDirtyList(p, PCACHE_DIRTYLIST_FRONT);
    ப்ரதிஜ்ஞா( sqlite3PcachePageSanity(p) );
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
வெறுமை sqlite3PcacheTruncate(PCache *pCache, Pgno pgno){
  எனில்( pCache->pCache ){
    PgHdr *p;
    PgHdr *pNext;
    pcacheTrace(("%p.TRUNCATE %d\n",pCache,pgno));
    க்கு(p=pCache->pDirty; p; p=pNext){
      pNext = p->pDirtyNext;
      /* This routine never gets call with a positive pgno except right
      ** after sqlite3PcacheCleanAll().  So if there are dirty pages,
      ** it must be that pgno==0.
      */
      ப்ரதிஜ்ஞா( p->pgno>0 );
      எனில்( p->pgno>pgno ){
        ப்ரதிஜ்ஞா( p->flags&PGHDR_DIRTY );
        sqlite3PcacheMakeClean(p);
      }
    }
    எனில்( pgno==0 && pCache->nRefSum ){
      sqlite3_pcache_page *pPage1;
      pPage1 = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache,1,0);
      எனில்( ALWAYS(pPage1) ){  /* Page 1 is always available in cache, because
                             ** pCache->nRefSum>0 */
        memset(pPage1->pBuf, 0, pCache->szPage);
        pgno = 1;
      }
    }
    sqlite3GlobalConfig.pcache2.xTruncate(pCache->pCache, pgno+1);
  }
}

/*
** Close a cache.
*/
வெறுமை sqlite3PcacheClose(PCache *pCache){
  ப்ரதிஜ்ஞா( pCache->pCache!=0 );
  pcacheTrace(("%p.CLOSE\n",pCache));
  sqlite3GlobalConfig.pcache2.xDestroy(pCache->pCache);
}

/* 
** Discard the contents of the cache.
*/
வெறுமை sqlite3PcacheClear(PCache *pCache){
  sqlite3PcacheTruncate(pCache, 0);
}

/*
** Merge two lists of pages connected by pDirty and in pgno order.
** Do not bother fixing the pDirtyPrev pointers.
*/
நிலையான PgHdr *pcacheMergeDirtyList(PgHdr *pA, PgHdr *pB){
  PgHdr result, *pTail;
  pTail = &result;
  ப்ரதிஜ்ஞா( pA!=0 && pB!=0 );
  க்கு(;;){
    எனில்( pA->pgno<pB->pgno ){
      pTail->pDirty = pA;
      pTail = pA;
      pA = pA->pDirty;
      எனில்( pA==0 ){
        pTail->pDirty = pB;
        நிறுத்து;
      }
    }இல்லையெனில்{
      pTail->pDirty = pB;
      pTail = pB;
      pB = pB->pDirty;
      எனில்( pB==0 ){
        pTail->pDirty = pA;
        நிறுத்து;
      }
    }
  }
  திரும்பு result.pDirty;
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
#define N_SORT_BUCKET  32
நிலையான PgHdr *pcacheSortDirtyList(PgHdr *pIn){
  PgHdr *a[N_SORT_BUCKET], *p;
  முழு i;
  memset(a, 0, அளவு(a));
  போது( pIn ){
    p = pIn;
    pIn = p->pDirty;
    p->pDirty = 0;
    க்கு(i=0; ALWAYS(i<N_SORT_BUCKET-1); i++){
      எனில்( a[i]==0 ){
        a[i] = p;
        நிறுத்து;
      }இல்லையெனில்{
        p = pcacheMergeDirtyList(a[i], p);
        a[i] = 0;
      }
    }
    எனில்( NEVER(i==N_SORT_BUCKET-1) ){
      /* To get here, there need to be 2^(N_SORT_BUCKET) elements in
      ** the input list.  But that is impossible.
      */
      a[i] = pcacheMergeDirtyList(a[i], p);
    }
  }
  p = a[0];
  க்கு(i=1; i<N_SORT_BUCKET; i++){
    எனில்( a[i]==0 ) தொடர்;
    p = p ? pcacheMergeDirtyList(p, a[i]) : a[i];
  }
  திரும்பு p;
}

/*
** Return a list of all dirty pages in the cache, sorted by page number.
*/
PgHdr *sqlite3PcacheDirtyList(PCache *pCache){
  PgHdr *p;
  க்கு(p=pCache->pDirty; p; p=p->pDirtyNext){
    p->pDirty = p->pDirtyNext;
  }
  திரும்பு pcacheSortDirtyList(pCache->pDirty);
}

/* 
** Return the total number of references to all pages held by the cache.
**
** This is not the total number of pages referenced, but the sum of the
** reference count for all pages.
*/
i64 sqlite3_Pcache_Ref_மொத்தம்(PCache *pCache){
  திரும்பு pCache->nRefSum;
}

/*
** Return the number of references to the page supplied as an argument.
*/
i64 sqlite3PcachePageRefcount(PgHdr *p){
  திரும்பு p->nRef;
}

/* 
** Return the total number of pages in the cache.
*/
முழு sqlite3PcachePagecount(PCache *pCache){
  ப்ரதிஜ்ஞா( pCache->pCache!=0 );
  திரும்பு sqlite3GlobalConfig.pcache2.xPagecount(pCache->pCache);
}

#ifdef SQLITE_TEST
/*
** Get the suggested cache-size value.
*/
முழு sqlite3PcacheGetCachesize(PCache *pCache){
  திரும்பு எண்_Of_Cache_Pages(pCache);
}
#endif

/*
** Set the suggested cache-size value.
*/
வெறுமை sqlite3PcacheSetCachesize(PCache *pCache, முழு mxPage){
  ப்ரதிஜ்ஞா( pCache->pCache!=0 );
  pCache->szCache = mxPage;
  sqlite3GlobalConfig.pcache2.xCachesize(pCache->pCache,
                                         எண்_Of_Cache_Pages(pCache));
}

/*
** Set the suggested cache-spill value.  Make no changes if if the
** argument is zero.  Return the effective cache-spill size, which will
** be the larger of the szSpill and szCache.
*/
முழு sqlite3PcacheSetSpillsize(PCache *p, முழு mxPage){
  முழு res;
  ப்ரதிஜ்ஞா( p->pCache!=0 );
  எனில்( mxPage ){
    எனில்( mxPage<0 ){
      mxPage = (முழு)((-1024*(i64)mxPage)/(p->szPage+p->szExtra));
    }
    p->szSpill = mxPage;
  }
  res = எண்_Of_Cache_Pages(p);
  எனில்( res<p->szSpill ) res = p->szSpill; 
  திரும்பு res;
}

/*
** Free up as much memory as possible from the page cache.
*/
வெறுமை sqlite3PcacheShrink(PCache *pCache){
  ப்ரதிஜ்ஞா( pCache->pCache!=0 );
  sqlite3GlobalConfig.pcache2.xShrink(pCache->pCache);
}

/*
** Return the size of the header added by this middleware layer
** in the page-cache hierarchy.
*/
முழு sqlite3HeaderSizePcache(வெறுமை){ திரும்பு ROUND8(அளவு(PgHdr)); }

/*
** Return the number of dirty pages currently in the cache, as a percentage
** of the configured cache size.
*/
முழு sqlite3PCachePercentDirty(PCache *pCache){
  PgHdr *pDirty;
  முழு nDirty = 0;
  முழு nCache = எண்_Of_Cache_Pages(pCache);
  க்கு(pDirty=pCache->pDirty; pDirty; pDirty=pDirty->pDirtyNext) nDirty++;
  திரும்பு nCache ? (முழு)(((i64)nDirty * 100) / nCache) : 0;
}

#ifdef SQLITE_DIRECT_OVERFLOW_READ
/* 
** Return true if there are one or more dirty pages in the cache. Else false.
*/
முழு sqlite3PCacheIsDirty(PCache *pCache){
  திரும்பு (pCache->pDirty!=0);
}
#endif

#எனில் defined(SQLITE_CHECK_PAGES) || defined(SQLITE_DEBUG)
/*
** For all dirty pages currently in the cache, invoke the specified
** callback. This is only used if the SQLITE_CHECK_PAGES macro is
** defined.
*/
வெறுமை sqlite3PcacheIterateDirty(PCache *pCache, வெறுமை (*xIter)(PgHdr *)){
  PgHdr *pDirty;
  க்கு(pDirty=pCache->pDirty; pDirty; pDirty=pDirty->pDirtyNext){
    xIter(pDirty);
  }
}
#endif
