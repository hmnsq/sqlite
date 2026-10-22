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

#اگر_غیر_تعریف_شدہ _PCACHE_H_
#تعریف_کرو _PCACHE_H_

قسمتعریف ساخت PgHdr PgHdr;
قسمتعریف ساخت PCache PCache;

/*
** Every page in the cache is controlled by an instance of the following
** structure.
*/
ساخت PgHdr {
  sqlite3_pcache_صفحہ *p_صفحہ;    /* Pcache object page handle */
  خالی *p_ڈیٹا;                   /* Page data */
  خالی *pExtra;                  /* Extra content */
  PCache *pCache;                /* PRIVATE: Cache that owns this page */
  PgHdr *pDirty;                 /* Transient list of dirty sorted by pgno */
  Pager *pPager;                 /* The pager this page is part of */
#اگر_تعریف_شدہ SQLITE_CHECK_صفحات
  u64 صفحہ_Hash;                  /* Hash of page content */
#اگر_ختم
  Pgno pgno;                     /* Page number for this page */
  u16 flags;                     /* PGHDR flags defined below */

  /**********************************************************************
  ** Elements above, except pCache, are public.  All that follow are 
  ** private to pcache.c and should not be accessed by other modules.
  ** pCache is grouped with the public elements for efficiency.
  */
  i64 nRef;                      /* Number of users of this page */
  PgHdr *p_Dirty_اگلا;             /* Next element in list of dirty pages */
  PgHdr *p_Dirty_پچھلا;             /* Previous element in list of dirty pages */
                          /* NB: pDirtyNext and pDirtyPrev are undefined if the
                          ** PgHdr object is not dirty */
};

/* Bit values for PgHdr.flags */
#تعریف_کرو PGHDR_CLEAN           0x001  /* Page not on the PCache.pDirty list */
#تعریف_کرو PGHDR_DIRTY           0x002  /* Page is on the PCache.pDirty list */
#تعریف_کرو PGHDR_WRITEABLE       0x004  /* Journaled and ready to modify */
#تعریف_کرو PGHDR_NEED_SYNC       0x008  /* Fsync the rollback journal before
                                     ** writing this page to the database */
#تعریف_کرو PGHDR_DONT_WRITE      0x010  /* Do not write content to disk */
#تعریف_کرو PGHDR_MMAP            0x020  /* This is an mmap page object */

#تعریف_کرو PGHDR_WAL_APPEND      0x040  /* Appended to wal file */

/* Initialize and shutdown the page cache subsystem */
صحیح sqlite3PcacheInitialize(خالی);
خالی sqlite3PcacheShutdown(خالی);

/* Page cache buffer management:
** These routines implement SQLITE_CONFIG_PAGECACHE.
*/
خالی sqlite3_PCache_بفر_تیاری(خالی *, صحیح sz, صحیح n);

/* Create a new pager cache.
** Under memory stress, invoke xStress to try to make pages clean.
** Only clean and unpinned pages can be reclaimed.
*/
صحیح sqlite3PcacheOpen(
  صحیح sz_صفحہ,                    /* Size of every page */
  صحیح szExtra,                   /* Extra space associated with each page */
  صحیح bPurgeable,                /* True if pages are on backing store */
  صحیح (*xStress)(خالی*, PgHdr*), /* Call to try to make pages clean */
  خالی *pStress,                 /* Argument to xStress */
  PCache *p_مقصد_Init                /* Preallocated space for the PCache */
);

/* Modify the page-size after the cache has been created. */
صحیح sqlite3_Pcache_سیٹ_صفحہ_سائز(PCache *, صحیح);

/* Return the size in bytes of a PCache object.  Used to preallocate
** storage space.
*/
صحیح sqlite3_Pcache_سائز(خالی);

/* One release per successful fetch.  Page is pinned until released.
** Reference counted. 
*/
sqlite3_pcache_صفحہ *sqlite3PcacheFetch(PCache*, Pgno, صحیح createFlag);
صحیح sqlite3PcacheFetchStress(PCache*, Pgno, sqlite3_pcache_صفحہ**);
PgHdr *sqlite3_Pcache_Fetch_اختتام(PCache*, Pgno, sqlite3_pcache_صفحہ *p_صفحہ);
خالی sqlite3PcacheRelease(PgHdr*);

