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

#अगर_अघोषित SQLITE_PAGER_H
#घोषणा SQLITE_PAGER_H

/*
** Default maximum size for persistent journal files. A negative 
** value means no limit. This value may be overridden using the 
** sqlite3PagerJournalSizeLimit() API. See also "PRAGMA journal_size_limit".
*/
#अगर_अघोषित SQLITE_DEFAULT_JOURNAL_आकार_LIMIT
  #घोषणा SQLITE_DEFAULT_JOURNAL_आकार_LIMIT -1
#पूर्ण_अगर

/*
** The type used to represent a page number.  The first page in a file
** is called page 1.  0 is used to represent "not a page".
*/
प्रकार u32 Pgno;

/*
** Each open file is managed by a separate instance of the "Pager" structure.
*/
प्रकार काष्ठा Pager Pager;

/*
** Handle type for pages.
*/
प्रकार काष्ठा PgHdr DbPage;

/*
** Page number PAGER_SJ_PGNO is never used in an SQLite database (it is
** reserved for working around a windows/posix incompatibility). It is
** used in the journal to signify that the remainder of the journal file 
** is devoted to storing a super-journal name - there are no more pages to
** roll back. See comments for function writeSuperJournal() in pager.c 
** for details.
*/
#घोषणा PAGER_SJ_PGNO_COMPUTED(x) ((Pgno)((PENDING_BYTE/((x)->page_आकार))+1))
#घोषणा PAGER_SJ_PGNO(x)          ((x)->lckPgno)

/*
** Allowed values for the flags parameter to sqlite3PagerOpen().
**
** NOTE: These values must match the corresponding BTREE_ values in btree.h.
*/
#घोषणा PAGER_OMIT_JOURNAL  0x0001    /* Do not use a rollback journal */
#घोषणा PAGER_MEMORY        0x0002    /* In-memory database */

/*
** Valid values for the second argument to sqlite3PagerLockingMode().
*/
#घोषणा PAGER_LOCKINGMODE_QUERY      -1
#घोषणा PAGER_LOCKINGMODE_NORMAL      0
#घोषणा PAGER_LOCKINGMODE_EXCLUSIVE   1

/*
** Numeric constants that encode the journalmode.
**
** The numeric values encoded here (other than PAGER_JOURNALMODE_QUERY)
** are exposed in the API via the "PRAGMA journal_mode" command and
** therefore cannot be changed without a compatibility break.
*/
#घोषणा PAGER_JOURNALMODE_QUERY     (-1)  /* Query the value of journalmode */
#घोषणा PAGER_JOURNALMODE_DELETE      0   /* Commit by deleting journal file */
#घोषणा PAGER_JOURNALMODE_PERSIST     1   /* Commit by zeroing journal header */
#घोषणा PAGER_JOURNALMODE_OFF         2   /* Journal omitted.  */
#घोषणा PAGER_JOURNALMODE_TRUNCATE    3   /* Commit by truncating journal */
#घोषणा PAGER_JOURNALMODE_MEMORY      4   /* In-memory journal file */
#घोषणा PAGER_JOURNALMODE_WAL         5   /* Use write-ahead logging */

#घोषणा isWalMode(x) ((x)==PAGER_JOURNALMODE_WAL)

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
#घोषणा isOpen(pFd) ((pFd)->pMethods!=0)

/*
** Flags that make up the mask passed to sqlite3PagerGet().
*/
#घोषणा PAGER_GET_NOCONTENT     0x01  /* Do not load data from disk */
#घोषणा PAGER_GET_READONLY      0x02  /* Read-only page is acceptable */

/*
** Flags for sqlite3PagerSetFlags()
**
** Value constraints (enforced via assert()):
**    PAGER_FULLFSYNC      == SQLITE_FullFSync
**    PAGER_CKPT_FULLFSYNC == SQLITE_CkptFullFSync
**    PAGER_CACHE_SPILL    == SQLITE_CacheSpill
*/
#घोषणा PAGER_SYNCHRONOUS_OFF       0x01  /* PRAGMA synchronous=OFF */
#घोषणा PAGER_SYNCHRONOUS_NORMAL    0x02  /* PRAGMA synchronous=NORMAL */
#घोषणा PAGER_SYNCHRONOUS_FULL      0x03  /* PRAGMA synchronous=FULL */
#घोषणा PAGER_SYNCHRONOUS_EXTRA     0x04  /* PRAGMA synchronous=EXTRA */
#घोषणा PAGER_SYNCHRONOUS_MASK      0x07  /* Mask for four values above */
#घोषणा PAGER_FULLFSYNC             0x08  /* PRAGMA fullfsync=ON */
#घोषणा PAGER_CKPT_FULLFSYNC        0x10  /* PRAGMA checkpoint_fullfsync=ON */
#घोषणा PAGER_CACHESPILL            0x20  /* PRAGMA cache_spill=ON */
#घोषणा PAGER_FLAGS_MASK            0x38  /* All above except SYNCHRONOUS */

