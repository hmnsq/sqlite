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
#اگر_تعریف_نشده SQLITE_BTREE_H
#تعریف_کن SQLITE_BTREE_H

/* TODO: This definition is just included so other modules compile. It
** needs to be revisited.
*/
#تعریف_کن SQLITE_N_BTREE_META 16

/*
** If defined as non-zero, auto-vacuum is enabled by default. Otherwise
** it must be turned on for each database using "PRAGMA auto_vacuum = 1".
*/
#اگر_تعریف_نشده SQLITE_DEFAULT_AUTOVACUUM
  #تعریف_کن SQLITE_DEFAULT_AUTOVACUUM 0
#پایان_اگر

#تعریف_کن BTREE_AUTOVACUUM_NONE 0        /* Do not do auto-vacuum */
#تعریف_کن BTREE_AUTOVACUUM_پر 1        /* Do full auto-vacuum */
#تعریف_کن BTREE_AUTOVACUUM_INCR 2        /* Incremental vacuum */

/*
** Forward declarations of structure
*/
تعريفنوع بنية Btree Btree;
تعريفنوع بنية BtCursor BtCursor;
تعريفنوع بنية BtShared BtShared;
تعريفنوع بنية BtreePayload BtreePayload;


صحيح sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,       /* VFS to use with this b-tree */
  ثابت حرف *zFilename,   /* Name of database file to open */
  sqlite3 *db,             /* Associated database connection */
  Btree **ppBtree,         /* Return open Btree* here */
  صحيح flags,               /* Flags */
  صحيح vfsFlags             /* Flags passed through to VFS open */
);

/* The flags parameter to sqlite3BtreeOpen can be the bitwise or of the
** following values.
**
** NOTE:  These values must match the corresponding PAGER_ values in
** pager.h.
*/
#تعریف_کن BTREE_OMIT_JOURNAL  1  /* Do not create or use a rollback journal */
#تعریف_کن BTREE_حافظه        2  /* This is an in-memory DB */
#تعریف_کن BTREE_SINGLE        4  /* The file contains at most 1 b-tree */
#تعریف_کن BTREE_UNORDERED     8  /* Use of a hash implementation is OK */

صحيح sqlite3BtreeClose(Btree*);
صحيح sqlite3_Btree_مجموعه_Cache_اندازه(Btree*,صحيح);
صحيح sqlite3_Btree_مجموعه_Spill_اندازه(Btree*,صحيح);
#إذا SQLITE_MAX_MMAP_اندازه>0
  صحيح sqlite3_Btree_مجموعه_Mmap_حد(Btree*,sqlite3_int64);
#پایان_اگر
صحيح sqlite3_Btree_مجموعه_Pager_Flags(Btree*,بیعلامت);
صحيح sqlite3_Btree_مجموعه_صفحه_اندازه(Btree *p, صحيح nPagesize, صحيح nReserve, صحيح eFix);
صحيح sqlite3_Btree_Get_صفحه_اندازه(Btree*);
Pgno sqlite3_Btree_Max_صفحه_شمار(Btree*,Pgno);
Pgno sqlite3_Btree_آخرین_صفحه(Btree*);
صحيح sqlite3BtreeSecureDelete(Btree*,صحيح);
صحيح sqlite3BtreeGetRequestedReserve(Btree*);
صحيح sqlite3BtreeGetReserveNoMutex(Btree *p);
صحيح sqlite3_Btree_مجموعه_Auto_Vacuum(Btree *, صحيح);
صحيح sqlite3BtreeGetAutoVacuum(Btree *);
صحيح sqlite3BtreeBeginTrans(Btree*,صحيح,صحيح*);
صحيح sqlite3BtreeCommitPhaseOne(Btree*, ثابت حرف*);
صحيح sqlite3BtreeCommitPhaseTwo(Btree*, صحيح);
صحيح sqlite3BtreeCommit(Btree*);
صحيح sqlite3BtreeRollback(Btree*,صحيح,صحيح);
صحيح sqlite3BtreeBeginStmt(Btree*,صحيح);
صحيح sqlite3_Btree_Create_جدول(Btree*, Pgno*, صحيح flags);
صحيح sqlite3BtreeTxnState(Btree*);
صحيح sqlite3_Btree_Is_درون_Backup(Btree*);

