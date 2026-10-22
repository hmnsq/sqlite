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

#که_نه_تعریف_شوی SQLITE_PAGER_H
#تعریف_کړه SQLITE_PAGER_H

/*
** Default maximum size for persistent journal files. A negative 
** value means no limit. This value may be overridden using the 
** sqlite3PagerJournalSizeLimit() API. See also "PRAGMA journal_size_limit".
*/
#که_نه_تعریف_شوی SQLITE_DEFAULT_JOURNAL_اندازه_حد
  #تعریف_کړه SQLITE_DEFAULT_JOURNAL_اندازه_حد -1
#د_که_پای

/*
** The type used to represent a page number.  The first page in a file
** is called page 1.  0 is used to represent "not a page".
*/
تعريفنوع u32 Pgno;

/*
** Each open file is managed by a separate instance of the "Pager" structure.
*/
تعريفنوع بنية Pager Pager;

/*
** Handle type for pages.
*/
تعريفنوع بنية PgHdr Db_پاڼه;

/*
** Page number PAGER_SJ_PGNO is never used in an SQLite database (it is
** reserved for working around a windows/posix incompatibility). It is
** used in the journal to signify that the remainder of the journal file 
** is devoted to storing a super-journal name - there are no more pages to
** roll back. See comments for function writeSuperJournal() in pager.c 
** for details.
*/
#تعریف_کړه PAGER_SJ_PGNO_COMPUTED(x) ((Pgno)((PENDING_BYTE/((x)->پاڼه_اندازه))+1))
#تعریف_کړه PAGER_SJ_PGNO(x)          ((x)->lckPgno)

/*
** Allowed values for the flags parameter to sqlite3PagerOpen().
**
** NOTE: These values must match the corresponding BTREE_ values in btree.h.
*/
#تعریف_کړه PAGER_OMIT_JOURNAL  0x0001    /* Do not use a rollback journal */
#تعریف_کړه PAGER_حافظه        0x0002    /* In-memory database */

/*
** Valid values for the second argument to sqlite3PagerLockingMode().
*/
#تعریف_کړه PAGER_LOCKINGMODE_QUERY      -1
#تعریف_کړه PAGER_LOCKINGMODE_نورمال      0
#تعریف_کړه PAGER_LOCKINGMODE_EXCLUSIVE   1

/*
** Numeric constants that encode the journalmode.
**
** The numeric values encoded here (other than PAGER_JOURNALMODE_QUERY)
** are exposed in the API via the "PRAGMA journal_mode" command and
** therefore cannot be changed without a compatibility break.
*/
#تعریف_کړه PAGER_JOURNALMODE_QUERY     (-1)  /* Query the value of journalmode */
#تعریف_کړه PAGER_JOURNALMODE_DELETE      0   /* Commit by deleting journal file */
#تعریف_کړه PAGER_JOURNALMODE_PERSIST     1   /* Commit by zeroing journal header */
#تعریف_کړه PAGER_JOURNALMODE_OFF         2   /* Journal omitted.  */
#تعریف_کړه PAGER_JOURNALMODE_TRUNCATE    3   /* Commit by truncating journal */
#تعریف_کړه PAGER_JOURNALMODE_حافظه      4   /* In-memory journal file */
#تعریف_کړه PAGER_JOURNALMODE_WAL         5   /* Use write-ahead logging */

#تعریف_کړه is_Wal_ډېرپېښ(x) ((x)==PAGER_JOURNALMODE_WAL)

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
#تعریف_کړه isOpen(pFd) ((pFd)->pMethods!=0)

/*
** Flags that make up the mask passed to sqlite3PagerGet().
*/
#تعریف_کړه PAGER_GET_NOCONTENT     0x01  /* Do not load data from disk */
#تعریف_کړه PAGER_GET_READONLY      0x02  /* Read-only page is acceptable */

