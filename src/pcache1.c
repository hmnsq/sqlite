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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ PCache1 PCache1;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ PgHdr1 PgHdr1;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ PgFreeslot PgFreeslot;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ PGroup PGroup;

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
ਬਣਤਰ PgHdr1 {
  sqlite3_pcache_ਪੰਨਾ ਪੰਨਾ; /* Base class. Must be first. pBuf & pExtra */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i_ਕੁੰਜੀ;        /* Key value (page number) */
  u16 isBulkLocal;          /* This page from bulk local storage */
  u16 isAnchor;             /* This is the PGroup.lru element */
  PgHdr1 *p_ਅਗਲਾ;            /* Next in hash table chain */
  PCache1 *pCache;          /* Cache that currently owns this page */
  PgHdr1 *p_Lru_ਅਗਲਾ;         /* Next in circular LRU list of unpinned pages */
  PgHdr1 *pLruPrev;         /* Previous in LRU list of unpinned pages */
                            /* NB: pLruPrev is only valid if pLruNext!=0 */
};

/*
** A page is pinned if it is not on the LRU list.  To be "pinned" means
** that the page is in active use and must not be deallocated.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੰਨਾ_IS_PINNED(p)    ((p)->p_Lru_ਅਗਲਾ==0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪੰਨਾ_IS_UNPINNED(p)  ((p)->p_Lru_ਅਗਲਾ!=0)

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
ਬਣਤਰ PGroup {
  sqlite3_mutex *mutex;          /* MUTEX_STATIC_LRU or NULL */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n_Max_ਪੰਨਾ;         /* Sum of nMax for purgeable caches */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n_Min_ਪੰਨਾ;         /* Sum of nMin for purgeable caches */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ mxPinned;         /* nMaxpage + 10 - nMinPage */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nPurgeable;       /* Number of purgeable pages allocated */
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
ਬਣਤਰ PCache1 {
  /* Cache configuration parameters. Page size (szPage) and the purgeable
  ** flag (bPurgeable) and the pnPurgeable pointer are all set when the
  ** cache is created and are never changed thereafter. nMax may be
  ** modified at any time by a call to the pcache1Cachesize() method.
  ** The PGroup mutex must be held when accessing nMax.
  */
  PGroup *p_ਸਮੂਹ;                     /* PGroup this cache belongs to */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *pnPurgeable;          /* Pointer to pGroup->nPurgeable */
  ਪੂਰਨ sz_ਪੰਨਾ;                         /* Size of database content section */
  ਪੂਰਨ szExtra;                        /* sizeof(MemPage)+sizeof(PgHdr) */
  ਪੂਰਨ szAlloc;                        /* Total size of one pcache line */
  ਪੂਰਨ bPurgeable;                     /* True if cache is purgeable */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nMin;                  /* Minimum number of pages reserved */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nMax;                  /* Configured "cache_size" value */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n90pct;                /* nMax*9/10 */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i_Max_ਕੁੰਜੀ;               /* Largest key seen since xTruncate() */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nPurgeableDummy;       /* pnPurgeable points here when not used*/

  /* Hash table of all pages. The following variables may only be accessed
  ** when the accessor is holding the PGroup mutex.
  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nRecyclable;           /* Number of pages in the LRU list */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n_ਪੰਨਾ;                 /* Total number of pages in apHash */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nHash;                 /* Number of slots in apHash[] */
  PgHdr1 **apHash;                    /* Hash table for fast lookup by key */
  PgHdr1 *pFree;                      /* List of unused pcache-local pages */
  ਖਾਲੀ *pBulk;                        /* Bulk memory used by pcache-local */
};

/*
** Free slots in the allocator used to divide up the global page cache
** buffer provided using the SQLITE_CONFIG_PAGECACHE mechanism.
*/
ਬਣਤਰ PgFreeslot {
  PgFreeslot *p_ਅਗਲਾ;  /* Next free slot */
};

/*
** Global data used by this cache.
*/
ਸਥਿਰ SQLITE_WSD ਬਣਤਰ PCacheGlobal {
  PGroup grp;                    /* The global PGroup for mode (2) */

  /* Variables related to SQLITE_CONFIG_PAGECACHE settings.  The
  ** szSlot, nSlot, pStart, pEnd, nReserve, and isInit values are all
  ** fixed at sqlite3_initialize() time and do not require mutex protection.
  ** The nFreeSlot and pFree values do require mutex protection.
  */
  ਪੂਰਨ isInit;                    /* True if initialized */
  ਪੂਰਨ separateCache;             /* Use a new PGroup for each PCache */
  ਪੂਰਨ n_Init_ਪੰਨਾ;                 /* Initial bulk allocation size */
  ਪੂਰਨ sz_ਥਾਂ;                    /* Size of each free slot */
  ਪੂਰਨ n_ਥਾਂ;                     /* The number of pcache slots */
  ਪੂਰਨ nReserve;                  /* Try to keep nFreeSlot above this */
  ਖਾਲੀ *p_ਸ਼ੁਰੂ, *p_ਸਿਰਾ;           /* Bounds of global page cache memory */
  /* Above requires no mutex.  Use mutex below for variable that follow. */
  sqlite3_mutex *mutex;          /* Mutex for accessing the following: */
  PgFreeslot *pFree;             /* Free page blocks */
  ਪੂਰਨ n_Free_ਥਾਂ;                 /* Number of unused pcache slots */
  ਪੂਰਨ bUnderPressure;            /* True if low on PAGECACHE memory */
} pcache1_g;

/*
** All code in this file should access the global structure above via the
** alias "pcache1". This ensures that the WSD emulation is used when
** compiling for systems that do not support real WSD.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ pcache1 (GLOBAL(ਬਣਤਰ PCacheGlobal, pcache1_g))

/*
** Macros to enter and leave the PCache LRU mutex.
*/
#ਜੇ !defined(SQLITE_ENABLE_ਸਮ੍ਰਿਤੀ_MANAGEMENT) || SQLITE_THREADSAFE==0
# define pcache1EnterMutex(X)  ਪ੍ਰਤਿਜ੍ਞਾ((X)->mutex==0)
# define pcache1LeaveMutex(X)  ਪ੍ਰਤਿਜ੍ਞਾ((X)->mutex==0)
# define PCACHE1_MIGHT_USE_ਸਮੂਹ_MUTEX 0
#ਨਹੀਂਤਾਂ
# define pcache1EnterMutex(X) sqlite3_mutex_enter((X)->mutex)
# define pcache1LeaveMutex(X) sqlite3_mutex_leave((X)->mutex)
# define PCACHE1_MIGHT_USE_ਸਮੂਹ_MUTEX 1
#ਜੇ_ਅੰਤ

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
ਖਾਲੀ sqlite3_PCache_ਬਫਰ_ਤਿਆਰੀ(ਖਾਲੀ *pBuf, ਪੂਰਨ sz, ਪੂਰਨ n){
  ਜੇ( pcache1.isInit ){
    PgFreeslot *p;
    ਜੇ( pBuf==0 ) sz = n = 0;
    ਜੇ( n==0 ) sz = 0;
    sz = ROUNDDOWN8(sz);
    pcache1.sz_ਥਾਂ = sz;
    pcache1.n_ਥਾਂ = pcache1.n_Free_ਥਾਂ = n;
    pcache1.nReserve = n>90 ? 10 : (n/10 + 1);
    pcache1.p_ਸ਼ੁਰੂ = pBuf;
    pcache1.pFree = 0;
    AtomicStore(&pcache1.bUnderPressure,0);
    ਜਦੋਂ( n-- ){
      p = (PgFreeslot*)pBuf;
      p->p_ਅਗਲਾ = pcache1.pFree;
      pcache1.pFree = p;
      pBuf = (ਖਾਲੀ*)&((ਅੱਖਰ*)pBuf)[sz];
    }
    pcache1.p_ਸਿਰਾ = pBuf;
  }
}

