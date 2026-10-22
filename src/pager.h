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
തരനിർവചനം u32 Pgno;

/*
** Each open file is managed by a separate instance of the "Pager" structure.
*/
തരനിർവചനം ഘടന Pager Pager;

/*
** Handle type for pages.
*/
തരനിർവചനം ഘടന PgHdr DbPage;

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
പൂർണ്ണസംഖ്യ sqlite3PagerOpen(
  sqlite3_vfs*,
  Pager **ppPager,
  സ്ഥിരം അക്ഷരം*,
  പൂർണ്ണസംഖ്യ,
  പൂർണ്ണസംഖ്യ,
  പൂർണ്ണസംഖ്യ,
  ശൂന്യം(*)(DbPage*)
);
പൂർണ്ണസംഖ്യ sqlite3PagerClose(Pager *pPager, sqlite3*);
പൂർണ്ണസംഖ്യ sqlite3PagerReadFileheader(Pager*, പൂർണ്ണസംഖ്യ, unsigned അക്ഷരം*);

/* Functions used to configure a Pager object. */
ശൂന്യം sqlite3PagerSetBusyHandler(Pager*, പൂർണ്ണസംഖ്യ(*)(ശൂന്യം *), ശൂന്യം *);
പൂർണ്ണസംഖ്യ sqlite3PagerSetPagesize(Pager*, u32*, പൂർണ്ണസംഖ്യ);
Pgno sqlite3_Pager_Max_Page_എണ്ണം(Pager*, Pgno);
ശൂന്യം sqlite3PagerSetCachesize(Pager*, പൂർണ്ണസംഖ്യ);
പൂർണ്ണസംഖ്യ sqlite3PagerSetSpillsize(Pager*, പൂർണ്ണസംഖ്യ);
ശൂന്യം sqlite3PagerSetMmapLimit(Pager *, sqlite3_int64);
ശൂന്യം sqlite3PagerShrink(Pager*);
ശൂന്യം sqlite3PagerSetFlags(Pager*,unsigned);
പൂർണ്ണസംഖ്യ sqlite3PagerLockingMode(Pager *, പൂർണ്ണസംഖ്യ);
പൂർണ്ണസംഖ്യ sqlite3PagerSetJournalMode(Pager *, പൂർണ്ണസംഖ്യ);
പൂർണ്ണസംഖ്യ sqlite3PagerGetJournalMode(Pager*);
പൂർണ്ണസംഖ്യ sqlite3PagerOkToChangeJournalMode(Pager*);
i64 sqlite3PagerJournalSizeLimit(Pager *, i64);
sqlite3_backup **sqlite3PagerBackupPtr(Pager*);
പൂർണ്ണസംഖ്യ sqlite3PagerFlush(Pager*);

/* Functions used to obtain and release page references. */ 
പൂർണ്ണസംഖ്യ sqlite3PagerGet(Pager *pPager, Pgno pgno, DbPage **ppPage, പൂർണ്ണസംഖ്യ clrFlag);
DbPage *sqlite3PagerLookup(Pager *pPager, Pgno pgno);
ശൂന്യം sqlite3PagerRef(DbPage*);
ശൂന്യം sqlite3PagerUnref(DbPage*);
ശൂന്യം sqlite3PagerUnrefNotNull(DbPage*);
ശൂന്യം sqlite3PagerUnrefPageOne(DbPage*);

/* Operations on page references. */
പൂർണ്ണസംഖ്യ sqlite3PagerWrite(DbPage*);
ശൂന്യം sqlite3PagerDontWrite(DbPage*);
പൂർണ്ണസംഖ്യ sqlite3PagerMovepage(Pager*,DbPage*,Pgno,പൂർണ്ണസംഖ്യ);
പൂർണ്ണസംഖ്യ sqlite3PagerPageRefcount(DbPage*);
ശൂന്യം *sqlite3PagerGetData(DbPage *); 
ശൂന്യം *sqlite3PagerGetExtra(DbPage *); 

/* Functions used to manage pager transactions and savepoints. */
ശൂന്യം sqlite3PagerPagecount(Pager*, പൂർണ്ണസംഖ്യ*);
പൂർണ്ണസംഖ്യ sqlite3PagerBegin(Pager*, പൂർണ്ണസംഖ്യ exFlag, പൂർണ്ണസംഖ്യ);
പൂർണ്ണസംഖ്യ sqlite3PagerCommitPhaseOne(Pager*,സ്ഥിരം അക്ഷരം *zSuper, പൂർണ്ണസംഖ്യ);
പൂർണ്ണസംഖ്യ sqlite3PagerExclusiveLock(Pager*);
പൂർണ്ണസംഖ്യ sqlite3PagerSync(Pager *pPager, സ്ഥിരം അക്ഷരം *zSuper);
പൂർണ്ണസംഖ്യ sqlite3PagerCommitPhaseTwo(Pager*);
പൂർണ്ണസംഖ്യ sqlite3PagerRollback(Pager*);
പൂർണ്ണസംഖ്യ sqlite3PagerOpenSavepoint(Pager *pPager, പൂർണ്ണസംഖ്യ n);
പൂർണ്ണസംഖ്യ sqlite3PagerSavepoint(Pager *pPager, പൂർണ്ണസംഖ്യ op, പൂർണ്ണസംഖ്യ iSavepoint);
പൂർണ്ണസംഖ്യ sqlite3PagerSharedLock(Pager *pPager);

