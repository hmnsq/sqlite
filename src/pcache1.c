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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ PCache1 PCache1;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ PgHdr1 PgHdr1;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ PgFreeslot PgFreeslot;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ PGroup PGroup;

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
ರಚನೆ PgHdr1 {
  sqlite3_pcache_ಪುಟ ಪುಟ; /* Base class. Must be first. pBuf & pExtra */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i_ಕೀಲಿ;        /* Key value (page number) */
  u16 isBulkLocal;          /* This page from bulk local storage */
  u16 isAnchor;             /* This is the PGroup.lru element */
  PgHdr1 *p_ಮುಂದಿನ;            /* Next in hash table chain */
  PCache1 *pCache;          /* Cache that currently owns this page */
  PgHdr1 *p_Lru_ಮುಂದಿನ;         /* Next in circular LRU list of unpinned pages */
  PgHdr1 *pLruPrev;         /* Previous in LRU list of unpinned pages */
                            /* NB: pLruPrev is only valid if pLruNext!=0 */
};

/*
** A page is pinned if it is not on the LRU list.  To be "pinned" means
** that the page is in active use and must not be deallocated.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ಪುಟ_IS_PINNED(p)    ((p)->p_Lru_ಮುಂದಿನ==0)
#ವ್ಯಾಖ್ಯಾನಿಸು ಪುಟ_IS_UNPINNED(p)  ((p)->p_Lru_ಮುಂದಿನ!=0)

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
ರಚನೆ PGroup {
  sqlite3_mutex *mutex;          /* MUTEX_STATIC_LRU or NULL */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n_Max_ಪುಟ;         /* Sum of nMax for purgeable caches */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n_Min_ಪುಟ;         /* Sum of nMin for purgeable caches */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ mxPinned;         /* nMaxpage + 10 - nMinPage */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nPurgeable;       /* Number of purgeable pages allocated */
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
ರಚನೆ PCache1 {
  /* Cache configuration parameters. Page size (szPage) and the purgeable
  ** flag (bPurgeable) and the pnPurgeable pointer are all set when the
  ** cache is created and are never changed thereafter. nMax may be
  ** modified at any time by a call to the pcache1Cachesize() method.
  ** The PGroup mutex must be held when accessing nMax.
  */
  PGroup *p_ಗುಂಪು;                     /* PGroup this cache belongs to */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *pnPurgeable;          /* Pointer to pGroup->nPurgeable */
  ಪೂರ್ಣಾಂಕ sz_ಪುಟ;                         /* Size of database content section */
  ಪೂರ್ಣಾಂಕ szExtra;                        /* sizeof(MemPage)+sizeof(PgHdr) */
  ಪೂರ್ಣಾಂಕ szAlloc;                        /* Total size of one pcache line */
  ಪೂರ್ಣಾಂಕ bPurgeable;                     /* True if cache is purgeable */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nMin;                  /* Minimum number of pages reserved */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nMax;                  /* Configured "cache_size" value */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n90pct;                /* nMax*9/10 */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i_Max_ಕೀಲಿ;               /* Largest key seen since xTruncate() */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nPurgeableDummy;       /* pnPurgeable points here when not used*/

  /* Hash table of all pages. The following variables may only be accessed
  ** when the accessor is holding the PGroup mutex.
  */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nRecyclable;           /* Number of pages in the LRU list */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ n_ಪುಟ;                 /* Total number of pages in apHash */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nHash;                 /* Number of slots in apHash[] */
  PgHdr1 **apHash;                    /* Hash table for fast lookup by key */
  PgHdr1 *pFree;                      /* List of unused pcache-local pages */
  ಶೂನ್ಯ *pBulk;                        /* Bulk memory used by pcache-local */
};

/*
** Free slots in the allocator used to divide up the global page cache
** buffer provided using the SQLITE_CONFIG_PAGECACHE mechanism.
*/
ರಚನೆ PgFreeslot {
  PgFreeslot *p_ಮುಂದಿನ;  /* Next free slot */
};

/*
** Global data used by this cache.
*/
ಸ್ಥಿರ SQLITE_WSD ರಚನೆ PCacheGlobal {
  PGroup grp;                    /* The global PGroup for mode (2) */

  /* Variables related to SQLITE_CONFIG_PAGECACHE settings.  The
  ** szSlot, nSlot, pStart, pEnd, nReserve, and isInit values are all
  ** fixed at sqlite3_initialize() time and do not require mutex protection.
  ** The nFreeSlot and pFree values do require mutex protection.
  */
  ಪೂರ್ಣಾಂಕ isInit;                    /* True if initialized */
  ಪೂರ್ಣಾಂಕ separateCache;             /* Use a new PGroup for each PCache */
  ಪೂರ್ಣಾಂಕ n_Init_ಪುಟ;                 /* Initial bulk allocation size */
  ಪೂರ್ಣಾಂಕ sz_ಸ್ಥಾನ;                    /* Size of each free slot */
  ಪೂರ್ಣಾಂಕ n_ಸ್ಥಾನ;                     /* The number of pcache slots */
  ಪೂರ್ಣಾಂಕ nReserve;                  /* Try to keep nFreeSlot above this */
  ಶೂನ್ಯ *p_ಆರಂಭ, *p_ಕೊನೆ;           /* Bounds of global page cache memory */
  /* Above requires no mutex.  Use mutex below for variable that follow. */
  sqlite3_mutex *mutex;          /* Mutex for accessing the following: */
  PgFreeslot *pFree;             /* Free page blocks */
  ಪೂರ್ಣಾಂಕ n_Free_ಸ್ಥಾನ;                 /* Number of unused pcache slots */
  ಪೂರ್ಣಾಂಕ bUnderPressure;            /* True if low on PAGECACHE memory */
} pcache1_g;

