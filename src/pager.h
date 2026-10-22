/*
** 2001 September 15
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
** subsystem.  The page cache subsystem reads and writes a file a page
** at a time and provides a journal for rollback.
*/

#ifndef SQLITE_PAGER_H
#define SQLITE_PAGER_H

/*
** Default maximum size for persistent journal files. A negative 
** value means no limit. This value may be overridden using the 
** sqlite3PagerJournalSizeLimit() API. See also "PRAGMA journal_size_limit".
*/
#ifndef SQLITE_DEFAULT_JOURNAL_আকার_LIMIT
  #define SQLITE_DEFAULT_JOURNAL_আকার_LIMIT -1
#endif

/*
** The type used to represent a page number.  The first page in a file
** is called page 1.  0 is used to represent "not a page".
*/
প্রকারলেখ u32 Pgno;

/*
** Each open file is managed by a separate instance of the "Pager" structure.
*/
প্রকারলেখ কাঠামো Pager Pager;

/*
** Handle type for pages.
*/
প্রকারলেখ কাঠামো PgHdr DbPage;

/*
** Page number PAGER_SJ_PGNO is never used in an SQLite database (it is
** reserved for working around a windows/posix incompatibility). It is
** used in the journal to signify that the remainder of the journal file 
** is devoted to storing a super-journal name - there are no more pages to
** roll back. See comments for function writeSuperJournal() in pager.c 
** for details.
*/
#define PAGER_SJ_PGNO_COMPUTED(x) ((Pgno)((PENDING_BYTE/((x)->page_আকার))+1))
#define PAGER_SJ_PGNO(x)          ((x)->lckPgno)

/*
** Allowed values for the flags parameter to sqlite3PagerOpen().
**
** NOTE: These values must match the corresponding BTREE_ values in btree.h.
*/
#define PAGER_OMIT_JOURNAL  0x0001    /* Do not use a rollback journal */
#define PAGER_MEMORY        0x0002    /* In-memory database */

/*
** Valid values for the second argument to sqlite3PagerLockingMode().
*/
#define PAGER_LOCKINGMODE_QUERY      -1
#define PAGER_LOCKINGMODE_NORMAL      0
#define PAGER_LOCKINGMODE_EXCLUSIVE   1

/*
** Numeric constants that encode the journalmode.
**
** The numeric values encoded here (other than PAGER_JOURNALMODE_QUERY)
** are exposed in the API via the "PRAGMA journal_mode" command and
** therefore cannot be changed without a compatibility break.
*/
#define PAGER_JOURNALMODE_QUERY     (-1)  /* Query the value of journalmode */
#define PAGER_JOURNALMODE_DELETE      0   /* Commit by deleting journal file */
#define PAGER_JOURNALMODE_PERSIST     1   /* Commit by zeroing journal header */
#define PAGER_JOURNALMODE_OFF         2   /* Journal omitted.  */
#define PAGER_JOURNALMODE_TRUNCATE    3   /* Commit by truncating journal */
#define PAGER_JOURNALMODE_MEMORY      4   /* In-memory journal file */
#define PAGER_JOURNALMODE_WAL         5   /* Use write-ahead logging */

#define isWalMode(x) ((x)==PAGER_JOURNALMODE_WAL)

/*
** The argument to this macro is a file descriptor (type sqlite3_file*).
** Return 0 if it is not open, or non-zero (but not 1) if it is.
**
** This is so that expressions can be written as:
**
**   if( isOpen(pPager->jfd) ){ ...
**
** instead of
**
**   if( pPager->jfd->pMethods ){ ...
*/
#define isOpen(pFd) ((pFd)->pMethods!=0)

/*
** Flags that make up the mask passed to sqlite3PagerGet().
*/
#define PAGER_GET_NOCONTENT     0x01  /* Do not load data from disk */
#define PAGER_GET_READONLY      0x02  /* Read-only page is acceptable */

/*
** Flags for sqlite3PagerSetFlags()
**
** Value constraints (enforced via assert()):
**    PAGER_FULLFSYNC      == SQLITE_FullFSync
**    PAGER_CKPT_FULLFSYNC == SQLITE_CkptFullFSync
**    PAGER_CACHE_SPILL    == SQLITE_CacheSpill
*/
#define PAGER_SYNCHRONOUS_OFF       0x01  /* PRAGMA synchronous=OFF */
#define PAGER_SYNCHRONOUS_NORMAL    0x02  /* PRAGMA synchronous=NORMAL */
#define PAGER_SYNCHRONOUS_FULL      0x03  /* PRAGMA synchronous=FULL */
#define PAGER_SYNCHRONOUS_EXTRA     0x04  /* PRAGMA synchronous=EXTRA */
#define PAGER_SYNCHRONOUS_MASK      0x07  /* Mask for four values above */
#define PAGER_FULLFSYNC             0x08  /* PRAGMA fullfsync=ON */
#define PAGER_CKPT_FULLFSYNC        0x10  /* PRAGMA checkpoint_fullfsync=ON */
#define PAGER_CACHESPILL            0x20  /* PRAGMA cache_spill=ON */
#define PAGER_FLAGS_MASK            0x38  /* All above except SYNCHRONOUS */

