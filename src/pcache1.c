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
#شامل "sqliteInt.h"

قسمتعریف ساخت PCache1 PCache1;
قسمتعریف ساخت PgHdr1 PgHdr1;
قسمتعریف ساخت PgFreeslot PgFreeslot;
قسمتعریف ساخت PGroup PGroup;

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
ساخت PgHdr1 {
  sqlite3_pcache_صفحہ صفحہ; /* Base class. Must be first. pBuf & pExtra */
  غیرعلامتی صحیح i_کلید;        /* Key value (page number) */
  u16 isBulkLocal;          /* This page from bulk local storage */
  u16 isAnchor;             /* This is the PGroup.lru element */
  PgHdr1 *p_اگلا;            /* Next in hash table chain */
  PCache1 *pCache;          /* Cache that currently owns this page */
  PgHdr1 *p_Lru_اگلا;         /* Next in circular LRU list of unpinned pages */
  PgHdr1 *p_Lru_پچھلا;         /* Previous in LRU list of unpinned pages */
                            /* NB: pLruPrev is only valid if pLruNext!=0 */
};

/*
** A page is pinned if it is not on the LRU list.  To be "pinned" means
** that the page is in active use and must not be deallocated.
*/
#تعریف_کرو صفحہ_IS_PINNED(p)    ((p)->p_Lru_اگلا==0)
#تعریف_کرو صفحہ_IS_UNPINNED(p)  ((p)->p_Lru_اگلا!=0)

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
ساخت PGroup {
  sqlite3_mutex *mutex;          /* MUTEX_STATIC_LRU or NULL */
  غیرعلامتی صحیح n_Max_صفحہ;         /* Sum of nMax for purgeable caches */
  غیرعلامتی صحیح n_Min_صفحہ;         /* Sum of nMin for purgeable caches */
  غیرعلامتی صحیح mxPinned;         /* nMaxpage + 10 - nMinPage */
  غیرعلامتی صحیح nPurgeable;       /* Number of purgeable pages allocated */
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
ساخت PCache1 {
  /* Cache configuration parameters. Page size (szPage) and the purgeable
  ** flag (bPurgeable) and the pnPurgeable pointer are all set when the
  ** cache is created and are never changed thereafter. nMax may be
  ** modified at any time by a call to the pcache1Cachesize() method.
  ** The PGroup mutex must be held when accessing nMax.
  */
  PGroup *p_گروہ;                     /* PGroup this cache belongs to */
  غیرعلامتی صحیح *pnPurgeable;          /* Pointer to pGroup->nPurgeable */
  صحیح sz_صفحہ;                         /* Size of database content section */
  صحیح szExtra;                        /* sizeof(MemPage)+sizeof(PgHdr) */
  صحیح szAlloc;                        /* Total size of one pcache line */
  صحیح bPurgeable;                     /* True if cache is purgeable */
  غیرعلامتی صحیح nMin;                  /* Minimum number of pages reserved */
  غیرعلامتی صحیح nMax;                  /* Configured "cache_size" value */
  غیرعلامتی صحیح n90pct;                /* nMax*9/10 */
  غیرعلامتی صحیح i_Max_کلید;               /* Largest key seen since xTruncate() */
  غیرعلامتی صحیح nPurgeableDummy;       /* pnPurgeable points here when not used*/

  /* Hash table of all pages. The following variables may only be accessed
  ** when the accessor is holding the PGroup mutex.
  */
  غیرعلامتی صحیح nRecyclable;           /* Number of pages in the LRU list */
  غیرعلامتی صحیح n_صفحہ;                 /* Total number of pages in apHash */
  غیرعلامتی صحیح nHash;                 /* Number of slots in apHash[] */
  PgHdr1 **apHash;                    /* Hash table for fast lookup by key */
  PgHdr1 *pFree;                      /* List of unused pcache-local pages */
  خالی *pBulk;                        /* Bulk memory used by pcache-local */
};

/*
** Free slots in the allocator used to divide up the global page cache
** buffer provided using the SQLITE_CONFIG_PAGECACHE mechanism.
*/
ساخت PgFreeslot {
  PgFreeslot *p_اگلا;  /* Next free slot */
};

/*
** Global data used by this cache.
*/
ساکن SQLITE_WSD ساخت PCacheGlobal {
  PGroup grp;                    /* The global PGroup for mode (2) */

  /* Variables related to SQLITE_CONFIG_PAGECACHE settings.  The
  ** szSlot, nSlot, pStart, pEnd, nReserve, and isInit values are all
  ** fixed at sqlite3_initialize() time and do not require mutex protection.
  ** The nFreeSlot and pFree values do require mutex protection.
  */
  صحیح isInit;                    /* True if initialized */
  صحیح separateCache;             /* Use a new PGroup for each PCache */
  صحیح n_Init_صفحہ;                 /* Initial bulk allocation size */
  صحیح sz_خانہ;                    /* Size of each free slot */
  صحیح n_خانہ;                     /* The number of pcache slots */
  صحیح nReserve;                  /* Try to keep nFreeSlot above this */
  خالی *p_آغاز, *p_انتہا;           /* Bounds of global page cache memory */
  /* Above requires no mutex.  Use mutex below for variable that follow. */
  sqlite3_mutex *mutex;          /* Mutex for accessing the following: */
  PgFreeslot *pFree;             /* Free page blocks */
  صحیح n_Free_خانہ;                 /* Number of unused pcache slots */
  صحیح bUnderPressure;            /* True if low on PAGECACHE memory */
} pcache1_g;

/*
** All code in this file should access the global structure above via the
** alias "pcache1". This ensures that the WSD emulation is used when
** compiling for systems that do not support real WSD.
*/
#تعریف_کرو pcache1 (GLOBAL(ساخت PCacheGlobal, pcache1_g))

/*
** Macros to enter and leave the PCache LRU mutex.
*/
#اگر !defined(SQLITE_ENABLE_حافظہ_MANAGEMENT) || SQLITE_THREADSAFE==0
# define pcache1EnterMutex(X)  أكد((X)->mutex==0)
# define pcache1LeaveMutex(X)  أكد((X)->mutex==0)
# define PCACHE1_MIGHT_USE_گروہ_MUTEX 0
#ورنہ
# define pcache1EnterMutex(X) sqlite3_mutex_enter((X)->mutex)
# define pcache1LeaveMutex(X) sqlite3_mutex_leave((X)->mutex)
# define PCACHE1_MIGHT_USE_گروہ_MUTEX 1
#اگر_ختم

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
خالی sqlite3_PCache_بفر_تیاری(خالی *pBuf, صحیح sz, صحیح n){
  اگر( pcache1.isInit ){
    PgFreeslot *p;
    اگر( pBuf==0 ) sz = n = 0;
    اگر( n==0 ) sz = 0;
    sz = ROUNDDOWN8(sz);
    pcache1.sz_خانہ = sz;
    pcache1.n_خانہ = pcache1.n_Free_خانہ = n;
    pcache1.nReserve = n>90 ? 10 : (n/10 + 1);
    pcache1.p_آغاز = pBuf;
    pcache1.pFree = 0;
    AtomicStore(&pcache1.bUnderPressure,0);
    جب( n-- ){
      p = (PgFreeslot*)pBuf;
      p->p_اگلا = pcache1.pFree;
      pcache1.pFree = p;
      pBuf = (خالی*)&((حرف*)pBuf)[sz];
    }
    pcache1.p_انتہا = pBuf;
  }
}

/*
** Try to initialize the pCache->pFree and pCache->pBulk fields.  Return
** true if pCache->pFree ends up containing one or more free pages.
*/
ساکن صحیح pcache1InitBulk(PCache1 *pCache){
  i64 szBulk;
  حرف *zBulk;
  اگر( pcache1.n_Init_صفحہ==0 ) واپس 0;
  /* Do not bother with a bulk allocation if the cache size very small */
  اگر( pCache->nMax<3 ) واپس 0;
  sqlite3BeginBenignMalloc();
  اگر( pcache1.n_Init_صفحہ>0 ){
    szBulk = pCache->szAlloc * (i64)pcache1.n_Init_صفحہ;
  }ورنہ{
    szBulk = -1024 * (i64)pcache1.n_Init_صفحہ;
  }
  اگر( szBulk > pCache->szAlloc*(i64)pCache->nMax ){
    szBulk = pCache->szAlloc*(i64)pCache->nMax;
  }
  اگر( szBulk>=pCache->szAlloc ){
    zBulk = pCache->pBulk = sqlite3Malloc( szBulk );
    sqlite3_انتہا_Benign_Malloc();
    اگر( zBulk ){
      صحیح nBulk = sqlite3_Malloc_سائز(zBulk)/pCache->szAlloc;
      کرو{
        PgHdr1 *pX = (PgHdr1*)&zBulk[pCache->sz_صفحہ];
        pX->صفحہ.pBuf = zBulk;
        pX->صفحہ.pExtra = (u8*)pX + ROUND8(حجم(*pX));
        أكد( EIGHT_BYTE_ALIGNMENT( pX->صفحہ.pExtra ) );
        pX->isBulkLocal = 1;
        pX->isAnchor = 0;
        pX->p_اگلا = pCache->pFree;
        pX->p_Lru_پچھلا = 0;    /* Initializing this saves a valgrind error */
        pCache->pFree = pX;
        zBulk += pCache->szAlloc;
      }جب( --nBulk );
    }
  }
  واپس pCache->pFree!=0;
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
ساکن خالی *pcache1Alloc(صحیح nByte){
  خالی *p = 0;
  أكد( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  اگر( nByte<=pcache1.sz_خانہ ){
    sqlite3_mutex_enter(pcache1.mutex);
    p = (PgHdr1 *)pcache1.pFree;
    اگر( p ){
      pcache1.pFree = pcache1.pFree->p_اگلا;
      pcache1.n_Free_خانہ--;
      AtomicStore(&pcache1.bUnderPressure,pcache1.n_Free_خانہ<pcache1.nReserve);
      أكد( pcache1.n_Free_خانہ>=0 );
      sqlite3_حالت_Highwater(SQLITE_حالت_PAGECACHE_سائز, nByte);
      sqlite3_حالت_اوپر(SQLITE_حالت_PAGECACHE_مستعمل, 1);
    }
    sqlite3_mutex_leave(pcache1.mutex);
  }
  اگر( p==0 ){
    /* Memory is not available in the SQLITE_CONFIG_PAGECACHE pool.  Get
    ** it from sqlite3Malloc instead.
    */
    p = sqlite3Malloc(nByte);
#اگر_غیر_تعریف_شدہ SQLITE_DISABLE_PAGECACHE_حد_سے_تجاوز_STATS
    اگر( p ){
      صحیح sz = sqlite3_Malloc_سائز(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3_حالت_Highwater(SQLITE_حالت_PAGECACHE_سائز, nByte);
      sqlite3_حالت_اوپر(SQLITE_حالت_PAGECACHE_حد_سے_تجاوز, sz);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#اگر_ختم
    sqlite3_Memdebug_سیٹ_Type(p, MEMTYPE_PCACHE);
  }
  واپس p;
}

/*
** Free an allocated buffer obtained from pcache1Alloc().
*/
ساکن خالی pcache1Free(خالی *p){
  اگر( p==0 ) واپس;
  اگر( SQLITE_WITHIN(p, pcache1.p_آغاز, pcache1.p_انتہا) ){
    PgFreeslot *p_خانہ;
    sqlite3_mutex_enter(pcache1.mutex);
    sqlite3_حالت_نیچے_کو(SQLITE_حالت_PAGECACHE_مستعمل, 1);
    p_خانہ = (PgFreeslot*)p;
    p_خانہ->p_اگلا = pcache1.pFree;
    pcache1.pFree = p_خانہ;
    pcache1.n_Free_خانہ++;
    AtomicStore(&pcache1.bUnderPressure,pcache1.n_Free_خانہ<pcache1.nReserve);
    أكد( pcache1.n_Free_خانہ<=pcache1.n_خانہ );
    sqlite3_mutex_leave(pcache1.mutex);
  }ورنہ{
    أكد( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3_Memdebug_سیٹ_Type(p, MEMTYPE_ڈھیر);
#اگر_غیر_تعریف_شدہ SQLITE_DISABLE_PAGECACHE_حد_سے_تجاوز_STATS
    {
      صحیح nFreed = 0;
      nFreed = sqlite3_Malloc_سائز(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3_حالت_نیچے_کو(SQLITE_حالت_PAGECACHE_حد_سے_تجاوز, nFreed);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#اگر_ختم
    sqlite3_free(p);
  }
}

#اگر_تعریف_شدہ SQLITE_ENABLE_حافظہ_MANAGEMENT
/*
** Return the size of a pcache allocation
*/
ساکن صحیح pcache1_Mem_سائز(خالی *p){
  اگر( p>=pcache1.p_آغاز && p<pcache1.p_انتہا ){
    واپس pcache1.sz_خانہ;
  }ورنہ{
    صحیح i_سائز;
    أكد( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3_Memdebug_سیٹ_Type(p, MEMTYPE_ڈھیر);
    i_سائز = sqlite3_Malloc_سائز(p);
    sqlite3_Memdebug_سیٹ_Type(p, MEMTYPE_PCACHE);
    واپس i_سائز;
  }
}
#اگر_ختم /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

/*
** Allocate a new page object initially associated with cache pCache.
*/
ساکن PgHdr1 *pcache1_Alloc_صفحہ(PCache1 *pCache, صحیح benignMalloc){
  PgHdr1 *p = 0;
  خالی *pPg;

  أكد( sqlite3_mutex_held(pCache->p_گروہ->mutex) );
  اگر( pCache->pFree || (pCache->n_صفحہ==0 && pcache1InitBulk(pCache)) ){
    أكد( pCache->pFree!=0 );
    p = pCache->pFree;
    pCache->pFree = p->p_اگلا;
    p->p_اگلا = 0;
  }ورنہ{
#اگر_تعریف_شدہ SQLITE_ENABLE_حافظہ_MANAGEMENT
    /* The group mutex must be released before pcache1Alloc() is called. This
    ** is because it might call sqlite3_release_memory(), which assumes that
    ** this mutex is not held. */
    أكد( pcache1.separateCache==0 );
    أكد( pCache->p_گروہ==&pcache1.grp );
    pcache1LeaveMutex(pCache->p_گروہ);
#اگر_ختم
    اگر( benignMalloc ){ sqlite3BeginBenignMalloc(); }
    pPg = pcache1Alloc(pCache->szAlloc);
    اگر( benignMalloc ){ sqlite3_انتہا_Benign_Malloc(); }
#اگر_تعریف_شدہ SQLITE_ENABLE_حافظہ_MANAGEMENT
    pcache1EnterMutex(pCache->p_گروہ);
#اگر_ختم
    اگر( pPg==0 ) واپس 0;
    p = (PgHdr1 *)&((u8 *)pPg)[pCache->sz_صفحہ];
    p->صفحہ.pBuf = pPg;
    p->صفحہ.pExtra = (u8*)p + ROUND8(حجم(*p));
    أكد( EIGHT_BYTE_ALIGNMENT( p->صفحہ.pExtra ) );
    p->isBulkLocal = 0;
    p->isAnchor = 0;
    p->p_Lru_پچھلا = 0;           /* Initializing this saves a valgrind error */
  }
  (*pCache->pnPurgeable)++;
  واپس p;
}

/*
** Free a page object allocated by pcache1AllocPage().
*/
ساکن خالی pcache1_Free_صفحہ(PgHdr1 *p){
  PCache1 *pCache;
  أكد( p!=0 );
  pCache = p->pCache;
  أكد( sqlite3_mutex_held(p->pCache->p_گروہ->mutex) );
  اگر( p->isBulkLocal ){
    p->p_اگلا = pCache->pFree;
    pCache->pFree = p;
  }ورنہ{
    pcache1Free(p->صفحہ.pBuf);
  }
  (*pCache->pnPurgeable)--;
}

/*
** Malloc function used by SQLite to obtain space from the buffer configured
** using sqlite3_config(SQLITE_CONFIG_PAGECACHE) option. If no such buffer
** exists, this function falls back to sqlite3Malloc().
*/
خالی *sqlite3_صفحہ_Malloc(صحیح sz){
  أكد( sz<=65536+8 ); /* These allocations are never very large */
  واپس pcache1Alloc(sz);
}

/*
** Free an allocated buffer obtained from sqlite3PageMalloc().
*/
خالی sqlite3_صفحہ_Free(خالی *p){
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
ساکن صحیح pcache1_Under_حافظہ_Pressure(PCache1 *pCache){
  اگر( pcache1.n_خانہ && (pCache->sz_صفحہ+pCache->szExtra)<=pcache1.sz_خانہ ){
    واپس Atomic_بھرو(&pcache1.bUnderPressure);
  }ورنہ{
    واپس sqlite3_ڈھیر_Nearly_بھرا();
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
ساکن خالی pcache1ResizeHash(PCache1 *p){
  PgHdr1 **apNew;
  u64 nNew;
  u32 i;

  أكد( sqlite3_mutex_held(p->p_گروہ->mutex) );

  nNew = 2*(u64)p->nHash;
  اگر( nNew<256 ){
    nNew = 256;
  }
  أكد( (nNew & (nNew-1))==0 );   /* nNew is always a power of two */

  pcache1LeaveMutex(p->p_گروہ);
  اگر( p->nHash ){ sqlite3BeginBenignMalloc(); }
  apNew = (PgHdr1 **)sqlite3MallocZero(حجم(PgHdr1 *)*nNew);
  اگر( p->nHash ){ sqlite3_انتہا_Benign_Malloc(); }
  pcache1EnterMutex(p->p_گروہ);
  اگر( apNew ){
    برائے(i=0; i<p->nHash; i++){
      PgHdr1 *p_صفحہ;
      PgHdr1 *p_اگلا = p->apHash[i];
      جب( (p_صفحہ = p_اگلا)!=0 ){
        غیرعلامتی صحیح h = (غیرعلامتی صحیح)(p_صفحہ->i_کلید & (nNew-1));
        p_اگلا = p_صفحہ->p_اگلا;
        p_صفحہ->p_اگلا = apNew[h];
        apNew[h] = p_صفحہ;
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
ساکن PgHdr1 *pcache1_Pin_صفحہ(PgHdr1 *p_صفحہ){
  أكد( p_صفحہ!=0 );
  أكد( صفحہ_IS_UNPINNED(p_صفحہ) );
  أكد( p_صفحہ->p_Lru_اگلا );
  أكد( p_صفحہ->p_Lru_پچھلا );
  أكد( sqlite3_mutex_held(p_صفحہ->pCache->p_گروہ->mutex) );
  p_صفحہ->p_Lru_پچھلا->p_Lru_اگلا = p_صفحہ->p_Lru_اگلا;
  p_صفحہ->p_Lru_اگلا->p_Lru_پچھلا = p_صفحہ->p_Lru_پچھلا;
  p_صفحہ->p_Lru_اگلا = 0;
  /* pPage->pLruPrev = 0;
  ** No need to clear pLruPrev as it is never accessed if pLruNext is 0 */
  أكد( p_صفحہ->isAnchor==0 );
  أكد( p_صفحہ->pCache->p_گروہ->lru.isAnchor==1 );
  p_صفحہ->pCache->nRecyclable--;
  واپس p_صفحہ;
}


/*
** Remove the page supplied as an argument from the hash table
** (PCache1.apHash structure) that it is currently stored in.
** Also free the page if freePage is true.
**
** The PGroup mutex must be held when this function is called.
*/
ساکن خالی pcache1RemoveFromHash(PgHdr1 *p_صفحہ, صحیح freeFlag){
  غیرعلامتی صحیح h;
  PCache1 *pCache = p_صفحہ->pCache;
  PgHdr1 **pp;

  أكد( sqlite3_mutex_held(pCache->p_گروہ->mutex) );
  أكد( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  h = p_صفحہ->i_کلید & (pCache->nHash-1);
  برائے(pp=&pCache->apHash[h]; (*pp)!=p_صفحہ; pp=&(*pp)->p_اگلا);
  *pp = (*pp)->p_اگلا;

  pCache->n_صفحہ--;
  اگر( freeFlag ) pcache1_Free_صفحہ(p_صفحہ);
}

/*
** If there are currently more than nMaxPage pages allocated, try
** to recycle pages to reduce the number allocated to nMaxPage.
*/
ساکن خالی pcache1_Enforce_Max_صفحہ(PCache1 *pCache){
  PGroup *p_گروہ = pCache->p_گروہ;
  PgHdr1 *p;
  أكد( sqlite3_mutex_held(p_گروہ->mutex) );
  جب( p_گروہ->nPurgeable>p_گروہ->n_Max_صفحہ
      && (p=p_گروہ->lru.p_Lru_پچھلا)->isAnchor==0
  ){
    أكد( p->pCache->p_گروہ==p_گروہ );
    أكد( صفحہ_IS_UNPINNED(p) );
    pcache1_Pin_صفحہ(p);
    pcache1RemoveFromHash(p, 1);
  }
  اگر( pCache->n_صفحہ==0 && pCache->pBulk ){
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
ساکن خالی pcache1TruncateUnsafe(
  PCache1 *pCache,             /* The cache to truncate */
  غیرعلامتی صحیح i_حد_بندی          /* Drop pages with this pgno or larger */
){
  TESTONLY( صحیح n_صفحہ = 0; )  /* To assert pCache->nPage is correct */
  غیرعلامتی صحیح h, iStop;
  أكد( sqlite3_mutex_held(pCache->p_گروہ->mutex) );
  أكد( pCache->i_Max_کلید >= i_حد_بندی );
  أكد( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  اگر( pCache->i_Max_کلید - i_حد_بندی < pCache->nHash ){
    /* If we are just shaving the last few pages off the end of the
    ** cache, then there is no point in scanning the entire hash table.
    ** Only scan those hash slots that might contain pages that need to
    ** be removed. */
    h = i_حد_بندی & (pCache->nHash-1);
    iStop = pCache->i_Max_کلید & (pCache->nHash-1);
    TESTONLY( n_صفحہ = -10; )  /* Disable the pCache->nPage validity check */
  }ورنہ{
    /* This is the general case where many pages are being removed.
    ** It is necessary to scan the entire hash table */
    h = pCache->nHash/2;
    iStop = h - 1;
  }
  برائے(;;){
    PgHdr1 **pp;
    PgHdr1 *p_صفحہ;
    أكد( h<pCache->nHash );
    pp = &pCache->apHash[h];
    جب( (p_صفحہ = *pp)!=0 ){
      اگر( p_صفحہ->i_کلید>=i_حد_بندی ){
        pCache->n_صفحہ--;
        *pp = p_صفحہ->p_اگلا;
        اگر( صفحہ_IS_UNPINNED(p_صفحہ) ) pcache1_Pin_صفحہ(p_صفحہ);
        pcache1_Free_صفحہ(p_صفحہ);
      }ورنہ{
        pp = &p_صفحہ->p_اگلا;
        TESTONLY( اگر( n_صفحہ>=0 ) n_صفحہ++; )
      }
    }
    اگر( h==iStop ) توڑو;
    h = (h+1) & (pCache->nHash-1);
  }
  أكد( n_صفحہ<0 || pCache->n_صفحہ==(غیرعلامتی)n_صفحہ );
}

/******************************************************************************/
/******** sqlite3_pcache Methods **********************************************/

/*
** Implementation of the sqlite3_pcache.xInit method.
*/
ساکن صحیح pcache1Init(خالی *Not_مستعمل){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  أكد( pcache1.isInit==0 );
  memset(&pcache1, 0, حجم(pcache1));


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
#اگر defined(SQLITE_ENABLE_حافظہ_MANAGEMENT)
  pcache1.separateCache = 0;
#وگرنہ_اگر SQLITE_THREADSAFE
  pcache1.separateCache = sqlite3GlobalConfig.p_صفحہ==0
                          || sqlite3GlobalConfig.bCoreMutex>0;
#ورنہ
  pcache1.separateCache = sqlite3GlobalConfig.p_صفحہ==0;
#اگر_ختم

#اگر SQLITE_THREADSAFE
  اگر( sqlite3GlobalConfig.bCoreMutex ){
    pcache1.grp.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_LRU);
    pcache1.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PMEM);
  }
#اگر_ختم
  اگر( pcache1.separateCache
   && sqlite3GlobalConfig.n_صفحہ!=0
   && sqlite3GlobalConfig.p_صفحہ==0
  ){
    pcache1.n_Init_صفحہ = sqlite3GlobalConfig.n_صفحہ;
  }ورنہ{
    pcache1.n_Init_صفحہ = 0;
  }
  pcache1.grp.mxPinned = 10;
  pcache1.isInit = 1;
  واپس SQLITE_OK;
}

/*
** Implementation of the sqlite3_pcache.xShutdown method.
** Note that the static mutex allocated in xInit does
** not need to be freed.
*/
ساکن خالی pcache1Shutdown(خالی *Not_مستعمل){
  غیر_مستعمل_PARAMETER(Not_مستعمل);
  أكد( pcache1.isInit!=0 );
  memset(&pcache1, 0, حجم(pcache1));
}

/* forward declaration */
ساکن خالی pcache1Destroy(sqlite3_pcache *p);

/*
** Implementation of the sqlite3_pcache.xCreate method.
**
** Allocate a new cache.
*/
ساکن sqlite3_pcache *pcache1Create(صحیح sz_صفحہ, صحیح szExtra, صحیح bPurgeable){
  PCache1 *pCache;      /* The newly created page cache */
  PGroup *p_گروہ;       /* The group the new page cache will belong to */
  i64 sz;               /* Bytes of memory required to allocate the new cache */

  أكد( (sz_صفحہ & (sz_صفحہ-1))==0 && sz_صفحہ>=512 && sz_صفحہ<=65536 );
  أكد( szExtra < 300 );

  sz = حجم(PCache1) + حجم(PGroup)*pcache1.separateCache;
  pCache = (PCache1 *)sqlite3MallocZero(sz);
  اگر( pCache ){
    اگر( pcache1.separateCache ){
      p_گروہ = (PGroup*)&pCache[1];
      p_گروہ->mxPinned = 10;
    }ورنہ{
      p_گروہ = &pcache1.grp;
    }
    pcache1EnterMutex(p_گروہ);
    اگر( p_گروہ->lru.isAnchor==0 ){
      p_گروہ->lru.isAnchor = 1;
      p_گروہ->lru.p_Lru_پچھلا = p_گروہ->lru.p_Lru_اگلا = &p_گروہ->lru;
    }
    pCache->p_گروہ = p_گروہ;
    pCache->sz_صفحہ = sz_صفحہ;
    pCache->szExtra = szExtra;
    pCache->szAlloc = sz_صفحہ + szExtra + ROUND8(حجم(PgHdr1));
    pCache->bPurgeable = (bPurgeable ? 1 : 0);
    pcache1ResizeHash(pCache);
    اگر( bPurgeable ){
      pCache->nMin = 10;
      p_گروہ->n_Min_صفحہ += pCache->nMin;
      p_گروہ->mxPinned = p_گروہ->n_Max_صفحہ + 10 - p_گروہ->n_Min_صفحہ;
      pCache->pnPurgeable = &p_گروہ->nPurgeable;
    }ورنہ{
      pCache->pnPurgeable = &pCache->nPurgeableDummy;
    }
    pcache1LeaveMutex(p_گروہ);
    اگر( pCache->nHash==0 ){
      pcache1Destroy((sqlite3_pcache*)pCache);
      pCache = 0;
    }
  }
  واپس (sqlite3_pcache *)pCache;
}

/*
** Implementation of the sqlite3_pcache.xCachesize method.
**
** Configure the cache_size limit for a cache.
*/
ساکن خالی pcache1Cachesize(sqlite3_pcache *p, صحیح nMax){
  PCache1 *pCache = (PCache1 *)p;
  u32 n;
  أكد( nMax>=0 );
  اگر( pCache->bPurgeable ){
    PGroup *p_گروہ = pCache->p_گروہ;
    pcache1EnterMutex(p_گروہ);
    n = (u32)nMax;
    اگر( n > 0x7fff0000 - p_گروہ->n_Max_صفحہ + pCache->nMax ){
      n = 0x7fff0000 - p_گروہ->n_Max_صفحہ + pCache->nMax;
    }
    p_گروہ->n_Max_صفحہ += (n - pCache->nMax);
    p_گروہ->mxPinned = p_گروہ->n_Max_صفحہ + 10 - p_گروہ->n_Min_صفحہ;
    pCache->nMax = n;
    pCache->n90pct = pCache->nMax*9/10;
    pcache1_Enforce_Max_صفحہ(pCache);
    pcache1LeaveMutex(p_گروہ);
  }
}

/*
** Implementation of the sqlite3_pcache.xShrink method.
**
** Free up as much memory as possible.
*/
ساکن خالی pcache1Shrink(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1*)p;
  اگر( pCache->bPurgeable ){
    PGroup *p_گروہ = pCache->p_گروہ;
    غیرعلامتی صحیح saved_Max_صفحہ;
    pcache1EnterMutex(p_گروہ);
    saved_Max_صفحہ = p_گروہ->n_Max_صفحہ;
    p_گروہ->n_Max_صفحہ = 0;
    pcache1_Enforce_Max_صفحہ(pCache);
    p_گروہ->n_Max_صفحہ = saved_Max_صفحہ;
    pcache1LeaveMutex(p_گروہ);
  }
}

/*
** Implementation of the sqlite3_pcache.xPagecount method.
*/
ساکن صحیح pcache1Pagecount(sqlite3_pcache *p){
  صحیح n;
  PCache1 *pCache = (PCache1*)p;
  pcache1EnterMutex(pCache->p_گروہ);
  n = pCache->n_صفحہ;
  pcache1LeaveMutex(pCache->p_گروہ);
  واپس n;
}


/*
** Implement steps 3, 4, and 5 of the pcache1Fetch() algorithm described
** in the header of the pcache1Fetch() procedure.
**
** This steps are broken out into a separate procedure because they are
** usually not needed, and by avoiding the stack initialization required
** for these steps, the main pcache1Fetch() procedure can run faster.
*/
ساکن SQLITE_NOINLINE PgHdr1 *pcache1FetchStage2(
  PCache1 *pCache,
  غیرعلامتی صحیح i_کلید,
  صحیح createFlag
){
  غیرعلامتی صحیح nPinned;
  PGroup *p_گروہ = pCache->p_گروہ;
  PgHdr1 *p_صفحہ = 0;

  /* Step 3: Abort if createFlag is 1 but the cache is nearly full */
  أكد( pCache->n_صفحہ >= pCache->nRecyclable );
  nPinned = pCache->n_صفحہ - pCache->nRecyclable;
  أكد( p_گروہ->mxPinned == p_گروہ->n_Max_صفحہ + 10 - p_گروہ->n_Min_صفحہ );
  أكد( pCache->n90pct == pCache->nMax*9/10 );
  اگر( createFlag==1 && (
        nPinned>=p_گروہ->mxPinned
     || nPinned>=pCache->n90pct
     || (pcache1_Under_حافظہ_Pressure(pCache) && pCache->nRecyclable<nPinned)
  )){
    واپس 0;
  }

  اگر( pCache->n_صفحہ>=pCache->nHash ) pcache1ResizeHash(pCache);
  أكد( pCache->nHash>0 && pCache->apHash );
  أكد( (pCache->nHash & (pCache->nHash-1))==0 );

  /* Step 4. Try to recycle a page. */
  اگر( pCache->bPurgeable
   && !p_گروہ->lru.p_Lru_پچھلا->isAnchor
   && ((pCache->n_صفحہ+1>=pCache->nMax) || pcache1_Under_حافظہ_Pressure(pCache))
  ){
    PCache1 *pOther;
    p_صفحہ = p_گروہ->lru.p_Lru_پچھلا;
    أكد( صفحہ_IS_UNPINNED(p_صفحہ) );
    pcache1RemoveFromHash(p_صفحہ, 0);
    pcache1_Pin_صفحہ(p_صفحہ);
    pOther = p_صفحہ->pCache;
    اگر( pOther->szAlloc != pCache->szAlloc ){
      pcache1_Free_صفحہ(p_صفحہ);
      p_صفحہ = 0;
    }ورنہ{
      p_گروہ->nPurgeable -= (pOther->bPurgeable - pCache->bPurgeable);
    }
  }

  /* Step 5. If a usable page buffer has still not been found,
  ** attempt to allocate a new one.
  */
  اگر( !p_صفحہ ){
    p_صفحہ = pcache1_Alloc_صفحہ(pCache, createFlag==1);
  }

  اگر( p_صفحہ ){
    غیرعلامتی صحیح h = i_کلید & (pCache->nHash-1);
    pCache->n_صفحہ++;
    p_صفحہ->i_کلید = i_کلید;
    p_صفحہ->p_اگلا = pCache->apHash[h];
    p_صفحہ->pCache = pCache;
    p_صفحہ->p_Lru_اگلا = 0;
    /* pPage->pLruPrev = 0;
    ** No need to clear pLruPrev since it is not accessed when pLruNext==0 */
    *(خالی **)p_صفحہ->صفحہ.pExtra = 0;
    pCache->apHash[h] = p_صفحہ;
    اگر( i_کلید>pCache->i_Max_کلید ){
      pCache->i_Max_کلید = i_کلید;
    }
  }
  واپس p_صفحہ;
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
ساکن PgHdr1 *pcache1FetchNoMutex(
  sqlite3_pcache *p,
  غیرعلامتی صحیح i_کلید,
  صحیح createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_صفحہ = 0;

  /* Step 1: Search the hash table for an existing entry.  nHash is always
  ** a power of two when the cache is in use (see pcache1ResizeHash()), so
  ** the modulo reduces to a mask, avoiding a hardware divide. */
  أكد( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  p_صفحہ = pCache->apHash[i_کلید & (pCache->nHash-1u)];
  جب( p_صفحہ && p_صفحہ->i_کلید!=i_کلید ){ p_صفحہ = p_صفحہ->p_اگلا; }

  /* Step 2: If the page was found in the hash table, then return it.
  ** If the page was not in the hash table and createFlag is 0, abort.
  ** Otherwise (page not in hash and createFlag!=0) continue with
  ** subsequent steps to try to create the page. */
  اگر( p_صفحہ ){
    اگر( صفحہ_IS_UNPINNED(p_صفحہ) ){
      واپس pcache1_Pin_صفحہ(p_صفحہ);
    }ورنہ{
      واپس p_صفحہ;
    }
  }ورنہ اگر( createFlag ){
    /* Steps 3, 4, and 5 implemented by this subroutine */
    واپس pcache1FetchStage2(pCache, i_کلید, createFlag);
  }ورنہ{
    واپس 0;
  }
}
#اگر PCACHE1_MIGHT_USE_گروہ_MUTEX
ساکن PgHdr1 *pcache1FetchWithMutex(
  sqlite3_pcache *p,
  غیرعلامتی صحیح i_کلید,
  صحیح createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_صفحہ;

  pcache1EnterMutex(pCache->p_گروہ);
  p_صفحہ = pcache1FetchNoMutex(p, i_کلید, createFlag);
  أكد( p_صفحہ==0 || pCache->i_Max_کلید>=i_کلید );
  pcache1LeaveMutex(pCache->p_گروہ);
  واپس p_صفحہ;
}
#اگر_ختم
ساکن sqlite3_pcache_صفحہ *pcache1Fetch(
  sqlite3_pcache *p,
  غیرعلامتی صحیح i_کلید,
  صحیح createFlag
){
#اگر PCACHE1_MIGHT_USE_گروہ_MUTEX || defined(SQLITE_DEBUG)
  PCache1 *pCache = (PCache1 *)p;
#اگر_ختم

  أكد( offsetof(PgHdr1,صفحہ)==0 );
  أكد( pCache->bPurgeable || createFlag!=1 );
  أكد( pCache->bPurgeable || pCache->nMin==0 );
  أكد( pCache->bPurgeable==0 || pCache->nMin==10 );
  أكد( pCache->nMin==0 || pCache->bPurgeable );
  أكد( pCache->nHash>0 );
#اگر PCACHE1_MIGHT_USE_گروہ_MUTEX
  اگر( pCache->p_گروہ->mutex ){
    واپس (sqlite3_pcache_صفحہ*)pcache1FetchWithMutex(p, i_کلید, createFlag);
  }ورنہ
#اگر_ختم
  {
    واپس (sqlite3_pcache_صفحہ*)pcache1FetchNoMutex(p, i_کلید, createFlag);
  }
}


/*
** Implementation of the sqlite3_pcache.xUnpin method.
**
** Mark a page as unpinned (eligible for asynchronous recycling).
*/
ساکن خالی pcache1Unpin(
  sqlite3_pcache *p,
  sqlite3_pcache_صفحہ *pPg,
  صحیح reuseUnlikely
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_صفحہ = (PgHdr1 *)pPg;
  PGroup *p_گروہ = pCache->p_گروہ;

  أكد( p_صفحہ->pCache==pCache );
  pcache1EnterMutex(p_گروہ);

  /* It is an error to call this function if the page is already
  ** part of the PGroup LRU list.
  */
  أكد( p_صفحہ->p_Lru_اگلا==0 );
  أكد( صفحہ_IS_PINNED(p_صفحہ) );

  اگر( reuseUnlikely || p_گروہ->nPurgeable>p_گروہ->n_Max_صفحہ ){
    pcache1RemoveFromHash(p_صفحہ, 1);
  }ورنہ{
    /* Add the page to the PGroup LRU list. */
    PgHdr1 **pp_پہلا = &p_گروہ->lru.p_Lru_اگلا;
    p_صفحہ->p_Lru_پچھلا = &p_گروہ->lru;
    (p_صفحہ->p_Lru_اگلا = *pp_پہلا)->p_Lru_پچھلا = p_صفحہ;
    *pp_پہلا = p_صفحہ;
    pCache->nRecyclable++;
  }

  pcache1LeaveMutex(pCache->p_گروہ);
}

/*
** Implementation of the sqlite3_pcache.xRekey method.
*/
ساکن خالی pcache1Rekey(
  sqlite3_pcache *p,
  sqlite3_pcache_صفحہ *pPg,
  غیرعلامتی صحیح iOld,
  غیرعلامتی صحیح iNew
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_صفحہ = (PgHdr1 *)pPg;
  PgHdr1 **pp;
  غیرعلامتی صحیح hOld, hNew;
  أكد( p_صفحہ->i_کلید==iOld );
  أكد( p_صفحہ->pCache==pCache );
  أكد( iOld!=iNew );               /* The page number really is changing */

  pcache1EnterMutex(pCache->p_گروہ);

  أكد( pcache1FetchNoMutex(p, iOld, 0)==p_صفحہ ); /* pPg really is iOld */
  أكد( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  hOld = iOld & (pCache->nHash-1);
  pp = &pCache->apHash[hOld];
  جب( (*pp)!=p_صفحہ ){
    pp = &(*pp)->p_اگلا;
  }
  *pp = p_صفحہ->p_اگلا;

  أكد( pcache1FetchNoMutex(p, iNew, 0)==0 ); /* iNew not in cache */
  hNew = iNew & (pCache->nHash-1);
  p_صفحہ->i_کلید = iNew;
  p_صفحہ->p_اگلا = pCache->apHash[hNew];
  pCache->apHash[hNew] = p_صفحہ;
  اگر( iNew>pCache->i_Max_کلید ){
    pCache->i_Max_کلید = iNew;
  }

  pcache1LeaveMutex(pCache->p_گروہ);
}

/*
** Implementation of the sqlite3_pcache.xTruncate method.
**
** Discard all unpinned pages in the cache with a page number equal to
** or greater than parameter iLimit. Any pinned pages with a page number
** equal to or greater than iLimit are implicitly unpinned.
*/
ساکن خالی pcache1Truncate(sqlite3_pcache *p, غیرعلامتی صحیح i_حد_بندی){
  PCache1 *pCache = (PCache1 *)p;
  pcache1EnterMutex(pCache->p_گروہ);
  اگر( i_حد_بندی<=pCache->i_Max_کلید ){
    pcache1TruncateUnsafe(pCache, i_حد_بندی);
    pCache->i_Max_کلید = i_حد_بندی-1;
  }
  pcache1LeaveMutex(pCache->p_گروہ);
}

/*
** Implementation of the sqlite3_pcache.xDestroy method.
**
** Destroy a cache allocated using pcache1Create().
*/
ساکن خالی pcache1Destroy(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1 *)p;
  PGroup *p_گروہ = pCache->p_گروہ;
  أكد( pCache->bPurgeable || (pCache->nMax==0 && pCache->nMin==0) );
  pcache1EnterMutex(p_گروہ);
  اگر( pCache->n_صفحہ ) pcache1TruncateUnsafe(pCache, 0);
  أكد( p_گروہ->n_Max_صفحہ >= pCache->nMax );
  p_گروہ->n_Max_صفحہ -= pCache->nMax;
  أكد( p_گروہ->n_Min_صفحہ >= pCache->nMin );
  p_گروہ->n_Min_صفحہ -= pCache->nMin;
  p_گروہ->mxPinned = p_گروہ->n_Max_صفحہ + 10 - p_گروہ->n_Min_صفحہ;
  pcache1_Enforce_Max_صفحہ(pCache);
  pcache1LeaveMutex(p_گروہ);
  sqlite3_free(pCache->pBulk);
  sqlite3_free(pCache->apHash);
  sqlite3_free(pCache);
}

/*
** This function is called during initialization (sqlite3_initialize()) to
** install the default pluggable cache module, assuming the user has not
** already provided an alternative.
*/
خالی sqlite3_PCache_سیٹ_Default(خالی){
  ساکن ثابت sqlite3_pcache_methods2 defaultMethods = {
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
صحیح sqlite3_سرخی_سائز_Pcache1(خالی){ واپس ROUND8(حجم(PgHdr1)); }

/*
** Return the global mutex used by this PCACHE implementation.  The
** sqlite3_status() routine needs access to this mutex.
*/
sqlite3_mutex *sqlite3Pcache1Mutex(خالی){
  واپس pcache1.mutex;
}

#اگر_تعریف_شدہ SQLITE_ENABLE_حافظہ_MANAGEMENT
/*
** This function is called to free superfluous dynamically allocated memory
** held by the pager system. Memory in use by any SQLite pager allocated
** by the current thread may be sqlite3_free()ed.
**
** nReq is the number of bytes of memory required. Once this much has
** been released, the function returns. The return value is the total number
** of bytes of memory released.
*/
صحیح sqlite3_Pcache_Release_حافظہ(صحیح nReq){
  صحیح nFree = 0;
  أكد( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  أكد( sqlite3_mutex_notheld(pcache1.mutex) );
  اگر( sqlite3GlobalConfig.p_صفحہ==0 ){
    PgHdr1 *p;
    pcache1EnterMutex(&pcache1.grp);
    جب( (nReq<0 || nFree<nReq)
       &&  (p=pcache1.grp.lru.p_Lru_پچھلا)!=0
       &&  p->isAnchor==0
    ){
      nFree += pcache1_Mem_سائز(p->صفحہ.pBuf);
      أكد( صفحہ_IS_UNPINNED(p) );
      pcache1_Pin_صفحہ(p);
      pcache1RemoveFromHash(p, 1);
    }
    pcache1LeaveMutex(&pcache1.grp);
  }
  واپس nFree;
}
#اگر_ختم /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

#اگر_تعریف_شدہ SQLITE_TEST
/*
** This function is used by test procedures to inspect the internal state
** of the global cache.
*/
خالی sqlite3PcacheStats(
  صحیح *pnCurrent,      /* OUT: Total number of pages cached */
  صحیح *pnMax,          /* OUT: Global maximum cache size */
  صحیح *pnMin,          /* OUT: Sum of PCache1.nMin for purgeable caches */
  صحیح *pnRecyclable    /* OUT: Total number of pages available for recycling */
){
  PgHdr1 *p;
  صحیح nRecyclable = 0;
  برائے(p=pcache1.grp.lru.p_Lru_اگلا; p && !p->isAnchor; p=p->p_Lru_اگلا){
    أكد( صفحہ_IS_UNPINNED(p) );
    nRecyclable++;
  }
  *pnCurrent = pcache1.grp.nPurgeable;
  *pnMax = (صحیح)pcache1.grp.n_Max_صفحہ;
  *pnMin = (صحیح)pcache1.grp.n_Min_صفحہ;
  *pnRecyclable = nRecyclable;
}
#اگر_ختم
