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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _PCACHE_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _PCACHE_H_

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ PgHdr PgHdr;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ PCache PCache;

/*
** Every page in the cache is controlled by an instance of the following
** structure.
*/
ਬਣਤਰ PgHdr {
  sqlite3_pcache_ਪੰਨਾ *p_ਪੰਨਾ;    /* Pcache object page handle */
  ਖਾਲੀ *p_ਅੰਕੜੇ;                   /* Page data */
  ਖਾਲੀ *pExtra;                  /* Extra content */
  PCache *pCache;                /* PRIVATE: Cache that owns this page */
  PgHdr *pDirty;                 /* Transient list of dirty sorted by pgno */
  Pager *pPager;                 /* The pager this page is part of */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_CHECK_ਪੰਨੇ
  u64 ਪੰਨਾ_Hash;                  /* Hash of page content */
#ਜੇ_ਅੰਤ
  Pgno pgno;                     /* Page number for this page */
  u16 flags;                     /* PGHDR flags defined below */

  /**********************************************************************
  ** Elements above, except pCache, are public.  All that follow are 
  ** private to pcache.c and should not be accessed by other modules.
  ** pCache is grouped with the public elements for efficiency.
  */
  i64 nRef;                      /* Number of users of this page */
  PgHdr *p_Dirty_ਅਗਲਾ;             /* Next element in list of dirty pages */
  PgHdr *pDirtyPrev;             /* Previous element in list of dirty pages */
                          /* NB: pDirtyNext and pDirtyPrev are undefined if the
                          ** PgHdr object is not dirty */
};

/* Bit values for PgHdr.flags */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PGHDR_CLEAN           0x001  /* Page not on the PCache.pDirty list */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PGHDR_DIRTY           0x002  /* Page is on the PCache.pDirty list */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PGHDR_WRITEABLE       0x004  /* Journaled and ready to modify */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PGHDR_NEED_SYNC       0x008  /* Fsync the rollback journal before
                                     ** writing this page to the database */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PGHDR_DONT_WRITE      0x010  /* Do not write content to disk */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PGHDR_MMAP            0x020  /* This is an mmap page object */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PGHDR_WAL_APPEND      0x040  /* Appended to wal file */

/* Initialize and shutdown the page cache subsystem */
ਪੂਰਨ sqlite3PcacheInitialize(ਖਾਲੀ);
ਖਾਲੀ sqlite3PcacheShutdown(ਖਾਲੀ);

/* Page cache buffer management:
** These routines implement SQLITE_CONFIG_PAGECACHE.
*/
ਖਾਲੀ sqlite3_PCache_ਬਫਰ_ਤਿਆਰੀ(ਖਾਲੀ *, ਪੂਰਨ sz, ਪੂਰਨ n);

/* Create a new pager cache.
** Under memory stress, invoke xStress to try to make pages clean.
** Only clean and unpinned pages can be reclaimed.
*/
ਪੂਰਨ sqlite3PcacheOpen(
  ਪੂਰਨ sz_ਪੰਨਾ,                    /* Size of every page */
  ਪੂਰਨ szExtra,                   /* Extra space associated with each page */
  ਪੂਰਨ bPurgeable,                /* True if pages are on backing store */
  ਪੂਰਨ (*xStress)(ਖਾਲੀ*, PgHdr*), /* Call to try to make pages clean */
  ਖਾਲੀ *pStress,                 /* Argument to xStress */
  PCache *p_ਮੰਜ਼ਿਲ_Init                /* Preallocated space for the PCache */
);

/* Modify the page-size after the cache has been created. */
ਪੂਰਨ sqlite3_Pcache_ਸੈੱਟ_ਪੰਨਾ_ਮਾਪ(PCache *, ਪੂਰਨ);

/* Return the size in bytes of a PCache object.  Used to preallocate
** storage space.
*/
ਪੂਰਨ sqlite3_Pcache_ਮਾਪ(ਖਾਲੀ);

/* One release per successful fetch.  Page is pinned until released.
** Reference counted. 
*/
sqlite3_pcache_ਪੰਨਾ *sqlite3PcacheFetch(PCache*, Pgno, ਪੂਰਨ createFlag);
ਪੂਰਨ sqlite3PcacheFetchStress(PCache*, Pgno, sqlite3_pcache_ਪੰਨਾ**);
PgHdr *sqlite3_Pcache_Fetch_ਅੰਤ(PCache*, Pgno, sqlite3_pcache_ਪੰਨਾ *p_ਪੰਨਾ);
ਖਾਲੀ sqlite3PcacheRelease(PgHdr*);