/*
** The remainder of this file contains the declarations of the functions
** that make up the Pager sub-system API. See source code comments for 
** a detailed description of each routine.
*/

/* Open and close a Pager connection. */ 
পূর্ণসংখ্যা sqlite3PagerOpen(
  sqlite3_vfs*,
  Pager **ppPager,
  ধ্রুবক অক্ষর*,
  পূর্ণসংখ্যা,
  পূর্ণসংখ্যা,
  পূর্ণসংখ্যা,
  শূন্য(*)(DbPage*)
);
পূর্ণসংখ্যা sqlite3PagerClose(Pager *pPager, sqlite3*);
পূর্ণসংখ্যা sqlite3PagerReadFileheader(Pager*, পূর্ণসংখ্যা, unsigned অক্ষর*);

/* Functions used to configure a Pager object. */
শূন্য sqlite3PagerSetBusyHandler(Pager*, পূর্ণসংখ্যা(*)(শূন্য *), শূন্য *);
পূর্ণসংখ্যা sqlite3PagerSetPagesize(Pager*, u32*, পূর্ণসংখ্যা);
Pgno sqlite3_Pager_Max_Page_গুনতি(Pager*, Pgno);
শূন্য sqlite3PagerSetCachesize(Pager*, পূর্ণসংখ্যা);
পূর্ণসংখ্যা sqlite3PagerSetSpillsize(Pager*, পূর্ণসংখ্যা);
শূন্য sqlite3PagerSetMmapLimit(Pager *, sqlite3_int64);
শূন্য sqlite3PagerShrink(Pager*);
শূন্য sqlite3PagerSetFlags(Pager*,unsigned);
পূর্ণসংখ্যা sqlite3PagerLockingMode(Pager *, পূর্ণসংখ্যা);
পূর্ণসংখ্যা sqlite3PagerSetJournalMode(Pager *, পূর্ণসংখ্যা);
পূর্ণসংখ্যা sqlite3PagerGetJournalMode(Pager*);
পূর্ণসংখ্যা sqlite3PagerOkToChangeJournalMode(Pager*);
i64 sqlite3_Pager_Journal_আকার_Limit(Pager *, i64);
sqlite3_backup **sqlite3PagerBackupPtr(Pager*);
পূর্ণসংখ্যা sqlite3PagerFlush(Pager*);

/* Functions used to obtain and release page references. */ 
পূর্ণসংখ্যা sqlite3PagerGet(Pager *pPager, Pgno pgno, DbPage **ppPage, পূর্ণসংখ্যা clrFlag);
DbPage *sqlite3PagerLookup(Pager *pPager, Pgno pgno);
শূন্য sqlite3PagerRef(DbPage*);
শূন্য sqlite3PagerUnref(DbPage*);
শূন্য sqlite3PagerUnrefNotNull(DbPage*);
শূন্য sqlite3PagerUnrefPageOne(DbPage*);

/* Operations on page references. */
পূর্ণসংখ্যা sqlite3PagerWrite(DbPage*);
শূন্য sqlite3PagerDontWrite(DbPage*);
পূর্ণসংখ্যা sqlite3PagerMovepage(Pager*,DbPage*,Pgno,পূর্ণসংখ্যা);
পূর্ণসংখ্যা sqlite3PagerPageRefcount(DbPage*);
শূন্য *sqlite3_Pager_Get_তথ্য(DbPage *); 
শূন্য *sqlite3PagerGetExtra(DbPage *); 

/* Functions used to manage pager transactions and savepoints. */
শূন্য sqlite3PagerPagecount(Pager*, পূর্ণসংখ্যা*);
পূর্ণসংখ্যা sqlite3PagerBegin(Pager*, পূর্ণসংখ্যা exFlag, পূর্ণসংখ্যা);
পূর্ণসংখ্যা sqlite3PagerCommitPhaseOne(Pager*,ধ্রুবক অক্ষর *zSuper, পূর্ণসংখ্যা);
পূর্ণসংখ্যা sqlite3PagerExclusiveLock(Pager*);
পূর্ণসংখ্যা sqlite3PagerSync(Pager *pPager, ধ্রুবক অক্ষর *zSuper);
পূর্ণসংখ্যা sqlite3PagerCommitPhaseTwo(Pager*);
পূর্ণসংখ্যা sqlite3PagerRollback(Pager*);
পূর্ণসংখ্যা sqlite3PagerOpenSavepoint(Pager *pPager, পূর্ণসংখ্যা n);
পূর্ণসংখ্যা sqlite3PagerSavepoint(Pager *pPager, পূর্ণসংখ্যা op, পূর্ণসংখ্যা iSavepoint);
পূর্ণসংখ্যা sqlite3PagerSharedLock(Pager *pPager);

