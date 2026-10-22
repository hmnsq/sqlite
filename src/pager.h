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

#నిర్వచితం_కాకపోతే SQLITE_PAGER_H
#నిర్వచించు SQLITE_PAGER_H

/*
** Default maximum size for persistent journal files. A negative 
** value means no limit. This value may be overridden using the 
** sqlite3PagerJournalSizeLimit() API. See also "PRAGMA journal_size_limit".
*/
#నిర్వచితం_కాకపోతే SQLITE_DEFAULT_JOURNAL_కొలత_హద్దు
  #నిర్వచించు SQLITE_DEFAULT_JOURNAL_కొలత_హద్దు -1
#అయితే_ముగింపు

/*
** The type used to represent a page number.  The first page in a file
** is called page 1.  0 is used to represent "not a page".
*/
రకనిర్వచనం u32 Pgno;

/*
** Each open file is managed by a separate instance of the "Pager" structure.
*/
రకనిర్వచనం నిర్మాణం Pager Pager;

/*
** Handle type for pages.
*/
రకనిర్వచనం నిర్మాణం PgHdr Db_పుట;

/*
** Page number PAGER_SJ_PGNO is never used in an SQLite database (it is
** reserved for working around a windows/posix incompatibility). It is
** used in the journal to signify that the remainder of the journal file 
** is devoted to storing a super-journal name - there are no more pages to
** roll back. See comments for function writeSuperJournal() in pager.c 
** for details.
*/
#నిర్వచించు PAGER_SJ_PGNO_COMPUTED(x) ((Pgno)((PENDING_BYTE/((x)->పుట_కొలత))+1))
#నిర్వచించు PAGER_SJ_PGNO(x)          ((x)->lckPgno)

/*
** Allowed values for the flags parameter to sqlite3PagerOpen().
**
** NOTE: These values must match the corresponding BTREE_ values in btree.h.
*/
#నిర్వచించు PAGER_OMIT_JOURNAL  0x0001    /* Do not use a rollback journal */
#నిర్వచించు PAGER_జ్ఞాపకం        0x0002    /* In-memory database */

/*
** Valid values for the second argument to sqlite3PagerLockingMode().
*/
#నిర్వచించు PAGER_LOCKINGMODE_QUERY      -1
#నిర్వచించు PAGER_LOCKINGMODE_సాధారణ      0
#నిర్వచించు PAGER_LOCKINGMODE_EXCLUSIVE   1

/*
** Numeric constants that encode the journalmode.
**
** The numeric values encoded here (other than PAGER_JOURNALMODE_QUERY)
** are exposed in the API via the "PRAGMA journal_mode" command and
** therefore cannot be changed without a compatibility break.
*/
#నిర్వచించు PAGER_JOURNALMODE_QUERY     (-1)  /* Query the value of journalmode */
#నిర్వచించు PAGER_JOURNALMODE_DELETE      0   /* Commit by deleting journal file */
#నిర్వచించు PAGER_JOURNALMODE_PERSIST     1   /* Commit by zeroing journal header */
#నిర్వచించు PAGER_JOURNALMODE_OFF         2   /* Journal omitted.  */
#నిర్వచించు PAGER_JOURNALMODE_TRUNCATE    3   /* Commit by truncating journal */
#నిర్వచించు PAGER_JOURNALMODE_జ్ఞాపకం      4   /* In-memory journal file */
#నిర్వచించు PAGER_JOURNALMODE_WAL         5   /* Use write-ahead logging */

#నిర్వచించు is_Wal_బహుళకం(x) ((x)==PAGER_JOURNALMODE_WAL)

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
#నిర్వచించు isOpen(pFd) ((pFd)->pMethods!=0)

/*
** Flags that make up the mask passed to sqlite3PagerGet().
*/
#నిర్వచించు PAGER_GET_NOCONTENT     0x01  /* Do not load data from disk */
#నిర్వచించు PAGER_GET_READONLY      0x02  /* Read-only page is acceptable */

/*
** Flags for sqlite3PagerSetFlags()
**
** Value constraints (enforced via assert()):
**    PAGER_FULLFSYNC      == SQLITE_FullFSync
**    PAGER_CKPT_FULLFSYNC == SQLITE_CkptFullFSync
**    PAGER_CACHE_SPILL    == SQLITE_CacheSpill
*/
#నిర్వచించు PAGER_SYNCHRONOUS_OFF       0x01  /* PRAGMA synchronous=OFF */
#నిర్వచించు PAGER_SYNCHRONOUS_సాధారణ    0x02  /* PRAGMA synchronous=NORMAL */
#నిర్వచించు PAGER_SYNCHRONOUS_నిండిన      0x03  /* PRAGMA synchronous=FULL */
#నిర్వచించు PAGER_SYNCHRONOUS_EXTRA     0x04  /* PRAGMA synchronous=EXTRA */
#నిర్వచించు PAGER_SYNCHRONOUS_ముసుగు      0x07  /* Mask for four values above */
#నిర్వచించు PAGER_FULLFSYNC             0x08  /* PRAGMA fullfsync=ON */
#నిర్వచించు PAGER_CKPT_FULLFSYNC        0x10  /* PRAGMA checkpoint_fullfsync=ON */
#నిర్వచించు PAGER_CACHESPILL            0x20  /* PRAGMA cache_spill=ON */
#నిర్వచించు PAGER_FLAGS_ముసుగు            0x38  /* All above except SYNCHRONOUS */

