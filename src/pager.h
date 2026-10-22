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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_PAGER_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_PAGER_H

/*
** Default maximum size for persistent journal files. A negative 
** value means no limit. This value may be overridden using the 
** sqlite3PagerJournalSizeLimit() API. See also "PRAGMA journal_size_limit".
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_JOURNAL_ਮਾਪ_ਹੱਦਬੰਦੀ
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_DEFAULT_JOURNAL_ਮਾਪ_ਹੱਦਬੰਦੀ -1
#ਜੇ_ਅੰਤ

/*
** The type used to represent a page number.  The first page in a file
** is called page 1.  0 is used to represent "not a page".
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ u32 Pgno;

/*
** Each open file is managed by a separate instance of the "Pager" structure.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Pager Pager;

/*
** Handle type for pages.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ PgHdr Db_ਪੰਨਾ;

/*
** Page number PAGER_SJ_PGNO is never used in an SQLite database (it is
** reserved for working around a windows/posix incompatibility). It is
** used in the journal to signify that the remainder of the journal file 
** is devoted to storing a super-journal name - there are no more pages to
** roll back. See comments for function writeSuperJournal() in pager.c 
** for details.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_SJ_PGNO_COMPUTED(x) ((Pgno)((PENDING_BYTE/((x)->ਪੰਨਾ_ਮਾਪ))+1))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_SJ_PGNO(x)          ((x)->lckPgno)

/*
** Allowed values for the flags parameter to sqlite3PagerOpen().
**
** NOTE: These values must match the corresponding BTREE_ values in btree.h.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_OMIT_JOURNAL  0x0001    /* Do not use a rollback journal */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_ਸਮ੍ਰਿਤੀ        0x0002    /* In-memory database */

/*
** Valid values for the second argument to sqlite3PagerLockingMode().
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_LOCKINGMODE_QUERY      -1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_LOCKINGMODE_ਸਾਧਾਰਨ      0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_LOCKINGMODE_EXCLUSIVE   1

/*
** Numeric constants that encode the journalmode.
**
** The numeric values encoded here (other than PAGER_JOURNALMODE_QUERY)
** are exposed in the API via the "PRAGMA journal_mode" command and
** therefore cannot be changed without a compatibility break.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_JOURNALMODE_QUERY     (-1)  /* Query the value of journalmode */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_JOURNALMODE_DELETE      0   /* Commit by deleting journal file */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_JOURNALMODE_PERSIST     1   /* Commit by zeroing journal header */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_JOURNALMODE_OFF         2   /* Journal omitted.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_JOURNALMODE_TRUNCATE    3   /* Commit by truncating journal */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_JOURNALMODE_ਸਮ੍ਰਿਤੀ      4   /* In-memory journal file */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_JOURNALMODE_WAL         5   /* Use write-ahead logging */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_Wal_ਬਹੁਲਕ(x) ((x)==PAGER_JOURNALMODE_WAL)

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ isOpen(pFd) ((pFd)->pMethods!=0)

/*
** Flags that make up the mask passed to sqlite3PagerGet().
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_GET_NOCONTENT     0x01  /* Do not load data from disk */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_GET_READONLY      0x02  /* Read-only page is acceptable */

/*
** Flags for sqlite3PagerSetFlags()
**
** Value constraints (enforced via assert()):
**    PAGER_FULLFSYNC      == SQLITE_FullFSync
**    PAGER_CKPT_FULLFSYNC == SQLITE_CkptFullFSync
**    PAGER_CACHE_SPILL    == SQLITE_CacheSpill
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_SYNCHRONOUS_OFF       0x01  /* PRAGMA synchronous=OFF */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_SYNCHRONOUS_ਸਾਧਾਰਨ    0x02  /* PRAGMA synchronous=NORMAL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_SYNCHRONOUS_ਭਰਿਆ      0x03  /* PRAGMA synchronous=FULL */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_SYNCHRONOUS_EXTRA     0x04  /* PRAGMA synchronous=EXTRA */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_SYNCHRONOUS_ਨਕਾਬ      0x07  /* Mask for four values above */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_FULLFSYNC             0x08  /* PRAGMA fullfsync=ON */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_CKPT_FULLFSYNC        0x10  /* PRAGMA checkpoint_fullfsync=ON */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_CACHESPILL            0x20  /* PRAGMA cache_spill=ON */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PAGER_FLAGS_ਨਕਾਬ            0x38  /* All above except SYNCHRONOUS */

/*
** The remainder of this file contains the declarations of the functions
** that make up the Pager sub-system API. See source code comments for 
** a detailed description of each routine.
*/

/* Open and close a Pager connection. */ 
ਪੂਰਨ sqlite3PagerOpen(
  sqlite3_vfs*,
  Pager **ppPager,
  ਅਚਲ ਅੱਖਰ*,
  ਪੂਰਨ,
  ਪੂਰਨ,
  ਪੂਰਨ,
  ਖਾਲੀ(*)(Db_ਪੰਨਾ*)
);
ਪੂਰਨ sqlite3PagerClose(Pager *pPager, sqlite3*);
ਪੂਰਨ sqlite3PagerReadFileheader(Pager*, ਪੂਰਨ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*);

