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

#यदि_अपरिभाषितम् _PCACHE_H_
#परिभाषय _PCACHE_H_

प्रकारनिर्देश संरचना PgHdr PgHdr;
प्रकारनिर्देश संरचना PCache PCache;

/*
** Every page in the cache is controlled by an instance of the following
** structure.
*/
संरचना PgHdr {
  sqlite3_pcache_page *pPage;    /* Pcache object page handle */
  शून्य *p_दत्त;                   /* Page data */
  शून्य *pExtra;                  /* Extra content */
  PCache *pCache;                /* PRIVATE: Cache that owns this page */
  PgHdr *pDirty;                 /* Transient list of dirty sorted by pgno */
  Pager *pPager;                 /* The pager this page is part of */
#यदि_परिभाषितम् SQLITE_CHECK_पृष्ठानि
  u64 pageHash;                  /* Hash of page content */
#यदि_अन्तः
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
#परिभाषय PGHDR_CLEAN           0x001  /* Page not on the PCache.pDirty list */
#परिभाषय PGHDR_DIRTY           0x002  /* Page is on the PCache.pDirty list */
#परिभाषय PGHDR_WRITEABLE       0x004  /* Journaled and ready to modify */
#परिभाषय PGHDR_NEED_SYNC       0x008  /* Fsync the rollback journal before
                                     ** writing this page to the database */
#परिभाषय PGHDR_DONT_WRITE      0x010  /* Do not write content to disk */
#परिभाषय PGHDR_MMAP            0x020  /* This is an mmap page object */

#परिभाषय PGHDR_WAL_APPEND      0x040  /* Appended to wal file */

/* Initialize and shutdown the page cache subsystem */
पूर्णाङ्क sqlite3PcacheInitialize(शून्य);
शून्य sqlite3PcacheShutdown(शून्य);

/* Page cache buffer management:
** These routines implement SQLITE_CONFIG_PAGECACHE.
*/
शून्य sqlite3_PCache_बफर_सज्जता(शून्य *, पूर्णाङ्क sz, पूर्णाङ्क n);

/* Create a new pager cache.
** Under memory stress, invoke xStress to try to make pages clean.
** Only clean and unpinned pages can be reclaimed.
*/
पूर्णाङ्क sqlite3PcacheOpen(
  पूर्णाङ्क szPage,                    /* Size of every page */
  पूर्णाङ्क szExtra,                   /* Extra space associated with each page */
  पूर्णाङ्क bPurgeable,                /* True if pages are on backing store */
  पूर्णाङ्क (*xStress)(शून्य*, PgHdr*), /* Call to try to make pages clean */
  शून्य *pStress,                 /* Argument to xStress */
  PCache *p_गन्तव्यम्_Init                /* Preallocated space for the PCache */
);

/* Modify the page-size after the cache has been created. */
पूर्णाङ्क sqlite3_Pcache_समुच्चयः_Page_आकार(PCache *, पूर्णाङ्क);

/* Return the size in bytes of a PCache object.  Used to preallocate
** storage space.
*/
पूर्णाङ्क sqlite3_Pcache_आकार(शून्य);

/* One release per successful fetch.  Page is pinned until released.
** Reference counted. 
*/
sqlite3_pcache_page *sqlite3PcacheFetch(PCache*, Pgno, पूर्णाङ्क createFlag);
पूर्णाङ्क sqlite3PcacheFetchStress(PCache*, Pgno, sqlite3_pcache_page**);
PgHdr *sqlite3PcacheFetchFinish(PCache*, Pgno, sqlite3_pcache_page *pPage);
शून्य sqlite3PcacheRelease(PgHdr*);

शून्य sqlite3PcacheDrop(PgHdr*);         /* Remove page from cache */
शून्य sqlite3PcacheMakeDirty(PgHdr*);    /* Make sure page is marked dirty */
शून्य sqlite3PcacheMakeClean(PgHdr*);    /* Mark a single page as clean */
शून्य sqlite3PcacheCleanAll(PCache*);    /* Mark all dirty list pages as clean */
शून्य sqlite3PcacheClearWritable(PCache*);

/* Change a page number.  Used by incr-vacuum. */
शून्य sqlite3PcacheMove(PgHdr*, Pgno);

/* Remove all pages with pgno>x.  Reset the cache if x==0 */
शून्य sqlite3PcacheTruncate(PCache*, Pgno x);

/* Get a list of all dirty pages in the cache, sorted by page number */
PgHdr *sqlite3PcacheDirtyList(PCache*);

/* Reset and close the cache object */
शून्य sqlite3PcacheClose(PCache*);

/* Clear flags from pages of the page cache */
शून्य sqlite3PcacheClearSyncFlags(PCache *);

/* Discard the contents of the cache */
शून्य sqlite3PcacheClear(PCache*);

/* Return the total number of outstanding page references */
i64 sqlite3_Pcache_Ref_गणनम्(PCache*);

/* Increment the reference count of an existing page */
शून्य sqlite3PcacheRef(PgHdr*);

i64 sqlite3PcachePageRefcount(PgHdr*);

/* Return the total number of pages stored in the cache */
पूर्णाङ्क sqlite3PcachePagecount(PCache*);

#यदि defined(SQLITE_CHECK_पृष्ठानि) || defined(SQLITE_DEBUG)
/* Iterate through all dirty pages currently stored in the cache. This
** interface is only available if SQLITE_CHECK_PAGES is defined when the 
** library is built.
*/
शून्य sqlite3PcacheIterateDirty(PCache *pCache, शून्य (*xIter)(PgHdr *));
#यदि_अन्तः

#यदि defined(SQLITE_DEBUG)
/* Check invariants on a PgHdr object */
पूर्णाङ्क sqlite3PcachePageSanity(PgHdr*);
#यदि_अन्तः

/* Set and get the suggested cache-size for the specified pager-cache.
**
** If no global maximum is configured, then the system attempts to limit
** the total number of pages cached by purgeable pager-caches to the sum
** of the suggested cache-sizes.
*/
शून्य sqlite3_Pcache_समुच्चयः_Cachesize(PCache *, पूर्णाङ्क);
#यदि_परिभाषितम् SQLITE_TEST
पूर्णाङ्क sqlite3PcacheGetCachesize(PCache *);
#यदि_अन्तः

/* Set or get the suggested spill-size for the specified pager-cache.
**
** The spill-size is the minimum number of pages in cache before the cache
** will attempt to spill dirty pages by calling xStress.
*/
पूर्णाङ्क sqlite3_Pcache_समुच्चयः_Spillsize(PCache *, पूर्णाङ्क);

/* Free up as much memory as possible from the page cache */
शून्य sqlite3PcacheShrink(PCache*);

#यदि_परिभाषितम् SQLITE_ENABLE_स्मृतिः_MANAGEMENT
/* Try to return memory used by the pcache module to the main memory heap */
पूर्णाङ्क sqlite3_Pcache_Release_स्मृतिः(पूर्णाङ्क);
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_TEST
शून्य sqlite3PcacheStats(पूर्णाङ्क*,पूर्णाङ्क*,पूर्णाङ्क*,पूर्णाङ्क*);
#यदि_अन्तः

शून्य sqlite3_PCache_समुच्चयः_Default(शून्य);

/* Return the header size */
पूर्णाङ्क sqlite3_शीर्षकम्_आकार_Pcache(शून्य);
पूर्णाङ्क sqlite3_शीर्षकम्_आकार_Pcache1(शून्य);

/* Number of dirty pages as a percentage of the configured cache size */
पूर्णाङ्क sqlite3PCachePercentDirty(PCache*);

#यदि_परिभाषितम् SQLITE_प्रत्यक्षम्_अतिप्रवाहः_READ
पूर्णाङ्क sqlite3PCacheIsDirty(PCache *pCache);
#यदि_अन्तः

#यदि_अन्तः /* _PCACHE_H_ */
