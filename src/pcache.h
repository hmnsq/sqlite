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

#که_نه_تعریف_شوی _PCACHE_H_
#تعریف_کړه _PCACHE_H_

تعريفنوع بنية PgHdr PgHdr;
تعريفنوع بنية PCache PCache;

/*
** Every page in the cache is controlled by an instance of the following
** structure.
*/
بنية PgHdr {
  sqlite3_pcache_پاڼه *p_پاڼه;    /* Pcache object page handle */
  فراغ *p_معلومات;                   /* Page data */
  فراغ *pExtra;                  /* Extra content */
  PCache *pCache;                /* PRIVATE: Cache that owns this page */
  PgHdr *pDirty;                 /* Transient list of dirty sorted by pgno */
  Pager *pPager;                 /* The pager this page is part of */
#که_تعریف_شوی SQLITE_CHECK_پاڼې
  u64 پاڼه_Hash;                  /* Hash of page content */
#د_که_پای
  Pgno pgno;                     /* Page number for this page */
  u16 flags;                     /* PGHDR flags defined below */

  /**********************************************************************
  ** Elements above, except pCache, are public.  All that follow are 
  ** private to pcache.c and should not be accessed by other modules.
  ** pCache is grouped with the public elements for efficiency.
  */
  i64 nRef;                      /* Number of users of this page */
  PgHdr *p_Dirty_بل;             /* Next element in list of dirty pages */
  PgHdr *pDirtyPrev;             /* Previous element in list of dirty pages */
                          /* NB: pDirtyNext and pDirtyPrev are undefined if the
                          ** PgHdr object is not dirty */
};

/* Bit values for PgHdr.flags */
#تعریف_کړه PGHDR_CLEAN           0x001  /* Page not on the PCache.pDirty list */
#تعریف_کړه PGHDR_DIRTY           0x002  /* Page is on the PCache.pDirty list */
#تعریف_کړه PGHDR_WRITEABLE       0x004  /* Journaled and ready to modify */
#تعریف_کړه PGHDR_NEED_SYNC       0x008  /* Fsync the rollback journal before
                                     ** writing this page to the database */
#تعریف_کړه PGHDR_DONT_WRITE      0x010  /* Do not write content to disk */
#تعریف_کړه PGHDR_MMAP            0x020  /* This is an mmap page object */

#تعریف_کړه PGHDR_WAL_APPEND      0x040  /* Appended to wal file */

/* Initialize and shutdown the page cache subsystem */
صحيح sqlite3PcacheInitialize(فراغ);
فراغ sqlite3PcacheShutdown(فراغ);

/* Page cache buffer management:
** These routines implement SQLITE_CONFIG_PAGECACHE.
*/
فراغ sqlite3_PCache_بفر_تیاری(فراغ *, صحيح sz, صحيح n);

/* Create a new pager cache.
** Under memory stress, invoke xStress to try to make pages clean.
** Only clean and unpinned pages can be reclaimed.
*/
صحيح sqlite3PcacheOpen(
  صحيح sz_پاڼه,                    /* Size of every page */
  صحيح szExtra,                   /* Extra space associated with each page */
  صحيح bPurgeable,                /* True if pages are on backing store */
  صحيح (*xStress)(فراغ*, PgHdr*), /* Call to try to make pages clean */
  فراغ *pStress,                 /* Argument to xStress */
  PCache *p_منزل_Init                /* Preallocated space for the PCache */
);

/* Modify the page-size after the cache has been created. */
صحيح sqlite3_Pcache_سېټ_پاڼه_اندازه(PCache *, صحيح);

/* Return the size in bytes of a PCache object.  Used to preallocate
** storage space.
*/
صحيح sqlite3_Pcache_اندازه(فراغ);

/* One release per successful fetch.  Page is pinned until released.
** Reference counted. 
*/
sqlite3_pcache_پاڼه *sqlite3PcacheFetch(PCache*, Pgno, صحيح createFlag);
صحيح sqlite3PcacheFetchStress(PCache*, Pgno, sqlite3_pcache_پاڼه**);
PgHdr *sqlite3_Pcache_Fetch_پای(PCache*, Pgno, sqlite3_pcache_پاڼه *p_پاڼه);
فراغ sqlite3PcacheRelease(PgHdr*);

