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
#اگر_غیر_تعریف_شدہ SQLITE_BTREE_H
#تعریف_کرو SQLITE_BTREE_H

/* TODO: This definition is just included so other modules compile. It
** needs to be revisited.
*/
#تعریف_کرو SQLITE_N_BTREE_META 16

/*
** If defined as non-zero, auto-vacuum is enabled by default. Otherwise
** it must be turned on for each database using "PRAGMA auto_vacuum = 1".
*/
#اگر_غیر_تعریف_شدہ SQLITE_DEFAULT_AUTOVACUUM
  #تعریف_کرو SQLITE_DEFAULT_AUTOVACUUM 0
#اگر_ختم

#تعریف_کرو BTREE_AUTOVACUUM_NONE 0        /* Do not do auto-vacuum */
#تعریف_کرو BTREE_AUTOVACUUM_بھرا 1        /* Do full auto-vacuum */
#تعریف_کرو BTREE_AUTOVACUUM_INCR 2        /* Incremental vacuum */

/*
** Forward declarations of structure
*/
قسمتعریف ساخت Btree Btree;
قسمتعریف ساخت BtCursor BtCursor;
قسمتعریف ساخت BtShared BtShared;
قسمتعریف ساخت BtreePayload BtreePayload;


صحیح sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,       /* VFS to use with this b-tree */
  ثابت حرف *zFilename,   /* Name of database file to open */
  sqlite3 *db,             /* Associated database connection */
  Btree **ppBtree,         /* Return open Btree* here */
  صحیح flags,               /* Flags */
  صحیح vfsFlags             /* Flags passed through to VFS open */
);

/* The flags parameter to sqlite3BtreeOpen can be the bitwise or of the
** following values.
**
** NOTE:  These values must match the corresponding PAGER_ values in
** pager.h.
*/
#تعریف_کرو BTREE_OMIT_JOURNAL  1  /* Do not create or use a rollback journal */
#تعریف_کرو BTREE_حافظہ        2  /* This is an in-memory DB */
#تعریف_کرو BTREE_SINGLE        4  /* The file contains at most 1 b-tree */
#تعریف_کرو BTREE_UNORDERED     8  /* Use of a hash implementation is OK */

صحیح sqlite3BtreeClose(Btree*);
صحیح sqlite3_Btree_سیٹ_Cache_سائز(Btree*,صحیح);
صحیح sqlite3_Btree_سیٹ_Spill_سائز(Btree*,صحیح);
#اگر SQLITE_MAX_MMAP_سائز>0
  صحیح sqlite3_Btree_سیٹ_Mmap_حد_بندی(Btree*,sqlite3_int64);
#اگر_ختم
صحیح sqlite3_Btree_سیٹ_Pager_Flags(Btree*,غیرعلامتی);
صحیح sqlite3_Btree_سیٹ_صفحہ_سائز(Btree *p, صحیح nPagesize, صحیح nReserve, صحیح eFix);
صحیح sqlite3_Btree_Get_صفحہ_سائز(Btree*);
Pgno sqlite3_Btree_Max_صفحہ_گنتی(Btree*,Pgno);
Pgno sqlite3_Btree_آخری_صفحہ(Btree*);
صحیح sqlite3BtreeSecureDelete(Btree*,صحیح);
صحیح sqlite3BtreeGetRequestedReserve(Btree*);
صحیح sqlite3BtreeGetReserveNoMutex(Btree *p);
صحیح sqlite3_Btree_سیٹ_Auto_Vacuum(Btree *, صحیح);
صحیح sqlite3BtreeGetAutoVacuum(Btree *);
صحیح sqlite3BtreeBeginTrans(Btree*,صحیح,صحیح*);
صحیح sqlite3BtreeCommitPhaseOne(Btree*, ثابت حرف*);
صحیح sqlite3BtreeCommitPhaseTwo(Btree*, صحیح);
صحیح sqlite3BtreeCommit(Btree*);
صحیح sqlite3BtreeRollback(Btree*,صحیح,صحیح);
صحیح sqlite3BtreeBeginStmt(Btree*,صحیح);
صحیح sqlite3_Btree_Create_جدول(Btree*, Pgno*, صحیح flags);
صحیح sqlite3BtreeTxnState(Btree*);
صحیح sqlite3_Btree_Is_اندر_Backup(Btree*);