فراغ *sqlite3BtreeSchema(Btree *, صحيح, فراغ(*)(فراغ *));
صحيح sqlite3BtreeSchemaLocked(Btree *pBtree);
#اگر_تعریف_نشده SQLITE_OMIT_SHARED_CACHE
صحيح sqlite3_Btree_قفل_جدول(Btree *pBtree, صحيح iTab, u8 is_Write_قفل);
#پایان_اگر

/* Savepoints are named, nestable SQL transactions mostly implemented */ 
/* in vdbe.c and pager.c See https://sqlite.org/lang_savepoint.html */
صحيح sqlite3BtreeSavepoint(Btree *, صحيح, صحيح);

/* "Checkpoint" only refers to WAL. See https://sqlite.org/wal.html#ckpt */
#اگر_تعریف_نشده SQLITE_OMIT_WAL
  صحيح sqlite3BtreeCheckpoint(Btree*, صحيح, صحيح *, صحيح *);  
#پایان_اگر

ثابت حرف *sqlite3BtreeGetFilename(Btree *);
ثابت حرف *sqlite3BtreeGetJournalname(Btree *);
صحيح sqlite3BtreeCopyFile(Btree*, Btree*);

صحيح sqlite3BtreeIncrVacuum(Btree *);

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
#تعریف_کن BTREE_INTKEY     1    /* Table has only 64-bit signed integer keys */
#تعریف_کن BTREE_BLOBKEY    2    /* Table has keys only - no data */

صحيح sqlite3_Btree_Drop_جدول(Btree*, صحيح, صحيح*);
صحيح sqlite3_Btree_Clear_جدول(Btree*, صحيح, i64*);
صحيح sqlite3_Btree_Clear_جدول_از_Cursor(BtCursor*);
صحيح sqlite3BtreeTripAllCursors(Btree*, صحيح, صحيح);

فراغ sqlite3BtreeGetMeta(Btree *pBtree, صحيح idx, u32 *p_مقدار);
صحيح sqlite3BtreeUpdateMeta(Btree*, صحيح idx, u32 مقدار);

صحيح sqlite3BtreeNewDb(Btree *p);

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
#تعریف_کن BTREE_FREE_صفحه_شمار     0
#تعریف_کن BTREE_SCHEMA_نسخه      1
#تعریف_کن BTREE_FILE_FORMAT         2
#تعریف_کن BTREE_DEFAULT_CACHE_اندازه  3
#تعریف_کن BTREE_بزرگترین_ریشه_صفحه   4
#تعریف_کن BTREE_متن_ENCODING       5
#تعریف_کن BTREE_USER_نسخه        6
#تعریف_کن BTREE_INCR_VACUUM         7
#تعریف_کن BTREE_APPLICATION_ID      8
#تعریف_کن BTREE_داده_نسخه        15  /* A virtual meta-value */

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
#تعریف_کن BTREE_HINT_RANGE 0       /* Range constraints on queries */
#تعریف_کن BTREE_HINT_TABLECURSOR 1 /* Table csr associated with this index csr */

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
#تعریف_کن BTREE_BULKLOAD 0x00000001  /* Used to full index in sorted order */
#تعریف_کن BTREE_SEEK_EQ  0x00000002  /* EQ seeks only - no range seeks */

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
#تعریف_کن BTREE_WRCSR     0x00000004     /* read-write cursor */
#تعریف_کن BTREE_FORDELETE 0x00000008     /* Cursor is for seek/delete only */

صحيح sqlite3BtreeCursor(
  Btree*,                              /* BTree containing table to open */
  Pgno i_جدول,                         /* Index of root page */
  صحيح wrFlag,                          /* 1 for writing.  0 for read-only */
  بنية کلید_Info*,                     /* First argument to compare function */
  BtCursor *pCursor                    /* Space to write cursor structure */
);
BtCursor *sqlite3BtreeFakeValidCursor(فراغ);
صحيح sqlite3_Btree_Cursor_اندازه(فراغ);
#اگر_تعریف_شده SQLITE_DEBUG
صحيح sqlite3BtreeClosesWithCursor(Btree*,BtCursor*);
#پایان_اگر
فراغ sqlite3BtreeCursorZero(BtCursor*);
فراغ sqlite3BtreeCursorHintFlags(BtCursor*, بیعلامت);
#اگر_تعریف_شده SQLITE_ENABLE_CURSOR_HINTS
فراغ sqlite3BtreeCursorHint(BtCursor*, صحيح, ...);
 #اگر_تعریف_شده SQLITE_DEBUG
 BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor*);
 #پایان_اگر
