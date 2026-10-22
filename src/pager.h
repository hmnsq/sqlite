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

#اگر_غیر_تعریف_شدہ SQLITE_PAGER_H
#تعریف_کرو SQLITE_PAGER_H

/*
** Default maximum size for persistent journal files. A negative 
** value means no limit. This value may be overridden using the 
** sqlite3PagerJournalSizeLimit() API. See also "PRAGMA journal_size_limit".
*/
#اگر_غیر_تعریف_شدہ SQLITE_DEFAULT_JOURNAL_سائز_حد_بندی
  #تعریف_کرو SQLITE_DEFAULT_JOURNAL_سائز_حد_بندی -1
#اگر_ختم

/*
** The type used to represent a page number.  The first page in a file
** is called page 1.  0 is used to represent "not a page".
*/
قسمتعریف u32 Pgno;

/*
** Each open file is managed by a separate instance of the "Pager" structure.
*/
قسمتعریف ساخت Pager Pager;

/*
** Handle type for pages.
*/
قسمتعریف ساخت PgHdr Db_صفحہ;

/*
** Page number PAGER_SJ_PGNO is never used in an SQLite database (it is
** reserved for working around a windows/posix incompatibility). It is
** used in the journal to signify that the remainder of the journal file 
** is devoted to storing a super-journal name - there are no more pages to
** roll back. See comments for function writeSuperJournal() in pager.c 
** for details.
*/
#تعریف_کرو PAGER_SJ_PGNO_COMPUTED(x) ((Pgno)((PENDING_BYTE/((x)->صفحہ_سائز))+1))
#تعریف_کرو PAGER_SJ_PGNO(x)          ((x)->lckPgno)

/*
** Allowed values for the flags parameter to sqlite3PagerOpen().
**
** NOTE: These values must match the corresponding BTREE_ values in btree.h.
*/
#تعریف_کرو PAGER_OMIT_JOURNAL  0x0001    /* Do not use a rollback journal */
#تعریف_کرو PAGER_حافظہ        0x0002    /* In-memory database */

/*
** Valid values for the second argument to sqlite3PagerLockingMode().
*/
#تعریف_کرو PAGER_LOCKINGMODE_QUERY      -1
#تعریف_کرو PAGER_LOCKINGMODE_معمول      0
#تعریف_کرو PAGER_LOCKINGMODE_EXCLUSIVE   1

/*
** Numeric constants that encode the journalmode.
**
** The numeric values encoded here (other than PAGER_JOURNALMODE_QUERY)
** are exposed in the API via the "PRAGMA journal_mode" command and
** therefore cannot be changed without a compatibility break.
*/
#تعریف_کرو PAGER_JOURNALMODE_QUERY     (-1)  /* Query the value of journalmode */
#تعریف_کرو PAGER_JOURNALMODE_DELETE      0   /* Commit by deleting journal file */
#تعریف_کرو PAGER_JOURNALMODE_PERSIST     1   /* Commit by zeroing journal header */
#تعریف_کرو PAGER_JOURNALMODE_OFF         2   /* Journal omitted.  */
#تعریف_کرو PAGER_JOURNALMODE_TRUNCATE    3   /* Commit by truncating journal */
#تعریف_کرو PAGER_JOURNALMODE_حافظہ      4   /* In-memory journal file */
#تعریف_کرو PAGER_JOURNALMODE_WAL         5   /* Use write-ahead logging */

#تعریف_کرو is_Wal_منوال(x) ((x)==PAGER_JOURNALMODE_WAL)

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
#تعریف_کرو isOpen(pFd) ((pFd)->pMethods!=0)

/*
** Flags that make up the mask passed to sqlite3PagerGet().
*/
#تعریف_کرو PAGER_GET_NOCONTENT     0x01  /* Do not load data from disk */
#تعریف_کرو PAGER_GET_READONLY      0x02  /* Read-only page is acceptable */

