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
#অন্তর্ভাব "sqliteInt.h"

প্রকারলেখ কাঠামো PCache1 PCache1;
প্রকারলেখ কাঠামো PgHdr1 PgHdr1;
প্রকারলেখ কাঠামো PgFreeslot PgFreeslot;
প্রকারলেখ কাঠামো PGroup PGroup;

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
কাঠামো PgHdr1 {
  sqlite3_pcache_page page; /* Base class. Must be first. pBuf & pExtra */
  unsigned পূর্ণসংখ্যা i_চাবি;        /* Key value (page number) */
  u16 isBulkLocal;          /* This page from bulk local storage */
  u16 isAnchor;             /* This is the PGroup.lru element */
  PgHdr1 *p_পরবর্তী;            /* Next in hash table chain */
  PCache1 *pCache;          /* Cache that currently owns this page */
  PgHdr1 *p_Lru_পরবর্তী;         /* Next in circular LRU list of unpinned pages */
  PgHdr1 *p_Lru_পূর্ব;         /* Previous in LRU list of unpinned pages */
                            /* NB: pLruPrev is only valid if pLruNext!=0 */
};

/*
** A page is pinned if it is not on the LRU list.  To be "pinned" means
** that the page is in active use and must not be deallocated.
*/
#define PAGE_IS_PINNED(p)    ((p)->p_Lru_পরবর্তী==0)
#define PAGE_IS_UNPINNED(p)  ((p)->p_Lru_পরবর্তী!=0)

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
কাঠামো PGroup {
  sqlite3_mutex *mutex;          /* MUTEX_STATIC_LRU or NULL */
  unsigned পূর্ণসংখ্যা nMaxPage;         /* Sum of nMax for purgeable caches */
  unsigned পূর্ণসংখ্যা nMinPage;         /* Sum of nMin for purgeable caches */
  unsigned পূর্ণসংখ্যা mxPinned;         /* nMaxpage + 10 - nMinPage */
  unsigned পূর্ণসংখ্যা nPurgeable;       /* Number of purgeable pages allocated */
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
কাঠামো PCache1 {
  /* Cache configuration parameters. Page size (szPage) and the purgeable
  ** flag (bPurgeable) and the pnPurgeable pointer are all set when the
  ** cache is created and are never changed thereafter. nMax may be
  ** modified at any time by a call to the pcache1Cachesize() method.
  ** The PGroup mutex must be held when accessing nMax.
  */
  PGroup *pGroup;                     /* PGroup this cache belongs to */
  unsigned পূর্ণসংখ্যা *pnPurgeable;          /* Pointer to pGroup->nPurgeable */
  পূর্ণসংখ্যা szPage;                         /* Size of database content section */
  পূর্ণসংখ্যা szExtra;                        /* sizeof(MemPage)+sizeof(PgHdr) */
  পূর্ণসংখ্যা szAlloc;                        /* Total size of one pcache line */
  পূর্ণসংখ্যা bPurgeable;                     /* True if cache is purgeable */
  unsigned পূর্ণসংখ্যা nMin;                  /* Minimum number of pages reserved */
  unsigned পূর্ণসংখ্যা nMax;                  /* Configured "cache_size" value */
  unsigned পূর্ণসংখ্যা n90pct;                /* nMax*9/10 */
  unsigned পূর্ণসংখ্যা i_Max_চাবি;               /* Largest key seen since xTruncate() */
  unsigned পূর্ণসংখ্যা nPurgeableDummy;       /* pnPurgeable points here when not used*/

  /* Hash table of all pages. The following variables may only be accessed
  ** when the accessor is holding the PGroup mutex.
  */
  unsigned পূর্ণসংখ্যা nRecyclable;           /* Number of pages in the LRU list */
  unsigned পূর্ণসংখ্যা nPage;                 /* Total number of pages in apHash */
  unsigned পূর্ণসংখ্যা nHash;                 /* Number of slots in apHash[] */
  PgHdr1 **apHash;                    /* Hash table for fast lookup by key */
  PgHdr1 *pFree;                      /* List of unused pcache-local pages */
  শূন্য *pBulk;                        /* Bulk memory used by pcache-local */
};

/*
** Free slots in the allocator used to divide up the global page cache
** buffer provided using the SQLITE_CONFIG_PAGECACHE mechanism.
*/
কাঠামো PgFreeslot {
  PgFreeslot *p_পরবর্তী;  /* Next free slot */
};

/*
** Global data used by this cache.
*/
স্থির SQLITE_WSD কাঠামো PCacheGlobal {
  PGroup grp;                    /* The global PGroup for mode (2) */

  /* Variables related to SQLITE_CONFIG_PAGECACHE settings.  The
  ** szSlot, nSlot, pStart, pEnd, nReserve, and isInit values are all
  ** fixed at sqlite3_initialize() time and do not require mutex protection.
  ** The nFreeSlot and pFree values do require mutex protection.
  */
  পূর্ণসংখ্যা isInit;                    /* True if initialized */
  পূর্ণসংখ্যা separateCache;             /* Use a new PGroup for each PCache */
  পূর্ণসংখ্যা nInitPage;                 /* Initial bulk allocation size */
  পূর্ণসংখ্যা szSlot;                    /* Size of each free slot */
  পূর্ণসংখ্যা nSlot;                     /* The number of pcache slots */
  পূর্ণসংখ্যা nReserve;                  /* Try to keep nFreeSlot above this */
  শূন্য *pStart, *pEnd;           /* Bounds of global page cache memory */
  /* Above requires no mutex.  Use mutex below for variable that follow. */
  sqlite3_mutex *mutex;          /* Mutex for accessing the following: */
  PgFreeslot *pFree;             /* Free page blocks */
  পূর্ণসংখ্যা nFreeSlot;                 /* Number of unused pcache slots */
  পূর্ণসংখ্যা bUnderPressure;            /* True if low on PAGECACHE memory */
} pcache1_g;

/*
** All code in this file should access the global structure above via the
** alias "pcache1". This ensures that the WSD emulation is used when
** compiling for systems that do not support real WSD.
*/
#define pcache1 (GLOBAL(কাঠামো PCacheGlobal, pcache1_g))

/*
** Macros to enter and leave the PCache LRU mutex.
*/
#যদি !defined(SQLITE_ENABLE_MEMORY_MANAGEMENT) || SQLITE_THREADSAFE==0
# define pcache1EnterMutex(X)  প্রতিজ্ঞা((X)->mutex==0)
# define pcache1LeaveMutex(X)  প্রতিজ্ঞা((X)->mutex==0)
# define PCACHE1_MIGHT_USE_GROUP_MUTEX 0
#অন্যথা
# define pcache1EnterMutex(X) sqlite3_mutex_enter((X)->mutex)
# define pcache1LeaveMutex(X) sqlite3_mutex_leave((X)->mutex)
# define PCACHE1_MIGHT_USE_GROUP_MUTEX 1
#endif

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
শূন্য sqlite3_PCache_বাফার_Setup(শূন্য *pBuf, পূর্ণসংখ্যা sz, পূর্ণসংখ্যা n){
  যদি( pcache1.isInit ){
    PgFreeslot *p;
    যদি( pBuf==0 ) sz = n = 0;
    যদি( n==0 ) sz = 0;
    sz = ROUNDDOWN8(sz);
    pcache1.szSlot = sz;
    pcache1.nSlot = pcache1.nFreeSlot = n;
    pcache1.nReserve = n>90 ? 10 : (n/10 + 1);
    pcache1.pStart = pBuf;
    pcache1.pFree = 0;
    AtomicStore(&pcache1.bUnderPressure,0);
    যখন( n-- ){
      p = (PgFreeslot*)pBuf;
      p->p_পরবর্তী = pcache1.pFree;
      pcache1.pFree = p;
      pBuf = (শূন্য*)&((অক্ষর*)pBuf)[sz];
    }
    pcache1.pEnd = pBuf;
  }
}

/*
** Try to initialize the pCache->pFree and pCache->pBulk fields.  Return
** true if pCache->pFree ends up containing one or more free pages.
*/
স্থির পূর্ণসংখ্যা pcache1InitBulk(PCache1 *pCache){
  i64 szBulk;
  অক্ষর *zBulk;
  যদি( pcache1.nInitPage==0 ) ফেরত 0;
  /* Do not bother with a bulk allocation if the cache size very small */
  যদি( pCache->nMax<3 ) ফেরত 0;
  sqlite3BeginBenignMalloc();
  যদি( pcache1.nInitPage>0 ){
    szBulk = pCache->szAlloc * (i64)pcache1.nInitPage;
  }অন্যথা{
    szBulk = -1024 * (i64)pcache1.nInitPage;
  }
  যদি( szBulk > pCache->szAlloc*(i64)pCache->nMax ){
    szBulk = pCache->szAlloc*(i64)pCache->nMax;
  }
  যদি( szBulk>=pCache->szAlloc ){
    zBulk = pCache->pBulk = sqlite3Malloc( szBulk );
    sqlite3EndBenignMalloc();
    যদি( zBulk ){
      পূর্ণসংখ্যা nBulk = sqlite3_Malloc_আকার(zBulk)/pCache->szAlloc;
      করো{
        PgHdr1 *pX = (PgHdr1*)&zBulk[pCache->szPage];
        pX->page.pBuf = zBulk;
        pX->page.pExtra = (u8*)pX + ROUND8(আকার(*pX));
        প্রতিজ্ঞা( EIGHT_BYTE_ALIGNMENT( pX->page.pExtra ) );
        pX->isBulkLocal = 1;
        pX->isAnchor = 0;
        pX->p_পরবর্তী = pCache->pFree;
        pX->p_Lru_পূর্ব = 0;    /* Initializing this saves a valgrind error */
        pCache->pFree = pX;
        zBulk += pCache->szAlloc;
      }যখন( --nBulk );
    }
  }
  ফেরত pCache->pFree!=0;
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
স্থির শূন্য *pcache1Alloc(পূর্ণসংখ্যা nByte){
  শূন্য *p = 0;
  প্রতিজ্ঞা( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  যদি( nByte<=pcache1.szSlot ){
    sqlite3_mutex_enter(pcache1.mutex);
    p = (PgHdr1 *)pcache1.pFree;
    যদি( p ){
      pcache1.pFree = pcache1.pFree->p_পরবর্তী;
      pcache1.nFreeSlot--;
      AtomicStore(&pcache1.bUnderPressure,pcache1.nFreeSlot<pcache1.nReserve);
      প্রতিজ্ঞা( pcache1.nFreeSlot>=0 );
      sqlite3StatusHighwater(SQLITE_STATUS_PAGECACHE_আকার, nByte);
      sqlite3StatusUp(SQLITE_STATUS_PAGECACHE_USED, 1);
    }
    sqlite3_mutex_leave(pcache1.mutex);
  }
  যদি( p==0 ){
    /* Memory is not available in the SQLITE_CONFIG_PAGECACHE pool.  Get
    ** it from sqlite3Malloc instead.
    */
    p = sqlite3Malloc(nByte);
#ifndef SQLITE_DISABLE_PAGECACHE_OVERFLOW_STATS
    যদি( p ){
      পূর্ণসংখ্যা sz = sqlite3_Malloc_আকার(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3StatusHighwater(SQLITE_STATUS_PAGECACHE_আকার, nByte);
      sqlite3StatusUp(SQLITE_STATUS_PAGECACHE_OVERFLOW, sz);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#endif
    sqlite3MemdebugSetType(p, MEMTYPE_PCACHE);
  }
  ফেরত p;
}

/*
** Free an allocated buffer obtained from pcache1Alloc().
*/
স্থির শূন্য pcache1Free(শূন্য *p){
  যদি( p==0 ) ফেরত;
  যদি( SQLITE_WITHIN(p, pcache1.pStart, pcache1.pEnd) ){
    PgFreeslot *pSlot;
    sqlite3_mutex_enter(pcache1.mutex);
    sqlite3StatusDown(SQLITE_STATUS_PAGECACHE_USED, 1);
    pSlot = (PgFreeslot*)p;
    pSlot->p_পরবর্তী = pcache1.pFree;
    pcache1.pFree = pSlot;
    pcache1.nFreeSlot++;
    AtomicStore(&pcache1.bUnderPressure,pcache1.nFreeSlot<pcache1.nReserve);
    প্রতিজ্ঞা( pcache1.nFreeSlot<=pcache1.nSlot );
    sqlite3_mutex_leave(pcache1.mutex);
  }অন্যথা{
    প্রতিজ্ঞা( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
#ifndef SQLITE_DISABLE_PAGECACHE_OVERFLOW_STATS
    {
      পূর্ণসংখ্যা nFreed = 0;
      nFreed = sqlite3_Malloc_আকার(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3StatusDown(SQLITE_STATUS_PAGECACHE_OVERFLOW, nFreed);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#endif
    sqlite3_free(p);
  }
}

#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
/*
** Return the size of a pcache allocation
*/
স্থির পূর্ণসংখ্যা pcache1_Mem_আকার(শূন্য *p){
  যদি( p>=pcache1.pStart && p<pcache1.pEnd ){
    ফেরত pcache1.szSlot;
  }অন্যথা{
    পূর্ণসংখ্যা i_আকার;
    প্রতিজ্ঞা( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
    i_আকার = sqlite3_Malloc_আকার(p);
    sqlite3MemdebugSetType(p, MEMTYPE_PCACHE);
    ফেরত i_আকার;
  }
}
#endif /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

/*
** Allocate a new page object initially associated with cache pCache.
*/
স্থির PgHdr1 *pcache1AllocPage(PCache1 *pCache, পূর্ণসংখ্যা benignMalloc){
  PgHdr1 *p = 0;
  শূন্য *pPg;

  প্রতিজ্ঞা( sqlite3_mutex_held(pCache->pGroup->mutex) );
  যদি( pCache->pFree || (pCache->nPage==0 && pcache1InitBulk(pCache)) ){
    প্রতিজ্ঞা( pCache->pFree!=0 );
    p = pCache->pFree;
    pCache->pFree = p->p_পরবর্তী;
    p->p_পরবর্তী = 0;
  }অন্যথা{
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    /* The group mutex must be released before pcache1Alloc() is called. This
    ** is because it might call sqlite3_release_memory(), which assumes that
    ** this mutex is not held. */
    প্রতিজ্ঞা( pcache1.separateCache==0 );
    প্রতিজ্ঞা( pCache->pGroup==&pcache1.grp );
    pcache1LeaveMutex(pCache->pGroup);
#endif
    যদি( benignMalloc ){ sqlite3BeginBenignMalloc(); }
    pPg = pcache1Alloc(pCache->szAlloc);
    যদি( benignMalloc ){ sqlite3EndBenignMalloc(); }
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    pcache1EnterMutex(pCache->pGroup);
#endif
    যদি( pPg==0 ) ফেরত 0;
    p = (PgHdr1 *)&((u8 *)pPg)[pCache->szPage];
    p->page.pBuf = pPg;
    p->page.pExtra = (u8*)p + ROUND8(আকার(*p));
    প্রতিজ্ঞা( EIGHT_BYTE_ALIGNMENT( p->page.pExtra ) );
    p->isBulkLocal = 0;
    p->isAnchor = 0;
    p->p_Lru_পূর্ব = 0;           /* Initializing this saves a valgrind error */
  }
  (*pCache->pnPurgeable)++;
  ফেরত p;
}

/*
** Free a page object allocated by pcache1AllocPage().
*/
স্থির শূন্য pcache1FreePage(PgHdr1 *p){
  PCache1 *pCache;
  প্রতিজ্ঞা( p!=0 );
  pCache = p->pCache;
  প্রতিজ্ঞা( sqlite3_mutex_held(p->pCache->pGroup->mutex) );
  যদি( p->isBulkLocal ){
    p->p_পরবর্তী = pCache->pFree;
    pCache->pFree = p;
  }অন্যথা{
    pcache1Free(p->page.pBuf);
  }
  (*pCache->pnPurgeable)--;
}

/*
** Malloc function used by SQLite to obtain space from the buffer configured
** using sqlite3_config(SQLITE_CONFIG_PAGECACHE) option. If no such buffer
** exists, this function falls back to sqlite3Malloc().
*/
শূন্য *sqlite3PageMalloc(পূর্ণসংখ্যা sz){
  প্রতিজ্ঞা( sz<=65536+8 ); /* These allocations are never very large */
  ফেরত pcache1Alloc(sz);
}

/*
** Free an allocated buffer obtained from sqlite3PageMalloc().
*/
শূন্য sqlite3PageFree(শূন্য *p){
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
স্থির পূর্ণসংখ্যা pcache1UnderMemoryPressure(PCache1 *pCache){
  যদি( pcache1.nSlot && (pCache->szPage+pCache->szExtra)<=pcache1.szSlot ){
    ফেরত AtomicLoad(&pcache1.bUnderPressure);
  }অন্যথা{
    ফেরত sqlite3HeapNearlyFull();
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
স্থির শূন্য pcache1ResizeHash(PCache1 *p){
  PgHdr1 **apNew;
  u64 nNew;
  u32 i;

  প্রতিজ্ঞা( sqlite3_mutex_held(p->pGroup->mutex) );

  nNew = 2*(u64)p->nHash;
  যদি( nNew<256 ){
    nNew = 256;
  }
  প্রতিজ্ঞা( (nNew & (nNew-1))==0 );   /* nNew is always a power of two */

  pcache1LeaveMutex(p->pGroup);
  যদি( p->nHash ){ sqlite3BeginBenignMalloc(); }
  apNew = (PgHdr1 **)sqlite3MallocZero(আকার(PgHdr1 *)*nNew);
  যদি( p->nHash ){ sqlite3EndBenignMalloc(); }
  pcache1EnterMutex(p->pGroup);
  যদি( apNew ){
    জন্য(i=0; i<p->nHash; i++){
      PgHdr1 *pPage;
      PgHdr1 *p_পরবর্তী = p->apHash[i];
      যখন( (pPage = p_পরবর্তী)!=0 ){
        unsigned পূর্ণসংখ্যা h = (unsigned পূর্ণসংখ্যা)(pPage->i_চাবি & (nNew-1));
        p_পরবর্তী = pPage->p_পরবর্তী;
        pPage->p_পরবর্তী = apNew[h];
        apNew[h] = pPage;
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
স্থির PgHdr1 *pcache1PinPage(PgHdr1 *pPage){
  প্রতিজ্ঞা( pPage!=0 );
  প্রতিজ্ঞা( PAGE_IS_UNPINNED(pPage) );
  প্রতিজ্ঞা( pPage->p_Lru_পরবর্তী );
  প্রতিজ্ঞা( pPage->p_Lru_পূর্ব );
  প্রতিজ্ঞা( sqlite3_mutex_held(pPage->pCache->pGroup->mutex) );
  pPage->p_Lru_পূর্ব->p_Lru_পরবর্তী = pPage->p_Lru_পরবর্তী;
  pPage->p_Lru_পরবর্তী->p_Lru_পূর্ব = pPage->p_Lru_পূর্ব;
  pPage->p_Lru_পরবর্তী = 0;
  /* pPage->pLruPrev = 0;
  ** No need to clear pLruPrev as it is never accessed if pLruNext is 0 */
  প্রতিজ্ঞা( pPage->isAnchor==0 );
  প্রতিজ্ঞা( pPage->pCache->pGroup->lru.isAnchor==1 );
  pPage->pCache->nRecyclable--;
  ফেরত pPage;
}


/*
** Remove the page supplied as an argument from the hash table
** (PCache1.apHash structure) that it is currently stored in.
** Also free the page if freePage is true.
**
** The PGroup mutex must be held when this function is called.
*/
স্থির শূন্য pcache1RemoveFromHash(PgHdr1 *pPage, পূর্ণসংখ্যা freeFlag){
  unsigned পূর্ণসংখ্যা h;
  PCache1 *pCache = pPage->pCache;
  PgHdr1 **pp;

  প্রতিজ্ঞা( sqlite3_mutex_held(pCache->pGroup->mutex) );
  প্রতিজ্ঞা( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  h = pPage->i_চাবি & (pCache->nHash-1);
  জন্য(pp=&pCache->apHash[h]; (*pp)!=pPage; pp=&(*pp)->p_পরবর্তী);
  *pp = (*pp)->p_পরবর্তী;

  pCache->nPage--;
  যদি( freeFlag ) pcache1FreePage(pPage);
}

/*
** If there are currently more than nMaxPage pages allocated, try
** to recycle pages to reduce the number allocated to nMaxPage.
*/
স্থির শূন্য pcache1EnforceMaxPage(PCache1 *pCache){
  PGroup *pGroup = pCache->pGroup;
  PgHdr1 *p;
  প্রতিজ্ঞা( sqlite3_mutex_held(pGroup->mutex) );
  যখন( pGroup->nPurgeable>pGroup->nMaxPage
      && (p=pGroup->lru.p_Lru_পূর্ব)->isAnchor==0
  ){
    প্রতিজ্ঞা( p->pCache->pGroup==pGroup );
    প্রতিজ্ঞা( PAGE_IS_UNPINNED(p) );
    pcache1PinPage(p);
    pcache1RemoveFromHash(p, 1);
  }
  যদি( pCache->nPage==0 && pCache->pBulk ){
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
স্থির শূন্য pcache1TruncateUnsafe(
  PCache1 *pCache,             /* The cache to truncate */
  unsigned পূর্ণসংখ্যা iLimit          /* Drop pages with this pgno or larger */
){
  TESTONLY( পূর্ণসংখ্যা nPage = 0; )  /* To assert pCache->nPage is correct */
  unsigned পূর্ণসংখ্যা h, iStop;
  প্রতিজ্ঞা( sqlite3_mutex_held(pCache->pGroup->mutex) );
  প্রতিজ্ঞা( pCache->i_Max_চাবি >= iLimit );
  প্রতিজ্ঞা( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  যদি( pCache->i_Max_চাবি - iLimit < pCache->nHash ){
    /* If we are just shaving the last few pages off the end of the
    ** cache, then there is no point in scanning the entire hash table.
    ** Only scan those hash slots that might contain pages that need to
    ** be removed. */
    h = iLimit & (pCache->nHash-1);
    iStop = pCache->i_Max_চাবি & (pCache->nHash-1);
    TESTONLY( nPage = -10; )  /* Disable the pCache->nPage validity check */
  }অন্যথা{
    /* This is the general case where many pages are being removed.
    ** It is necessary to scan the entire hash table */
    h = pCache->nHash/2;
    iStop = h - 1;
  }
  জন্য(;;){
    PgHdr1 **pp;
    PgHdr1 *pPage;
    প্রতিজ্ঞা( h<pCache->nHash );
    pp = &pCache->apHash[h];
    যখন( (pPage = *pp)!=0 ){
      যদি( pPage->i_চাবি>=iLimit ){
        pCache->nPage--;
        *pp = pPage->p_পরবর্তী;
        যদি( PAGE_IS_UNPINNED(pPage) ) pcache1PinPage(pPage);
        pcache1FreePage(pPage);
      }অন্যথা{
        pp = &pPage->p_পরবর্তী;
        TESTONLY( যদি( nPage>=0 ) nPage++; )
      }
    }
    যদি( h==iStop ) বিরতি;
    h = (h+1) & (pCache->nHash-1);
  }
  প্রতিজ্ঞা( nPage<0 || pCache->nPage==(unsigned)nPage );
}

/******************************************************************************/
/******** sqlite3_pcache Methods **********************************************/

/*
** Implementation of the sqlite3_pcache.xInit method.
*/
স্থির পূর্ণসংখ্যা pcache1Init(শূন্য *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  প্রতিজ্ঞা( pcache1.isInit==0 );
  memset(&pcache1, 0, আকার(pcache1));


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
#যদি defined(SQLITE_ENABLE_MEMORY_MANAGEMENT)
  pcache1.separateCache = 0;
#নতুবা_যদি SQLITE_THREADSAFE
  pcache1.separateCache = sqlite3GlobalConfig.pPage==0
                          || sqlite3GlobalConfig.bCoreMutex>0;
#অন্যথা
  pcache1.separateCache = sqlite3GlobalConfig.pPage==0;
#endif

#যদি SQLITE_THREADSAFE
  যদি( sqlite3GlobalConfig.bCoreMutex ){
    pcache1.grp.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_LRU);
    pcache1.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PMEM);
  }
#endif
  যদি( pcache1.separateCache
   && sqlite3GlobalConfig.nPage!=0
   && sqlite3GlobalConfig.pPage==0
  ){
    pcache1.nInitPage = sqlite3GlobalConfig.nPage;
  }অন্যথা{
    pcache1.nInitPage = 0;
  }
  pcache1.grp.mxPinned = 10;
  pcache1.isInit = 1;
  ফেরত SQLITE_OK;
}

/*
** Implementation of the sqlite3_pcache.xShutdown method.
** Note that the static mutex allocated in xInit does
** not need to be freed.
*/
স্থির শূন্য pcache1Shutdown(শূন্য *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  প্রতিজ্ঞা( pcache1.isInit!=0 );
  memset(&pcache1, 0, আকার(pcache1));
}

/* forward declaration */
স্থির শূন্য pcache1Destroy(sqlite3_pcache *p);

/*
** Implementation of the sqlite3_pcache.xCreate method.
**
** Allocate a new cache.
*/
স্থির sqlite3_pcache *pcache1Create(পূর্ণসংখ্যা szPage, পূর্ণসংখ্যা szExtra, পূর্ণসংখ্যা bPurgeable){
  PCache1 *pCache;      /* The newly created page cache */
  PGroup *pGroup;       /* The group the new page cache will belong to */
  i64 sz;               /* Bytes of memory required to allocate the new cache */

  প্রতিজ্ঞা( (szPage & (szPage-1))==0 && szPage>=512 && szPage<=65536 );
  প্রতিজ্ঞা( szExtra < 300 );

  sz = আকার(PCache1) + আকার(PGroup)*pcache1.separateCache;
  pCache = (PCache1 *)sqlite3MallocZero(sz);
  যদি( pCache ){
    যদি( pcache1.separateCache ){
      pGroup = (PGroup*)&pCache[1];
      pGroup->mxPinned = 10;
    }অন্যথা{
      pGroup = &pcache1.grp;
    }
    pcache1EnterMutex(pGroup);
    যদি( pGroup->lru.isAnchor==0 ){
      pGroup->lru.isAnchor = 1;
      pGroup->lru.p_Lru_পূর্ব = pGroup->lru.p_Lru_পরবর্তী = &pGroup->lru;
    }
    pCache->pGroup = pGroup;
    pCache->szPage = szPage;
    pCache->szExtra = szExtra;
    pCache->szAlloc = szPage + szExtra + ROUND8(আকার(PgHdr1));
    pCache->bPurgeable = (bPurgeable ? 1 : 0);
    pcache1ResizeHash(pCache);
    যদি( bPurgeable ){
      pCache->nMin = 10;
      pGroup->nMinPage += pCache->nMin;
      pGroup->mxPinned = pGroup->nMaxPage + 10 - pGroup->nMinPage;
      pCache->pnPurgeable = &pGroup->nPurgeable;
    }অন্যথা{
      pCache->pnPurgeable = &pCache->nPurgeableDummy;
    }
    pcache1LeaveMutex(pGroup);
    যদি( pCache->nHash==0 ){
      pcache1Destroy((sqlite3_pcache*)pCache);
      pCache = 0;
    }
  }
  ফেরত (sqlite3_pcache *)pCache;
}

/*
** Implementation of the sqlite3_pcache.xCachesize method.
**
** Configure the cache_size limit for a cache.
*/
স্থির শূন্য pcache1Cachesize(sqlite3_pcache *p, পূর্ণসংখ্যা nMax){
  PCache1 *pCache = (PCache1 *)p;
  u32 n;
  প্রতিজ্ঞা( nMax>=0 );
  যদি( pCache->bPurgeable ){
    PGroup *pGroup = pCache->pGroup;
    pcache1EnterMutex(pGroup);
    n = (u32)nMax;
    যদি( n > 0x7fff0000 - pGroup->nMaxPage + pCache->nMax ){
      n = 0x7fff0000 - pGroup->nMaxPage + pCache->nMax;
    }
    pGroup->nMaxPage += (n - pCache->nMax);
    pGroup->mxPinned = pGroup->nMaxPage + 10 - pGroup->nMinPage;
    pCache->nMax = n;
    pCache->n90pct = pCache->nMax*9/10;
    pcache1EnforceMaxPage(pCache);
    pcache1LeaveMutex(pGroup);
  }
}

/*
** Implementation of the sqlite3_pcache.xShrink method.
**
** Free up as much memory as possible.
*/
স্থির শূন্য pcache1Shrink(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1*)p;
  যদি( pCache->bPurgeable ){
    PGroup *pGroup = pCache->pGroup;
    unsigned পূর্ণসংখ্যা savedMaxPage;
    pcache1EnterMutex(pGroup);
    savedMaxPage = pGroup->nMaxPage;
    pGroup->nMaxPage = 0;
    pcache1EnforceMaxPage(pCache);
    pGroup->nMaxPage = savedMaxPage;
    pcache1LeaveMutex(pGroup);
  }
}

/*
** Implementation of the sqlite3_pcache.xPagecount method.
*/
স্থির পূর্ণসংখ্যা pcache1Pagecount(sqlite3_pcache *p){
  পূর্ণসংখ্যা n;
  PCache1 *pCache = (PCache1*)p;
  pcache1EnterMutex(pCache->pGroup);
  n = pCache->nPage;
  pcache1LeaveMutex(pCache->pGroup);
  ফেরত n;
}


/*
** Implement steps 3, 4, and 5 of the pcache1Fetch() algorithm described
** in the header of the pcache1Fetch() procedure.
**
** This steps are broken out into a separate procedure because they are
** usually not needed, and by avoiding the stack initialization required
** for these steps, the main pcache1Fetch() procedure can run faster.
*/
স্থির SQLITE_NOINLINE PgHdr1 *pcache1FetchStage2(
  PCache1 *pCache,
  unsigned পূর্ণসংখ্যা i_চাবি,
  পূর্ণসংখ্যা createFlag
){
  unsigned পূর্ণসংখ্যা nPinned;
  PGroup *pGroup = pCache->pGroup;
  PgHdr1 *pPage = 0;

  /* Step 3: Abort if createFlag is 1 but the cache is nearly full */
  প্রতিজ্ঞা( pCache->nPage >= pCache->nRecyclable );
  nPinned = pCache->nPage - pCache->nRecyclable;
  প্রতিজ্ঞা( pGroup->mxPinned == pGroup->nMaxPage + 10 - pGroup->nMinPage );
  প্রতিজ্ঞা( pCache->n90pct == pCache->nMax*9/10 );
  যদি( createFlag==1 && (
        nPinned>=pGroup->mxPinned
     || nPinned>=pCache->n90pct
     || (pcache1UnderMemoryPressure(pCache) && pCache->nRecyclable<nPinned)
  )){
    ফেরত 0;
  }

  যদি( pCache->nPage>=pCache->nHash ) pcache1ResizeHash(pCache);
  প্রতিজ্ঞা( pCache->nHash>0 && pCache->apHash );
  প্রতিজ্ঞা( (pCache->nHash & (pCache->nHash-1))==0 );

  /* Step 4. Try to recycle a page. */
  যদি( pCache->bPurgeable
   && !pGroup->lru.p_Lru_পূর্ব->isAnchor
   && ((pCache->nPage+1>=pCache->nMax) || pcache1UnderMemoryPressure(pCache))
  ){
    PCache1 *pOther;
    pPage = pGroup->lru.p_Lru_পূর্ব;
    প্রতিজ্ঞা( PAGE_IS_UNPINNED(pPage) );
    pcache1RemoveFromHash(pPage, 0);
    pcache1PinPage(pPage);
    pOther = pPage->pCache;
    যদি( pOther->szAlloc != pCache->szAlloc ){
      pcache1FreePage(pPage);
      pPage = 0;
    }অন্যথা{
      pGroup->nPurgeable -= (pOther->bPurgeable - pCache->bPurgeable);
    }
  }

  /* Step 5. If a usable page buffer has still not been found,
  ** attempt to allocate a new one.
  */
  যদি( !pPage ){
    pPage = pcache1AllocPage(pCache, createFlag==1);
  }

  যদি( pPage ){
    unsigned পূর্ণসংখ্যা h = i_চাবি & (pCache->nHash-1);
    pCache->nPage++;
    pPage->i_চাবি = i_চাবি;
    pPage->p_পরবর্তী = pCache->apHash[h];
    pPage->pCache = pCache;
    pPage->p_Lru_পরবর্তী = 0;
    /* pPage->pLruPrev = 0;
    ** No need to clear pLruPrev since it is not accessed when pLruNext==0 */
    *(শূন্য **)pPage->page.pExtra = 0;
    pCache->apHash[h] = pPage;
    যদি( i_চাবি>pCache->i_Max_চাবি ){
      pCache->i_Max_চাবি = i_চাবি;
    }
  }
  ফেরত pPage;
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
স্থির PgHdr1 *pcache1FetchNoMutex(
  sqlite3_pcache *p,
  unsigned পূর্ণসংখ্যা i_চাবি,
  পূর্ণসংখ্যা createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage = 0;

  /* Step 1: Search the hash table for an existing entry.  nHash is always
  ** a power of two when the cache is in use (see pcache1ResizeHash()), so
  ** the modulo reduces to a mask, avoiding a hardware divide. */
  প্রতিজ্ঞা( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  pPage = pCache->apHash[i_চাবি & (pCache->nHash-1u)];
  যখন( pPage && pPage->i_চাবি!=i_চাবি ){ pPage = pPage->p_পরবর্তী; }

  /* Step 2: If the page was found in the hash table, then return it.
  ** If the page was not in the hash table and createFlag is 0, abort.
  ** Otherwise (page not in hash and createFlag!=0) continue with
  ** subsequent steps to try to create the page. */
  যদি( pPage ){
    যদি( PAGE_IS_UNPINNED(pPage) ){
      ফেরত pcache1PinPage(pPage);
    }অন্যথা{
      ফেরত pPage;
    }
  }অন্যথা যদি( createFlag ){
    /* Steps 3, 4, and 5 implemented by this subroutine */
    ফেরত pcache1FetchStage2(pCache, i_চাবি, createFlag);
  }অন্যথা{
    ফেরত 0;
  }
}
#যদি PCACHE1_MIGHT_USE_GROUP_MUTEX
স্থির PgHdr1 *pcache1FetchWithMutex(
  sqlite3_pcache *p,
  unsigned পূর্ণসংখ্যা i_চাবি,
  পূর্ণসংখ্যা createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage;

  pcache1EnterMutex(pCache->pGroup);
  pPage = pcache1FetchNoMutex(p, i_চাবি, createFlag);
  প্রতিজ্ঞা( pPage==0 || pCache->i_Max_চাবি>=i_চাবি );
  pcache1LeaveMutex(pCache->pGroup);
  ফেরত pPage;
}
#endif
স্থির sqlite3_pcache_page *pcache1Fetch(
  sqlite3_pcache *p,
  unsigned পূর্ণসংখ্যা i_চাবি,
  পূর্ণসংখ্যা createFlag
){
#যদি PCACHE1_MIGHT_USE_GROUP_MUTEX || defined(SQLITE_DEBUG)
  PCache1 *pCache = (PCache1 *)p;
#endif

  প্রতিজ্ঞা( offsetof(PgHdr1,page)==0 );
  প্রতিজ্ঞা( pCache->bPurgeable || createFlag!=1 );
  প্রতিজ্ঞা( pCache->bPurgeable || pCache->nMin==0 );
  প্রতিজ্ঞা( pCache->bPurgeable==0 || pCache->nMin==10 );
  প্রতিজ্ঞা( pCache->nMin==0 || pCache->bPurgeable );
  প্রতিজ্ঞা( pCache->nHash>0 );
#যদি PCACHE1_MIGHT_USE_GROUP_MUTEX
  যদি( pCache->pGroup->mutex ){
    ফেরত (sqlite3_pcache_page*)pcache1FetchWithMutex(p, i_চাবি, createFlag);
  }অন্যথা
#endif
  {
    ফেরত (sqlite3_pcache_page*)pcache1FetchNoMutex(p, i_চাবি, createFlag);
  }
}


/*
** Implementation of the sqlite3_pcache.xUnpin method.
**
** Mark a page as unpinned (eligible for asynchronous recycling).
*/
স্থির শূন্য pcache1Unpin(
  sqlite3_pcache *p,
  sqlite3_pcache_page *pPg,
  পূর্ণসংখ্যা reuseUnlikely
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage = (PgHdr1 *)pPg;
  PGroup *pGroup = pCache->pGroup;

  প্রতিজ্ঞা( pPage->pCache==pCache );
  pcache1EnterMutex(pGroup);

  /* It is an error to call this function if the page is already
  ** part of the PGroup LRU list.
  */
  প্রতিজ্ঞা( pPage->p_Lru_পরবর্তী==0 );
  প্রতিজ্ঞা( PAGE_IS_PINNED(pPage) );

  যদি( reuseUnlikely || pGroup->nPurgeable>pGroup->nMaxPage ){
    pcache1RemoveFromHash(pPage, 1);
  }অন্যথা{
    /* Add the page to the PGroup LRU list. */
    PgHdr1 **ppFirst = &pGroup->lru.p_Lru_পরবর্তী;
    pPage->p_Lru_পূর্ব = &pGroup->lru;
    (pPage->p_Lru_পরবর্তী = *ppFirst)->p_Lru_পূর্ব = pPage;
    *ppFirst = pPage;
    pCache->nRecyclable++;
  }

  pcache1LeaveMutex(pCache->pGroup);
}

/*
** Implementation of the sqlite3_pcache.xRekey method.
*/
স্থির শূন্য pcache1Rekey(
  sqlite3_pcache *p,
  sqlite3_pcache_page *pPg,
  unsigned পূর্ণসংখ্যা iOld,
  unsigned পূর্ণসংখ্যা iNew
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage = (PgHdr1 *)pPg;
  PgHdr1 **pp;
  unsigned পূর্ণসংখ্যা hOld, hNew;
  প্রতিজ্ঞা( pPage->i_চাবি==iOld );
  প্রতিজ্ঞা( pPage->pCache==pCache );
  প্রতিজ্ঞা( iOld!=iNew );               /* The page number really is changing */

  pcache1EnterMutex(pCache->pGroup);

  প্রতিজ্ঞা( pcache1FetchNoMutex(p, iOld, 0)==pPage ); /* pPg really is iOld */
  প্রতিজ্ঞা( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  hOld = iOld & (pCache->nHash-1);
  pp = &pCache->apHash[hOld];
  যখন( (*pp)!=pPage ){
    pp = &(*pp)->p_পরবর্তী;
  }
  *pp = pPage->p_পরবর্তী;

  প্রতিজ্ঞা( pcache1FetchNoMutex(p, iNew, 0)==0 ); /* iNew not in cache */
  hNew = iNew & (pCache->nHash-1);
  pPage->i_চাবি = iNew;
  pPage->p_পরবর্তী = pCache->apHash[hNew];
  pCache->apHash[hNew] = pPage;
  যদি( iNew>pCache->i_Max_চাবি ){
    pCache->i_Max_চাবি = iNew;
  }

  pcache1LeaveMutex(pCache->pGroup);
}

/*
** Implementation of the sqlite3_pcache.xTruncate method.
**
** Discard all unpinned pages in the cache with a page number equal to
** or greater than parameter iLimit. Any pinned pages with a page number
** equal to or greater than iLimit are implicitly unpinned.
*/
স্থির শূন্য pcache1Truncate(sqlite3_pcache *p, unsigned পূর্ণসংখ্যা iLimit){
  PCache1 *pCache = (PCache1 *)p;
  pcache1EnterMutex(pCache->pGroup);
  যদি( iLimit<=pCache->i_Max_চাবি ){
    pcache1TruncateUnsafe(pCache, iLimit);
    pCache->i_Max_চাবি = iLimit-1;
  }
  pcache1LeaveMutex(pCache->pGroup);
}

/*
** Implementation of the sqlite3_pcache.xDestroy method.
**
** Destroy a cache allocated using pcache1Create().
*/
স্থির শূন্য pcache1Destroy(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1 *)p;
  PGroup *pGroup = pCache->pGroup;
  প্রতিজ্ঞা( pCache->bPurgeable || (pCache->nMax==0 && pCache->nMin==0) );
  pcache1EnterMutex(pGroup);
  যদি( pCache->nPage ) pcache1TruncateUnsafe(pCache, 0);
  প্রতিজ্ঞা( pGroup->nMaxPage >= pCache->nMax );
  pGroup->nMaxPage -= pCache->nMax;
  প্রতিজ্ঞা( pGroup->nMinPage >= pCache->nMin );
  pGroup->nMinPage -= pCache->nMin;
  pGroup->mxPinned = pGroup->nMaxPage + 10 - pGroup->nMinPage;
  pcache1EnforceMaxPage(pCache);
  pcache1LeaveMutex(pGroup);
  sqlite3_free(pCache->pBulk);
  sqlite3_free(pCache->apHash);
  sqlite3_free(pCache);
}

/*
** This function is called during initialization (sqlite3_initialize()) to
** install the default pluggable cache module, assuming the user has not
** already provided an alternative.
*/
শূন্য sqlite3PCacheSetDefault(শূন্য){
  স্থির ধ্রুবক sqlite3_pcache_methods2 defaultMethods = {
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
পূর্ণসংখ্যা sqlite3_Header_আকার_Pcache1(শূন্য){ ফেরত ROUND8(আকার(PgHdr1)); }

/*
** Return the global mutex used by this PCACHE implementation.  The
** sqlite3_status() routine needs access to this mutex.
*/
sqlite3_mutex *sqlite3Pcache1Mutex(শূন্য){
  ফেরত pcache1.mutex;
}

#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
/*
** This function is called to free superfluous dynamically allocated memory
** held by the pager system. Memory in use by any SQLite pager allocated
** by the current thread may be sqlite3_free()ed.
**
** nReq is the number of bytes of memory required. Once this much has
** been released, the function returns. The return value is the total number
** of bytes of memory released.
*/
পূর্ণসংখ্যা sqlite3PcacheReleaseMemory(পূর্ণসংখ্যা nReq){
  পূর্ণসংখ্যা nFree = 0;
  প্রতিজ্ঞা( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  প্রতিজ্ঞা( sqlite3_mutex_notheld(pcache1.mutex) );
  যদি( sqlite3GlobalConfig.pPage==0 ){
    PgHdr1 *p;
    pcache1EnterMutex(&pcache1.grp);
    যখন( (nReq<0 || nFree<nReq)
       &&  (p=pcache1.grp.lru.p_Lru_পূর্ব)!=0
       &&  p->isAnchor==0
    ){
      nFree += pcache1_Mem_আকার(p->page.pBuf);
      প্রতিজ্ঞা( PAGE_IS_UNPINNED(p) );
      pcache1PinPage(p);
      pcache1RemoveFromHash(p, 1);
    }
    pcache1LeaveMutex(&pcache1.grp);
  }
  ফেরত nFree;
}
#endif /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

#ifdef SQLITE_TEST
/*
** This function is used by test procedures to inspect the internal state
** of the global cache.
*/
শূন্য sqlite3PcacheStats(
  পূর্ণসংখ্যা *pnCurrent,      /* OUT: Total number of pages cached */
  পূর্ণসংখ্যা *pnMax,          /* OUT: Global maximum cache size */
  পূর্ণসংখ্যা *pnMin,          /* OUT: Sum of PCache1.nMin for purgeable caches */
  পূর্ণসংখ্যা *pnRecyclable    /* OUT: Total number of pages available for recycling */
){
  PgHdr1 *p;
  পূর্ণসংখ্যা nRecyclable = 0;
  জন্য(p=pcache1.grp.lru.p_Lru_পরবর্তী; p && !p->isAnchor; p=p->p_Lru_পরবর্তী){
    প্রতিজ্ঞা( PAGE_IS_UNPINNED(p) );
    nRecyclable++;
  }
  *pnCurrent = pcache1.grp.nPurgeable;
  *pnMax = (পূর্ণসংখ্যা)pcache1.grp.nMaxPage;
  *pnMin = (পূর্ণসংখ্যা)pcache1.grp.nMinPage;
  *pnRecyclable = nRecyclable;
}
#endif
