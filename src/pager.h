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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_PAGER_H
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_PAGER_H

/*
** Default maximum size for persistent journal files. A negative 
** value means no limit. This value may be overridden using the 
** sqlite3PagerJournalSizeLimit() API. See also "PRAGMA journal_size_limit".
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEFAULT_JOURNAL_ಗಾತ್ರ_ಎಲ್ಲೆ
  #ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_DEFAULT_JOURNAL_ಗಾತ್ರ_ಎಲ್ಲೆ -1
#ಆದರೆ_ಅಂತ್ಯ

/*
** The type used to represent a page number.  The first page in a file
** is called page 1.  0 is used to represent "not a page".
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ u32 Pgno;

/*
** Each open file is managed by a separate instance of the "Pager" structure.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Pager Pager;

/*
** Handle type for pages.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ PgHdr Db_ಪುಟ;

/*
** Page number PAGER_SJ_PGNO is never used in an SQLite database (it is
** reserved for working around a windows/posix incompatibility). It is
** used in the journal to signify that the remainder of the journal file 
** is devoted to storing a super-journal name - there are no more pages to
** roll back. See comments for function writeSuperJournal() in pager.c 
** for details.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_SJ_PGNO_COMPUTED(x) ((Pgno)((PENDING_BYTE/((x)->ಪುಟ_ಗಾತ್ರ))+1))
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_SJ_PGNO(x)          ((x)->lckPgno)

/*
** Allowed values for the flags parameter to sqlite3PagerOpen().
**
** NOTE: These values must match the corresponding BTREE_ values in btree.h.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_OMIT_JOURNAL  0x0001    /* Do not use a rollback journal */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_ಸ್ಮರಣೆ        0x0002    /* In-memory database */

/*
** Valid values for the second argument to sqlite3PagerLockingMode().
*/
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_LOCKINGMODE_QUERY      -1
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_LOCKINGMODE_ಸಾಮಾನ್ಯ      0
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_LOCKINGMODE_EXCLUSIVE   1

/*
** Numeric constants that encode the journalmode.
**
** The numeric values encoded here (other than PAGER_JOURNALMODE_QUERY)
** are exposed in the API via the "PRAGMA journal_mode" command and
** therefore cannot be changed without a compatibility break.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_JOURNALMODE_QUERY     (-1)  /* Query the value of journalmode */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_JOURNALMODE_DELETE      0   /* Commit by deleting journal file */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_JOURNALMODE_PERSIST     1   /* Commit by zeroing journal header */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_JOURNALMODE_OFF         2   /* Journal omitted.  */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_JOURNALMODE_TRUNCATE    3   /* Commit by truncating journal */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_JOURNALMODE_ಸ್ಮರಣೆ      4   /* In-memory journal file */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_JOURNALMODE_WAL         5   /* Use write-ahead logging */

#ವ್ಯಾಖ್ಯಾನಿಸು is_Wal_ಬಹುಲಕ(x) ((x)==PAGER_JOURNALMODE_WAL)

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
#ವ್ಯಾಖ್ಯಾನಿಸು isOpen(pFd) ((pFd)->pMethods!=0)

/*
** Flags that make up the mask passed to sqlite3PagerGet().
*/
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_GET_NOCONTENT     0x01  /* Do not load data from disk */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_GET_READONLY      0x02  /* Read-only page is acceptable */

/*
** Flags for sqlite3PagerSetFlags()
**
** Value constraints (enforced via assert()):
**    PAGER_FULLFSYNC      == SQLITE_FullFSync
**    PAGER_CKPT_FULLFSYNC == SQLITE_CkptFullFSync
**    PAGER_CACHE_SPILL    == SQLITE_CacheSpill
*/
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_SYNCHRONOUS_OFF       0x01  /* PRAGMA synchronous=OFF */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_SYNCHRONOUS_ಸಾಮಾನ್ಯ    0x02  /* PRAGMA synchronous=NORMAL */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_SYNCHRONOUS_ತುಂಬಿದ      0x03  /* PRAGMA synchronous=FULL */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_SYNCHRONOUS_EXTRA     0x04  /* PRAGMA synchronous=EXTRA */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_SYNCHRONOUS_ಮುಖವಾಡ      0x07  /* Mask for four values above */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_FULLFSYNC             0x08  /* PRAGMA fullfsync=ON */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_CKPT_FULLFSYNC        0x10  /* PRAGMA checkpoint_fullfsync=ON */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_CACHESPILL            0x20  /* PRAGMA cache_spill=ON */
#ವ್ಯಾಖ್ಯಾನಿಸು PAGER_FLAGS_ಮುಖವಾಡ            0x38  /* All above except SYNCHRONOUS */

/*
** The remainder of this file contains the declarations of the functions
** that make up the Pager sub-system API. See source code comments for 
** a detailed description of each routine.
*/

