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
** This header file defines the interface that the sqlite B-Tree file
** subsystem.  See comments in the source code for a detailed description
** of what each interface routine does.
*/
#ifndef SQLITE_BTREE_H
#define SQLITE_BTREE_H

/* TODO: This definition is just included so other modules compile. It
** needs to be revisited.
*/
#define SQLITE_N_BTREE_META 16

/*
** If defined as non-zero, auto-vacuum is enabled by default. Otherwise
** it must be turned on for each database using "PRAGMA auto_vacuum = 1".
*/
#ifndef SQLITE_DEFAULT_AUTOVACUUM
  #define SQLITE_DEFAULT_AUTOVACUUM 0
#endif

#define BTREE_AUTOVACUUM_NONE 0        /* Do not do auto-vacuum */
#define BTREE_AUTOVACUUM_FULL 1        /* Do full auto-vacuum */
#define BTREE_AUTOVACUUM_INCR 2        /* Incremental vacuum */

/*
** Forward declarations of structure
*/
வகைவரை கட்டமைப்பு Btree Btree;
வகைவரை கட்டமைப்பு BtCursor BtCursor;
வகைவரை கட்டமைப்பு BtShared BtShared;
வகைவரை கட்டமைப்பு BtreePayload BtreePayload;


முழு sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,       /* VFS to use with this b-tree */
  மாறிலி எழுத்து *zFilename,   /* Name of database file to open */
  sqlite3 *db,             /* Associated database connection */
  Btree **ppBtree,         /* Return open Btree* here */
  முழு flags,               /* Flags */
  முழு vfsFlags             /* Flags passed through to VFS open */
);

/* The flags parameter to sqlite3BtreeOpen can be the bitwise or of the
** following values.
**
** NOTE:  These values must match the corresponding PAGER_ values in
** pager.h.
*/
#define BTREE_OMIT_JOURNAL  1  /* Do not create or use a rollback journal */
#define BTREE_MEMORY        2  /* This is an in-memory DB */
#define BTREE_SINGLE        4  /* The file contains at most 1 b-tree */
#define BTREE_UNORDERED     8  /* Use of a hash implementation is OK */

முழு sqlite3BtreeClose(Btree*);
முழு sqlite3BtreeSetCacheSize(Btree*,முழு);
முழு sqlite3BtreeSetSpillSize(Btree*,முழு);
#எனில் SQLITE_MAX_MMAP_SIZE>0
  முழு sqlite3BtreeSetMmapLimit(Btree*,sqlite3_int64);
#endif
முழு sqlite3BtreeSetPagerFlags(Btree*,unsigned);
முழு sqlite3BtreeSetPageSize(Btree *p, முழு nPagesize, முழு nReserve, முழு eFix);
முழு sqlite3BtreeGetPageSize(Btree*);
Pgno sqlite3_Btree_Max_Page_மொத்தம்(Btree*,Pgno);
Pgno sqlite3BtreeLastPage(Btree*);
முழு sqlite3BtreeSecureDelete(Btree*,முழு);
முழு sqlite3BtreeGetRequestedReserve(Btree*);
முழு sqlite3BtreeGetReserveNoMutex(Btree *p);
முழு sqlite3BtreeSetAutoVacuum(Btree *, முழு);
முழு sqlite3BtreeGetAutoVacuum(Btree *);
முழு sqlite3BtreeBeginTrans(Btree*,முழு,முழு*);
முழு sqlite3BtreeCommitPhaseOne(Btree*, மாறிலி எழுத்து*);
முழு sqlite3BtreeCommitPhaseTwo(Btree*, முழு);
முழு sqlite3BtreeCommit(Btree*);
முழு sqlite3BtreeRollback(Btree*,முழு,முழு);
முழு sqlite3BtreeBeginStmt(Btree*,முழு);
முழு sqlite3BtreeCreateTable(Btree*, Pgno*, முழு flags);
முழு sqlite3BtreeTxnState(Btree*);
முழு sqlite3BtreeIsInBackup(Btree*);

வெறுமை *sqlite3BtreeSchema(Btree *, முழு, வெறுமை(*)(வெறுமை *));
முழு sqlite3BtreeSchemaLocked(Btree *pBtree);
#ifndef SQLITE_OMIT_SHARED_CACHE
முழு sqlite3BtreeLockTable(Btree *pBtree, முழு iTab, u8 isWriteLock);
#endif