خالی sqlite3PcacheDrop(PgHdr*);         /* Remove page from cache */
خالی sqlite3PcacheMakeDirty(PgHdr*);    /* Make sure page is marked dirty */
خالی sqlite3PcacheMakeClean(PgHdr*);    /* Mark a single page as clean */
خالی sqlite3PcacheCleanAll(PCache*);    /* Mark all dirty list pages as clean */
خالی sqlite3PcacheClearWritable(PCache*);

/* Change a page number.  Used by incr-vacuum. */
خالی sqlite3PcacheMove(PgHdr*, Pgno);

/* Remove all pages with pgno>x.  Reset the cache if x==0 */
خالی sqlite3PcacheTruncate(PCache*, Pgno x);

/* Get a list of all dirty pages in the cache, sorted by page number */
PgHdr *sqlite3PcacheDirtyList(PCache*);

/* Reset and close the cache object */
خالی sqlite3PcacheClose(PCache*);

/* Clear flags from pages of the page cache */
خالی sqlite3PcacheClearSyncFlags(PCache *);

/* Discard the contents of the cache */
خالی sqlite3PcacheClear(PCache*);

/* Return the total number of outstanding page references */
i64 sqlite3_Pcache_Ref_گنتی(PCache*);

/* Increment the reference count of an existing page */
خالی sqlite3PcacheRef(PgHdr*);

i64 sqlite3_Pcache_صفحہ_Refcount(PgHdr*);

/* Return the total number of pages stored in the cache */
صحیح sqlite3PcachePagecount(PCache*);

#اگر defined(SQLITE_CHECK_صفحات) || defined(SQLITE_DEBUG)
/* Iterate through all dirty pages currently stored in the cache. This
** interface is only available if SQLITE_CHECK_PAGES is defined when the 
** library is built.
*/
خالی sqlite3PcacheIterateDirty(PCache *pCache, خالی (*xIter)(PgHdr *));
#اگر_ختم

#اگر defined(SQLITE_DEBUG)
/* Check invariants on a PgHdr object */
صحیح sqlite3_Pcache_صفحہ_Sanity(PgHdr*);
#اگر_ختم

/* Set and get the suggested cache-size for the specified pager-cache.
**
** If no global maximum is configured, then the system attempts to limit
** the total number of pages cached by purgeable pager-caches to the sum
** of the suggested cache-sizes.
*/
خالی sqlite3_Pcache_سیٹ_Cachesize(PCache *, صحیح);
#اگر_تعریف_شدہ SQLITE_TEST
صحیح sqlite3PcacheGetCachesize(PCache *);
#اگر_ختم

/* Set or get the suggested spill-size for the specified pager-cache.
**
** The spill-size is the minimum number of pages in cache before the cache
** will attempt to spill dirty pages by calling xStress.
*/
صحیح sqlite3_Pcache_سیٹ_Spillsize(PCache *, صحیح);

/* Free up as much memory as possible from the page cache */
خالی sqlite3PcacheShrink(PCache*);

#اگر_تعریف_شدہ SQLITE_ENABLE_حافظہ_MANAGEMENT
/* Try to return memory used by the pcache module to the main memory heap */
صحیح sqlite3_Pcache_Release_حافظہ(صحیح);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_TEST
خالی sqlite3PcacheStats(صحیح*,صحیح*,صحیح*,صحیح*);
#اگر_ختم

خالی sqlite3_PCache_سیٹ_Default(خالی);

/* Return the header size */
صحیح sqlite3_سرخی_سائز_Pcache(خالی);
صحیح sqlite3_سرخی_سائز_Pcache1(خالی);

/* Number of dirty pages as a percentage of the configured cache size */
صحیح sqlite3PCachePercentDirty(PCache*);

#اگر_تعریف_شدہ SQLITE_براہ_راست_حد_سے_تجاوز_READ
صحیح sqlite3PCacheIsDirty(PCache *pCache);
#اگر_ختم

#اگر_ختم /* _PCACHE_H_ */