#ifndef SQLITE_OMIT_WAL
  পূর্ণসংখ্যা sqlite3PagerCheckpoint(Pager *pPager, sqlite3*, পূর্ণসংখ্যা, পূর্ণসংখ্যা*, পূর্ণসংখ্যা*);
  পূর্ণসংখ্যা sqlite3PagerWalSupported(Pager *pPager);
  পূর্ণসংখ্যা sqlite3PagerWalCallback(Pager *pPager);
  পূর্ণসংখ্যা sqlite3PagerOpenWal(Pager *pPager, পূর্ণসংখ্যা *pisOpen);
  পূর্ণসংখ্যা sqlite3PagerCloseWal(Pager *pPager, sqlite3*);
# ifdef SQLITE_ENABLE_SNAPSHOT
  পূর্ণসংখ্যা sqlite3PagerSnapshotGet(Pager*, sqlite3_snapshot **ppSnapshot);
  পূর্ণসংখ্যা sqlite3PagerSnapshotOpen(Pager*, sqlite3_snapshot *pSnapshot);
  পূর্ণসংখ্যা sqlite3PagerSnapshotRecover(Pager *pPager);
  পূর্ণসংখ্যা sqlite3PagerSnapshotCheck(Pager *pPager, sqlite3_snapshot *pSnapshot);
  শূন্য sqlite3PagerSnapshotUnlock(Pager *pPager);
# endif
#endif

#যদি !defined(SQLITE_OMIT_WAL) && defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  পূর্ণসংখ্যা sqlite3PagerWalWriteLock(Pager*, পূর্ণসংখ্যা);
  শূন্য sqlite3PagerWalDb(Pager*, sqlite3*);
#অন্যথা
# define sqlite3PagerWalWriteLock(y,z) SQLITE_OK
# define sqlite3PagerWalDb(x,y)
#endif

#ifdef SQLITE_DIRECT_OVERFLOW_READ
  পূর্ণসংখ্যা sqlite3PagerDirectReadOk(Pager *pPager, Pgno pgno);
#endif

#ifdef SQLITE_ENABLE_ZIPVFS
  পূর্ণসংখ্যা sqlite3PagerWalFramesize(Pager *pPager);
#endif

/* Functions used to query pager state and configuration. */
u8 sqlite3PagerIsreadonly(Pager*);
u32 sqlite3_Pager_তথ্য_Version(Pager*);
#ifdef SQLITE_DEBUG
  পূর্ণসংখ্যা sqlite3PagerRefcount(Pager*);
#endif
পূর্ণসংখ্যা sqlite3PagerMemUsed(Pager*);
ধ্রুবক অক্ষর *sqlite3PagerFilename(ধ্রুবক Pager*, পূর্ণসংখ্যা);
sqlite3_vfs *sqlite3PagerVfs(Pager*);
sqlite3_file *sqlite3PagerFile(Pager*);
sqlite3_file *sqlite3PagerJrnlFile(Pager*);
ধ্রুবক অক্ষর *sqlite3PagerJournalname(Pager*);
শূন্য *sqlite3_Pager_অস্থায়ী_Space(Pager*);
পূর্ণসংখ্যা sqlite3PagerIsMemdb(Pager*);
শূন্য sqlite3PagerCacheStat(Pager *, পূর্ণসংখ্যা, পূর্ণসংখ্যা, u64*);
শূন্য sqlite3PagerClearCache(Pager*);
পূর্ণসংখ্যা sqlite3_Sector_আকার(sqlite3_file *);

/* Functions used to truncate the database file. */
শূন্য sqlite3PagerTruncateImage(Pager*,Pgno);

শূন্য sqlite3PagerRekey(DbPage*, Pgno, u16);

/* Functions to support testing and debugging. */
#যদি !defined(NDEBUG) || defined(SQLITE_TEST)
  Pgno sqlite3PagerPagenumber(DbPage*);
  পূর্ণসংখ্যা sqlite3PagerIswriteable(DbPage*);
#endif
#ifdef SQLITE_TEST
  পূর্ণসংখ্যা *sqlite3PagerStats(Pager*);
  শূন্য sqlite3PagerRefdump(Pager*);
  শূন্য disable_simulated_io_errors(শূন্য);
  শূন্য enable_simulated_io_errors(শূন্য);
#অন্যথা
# define disable_simulated_io_errors()
# define enable_simulated_io_errors()
#endif

#যদি defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
পূর্ণসংখ্যা sqlite3PagerWalSystemErrno(Pager*);
#endif

#যদি defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  শূন্য sqlite3PagerWalStat(Pager*,sqlite3_str*);
#endif

#endif /* SQLITE_PAGER_H */
