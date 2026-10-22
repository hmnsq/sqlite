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

#यदि_अपरिभाषितम् SQLITE_PAGER_H
#परिभाषय SQLITE_PAGER_H

/*
** Default maximum size for persistent journal files. A negative 
** value means no limit. This value may be overridden using the 
** sqlite3PagerJournalSizeLimit() API. See also "PRAGMA journal_size_limit".
*/
#यदि_अपरिभाषितम् SQLITE_DEFAULT_JOURNAL_आकार_सीमा
  #परिभाषय SQLITE_DEFAULT_JOURNAL_आकार_सीमा -1
#यदि_अन्तः

/*
** The type used to represent a page number.  The first page in a file
** is called page 1.  0 is used to represent "not a page".
*/
प्रकारनिर्देश u32 Pgno;

/*
** Each open file is managed by a separate instance of the "Pager" structure.
*/
प्रकारनिर्देश संरचना Pager Pager;

/*
** Handle type for pages.
*/
प्रकारनिर्देश संरचना PgHdr DbPage;

/*
** Page number PAGER_SJ_PGNO is never used in an SQLite database (it is
** reserved for working around a windows/posix incompatibility). It is
** used in the journal to signify that the remainder of the journal file 
** is devoted to storing a super-journal name - there are no more pages to
** roll back. See comments for function writeSuperJournal() in pager.c 
** for details.
*/
#परिभाषय PAGER_SJ_PGNO_COMPUTED(x) ((Pgno)((PENDING_BYTE/((x)->page_आकार))+1))
#परिभाषय PAGER_SJ_PGNO(x)          ((x)->lckPgno)

/*
** Allowed values for the flags parameter to sqlite3PagerOpen().
**
** NOTE: These values must match the corresponding BTREE_ values in btree.h.
*/
#परिभाषय PAGER_OMIT_JOURNAL  0x0001    /* Do not use a rollback journal */
#परिभाषय PAGER_स्मृतिः        0x0002    /* In-memory database */

/*
** Valid values for the second argument to sqlite3PagerLockingMode().
*/
#परिभाषय PAGER_LOCKINGMODE_QUERY      -1
#परिभाषय PAGER_LOCKINGMODE_प्रसामान्यम्      0
#परिभाषय PAGER_LOCKINGMODE_EXCLUSIVE   1

/*
** Numeric constants that encode the journalmode.
**
** The numeric values encoded here (other than PAGER_JOURNALMODE_QUERY)
** are exposed in the API via the "PRAGMA journal_mode" command and
** therefore cannot be changed without a compatibility break.
*/
#परिभाषय PAGER_JOURNALMODE_QUERY     (-1)  /* Query the value of journalmode */
#परिभाषय PAGER_JOURNALMODE_DELETE      0   /* Commit by deleting journal file */
#परिभाषय PAGER_JOURNALMODE_PERSIST     1   /* Commit by zeroing journal header */
#परिभाषय PAGER_JOURNALMODE_OFF         2   /* Journal omitted.  */
#परिभाषय PAGER_JOURNALMODE_TRUNCATE    3   /* Commit by truncating journal */
#परिभाषय PAGER_JOURNALMODE_स्मृतिः      4   /* In-memory journal file */
#परिभाषय PAGER_JOURNALMODE_WAL         5   /* Use write-ahead logging */

#परिभाषय is_Wal_बहुलकम्(x) ((x)==PAGER_JOURNALMODE_WAL)

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
#परिभाषय isOpen(pFd) ((pFd)->pMethods!=0)

/*
** Flags that make up the mask passed to sqlite3PagerGet().
*/
#परिभाषय PAGER_GET_NOCONTENT     0x01  /* Do not load data from disk */
#परिभाषय PAGER_GET_READONLY      0x02  /* Read-only page is acceptable */

/*
** Flags for sqlite3PagerSetFlags()
**
** Value constraints (enforced via assert()):
**    PAGER_FULLFSYNC      == SQLITE_FullFSync
**    PAGER_CKPT_FULLFSYNC == SQLITE_CkptFullFSync
**    PAGER_CACHE_SPILL    == SQLITE_CacheSpill
*/
#परिभाषय PAGER_SYNCHRONOUS_OFF       0x01  /* PRAGMA synchronous=OFF */
#परिभाषय PAGER_SYNCHRONOUS_प्रसामान्यम्    0x02  /* PRAGMA synchronous=NORMAL */
#परिभाषय PAGER_SYNCHRONOUS_पूर्णम्      0x03  /* PRAGMA synchronous=FULL */
#परिभाषय PAGER_SYNCHRONOUS_EXTRA     0x04  /* PRAGMA synchronous=EXTRA */
#परिभाषय PAGER_SYNCHRONOUS_आवरणम्      0x07  /* Mask for four values above */
#परिभाषय PAGER_FULLFSYNC             0x08  /* PRAGMA fullfsync=ON */
#परिभाषय PAGER_CKPT_FULLFSYNC        0x10  /* PRAGMA checkpoint_fullfsync=ON */
#परिभाषय PAGER_CACHESPILL            0x20  /* PRAGMA cache_spill=ON */
#परिभाषय PAGER_FLAGS_आवरणम्            0x38  /* All above except SYNCHRONOUS */