/*
** Try to initialize the pCache->pFree and pCache->pBulk fields.  Return
** true if pCache->pFree ends up containing one or more free pages.
*/
ਸਥਿਰ ਪੂਰਨ pcache1InitBulk(PCache1 *pCache){
  i64 szBulk;
  ਅੱਖਰ *zBulk;
  ਜੇ( pcache1.n_Init_ਪੰਨਾ==0 ) ਵਾਪਸ 0;
  /* Do not bother with a bulk allocation if the cache size very small */
  ਜੇ( pCache->nMax<3 ) ਵਾਪਸ 0;
  sqlite3BeginBenignMalloc();
  ਜੇ( pcache1.n_Init_ਪੰਨਾ>0 ){
    szBulk = pCache->szAlloc * (i64)pcache1.n_Init_ਪੰਨਾ;
  }ਨਹੀਂਤਾਂ{
    szBulk = -1024 * (i64)pcache1.n_Init_ਪੰਨਾ;
  }
  ਜੇ( szBulk > pCache->szAlloc*(i64)pCache->nMax ){
    szBulk = pCache->szAlloc*(i64)pCache->nMax;
  }
  ਜੇ( szBulk>=pCache->szAlloc ){
    zBulk = pCache->pBulk = sqlite3Malloc( szBulk );
    sqlite3_ਸਿਰਾ_Benign_Malloc();
    ਜੇ( zBulk ){
      ਪੂਰਨ nBulk = sqlite3_Malloc_ਮਾਪ(zBulk)/pCache->szAlloc;
      ਕਰੋ{
        PgHdr1 *pX = (PgHdr1*)&zBulk[pCache->sz_ਪੰਨਾ];
        pX->ਪੰਨਾ.pBuf = zBulk;
        pX->ਪੰਨਾ.pExtra = (u8*)pX + ROUND8(ਆਕਾਰ(*pX));
        ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT( pX->ਪੰਨਾ.pExtra ) );
        pX->isBulkLocal = 1;
        pX->isAnchor = 0;
        pX->p_ਅਗਲਾ = pCache->pFree;
        pX->pLruPrev = 0;    /* Initializing this saves a valgrind error */
        pCache->pFree = pX;
        zBulk += pCache->szAlloc;
      }ਜਦੋਂ( --nBulk );
    }
  }
  ਵਾਪਸ pCache->pFree!=0;
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
ਸਥਿਰ ਖਾਲੀ *pcache1Alloc(ਪੂਰਨ nByte){
  ਖਾਲੀ *p = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  ਜੇ( nByte<=pcache1.sz_ਥਾਂ ){
    sqlite3_mutex_enter(pcache1.mutex);
    p = (PgHdr1 *)pcache1.pFree;
    ਜੇ( p ){
      pcache1.pFree = pcache1.pFree->p_ਅਗਲਾ;
      pcache1.n_Free_ਥਾਂ--;
      AtomicStore(&pcache1.bUnderPressure,pcache1.n_Free_ਥਾਂ<pcache1.nReserve);
      ਪ੍ਰਤਿਜ੍ਞਾ( pcache1.n_Free_ਥਾਂ>=0 );
      sqlite3_ਸਥਿਤੀ_Highwater(SQLITE_ਸਥਿਤੀ_PAGECACHE_ਮਾਪ, nByte);
      sqlite3_ਸਥਿਤੀ_ਉੱਪਰ(SQLITE_ਸਥਿਤੀ_PAGECACHE_ਵਰਤਿਆ, 1);
    }
    sqlite3_mutex_leave(pcache1.mutex);
  }
  ਜੇ( p==0 ){
    /* Memory is not available in the SQLITE_CONFIG_PAGECACHE pool.  Get
    ** it from sqlite3Malloc instead.
    */
    p = sqlite3Malloc(nByte);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DISABLE_PAGECACHE_ਉਛਾਲ_STATS
    ਜੇ( p ){
      ਪੂਰਨ sz = sqlite3_Malloc_ਮਾਪ(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3_ਸਥਿਤੀ_Highwater(SQLITE_ਸਥਿਤੀ_PAGECACHE_ਮਾਪ, nByte);
      sqlite3_ਸਥਿਤੀ_ਉੱਪਰ(SQLITE_ਸਥਿਤੀ_PAGECACHE_ਉਛਾਲ, sz);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#ਜੇ_ਅੰਤ
    sqlite3_Memdebug_ਸੈੱਟ_Type(p, MEMTYPE_PCACHE);
  }
  ਵਾਪਸ p;
}

/*
** Free an allocated buffer obtained from pcache1Alloc().
*/
ਸਥਿਰ ਖਾਲੀ pcache1Free(ਖਾਲੀ *p){
  ਜੇ( p==0 ) ਵਾਪਸ;
  ਜੇ( SQLITE_WITHIN(p, pcache1.p_ਸ਼ੁਰੂ, pcache1.p_ਸਿਰਾ) ){
    PgFreeslot *p_ਥਾਂ;
    sqlite3_mutex_enter(pcache1.mutex);
    sqlite3_ਸਥਿਤੀ_ਥੱਲੇ(SQLITE_ਸਥਿਤੀ_PAGECACHE_ਵਰਤਿਆ, 1);
    p_ਥਾਂ = (PgFreeslot*)p;
    p_ਥਾਂ->p_ਅਗਲਾ = pcache1.pFree;
    pcache1.pFree = p_ਥਾਂ;
    pcache1.n_Free_ਥਾਂ++;
    AtomicStore(&pcache1.bUnderPressure,pcache1.n_Free_ਥਾਂ<pcache1.nReserve);
    ਪ੍ਰਤਿਜ੍ਞਾ( pcache1.n_Free_ਥਾਂ<=pcache1.n_ਥਾਂ );
    sqlite3_mutex_leave(pcache1.mutex);
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3_Memdebug_ਸੈੱਟ_Type(p, MEMTYPE_ਢੇਰ);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DISABLE_PAGECACHE_ਉਛਾਲ_STATS
    {
      ਪੂਰਨ nFreed = 0;
      nFreed = sqlite3_Malloc_ਮਾਪ(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3_ਸਥਿਤੀ_ਥੱਲੇ(SQLITE_ਸਥਿਤੀ_PAGECACHE_ਉਛਾਲ, nFreed);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#ਜੇ_ਅੰਤ
    sqlite3_free(p);
  }
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਸਮ੍ਰਿਤੀ_MANAGEMENT
/*
** Return the size of a pcache allocation
*/
ਸਥਿਰ ਪੂਰਨ pcache1_Mem_ਮਾਪ(ਖਾਲੀ *p){
  ਜੇ( p>=pcache1.p_ਸ਼ੁਰੂ && p<pcache1.p_ਸਿਰਾ ){
    ਵਾਪਸ pcache1.sz_ਥਾਂ;
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ i_ਮਾਪ;
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3_Memdebug_ਸੈੱਟ_Type(p, MEMTYPE_ਢੇਰ);
    i_ਮਾਪ = sqlite3_Malloc_ਮਾਪ(p);
    sqlite3_Memdebug_ਸੈੱਟ_Type(p, MEMTYPE_PCACHE);
    ਵਾਪਸ i_ਮਾਪ;
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

/*
** Allocate a new page object initially associated with cache pCache.
*/
ਸਥਿਰ PgHdr1 *pcache1_Alloc_ਪੰਨਾ(PCache1 *pCache, ਪੂਰਨ benignMalloc){
  PgHdr1 *p = 0;
  ਖਾਲੀ *pPg;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCache->p_ਸਮੂਹ->mutex) );
  ਜੇ( pCache->pFree || (pCache->n_ਪੰਨਾ==0 && pcache1InitBulk(pCache)) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pCache->pFree!=0 );
    p = pCache->pFree;
    pCache->pFree = p->p_ਅਗਲਾ;
    p->p_ਅਗਲਾ = 0;
  }ਨਹੀਂਤਾਂ{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਸਮ੍ਰਿਤੀ_MANAGEMENT
    /* The group mutex must be released before pcache1Alloc() is called. This
    ** is because it might call sqlite3_release_memory(), which assumes that
    ** this mutex is not held. */
    ਪ੍ਰਤਿਜ੍ਞਾ( pcache1.separateCache==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pCache->p_ਸਮੂਹ==&pcache1.grp );
    pcache1LeaveMutex(pCache->p_ਸਮੂਹ);
#ਜੇ_ਅੰਤ
    ਜੇ( benignMalloc ){ sqlite3BeginBenignMalloc(); }
    pPg = pcache1Alloc(pCache->szAlloc);
    ਜੇ( benignMalloc ){ sqlite3_ਸਿਰਾ_Benign_Malloc(); }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਸਮ੍ਰਿਤੀ_MANAGEMENT
    pcache1EnterMutex(pCache->p_ਸਮੂਹ);
#ਜੇ_ਅੰਤ
    ਜੇ( pPg==0 ) ਵਾਪਸ 0;
    p = (PgHdr1 *)&((u8 *)pPg)[pCache->sz_ਪੰਨਾ];
    p->ਪੰਨਾ.pBuf = pPg;
    p->ਪੰਨਾ.pExtra = (u8*)p + ROUND8(ਆਕਾਰ(*p));
    ਪ੍ਰਤਿਜ੍ਞਾ( EIGHT_BYTE_ALIGNMENT( p->ਪੰਨਾ.pExtra ) );
    p->isBulkLocal = 0;
    p->isAnchor = 0;
    p->pLruPrev = 0;           /* Initializing this saves a valgrind error */
  }
  (*pCache->pnPurgeable)++;
  ਵਾਪਸ p;
}

/*
** Free a page object allocated by pcache1AllocPage().
*/
ਸਥਿਰ ਖਾਲੀ pcache1_Free_ਪੰਨਾ(PgHdr1 *p){
  PCache1 *pCache;
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  pCache = p->pCache;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->pCache->p_ਸਮੂਹ->mutex) );
  ਜੇ( p->isBulkLocal ){
    p->p_ਅਗਲਾ = pCache->pFree;
    pCache->pFree = p;
  }ਨਹੀਂਤਾਂ{
    pcache1Free(p->ਪੰਨਾ.pBuf);
  }
  (*pCache->pnPurgeable)--;
}

/*
** Malloc function used by SQLite to obtain space from the buffer configured
** using sqlite3_config(SQLITE_CONFIG_PAGECACHE) option. If no such buffer
** exists, this function falls back to sqlite3Malloc().
*/
ਖਾਲੀ *sqlite3_ਪੰਨਾ_Malloc(ਪੂਰਨ sz){
  ਪ੍ਰਤਿਜ੍ਞਾ( sz<=65536+8 ); /* These allocations are never very large */
  ਵਾਪਸ pcache1Alloc(sz);
}

/*
** Free an allocated buffer obtained from sqlite3PageMalloc().
*/
ਖਾਲੀ sqlite3_ਪੰਨਾ_Free(ਖਾਲੀ *p){
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
ਸਥਿਰ ਪੂਰਨ pcache1_Under_ਸਮ੍ਰਿਤੀ_Pressure(PCache1 *pCache){
  ਜੇ( pcache1.n_ਥਾਂ && (pCache->sz_ਪੰਨਾ+pCache->szExtra)<=pcache1.sz_ਥਾਂ ){
    ਵਾਪਸ Atomic_ਲੱਦੋ(&pcache1.bUnderPressure);
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ sqlite3_ਢੇਰ_Nearly_ਭਰਿਆ();
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
ਸਥਿਰ ਖਾਲੀ pcache1ResizeHash(PCache1 *p){
  PgHdr1 **apNew;
  u64 nNew;
  u32 i;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p->p_ਸਮੂਹ->mutex) );

  nNew = 2*(u64)p->nHash;
  ਜੇ( nNew<256 ){
    nNew = 256;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( (nNew & (nNew-1))==0 );   /* nNew is always a power of two */

  pcache1LeaveMutex(p->p_ਸਮੂਹ);
  ਜੇ( p->nHash ){ sqlite3BeginBenignMalloc(); }
  apNew = (PgHdr1 **)sqlite3MallocZero(ਆਕਾਰ(PgHdr1 *)*nNew);
  ਜੇ( p->nHash ){ sqlite3_ਸਿਰਾ_Benign_Malloc(); }
  pcache1EnterMutex(p->p_ਸਮੂਹ);
  ਜੇ( apNew ){
    ਲਈ(i=0; i<p->nHash; i++){
      PgHdr1 *p_ਪੰਨਾ;
      PgHdr1 *p_ਅਗਲਾ = p->apHash[i];
      ਜਦੋਂ( (p_ਪੰਨਾ = p_ਅਗਲਾ)!=0 ){
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ h = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(p_ਪੰਨਾ->i_ਕੁੰਜੀ & (nNew-1));
        p_ਅਗਲਾ = p_ਪੰਨਾ->p_ਅਗਲਾ;
        p_ਪੰਨਾ->p_ਅਗਲਾ = apNew[h];
        apNew[h] = p_ਪੰਨਾ;
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
ਸਥਿਰ PgHdr1 *pcache1_Pin_ਪੰਨਾ(PgHdr1 *p_ਪੰਨਾ){
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਪੰਨਾ_IS_UNPINNED(p_ਪੰਨਾ) );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->p_Lru_ਅਗਲਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pLruPrev );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਪੰਨਾ->pCache->p_ਸਮੂਹ->mutex) );
  p_ਪੰਨਾ->pLruPrev->p_Lru_ਅਗਲਾ = p_ਪੰਨਾ->p_Lru_ਅਗਲਾ;
  p_ਪੰਨਾ->p_Lru_ਅਗਲਾ->pLruPrev = p_ਪੰਨਾ->pLruPrev;
  p_ਪੰਨਾ->p_Lru_ਅਗਲਾ = 0;
  /* pPage->pLruPrev = 0;
  ** No need to clear pLruPrev as it is never accessed if pLruNext is 0 */
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->isAnchor==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pCache->p_ਸਮੂਹ->lru.isAnchor==1 );
  p_ਪੰਨਾ->pCache->nRecyclable--;
  ਵਾਪਸ p_ਪੰਨਾ;
}


/*
** Remove the page supplied as an argument from the hash table
** (PCache1.apHash structure) that it is currently stored in.
** Also free the page if freePage is true.
**
** The PGroup mutex must be held when this function is called.
*/
ਸਥਿਰ ਖਾਲੀ pcache1RemoveFromHash(PgHdr1 *p_ਪੰਨਾ, ਪੂਰਨ freeFlag){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ h;
  PCache1 *pCache = p_ਪੰਨਾ->pCache;
  PgHdr1 **pp;

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCache->p_ਸਮੂਹ->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  h = p_ਪੰਨਾ->i_ਕੁੰਜੀ & (pCache->nHash-1);
  ਲਈ(pp=&pCache->apHash[h]; (*pp)!=p_ਪੰਨਾ; pp=&(*pp)->p_ਅਗਲਾ);
  *pp = (*pp)->p_ਅਗਲਾ;

  pCache->n_ਪੰਨਾ--;
  ਜੇ( freeFlag ) pcache1_Free_ਪੰਨਾ(p_ਪੰਨਾ);
}

/*
** If there are currently more than nMaxPage pages allocated, try
** to recycle pages to reduce the number allocated to nMaxPage.
*/
ਸਥਿਰ ਖਾਲੀ pcache1_Enforce_Max_ਪੰਨਾ(PCache1 *pCache){
  PGroup *p_ਸਮੂਹ = pCache->p_ਸਮੂਹ;
  PgHdr1 *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(p_ਸਮੂਹ->mutex) );
  ਜਦੋਂ( p_ਸਮੂਹ->nPurgeable>p_ਸਮੂਹ->n_Max_ਪੰਨਾ
      && (p=p_ਸਮੂਹ->lru.pLruPrev)->isAnchor==0
  ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->pCache->p_ਸਮੂਹ==p_ਸਮੂਹ );
    ਪ੍ਰਤਿਜ੍ਞਾ( ਪੰਨਾ_IS_UNPINNED(p) );
    pcache1_Pin_ਪੰਨਾ(p);
    pcache1RemoveFromHash(p, 1);
  }
  ਜੇ( pCache->n_ਪੰਨਾ==0 && pCache->pBulk ){
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
ਸਥਿਰ ਖਾਲੀ pcache1TruncateUnsafe(
  PCache1 *pCache,             /* The cache to truncate */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i_ਹੱਦਬੰਦੀ          /* Drop pages with this pgno or larger */
){
  TESTONLY( ਪੂਰਨ n_ਪੰਨਾ = 0; )  /* To assert pCache->nPage is correct */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ h, iStop;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_held(pCache->p_ਸਮੂਹ->mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->i_Max_ਕੁੰਜੀ >= i_ਹੱਦਬੰਦੀ );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  ਜੇ( pCache->i_Max_ਕੁੰਜੀ - i_ਹੱਦਬੰਦੀ < pCache->nHash ){
    /* If we are just shaving the last few pages off the end of the
    ** cache, then there is no point in scanning the entire hash table.
    ** Only scan those hash slots that might contain pages that need to
    ** be removed. */
    h = i_ਹੱਦਬੰਦੀ & (pCache->nHash-1);
    iStop = pCache->i_Max_ਕੁੰਜੀ & (pCache->nHash-1);
    TESTONLY( n_ਪੰਨਾ = -10; )  /* Disable the pCache->nPage validity check */
  }ਨਹੀਂਤਾਂ{
    /* This is the general case where many pages are being removed.
    ** It is necessary to scan the entire hash table */
    h = pCache->nHash/2;
    iStop = h - 1;
  }
  ਲਈ(;;){
    PgHdr1 **pp;
    PgHdr1 *p_ਪੰਨਾ;
    ਪ੍ਰਤਿਜ੍ਞਾ( h<pCache->nHash );
    pp = &pCache->apHash[h];
    ਜਦੋਂ( (p_ਪੰਨਾ = *pp)!=0 ){
      ਜੇ( p_ਪੰਨਾ->i_ਕੁੰਜੀ>=i_ਹੱਦਬੰਦੀ ){
        pCache->n_ਪੰਨਾ--;
        *pp = p_ਪੰਨਾ->p_ਅਗਲਾ;
        ਜੇ( ਪੰਨਾ_IS_UNPINNED(p_ਪੰਨਾ) ) pcache1_Pin_ਪੰਨਾ(p_ਪੰਨਾ);
        pcache1_Free_ਪੰਨਾ(p_ਪੰਨਾ);
      }ਨਹੀਂਤਾਂ{
        pp = &p_ਪੰਨਾ->p_ਅਗਲਾ;
        TESTONLY( ਜੇ( n_ਪੰਨਾ>=0 ) n_ਪੰਨਾ++; )
      }
    }
    ਜੇ( h==iStop ) ਰੋਕੋ;
    h = (h+1) & (pCache->nHash-1);
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਪੰਨਾ<0 || pCache->n_ਪੰਨਾ==(ਬਿਨਾਨਿਸ਼ਾਨੀ)n_ਪੰਨਾ );
}

/******************************************************************************/
/******** sqlite3_pcache Methods **********************************************/

/*
** Implementation of the sqlite3_pcache.xInit method.
*/
ਸਥਿਰ ਪੂਰਨ pcache1Init(ਖਾਲੀ *Not_ਵਰਤਿਆ){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਪ੍ਰਤਿਜ੍ਞਾ( pcache1.isInit==0 );
  memset(&pcache1, 0, ਆਕਾਰ(pcache1));


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
#ਜੇ defined(SQLITE_ENABLE_ਸਮ੍ਰਿਤੀ_MANAGEMENT)
  pcache1.separateCache = 0;
#ਜਾਂਜੇ SQLITE_THREADSAFE
  pcache1.separateCache = sqlite3GlobalConfig.p_ਪੰਨਾ==0
                          || sqlite3GlobalConfig.bCoreMutex>0;
#ਨਹੀਂਤਾਂ
  pcache1.separateCache = sqlite3GlobalConfig.p_ਪੰਨਾ==0;
#ਜੇ_ਅੰਤ

#ਜੇ SQLITE_THREADSAFE
  ਜੇ( sqlite3GlobalConfig.bCoreMutex ){
    pcache1.grp.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_LRU);
    pcache1.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PMEM);
  }
#ਜੇ_ਅੰਤ
  ਜੇ( pcache1.separateCache
   && sqlite3GlobalConfig.n_ਪੰਨਾ!=0
   && sqlite3GlobalConfig.p_ਪੰਨਾ==0
  ){
    pcache1.n_Init_ਪੰਨਾ = sqlite3GlobalConfig.n_ਪੰਨਾ;
  }ਨਹੀਂਤਾਂ{
    pcache1.n_Init_ਪੰਨਾ = 0;
  }
  pcache1.grp.mxPinned = 10;
  pcache1.isInit = 1;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Implementation of the sqlite3_pcache.xShutdown method.
** Note that the static mutex allocated in xInit does
** not need to be freed.
*/
ਸਥਿਰ ਖਾਲੀ pcache1Shutdown(ਖਾਲੀ *Not_ਵਰਤਿਆ){
  ਅਣਵਰਤਿਆ_PARAMETER(Not_ਵਰਤਿਆ);
  ਪ੍ਰਤਿਜ੍ਞਾ( pcache1.isInit!=0 );
  memset(&pcache1, 0, ਆਕਾਰ(pcache1));
}

/* forward declaration */
ਸਥਿਰ ਖਾਲੀ pcache1Destroy(sqlite3_pcache *p);

/*
** Implementation of the sqlite3_pcache.xCreate method.
**
** Allocate a new cache.
*/
ਸਥਿਰ sqlite3_pcache *pcache1Create(ਪੂਰਨ sz_ਪੰਨਾ, ਪੂਰਨ szExtra, ਪੂਰਨ bPurgeable){
  PCache1 *pCache;      /* The newly created page cache */
  PGroup *p_ਸਮੂਹ;       /* The group the new page cache will belong to */
  i64 sz;               /* Bytes of memory required to allocate the new cache */

  ਪ੍ਰਤਿਜ੍ਞਾ( (sz_ਪੰਨਾ & (sz_ਪੰਨਾ-1))==0 && sz_ਪੰਨਾ>=512 && sz_ਪੰਨਾ<=65536 );
  ਪ੍ਰਤਿਜ੍ਞਾ( szExtra < 300 );

  sz = ਆਕਾਰ(PCache1) + ਆਕਾਰ(PGroup)*pcache1.separateCache;
  pCache = (PCache1 *)sqlite3MallocZero(sz);
  ਜੇ( pCache ){
    ਜੇ( pcache1.separateCache ){
      p_ਸਮੂਹ = (PGroup*)&pCache[1];
      p_ਸਮੂਹ->mxPinned = 10;
    }ਨਹੀਂਤਾਂ{
      p_ਸਮੂਹ = &pcache1.grp;
    }
    pcache1EnterMutex(p_ਸਮੂਹ);
    ਜੇ( p_ਸਮੂਹ->lru.isAnchor==0 ){
      p_ਸਮੂਹ->lru.isAnchor = 1;
      p_ਸਮੂਹ->lru.pLruPrev = p_ਸਮੂਹ->lru.p_Lru_ਅਗਲਾ = &p_ਸਮੂਹ->lru;
    }
    pCache->p_ਸਮੂਹ = p_ਸਮੂਹ;
    pCache->sz_ਪੰਨਾ = sz_ਪੰਨਾ;
    pCache->szExtra = szExtra;
    pCache->szAlloc = sz_ਪੰਨਾ + szExtra + ROUND8(ਆਕਾਰ(PgHdr1));
    pCache->bPurgeable = (bPurgeable ? 1 : 0);
    pcache1ResizeHash(pCache);
    ਜੇ( bPurgeable ){
      pCache->nMin = 10;
      p_ਸਮੂਹ->n_Min_ਪੰਨਾ += pCache->nMin;
      p_ਸਮੂਹ->mxPinned = p_ਸਮੂਹ->n_Max_ਪੰਨਾ + 10 - p_ਸਮੂਹ->n_Min_ਪੰਨਾ;
      pCache->pnPurgeable = &p_ਸਮੂਹ->nPurgeable;
    }ਨਹੀਂਤਾਂ{
      pCache->pnPurgeable = &pCache->nPurgeableDummy;
    }
    pcache1LeaveMutex(p_ਸਮੂਹ);
    ਜੇ( pCache->nHash==0 ){
      pcache1Destroy((sqlite3_pcache*)pCache);
      pCache = 0;
    }
  }
  ਵਾਪਸ (sqlite3_pcache *)pCache;
}

/*
** Implementation of the sqlite3_pcache.xCachesize method.
**
** Configure the cache_size limit for a cache.
*/
ਸਥਿਰ ਖਾਲੀ pcache1Cachesize(sqlite3_pcache *p, ਪੂਰਨ nMax){
  PCache1 *pCache = (PCache1 *)p;
  u32 n;
  ਪ੍ਰਤਿਜ੍ਞਾ( nMax>=0 );
  ਜੇ( pCache->bPurgeable ){
    PGroup *p_ਸਮੂਹ = pCache->p_ਸਮੂਹ;
    pcache1EnterMutex(p_ਸਮੂਹ);
    n = (u32)nMax;
    ਜੇ( n > 0x7fff0000 - p_ਸਮੂਹ->n_Max_ਪੰਨਾ + pCache->nMax ){
      n = 0x7fff0000 - p_ਸਮੂਹ->n_Max_ਪੰਨਾ + pCache->nMax;
    }
    p_ਸਮੂਹ->n_Max_ਪੰਨਾ += (n - pCache->nMax);
    p_ਸਮੂਹ->mxPinned = p_ਸਮੂਹ->n_Max_ਪੰਨਾ + 10 - p_ਸਮੂਹ->n_Min_ਪੰਨਾ;
    pCache->nMax = n;
    pCache->n90pct = pCache->nMax*9/10;
    pcache1_Enforce_Max_ਪੰਨਾ(pCache);
    pcache1LeaveMutex(p_ਸਮੂਹ);
  }
}

/*
** Implementation of the sqlite3_pcache.xShrink method.
**
** Free up as much memory as possible.
*/
ਸਥਿਰ ਖਾਲੀ pcache1Shrink(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1*)p;
  ਜੇ( pCache->bPurgeable ){
    PGroup *p_ਸਮੂਹ = pCache->p_ਸਮੂਹ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ saved_Max_ਪੰਨਾ;
    pcache1EnterMutex(p_ਸਮੂਹ);
    saved_Max_ਪੰਨਾ = p_ਸਮੂਹ->n_Max_ਪੰਨਾ;
    p_ਸਮੂਹ->n_Max_ਪੰਨਾ = 0;
    pcache1_Enforce_Max_ਪੰਨਾ(pCache);
    p_ਸਮੂਹ->n_Max_ਪੰਨਾ = saved_Max_ਪੰਨਾ;
    pcache1LeaveMutex(p_ਸਮੂਹ);
  }
}

/*
** Implementation of the sqlite3_pcache.xPagecount method.
*/
ਸਥਿਰ ਪੂਰਨ pcache1Pagecount(sqlite3_pcache *p){
  ਪੂਰਨ n;
  PCache1 *pCache = (PCache1*)p;
  pcache1EnterMutex(pCache->p_ਸਮੂਹ);
  n = pCache->n_ਪੰਨਾ;
  pcache1LeaveMutex(pCache->p_ਸਮੂਹ);
  ਵਾਪਸ n;
}


/*
** Implement steps 3, 4, and 5 of the pcache1Fetch() algorithm described
** in the header of the pcache1Fetch() procedure.
**
** This steps are broken out into a separate procedure because they are
** usually not needed, and by avoiding the stack initialization required
** for these steps, the main pcache1Fetch() procedure can run faster.
*/
ਸਥਿਰ SQLITE_NOINLINE PgHdr1 *pcache1FetchStage2(
  PCache1 *pCache,
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i_ਕੁੰਜੀ,
  ਪੂਰਨ createFlag
){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nPinned;
  PGroup *p_ਸਮੂਹ = pCache->p_ਸਮੂਹ;
  PgHdr1 *p_ਪੰਨਾ = 0;

  /* Step 3: Abort if createFlag is 1 but the cache is nearly full */
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->n_ਪੰਨਾ >= pCache->nRecyclable );
  nPinned = pCache->n_ਪੰਨਾ - pCache->nRecyclable;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਮੂਹ->mxPinned == p_ਸਮੂਹ->n_Max_ਪੰਨਾ + 10 - p_ਸਮੂਹ->n_Min_ਪੰਨਾ );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->n90pct == pCache->nMax*9/10 );
  ਜੇ( createFlag==1 && (
        nPinned>=p_ਸਮੂਹ->mxPinned
     || nPinned>=pCache->n90pct
     || (pcache1_Under_ਸਮ੍ਰਿਤੀ_Pressure(pCache) && pCache->nRecyclable<nPinned)
  )){
    ਵਾਪਸ 0;
  }

  ਜੇ( pCache->n_ਪੰਨਾ>=pCache->nHash ) pcache1ResizeHash(pCache);
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->nHash>0 && pCache->apHash );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pCache->nHash & (pCache->nHash-1))==0 );

  /* Step 4. Try to recycle a page. */
  ਜੇ( pCache->bPurgeable
   && !p_ਸਮੂਹ->lru.pLruPrev->isAnchor
   && ((pCache->n_ਪੰਨਾ+1>=pCache->nMax) || pcache1_Under_ਸਮ੍ਰਿਤੀ_Pressure(pCache))
  ){
    PCache1 *pOther;
    p_ਪੰਨਾ = p_ਸਮੂਹ->lru.pLruPrev;
    ਪ੍ਰਤਿਜ੍ਞਾ( ਪੰਨਾ_IS_UNPINNED(p_ਪੰਨਾ) );
    pcache1RemoveFromHash(p_ਪੰਨਾ, 0);
    pcache1_Pin_ਪੰਨਾ(p_ਪੰਨਾ);
    pOther = p_ਪੰਨਾ->pCache;
    ਜੇ( pOther->szAlloc != pCache->szAlloc ){
      pcache1_Free_ਪੰਨਾ(p_ਪੰਨਾ);
      p_ਪੰਨਾ = 0;
    }ਨਹੀਂਤਾਂ{
      p_ਸਮੂਹ->nPurgeable -= (pOther->bPurgeable - pCache->bPurgeable);
    }
  }

  /* Step 5. If a usable page buffer has still not been found,
  ** attempt to allocate a new one.
  */
  ਜੇ( !p_ਪੰਨਾ ){
    p_ਪੰਨਾ = pcache1_Alloc_ਪੰਨਾ(pCache, createFlag==1);
  }

  ਜੇ( p_ਪੰਨਾ ){
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ h = i_ਕੁੰਜੀ & (pCache->nHash-1);
    pCache->n_ਪੰਨਾ++;
    p_ਪੰਨਾ->i_ਕੁੰਜੀ = i_ਕੁੰਜੀ;
    p_ਪੰਨਾ->p_ਅਗਲਾ = pCache->apHash[h];
    p_ਪੰਨਾ->pCache = pCache;
    p_ਪੰਨਾ->p_Lru_ਅਗਲਾ = 0;
    /* pPage->pLruPrev = 0;
    ** No need to clear pLruPrev since it is not accessed when pLruNext==0 */
    *(ਖਾਲੀ **)p_ਪੰਨਾ->ਪੰਨਾ.pExtra = 0;
    pCache->apHash[h] = p_ਪੰਨਾ;
    ਜੇ( i_ਕੁੰਜੀ>pCache->i_Max_ਕੁੰਜੀ ){
      pCache->i_Max_ਕੁੰਜੀ = i_ਕੁੰਜੀ;
    }
  }
  ਵਾਪਸ p_ਪੰਨਾ;
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
ਸਥਿਰ PgHdr1 *pcache1FetchNoMutex(
  sqlite3_pcache *p,
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i_ਕੁੰਜੀ,
  ਪੂਰਨ createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_ਪੰਨਾ = 0;

  /* Step 1: Search the hash table for an existing entry.  nHash is always
  ** a power of two when the cache is in use (see pcache1ResizeHash()), so
  ** the modulo reduces to a mask, avoiding a hardware divide. */
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  p_ਪੰਨਾ = pCache->apHash[i_ਕੁੰਜੀ & (pCache->nHash-1u)];
  ਜਦੋਂ( p_ਪੰਨਾ && p_ਪੰਨਾ->i_ਕੁੰਜੀ!=i_ਕੁੰਜੀ ){ p_ਪੰਨਾ = p_ਪੰਨਾ->p_ਅਗਲਾ; }

  /* Step 2: If the page was found in the hash table, then return it.
  ** If the page was not in the hash table and createFlag is 0, abort.
  ** Otherwise (page not in hash and createFlag!=0) continue with
  ** subsequent steps to try to create the page. */
  ਜੇ( p_ਪੰਨਾ ){
    ਜੇ( ਪੰਨਾ_IS_UNPINNED(p_ਪੰਨਾ) ){
      ਵਾਪਸ pcache1_Pin_ਪੰਨਾ(p_ਪੰਨਾ);
    }ਨਹੀਂਤਾਂ{
      ਵਾਪਸ p_ਪੰਨਾ;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( createFlag ){
    /* Steps 3, 4, and 5 implemented by this subroutine */
    ਵਾਪਸ pcache1FetchStage2(pCache, i_ਕੁੰਜੀ, createFlag);
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
  }
}
#ਜੇ PCACHE1_MIGHT_USE_ਸਮੂਹ_MUTEX
ਸਥਿਰ PgHdr1 *pcache1FetchWithMutex(
  sqlite3_pcache *p,
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i_ਕੁੰਜੀ,
  ਪੂਰਨ createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_ਪੰਨਾ;

  pcache1EnterMutex(pCache->p_ਸਮੂਹ);
  p_ਪੰਨਾ = pcache1FetchNoMutex(p, i_ਕੁੰਜੀ, createFlag);
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ==0 || pCache->i_Max_ਕੁੰਜੀ>=i_ਕੁੰਜੀ );
  pcache1LeaveMutex(pCache->p_ਸਮੂਹ);
  ਵਾਪਸ p_ਪੰਨਾ;
}
#ਜੇ_ਅੰਤ
ਸਥਿਰ sqlite3_pcache_ਪੰਨਾ *pcache1Fetch(
  sqlite3_pcache *p,
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i_ਕੁੰਜੀ,
  ਪੂਰਨ createFlag
){
#ਜੇ PCACHE1_MIGHT_USE_ਸਮੂਹ_MUTEX || defined(SQLITE_DEBUG)
  PCache1 *pCache = (PCache1 *)p;
#ਜੇ_ਅੰਤ

  ਪ੍ਰਤਿਜ੍ਞਾ( offsetof(PgHdr1,ਪੰਨਾ)==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->bPurgeable || createFlag!=1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->bPurgeable || pCache->nMin==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->bPurgeable==0 || pCache->nMin==10 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->nMin==0 || pCache->bPurgeable );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->nHash>0 );
#ਜੇ PCACHE1_MIGHT_USE_ਸਮੂਹ_MUTEX
  ਜੇ( pCache->p_ਸਮੂਹ->mutex ){
    ਵਾਪਸ (sqlite3_pcache_ਪੰਨਾ*)pcache1FetchWithMutex(p, i_ਕੁੰਜੀ, createFlag);
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
  {
    ਵਾਪਸ (sqlite3_pcache_ਪੰਨਾ*)pcache1FetchNoMutex(p, i_ਕੁੰਜੀ, createFlag);
  }
}


/*
** Implementation of the sqlite3_pcache.xUnpin method.
**
** Mark a page as unpinned (eligible for asynchronous recycling).
*/
ਸਥਿਰ ਖਾਲੀ pcache1Unpin(
  sqlite3_pcache *p,
  sqlite3_pcache_ਪੰਨਾ *pPg,
  ਪੂਰਨ reuseUnlikely
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_ਪੰਨਾ = (PgHdr1 *)pPg;
  PGroup *p_ਸਮੂਹ = pCache->p_ਸਮੂਹ;

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pCache==pCache );
  pcache1EnterMutex(p_ਸਮੂਹ);

  /* It is an error to call this function if the page is already
  ** part of the PGroup LRU list.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->p_Lru_ਅਗਲਾ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਪੰਨਾ_IS_PINNED(p_ਪੰਨਾ) );

  ਜੇ( reuseUnlikely || p_ਸਮੂਹ->nPurgeable>p_ਸਮੂਹ->n_Max_ਪੰਨਾ ){
    pcache1RemoveFromHash(p_ਪੰਨਾ, 1);
  }ਨਹੀਂਤਾਂ{
    /* Add the page to the PGroup LRU list. */
    PgHdr1 **pp_ਪਹਿਲਾ = &p_ਸਮੂਹ->lru.p_Lru_ਅਗਲਾ;
    p_ਪੰਨਾ->pLruPrev = &p_ਸਮੂਹ->lru;
    (p_ਪੰਨਾ->p_Lru_ਅਗਲਾ = *pp_ਪਹਿਲਾ)->pLruPrev = p_ਪੰਨਾ;
    *pp_ਪਹਿਲਾ = p_ਪੰਨਾ;
    pCache->nRecyclable++;
  }

  pcache1LeaveMutex(pCache->p_ਸਮੂਹ);
}