/*
** Flags for sqlite3PagerSetFlags()
**
** Value constraints (enforced via assert()):
**    PAGER_FULLFSYNC      == SQLITE_FullFSync
**    PAGER_CKPT_FULLFSYNC == SQLITE_CkptFullFSync
**    PAGER_CACHE_SPILL    == SQLITE_CacheSpill
*/
#تعریف_کړه PAGER_SYNCHRONOUS_OFF       0x01  /* PRAGMA synchronous=OFF */
#تعریف_کړه PAGER_SYNCHRONOUS_نورمال    0x02  /* PRAGMA synchronous=NORMAL */
#تعریف_کړه PAGER_SYNCHRONOUS_ډک      0x03  /* PRAGMA synchronous=FULL */
#تعریف_کړه PAGER_SYNCHRONOUS_EXTRA     0x04  /* PRAGMA synchronous=EXTRA */
#تعریف_کړه PAGER_SYNCHRONOUS_ماسک      0x07  /* Mask for four values above */
#تعریف_کړه PAGER_FULLFSYNC             0x08  /* PRAGMA fullfsync=ON */
#تعریف_کړه PAGER_CKPT_FULLFSYNC        0x10  /* PRAGMA checkpoint_fullfsync=ON */
#تعریف_کړه PAGER_CACHESPILL            0x20  /* PRAGMA cache_spill=ON */
#تعریف_کړه PAGER_FLAGS_ماسک            0x38  /* All above except SYNCHRONOUS */

/*
** The remainder of this file contains the declarations of the functions
** that make up the Pager sub-system API. See source code comments for 
** a detailed description of each routine.
*/

/* Open and close a Pager connection. */ 
صحيح sqlite3PagerOpen(
  sqlite3_vfs*,
  Pager **ppPager,
  ثابت حرف*,
  صحيح,
  صحيح,
  صحيح,
  فراغ(*)(Db_پاڼه*)
);
صحيح sqlite3PagerClose(Pager *pPager, sqlite3*);
صحيح sqlite3PagerReadFileheader(Pager*, صحيح, بینښه حرف*);

/* Functions used to configure a Pager object. */
فراغ sqlite3_Pager_سېټ_Busy_Handler(Pager*, صحيح(*)(فراغ *), فراغ *);
صحيح sqlite3_Pager_سېټ_Pagesize(Pager*, u32*, صحيح);
Pgno sqlite3_Pager_Max_پاڼه_شمېر(Pager*, Pgno);
فراغ sqlite3_Pager_سېټ_Cachesize(Pager*, صحيح);
صحيح sqlite3_Pager_سېټ_Spillsize(Pager*, صحيح);
فراغ sqlite3_Pager_سېټ_Mmap_حد(Pager *, sqlite3_int64);
فراغ sqlite3PagerShrink(Pager*);
فراغ sqlite3_Pager_سېټ_Flags(Pager*,بینښه);
صحيح sqlite3_Pager_Locking_ډېرپېښ(Pager *, صحيح);
صحيح sqlite3_Pager_سېټ_Journal_ډېرپېښ(Pager *, صحيح);
صحيح sqlite3_Pager_Get_Journal_ډېرپېښ(Pager*);
صحيح sqlite3_Pager_Ok_منزل_Change_Journal_ډېرپېښ(Pager*);
i64 sqlite3_Pager_Journal_اندازه_حد(Pager *, i64);
sqlite3_backup **sqlite3PagerBackupPtr(Pager*);
صحيح sqlite3PagerFlush(Pager*);

/* Functions used to obtain and release page references. */ 
صحيح sqlite3PagerGet(Pager *pPager, Pgno pgno, Db_پاڼه **pp_پاڼه, صحيح clrFlag);
Db_پاڼه *sqlite3PagerLookup(Pager *pPager, Pgno pgno);
فراغ sqlite3PagerRef(Db_پاڼه*);
فراغ sqlite3PagerUnref(Db_پاڼه*);
فراغ sqlite3PagerUnrefNotNull(Db_پاڼه*);
فراغ sqlite3_Pager_Unref_پاڼه_One(Db_پاڼه*);

/* Operations on page references. */
صحيح sqlite3PagerWrite(Db_پاڼه*);
فراغ sqlite3PagerDontWrite(Db_پاڼه*);
صحيح sqlite3PagerMovepage(Pager*,Db_پاڼه*,Pgno,صحيح);
صحيح sqlite3_Pager_پاڼه_Refcount(Db_پاڼه*);
فراغ *sqlite3_Pager_Get_معلومات(Db_پاڼه *); 
فراغ *sqlite3PagerGetExtra(Db_پاڼه *); 

/* Functions used to manage pager transactions and savepoints. */
فراغ sqlite3PagerPagecount(Pager*, صحيح*);
صحيح sqlite3PagerBegin(Pager*, صحيح exFlag, صحيح);
صحيح sqlite3PagerCommitPhaseOne(Pager*,ثابت حرف *zSuper, صحيح);
صحيح sqlite3_Pager_Exclusive_قلف(Pager*);
صحيح sqlite3PagerSync(Pager *pPager, ثابت حرف *zSuper);
صحيح sqlite3PagerCommitPhaseTwo(Pager*);
صحيح sqlite3PagerRollback(Pager*);
صحيح sqlite3PagerOpenSavepoint(Pager *pPager, صحيح n);
صحيح sqlite3PagerSavepoint(Pager *pPager, صحيح op, صحيح iSavepoint);
صحيح sqlite3_Pager_Shared_قلف(Pager *pPager);