#ifndef SQLITE_OMIT_WAL
  പൂർണ്ണസംഖ്യ sqlite3PagerCheckpoint(Pager *pPager, sqlite3*, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ*, പൂർണ്ണസംഖ്യ*);
  പൂർണ്ണസംഖ്യ sqlite3PagerWalSupported(Pager *pPager);
  പൂർണ്ണസംഖ്യ sqlite3PagerWalCallback(Pager *pPager);
  പൂർണ്ണസംഖ്യ sqlite3PagerOpenWal(Pager *pPager, പൂർണ്ണസംഖ്യ *pisOpen);
  പൂർണ്ണസംഖ്യ sqlite3PagerCloseWal(Pager *pPager, sqlite3*);
# ifdef SQLITE_ENABLE_SNAPSHOT
  പൂർണ്ണസംഖ്യ sqlite3PagerSnapshotGet(Pager*, sqlite3_snapshot **ppSnapshot);
  പൂർണ്ണസംഖ്യ sqlite3PagerSnapshotOpen(Pager*, sqlite3_snapshot *pSnapshot);
  പൂർണ്ണസംഖ്യ sqlite3PagerSnapshotRecover(Pager *pPager);
  പൂർണ്ണസംഖ്യ sqlite3PagerSnapshotCheck(Pager *pPager, sqlite3_snapshot *pSnapshot);
  ശൂന്യം sqlite3PagerSnapshotUnlock(Pager *pPager);
# endif
#endif

#എങ്കിൽ !defined(SQLITE_OMIT_WAL) && defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  പൂർണ്ണസംഖ്യ sqlite3PagerWalWriteLock(Pager*, പൂർണ്ണസംഖ്യ);
  ശൂന്യം sqlite3PagerWalDb(Pager*, sqlite3*);
#അല്ലെങ്കിൽ
# define sqlite3PagerWalWriteLock(y,z) SQLITE_OK
# define sqlite3PagerWalDb(x,y)
#endif

#ifdef SQLITE_DIRECT_OVERFLOW_READ
  പൂർണ്ണസംഖ്യ sqlite3PagerDirectReadOk(Pager *pPager, Pgno pgno);
#endif

#ifdef SQLITE_ENABLE_ZIPVFS
  പൂർണ്ണസംഖ്യ sqlite3PagerWalFramesize(Pager *pPager);
#endif

/* Functions used to query pager state and configuration. */
u8 sqlite3PagerIsreadonly(Pager*);
u32 sqlite3PagerDataVersion(Pager*);
#ifdef SQLITE_DEBUG
  പൂർണ്ണസംഖ്യ sqlite3PagerRefcount(Pager*);
#endif
പൂർണ്ണസംഖ്യ sqlite3PagerMemUsed(Pager*);
സ്ഥിരം അക്ഷരം *sqlite3PagerFilename(സ്ഥിരം Pager*, പൂർണ്ണസംഖ്യ);
sqlite3_vfs *sqlite3PagerVfs(Pager*);
sqlite3_file *sqlite3PagerFile(Pager*);
sqlite3_file *sqlite3PagerJrnlFile(Pager*);
സ്ഥിരം അക്ഷരം *sqlite3PagerJournalname(Pager*);
ശൂന്യം *sqlite3PagerTempSpace(Pager*);
പൂർണ്ണസംഖ്യ sqlite3PagerIsMemdb(Pager*);
ശൂന്യം sqlite3PagerCacheStat(Pager *, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ, u64*);
ശൂന്യം sqlite3PagerClearCache(Pager*);
പൂർണ്ണസംഖ്യ sqlite3SectorSize(sqlite3_file *);

/* Functions used to truncate the database file. */
ശൂന്യം sqlite3PagerTruncateImage(Pager*,Pgno);

ശൂന്യം sqlite3PagerRekey(DbPage*, Pgno, u16);

/* Functions to support testing and debugging. */
#എങ്കിൽ !defined(NDEBUG) || defined(SQLITE_TEST)
  Pgno sqlite3PagerPagenumber(DbPage*);
  പൂർണ്ണസംഖ്യ sqlite3PagerIswriteable(DbPage*);
#endif
#ifdef SQLITE_TEST
  പൂർണ്ണസംഖ്യ *sqlite3PagerStats(Pager*);
  ശൂന്യം sqlite3PagerRefdump(Pager*);
  ശൂന്യം disable_simulated_io_errors(ശൂന്യം);
  ശൂന്യം enable_simulated_io_errors(ശൂന്യം);
#അല്ലെങ്കിൽ
# define disable_simulated_io_errors()
# define enable_simulated_io_errors()
#endif

#എങ്കിൽ defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
പൂർണ്ണസംഖ്യ sqlite3PagerWalSystemErrno(Pager*);
#endif

#എങ്കിൽ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  ശൂന്യം sqlite3PagerWalStat(Pager*,sqlite3_str*);
#endif

#endif /* SQLITE_PAGER_H */