ਖਾਲੀ sqlite3PcacheDrop(PgHdr*);         /* Remove page from cache */
ਖਾਲੀ sqlite3PcacheMakeDirty(PgHdr*);    /* Make sure page is marked dirty */
ਖਾਲੀ sqlite3PcacheMakeClean(PgHdr*);    /* Mark a single page as clean */
ਖਾਲੀ sqlite3PcacheCleanAll(PCache*);    /* Mark all dirty list pages as clean */
ਖਾਲੀ sqlite3PcacheClearWritable(PCache*);

/* Change a page number.  Used by incr-vacuum. */
ਖਾਲੀ sqlite3PcacheMove(PgHdr*, Pgno);

/* Remove all pages with pgno>x.  Reset the cache if x==0 */
ਖਾਲੀ sqlite3PcacheTruncate(PCache*, Pgno x);

/* Get a list of all dirty pages in the cache, sorted by page number */
PgHdr *sqlite3PcacheDirtyList(PCache*);

/* Reset and close the cache object */
ਖਾਲੀ sqlite3PcacheClose(PCache*);

/* Clear flags from pages of the page cache */
ਖਾਲੀ sqlite3PcacheClearSyncFlags(PCache *);

/* Discard the contents of the cache */
ਖਾਲੀ sqlite3PcacheClear(PCache*);

/* Return the total number of outstanding page references */
i64 sqlite3_Pcache_Ref_ਗਿਣਤੀ(PCache*);

/* Increment the reference count of an existing page */
ਖਾਲੀ sqlite3PcacheRef(PgHdr*);

i64 sqlite3_Pcache_ਪੰਨਾ_Refcount(PgHdr*);

/* Return the total number of pages stored in the cache */
ਪੂਰਨ sqlite3PcachePagecount(PCache*);

#ਜੇ defined(SQLITE_CHECK_ਪੰਨੇ) || defined(SQLITE_DEBUG)
/* Iterate through all dirty pages currently stored in the cache. This
** interface is only available if SQLITE_CHECK_PAGES is defined when the 
** library is built.
*/
ਖਾਲੀ sqlite3PcacheIterateDirty(PCache *pCache, ਖਾਲੀ (*xIter)(PgHdr *));
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_DEBUG)
/* Check invariants on a PgHdr object */
ਪੂਰਨ sqlite3_Pcache_ਪੰਨਾ_Sanity(PgHdr*);
#ਜੇ_ਅੰਤ

/* Set and get the suggested cache-size for the specified pager-cache.
**
** If no global maximum is configured, then the system attempts to limit
** the total number of pages cached by purgeable pager-caches to the sum
** of the suggested cache-sizes.
*/
ਖਾਲੀ sqlite3_Pcache_ਸੈੱਟ_Cachesize(PCache *, ਪੂਰਨ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
ਪੂਰਨ sqlite3PcacheGetCachesize(PCache *);
#ਜੇ_ਅੰਤ

/* Set or get the suggested spill-size for the specified pager-cache.
**
** The spill-size is the minimum number of pages in cache before the cache
** will attempt to spill dirty pages by calling xStress.
*/
ਪੂਰਨ sqlite3_Pcache_ਸੈੱਟ_Spillsize(PCache *, ਪੂਰਨ);

/* Free up as much memory as possible from the page cache */
ਖਾਲੀ sqlite3PcacheShrink(PCache*);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਸਮ੍ਰਿਤੀ_MANAGEMENT
/* Try to return memory used by the pcache module to the main memory heap */
ਪੂਰਨ sqlite3_Pcache_Release_ਸਮ੍ਰਿਤੀ(ਪੂਰਨ);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
ਖਾਲੀ sqlite3PcacheStats(ਪੂਰਨ*,ਪੂਰਨ*,ਪੂਰਨ*,ਪੂਰਨ*);
#ਜੇ_ਅੰਤ

ਖਾਲੀ sqlite3_PCache_ਸੈੱਟ_Default(ਖਾਲੀ);

/* Return the header size */
ਪੂਰਨ sqlite3_ਸਿਰਲੇਖ_ਮਾਪ_Pcache(ਖਾਲੀ);
ਪੂਰਨ sqlite3_ਸਿਰਲੇਖ_ਮਾਪ_Pcache1(ਖਾਲੀ);

/* Number of dirty pages as a percentage of the configured cache size */
ਪੂਰਨ sqlite3PCachePercentDirty(PCache*);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ਸਿੱਧਾ_ਉਛਾਲ_READ
ਪੂਰਨ sqlite3PCacheIsDirty(PCache *pCache);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* _PCACHE_H_ */
