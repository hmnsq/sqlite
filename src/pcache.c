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
#समावेश "sqliteInt.h"

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
काष्ठा PCache {
  PgHdr *pDirty, *p_Dirty_पुच्छ;         /* List of dirty pages in LRU order */
  PgHdr *pSynced;                     /* Last synced page in dirty page list */
  i64 n_Ref_योग;                        /* Sum of ref counts over all pages */
  पूर्णांक szCache;                        /* Configured cache size */
  पूर्णांक szSpill;                        /* Size before spilling occurs */
  पूर्णांक szPage;                         /* Size of every page in this cache */
  पूर्णांक szExtra;                        /* Size of extra space for each page */
  u8 bPurgeable;                      /* True if pages are on backing store */
  u8 eCreate;                         /* eCreate value for for xFetch() */
  पूर्णांक (*xStress)(व्योम*,PgHdr*);       /* Call to try make a page clean */
  व्योम *pStress;                      /* Argument to xStress */
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
#अगर defined(SQLITE_DEBUG) && 0
  पूर्णांक sqlite3PcacheTrace = 2;       /* 0: off  1: simple  2: cache dumps */
  पूर्णांक sqlite3PcacheMxDump = 9999;   /* Max cache entries for pcacheDump() */
# define pcacheTrace(X) अगर(sqlite3PcacheTrace){sqlite3DebugPrintf X;}
  अटल व्योम pcachePageTrace(पूर्णांक i, sqlite3_pcache_page *pLower){
    PgHdr *pPg;
    अचिन्हित अक्षर *a;
    पूर्णांक j;
    अगर( pLower==0 ){
      म_लिखो("%3d: NULL\n", i);
    }अन्यथा{
      pPg = (PgHdr*)pLower->pExtra;
      म_लिखो("%3d: nRef %2lld flgs %02x data ", i, pPg->nRef, pPg->flags);
      a = (अचिन्हित अक्षर *)pLower->pBuf;
      क्रम(j=0; j<12; j++) म_लिखो("%02x", a[j]);
      म_लिखो(" ptr %p\n", pPg);
    }
  }
  अटल व्योम pcacheDump(PCache *pCache){
    पूर्णांक N;
    पूर्णांक i;
    sqlite3_pcache_page *pLower;
  
    अगर( sqlite3PcacheTrace<2 ) वापस;
    अगर( pCache->pCache==0 ) वापस;
    N = sqlite3PcachePagecount(pCache);
    अगर( N>sqlite3PcacheMxDump ) N = sqlite3PcacheMxDump;
    क्रम(i=1; i<=N; i++){
       pLower = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, i, 0);
       pcachePageTrace(i, pLower);
       अगर( pLower && ((PgHdr*)pLower)->pPage==0 ){
         sqlite3GlobalConfig.pcache2.xUnpin(pCache->pCache, pLower, 0);
       }
    }
  }
#अन्यथा
# define pcacheTrace(X)
# define pcachePageTrace(PGNO, X)
# define pcacheDump(X)
#पूर्ण_अगर

/*
** Return 1 if pPg is on the dirty list for pCache.  Return 0 if not.
** This routine runs inside of assert() statements only.
*/
#अगर defined(SQLITE_ENABLE_EXPENSIVE_ASSERT)
अटल पूर्णांक pageOnDirtyList(PCache *pCache, PgHdr *pPg){
  PgHdr *p;
  क्रम(p=pCache->pDirty; p; p=p->p_Dirty_अग्र){
    अगर( p==pPg ) वापस 1;
  }
  वापस 0;
}
अटल पूर्णांक pageNotOnDirtyList(PCache *pCache, PgHdr *pPg){
  PgHdr *p;
  क्रम(p=pCache->pDirty; p; p=p->p_Dirty_अग्र){
    अगर( p==pPg ) वापस 0;
  }
  वापस 1;
}
#अन्यथा
# define pageOnDirtyList(A,B)    1
# define pageNotOnDirtyList(A,B) 1
#पूर्ण_अगर