#که_نه_تعریف_شوی SQLITE_OMIT_WAL
  صحيح sqlite3PagerCheckpoint(Pager *pPager, sqlite3*, صحيح, صحيح*, صحيح*);
  صحيح sqlite3PagerWalSupported(Pager *pPager);
  صحيح sqlite3PagerWalCallback(Pager *pPager);
  صحيح sqlite3PagerOpenWal(Pager *pPager, صحيح *pisOpen);
  صحيح sqlite3PagerCloseWal(Pager *pPager, sqlite3*);
# ifdef SQLITE_ENABLE_SNAPSHOT
  صحيح sqlite3PagerSnapshotGet(Pager*, sqlite3_snapshot **ppSnapshot);
  صحيح sqlite3PagerSnapshotOpen(Pager*, sqlite3_snapshot *pSnapshot);
  صحيح sqlite3PagerSnapshotRecover(Pager *pPager);
  صحيح sqlite3PagerSnapshotCheck(Pager *pPager, sqlite3_snapshot *pSnapshot);
  فراغ sqlite3PagerSnapshotUnlock(Pager *pPager);
# endif
#د_که_پای

#إذا !defined(SQLITE_OMIT_WAL) && defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  صحيح sqlite3_Pager_Wal_Write_قلف(Pager*, صحيح);
  فراغ sqlite3PagerWalDb(Pager*, sqlite3*);
#وإلا
# define sqlite3_Pager_Wal_Write_قلف(y,z) SQLITE_OK
# define sqlite3PagerWalDb(x,y)
#د_که_پای

#که_تعریف_شوی SQLITE_مستقیم_ډکېدنه_READ
  صحيح sqlite3_Pager_مستقیم_Read_Ok(Pager *pPager, Pgno pgno);
#د_که_پای

#که_تعریف_شوی SQLITE_ENABLE_ZIPVFS
  صحيح sqlite3PagerWalFramesize(Pager *pPager);
#د_که_پای

/* Functions used to query pager state and configuration. */
u8 sqlite3PagerIsreadonly(Pager*);
u32 sqlite3_Pager_معلومات_نسخه(Pager*);
#که_تعریف_شوی SQLITE_DEBUG
  صحيح sqlite3PagerRefcount(Pager*);
#د_که_پای
صحيح sqlite3_Pager_Mem_کارول_شوی(Pager*);
ثابت حرف *sqlite3PagerFilename(ثابت Pager*, صحيح);
sqlite3_vfs *sqlite3PagerVfs(Pager*);
sqlite3_file *sqlite3PagerFile(Pager*);
sqlite3_file *sqlite3PagerJrnlFile(Pager*);
ثابت حرف *sqlite3PagerJournalname(Pager*);
فراغ *sqlite3_Pager_لنډمهاله_Space(Pager*);
صحيح sqlite3PagerIsMemdb(Pager*);
فراغ sqlite3PagerCacheStat(Pager *, صحيح, صحيح, u64*);
فراغ sqlite3PagerClearCache(Pager*);
صحيح sqlite3_Sector_اندازه(sqlite3_file *);

/* Functions used to truncate the database file. */
فراغ sqlite3PagerTruncateImage(Pager*,Pgno);

فراغ sqlite3PagerRekey(Db_پاڼه*, Pgno, u16);

/* Functions to support testing and debugging. */
#إذا !defined(NDEBUG) || defined(SQLITE_TEST)
  Pgno sqlite3PagerPagenumber(Db_پاڼه*);
  صحيح sqlite3PagerIswriteable(Db_پاڼه*);
#د_که_پای
#که_تعریف_شوی SQLITE_TEST
  صحيح *sqlite3PagerStats(Pager*);
  فراغ sqlite3PagerRefdump(Pager*);
  فراغ disable_simulated_io_errors(فراغ);
  فراغ enable_simulated_io_errors(فراغ);
#وإلا
# define disable_simulated_io_errors()
# define enable_simulated_io_errors()
#د_که_پای

#إذا defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
صحيح sqlite3PagerWalSystemErrno(Pager*);
#د_که_پای

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  فراغ sqlite3PagerWalStat(Pager*,sqlite3_str*);
#د_که_پای

#د_که_پای /* SQLITE_PAGER_H */