/* Savepoints are named, nestable SQL transactions mostly implemented */ 
/* in vdbe.c and pager.c See https://sqlite.org/lang_savepoint.html */
முழு sqlite3BtreeSavepoint(Btree *, முழு, முழு);

/* "Checkpoint" only refers to WAL. See https://sqlite.org/wal.html#ckpt */
#ifndef SQLITE_OMIT_WAL
  முழு sqlite3BtreeCheckpoint(Btree*, முழு, முழு *, முழு *);  
#endif

மாறிலி எழுத்து *sqlite3BtreeGetFilename(Btree *);
மாறிலி எழுத்து *sqlite3BtreeGetJournalname(Btree *);
முழு sqlite3BtreeCopyFile(Btree*, Btree*);

முழு sqlite3BtreeIncrVacuum(Btree *);

/* The flags parameter to sqlite3BtreeCreateTable can be the bitwise OR
** of the flags shown below.
**
** Every SQLite table must have either BTREE_INTKEY or BTREE_BLOBKEY set.
** With BTREE_INTKEY, the table key is a 64-bit integer and arbitrary data
** is stored in the leaves.  (BTREE_INTKEY is used for SQL tables.)  With
** BTREE_BLOBKEY, the key is an arbitrary BLOB and no content is stored
** anywhere - the key is the content.  (BTREE_BLOBKEY is used for SQL
** indices.)
*/
#define BTREE_INTKEY     1    /* Table has only 64-bit signed integer keys */
#define BTREE_BLOBKEY    2    /* Table has keys only - no data */

முழு sqlite3BtreeDropTable(Btree*, முழு, முழு*);
முழு sqlite3BtreeClearTable(Btree*, முழு, i64*);
முழு sqlite3BtreeClearTableOfCursor(BtCursor*);
முழு sqlite3BtreeTripAllCursors(Btree*, முழு, முழு);

வெறுமை sqlite3BtreeGetMeta(Btree *pBtree, முழு idx, u32 *p_மதிப்பு);
முழு sqlite3BtreeUpdateMeta(Btree*, முழு idx, u32 மதிப்பு);

முழு sqlite3BtreeNewDb(Btree *p);

/*
** The second parameter to sqlite3BtreeGetMeta or sqlite3BtreeUpdateMeta
** should be one of the following values. The integer values are assigned 
** to constants so that the offset of the corresponding field in an
** SQLite database header may be found using the following formula:
**
**   offset = 36 + (idx * 4)
**
** For example, the free-page-count field is located at byte offset 36 of
** the database file header. The incr-vacuum-flag field is located at
** byte offset 64 (== 36+4*7).
**
** The BTREE_DATA_VERSION value is not really a value stored in the header.
** It is a read-only number computed by the pager.  But we merge it with
** the header value access routines since its access pattern is the same.
** Call it a "virtual meta value".
*/
#define BTREE_FREE_PAGE_மொத்தம்     0
#define BTREE_SCHEMA_VERSION      1
#define BTREE_FILE_FORMAT         2
#define BTREE_DEFAULT_CACHE_SIZE  3
#define BTREE_LARGEST_வேர்_PAGE   4
#define BTREE_TEXT_ENCODING       5
#define BTREE_USER_VERSION        6
#define BTREE_INCR_VACUUM         7
#define BTREE_APPLICATION_ID      8
#define BTREE_DATA_VERSION        15  /* A virtual meta-value */

