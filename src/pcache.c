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
#ضمن "sqliteInt.h"

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
بنية PCache {
  PgHdr *pDirty, *p_Dirty_پوچھ;         /* List of dirty pages in LRU order */
  PgHdr *pSynced;                     /* Last synced page in dirty page list */
  i64 n_Ref_جوڑ;                        /* Sum of ref counts over all pages */
  صحيح szCache;                        /* Configured cache size */
  صحيح szSpill;                        /* Size before spilling occurs */
  صحيح sz_ورقہ;                         /* Size of every page in this cache */
  صحيح szExtra;                        /* Size of extra space for each page */
  u8 bPurgeable;                      /* True if pages are on backing store */
  u8 eCreate;                         /* eCreate value for for xFetch() */
  صحيح (*xStress)(فراغ*,PgHdr*);       /* Call to try make a page clean */
  فراغ *pStress;                      /* Argument to xStress */
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
#إذا defined(SQLITE_DEBUG) && 0
  صحيح sqlite3PcacheTrace = 2;       /* 0: off  1: simple  2: cache dumps */
  صحيح sqlite3PcacheMxDump = 9999;   /* Max cache entries for pcacheDump() */
# define pcacheTrace(X) إذا(sqlite3PcacheTrace){sqlite3DebugPrintf X;}
  ساكن فراغ pcache_ورقہ_Trace(صحيح i, sqlite3_pcache_ورقہ *p_ہیٹھلا){
    PgHdr *pPg;
    بنانشانی حرف *a;
    صحيح j;
    إذا( p_ہیٹھلا==0 ){
      لکھو("%3d: NULL\n", i);
    }وإلا{
      pPg = (PgHdr*)p_ہیٹھلا->pExtra;
      لکھو("%3d: nRef %2lld flgs %02x data ", i, pPg->nRef, pPg->flags);
      a = (بنانشانی حرف *)p_ہیٹھلا->pBuf;
      لكل(j=0; j<12; j++) لکھو("%02x", a[j]);
      لکھو(" ptr %p\n", pPg);
    }
  }
  ساكن فراغ pcacheDump(PCache *pCache){
    صحيح N;
    صحيح i;
    sqlite3_pcache_ورقہ *p_ہیٹھلا;
  
    إذا( sqlite3PcacheTrace<2 ) أرجع;
    إذا( pCache->pCache==0 ) أرجع;
    N = sqlite3PcachePagecount(pCache);
    إذا( N>sqlite3PcacheMxDump ) N = sqlite3PcacheMxDump;
    لكل(i=1; i<=N; i++){
       p_ہیٹھلا = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, i, 0);
       pcache_ورقہ_Trace(i, p_ہیٹھلا);
       إذا( p_ہیٹھلا && ((PgHdr*)p_ہیٹھلا)->p_ورقہ==0 ){
         sqlite3GlobalConfig.pcache2.xUnpin(pCache->pCache, p_ہیٹھلا, 0);
       }
    }
  }
#وإلا
# define pcacheTrace(X)
# define pcache_ورقہ_Trace(PGNO, X)
# define pcacheDump(X)
#جے_انت

/*
** Return 1 if pPg is on the dirty list for pCache.  Return 0 if not.
** This routine runs inside of assert() statements only.
*/
#إذا defined(SQLITE_ENABLE_EXPENSIVE_ASSERT)
ساكن صحيح ورقہ_On_Dirty_List(PCache *pCache, PgHdr *pPg){
  PgHdr *p;
  لكل(p=pCache->pDirty; p; p=p->p_Dirty_اگلا){
    إذا( p==pPg ) أرجع 1;
  }
  أرجع 0;
}
ساكن صحيح ورقہ_Not_On_Dirty_List(PCache *pCache, PgHdr *pPg){
  PgHdr *p;
  لكل(p=pCache->pDirty; p; p=p->p_Dirty_اگلا){
    إذا( p==pPg ) أرجع 0;
  }
  أرجع 1;
}
#وإلا
# define ورقہ_On_Dirty_List(A,B)    1
# define ورقہ_Not_On_Dirty_List(A,B) 1
#جے_انت

