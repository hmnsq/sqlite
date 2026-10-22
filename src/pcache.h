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

#अगर_अघोषित _PCACHE_H_
#घोषणा _PCACHE_H_

प्रकार काष्ठा PgHdr PgHdr;
प्रकार काष्ठा PCache PCache;

/*
** Every page in the cache is controlled by an instance of the following
** structure.
*/
काष्ठा PgHdr {
  sqlite3_pcache_page *pPage;    /* Pcache object page handle */
  व्योम *p_तथ्य;                   /* Page data */
  व्योम *pExtra;                  /* Extra content */
  PCache *pCache;                /* PRIVATE: Cache that owns this page */
  PgHdr *pDirty;                 /* Transient list of dirty sorted by pgno */
  Pager *pPager;                 /* The pager this page is part of */
#अगर_घोषित SQLITE_CHECK_PAGES
  u64 pageHash;                  /* Hash of page content */
#पूर्ण_अगर
  Pgno pgno;                     /* Page number for this page */
  u16 flags;                     /* PGHDR flags defined below */

  /**********************************************************************
  ** Elements above, except pCache, are public.  All that follow are 
  ** private to pcache.c and should not be accessed by other modules.
  ** pCache is grouped with the public elements for efficiency.
  */
  i64 nRef;                      /* Number of users of this page */
  PgHdr *p_Dirty_अग्र;             /* Next element in list of dirty pages */
  PgHdr *p_Dirty_पूर्व;             /* Previous element in list of dirty pages */
                          /* NB: pDirtyNext and pDirtyPrev are undefined if the
                          ** PgHdr object is not dirty */
};

/* Bit values for PgHdr.flags */
#घोषणा PGHDR_CLEAN           0x001  /* Page not on the PCache.pDirty list */
#घोषणा PGHDR_DIRTY           0x002  /* Page is on the PCache.pDirty list */
#घोषणा PGHDR_WRITEABLE       0x004  /* Journaled and ready to modify */
#घोषणा PGHDR_NEED_SYNC       0x008  /* Fsync the rollback journal before
                                     ** writing this page to the database */
#घोषणा PGHDR_DONT_WRITE      0x010  /* Do not write content to disk */
#घोषणा PGHDR_MMAP            0x020  /* This is an mmap page object */

#घोषणा PGHDR_WAL_APPEND      0x040  /* Appended to wal file */

/* Initialize and shutdown the page cache subsystem */
पूर्णांक sqlite3PcacheInitialize(व्योम);
व्योम sqlite3PcacheShutdown(व्योम);

/* Page cache buffer management:
** These routines implement SQLITE_CONFIG_PAGECACHE.
*/
व्योम sqlite3_PCache_बफर_Setup(व्योम *, पूर्णांक sz, पूर्णांक n);

/* Create a new pager cache.
** Under memory stress, invoke xStress to try to make pages clean.
** Only clean and unpinned pages can be reclaimed.
*/
पूर्णांक sqlite3PcacheOpen(
  पूर्णांक szPage,                    /* Size of every page */
  पूर्णांक szExtra,                   /* Extra space associated with each page */
  पूर्णांक bPurgeable,                /* True if pages are on backing store */
  पूर्णांक (*xStress)(व्योम*, PgHdr*), /* Call to try to make pages clean */
  व्योम *pStress,                 /* Argument to xStress */
  PCache *pToInit                /* Preallocated space for the PCache */
);

/* Modify the page-size after the cache has been created. */
पूर्णांक sqlite3_Pcache_Set_Page_आकार(PCache *, पूर्णांक);

/* Return the size in bytes of a PCache object.  Used to preallocate
** storage space.
*/
पूर्णांक sqlite3_Pcache_आकार(व्योम);

/* One release per successful fetch.  Page is pinned until released.
** Reference counted. 
*/
sqlite3_pcache_page *sqlite3PcacheFetch(PCache*, Pgno, पूर्णांक createFlag);
पूर्णांक sqlite3PcacheFetchStress(PCache*, Pgno, sqlite3_pcache_page**);
PgHdr *sqlite3PcacheFetchFinish(PCache*, Pgno, sqlite3_pcache_page *pPage);
व्योम sqlite3PcacheRelease(PgHdr*);

