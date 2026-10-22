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

પ્રકારવ્યાખ્યા સંરચના PgHdr PgHdr;
પ્રકારવ્યાખ્યા સંરચના PCache PCache;

/*
** Every page in the cache is controlled by an instance of the following
** structure.
*/
સંરચના PgHdr {
  sqlite3_pcache_page *pPage;    /* Pcache object page handle */
  શૂન્ય *pData;                   /* Page data */
  શૂન્ય *pExtra;                  /* Extra content */
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
પૂર્ણાંક sqlite3PcacheInitialize(શૂન્ય);
શૂન્ય sqlite3PcacheShutdown(શૂન્ય);

/* Page cache buffer management:
** These routines implement SQLITE_CONFIG_PAGECACHE.
*/
શૂન્ય sqlite3PCacheBufferSetup(શૂન્ય *, પૂર્ણાંક sz, પૂર્ણાંક n);

/* Create a new pager cache.
** Under memory stress, invoke xStress to try to make pages clean.
** Only clean and unpinned pages can be reclaimed.
*/
પૂર્ણાંક sqlite3PcacheOpen(
  પૂર્ણાંક szPage,                    /* Size of every page */
  પૂર્ણાંક szExtra,                   /* Extra space associated with each page */
  પૂર્ણાંક bPurgeable,                /* True if pages are on backing store */
  પૂર્ણાંક (*xStress)(શૂન્ય*, PgHdr*), /* Call to try to make pages clean */
  શૂન્ય *pStress,                 /* Argument to xStress */
  PCache *pToInit                /* Preallocated space for the PCache */
);

/* Modify the page-size after the cache has been created. */
પૂર્ણાંક sqlite3PcacheSetPageSize(PCache *, પૂર્ણાંક);

/* Return the size in bytes of a PCache object.  Used to preallocate
** storage space.
*/
પૂર્ણાંક sqlite3PcacheSize(શૂન્ય);

/* One release per successful fetch.  Page is pinned until released.
** Reference counted. 
*/
sqlite3_pcache_page *sqlite3PcacheFetch(PCache*, Pgno, પૂર્ણાંક createFlag);
પૂર્ણાંક sqlite3PcacheFetchStress(PCache*, Pgno, sqlite3_pcache_page**);
PgHdr *sqlite3PcacheFetchFinish(PCache*, Pgno, sqlite3_pcache_page *pPage);
શૂન્ય sqlite3PcacheRelease(PgHdr*);

શૂન્ય sqlite3PcacheDrop(PgHdr*);         /* Remove page from cache */
શૂન્ય sqlite3PcacheMakeDirty(PgHdr*);    /* Make sure page is marked dirty */
શૂન્ય sqlite3PcacheMakeClean(PgHdr*);    /* Mark a single page as clean */
શૂન્ય sqlite3PcacheCleanAll(PCache*);    /* Mark all dirty list pages as clean */
શૂન્ય sqlite3PcacheClearWritable(PCache*);

/* Change a page number.  Used by incr-vacuum. */
શૂન્ય sqlite3PcacheMove(PgHdr*, Pgno);

/* Remove all pages with pgno>x.  Reset the cache if x==0 */
શૂન્ય sqlite3PcacheTruncate(PCache*, Pgno x);

/* Get a list of all dirty pages in the cache, sorted by page number */
PgHdr *sqlite3PcacheDirtyList(PCache*);

/* Reset and close the cache object */
શૂન્ય sqlite3PcacheClose(PCache*);

/* Clear flags from pages of the page cache */
શૂન્ય sqlite3PcacheClearSyncFlags(PCache *);

/* Discard the contents of the cache */
શૂન્ય sqlite3PcacheClear(PCache*);

/* Return the total number of outstanding page references */
i64 sqlite3_Pcache_Ref_ગણતરી(PCache*);

/* Increment the reference count of an existing page */
શૂન્ય sqlite3PcacheRef(PgHdr*);

i64 sqlite3PcachePageRefcount(PgHdr*);

/* Return the total number of pages stored in the cache */
પૂર્ણાંક sqlite3PcachePagecount(PCache*);

#જો defined(SQLITE_CHECK_PAGES) || defined(SQLITE_DEBUG)
/* Iterate through all dirty pages currently stored in the cache. This
** interface is only available if SQLITE_CHECK_PAGES is defined when the 
** library is built.
*/
શૂન્ય sqlite3PcacheIterateDirty(PCache *pCache, શૂન્ય (*xIter)(PgHdr *));
#endif

#જો defined(SQLITE_DEBUG)
/* Check invariants on a PgHdr object */
પૂર્ણાંક sqlite3PcachePageSanity(PgHdr*);
#endif

/* Set and get the suggested cache-size for the specified pager-cache.
**
** If no global maximum is configured, then the system attempts to limit
** the total number of pages cached by purgeable pager-caches to the sum
** of the suggested cache-sizes.
*/
શૂન્ય sqlite3PcacheSetCachesize(PCache *, પૂર્ણાંક);
#ifdef SQLITE_TEST
પૂર્ણાંક sqlite3PcacheGetCachesize(PCache *);
#endif

/* Set or get the suggested spill-size for the specified pager-cache.
**
** The spill-size is the minimum number of pages in cache before the cache
** will attempt to spill dirty pages by calling xStress.
*/
પૂર્ણાંક sqlite3PcacheSetSpillsize(PCache *, પૂર્ણાંક);

/* Free up as much memory as possible from the page cache */
શૂન્ય sqlite3PcacheShrink(PCache*);

#ifdef SQLITE_ENABLE_MEMORY_MANAGEMENT
/* Try to return memory used by the pcache module to the main memory heap */
પૂર્ણાંક sqlite3PcacheReleaseMemory(પૂર્ણાંક);
#endif

#ifdef SQLITE_TEST
શૂન્ય sqlite3PcacheStats(પૂર્ણાંક*,પૂર્ણાંક*,પૂર્ણાંક*,પૂર્ણાંક*);
#endif

શૂન્ય sqlite3PCacheSetDefault(શૂન્ય);

/* Return the header size */
પૂર્ણાંક sqlite3HeaderSizePcache(શૂન્ય);
પૂર્ણાંક sqlite3HeaderSizePcache1(શૂન્ય);

/* Number of dirty pages as a percentage of the configured cache size */
પૂર્ણાંક sqlite3PCachePercentDirty(PCache*);

#ifdef SQLITE_DIRECT_OVERFLOW_READ
પૂર્ણાંક sqlite3PCacheIsDirty(PCache *pCache);
#endif

#endif /* _PCACHE_H_ */
