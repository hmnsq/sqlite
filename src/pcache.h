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

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା PgHdr PgHdr;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା PCache PCache;

/*
** Every page in the cache is controlled by an instance of the following
** structure.
*/
ସଂରଚନା PgHdr {
  sqlite3_pcache_page *pPage;    /* Pcache object page handle */
  ଶୂନ୍ୟ *pData;                   /* Page data */
  ଶୂନ୍ୟ *pExtra;                  /* Extra content */
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PcacheInitialize(ଶୂନ୍ୟ);
ଶୂନ୍ୟ sqlite3PcacheShutdown(ଶୂନ୍ୟ);

/* Page cache buffer management:
** These routines implement SQLITE_CONFIG_PAGECACHE.
*/
ଶୂନ୍ୟ sqlite3PCacheBufferSetup(ଶୂନ୍ୟ *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n);

/* Create a new pager cache.
** Under memory stress, invoke xStress to try to make pages clean.
** Only clean and unpinned pages can be reclaimed.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PcacheOpen(
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szPage,                    /* Size of every page */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szExtra,                   /* Extra space associated with each page */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bPurgeable,                /* True if pages are on backing store */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*xStress)(ଶୂନ୍ୟ*, PgHdr*), /* Call to try to make pages clean */
  ଶୂନ୍ୟ *pStress,                 /* Argument to xStress */
  PCache *pToInit                /* Preallocated space for the PCache */
);

/* Modify the page-size after the cache has been created. */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PcacheSetPageSize(PCache *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/* Return the size in bytes of a PCache object.  Used to preallocate
** storage space.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PcacheSize(ଶୂନ୍ୟ);

/* One release per successful fetch.  Page is pinned until released.
** Reference counted. 
*/
sqlite3_pcache_page *sqlite3PcacheFetch(PCache*, Pgno, ପୂର୍ଣ୍ଣସଂଖ୍ୟା createFlag);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PcacheFetchStress(PCache*, Pgno, sqlite3_pcache_page**);
PgHdr *sqlite3PcacheFetchFinish(PCache*, Pgno, sqlite3_pcache_page *pPage);
ଶୂନ୍ୟ sqlite3PcacheRelease(PgHdr*);

ଶୂନ୍ୟ sqlite3PcacheDrop(PgHdr*);         /* Remove page from cache */
ଶୂନ୍ୟ sqlite3PcacheMakeDirty(PgHdr*);    /* Make sure page is marked dirty */
ଶୂନ୍ୟ sqlite3PcacheMakeClean(PgHdr*);    /* Mark a single page as clean */
ଶୂନ୍ୟ sqlite3PcacheCleanAll(PCache*);    /* Mark all dirty list pages as clean */
ଶୂନ୍ୟ sqlite3PcacheClearWritable(PCache*);

/* Change a page number.  Used by incr-vacuum. */
ଶୂନ୍ୟ sqlite3PcacheMove(PgHdr*, Pgno);

/* Remove all pages with pgno>x.  Reset the cache if x==0 */
ଶୂନ୍ୟ sqlite3PcacheTruncate(PCache*, Pgno x);

/* Get a list of all dirty pages in the cache, sorted by page number */
PgHdr *sqlite3PcacheDirtyList(PCache*);

/* Reset and close the cache object */
ଶୂନ୍ୟ sqlite3PcacheClose(PCache*);

/* Clear flags from pages of the page cache */
ଶୂନ୍ୟ sqlite3PcacheClearSyncFlags(PCache *);

/* Discard the contents of the cache */
ଶୂନ୍ୟ sqlite3PcacheClear(PCache*);

/* Return the total number of outstanding page references */
i64 sqlite3_Pcache_Ref_ଗଣନା(PCache*);

/* Increment the reference count of an existing page */
ଶୂନ୍ୟ sqlite3PcacheRef(PgHdr*);

i64 sqlite3PcachePageRefcount(PgHdr*);

/* Return the total number of pages stored in the cache */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PcachePagecount(PCache*);

#ଯଦି defined(SQLITE_CHECK_PAGES) || defined(SQLITE_DEBUG)
/* Iterate through all dirty pages currently stored in the cache. This
** interface is only available if SQLITE_CHECK_PAGES is defined when the 
** library is built.
*/
ଶୂନ୍ୟ sqlite3PcacheIterateDirty(PCache *pCache, ଶୂନ୍ୟ (*xIter)(PgHdr *));
#endif

#ଯଦି defined(SQLITE_DEBUG)
/* Check invariants on a PgHdr object */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PcachePageSanity(PgHdr*);
#endif

/* Set and get the suggested cache-size for the specified pager-cache.
**
** If no global maximum is configured, then the system attempts to limit
** the total number of pages cached by purgeable pager-caches to the sum
** of the suggested cache-sizes.
*/
ଶୂନ୍ୟ sqlite3PcacheSetCachesize(PCache *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
#ifdef SQLITE_TEST
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PcacheGetCachesize(PCache *);
#endif

/* Set or get the suggested spill-size for the specified pager-cache.
**
** The spill-size is the minimum number of pages in cache before the cache
** will attempt to spill dirty pages by calling xStress.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PcacheSetSpillsize(PCache *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/* Free up as much memory as possible from the page cache */
ଶୂନ୍ୟ sqlite3PcacheShrink(PCache*);

#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
/* Try to return memory used by the pcache module to the main memory heap */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PcacheReleaseMemory(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
#endif

#ifdef SQLITE_TEST
ଶୂନ୍ୟ sqlite3PcacheStats(ପୂର୍ଣ୍ଣସଂଖ୍ୟା*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା*);
#endif

ଶୂନ୍ୟ sqlite3PCacheSetDefault(ଶୂନ୍ୟ);

/* Return the header size */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3HeaderSizePcache(ଶୂନ୍ୟ);
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3HeaderSizePcache1(ଶୂନ୍ୟ);

/* Number of dirty pages as a percentage of the configured cache size */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PCachePercentDirty(PCache*);

#ifdef SQLITE_DIRECT_OVERFLOW_READ
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3PCacheIsDirty(PCache *pCache);
#endif

#endif /* _PCACHE_H_ */
