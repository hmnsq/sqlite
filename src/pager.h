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

#جيڪڏهن_وصف_نه_ٿيل SQLITE_PAGER_H
#وصف_ڪريو SQLITE_PAGER_H

/*
** Default maximum size for persistent journal files. A negative 
** value means no limit. This value may be overridden using the 
** sqlite3PagerJournalSizeLimit() API. See also "PRAGMA journal_size_limit".
*/
#جيڪڏهن_وصف_نه_ٿيل SQLITE_DEFAULT_JOURNAL_سائيز_حد_بندي
  #وصف_ڪريو SQLITE_DEFAULT_JOURNAL_سائيز_حد_بندي -1
#جيڪڏهن_پورو

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
تعريفنوع بنية PgHdr Db_صفحو;

/*
** Page number PAGER_SJ_PGNO is never used in an SQLite database (it is
** reserved for working around a windows/posix incompatibility). It is
** used in the journal to signify that the remainder of the journal file 
** is devoted to storing a super-journal name - there are no more pages to
** roll back. See comments for function writeSuperJournal() in pager.c 
** for details.
*/
#وصف_ڪريو PAGER_SJ_PGNO_COMPUTED(x) ((Pgno)((PENDING_BYTE/((x)->صفحو_سائيز))+1))
#وصف_ڪريو PAGER_SJ_PGNO(x)          ((x)->lckPgno)

/*
** Allowed values for the flags parameter to sqlite3PagerOpen().
**
** NOTE: These values must match the corresponding BTREE_ values in btree.h.
*/
#وصف_ڪريو PAGER_OMIT_JOURNAL  0x0001    /* Do not use a rollback journal */
#وصف_ڪريو PAGER_يادگيري        0x0002    /* In-memory database */

/*
** Valid values for the second argument to sqlite3PagerLockingMode().
*/
#وصف_ڪريو PAGER_LOCKINGMODE_QUERY      -1
#وصف_ڪريو PAGER_LOCKINGMODE_عام      0
#وصف_ڪريو PAGER_LOCKINGMODE_EXCLUSIVE   1

/*
** Numeric constants that encode the journalmode.
**
** The numeric values encoded here (other than PAGER_JOURNALMODE_QUERY)
** are exposed in the API via the "PRAGMA journal_mode" command and
** therefore cannot be changed without a compatibility break.
*/
#وصف_ڪريو PAGER_JOURNALMODE_QUERY     (-1)  /* Query the value of journalmode */
#وصف_ڪريو PAGER_JOURNALMODE_DELETE      0   /* Commit by deleting journal file */
#وصف_ڪريو PAGER_JOURNALMODE_PERSIST     1   /* Commit by zeroing journal header */
#وصف_ڪريو PAGER_JOURNALMODE_OFF         2   /* Journal omitted.  */
#وصف_ڪريو PAGER_JOURNALMODE_TRUNCATE    3   /* Commit by truncating journal */
#وصف_ڪريو PAGER_JOURNALMODE_يادگيري      4   /* In-memory journal file */
#وصف_ڪريو PAGER_JOURNALMODE_WAL         5   /* Use write-ahead logging */

#وصف_ڪريو is_Wal_گهڻائي(x) ((x)==PAGER_JOURNALMODE_WAL)

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
#وصف_ڪريو isOpen(pFd) ((pFd)->pMethods!=0)

/*
** Flags that make up the mask passed to sqlite3PagerGet().
*/
#وصف_ڪريو PAGER_GET_NOCONTENT     0x01  /* Do not load data from disk */
#وصف_ڪريو PAGER_GET_READONLY      0x02  /* Read-only page is acceptable */