/*
** Implementation of the sqlite3_pcache.xRekey method.
*/
ਸਥਿਰ ਖਾਲੀ pcache1Rekey(
  sqlite3_pcache *p,
  sqlite3_pcache_ਪੰਨਾ *pPg,
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ iOld,
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ iNew
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_ਪੰਨਾ = (PgHdr1 *)pPg;
  PgHdr1 **pp;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ hOld, hNew;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->i_ਕੁੰਜੀ==iOld );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੰਨਾ->pCache==pCache );
  ਪ੍ਰਤਿਜ੍ਞਾ( iOld!=iNew );               /* The page number really is changing */

  pcache1EnterMutex(pCache->p_ਸਮੂਹ);

  ਪ੍ਰਤਿਜ੍ਞਾ( pcache1FetchNoMutex(p, iOld, 0)==p_ਪੰਨਾ ); /* pPg really is iOld */
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  hOld = iOld & (pCache->nHash-1);
  pp = &pCache->apHash[hOld];
  ਜਦੋਂ( (*pp)!=p_ਪੰਨਾ ){
    pp = &(*pp)->p_ਅਗਲਾ;
  }
  *pp = p_ਪੰਨਾ->p_ਅਗਲਾ;

  ਪ੍ਰਤਿਜ੍ਞਾ( pcache1FetchNoMutex(p, iNew, 0)==0 ); /* iNew not in cache */
  hNew = iNew & (pCache->nHash-1);
  p_ਪੰਨਾ->i_ਕੁੰਜੀ = iNew;
  p_ਪੰਨਾ->p_ਅਗਲਾ = pCache->apHash[hNew];
  pCache->apHash[hNew] = p_ਪੰਨਾ;
  ਜੇ( iNew>pCache->i_Max_ਕੁੰਜੀ ){
    pCache->i_Max_ਕੁੰਜੀ = iNew;
  }

  pcache1LeaveMutex(pCache->p_ਸਮੂਹ);
}