/*
** Flags for sqlite3PagerSetFlags()
**
** Value constraints (enforced via assert()):
**    PAGER_FULLFSYNC      == SQLITE_FullFSync
**    PAGER_CKPT_FULLFSYNC == SQLITE_CkptFullFSync
**    PAGER_CACHE_SPILL    == SQLITE_CacheSpill
*/
#تعریف_کرو PAGER_SYNCHRONOUS_OFF       0x01  /* PRAGMA synchronous=OFF */
#تعریف_کرو PAGER_SYNCHRONOUS_معمول    0x02  /* PRAGMA synchronous=NORMAL */
#تعریف_کرو PAGER_SYNCHRONOUS_بھرا      0x03  /* PRAGMA synchronous=FULL */
#تعریف_کرو PAGER_SYNCHRONOUS_EXTRA     0x04  /* PRAGMA synchronous=EXTRA */
#تعریف_کرو PAGER_SYNCHRONOUS_نقاب      0x07  /* Mask for four values above */
#تعریف_کرو PAGER_FULLFSYNC             0x08  /* PRAGMA fullfsync=ON */
#تعریف_کرو PAGER_CKPT_FULLFSYNC        0x10  /* PRAGMA checkpoint_fullfsync=ON */
#تعریف_کرو PAGER_CACHESPILL            0x20  /* PRAGMA cache_spill=ON */
#تعریف_کرو PAGER_FLAGS_نقاب            0x38  /* All above except SYNCHRONOUS */

/*
** The remainder of this file contains the declarations of the functions
** that make up the Pager sub-system API. See source code comments for 
** a detailed description of each routine.
*/

/* Open and close a Pager connection. */ 
صحیح sqlite3PagerOpen(
  sqlite3_vfs*,
  Pager **ppPager,
  ثابت حرف*,
  صحیح,
  صحیح,
  صحیح,
  خالی(*)(Db_صفحہ*)
);
صحیح sqlite3PagerClose(Pager *pPager, sqlite3*);
صحیح sqlite3PagerReadFileheader(Pager*, صحیح, غیرعلامتی حرف*);

/* Functions used to configure a Pager object. */
خالی sqlite3_Pager_سیٹ_Busy_Handler(Pager*, صحیح(*)(خالی *), خالی *);
صحیح sqlite3_Pager_سیٹ_Pagesize(Pager*, u32*, صحیح);
Pgno sqlite3_Pager_Max_صفحہ_گنتی(Pager*, Pgno);
خالی sqlite3_Pager_سیٹ_Cachesize(Pager*, صحیح);
صحیح sqlite3_Pager_سیٹ_Spillsize(Pager*, صحیح);
خالی sqlite3_Pager_سیٹ_Mmap_حد_بندی(Pager *, sqlite3_int64);
خالی sqlite3PagerShrink(Pager*);
خالی sqlite3_Pager_سیٹ_Flags(Pager*,غیرعلامتی);
صحیح sqlite3_Pager_Locking_منوال(Pager *, صحیح);
صحیح sqlite3_Pager_سیٹ_Journal_منوال(Pager *, صحیح);
صحیح sqlite3_Pager_Get_Journal_منوال(Pager*);
صحیح sqlite3_Pager_Ok_مقصد_Change_Journal_منوال(Pager*);
i64 sqlite3_Pager_Journal_سائز_حد_بندی(Pager *, i64);
sqlite3_backup **sqlite3PagerBackupPtr(Pager*);
صحیح sqlite3PagerFlush(Pager*);

/* Functions used to obtain and release page references. */ 
صحیح sqlite3PagerGet(Pager *pPager, Pgno pgno, Db_صفحہ **pp_صفحہ, صحیح clrFlag);
Db_صفحہ *sqlite3PagerLookup(Pager *pPager, Pgno pgno);
خالی sqlite3PagerRef(Db_صفحہ*);
خالی sqlite3PagerUnref(Db_صفحہ*);
خالی sqlite3PagerUnrefNotNull(Db_صفحہ*);
خالی sqlite3_Pager_Unref_صفحہ_One(Db_صفحہ*);

/* Operations on page references. */
صحیح sqlite3PagerWrite(Db_صفحہ*);
خالی sqlite3PagerDontWrite(Db_صفحہ*);
صحیح sqlite3PagerMovepage(Pager*,Db_صفحہ*,Pgno,صحیح);
صحیح sqlite3_Pager_صفحہ_Refcount(Db_صفحہ*);
خالی *sqlite3_Pager_Get_ڈیٹا(Db_صفحہ *); 
خالی *sqlite3PagerGetExtra(Db_صفحہ *); 