#پایان_اگر

صحيح sqlite3BtreeCloseCursor(BtCursor*);
صحيح sqlite3_Btree_جدول_Moveto(
  BtCursor*,
  i64 int_کلید,
  صحيح bias,
  صحيح *pRes
);
صحيح sqlite3_Btree_شاخص_Moveto(
  BtCursor*,
  Unpacked_رکورد *p_Un_کلید,
  صحيح *pRes
);
صحيح sqlite3BtreeCursorHasMoved(BtCursor*);
صحيح sqlite3BtreeCursorRestore(BtCursor*, صحيح*);
صحيح sqlite3BtreeDelete(BtCursor*, u8 flags);

/* Allowed flags for sqlite3BtreeDelete() and sqlite3BtreeInsert() */
#تعریف_کن BTREE_SAVEPOSITION 0x02  /* Leave cursor pointing at NEXT or PREV */
#تعریف_کن BTREE_AUXDELETE    0x04  /* not the primary delete operation */
#تعریف_کن BTREE_APPEND       0x08  /* Insert is likely an append */
#تعریف_کن BTREE_PREFORMAT    0x80  /* Inserted data is a preformated cell */

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
بنية BtreePayload {
  ثابت فراغ *p_کلید;       /* Key content for indexes.  NULL for tables */
  sqlite3_int64 n_کلید;     /* Size of pKey for indexes.  PRIMARY KEY for tabs */
  ثابت فراغ *p_داده;      /* Data for tables. */
  sqlite3_مقدار *aMem;    /* First of nMem value in the unpacked pKey */
  u16 nMem;               /* Number of aMem[] value.  Might be zero */
  صحيح n_داده;              /* Size of pData.  0 if none. */
  صحيح nZero;              /* Extra zero data appended after pData,nData */
};

صحيح sqlite3_Btree_درج(BtCursor*, ثابت BtreePayload *pPayload,
                       صحيح flags, صحيح seek_نتیجه);
صحيح sqlite3_Btree_اولی(BtCursor*, صحيح *pRes);
صحيح sqlite3_Btree_Is_خالی(BtCursor *pCur, صحيح *pRes);
صحيح sqlite3_Btree_آخرین(BtCursor*, صحيح *pRes);
صحيح sqlite3_Btree_بعدی(BtCursor*, صحيح flags);
صحيح sqlite3BtreeEof(BtCursor*);
صحيح sqlite3_Btree_قبلی(BtCursor*, صحيح flags);
i64 sqlite3_Btree_Integer_کلید(BtCursor*);
فراغ sqlite3BtreeCursorPin(BtCursor*);
فراغ sqlite3BtreeCursorUnpin(BtCursor*);
i64 sqlite3_Btree_جابه_جایی(BtCursor*);
صحيح sqlite3BtreePayload(BtCursor*, u32 جابه_جایی, u32 amt, فراغ*);
ثابت فراغ *sqlite3BtreePayloadFetch(BtCursor*, u32 *pAmt);
u32 sqlite3_Btree_Payload_اندازه(BtCursor*);
sqlite3_int64 sqlite3_Btree_Max_رکورد_اندازه(BtCursor*);

صحيح sqlite3BtreeIntegrityCheck(
  sqlite3 *db,  /* Database connection that is running the check */
  Btree *p,     /* The btree to be checked */
  Pgno *a_ریشه,  /* An array of root pages numbers for individual trees */
  sqlite3_مقدار *aCnt,  /* OUT: entry counts for each btree in aRoot[] */
  صحيح n_ریشه,    /* Number of entries in aRoot[] */
  صحيح mxErr,    /* Stop reporting errors after this many */
  صحيح *pnErr,   /* OUT: Write number of errors seen to this variable */
  حرف **pz_بیرون  /* OUT: Write the error message string here */
);
بنية Pager *sqlite3BtreePager(Btree*);
i64 sqlite3_Btree_ردیف_شمار_Est(BtCursor*);