/* Functions used to configure a Pager object. */
ਖਾਲੀ sqlite3_Pager_ਸੈੱਟ_Busy_Handler(Pager*, ਪੂਰਨ(*)(ਖਾਲੀ *), ਖਾਲੀ *);
ਪੂਰਨ sqlite3_Pager_ਸੈੱਟ_Pagesize(Pager*, u32*, ਪੂਰਨ);
Pgno sqlite3_Pager_Max_ਪੰਨਾ_ਗਿਣਤੀ(Pager*, Pgno);
ਖਾਲੀ sqlite3_Pager_ਸੈੱਟ_Cachesize(Pager*, ਪੂਰਨ);
ਪੂਰਨ sqlite3_Pager_ਸੈੱਟ_Spillsize(Pager*, ਪੂਰਨ);
ਖਾਲੀ sqlite3_Pager_ਸੈੱਟ_Mmap_ਹੱਦਬੰਦੀ(Pager *, sqlite3_int64);
ਖਾਲੀ sqlite3PagerShrink(Pager*);
ਖਾਲੀ sqlite3_Pager_ਸੈੱਟ_Flags(Pager*,ਬਿਨਾਨਿਸ਼ਾਨੀ);
ਪੂਰਨ sqlite3_Pager_Locking_ਬਹੁਲਕ(Pager *, ਪੂਰਨ);
ਪੂਰਨ sqlite3_Pager_ਸੈੱਟ_Journal_ਬਹੁਲਕ(Pager *, ਪੂਰਨ);
ਪੂਰਨ sqlite3_Pager_Get_Journal_ਬਹੁਲਕ(Pager*);
ਪੂਰਨ sqlite3_Pager_Ok_ਮੰਜ਼ਿਲ_Change_Journal_ਬਹੁਲਕ(Pager*);
i64 sqlite3_Pager_Journal_ਮਾਪ_ਹੱਦਬੰਦੀ(Pager *, i64);
sqlite3_backup **sqlite3PagerBackupPtr(Pager*);
ਪੂਰਨ sqlite3PagerFlush(Pager*);

/* Functions used to obtain and release page references. */ 
ਪੂਰਨ sqlite3PagerGet(Pager *pPager, Pgno pgno, Db_ਪੰਨਾ **pp_ਪੰਨਾ, ਪੂਰਨ clrFlag);
Db_ਪੰਨਾ *sqlite3PagerLookup(Pager *pPager, Pgno pgno);
ਖਾਲੀ sqlite3PagerRef(Db_ਪੰਨਾ*);
ਖਾਲੀ sqlite3PagerUnref(Db_ਪੰਨਾ*);
ਖਾਲੀ sqlite3PagerUnrefNotNull(Db_ਪੰਨਾ*);
ਖਾਲੀ sqlite3_Pager_Unref_ਪੰਨਾ_One(Db_ਪੰਨਾ*);

/* Operations on page references. */
ਪੂਰਨ sqlite3PagerWrite(Db_ਪੰਨਾ*);
ਖਾਲੀ sqlite3PagerDontWrite(Db_ਪੰਨਾ*);
ਪੂਰਨ sqlite3PagerMovepage(Pager*,Db_ਪੰਨਾ*,Pgno,ਪੂਰਨ);
ਪੂਰਨ sqlite3_Pager_ਪੰਨਾ_Refcount(Db_ਪੰਨਾ*);
ਖਾਲੀ *sqlite3_Pager_Get_ਅੰਕੜੇ(Db_ਪੰਨਾ *); 
ਖਾਲੀ *sqlite3PagerGetExtra(Db_ਪੰਨਾ *); 

/* Functions used to manage pager transactions and savepoints. */
ਖਾਲੀ sqlite3PagerPagecount(Pager*, ਪੂਰਨ*);
ਪੂਰਨ sqlite3PagerBegin(Pager*, ਪੂਰਨ exFlag, ਪੂਰਨ);
ਪੂਰਨ sqlite3PagerCommitPhaseOne(Pager*,ਅਚਲ ਅੱਖਰ *zSuper, ਪੂਰਨ);
ਪੂਰਨ sqlite3_Pager_Exclusive_ਤਾਲਾ(Pager*);
ਪੂਰਨ sqlite3PagerSync(Pager *pPager, ਅਚਲ ਅੱਖਰ *zSuper);
ਪੂਰਨ sqlite3PagerCommitPhaseTwo(Pager*);
ਪੂਰਨ sqlite3PagerRollback(Pager*);
ਪੂਰਨ sqlite3PagerOpenSavepoint(Pager *pPager, ਪੂਰਨ n);
ਪੂਰਨ sqlite3PagerSavepoint(Pager *pPager, ਪੂਰਨ op, ਪੂਰਨ iSavepoint);
ਪੂਰਨ sqlite3_Pager_Shared_ਤਾਲਾ(Pager *pPager);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  ਪੂਰਨ sqlite3PagerCheckpoint(Pager *pPager, sqlite3*, ਪੂਰਨ, ਪੂਰਨ*, ਪੂਰਨ*);
  ਪੂਰਨ sqlite3PagerWalSupported(Pager *pPager);
  ਪੂਰਨ sqlite3PagerWalCallback(Pager *pPager);
  ਪੂਰਨ sqlite3PagerOpenWal(Pager *pPager, ਪੂਰਨ *pisOpen);
  ਪੂਰਨ sqlite3PagerCloseWal(Pager *pPager, sqlite3*);