خالی *sqlite3BtreeSchema(Btree *, صحیح, خالی(*)(خالی *));
صحیح sqlite3BtreeSchemaLocked(Btree *pBtree);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
صحیح sqlite3_Btree_تالا_جدول(Btree *pBtree, صحیح iTab, u8 is_Write_تالا);
#اگر_ختم

/* Savepoints are named, nestable SQL transactions mostly implemented */ 
/* in vdbe.c and pager.c See https://sqlite.org/lang_savepoint.html */
صحیح sqlite3BtreeSavepoint(Btree *, صحیح, صحیح);

/* "Checkpoint" only refers to WAL. See https://sqlite.org/wal.html#ckpt */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
  صحیح sqlite3BtreeCheckpoint(Btree*, صحیح, صحیح *, صحیح *);  
#اگر_ختم

ثابت حرف *sqlite3BtreeGetFilename(Btree *);
ثابت حرف *sqlite3BtreeGetJournalname(Btree *);
صحیح sqlite3BtreeCopyFile(Btree*, Btree*);

صحیح sqlite3BtreeIncrVacuum(Btree *);

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
#تعریف_کرو BTREE_INTKEY     1    /* Table has only 64-bit signed integer keys */
#تعریف_کرو BTREE_BLOBKEY    2    /* Table has keys only - no data */

صحیح sqlite3_Btree_Drop_جدول(Btree*, صحیح, صحیح*);
صحیح sqlite3_Btree_Clear_جدول(Btree*, صحیح, i64*);
صحیح sqlite3_Btree_Clear_جدول_کا_Cursor(BtCursor*);
صحیح sqlite3BtreeTripAllCursors(Btree*, صحیح, صحیح);

خالی sqlite3BtreeGetMeta(Btree *pBtree, صحیح idx, u32 *p_قدر);
صحیح sqlite3BtreeUpdateMeta(Btree*, صحیح idx, u32 قدر);

صحیح sqlite3BtreeNewDb(Btree *p);

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
#تعریف_کرو BTREE_FREE_صفحہ_گنتی     0
#تعریف_کرو BTREE_SCHEMA_نسخہ      1
#تعریف_کرو BTREE_FILE_FORMAT         2
#تعریف_کرو BTREE_DEFAULT_CACHE_سائز  3
#تعریف_کرو BTREE_سب_سے_بڑا_جڑ_صفحہ   4
#تعریف_کرو BTREE_متن_ENCODING       5
#تعریف_کرو BTREE_USER_نسخہ        6
#تعریف_کرو BTREE_INCR_VACUUM         7
#تعریف_کرو BTREE_APPLICATION_ID      8
#تعریف_کرو BTREE_ڈیٹا_نسخہ        15  /* A virtual meta-value */

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
#تعریف_کرو BTREE_HINT_RANGE 0       /* Range constraints on queries */
#تعریف_کرو BTREE_HINT_TABLECURSOR 1 /* Table csr associated with this index csr */

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
#تعریف_کرو BTREE_BULKLOAD 0x00000001  /* Used to full index in sorted order */
#تعریف_کرو BTREE_SEEK_EQ  0x00000002  /* EQ seeks only - no range seeks */

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
#تعریف_کرو BTREE_WRCSR     0x00000004     /* read-write cursor */
#تعریف_کرو BTREE_FORDELETE 0x00000008     /* Cursor is for seek/delete only */

صحیح sqlite3BtreeCursor(
  Btree*,                              /* BTree containing table to open */
  Pgno i_جدول,                         /* Index of root page */
  صحیح wrFlag,                          /* 1 for writing.  0 for read-only */
  ساخت کلید_Info*,                     /* First argument to compare function */
  BtCursor *pCursor                    /* Space to write cursor structure */
);
BtCursor *sqlite3BtreeFakeValidCursor(خالی);
صحیح sqlite3_Btree_Cursor_سائز(خالی);
#اگر_تعریف_شدہ SQLITE_DEBUG
صحیح sqlite3BtreeClosesWithCursor(Btree*,BtCursor*);
#اگر_ختم
خالی sqlite3BtreeCursorZero(BtCursor*);
خالی sqlite3BtreeCursorHintFlags(BtCursor*, غیرعلامتی);
#اگر_تعریف_شدہ SQLITE_ENABLE_CURSOR_HINTS
خالی sqlite3BtreeCursorHint(BtCursor*, صحیح, ...);
 #اگر_تعریف_شدہ SQLITE_DEBUG
 BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor*);
 #اگر_ختم
