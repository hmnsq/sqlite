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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_BTREE_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_BTREE_H

/* TODO: This definition is just included so other modules compile. It
** needs to be revisited.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_N_BTREE_META 16

/*
** If defined as non-zero, auto-vacuum is enabled by default. Otherwise
** it must be turned on for each database using "PRAGMA auto_vacuum = 1".
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_DEFAULT_AUTOVACUUM
  #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_DEFAULT_AUTOVACUUM 0
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_AUTOVACUUM_NONE 0        /* Do not do auto-vacuum */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_AUTOVACUUM_ਭਰਿਆ 1        /* Do full auto-vacuum */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_AUTOVACUUM_INCR 2        /* Incremental vacuum */

/*
** Forward declarations of structure
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Btree Btree;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ BtCursor BtCursor;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ BtShared BtShared;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ BtreePayload BtreePayload;


ਪੂਰਨ sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,       /* VFS to use with this b-tree */
  ਅਚਲ ਅੱਖਰ *zFilename,   /* Name of database file to open */
  sqlite3 *db,             /* Associated database connection */
  Btree **ppBtree,         /* Return open Btree* here */
  ਪੂਰਨ flags,               /* Flags */
  ਪੂਰਨ vfsFlags             /* Flags passed through to VFS open */
);

/* The flags parameter to sqlite3BtreeOpen can be the bitwise or of the
** following values.
**
** NOTE:  These values must match the corresponding PAGER_ values in
** pager.h.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_OMIT_JOURNAL  1  /* Do not create or use a rollback journal */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_ਸਮ੍ਰਿਤੀ        2  /* This is an in-memory DB */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_SINGLE        4  /* The file contains at most 1 b-tree */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_UNORDERED     8  /* Use of a hash implementation is OK */

ਪੂਰਨ sqlite3BtreeClose(Btree*);
ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_Cache_ਮਾਪ(Btree*,ਪੂਰਨ);
ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_Spill_ਮਾਪ(Btree*,ਪੂਰਨ);
#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
  ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_Mmap_ਹੱਦਬੰਦੀ(Btree*,sqlite3_int64);
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_Pager_Flags(Btree*,ਬਿਨਾਨਿਸ਼ਾਨੀ);
ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_ਪੰਨਾ_ਮਾਪ(Btree *p, ਪੂਰਨ nPagesize, ਪੂਰਨ nReserve, ਪੂਰਨ eFix);
ਪੂਰਨ sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(Btree*);
Pgno sqlite3_Btree_Max_ਪੰਨਾ_ਗਿਣਤੀ(Btree*,Pgno);
Pgno sqlite3_Btree_ਆਖਰੀ_ਪੰਨਾ(Btree*);
ਪੂਰਨ sqlite3BtreeSecureDelete(Btree*,ਪੂਰਨ);
ਪੂਰਨ sqlite3BtreeGetRequestedReserve(Btree*);
ਪੂਰਨ sqlite3BtreeGetReserveNoMutex(Btree *p);
ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_Auto_Vacuum(Btree *, ਪੂਰਨ);
ਪੂਰਨ sqlite3BtreeGetAutoVacuum(Btree *);
ਪੂਰਨ sqlite3BtreeBeginTrans(Btree*,ਪੂਰਨ,ਪੂਰਨ*);
ਪੂਰਨ sqlite3BtreeCommitPhaseOne(Btree*, ਅਚਲ ਅੱਖਰ*);
ਪੂਰਨ sqlite3BtreeCommitPhaseTwo(Btree*, ਪੂਰਨ);
ਪੂਰਨ sqlite3BtreeCommit(Btree*);
ਪੂਰਨ sqlite3BtreeRollback(Btree*,ਪੂਰਨ,ਪੂਰਨ);
ਪੂਰਨ sqlite3BtreeBeginStmt(Btree*,ਪੂਰਨ);
ਪੂਰਨ sqlite3_Btree_Create_ਸਾਰਣੀ(Btree*, Pgno*, ਪੂਰਨ flags);
ਪੂਰਨ sqlite3BtreeTxnState(Btree*);
ਪੂਰਨ sqlite3_Btree_Is_ਵਿੱਚ_Backup(Btree*);

ਖਾਲੀ *sqlite3BtreeSchema(Btree *, ਪੂਰਨ, ਖਾਲੀ(*)(ਖਾਲੀ *));
ਪੂਰਨ sqlite3BtreeSchemaLocked(Btree *pBtree);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
ਪੂਰਨ sqlite3_Btree_ਤਾਲਾ_ਸਾਰਣੀ(Btree *pBtree, ਪੂਰਨ iTab, u8 is_Write_ਤਾਲਾ);
#ਜੇ_ਅੰਤ