/*
** Flags for sqlite3PagerSetFlags()
**
** Value constraints (enforced via assert()):
**    PAGER_FULLFSYNC      == SQLITE_FullFSync
**    PAGER_CKPT_FULLFSYNC == SQLITE_CkptFullFSync
**    PAGER_CACHE_SPILL    == SQLITE_CacheSpill
*/
#وصف_ڪريو PAGER_SYNCHRONOUS_OFF       0x01  /* PRAGMA synchronous=OFF */
#وصف_ڪريو PAGER_SYNCHRONOUS_عام    0x02  /* PRAGMA synchronous=NORMAL */
#وصف_ڪريو PAGER_SYNCHRONOUS_ڀريل      0x03  /* PRAGMA synchronous=FULL */
#وصف_ڪريو PAGER_SYNCHRONOUS_EXTRA     0x04  /* PRAGMA synchronous=EXTRA */
#وصف_ڪريو PAGER_SYNCHRONOUS_نقاب      0x07  /* Mask for four values above */
#وصف_ڪريو PAGER_FULLFSYNC             0x08  /* PRAGMA fullfsync=ON */
#وصف_ڪريو PAGER_CKPT_FULLFSYNC        0x10  /* PRAGMA checkpoint_fullfsync=ON */
#وصف_ڪريو PAGER_CACHESPILL            0x20  /* PRAGMA cache_spill=ON */
#وصف_ڪريو PAGER_FLAGS_نقاب            0x38  /* All above except SYNCHRONOUS */

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
  فراغ(*)(Db_صفحو*)
);
صحيح sqlite3PagerClose(Pager *pPager, sqlite3*);
صحيح sqlite3PagerReadFileheader(Pager*, صحيح, بنانشان حرف*);

/* Functions used to configure a Pager object. */
فراغ sqlite3_Pager_سيٽ_Busy_Handler(Pager*, صحيح(*)(فراغ *), فراغ *);
صحيح sqlite3_Pager_سيٽ_Pagesize(Pager*, u32*, صحيح);
Pgno sqlite3_Pager_Max_صفحو_ڳڻپ(Pager*, Pgno);
فراغ sqlite3_Pager_سيٽ_Cachesize(Pager*, صحيح);
صحيح sqlite3_Pager_سيٽ_Spillsize(Pager*, صحيح);
فراغ sqlite3_Pager_سيٽ_Mmap_حد_بندي(Pager *, sqlite3_int64);
فراغ sqlite3PagerShrink(Pager*);
فراغ sqlite3_Pager_سيٽ_Flags(Pager*,بنانشان);
صحيح sqlite3_Pager_Locking_گهڻائي(Pager *, صحيح);
صحيح sqlite3_Pager_سيٽ_Journal_گهڻائي(Pager *, صحيح);
صحيح sqlite3_Pager_Get_Journal_گهڻائي(Pager*);
صحيح sqlite3_Pager_Ok_منزل_Change_Journal_گهڻائي(Pager*);
i64 sqlite3_Pager_Journal_سائيز_حد_بندي(Pager *, i64);
sqlite3_backup **sqlite3PagerBackupPtr(Pager*);
صحيح sqlite3PagerFlush(Pager*);

/* Functions used to obtain and release page references. */ 
صحيح sqlite3PagerGet(Pager *pPager, Pgno pgno, Db_صفحو **pp_صفحو, صحيح clrFlag);
Db_صفحو *sqlite3PagerLookup(Pager *pPager, Pgno pgno);
فراغ sqlite3PagerRef(Db_صفحو*);
فراغ sqlite3PagerUnref(Db_صفحو*);
فراغ sqlite3PagerUnrefNotNull(Db_صفحو*);
فراغ sqlite3_Pager_Unref_صفحو_One(Db_صفحو*);

/* Operations on page references. */
صحيح sqlite3PagerWrite(Db_صفحو*);
فراغ sqlite3PagerDontWrite(Db_صفحو*);
صحيح sqlite3PagerMovepage(Pager*,Db_صفحو*,Pgno,صحيح);
صحيح sqlite3_Pager_صفحو_Refcount(Db_صفحو*);
فراغ *sqlite3_Pager_Get_معلومات(Db_صفحو *); 
فراغ *sqlite3PagerGetExtra(Db_صفحو *); 

