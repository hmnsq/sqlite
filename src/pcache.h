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

തരനിർവചനം ഘടന PgHdr PgHdr;
തരനിർവചനം ഘടന PCache PCache;

/*
** Every page in the cache is controlled by an instance of the following
** structure.
*/
ഘടന PgHdr {
  sqlite3_pcache_page *pPage;    /* Pcache object page handle */
  ശൂന്യം *pData;                   /* Page data */
  ശൂന്യം *pExtra;                  /* Extra content */
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
പൂർണ്ണസംഖ്യ sqlite3PcacheInitialize(ശൂന്യം);
ശൂന്യം sqlite3PcacheShutdown(ശൂന്യം);

/* Page cache buffer management:
** These routines implement SQLITE_CONFIG_PAGECACHE.
*/
ശൂന്യം sqlite3PCacheBufferSetup(ശൂന്യം *, പൂർണ്ണസംഖ്യ sz, പൂർണ്ണസംഖ്യ n);

/* Create a new pager cache.
** Under memory stress, invoke xStress to try to make pages clean.
** Only clean and unpinned pages can be reclaimed.
*/
പൂർണ്ണസംഖ്യ sqlite3PcacheOpen(
  പൂർണ്ണസംഖ്യ szPage,                    /* Size of every page */
  പൂർണ്ണസംഖ്യ szExtra,                   /* Extra space associated with each page */
  പൂർണ്ണസംഖ്യ bPurgeable,                /* True if pages are on backing store */
  പൂർണ്ണസംഖ്യ (*xStress)(ശൂന്യം*, PgHdr*), /* Call to try to make pages clean */
  ശൂന്യം *pStress,                 /* Argument to xStress */
  PCache *pToInit                /* Preallocated space for the PCache */
);

/* Modify the page-size after the cache has been created. */
പൂർണ്ണസംഖ്യ sqlite3PcacheSetPageSize(PCache *, പൂർണ്ണസംഖ്യ);

/* Return the size in bytes of a PCache object.  Used to preallocate
** storage space.
*/
പൂർണ്ണസംഖ്യ sqlite3PcacheSize(ശൂന്യം);

/* One release per successful fetch.  Page is pinned until released.
** Reference counted. 
*/
sqlite3_pcache_page *sqlite3PcacheFetch(PCache*, Pgno, പൂർണ്ണസംഖ്യ createFlag);
പൂർണ്ണസംഖ്യ sqlite3PcacheFetchStress(PCache*, Pgno, sqlite3_pcache_page**);
PgHdr *sqlite3PcacheFetchFinish(PCache*, Pgno, sqlite3_pcache_page *pPage);
ശൂന്യം sqlite3PcacheRelease(PgHdr*);

ശൂന്യം sqlite3PcacheDrop(PgHdr*);         /* Remove page from cache */
ശൂന്യം sqlite3PcacheMakeDirty(PgHdr*);    /* Make sure page is marked dirty */
ശൂന്യം sqlite3PcacheMakeClean(PgHdr*);    /* Mark a single page as clean */
ശൂന്യം sqlite3PcacheCleanAll(PCache*);    /* Mark all dirty list pages as clean */
ശൂന്യം sqlite3PcacheClearWritable(PCache*);

/* Change a page number.  Used by incr-vacuum. */
ശൂന്യം sqlite3PcacheMove(PgHdr*, Pgno);

/* Remove all pages with pgno>x.  Reset the cache if x==0 */
ശൂന്യം sqlite3PcacheTruncate(PCache*, Pgno x);

/* Get a list of all dirty pages in the cache, sorted by page number */
PgHdr *sqlite3PcacheDirtyList(PCache*);

/* Reset and close the cache object */
ശൂന്യം sqlite3PcacheClose(PCache*);

/* Clear flags from pages of the page cache */
ശൂന്യം sqlite3PcacheClearSyncFlags(PCache *);

/* Discard the contents of the cache */
ശൂന്യം sqlite3PcacheClear(PCache*);

/* Return the total number of outstanding page references */
i64 sqlite3_Pcache_Ref_എണ്ണം(PCache*);

/* Increment the reference count of an existing page */
ശൂന്യം sqlite3PcacheRef(PgHdr*);

i64 sqlite3PcachePageRefcount(PgHdr*);

/* Return the total number of pages stored in the cache */
പൂർണ്ണസംഖ്യ sqlite3PcachePagecount(PCache*);

#എങ്കിൽ defined(SQLITE_CHECK_PAGES) || defined(SQLITE_DEBUG)
/* Iterate through all dirty pages currently stored in the cache. This
** interface is only available if SQLITE_CHECK_PAGES is defined when the 
** library is built.
*/
ശൂന്യം sqlite3PcacheIterateDirty(PCache *pCache, ശൂന്യം (*xIter)(PgHdr *));
#endif

#എങ്കിൽ defined(SQLITE_DEBUG)
/* Check invariants on a PgHdr object */
പൂർണ്ണസംഖ്യ sqlite3PcachePageSanity(PgHdr*);
#endif

/* Set and get the suggested cache-size for the specified pager-cache.
**
** If no global maximum is configured, then the system attempts to limit
** the total number of pages cached by purgeable pager-caches to the sum
** of the suggested cache-sizes.
*/
ശൂന്യം sqlite3PcacheSetCachesize(PCache *, പൂർണ്ണസംഖ്യ);
#ifdef SQLITE_TEST
പൂർണ്ണസംഖ്യ sqlite3PcacheGetCachesize(PCache *);
#endif

/* Set or get the suggested spill-size for the specified pager-cache.
**
** The spill-size is the minimum number of pages in cache before the cache
** will attempt to spill dirty pages by calling xStress.
*/
പൂർണ്ണസംഖ്യ sqlite3PcacheSetSpillsize(PCache *, പൂർണ്ണസംഖ്യ);

/* Free up as much memory as possible from the page cache */
ശൂന്യം sqlite3PcacheShrink(PCache*);

#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
/* Try to return memory used by the pcache module to the main memory heap */
പൂർണ്ണസംഖ്യ sqlite3PcacheReleaseMemory(പൂർണ്ണസംഖ്യ);
#endif

#ifdef SQLITE_TEST
ശൂന്യം sqlite3PcacheStats(പൂർണ്ണസംഖ്യ*,പൂർണ്ണസംഖ്യ*,പൂർണ്ണസംഖ്യ*,പൂർണ്ണസംഖ്യ*);
#endif

ശൂന്യം sqlite3PCacheSetDefault(ശൂന്യം);

/* Return the header size */
പൂർണ്ണസംഖ്യ sqlite3HeaderSizePcache(ശൂന്യം);
പൂർണ്ണസംഖ്യ sqlite3HeaderSizePcache1(ശൂന്യം);

/* Number of dirty pages as a percentage of the configured cache size */
പൂർണ്ണസംഖ്യ sqlite3PCachePercentDirty(PCache*);

#ifdef SQLITE_DIRECT_OVERFLOW_READ
പൂർണ്ണസംഖ്യ sqlite3PCacheIsDirty(PCache *pCache);
#endif

#endif /* _PCACHE_H_ */
