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
#ifndef SQLITE_DEFAULT_JOURNAL_SIZE_LIMIT
  #define SQLITE_DEFAULT_JOURNAL_SIZE_LIMIT -1
#endif

/*
** The type used to represent a page number.  The first page in a file
** is called page 1.  0 is used to represent "not a page".
*/
வகைவரை u32 Pgno;

/*
** Each open file is managed by a separate instance of the "Pager" structure.
*/
வகைவரை கட்டமைப்பு Pager Pager;

/*
** Handle type for pages.
*/
வகைவரை கட்டமைப்பு PgHdr DbPage;

/*
** Page number PAGER_SJ_PGNO is never used in an SQLite database (it is
** reserved for working around a windows/posix incompatibility). It is
** used in the journal to signify that the remainder of the journal file 
** is devoted to storing a super-journal name - there are no more pages to
** roll back. See comments for function writeSuperJournal() in pager.c 
** for details.
*/
#define PAGER_SJ_PGNO_COMPUTED(x) ((Pgno)((PENDING_BYTE/((x)->pageSize))+1))
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
முழு sqlite3PagerOpen(
  sqlite3_vfs*,
  Pager **ppPager,
  மாறிலி எழுத்து*,
  முழு,
  முழு,
  முழு,
  வெறுமை(*)(DbPage*)
);
முழு sqlite3PagerClose(Pager *pPager, sqlite3*);
முழு sqlite3PagerReadFileheader(Pager*, முழு, unsigned எழுத்து*);

/* Functions used to configure a Pager object. */
வெறுமை sqlite3PagerSetBusyHandler(Pager*, முழு(*)(வெறுமை *), வெறுமை *);
முழு sqlite3PagerSetPagesize(Pager*, u32*, முழு);
Pgno sqlite3_Pager_Max_Page_மொத்தம்(Pager*, Pgno);
வெறுமை sqlite3PagerSetCachesize(Pager*, முழு);
முழு sqlite3PagerSetSpillsize(Pager*, முழு);
வெறுமை sqlite3PagerSetMmapLimit(Pager *, sqlite3_int64);
வெறுமை sqlite3PagerShrink(Pager*);
வெறுமை sqlite3PagerSetFlags(Pager*,unsigned);
முழு sqlite3PagerLockingMode(Pager *, முழு);
முழு sqlite3PagerSetJournalMode(Pager *, முழு);
முழு sqlite3PagerGetJournalMode(Pager*);
முழு sqlite3PagerOkToChangeJournalMode(Pager*);
i64 sqlite3PagerJournalSizeLimit(Pager *, i64);
sqlite3_backup **sqlite3PagerBackupPtr(Pager*);
முழு sqlite3PagerFlush(Pager*);

/* Functions used to obtain and release page references. */ 
முழு sqlite3PagerGet(Pager *pPager, Pgno pgno, DbPage **ppPage, முழு clrFlag);
DbPage *sqlite3PagerLookup(Pager *pPager, Pgno pgno);
வெறுமை sqlite3PagerRef(DbPage*);
வெறுமை sqlite3PagerUnref(DbPage*);
வெறுமை sqlite3PagerUnrefNotNull(DbPage*);
வெறுமை sqlite3PagerUnrefPageOne(DbPage*);

/* Operations on page references. */
முழு sqlite3PagerWrite(DbPage*);
வெறுமை sqlite3PagerDontWrite(DbPage*);
முழு sqlite3PagerMovepage(Pager*,DbPage*,Pgno,முழு);
முழு sqlite3PagerPageRefcount(DbPage*);
வெறுமை *sqlite3PagerGetData(DbPage *); 
வெறுமை *sqlite3PagerGetExtra(DbPage *); 

/* Functions used to manage pager transactions and savepoints. */
வெறுமை sqlite3PagerPagecount(Pager*, முழு*);
முழு sqlite3PagerBegin(Pager*, முழு exFlag, முழு);
முழு sqlite3PagerCommitPhaseOne(Pager*,மாறிலி எழுத்து *zSuper, முழு);
முழு sqlite3PagerExclusiveLock(Pager*);
முழு sqlite3PagerSync(Pager *pPager, மாறிலி எழுத்து *zSuper);
முழு sqlite3PagerCommitPhaseTwo(Pager*);
முழு sqlite3PagerRollback(Pager*);
முழு sqlite3PagerOpenSavepoint(Pager *pPager, முழு n);
முழு sqlite3PagerSavepoint(Pager *pPager, முழு op, முழு iSavepoint);
முழு sqlite3PagerSharedLock(Pager *pPager);