/*
** All code in this file should access the global structure above via the
** alias "pcache1". This ensures that the WSD emulation is used when
** compiling for systems that do not support real WSD.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು pcache1 (GLOBAL(ರಚನೆ PCacheGlobal, pcache1_g))

/*
** Macros to enter and leave the PCache LRU mutex.
*/
#ಆದರೆ !defined(SQLITE_ENABLE_ಸ್ಮರಣೆ_MANAGEMENT) || SQLITE_THREADSAFE==0
# define pcache1EnterMutex(X)  ಪ್ರತಿಜ್ಞಾ((X)->mutex==0)
# define pcache1LeaveMutex(X)  ಪ್ರತಿಜ್ಞಾ((X)->mutex==0)
# define PCACHE1_MIGHT_USE_ಗುಂಪು_MUTEX 0
#ಇಲ್ಲದಿದ್ದರೆ
# define pcache1EnterMutex(X) sqlite3_mutex_enter((X)->mutex)
# define pcache1LeaveMutex(X) sqlite3_mutex_leave((X)->mutex)
# define PCACHE1_MIGHT_USE_ಗುಂಪು_MUTEX 1
#ಆದರೆ_ಅಂತ್ಯ

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
ಶೂನ್ಯ sqlite3_PCache_ಬಫರ್_ಸಿದ್ಧತೆ(ಶೂನ್ಯ *pBuf, ಪೂರ್ಣಾಂಕ sz, ಪೂರ್ಣಾಂಕ n){
  ಆದರೆ( pcache1.isInit ){
    PgFreeslot *p;
    ಆದರೆ( pBuf==0 ) sz = n = 0;
    ಆದರೆ( n==0 ) sz = 0;
    sz = ROUNDDOWN8(sz);
    pcache1.sz_ಸ್ಥಾನ = sz;
    pcache1.n_ಸ್ಥಾನ = pcache1.n_Free_ಸ್ಥಾನ = n;
    pcache1.nReserve = n>90 ? 10 : (n/10 + 1);
    pcache1.p_ಆರಂಭ = pBuf;
    pcache1.pFree = 0;
    AtomicStore(&pcache1.bUnderPressure,0);
    ಆಗ( n-- ){
      p = (PgFreeslot*)pBuf;
      p->p_ಮುಂದಿನ = pcache1.pFree;
      pcache1.pFree = p;
      pBuf = (ಶೂನ್ಯ*)&((ಅಕ್ಷರ*)pBuf)[sz];
    }
    pcache1.p_ಕೊನೆ = pBuf;
  }
}