فراغ sqlite3PcacheDrop(PgHdr*);         /* Remove page from cache */
فراغ sqlite3PcacheMakeDirty(PgHdr*);    /* Make sure page is marked dirty */
فراغ sqlite3PcacheMakeClean(PgHdr*);    /* Mark a single page as clean */
فراغ sqlite3PcacheCleanAll(PCache*);    /* Mark all dirty list pages as clean */
فراغ sqlite3PcacheClearWritable(PCache*);

/* Change a page number.  Used by incr-vacuum. */
فراغ sqlite3PcacheMove(PgHdr*, Pgno);

/* Remove all pages with pgno>x.  Reset the cache if x==0 */
فراغ sqlite3PcacheTruncate(PCache*, Pgno x);

/* Get a list of all dirty pages in the cache, sorted by page number */
PgHdr *sqlite3PcacheDirtyList(PCache*);

/* Reset and close the cache object */
فراغ sqlite3PcacheClose(PCache*);

/* Clear flags from pages of the page cache */
فراغ sqlite3PcacheClearSyncFlags(PCache *);

/* Discard the contents of the cache */
فراغ sqlite3PcacheClear(PCache*);

/* Return the total number of outstanding page references */
i64 sqlite3_Pcache_Ref_شمېر(PCache*);

/* Increment the reference count of an existing page */
فراغ sqlite3PcacheRef(PgHdr*);

i64 sqlite3_Pcache_پاڼه_Refcount(PgHdr*);

/* Return the total number of pages stored in the cache */
صحيح sqlite3PcachePagecount(PCache*);

#إذا defined(SQLITE_CHECK_پاڼې) || defined(SQLITE_DEBUG)
/* Iterate through all dirty pages currently stored in the cache. This
** interface is only available if SQLITE_CHECK_PAGES is defined when the 
** library is built.
*/
فراغ sqlite3PcacheIterateDirty(PCache *pCache, فراغ (*xIter)(PgHdr *));
#د_که_پای

#إذا defined(SQLITE_DEBUG)
/* Check invariants on a PgHdr object */
صحيح sqlite3_Pcache_پاڼه_Sanity(PgHdr*);
#د_که_پای

/* Set and get the suggested cache-size for the specified pager-cache.
**
** If no global maximum is configured, then the system attempts to limit
** the total number of pages cached by purgeable pager-caches to the sum
** of the suggested cache-sizes.
*/
فراغ sqlite3_Pcache_سېټ_Cachesize(PCache *, صحيح);
#که_تعریف_شوی SQLITE_TEST
صحيح sqlite3PcacheGetCachesize(PCache *);
#د_که_پای

/* Set or get the suggested spill-size for the specified pager-cache.
**
** The spill-size is the minimum number of pages in cache before the cache
** will attempt to spill dirty pages by calling xStress.
*/
صحيح sqlite3_Pcache_سېټ_Spillsize(PCache *, صحيح);

/* Free up as much memory as possible from the page cache */
فراغ sqlite3PcacheShrink(PCache*);

#که_تعریف_شوی SQLITE_ENABLE_حافظه_MANAGEMENT
/* Try to return memory used by the pcache module to the main memory heap */
صحيح sqlite3_Pcache_Release_حافظه(صحيح);
#د_که_پای

#که_تعریف_شوی SQLITE_TEST
فراغ sqlite3PcacheStats(صحيح*,صحيح*,صحيح*,صحيح*);
#د_که_پای

فراغ sqlite3_PCache_سېټ_Default(فراغ);

/* Return the header size */
صحيح sqlite3_سرلیک_اندازه_Pcache(فراغ);
صحيح sqlite3_سرلیک_اندازه_Pcache1(فراغ);

/* Number of dirty pages as a percentage of the configured cache size */
صحيح sqlite3PCachePercentDirty(PCache*);

#که_تعریف_شوی SQLITE_مستقیم_ډکېدنه_READ
صحيح sqlite3PCacheIsDirty(PCache *pCache);
#د_که_پای

#د_که_پای /* _PCACHE_H_ */
