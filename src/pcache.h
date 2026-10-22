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

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ PgHdr PgHdr;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ PCache PCache;

/*
** Every page in the cache is controlled by an instance of the following
** structure.
*/
ಸಂರಚನಾ PgHdr {
  sqlite3_pcache_page *pPage;    /* Pcache object page handle */
  ಶೂನ್ಯ *pData;                   /* Page data */
  ಶೂನ್ಯ *pExtra;                  /* Extra content */
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
ಪೂರ್ಣಾಙ್ಕ sqlite3PcacheInitialize(ಶೂನ್ಯ);
ಶೂನ್ಯ sqlite3PcacheShutdown(ಶೂನ್ಯ);

/* Page cache buffer management:
** These routines implement SQLITE_CONFIG_PAGECACHE.
*/
ಶೂನ್ಯ sqlite3PCacheBufferSetup(ಶೂನ್ಯ *, ಪೂರ್ಣಾಙ್ಕ sz, ಪೂರ್ಣಾಙ್ಕ n);

/* Create a new pager cache.
** Under memory stress, invoke xStress to try to make pages clean.
** Only clean and unpinned pages can be reclaimed.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3PcacheOpen(
  ಪೂರ್ಣಾಙ್ಕ szPage,                    /* Size of every page */
  ಪೂರ್ಣಾಙ್ಕ szExtra,                   /* Extra space associated with each page */
  ಪೂರ್ಣಾಙ್ಕ bPurgeable,                /* True if pages are on backing store */
  ಪೂರ್ಣಾಙ್ಕ (*xStress)(ಶೂನ್ಯ*, PgHdr*), /* Call to try to make pages clean */
  ಶೂನ್ಯ *pStress,                 /* Argument to xStress */
  PCache *pToInit                /* Preallocated space for the PCache */
);

/* Modify the page-size after the cache has been created. */
ಪೂರ್ಣಾಙ್ಕ sqlite3PcacheSetPageSize(PCache *, ಪೂರ್ಣಾಙ್ಕ);

/* Return the size in bytes of a PCache object.  Used to preallocate
** storage space.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3PcacheSize(ಶೂನ್ಯ);

/* One release per successful fetch.  Page is pinned until released.
** Reference counted. 
*/
sqlite3_pcache_page *sqlite3PcacheFetch(PCache*, Pgno, ಪೂರ್ಣಾಙ್ಕ createFlag);
ಪೂರ್ಣಾಙ್ಕ sqlite3PcacheFetchStress(PCache*, Pgno, sqlite3_pcache_page**);
PgHdr *sqlite3PcacheFetchFinish(PCache*, Pgno, sqlite3_pcache_page *pPage);
ಶೂನ್ಯ sqlite3PcacheRelease(PgHdr*);

ಶೂನ್ಯ sqlite3PcacheDrop(PgHdr*);         /* Remove page from cache */
ಶೂನ್ಯ sqlite3PcacheMakeDirty(PgHdr*);    /* Make sure page is marked dirty */
ಶೂನ್ಯ sqlite3PcacheMakeClean(PgHdr*);    /* Mark a single page as clean */
ಶೂನ್ಯ sqlite3PcacheCleanAll(PCache*);    /* Mark all dirty list pages as clean */
ಶೂನ್ಯ sqlite3PcacheClearWritable(PCache*);

/* Change a page number.  Used by incr-vacuum. */
ಶೂನ್ಯ sqlite3PcacheMove(PgHdr*, Pgno);

/* Remove all pages with pgno>x.  Reset the cache if x==0 */
ಶೂನ್ಯ sqlite3PcacheTruncate(PCache*, Pgno x);

/* Get a list of all dirty pages in the cache, sorted by page number */
PgHdr *sqlite3PcacheDirtyList(PCache*);

/* Reset and close the cache object */
ಶೂನ್ಯ sqlite3PcacheClose(PCache*);

/* Clear flags from pages of the page cache */
ಶೂನ್ಯ sqlite3PcacheClearSyncFlags(PCache *);

/* Discard the contents of the cache */
ಶೂನ್ಯ sqlite3PcacheClear(PCache*);

/* Return the total number of outstanding page references */
i64 sqlite3_Pcache_Ref_ಎಣಿಕೆ(PCache*);

/* Increment the reference count of an existing page */
ಶೂನ್ಯ sqlite3PcacheRef(PgHdr*);

i64 sqlite3PcachePageRefcount(PgHdr*);

/* Return the total number of pages stored in the cache */
ಪೂರ್ಣಾಙ್ಕ sqlite3PcachePagecount(PCache*);

#ಯದಿ defined(SQLITE_CHECK_PAGES) || defined(SQLITE_DEBUG)
/* Iterate through all dirty pages currently stored in the cache. This
** interface is only available if SQLITE_CHECK_PAGES is defined when the 
** library is built.
*/
ಶೂನ್ಯ sqlite3PcacheIterateDirty(PCache *pCache, ಶೂನ್ಯ (*xIter)(PgHdr *));
#endif

#ಯದಿ defined(SQLITE_DEBUG)
/* Check invariants on a PgHdr object */
ಪೂರ್ಣಾಙ್ಕ sqlite3PcachePageSanity(PgHdr*);
#endif

/* Set and get the suggested cache-size for the specified pager-cache.
**
** If no global maximum is configured, then the system attempts to limit
** the total number of pages cached by purgeable pager-caches to the sum
** of the suggested cache-sizes.
*/
ಶೂನ್ಯ sqlite3PcacheSetCachesize(PCache *, ಪೂರ್ಣಾಙ್ಕ);
#ifdef SQLITE_TEST
ಪೂರ್ಣಾಙ್ಕ sqlite3PcacheGetCachesize(PCache *);
#endif

/* Set or get the suggested spill-size for the specified pager-cache.
**
** The spill-size is the minimum number of pages in cache before the cache
** will attempt to spill dirty pages by calling xStress.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3PcacheSetSpillsize(PCache *, ಪೂರ್ಣಾಙ್ಕ);

/* Free up as much memory as possible from the page cache */
ಶೂನ್ಯ sqlite3PcacheShrink(PCache*);

#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
/* Try to return memory used by the pcache module to the main memory heap */
ಪೂರ್ಣಾಙ್ಕ sqlite3PcacheReleaseMemory(ಪೂರ್ಣಾಙ್ಕ);
#endif

#ifdef SQLITE_TEST
ಶೂನ್ಯ sqlite3PcacheStats(ಪೂರ್ಣಾಙ್ಕ*,ಪೂರ್ಣಾಙ್ಕ*,ಪೂರ್ಣಾಙ್ಕ*,ಪೂರ್ಣಾಙ್ಕ*);
#endif

ಶೂನ್ಯ sqlite3PCacheSetDefault(ಶೂನ್ಯ);

/* Return the header size */
ಪೂರ್ಣಾಙ್ಕ sqlite3HeaderSizePcache(ಶೂನ್ಯ);
ಪೂರ್ಣಾಙ್ಕ sqlite3HeaderSizePcache1(ಶೂನ್ಯ);

/* Number of dirty pages as a percentage of the configured cache size */
ಪೂರ್ಣಾಙ್ಕ sqlite3PCachePercentDirty(PCache*);

#ifdef SQLITE_DIRECT_OVERFLOW_READ
ಪೂರ್ಣಾಙ್ಕ sqlite3PCacheIsDirty(PCache *pCache);
#endif

#endif /* _PCACHE_H_ */