/*
** Check invariants on a PgHdr entry.  Return true if everything is OK.
** Return false if any invariant is violated.
**
** This routine is for use inside of assert() statements only.  For
** example:
**
**          assert( sqlite3PcachePageSanity(pPg) );
*/
#अगर_घोषित SQLITE_DEBUG
पूर्णांक sqlite3PcachePageSanity(PgHdr *pPg){
  PCache *pCache;
  निश्चित( pPg!=0 );
  निश्चित( pPg->pgno>0 || pPg->pPager==0 );    /* Page number is 1 or more */
  pCache = pPg->pCache;
  निश्चित( pCache!=0 );      /* Every page has an associated PCache */
  अगर( pPg->flags & PGHDR_CLEAN ){
    निश्चित( (pPg->flags & PGHDR_DIRTY)==0 );/* Cannot be both CLEAN and DIRTY */
    निश्चित( pageNotOnDirtyList(pCache, pPg) );/* CLEAN pages not on dirtylist */
  }अन्यथा{
    निश्चित( (pPg->flags & PGHDR_DIRTY)!=0 );/* If not CLEAN must be DIRTY */
    निश्चित( pPg->p_Dirty_अग्र==0 || pPg->p_Dirty_अग्र->p_Dirty_पूर्व==pPg );
    निश्चित( pPg->p_Dirty_पूर्व==0 || pPg->p_Dirty_पूर्व->p_Dirty_अग्र==pPg );
    निश्चित( pPg->p_Dirty_पूर्व!=0 || pCache->pDirty==pPg );
    निश्चित( pageOnDirtyList(pCache, pPg) );
  }
  /* WRITEABLE pages must also be DIRTY */
  अगर( pPg->flags & PGHDR_WRITEABLE ){
    निश्चित( pPg->flags & PGHDR_DIRTY );     /* WRITEABLE implies DIRTY */
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
  वापस 1;
}
#पूर्ण_अगर /* SQLITE_DEBUG */


/********************************** Linked List Management ********************/

/* Allowed values for second argument to pcacheManageDirtyList() */
#घोषणा PCACHE_DIRTYLIST_REMOVE   1    /* Remove pPage from dirty list */
#घोषणा PCACHE_DIRTYLIST_ADD      2    /* Add pPage to the dirty list */
#घोषणा PCACHE_DIRTYLIST_FRONT    3    /* Move pPage to the front of the list */

/*
** Manage pPage's participation on the dirty list.  Bits of the addRemove
** argument determines what operation to do.  The 0x01 bit means first
** remove pPage from the dirty list.  The 0x02 means add pPage back to
** the dirty list.  Doing both moves pPage to the front of the dirty list.
*/
अटल व्योम pcacheManageDirtyList(PgHdr *pPage, u8 addRemove){
  PCache *p = pPage->pCache;

  pcacheTrace(("%p.DIRTYLIST.%s %d\n", p,
                addRemove==1 ? "REMOVE" : addRemove==2 ? "ADD" : "FRONT",
                pPage->pgno));
  अगर( addRemove & PCACHE_DIRTYLIST_REMOVE ){
    निश्चित( pPage->p_Dirty_अग्र || pPage==p->p_Dirty_पुच्छ );
    निश्चित( pPage->p_Dirty_पूर्व || pPage==p->pDirty );
  
    /* Update the PCache1.pSynced variable if necessary. */
    अगर( p->pSynced==pPage ){
      p->pSynced = pPage->p_Dirty_पूर्व;
    }
  
    अगर( pPage->p_Dirty_अग्र ){
      pPage->p_Dirty_अग्र->p_Dirty_पूर्व = pPage->p_Dirty_पूर्व;
    }अन्यथा{
      निश्चित( pPage==p->p_Dirty_पुच्छ );
      p->p_Dirty_पुच्छ = pPage->p_Dirty_पूर्व;
    }
    अगर( pPage->p_Dirty_पूर्व ){
      pPage->p_Dirty_पूर्व->p_Dirty_अग्र = pPage->p_Dirty_अग्र;
    }अन्यथा{
      /* If there are now no dirty pages in the cache, set eCreate to 2. 
      ** This is an optimization that allows sqlite3PcacheFetch() to skip
      ** searching for a dirty page to eject from the cache when it might
      ** otherwise have to.  */
      निश्चित( pPage==p->pDirty );
      p->pDirty = pPage->p_Dirty_अग्र;
      निश्चित( p->bPurgeable || p->eCreate==2 );
      अगर( p->pDirty==0 ){         /*OPTIMIZATION-IF-TRUE*/
        निश्चित( p->bPurgeable==0 || p->eCreate==1 );
        p->eCreate = 2;
      }
    }
  }
  अगर( addRemove & PCACHE_DIRTYLIST_ADD ){
    pPage->p_Dirty_पूर्व = 0;
    pPage->p_Dirty_अग्र = p->pDirty;
    अगर( pPage->p_Dirty_अग्र ){
      निश्चित( pPage->p_Dirty_अग्र->p_Dirty_पूर्व==0 );
      pPage->p_Dirty_अग्र->p_Dirty_पूर्व = pPage;
    }अन्यथा{
      p->p_Dirty_पुच्छ = pPage;
      अगर( p->bPurgeable ){
        निश्चित( p->eCreate==2 );
        p->eCreate = 1;
      }
    }
    p->pDirty = pPage;

    /* If pSynced is NULL and this page has a clear NEED_SYNC flag, set
    ** pSynced to point to it. Checking the NEED_SYNC flag is an 
    ** optimization, as if pSynced points to a page with the NEED_SYNC
    ** flag set sqlite3PcacheFetchStress() searches through all newer 
    ** entries of the dirty-list for a page with NEED_SYNC clear anyway.  */
    अगर( !p->pSynced 
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
अटल व्योम pcacheUnpin(PgHdr *p){
  अगर( p->pCache->bPurgeable ){
    pcacheTrace(("%p.UNPIN %d\n", p->pCache, p->pgno));
    sqlite3GlobalConfig.pcache2.xUnpin(p->pCache->pCache, p->pPage, 0);
    pcacheDump(p->pCache);
  }
}

/*
** Compute the number of pages of cache requested.   p->szCache is the
** cache size requested by the "PRAGMA cache_size" statement.
*/
अटल पूर्णांक संख्या_Of_Cache_Pages(PCache *p){
  अगर( p->szCache>=0 ){
    /* IMPLEMENTATION-OF: R-42059-47211 If the argument N is positive then the
    ** suggested cache size is set to N. */
    वापस p->szCache;
  }अन्यथा{
    i64 n;
    /* IMPLEMENTATION-OF: R-59858-46238 If the argument N is negative, then the
    ** number of cache pages is adjusted to be a number of pages that would
    ** use approximately abs(N*1024) bytes of memory based on the current
    ** page size. */
    n = ((-1024*(i64)p->szCache)/(p->szPage+p->szExtra));
    अगर( n>1000000000 ) n = 1000000000;
    वापस (पूर्णांक)n;
  }
}

/*************************************************** General Interfaces ******
**
** Initialize and shutdown the page cache subsystem. Neither of these 
** functions are threadsafe.
*/
पूर्णांक sqlite3PcacheInitialize(व्योम){
  अगर( sqlite3GlobalConfig.pcache2.xInit==0 ){
    /* IMPLEMENTATION-OF: R-26801-64137 If the xInit() method is NULL, then the
    ** built-in default page cache is used instead of the application defined
    ** page cache. */
    sqlite3PCacheSetDefault();
    निश्चित( sqlite3GlobalConfig.pcache2.xInit!=0 );
  }
  वापस sqlite3GlobalConfig.pcache2.xInit(sqlite3GlobalConfig.pcache2.pArg);
}
व्योम sqlite3PcacheShutdown(व्योम){
  अगर( sqlite3GlobalConfig.pcache2.xShutdown ){
    /* IMPLEMENTATION-OF: R-26000-56589 The xShutdown() method may be NULL. */
    sqlite3GlobalConfig.pcache2.xShutdown(sqlite3GlobalConfig.pcache2.pArg);
  }
}

/*
** Return the size in bytes of a PCache object.
*/
पूर्णांक sqlite3_Pcache_आकार(व्योम){ वापस माप(PCache); }

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
पूर्णांक sqlite3PcacheOpen(
  पूर्णांक szPage,                  /* Size of every page */
  पूर्णांक szExtra,                 /* Extra space associated with each page */
  पूर्णांक bPurgeable,              /* True if pages are on backing store */
  पूर्णांक (*xStress)(व्योम*,PgHdr*),/* Call to try to make pages clean */
  व्योम *pStress,               /* Argument to xStress */
  PCache *p                    /* Preallocated space for the PCache */
){
  स_रखो(p, 0, माप(PCache));
  p->szPage = 1;
  p->szExtra = szExtra;
  निश्चित( szExtra>=8 );  /* First 8 bytes will be zeroed */
  p->bPurgeable = bPurgeable;
  p->eCreate = 2;
  p->xStress = xStress;
  p->pStress = pStress;
  p->szCache = 100;
  p->szSpill = 1;
  pcacheTrace(("%p.OPEN szPage %d bPurgeable %d\n",p,szPage,bPurgeable));
  वापस sqlite3_Pcache_Set_Page_आकार(p, szPage);
}

/*
** Change the page size for PCache object. The caller must ensure that there
** are no outstanding page references when this function is called.
*/
पूर्णांक sqlite3_Pcache_Set_Page_आकार(PCache *pCache, पूर्णांक szPage){
  निश्चित( pCache->n_Ref_योग==0 && pCache->pDirty==0 );
  अगर( pCache->szPage ){
    sqlite3_pcache *pNew;
    pNew = sqlite3GlobalConfig.pcache2.xCreate(
                szPage, pCache->szExtra + ROUND8(माप(PgHdr)),
                pCache->bPurgeable
    );
    अगर( pNew==0 ) वापस SQLITE_NOMEM_BKPT;
    sqlite3GlobalConfig.pcache2.xCachesize(pNew, संख्या_Of_Cache_Pages(pCache));
    अगर( pCache->pCache ){
      sqlite3GlobalConfig.pcache2.xDestroy(pCache->pCache);
    }
    pCache->pCache = pNew;
    pCache->szPage = szPage;
    pcacheTrace(("%p.PAGESIZE %d\n",pCache,szPage));
  }
  वापस SQLITE_OK;
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
  पूर्णांक createFlag        /* If true, create page if it does not exist already */
){
  पूर्णांक eCreate;
  sqlite3_pcache_page *pRes;

  निश्चित( pCache!=0 );
  निश्चित( pCache->pCache!=0 );
  निश्चित( createFlag==3 || createFlag==0 );
  निश्चित( pCache->eCreate==((pCache->bPurgeable && pCache->pDirty) ? 1 : 2) );

  /* eCreate defines what to do if the page does not exist.
  **    0     Do not allocate a new page.  (createFlag==0)
  **    1     Allocate a new page if doing so is inexpensive.
  **          (createFlag==1 AND bPurgeable AND pDirty)
  **    2     Allocate a new page even it doing so is difficult.
  **          (createFlag==1 AND !(bPurgeable AND pDirty)
  */
  eCreate = createFlag & pCache->eCreate;
  निश्चित( eCreate==0 || eCreate==1 || eCreate==2 );
  निश्चित( createFlag==0 || pCache->eCreate==eCreate );
  निश्चित( createFlag==0 || eCreate==1+(!pCache->bPurgeable||!pCache->pDirty) );
  pRes = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, pgno, eCreate);
  pcacheTrace(("%p.FETCH %d%s (result: %p) ",pCache,pgno,
               createFlag?" create":"",pRes));
  pcachePageTrace(pgno, pRes);
  वापस pRes;
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
पूर्णांक sqlite3PcacheFetchStress(
  PCache *pCache,                 /* Obtain the page from this cache */
  Pgno pgno,                      /* Page number to obtain */
  sqlite3_pcache_page **ppPage    /* Write result here */
){
  PgHdr *pPg;
  अगर( pCache->eCreate==2 ) वापस 0;

  अगर( sqlite3PcachePagecount(pCache)>pCache->szSpill ){
    /* Find a dirty page to write-out and recycle. First try to find a 
    ** page that does not require a journal-sync (one with PGHDR_NEED_SYNC
    ** cleared), but if that is not possible settle for any other 
    ** unreferenced dirty page.
    **
    ** If the LRU page in the dirty list that has a clear PGHDR_NEED_SYNC
    ** flag is currently referenced, then the following may leave pSynced
    ** set incorrectly (pointing to other than the LRU page with NEED_SYNC
    ** cleared). This is Ok, as pSynced is just an optimization.  */
    क्रम(pPg=pCache->pSynced; 
        pPg && (pPg->nRef || (pPg->flags&PGHDR_NEED_SYNC)); 
        pPg=pPg->p_Dirty_पूर्व
    );
    pCache->pSynced = pPg;
    अगर( !pPg ){
      क्रम(pPg=pCache->p_Dirty_पुच्छ; pPg && pPg->nRef; pPg=pPg->p_Dirty_पूर्व);
    }
    अगर( pPg ){
      पूर्णांक rc;
#अगर_घोषित SQLITE_LOG_CACHE_SPILL
      sqlite3_log(SQLITE_FULL, 
                  "spill page %d making room for %d - cache used: %d/%d",
                  pPg->pgno, pgno,
                  sqlite3GlobalConfig.pcache2.xPagecount(pCache->pCache),
                संख्या_Of_Cache_Pages(pCache));
#पूर्ण_अगर
      pcacheTrace(("%p.SPILL %d\n",pCache,pPg->pgno));
      rc = pCache->xStress(pCache->pStress, pPg);
      pcacheDump(pCache);
      अगर( rc!=SQLITE_OK && rc!=SQLITE_BUSY ){
        वापस rc;
      }
    }
  }
  *ppPage = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, pgno, 2);
  वापस *ppPage==0 ? SQLITE_NOMEM_BKPT : SQLITE_OK;
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
अटल SQLITE_NOINLINE PgHdr *pcacheFetchFinishWithInit(
  PCache *pCache,             /* Obtain the page from this cache */
  Pgno pgno,                  /* Page number obtained */
  sqlite3_pcache_page *pPage  /* Page obtained by prior PcacheFetch() call */
){
  PgHdr *pPgHdr;
  निश्चित( pPage!=0 );
  pPgHdr = (PgHdr*)pPage->pExtra;
  निश्चित( pPgHdr->pPage==0 );
  स_रखो(&pPgHdr->pDirty, 0, माप(PgHdr) - दुरत्व(PgHdr,pDirty));
  pPgHdr->pPage = pPage;
  pPgHdr->p_तथ्य = pPage->pBuf;
  pPgHdr->pExtra = (व्योम *)&pPgHdr[1];
  स_रखो(pPgHdr->pExtra, 0, 8);
  निश्चित( EIGHT_BYTE_ALIGNMENT( pPgHdr->pExtra ) );
  pPgHdr->pCache = pCache;
  pPgHdr->pgno = pgno;
  pPgHdr->flags = PGHDR_CLEAN;
  वापस sqlite3PcacheFetchFinish(pCache,pgno,pPage);
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

  निश्चित( pPage!=0 );
  pPgHdr = (PgHdr *)pPage->pExtra;

  अगर( !pPgHdr->pPage ){
    वापस pcacheFetchFinishWithInit(pCache, pgno, pPage);
  }
  pCache->n_Ref_योग++;
  pPgHdr->nRef++;
  निश्चित( sqlite3PcachePageSanity(pPgHdr) );
  वापस pPgHdr;
}

/*
** Decrement the reference count on a page. If the page is clean and the
** reference count drops to 0, then it is made eligible for recycling.
*/
व्योम SQLITE_NOINLINE sqlite3PcacheRelease(PgHdr *p){
  निश्चित( p->nRef>0 );
  p->pCache->n_Ref_योग--;
  अगर( (--p->nRef)==0 ){
    अगर( p->flags&PGHDR_CLEAN ){
      pcacheUnpin(p);
    }अन्यथा{
      pcacheManageDirtyList(p, PCACHE_DIRTYLIST_FRONT);
      निश्चित( sqlite3PcachePageSanity(p) );
    }
  }
}

/*
** Increase the reference count of a supplied page by 1.
*/
व्योम sqlite3PcacheRef(PgHdr *p){
  निश्चित(p->nRef>0);
  निश्चित( sqlite3PcachePageSanity(p) );
  p->nRef++;
  p->pCache->n_Ref_योग++;
}

/*
** Drop a page from the cache. There must be exactly one reference to the
** page. This function deletes that reference, so after it returns the
** page pointed to by p is invalid.
*/
व्योम sqlite3PcacheDrop(PgHdr *p){
  निश्चित( p->nRef==1 );
  निश्चित( sqlite3PcachePageSanity(p) );
  अगर( p->flags&PGHDR_DIRTY ){
    pcacheManageDirtyList(p, PCACHE_DIRTYLIST_REMOVE);
  }
  p->pCache->n_Ref_योग--;
  sqlite3GlobalConfig.pcache2.xUnpin(p->pCache->pCache, p->pPage, 1);
}

/*
** Make sure the page is marked as dirty. If it isn't dirty already,
** make it so.
*/
व्योम sqlite3PcacheMakeDirty(PgHdr *p){
  निश्चित( p->nRef>0 );
  निश्चित( sqlite3PcachePageSanity(p) );
  अगर( p->flags & (PGHDR_CLEAN|PGHDR_DONT_WRITE) ){    /*OPTIMIZATION-IF-FALSE*/
    p->flags &= ~PGHDR_DONT_WRITE;
    अगर( p->flags & PGHDR_CLEAN ){
      p->flags ^= (PGHDR_DIRTY|PGHDR_CLEAN);
      pcacheTrace(("%p.DIRTY %d\n",p->pCache,p->pgno));
      निश्चित( (p->flags & (PGHDR_DIRTY|PGHDR_CLEAN))==PGHDR_DIRTY );
      pcacheManageDirtyList(p, PCACHE_DIRTYLIST_ADD);
      निश्चित( sqlite3PcachePageSanity(p) );
    }
    निश्चित( sqlite3PcachePageSanity(p) );
  }
}

/*
** Make sure the page is marked as clean. If it isn't clean already,
** make it so.
*/
व्योम sqlite3PcacheMakeClean(PgHdr *p){
  निश्चित( sqlite3PcachePageSanity(p) );
  निश्चित( (p->flags & PGHDR_DIRTY)!=0 );
  निश्चित( (p->flags & PGHDR_CLEAN)==0 );
  pcacheManageDirtyList(p, PCACHE_DIRTYLIST_REMOVE);
  p->flags &= ~(PGHDR_DIRTY|PGHDR_NEED_SYNC|PGHDR_WRITEABLE);
  p->flags |= PGHDR_CLEAN;
  pcacheTrace(("%p.CLEAN %d\n",p->pCache,p->pgno));
  निश्चित( sqlite3PcachePageSanity(p) );
  अगर( p->nRef==0 ){
    pcacheUnpin(p);
  }
}

/*
** Make every page in the cache clean.
*/
व्योम sqlite3PcacheCleanAll(PCache *pCache){
  PgHdr *p;
  pcacheTrace(("%p.CLEAN-ALL\n",pCache));
  जबतक( (p = pCache->pDirty)!=0 ){
    sqlite3PcacheMakeClean(p);
  }
}

/*
** Clear the PGHDR_NEED_SYNC and PGHDR_WRITEABLE flag from all dirty pages.
*/
व्योम sqlite3PcacheClearWritable(PCache *pCache){
  PgHdr *p;
  pcacheTrace(("%p.CLEAR-WRITEABLE\n",pCache));
  क्रम(p=pCache->pDirty; p; p=p->p_Dirty_अग्र){
    p->flags &= ~(PGHDR_NEED_SYNC|PGHDR_WRITEABLE);
  }
  pCache->pSynced = pCache->p_Dirty_पुच्छ;
}

/*
** Clear the PGHDR_NEED_SYNC flag from all dirty pages.
*/
व्योम sqlite3PcacheClearSyncFlags(PCache *pCache){
  PgHdr *p;
  क्रम(p=pCache->pDirty; p; p=p->p_Dirty_अग्र){
    p->flags &= ~PGHDR_NEED_SYNC;
  }
  pCache->pSynced = pCache->p_Dirty_पुच्छ;
}

/*
** Change the page number of page p to newPgno. 
*/
व्योम sqlite3PcacheMove(PgHdr *p, Pgno newPgno){
  PCache *pCache = p->pCache;
  sqlite3_pcache_page *pOther;
  निश्चित( p->nRef>0 );
  निश्चित( newPgno>0 );
  निश्चित( sqlite3PcachePageSanity(p) );
  pcacheTrace(("%p.MOVE %d -> %d\n",pCache,p->pgno,newPgno));
  pOther = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache, newPgno, 0);
  अगर( pOther ){
    PgHdr *pXPage = (PgHdr*)pOther->pExtra;
    निश्चित( pXPage->nRef==0 );
    pXPage->nRef++;
    pCache->n_Ref_योग++;
    sqlite3PcacheDrop(pXPage);
  }
  sqlite3GlobalConfig.pcache2.xRekey(pCache->pCache, p->pPage, p->pgno,newPgno);
  p->pgno = newPgno;
  अगर( (p->flags&PGHDR_DIRTY) && (p->flags&PGHDR_NEED_SYNC) ){
    pcacheManageDirtyList(p, PCACHE_DIRTYLIST_FRONT);
    निश्चित( sqlite3PcachePageSanity(p) );
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
व्योम sqlite3PcacheTruncate(PCache *pCache, Pgno pgno){
  अगर( pCache->pCache ){
    PgHdr *p;
    PgHdr *p_अग्र;
    pcacheTrace(("%p.TRUNCATE %d\n",pCache,pgno));
    क्रम(p=pCache->pDirty; p; p=p_अग्र){
      p_अग्र = p->p_Dirty_अग्र;
      /* This routine never gets call with a positive pgno except right
      ** after sqlite3PcacheCleanAll().  So if there are dirty pages,
      ** it must be that pgno==0.
      */
      निश्चित( p->pgno>0 );
      अगर( p->pgno>pgno ){
        निश्चित( p->flags&PGHDR_DIRTY );
        sqlite3PcacheMakeClean(p);
      }
    }
    अगर( pgno==0 && pCache->n_Ref_योग ){
      sqlite3_pcache_page *pPage1;
      pPage1 = sqlite3GlobalConfig.pcache2.xFetch(pCache->pCache,1,0);
      अगर( ALWAYS(pPage1) ){  /* Page 1 is always available in cache, because
                             ** pCache->nRefSum>0 */
        स_रखो(pPage1->pBuf, 0, pCache->szPage);
        pgno = 1;
      }
    }
    sqlite3GlobalConfig.pcache2.xTruncate(pCache->pCache, pgno+1);
  }
}

/*
** Close a cache.
*/
व्योम sqlite3PcacheClose(PCache *pCache){
  निश्चित( pCache->pCache!=0 );
  pcacheTrace(("%p.CLOSE\n",pCache));
  sqlite3GlobalConfig.pcache2.xDestroy(pCache->pCache);
}

/* 
** Discard the contents of the cache.
*/
व्योम sqlite3PcacheClear(PCache *pCache){
  sqlite3PcacheTruncate(pCache, 0);
}

/*
** Merge two lists of pages connected by pDirty and in pgno order.
** Do not bother fixing the pDirtyPrev pointers.
*/
अटल PgHdr *pcacheMergeDirtyList(PgHdr *pA, PgHdr *pB){
  PgHdr परिणाम, *p_पुच्छ;
  p_पुच्छ = &परिणाम;
  निश्चित( pA!=0 && pB!=0 );
  क्रम(;;){
    अगर( pA->pgno<pB->pgno ){
      p_पुच्छ->pDirty = pA;
      p_पुच्छ = pA;
      pA = pA->pDirty;
      अगर( pA==0 ){
        p_पुच्छ->pDirty = pB;
        अवरोध;
      }
    }अन्यथा{
      p_पुच्छ->pDirty = pB;
      p_पुच्छ = pB;
      pB = pB->pDirty;
      अगर( pB==0 ){
        p_पुच्छ->pDirty = pA;
        अवरोध;
      }
    }
  }
  वापस परिणाम.pDirty;
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
#घोषणा N_SORT_BUCKET  32
अटल PgHdr *pcacheSortDirtyList(PgHdr *pIn){
  PgHdr *a[N_SORT_BUCKET], *p;
  पूर्णांक i;
  स_रखो(a, 0, माप(a));
  जबतक( pIn ){
    p = pIn;
    pIn = p->pDirty;
    p->pDirty = 0;
    क्रम(i=0; ALWAYS(i<N_SORT_BUCKET-1); i++){
      अगर( a[i]==0 ){
        a[i] = p;
        अवरोध;
      }अन्यथा{
        p = pcacheMergeDirtyList(a[i], p);
        a[i] = 0;
      }
    }
    अगर( NEVER(i==N_SORT_BUCKET-1) ){
      /* To get here, there need to be 2^(N_SORT_BUCKET) elements in
      ** the input list.  But that is impossible.
      */
      a[i] = pcacheMergeDirtyList(a[i], p);
    }
  }
  p = a[0];
  क्रम(i=1; i<N_SORT_BUCKET; i++){
    अगर( a[i]==0 ) जारी;
    p = p ? pcacheMergeDirtyList(p, a[i]) : a[i];
  }
  वापस p;
}

/*
** Return a list of all dirty pages in the cache, sorted by page number.
*/
PgHdr *sqlite3PcacheDirtyList(PCache *pCache){
  PgHdr *p;
  क्रम(p=pCache->pDirty; p; p=p->p_Dirty_अग्र){
    p->pDirty = p->p_Dirty_अग्र;
  }
  वापस pcacheSortDirtyList(pCache->pDirty);
}

/* 
** Return the total number of references to all pages held by the cache.
**
** This is not the total number of pages referenced, but the sum of the
** reference count for all pages.
*/
i64 sqlite3_Pcache_Ref_गिनती(PCache *pCache){
  वापस pCache->n_Ref_योग;
}

/*
** Return the number of references to the page supplied as an argument.
*/
i64 sqlite3PcachePageRefcount(PgHdr *p){
  वापस p->nRef;
}

/* 
** Return the total number of pages in the cache.
*/
पूर्णांक sqlite3PcachePagecount(PCache *pCache){
  निश्चित( pCache->pCache!=0 );
  वापस sqlite3GlobalConfig.pcache2.xPagecount(pCache->pCache);
}

#अगर_घोषित SQLITE_TEST
/*
** Get the suggested cache-size value.
*/
पूर्णांक sqlite3PcacheGetCachesize(PCache *pCache){
  वापस संख्या_Of_Cache_Pages(pCache);
}
#पूर्ण_अगर

/*
** Set the suggested cache-size value.
*/
व्योम sqlite3PcacheSetCachesize(PCache *pCache, पूर्णांक mxPage){
  निश्चित( pCache->pCache!=0 );
  pCache->szCache = mxPage;
  sqlite3GlobalConfig.pcache2.xCachesize(pCache->pCache,
                                         संख्या_Of_Cache_Pages(pCache));
}

/*
** Set the suggested cache-spill value.  Make no changes if if the
** argument is zero.  Return the effective cache-spill size, which will
** be the larger of the szSpill and szCache.
*/
पूर्णांक sqlite3PcacheSetSpillsize(PCache *p, पूर्णांक mxPage){
  पूर्णांक res;
  निश्चित( p->pCache!=0 );
  अगर( mxPage ){
    अगर( mxPage<0 ){
      mxPage = (पूर्णांक)((-1024*(i64)mxPage)/(p->szPage+p->szExtra));
    }
    p->szSpill = mxPage;
  }
  res = संख्या_Of_Cache_Pages(p);
  अगर( res<p->szSpill ) res = p->szSpill; 
  वापस res;
}

/*
** Free up as much memory as possible from the page cache.
*/
व्योम sqlite3PcacheShrink(PCache *pCache){
  निश्चित( pCache->pCache!=0 );
  sqlite3GlobalConfig.pcache2.xShrink(pCache->pCache);
}

/*
** Return the size of the header added by this middleware layer
** in the page-cache hierarchy.
*/
पूर्णांक sqlite3_Header_आकार_Pcache(व्योम){ वापस ROUND8(माप(PgHdr)); }

/*
** Return the number of dirty pages currently in the cache, as a percentage
** of the configured cache size.
*/
पूर्णांक sqlite3PCachePercentDirty(PCache *pCache){
  PgHdr *pDirty;
  पूर्णांक nDirty = 0;
  पूर्णांक nCache = संख्या_Of_Cache_Pages(pCache);
  क्रम(pDirty=pCache->pDirty; pDirty; pDirty=pDirty->p_Dirty_अग्र) nDirty++;
  वापस nCache ? (पूर्णांक)(((i64)nDirty * 100) / nCache) : 0;
}

#अगर_घोषित SQLITE_DIRECT_OVERFLOW_READ
/* 
** Return true if there are one or more dirty pages in the cache. Else false.
*/
पूर्णांक sqlite3PCacheIsDirty(PCache *pCache){
  वापस (pCache->pDirty!=0);
}
#पूर्ण_अगर

#अगर defined(SQLITE_CHECK_PAGES) || defined(SQLITE_DEBUG)
/*
** For all dirty pages currently in the cache, invoke the specified
** callback. This is only used if the SQLITE_CHECK_PAGES macro is
** defined.
*/
व्योम sqlite3PcacheIterateDirty(PCache *pCache, व्योम (*xIter)(PgHdr *)){
  PgHdr *pDirty;
  क्रम(pDirty=pCache->pDirty; pDirty; pDirty=pDirty->p_Dirty_अग्र){
    xIter(pDirty);
  }
}
#पूर्ण_अगर
