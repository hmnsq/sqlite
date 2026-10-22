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

#నిర్వచితం_కాకపోతే _PCACHE_H_
#నిర్వచించు _PCACHE_H_

రకనిర్వచనం నిర్మాణం PgHdr PgHdr;
రకనిర్వచనం నిర్మాణం PCache PCache;

/*
** Every page in the cache is controlled by an instance of the following
** structure.
*/
నిర్మాణం PgHdr {
  sqlite3_pcache_పుట *p_పుట;    /* Pcache object page handle */
  శూన్య *p_దత్తాంశం;                   /* Page data */
  శూన్య *pExtra;                  /* Extra content */
  PCache *pCache;                /* PRIVATE: Cache that owns this page */
  PgHdr *pDirty;                 /* Transient list of dirty sorted by pgno */
  Pager *pPager;                 /* The pager this page is part of */
#నిర్వచితమైతే SQLITE_CHECK_పుటలు
  u64 పుట_Hash;                  /* Hash of page content */
#అయితే_ముగింపు
  Pgno pgno;                     /* Page number for this page */
  u16 flags;                     /* PGHDR flags defined below */

  /**********************************************************************
  ** Elements above, except pCache, are public.  All that follow are 
  ** private to pcache.c and should not be accessed by other modules.
  ** pCache is grouped with the public elements for efficiency.
  */
  i64 nRef;                      /* Number of users of this page */
  PgHdr *p_Dirty_తదుపరి;             /* Next element in list of dirty pages */
  PgHdr *pDirtyPrev;             /* Previous element in list of dirty pages */
                          /* NB: pDirtyNext and pDirtyPrev are undefined if the
                          ** PgHdr object is not dirty */
};

/* Bit values for PgHdr.flags */
#నిర్వచించు PGHDR_CLEAN           0x001  /* Page not on the PCache.pDirty list */
#నిర్వచించు PGHDR_DIRTY           0x002  /* Page is on the PCache.pDirty list */
#నిర్వచించు PGHDR_WRITEABLE       0x004  /* Journaled and ready to modify */
#నిర్వచించు PGHDR_NEED_SYNC       0x008  /* Fsync the rollback journal before
                                     ** writing this page to the database */
#నిర్వచించు PGHDR_DONT_WRITE      0x010  /* Do not write content to disk */
#నిర్వచించు PGHDR_MMAP            0x020  /* This is an mmap page object */

#నిర్వచించు PGHDR_WAL_APPEND      0x040  /* Appended to wal file */

/* Initialize and shutdown the page cache subsystem */
పూర్ణాంకం sqlite3PcacheInitialize(శూన్య);
శూన్య sqlite3PcacheShutdown(శూన్య);

/* Page cache buffer management:
** These routines implement SQLITE_CONFIG_PAGECACHE.
*/
శూన్య sqlite3_PCache_బఫరు_అమరిక(శూన్య *, పూర్ణాంకం sz, పూర్ణాంకం n);

/* Create a new pager cache.
** Under memory stress, invoke xStress to try to make pages clean.
** Only clean and unpinned pages can be reclaimed.
*/
పూర్ణాంకం sqlite3PcacheOpen(
  పూర్ణాంకం sz_పుట,                    /* Size of every page */
  పూర్ణాంకం szExtra,                   /* Extra space associated with each page */
  పూర్ణాంకం bPurgeable,                /* True if pages are on backing store */
  పూర్ణాంకం (*xStress)(శూన్య*, PgHdr*), /* Call to try to make pages clean */
  శూన్య *pStress,                 /* Argument to xStress */
  PCache *p_గమ్యం_Init                /* Preallocated space for the PCache */
);

/* Modify the page-size after the cache has been created. */
పూర్ణాంకం sqlite3_Pcache_సమితి_పుట_కొలత(PCache *, పూర్ణాంకం);

/* Return the size in bytes of a PCache object.  Used to preallocate
** storage space.
*/
పూర్ణాంకం sqlite3_Pcache_కొలత(శూన్య);

/* One release per successful fetch.  Page is pinned until released.
** Reference counted. 
*/
sqlite3_pcache_పుట *sqlite3PcacheFetch(PCache*, Pgno, పూర్ణాంకం createFlag);
పూర్ణాంకం sqlite3PcacheFetchStress(PCache*, Pgno, sqlite3_pcache_పుట**);
PgHdr *sqlite3_Pcache_Fetch_ముగింపు(PCache*, Pgno, sqlite3_pcache_పుట *p_పుట);
శూన్య sqlite3PcacheRelease(PgHdr*);

