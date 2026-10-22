/*
** 2008 November 05
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file implements the default page cache implementation (the
** sqlite3_pcache interface). It also contains part of the implementation
** of the SQLITE_CONFIG_PAGECACHE and sqlite3_release_memory() features.
** If the default page cache implementation is overridden, then neither of
** these two features are available.
**
** A Page cache line looks like this:
**
**  -------------------------------------------------------------
**  |  database page content   |  PgHdr1  |  MemPage  |  PgHdr  |
**  -------------------------------------------------------------
**
** The database page content is up front (so that buffer overreads tend to
** flow harmlessly into the PgHdr1, MemPage, and PgHdr extensions).   MemPage
** is the extension added by the btree.c module containing information such
** as the database page number and how that database page is used.  PgHdr
** is added by the pcache.c layer and contains information used to keep track
** of which pages are "dirty".  PgHdr1 is an extension added by this
** module (pcache1.c).  The PgHdr1 header is a subclass of sqlite3_pcache_page.
** PgHdr1 contains information needed to look up a page by its page number.
** The superclass sqlite3_pcache_page.pBuf points to the start of the
** database page content and sqlite3_pcache_page.pExtra points to PgHdr.
**
** The size of the extension (MemPage+PgHdr+PgHdr1) can be determined at
** runtime using sqlite3_config(SQLITE_CONFIG_PCACHE_HDRSZ, &size).  The
** sizes of the extensions sum to 272 bytes on x64 for 3.8.10, but this
** size can vary according to architecture, compile-time options, and
** SQLite library version number.
**
** Historical note:  It used to be that if the SQLITE_PCACHE_SEPARATE_HEADER
** was defined, then the page content would be held in a separate memory
** allocation from the PgHdr1.  This was intended to avoid clownshoe memory
** allocations.  However, the btree layer needs a small (16-byte) overrun
** area after the page content buffer.  The header serves as that overrun
** area.  Therefore SQLITE_PCACHE_SEPARATE_HEADER was discontinued to avoid
** any possibility of a memory error.
**
** This module tracks pointers to PgHdr1 objects.  Only pcache.c communicates
** with this module.  Information is passed back and forth as PgHdr1 pointers.
**
** The pcache.c and pager.c modules deal pointers to PgHdr objects.
** The btree.c module deals with pointers to MemPage objects.
**
** SOURCE OF PAGE CACHE MEMORY:
**
** Memory for a page might come from any of three sources:
**
**    (1)  The general-purpose memory allocator - sqlite3Malloc()
**    (2)  Global page-cache memory provided using sqlite3_config() with
**         SQLITE_CONFIG_PAGECACHE.
**    (3)  PCache-local bulk allocation.
**
** The third case is a chunk of heap memory (defaulting to 100 pages worth)
** that is allocated when the page cache is created.  The size of the local
** bulk allocation can be adjusted using
**
**     sqlite3_config(SQLITE_CONFIG_PAGECACHE, (void*)0, 0, N).
**
** If N is positive, then N pages worth of memory are allocated using a single
** sqlite3Malloc() call and that memory is used for the first N pages allocated.
** Or if N is negative, then -1024*N bytes of memory are allocated and used
** for as many pages as can be accommodated.
**
** Only one of (2) or (3) can be used.  Once the memory available to (2) or
** (3) is exhausted, subsequent allocations fail over to the general-purpose
** memory allocator (1).
**
** Earlier versions of SQLite used only methods (1) and (2).  But experiments
** show that method (3) with N==100 provides about a 5% performance boost for
** common workloads.
*/
#అన్తర్భావ "sqliteInt.h"

రకనిర్వచనం నిర్మాణం PCache1 PCache1;
రకనిర్వచనం నిర్మాణం PgHdr1 PgHdr1;
రకనిర్వచనం నిర్మాణం PgFreeslot PgFreeslot;
రకనిర్వచనం నిర్మాణం PGroup PGroup;

/*
** Each cache entry is represented by an instance of the following
** structure. A buffer of PgHdr1.pCache->szPage bytes is allocated
** directly before this structure and is used to cache the page content.
**
** When reading a corrupt database file, it is possible that SQLite might
** read a few bytes (no more than 16 bytes) past the end of the page buffer.
** It will only read past the end of the page buffer, never write.  This
** object is positioned immediately after the page buffer to serve as an
** overrun area, so that overreads are harmless.
**
** Variables isBulkLocal and isAnchor were once type "u8". That works,
** but causes a 2-byte gap in the structure for most architectures (since
** pointers must be either 4 or 8-byte aligned). As this structure is located
** in memory directly after the associated page data, if the database is
** corrupt, code at the b-tree layer may overread the page buffer and
** read part of this structure before the corruption is detected. This
** can cause a valgrind error if the uninitialized gap is accessed. Using u16
** ensures there is no such gap, and therefore no bytes of uninitialized
** memory in the structure.
**
** The pLruNext and pLruPrev pointers form a double-linked circular list
** of all pages that are unpinned.  The PGroup.lru element (which should be
** the only element on the list with PgHdr1.isAnchor set to 1) forms the
** beginning and the end of the list.
*/
నిర్మాణం PgHdr1 {
  sqlite3_pcache_పుట పుట; /* Base class. Must be first. pBuf & pExtra */
  గుర్తులేని పూర్ణాంకం i_కీలకం;        /* Key value (page number) */
  u16 isBulkLocal;          /* This page from bulk local storage */
  u16 isAnchor;             /* This is the PGroup.lru element */
  PgHdr1 *p_తదుపరి;            /* Next in hash table chain */
  PCache1 *pCache;          /* Cache that currently owns this page */
  PgHdr1 *p_Lru_తదుపరి;         /* Next in circular LRU list of unpinned pages */
  PgHdr1 *pLruPrev;         /* Previous in LRU list of unpinned pages */
                            /* NB: pLruPrev is only valid if pLruNext!=0 */
};

/*
** A page is pinned if it is not on the LRU list.  To be "pinned" means
** that the page is in active use and must not be deallocated.
*/
#నిర్వచించు పుట_IS_PINNED(p)    ((p)->p_Lru_తదుపరి==0)
#నిర్వచించు పుట_IS_UNPINNED(p)  ((p)->p_Lru_తదుపరి!=0)

/* Each page cache (or PCache) belongs to a PGroup.  A PGroup is a set
** of one or more PCaches that are able to recycle each other's unpinned
** pages when they are under memory pressure.  A PGroup is an instance of
** the following object.
**
** This page cache implementation works in one of two modes:
**
**   (1)  Every PCache is the sole member of its own PGroup.  There is
**        one PGroup per PCache.
**
**   (2)  There is a single global PGroup that all PCaches are a member
**        of.
**
** Mode 1 uses more memory (since PCache instances are not able to rob
** unused pages from other PCaches) but it also operates without a mutex,
** and is therefore often faster.  Mode 2 requires a mutex in order to be
** threadsafe, but recycles pages more efficiently.
**
** For mode (1), PGroup.mutex is NULL.  For mode (2) there is only a single
** PGroup which is the pcache1.grp global variable and its mutex is
** SQLITE_MUTEX_STATIC_LRU.
*/
నిర్మాణం PGroup {
  sqlite3_mutex *mutex;          /* MUTEX_STATIC_LRU or NULL */
  గుర్తులేని పూర్ణాంకం n_Max_పుట;         /* Sum of nMax for purgeable caches */
  గుర్తులేని పూర్ణాంకం n_Min_పుట;         /* Sum of nMin for purgeable caches */
  గుర్తులేని పూర్ణాంకం mxPinned;         /* nMaxpage + 10 - nMinPage */
  గుర్తులేని పూర్ణాంకం nPurgeable;       /* Number of purgeable pages allocated */
  PgHdr1 lru;                    /* The beginning and end of the LRU list */
};