/* Open and close a Pager connection. */ 
ಪೂರ್ಣಾಂಕ sqlite3PagerOpen(
  sqlite3_vfs*,
  Pager **ppPager,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*,
  ಪೂರ್ಣಾಂಕ,
  ಪೂರ್ಣಾಂಕ,
  ಪೂರ್ಣಾಂಕ,
  ಶೂನ್ಯ(*)(Db_ಪುಟ*)
);
ಪೂರ್ಣಾಂಕ sqlite3PagerClose(Pager *pPager, sqlite3*);
ಪೂರ್ಣಾಂಕ sqlite3PagerReadFileheader(Pager*, ಪೂರ್ಣಾಂಕ, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ*);

/* Functions used to configure a Pager object. */
ಶೂನ್ಯ sqlite3_Pager_ಗಣ_Busy_Handler(Pager*, ಪೂರ್ಣಾಂಕ(*)(ಶೂನ್ಯ *), ಶೂನ್ಯ *);
ಪೂರ್ಣಾಂಕ sqlite3_Pager_ಗಣ_Pagesize(Pager*, u32*, ಪೂರ್ಣಾಂಕ);
Pgno sqlite3_Pager_Max_ಪುಟ_ಎಣಿಕೆ(Pager*, Pgno);
ಶೂನ್ಯ sqlite3_Pager_ಗಣ_Cachesize(Pager*, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_Pager_ಗಣ_Spillsize(Pager*, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3_Pager_ಗಣ_Mmap_ಎಲ್ಲೆ(Pager *, sqlite3_int64);
ಶೂನ್ಯ sqlite3PagerShrink(Pager*);
ಶೂನ್ಯ sqlite3_Pager_ಗಣ_Flags(Pager*,ಚಿಹ್ನೆರಹಿತ);
ಪೂರ್ಣಾಂಕ sqlite3_Pager_Locking_ಬಹುಲಕ(Pager *, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_Pager_ಗಣ_Journal_ಬಹುಲಕ(Pager *, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_Pager_Get_Journal_ಬಹುಲಕ(Pager*);
ಪೂರ್ಣಾಂಕ sqlite3_Pager_Ok_ಗಮ್ಯ_Change_Journal_ಬಹುಲಕ(Pager*);
i64 sqlite3_Pager_Journal_ಗಾತ್ರ_ಎಲ್ಲೆ(Pager *, i64);
sqlite3_backup **sqlite3PagerBackupPtr(Pager*);
ಪೂರ್ಣಾಂಕ sqlite3PagerFlush(Pager*);

/* Functions used to obtain and release page references. */ 
ಪೂರ್ಣಾಂಕ sqlite3PagerGet(Pager *pPager, Pgno pgno, Db_ಪುಟ **pp_ಪುಟ, ಪೂರ್ಣಾಂಕ clrFlag);
Db_ಪುಟ *sqlite3PagerLookup(Pager *pPager, Pgno pgno);
ಶೂನ್ಯ sqlite3PagerRef(Db_ಪುಟ*);
ಶೂನ್ಯ sqlite3PagerUnref(Db_ಪುಟ*);
ಶೂನ್ಯ sqlite3PagerUnrefNotNull(Db_ಪುಟ*);
ಶೂನ್ಯ sqlite3_Pager_Unref_ಪುಟ_One(Db_ಪುಟ*);

/* Operations on page references. */
ಪೂರ್ಣಾಂಕ sqlite3PagerWrite(Db_ಪುಟ*);
ಶೂನ್ಯ sqlite3PagerDontWrite(Db_ಪುಟ*);
ಪೂರ್ಣಾಂಕ sqlite3PagerMovepage(Pager*,Db_ಪುಟ*,Pgno,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_Pager_ಪುಟ_Refcount(Db_ಪುಟ*);
ಶೂನ್ಯ *sqlite3_Pager_Get_ದತ್ತಾಂಶ(Db_ಪುಟ *); 
ಶೂನ್ಯ *sqlite3PagerGetExtra(Db_ಪುಟ *); 

/* Functions used to manage pager transactions and savepoints. */
ಶೂನ್ಯ sqlite3PagerPagecount(Pager*, ಪೂರ್ಣಾಂಕ*);
ಪೂರ್ಣಾಂಕ sqlite3PagerBegin(Pager*, ಪೂರ್ಣಾಂಕ exFlag, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3PagerCommitPhaseOne(Pager*,ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSuper, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_Pager_Exclusive_ಬೀಗ(Pager*);
ಪೂರ್ಣಾಂಕ sqlite3PagerSync(Pager *pPager, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSuper);
ಪೂರ್ಣಾಂಕ sqlite3PagerCommitPhaseTwo(Pager*);
ಪೂರ್ಣಾಂಕ sqlite3PagerRollback(Pager*);
ಪೂರ್ಣಾಂಕ sqlite3PagerOpenSavepoint(Pager *pPager, ಪೂರ್ಣಾಂಕ n);
ಪೂರ್ಣಾಂಕ sqlite3PagerSavepoint(Pager *pPager, ಪೂರ್ಣಾಂಕ op, ಪೂರ್ಣಾಂಕ iSavepoint);
ಪೂರ್ಣಾಂಕ sqlite3_Pager_Shared_ಬೀಗ(Pager *pPager);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  ಪೂರ್ಣಾಂಕ sqlite3PagerCheckpoint(Pager *pPager, sqlite3*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ*, ಪೂರ್ಣಾಂಕ*);
  ಪೂರ್ಣಾಂಕ sqlite3PagerWalSupported(Pager *pPager);
  ಪೂರ್ಣಾಂಕ sqlite3PagerWalCallback(Pager *pPager);
  ಪೂರ್ಣಾಂಕ sqlite3PagerOpenWal(Pager *pPager, ಪೂರ್ಣಾಂಕ *pisOpen);
  ಪೂರ್ಣಾಂಕ sqlite3PagerCloseWal(Pager *pPager, sqlite3*);
# ifdef SQLITE_ENABLE_SNAPSHOT
  ಪೂರ್ಣಾಂಕ sqlite3PagerSnapshotGet(Pager*, sqlite3_snapshot **ppSnapshot);
  ಪೂರ್ಣಾಂಕ sqlite3PagerSnapshotOpen(Pager*, sqlite3_snapshot *pSnapshot);
  ಪೂರ್ಣಾಂಕ sqlite3PagerSnapshotRecover(Pager *pPager);
  ಪೂರ್ಣಾಂಕ sqlite3PagerSnapshotCheck(Pager *pPager, sqlite3_snapshot *pSnapshot);
  ಶೂನ್ಯ sqlite3PagerSnapshotUnlock(Pager *pPager);
# endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_WAL) && defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  ಪೂರ್ಣಾಂಕ sqlite3_Pager_Wal_Write_ಬೀಗ(Pager*, ಪೂರ್ಣಾಂಕ);
  ಶೂನ್ಯ sqlite3PagerWalDb(Pager*, sqlite3*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_Pager_Wal_Write_ಬೀಗ(y,z) SQLITE_OK
# define sqlite3PagerWalDb(x,y)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ನೇರ_ಅತಿಪ್ರವಾಹ_READ
  ಪೂರ್ಣಾಂಕ sqlite3_Pager_ನೇರ_Read_Ok(Pager *pPager, Pgno pgno);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ZIPVFS
  ಪೂರ್ಣಾಂಕ sqlite3PagerWalFramesize(Pager *pPager);
#ಆದರೆ_ಅಂತ್ಯ

/* Functions used to query pager state and configuration. */
u8 sqlite3PagerIsreadonly(Pager*);
u32 sqlite3_Pager_ದತ್ತಾಂಶ_ಆವೃತ್ತಿ(Pager*);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  ಪೂರ್ಣಾಂಕ sqlite3PagerRefcount(Pager*);
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3_Pager_Mem_ಬಳಸಿದ(Pager*);
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3PagerFilename(ಸ್ಥಿರಾಂಕ Pager*, ಪೂರ್ಣಾಂಕ);
sqlite3_vfs *sqlite3PagerVfs(Pager*);
sqlite3_file *sqlite3PagerFile(Pager*);
sqlite3_file *sqlite3PagerJrnlFile(Pager*);
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3PagerJournalname(Pager*);
ಶೂನ್ಯ *sqlite3_Pager_ತಾತ್ಕಾಲಿಕ_Space(Pager*);
ಪೂರ್ಣಾಂಕ sqlite3PagerIsMemdb(Pager*);
ಶೂನ್ಯ sqlite3PagerCacheStat(Pager *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, u64*);
ಶೂನ್ಯ sqlite3PagerClearCache(Pager*);
ಪೂರ್ಣಾಂಕ sqlite3_Sector_ಗಾತ್ರ(sqlite3_file *);

/* Functions used to truncate the database file. */
ಶೂನ್ಯ sqlite3PagerTruncateImage(Pager*,Pgno);

ಶೂನ್ಯ sqlite3PagerRekey(Db_ಪುಟ*, Pgno, u16);

/* Functions to support testing and debugging. */
#ಆದರೆ !defined(NDEBUG) || defined(SQLITE_TEST)
  Pgno sqlite3PagerPagenumber(Db_ಪುಟ*);
  ಪೂರ್ಣಾಂಕ sqlite3PagerIswriteable(Db_ಪುಟ*);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
  ಪೂರ್ಣಾಂಕ *sqlite3PagerStats(Pager*);
  ಶೂನ್ಯ sqlite3PagerRefdump(Pager*);
  ಶೂನ್ಯ disable_simulated_io_errors(ಶೂನ್ಯ);
  ಶೂನ್ಯ enable_simulated_io_errors(ಶೂನ್ಯ);
#ಇಲ್ಲದಿದ್ದರೆ
# define disable_simulated_io_errors()
# define enable_simulated_io_errors()
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
ಪೂರ್ಣಾಂಕ sqlite3PagerWalSystemErrno(Pager*);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  ಶೂನ್ಯ sqlite3PagerWalStat(Pager*,sqlite3_str*);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_PAGER_H */
