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
#ضمن "sqliteInt.h"

تعريفنوع بنية PCache1 PCache1;
تعريفنوع بنية PgHdr1 PgHdr1;
تعريفنوع بنية PgFreeslot PgFreeslot;
تعريفنوع بنية PGroup PGroup;

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
بنية PgHdr1 {
  sqlite3_pcache_صفحو صفحو; /* Base class. Must be first. pBuf & pExtra */
  بنانشان صحيح i_ڪنجي;        /* Key value (page number) */
  u16 isBulkLocal;          /* This page from bulk local storage */
  u16 isAnchor;             /* This is the PGroup.lru element */
  PgHdr1 *p_ايندڙ;            /* Next in hash table chain */
  PCache1 *pCache;          /* Cache that currently owns this page */
  PgHdr1 *p_Lru_ايندڙ;         /* Next in circular LRU list of unpinned pages */
  PgHdr1 *pLruPrev;         /* Previous in LRU list of unpinned pages */
                            /* NB: pLruPrev is only valid if pLruNext!=0 */
};

/*
** A page is pinned if it is not on the LRU list.  To be "pinned" means
** that the page is in active use and must not be deallocated.
*/
#وصف_ڪريو صفحو_IS_PINNED(p)    ((p)->p_Lru_ايندڙ==0)
#وصف_ڪريو صفحو_IS_UNPINNED(p)  ((p)->p_Lru_ايندڙ!=0)

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
بنية PGroup {
  sqlite3_mutex *mutex;          /* MUTEX_STATIC_LRU or NULL */
  بنانشان صحيح n_Max_صفحو;         /* Sum of nMax for purgeable caches */
  بنانشان صحيح n_Min_صفحو;         /* Sum of nMin for purgeable caches */
  بنانشان صحيح mxPinned;         /* nMaxpage + 10 - nMinPage */
  بنانشان صحيح nPurgeable;       /* Number of purgeable pages allocated */
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
بنية PCache1 {
  /* Cache configuration parameters. Page size (szPage) and the purgeable
  ** flag (bPurgeable) and the pnPurgeable pointer are all set when the
  ** cache is created and are never changed thereafter. nMax may be
  ** modified at any time by a call to the pcache1Cachesize() method.
  ** The PGroup mutex must be held when accessing nMax.
  */
  PGroup *p_گروهه;                     /* PGroup this cache belongs to */
  بنانشان صحيح *pnPurgeable;          /* Pointer to pGroup->nPurgeable */
  صحيح sz_صفحو;                         /* Size of database content section */
  صحيح szExtra;                        /* sizeof(MemPage)+sizeof(PgHdr) */
  صحيح szAlloc;                        /* Total size of one pcache line */
  صحيح bPurgeable;                     /* True if cache is purgeable */
  بنانشان صحيح nMin;                  /* Minimum number of pages reserved */
  بنانشان صحيح nMax;                  /* Configured "cache_size" value */
  بنانشان صحيح n90pct;                /* nMax*9/10 */
  بنانشان صحيح i_Max_ڪنجي;               /* Largest key seen since xTruncate() */
  بنانشان صحيح nPurgeableDummy;       /* pnPurgeable points here when not used*/

  /* Hash table of all pages. The following variables may only be accessed
  ** when the accessor is holding the PGroup mutex.
  */
  بنانشان صحيح nRecyclable;           /* Number of pages in the LRU list */
  بنانشان صحيح n_صفحو;                 /* Total number of pages in apHash */
  بنانشان صحيح nHash;                 /* Number of slots in apHash[] */
  PgHdr1 **apHash;                    /* Hash table for fast lookup by key */
  PgHdr1 *pFree;                      /* List of unused pcache-local pages */
  فراغ *pBulk;                        /* Bulk memory used by pcache-local */
};

/*
** Free slots in the allocator used to divide up the global page cache
** buffer provided using the SQLITE_CONFIG_PAGECACHE mechanism.
*/
بنية PgFreeslot {
  PgFreeslot *p_ايندڙ;  /* Next free slot */
};

/*
** Global data used by this cache.
*/
ساكن SQLITE_WSD بنية PCacheGlobal {
  PGroup grp;                    /* The global PGroup for mode (2) */

  /* Variables related to SQLITE_CONFIG_PAGECACHE settings.  The
  ** szSlot, nSlot, pStart, pEnd, nReserve, and isInit values are all
  ** fixed at sqlite3_initialize() time and do not require mutex protection.
  ** The nFreeSlot and pFree values do require mutex protection.
  */
  صحيح isInit;                    /* True if initialized */
  صحيح separateCache;             /* Use a new PGroup for each PCache */
  صحيح n_Init_صفحو;                 /* Initial bulk allocation size */
  صحيح sz_جڳهه;                    /* Size of each free slot */
  صحيح n_جڳهه;                     /* The number of pcache slots */
  صحيح nReserve;                  /* Try to keep nFreeSlot above this */
  فراغ *p_شروعات, *p_پڇاڙي;           /* Bounds of global page cache memory */
  /* Above requires no mutex.  Use mutex below for variable that follow. */
  sqlite3_mutex *mutex;          /* Mutex for accessing the following: */
  PgFreeslot *pFree;             /* Free page blocks */
  صحيح n_Free_جڳهه;                 /* Number of unused pcache slots */
  صحيح bUnderPressure;            /* True if low on PAGECACHE memory */
} pcache1_g;

/*
** All code in this file should access the global structure above via the
** alias "pcache1". This ensures that the WSD emulation is used when
** compiling for systems that do not support real WSD.
*/
#وصف_ڪريو pcache1 (GLOBAL(بنية PCacheGlobal, pcache1_g))

/*
** Macros to enter and leave the PCache LRU mutex.
*/
#إذا !defined(SQLITE_ENABLE_يادگيري_MANAGEMENT) || SQLITE_THREADSAFE==0
# define pcache1EnterMutex(X)  أكد((X)->mutex==0)
# define pcache1LeaveMutex(X)  أكد((X)->mutex==0)
# define PCACHE1_MIGHT_USE_گروهه_MUTEX 0
#وإلا
# define pcache1EnterMutex(X) sqlite3_mutex_enter((X)->mutex)
# define pcache1LeaveMutex(X) sqlite3_mutex_leave((X)->mutex)
# define PCACHE1_MIGHT_USE_گروهه_MUTEX 1
#جيڪڏهن_پورو

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
فراغ sqlite3_PCache_بفر_تياري(فراغ *pBuf, صحيح sz, صحيح n){
  إذا( pcache1.isInit ){
    PgFreeslot *p;
    إذا( pBuf==0 ) sz = n = 0;
    إذا( n==0 ) sz = 0;
    sz = ROUNDDOWN8(sz);
    pcache1.sz_جڳهه = sz;
    pcache1.n_جڳهه = pcache1.n_Free_جڳهه = n;
    pcache1.nReserve = n>90 ? 10 : (n/10 + 1);
    pcache1.p_شروعات = pBuf;
    pcache1.pFree = 0;
    AtomicStore(&pcache1.bUnderPressure,0);
    طالما( n-- ){
      p = (PgFreeslot*)pBuf;
      p->p_ايندڙ = pcache1.pFree;
      pcache1.pFree = p;
      pBuf = (فراغ*)&((حرف*)pBuf)[sz];
    }
    pcache1.p_پڇاڙي = pBuf;
  }
}

/*
** Try to initialize the pCache->pFree and pCache->pBulk fields.  Return
** true if pCache->pFree ends up containing one or more free pages.
*/
ساكن صحيح pcache1InitBulk(PCache1 *pCache){
  i64 szBulk;
  حرف *zBulk;
  إذا( pcache1.n_Init_صفحو==0 ) أرجع 0;
  /* Do not bother with a bulk allocation if the cache size very small */
  إذا( pCache->nMax<3 ) أرجع 0;
  sqlite3BeginBenignMalloc();
  إذا( pcache1.n_Init_صفحو>0 ){
    szBulk = pCache->szAlloc * (i64)pcache1.n_Init_صفحو;
  }وإلا{
    szBulk = -1024 * (i64)pcache1.n_Init_صفحو;
  }
  إذا( szBulk > pCache->szAlloc*(i64)pCache->nMax ){
    szBulk = pCache->szAlloc*(i64)pCache->nMax;
  }
  إذا( szBulk>=pCache->szAlloc ){
    zBulk = pCache->pBulk = sqlite3Malloc( szBulk );
    sqlite3_پڇاڙي_Benign_Malloc();
    إذا( zBulk ){
      صحيح nBulk = sqlite3_Malloc_سائيز(zBulk)/pCache->szAlloc;
      افعل{
        PgHdr1 *pX = (PgHdr1*)&zBulk[pCache->sz_صفحو];
        pX->صفحو.pBuf = zBulk;
        pX->صفحو.pExtra = (u8*)pX + ROUND8(حجم(*pX));
        أكد( EIGHT_BYTE_ALIGNMENT( pX->صفحو.pExtra ) );
        pX->isBulkLocal = 1;
        pX->isAnchor = 0;
        pX->p_ايندڙ = pCache->pFree;
        pX->pLruPrev = 0;    /* Initializing this saves a valgrind error */
        pCache->pFree = pX;
        zBulk += pCache->szAlloc;
      }طالما( --nBulk );
    }
  }
  أرجع pCache->pFree!=0;
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
ساكن فراغ *pcache1Alloc(صحيح nByte){
  فراغ *p = 0;
  أكد( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  إذا( nByte<=pcache1.sz_جڳهه ){
    sqlite3_mutex_enter(pcache1.mutex);
    p = (PgHdr1 *)pcache1.pFree;
    إذا( p ){
      pcache1.pFree = pcache1.pFree->p_ايندڙ;
      pcache1.n_Free_جڳهه--;
      AtomicStore(&pcache1.bUnderPressure,pcache1.n_Free_جڳهه<pcache1.nReserve);
      أكد( pcache1.n_Free_جڳهه>=0 );
      sqlite3_حالت_Highwater(SQLITE_حالت_PAGECACHE_سائيز, nByte);
      sqlite3_حالت_مٿي(SQLITE_حالت_PAGECACHE_استعمال_ٿيل, 1);
    }
    sqlite3_mutex_leave(pcache1.mutex);
  }
  إذا( p==0 ){
    /* Memory is not available in the SQLITE_CONFIG_PAGECACHE pool.  Get
    ** it from sqlite3Malloc instead.
    */
    p = sqlite3Malloc(nByte);
#جيڪڏهن_وصف_نه_ٿيل SQLITE_DISABLE_PAGECACHE_حد_کان_مٿي_STATS
    إذا( p ){
      صحيح sz = sqlite3_Malloc_سائيز(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3_حالت_Highwater(SQLITE_حالت_PAGECACHE_سائيز, nByte);
      sqlite3_حالت_مٿي(SQLITE_حالت_PAGECACHE_حد_کان_مٿي, sz);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#جيڪڏهن_پورو
    sqlite3_Memdebug_سيٽ_Type(p, MEMTYPE_PCACHE);
  }
  أرجع p;
}

/*
** Free an allocated buffer obtained from pcache1Alloc().
*/
ساكن فراغ pcache1Free(فراغ *p){
  إذا( p==0 ) أرجع;
  إذا( SQLITE_WITHIN(p, pcache1.p_شروعات, pcache1.p_پڇاڙي) ){
    PgFreeslot *p_جڳهه;
    sqlite3_mutex_enter(pcache1.mutex);
    sqlite3_حالت_هيٺ_ڏانهن(SQLITE_حالت_PAGECACHE_استعمال_ٿيل, 1);
    p_جڳهه = (PgFreeslot*)p;
    p_جڳهه->p_ايندڙ = pcache1.pFree;
    pcache1.pFree = p_جڳهه;
    pcache1.n_Free_جڳهه++;
    AtomicStore(&pcache1.bUnderPressure,pcache1.n_Free_جڳهه<pcache1.nReserve);
    أكد( pcache1.n_Free_جڳهه<=pcache1.n_جڳهه );
    sqlite3_mutex_leave(pcache1.mutex);
  }وإلا{
    أكد( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3_Memdebug_سيٽ_Type(p, MEMTYPE_ڍير);
#جيڪڏهن_وصف_نه_ٿيل SQLITE_DISABLE_PAGECACHE_حد_کان_مٿي_STATS
    {
      صحيح nFreed = 0;
      nFreed = sqlite3_Malloc_سائيز(p);
      sqlite3_mutex_enter(pcache1.mutex);
      sqlite3_حالت_هيٺ_ڏانهن(SQLITE_حالت_PAGECACHE_حد_کان_مٿي, nFreed);
      sqlite3_mutex_leave(pcache1.mutex);
    }
#جيڪڏهن_پورو
    sqlite3_free(p);
  }
}

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_يادگيري_MANAGEMENT
/*
** Return the size of a pcache allocation
*/
ساكن صحيح pcache1_Mem_سائيز(فراغ *p){
  إذا( p>=pcache1.p_شروعات && p<pcache1.p_پڇاڙي ){
    أرجع pcache1.sz_جڳهه;
  }وإلا{
    صحيح i_سائيز;
    أكد( sqlite3MemdebugHasType(p, MEMTYPE_PCACHE) );
    sqlite3_Memdebug_سيٽ_Type(p, MEMTYPE_ڍير);
    i_سائيز = sqlite3_Malloc_سائيز(p);
    sqlite3_Memdebug_سيٽ_Type(p, MEMTYPE_PCACHE);
    أرجع i_سائيز;
  }
}
#جيڪڏهن_پورو /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

/*
** Allocate a new page object initially associated with cache pCache.
*/
ساكن PgHdr1 *pcache1_Alloc_صفحو(PCache1 *pCache, صحيح benignMalloc){
  PgHdr1 *p = 0;
  فراغ *pPg;

  أكد( sqlite3_mutex_held(pCache->p_گروهه->mutex) );
  إذا( pCache->pFree || (pCache->n_صفحو==0 && pcache1InitBulk(pCache)) ){
    أكد( pCache->pFree!=0 );
    p = pCache->pFree;
    pCache->pFree = p->p_ايندڙ;
    p->p_ايندڙ = 0;
  }وإلا{
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_يادگيري_MANAGEMENT
    /* The group mutex must be released before pcache1Alloc() is called. This
    ** is because it might call sqlite3_release_memory(), which assumes that
    ** this mutex is not held. */
    أكد( pcache1.separateCache==0 );
    أكد( pCache->p_گروهه==&pcache1.grp );
    pcache1LeaveMutex(pCache->p_گروهه);
#جيڪڏهن_پورو
    إذا( benignMalloc ){ sqlite3BeginBenignMalloc(); }
    pPg = pcache1Alloc(pCache->szAlloc);
    إذا( benignMalloc ){ sqlite3_پڇاڙي_Benign_Malloc(); }
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_يادگيري_MANAGEMENT
    pcache1EnterMutex(pCache->p_گروهه);
#جيڪڏهن_پورو
    إذا( pPg==0 ) أرجع 0;
    p = (PgHdr1 *)&((u8 *)pPg)[pCache->sz_صفحو];
    p->صفحو.pBuf = pPg;
    p->صفحو.pExtra = (u8*)p + ROUND8(حجم(*p));
    أكد( EIGHT_BYTE_ALIGNMENT( p->صفحو.pExtra ) );
    p->isBulkLocal = 0;
    p->isAnchor = 0;
    p->pLruPrev = 0;           /* Initializing this saves a valgrind error */
  }
  (*pCache->pnPurgeable)++;
  أرجع p;
}

/*
** Free a page object allocated by pcache1AllocPage().
*/
ساكن فراغ pcache1_Free_صفحو(PgHdr1 *p){
  PCache1 *pCache;
  أكد( p!=0 );
  pCache = p->pCache;
  أكد( sqlite3_mutex_held(p->pCache->p_گروهه->mutex) );
  إذا( p->isBulkLocal ){
    p->p_ايندڙ = pCache->pFree;
    pCache->pFree = p;
  }وإلا{
    pcache1Free(p->صفحو.pBuf);
  }
  (*pCache->pnPurgeable)--;
}

/*
** Malloc function used by SQLite to obtain space from the buffer configured
** using sqlite3_config(SQLITE_CONFIG_PAGECACHE) option. If no such buffer
** exists, this function falls back to sqlite3Malloc().
*/
فراغ *sqlite3_صفحو_Malloc(صحيح sz){
  أكد( sz<=65536+8 ); /* These allocations are never very large */
  أرجع pcache1Alloc(sz);
}

/*
** Free an allocated buffer obtained from sqlite3PageMalloc().
*/
فراغ sqlite3_صفحو_Free(فراغ *p){
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
ساكن صحيح pcache1_Under_يادگيري_Pressure(PCache1 *pCache){
  إذا( pcache1.n_جڳهه && (pCache->sz_صفحو+pCache->szExtra)<=pcache1.sz_جڳهه ){
    أرجع Atomic_ڀريو(&pcache1.bUnderPressure);
  }وإلا{
    أرجع sqlite3_ڍير_Nearly_ڀريل();
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
ساكن فراغ pcache1ResizeHash(PCache1 *p){
  PgHdr1 **apNew;
  u64 nNew;
  u32 i;

  أكد( sqlite3_mutex_held(p->p_گروهه->mutex) );

  nNew = 2*(u64)p->nHash;
  إذا( nNew<256 ){
    nNew = 256;
  }
  أكد( (nNew & (nNew-1))==0 );   /* nNew is always a power of two */

  pcache1LeaveMutex(p->p_گروهه);
  إذا( p->nHash ){ sqlite3BeginBenignMalloc(); }
  apNew = (PgHdr1 **)sqlite3MallocZero(حجم(PgHdr1 *)*nNew);
  إذا( p->nHash ){ sqlite3_پڇاڙي_Benign_Malloc(); }
  pcache1EnterMutex(p->p_گروهه);
  إذا( apNew ){
    لكل(i=0; i<p->nHash; i++){
      PgHdr1 *p_صفحو;
      PgHdr1 *p_ايندڙ = p->apHash[i];
      طالما( (p_صفحو = p_ايندڙ)!=0 ){
        بنانشان صحيح h = (بنانشان صحيح)(p_صفحو->i_ڪنجي & (nNew-1));
        p_ايندڙ = p_صفحو->p_ايندڙ;
        p_صفحو->p_ايندڙ = apNew[h];
        apNew[h] = p_صفحو;
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
ساكن PgHdr1 *pcache1_Pin_صفحو(PgHdr1 *p_صفحو){
  أكد( p_صفحو!=0 );
  أكد( صفحو_IS_UNPINNED(p_صفحو) );
  أكد( p_صفحو->p_Lru_ايندڙ );
  أكد( p_صفحو->pLruPrev );
  أكد( sqlite3_mutex_held(p_صفحو->pCache->p_گروهه->mutex) );
  p_صفحو->pLruPrev->p_Lru_ايندڙ = p_صفحو->p_Lru_ايندڙ;
  p_صفحو->p_Lru_ايندڙ->pLruPrev = p_صفحو->pLruPrev;
  p_صفحو->p_Lru_ايندڙ = 0;
  /* pPage->pLruPrev = 0;
  ** No need to clear pLruPrev as it is never accessed if pLruNext is 0 */
  أكد( p_صفحو->isAnchor==0 );
  أكد( p_صفحو->pCache->p_گروهه->lru.isAnchor==1 );
  p_صفحو->pCache->nRecyclable--;
  أرجع p_صفحو;
}


/*
** Remove the page supplied as an argument from the hash table
** (PCache1.apHash structure) that it is currently stored in.
** Also free the page if freePage is true.
**
** The PGroup mutex must be held when this function is called.
*/
ساكن فراغ pcache1RemoveFromHash(PgHdr1 *p_صفحو, صحيح freeFlag){
  بنانشان صحيح h;
  PCache1 *pCache = p_صفحو->pCache;
  PgHdr1 **pp;

  أكد( sqlite3_mutex_held(pCache->p_گروهه->mutex) );
  أكد( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  h = p_صفحو->i_ڪنجي & (pCache->nHash-1);
  لكل(pp=&pCache->apHash[h]; (*pp)!=p_صفحو; pp=&(*pp)->p_ايندڙ);
  *pp = (*pp)->p_ايندڙ;

  pCache->n_صفحو--;
  إذا( freeFlag ) pcache1_Free_صفحو(p_صفحو);
}

/*
** If there are currently more than nMaxPage pages allocated, try
** to recycle pages to reduce the number allocated to nMaxPage.
*/
ساكن فراغ pcache1_Enforce_Max_صفحو(PCache1 *pCache){
  PGroup *p_گروهه = pCache->p_گروهه;
  PgHdr1 *p;
  أكد( sqlite3_mutex_held(p_گروهه->mutex) );
  طالما( p_گروهه->nPurgeable>p_گروهه->n_Max_صفحو
      && (p=p_گروهه->lru.pLruPrev)->isAnchor==0
  ){
    أكد( p->pCache->p_گروهه==p_گروهه );
    أكد( صفحو_IS_UNPINNED(p) );
    pcache1_Pin_صفحو(p);
    pcache1RemoveFromHash(p, 1);
  }
  إذا( pCache->n_صفحو==0 && pCache->pBulk ){
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
ساكن فراغ pcache1TruncateUnsafe(
  PCache1 *pCache,             /* The cache to truncate */
  بنانشان صحيح i_حد_بندي          /* Drop pages with this pgno or larger */
){
  TESTONLY( صحيح n_صفحو = 0; )  /* To assert pCache->nPage is correct */
  بنانشان صحيح h, iStop;
  أكد( sqlite3_mutex_held(pCache->p_گروهه->mutex) );
  أكد( pCache->i_Max_ڪنجي >= i_حد_بندي );
  أكد( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  إذا( pCache->i_Max_ڪنجي - i_حد_بندي < pCache->nHash ){
    /* If we are just shaving the last few pages off the end of the
    ** cache, then there is no point in scanning the entire hash table.
    ** Only scan those hash slots that might contain pages that need to
    ** be removed. */
    h = i_حد_بندي & (pCache->nHash-1);
    iStop = pCache->i_Max_ڪنجي & (pCache->nHash-1);
    TESTONLY( n_صفحو = -10; )  /* Disable the pCache->nPage validity check */
  }وإلا{
    /* This is the general case where many pages are being removed.
    ** It is necessary to scan the entire hash table */
    h = pCache->nHash/2;
    iStop = h - 1;
  }
  لكل(;;){
    PgHdr1 **pp;
    PgHdr1 *p_صفحو;
    أكد( h<pCache->nHash );
    pp = &pCache->apHash[h];
    طالما( (p_صفحو = *pp)!=0 ){
      إذا( p_صفحو->i_ڪنجي>=i_حد_بندي ){
        pCache->n_صفحو--;
        *pp = p_صفحو->p_ايندڙ;
        إذا( صفحو_IS_UNPINNED(p_صفحو) ) pcache1_Pin_صفحو(p_صفحو);
        pcache1_Free_صفحو(p_صفحو);
      }وإلا{
        pp = &p_صفحو->p_ايندڙ;
        TESTONLY( إذا( n_صفحو>=0 ) n_صفحو++; )
      }
    }
    إذا( h==iStop ) اكسر;
    h = (h+1) & (pCache->nHash-1);
  }
  أكد( n_صفحو<0 || pCache->n_صفحو==(بنانشان)n_صفحو );
}

/******************************************************************************/
/******** sqlite3_pcache Methods **********************************************/

/*
** Implementation of the sqlite3_pcache.xInit method.
*/
ساكن صحيح pcache1Init(فراغ *Not_استعمال_ٿيل){
  اڻ_استعمال_ٿيل_PARAMETER(Not_استعمال_ٿيل);
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
#إذا defined(SQLITE_ENABLE_يادگيري_MANAGEMENT)
  pcache1.separateCache = 0;
#وإلاإذا SQLITE_THREADSAFE
  pcache1.separateCache = sqlite3GlobalConfig.p_صفحو==0
                          || sqlite3GlobalConfig.bCoreMutex>0;
#وإلا
  pcache1.separateCache = sqlite3GlobalConfig.p_صفحو==0;
#جيڪڏهن_پورو

#إذا SQLITE_THREADSAFE
  إذا( sqlite3GlobalConfig.bCoreMutex ){
    pcache1.grp.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_LRU);
    pcache1.mutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_PMEM);
  }
#جيڪڏهن_پورو
  إذا( pcache1.separateCache
   && sqlite3GlobalConfig.n_صفحو!=0
   && sqlite3GlobalConfig.p_صفحو==0
  ){
    pcache1.n_Init_صفحو = sqlite3GlobalConfig.n_صفحو;
  }وإلا{
    pcache1.n_Init_صفحو = 0;
  }
  pcache1.grp.mxPinned = 10;
  pcache1.isInit = 1;
  أرجع SQLITE_OK;
}

/*
** Implementation of the sqlite3_pcache.xShutdown method.
** Note that the static mutex allocated in xInit does
** not need to be freed.
*/
ساكن فراغ pcache1Shutdown(فراغ *Not_استعمال_ٿيل){
  اڻ_استعمال_ٿيل_PARAMETER(Not_استعمال_ٿيل);
  أكد( pcache1.isInit!=0 );
  memset(&pcache1, 0, حجم(pcache1));
}

/* forward declaration */
ساكن فراغ pcache1Destroy(sqlite3_pcache *p);

/*
** Implementation of the sqlite3_pcache.xCreate method.
**
** Allocate a new cache.
*/
ساكن sqlite3_pcache *pcache1Create(صحيح sz_صفحو, صحيح szExtra, صحيح bPurgeable){
  PCache1 *pCache;      /* The newly created page cache */
  PGroup *p_گروهه;       /* The group the new page cache will belong to */
  i64 sz;               /* Bytes of memory required to allocate the new cache */

  أكد( (sz_صفحو & (sz_صفحو-1))==0 && sz_صفحو>=512 && sz_صفحو<=65536 );
  أكد( szExtra < 300 );

  sz = حجم(PCache1) + حجم(PGroup)*pcache1.separateCache;
  pCache = (PCache1 *)sqlite3MallocZero(sz);
  إذا( pCache ){
    إذا( pcache1.separateCache ){
      p_گروهه = (PGroup*)&pCache[1];
      p_گروهه->mxPinned = 10;
    }وإلا{
      p_گروهه = &pcache1.grp;
    }
    pcache1EnterMutex(p_گروهه);
    إذا( p_گروهه->lru.isAnchor==0 ){
      p_گروهه->lru.isAnchor = 1;
      p_گروهه->lru.pLruPrev = p_گروهه->lru.p_Lru_ايندڙ = &p_گروهه->lru;
    }
    pCache->p_گروهه = p_گروهه;
    pCache->sz_صفحو = sz_صفحو;
    pCache->szExtra = szExtra;
    pCache->szAlloc = sz_صفحو + szExtra + ROUND8(حجم(PgHdr1));
    pCache->bPurgeable = (bPurgeable ? 1 : 0);
    pcache1ResizeHash(pCache);
    إذا( bPurgeable ){
      pCache->nMin = 10;
      p_گروهه->n_Min_صفحو += pCache->nMin;
      p_گروهه->mxPinned = p_گروهه->n_Max_صفحو + 10 - p_گروهه->n_Min_صفحو;
      pCache->pnPurgeable = &p_گروهه->nPurgeable;
    }وإلا{
      pCache->pnPurgeable = &pCache->nPurgeableDummy;
    }
    pcache1LeaveMutex(p_گروهه);
    إذا( pCache->nHash==0 ){
      pcache1Destroy((sqlite3_pcache*)pCache);
      pCache = 0;
    }
  }
  أرجع (sqlite3_pcache *)pCache;
}

/*
** Implementation of the sqlite3_pcache.xCachesize method.
**
** Configure the cache_size limit for a cache.
*/
ساكن فراغ pcache1Cachesize(sqlite3_pcache *p, صحيح nMax){
  PCache1 *pCache = (PCache1 *)p;
  u32 n;
  أكد( nMax>=0 );
  إذا( pCache->bPurgeable ){
    PGroup *p_گروهه = pCache->p_گروهه;
    pcache1EnterMutex(p_گروهه);
    n = (u32)nMax;
    إذا( n > 0x7fff0000 - p_گروهه->n_Max_صفحو + pCache->nMax ){
      n = 0x7fff0000 - p_گروهه->n_Max_صفحو + pCache->nMax;
    }
    p_گروهه->n_Max_صفحو += (n - pCache->nMax);
    p_گروهه->mxPinned = p_گروهه->n_Max_صفحو + 10 - p_گروهه->n_Min_صفحو;
    pCache->nMax = n;
    pCache->n90pct = pCache->nMax*9/10;
    pcache1_Enforce_Max_صفحو(pCache);
    pcache1LeaveMutex(p_گروهه);
  }
}

/*
** Implementation of the sqlite3_pcache.xShrink method.
**
** Free up as much memory as possible.
*/
ساكن فراغ pcache1Shrink(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1*)p;
  إذا( pCache->bPurgeable ){
    PGroup *p_گروهه = pCache->p_گروهه;
    بنانشان صحيح saved_Max_صفحو;
    pcache1EnterMutex(p_گروهه);
    saved_Max_صفحو = p_گروهه->n_Max_صفحو;
    p_گروهه->n_Max_صفحو = 0;
    pcache1_Enforce_Max_صفحو(pCache);
    p_گروهه->n_Max_صفحو = saved_Max_صفحو;
    pcache1LeaveMutex(p_گروهه);
  }
}

/*
** Implementation of the sqlite3_pcache.xPagecount method.
*/
ساكن صحيح pcache1Pagecount(sqlite3_pcache *p){
  صحيح n;
  PCache1 *pCache = (PCache1*)p;
  pcache1EnterMutex(pCache->p_گروهه);
  n = pCache->n_صفحو;
  pcache1LeaveMutex(pCache->p_گروهه);
  أرجع n;
}


/*
** Implement steps 3, 4, and 5 of the pcache1Fetch() algorithm described
** in the header of the pcache1Fetch() procedure.
**
** This steps are broken out into a separate procedure because they are
** usually not needed, and by avoiding the stack initialization required
** for these steps, the main pcache1Fetch() procedure can run faster.
*/
ساكن SQLITE_NOINLINE PgHdr1 *pcache1FetchStage2(
  PCache1 *pCache,
  بنانشان صحيح i_ڪنجي,
  صحيح createFlag
){
  بنانشان صحيح nPinned;
  PGroup *p_گروهه = pCache->p_گروهه;
  PgHdr1 *p_صفحو = 0;

  /* Step 3: Abort if createFlag is 1 but the cache is nearly full */
  أكد( pCache->n_صفحو >= pCache->nRecyclable );
  nPinned = pCache->n_صفحو - pCache->nRecyclable;
  أكد( p_گروهه->mxPinned == p_گروهه->n_Max_صفحو + 10 - p_گروهه->n_Min_صفحو );
  أكد( pCache->n90pct == pCache->nMax*9/10 );
  إذا( createFlag==1 && (
        nPinned>=p_گروهه->mxPinned
     || nPinned>=pCache->n90pct
     || (pcache1_Under_يادگيري_Pressure(pCache) && pCache->nRecyclable<nPinned)
  )){
    أرجع 0;
  }

  إذا( pCache->n_صفحو>=pCache->nHash ) pcache1ResizeHash(pCache);
  أكد( pCache->nHash>0 && pCache->apHash );
  أكد( (pCache->nHash & (pCache->nHash-1))==0 );

  /* Step 4. Try to recycle a page. */
  إذا( pCache->bPurgeable
   && !p_گروهه->lru.pLruPrev->isAnchor
   && ((pCache->n_صفحو+1>=pCache->nMax) || pcache1_Under_يادگيري_Pressure(pCache))
  ){
    PCache1 *pOther;
    p_صفحو = p_گروهه->lru.pLruPrev;
    أكد( صفحو_IS_UNPINNED(p_صفحو) );
    pcache1RemoveFromHash(p_صفحو, 0);
    pcache1_Pin_صفحو(p_صفحو);
    pOther = p_صفحو->pCache;
    إذا( pOther->szAlloc != pCache->szAlloc ){
      pcache1_Free_صفحو(p_صفحو);
      p_صفحو = 0;
    }وإلا{
      p_گروهه->nPurgeable -= (pOther->bPurgeable - pCache->bPurgeable);
    }
  }

  /* Step 5. If a usable page buffer has still not been found,
  ** attempt to allocate a new one.
  */
  إذا( !p_صفحو ){
    p_صفحو = pcache1_Alloc_صفحو(pCache, createFlag==1);
  }

  إذا( p_صفحو ){
    بنانشان صحيح h = i_ڪنجي & (pCache->nHash-1);
    pCache->n_صفحو++;
    p_صفحو->i_ڪنجي = i_ڪنجي;
    p_صفحو->p_ايندڙ = pCache->apHash[h];
    p_صفحو->pCache = pCache;
    p_صفحو->p_Lru_ايندڙ = 0;
    /* pPage->pLruPrev = 0;
    ** No need to clear pLruPrev since it is not accessed when pLruNext==0 */
    *(فراغ **)p_صفحو->صفحو.pExtra = 0;
    pCache->apHash[h] = p_صفحو;
    إذا( i_ڪنجي>pCache->i_Max_ڪنجي ){
      pCache->i_Max_ڪنجي = i_ڪنجي;
    }
  }
  أرجع p_صفحو;
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
ساكن PgHdr1 *pcache1FetchNoMutex(
  sqlite3_pcache *p,
  بنانشان صحيح i_ڪنجي,
  صحيح createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_صفحو = 0;

  /* Step 1: Search the hash table for an existing entry.  nHash is always
  ** a power of two when the cache is in use (see pcache1ResizeHash()), so
  ** the modulo reduces to a mask, avoiding a hardware divide. */
  أكد( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  p_صفحو = pCache->apHash[i_ڪنجي & (pCache->nHash-1u)];
  طالما( p_صفحو && p_صفحو->i_ڪنجي!=i_ڪنجي ){ p_صفحو = p_صفحو->p_ايندڙ; }

  /* Step 2: If the page was found in the hash table, then return it.
  ** If the page was not in the hash table and createFlag is 0, abort.
  ** Otherwise (page not in hash and createFlag!=0) continue with
  ** subsequent steps to try to create the page. */
  إذا( p_صفحو ){
    إذا( صفحو_IS_UNPINNED(p_صفحو) ){
      أرجع pcache1_Pin_صفحو(p_صفحو);
    }وإلا{
      أرجع p_صفحو;
    }
  }وإلا إذا( createFlag ){
    /* Steps 3, 4, and 5 implemented by this subroutine */
    أرجع pcache1FetchStage2(pCache, i_ڪنجي, createFlag);
  }وإلا{
    أرجع 0;
  }
}
#إذا PCACHE1_MIGHT_USE_گروهه_MUTEX
ساكن PgHdr1 *pcache1FetchWithMutex(
  sqlite3_pcache *p,
  بنانشان صحيح i_ڪنجي,
  صحيح createFlag
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_صفحو;

  pcache1EnterMutex(pCache->p_گروهه);
  p_صفحو = pcache1FetchNoMutex(p, i_ڪنجي, createFlag);
  أكد( p_صفحو==0 || pCache->i_Max_ڪنجي>=i_ڪنجي );
  pcache1LeaveMutex(pCache->p_گروهه);
  أرجع p_صفحو;
}
#جيڪڏهن_پورو
ساكن sqlite3_pcache_صفحو *pcache1Fetch(
  sqlite3_pcache *p,
  بنانشان صحيح i_ڪنجي,
  صحيح createFlag
){
#إذا PCACHE1_MIGHT_USE_گروهه_MUTEX || defined(SQLITE_DEBUG)
  PCache1 *pCache = (PCache1 *)p;
#جيڪڏهن_پورو

  أكد( offsetof(PgHdr1,صفحو)==0 );
  أكد( pCache->bPurgeable || createFlag!=1 );
  أكد( pCache->bPurgeable || pCache->nMin==0 );
  أكد( pCache->bPurgeable==0 || pCache->nMin==10 );
  أكد( pCache->nMin==0 || pCache->bPurgeable );
  أكد( pCache->nHash>0 );
#إذا PCACHE1_MIGHT_USE_گروهه_MUTEX
  إذا( pCache->p_گروهه->mutex ){
    أرجع (sqlite3_pcache_صفحو*)pcache1FetchWithMutex(p, i_ڪنجي, createFlag);
  }وإلا
#جيڪڏهن_پورو
  {
    أرجع (sqlite3_pcache_صفحو*)pcache1FetchNoMutex(p, i_ڪنجي, createFlag);
  }
}


/*
** Implementation of the sqlite3_pcache.xUnpin method.
**
** Mark a page as unpinned (eligible for asynchronous recycling).
*/
ساكن فراغ pcache1Unpin(
  sqlite3_pcache *p,
  sqlite3_pcache_صفحو *pPg,
  صحيح reuseUnlikely
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_صفحو = (PgHdr1 *)pPg;
  PGroup *p_گروهه = pCache->p_گروهه;

  أكد( p_صفحو->pCache==pCache );
  pcache1EnterMutex(p_گروهه);

  /* It is an error to call this function if the page is already
  ** part of the PGroup LRU list.
  */
  أكد( p_صفحو->p_Lru_ايندڙ==0 );
  أكد( صفحو_IS_PINNED(p_صفحو) );

  إذا( reuseUnlikely || p_گروهه->nPurgeable>p_گروهه->n_Max_صفحو ){
    pcache1RemoveFromHash(p_صفحو, 1);
  }وإلا{
    /* Add the page to the PGroup LRU list. */
    PgHdr1 **pp_پهريون = &p_گروهه->lru.p_Lru_ايندڙ;
    p_صفحو->pLruPrev = &p_گروهه->lru;
    (p_صفحو->p_Lru_ايندڙ = *pp_پهريون)->pLruPrev = p_صفحو;
    *pp_پهريون = p_صفحو;
    pCache->nRecyclable++;
  }

  pcache1LeaveMutex(pCache->p_گروهه);
}

/*
** Implementation of the sqlite3_pcache.xRekey method.
*/
ساكن فراغ pcache1Rekey(
  sqlite3_pcache *p,
  sqlite3_pcache_صفحو *pPg,
  بنانشان صحيح iOld,
  بنانشان صحيح iNew
){
  PCache1 *pCache = (PCache1 *)p;
  PgHdr1 *p_صفحو = (PgHdr1 *)pPg;
  PgHdr1 **pp;
  بنانشان صحيح hOld, hNew;
  أكد( p_صفحو->i_ڪنجي==iOld );
  أكد( p_صفحو->pCache==pCache );
  أكد( iOld!=iNew );               /* The page number really is changing */

  pcache1EnterMutex(pCache->p_گروهه);

  أكد( pcache1FetchNoMutex(p, iOld, 0)==p_صفحو ); /* pPg really is iOld */
  أكد( pCache->nHash>0 && (pCache->nHash & (pCache->nHash-1))==0 );
  hOld = iOld & (pCache->nHash-1);
  pp = &pCache->apHash[hOld];
  طالما( (*pp)!=p_صفحو ){
    pp = &(*pp)->p_ايندڙ;
  }
  *pp = p_صفحو->p_ايندڙ;

  أكد( pcache1FetchNoMutex(p, iNew, 0)==0 ); /* iNew not in cache */
  hNew = iNew & (pCache->nHash-1);
  p_صفحو->i_ڪنجي = iNew;
  p_صفحو->p_ايندڙ = pCache->apHash[hNew];
  pCache->apHash[hNew] = p_صفحو;
  إذا( iNew>pCache->i_Max_ڪنجي ){
    pCache->i_Max_ڪنجي = iNew;
  }

  pcache1LeaveMutex(pCache->p_گروهه);
}

/*
** Implementation of the sqlite3_pcache.xTruncate method.
**
** Discard all unpinned pages in the cache with a page number equal to
** or greater than parameter iLimit. Any pinned pages with a page number
** equal to or greater than iLimit are implicitly unpinned.
*/
ساكن فراغ pcache1Truncate(sqlite3_pcache *p, بنانشان صحيح i_حد_بندي){
  PCache1 *pCache = (PCache1 *)p;
  pcache1EnterMutex(pCache->p_گروهه);
  إذا( i_حد_بندي<=pCache->i_Max_ڪنجي ){
    pcache1TruncateUnsafe(pCache, i_حد_بندي);
    pCache->i_Max_ڪنجي = i_حد_بندي-1;
  }
  pcache1LeaveMutex(pCache->p_گروهه);
}

/*
** Implementation of the sqlite3_pcache.xDestroy method.
**
** Destroy a cache allocated using pcache1Create().
*/
ساكن فراغ pcache1Destroy(sqlite3_pcache *p){
  PCache1 *pCache = (PCache1 *)p;
  PGroup *p_گروهه = pCache->p_گروهه;
  أكد( pCache->bPurgeable || (pCache->nMax==0 && pCache->nMin==0) );
  pcache1EnterMutex(p_گروهه);
  إذا( pCache->n_صفحو ) pcache1TruncateUnsafe(pCache, 0);
  أكد( p_گروهه->n_Max_صفحو >= pCache->nMax );
  p_گروهه->n_Max_صفحو -= pCache->nMax;
  أكد( p_گروهه->n_Min_صفحو >= pCache->nMin );
  p_گروهه->n_Min_صفحو -= pCache->nMin;
  p_گروهه->mxPinned = p_گروهه->n_Max_صفحو + 10 - p_گروهه->n_Min_صفحو;
  pcache1_Enforce_Max_صفحو(pCache);
  pcache1LeaveMutex(p_گروهه);
  sqlite3_free(pCache->pBulk);
  sqlite3_free(pCache->apHash);
  sqlite3_free(pCache);
}

/*
** This function is called during initialization (sqlite3_initialize()) to
** install the default pluggable cache module, assuming the user has not
** already provided an alternative.
*/
فراغ sqlite3_PCache_سيٽ_Default(فراغ){
  ساكن ثابت sqlite3_pcache_methods2 defaultMethods = {
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
صحيح sqlite3_سرو_سائيز_Pcache1(فراغ){ أرجع ROUND8(حجم(PgHdr1)); }

/*
** Return the global mutex used by this PCACHE implementation.  The
** sqlite3_status() routine needs access to this mutex.
*/
sqlite3_mutex *sqlite3Pcache1Mutex(فراغ){
  أرجع pcache1.mutex;
}

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_يادگيري_MANAGEMENT
/*
** This function is called to free superfluous dynamically allocated memory
** held by the pager system. Memory in use by any SQLite pager allocated
** by the current thread may be sqlite3_free()ed.
**
** nReq is the number of bytes of memory required. Once this much has
** been released, the function returns. The return value is the total number
** of bytes of memory released.
*/
صحيح sqlite3_Pcache_Release_يادگيري(صحيح nReq){
  صحيح nFree = 0;
  أكد( sqlite3_mutex_notheld(pcache1.grp.mutex) );
  أكد( sqlite3_mutex_notheld(pcache1.mutex) );
  إذا( sqlite3GlobalConfig.p_صفحو==0 ){
    PgHdr1 *p;
    pcache1EnterMutex(&pcache1.grp);
    طالما( (nReq<0 || nFree<nReq)
       &&  (p=pcache1.grp.lru.pLruPrev)!=0
       &&  p->isAnchor==0
    ){
      nFree += pcache1_Mem_سائيز(p->صفحو.pBuf);
      أكد( صفحو_IS_UNPINNED(p) );
      pcache1_Pin_صفحو(p);
      pcache1RemoveFromHash(p, 1);
    }
    pcache1LeaveMutex(&pcache1.grp);
  }
  أرجع nFree;
}
#جيڪڏهن_پورو /* SQLITE_ENABLE_MEMORY_MANAGEMENT */

#جيڪڏهن_وصف_ٿيل SQLITE_TEST
/*
** This function is used by test procedures to inspect the internal state
** of the global cache.
*/
فراغ sqlite3PcacheStats(
  صحيح *pnCurrent,      /* OUT: Total number of pages cached */
  صحيح *pnMax,          /* OUT: Global maximum cache size */
  صحيح *pnMin,          /* OUT: Sum of PCache1.nMin for purgeable caches */
  صحيح *pnRecyclable    /* OUT: Total number of pages available for recycling */
){
  PgHdr1 *p;
  صحيح nRecyclable = 0;
  لكل(p=pcache1.grp.lru.p_Lru_ايندڙ; p && !p->isAnchor; p=p->p_Lru_ايندڙ){
    أكد( صفحو_IS_UNPINNED(p) );
    nRecyclable++;
  }
  *pnCurrent = pcache1.grp.nPurgeable;
  *pnMax = (صحيح)pcache1.grp.n_Max_صفحو;
  *pnMin = (صحيح)pcache1.grp.n_Min_صفحو;
  *pnRecyclable = nRecyclable;
}
#جيڪڏهن_پورو