# ifdef SQLITE_ENABLE_SNAPSHOT
  ਪੂਰਨ sqlite3PagerSnapshotGet(Pager*, sqlite3_snapshot **ppSnapshot);
  ਪੂਰਨ sqlite3PagerSnapshotOpen(Pager*, sqlite3_snapshot *pSnapshot);
  ਪੂਰਨ sqlite3PagerSnapshotRecover(Pager *pPager);
  ਪੂਰਨ sqlite3PagerSnapshotCheck(Pager *pPager, sqlite3_snapshot *pSnapshot);
  ਖਾਲੀ sqlite3PagerSnapshotUnlock(Pager *pPager);
# endif
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SQLITE_OMIT_WAL) && defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  ਪੂਰਨ sqlite3_Pager_Wal_Write_ਤਾਲਾ(Pager*, ਪੂਰਨ);
  ਖਾਲੀ sqlite3PagerWalDb(Pager*, sqlite3*);
#ਨਹੀਂਤਾਂ
# define sqlite3_Pager_Wal_Write_ਤਾਲਾ(y,z) SQLITE_OK
# define sqlite3PagerWalDb(x,y)
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ਸਿੱਧਾ_ਉਛਾਲ_READ
  ਪੂਰਨ sqlite3_Pager_ਸਿੱਧਾ_Read_Ok(Pager *pPager, Pgno pgno);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ZIPVFS
  ਪੂਰਨ sqlite3PagerWalFramesize(Pager *pPager);
#ਜੇ_ਅੰਤ

/* Functions used to query pager state and configuration. */
u8 sqlite3PagerIsreadonly(Pager*);
u32 sqlite3_Pager_ਅੰਕੜੇ_ਸੰਸਕਰਣ(Pager*);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  ਪੂਰਨ sqlite3PagerRefcount(Pager*);
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3_Pager_Mem_ਵਰਤਿਆ(Pager*);
ਅਚਲ ਅੱਖਰ *sqlite3PagerFilename(ਅਚਲ Pager*, ਪੂਰਨ);
sqlite3_vfs *sqlite3PagerVfs(Pager*);
sqlite3_file *sqlite3PagerFile(Pager*);
sqlite3_file *sqlite3PagerJrnlFile(Pager*);
ਅਚਲ ਅੱਖਰ *sqlite3PagerJournalname(Pager*);
ਖਾਲੀ *sqlite3_Pager_ਆਰਜ਼ੀ_Space(Pager*);
ਪੂਰਨ sqlite3PagerIsMemdb(Pager*);
ਖਾਲੀ sqlite3PagerCacheStat(Pager *, ਪੂਰਨ, ਪੂਰਨ, u64*);
ਖਾਲੀ sqlite3PagerClearCache(Pager*);
ਪੂਰਨ sqlite3_Sector_ਮਾਪ(sqlite3_file *);

/* Functions used to truncate the database file. */
ਖਾਲੀ sqlite3PagerTruncateImage(Pager*,Pgno);

ਖਾਲੀ sqlite3PagerRekey(Db_ਪੰਨਾ*, Pgno, u16);

/* Functions to support testing and debugging. */
#ਜੇ !defined(NDEBUG) || defined(SQLITE_TEST)
  Pgno sqlite3PagerPagenumber(Db_ਪੰਨਾ*);
  ਪੂਰਨ sqlite3PagerIswriteable(Db_ਪੰਨਾ*);
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
  ਪੂਰਨ *sqlite3PagerStats(Pager*);
  ਖਾਲੀ sqlite3PagerRefdump(Pager*);
  ਖਾਲੀ disable_simulated_io_errors(ਖਾਲੀ);
  ਖਾਲੀ enable_simulated_io_errors(ਖਾਲੀ);
#ਨਹੀਂਤਾਂ
# define disable_simulated_io_errors()
# define enable_simulated_io_errors()
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
ਪੂਰਨ sqlite3PagerWalSystemErrno(Pager*);
#ਜੇ_ਅੰਤ

#ਜੇ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  ਖਾਲੀ sqlite3PagerWalStat(Pager*,sqlite3_str*);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* SQLITE_PAGER_H */