/*
** Implementation of the sqlite3_pcache.xTruncate method.
**
** Discard all unpinned pages in the cache with a page number equal to
** or greater than parameter iLimit. Any pinned pages with a page number
** equal to or greater than iLimit are implicitly unpinned.
*/
ਸਥਿਰ ਖਾਲੀ pcache1Truncate(sqlite3_pcache *p, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i_ਹੱਦਬੰਦੀ){
  PCache1 *pCache = (PCache1 *)p;
  pcache1EnterMutex(pCache->p_ਸਮੂਹ);
  ਜੇ( i_ਹੱਦਬੰਦੀ<=pCache->i_Max_ਕੁੰਜੀ ){
    pcache1TruncateUnsafe(pCache, i_ਹੱਦਬੰਦੀ);
    pCache->i_Max_ਕੁੰਜੀ = i_ਹੱਦਬੰਦੀ-1;
  }
  pcache1LeaveMutex(pCache->p_ਸਮੂਹ);
}

/*
** Implementation of the sqlite3_pcache.xDestroy method.
**
** Destroy a cache allocated using pcache1Create().
*/
ਸਥਿਰ ਖਾਲੀ pcache1Destroy(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1 *)p;
  PGroup *p_ਸਮੂਹ = pCache->p_ਸਮੂਹ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pCache->bPurgeable || (pCache->nMax==0 && pCache->nMin==0) );
  pcache1EnterMutex(p_ਸਮੂਹ);
  ਜੇ( pCache->n_ਪੰਨਾ ) pcache1TruncateUnsafe(pCache, 0);
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਮੂਹ->n_Max_ਪੰਨਾ >= pCache->nMax );
  p_ਸਮੂਹ->n_Max_ਪੰਨਾ -= pCache->nMax;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਮੂਹ->n_Min_ਪੰਨਾ >= pCache->nMin );
  p_ਸਮੂਹ->n_Min_ਪੰਨਾ -= pCache->nMin;
  p_ਸਮੂਹ->mxPinned = p_ਸਮੂਹ->n_Max_ਪੰਨਾ + 10 - p_ਸਮੂਹ->n_Min_ਪੰਨਾ;
  pcache1_Enforce_Max_ਪੰਨਾ(pCache);
  pcache1LeaveMutex(p_ਸਮੂਹ);
  sqlite3_free(pCache->pBulk);
  sqlite3_free(pCache->apHash);
  sqlite3_free(pCache);
}