व्योम sqlite3PcacheDrop(PgHdr*);         /* Remove page from cache */
व्योम sqlite3PcacheMakeDirty(PgHdr*);    /* Make sure page is marked dirty */
व्योम sqlite3PcacheMakeClean(PgHdr*);    /* Mark a single page as clean */
व्योम sqlite3PcacheCleanAll(PCache*);    /* Mark all dirty list pages as clean */
व्योम sqlite3PcacheClearWritable(PCache*);

/* Change a page number.  Used by incr-vacuum. */
व्योम sqlite3PcacheMove(PgHdr*, Pgno);

/* Remove all pages with pgno>x.  Reset the cache if x==0 */
व्योम sqlite3PcacheTruncate(PCache*, Pgno x);

/* Get a list of all dirty pages in the cache, sorted by page number */
PgHdr *sqlite3PcacheDirtyList(PCache*);

/* Reset and close the cache object */
व्योम sqlite3PcacheClose(PCache*);

/* Clear flags from pages of the page cache */
व्योम sqlite3PcacheClearSyncFlags(PCache *);

/* Discard the contents of the cache */
व्योम sqlite3PcacheClear(PCache*);

/* Return the total number of outstanding page references */
i64 sqlite3_Pcache_Ref_गिनती(PCache*);

/* Increment the reference count of an existing page */
व्योम sqlite3PcacheRef(PgHdr*);

i64 sqlite3PcachePageRefcount(PgHdr*);

/* Return the total number of pages stored in the cache */
पूर्णांक sqlite3PcachePagecount(PCache*);

#अगर defined(SQLITE_CHECK_PAGES) || defined(SQLITE_DEBUG)
/* Iterate through all dirty pages currently stored in the cache. This
** interface is only available if SQLITE_CHECK_PAGES is defined when the 
** library is built.
*/
व्योम sqlite3PcacheIterateDirty(PCache *pCache, व्योम (*xIter)(PgHdr *));
#पूर्ण_अगर

#अगर defined(SQLITE_DEBUG)
/* Check invariants on a PgHdr object */
पूर्णांक sqlite3PcachePageSanity(PgHdr*);
#पूर्ण_अगर

/* Set and get the suggested cache-size for the specified pager-cache.
**
** If no global maximum is configured, then the system attempts to limit
** the total number of pages cached by purgeable pager-caches to the sum
** of the suggested cache-sizes.
*/
व्योम sqlite3PcacheSetCachesize(PCache *, पूर्णांक);
#अगर_घोषित SQLITE_TEST
पूर्णांक sqlite3PcacheGetCachesize(PCache *);
#पूर्ण_अगर

/* Set or get the suggested spill-size for the specified pager-cache.
**
** The spill-size is the minimum number of pages in cache before the cache
** will attempt to spill dirty pages by calling xStress.
*/
पूर्णांक sqlite3PcacheSetSpillsize(PCache *, पूर्णांक);

/* Free up as much memory as possible from the page cache */
व्योम sqlite3PcacheShrink(PCache*);

#अगर_घोषित SQLITE_ENABLE_MEMORY_MANAGEMENT
/* Try to return memory used by the pcache module to the main memory heap */
पूर्णांक sqlite3PcacheReleaseMemory(पूर्णांक);
#पूर्ण_अगर

#अगर_घोषित SQLITE_TEST
व्योम sqlite3PcacheStats(पूर्णांक*,पूर्णांक*,पूर्णांक*,पूर्णांक*);
#पूर्ण_अगर

व्योम sqlite3PCacheSetDefault(व्योम);

/* Return the header size */
पूर्णांक sqlite3_Header_आकार_Pcache(व्योम);
पूर्णांक sqlite3_Header_आकार_Pcache1(व्योम);

/* Number of dirty pages as a percentage of the configured cache size */
पूर्णांक sqlite3PCachePercentDirty(PCache*);

#अगर_घोषित SQLITE_DIRECT_OVERFLOW_READ
पूर्णांक sqlite3PCacheIsDirty(PCache *pCache);
#पूर्ण_अगर

#पूर्ण_अगर /* _PCACHE_H_ */