#ifndef SQLITE_OMIT_WAL
  முழு sqlite3PagerCheckpoint(Pager *pPager, sqlite3*, முழு, முழு*, முழு*);
  முழு sqlite3PagerWalSupported(Pager *pPager);
  முழு sqlite3PagerWalCallback(Pager *pPager);
  முழு sqlite3PagerOpenWal(Pager *pPager, முழு *pisOpen);
  முழு sqlite3PagerCloseWal(Pager *pPager, sqlite3*);
# ifdef SQLITE_ENABLE_SNAPSHOT
  முழு sqlite3PagerSnapshotGet(Pager*, sqlite3_snapshot **ppSnapshot);
  முழு sqlite3PagerSnapshotOpen(Pager*, sqlite3_snapshot *pSnapshot);
  முழு sqlite3PagerSnapshotRecover(Pager *pPager);
  முழு sqlite3PagerSnapshotCheck(Pager *pPager, sqlite3_snapshot *pSnapshot);
  வெறுமை sqlite3PagerSnapshotUnlock(Pager *pPager);
# endif
#endif

#எனில் !defined(SQLITE_OMIT_WAL) && defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  முழு sqlite3PagerWalWriteLock(Pager*, முழு);
  வெறுமை sqlite3PagerWalDb(Pager*, sqlite3*);
#இல்லையெனில்
# define sqlite3PagerWalWriteLock(y,z) SQLITE_OK
# define sqlite3PagerWalDb(x,y)
#endif

#ifdef SQLITE_DIRECT_OVERFLOW_READ
  முழு sqlite3PagerDirectReadOk(Pager *pPager, Pgno pgno);
#endif

#ifdef SQLITE_ENABLE_ZIPVFS
  முழு sqlite3PagerWalFramesize(Pager *pPager);
#endif

/* Functions used to query pager state and configuration. */
u8 sqlite3PagerIsreadonly(Pager*);
u32 sqlite3PagerDataVersion(Pager*);
#ifdef SQLITE_DEBUG
  முழு sqlite3PagerRefcount(Pager*);
#endif
முழு sqlite3PagerMemUsed(Pager*);
மாறிலி எழுத்து *sqlite3PagerFilename(மாறிலி Pager*, முழு);
sqlite3_vfs *sqlite3PagerVfs(Pager*);
sqlite3_file *sqlite3PagerFile(Pager*);
sqlite3_file *sqlite3PagerJrnlFile(Pager*);
மாறிலி எழுத்து *sqlite3PagerJournalname(Pager*);
வெறுமை *sqlite3PagerTempSpace(Pager*);
முழு sqlite3PagerIsMemdb(Pager*);
வெறுமை sqlite3PagerCacheStat(Pager *, முழு, முழு, u64*);
வெறுமை sqlite3PagerClearCache(Pager*);
முழு sqlite3SectorSize(sqlite3_file *);

/* Functions used to truncate the database file. */
வெறுமை sqlite3PagerTruncateImage(Pager*,Pgno);

வெறுமை sqlite3PagerRekey(DbPage*, Pgno, u16);

/* Functions to support testing and debugging. */
#எனில் !defined(NDEBUG) || defined(SQLITE_TEST)
  Pgno sqlite3PagerPagenumber(DbPage*);
  முழு sqlite3PagerIswriteable(DbPage*);
#endif
#ifdef SQLITE_TEST
  முழு *sqlite3PagerStats(Pager*);
  வெறுமை sqlite3PagerRefdump(Pager*);
  வெறுமை disable_simulated_io_errors(வெறுமை);
  வெறுமை enable_simulated_io_errors(வெறுமை);
#இல்லையெனில்
# define disable_simulated_io_errors()
# define enable_simulated_io_errors()
#endif

#எனில் defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
முழு sqlite3PagerWalSystemErrno(Pager*);
#endif

#எனில் defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  வெறுமை sqlite3PagerWalStat(Pager*,sqlite3_str*);
#endif

#endif /* SQLITE_PAGER_H */
