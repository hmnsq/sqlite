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
** This header file defines the interface that the sqlite page cache
** subsystem. 
*/

#ifndef _PCACHE_H_
#define _PCACHE_H_

প্রকারনির্দেশ সংরচনা PgHdr PgHdr;
প্রকারনির্দেশ সংরচনা PCache PCache;

/*
** Every page in the cache is controlled by an instance of the following
** structure.
*/
সংরচনা PgHdr {
  sqlite3_pcache_page *pPage;    /* Pcache object page handle */
  শূন্য *pData;                   /* Page data */
  শূন্য *pExtra;                  /* Extra content */
  PCache *pCache;                /* PRIVATE: Cache that owns this page */
  PgHdr *pDirty;                 /* Transient list of dirty sorted by pgno */
  Pager *pPager;                 /* The pager this page is part of */
#ifdef SQLITE_CHECK_PAGES
  u64 pageHash;                  /* Hash of page content */
#endif
  Pgno pgno;                     /* Page number for this page */
  u16 flags;                     /* PGHDR flags defined below */

  /**********************************************************************
  ** Elements above, except pCache, are public.  All that follow are 
  ** private to pcache.c and should not be accessed by other modules.
  ** pCache is grouped with the public elements for efficiency.
  */
  i64 nRef;                      /* Number of users of this page */
  PgHdr *pDirtyNext;             /* Next element in list of dirty pages */
  PgHdr *pDirtyPrev;             /* Previous element in list of dirty pages */
                          /* NB: pDirtyNext and pDirtyPrev are undefined if the
                          ** PgHdr object is not dirty */
};

/* Bit values for PgHdr.flags */
#define PGHDR_CLEAN           0x001  /* Page not on the PCache.pDirty list */
#define PGHDR_DIRTY           0x002  /* Page is on the PCache.pDirty list */
#define PGHDR_WRITEABLE       0x004  /* Journaled and ready to modify */
#define PGHDR_NEED_SYNC       0x008  /* Fsync the rollback journal before
                                     ** writing this page to the database */
#define PGHDR_DONT_WRITE      0x010  /* Do not write content to disk */
#define PGHDR_MMAP            0x020  /* This is an mmap page object */

#define PGHDR_WAL_APPEND      0x040  /* Appended to wal file */

/* Initialize and shutdown the page cache subsystem */
পূর্ণাঙ্ক sqlite3PcacheInitialize(শূন্য);
শূন্য sqlite3PcacheShutdown(শূন্য);

/* Page cache buffer management:
** These routines implement SQLITE_CONFIG_PAGECACHE.
*/
শূন্য sqlite3PCacheBufferSetup(শূন্য *, পূর্ণাঙ্ক sz, পূর্ণাঙ্ক n);

/* Create a new pager cache.
** Under memory stress, invoke xStress to try to make pages clean.
** Only clean and unpinned pages can be reclaimed.
*/
পূর্ণাঙ্ক sqlite3PcacheOpen(
  পূর্ণাঙ্ক szPage,                    /* Size of every page */
  পূর্ণাঙ্ক szExtra,                   /* Extra space associated with each page */
  পূর্ণাঙ্ক bPurgeable,                /* True if pages are on backing store */
  পূর্ণাঙ্ক (*xStress)(শূন্য*, PgHdr*), /* Call to try to make pages clean */
  শূন্য *pStress,                 /* Argument to xStress */
  PCache *pToInit                /* Preallocated space for the PCache */
);

/* Modify the page-size after the cache has been created. */
পূর্ণাঙ্ক sqlite3PcacheSetPageSize(PCache *, পূর্ণাঙ্ক);

/* Return the size in bytes of a PCache object.  Used to preallocate
** storage space.
*/
পূর্ণাঙ্ক sqlite3PcacheSize(শূন্য);

/* One release per successful fetch.  Page is pinned until released.
** Reference counted. 
*/
sqlite3_pcache_page *sqlite3PcacheFetch(PCache*, Pgno, পূর্ণাঙ্ক createFlag);
পূর্ণাঙ্ক sqlite3PcacheFetchStress(PCache*, Pgno, sqlite3_pcache_page**);
PgHdr *sqlite3PcacheFetchFinish(PCache*, Pgno, sqlite3_pcache_page *pPage);
শূন্য sqlite3PcacheRelease(PgHdr*);