/*
** Kinds of hints that can be passed into the sqlite3BtreeCursorHint()
** interface.
**
** BTREE_HINT_RANGE  (arguments: Expr*, Mem*)
**
**     The first argument is an Expr* (which is guaranteed to be constant for
**     the lifetime of the cursor) that defines constraints on which rows
**     might be fetched with this cursor.  The Expr* tree may contain
**     TK_REGISTER nodes that refer to values stored in the array of registers
**     passed as the second parameter.  In other words, if Expr.op==TK_REGISTER
**     then the value of the node is the value in Mem[pExpr.iTable].  Any
**     TK_COLUMN node in the expression tree refers to the Expr.iColumn-th
**     column of the b-tree of the cursor.  The Expr tree will not contain
**     any function calls nor subqueries nor references to b-trees other than
**     the cursor being hinted.
**
**     The design of the _RANGE hint is aid b-tree implementations that try
**     to prefetch content from remote machines - to provide those
**     implementations with limits on what needs to be prefetched and thereby
**     reduce network bandwidth.
**
** BTREE_HINT_TABLECURSOR  (arguments: BtCursor*)
**
**     This hint is invoked on a non-covering index cursor soon after it
**     is opened. The only argument is a pointer to the table cursor used to
**     obtain non-covered fields from the database.
**
** Note that BTREE_HINT_FLAGS with BTREE_BULKLOAD is the only hint used by
** standard SQLite.  The other hints are provided for extensions that use
** the SQLite parser and code generator but substitute their own storage
** engine.
*/
#define BTREE_HINT_RANGE 0       /* Range constraints on queries */
#define BTREE_HINT_TABLECURSOR 1 /* Table csr associated with this index csr */

/*
** Values that may be OR'd together to form the argument to the
** BTREE_HINT_FLAGS hint for sqlite3BtreeCursorHint():
**
** The BTREE_BULKLOAD flag is set on index cursors when the index is going
** to be filled with content that is already in sorted order.
**
** The BTREE_SEEK_EQ flag is set on cursors that will get OP_SeekGE or
** OP_SeekLE opcodes for a range search, but where the range of entries
** selected will all have the same key.  In other words, the cursor will
** be used only for equality key searches.
**
*/
#define BTREE_BULKLOAD 0x00000001  /* Used to full index in sorted order */
#define BTREE_SEEK_EQ  0x00000002  /* EQ seeks only - no range seeks */

/* 
** Flags passed as the third argument to sqlite3BtreeCursor().
**
** For read-only cursors the wrFlag argument is always zero. For read-write
** cursors it may be set to either (BTREE_WRCSR|BTREE_FORDELETE) or just
** (BTREE_WRCSR). If the BTREE_FORDELETE bit is set, then the cursor will
** only be used by SQLite for the following:
**
**   * to seek to and then delete specific entries, and/or
**
**   * to read values that will be used to create keys that other
**     BTREE_FORDELETE cursors will seek to and delete.
**
** The BTREE_FORDELETE flag is an optimization hint.  It is not used by
** by this, the native b-tree engine of SQLite, but it is available to
** alternative storage engines that might be substituted in place of this
** b-tree system.  For alternative storage engines in which a delete of
** the main table row automatically deletes corresponding index rows,
** the FORDELETE flag hint allows those alternative storage engines to
** skip a lot of work.  Namely:  FORDELETE cursors may treat all SEEK
** and DELETE operations as no-ops, and any READ operation against a
** FORDELETE cursor may return a null row: 0x01 0x00.
*/
#define BTREE_WRCSR     0x00000004     /* read-write cursor */
#define BTREE_FORDELETE 0x00000008     /* Cursor is for seek/delete only */

முழு sqlite3BtreeCursor(
  Btree*,                              /* BTree containing table to open */
  Pgno iTable,                         /* Index of root page */
  முழு wrFlag,                          /* 1 for writing.  0 for read-only */
  கட்டமைப்பு சாவி_Info*,                     /* First argument to compare function */
  BtCursor *pCursor                    /* Space to write cursor structure */
);
BtCursor *sqlite3BtreeFakeValidCursor(வெறுமை);
முழு sqlite3BtreeCursorSize(வெறுமை);
#ifdef SQLITE_DEBUG
முழு sqlite3BtreeClosesWithCursor(Btree*,BtCursor*);
#endif
வெறுமை sqlite3BtreeCursorZero(BtCursor*);
வெறுமை sqlite3BtreeCursorHintFlags(BtCursor*, unsigned);
#ifdef SQLITE_ENABLE_CURSOR_HINTS
வெறுமை sqlite3BtreeCursorHint(BtCursor*, முழு, ...);
 #ifdef SQLITE_DEBUG
 BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor*);
 #endif
#endif

முழு sqlite3BtreeCloseCursor(BtCursor*);
முழு sqlite3BtreeTableMoveto(
  BtCursor*,
  i64 int_சாவி,
  முழு bias,
  முழு *pRes
);
முழு sqlite3BtreeIndexMoveto(
  BtCursor*,
  UnpackedRecord *p_Un_சாவி,
  முழு *pRes
);
முழு sqlite3BtreeCursorHasMoved(BtCursor*);
முழு sqlite3BtreeCursorRestore(BtCursor*, முழு*);
முழு sqlite3BtreeDelete(BtCursor*, u8 flags);