/* Functions used to manage pager transactions and savepoints. */
خالی sqlite3PagerPagecount(Pager*, صحیح*);
صحیح sqlite3PagerBegin(Pager*, صحیح exFlag, صحیح);
صحیح sqlite3PagerCommitPhaseOne(Pager*,ثابت حرف *zSuper, صحیح);
صحیح sqlite3_Pager_Exclusive_تالا(Pager*);
صحیح sqlite3PagerSync(Pager *pPager, ثابت حرف *zSuper);
صحیح sqlite3PagerCommitPhaseTwo(Pager*);
صحیح sqlite3PagerRollback(Pager*);
صحیح sqlite3PagerOpenSavepoint(Pager *pPager, صحیح n);
صحیح sqlite3PagerSavepoint(Pager *pPager, صحیح op, صحیح iSavepoint);
صحیح sqlite3_Pager_Shared_تالا(Pager *pPager);

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
  صحیح sqlite3PagerCheckpoint(Pager *pPager, sqlite3*, صحیح, صحیح*, صحیح*);
  صحیح sqlite3PagerWalSupported(Pager *pPager);
  صحیح sqlite3PagerWalCallback(Pager *pPager);
  صحیح sqlite3PagerOpenWal(Pager *pPager, صحیح *pisOpen);
  صحیح sqlite3PagerCloseWal(Pager *pPager, sqlite3*);
# ifdef SQLITE_ENABLE_SNAPSHOT
  صحیح sqlite3PagerSnapshotGet(Pager*, sqlite3_snapshot **ppSnapshot);
  صحیح sqlite3PagerSnapshotOpen(Pager*, sqlite3_snapshot *pSnapshot);
  صحیح sqlite3PagerSnapshotRecover(Pager *pPager);
  صحیح sqlite3PagerSnapshotCheck(Pager *pPager, sqlite3_snapshot *pSnapshot);
  خالی sqlite3PagerSnapshotUnlock(Pager *pPager);
# endif
#اگر_ختم

#اگر !defined(SQLITE_OMIT_WAL) && defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  صحیح sqlite3_Pager_Wal_Write_تالا(Pager*, صحیح);
  خالی sqlite3PagerWalDb(Pager*, sqlite3*);
#ورنہ
# define sqlite3_Pager_Wal_Write_تالا(y,z) SQLITE_OK
# define sqlite3PagerWalDb(x,y)
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_براہ_راست_حد_سے_تجاوز_READ
  صحیح sqlite3_Pager_براہ_راست_Read_Ok(Pager *pPager, Pgno pgno);
#اگر_ختم

#اگر_تعریف_شدہ SQLITE_ENABLE_ZIPVFS
  صحیح sqlite3PagerWalFramesize(Pager *pPager);
#اگر_ختم

/* Functions used to query pager state and configuration. */
u8 sqlite3PagerIsreadonly(Pager*);
u32 sqlite3_Pager_ڈیٹا_نسخہ(Pager*);
#اگر_تعریف_شدہ SQLITE_DEBUG
  صحیح sqlite3PagerRefcount(Pager*);
#اگر_ختم
صحیح sqlite3_Pager_Mem_مستعمل(Pager*);
ثابت حرف *sqlite3PagerFilename(ثابت Pager*, صحیح);
sqlite3_vfs *sqlite3PagerVfs(Pager*);
sqlite3_file *sqlite3PagerFile(Pager*);
sqlite3_file *sqlite3PagerJrnlFile(Pager*);
ثابت حرف *sqlite3PagerJournalname(Pager*);
خالی *sqlite3_Pager_عارضی_Space(Pager*);
صحیح sqlite3PagerIsMemdb(Pager*);
خالی sqlite3PagerCacheStat(Pager *, صحیح, صحیح, u64*);
خالی sqlite3PagerClearCache(Pager*);
صحیح sqlite3_Sector_سائز(sqlite3_file *);

/* Functions used to truncate the database file. */
خالی sqlite3PagerTruncateImage(Pager*,Pgno);

خالی sqlite3PagerRekey(Db_صفحہ*, Pgno, u16);

/* Functions to support testing and debugging. */
#اگر !defined(NDEBUG) || defined(SQLITE_TEST)
  Pgno sqlite3PagerPagenumber(Db_صفحہ*);
  صحیح sqlite3PagerIswriteable(Db_صفحہ*);
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_TEST
  صحیح *sqlite3PagerStats(Pager*);
  خالی sqlite3PagerRefdump(Pager*);
  خالی disable_simulated_io_errors(خالی);
  خالی enable_simulated_io_errors(خالی);
#ورنہ
# define disable_simulated_io_errors()
# define enable_simulated_io_errors()
#اگر_ختم

#اگر defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
صحیح sqlite3PagerWalSystemErrno(Pager*);
#اگر_ختم

#اگر defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  خالی sqlite3PagerWalStat(Pager*,sqlite3_str*);
#اگر_ختم

#اگر_ختم /* SQLITE_PAGER_H */