/*
** The remainder of this file contains the declarations of the functions
** that make up the Pager sub-system API. See source code comments for 
** a detailed description of each routine.
*/

/* Open and close a Pager connection. */ 
पूर्णाङ्क sqlite3PagerOpen(
  sqlite3_vfs*,
  Pager **ppPager,
  ध्रुव वर्ण*,
  पूर्णाङ्क,
  पूर्णाङ्क,
  पूर्णाङ्क,
  शून्य(*)(DbPage*)
);
पूर्णाङ्क sqlite3PagerClose(Pager *pPager, sqlite3*);
पूर्णाङ्क sqlite3PagerReadFileheader(Pager*, पूर्णाङ्क, अचिह्नित वर्ण*);

/* Functions used to configure a Pager object. */
शून्य sqlite3_Pager_समुच्चयः_Busy_Handler(Pager*, पूर्णाङ्क(*)(शून्य *), शून्य *);
पूर्णाङ्क sqlite3_Pager_समुच्चयः_Pagesize(Pager*, u32*, पूर्णाङ्क);
Pgno sqlite3_Pager_Max_Page_गणनम्(Pager*, Pgno);
शून्य sqlite3_Pager_समुच्चयः_Cachesize(Pager*, पूर्णाङ्क);
पूर्णाङ्क sqlite3_Pager_समुच्चयः_Spillsize(Pager*, पूर्णाङ्क);
शून्य sqlite3_Pager_समुच्चयः_Mmap_सीमा(Pager *, sqlite3_int64);
शून्य sqlite3PagerShrink(Pager*);
शून्य sqlite3_Pager_समुच्चयः_Flags(Pager*,अचिह्नित);
पूर्णाङ्क sqlite3_Pager_Locking_बहुलकम्(Pager *, पूर्णाङ्क);
पूर्णाङ्क sqlite3_Pager_समुच्चयः_Journal_बहुलकम्(Pager *, पूर्णाङ्क);
पूर्णाङ्क sqlite3_Pager_Get_Journal_बहुलकम्(Pager*);
पूर्णाङ्क sqlite3_Pager_Ok_गन्तव्यम्_Change_Journal_बहुलकम्(Pager*);
i64 sqlite3_Pager_Journal_आकार_सीमा(Pager *, i64);
sqlite3_backup **sqlite3PagerBackupPtr(Pager*);
पूर्णाङ्क sqlite3PagerFlush(Pager*);

/* Functions used to obtain and release page references. */ 
पूर्णाङ्क sqlite3PagerGet(Pager *pPager, Pgno pgno, DbPage **ppPage, पूर्णाङ्क clrFlag);
DbPage *sqlite3PagerLookup(Pager *pPager, Pgno pgno);
शून्य sqlite3PagerRef(DbPage*);
शून्य sqlite3PagerUnref(DbPage*);
शून्य sqlite3PagerUnrefNotNull(DbPage*);
शून्य sqlite3PagerUnrefPageOne(DbPage*);

/* Operations on page references. */
पूर्णाङ्क sqlite3PagerWrite(DbPage*);
शून्य sqlite3PagerDontWrite(DbPage*);
पूर्णाङ्क sqlite3PagerMovepage(Pager*,DbPage*,Pgno,पूर्णाङ्क);
पूर्णाङ्क sqlite3PagerPageRefcount(DbPage*);
शून्य *sqlite3_Pager_Get_दत्त(DbPage *); 
शून्य *sqlite3PagerGetExtra(DbPage *); 

/* Functions used to manage pager transactions and savepoints. */
शून्य sqlite3PagerPagecount(Pager*, पूर्णाङ्क*);
पूर्णाङ्क sqlite3PagerBegin(Pager*, पूर्णाङ्क exFlag, पूर्णाङ्क);
पूर्णाङ्क sqlite3PagerCommitPhaseOne(Pager*,ध्रुव वर्ण *zSuper, पूर्णाङ्क);
पूर्णाङ्क sqlite3_Pager_Exclusive_तालकम्(Pager*);
पूर्णाङ्क sqlite3PagerSync(Pager *pPager, ध्रुव वर्ण *zSuper);
पूर्णाङ्क sqlite3PagerCommitPhaseTwo(Pager*);
पूर्णाङ्क sqlite3PagerRollback(Pager*);
पूर्णाङ्क sqlite3PagerOpenSavepoint(Pager *pPager, पूर्णाङ्क n);
पूर्णाङ्क sqlite3PagerSavepoint(Pager *pPager, पूर्णाङ्क op, पूर्णाङ्क iSavepoint);
पूर्णाङ्क sqlite3_Pager_Shared_तालकम्(Pager *pPager);