/*
** This function is called during initialization (sqlite3_initialize()) to
** install the default pluggable cache module, assuming the user has not
** already provided an alternative.
*/
ਖਾਲੀ sqlite3_PCache_ਸੈੱਟ_Default(ਖਾਲੀ){
  ਸਥਿਰ ਅਚਲ sqlite3_pcache_methods2 defaultMethods = {
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
ਪੂਰਨ sqlite3_ਸਿਰਲੇਖ_ਮਾਪ_Pcache1(ਖਾਲੀ){ ਵਾਪਸ ROUND8(ਆਕਾਰ(PgHdr1)); }

/*
** Return the global mutex used by this PCACHE implementation.  The
** sqlite3_status() routine needs access to this mutex.
*/
sqlite3_mutex *sqlite3Pcache1Mutex(ਖਾਲੀ){
  ਵਾਪਸ pcache1.mutex;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਸਮ੍ਰਿਤੀ_MANAGEMENT
/*
** This function is called to free superfluous dynamically allocated memory
** held by the pager system. Memory in use by any SQLite pager allocated
** by the current thread may be sqlite3_free()ed.
**
** nReq is the number of bytes of memory required. Once this much has
** been released, the function returns. The return value is the total number
** of bytes of memory released.
*/
ਪੂਰਨ sqlite3_Pcache_Release_ਸਮ੍ਰਿਤੀ(ਪੂਰਨ nReq){
  ਪੂਰਨ nFree = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_mutex_notheld(pcache1.mutex) );
  ਜੇ( sqlite3GlobalConfig.p_ਪੰਨਾ==0 ){
    PgHdr1 *p;
    pcache1EnterMutex(&pcache1.grp);
    ਜਦੋਂ( (nReq<0 || nFree<nReq)
       &&  (p=pcache1.grp.lru.pLruPrev)!=0
       &&  p->isAnchor==0
    ){
      nFree += pcache1_Mem_ਮਾਪ(p->ਪੰਨਾ.pBuf);
      ਪ੍ਰਤਿਜ੍ਞਾ( ਪੰਨਾ_IS_UNPINNED(p) );
      pcache1_Pin_ਪੰਨਾ(p);
      pcache1RemoveFromHash(p, 1);
    }
    pcache1LeaveMutex(&pcache1.grp);
  }
  ਵਾਪਸ nFree;
}
#ਜੇ_ਅੰਤ /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
/*
** This function is used by test procedures to inspect the internal state
** of the global cache.
*/
ਖਾਲੀ sqlite3PcacheStats(
  ਪੂਰਨ *pnCurrent,      /* OUT: Total number of pages cached */
  ਪੂਰਨ *pnMax,          /* OUT: Global maximum cache size */
  ਪੂਰਨ *pnMin,          /* OUT: Sum of PCache1.nMin for purgeable caches */
  ਪੂਰਨ *pnRecyclable    /* OUT: Total number of pages available for recycling */
){
  PgHdr1 *p;
  ਪੂਰਨ nRecyclable = 0;
  ਲਈ(p=pcache1.grp.lru.p_Lru_ਅਗਲਾ; p && !p->isAnchor; p=p->p_Lru_ਅਗਲਾ){
    ਪ੍ਰਤਿਜ੍ਞਾ( ਪੰਨਾ_IS_UNPINNED(p) );
    nRecyclable++;
  }
  *pnCurrent = pcache1.grp.nPurgeable;
  *pnMax = (ਪੂਰਨ)pcache1.grp.n_Max_ਪੰਨਾ;
  *pnMin = (ਪੂਰਨ)pcache1.grp.n_Min_ਪੰਨਾ;
  *pnRecyclable = nRecyclable;
}
#ਜੇ_ਅੰਤ