/*
** The remainder of this file contains the declarations of the functions
** that make up the Pager sub-system API. See source code comments for 
** a detailed description of each routine.
*/

/* Open and close a Pager connection. */ 
पूर्णांक sqlite3PagerOpen(
  sqlite3_vfs*,
  Pager **ppPager,
  स्थिर अक्षर*,
  पूर्णांक,
  पूर्णांक,
  पूर्णांक,
  व्योम(*)(DbPage*)
);
पूर्णांक sqlite3PagerClose(Pager *pPager, sqlite3*);
पूर्णांक sqlite3PagerReadFileheader(Pager*, पूर्णांक, अचिन्हित अक्षर*);

/* Functions used to configure a Pager object. */
व्योम sqlite3PagerSetBusyHandler(Pager*, पूर्णांक(*)(व्योम *), व्योम *);
पूर्णांक sqlite3PagerSetPagesize(Pager*, u32*, पूर्णांक);
Pgno sqlite3_Pager_Max_Page_गिनती(Pager*, Pgno);
व्योम sqlite3PagerSetCachesize(Pager*, पूर्णांक);
पूर्णांक sqlite3PagerSetSpillsize(Pager*, पूर्णांक);
व्योम sqlite3PagerSetMmapLimit(Pager *, sqlite3_int64);
व्योम sqlite3PagerShrink(Pager*);
व्योम sqlite3PagerSetFlags(Pager*,अचिन्हित);
पूर्णांक sqlite3PagerLockingMode(Pager *, पूर्णांक);
पूर्णांक sqlite3PagerSetJournalMode(Pager *, पूर्णांक);
पूर्णांक sqlite3PagerGetJournalMode(Pager*);
पूर्णांक sqlite3PagerOkToChangeJournalMode(Pager*);
i64 sqlite3_Pager_Journal_आकार_Limit(Pager *, i64);
sqlite3_backup **sqlite3PagerBackupPtr(Pager*);
पूर्णांक sqlite3PagerFlush(Pager*);

/* Functions used to obtain and release page references. */ 
पूर्णांक sqlite3PagerGet(Pager *pPager, Pgno pgno, DbPage **ppPage, पूर्णांक clrFlag);
DbPage *sqlite3PagerLookup(Pager *pPager, Pgno pgno);
व्योम sqlite3PagerRef(DbPage*);
व्योम sqlite3PagerUnref(DbPage*);
व्योम sqlite3PagerUnrefNotNull(DbPage*);
व्योम sqlite3PagerUnrefPageOne(DbPage*);

/* Operations on page references. */
पूर्णांक sqlite3PagerWrite(DbPage*);
व्योम sqlite3PagerDontWrite(DbPage*);
पूर्णांक sqlite3PagerMovepage(Pager*,DbPage*,Pgno,पूर्णांक);
पूर्णांक sqlite3PagerPageRefcount(DbPage*);
व्योम *sqlite3_Pager_Get_तथ्य(DbPage *); 
व्योम *sqlite3PagerGetExtra(DbPage *); 

/* Functions used to manage pager transactions and savepoints. */
व्योम sqlite3PagerPagecount(Pager*, पूर्णांक*);
पूर्णांक sqlite3PagerBegin(Pager*, पूर्णांक exFlag, पूर्णांक);
पूर्णांक sqlite3PagerCommitPhaseOne(Pager*,स्थिर अक्षर *zSuper, पूर्णांक);
पूर्णांक sqlite3PagerExclusiveLock(Pager*);
पूर्णांक sqlite3PagerSync(Pager *pPager, स्थिर अक्षर *zSuper);
पूर्णांक sqlite3PagerCommitPhaseTwo(Pager*);
पूर्णांक sqlite3PagerRollback(Pager*);
पूर्णांक sqlite3PagerOpenSavepoint(Pager *pPager, पूर्णांक n);
पूर्णांक sqlite3PagerSavepoint(Pager *pPager, पूर्णांक op, पूर्णांक iSavepoint);
पूर्णांक sqlite3PagerSharedLock(Pager *pPager);

