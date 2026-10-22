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

typedef sa_nracanaa PgHdr PgHdr;
typedef sa_nracanaa PCache PCache;

/*
** Every page in the cache is controlled by an instance of the following
** structure.
*/
sa_nracanaa PgHdr {
  sqlite3_pcache_page *pPage;    /* Pcache object page handle */
  void *pData;                   /* Page data */
  void *pExtra;                  /* Extra content */
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
puur_naa_nka sqlite3PcacheInitialize(void);
void sqlite3PcacheShutdown(void);

/* Page cache buffer management:
** These routines implement SQLITE_CONFIG_PAGECACHE.
*/
void sqlite3PCacheBufferSetup(void *, puur_naa_nka sz, puur_naa_nka n);

/* Create a new pager cache.
** Under memory stress, invoke xStress to try to make pages clean.
** Only clean and unpinned pages can be reclaimed.
*/
puur_naa_nka sqlite3PcacheOpen(
  puur_naa_nka szPage,                    /* Size of every page */
  puur_naa_nka szExtra,                   /* Extra space associated with each page */
  puur_naa_nka bPurgeable,                /* True if pages are on backing store */
  puur_naa_nka (*xStress)(void*, PgHdr*), /* Call to try to make pages clean */
  void *pStress,                 /* Argument to xStress */
  PCache *pToInit                /* Preallocated space for the PCache */
);

/* Modify the page-size after the cache has been created. */
puur_naa_nka sqlite3PcacheSetPageSize(PCache *, puur_naa_nka);

/* Return the size in bytes of a PCache object.  Used to preallocate
** storage space.
*/
puur_naa_nka sqlite3PcacheSize(void);

/* One release per successful fetch.  Page is pinned until released.
** Reference counted. 
*/
sqlite3_pcache_page *sqlite3PcacheFetch(PCache*, Pgno, puur_naa_nka createFlag);
puur_naa_nka sqlite3PcacheFetchStress(PCache*, Pgno, sqlite3_pcache_page**);
PgHdr *sqlite3PcacheFetchFinish(PCache*, Pgno, sqlite3_pcache_page *pPage);
void sqlite3PcacheRelease(PgHdr*);

void sqlite3PcacheDrop(PgHdr*);         /* Remove page from cache */
void sqlite3PcacheMakeDirty(PgHdr*);    /* Make sure page is marked dirty */
void sqlite3PcacheMakeClean(PgHdr*);    /* Mark a single page as clean */
void sqlite3PcacheCleanAll(PCache*);    /* Mark all dirty list pages as clean */
void sqlite3PcacheClearWritable(PCache*);

/* Change a page number.  Used by incr-vacuum. */
void sqlite3PcacheMove(PgHdr*, Pgno);

/* Remove all pages with pgno>x.  Reset the cache if x==0 */
void sqlite3PcacheTruncate(PCache*, Pgno x);

/* Get a list of all dirty pages in the cache, sorted by page number */
PgHdr *sqlite3PcacheDirtyList(PCache*);

/* Reset and close the cache object */
void sqlite3PcacheClose(PCache*);

/* Clear flags from pages of the page cache */
void sqlite3PcacheClearSyncFlags(PCache *);

/* Discard the contents of the cache */
void sqlite3PcacheClear(PCache*);

/* Return the total number of outstanding page references */
i64 sqlite3PcacheRefCount(PCache*);

/* Increment the reference count of an existing page */
void sqlite3PcacheRef(PgHdr*);

i64 sqlite3PcachePageRefcount(PgHdr*);

/* Return the total number of pages stored in the cache */
puur_naa_nka sqlite3PcachePagecount(PCache*);

#ᤀᤡ᤺ defined(SQLITE_CHECK_PAGES) || defined(SQLITE_DEBUG)
/* Iterate through all dirty pages currently stored in the cache. This
** interface is only available if SQLITE_CHECK_PAGES is defined when the 
** library is built.
*/
void sqlite3PcacheIterateDirty(PCache *pCache, void (*xIter)(PgHdr *));
#endif

#ᤀᤡ᤺ defined(SQLITE_DEBUG)
/* Check invariants on a PgHdr object */
puur_naa_nka sqlite3PcachePageSanity(PgHdr*);
#endif

/* Set and get the suggested cache-size for the specified pager-cache.
**
** If no global maximum is configured, then the system attempts to limit
** the total number of pages cached by purgeable pager-caches to the sum
** of the suggested cache-sizes.
*/
void sqlite3PcacheSetCachesize(PCache *, puur_naa_nka);
#ifdef SQLITE_TEST
puur_naa_nka sqlite3PcacheGetCachesize(PCache *);
#endif

/* Set or get the suggested spill-size for the specified pager-cache.
**
** The spill-size is the minimum number of pages in cache before the cache
** will attempt to spill dirty pages by calling xStress.
*/
puur_naa_nka sqlite3PcacheSetSpillsize(PCache *, puur_naa_nka);

/* Free up as much memory as possible from the page cache */
void sqlite3PcacheShrink(PCache*);

#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
/* Try to return memory used by the pcache module to the main memory heap */
puur_naa_nka sqlite3PcacheReleaseMemory(puur_naa_nka);
#endif

#ifdef SQLITE_TEST
void sqlite3PcacheStats(puur_naa_nka*,puur_naa_nka*,puur_naa_nka*,puur_naa_nka*);
#endif

void sqlite3PCacheSetDefault(void);

/* Return the header size */
puur_naa_nka sqlite3HeaderSizePcache(void);
puur_naa_nka sqlite3HeaderSizePcache1(void);

/* Number of dirty pages as a percentage of the configured cache size */
puur_naa_nka sqlite3PCachePercentDirty(PCache*);

#ifdef SQLITE_DIRECT_OVERFLOW_READ
puur_naa_nka sqlite3PCacheIsDirty(PCache *pCache);
#endif

#endif /* _PCACHE_H_ */