/*
** The remainder of this file contains the declarations of the functions
** that make up the Pager sub-system API. See source code comments for 
** a detailed description of each routine.
*/

/* Open and close a Pager connection. */ 
పూర్ణాంకం sqlite3PagerOpen(
  sqlite3_vfs*,
  Pager **ppPager,
  స్థిరరాశి అక్షరం*,
  పూర్ణాంకం,
  పూర్ణాంకం,
  పూర్ణాంకం,
  శూన్య(*)(Db_పుట*)
);
పూర్ణాంకం sqlite3PagerClose(Pager *pPager, sqlite3*);
పూర్ణాంకం sqlite3PagerReadFileheader(Pager*, పూర్ణాంకం, గుర్తులేని అక్షరం*);

/* Functions used to configure a Pager object. */
శూన్య sqlite3_Pager_సమితి_Busy_Handler(Pager*, పూర్ణాంకం(*)(శూన్య *), శూన్య *);
పూర్ణాంకం sqlite3_Pager_సమితి_Pagesize(Pager*, u32*, పూర్ణాంకం);
Pgno sqlite3_Pager_Max_పుట_లెక్క(Pager*, Pgno);
శూన్య sqlite3_Pager_సమితి_Cachesize(Pager*, పూర్ణాంకం);
పూర్ణాంకం sqlite3_Pager_సమితి_Spillsize(Pager*, పూర్ణాంకం);
శూన్య sqlite3_Pager_సమితి_Mmap_హద్దు(Pager *, sqlite3_int64);
శూన్య sqlite3PagerShrink(Pager*);
శూన్య sqlite3_Pager_సమితి_Flags(Pager*,గుర్తులేని);
పూర్ణాంకం sqlite3_Pager_Locking_బహుళకం(Pager *, పూర్ణాంకం);
పూర్ణాంకం sqlite3_Pager_సమితి_Journal_బహుళకం(Pager *, పూర్ణాంకం);
పూర్ణాంకం sqlite3_Pager_Get_Journal_బహుళకం(Pager*);
పూర్ణాంకం sqlite3_Pager_Ok_గమ్యం_Change_Journal_బహుళకం(Pager*);
i64 sqlite3_Pager_Journal_కొలత_హద్దు(Pager *, i64);
sqlite3_backup **sqlite3PagerBackupPtr(Pager*);
పూర్ణాంకం sqlite3PagerFlush(Pager*);

/* Functions used to obtain and release page references. */ 
పూర్ణాంకం sqlite3PagerGet(Pager *pPager, Pgno pgno, Db_పుట **pp_పుట, పూర్ణాంకం clrFlag);
Db_పుట *sqlite3PagerLookup(Pager *pPager, Pgno pgno);
శూన్య sqlite3PagerRef(Db_పుట*);
శూన్య sqlite3PagerUnref(Db_పుట*);
శూన్య sqlite3PagerUnrefNotNull(Db_పుట*);
శూన్య sqlite3_Pager_Unref_పుట_One(Db_పుట*);

/* Operations on page references. */
పూర్ణాంకం sqlite3PagerWrite(Db_పుట*);
శూన్య sqlite3PagerDontWrite(Db_పుట*);
పూర్ణాంకం sqlite3PagerMovepage(Pager*,Db_పుట*,Pgno,పూర్ణాంకం);
పూర్ణాంకం sqlite3_Pager_పుట_Refcount(Db_పుట*);
శూన్య *sqlite3_Pager_Get_దత్తాంశం(Db_పుట *); 
శూన్య *sqlite3PagerGetExtra(Db_పుట *); 

/* Functions used to manage pager transactions and savepoints. */
శూన్య sqlite3PagerPagecount(Pager*, పూర్ణాంకం*);
పూర్ణాంకం sqlite3PagerBegin(Pager*, పూర్ణాంకం exFlag, పూర్ణాంకం);
పూర్ణాంకం sqlite3PagerCommitPhaseOne(Pager*,స్థిరరాశి అక్షరం *zSuper, పూర్ణాంకం);
పూర్ణాంకం sqlite3_Pager_Exclusive_తాళం(Pager*);
పూర్ణాంకం sqlite3PagerSync(Pager *pPager, స్థిరరాశి అక్షరం *zSuper);
పూర్ణాంకం sqlite3PagerCommitPhaseTwo(Pager*);
పూర్ణాంకం sqlite3PagerRollback(Pager*);
పూర్ణాంకం sqlite3PagerOpenSavepoint(Pager *pPager, పూర్ణాంకం n);
పూర్ణాంకం sqlite3PagerSavepoint(Pager *pPager, పూర్ణాంకం op, పూర్ణాంకం iSavepoint);
పూర్ణాంకం sqlite3_Pager_Shared_తాళం(Pager *pPager);

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL
  పూర్ణాంకం sqlite3PagerCheckpoint(Pager *pPager, sqlite3*, పూర్ణాంకం, పూర్ణాంకం*, పూర్ణాంకం*);
  పూర్ణాంకం sqlite3PagerWalSupported(Pager *pPager);
  పూర్ణాంకం sqlite3PagerWalCallback(Pager *pPager);
  పూర్ణాంకం sqlite3PagerOpenWal(Pager *pPager, పూర్ణాంకం *pisOpen);
  పూర్ణాంకం sqlite3PagerCloseWal(Pager *pPager, sqlite3*);