#اگر_تعریف_نشده SQLITE_OMIT_INCRBLOB
صحيح sqlite3BtreePayloadChecked(BtCursor*, u32 جابه_جایی, u32 amt, فراغ*);
صحيح sqlite3_Btree_Put_داده(BtCursor*, u32 جابه_جایی, u32 amt, فراغ*);
فراغ sqlite3BtreeIncrblobCursor(BtCursor *);
#پایان_اگر
فراغ sqlite3BtreeClearCursor(BtCursor *);
صحيح sqlite3_Btree_مجموعه_نسخه(Btree *pBt, صحيح i_نسخه);
صحيح sqlite3BtreeCursorHasHint(BtCursor*, بیعلامت صحيح ماسک);
صحيح sqlite3BtreeIsReadonly(Btree *pBt);
صحيح sqlite3_سرآیند_اندازه_Btree(فراغ);

#اگر_تعریف_شده SQLITE_DEBUG
sqlite3_uint64 sqlite3_Btree_Seek_شمار(Btree*);
#وإلا
# define sqlite3_Btree_Seek_شمار(X) 0
#پایان_اگر

#اگر_تعریف_نشده NDEBUG
صحيح sqlite3BtreeCursorIsValid(BtCursor*);
#پایان_اگر
صحيح sqlite3BtreeCursorIsValidNN(BtCursor*);

صحيح sqlite3_Btree_شمار(sqlite3*, BtCursor*, i64*);

#اگر_تعریف_شده SQLITE_TEST
صحيح sqlite3BtreeCursorInfo(BtCursor*, صحيح*, صحيح);
فراغ sqlite3BtreeCursorList(Btree*);
#پایان_اگر

#اگر_تعریف_نشده SQLITE_OMIT_WAL
  صحيح sqlite3BtreeCheckpoint(Btree*, صحيح, صحيح *, صحيح *);
#پایان_اگر

صحيح sqlite3_Btree_Transfer_ردیف(BtCursor*, BtCursor*, i64);

فراغ sqlite3BtreeClearCache(Btree*);

/*
** If we are not using shared cache, then there is no need to
** use mutexes to access the BtShared structures.  So make the
** Enter and Leave procedures no-ops.
*/
#اگر_تعریف_نشده SQLITE_OMIT_SHARED_CACHE
  فراغ sqlite3BtreeEnter(Btree*);
  فراغ sqlite3BtreeEnterAll(sqlite3*);
  صحيح sqlite3BtreeSharable(Btree*);
  فراغ sqlite3BtreeEnterCursor(BtCursor*);
  صحيح sqlite3_Btree_Connection_شمار(Btree*);
#وإلا
# define sqlite3BtreeEnter(X) 
# define sqlite3BtreeEnterAll(X)
# define sqlite3BtreeSharable(X) 0
# define sqlite3BtreeEnterCursor(X)
# define sqlite3_Btree_Connection_شمار(X) 1
#پایان_اگر

#إذا !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE
  فراغ sqlite3BtreeLeave(Btree*);
  فراغ sqlite3BtreeLeaveCursor(BtCursor*);
  فراغ sqlite3BtreeLeaveAll(sqlite3*);
#اگر_تعریف_نشده NDEBUG
  /* These routines are used inside assert() statements only. */
  صحيح sqlite3BtreeHoldsMutex(Btree*);
  صحيح sqlite3BtreeHoldsAllMutexes(sqlite3*);
  صحيح sqlite3SchemaMutexHeld(sqlite3*,صحيح,Schema*);
#پایان_اگر
#وإلا

# define sqlite3BtreeLeave(X)
# define sqlite3BtreeLeaveCursor(X)
# define sqlite3BtreeLeaveAll(X)

# define sqlite3BtreeHoldsMutex(X) 1
# define sqlite3BtreeHoldsAllMutexes(X) 1
# define sqlite3SchemaMutexHeld(X,Y,Z) 1
#پایان_اگر


#پایان_اگر /* SQLITE_BTREE_H */