/* Each page cache is an instance of the following object.  Every
** open database file (including each in-memory database and each
** temporary or transient database) has a single page cache which
** is an instance of this object.
**
** Pointers to structures of this type are cast and returned as
** opaque sqlite3_pcache* handles.
*/
నిర్మాణం PCache1 {
  /* Cache configuration parameters. Page size (szPage) and the purgeable
  ** flag (bPurgeable) and the pnPurgeable pointer are all set when the
  ** cache is created and are never changed thereafter. nMax may be
  ** modified at any time by a call to the pcache1Cachesize() method.
  ** The PGroup mutex must be held when accessing nMax.
  */
  PGroup *p_సమూహం;                     /* PGroup this cache belongs to */
  గుర్తులేని పూర్ణాంకం *pnPurgeable;          /* Pointer to pGroup->nPurgeable */
  పూర్ణాంకం sz_పుట;                         /* Size of database content section */
  పూర్ణాంకం szExtra;                        /* sizeof(MemPage)+sizeof(PgHdr) */
  పూర్ణాంకం szAlloc;                        /* Total size of one pcache line */
  పూర్ణాంకం bPurgeable;                     /* True if cache is purgeable */
  గుర్తులేని పూర్ణాంకం nMin;                  /* Minimum number of pages reserved */
  గుర్తులేని పూర్ణాంకం nMax;                  /* Configured "cache_size" value */
  గుర్తులేని పూర్ణాంకం n90pct;                /* nMax*9/10 */
  గుర్తులేని పూర్ణాంకం i_Max_కీలకం;               /* Largest key seen since xTruncate() */
  గుర్తులేని పూర్ణాంకం nPurgeableDummy;       /* pnPurgeable points here when not used*/

  /* Hash table of all pages. The following variables may only be accessed
  ** when the accessor is holding the PGroup mutex.
  */
  గుర్తులేని పూర్ణాంకం nRecyclable;           /* Number of pages in the LRU list */
  గుర్తులేని పూర్ణాంకం n_పుట;                 /* Total number of pages in apHash */
  గుర్తులేని పూర్ణాంకం nHash;                 /* Number of slots in apHash[] */
  PgHdr1 **apHash;                    /* Hash table for fast lookup by key */
  PgHdr1 *pFree;                      /* List of unused pcache-local pages */
  శూన్య *pBulk;                        /* Bulk memory used by pcache-local */
};

/*
** Free slots in the allocator used to divide up the global page cache
** buffer provided using the SQLITE_CONFIG_PAGECACHE mechanism.
*/
నిర్మాణం PgFreeslot {
  PgFreeslot *p_తదుపరి;  /* Next free slot */
};

/*
** Global data used by this cache.
*/
స్థిర SQLITE_WSD నిర్మాణం PCacheGlobal {
  PGroup grp;                    /* The global PGroup for mode (2) */

  /* Variables related to SQLITE_CONFIG_PAGECACHE settings.  The
  ** szSlot, nSlot, pStart, pEnd, nReserve, and isInit values are all
  ** fixed at sqlite3_initialize() time and do not require mutex protection.
  ** The nFreeSlot and pFree values do require mutex protection.
  */
  పూర్ణాంకం isInit;                    /* True if initialized */
  పూర్ణాంకం separateCache;             /* Use a new PGroup for each PCache */
  పూర్ణాంకం n_Init_పుట;                 /* Initial bulk allocation size */
  పూర్ణాంకం sz_స్థానం;                    /* Size of each free slot */
  పూర్ణాంకం n_స్థానం;                     /* The number of pcache slots */
  పూర్ణాంకం nReserve;                  /* Try to keep nFreeSlot above this */
  శూన్య *p_ప్రారంభం, *p_చివర;           /* Bounds of global page cache memory */
  /* Above requires no mutex.  Use mutex below for variable that follow. */
  sqlite3_mutex *mutex;          /* Mutex for accessing the following: */
  PgFreeslot *pFree;             /* Free page blocks */
  పూర్ణాంకం n_Free_స్థానం;                 /* Number of unused pcache slots */
  పూర్ణాంకం bUnderPressure;            /* True if low on PAGECACHE memory */
} pcache1_g;

/*
** All code in this file should access the global structure above via the
** alias "pcache1". This ensures that the WSD emulation is used when
** compiling for systems that do not support real WSD.
*/
#నిర్వచించు pcache1 (GLOBAL(నిర్మాణం PCacheGlobal, pcache1_g))

/*
** Macros to enter and leave the PCache LRU mutex.
*/
#ఒకవేళ !defined(SQLITE_ENABLE_జ్ఞాపకం_MANAGEMENT) || SQLITE_THREADSAFE==0
# define pcache1EnterMutex(X)  ప్రతిజ్ఞా((X)->mutex==0)
# define pcache1LeaveMutex(X)  ప్రతిజ్ఞా((X)->mutex==0)
# define PCACHE1_MIGHT_USE_సమూహం_MUTEX 0
#లేకపోతే
# define pcache1EnterMutex(X) sqlite3_mutex_enter((X)->mutex)
# define pcache1LeaveMutex(X) sqlite3_mutex_leave((X)->mutex)
# define PCACHE1_MIGHT_USE_సమూహం_MUTEX 1
#అయితే_ముగింపు

/******************************************************************************/
/******** Page Allocation/SQLITE_CONFIG_PCACHE Related Functions **************/


/*
** This function is called during initialization if a static buffer is
** supplied to use for the page-cache by passing the SQLITE_CONFIG_PAGECACHE
** verb to sqlite3_config(). Parameter pBuf points to an allocation large
** enough to contain 'n' buffers of 'sz' bytes each.
**
** This routine is called from sqlite3_initialize() and so it is guaranteed
** to be serialized already.  There is no need for further mutexing.
*/
శూన్య sqlite3_PCache_బఫరు_అమరిక(శూన్య *pBuf, పూర్ణాంకం sz, పూర్ణాంకం n){
  ఒకవేళ( pcache1.isInit ){
    PgFreeslot *p;
    ఒకవేళ( pBuf==0 ) sz = n = 0;
    ఒకవేళ( n==0 ) sz = 0;
    sz = ROUNDDOWN8(sz);
    pcache1.sz_స్థానం = sz;
    pcache1.n_స్థానం = pcache1.n_Free_స్థానం = n;
    pcache1.nReserve = n>90 ? 10 : (n/10 + 1);
    pcache1.p_ప్రారంభం = pBuf;
    pcache1.pFree = 0;
    AtomicStore(&pcache1.bUnderPressure,0);
    అయితే( n-- ){
      p = (PgFreeslot*)pBuf;
      p->p_తదుపరి = pcache1.pFree;
      pcache1.pFree = p;
      pBuf = (శూన్య*)&((అక్షరం*)pBuf)[sz];
    }
    pcache1.p_చివర = pBuf;
  }
}