/*
** Try to initialize the pCache->pFree and pCache->pBulk fields.  Return
** true if pCache->pFree ends up containing one or more free pages.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pcache1InitBulk(PCache1 *pCache){
  i64 szBulk;
  ಅಕ್ಷರ *zBulk;
  ಆದರೆ( pcache1.n_Init_ಪುಟ==0 ) ಹಿಂತಿರುಗಿ 0;
  /* Do not bother with a bulk allocation if the cache size very small */
  ಆದರೆ( pCache->nMax<3 ) ಹಿಂತಿರುಗಿ 0;
  sqlite3BeginBenignMalloc();
  ಆದರೆ( pcache1.n_Init_ಪುಟ>0 ){
    szBulk = pCache->szAlloc * (i64)pcache1.n_Init_ಪುಟ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    szBulk = -1024 * (i64)pcache1.n_Init_ಪುಟ;
  }
  ಆದರೆ( szBulk > pCache->szAlloc*(i64)pCache->nMax ){
    szBulk = pCache->szAlloc*(i64)pCache->nMax;
  }
  ಆದರೆ( szBulk>=pCache->szAlloc ){
    zBulk = pCache->pBulk = sqlite3Malloc( szBulk );
    sqlite3_ಕೊನೆ_Benign_Malloc();
    ಆದರೆ( zBulk ){
      ಪೂರ್ಣಾಂಕ nBulk = sqlite3_Malloc_ಗಾತ್ರ(zBulk)/pCache->szAlloc;
      ಮಾಡು{
        PgHdr1 *pX = (PgHdr1*)&zBulk[pCache->sz_ಪುಟ];
        pX->ಪುಟ.pBuf = zBulk;
        pX->ಪುಟ.pExtra = (u8*)pX + ROUND8(ಗಾತ್ರ(*pX));
        ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT( pX->ಪುಟ.pExtra ) );
        pX->isBulkLocal = 1;
        pX->isAnchor = 0;
        pX->p_ಮುಂದಿನ = pCache->pFree;
        pX->pLruPrev = 0;    /* Initializing this saves a valgrind error */
        pCache->pFree = pX;
        zBulk += pCache->szAlloc;
      }ಆಗ( --nBulk );
    }
  }
  ಹಿಂತಿರುಗಿ pCache->pFree!=0;
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
ಸ್ಥಿರ ಶೂನ್ಯ *pcache1Alloc(ಪೂರ್ಣಾಂಕ nByte){
  ಶೂನ್ಯ *p = 0;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  ಆದರೆ( nByte<=pcache1.sz_ಸ್ಥಾನ ){
    sqlite3_mutex_enter(pcache1.mutex);
    p = (PgHdr1 *)pcache1.pFree;
    ಆದರೆ( p ){
      pcache1.pFree = pcache1.pFree->p_ಮುಂದಿನ;
      pcache1.n_Free_ಸ್ಥಾನ--;
      AtomicStore(&pcache1.bUnderPressure,pcache1.n_Free_ಸ್ಥಾನ<pcache1.nReserve);
      ಪ್ರತಿಜ್ಞಾ( pcache1.n_Free_ಸ್ಥಾನ>=0 );
      sqlite3_ಸ್ಥಿತಿ_Highwater(SQLITE_ಸ್ಥಿತಿ_PAGECACHE_ಗಾತ್ರ, nByte);
      sqlite3_ಸ್ಥಿತಿ_ಮೇಲೆ(SQLITE_ಸ್ಥಿತಿ_PAGECACHE_ಬಳಸಿದ, 1);
    }
    sqlite3_mutex_leave(pcache1.mutex);
  }
  ಆದರೆ( p==0 ){
    /* Memory is not available in the SQLITE_CONFIG_PAGECACHE pool.  Get
    ** it from sqlite3Malloc instead.
    */
    p = sqlite3Malloc(nByte);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DISABLE_PAGECACHE_ಅತಿಪ್ರವಾಹ_STATS
    ಆದರೆ( p ){
      ಪೂರ್ಣಾಂಕ sz = sqlite3_Malloc_ಗಾತ್ರ(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3_ಸ್ಥಿತಿ_Highwater(SQLITE_ಸ್ಥಿತಿ_PAGECACHE_ಗಾತ್ರ, nByte);
      sqlite3_ಸ್ಥಿತಿ_ಮೇಲೆ(SQLITE_ಸ್ಥಿತಿ_PAGECACHE_ಅತಿಪ್ರವಾಹ, sz);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_Memdebug_ಗಣ_Type(p, MEMTYPE_PCACHE);
  }
  ಹಿಂತಿರುಗಿ p;
}

/*
** Free an allocated buffer obtained from pcache1Alloc().
*/
ಸ್ಥಿರ ಶೂನ್ಯ pcache1Free(ಶೂನ್ಯ *p){
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( SQLITE_WITHIN(p, pcache1.p_ಆರಂಭ, pcache1.p_ಕೊನೆ) ){
    PgFreeslot *p_ಸ್ಥಾನ;
    sqlite3_mutex_enter(pcache1.mutex);
    sqlite3_ಸ್ಥಿತಿ_ಕೆಳಕ್ಕೆ(SQLITE_ಸ್ಥಿತಿ_PAGECACHE_ಬಳಸಿದ, 1);
    p_ಸ್ಥಾನ = (PgFreeslot*)p;
    p_ಸ್ಥಾನ->p_ಮುಂದಿನ = pcache1.pFree;
    pcache1.pFree = p_ಸ್ಥಾನ;
    pcache1.n_Free_ಸ್ಥಾನ++;
    AtomicStore(&pcache1.bUnderPressure,pcache1.n_Free_ಸ್ಥಾನ<pcache1.nReserve);
    ಪ್ರತಿಜ್ಞಾ( pcache1.n_Free_ಸ್ಥಾನ<=pcache1.n_ಸ್ಥಾನ );
    sqlite3_mutex_leave(pcache1.mutex);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3_Memdebug_ಗಣ_Type(p, MEMTYPE_ರಾಶಿ);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DISABLE_PAGECACHE_ಅತಿಪ್ರವಾಹ_STATS
    {
      ಪೂರ್ಣಾಂಕ nFreed = 0;
      nFreed = sqlite3_Malloc_ಗಾತ್ರ(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3_ಸ್ಥಿತಿ_ಕೆಳಕ್ಕೆ(SQLITE_ಸ್ಥಿತಿ_PAGECACHE_ಅತಿಪ್ರವಾಹ, nFreed);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_free(p);
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ಮರಣೆ_MANAGEMENT
/*
** Return the size of a pcache allocation
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pcache1_Mem_ಗಾತ್ರ(ಶೂನ್ಯ *p){
  ಆದರೆ( p>=pcache1.p_ಆರಂಭ && p<pcache1.p_ಕೊನೆ ){
    ಹಿಂತಿರುಗಿ pcache1.sz_ಸ್ಥಾನ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ i_ಗಾತ್ರ;
    ಪ್ರತಿಜ್ಞಾ( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3_Memdebug_ಗಣ_Type(p, MEMTYPE_ರಾಶಿ);
    i_ಗಾತ್ರ = sqlite3_Malloc_ಗಾತ್ರ(p);
    sqlite3_Memdebug_ಗಣ_Type(p, MEMTYPE_PCACHE);
    ಹಿಂತಿರುಗಿ i_ಗಾತ್ರ;
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

/*
** Allocate a new page object initially associated with cache pCache.
*/
ಸ್ಥಿರ PgHdr1 *pcache1_Alloc_ಪುಟ(PCache1 *pCache, ಪೂರ್ಣಾಂಕ benignMalloc){
  PgHdr1 *p = 0;
  ಶೂನ್ಯ *pPg;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCache->p_ಗುಂಪು->mutex) );
  ಆದರೆ( pCache->pFree || (pCache->n_ಪುಟ==0 && pcache1InitBulk(pCache)) ){
    ಪ್ರತಿಜ್ಞಾ( pCache->pFree!=0 );
    p = pCache->pFree;
    pCache->pFree = p->p_ಮುಂದಿನ;
    p->p_ಮುಂದಿನ = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ಮರಣೆ_MANAGEMENT
    /* The group mutex must be released before pcache1Alloc() is called. This
    ** is because it might call sqlite3_release_memory(), which assumes that
    ** this mutex is not held. */
    ಪ್ರತಿಜ್ಞಾ( pcache1.separateCache==0 );
    ಪ್ರತಿಜ್ಞಾ( pCache->p_ಗುಂಪು==&pcache1.grp );
    pcache1LeaveMutex(pCache->p_ಗುಂಪು);
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( benignMalloc ){ sqlite3BeginBenignMalloc(); }
    pPg = pcache1Alloc(pCache->szAlloc);
    ಆದರೆ( benignMalloc ){ sqlite3_ಕೊನೆ_Benign_Malloc(); }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ಮರಣೆ_MANAGEMENT
    pcache1EnterMutex(pCache->p_ಗುಂಪು);
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( pPg==0 ) ಹಿಂತಿರುಗಿ 0;
    p = (PgHdr1 *)&((u8 *)pPg)[pCache->sz_ಪುಟ];
    p->ಪುಟ.pBuf = pPg;
    p->ಪುಟ.pExtra = (u8*)p + ROUND8(ಗಾತ್ರ(*p));
    ಪ್ರತಿಜ್ಞಾ( EIGHT_BYTE_ALIGNMENT( p->ಪುಟ.pExtra ) );
    p->isBulkLocal = 0;
    p->isAnchor = 0;
    p->pLruPrev = 0;           /* Initializing this saves a valgrind error */
  }
  (*pCache->pnPurgeable)++;
  ಹಿಂತಿರುಗಿ p;
}

/*
** Free a page object allocated by pcache1AllocPage().
*/
ಸ್ಥಿರ ಶೂನ್ಯ pcache1_Free_ಪುಟ(PgHdr1 *p){
  PCache1 *pCache;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  pCache = p->pCache;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->pCache->p_ಗುಂಪು->mutex) );
  ಆದರೆ( p->isBulkLocal ){
    p->p_ಮುಂದಿನ = pCache->pFree;
    pCache->pFree = p;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pcache1Free(p->ಪುಟ.pBuf);
  }
  (*pCache->pnPurgeable)--;
}

/*
** Malloc function used by SQLite to obtain space from the buffer configured
** using sqlite3_config(SQLITE_CONFIG_PAGECACHE) option. If no such buffer
** exists, this function falls back to sqlite3Malloc().
*/
ಶೂನ್ಯ *sqlite3_ಪುಟ_Malloc(ಪೂರ್ಣಾಂಕ sz){
  ಪ್ರತಿಜ್ಞಾ( sz<=65536+8 ); /* These allocations are never very large */
  ಹಿಂತಿರುಗಿ pcache1Alloc(sz);
}

/*
** Free an allocated buffer obtained from sqlite3PageMalloc().
*/
ಶೂನ್ಯ sqlite3_ಪುಟ_Free(ಶೂನ್ಯ *p){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pcache1_Under_ಸ್ಮರಣೆ_Pressure(PCache1 *pCache){
  ಆದರೆ( pcache1.n_ಸ್ಥಾನ && (pCache->sz_ಪುಟ+pCache->szExtra)<=pcache1.sz_ಸ್ಥಾನ ){
    ಹಿಂತಿರುಗಿ Atomic_ತುಂಬು(&pcache1.bUnderPressure);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ sqlite3_ರಾಶಿ_Nearly_ತುಂಬಿದ();
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
ಸ್ಥಿರ ಶೂನ್ಯ pcache1ResizeHash(PCache1 *p){
  PgHdr1 **apNew;
  u64 nNew;
  u32 i;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p->p_ಗುಂಪು->mutex) );

  nNew = 2*(u64)p->nHash;
  ಆದರೆ( nNew<256 ){
    nNew = 256;
  }
  ಪ್ರತಿಜ್ಞಾ( (nNew & (nNew-1))==0 );   /* nNew is always a power of two */

  pcache1LeaveMutex(p->p_ಗುಂಪು);
  ಆದರೆ( p->nHash ){ sqlite3BeginBenignMalloc(); }
  apNew = (PgHdr1 **)sqlite3MallocZero(ಗಾತ್ರ(PgHdr1 *)*nNew);
  ಆದರೆ( p->nHash ){ sqlite3_ಕೊನೆ_Benign_Malloc(); }
  pcache1EnterMutex(p->p_ಗುಂಪು);
  ಆದರೆ( apNew ){
    ಗಾಗಿ(i=0; i<p->nHash; i++){
      PgHdr1 *p_ಪುಟ;
      PgHdr1 *p_ಮುಂದಿನ = p->apHash[i];
      ಆಗ( (p_ಪುಟ = p_ಮುಂದಿನ)!=0 ){
        ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ h = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(p_ಪುಟ->i_ಕೀಲಿ & (nNew-1));
        p_ಮುಂದಿನ = p_ಪುಟ->p_ಮುಂದಿನ;
        p_ಪುಟ->p_ಮುಂದಿನ = apNew[h];
        apNew[h] = p_ಪುಟ;
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
ಸ್ಥಿರ PgHdr1 *pcache1_Pin_ಪುಟ(PgHdr1 *p_ಪುಟ){
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ!=0 );
  ಪ್ರತಿಜ್ಞಾ( ಪುಟ_IS_UNPINNED(p_ಪುಟ) );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->p_Lru_ಮುಂದಿನ );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pLruPrev );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಪುಟ->pCache->p_ಗುಂಪು->mutex) );
  p_ಪುಟ->pLruPrev->p_Lru_ಮುಂದಿನ = p_ಪುಟ->p_Lru_ಮುಂದಿನ;
  p_ಪುಟ->p_Lru_ಮುಂದಿನ->pLruPrev = p_ಪುಟ->pLruPrev;
  p_ಪುಟ->p_Lru_ಮುಂದಿನ = 0;
  /* pPage->pLruPrev = 0;
  ** No need to clear pLruPrev as it is never accessed if pLruNext is 0 */
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->isAnchor==0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pCache->p_ಗುಂಪು->lru.isAnchor==1 );
  p_ಪುಟ->pCache->nRecyclable--;
  ಹಿಂತಿರುಗಿ p_ಪುಟ;
}


/*
** Remove the page supplied as an argument from the hash table
** (PCache1.apHash structure) that it is currently stored in.
** Also free the page if freePage is true.
**
** The PGroup mutex must be held when this function is called.
*/
ಸ್ಥಿರ ಶೂನ್ಯ pcache1RemoveFromHash(PgHdr1 *p_ಪುಟ, ಪೂರ್ಣಾಂಕ freeFlag){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ h;
  PCache1 *pCache = p_ಪುಟ->pCache;
  PgHdr1 **pp;

  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCache->p_ಗುಂಪು->mutex) );
  ಪ್ರತಿಜ್ಞಾ( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  h = p_ಪುಟ->i_ಕೀಲಿ & (pCache->nHash-1);
  ಗಾಗಿ(pp=&pCache->apHash[h]; (*pp)!=p_ಪುಟ; pp=&(*pp)->p_ಮುಂದಿನ);
  *pp = (*pp)->p_ಮುಂದಿನ;

  pCache->n_ಪುಟ--;
  ಆದರೆ( freeFlag ) pcache1_Free_ಪುಟ(p_ಪುಟ);
}