/* Allowed flags for sqlite3BtreeDelete() and sqlite3BtreeInsert() */
#define BTREE_SAVEPOSITION 0x02  /* Leave cursor pointing at NEXT or PREV */
#define BTREE_AUXDELETE    0x04  /* not the primary delete operation */
#define BTREE_APPEND       0x08  /* Insert is likely an append */
#define BTREE_PREFORMAT    0x80  /* Inserted data is a preformated cell */

/* An instance of the BtreePayload object describes the content of a single
** entry in either an index or table btree.
**
** Index btrees (used for indexes and also WITHOUT ROWID tables) contain
** an arbitrary key and no data.  These btrees have pKey,nKey set to the
** key and the pData,nData,nZero fields are uninitialized.  The aMem,nMem
** fields give an array of Mem objects that are a decomposition of the key.
** The nMem field might be zero, indicating that no decomposition is available.
**
** Table btrees (used for rowid tables) contain an integer rowid used as
** the key and passed in the nKey field.  The pKey field is zero.  
** pData,nData hold the content of the new entry.  nZero extra zero bytes
** are appended to the end of the content when constructing the entry.
** The aMem,nMem fields are uninitialized for table btrees.
**
** Field usage summary:
**
**               Table BTrees                   Index Btrees
**
**   pKey        always NULL                    encoded key
**   nKey        the ROWID                      length of pKey
**   pData       data                           not used
**   aMem        not used                       decomposed key value
**   nMem        not used                       entries in aMem
**   nData       length of pData                not used
**   nZero       extra zeros after pData        not used
**
** This object is used to pass information into sqlite3BtreeInsert().  The
** same information used to be passed as five separate parameters.  But placing
** the information into this object helps to keep the interface more 
** organized and understandable, and it also helps the resulting code to
** run a little faster by using fewer registers for parameter passing.
*/
கட்டமைப்பு BtreePayload {
  மாறிலி வெறுமை *p_சாவி;       /* Key content for indexes.  NULL for tables */
  sqlite3_int64 n_சாவி;     /* Size of pKey for indexes.  PRIMARY KEY for tabs */
  மாறிலி வெறுமை *pData;      /* Data for tables. */
  sqlite3_மதிப்பு *aMem;    /* First of nMem value in the unpacked pKey */
  u16 nMem;               /* Number of aMem[] value.  Might be zero */
  முழு nData;              /* Size of pData.  0 if none. */
  முழு nZero;              /* Extra zero data appended after pData,nData */
};

முழு sqlite3_Btree_செருகு(BtCursor*, மாறிலி BtreePayload *pPayload,
                       முழு flags, முழு seekResult);
முழு sqlite3BtreeFirst(BtCursor*, முழு *pRes);
முழு sqlite3BtreeIsEmpty(BtCursor *pCur, முழு *pRes);
முழு sqlite3BtreeLast(BtCursor*, முழு *pRes);
முழு sqlite3BtreeNext(BtCursor*, முழு flags);
முழு sqlite3BtreeEof(BtCursor*);
முழு sqlite3BtreePrevious(BtCursor*, முழு flags);
i64 sqlite3_Btree_Integer_சாவி(BtCursor*);
வெறுமை sqlite3BtreeCursorPin(BtCursor*);
வெறுமை sqlite3BtreeCursorUnpin(BtCursor*);
i64 sqlite3BtreeOffset(BtCursor*);
முழு sqlite3BtreePayload(BtCursor*, u32 offset, u32 amt, வெறுமை*);
மாறிலி வெறுமை *sqlite3BtreePayloadFetch(BtCursor*, u32 *pAmt);
u32 sqlite3BtreePayloadSize(BtCursor*);
sqlite3_int64 sqlite3BtreeMaxRecordSize(BtCursor*);