#اگر_ختم

صحیح sqlite3BtreeCloseCursor(BtCursor*);
صحیح sqlite3_Btree_جدول_Moveto(
  BtCursor*,
  i64 int_کلید,
  صحیح bias,
  صحیح *pRes
);
صحیح sqlite3_Btree_اشاریہ_Moveto(
  BtCursor*,
  Unpacked_ریکارڈ *p_Un_کلید,
  صحیح *pRes
);
صحیح sqlite3BtreeCursorHasMoved(BtCursor*);
صحیح sqlite3BtreeCursorRestore(BtCursor*, صحیح*);
صحیح sqlite3BtreeDelete(BtCursor*, u8 flags);

/* Allowed flags for sqlite3BtreeDelete() and sqlite3BtreeInsert() */
#تعریف_کرو BTREE_SAVEPOSITION 0x02  /* Leave cursor pointing at NEXT or PREV */
#تعریف_کرو BTREE_AUXDELETE    0x04  /* not the primary delete operation */
#تعریف_کرو BTREE_APPEND       0x08  /* Insert is likely an append */
#تعریف_کرو BTREE_PREFORMAT    0x80  /* Inserted data is a preformated cell */

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
ساخت BtreePayload {
  ثابت خالی *p_کلید;       /* Key content for indexes.  NULL for tables */
  sqlite3_int64 n_کلید;     /* Size of pKey for indexes.  PRIMARY KEY for tabs */
  ثابت خالی *p_ڈیٹا;      /* Data for tables. */
  sqlite3_قدر *aMem;    /* First of nMem value in the unpacked pKey */
  u16 nMem;               /* Number of aMem[] value.  Might be zero */
  صحیح n_ڈیٹا;              /* Size of pData.  0 if none. */
  صحیح nZero;              /* Extra zero data appended after pData,nData */
};

صحیح sqlite3_Btree_داخل_کرو(BtCursor*, ثابت BtreePayload *pPayload,
                       صحیح flags, صحیح seek_نتیجہ);
صحیح sqlite3_Btree_پہلا(BtCursor*, صحیح *pRes);
صحیح sqlite3_Btree_Is_خالی_خانے(BtCursor *pCur, صحیح *pRes);
صحیح sqlite3_Btree_آخری(BtCursor*, صحیح *pRes);
صحیح sqlite3_Btree_اگلا(BtCursor*, صحیح flags);
صحیح sqlite3BtreeEof(BtCursor*);
صحیح sqlite3_Btree_پچھلا(BtCursor*, صحیح flags);
i64 sqlite3_Btree_Integer_کلید(BtCursor*);
خالی sqlite3BtreeCursorPin(BtCursor*);
خالی sqlite3BtreeCursorUnpin(BtCursor*);
i64 sqlite3_Btree_انحراف(BtCursor*);
صحیح sqlite3BtreePayload(BtCursor*, u32 انحراف, u32 amt, خالی*);
ثابت خالی *sqlite3BtreePayloadFetch(BtCursor*, u32 *pAmt);
u32 sqlite3_Btree_Payload_سائز(BtCursor*);
sqlite3_int64 sqlite3_Btree_Max_ریکارڈ_سائز(BtCursor*);