#अगर_अघोषित SQLITE_OMIT_WAL
  पूर्णांक sqlite3PagerCheckpoint(Pager *pPager, sqlite3*, पूर्णांक, पूर्णांक*, पूर्णांक*);
  पूर्णांक sqlite3PagerWalSupported(Pager *pPager);
  पूर्णांक sqlite3PagerWalCallback(Pager *pPager);
  पूर्णांक sqlite3PagerOpenWal(Pager *pPager, पूर्णांक *pisOpen);
  पूर्णांक sqlite3PagerCloseWal(Pager *pPager, sqlite3*);
# ifdef SQLITE_ENABLE_SNAPSHOT
  पूर्णांक sqlite3PagerSnapshotGet(Pager*, sqlite3_snapshot **ppSnapshot);
  पूर्णांक sqlite3PagerSnapshotOpen(Pager*, sqlite3_snapshot *pSnapshot);
  पूर्णांक sqlite3PagerSnapshotRecover(Pager *pPager);
  पूर्णांक sqlite3PagerSnapshotCheck(Pager *pPager, sqlite3_snapshot *pSnapshot);
  व्योम sqlite3PagerSnapshotUnlock(Pager *pPager);
# endif
#पूर्ण_अगर

#अगर !defined(SQLITE_OMIT_WAL) && defined(SQLITE_ENABLE_SETLK_TIMEOUT)
  पूर्णांक sqlite3PagerWalWriteLock(Pager*, पूर्णांक);
  व्योम sqlite3PagerWalDb(Pager*, sqlite3*);
#अन्यथा
# define sqlite3PagerWalWriteLock(y,z) SQLITE_OK
# define sqlite3PagerWalDb(x,y)
#पूर्ण_अगर

#अगर_घोषित SQLITE_DIRECT_OVERFLOW_READ
  पूर्णांक sqlite3PagerDirectReadOk(Pager *pPager, Pgno pgno);
#पूर्ण_अगर

#अगर_घोषित SQLITE_ENABLE_ZIPVFS
  पूर्णांक sqlite3PagerWalFramesize(Pager *pPager);
#पूर्ण_अगर

/* Functions used to query pager state and configuration. */
u8 sqlite3PagerIsreadonly(Pager*);
u32 sqlite3_Pager_तथ्य_Version(Pager*);
#अगर_घोषित SQLITE_DEBUG
  पूर्णांक sqlite3PagerRefcount(Pager*);
#पूर्ण_अगर
पूर्णांक sqlite3PagerMemUsed(Pager*);
स्थिर अक्षर *sqlite3PagerFilename(स्थिर Pager*, पूर्णांक);
sqlite3_vfs *sqlite3PagerVfs(Pager*);
sqlite3_file *sqlite3PagerFile(Pager*);
sqlite3_file *sqlite3PagerJrnlFile(Pager*);
स्थिर अक्षर *sqlite3PagerJournalname(Pager*);
व्योम *sqlite3_Pager_अस्थायी_Space(Pager*);
पूर्णांक sqlite3PagerIsMemdb(Pager*);
व्योम sqlite3PagerCacheStat(Pager *, पूर्णांक, पूर्णांक, u64*);
व्योम sqlite3PagerClearCache(Pager*);
पूर्णांक sqlite3_Sector_आकार(sqlite3_file *);

/* Functions used to truncate the database file. */
व्योम sqlite3PagerTruncateImage(Pager*,Pgno);

व्योम sqlite3PagerRekey(DbPage*, Pgno, u16);

/* Functions to support testing and debugging. */
#अगर !defined(न_संशोधन) || defined(SQLITE_TEST)
  Pgno sqlite3PagerPagenumber(DbPage*);
  पूर्णांक sqlite3PagerIswriteable(DbPage*);
#पूर्ण_अगर
#अगर_घोषित SQLITE_TEST
  पूर्णांक *sqlite3PagerStats(Pager*);
  व्योम sqlite3PagerRefdump(Pager*);
  व्योम disable_simulated_io_errors(व्योम);
  व्योम enable_simulated_io_errors(व्योम);
#अन्यथा
# define disable_simulated_io_errors()
# define enable_simulated_io_errors()
#पूर्ण_अगर

#अगर defined(SQLITE_USE_SEH) && !defined(SQLITE_OMIT_WAL)
पूर्णांक sqlite3PagerWalSystemErrno(Pager*);
#पूर्ण_अगर

#अगर defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
  व्योम sqlite3PagerWalStat(Pager*,sqlite3_str*);
#पूर्ण_अगर

#पूर्ण_अगर /* SQLITE_PAGER_H */
