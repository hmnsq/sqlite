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
typedef u32 Pgno;

/*
** Each open file is managed by a separate instance of the "Pager" structure.
*/
typedef struct Pager Pager;

/*
** Handle type for pages.
*/
typedef struct PgHdr DbPage;

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
整数 sqlite3PagerOpen(
  sqlite3_vfs*,
  Pager **ppPager,
  const char*,
  整数,
  整数,
  整数,
  void(*)(DbPage*)
);
整数 sqlite3PagerClose(Pager *pPager, sqlite3*);
整数 sqlite3PagerReadFileheader(Pager*, 整数, unsigned char*);

/* Functions used to configure a Pager object. */
void sqlite3PagerSetBusyHandler(Pager*, 整数(*)(void *), void *);
整数 sqlite3PagerSetPagesize(Pager*, u32*, 整数);
Pgno sqlite3PagerMaxPageCount(Pager*, Pgno);
void sqlite3PagerSetCachesize(Pager*, 整数);
整数 sqlite3PagerSetSpillsize(Pager*, 整数);
void sqlite3PagerSetMmapLimit(Pager *, sqlite3_int64);
void sqlite3PagerShrink(Pager*);
void sqlite3PagerSetFlags(Pager*,unsigned);
整数 sqlite3PagerLockingMode(Pager *, 整数);
整数 sqlite3PagerSetJournalMode(Pager *, 整数);
整数 sqlite3PagerGetJournalMode(Pager*);
整数 sqlite3PagerOkToChangeJournalMode(Pager*);
i64 sqlite3PagerJournalSizeLimit(Pager *, i64);
sqlite3_backup **sqlite3PagerBackupPtr(Pager*);
整数 sqlite3PagerFlush(Pager*);

/* Functions used to obtain and release page references. */ 
整数 sqlite3PagerGet(Pager *pPager, Pgno pgno, DbPage **ppPage, 整数 clrFlag);
DbPage *sqlite3PagerLookup(Pager *pPager, Pgno pgno);
void sqlite3PagerRef(DbPage*);
void sqlite3PagerUnref(DbPage*);
void sqlite3PagerUnrefNotNull(DbPage*);
void sqlite3PagerUnrefPageOne(DbPage*);

/* Operations on page references. */
整数 sqlite3PagerWrite(DbPage*);
void sqlite3PagerDontWrite(DbPage*);
整数 sqlite3PagerMovepage(Pager*,DbPage*,Pgno,整数);
整数 sqlite3PagerPageRefcount(DbPage*);
void *sqlite3PagerGetData(DbPage *); 
void *sqlite3PagerGetExtra(DbPage *); 

/* Functions used to manage pager transactions and savepoints. */
void sqlite3PagerPagecount(Pager*, 整数*);
整数 sqlite3PagerBegin(Pager*, 整数 exFlag, 整数);
整数 sqlite3PagerCommitPhaseOne(Pager*,const char *zSuper, 整数);
整数 sqlite3PagerExclusiveLock(Pager*);
整数 sqlite3PagerSync(Pager *pPager, const char *zSuper);
整数 sqlite3PagerCommitPhaseTwo(Pager*);
整数 sqlite3PagerRollback(Pager*);
整数 sqlite3PagerOpenSavepoint(Pager *pPager, 整数 n);
整数 sqlite3PagerSavepoint(Pager *pPager, 整数 op, 整数 iSavepoint);
整数 sqlite3PagerSharedLock(Pager *pPager);

#ifndef SQLITE_OMIT_WAL
  整数 sqlite3PagerCheckpoint(Pager *pPager, sqlite3*, 整数, 整数*, 整数*);
  整数 sqlite3PagerWalSupported(Pager *pPager);
  整数 sqlite3PagerWalCallback(Pager *pPager);
  整数 sqlite3PagerOpenWal(Pager *pPager, 整数 *pisOpen);
  整数 sqlite3PagerCloseWal(Pager *pPager, sqlite3*);
# ifdef SQLITE_ENABLE_SNAPSHOT
  整数 sqlite3PagerSnapshotGet(Pager*, sqlite3_snapshot **ppSnapshot);
  整数 sqlite3PagerSnapshotOpen(Pager*, sqlite3_snapshot *pSnapshot);
  整数 sqlite3PagerSnapshotRecover(Pager *pPager);
  整数 sqlite3PagerSnapshotCheck(Pager *pPager, sqlite3_snapshot *pSnapshot);
  void sqlite3PagerSnapshotUnlock(Pager *pPager);
# endif
#endif

#如果 !defined(SQLITE_OMIT_WAL) && defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  整数 sqlite3PagerWalWriteLock(Pager*, 整数);
  void sqlite3PagerWalDb(Pager*, sqlite3*);
#否则
# define sqlite3PagerWalWriteLock(y,z) SQLITE_OK
# define sqlite3PagerWalDb(x,y)
#endif

#ifdef SQLITE_DIRECT_OVERFLOW_READ
  整数 sqlite3PagerDirectReadOk(Pager *pPager, Pgno pgno);
#endif

#ifdef SQLITE_ENABLE_ZIPVFS
  整数 sqlite3PagerWalFramesize(Pager *pPager);
#endif

/* Functions used to query pager state and configuration. */
u8 sqlite3PagerIsreadonly(Pager*);
u32 sqlite3PagerDataVersion(Pager*);
#ifdef SQLITE_DEBUG
  整数 sqlite3PagerRefcount(Pager*);
#endif
整数 sqlite3PagerMemUsed(Pager*);
const char *sqlite3PagerFilename(const Pager*, 整数);
sqlite3_vfs *sqlite3PagerVfs(Pager*);
sqlite3_file *sqlite3PagerFile(Pager*);
sqlite3_file *sqlite3PagerJrnlFile(Pager*);
const char *sqlite3PagerJournalname(Pager*);
void *sqlite3PagerTempSpace(Pager*);
整数 sqlite3PagerIsMemdb(Pager*);
void sqlite3PagerCacheStat(Pager *, 整数, 整数, u64*);
void sqlite3PagerClearCache(Pager*);
整数 sqlite3SectorSize(sqlite3_file *);

/* Functions used to truncate the database file. */
void sqlite3PagerTruncateImage(Pager*,Pgno);

void sqlite3PagerRekey(DbPage*, Pgno, u16);

/* Functions to support testing and debugging. */
#如果 !defined(NDEBUG) || defined(SQLITE_TEST)
  Pgno sqlite3PagerPagenumber(DbPage*);
  整数 sqlite3PagerIswriteable(DbPage*);
#endif
#ifdef SQLITE_TEST
  整数 *sqlite3PagerStats(Pager*);
  void sqlite3PagerRefdump(Pager*);
  void disable_simulated_io_errors(void);
  void enable_simulated_io_errors(void);
#否则
# define disable_simulated_io_errors()
# define enable_simulated_io_errors()
#endif

#如果 defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
整数 sqlite3PagerWalSystemErrno(Pager*);
#endif

#如果 defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  void sqlite3PagerWalStat(Pager*,sqlite3_str*);
#endif

#endif /* SQLITE_PAGER_H */