শূন্য sqlite3PcacheDrop(PgHdr*);         /* Remove page from cache */
শূন্য sqlite3PcacheMakeDirty(PgHdr*);    /* Make sure page is marked dirty */
শূন্য sqlite3PcacheMakeClean(PgHdr*);    /* Mark a single page as clean */
শূন্য sqlite3PcacheCleanAll(PCache*);    /* Mark all dirty list pages as clean */
শূন্য sqlite3PcacheClearWritable(PCache*);

/* Change a page number.  Used by incr-vacuum. */
শূন্য sqlite3PcacheMove(PgHdr*, Pgno);

/* Remove all pages with pgno>x.  Reset the cache if x==0 */
শূন্য sqlite3PcacheTruncate(PCache*, Pgno x);

/* Get a list of all dirty pages in the cache, sorted by page number */
PgHdr *sqlite3PcacheDirtyList(PCache*);

/* Reset and close the cache object */
শূন্য sqlite3PcacheClose(PCache*);

/* Clear flags from pages of the page cache */
শূন্য sqlite3PcacheClearSyncFlags(PCache *);

/* Discard the contents of the cache */
শূন্য sqlite3PcacheClear(PCache*);

/* Return the total number of outstanding page references */
i64 sqlite3_Pcache_Ref_গুনতি(PCache*);

/* Increment the reference count of an existing page */
শূন্য sqlite3PcacheRef(PgHdr*);

i64 sqlite3PcachePageRefcount(PgHdr*);

/* Return the total number of pages stored in the cache */
পূর্ণাঙ্ক sqlite3PcachePagecount(PCache*);

#যদি defined(SQLITE_CHECK_PAGES) || defined(SQLITE_DEBUG)
/* Iterate through all dirty pages currently stored in the cache. This
** interface is only available if SQLITE_CHECK_PAGES is defined when the 
** library is built.
*/
শূন্য sqlite3PcacheIterateDirty(PCache *pCache, শূন্য (*xIter)(PgHdr *));
#endif

#যদি defined(SQLITE_DEBUG)
/* Check invariants on a PgHdr object */
পূর্ণাঙ্ক sqlite3PcachePageSanity(PgHdr*);
#endif

/* Set and get the suggested cache-size for the specified pager-cache.
**
** If no global maximum is configured, then the system attempts to limit
** the total number of pages cached by purgeable pager-caches to the sum
** of the suggested cache-sizes.
*/
শূন্য sqlite3PcacheSetCachesize(PCache *, পূর্ণাঙ্ক);
#ifdef SQLITE_TEST
পূর্ণাঙ্ক sqlite3PcacheGetCachesize(PCache *);
#endif

/* Set or get the suggested spill-size for the specified pager-cache.
**
** The spill-size is the minimum number of pages in cache before the cache
** will attempt to spill dirty pages by calling xStress.
*/
পূর্ণাঙ্ক sqlite3PcacheSetSpillsize(PCache *, পূর্ণাঙ্ক);

/* Free up as much memory as possible from the page cache */
শূন্য sqlite3PcacheShrink(PCache*);

#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
/* Try to return memory used by the pcache module to the main memory heap */
পূর্ণাঙ্ক sqlite3PcacheReleaseMemory(পূর্ণাঙ্ক);
#endif

#ifdef SQLITE_TEST
শূন্য sqlite3PcacheStats(পূর্ণাঙ্ক*,পূর্ণাঙ্ক*,পূর্ণাঙ্ক*,পূর্ণাঙ্ক*);
#endif

শূন্য sqlite3PCacheSetDefault(শূন্য);

/* Return the header size */
পূর্ণাঙ্ক sqlite3HeaderSizePcache(শূন্য);
পূর্ণাঙ্ক sqlite3HeaderSizePcache1(শূন্য);

/* Number of dirty pages as a percentage of the configured cache size */
পূর্ণাঙ্ক sqlite3PCachePercentDirty(PCache*);

#ifdef SQLITE_DIRECT_OVERFLOW_READ
পূর্ণাঙ্ক sqlite3PCacheIsDirty(PCache *pCache);
#endif

#endif /* _PCACHE_H_ */
