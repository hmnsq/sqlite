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
#അന്തര്ഭാവ "sqliteInt.h"

തരനിർവചനം ഘടന PCache1 PCache1;
തരനിർവചനം ഘടന PgHdr1 PgHdr1;
തരനിർവചനം ഘടന PgFreeslot PgFreeslot;
തരനിർവചനം ഘടന PGroup PGroup;

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
ഘടന PgHdr1 {
  sqlite3_pcache_page page; /* Base class. Must be first. pBuf & pExtra */
  unsigned പൂർണ്ണസംഖ്യ i_താക്കോൽ;        /* Key value (page number) */
  u16 isBulkLocal;          /* This page from bulk local storage */
  u16 isAnchor;             /* This is the PGroup.lru element */
  PgHdr1 *pNext;            /* Next in hash table chain */
  PCache1 *pCache;          /* Cache that currently owns this page */
  PgHdr1 *pLruNext;         /* Next in circular LRU list of unpinned pages */
  PgHdr1 *pLruPrev;         /* Previous in LRU list of unpinned pages */
                            /* NB: pLruPrev is only valid if pLruNext!=0 */
};

/*
** A page is pinned if it is not on the LRU list.  To be "pinned" means
** that the page is in active use and must not be deallocated.
*/
#define PAGE_IS_PINNED(p)    ((p)->pLruNext==0)
#define PAGE_IS_UNPINNED(p)  ((p)->pLruNext!=0)

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
ഘടന PGroup {
  sqlite3_mutex *mutex;          /* MUTEX_STATIC_LRU or NULL */
  unsigned പൂർണ്ണസംഖ്യ nMaxPage;         /* Sum of nMax for purgeable caches */
  unsigned പൂർണ്ണസംഖ്യ nMinPage;         /* Sum of nMin for purgeable caches */
  unsigned പൂർണ്ണസംഖ്യ mxPinned;         /* nMaxpage + 10 - nMinPage */
  unsigned പൂർണ്ണസംഖ്യ nPurgeable;       /* Number of purgeable pages allocated */
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
ഘടന PCache1 {
  /* Cache configuration parameters. Page size (szPage) and the purgeable
  ** flag (bPurgeable) and the pnPurgeable pointer are all set when the
  ** cache is created and are never changed thereafter. nMax may be
  ** modified at any time by a call to the pcache1Cachesize() method.
  ** The PGroup mutex must be held when accessing nMax.
  */
  PGroup *pGroup;                     /* PGroup this cache belongs to */
  unsigned പൂർണ്ണസംഖ്യ *pnPurgeable;          /* Pointer to pGroup->nPurgeable */
  പൂർണ്ണസംഖ്യ szPage;                         /* Size of database content section */
  പൂർണ്ണസംഖ്യ szExtra;                        /* sizeof(MemPage)+sizeof(PgHdr) */
  പൂർണ്ണസംഖ്യ szAlloc;                        /* Total size of one pcache line */
  പൂർണ്ണസംഖ്യ bPurgeable;                     /* True if cache is purgeable */
  unsigned പൂർണ്ണസംഖ്യ nMin;                  /* Minimum number of pages reserved */
  unsigned പൂർണ്ണസംഖ്യ nMax;                  /* Configured "cache_size" value */
  unsigned പൂർണ്ണസംഖ്യ n90pct;                /* nMax*9/10 */
  unsigned പൂർണ്ണസംഖ്യ i_Max_താക്കോൽ;               /* Largest key seen since xTruncate() */
  unsigned പൂർണ്ണസംഖ്യ nPurgeableDummy;       /* pnPurgeable points here when not used*/

  /* Hash table of all pages. The following variables may only be accessed
  ** when the accessor is holding the PGroup mutex.
  */
  unsigned പൂർണ്ണസംഖ്യ nRecyclable;           /* Number of pages in the LRU list */
  unsigned പൂർണ്ണസംഖ്യ nPage;                 /* Total number of pages in apHash */
  unsigned പൂർണ്ണസംഖ്യ nHash;                 /* Number of slots in apHash[] */
  PgHdr1 **apHash;                    /* Hash table for fast lookup by key */
  PgHdr1 *pFree;                      /* List of unused pcache-local pages */
  ശൂന്യം *pBulk;                        /* Bulk memory used by pcache-local */
};

/*
** Free slots in the allocator used to divide up the global page cache
** buffer provided using the SQLITE_CONFIG_PAGECACHE mechanism.
*/
ഘടന PgFreeslot {
  PgFreeslot *pNext;  /* Next free slot */
};

/*
** Global data used by this cache.
*/
സ്ഥിതം SQLITE_WSD ഘടന PCacheGlobal {
  PGroup grp;                    /* The global PGroup for mode (2) */

  /* Variables related to SQLITE_CONFIG_PAGECACHE settings.  The
  ** szSlot, nSlot, pStart, pEnd, nReserve, and isInit values are all
  ** fixed at sqlite3_initialize() time and do not require mutex protection.
  ** The nFreeSlot and pFree values do require mutex protection.
  */
  പൂർണ്ണസംഖ്യ isInit;                    /* True if initialized */
  പൂർണ്ണസംഖ്യ separateCache;             /* Use a new PGroup for each PCache */
  പൂർണ്ണസംഖ്യ nInitPage;                 /* Initial bulk allocation size */
  പൂർണ്ണസംഖ്യ szSlot;                    /* Size of each free slot */
  പൂർണ്ണസംഖ്യ nSlot;                     /* The number of pcache slots */
  പൂർണ്ണസംഖ്യ nReserve;                  /* Try to keep nFreeSlot above this */
  ശൂന്യം *pStart, *pEnd;           /* Bounds of global page cache memory */
  /* Above requires no mutex.  Use mutex below for variable that follow. */
  sqlite3_mutex *mutex;          /* Mutex for accessing the following: */
  PgFreeslot *pFree;             /* Free page blocks */
  പൂർണ്ണസംഖ്യ nFreeSlot;                 /* Number of unused pcache slots */
  പൂർണ്ണസംഖ്യ bUnderPressure;            /* True if low on PAGECACHE memory */
} pcache1_g;

/*
** All code in this file should access the global structure above via the
** alias "pcache1". This ensures that the WSD emulation is used when
** compiling for systems that do not support real WSD.
*/
#define pcache1 (GLOBAL(ഘടന PCacheGlobal, pcache1_g))

/*
** Macros to enter and leave the PCache LRU mutex.
*/
#എങ്കിൽ !defined(SQLITE_ENABLE_MEMORY_MANAGEMENT) || SQLITE_THREADSAFE==0
# define pcache1EnterMutex(X)  പ്രതിജ്ഞാ((X)->mutex==0)
# define pcache1LeaveMutex(X)  പ്രതിജ്ഞാ((X)->mutex==0)
# define PCACHE1_MIGHT_USE_GROUP_MUTEX 0
#അല്ലെങ്കിൽ
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
ശൂന്യം sqlite3PCacheBufferSetup(ശൂന്യം *pBuf, പൂർണ്ണസംഖ്യ sz, പൂർണ്ണസംഖ്യ n){
  എങ്കിൽ( pcache1.isInit ){
    PgFreeslot *p;
    എങ്കിൽ( pBuf==0 ) sz = n = 0;
    എങ്കിൽ( n==0 ) sz = 0;
    sz = ROUNDDOWN8(sz);
    pcache1.szSlot = sz;
    pcache1.nSlot = pcache1.nFreeSlot = n;
    pcache1.nReserve = n>90 ? 10 : (n/10 + 1);
    pcache1.pStart = pBuf;
    pcache1.pFree = 0;
    AtomicStore(&pcache1.bUnderPressure,0);
    ആകുമ്പോൾ( n-- ){
      p = (PgFreeslot*)pBuf;
      p->pNext = pcache1.pFree;
      pcache1.pFree = p;
      pBuf = (ശൂന്യം*)&((അക്ഷരം*)pBuf)[sz];
    }
    pcache1.pEnd = pBuf;
  }
}

/*
** Try to initialize the pCache->pFree and pCache->pBulk fields.  Return
** true if pCache->pFree ends up containing one or more free pages.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ pcache1InitBulk(PCache1 *pCache){
  i64 szBulk;
  അക്ഷരം *zBulk;
  എങ്കിൽ( pcache1.nInitPage==0 ) തിരികെ 0;
  /* Do not bother with a bulk allocation if the cache size very small */
  എങ്കിൽ( pCache->nMax<3 ) തിരികെ 0;
  sqlite3BeginBenignMalloc();
  എങ്കിൽ( pcache1.nInitPage>0 ){
    szBulk = pCache->szAlloc * (i64)pcache1.nInitPage;
  }അല്ലെങ്കിൽ{
    szBulk = -1024 * (i64)pcache1.nInitPage;
  }
  എങ്കിൽ( szBulk > pCache->szAlloc*(i64)pCache->nMax ){
    szBulk = pCache->szAlloc*(i64)pCache->nMax;
  }
  എങ്കിൽ( szBulk>=pCache->szAlloc ){
    zBulk = pCache->pBulk = sqlite3Malloc( szBulk );
    sqlite3EndBenignMalloc();
    എങ്കിൽ( zBulk ){
      പൂർണ്ണസംഖ്യ nBulk = sqlite3MallocSize(zBulk)/pCache->szAlloc;
      ചെയ്യുക{
        PgHdr1 *pX = (PgHdr1*)&zBulk[pCache->szPage];
        pX->page.pBuf = zBulk;
        pX->page.pExtra = (u8*)pX + ROUND8(വലുപ്പം(*pX));
        പ്രതിജ്ഞാ( EIGHT_BYTE_ALIGNMENT( pX->page.pExtra ) );
        pX->isBulkLocal = 1;
        pX->isAnchor = 0;
        pX->pNext = pCache->pFree;
        pX->pLruPrev = 0;    /* Initializing this saves a valgrind error */
        pCache->pFree = pX;
        zBulk += pCache->szAlloc;
      }ആകുമ്പോൾ( --nBulk );
    }
  }
  തിരികെ pCache->pFree!=0;
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
സ്ഥിതം ശൂന്യം *pcache1Alloc(പൂർണ്ണസംഖ്യ nByte){
  ശൂന്യം *p = 0;
  പ്രതിജ്ഞാ( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  എങ്കിൽ( nByte<=pcache1.szSlot ){
    sqlite3_mutex_enter(pcache1.mutex);
    p = (PgHdr1 *)pcache1.pFree;
    എങ്കിൽ( p ){
      pcache1.pFree = pcache1.pFree->pNext;
      pcache1.nFreeSlot--;
      AtomicStore(&pcache1.bUnderPressure,pcache1.nFreeSlot<pcache1.nReserve);
      പ്രതിജ്ഞാ( pcache1.nFreeSlot>=0 );
      sqlite3StatusHighwater(SQLITE_STATUS_PAGECACHE_SIZE, nByte);
      sqlite3StatusUp(SQLITE_STATUS_PAGECACHE_USED, 1);
    }
    sqlite3_mutex_leave(pcache1.mutex);
  }
  എങ്കിൽ( p==0 ){
    /* Memory is not available in the SQLITE_CONFIG_PAGECACHE pool.  Get
    ** it from sqlite3Malloc instead.
    */
    p = sqlite3Malloc(nByte);
#ifndef SQLITE_DISABLE_PAGECACHE_OVERFLOW_STATS
    എങ്കിൽ( p ){
      പൂർണ്ണസംഖ്യ sz = sqlite3MallocSize(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3StatusHighwater(SQLITE_STATUS_PAGECACHE_SIZE, nByte);
      sqlite3StatusUp(SQLITE_STATUS_PAGECACHE_OVERFLOW, sz);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#endif
    sqlite3MemdebugSetType(p, MEMTYPE_PCACHE);
  }
  തിരികെ p;
}

/*
** Free an allocated buffer obtained from pcache1Alloc().
*/
സ്ഥിതം ശൂന്യം pcache1Free(ശൂന്യം *p){
  എങ്കിൽ( p==0 ) തിരികെ;
  എങ്കിൽ( SQLITE_WITHIN(p, pcache1.pStart, pcache1.pEnd) ){
    PgFreeslot *pSlot;
    sqlite3_mutex_enter(pcache1.mutex);
    sqlite3StatusDown(SQLITE_STATUS_PAGECACHE_USED, 1);
    pSlot = (PgFreeslot*)p;
    pSlot->pNext = pcache1.pFree;
    pcache1.pFree = pSlot;
    pcache1.nFreeSlot++;
    AtomicStore(&pcache1.bUnderPressure,pcache1.nFreeSlot<pcache1.nReserve);
    പ്രതിജ്ഞാ( pcache1.nFreeSlot<=pcache1.nSlot );
    sqlite3_mutex_leave(pcache1.mutex);
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
#ifndef SQLITE_DISABLE_PAGECACHE_OVERFLOW_STATS
    {
      പൂർണ്ണസംഖ്യ nFreed = 0;
      nFreed = sqlite3MallocSize(p);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ pcache1MemSize(ശൂന്യം *p){
  എങ്കിൽ( p>=pcache1.pStart && p<pcache1.pEnd ){
    തിരികെ pcache1.szSlot;
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ iSize;
    പ്രതിജ്ഞാ( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3MemdebugSetType(p, MEMTYPE_HEAP);
    iSize = sqlite3MallocSize(p);
    sqlite3MemdebugSetType(p, MEMTYPE_PCACHE);
    തിരികെ iSize;
  }
}
#endif /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

/*
** Allocate a new page object initially associated with cache pCache.
*/
സ്ഥിതം PgHdr1 *pcache1AllocPage(PCache1 *pCache, പൂർണ്ണസംഖ്യ benignMalloc){
  PgHdr1 *p = 0;
  ശൂന്യം *pPg;

  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCache->pGroup->mutex) );
  എങ്കിൽ( pCache->pFree || (pCache->nPage==0 && pcache1InitBulk(pCache)) ){
    പ്രതിജ്ഞാ( pCache->pFree!=0 );
    p = pCache->pFree;
    pCache->pFree = p->pNext;
    p->pNext = 0;
  }അല്ലെങ്കിൽ{
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    /* The group mutex must be released before pcache1Alloc() is called. This
    ** is because it might call sqlite3_release_memory(), which assumes that
    ** this mutex is not held. */
    പ്രതിജ്ഞാ( pcache1.separateCache==0 );
    പ്രതിജ്ഞാ( pCache->pGroup==&pcache1.grp );
    pcache1LeaveMutex(pCache->pGroup);
#endif
    എങ്കിൽ( benignMalloc ){ sqlite3BeginBenignMalloc(); }
    pPg = pcache1Alloc(pCache->szAlloc);
    എങ്കിൽ( benignMalloc ){ sqlite3EndBenignMalloc(); }
#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
    pcache1EnterMutex(pCache->pGroup);
#endif
    എങ്കിൽ( pPg==0 ) തിരികെ 0;
    p = (PgHdr1 *)&((u8 *)pPg)[pCache->szPage];
    p->page.pBuf = pPg;
    p->page.pExtra = (u8*)p + ROUND8(വലുപ്പം(*p));
    പ്രതിജ്ഞാ( EIGHT_BYTE_ALIGNMENT( p->page.pExtra ) );
    p->isBulkLocal = 0;
    p->isAnchor = 0;
    p->pLruPrev = 0;           /* Initializing this saves a valgrind error */
  }
  (*pCache->pnPurgeable)++;
  തിരികെ p;
}

/*
** Free a page object allocated by pcache1AllocPage().
*/
സ്ഥിതം ശൂന്യം pcache1FreePage(PgHdr1 *p){
  PCache1 *pCache;
  പ്രതിജ്ഞാ( p!=0 );
  pCache = p->pCache;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(p->pCache->pGroup->mutex) );
  എങ്കിൽ( p->isBulkLocal ){
    p->pNext = pCache->pFree;
    pCache->pFree = p;
  }അല്ലെങ്കിൽ{
    pcache1Free(p->page.pBuf);
  }
  (*pCache->pnPurgeable)--;
}

/*
** Malloc function used by SQLite to obtain space from the buffer configured
** using sqlite3_config(SQLITE_CONFIG_PAGECACHE) option. If no such buffer
** exists, this function falls back to sqlite3Malloc().
*/
ശൂന്യം *sqlite3PageMalloc(പൂർണ്ണസംഖ്യ sz){
  പ്രതിജ്ഞാ( sz<=65536+8 ); /* These allocations are never very large */
  തിരികെ pcache1Alloc(sz);
}

/*
** Free an allocated buffer obtained from sqlite3PageMalloc().
*/
ശൂന്യം sqlite3PageFree(ശൂന്യം *p){
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
സ്ഥിതം പൂർണ്ണസംഖ്യ pcache1UnderMemoryPressure(PCache1 *pCache){
  എങ്കിൽ( pcache1.nSlot && (pCache->szPage+pCache->szExtra)<=pcache1.szSlot ){
    തിരികെ AtomicLoad(&pcache1.bUnderPressure);
  }അല്ലെങ്കിൽ{
    തിരികെ sqlite3HeapNearlyFull();
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
സ്ഥിതം ശൂന്യം pcache1ResizeHash(PCache1 *p){
  PgHdr1 **apNew;
  u64 nNew;
  u32 i;

  പ്രതിജ്ഞാ( sqlite3_mutex_held(p->pGroup->mutex) );

  nNew = 2*(u64)p->nHash;
  എങ്കിൽ( nNew<256 ){
    nNew = 256;
  }
  പ്രതിജ്ഞാ( (nNew & (nNew-1))==0 );   /* nNew is always a power of two */

  pcache1LeaveMutex(p->pGroup);
  എങ്കിൽ( p->nHash ){ sqlite3BeginBenignMalloc(); }
  apNew = (PgHdr1 **)sqlite3MallocZero(വലുപ്പം(PgHdr1 *)*nNew);
  എങ്കിൽ( p->nHash ){ sqlite3EndBenignMalloc(); }
  pcache1EnterMutex(p->pGroup);
  എങ്കിൽ( apNew ){
    വേണ്ടി(i=0; i<p->nHash; i++){
      PgHdr1 *pPage;
      PgHdr1 *pNext = p->apHash[i];
      ആകുമ്പോൾ( (pPage = pNext)!=0 ){
        unsigned പൂർണ്ണസംഖ്യ h = (unsigned പൂർണ്ണസംഖ്യ)(pPage->i_താക്കോൽ & (nNew-1));
        pNext = pPage->pNext;
        pPage->pNext = apNew[h];
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
സ്ഥിതം PgHdr1 *pcache1PinPage(PgHdr1 *pPage){
  പ്രതിജ്ഞാ( pPage!=0 );
  പ്രതിജ്ഞാ( PAGE_IS_UNPINNED(pPage) );
  പ്രതിജ്ഞാ( pPage->pLruNext );
  പ്രതിജ്ഞാ( pPage->pLruPrev );
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pPage->pCache->pGroup->mutex) );
  pPage->pLruPrev->pLruNext = pPage->pLruNext;
  pPage->pLruNext->pLruPrev = pPage->pLruPrev;
  pPage->pLruNext = 0;
  /* pPage->pLruPrev = 0;
  ** No need to clear pLruPrev as it is never accessed if pLruNext is 0 */
  പ്രതിജ്ഞാ( pPage->isAnchor==0 );
  പ്രതിജ്ഞാ( pPage->pCache->pGroup->lru.isAnchor==1 );
  pPage->pCache->nRecyclable--;
  തിരികെ pPage;
}


/*
** Remove the page supplied as an argument from the hash table
** (PCache1.apHash structure) that it is currently stored in.
** Also free the page if freePage is true.
**
** The PGroup mutex must be held when this function is called.
*/
സ്ഥിതം ശൂന്യം pcache1RemoveFromHash(PgHdr1 *pPage, പൂർണ്ണസംഖ്യ freeFlag){
  unsigned പൂർണ്ണസംഖ്യ h;
  PCache1 *pCache = pPage->pCache;
  PgHdr1 **pp;

  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCache->pGroup->mutex) );
  പ്രതിജ്ഞാ( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  h = pPage->i_താക്കോൽ & (pCache->nHash-1);
  വേണ്ടി(pp=&pCache->apHash[h]; (*pp)!=pPage; pp=&(*pp)->pNext);
  *pp = (*pp)->pNext;

  pCache->nPage--;
  എങ്കിൽ( freeFlag ) pcache1FreePage(pPage);
}

/*
** If there are currently more than nMaxPage pages allocated, try
** to recycle pages to reduce the number allocated to nMaxPage.
*/
സ്ഥിതം ശൂന്യം pcache1EnforceMaxPage(PCache1 *pCache){
  PGroup *pGroup = pCache->pGroup;
  PgHdr1 *p;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pGroup->mutex) );
  ആകുമ്പോൾ( pGroup->nPurgeable>pGroup->nMaxPage
      && (p=pGroup->lru.pLruPrev)->isAnchor==0
  ){
    പ്രതിജ്ഞാ( p->pCache->pGroup==pGroup );
    പ്രതിജ്ഞാ( PAGE_IS_UNPINNED(p) );
    pcache1PinPage(p);
    pcache1RemoveFromHash(p, 1);
  }
  എങ്കിൽ( pCache->nPage==0 && pCache->pBulk ){
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
സ്ഥിതം ശൂന്യം pcache1TruncateUnsafe(
  PCache1 *pCache,             /* The cache to truncate */
  unsigned പൂർണ്ണസംഖ്യ iLimit          /* Drop pages with this pgno or larger */
){
  TESTONLY( പൂർണ്ണസംഖ്യ nPage = 0; )  /* To assert pCache->nPage is correct */
  unsigned പൂർണ്ണസംഖ്യ h, iStop;
  പ്രതിജ്ഞാ( sqlite3_mutex_held(pCache->pGroup->mutex) );
  പ്രതിജ്ഞാ( pCache->i_Max_താക്കോൽ >= iLimit );
  പ്രതിജ്ഞാ( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  എങ്കിൽ( pCache->i_Max_താക്കോൽ - iLimit < pCache->nHash ){
    /* If we are just shaving the last few pages off the end of the
    ** cache, then there is no point in scanning the entire hash table.
    ** Only scan those hash slots that might contain pages that need to
    ** be removed. */
    h = iLimit & (pCache->nHash-1);
    iStop = pCache->i_Max_താക്കോൽ & (pCache->nHash-1);
    TESTONLY( nPage = -10; )  /* Disable the pCache->nPage validity check */
  }അല്ലെങ്കിൽ{
    /* This is the general case where many pages are being removed.
    ** It is necessary to scan the entire hash table */
    h = pCache->nHash/2;
    iStop = h - 1;
  }
  വേണ്ടി(;;){
    PgHdr1 **pp;
    PgHdr1 *pPage;
    പ്രതിജ്ഞാ( h<pCache->nHash );
    pp = &pCache->apHash[h];
    ആകുമ്പോൾ( (pPage = *pp)!=0 ){
      എങ്കിൽ( pPage->i_താക്കോൽ>=iLimit ){
        pCache->nPage--;
        *pp = pPage->pNext;
        എങ്കിൽ( PAGE_IS_UNPINNED(pPage) ) pcache1PinPage(pPage);
        pcache1FreePage(pPage);
      }അല്ലെങ്കിൽ{
        pp = &pPage->pNext;
        TESTONLY( എങ്കിൽ( nPage>=0 ) nPage++; )
      }
    }
    എങ്കിൽ( h==iStop ) നിർത്തുക;
    h = (h+1) & (pCache->nHash-1);
  }
  പ്രതിജ്ഞാ( nPage<0 || pCache->nPage==(unsigned)nPage );
}

/******************************************************************************/
/******** sqlite3_pcache Methods **********************************************/

/*
** Implementation of the sqlite3_pcache.xInit method.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ pcache1Init(ശൂന്യം *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  പ്രതിജ്ഞാ( pcache1.isInit==0 );
  memset(&pcache1, 0, വലുപ്പം(pcache1));


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
#എങ്കിൽ defined(SQLITE_ENABLE_MEMORY_MANAGEMENT)
  pcache1.separateCache = 0;
#അല്ലെങ്കിലെങ്കിൽ SQLITE_THREADSAFE
  pcache1.separateCache = sqlite3GlobalConfig.pPage==0
                          || sqlite3GlobalConfig.bCoreMutex>0;
#അല്ലെങ്കിൽ
  pcache1.separateCache = sqlite3GlobalConfig.pPage==0;
#endif

#എങ്കിൽ SQLITE_THREADSAFE
  എങ്കിൽ( sqlite3GlobalConfig.bCoreMutex ){
    pcache1.grp.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_LRU);
    pcache1.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PMEM);
  }
#endif
  എങ്കിൽ( pcache1.separateCache
   && sqlite3GlobalConfig.nPage!=0
   && sqlite3GlobalConfig.pPage==0
  ){
    pcache1.nInitPage = sqlite3GlobalConfig.nPage;
  }അല്ലെങ്കിൽ{
    pcache1.nInitPage = 0;
  }
  pcache1.grp.mxPinned = 10;
  pcache1.isInit = 1;
  തിരികെ SQLITE_OK;
}

/*
** Implementation of the sqlite3_pcache.xShutdown method.
** Note that the static mutex allocated in xInit does
** not need to be freed.
*/
സ്ഥിതം ശൂന്യം pcache1Shutdown(ശൂന്യം *NotUsed){
  UNUSED_PARAMETER(NotUsed);
  പ്രതിജ്ഞാ( pcache1.isInit!=0 );
  memset(&pcache1, 0, വലുപ്പം(pcache1));
}

/* forward declaration */
സ്ഥിതം ശൂന്യം pcache1Destroy(sqlite3_pcache *p);

/*
** Implementation of the sqlite3_pcache.xCreate method.
**
** Allocate a new cache.
*/
സ്ഥിതം sqlite3_pcache *pcache1Create(പൂർണ്ണസംഖ്യ szPage, പൂർണ്ണസംഖ്യ szExtra, പൂർണ്ണസംഖ്യ bPurgeable){
  PCache1 *pCache;      /* The newly created page cache */
  PGroup *pGroup;       /* The group the new page cache will belong to */
  i64 sz;               /* Bytes of memory required to allocate the new cache */

  പ്രതിജ്ഞാ( (szPage & (szPage-1))==0 && szPage>=512 && szPage<=65536 );
  പ്രതിജ്ഞാ( szExtra < 300 );

  sz = വലുപ്പം(PCache1) + വലുപ്പം(PGroup)*pcache1.separateCache;
  pCache = (PCache1 *)sqlite3MallocZero(sz);
  എങ്കിൽ( pCache ){
    എങ്കിൽ( pcache1.separateCache ){
      pGroup = (PGroup*)&pCache[1];
      pGroup->mxPinned = 10;
    }അല്ലെങ്കിൽ{
      pGroup = &pcache1.grp;
    }
    pcache1EnterMutex(pGroup);
    എങ്കിൽ( pGroup->lru.isAnchor==0 ){
      pGroup->lru.isAnchor = 1;
      pGroup->lru.pLruPrev = pGroup->lru.pLruNext = &pGroup->lru;
    }
    pCache->pGroup = pGroup;
    pCache->szPage = szPage;
    pCache->szExtra = szExtra;
    pCache->szAlloc = szPage + szExtra + ROUND8(വലുപ്പം(PgHdr1));
    pCache->bPurgeable = (bPurgeable ? 1 : 0);
    pcache1ResizeHash(pCache);
    എങ്കിൽ( bPurgeable ){
      pCache->nMin = 10;
      pGroup->nMinPage += pCache->nMin;
      pGroup->mxPinned = pGroup->nMaxPage + 10 - pGroup->nMinPage;
      pCache->pnPurgeable = &pGroup->nPurgeable;
    }അല്ലെങ്കിൽ{
      pCache->pnPurgeable = &pCache->nPurgeableDummy;
    }
    pcache1LeaveMutex(pGroup);
    എങ്കിൽ( pCache->nHash==0 ){
      pcache1Destroy((sqlite3_pcache*)pCache);
      pCache = 0;
    }
  }
  തിരികെ (sqlite3_pcache *)pCache;
}

/*
** Implementation of the sqlite3_pcache.xCachesize method.
**
** Configure the cache_size limit for a cache.
*/
സ്ഥിതം ശൂന്യം pcache1Cachesize(sqlite3_pcache *p, പൂർണ്ണസംഖ്യ nMax){
  PCache1 *pCache = (PCache1 *)p;
  u32 n;
  പ്രതിജ്ഞാ( nMax>=0 );
  എങ്കിൽ( pCache->bPurgeable ){
    PGroup *pGroup = pCache->pGroup;
    pcache1EnterMutex(pGroup);
    n = (u32)nMax;
    എങ്കിൽ( n > 0x7fff0000 - pGroup->nMaxPage + pCache->nMax ){
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
സ്ഥിതം ശൂന്യം pcache1Shrink(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1*)p;
  എങ്കിൽ( pCache->bPurgeable ){
    PGroup *pGroup = pCache->pGroup;
    unsigned പൂർണ്ണസംഖ്യ savedMaxPage;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ pcache1Pagecount(sqlite3_pcache *p){
  പൂർണ്ണസംഖ്യ n;
  PCache1 *pCache = (PCache1*)p;
  pcache1EnterMutex(pCache->pGroup);
  n = pCache->nPage;
  pcache1LeaveMutex(pCache->pGroup);
  തിരികെ n;
}


/*
** Implement steps 3, 4, and 5 of the pcache1Fetch() algorithm described
** in the header of the pcache1Fetch() procedure.
**
** This steps are broken out into a separate procedure because they are
** usually not needed, and by avoiding the stack initialization required
** for these steps, the main pcache1Fetch() procedure can run faster.
*/
സ്ഥിതം SQLITE_NOINLINE PgHdr1 *pcache1FetchStage2(
  PCache1 *pCache,
  unsigned പൂർണ്ണസംഖ്യ i_താക്കോൽ,
  പൂർണ്ണസംഖ്യ createFlag
){
  unsigned പൂർണ്ണസംഖ്യ nPinned;
  PGroup *pGroup = pCache->pGroup;
  PgHdr1 *pPage = 0;

  /* Step 3: Abort if createFlag is 1 but the cache is nearly full */
  പ്രതിജ്ഞാ( pCache->nPage >= pCache->nRecyclable );
  nPinned = pCache->nPage - pCache->nRecyclable;
  പ്രതിജ്ഞാ( pGroup->mxPinned == pGroup->nMaxPage + 10 - pGroup->nMinPage );
  പ്രതിജ്ഞാ( pCache->n90pct == pCache->nMax*9/10 );
  എങ്കിൽ( createFlag==1 && (
        nPinned>=pGroup->mxPinned
     || nPinned>=pCache->n90pct
     || (pcache1UnderMemoryPressure(pCache) && pCache->nRecyclable<nPinned)
  )){
    തിരികെ 0;
  }

  എങ്കിൽ( pCache->nPage>=pCache->nHash ) pcache1ResizeHash(pCache);
  പ്രതിജ്ഞാ( pCache->nHash>0 && pCache->apHash );
  പ്രതിജ്ഞാ( (pCache->nHash & (pCache->nHash-1))==0 );

  /* Step 4. Try to recycle a page. */
  എങ്കിൽ( pCache->bPurgeable
   && !pGroup->lru.pLruPrev->isAnchor
   && ((pCache->nPage+1>=pCache->nMax) || pcache1UnderMemoryPressure(pCache))
  ){
    PCache1 *pOther;
    pPage = pGroup->lru.pLruPrev;
    പ്രതിജ്ഞാ( PAGE_IS_UNPINNED(pPage) );
    pcache1RemoveFromHash(pPage, 0);
    pcache1PinPage(pPage);
    pOther = pPage->pCache;
    എങ്കിൽ( pOther->szAlloc != pCache->szAlloc ){
      pcache1FreePage(pPage);
      pPage = 0;
    }അല്ലെങ്കിൽ{
      pGroup->nPurgeable -= (pOther->bPurgeable - pCache->bPurgeable);
    }
  }

  /* Step 5. If a usable page buffer has still not been found,
  ** attempt to allocate a new one.
  */
  എങ്കിൽ( !pPage ){
    pPage = pcache1AllocPage(pCache, createFlag==1);
  }

  എങ്കിൽ( pPage ){
    unsigned പൂർണ്ണസംഖ്യ h = i_താക്കോൽ & (pCache->nHash-1);
    pCache->nPage++;
    pPage->i_താക്കോൽ = i_താക്കോൽ;
    pPage->pNext = pCache->apHash[h];
    pPage->pCache = pCache;
    pPage->pLruNext = 0;
    /* pPage->pLruPrev = 0;
    ** No need to clear pLruPrev since it is not accessed when pLruNext==0 */
    *(ശൂന്യം **)pPage->page.pExtra = 0;
    pCache->apHash[h] = pPage;
    എങ്കിൽ( i_താക്കോൽ>pCache->i_Max_താക്കോൽ ){
      pCache->i_Max_താക്കോൽ = i_താക്കോൽ;
    }
  }
  തിരികെ pPage;
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
സ്ഥിതം PgHdr1 *pcache1FetchNoMutex(
  sqlite3_pcache *p,
  unsigned പൂർണ്ണസംഖ്യ i_താക്കോൽ,
  പൂർണ്ണസംഖ്യ createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage = 0;

  /* Step 1: Search the hash table for an existing entry.  nHash is always
  ** a power of two when the cache is in use (see pcache1ResizeHash()), so
  ** the modulo reduces to a mask, avoiding a hardware divide. */
  പ്രതിജ്ഞാ( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  pPage = pCache->apHash[i_താക്കോൽ & (pCache->nHash-1u)];
  ആകുമ്പോൾ( pPage && pPage->i_താക്കോൽ!=i_താക്കോൽ ){ pPage = pPage->pNext; }

  /* Step 2: If the page was found in the hash table, then return it.
  ** If the page was not in the hash table and createFlag is 0, abort.
  ** Otherwise (page not in hash and createFlag!=0) continue with
  ** subsequent steps to try to create the page. */
  എങ്കിൽ( pPage ){
    എങ്കിൽ( PAGE_IS_UNPINNED(pPage) ){
      തിരികെ pcache1PinPage(pPage);
    }അല്ലെങ്കിൽ{
      തിരികെ pPage;
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( createFlag ){
    /* Steps 3, 4, and 5 implemented by this subroutine */
    തിരികെ pcache1FetchStage2(pCache, i_താക്കോൽ, createFlag);
  }അല്ലെങ്കിൽ{
    തിരികെ 0;
  }
}
#എങ്കിൽ PCACHE1_MIGHT_USE_GROUP_MUTEX
സ്ഥിതം PgHdr1 *pcache1FetchWithMutex(
  sqlite3_pcache *p,
  unsigned പൂർണ്ണസംഖ്യ i_താക്കോൽ,
  പൂർണ്ണസംഖ്യ createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage;

  pcache1EnterMutex(pCache->pGroup);
  pPage = pcache1FetchNoMutex(p, i_താക്കോൽ, createFlag);
  പ്രതിജ്ഞാ( pPage==0 || pCache->i_Max_താക്കോൽ>=i_താക്കോൽ );
  pcache1LeaveMutex(pCache->pGroup);
  തിരികെ pPage;
}
#endif
സ്ഥിതം sqlite3_pcache_page *pcache1Fetch(
  sqlite3_pcache *p,
  unsigned പൂർണ്ണസംഖ്യ i_താക്കോൽ,
  പൂർണ്ണസംഖ്യ createFlag
){
#എങ്കിൽ PCACHE1_MIGHT_USE_GROUP_MUTEX || defined(SQLITE_DEBUG)
  PCache1 *pCache = (PCache1 *)p;
#endif

  പ്രതിജ്ഞാ( offsetof(PgHdr1,page)==0 );
  പ്രതിജ്ഞാ( pCache->bPurgeable || createFlag!=1 );
  പ്രതിജ്ഞാ( pCache->bPurgeable || pCache->nMin==0 );
  പ്രതിജ്ഞാ( pCache->bPurgeable==0 || pCache->nMin==10 );
  പ്രതിജ്ഞാ( pCache->nMin==0 || pCache->bPurgeable );
  പ്രതിജ്ഞാ( pCache->nHash>0 );
#എങ്കിൽ PCACHE1_MIGHT_USE_GROUP_MUTEX
  എങ്കിൽ( pCache->pGroup->mutex ){
    തിരികെ (sqlite3_pcache_page*)pcache1FetchWithMutex(p, i_താക്കോൽ, createFlag);
  }അല്ലെങ്കിൽ
#endif
  {
    തിരികെ (sqlite3_pcache_page*)pcache1FetchNoMutex(p, i_താക്കോൽ, createFlag);
  }
}


/*
** Implementation of the sqlite3_pcache.xUnpin method.
**
** Mark a page as unpinned (eligible for asynchronous recycling).
*/
സ്ഥിതം ശൂന്യം pcache1Unpin(
  sqlite3_pcache *p,
  sqlite3_pcache_page *pPg,
  പൂർണ്ണസംഖ്യ reuseUnlikely
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage = (PgHdr1 *)pPg;
  PGroup *pGroup = pCache->pGroup;

  പ്രതിജ്ഞാ( pPage->pCache==pCache );
  pcache1EnterMutex(pGroup);

  /* It is an error to call this function if the page is already
  ** part of the PGroup LRU list.
  */
  പ്രതിജ്ഞാ( pPage->pLruNext==0 );
  പ്രതിജ്ഞാ( PAGE_IS_PINNED(pPage) );

  എങ്കിൽ( reuseUnlikely || pGroup->nPurgeable>pGroup->nMaxPage ){
    pcache1RemoveFromHash(pPage, 1);
  }അല്ലെങ്കിൽ{
    /* Add the page to the PGroup LRU list. */
    PgHdr1 **ppFirst = &pGroup->lru.pLruNext;
    pPage->pLruPrev = &pGroup->lru;
    (pPage->pLruNext = *ppFirst)->pLruPrev = pPage;
    *ppFirst = pPage;
    pCache->nRecyclable++;
  }

  pcache1LeaveMutex(pCache->pGroup);
}

/*
** Implementation of the sqlite3_pcache.xRekey method.
*/
സ്ഥിതം ശൂന്യം pcache1Rekey(
  sqlite3_pcache *p,
  sqlite3_pcache_page *pPg,
  unsigned പൂർണ്ണസംഖ്യ iOld,
  unsigned പൂർണ്ണസംഖ്യ iNew
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *pPage = (PgHdr1 *)pPg;
  PgHdr1 **pp;
  unsigned പൂർണ്ണസംഖ്യ hOld, hNew;
  പ്രതിജ്ഞാ( pPage->i_താക്കോൽ==iOld );
  പ്രതിജ്ഞാ( pPage->pCache==pCache );
  പ്രതിജ്ഞാ( iOld!=iNew );               /* The page number really is changing */

  pcache1EnterMutex(pCache->pGroup);

  പ്രതിജ്ഞാ( pcache1FetchNoMutex(p, iOld, 0)==pPage ); /* pPg really is iOld */
  പ്രതിജ്ഞാ( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  hOld = iOld & (pCache->nHash-1);
  pp = &pCache->apHash[hOld];
  ആകുമ്പോൾ( (*pp)!=pPage ){
    pp = &(*pp)->pNext;
  }
  *pp = pPage->pNext;

  പ്രതിജ്ഞാ( pcache1FetchNoMutex(p, iNew, 0)==0 ); /* iNew not in cache */
  hNew = iNew & (pCache->nHash-1);
  pPage->i_താക്കോൽ = iNew;
  pPage->pNext = pCache->apHash[hNew];
  pCache->apHash[hNew] = pPage;
  എങ്കിൽ( iNew>pCache->i_Max_താക്കോൽ ){
    pCache->i_Max_താക്കോൽ = iNew;
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
സ്ഥിതം ശൂന്യം pcache1Truncate(sqlite3_pcache *p, unsigned പൂർണ്ണസംഖ്യ iLimit){
  PCache1 *pCache = (PCache1 *)p;
  pcache1EnterMutex(pCache->pGroup);
  എങ്കിൽ( iLimit<=pCache->i_Max_താക്കോൽ ){
    pcache1TruncateUnsafe(pCache, iLimit);
    pCache->i_Max_താക്കോൽ = iLimit-1;
  }
  pcache1LeaveMutex(pCache->pGroup);
}

/*
** Implementation of the sqlite3_pcache.xDestroy method.
**
** Destroy a cache allocated using pcache1Create().
*/
സ്ഥിതം ശൂന്യം pcache1Destroy(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1 *)p;
  PGroup *pGroup = pCache->pGroup;
  പ്രതിജ്ഞാ( pCache->bPurgeable || (pCache->nMax==0 && pCache->nMin==0) );
  pcache1EnterMutex(pGroup);
  എങ്കിൽ( pCache->nPage ) pcache1TruncateUnsafe(pCache, 0);
  പ്രതിജ്ഞാ( pGroup->nMaxPage >= pCache->nMax );
  pGroup->nMaxPage -= pCache->nMax;
  പ്രതിജ്ഞാ( pGroup->nMinPage >= pCache->nMin );
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
ശൂന്യം sqlite3PCacheSetDefault(ശൂന്യം){
  സ്ഥിതം സ്ഥിരം sqlite3_pcache_methods2 defaultMethods = {
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
പൂർണ്ണസംഖ്യ sqlite3HeaderSizePcache1(ശൂന്യം){ തിരികെ ROUND8(വലുപ്പം(PgHdr1)); }

/*
** Return the global mutex used by this PCACHE implementation.  The
** sqlite3_status() routine needs access to this mutex.
*/
sqlite3_mutex *sqlite3Pcache1Mutex(ശൂന്യം){
  തിരികെ pcache1.mutex;
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
പൂർണ്ണസംഖ്യ sqlite3PcacheReleaseMemory(പൂർണ്ണസംഖ്യ nReq){
  പൂർണ്ണസംഖ്യ nFree = 0;
  പ്രതിജ്ഞാ( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  പ്രതിജ്ഞാ( sqlite3_mutex_notheld(pcache1.mutex) );
  എങ്കിൽ( sqlite3GlobalConfig.pPage==0 ){
    PgHdr1 *p;
    pcache1EnterMutex(&pcache1.grp);
    ആകുമ്പോൾ( (nReq<0 || nFree<nReq)
       &&  (p=pcache1.grp.lru.pLruPrev)!=0
       &&  p->isAnchor==0
    ){
      nFree += pcache1MemSize(p->page.pBuf);
      പ്രതിജ്ഞാ( PAGE_IS_UNPINNED(p) );
      pcache1PinPage(p);
      pcache1RemoveFromHash(p, 1);
    }
    pcache1LeaveMutex(&pcache1.grp);
  }
  തിരികെ nFree;
}
#endif /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

#ifdef SQLITE_TEST
/*
** This function is used by test procedures to inspect the internal state
** of the global cache.
*/
ശൂന്യം sqlite3PcacheStats(
  പൂർണ്ണസംഖ്യ *pnCurrent,      /* OUT: Total number of pages cached */
  പൂർണ്ണസംഖ്യ *pnMax,          /* OUT: Global maximum cache size */
  പൂർണ്ണസംഖ്യ *pnMin,          /* OUT: Sum of PCache1.nMin for purgeable caches */
  പൂർണ്ണസംഖ്യ *pnRecyclable    /* OUT: Total number of pages available for recycling */
){
  PgHdr1 *p;
  പൂർണ്ണസംഖ്യ nRecyclable = 0;
  വേണ്ടി(p=pcache1.grp.lru.pLruNext; p && !p->isAnchor; p=p->pLruNext){
    പ്രതിജ്ഞാ( PAGE_IS_UNPINNED(p) );
    nRecyclable++;
  }
  *pnCurrent = pcache1.grp.nPurgeable;
  *pnMax = (പൂർണ്ണസംഖ്യ)pcache1.grp.nMaxPage;
  *pnMin = (പൂർണ്ണസംഖ്യ)pcache1.grp.nMinPage;
  *pnRecyclable = nRecyclable;
}
#endif