/* Functions used to manage pager transactions and savepoints. */
فراغ sqlite3PagerPagecount(Pager*, صحيح*);
صحيح sqlite3PagerBegin(Pager*, صحيح exFlag, صحيح);
صحيح sqlite3PagerCommitPhaseOne(Pager*,ثابت حرف *zSuper, صحيح);
صحيح sqlite3_Pager_Exclusive_تالو(Pager*);
صحيح sqlite3PagerSync(Pager *pPager, ثابت حرف *zSuper);
صحيح sqlite3PagerCommitPhaseTwo(Pager*);
صحيح sqlite3PagerRollback(Pager*);
صحيح sqlite3PagerOpenSavepoint(Pager *pPager, صحيح n);
صحيح sqlite3PagerSavepoint(Pager *pPager, صحيح op, صحيح iSavepoint);
صحيح sqlite3_Pager_Shared_تالو(Pager *pPager);

#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_WAL
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
#جيڪڏهن_پورو

#إذا !defined(SQLITE_OMIT_WAL) && defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  صحيح sqlite3_Pager_Wal_Write_تالو(Pager*, صحيح);
  فراغ sqlite3PagerWalDb(Pager*, sqlite3*);
#وإلا
# define sqlite3_Pager_Wal_Write_تالو(y,z) SQLITE_OK
# define sqlite3PagerWalDb(x,y)
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل SQLITE_سڌو_حد_کان_مٿي_READ
  صحيح sqlite3_Pager_سڌو_Read_Ok(Pager *pPager, Pgno pgno);
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_ZIPVFS
  صحيح sqlite3PagerWalFramesize(Pager *pPager);
#جيڪڏهن_پورو

/* Functions used to query pager state and configuration. */
u8 sqlite3PagerIsreadonly(Pager*);
u32 sqlite3_Pager_معلومات_نسخو(Pager*);
#جيڪڏهن_وصف_ٿيل SQLITE_DEBUG
  صحيح sqlite3PagerRefcount(Pager*);
#جيڪڏهن_پورو
صحيح sqlite3_Pager_Mem_استعمال_ٿيل(Pager*);
ثابت حرف *sqlite3PagerFilename(ثابت Pager*, صحيح);
sqlite3_vfs *sqlite3PagerVfs(Pager*);
sqlite3_file *sqlite3PagerFile(Pager*);
sqlite3_file *sqlite3PagerJrnlFile(Pager*);
ثابت حرف *sqlite3PagerJournalname(Pager*);
فراغ *sqlite3_Pager_عارضي_Space(Pager*);
صحيح sqlite3PagerIsMemdb(Pager*);
فراغ sqlite3PagerCacheStat(Pager *, صحيح, صحيح, u64*);
فراغ sqlite3PagerClearCache(Pager*);
صحيح sqlite3_Sector_سائيز(sqlite3_file *);

/* Functions used to truncate the database file. */
فراغ sqlite3PagerTruncateImage(Pager*,Pgno);

فراغ sqlite3PagerRekey(Db_صفحو*, Pgno, u16);

/* Functions to support testing and debugging. */
#إذا !defined(NDEBUG) || defined(SQLITE_TEST)
  Pgno sqlite3PagerPagenumber(Db_صفحو*);
  صحيح sqlite3PagerIswriteable(Db_صفحو*);
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل SQLITE_TEST
  صحيح *sqlite3PagerStats(Pager*);
  فراغ sqlite3PagerRefdump(Pager*);
  فراغ disable_simulated_io_errors(فراغ);
  فراغ enable_simulated_io_errors(فراغ);
#وإلا
# define disable_simulated_io_errors()
# define enable_simulated_io_errors()
#جيڪڏهن_پورو

#إذا defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
صحيح sqlite3PagerWalSystemErrno(Pager*);
#جيڪڏهن_پورو

#إذا defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  فراغ sqlite3PagerWalStat(Pager*,sqlite3_str*);
#جيڪڏهن_پورو

#جيڪڏهن_پورو /* SQLITE_PAGER_H */