/*
** If there are currently more than nMaxPage pages allocated, try
** to recycle pages to reduce the number allocated to nMaxPage.
*/
ಸ್ಥಿರ ಶೂನ್ಯ pcache1_Enforce_Max_ಪುಟ(PCache1 *pCache){
  PGroup *p_ಗುಂಪು = pCache->p_ಗುಂಪು;
  PgHdr1 *p;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(p_ಗುಂಪು->mutex) );
  ಆಗ( p_ಗುಂಪು->nPurgeable>p_ಗುಂಪು->n_Max_ಪುಟ
      && (p=p_ಗುಂಪು->lru.pLruPrev)->isAnchor==0
  ){
    ಪ್ರತಿಜ್ಞಾ( p->pCache->p_ಗುಂಪು==p_ಗುಂಪು );
    ಪ್ರತಿಜ್ಞಾ( ಪುಟ_IS_UNPINNED(p) );
    pcache1_Pin_ಪುಟ(p);
    pcache1RemoveFromHash(p, 1);
  }
  ಆದರೆ( pCache->n_ಪುಟ==0 && pCache->pBulk ){
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
ಸ್ಥಿರ ಶೂನ್ಯ pcache1TruncateUnsafe(
  PCache1 *pCache,             /* The cache to truncate */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i_ಎಲ್ಲೆ          /* Drop pages with this pgno or larger */
){
  TESTONLY( ಪೂರ್ಣಾಂಕ n_ಪುಟ = 0; )  /* To assert pCache->nPage is correct */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ h, iStop;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_held(pCache->p_ಗುಂಪು->mutex) );
  ಪ್ರತಿಜ್ಞಾ( pCache->i_Max_ಕೀಲಿ >= i_ಎಲ್ಲೆ );
  ಪ್ರತಿಜ್ಞಾ( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  ಆದರೆ( pCache->i_Max_ಕೀಲಿ - i_ಎಲ್ಲೆ < pCache->nHash ){
    /* If we are just shaving the last few pages off the end of the
    ** cache, then there is no point in scanning the entire hash table.
    ** Only scan those hash slots that might contain pages that need to
    ** be removed. */
    h = i_ಎಲ್ಲೆ & (pCache->nHash-1);
    iStop = pCache->i_Max_ಕೀಲಿ & (pCache->nHash-1);
    TESTONLY( n_ಪುಟ = -10; )  /* Disable the pCache->nPage validity check */
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* This is the general case where many pages are being removed.
    ** It is necessary to scan the entire hash table */
    h = pCache->nHash/2;
    iStop = h - 1;
  }
  ಗಾಗಿ(;;){
    PgHdr1 **pp;
    PgHdr1 *p_ಪುಟ;
    ಪ್ರತಿಜ್ಞಾ( h<pCache->nHash );
    pp = &pCache->apHash[h];
    ಆಗ( (p_ಪುಟ = *pp)!=0 ){
      ಆದರೆ( p_ಪುಟ->i_ಕೀಲಿ>=i_ಎಲ್ಲೆ ){
        pCache->n_ಪುಟ--;
        *pp = p_ಪುಟ->p_ಮುಂದಿನ;
        ಆದರೆ( ಪುಟ_IS_UNPINNED(p_ಪುಟ) ) pcache1_Pin_ಪುಟ(p_ಪುಟ);
        pcache1_Free_ಪುಟ(p_ಪುಟ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pp = &p_ಪುಟ->p_ಮುಂದಿನ;
        TESTONLY( ಆದರೆ( n_ಪುಟ>=0 ) n_ಪುಟ++; )
      }
    }
    ಆದರೆ( h==iStop ) ನಿಲ್ಲಿಸು;
    h = (h+1) & (pCache->nHash-1);
  }
  ಪ್ರತಿಜ್ಞಾ( n_ಪುಟ<0 || pCache->n_ಪುಟ==(ಚಿಹ್ನೆರಹಿತ)n_ಪುಟ );
}

/******************************************************************************/
/******** sqlite3_pcache Methods **********************************************/

/*
** Implementation of the sqlite3_pcache.xInit method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pcache1Init(ಶೂನ್ಯ *Not_ಬಳಸಿದ){
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  ಪ್ರತಿಜ್ಞಾ( pcache1.isInit==0 );
  memset(&pcache1, 0, ಗಾತ್ರ(pcache1));


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
#ಆದರೆ defined(SQLITE_ENABLE_ಸ್ಮರಣೆ_MANAGEMENT)
  pcache1.separateCache = 0;
#ಅಥವಾಆದರೆ SQLITE_THREADSAFE
  pcache1.separateCache = sqlite3GlobalConfig.p_ಪುಟ==0
                          || sqlite3GlobalConfig.bCoreMutex>0;
#ಇಲ್ಲದಿದ್ದರೆ
  pcache1.separateCache = sqlite3GlobalConfig.p_ಪುಟ==0;
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ SQLITE_THREADSAFE
  ಆದರೆ( sqlite3GlobalConfig.bCoreMutex ){
    pcache1.grp.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_LRU);
    pcache1.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PMEM);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( pcache1.separateCache
   && sqlite3GlobalConfig.n_ಪುಟ!=0
   && sqlite3GlobalConfig.p_ಪುಟ==0
  ){
    pcache1.n_Init_ಪುಟ = sqlite3GlobalConfig.n_ಪುಟ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pcache1.n_Init_ಪುಟ = 0;
  }
  pcache1.grp.mxPinned = 10;
  pcache1.isInit = 1;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Implementation of the sqlite3_pcache.xShutdown method.
** Note that the static mutex allocated in xInit does
** not need to be freed.
*/
ಸ್ಥಿರ ಶೂನ್ಯ pcache1Shutdown(ಶೂನ್ಯ *Not_ಬಳಸಿದ){
  ಬಳಸದ_PARAMETER(Not_ಬಳಸಿದ);
  ಪ್ರತಿಜ್ಞಾ( pcache1.isInit!=0 );
  memset(&pcache1, 0, ಗಾತ್ರ(pcache1));
}

/* forward declaration */
ಸ್ಥಿರ ಶೂನ್ಯ pcache1Destroy(sqlite3_pcache *p);

/*
** Implementation of the sqlite3_pcache.xCreate method.
**
** Allocate a new cache.
*/
ಸ್ಥಿರ sqlite3_pcache *pcache1Create(ಪೂರ್ಣಾಂಕ sz_ಪುಟ, ಪೂರ್ಣಾಂಕ szExtra, ಪೂರ್ಣಾಂಕ bPurgeable){
  PCache1 *pCache;      /* The newly created page cache */
  PGroup *p_ಗುಂಪು;       /* The group the new page cache will belong to */
  i64 sz;               /* Bytes of memory required to allocate the new cache */

  ಪ್ರತಿಜ್ಞಾ( (sz_ಪುಟ & (sz_ಪುಟ-1))==0 && sz_ಪುಟ>=512 && sz_ಪುಟ<=65536 );
  ಪ್ರತಿಜ್ಞಾ( szExtra < 300 );

  sz = ಗಾತ್ರ(PCache1) + ಗಾತ್ರ(PGroup)*pcache1.separateCache;
  pCache = (PCache1 *)sqlite3MallocZero(sz);
  ಆದರೆ( pCache ){
    ಆದರೆ( pcache1.separateCache ){
      p_ಗುಂಪು = (PGroup*)&pCache[1];
      p_ಗುಂಪು->mxPinned = 10;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಗುಂಪು = &pcache1.grp;
    }
    pcache1EnterMutex(p_ಗುಂಪು);
    ಆದರೆ( p_ಗುಂಪು->lru.isAnchor==0 ){
      p_ಗುಂಪು->lru.isAnchor = 1;
      p_ಗುಂಪು->lru.pLruPrev = p_ಗುಂಪು->lru.p_Lru_ಮುಂದಿನ = &p_ಗುಂಪು->lru;
    }
    pCache->p_ಗುಂಪು = p_ಗುಂಪು;
    pCache->sz_ಪುಟ = sz_ಪುಟ;
    pCache->szExtra = szExtra;
    pCache->szAlloc = sz_ಪುಟ + szExtra + ROUND8(ಗಾತ್ರ(PgHdr1));
    pCache->bPurgeable = (bPurgeable ? 1 : 0);
    pcache1ResizeHash(pCache);
    ಆದರೆ( bPurgeable ){
      pCache->nMin = 10;
      p_ಗುಂಪು->n_Min_ಪುಟ += pCache->nMin;
      p_ಗುಂಪು->mxPinned = p_ಗುಂಪು->n_Max_ಪುಟ + 10 - p_ಗುಂಪು->n_Min_ಪುಟ;
      pCache->pnPurgeable = &p_ಗುಂಪು->nPurgeable;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pCache->pnPurgeable = &pCache->nPurgeableDummy;
    }
    pcache1LeaveMutex(p_ಗುಂಪು);
    ಆದರೆ( pCache->nHash==0 ){
      pcache1Destroy((sqlite3_pcache*)pCache);
      pCache = 0;
    }
  }
  ಹಿಂತಿರುಗಿ (sqlite3_pcache *)pCache;
}

/*
** Implementation of the sqlite3_pcache.xCachesize method.
**
** Configure the cache_size limit for a cache.
*/
ಸ್ಥಿರ ಶೂನ್ಯ pcache1Cachesize(sqlite3_pcache *p, ಪೂರ್ಣಾಂಕ nMax){
  PCache1 *pCache = (PCache1 *)p;
  u32 n;
  ಪ್ರತಿಜ್ಞಾ( nMax>=0 );
  ಆದರೆ( pCache->bPurgeable ){
    PGroup *p_ಗುಂಪು = pCache->p_ಗುಂಪು;
    pcache1EnterMutex(p_ಗುಂಪು);
    n = (u32)nMax;
    ಆದರೆ( n > 0x7fff0000 - p_ಗುಂಪು->n_Max_ಪುಟ + pCache->nMax ){
      n = 0x7fff0000 - p_ಗುಂಪು->n_Max_ಪುಟ + pCache->nMax;
    }
    p_ಗುಂಪು->n_Max_ಪುಟ += (n - pCache->nMax);
    p_ಗುಂಪು->mxPinned = p_ಗುಂಪು->n_Max_ಪುಟ + 10 - p_ಗುಂಪು->n_Min_ಪುಟ;
    pCache->nMax = n;
    pCache->n90pct = pCache->nMax*9/10;
    pcache1_Enforce_Max_ಪುಟ(pCache);
    pcache1LeaveMutex(p_ಗುಂಪು);
  }
}

/*
** Implementation of the sqlite3_pcache.xShrink method.
**
** Free up as much memory as possible.
*/
ಸ್ಥಿರ ಶೂನ್ಯ pcache1Shrink(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1*)p;
  ಆದರೆ( pCache->bPurgeable ){
    PGroup *p_ಗುಂಪು = pCache->p_ಗುಂಪು;
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ saved_Max_ಪುಟ;
    pcache1EnterMutex(p_ಗುಂಪು);
    saved_Max_ಪುಟ = p_ಗುಂಪು->n_Max_ಪುಟ;
    p_ಗುಂಪು->n_Max_ಪುಟ = 0;
    pcache1_Enforce_Max_ಪುಟ(pCache);
    p_ಗುಂಪು->n_Max_ಪುಟ = saved_Max_ಪುಟ;
    pcache1LeaveMutex(p_ಗುಂಪು);
  }
}

/*
** Implementation of the sqlite3_pcache.xPagecount method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ pcache1Pagecount(sqlite3_pcache *p){
  ಪೂರ್ಣಾಂಕ n;
  PCache1 *pCache = (PCache1*)p;
  pcache1EnterMutex(pCache->p_ಗುಂಪು);
  n = pCache->n_ಪುಟ;
  pcache1LeaveMutex(pCache->p_ಗುಂಪು);
  ಹಿಂತಿರುಗಿ n;
}


/*
** Implement steps 3, 4, and 5 of the pcache1Fetch() algorithm described
** in the header of the pcache1Fetch() procedure.
**
** This steps are broken out into a separate procedure because they are
** usually not needed, and by avoiding the stack initialization required
** for these steps, the main pcache1Fetch() procedure can run faster.
*/
ಸ್ಥಿರ SQLITE_NOINLINE PgHdr1 *pcache1FetchStage2(
  PCache1 *pCache,
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i_ಕೀಲಿ,
  ಪೂರ್ಣಾಂಕ createFlag
){
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nPinned;
  PGroup *p_ಗುಂಪು = pCache->p_ಗುಂಪು;
  PgHdr1 *p_ಪುಟ = 0;

  /* Step 3: Abort if createFlag is 1 but the cache is nearly full */
  ಪ್ರತಿಜ್ಞಾ( pCache->n_ಪುಟ >= pCache->nRecyclable );
  nPinned = pCache->n_ಪುಟ - pCache->nRecyclable;
  ಪ್ರತಿಜ್ಞಾ( p_ಗುಂಪು->mxPinned == p_ಗುಂಪು->n_Max_ಪುಟ + 10 - p_ಗುಂಪು->n_Min_ಪುಟ );
  ಪ್ರತಿಜ್ಞಾ( pCache->n90pct == pCache->nMax*9/10 );
  ಆದರೆ( createFlag==1 && (
        nPinned>=p_ಗುಂಪು->mxPinned
     || nPinned>=pCache->n90pct
     || (pcache1_Under_ಸ್ಮರಣೆ_Pressure(pCache) && pCache->nRecyclable<nPinned)
  )){
    ಹಿಂತಿರುಗಿ 0;
  }

  ಆದರೆ( pCache->n_ಪುಟ>=pCache->nHash ) pcache1ResizeHash(pCache);
  ಪ್ರತಿಜ್ಞಾ( pCache->nHash>0 && pCache->apHash );
  ಪ್ರತಿಜ್ಞಾ( (pCache->nHash & (pCache->nHash-1))==0 );

  /* Step 4. Try to recycle a page. */
  ಆದರೆ( pCache->bPurgeable
   && !p_ಗುಂಪು->lru.pLruPrev->isAnchor
   && ((pCache->n_ಪುಟ+1>=pCache->nMax) || pcache1_Under_ಸ್ಮರಣೆ_Pressure(pCache))
  ){
    PCache1 *pOther;
    p_ಪುಟ = p_ಗುಂಪು->lru.pLruPrev;
    ಪ್ರತಿಜ್ಞಾ( ಪುಟ_IS_UNPINNED(p_ಪುಟ) );
    pcache1RemoveFromHash(p_ಪುಟ, 0);
    pcache1_Pin_ಪುಟ(p_ಪುಟ);
    pOther = p_ಪುಟ->pCache;
    ಆದರೆ( pOther->szAlloc != pCache->szAlloc ){
      pcache1_Free_ಪುಟ(p_ಪುಟ);
      p_ಪುಟ = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಗುಂಪು->nPurgeable -= (pOther->bPurgeable - pCache->bPurgeable);
    }
  }

  /* Step 5. If a usable page buffer has still not been found,
  ** attempt to allocate a new one.
  */
  ಆದರೆ( !p_ಪುಟ ){
    p_ಪುಟ = pcache1_Alloc_ಪುಟ(pCache, createFlag==1);
  }

  ಆದರೆ( p_ಪುಟ ){
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ h = i_ಕೀಲಿ & (pCache->nHash-1);
    pCache->n_ಪುಟ++;
    p_ಪುಟ->i_ಕೀಲಿ = i_ಕೀಲಿ;
    p_ಪುಟ->p_ಮುಂದಿನ = pCache->apHash[h];
    p_ಪುಟ->pCache = pCache;
    p_ಪುಟ->p_Lru_ಮುಂದಿನ = 0;
    /* pPage->pLruPrev = 0;
    ** No need to clear pLruPrev since it is not accessed when pLruNext==0 */
    *(ಶೂನ್ಯ **)p_ಪುಟ->ಪುಟ.pExtra = 0;
    pCache->apHash[h] = p_ಪುಟ;
    ಆದರೆ( i_ಕೀಲಿ>pCache->i_Max_ಕೀಲಿ ){
      pCache->i_Max_ಕೀಲಿ = i_ಕೀಲಿ;
    }
  }
  ಹಿಂತಿರುಗಿ p_ಪುಟ;
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
ಸ್ಥಿರ PgHdr1 *pcache1FetchNoMutex(
  sqlite3_pcache *p,
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i_ಕೀಲಿ,
  ಪೂರ್ಣಾಂಕ createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_ಪುಟ = 0;

  /* Step 1: Search the hash table for an existing entry.  nHash is always
  ** a power of two when the cache is in use (see pcache1ResizeHash()), so
  ** the modulo reduces to a mask, avoiding a hardware divide. */
  ಪ್ರತಿಜ್ಞಾ( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  p_ಪುಟ = pCache->apHash[i_ಕೀಲಿ & (pCache->nHash-1u)];
  ಆಗ( p_ಪುಟ && p_ಪುಟ->i_ಕೀಲಿ!=i_ಕೀಲಿ ){ p_ಪುಟ = p_ಪುಟ->p_ಮುಂದಿನ; }

  /* Step 2: If the page was found in the hash table, then return it.
  ** If the page was not in the hash table and createFlag is 0, abort.
  ** Otherwise (page not in hash and createFlag!=0) continue with
  ** subsequent steps to try to create the page. */
  ಆದರೆ( p_ಪುಟ ){
    ಆದರೆ( ಪುಟ_IS_UNPINNED(p_ಪುಟ) ){
      ಹಿಂತಿರುಗಿ pcache1_Pin_ಪುಟ(p_ಪುಟ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಹಿಂತಿರುಗಿ p_ಪುಟ;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( createFlag ){
    /* Steps 3, 4, and 5 implemented by this subroutine */
    ಹಿಂತಿರುಗಿ pcache1FetchStage2(pCache, i_ಕೀಲಿ, createFlag);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
  }
}
#ಆದರೆ PCACHE1_MIGHT_USE_ಗುಂಪು_MUTEX
ಸ್ಥಿರ PgHdr1 *pcache1FetchWithMutex(
  sqlite3_pcache *p,
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i_ಕೀಲಿ,
  ಪೂರ್ಣಾಂಕ createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_ಪುಟ;

  pcache1EnterMutex(pCache->p_ಗುಂಪು);
  p_ಪುಟ = pcache1FetchNoMutex(p, i_ಕೀಲಿ, createFlag);
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ==0 || pCache->i_Max_ಕೀಲಿ>=i_ಕೀಲಿ );
  pcache1LeaveMutex(pCache->p_ಗುಂಪು);
  ಹಿಂತಿರುಗಿ p_ಪುಟ;
}
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ sqlite3_pcache_ಪುಟ *pcache1Fetch(
  sqlite3_pcache *p,
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i_ಕೀಲಿ,
  ಪೂರ್ಣಾಂಕ createFlag
){
#ಆದರೆ PCACHE1_MIGHT_USE_ಗುಂಪು_MUTEX || defined(SQLITE_DEBUG)
  PCache1 *pCache = (PCache1 *)p;
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( offsetof(PgHdr1,ಪುಟ)==0 );
  ಪ್ರತಿಜ್ಞಾ( pCache->bPurgeable || createFlag!=1 );
  ಪ್ರತಿಜ್ಞಾ( pCache->bPurgeable || pCache->nMin==0 );
  ಪ್ರತಿಜ್ಞಾ( pCache->bPurgeable==0 || pCache->nMin==10 );
  ಪ್ರತಿಜ್ಞಾ( pCache->nMin==0 || pCache->bPurgeable );
  ಪ್ರತಿಜ್ಞಾ( pCache->nHash>0 );
#ಆದರೆ PCACHE1_MIGHT_USE_ಗುಂಪು_MUTEX
  ಆದರೆ( pCache->p_ಗುಂಪು->mutex ){
    ಹಿಂತಿರುಗಿ (sqlite3_pcache_ಪುಟ*)pcache1FetchWithMutex(p, i_ಕೀಲಿ, createFlag);
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
  {
    ಹಿಂತಿರುಗಿ (sqlite3_pcache_ಪುಟ*)pcache1FetchNoMutex(p, i_ಕೀಲಿ, createFlag);
  }
}


/*
** Implementation of the sqlite3_pcache.xUnpin method.
**
** Mark a page as unpinned (eligible for asynchronous recycling).
*/
ಸ್ಥಿರ ಶೂನ್ಯ pcache1Unpin(
  sqlite3_pcache *p,
  sqlite3_pcache_ಪುಟ *pPg,
  ಪೂರ್ಣಾಂಕ reuseUnlikely
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_ಪುಟ = (PgHdr1 *)pPg;
  PGroup *p_ಗುಂಪು = pCache->p_ಗುಂಪು;

  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pCache==pCache );
  pcache1EnterMutex(p_ಗುಂಪು);

  /* It is an error to call this function if the page is already
  ** part of the PGroup LRU list.
  */
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->p_Lru_ಮುಂದಿನ==0 );
  ಪ್ರತಿಜ್ಞಾ( ಪುಟ_IS_PINNED(p_ಪುಟ) );

  ಆದರೆ( reuseUnlikely || p_ಗುಂಪು->nPurgeable>p_ಗುಂಪು->n_Max_ಪುಟ ){
    pcache1RemoveFromHash(p_ಪುಟ, 1);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Add the page to the PGroup LRU list. */
    PgHdr1 **pp_ಮೊದಲ = &p_ಗುಂಪು->lru.p_Lru_ಮುಂದಿನ;
    p_ಪುಟ->pLruPrev = &p_ಗುಂಪು->lru;
    (p_ಪುಟ->p_Lru_ಮುಂದಿನ = *pp_ಮೊದಲ)->pLruPrev = p_ಪುಟ;
    *pp_ಮೊದಲ = p_ಪುಟ;
    pCache->nRecyclable++;
  }

  pcache1LeaveMutex(pCache->p_ಗುಂಪು);
}

/*
** Implementation of the sqlite3_pcache.xRekey method.
*/
ಸ್ಥಿರ ಶೂನ್ಯ pcache1Rekey(
  sqlite3_pcache *p,
  sqlite3_pcache_ಪುಟ *pPg,
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ iOld,
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ iNew
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_ಪುಟ = (PgHdr1 *)pPg;
  PgHdr1 **pp;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ hOld, hNew;
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->i_ಕೀಲಿ==iOld );
  ಪ್ರತಿಜ್ಞಾ( p_ಪುಟ->pCache==pCache );
  ಪ್ರತಿಜ್ಞಾ( iOld!=iNew );               /* The page number really is changing */

  pcache1EnterMutex(pCache->p_ಗುಂಪು);

  ಪ್ರತಿಜ್ಞಾ( pcache1FetchNoMutex(p, iOld, 0)==p_ಪುಟ ); /* pPg really is iOld */
  ಪ್ರತಿಜ್ಞಾ( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  hOld = iOld & (pCache->nHash-1);
  pp = &pCache->apHash[hOld];
  ಆಗ( (*pp)!=p_ಪುಟ ){
    pp = &(*pp)->p_ಮುಂದಿನ;
  }
  *pp = p_ಪುಟ->p_ಮುಂದಿನ;

  ಪ್ರತಿಜ್ಞಾ( pcache1FetchNoMutex(p, iNew, 0)==0 ); /* iNew not in cache */
  hNew = iNew & (pCache->nHash-1);
  p_ಪುಟ->i_ಕೀಲಿ = iNew;
  p_ಪುಟ->p_ಮುಂದಿನ = pCache->apHash[hNew];
  pCache->apHash[hNew] = p_ಪುಟ;
  ಆದರೆ( iNew>pCache->i_Max_ಕೀಲಿ ){
    pCache->i_Max_ಕೀಲಿ = iNew;
  }

  pcache1LeaveMutex(pCache->p_ಗುಂಪು);
}

/*
** Implementation of the sqlite3_pcache.xTruncate method.
**
** Discard all unpinned pages in the cache with a page number equal to
** or greater than parameter iLimit. Any pinned pages with a page number
** equal to or greater than iLimit are implicitly unpinned.
*/
ಸ್ಥಿರ ಶೂನ್ಯ pcache1Truncate(sqlite3_pcache *p, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i_ಎಲ್ಲೆ){
  PCache1 *pCache = (PCache1 *)p;
  pcache1EnterMutex(pCache->p_ಗುಂಪು);
  ಆದರೆ( i_ಎಲ್ಲೆ<=pCache->i_Max_ಕೀಲಿ ){
    pcache1TruncateUnsafe(pCache, i_ಎಲ್ಲೆ);
    pCache->i_Max_ಕೀಲಿ = i_ಎಲ್ಲೆ-1;
  }
  pcache1LeaveMutex(pCache->p_ಗುಂಪು);
}

/*
** Implementation of the sqlite3_pcache.xDestroy method.
**
** Destroy a cache allocated using pcache1Create().
*/
ಸ್ಥಿರ ಶೂನ್ಯ pcache1Destroy(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1 *)p;
  PGroup *p_ಗುಂಪು = pCache->p_ಗುಂಪು;
  ಪ್ರತಿಜ್ಞಾ( pCache->bPurgeable || (pCache->nMax==0 && pCache->nMin==0) );
  pcache1EnterMutex(p_ಗುಂಪು);
  ಆದರೆ( pCache->n_ಪುಟ ) pcache1TruncateUnsafe(pCache, 0);
  ಪ್ರತಿಜ್ಞಾ( p_ಗುಂಪು->n_Max_ಪುಟ >= pCache->nMax );
  p_ಗುಂಪು->n_Max_ಪುಟ -= pCache->nMax;
  ಪ್ರತಿಜ್ಞಾ( p_ಗುಂಪು->n_Min_ಪುಟ >= pCache->nMin );
  p_ಗುಂಪು->n_Min_ಪುಟ -= pCache->nMin;
  p_ಗುಂಪು->mxPinned = p_ಗುಂಪು->n_Max_ಪುಟ + 10 - p_ಗುಂಪು->n_Min_ಪುಟ;
  pcache1_Enforce_Max_ಪುಟ(pCache);
  pcache1LeaveMutex(p_ಗುಂಪು);
  sqlite3_free(pCache->pBulk);
  sqlite3_free(pCache->apHash);
  sqlite3_free(pCache);
}

/*
** This function is called during initialization (sqlite3_initialize()) to
** install the default pluggable cache module, assuming the user has not
** already provided an alternative.
*/
ಶೂನ್ಯ sqlite3_PCache_ಗಣ_Default(ಶೂನ್ಯ){
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_pcache_methods2 defaultMethods = {
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
ಪೂರ್ಣಾಂಕ sqlite3_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ_Pcache1(ಶೂನ್ಯ){ ಹಿಂತಿರುಗಿ ROUND8(ಗಾತ್ರ(PgHdr1)); }

/*
** Return the global mutex used by this PCACHE implementation.  The
** sqlite3_status() routine needs access to this mutex.
*/
sqlite3_mutex *sqlite3Pcache1Mutex(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ pcache1.mutex;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ಮರಣೆ_MANAGEMENT
/*
** This function is called to free superfluous dynamically allocated memory
** held by the pager system. Memory in use by any SQLite pager allocated
** by the current thread may be sqlite3_free()ed.
**
** nReq is the number of bytes of memory required. Once this much has
** been released, the function returns. The return value is the total number
** of bytes of memory released.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Pcache_Release_ಸ್ಮರಣೆ(ಪೂರ್ಣಾಂಕ nReq){
  ಪೂರ್ಣಾಂಕ nFree = 0;
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_mutex_notheld(pcache1.mutex) );
  ಆದರೆ( sqlite3GlobalConfig.p_ಪುಟ==0 ){
    PgHdr1 *p;
    pcache1EnterMutex(&pcache1.grp);
    ಆಗ( (nReq<0 || nFree<nReq)
       &&  (p=pcache1.grp.lru.pLruPrev)!=0
       &&  p->isAnchor==0
    ){
      nFree += pcache1_Mem_ಗಾತ್ರ(p->ಪುಟ.pBuf);
      ಪ್ರತಿಜ್ಞಾ( ಪುಟ_IS_UNPINNED(p) );
      pcache1_Pin_ಪುಟ(p);
      pcache1RemoveFromHash(p, 1);
    }
    pcache1LeaveMutex(&pcache1.grp);
  }
  ಹಿಂತಿರುಗಿ nFree;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
/*
** This function is used by test procedures to inspect the internal state
** of the global cache.
*/
ಶೂನ್ಯ sqlite3PcacheStats(
  ಪೂರ್ಣಾಂಕ *pnCurrent,      /* OUT: Total number of pages cached */
  ಪೂರ್ಣಾಂಕ *pnMax,          /* OUT: Global maximum cache size */
  ಪೂರ್ಣಾಂಕ *pnMin,          /* OUT: Sum of PCache1.nMin for purgeable caches */
  ಪೂರ್ಣಾಂಕ *pnRecyclable    /* OUT: Total number of pages available for recycling */
){
  PgHdr1 *p;
  ಪೂರ್ಣಾಂಕ nRecyclable = 0;
  ಗಾಗಿ(p=pcache1.grp.lru.p_Lru_ಮುಂದಿನ; p && !p->isAnchor; p=p->p_Lru_ಮುಂದಿನ){
    ಪ್ರತಿಜ್ಞಾ( ಪುಟ_IS_UNPINNED(p) );
    nRecyclable++;
  }
  *pnCurrent = pcache1.grp.nPurgeable;
  *pnMax = (ಪೂರ್ಣಾಂಕ)pcache1.grp.n_Max_ಪುಟ;
  *pnMin = (ಪೂರ್ಣಾಂಕ)pcache1.grp.n_Min_ಪುಟ;
  *pnRecyclable = nRecyclable;
}
#ಆದರೆ_ಅಂತ್ಯ