/* Savepoints are named, nestable SQL transactions mostly implemented */ 
/* in vdbe.c and pager.c See https://sqlite.org/lang_savepoint.html */
ਪੂਰਨ sqlite3BtreeSavepoint(Btree *, ਪੂਰਨ, ਪੂਰਨ);

/* "Checkpoint" only refers to WAL. See https://sqlite.org/wal.html#ckpt */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  ਪੂਰਨ sqlite3BtreeCheckpoint(Btree*, ਪੂਰਨ, ਪੂਰਨ *, ਪੂਰਨ *);  
#ਜੇ_ਅੰਤ

ਅਚਲ ਅੱਖਰ *sqlite3BtreeGetFilename(Btree *);
ਅਚਲ ਅੱਖਰ *sqlite3BtreeGetJournalname(Btree *);
ਪੂਰਨ sqlite3BtreeCopyFile(Btree*, Btree*);

ਪੂਰਨ sqlite3BtreeIncrVacuum(Btree *);

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_INTKEY     1    /* Table has only 64-bit signed integer keys */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_BLOBKEY    2    /* Table has keys only - no data */

ਪੂਰਨ sqlite3_Btree_Drop_ਸਾਰਣੀ(Btree*, ਪੂਰਨ, ਪੂਰਨ*);
ਪੂਰਨ sqlite3_Btree_Clear_ਸਾਰਣੀ(Btree*, ਪੂਰਨ, i64*);
ਪੂਰਨ sqlite3_Btree_Clear_ਸਾਰਣੀ_ਦਾ_Cursor(BtCursor*);
ਪੂਰਨ sqlite3BtreeTripAllCursors(Btree*, ਪੂਰਨ, ਪੂਰਨ);

ਖਾਲੀ sqlite3BtreeGetMeta(Btree *pBtree, ਪੂਰਨ idx, u32 *p_ਮੁੱਲ);
ਪੂਰਨ sqlite3BtreeUpdateMeta(Btree*, ਪੂਰਨ idx, u32 ਮੁੱਲ);

ਪੂਰਨ sqlite3BtreeNewDb(Btree *p);

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_FREE_ਪੰਨਾ_ਗਿਣਤੀ     0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_SCHEMA_ਸੰਸਕਰਣ      1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_FILE_FORMAT         2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_DEFAULT_CACHE_ਮਾਪ  3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_ਸਭ_ਤੋਂ_ਵੱਡਾ_ਜੜ੍ਹ_ਪੰਨਾ   4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_ਪਾਠ_ENCODING       5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_USER_ਸੰਸਕਰਣ        6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_INCR_VACUUM         7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_APPLICATION_ID      8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_ਅੰਕੜੇ_ਸੰਸਕਰਣ        15  /* A virtual meta-value */

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_HINT_RANGE 0       /* Range constraints on queries */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_HINT_TABLECURSOR 1 /* Table csr associated with this index csr */

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_BULKLOAD 0x00000001  /* Used to full index in sorted order */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_SEEK_EQ  0x00000002  /* EQ seeks only - no range seeks */

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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_WRCSR     0x00000004     /* read-write cursor */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_FORDELETE 0x00000008     /* Cursor is for seek/delete only */