# ifdef SQLITE_ENABLE_SNAPSHOT
  పూర్ణాంకం sqlite3PagerSnapshotGet(Pager*, sqlite3_snapshot **ppSnapshot);
  పూర్ణాంకం sqlite3PagerSnapshotOpen(Pager*, sqlite3_snapshot *pSnapshot);
  పూర్ణాంకం sqlite3PagerSnapshotRecover(Pager *pPager);
  పూర్ణాంకం sqlite3PagerSnapshotCheck(Pager *pPager, sqlite3_snapshot *pSnapshot);
  శూన్య sqlite3PagerSnapshotUnlock(Pager *pPager);
# endif
#అయితే_ముగింపు

#ఒకవేళ !defined(SQLITE_OMIT_WAL) && defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  పూర్ణాంకం sqlite3_Pager_Wal_Write_తాళం(Pager*, పూర్ణాంకం);
  శూన్య sqlite3PagerWalDb(Pager*, sqlite3*);
#లేకపోతే
# define sqlite3_Pager_Wal_Write_తాళం(y,z) SQLITE_OK
# define sqlite3PagerWalDb(x,y)
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ప్రత్యక్ష_అధికప్రవాహం_READ
  పూర్ణాంకం sqlite3_Pager_ప్రత్యక్ష_Read_Ok(Pager *pPager, Pgno pgno);
#అయితే_ముగింపు

#నిర్వచితమైతే SQLITE_ENABLE_ZIPVFS
  పూర్ణాంకం sqlite3PagerWalFramesize(Pager *pPager);
#అయితే_ముగింపు

/* Functions used to query pager state and configuration. */
u8 sqlite3PagerIsreadonly(Pager*);
u32 sqlite3_Pager_దత్తాంశం_సంస్కరణ(Pager*);
#నిర్వచితమైతే SQLITE_DEBUG
  పూర్ణాంకం sqlite3PagerRefcount(Pager*);
#అయితే_ముగింపు
పూర్ణాంకం sqlite3_Pager_Mem_వాడిన(Pager*);
స్థిరరాశి అక్షరం *sqlite3PagerFilename(స్థిరరాశి Pager*, పూర్ణాంకం);
sqlite3_vfs *sqlite3PagerVfs(Pager*);
sqlite3_file *sqlite3PagerFile(Pager*);
sqlite3_file *sqlite3PagerJrnlFile(Pager*);
స్థిరరాశి అక్షరం *sqlite3PagerJournalname(Pager*);
శూన్య *sqlite3_Pager_తాత్కాలిక_Space(Pager*);
పూర్ణాంకం sqlite3PagerIsMemdb(Pager*);
శూన్య sqlite3PagerCacheStat(Pager *, పూర్ణాంకం, పూర్ణాంకం, u64*);
శూన్య sqlite3PagerClearCache(Pager*);
పూర్ణాంకం sqlite3_Sector_కొలత(sqlite3_file *);

/* Functions used to truncate the database file. */
శూన్య sqlite3PagerTruncateImage(Pager*,Pgno);

శూన్య sqlite3PagerRekey(Db_పుట*, Pgno, u16);

/* Functions to support testing and debugging. */
#ఒకవేళ !defined(NDEBUG) || defined(SQLITE_TEST)
  Pgno sqlite3PagerPagenumber(Db_పుట*);
  పూర్ణాంకం sqlite3PagerIswriteable(Db_పుట*);
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_TEST
  పూర్ణాంకం *sqlite3PagerStats(Pager*);
  శూన్య sqlite3PagerRefdump(Pager*);
  శూన్య disable_simulated_io_errors(శూన్య);
  శూన్య enable_simulated_io_errors(శూన్య);
#లేకపోతే
# define disable_simulated_io_errors()
# define enable_simulated_io_errors()
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
పూర్ణాంకం sqlite3PagerWalSystemErrno(Pager*);
#అయితే_ముగింపు

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  శూన్య sqlite3PagerWalStat(Pager*,sqlite3_str*);
#అయితే_ముగింపు

#అయితే_ముగింపు /* SQLITE_PAGER_H */