శూన్య sqlite3PcacheDrop(PgHdr*);         /* Remove page from cache */
శూన్య sqlite3PcacheMakeDirty(PgHdr*);    /* Make sure page is marked dirty */
శూన్య sqlite3PcacheMakeClean(PgHdr*);    /* Mark a single page as clean */
శూన్య sqlite3PcacheCleanAll(PCache*);    /* Mark all dirty list pages as clean */
శూన్య sqlite3PcacheClearWritable(PCache*);

/* Change a page number.  Used by incr-vacuum. */
శూన్య sqlite3PcacheMove(PgHdr*, Pgno);

/* Remove all pages with pgno>x.  Reset the cache if x==0 */
శూన్య sqlite3PcacheTruncate(PCache*, Pgno x);

/* Get a list of all dirty pages in the cache, sorted by page number */
PgHdr *sqlite3PcacheDirtyList(PCache*);

/* Reset and close the cache object */
శూన్య sqlite3PcacheClose(PCache*);

/* Clear flags from pages of the page cache */
శూన్య sqlite3PcacheClearSyncFlags(PCache *);

/* Discard the contents of the cache */
శూన్య sqlite3PcacheClear(PCache*);

/* Return the total number of outstanding page references */
i64 sqlite3_Pcache_Ref_లెక్క(PCache*);

/* Increment the reference count of an existing page */
శూన్య sqlite3PcacheRef(PgHdr*);

i64 sqlite3_Pcache_పుట_Refcount(PgHdr*);

/* Return the total number of pages stored in the cache */
పూర్ణాంకం sqlite3PcachePagecount(PCache*);

#ఒకవేళ defined(SQLITE_CHECK_పుటలు) || defined(SQLITE_DEBUG)
/* Iterate through all dirty pages currently stored in the cache. This
** interface is only available if SQLITE_CHECK_PAGES is defined when the 
** library is built.
*/
శూన్య sqlite3PcacheIterateDirty(PCache *pCache, శూన్య (*xIter)(PgHdr *));
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_DEBUG)
/* Check invariants on a PgHdr object */
పూర్ణాంకం sqlite3_Pcache_పుట_Sanity(PgHdr*);
#అయితే_ముగింపు

/* Set and get the suggested cache-size for the specified pager-cache.
**
** If no global maximum is configured, then the system attempts to limit
** the total number of pages cached by purgeable pager-caches to the sum
** of the suggested cache-sizes.
*/
శూన్య sqlite3_Pcache_సమితి_Cachesize(PCache *, పూర్ణాంకం);
#నిర్వచితమైతే SQLITE_TEST
పూర్ణాంకం sqlite3PcacheGetCachesize(PCache *);
#అయితే_ముగింపు

/* Set or get the suggested spill-size for the specified pager-cache.
**
** The spill-size is the minimum number of pages in cache before the cache
** will attempt to spill dirty pages by calling xStress.
*/
పూర్ణాంకం sqlite3_Pcache_సమితి_Spillsize(PCache *, పూర్ణాంకం);

/* Free up as much memory as possible from the page cache */
శూన్య sqlite3PcacheShrink(PCache*);

#నిర్వచితమైతే SQLITE_ENABLE_జ్ఞాపకం_MANAGEMENT
/* Try to return memory used by the pcache module to the main memory heap */
పూర్ణాంకం sqlite3_Pcache_Release_జ్ఞాపకం(పూర్ణాంకం);
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_TEST
శూన్య sqlite3PcacheStats(పూర్ణాంకం*,పూర్ణాంకం*,పూర్ణాంకం*,పూర్ణాంకం*);
#అయితే_ముగింపు

శూన్య sqlite3_PCache_సమితి_Default(శూన్య);

/* Return the header size */
పూర్ణాంకం sqlite3_శీర్షిక_కొలత_Pcache(శూన్య);
పూర్ణాంకం sqlite3_శీర్షిక_కొలత_Pcache1(శూన్య);

/* Number of dirty pages as a percentage of the configured cache size */
పూర్ణాంకం sqlite3PCachePercentDirty(PCache*);

#నిర్వచితమైతే SQLITE_ప్రత్యక్ష_అధికప్రవాహం_READ
పూర్ణాంకం sqlite3PCacheIsDirty(PCache *pCache);
#అయితే_ముగింపు

#అయితే_ముగింపు /* _PCACHE_H_ */