ਪੂਰਨ sqlite3BtreeCursor(
  Btree*,                              /* BTree containing table to open */
  Pgno i_ਸਾਰਣੀ,                         /* Index of root page */
  ਪੂਰਨ wrFlag,                          /* 1 for writing.  0 for read-only */
  ਬਣਤਰ ਕੁੰਜੀ_Info*,                     /* First argument to compare function */
  BtCursor *pCursor                    /* Space to write cursor structure */
);
BtCursor *sqlite3BtreeFakeValidCursor(ਖਾਲੀ);
ਪੂਰਨ sqlite3_Btree_Cursor_ਮਾਪ(ਖਾਲੀ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
ਪੂਰਨ sqlite3BtreeClosesWithCursor(Btree*,BtCursor*);
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3BtreeCursorZero(BtCursor*);
ਖਾਲੀ sqlite3BtreeCursorHintFlags(BtCursor*, ਬਿਨਾਨਿਸ਼ਾਨੀ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_CURSOR_HINTS
ਖਾਲੀ sqlite3BtreeCursorHint(BtCursor*, ਪੂਰਨ, ...);
 #ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
 BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor*);
 #ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

ਪੂਰਨ sqlite3BtreeCloseCursor(BtCursor*);
ਪੂਰਨ sqlite3_Btree_ਸਾਰਣੀ_Moveto(
  BtCursor*,
  i64 int_ਕੁੰਜੀ,
  ਪੂਰਨ bias,
  ਪੂਰਨ *pRes
);
ਪੂਰਨ sqlite3_Btree_ਸੂਚਕਾਂਕ_Moveto(
  BtCursor*,
  Unpacked_ਰਿਕਾਰਡ *p_Un_ਕੁੰਜੀ,
  ਪੂਰਨ *pRes
);
ਪੂਰਨ sqlite3BtreeCursorHasMoved(BtCursor*);
ਪੂਰਨ sqlite3BtreeCursorRestore(BtCursor*, ਪੂਰਨ*);
ਪੂਰਨ sqlite3BtreeDelete(BtCursor*, u8 flags);

/* Allowed flags for sqlite3BtreeDelete() and sqlite3BtreeInsert() */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_SAVEPOSITION 0x02  /* Leave cursor pointing at NEXT or PREV */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_AUXDELETE    0x04  /* not the primary delete operation */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_APPEND       0x08  /* Insert is likely an append */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTREE_PREFORMAT    0x80  /* Inserted data is a preformated cell */

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
ਬਣਤਰ BtreePayload {
  ਅਚਲ ਖਾਲੀ *p_ਕੁੰਜੀ;       /* Key content for indexes.  NULL for tables */
  sqlite3_int64 n_ਕੁੰਜੀ;     /* Size of pKey for indexes.  PRIMARY KEY for tabs */
  ਅਚਲ ਖਾਲੀ *p_ਅੰਕੜੇ;      /* Data for tables. */
  sqlite3_ਮੁੱਲ *aMem;    /* First of nMem value in the unpacked pKey */
  u16 nMem;               /* Number of aMem[] value.  Might be zero */
  ਪੂਰਨ n_ਅੰਕੜੇ;              /* Size of pData.  0 if none. */
  ਪੂਰਨ nZero;              /* Extra zero data appended after pData,nData */
};

ਪੂਰਨ sqlite3_Btree_ਪਾਓ(BtCursor*, ਅਚਲ BtreePayload *pPayload,
                       ਪੂਰਨ flags, ਪੂਰਨ seek_ਨਤੀਜਾ);
ਪੂਰਨ sqlite3_Btree_ਪਹਿਲਾ(BtCursor*, ਪੂਰਨ *pRes);
ਪੂਰਨ sqlite3_Btree_Is_ਖਾਲੀ_ਥਾਵਾਂ(BtCursor *pCur, ਪੂਰਨ *pRes);
ਪੂਰਨ sqlite3_Btree_ਆਖਰੀ(BtCursor*, ਪੂਰਨ *pRes);
ਪੂਰਨ sqlite3_Btree_ਅਗਲਾ(BtCursor*, ਪੂਰਨ flags);
ਪੂਰਨ sqlite3BtreeEof(BtCursor*);
ਪੂਰਨ sqlite3_Btree_ਪਿਛਲਾ(BtCursor*, ਪੂਰਨ flags);
i64 sqlite3_Btree_Integer_ਕੁੰਜੀ(BtCursor*);
ਖਾਲੀ sqlite3BtreeCursorPin(BtCursor*);
ਖਾਲੀ sqlite3BtreeCursorUnpin(BtCursor*);
i64 sqlite3_Btree_ਵਿੱਥ(BtCursor*);
ਪੂਰਨ sqlite3BtreePayload(BtCursor*, u32 ਵਿੱਥ, u32 amt, ਖਾਲੀ*);
ਅਚਲ ਖਾਲੀ *sqlite3BtreePayloadFetch(BtCursor*, u32 *pAmt);
u32 sqlite3_Btree_Payload_ਮਾਪ(BtCursor*);
sqlite3_int64 sqlite3_Btree_Max_ਰਿਕਾਰਡ_ਮਾਪ(BtCursor*);

ਪੂਰਨ sqlite3BtreeIntegrityCheck(
  sqlite3 *db,  /* Database connection that is running the check */
  Btree *p,     /* The btree to be checked */
  Pgno *a_ਜੜ੍ਹ,  /* An array of root pages numbers for individual trees */
  sqlite3_ਮੁੱਲ *aCnt,  /* OUT: entry counts for each btree in aRoot[] */
  ਪੂਰਨ n_ਜੜ੍ਹ,    /* Number of entries in aRoot[] */
  ਪੂਰਨ mxErr,    /* Stop reporting errors after this many */
  ਪੂਰਨ *pnErr,   /* OUT: Write number of errors seen to this variable */
  ਅੱਖਰ **pz_ਬਾਹਰਲਾ  /* OUT: Write the error message string here */
);
ਬਣਤਰ Pager *sqlite3BtreePager(Btree*);
i64 sqlite3_Btree_ਪੰਕਤੀ_ਗਿਣਤੀ_Est(BtCursor*);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_INCRBLOB
ਪੂਰਨ sqlite3BtreePayloadChecked(BtCursor*, u32 ਵਿੱਥ, u32 amt, ਖਾਲੀ*);
ਪੂਰਨ sqlite3_Btree_Put_ਅੰਕੜੇ(BtCursor*, u32 ਵਿੱਥ, u32 amt, ਖਾਲੀ*);
ਖਾਲੀ sqlite3BtreeIncrblobCursor(BtCursor *);
#ਜੇ_ਅੰਤ
ਖਾਲੀ sqlite3BtreeClearCursor(BtCursor *);
ਪੂਰਨ sqlite3_Btree_ਸੈੱਟ_ਸੰਸਕਰਣ(Btree *pBt, ਪੂਰਨ i_ਸੰਸਕਰਣ);
ਪੂਰਨ sqlite3BtreeCursorHasHint(BtCursor*, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਨਕਾਬ);
ਪੂਰਨ sqlite3BtreeIsReadonly(Btree *pBt);
ਪੂਰਨ sqlite3_ਸਿਰਲੇਖ_ਮਾਪ_Btree(ਖਾਲੀ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
sqlite3_uint64 sqlite3_Btree_Seek_ਗਿਣਤੀ(Btree*);
#ਨਹੀਂਤਾਂ
# define sqlite3_Btree_Seek_ਗਿਣਤੀ(X) 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
ਪੂਰਨ sqlite3BtreeCursorIsValid(BtCursor*);
#ਜੇ_ਅੰਤ
ਪੂਰਨ sqlite3BtreeCursorIsValidNN(BtCursor*);

ਪੂਰਨ sqlite3_Btree_ਗਿਣਤੀ(sqlite3*, BtCursor*, i64*);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST
ਪੂਰਨ sqlite3BtreeCursorInfo(BtCursor*, ਪੂਰਨ*, ਪੂਰਨ);
ਖਾਲੀ sqlite3BtreeCursorList(Btree*);
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WAL
  ਪੂਰਨ sqlite3BtreeCheckpoint(Btree*, ਪੂਰਨ, ਪੂਰਨ *, ਪੂਰਨ *);
#ਜੇ_ਅੰਤ

ਪੂਰਨ sqlite3_Btree_Transfer_ਪੰਕਤੀ(BtCursor*, BtCursor*, i64);

ਖਾਲੀ sqlite3BtreeClearCache(Btree*);

/*
** If we are not using shared cache, then there is no need to
** use mutexes to access the BtShared structures.  So make the
** Enter and Leave procedures no-ops.
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SHARED_CACHE
  ਖਾਲੀ sqlite3BtreeEnter(Btree*);
  ਖਾਲੀ sqlite3BtreeEnterAll(sqlite3*);
  ਪੂਰਨ sqlite3BtreeSharable(Btree*);
  ਖਾਲੀ sqlite3BtreeEnterCursor(BtCursor*);
  ਪੂਰਨ sqlite3_Btree_Connection_ਗਿਣਤੀ(Btree*);
#ਨਹੀਂਤਾਂ
# define sqlite3BtreeEnter(X) 
# define sqlite3BtreeEnterAll(X)
# define sqlite3BtreeSharable(X) 0
# define sqlite3BtreeEnterCursor(X)
# define sqlite3_Btree_Connection_ਗਿਣਤੀ(X) 1
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE
  ਖਾਲੀ sqlite3BtreeLeave(Btree*);
  ਖਾਲੀ sqlite3BtreeLeaveCursor(BtCursor*);
  ਖਾਲੀ sqlite3BtreeLeaveAll(sqlite3*);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  /* These routines are used inside assert() statements only. */
  ਪੂਰਨ sqlite3BtreeHoldsMutex(Btree*);
  ਪੂਰਨ sqlite3BtreeHoldsAllMutexes(sqlite3*);
  ਪੂਰਨ sqlite3SchemaMutexHeld(sqlite3*,ਪੂਰਨ,Schema*);
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ

# define sqlite3BtreeLeave(X)
# define sqlite3BtreeLeaveCursor(X)
# define sqlite3BtreeLeaveAll(X)

# define sqlite3BtreeHoldsMutex(X) 1
# define sqlite3BtreeHoldsAllMutexes(X) 1
# define sqlite3SchemaMutexHeld(X,Y,Z) 1
#ਜੇ_ਅੰਤ


#ਜੇ_ਅੰਤ /* SQLITE_BTREE_H */