/*
** Try to initialize the pCache->pFree and pCache->pBulk fields.  Return
** true if pCache->pFree ends up containing one or more free pages.
*/
స్థిర పూర్ణాంకం pcache1InitBulk(PCache1 *pCache){
  i64 szBulk;
  అక్షరం *zBulk;
  ఒకవేళ( pcache1.n_Init_పుట==0 ) తిరిగి 0;
  /* Do not bother with a bulk allocation if the cache size very small */
  ఒకవేళ( pCache->nMax<3 ) తిరిగి 0;
  sqlite3BeginBenignMalloc();
  ఒకవేళ( pcache1.n_Init_పుట>0 ){
    szBulk = pCache->szAlloc * (i64)pcache1.n_Init_పుట;
  }లేకపోతే{
    szBulk = -1024 * (i64)pcache1.n_Init_పుట;
  }
  ఒకవేళ( szBulk > pCache->szAlloc*(i64)pCache->nMax ){
    szBulk = pCache->szAlloc*(i64)pCache->nMax;
  }
  ఒకవేళ( szBulk>=pCache->szAlloc ){
    zBulk = pCache->pBulk = sqlite3Malloc( szBulk );
    sqlite3_చివర_Benign_Malloc();
    ఒకవేళ( zBulk ){
      పూర్ణాంకం nBulk = sqlite3_Malloc_కొలత(zBulk)/pCache->szAlloc;
      చేయి{
        PgHdr1 *pX = (PgHdr1*)&zBulk[pCache->sz_పుట];
        pX->పుట.pBuf = zBulk;
        pX->పుట.pExtra = (u8*)pX + ROUND8(పరిమాణం(*pX));
        ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT( pX->పుట.pExtra ) );
        pX->isBulkLocal = 1;
        pX->isAnchor = 0;
        pX->p_తదుపరి = pCache->pFree;
        pX->pLruPrev = 0;    /* Initializing this saves a valgrind error */
        pCache->pFree = pX;
        zBulk += pCache->szAlloc;
      }అయితే( --nBulk );
    }
  }
  తిరిగి pCache->pFree!=0;
}

