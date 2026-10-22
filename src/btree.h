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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_BTREE_H
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_BTREE_H

/* TODO: This definition is just included so other modules compile. It
** needs to be revisited.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_N_BTREE_META 16

/*
** If defined as non-zero, auto-vacuum is enabled by default. Otherwise
** it must be turned on for each database using "PRAGMA auto_vacuum = 1".
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_DEFAULT_AUTOVACUUM
  #ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_DEFAULT_AUTOVACUUM 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_AUTOVACUUM_NONE 0        /* Do not do auto-vacuum */
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_AUTOVACUUM_ತುಂಬಿದ 1        /* Do full auto-vacuum */
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_AUTOVACUUM_INCR 2        /* Incremental vacuum */

/*
** Forward declarations of structure
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Btree Btree;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ BtCursor BtCursor;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ BtShared BtShared;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ BtreePayload BtreePayload;


ಪೂರ್ಣಾಂಕ sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,       /* VFS to use with this b-tree */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename,   /* Name of database file to open */
  sqlite3 *db,             /* Associated database connection */
  Btree **ppBtree,         /* Return open Btree* here */
  ಪೂರ್ಣಾಂಕ flags,               /* Flags */
  ಪೂರ್ಣಾಂಕ vfsFlags             /* Flags passed through to VFS open */
);

/* The flags parameter to sqlite3BtreeOpen can be the bitwise or of the
** following values.
**
** NOTE:  These values must match the corresponding PAGER_ values in
** pager.h.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_OMIT_JOURNAL  1  /* Do not create or use a rollback journal */
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_ಸ್ಮರಣೆ        2  /* This is an in-memory DB */
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_SINGLE        4  /* The file contains at most 1 b-tree */
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_UNORDERED     8  /* Use of a hash implementation is OK */