#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
  पूर्णाङ्क sqlite3PagerCheckpoint(Pager *pPager, sqlite3*, पूर्णाङ्क, पूर्णाङ्क*, पूर्णाङ्क*);
  पूर्णाङ्क sqlite3PagerWalSupported(Pager *pPager);
  पूर्णाङ्क sqlite3PagerWalCallback(Pager *pPager);
  पूर्णाङ्क sqlite3PagerOpenWal(Pager *pPager, पूर्णाङ्क *pisOpen);
  पूर्णाङ्क sqlite3PagerCloseWal(Pager *pPager, sqlite3*);
# ifdef SQLITE_ENABLE_SNAPSHOT
  पूर्णाङ्क sqlite3PagerSnapshotGet(Pager*, sqlite3_snapshot **ppSnapshot);
  पूर्णाङ्क sqlite3PagerSnapshotOpen(Pager*, sqlite3_snapshot *pSnapshot);
  पूर्णाङ्क sqlite3PagerSnapshotRecover(Pager *pPager);
  पूर्णाङ्क sqlite3PagerSnapshotCheck(Pager *pPager, sqlite3_snapshot *pSnapshot);
  शून्य sqlite3PagerSnapshotUnlock(Pager *pPager);
# endif
#यदि_अन्तः

#यदि !defined(SQLITE_OMIT_WAL) && defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  पूर्णाङ्क sqlite3_Pager_Wal_Write_तालकम्(Pager*, पूर्णाङ्क);
  शून्य sqlite3PagerWalDb(Pager*, sqlite3*);
#अन्यथा
# define sqlite3_Pager_Wal_Write_तालकम्(y,z) SQLITE_OK
# define sqlite3PagerWalDb(x,y)
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_प्रत्यक्षम्_अतिप्रवाहः_READ
  पूर्णाङ्क sqlite3_Pager_प्रत्यक्षम्_Read_Ok(Pager *pPager, Pgno pgno);
#यदि_अन्तः

#यदि_परिभाषितम् SQLITE_ENABLE_ZIPVFS
  पूर्णाङ्क sqlite3PagerWalFramesize(Pager *pPager);
#यदि_अन्तः

/* Functions used to query pager state and configuration. */
u8 sqlite3PagerIsreadonly(Pager*);
u32 sqlite3_Pager_दत्त_संस्करणम्(Pager*);
#यदि_परिभाषितम् SQLITE_DEBUG
  पूर्णाङ्क sqlite3PagerRefcount(Pager*);
#यदि_अन्तः
पूर्णाङ्क sqlite3_Pager_Mem_प्रयुक्तम्(Pager*);
ध्रुव वर्ण *sqlite3PagerFilename(ध्रुव Pager*, पूर्णाङ्क);
sqlite3_vfs *sqlite3PagerVfs(Pager*);
sqlite3_file *sqlite3PagerFile(Pager*);
sqlite3_file *sqlite3PagerJrnlFile(Pager*);
ध्रुव वर्ण *sqlite3PagerJournalname(Pager*);
शून्य *sqlite3_Pager_अस्थायी_Space(Pager*);
पूर्णाङ्क sqlite3PagerIsMemdb(Pager*);
शून्य sqlite3PagerCacheStat(Pager *, पूर्णाङ्क, पूर्णाङ्क, u64*);
शून्य sqlite3PagerClearCache(Pager*);
पूर्णाङ्क sqlite3_Sector_आकार(sqlite3_file *);

/* Functions used to truncate the database file. */
शून्य sqlite3PagerTruncateImage(Pager*,Pgno);

शून्य sqlite3PagerRekey(DbPage*, Pgno, u16);

/* Functions to support testing and debugging. */
#यदि !defined(NDEBUG) || defined(SQLITE_TEST)
  Pgno sqlite3PagerPagenumber(DbPage*);
  पूर्णाङ्क sqlite3PagerIswriteable(DbPage*);
#यदि_अन्तः
#यदि_परिभाषितम् SQLITE_TEST
  पूर्णाङ्क *sqlite3PagerStats(Pager*);
  शून्य sqlite3PagerRefdump(Pager*);
  शून्य disable_simulated_io_errors(शून्य);
  शून्य enable_simulated_io_errors(शून्य);
#अन्यथा
# define disable_simulated_io_errors()
# define enable_simulated_io_errors()
#यदि_अन्तः

#यदि defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
पूर्णाङ्क sqlite3PagerWalSystemErrno(Pager*);
#यदि_अन्तः

#यदि defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  शून्य sqlite3PagerWalStat(Pager*,sqlite3_str*);
#यदि_अन्तः

#यदि_अन्तः /* SQLITE_PAGER_H */