முழு sqlite3BtreeIntegrityCheck(
  sqlite3 *db,  /* Database connection that is running the check */
  Btree *p,     /* The btree to be checked */
  Pgno *a_வேர்,  /* An array of root pages numbers for individual trees */
  sqlite3_மதிப்பு *aCnt,  /* OUT: entry counts for each btree in aRoot[] */
  முழு n_வேர்,    /* Number of entries in aRoot[] */
  முழு mxErr,    /* Stop reporting errors after this many */
  முழு *pnErr,   /* OUT: Write number of errors seen to this variable */
  எழுத்து **pzOut  /* OUT: Write the error message string here */
);
கட்டமைப்பு Pager *sqlite3BtreePager(Btree*);
i64 sqlite3_Btree_Row_மொத்தம்_Est(BtCursor*);

#ifndef SQLITE_OMIT_INCRBLOB
முழு sqlite3BtreePayloadChecked(BtCursor*, u32 offset, u32 amt, வெறுமை*);
முழு sqlite3BtreePutData(BtCursor*, u32 offset, u32 amt, வெறுமை*);
வெறுமை sqlite3BtreeIncrblobCursor(BtCursor *);
#endif
வெறுமை sqlite3BtreeClearCursor(BtCursor *);
முழு sqlite3BtreeSetVersion(Btree *pBt, முழு iVersion);
முழு sqlite3BtreeCursorHasHint(BtCursor*, unsigned முழு mask);
முழு sqlite3BtreeIsReadonly(Btree *pBt);
முழு sqlite3HeaderSizeBtree(வெறுமை);

#ifdef SQLITE_DEBUG
sqlite3_uint64 sqlite3_Btree_Seek_மொத்தம்(Btree*);
#இல்லையெனில்
# define sqlite3_Btree_Seek_மொத்தம்(X) 0
#endif

#ifndef NDEBUG
முழு sqlite3BtreeCursorIsValid(BtCursor*);
#endif
முழு sqlite3BtreeCursorIsValidNN(BtCursor*);

முழு sqlite3_Btree_மொத்தம்(sqlite3*, BtCursor*, i64*);

#ifdef SQLITE_TEST
முழு sqlite3BtreeCursorInfo(BtCursor*, முழு*, முழு);
வெறுமை sqlite3BtreeCursorList(Btree*);
#endif

#ifndef SQLITE_OMIT_WAL
  முழு sqlite3BtreeCheckpoint(Btree*, முழு, முழு *, முழு *);
#endif

முழு sqlite3BtreeTransferRow(BtCursor*, BtCursor*, i64);

வெறுமை sqlite3BtreeClearCache(Btree*);

/*
** If we are not using shared cache, then there is no need to
** use mutexes to access the BtShared structures.  So make the
** Enter and Leave procedures no-ops.
*/
#ifndef SQLITE_OMIT_SHARED_CACHE
  வெறுமை sqlite3BtreeEnter(Btree*);
  வெறுமை sqlite3BtreeEnterAll(sqlite3*);
  முழு sqlite3BtreeSharable(Btree*);
  வெறுமை sqlite3BtreeEnterCursor(BtCursor*);
  முழு sqlite3_Btree_Connection_மொத்தம்(Btree*);
#இல்லையெனில்
# define sqlite3BtreeEnter(X) 
# define sqlite3BtreeEnterAll(X)
# define sqlite3BtreeSharable(X) 0
# define sqlite3BtreeEnterCursor(X)
# define sqlite3_Btree_Connection_மொத்தம்(X) 1
#endif

#எனில் !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE
  வெறுமை sqlite3BtreeLeave(Btree*);
  வெறுமை sqlite3BtreeLeaveCursor(BtCursor*);
  வெறுமை sqlite3BtreeLeaveAll(sqlite3*);
#ifndef NDEBUG
  /* These routines are used inside assert() statements only. */
  முழு sqlite3BtreeHoldsMutex(Btree*);
  முழு sqlite3BtreeHoldsAllMutexes(sqlite3*);
  முழு sqlite3SchemaMutexHeld(sqlite3*,முழு,Schema*);
#endif
#இல்லையெனில்

# define sqlite3BtreeLeave(X)
# define sqlite3BtreeLeaveCursor(X)
# define sqlite3BtreeLeaveAll(X)

# define sqlite3BtreeHoldsMutex(X) 1
# define sqlite3BtreeHoldsAllMutexes(X) 1
# define sqlite3SchemaMutexHeld(X,Y,Z) 1
#endif


#endif /* SQLITE_BTREE_H */