ಪೂರ್ಣಾಂಕ sqlite3BtreeClose(Btree*);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_Cache_ಗಾತ್ರ(Btree*,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_Spill_ಗಾತ್ರ(Btree*,ಪೂರ್ಣಾಂಕ);
#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
  ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_Mmap_ಎಲ್ಲೆ(Btree*,sqlite3_int64);
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_Pager_Flags(Btree*,ಚಿಹ್ನೆರಹಿತ);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_ಪುಟ_ಗಾತ್ರ(Btree *p, ಪೂರ್ಣಾಂಕ nPagesize, ಪೂರ್ಣಾಂಕ nReserve, ಪೂರ್ಣಾಂಕ eFix);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(Btree*);
Pgno sqlite3_Btree_Max_ಪುಟ_ಎಣಿಕೆ(Btree*,Pgno);
Pgno sqlite3_Btree_ಕೊನೆಯ_ಪುಟ(Btree*);
ಪೂರ್ಣಾಂಕ sqlite3BtreeSecureDelete(Btree*,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3BtreeGetRequestedReserve(Btree*);
ಪೂರ್ಣಾಂಕ sqlite3BtreeGetReserveNoMutex(Btree *p);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_Auto_Vacuum(Btree *, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3BtreeGetAutoVacuum(Btree *);
ಪೂರ್ಣಾಂಕ sqlite3BtreeBeginTrans(Btree*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ*);
ಪೂರ್ಣಾಂಕ sqlite3BtreeCommitPhaseOne(Btree*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*);
ಪೂರ್ಣಾಂಕ sqlite3BtreeCommitPhaseTwo(Btree*, ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3BtreeCommit(Btree*);
ಪೂರ್ಣಾಂಕ sqlite3BtreeRollback(Btree*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3BtreeBeginStmt(Btree*,ಪೂರ್ಣಾಂಕ);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Create_ಕೋಷ್ಟಕ(Btree*, Pgno*, ಪೂರ್ಣಾಂಕ flags);
ಪೂರ್ಣಾಂಕ sqlite3BtreeTxnState(Btree*);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Is_ಒಳ_Backup(Btree*);

ಶೂನ್ಯ *sqlite3BtreeSchema(Btree *, ಪೂರ್ಣಾಂಕ, ಶೂನ್ಯ(*)(ಶೂನ್ಯ *));
ಪೂರ್ಣಾಂಕ sqlite3BtreeSchemaLocked(Btree *pBtree);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಬೀಗ_ಕೋಷ್ಟಕ(Btree *pBtree, ಪೂರ್ಣಾಂಕ iTab, u8 is_Write_ಬೀಗ);
#ಆದರೆ_ಅಂತ್ಯ

/* Savepoints are named, nestable SQL transactions mostly implemented */ 
/* in vdbe.c and pager.c See https://sqlite.org/lang_savepoint.html */
ಪೂರ್ಣಾಂಕ sqlite3BtreeSavepoint(Btree *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);

/* "Checkpoint" only refers to WAL. See https://sqlite.org/wal.html#ckpt */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  ಪೂರ್ಣಾಂಕ sqlite3BtreeCheckpoint(Btree*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ *, ಪೂರ್ಣಾಂಕ *);  
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3BtreeGetFilename(Btree *);
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3BtreeGetJournalname(Btree *);
ಪೂರ್ಣಾಂಕ sqlite3BtreeCopyFile(Btree*, Btree*);

ಪೂರ್ಣಾಂಕ sqlite3BtreeIncrVacuum(Btree *);

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
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_INTKEY     1    /* Table has only 64-bit signed integer keys */
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_BLOBKEY    2    /* Table has keys only - no data */

ಪೂರ್ಣಾಂಕ sqlite3_Btree_Drop_ಕೋಷ್ಟಕ(Btree*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ*);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Clear_ಕೋಷ್ಟಕ(Btree*, ಪೂರ್ಣಾಂಕ, i64*);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Clear_ಕೋಷ್ಟಕ_ದ_Cursor(BtCursor*);
ಪೂರ್ಣಾಂಕ sqlite3BtreeTripAllCursors(Btree*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);

ಶೂನ್ಯ sqlite3BtreeGetMeta(Btree *pBtree, ಪೂರ್ಣಾಂಕ idx, u32 *p_ಮೌಲ್ಯ);
ಪೂರ್ಣಾಂಕ sqlite3BtreeUpdateMeta(Btree*, ಪೂರ್ಣಾಂಕ idx, u32 ಮೌಲ್ಯ);

ಪೂರ್ಣಾಂಕ sqlite3BtreeNewDb(Btree *p);

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
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_FREE_ಪುಟ_ಎಣಿಕೆ     0
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_SCHEMA_ಆವೃತ್ತಿ      1
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_FILE_FORMAT         2
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_DEFAULT_CACHE_ಗಾತ್ರ  3
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_ಅತಿದೊಡ್ಡ_ಬೇರು_ಪುಟ   4
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_ಪಠ್ಯ_ENCODING       5
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_USER_ಆವೃತ್ತಿ        6
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_INCR_VACUUM         7
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_APPLICATION_ID      8
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_ದತ್ತಾಂಶ_ಆವೃತ್ತಿ        15  /* A virtual meta-value */

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
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_HINT_RANGE 0       /* Range constraints on queries */
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_HINT_TABLECURSOR 1 /* Table csr associated with this index csr */

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
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_BULKLOAD 0x00000001  /* Used to full index in sorted order */
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_SEEK_EQ  0x00000002  /* EQ seeks only - no range seeks */

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
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_WRCSR     0x00000004     /* read-write cursor */
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_FORDELETE 0x00000008     /* Cursor is for seek/delete only */

ಪೂರ್ಣಾಂಕ sqlite3BtreeCursor(
  Btree*,                              /* BTree containing table to open */
  Pgno i_ಕೋಷ್ಟಕ,                         /* Index of root page */
  ಪೂರ್ಣಾಂಕ wrFlag,                          /* 1 for writing.  0 for read-only */
  ರಚನೆ ಕೀಲಿ_Info*,                     /* First argument to compare function */
  BtCursor *pCursor                    /* Space to write cursor structure */
);
BtCursor *sqlite3BtreeFakeValidCursor(ಶೂನ್ಯ);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Cursor_ಗಾತ್ರ(ಶೂನ್ಯ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
ಪೂರ್ಣಾಂಕ sqlite3BtreeClosesWithCursor(Btree*,BtCursor*);
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3BtreeCursorZero(BtCursor*);
ಶೂನ್ಯ sqlite3BtreeCursorHintFlags(BtCursor*, ಚಿಹ್ನೆರಹಿತ);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_CURSOR_HINTS
ಶೂನ್ಯ sqlite3BtreeCursorHint(BtCursor*, ಪೂರ್ಣಾಂಕ, ...);
 #ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
 BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor*);
 #ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ sqlite3BtreeCloseCursor(BtCursor*);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಕೋಷ್ಟಕ_Moveto(
  BtCursor*,
  i64 int_ಕೀಲಿ,
  ಪೂರ್ಣಾಂಕ bias,
  ಪೂರ್ಣಾಂಕ *pRes
);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಸೂಚ್ಯಂಕ_Moveto(
  BtCursor*,
  Unpacked_ದಾಖಲೆ *p_Un_ಕೀಲಿ,
  ಪೂರ್ಣಾಂಕ *pRes
);
ಪೂರ್ಣಾಂಕ sqlite3BtreeCursorHasMoved(BtCursor*);
ಪೂರ್ಣಾಂಕ sqlite3BtreeCursorRestore(BtCursor*, ಪೂರ್ಣಾಂಕ*);
ಪೂರ್ಣಾಂಕ sqlite3BtreeDelete(BtCursor*, u8 flags);

/* Allowed flags for sqlite3BtreeDelete() and sqlite3BtreeInsert() */
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_SAVEPOSITION 0x02  /* Leave cursor pointing at NEXT or PREV */
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_AUXDELETE    0x04  /* not the primary delete operation */
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_APPEND       0x08  /* Insert is likely an append */
#ವ್ಯಾಖ್ಯಾನಿಸು BTREE_PREFORMAT    0x80  /* Inserted data is a preformated cell */

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
ರಚನೆ BtreePayload {
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p_ಕೀಲಿ;       /* Key content for indexes.  NULL for tables */
  sqlite3_int64 n_ಕೀಲಿ;     /* Size of pKey for indexes.  PRIMARY KEY for tabs */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p_ದತ್ತಾಂಶ;      /* Data for tables. */
  sqlite3_ಮೌಲ್ಯ *aMem;    /* First of nMem value in the unpacked pKey */
  u16 nMem;               /* Number of aMem[] value.  Might be zero */
  ಪೂರ್ಣಾಂಕ n_ದತ್ತಾಂಶ;              /* Size of pData.  0 if none. */
  ಪೂರ್ಣಾಂಕ nZero;              /* Extra zero data appended after pData,nData */
};

ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಸೇರಿಸು(BtCursor*, ಸ್ಥಿರಾಂಕ BtreePayload *pPayload,
                       ಪೂರ್ಣಾಂಕ flags, ಪೂರ್ಣಾಂಕ seek_ಫಲಿತಾಂಶ);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಮೊದಲ(BtCursor*, ಪೂರ್ಣಾಂಕ *pRes);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Is_ಖಾಲಿಗಳು(BtCursor *pCur, ಪೂರ್ಣಾಂಕ *pRes);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಕೊನೆಯ(BtCursor*, ಪೂರ್ಣಾಂಕ *pRes);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಮುಂದಿನ(BtCursor*, ಪೂರ್ಣಾಂಕ flags);
ಪೂರ್ಣಾಂಕ sqlite3BtreeEof(BtCursor*);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಹಿಂದಿನ(BtCursor*, ಪೂರ್ಣಾಂಕ flags);
i64 sqlite3_Btree_Integer_ಕೀಲಿ(BtCursor*);
ಶೂನ್ಯ sqlite3BtreeCursorPin(BtCursor*);
ಶೂನ್ಯ sqlite3BtreeCursorUnpin(BtCursor*);
i64 sqlite3_Btree_ಸ್ಥಾನಾಂತರ(BtCursor*);
ಪೂರ್ಣಾಂಕ sqlite3BtreePayload(BtCursor*, u32 ಸ್ಥಾನಾಂತರ, u32 amt, ಶೂನ್ಯ*);
ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *sqlite3BtreePayloadFetch(BtCursor*, u32 *pAmt);
u32 sqlite3_Btree_Payload_ಗಾತ್ರ(BtCursor*);
sqlite3_int64 sqlite3_Btree_Max_ದಾಖಲೆ_ಗಾತ್ರ(BtCursor*);

ಪೂರ್ಣಾಂಕ sqlite3BtreeIntegrityCheck(
  sqlite3 *db,  /* Database connection that is running the check */
  Btree *p,     /* The btree to be checked */
  Pgno *a_ಬೇರು,  /* An array of root pages numbers for individual trees */
  sqlite3_ಮೌಲ್ಯ *aCnt,  /* OUT: entry counts for each btree in aRoot[] */
  ಪೂರ್ಣಾಂಕ n_ಬೇರು,    /* Number of entries in aRoot[] */
  ಪೂರ್ಣಾಂಕ mxErr,    /* Stop reporting errors after this many */
  ಪೂರ್ಣಾಂಕ *pnErr,   /* OUT: Write number of errors seen to this variable */
  ಅಕ್ಷರ **pz_ಹೊರಹೋಗು  /* OUT: Write the error message string here */
);
ರಚನೆ Pager *sqlite3BtreePager(Btree*);
i64 sqlite3_Btree_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ_Est(BtCursor*);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_INCRBLOB
ಪೂರ್ಣಾಂಕ sqlite3BtreePayloadChecked(BtCursor*, u32 ಸ್ಥಾನಾಂತರ, u32 amt, ಶೂನ್ಯ*);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_Put_ದತ್ತಾಂಶ(BtCursor*, u32 ಸ್ಥಾನಾಂತರ, u32 amt, ಶೂನ್ಯ*);
ಶೂನ್ಯ sqlite3BtreeIncrblobCursor(BtCursor *);
#ಆದರೆ_ಅಂತ್ಯ
ಶೂನ್ಯ sqlite3BtreeClearCursor(BtCursor *);
ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಗಣ_ಆವೃತ್ತಿ(Btree *pBt, ಪೂರ್ಣಾಂಕ i_ಆವೃತ್ತಿ);
ಪೂರ್ಣಾಂಕ sqlite3BtreeCursorHasHint(BtCursor*, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ);
ಪೂರ್ಣಾಂಕ sqlite3BtreeIsReadonly(Btree *pBt);
ಪೂರ್ಣಾಂಕ sqlite3_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ_Btree(ಶೂನ್ಯ);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
sqlite3_uint64 sqlite3_Btree_Seek_ಎಣಿಕೆ(Btree*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3_Btree_Seek_ಎಣಿಕೆ(X) 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
ಪೂರ್ಣಾಂಕ sqlite3BtreeCursorIsValid(BtCursor*);
#ಆದರೆ_ಅಂತ್ಯ
ಪೂರ್ಣಾಂಕ sqlite3BtreeCursorIsValidNN(BtCursor*);

ಪೂರ್ಣಾಂಕ sqlite3_Btree_ಎಣಿಕೆ(sqlite3*, BtCursor*, i64*);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST
ಪೂರ್ಣಾಂಕ sqlite3BtreeCursorInfo(BtCursor*, ಪೂರ್ಣಾಂಕ*, ಪೂರ್ಣಾಂಕ);
ಶೂನ್ಯ sqlite3BtreeCursorList(Btree*);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WAL
  ಪೂರ್ಣಾಂಕ sqlite3BtreeCheckpoint(Btree*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ *, ಪೂರ್ಣಾಂಕ *);
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ sqlite3_Btree_Transfer_ಅಡ್ಡಸಾಲು(BtCursor*, BtCursor*, i64);

ಶೂನ್ಯ sqlite3BtreeClearCache(Btree*);

/*
** If we are not using shared cache, then there is no need to
** use mutexes to access the BtShared structures.  So make the
** Enter and Leave procedures no-ops.
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SHARED_CACHE
  ಶೂನ್ಯ sqlite3BtreeEnter(Btree*);
  ಶೂನ್ಯ sqlite3BtreeEnterAll(sqlite3*);
  ಪೂರ್ಣಾಂಕ sqlite3BtreeSharable(Btree*);
  ಶೂನ್ಯ sqlite3BtreeEnterCursor(BtCursor*);
  ಪೂರ್ಣಾಂಕ sqlite3_Btree_Connection_ಎಣಿಕೆ(Btree*);
#ಇಲ್ಲದಿದ್ದರೆ
# define sqlite3BtreeEnter(X) 
# define sqlite3BtreeEnterAll(X)
# define sqlite3BtreeSharable(X) 0
# define sqlite3BtreeEnterCursor(X)
# define sqlite3_Btree_Connection_ಎಣಿಕೆ(X) 1
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE
  ಶೂನ್ಯ sqlite3BtreeLeave(Btree*);
  ಶೂನ್ಯ sqlite3BtreeLeaveCursor(BtCursor*);
  ಶೂನ್ಯ sqlite3BtreeLeaveAll(sqlite3*);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  /* These routines are used inside assert() statements only. */
  ಪೂರ್ಣಾಂಕ sqlite3BtreeHoldsMutex(Btree*);
  ಪೂರ್ಣಾಂಕ sqlite3BtreeHoldsAllMutexes(sqlite3*);
  ಪೂರ್ಣಾಂಕ sqlite3SchemaMutexHeld(sqlite3*,ಪೂರ್ಣಾಂಕ,Schema*);
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ

# define sqlite3BtreeLeave(X)
# define sqlite3BtreeLeaveCursor(X)
# define sqlite3BtreeLeaveAll(X)

# define sqlite3BtreeHoldsMutex(X) 1
# define sqlite3BtreeHoldsAllMutexes(X) 1
# define sqlite3SchemaMutexHeld(X,Y,Z) 1
#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_BTREE_H */