/*
** Malloc function used within this file to allocate space from the buffer
** configured using sqlite3_config(SQLITE_CONFIG_PAGECACHE) option. If no
** such buffer exists or there is no space left in it, this function falls
** back to sqlite3Malloc().
**
** Multiple threads can run this routine at the same time.  Global variables
** in pcache1 need to be protected via mutex.
*/
స్థిర శూన్య *pcache1Alloc(పూర్ణాంకం nByte){
  శూన్య *p = 0;
  ప్రతిజ్ఞా( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  ఒకవేళ( nByte<=pcache1.sz_స్థానం ){
    sqlite3_mutex_enter(pcache1.mutex);
    p = (PgHdr1 *)pcache1.pFree;
    ఒకవేళ( p ){
      pcache1.pFree = pcache1.pFree->p_తదుపరి;
      pcache1.n_Free_స్థానం--;
      AtomicStore(&pcache1.bUnderPressure,pcache1.n_Free_స్థానం<pcache1.nReserve);
      ప్రతిజ్ఞా( pcache1.n_Free_స్థానం>=0 );
      sqlite3_స్థితి_Highwater(SQLITE_స్థితి_PAGECACHE_కొలత, nByte);
      sqlite3_స్థితి_పైకి(SQLITE_స్థితి_PAGECACHE_వాడిన, 1);
    }
    sqlite3_mutex_leave(pcache1.mutex);
  }
  ఒకవేళ( p==0 ){
    /* Memory is not available in the SQLITE_CONFIG_PAGECACHE pool.  Get
    ** it from sqlite3Malloc instead.
    */
    p = sqlite3Malloc(nByte);
#నిర్వచితం_కాకపోతే SQLITE_DISABLE_PAGECACHE_అధికప్రవాహం_STATS
    ఒకవేళ( p ){
      పూర్ణాంకం sz = sqlite3_Malloc_కొలత(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3_స్థితి_Highwater(SQLITE_స్థితి_PAGECACHE_కొలత, nByte);
      sqlite3_స్థితి_పైకి(SQLITE_స్థితి_PAGECACHE_అధికప్రవాహం, sz);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#అయితే_ముగింపు
    sqlite3_Memdebug_సమితి_Type(p, MEMTYPE_PCACHE);
  }
  తిరిగి p;
}

/*
** Free an allocated buffer obtained from pcache1Alloc().
*/
స్థిర శూన్య pcache1Free(శూన్య *p){
  ఒకవేళ( p==0 ) తిరిగి;
  ఒకవేళ( SQLITE_WITHIN(p, pcache1.p_ప్రారంభం, pcache1.p_చివర) ){
    PgFreeslot *p_స్థానం;
    sqlite3_mutex_enter(pcache1.mutex);
    sqlite3_స్థితి_కిందికి(SQLITE_స్థితి_PAGECACHE_వాడిన, 1);
    p_స్థానం = (PgFreeslot*)p;
    p_స్థానం->p_తదుపరి = pcache1.pFree;
    pcache1.pFree = p_స్థానం;
    pcache1.n_Free_స్థానం++;
    AtomicStore(&pcache1.bUnderPressure,pcache1.n_Free_స్థానం<pcache1.nReserve);
    ప్రతిజ్ఞా( pcache1.n_Free_స్థానం<=pcache1.n_స్థానం );
    sqlite3_mutex_leave(pcache1.mutex);
  }లేకపోతే{
    ప్రతిజ్ఞా( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3_Memdebug_సమితి_Type(p, MEMTYPE_కుప్ప);
#నిర్వచితం_కాకపోతే SQLITE_DISABLE_PAGECACHE_అధికప్రవాహం_STATS
    {
      పూర్ణాంకం nFreed = 0;
      nFreed = sqlite3_Malloc_కొలత(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3_స్థితి_కిందికి(SQLITE_స్థితి_PAGECACHE_అధికప్రవాహం, nFreed);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#అయితే_ముగింపు
    sqlite3_free(p);
  }
}

#నిర్వచితమైతే SQLITE_ENABLE_జ్ఞాపకం_MANAGEMENT
/*
** Return the size of a pcache allocation
*/
స్థిర పూర్ణాంకం pcache1_Mem_కొలత(శూన్య *p){
  ఒకవేళ( p>=pcache1.p_ప్రారంభం && p<pcache1.p_చివర ){
    తిరిగి pcache1.sz_స్థానం;
  }లేకపోతే{
    పూర్ణాంకం i_కొలత;
    ప్రతిజ్ఞా( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3_Memdebug_సమితి_Type(p, MEMTYPE_కుప్ప);
    i_కొలత = sqlite3_Malloc_కొలత(p);
    sqlite3_Memdebug_సమితి_Type(p, MEMTYPE_PCACHE);
    తిరిగి i_కొలత;
  }
}
#అయితే_ముగింపు /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

/*
** Allocate a new page object initially associated with cache pCache.
*/
స్థిర PgHdr1 *pcache1_Alloc_పుట(PCache1 *pCache, పూర్ణాంకం benignMalloc){
  PgHdr1 *p = 0;
  శూన్య *pPg;

  ప్రతిజ్ఞా( sqlite3_mutex_held(pCache->p_సమూహం->mutex) );
  ఒకవేళ( pCache->pFree || (pCache->n_పుట==0 && pcache1InitBulk(pCache)) ){
    ప్రతిజ్ఞా( pCache->pFree!=0 );
    p = pCache->pFree;
    pCache->pFree = p->p_తదుపరి;
    p->p_తదుపరి = 0;
  }లేకపోతే{
#నిర్వచితమైతే SQLITE_ENABLE_జ్ఞాపకం_MANAGEMENT
    /* The group mutex must be released before pcache1Alloc() is called. This
    ** is because it might call sqlite3_release_memory(), which assumes that
    ** this mutex is not held. */
    ప్రతిజ్ఞా( pcache1.separateCache==0 );
    ప్రతిజ్ఞా( pCache->p_సమూహం==&pcache1.grp );
    pcache1LeaveMutex(pCache->p_సమూహం);
#అయితే_ముగింపు
    ఒకవేళ( benignMalloc ){ sqlite3BeginBenignMalloc(); }
    pPg = pcache1Alloc(pCache->szAlloc);
    ఒకవేళ( benignMalloc ){ sqlite3_చివర_Benign_Malloc(); }
#నిర్వచితమైతే SQLITE_ENABLE_జ్ఞాపకం_MANAGEMENT
    pcache1EnterMutex(pCache->p_సమూహం);
#అయితే_ముగింపు
    ఒకవేళ( pPg==0 ) తిరిగి 0;
    p = (PgHdr1 *)&((u8 *)pPg)[pCache->sz_పుట];
    p->పుట.pBuf = pPg;
    p->పుట.pExtra = (u8*)p + ROUND8(పరిమాణం(*p));
    ప్రతిజ్ఞా( EIGHT_BYTE_ALIGNMENT( p->పుట.pExtra ) );
    p->isBulkLocal = 0;
    p->isAnchor = 0;
    p->pLruPrev = 0;           /* Initializing this saves a valgrind error */
  }
  (*pCache->pnPurgeable)++;
  తిరిగి p;
}

/*
** Free a page object allocated by pcache1AllocPage().
*/
స్థిర శూన్య pcache1_Free_పుట(PgHdr1 *p){
  PCache1 *pCache;
  ప్రతిజ్ఞా( p!=0 );
  pCache = p->pCache;
  ప్రతిజ్ఞా( sqlite3_mutex_held(p->pCache->p_సమూహం->mutex) );
  ఒకవేళ( p->isBulkLocal ){
    p->p_తదుపరి = pCache->pFree;
    pCache->pFree = p;
  }లేకపోతే{
    pcache1Free(p->పుట.pBuf);
  }
  (*pCache->pnPurgeable)--;
}

/*
** Malloc function used by SQLite to obtain space from the buffer configured
** using sqlite3_config(SQLITE_CONFIG_PAGECACHE) option. If no such buffer
** exists, this function falls back to sqlite3Malloc().
*/
శూన్య *sqlite3_పుట_Malloc(పూర్ణాంకం sz){
  ప్రతిజ్ఞా( sz<=65536+8 ); /* These allocations are never very large */
  తిరిగి pcache1Alloc(sz);
}

/*
** Free an allocated buffer obtained from sqlite3PageMalloc().
*/
శూన్య sqlite3_పుట_Free(శూన్య *p){
  pcache1Free(p);
}


/*
** Return true if it desirable to avoid allocating a new page cache
** entry.
**
** If memory was allocated specifically to the page cache using
** SQLITE_CONFIG_PAGECACHE but that memory has all been used, then
** it is desirable to avoid allocating a new page cache entry because
** presumably SQLITE_CONFIG_PAGECACHE was suppose to be sufficient
** for all page cache needs and we should not need to spill the
** allocation onto the heap.
**
** Or, the heap is used for all page cache memory but the heap is
** under memory pressure, then again it is desirable to avoid
** allocating a new page cache entry in order to avoid stressing
** the heap even further.
*/
స్థిర పూర్ణాంకం pcache1_Under_జ్ఞాపకం_Pressure(PCache1 *pCache){
  ఒకవేళ( pcache1.n_స్థానం && (pCache->sz_పుట+pCache->szExtra)<=pcache1.sz_స్థానం ){
    తిరిగి Atomic_ఎక్కించు(&pcache1.bUnderPressure);
  }లేకపోతే{
    తిరిగి sqlite3_కుప్ప_Nearly_నిండిన();
  }
}

/******************************************************************************/
/******** General Implementation Functions ************************************/

/*
** This function is used to resize the hash table used by the cache passed
** as the first argument.
**
** The PCache mutex must be held when this function is called.
*/
స్థిర శూన్య pcache1ResizeHash(PCache1 *p){
  PgHdr1 **apNew;
  u64 nNew;
  u32 i;

  ప్రతిజ్ఞా( sqlite3_mutex_held(p->p_సమూహం->mutex) );

  nNew = 2*(u64)p->nHash;
  ఒకవేళ( nNew<256 ){
    nNew = 256;
  }
  ప్రతిజ్ఞా( (nNew & (nNew-1))==0 );   /* nNew is always a power of two */

  pcache1LeaveMutex(p->p_సమూహం);
  ఒకవేళ( p->nHash ){ sqlite3BeginBenignMalloc(); }
  apNew = (PgHdr1 **)sqlite3MallocZero(పరిమాణం(PgHdr1 *)*nNew);
  ఒకవేళ( p->nHash ){ sqlite3_చివర_Benign_Malloc(); }
  pcache1EnterMutex(p->p_సమూహం);
  ఒకవేళ( apNew ){
    కొరకు(i=0; i<p->nHash; i++){
      PgHdr1 *p_పుట;
      PgHdr1 *p_తదుపరి = p->apHash[i];
      అయితే( (p_పుట = p_తదుపరి)!=0 ){
        గుర్తులేని పూర్ణాంకం h = (గుర్తులేని పూర్ణాంకం)(p_పుట->i_కీలకం & (nNew-1));
        p_తదుపరి = p_పుట->p_తదుపరి;
        p_పుట->p_తదుపరి = apNew[h];
        apNew[h] = p_పుట;
      }
    }
    sqlite3_free(p->apHash);
    p->apHash = apNew;
    p->nHash = nNew;
  }
}

/*
** This function is used internally to remove the page pPage from the
** PGroup LRU list, if is part of it. If pPage is not part of the PGroup
** LRU list, then this function is a no-op.
**
** The PGroup mutex must be held when this function is called.
*/
స్థిర PgHdr1 *pcache1_Pin_పుట(PgHdr1 *p_పుట){
  ప్రతిజ్ఞా( p_పుట!=0 );
  ప్రతిజ్ఞా( పుట_IS_UNPINNED(p_పుట) );
  ప్రతిజ్ఞా( p_పుట->p_Lru_తదుపరి );
  ప్రతిజ్ఞా( p_పుట->pLruPrev );
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_పుట->pCache->p_సమూహం->mutex) );
  p_పుట->pLruPrev->p_Lru_తదుపరి = p_పుట->p_Lru_తదుపరి;
  p_పుట->p_Lru_తదుపరి->pLruPrev = p_పుట->pLruPrev;
  p_పుట->p_Lru_తదుపరి = 0;
  /* pPage->pLruPrev = 0;
  ** No need to clear pLruPrev as it is never accessed if pLruNext is 0 */
  ప్రతిజ్ఞా( p_పుట->isAnchor==0 );
  ప్రతిజ్ఞా( p_పుట->pCache->p_సమూహం->lru.isAnchor==1 );
  p_పుట->pCache->nRecyclable--;
  తిరిగి p_పుట;
}


/*
** Remove the page supplied as an argument from the hash table
** (PCache1.apHash structure) that it is currently stored in.
** Also free the page if freePage is true.
**
** The PGroup mutex must be held when this function is called.
*/
స్థిర శూన్య pcache1RemoveFromHash(PgHdr1 *p_పుట, పూర్ణాంకం freeFlag){
  గుర్తులేని పూర్ణాంకం h;
  PCache1 *pCache = p_పుట->pCache;
  PgHdr1 **pp;

  ప్రతిజ్ఞా( sqlite3_mutex_held(pCache->p_సమూహం->mutex) );
  ప్రతిజ్ఞా( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  h = p_పుట->i_కీలకం & (pCache->nHash-1);
  కొరకు(pp=&pCache->apHash[h]; (*pp)!=p_పుట; pp=&(*pp)->p_తదుపరి);
  *pp = (*pp)->p_తదుపరి;

  pCache->n_పుట--;
  ఒకవేళ( freeFlag ) pcache1_Free_పుట(p_పుట);
}

/*
** If there are currently more than nMaxPage pages allocated, try
** to recycle pages to reduce the number allocated to nMaxPage.
*/
స్థిర శూన్య pcache1_Enforce_Max_పుట(PCache1 *pCache){
  PGroup *p_సమూహం = pCache->p_సమూహం;
  PgHdr1 *p;
  ప్రతిజ్ఞా( sqlite3_mutex_held(p_సమూహం->mutex) );
  అయితే( p_సమూహం->nPurgeable>p_సమూహం->n_Max_పుట
      && (p=p_సమూహం->lru.pLruPrev)->isAnchor==0
  ){
    ప్రతిజ్ఞా( p->pCache->p_సమూహం==p_సమూహం );
    ప్రతిజ్ఞా( పుట_IS_UNPINNED(p) );
    pcache1_Pin_పుట(p);
    pcache1RemoveFromHash(p, 1);
  }
  ఒకవేళ( pCache->n_పుట==0 && pCache->pBulk ){
    sqlite3_free(pCache->pBulk);
    pCache->pBulk = pCache->pFree = 0;
  }
}

/*
** Discard all pages from cache pCache with a page number (key value)
** greater than or equal to iLimit. Any pinned pages that meet this
** criteria are unpinned before they are discarded.
**
** The PCache mutex must be held when this function is called.
*/
స్థిర శూన్య pcache1TruncateUnsafe(
  PCache1 *pCache,             /* The cache to truncate */
  గుర్తులేని పూర్ణాంకం i_హద్దు          /* Drop pages with this pgno or larger */
){
  TESTONLY( పూర్ణాంకం n_పుట = 0; )  /* To assert pCache->nPage is correct */
  గుర్తులేని పూర్ణాంకం h, iStop;
  ప్రతిజ్ఞా( sqlite3_mutex_held(pCache->p_సమూహం->mutex) );
  ప్రతిజ్ఞా( pCache->i_Max_కీలకం >= i_హద్దు );
  ప్రతిజ్ఞా( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  ఒకవేళ( pCache->i_Max_కీలకం - i_హద్దు < pCache->nHash ){
    /* If we are just shaving the last few pages off the end of the
    ** cache, then there is no point in scanning the entire hash table.
    ** Only scan those hash slots that might contain pages that need to
    ** be removed. */
    h = i_హద్దు & (pCache->nHash-1);
    iStop = pCache->i_Max_కీలకం & (pCache->nHash-1);
    TESTONLY( n_పుట = -10; )  /* Disable the pCache->nPage validity check */
  }లేకపోతే{
    /* This is the general case where many pages are being removed.
    ** It is necessary to scan the entire hash table */
    h = pCache->nHash/2;
    iStop = h - 1;
  }
  కొరకు(;;){
    PgHdr1 **pp;
    PgHdr1 *p_పుట;
    ప్రతిజ్ఞా( h<pCache->nHash );
    pp = &pCache->apHash[h];
    అయితే( (p_పుట = *pp)!=0 ){
      ఒకవేళ( p_పుట->i_కీలకం>=i_హద్దు ){
        pCache->n_పుట--;
        *pp = p_పుట->p_తదుపరి;
        ఒకవేళ( పుట_IS_UNPINNED(p_పుట) ) pcache1_Pin_పుట(p_పుట);
        pcache1_Free_పుట(p_పుట);
      }లేకపోతే{
        pp = &p_పుట->p_తదుపరి;
        TESTONLY( ఒకవేళ( n_పుట>=0 ) n_పుట++; )
      }
    }
    ఒకవేళ( h==iStop ) ఆపు;
    h = (h+1) & (pCache->nHash-1);
  }
  ప్రతిజ్ఞా( n_పుట<0 || pCache->n_పుట==(గుర్తులేని)n_పుట );
}

/******************************************************************************/
/******** sqlite3_pcache Methods **********************************************/

/*
** Implementation of the sqlite3_pcache.xInit method.
*/
స్థిర పూర్ణాంకం pcache1Init(శూన్య *Not_వాడిన){
  వాడని_PARAMETER(Not_వాడిన);
  ప్రతిజ్ఞా( pcache1.isInit==0 );
  memset(&pcache1, 0, పరిమాణం(pcache1));


  /*
  ** The pcache1.separateCache variable is true if each PCache has its own
  ** private PGroup (mode-1).  pcache1.separateCache is false if the single
  ** PGroup in pcache1.grp is used for all page caches (mode-2).
  **
  **   *  Always use a unified cache (mode-2) if ENABLE_MEMORY_MANAGEMENT
  **
  **   *  Use a unified cache in single-threaded applications that have
  **      configured a start-time buffer for use as page-cache memory using
  **      sqlite3_config(SQLITE_CONFIG_PAGECACHE, pBuf, sz, N) with non-NULL
  **      pBuf argument.
  **
  **   *  Otherwise use separate caches (mode-1)
  */
#ఒకవేళ defined(SQLITE_ENABLE_జ్ఞాపకం_MANAGEMENT)
  pcache1.separateCache = 0;
#లేదావేళ SQLITE_THREADSAFE
  pcache1.separateCache = sqlite3GlobalConfig.p_పుట==0
                          || sqlite3GlobalConfig.bCoreMutex>0;
#లేకపోతే
  pcache1.separateCache = sqlite3GlobalConfig.p_పుట==0;
#అయితే_ముగింపు

#ఒకవేళ SQLITE_THREADSAFE
  ఒకవేళ( sqlite3GlobalConfig.bCoreMutex ){
    pcache1.grp.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_LRU);
    pcache1.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PMEM);
  }
#అయితే_ముగింపు
  ఒకవేళ( pcache1.separateCache
   && sqlite3GlobalConfig.n_పుట!=0
   && sqlite3GlobalConfig.p_పుట==0
  ){
    pcache1.n_Init_పుట = sqlite3GlobalConfig.n_పుట;
  }లేకపోతే{
    pcache1.n_Init_పుట = 0;
  }
  pcache1.grp.mxPinned = 10;
  pcache1.isInit = 1;
  తిరిగి SQLITE_OK;
}

/*
** Implementation of the sqlite3_pcache.xShutdown method.
** Note that the static mutex allocated in xInit does
** not need to be freed.
*/
స్థిర శూన్య pcache1Shutdown(శూన్య *Not_వాడిన){
  వాడని_PARAMETER(Not_వాడిన);
  ప్రతిజ్ఞా( pcache1.isInit!=0 );
  memset(&pcache1, 0, పరిమాణం(pcache1));
}

/* forward declaration */
స్థిర శూన్య pcache1Destroy(sqlite3_pcache *p);

/*
** Implementation of the sqlite3_pcache.xCreate method.
**
** Allocate a new cache.
*/
స్థిర sqlite3_pcache *pcache1Create(పూర్ణాంకం sz_పుట, పూర్ణాంకం szExtra, పూర్ణాంకం bPurgeable){
  PCache1 *pCache;      /* The newly created page cache */
  PGroup *p_సమూహం;       /* The group the new page cache will belong to */
  i64 sz;               /* Bytes of memory required to allocate the new cache */

  ప్రతిజ్ఞా( (sz_పుట & (sz_పుట-1))==0 && sz_పుట>=512 && sz_పుట<=65536 );
  ప్రతిజ్ఞా( szExtra < 300 );

  sz = పరిమాణం(PCache1) + పరిమాణం(PGroup)*pcache1.separateCache;
  pCache = (PCache1 *)sqlite3MallocZero(sz);
  ఒకవేళ( pCache ){
    ఒకవేళ( pcache1.separateCache ){
      p_సమూహం = (PGroup*)&pCache[1];
      p_సమూహం->mxPinned = 10;
    }లేకపోతే{
      p_సమూహం = &pcache1.grp;
    }
    pcache1EnterMutex(p_సమూహం);
    ఒకవేళ( p_సమూహం->lru.isAnchor==0 ){
      p_సమూహం->lru.isAnchor = 1;
      p_సమూహం->lru.pLruPrev = p_సమూహం->lru.p_Lru_తదుపరి = &p_సమూహం->lru;
    }
    pCache->p_సమూహం = p_సమూహం;
    pCache->sz_పుట = sz_పుట;
    pCache->szExtra = szExtra;
    pCache->szAlloc = sz_పుట + szExtra + ROUND8(పరిమాణం(PgHdr1));
    pCache->bPurgeable = (bPurgeable ? 1 : 0);
    pcache1ResizeHash(pCache);
    ఒకవేళ( bPurgeable ){
      pCache->nMin = 10;
      p_సమూహం->n_Min_పుట += pCache->nMin;
      p_సమూహం->mxPinned = p_సమూహం->n_Max_పుట + 10 - p_సమూహం->n_Min_పుట;
      pCache->pnPurgeable = &p_సమూహం->nPurgeable;
    }లేకపోతే{
      pCache->pnPurgeable = &pCache->nPurgeableDummy;
    }
    pcache1LeaveMutex(p_సమూహం);
    ఒకవేళ( pCache->nHash==0 ){
      pcache1Destroy((sqlite3_pcache*)pCache);
      pCache = 0;
    }
  }
  తిరిగి (sqlite3_pcache *)pCache;
}

/*
** Implementation of the sqlite3_pcache.xCachesize method.
**
** Configure the cache_size limit for a cache.
*/
స్థిర శూన్య pcache1Cachesize(sqlite3_pcache *p, పూర్ణాంకం nMax){
  PCache1 *pCache = (PCache1 *)p;
  u32 n;
  ప్రతిజ్ఞా( nMax>=0 );
  ఒకవేళ( pCache->bPurgeable ){
    PGroup *p_సమూహం = pCache->p_సమూహం;
    pcache1EnterMutex(p_సమూహం);
    n = (u32)nMax;
    ఒకవేళ( n > 0x7fff0000 - p_సమూహం->n_Max_పుట + pCache->nMax ){
      n = 0x7fff0000 - p_సమూహం->n_Max_పుట + pCache->nMax;
    }
    p_సమూహం->n_Max_పుట += (n - pCache->nMax);
    p_సమూహం->mxPinned = p_సమూహం->n_Max_పుట + 10 - p_సమూహం->n_Min_పుట;
    pCache->nMax = n;
    pCache->n90pct = pCache->nMax*9/10;
    pcache1_Enforce_Max_పుట(pCache);
    pcache1LeaveMutex(p_సమూహం);
  }
}

/*
** Implementation of the sqlite3_pcache.xShrink method.
**
** Free up as much memory as possible.
*/
స్థిర శూన్య pcache1Shrink(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1*)p;
  ఒకవేళ( pCache->bPurgeable ){
    PGroup *p_సమూహం = pCache->p_సమూహం;
    గుర్తులేని పూర్ణాంకం saved_Max_పుట;
    pcache1EnterMutex(p_సమూహం);
    saved_Max_పుట = p_సమూహం->n_Max_పుట;
    p_సమూహం->n_Max_పుట = 0;
    pcache1_Enforce_Max_పుట(pCache);
    p_సమూహం->n_Max_పుట = saved_Max_పుట;
    pcache1LeaveMutex(p_సమూహం);
  }
}

/*
** Implementation of the sqlite3_pcache.xPagecount method.
*/
స్థిర పూర్ణాంకం pcache1Pagecount(sqlite3_pcache *p){
  పూర్ణాంకం n;
  PCache1 *pCache = (PCache1*)p;
  pcache1EnterMutex(pCache->p_సమూహం);
  n = pCache->n_పుట;
  pcache1LeaveMutex(pCache->p_సమూహం);
  తిరిగి n;
}


/*
** Implement steps 3, 4, and 5 of the pcache1Fetch() algorithm described
** in the header of the pcache1Fetch() procedure.
**
** This steps are broken out into a separate procedure because they are
** usually not needed, and by avoiding the stack initialization required
** for these steps, the main pcache1Fetch() procedure can run faster.
*/
స్థిర SQLITE_NOINLINE PgHdr1 *pcache1FetchStage2(
  PCache1 *pCache,
  గుర్తులేని పూర్ణాంకం i_కీలకం,
  పూర్ణాంకం createFlag
){
  గుర్తులేని పూర్ణాంకం nPinned;
  PGroup *p_సమూహం = pCache->p_సమూహం;
  PgHdr1 *p_పుట = 0;

  /* Step 3: Abort if createFlag is 1 but the cache is nearly full */
  ప్రతిజ్ఞా( pCache->n_పుట >= pCache->nRecyclable );
  nPinned = pCache->n_పుట - pCache->nRecyclable;
  ప్రతిజ్ఞా( p_సమూహం->mxPinned == p_సమూహం->n_Max_పుట + 10 - p_సమూహం->n_Min_పుట );
  ప్రతిజ్ఞా( pCache->n90pct == pCache->nMax*9/10 );
  ఒకవేళ( createFlag==1 && (
        nPinned>=p_సమూహం->mxPinned
     || nPinned>=pCache->n90pct
     || (pcache1_Under_జ్ఞాపకం_Pressure(pCache) && pCache->nRecyclable<nPinned)
  )){
    తిరిగి 0;
  }

  ఒకవేళ( pCache->n_పుట>=pCache->nHash ) pcache1ResizeHash(pCache);
  ప్రతిజ్ఞా( pCache->nHash>0 && pCache->apHash );
  ప్రతిజ్ఞా( (pCache->nHash & (pCache->nHash-1))==0 );

  /* Step 4. Try to recycle a page. */
  ఒకవేళ( pCache->bPurgeable
   && !p_సమూహం->lru.pLruPrev->isAnchor
   && ((pCache->n_పుట+1>=pCache->nMax) || pcache1_Under_జ్ఞాపకం_Pressure(pCache))
  ){
    PCache1 *pOther;
    p_పుట = p_సమూహం->lru.pLruPrev;
    ప్రతిజ్ఞా( పుట_IS_UNPINNED(p_పుట) );
    pcache1RemoveFromHash(p_పుట, 0);
    pcache1_Pin_పుట(p_పుట);
    pOther = p_పుట->pCache;
    ఒకవేళ( pOther->szAlloc != pCache->szAlloc ){
      pcache1_Free_పుట(p_పుట);
      p_పుట = 0;
    }లేకపోతే{
      p_సమూహం->nPurgeable -= (pOther->bPurgeable - pCache->bPurgeable);
    }
  }

  /* Step 5. If a usable page buffer has still not been found,
  ** attempt to allocate a new one.
  */
  ఒకవేళ( !p_పుట ){
    p_పుట = pcache1_Alloc_పుట(pCache, createFlag==1);
  }

  ఒకవేళ( p_పుట ){
    గుర్తులేని పూర్ణాంకం h = i_కీలకం & (pCache->nHash-1);
    pCache->n_పుట++;
    p_పుట->i_కీలకం = i_కీలకం;
    p_పుట->p_తదుపరి = pCache->apHash[h];
    p_పుట->pCache = pCache;
    p_పుట->p_Lru_తదుపరి = 0;
    /* pPage->pLruPrev = 0;
    ** No need to clear pLruPrev since it is not accessed when pLruNext==0 */
    *(శూన్య **)p_పుట->పుట.pExtra = 0;
    pCache->apHash[h] = p_పుట;
    ఒకవేళ( i_కీలకం>pCache->i_Max_కీలకం ){
      pCache->i_Max_కీలకం = i_కీలకం;
    }
  }
  తిరిగి p_పుట;
}

/*
** Implementation of the sqlite3_pcache.xFetch method.
**
** Fetch a page by key value.
**
** Whether or not a new page may be allocated by this function depends on
** the value of the createFlag argument.  0 means do not allocate a new
** page.  1 means allocate a new page if space is easily available.  2
** means to try really hard to allocate a new page.
**
** For a non-purgeable cache (a cache used as the storage for an in-memory
** database) there is really no difference between createFlag 1 and 2.  So
** the calling function (pcache.c) will never have a createFlag of 1 on
** a non-purgeable cache.
**
** There are three different approaches to obtaining space for a page,
** depending on the value of parameter createFlag (which may be 0, 1 or 2).
**
**   1. Regardless of the value of createFlag, the cache is searched for a
**      copy of the requested page. If one is found, it is returned.
**
**   2. If createFlag==0 and the page is not already in the cache, NULL is
**      returned.
**
**   3. If createFlag is 1, and the page is not already in the cache, then
**      return NULL (do not allocate a new page) if any of the following
**      conditions are true:
**
**       (a) the number of pages pinned by the cache is greater than
**           PCache1.nMax, or
**
**       (b) the number of pages pinned by the cache is greater than
**           the sum of nMax for all purgeable caches, less the sum of
**           nMin for all other purgeable caches, or
**
**   4. If none of the first three conditions apply and the cache is marked
**      as purgeable, and if one of the following is true:
**
**       (a) The number of pages allocated for the cache is already
**           PCache1.nMax, or
**
**       (b) The number of pages allocated for all purgeable caches is
**           already equal to or greater than the sum of nMax for all
**           purgeable caches,
**
**       (c) The system is under memory pressure and wants to avoid
**           unnecessary pages cache entry allocations
**
**      then attempt to recycle a page from the LRU list. If it is the right
**      size, return the recycled buffer. Otherwise, free the buffer and
**      proceed to step 5.
**
**   5. Otherwise, allocate and return a new page buffer.
**
** There are two versions of this routine.  pcache1FetchWithMutex() is
** the general case.  pcache1FetchNoMutex() is a faster implementation for
** the common case where pGroup->mutex is NULL.  The pcache1Fetch() wrapper
** invokes the appropriate routine.
*/
స్థిర PgHdr1 *pcache1FetchNoMutex(
  sqlite3_pcache *p,
  గుర్తులేని పూర్ణాంకం i_కీలకం,
  పూర్ణాంకం createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_పుట = 0;

  /* Step 1: Search the hash table for an existing entry.  nHash is always
  ** a power of two when the cache is in use (see pcache1ResizeHash()), so
  ** the modulo reduces to a mask, avoiding a hardware divide. */
  ప్రతిజ్ఞా( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  p_పుట = pCache->apHash[i_కీలకం & (pCache->nHash-1u)];
  అయితే( p_పుట && p_పుట->i_కీలకం!=i_కీలకం ){ p_పుట = p_పుట->p_తదుపరి; }

  /* Step 2: If the page was found in the hash table, then return it.
  ** If the page was not in the hash table and createFlag is 0, abort.
  ** Otherwise (page not in hash and createFlag!=0) continue with
  ** subsequent steps to try to create the page. */
  ఒకవేళ( p_పుట ){
    ఒకవేళ( పుట_IS_UNPINNED(p_పుట) ){
      తిరిగి pcache1_Pin_పుట(p_పుట);
    }లేకపోతే{
      తిరిగి p_పుట;
    }
  }లేకపోతే ఒకవేళ( createFlag ){
    /* Steps 3, 4, and 5 implemented by this subroutine */
    తిరిగి pcache1FetchStage2(pCache, i_కీలకం, createFlag);
  }లేకపోతే{
    తిరిగి 0;
  }
}
#ఒకవేళ PCACHE1_MIGHT_USE_సమూహం_MUTEX
స్థిర PgHdr1 *pcache1FetchWithMutex(
  sqlite3_pcache *p,
  గుర్తులేని పూర్ణాంకం i_కీలకం,
  పూర్ణాంకం createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_పుట;

  pcache1EnterMutex(pCache->p_సమూహం);
  p_పుట = pcache1FetchNoMutex(p, i_కీలకం, createFlag);
  ప్రతిజ్ఞా( p_పుట==0 || pCache->i_Max_కీలకం>=i_కీలకం );
  pcache1LeaveMutex(pCache->p_సమూహం);
  తిరిగి p_పుట;
}
#అయితే_ముగింపు
స్థిర sqlite3_pcache_పుట *pcache1Fetch(
  sqlite3_pcache *p,
  గుర్తులేని పూర్ణాంకం i_కీలకం,
  పూర్ణాంకం createFlag
){
#ఒకవేళ PCACHE1_MIGHT_USE_సమూహం_MUTEX || defined(SQLITE_DEBUG)
  PCache1 *pCache = (PCache1 *)p;
#అయితే_ముగింపు

  ప్రతిజ్ఞా( offsetof(PgHdr1,పుట)==0 );
  ప్రతిజ్ఞా( pCache->bPurgeable || createFlag!=1 );
  ప్రతిజ్ఞా( pCache->bPurgeable || pCache->nMin==0 );
  ప్రతిజ్ఞా( pCache->bPurgeable==0 || pCache->nMin==10 );
  ప్రతిజ్ఞా( pCache->nMin==0 || pCache->bPurgeable );
  ప్రతిజ్ఞా( pCache->nHash>0 );
#ఒకవేళ PCACHE1_MIGHT_USE_సమూహం_MUTEX
  ఒకవేళ( pCache->p_సమూహం->mutex ){
    తిరిగి (sqlite3_pcache_పుట*)pcache1FetchWithMutex(p, i_కీలకం, createFlag);
  }లేకపోతే
#అయితే_ముగింపు
  {
    తిరిగి (sqlite3_pcache_పుట*)pcache1FetchNoMutex(p, i_కీలకం, createFlag);
  }
}


/*
** Implementation of the sqlite3_pcache.xUnpin method.
**
** Mark a page as unpinned (eligible for asynchronous recycling).
*/
స్థిర శూన్య pcache1Unpin(
  sqlite3_pcache *p,
  sqlite3_pcache_పుట *pPg,
  పూర్ణాంకం reuseUnlikely
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_పుట = (PgHdr1 *)pPg;
  PGroup *p_సమూహం = pCache->p_సమూహం;

  ప్రతిజ్ఞా( p_పుట->pCache==pCache );
  pcache1EnterMutex(p_సమూహం);

  /* It is an error to call this function if the page is already
  ** part of the PGroup LRU list.
  */
  ప్రతిజ్ఞా( p_పుట->p_Lru_తదుపరి==0 );
  ప్రతిజ్ఞా( పుట_IS_PINNED(p_పుట) );

  ఒకవేళ( reuseUnlikely || p_సమూహం->nPurgeable>p_సమూహం->n_Max_పుట ){
    pcache1RemoveFromHash(p_పుట, 1);
  }లేకపోతే{
    /* Add the page to the PGroup LRU list. */
    PgHdr1 **pp_మొదటి = &p_సమూహం->lru.p_Lru_తదుపరి;
    p_పుట->pLruPrev = &p_సమూహం->lru;
    (p_పుట->p_Lru_తదుపరి = *pp_మొదటి)->pLruPrev = p_పుట;
    *pp_మొదటి = p_పుట;
    pCache->nRecyclable++;
  }

  pcache1LeaveMutex(pCache->p_సమూహం);
}

/*
** Implementation of the sqlite3_pcache.xRekey method.
*/
స్థిర శూన్య pcache1Rekey(
  sqlite3_pcache *p,
  sqlite3_pcache_పుట *pPg,
  గుర్తులేని పూర్ణాంకం iOld,
  గుర్తులేని పూర్ణాంకం iNew
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_పుట = (PgHdr1 *)pPg;
  PgHdr1 **pp;
  గుర్తులేని పూర్ణాంకం hOld, hNew;
  ప్రతిజ్ఞా( p_పుట->i_కీలకం==iOld );
  ప్రతిజ్ఞా( p_పుట->pCache==pCache );
  ప్రతిజ్ఞా( iOld!=iNew );               /* The page number really is changing */

  pcache1EnterMutex(pCache->p_సమూహం);

  ప్రతిజ్ఞా( pcache1FetchNoMutex(p, iOld, 0)==p_పుట ); /* pPg really is iOld */
  ప్రతిజ్ఞా( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  hOld = iOld & (pCache->nHash-1);
  pp = &pCache->apHash[hOld];
  అయితే( (*pp)!=p_పుట ){
    pp = &(*pp)->p_తదుపరి;
  }
  *pp = p_పుట->p_తదుపరి;

  ప్రతిజ్ఞా( pcache1FetchNoMutex(p, iNew, 0)==0 ); /* iNew not in cache */
  hNew = iNew & (pCache->nHash-1);
  p_పుట->i_కీలకం = iNew;
  p_పుట->p_తదుపరి = pCache->apHash[hNew];
  pCache->apHash[hNew] = p_పుట;
  ఒకవేళ( iNew>pCache->i_Max_కీలకం ){
    pCache->i_Max_కీలకం = iNew;
  }

  pcache1LeaveMutex(pCache->p_సమూహం);
}

/*
** Implementation of the sqlite3_pcache.xTruncate method.
**
** Discard all unpinned pages in the cache with a page number equal to
** or greater than parameter iLimit. Any pinned pages with a page number
** equal to or greater than iLimit are implicitly unpinned.
*/
స్థిర శూన్య pcache1Truncate(sqlite3_pcache *p, గుర్తులేని పూర్ణాంకం i_హద్దు){
  PCache1 *pCache = (PCache1 *)p;
  pcache1EnterMutex(pCache->p_సమూహం);
  ఒకవేళ( i_హద్దు<=pCache->i_Max_కీలకం ){
    pcache1TruncateUnsafe(pCache, i_హద్దు);
    pCache->i_Max_కీలకం = i_హద్దు-1;
  }
  pcache1LeaveMutex(pCache->p_సమూహం);
}

/*
** Implementation of the sqlite3_pcache.xDestroy method.
**
** Destroy a cache allocated using pcache1Create().
*/
స్థిర శూన్య pcache1Destroy(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1 *)p;
  PGroup *p_సమూహం = pCache->p_సమూహం;
  ప్రతిజ్ఞా( pCache->bPurgeable || (pCache->nMax==0 && pCache->nMin==0) );
  pcache1EnterMutex(p_సమూహం);
  ఒకవేళ( pCache->n_పుట ) pcache1TruncateUnsafe(pCache, 0);
  ప్రతిజ్ఞా( p_సమూహం->n_Max_పుట >= pCache->nMax );
  p_సమూహం->n_Max_పుట -= pCache->nMax;
  ప్రతిజ్ఞా( p_సమూహం->n_Min_పుట >= pCache->nMin );
  p_సమూహం->n_Min_పుట -= pCache->nMin;
  p_సమూహం->mxPinned = p_సమూహం->n_Max_పుట + 10 - p_సమూహం->n_Min_పుట;
  pcache1_Enforce_Max_పుట(pCache);
  pcache1LeaveMutex(p_సమూహం);
  sqlite3_free(pCache->pBulk);
  sqlite3_free(pCache->apHash);
  sqlite3_free(pCache);
}

/*
** This function is called during initialization (sqlite3_initialize()) to
** install the default pluggable cache module, assuming the user has not
** already provided an alternative.
*/
శూన్య sqlite3_PCache_సమితి_Default(శూన్య){
  స్థిర స్థిరరాశి sqlite3_pcache_methods2 defaultMethods = {
    1,                       /* iVersion */
    0,                       /* pArg */
    pcache1Init,             /* xInit */
    pcache1Shutdown,         /* xShutdown */
    pcache1Create,           /* xCreate */
    pcache1Cachesize,        /* xCachesize */
    pcache1Pagecount,        /* xPagecount */
    pcache1Fetch,            /* xFetch */
    pcache1Unpin,            /* xUnpin */
    pcache1Rekey,            /* xRekey */
    pcache1Truncate,         /* xTruncate */
    pcache1Destroy,          /* xDestroy */
    pcache1Shrink            /* xShrink */
  };
  sqlite3_config(SQLITE_CONFIG_PCACHE2, &defaultMethods);
}

/*
** Return the size of the header on each page of this PCACHE implementation.
*/
పూర్ణాంకం sqlite3_శీర్షిక_కొలత_Pcache1(శూన్య){ తిరిగి ROUND8(పరిమాణం(PgHdr1)); }

/*
** Return the global mutex used by this PCACHE implementation.  The
** sqlite3_status() routine needs access to this mutex.
*/
sqlite3_mutex *sqlite3Pcache1Mutex(శూన్య){
  తిరిగి pcache1.mutex;
}

#నిర్వచితమైతే SQLITE_ENABLE_జ్ఞాపకం_MANAGEMENT
/*
** This function is called to free superfluous dynamically allocated memory
** held by the pager system. Memory in use by any SQLite pager allocated
** by the current thread may be sqlite3_free()ed.
**
** nReq is the number of bytes of memory required. Once this much has
** been released, the function returns. The return value is the total number
** of bytes of memory released.
*/
పూర్ణాంకం sqlite3_Pcache_Release_జ్ఞాపకం(పూర్ణాంకం nReq){
  పూర్ణాంకం nFree = 0;
  ప్రతిజ్ఞా( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  ప్రతిజ్ఞా( sqlite3_mutex_notheld(pcache1.mutex) );
  ఒకవేళ( sqlite3GlobalConfig.p_పుట==0 ){
    PgHdr1 *p;
    pcache1EnterMutex(&pcache1.grp);
    అయితే( (nReq<0 || nFree<nReq)
       &&  (p=pcache1.grp.lru.pLruPrev)!=0
       &&  p->isAnchor==0
    ){
      nFree += pcache1_Mem_కొలత(p->పుట.pBuf);
      ప్రతిజ్ఞా( పుట_IS_UNPINNED(p) );
      pcache1_Pin_పుట(p);
      pcache1RemoveFromHash(p, 1);
    }
    pcache1LeaveMutex(&pcache1.grp);
  }
  తిరిగి nFree;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

#నిర్వచితమైతే SQLITE_TEST
/*
** This function is used by test procedures to inspect the internal state
** of the global cache.
*/
శూన్య sqlite3PcacheStats(
  పూర్ణాంకం *pnCurrent,      /* OUT: Total number of pages cached */
  పూర్ణాంకం *pnMax,          /* OUT: Global maximum cache size */
  పూర్ణాంకం *pnMin,          /* OUT: Sum of PCache1.nMin for purgeable caches */
  పూర్ణాంకం *pnRecyclable    /* OUT: Total number of pages available for recycling */
){
  PgHdr1 *p;
  పూర్ణాంకం nRecyclable = 0;
  కొరకు(p=pcache1.grp.lru.p_Lru_తదుపరి; p && !p->isAnchor; p=p->p_Lru_తదుపరి){
    ప్రతిజ్ఞా( పుట_IS_UNPINNED(p) );
    nRecyclable++;
  }
  *pnCurrent = pcache1.grp.nPurgeable;
  *pnMax = (పూర్ణాంకం)pcache1.grp.n_Max_పుట;
  *pnMin = (పూర్ణాంకం)pcache1.grp.n_Min_పుట;
  *pnRecyclable = nRecyclable;
}
#అయితే_ముగింపు