صحیح sqlite3BtreeIntegrityCheck(
  sqlite3 *db,  /* Database connection that is running the check */
  Btree *p,     /* The btree to be checked */
  Pgno *a_جڑ,  /* An array of root pages numbers for individual trees */
  sqlite3_قدر *aCnt,  /* OUT: entry counts for each btree in aRoot[] */
  صحیح n_جڑ,    /* Number of entries in aRoot[] */
  صحیح mxErr,    /* Stop reporting errors after this many */
  صحیح *pnErr,   /* OUT: Write number of errors seen to this variable */
  حرف **pz_برآمد  /* OUT: Write the error message string here */
);
ساخت Pager *sqlite3BtreePager(Btree*);
i64 sqlite3_Btree_صف_گنتی_Est(BtCursor*);

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_INCRBLOB
صحیح sqlite3BtreePayloadChecked(BtCursor*, u32 انحراف, u32 amt, خالی*);
صحیح sqlite3_Btree_Put_ڈیٹا(BtCursor*, u32 انحراف, u32 amt, خالی*);
خالی sqlite3BtreeIncrblobCursor(BtCursor *);
#اگر_ختم
خالی sqlite3BtreeClearCursor(BtCursor *);
صحیح sqlite3_Btree_سیٹ_نسخہ(Btree *pBt, صحیح i_نسخہ);
صحیح sqlite3BtreeCursorHasHint(BtCursor*, غیرعلامتی صحیح نقاب);
صحیح sqlite3BtreeIsReadonly(Btree *pBt);
صحیح sqlite3_سرخی_سائز_Btree(خالی);

#اگر_تعریف_شدہ SQLITE_DEBUG
sqlite3_uint64 sqlite3_Btree_Seek_گنتی(Btree*);
#ورنہ
# define sqlite3_Btree_Seek_گنتی(X) 0
#اگر_ختم

#اگر_غیر_تعریف_شدہ NDEBUG
صحیح sqlite3BtreeCursorIsValid(BtCursor*);
#اگر_ختم
صحیح sqlite3BtreeCursorIsValidNN(BtCursor*);

صحیح sqlite3_Btree_گنتی(sqlite3*, BtCursor*, i64*);

#اگر_تعریف_شدہ SQLITE_TEST
صحیح sqlite3BtreeCursorInfo(BtCursor*, صحیح*, صحیح);
خالی sqlite3BtreeCursorList(Btree*);
#اگر_ختم

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WAL
  صحیح sqlite3BtreeCheckpoint(Btree*, صحیح, صحیح *, صحیح *);
#اگر_ختم

صحیح sqlite3_Btree_Transfer_صف(BtCursor*, BtCursor*, i64);

خالی sqlite3BtreeClearCache(Btree*);

/*
** If we are not using shared cache, then there is no need to
** use mutexes to access the BtShared structures.  So make the
** Enter and Leave procedures no-ops.
*/
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SHARED_CACHE
  خالی sqlite3BtreeEnter(Btree*);
  خالی sqlite3BtreeEnterAll(sqlite3*);
  صحیح sqlite3BtreeSharable(Btree*);
  خالی sqlite3BtreeEnterCursor(BtCursor*);
  صحیح sqlite3_Btree_Connection_گنتی(Btree*);
#ورنہ
# define sqlite3BtreeEnter(X) 
# define sqlite3BtreeEnterAll(X)
# define sqlite3BtreeSharable(X) 0
# define sqlite3BtreeEnterCursor(X)
# define sqlite3_Btree_Connection_گنتی(X) 1
#اگر_ختم

#اگر !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE
  خالی sqlite3BtreeLeave(Btree*);
  خالی sqlite3BtreeLeaveCursor(BtCursor*);
  خالی sqlite3BtreeLeaveAll(sqlite3*);
#اگر_غیر_تعریف_شدہ NDEBUG
  /* These routines are used inside assert() statements only. */
  صحیح sqlite3BtreeHoldsMutex(Btree*);
  صحیح sqlite3BtreeHoldsAllMutexes(sqlite3*);
  صحیح sqlite3SchemaMutexHeld(sqlite3*,صحیح,Schema*);
#اگر_ختم
#ورنہ

# define sqlite3BtreeLeave(X)
# define sqlite3BtreeLeaveCursor(X)
# define sqlite3BtreeLeaveAll(X)

# define sqlite3BtreeHoldsMutex(X) 1
# define sqlite3BtreeHoldsAllMutexes(X) 1
# define sqlite3SchemaMutexHeld(X,Y,Z) 1
#اگر_ختم


#اگر_ختم /* SQLITE_BTREE_H */