/*
** Check invariants on a PgHdr entry.  Return true if everything is OK.
** Return false if any invariant is violated.
**
** This routine is for use inside of assert() statements only.  For
** example:
**
**          assert( sqlite3PcachePageSanity(pPg) );
*/
#جے_تعریف_شدہ SQLITE_DEBUG
صحيح sqlite3_Pcache_ورقہ_Sanity(PgHdr *pPg){
  PCache *pCache;
  أكد( pPg!=0 );
  أكد( pPg->pgno>0 || pPg->pPager==0 );    /* Page number is 1 or more */
  pCache = pPg->pCache;
  أكد( pCache!=0 );      /* Every page has an associated PCache */
  إذا( pPg->flags & PGHDR_CLEAN ){
    أكد( (pPg->flags & PGHDR_DIRTY)==0 );/* Cannot be both CLEAN and DIRTY */
    أكد( ورقہ_Not_On_Dirty_List(pCache, pPg) );/* CLEAN pages not on dirtylist */
  }وإلا{
    أكد( (pPg->flags & PGHDR_DIRTY)!=0 );/* If not CLEAN must be DIRTY */
    أكد( pPg->p_Dirty_اگلا==0 || pPg->p_Dirty_اگلا->pDirtyPrev==pPg );
    أكد( pPg->pDirtyPrev==0 || pPg->pDirtyPrev->p_Dirty_اگلا==pPg );
    أكد( pPg->pDirtyPrev!=0 || pCache->pDirty==pPg );
    أكد( ورقہ_On_Dirty_List(pCache, pPg) );
  }
  /* WRITEABLE pages must also be DIRTY */
  إذا( pPg->flags & PGHDR_WRITEABLE ){
    أكد( pPg->flags & PGHDR_DIRTY );     /* WRITEABLE implies DIRTY */
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
  أرجع 1;
}
#جے_انت /* SQLITE_DEBUG */


/********************************** Linked List Management ********************/

/* Allowed values for second argument to pcacheManageDirtyList() */
#تعریف_کرو PCACHE_DIRTYLIST_REMOVE   1    /* Remove pPage from dirty list */
#تعریف_کرو PCACHE_DIRTYLIST_جوڑو      2    /* Add pPage to the dirty list */
#تعریف_کرو PCACHE_DIRTYLIST_FRONT    3    /* Move pPage to the front of the list */

/*
** Manage pPage's participation on the dirty list.  Bits of the addRemove
** argument determines what operation to do.  The 0x01 bit means first
** remove pPage from the dirty list.  The 0x02 means add pPage back to
** the dirty list.  Doing both moves pPage to the front of the dirty list.
*/
ساكن فراغ pcacheManageDirtyList(PgHdr *p_ورقہ, u8 جوڑو_Remove){
  PCache *p = p_ورقہ->pCache;

  pcacheTrace(("%p.DIRTYLIST.%s %d\n", p,
                جوڑو_Remove==1 ? "REMOVE" : جوڑو_Remove==2 ? "ADD" : "FRONT",
                p_ورقہ->pgno));
  إذا( جوڑو_Remove & PCACHE_DIRTYLIST_REMOVE ){
    أكد( p_ورقہ->p_Dirty_اگلا || p_ورقہ==p->p_Dirty_پوچھ );
    أكد( p_ورقہ->pDirtyPrev || p_ورقہ==p->pDirty );
  
    /* Update the PCache1.pSynced variable if necessary. */
    إذا( p->pSynced==p_ورقہ ){
      p->pSynced = p_ورقہ->pDirtyPrev;
    }
  
    إذا( p_ورقہ->p_Dirty_اگلا ){
      p_ورقہ->p_Dirty_اگلا->pDirtyPrev = p_ورقہ->pDirtyPrev;
    }وإلا{
      أكد( p_ورقہ==p->p_Dirty_پوچھ );
      p->p_Dirty_پوچھ = p_ورقہ->pDirtyPrev;
    }
    إذا( p_ورقہ->pDirtyPrev ){
      p_ورقہ->pDirtyPrev->p_Dirty_اگلا = p_ورقہ->p_Dirty_اگلا;
    }وإلا{
      /* If there are now no dirty pages in the cache, set eCreate to 2. 
      ** This is an optimization that allows sqlite3PcacheFetch() to skip
      ** searching for a dirty page to eject from the cache when it might
      ** otherwise have to.  */
      أكد( p_ورقہ==p->pDirty );
      p->pDirty = p_ورقہ->p_Dirty_اگلا;
      أكد( p->bPurgeable || p->eCreate==2 );
      إذا( p->pDirty==0 ){         /*OPTIMIZATION-IF-TRUE*/
        أكد( p->bPurgeable==0 || p->eCreate==1 );
        p->eCreate = 2;
      }
    }
  }
  إذا( جوڑو_Remove & PCACHE_DIRTYLIST_جوڑو ){
    p_ورقہ->pDirtyPrev = 0;
    p_ورقہ->p_Dirty_اگلا = p->pDirty;
    إذا( p_ورقہ->p_Dirty_اگلا ){
      أكد( p_ورقہ->p_Dirty_اگلا->pDirtyPrev==0 );
      p_ورقہ->p_Dirty_اگلا->pDirtyPrev = p_ورقہ;
    }وإلا{
      p->p_Dirty_پوچھ = p_ورقہ;
      إذا( p->bPurgeable ){
        أكد( p->eCreate==2 );
        p->eCreate = 1;
      }
    }
    p->pDirty = p_ورقہ;

    /* If pSynced is NULL and this page has a clear NEED_SYNC flag, set
    ** pSynced to point to it. Checking the NEED_SYNC flag is an 
    ** optimization, as if pSynced points to a page with the NEED_SYNC
    ** flag set sqlite3PcacheFetchStress() searches through all newer 
    ** entries of the dirty-list for a page with NEED_SYNC clear anyway.  */
    إذا( !p->pSynced 
     && 0==(p_ورقہ->flags&PGHDR_NEED_SYNC)   /*OPTIMIZATION-IF-FALSE*/
    ){
      p->pSynced = p_ورقہ;
    }
  }
  pcacheDump(p);
}

/*
** Wrapper around the pluggable caches xUnpin method. If the cache is
** being used for an in-memory database, this function is a no-op.
*/
ساكن فراغ pcacheUnpin(PgHdr *p){
  إذا( p->pCache->bPurgeable ){
    pcacheTrace(("%p.UNPIN %d\n", p->pCache, p->pgno));
    sqlite3GlobalConfig.pcache2.xUnpin(p->pCache->pCache, p->p_ورقہ, 0);
    pcacheDump(p->pCache);
  }
}

/*
** Compute the number of pages of cache requested.   p->szCache is the
** cache size requested by the "PRAGMA cache_size" statement.
*/
ساكن صحيح نمبر_دا_Cache_ورقے(PCache *p){
  إذا( p->szCache>=0 ){
    /* IMPLEMENTATION-OF: R-42059-47211 If the argument N is positive then the
    ** suggested cache size is set to N. */
    أرجع p->szCache;
  }وإلا{
    i64 n;
    /* IMPLEMENTATION-OF: R-59858-46238 If the argument N is negative, then the
    ** number of cache pages is adjusted to be a number of pages that would
    ** use approximately abs(N*1024) bytes of memory based on the current
    ** page size. */
    n = ((-1024*(i64)p->szCache)/(p->sz_ورقہ+p->szExtra));
    إذا( n>1000000000 ) n = 1000000000;
    أرجع (صحيح)n;
  }
}

/*************************************************** General Interfaces ******
**
** Initialize and shutdown the page cache subsystem. Neither of these 
** functions are threadsafe.
*/
صحيح sqlite3PcacheInitialize(فراغ){
  إذا( sqlite3GlobalConfig.pcache2.xInit==0 ){
    /* IMPLEMENTATION-OF: R-26801-64137 If the xInit() method is NULL, then the
    ** built-in default page cache is used instead of the application defined
    ** page cache. */
    sqlite3_PCache_سیٹ_Default();
    أكد( sqlite3GlobalConfig.pcache2.xInit!=0 );
  }
  أرجع sqlite3GlobalConfig.pcache2.xInit(sqlite3GlobalConfig.pcache2.pArg);
}
فراغ sqlite3PcacheShutdown(فراغ){
  إذا( sqlite3GlobalConfig.pcache2.xShutdown ){
    /* IMPLEMENTATION-OF: R-26000-56589 The xShutdown() method may be NULL. */
    sqlite3GlobalConfig.pcache2.xShutdown(sqlite3GlobalConfig.pcache2.pArg);
  }
}

/*
** Return the size in bytes of a PCache object.
*/
صحيح sqlite3_Pcache_ناپ(فراغ){ أرجع حجم(PCache); }

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
صحيح sqlite3PcacheOpen(
  صحيح sz_ورقہ,                  /* Size of every page */
  صحيح szExtra,                 /* Extra space associated with each page */
  صحيح bPurgeable,              /* True if pages are on backing store */
  صحيح (*xStress)(فراغ*,PgHdr*),/* Call to try to make pages clean */
  فراغ *pStress,               /* Argument to xStress */
  PCache *p                    /* Preallocated space for the PCache */
){
  memset(p, 0, حجم(PCache));
  p->sz_ورقہ = 1;
  p->szExtra = szExtra;
  أكد( szExtra>=8 );  /* First 8 bytes will be zeroed */
  p->bPurgeable = bPurgeable;
  p->eCreate = 2;
  p->xStress = xStress;
  p->pStress = pStress;
  p->szCache = 100;
  p->szSpill = 1;
  pcacheTrace(("%p.OPEN szPage %d bPurgeable %d\n",p,sz_ورقہ,bPurgeable));
  أرجع sqlite3_Pcache_سیٹ_ورقہ_ناپ(p, sz_ورقہ);
}

/*
** Change the page size for PCache object. The caller must ensure that there
** are no outstanding page references when this function is called.
*/
صحيح sqlite3_Pcache_سیٹ_ورقہ_ناپ(PCache *pCache, صحيح sz_ورقہ){
  أكد( pCache->n_Ref_جوڑ==0 && pCache->pDirty==0 );
  إذا( pCache->sz_ورقہ ){
    sqlite3_pcache *pNew;
    pNew = sqlite3GlobalConfig.pcache2.xCreate(
                sz_ورقہ, pCache->szExtra + ROUND8(حجم(PgHdr)),
                pCache->bPurgeable
    );
    إذا( pNew==0 ) أرجع SQLITE_NOMEM_BKPT;
    sqlite3GlobalConfig.pcache2.xCachesize(pNew, نمبر_دا_Cache_ورقے(pCache));
    إذا( pCache->pCache ){
      sqlite3GlobalConfig.pcache2.xDestroy(pCache->pCache);
    }
    pCache->pCache = pNew;
    pCache->sz_ورقہ = sz_ورقہ;
    pcacheTrace(("%p.PAGESIZE %d\n",pCache,sz_ورقہ));
  }
  أرجع SQLITE_OK;
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
sqlite3_pcache_ورقہ *sqlite3PcacheFetch(
  PCache *pCache,       /* Obtain the page from this cache */
  Pgno pgno,            /* Page number to obtain */
  صحيح createFlag        /* If true, create page if it does not exist already */
){
  صحيح eCreate;
  sqlite3_pcache_ورقہ *pRes;

  أكد( pCache!=0 );
  أكد( pCache->pCache!=0 );
  أكد( createFlag==3 || createFlag==0 );
  أكد( pCache->eCreate==((pCache->bPurgeable && pCache->pDirty) ? 1 : 2) );

  /* eCreate defines what to do if the page does not exist.
  **    0     Do not allocate a new page.  (createFlag==0)
  **    1     Allocate a new page if doing so is inexpensive.
  **          (createFlag==1 AND bPurgeable AND pDirty)
  **    2     Allocate a new page even it doing so is difficult.
  **          (createFlag==1 AND !(bPurgeable AND pDirty)
  */
  eCreate = createFlag & pCache->eCreate;
  أكد( eCreate==0 || eCreate==1 || eCreate==2 );
  أكد( createFlag==0 || pCache->eCreate==eCreate );
  أكد( createFlag==0 || eCreate==1+(!pCache->bPurgeable||!pCache->pDirty) );
  pRes = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, pgno, eCreate);
  pcacheTrace(("%p.FETCH %d%s (result: %p) ",pCache,pgno,
               createFlag?" create":"",pRes));
  pcache_ورقہ_Trace(pgno, pRes);
  أرجع pRes;
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
صحيح sqlite3PcacheFetchStress(
  PCache *pCache,                 /* Obtain the page from this cache */
  Pgno pgno,                      /* Page number to obtain */
  sqlite3_pcache_ورقہ **pp_ورقہ    /* Write result here */
){
  PgHdr *pPg;
  إذا( pCache->eCreate==2 ) أرجع 0;

  إذا( sqlite3PcachePagecount(pCache)>pCache->szSpill ){
    /* Find a dirty page to write-out and recycle. First try to find a 
    ** page that does not require a journal-sync (one with PGHDR_NEED_SYNC
    ** cleared), but if that is not possible settle for any other 
    ** unreferenced dirty page.
    **
    ** If the LRU page in the dirty list that has a clear PGHDR_NEED_SYNC
    ** flag is currently referenced, then the following may leave pSynced
    ** set incorrectly (pointing to other than the LRU page with NEED_SYNC
    ** cleared). This is Ok, as pSynced is just an optimization.  */
    لكل(pPg=pCache->pSynced; 
        pPg && (pPg->nRef || (pPg->flags&PGHDR_NEED_SYNC)); 
        pPg=pPg->pDirtyPrev
    );
    pCache->pSynced = pPg;
    إذا( !pPg ){
      لكل(pPg=pCache->p_Dirty_پوچھ; pPg && pPg->nRef; pPg=pPg->pDirtyPrev);
    }
    إذا( pPg ){
      صحيح rc;
#جے_تعریف_شدہ SQLITE_LOG_CACHE_SPILL
      sqlite3_log(SQLITE_بھریا, 
                  "spill page %d making room for %d - cache used: %d/%d",
                  pPg->pgno, pgno,
                  sqlite3GlobalConfig.pcache2.xPagecount(pCache->pCache),
                نمبر_دا_Cache_ورقے(pCache));
#جے_انت
      pcacheTrace(("%p.SPILL %d\n",pCache,pPg->pgno));
      rc = pCache->xStress(pCache->pStress, pPg);
      pcacheDump(pCache);
      إذا( rc!=SQLITE_OK && rc!=SQLITE_BUSY ){
        أرجع rc;
      }
    }
  }
  *pp_ورقہ = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, pgno, 2);
  أرجع *pp_ورقہ==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
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
ساكن SQLITE_NOINLINE PgHdr *pcache_Fetch_انت_With_Init(
  PCache *pCache,             /* Obtain the page from this cache */
  Pgno pgno,                  /* Page number obtained */
  sqlite3_pcache_ورقہ *p_ورقہ  /* Page obtained by prior PcacheFetch() call */
){
  PgHdr *pPgHdr;
  أكد( p_ورقہ!=0 );
  pPgHdr = (PgHdr*)p_ورقہ->pExtra;
  أكد( pPgHdr->p_ورقہ==0 );
  memset(&pPgHdr->pDirty, 0, حجم(PgHdr) - offsetof(PgHdr,pDirty));
  pPgHdr->p_ورقہ = p_ورقہ;
  pPgHdr->p_انکڑے = p_ورقہ->pBuf;
  pPgHdr->pExtra = (فراغ *)&pPgHdr[1];
  memset(pPgHdr->pExtra, 0, 8);
  أكد( EIGHT_BYTE_ALIGNMENT( pPgHdr->pExtra ) );
  pPgHdr->pCache = pCache;
  pPgHdr->pgno = pgno;
  pPgHdr->flags = PGHDR_CLEAN;
  أرجع sqlite3_Pcache_Fetch_انت(pCache,pgno,p_ورقہ);
}

/*
** This routine converts the sqlite3_pcache_page object returned by
** sqlite3PcacheFetch() into an initialized PgHdr object.  This routine
** must be called after sqlite3PcacheFetch() in order to get a usable
** result.
*/
PgHdr *sqlite3_Pcache_Fetch_انت(
  PCache *pCache,             /* Obtain the page from this cache */
  Pgno pgno,                  /* Page number obtained */
  sqlite3_pcache_ورقہ *p_ورقہ  /* Page obtained by prior PcacheFetch() call */
){
  PgHdr *pPgHdr;

  أكد( p_ورقہ!=0 );
  pPgHdr = (PgHdr *)p_ورقہ->pExtra;

  إذا( !pPgHdr->p_ورقہ ){
    أرجع pcache_Fetch_انت_With_Init(pCache, pgno, p_ورقہ);
  }
  pCache->n_Ref_جوڑ++;
  pPgHdr->nRef++;
  أكد( sqlite3_Pcache_ورقہ_Sanity(pPgHdr) );
  أرجع pPgHdr;
}

/*
** Decrement the reference count on a page. If the page is clean and the
** reference count drops to 0, then it is made eligible for recycling.
*/
فراغ SQLITE_NOINLINE sqlite3PcacheRelease(PgHdr *p){
  أكد( p->nRef>0 );
  p->pCache->n_Ref_جوڑ--;
  إذا( (--p->nRef)==0 ){
    إذا( p->flags&PGHDR_CLEAN ){
      pcacheUnpin(p);
    }وإلا{
      pcacheManageDirtyList(p, PCACHE_DIRTYLIST_FRONT);
      أكد( sqlite3_Pcache_ورقہ_Sanity(p) );
    }
  }
}

/*
** Increase the reference count of a supplied page by 1.
*/
فراغ sqlite3PcacheRef(PgHdr *p){
  أكد(p->nRef>0);
  أكد( sqlite3_Pcache_ورقہ_Sanity(p) );
  p->nRef++;
  p->pCache->n_Ref_جوڑ++;
}

/*
** Drop a page from the cache. There must be exactly one reference to the
** page. This function deletes that reference, so after it returns the
** page pointed to by p is invalid.
*/
فراغ sqlite3PcacheDrop(PgHdr *p){
  أكد( p->nRef==1 );
  أكد( sqlite3_Pcache_ورقہ_Sanity(p) );
  إذا( p->flags&PGHDR_DIRTY ){
    pcacheManageDirtyList(p, PCACHE_DIRTYLIST_REMOVE);
  }
  p->pCache->n_Ref_جوڑ--;
  sqlite3GlobalConfig.pcache2.xUnpin(p->pCache->pCache, p->p_ورقہ, 1);
}

/*
** Make sure the page is marked as dirty. If it isn't dirty already,
** make it so.
*/
فراغ sqlite3PcacheMakeDirty(PgHdr *p){
  أكد( p->nRef>0 );
  أكد( sqlite3_Pcache_ورقہ_Sanity(p) );
  إذا( p->flags & (PGHDR_CLEAN|PGHDR_DONT_WRITE) ){    /*OPTIMIZATION-IF-FALSE*/
    p->flags &= ~PGHDR_DONT_WRITE;
    إذا( p->flags & PGHDR_CLEAN ){
      p->flags ^= (PGHDR_DIRTY|PGHDR_CLEAN);
      pcacheTrace(("%p.DIRTY %d\n",p->pCache,p->pgno));
      أكد( (p->flags & (PGHDR_DIRTY|PGHDR_CLEAN))==PGHDR_DIRTY );
      pcacheManageDirtyList(p, PCACHE_DIRTYLIST_جوڑو);
      أكد( sqlite3_Pcache_ورقہ_Sanity(p) );
    }
    أكد( sqlite3_Pcache_ورقہ_Sanity(p) );
  }
}

/*
** Make sure the page is marked as clean. If it isn't clean already,
** make it so.
*/
فراغ sqlite3PcacheMakeClean(PgHdr *p){
  أكد( sqlite3_Pcache_ورقہ_Sanity(p) );
  أكد( (p->flags & PGHDR_DIRTY)!=0 );
  أكد( (p->flags & PGHDR_CLEAN)==0 );
  pcacheManageDirtyList(p, PCACHE_DIRTYLIST_REMOVE);
  p->flags &= ~(PGHDR_DIRTY|PGHDR_NEED_SYNC|PGHDR_WRITEABLE);
  p->flags |= PGHDR_CLEAN;
  pcacheTrace(("%p.CLEAN %d\n",p->pCache,p->pgno));
  أكد( sqlite3_Pcache_ورقہ_Sanity(p) );
  إذا( p->nRef==0 ){
    pcacheUnpin(p);
  }
}

/*
** Make every page in the cache clean.
*/
فراغ sqlite3PcacheCleanAll(PCache *pCache){
  PgHdr *p;
  pcacheTrace(("%p.CLEAN-ALL\n",pCache));
  طالما( (p = pCache->pDirty)!=0 ){
    sqlite3PcacheMakeClean(p);
  }
}

/*
** Clear the PGHDR_NEED_SYNC and PGHDR_WRITEABLE flag from all dirty pages.
*/
فراغ sqlite3PcacheClearWritable(PCache *pCache){
  PgHdr *p;
  pcacheTrace(("%p.CLEAR-WRITEABLE\n",pCache));
  لكل(p=pCache->pDirty; p; p=p->p_Dirty_اگلا){
    p->flags &= ~(PGHDR_NEED_SYNC|PGHDR_WRITEABLE);
  }
  pCache->pSynced = pCache->p_Dirty_پوچھ;
}

/*
** Clear the PGHDR_NEED_SYNC flag from all dirty pages.
*/
فراغ sqlite3PcacheClearSyncFlags(PCache *pCache){
  PgHdr *p;
  لكل(p=pCache->pDirty; p; p=p->p_Dirty_اگلا){
    p->flags &= ~PGHDR_NEED_SYNC;
  }
  pCache->pSynced = pCache->p_Dirty_پوچھ;
}

/*
** Change the page number of page p to newPgno. 
*/
فراغ sqlite3PcacheMove(PgHdr *p, Pgno newPgno){
  PCache *pCache = p->pCache;
  sqlite3_pcache_ورقہ *pOther;
  أكد( p->nRef>0 );
  أكد( newPgno>0 );
  أكد( sqlite3_Pcache_ورقہ_Sanity(p) );
  pcacheTrace(("%p.MOVE %d -> %d\n",pCache,p->pgno,newPgno));
  pOther = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, newPgno, 0);
  إذا( pOther ){
    PgHdr *pXPage = (PgHdr*)pOther->pExtra;
    أكد( pXPage->nRef==0 );
    pXPage->nRef++;
    pCache->n_Ref_جوڑ++;
    sqlite3PcacheDrop(pXPage);
  }
  sqlite3GlobalConfig.pcache2.xRekey(pCache->pCache, p->p_ورقہ, p->pgno,newPgno);
  p->pgno = newPgno;
  إذا( (p->flags&PGHDR_DIRTY) && (p->flags&PGHDR_NEED_SYNC) ){
    pcacheManageDirtyList(p, PCACHE_DIRTYLIST_FRONT);
    أكد( sqlite3_Pcache_ورقہ_Sanity(p) );
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
فراغ sqlite3PcacheTruncate(PCache *pCache, Pgno pgno){
  إذا( pCache->pCache ){
    PgHdr *p;
    PgHdr *p_اگلا;
    pcacheTrace(("%p.TRUNCATE %d\n",pCache,pgno));
    لكل(p=pCache->pDirty; p; p=p_اگلا){
      p_اگلا = p->p_Dirty_اگلا;
      /* This routine never gets call with a positive pgno except right
      ** after sqlite3PcacheCleanAll().  So if there are dirty pages,
      ** it must be that pgno==0.
      */
      أكد( p->pgno>0 );
      إذا( p->pgno>pgno ){
        أكد( p->flags&PGHDR_DIRTY );
        sqlite3PcacheMakeClean(p);
      }
    }
    إذا( pgno==0 && pCache->n_Ref_جوڑ ){
      sqlite3_pcache_ورقہ *pPage1;
      pPage1 = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache,1,0);
      إذا( ALWAYS(pPage1) ){  /* Page 1 is always available in cache, because
                             ** pCache->nRefSum>0 */
        memset(pPage1->pBuf, 0, pCache->sz_ورقہ);
        pgno = 1;
      }
    }
    sqlite3GlobalConfig.pcache2.xTruncate(pCache->pCache, pgno+1);
  }
}

/*
** Close a cache.
*/
فراغ sqlite3PcacheClose(PCache *pCache){
  أكد( pCache->pCache!=0 );
  pcacheTrace(("%p.CLOSE\n",pCache));
  sqlite3GlobalConfig.pcache2.xDestroy(pCache->pCache);
}

/* 
** Discard the contents of the cache.
*/
فراغ sqlite3PcacheClear(PCache *pCache){
  sqlite3PcacheTruncate(pCache, 0);
}

/*
** Merge two lists of pages connected by pDirty and in pgno order.
** Do not bother fixing the pDirtyPrev pointers.
*/
ساكن PgHdr *pcache_ملاؤ_Dirty_List(PgHdr *pA, PgHdr *pB){
  PgHdr نتیجہ, *p_پوچھ;
  p_پوچھ = &نتیجہ;
  أكد( pA!=0 && pB!=0 );
  لكل(;;){
    إذا( pA->pgno<pB->pgno ){
      p_پوچھ->pDirty = pA;
      p_پوچھ = pA;
      pA = pA->pDirty;
      إذا( pA==0 ){
        p_پوچھ->pDirty = pB;
        اكسر;
      }
    }وإلا{
      p_پوچھ->pDirty = pB;
      p_پوچھ = pB;
      pB = pB->pDirty;
      إذا( pB==0 ){
        p_پوچھ->pDirty = pA;
        اكسر;
      }
    }
  }
  أرجع نتیجہ.pDirty;
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
#تعریف_کرو N_ترتیب_دیو_بالٹی  32
ساكن PgHdr *pcache_ترتیب_دیو_Dirty_List(PgHdr *p_وچ){
  PgHdr *a[N_ترتیب_دیو_بالٹی], *p;
  صحيح i;
  memset(a, 0, حجم(a));
  طالما( p_وچ ){
    p = p_وچ;
    p_وچ = p->pDirty;
    p->pDirty = 0;
    لكل(i=0; ALWAYS(i<N_ترتیب_دیو_بالٹی-1); i++){
      إذا( a[i]==0 ){
        a[i] = p;
        اكسر;
      }وإلا{
        p = pcache_ملاؤ_Dirty_List(a[i], p);
        a[i] = 0;
      }
    }
    إذا( NEVER(i==N_ترتیب_دیو_بالٹی-1) ){
      /* To get here, there need to be 2^(N_SORT_BUCKET) elements in
      ** the input list.  But that is impossible.
      */
      a[i] = pcache_ملاؤ_Dirty_List(a[i], p);
    }
  }
  p = a[0];
  لكل(i=1; i<N_ترتیب_دیو_بالٹی; i++){
    إذا( a[i]==0 ) استمر;
    p = p ? pcache_ملاؤ_Dirty_List(p, a[i]) : a[i];
  }
  أرجع p;
}

/*
** Return a list of all dirty pages in the cache, sorted by page number.
*/
PgHdr *sqlite3PcacheDirtyList(PCache *pCache){
  PgHdr *p;
  لكل(p=pCache->pDirty; p; p=p->p_Dirty_اگلا){
    p->pDirty = p->p_Dirty_اگلا;
  }
  أرجع pcache_ترتیب_دیو_Dirty_List(pCache->pDirty);
}

/* 
** Return the total number of references to all pages held by the cache.
**
** This is not the total number of pages referenced, but the sum of the
** reference count for all pages.
*/
i64 sqlite3_Pcache_Ref_گنتی(PCache *pCache){
  أرجع pCache->n_Ref_جوڑ;
}

/*
** Return the number of references to the page supplied as an argument.
*/
i64 sqlite3_Pcache_ورقہ_Refcount(PgHdr *p){
  أرجع p->nRef;
}

/* 
** Return the total number of pages in the cache.
*/
صحيح sqlite3PcachePagecount(PCache *pCache){
  أكد( pCache->pCache!=0 );
  أرجع sqlite3GlobalConfig.pcache2.xPagecount(pCache->pCache);
}

#جے_تعریف_شدہ SQLITE_TEST
/*
** Get the suggested cache-size value.
*/
صحيح sqlite3PcacheGetCachesize(PCache *pCache){
  أرجع نمبر_دا_Cache_ورقے(pCache);
}
#جے_انت

/*
** Set the suggested cache-size value.
*/
فراغ sqlite3_Pcache_سیٹ_Cachesize(PCache *pCache, صحيح mx_ورقہ){
  أكد( pCache->pCache!=0 );
  pCache->szCache = mx_ورقہ;
  sqlite3GlobalConfig.pcache2.xCachesize(pCache->pCache,
                                         نمبر_دا_Cache_ورقے(pCache));
}

/*
** Set the suggested cache-spill value.  Make no changes if if the
** argument is zero.  Return the effective cache-spill size, which will
** be the larger of the szSpill and szCache.
*/
صحيح sqlite3_Pcache_سیٹ_Spillsize(PCache *p, صحيح mx_ورقہ){
  صحيح res;
  أكد( p->pCache!=0 );
  إذا( mx_ورقہ ){
    إذا( mx_ورقہ<0 ){
      mx_ورقہ = (صحيح)((-1024*(i64)mx_ورقہ)/(p->sz_ورقہ+p->szExtra));
    }
    p->szSpill = mx_ورقہ;
  }
  res = نمبر_دا_Cache_ورقے(p);
  إذا( res<p->szSpill ) res = p->szSpill; 
  أرجع res;
}

/*
** Free up as much memory as possible from the page cache.
*/
فراغ sqlite3PcacheShrink(PCache *pCache){
  أكد( pCache->pCache!=0 );
  sqlite3GlobalConfig.pcache2.xShrink(pCache->pCache);
}

/*
** Return the size of the header added by this middleware layer
** in the page-cache hierarchy.
*/
صحيح sqlite3_سرخی_ناپ_Pcache(فراغ){ أرجع ROUND8(حجم(PgHdr)); }

/*
** Return the number of dirty pages currently in the cache, as a percentage
** of the configured cache size.
*/
صحيح sqlite3PCachePercentDirty(PCache *pCache){
  PgHdr *pDirty;
  صحيح nDirty = 0;
  صحيح nCache = نمبر_دا_Cache_ورقے(pCache);
  لكل(pDirty=pCache->pDirty; pDirty; pDirty=pDirty->p_Dirty_اگلا) nDirty++;
  أرجع nCache ? (صحيح)(((i64)nDirty * 100) / nCache) : 0;
}

#جے_تعریف_شدہ SQLITE_سدھا_حد_توں_ودھ_READ
/* 
** Return true if there are one or more dirty pages in the cache. Else false.
*/
صحيح sqlite3PCacheIsDirty(PCache *pCache){
  أرجع (pCache->pDirty!=0);
}
#جے_انت

#إذا defined(SQLITE_CHECK_ورقے) || defined(SQLITE_DEBUG)
/*
** For all dirty pages currently in the cache, invoke the specified
** callback. This is only used if the SQLITE_CHECK_PAGES macro is
** defined.
*/
فراغ sqlite3PcacheIterateDirty(PCache *pCache, فراغ (*xIter)(PgHdr *)){
  PgHdr *pDirty;
  لكل(pDirty=pCache->pDirty; pDirty; pDirty=pDirty->p_Dirty_اگلا){
    xIter(pDirty);
  }
}
#جے_انت
