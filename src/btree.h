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
#यदि_अपरिभाषितम् SQLITE_BTREE_H
#परिभाषय SQLITE_BTREE_H

/* TODO: This definition is just included so other modules compile. It
** needs to be revisited.
*/
#परिभाषय SQLITE_N_BTREE_META 16

/*
** If defined as non-zero, auto-vacuum is enabled by default. Otherwise
** it must be turned on for each database using "PRAGMA auto_vacuum = 1".
*/
#यदि_अपरिभाषितम् SQLITE_DEFAULT_AUTOVACUUM
  #परिभाषय SQLITE_DEFAULT_AUTOVACUUM 0
#यदि_अन्तः

#परिभाषय BTREE_AUTOVACUUM_NONE 0        /* Do not do auto-vacuum */
#परिभाषय BTREE_AUTOVACUUM_पूर्णम् 1        /* Do full auto-vacuum */
#परिभाषय BTREE_AUTOVACUUM_INCR 2        /* Incremental vacuum */

/*
** Forward declarations of structure
*/
प्रकारनिर्देश संरचना Btree Btree;
प्रकारनिर्देश संरचना BtCursor BtCursor;
प्रकारनिर्देश संरचना BtShared BtShared;
प्रकारनिर्देश संरचना BtreePayload BtreePayload;


पूर्णाङ्क sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,       /* VFS to use with this b-tree */
  ध्रुव वर्ण *zFilename,   /* Name of database file to open */
  sqlite3 *db,             /* Associated database connection */
  Btree **ppBtree,         /* Return open Btree* here */
  पूर्णाङ्क flags,               /* Flags */
  पूर्णाङ्क vfsFlags             /* Flags passed through to VFS open */
);

/* The flags parameter to sqlite3BtreeOpen can be the bitwise or of the
** following values.
**
** NOTE:  These values must match the corresponding PAGER_ values in
** pager.h.
*/
#परिभाषय BTREE_OMIT_JOURNAL  1  /* Do not create or use a rollback journal */
#परिभाषय BTREE_स्मृतिः        2  /* This is an in-memory DB */
#परिभाषय BTREE_SINGLE        4  /* The file contains at most 1 b-tree */
#परिभाषय BTREE_UNORDERED     8  /* Use of a hash implementation is OK */

पूर्णाङ्क sqlite3BtreeClose(Btree*);
पूर्णाङ्क sqlite3_Btree_समुच्चयः_Cache_आकार(Btree*,पूर्णाङ्क);
पूर्णाङ्क sqlite3_Btree_समुच्चयः_Spill_आकार(Btree*,पूर्णाङ्क);
#यदि SQLITE_MAX_MMAP_आकार>0
  पूर्णाङ्क sqlite3_Btree_समुच्चयः_Mmap_सीमा(Btree*,sqlite3_int64);
#यदि_अन्तः
पूर्णाङ्क sqlite3_Btree_समुच्चयः_Pager_Flags(Btree*,अचिह्नित);
पूर्णाङ्क sqlite3_Btree_समुच्चयः_Page_आकार(Btree *p, पूर्णाङ्क nPagesize, पूर्णाङ्क nReserve, पूर्णाङ्क eFix);
पूर्णाङ्क sqlite3_Btree_Get_Page_आकार(Btree*);
Pgno sqlite3_Btree_Max_Page_गणनम्(Btree*,Pgno);
Pgno sqlite3_Btree_चरमः_Page(Btree*);
पूर्णाङ्क sqlite3BtreeSecureDelete(Btree*,पूर्णाङ्क);
पूर्णाङ्क sqlite3BtreeGetRequestedReserve(Btree*);
पूर्णाङ्क sqlite3BtreeGetReserveNoMutex(Btree *p);
पूर्णाङ्क sqlite3_Btree_समुच्चयः_Auto_Vacuum(Btree *, पूर्णाङ्क);
पूर्णाङ्क sqlite3BtreeGetAutoVacuum(Btree *);
पूर्णाङ्क sqlite3BtreeBeginTrans(Btree*,पूर्णाङ्क,पूर्णाङ्क*);
पूर्णाङ्क sqlite3BtreeCommitPhaseOne(Btree*, ध्रुव वर्ण*);
पूर्णाङ्क sqlite3BtreeCommitPhaseTwo(Btree*, पूर्णाङ्क);
पूर्णाङ्क sqlite3BtreeCommit(Btree*);
पूर्णाङ्क sqlite3BtreeRollback(Btree*,पूर्णाङ्क,पूर्णाङ्क);
पूर्णाङ्क sqlite3BtreeBeginStmt(Btree*,पूर्णाङ्क);
पूर्णाङ्क sqlite3_Btree_Create_सारणी(Btree*, Pgno*, पूर्णाङ्क flags);
पूर्णाङ्क sqlite3BtreeTxnState(Btree*);
पूर्णाङ्क sqlite3_Btree_Is_अन्तर्_Backup(Btree*);

शून्य *sqlite3BtreeSchema(Btree *, पूर्णाङ्क, शून्य(*)(शून्य *));
पूर्णाङ्क sqlite3BtreeSchemaLocked(Btree *pBtree);
#यदि_अपरिभाषितम् SQLITE_OMIT_SHARED_CACHE
पूर्णाङ्क sqlite3_Btree_तालकम्_सारणी(Btree *pBtree, पूर्णाङ्क iTab, u8 is_Write_तालकम्);
#यदि_अन्तः

/* Savepoints are named, nestable SQL transactions mostly implemented */ 
/* in vdbe.c and pager.c See https://sqlite.org/lang_savepoint.html */
पूर्णाङ्क sqlite3BtreeSavepoint(Btree *, पूर्णाङ्क, पूर्णाङ्क);

/* "Checkpoint" only refers to WAL. See https://sqlite.org/wal.html#ckpt */
#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
  पूर्णाङ्क sqlite3BtreeCheckpoint(Btree*, पूर्णाङ्क, पूर्णाङ्क *, पूर्णाङ्क *);  
#यदि_अन्तः

ध्रुव वर्ण *sqlite3BtreeGetFilename(Btree *);
ध्रुव वर्ण *sqlite3BtreeGetJournalname(Btree *);
पूर्णाङ्क sqlite3BtreeCopyFile(Btree*, Btree*);

पूर्णाङ्क sqlite3BtreeIncrVacuum(Btree *);

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
#परिभाषय BTREE_INTKEY     1    /* Table has only 64-bit signed integer keys */
#परिभाषय BTREE_BLOBKEY    2    /* Table has keys only - no data */

पूर्णाङ्क sqlite3_Btree_Drop_सारणी(Btree*, पूर्णाङ्क, पूर्णाङ्क*);
पूर्णाङ्क sqlite3_Btree_Clear_सारणी(Btree*, पूर्णाङ्क, i64*);
पूर्णाङ्क sqlite3_Btree_Clear_सारणी_सम्बन्धि_Cursor(BtCursor*);
पूर्णाङ्क sqlite3BtreeTripAllCursors(Btree*, पूर्णाङ्क, पूर्णाङ्क);

शून्य sqlite3BtreeGetMeta(Btree *pBtree, पूर्णाङ्क idx, u32 *p_मानम्);
पूर्णाङ्क sqlite3BtreeUpdateMeta(Btree*, पूर्णाङ्क idx, u32 मानम्);

पूर्णाङ्क sqlite3BtreeNewDb(Btree *p);

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
#परिभाषय BTREE_FREE_PAGE_गणनम्     0
#परिभाषय BTREE_SCHEMA_संस्करणम्      1
#परिभाषय BTREE_FILE_FORMAT         2
#परिभाषय BTREE_DEFAULT_CACHE_आकार  3
#परिभाषय BTREE_LARGEST_मूलम्_PAGE   4
#परिभाषय BTREE_TEXT_ENCODING       5
#परिभाषय BTREE_USER_संस्करणम्        6
#परिभाषय BTREE_INCR_VACUUM         7
#परिभाषय BTREE_APPLICATION_ID      8
#परिभाषय BTREE_दत्त_संस्करणम्        15  /* A virtual meta-value */

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
#परिभाषय BTREE_HINT_RANGE 0       /* Range constraints on queries */
#परिभाषय BTREE_HINT_TABLECURSOR 1 /* Table csr associated with this index csr */

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
#परिभाषय BTREE_BULKLOAD 0x00000001  /* Used to full index in sorted order */
#परिभाषय BTREE_SEEK_EQ  0x00000002  /* EQ seeks only - no range seeks */

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
#परिभाषय BTREE_WRCSR     0x00000004     /* read-write cursor */
#परिभाषय BTREE_FORDELETE 0x00000008     /* Cursor is for seek/delete only */

पूर्णाङ्क sqlite3BtreeCursor(
  Btree*,                              /* BTree containing table to open */
  Pgno i_सारणी,                         /* Index of root page */
  पूर्णाङ्क wrFlag,                          /* 1 for writing.  0 for read-only */
  संरचना कुञ्चिका_Info*,                     /* First argument to compare function */
  BtCursor *pCursor                    /* Space to write cursor structure */
);
BtCursor *sqlite3BtreeFakeValidCursor(शून्य);
पूर्णाङ्क sqlite3_Btree_Cursor_आकार(शून्य);
#यदि_परिभाषितम् SQLITE_DEBUG
पूर्णाङ्क sqlite3BtreeClosesWithCursor(Btree*,BtCursor*);
#यदि_अन्तः
शून्य sqlite3BtreeCursorZero(BtCursor*);
शून्य sqlite3BtreeCursorHintFlags(BtCursor*, अचिह्नित);
#यदि_परिभाषितम् SQLITE_ENABLE_CURSOR_HINTS
शून्य sqlite3BtreeCursorHint(BtCursor*, पूर्णाङ्क, ...);
 #यदि_परिभाषितम् SQLITE_DEBUG
 BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor*);
 #यदि_अन्तः
#यदि_अन्तः

पूर्णाङ्क sqlite3BtreeCloseCursor(BtCursor*);
पूर्णाङ्क sqlite3_Btree_सारणी_Moveto(
  BtCursor*,
  i64 int_कुञ्चिका,
  पूर्णाङ्क bias,
  पूर्णाङ्क *pRes
);
पूर्णाङ्क sqlite3_Btree_सूचकांक_Moveto(
  BtCursor*,
  Unpacked_लेखः *p_Un_कुञ्चिका,
  पूर्णाङ्क *pRes
);
पूर्णाङ्क sqlite3BtreeCursorHasMoved(BtCursor*);
पूर्णाङ्क sqlite3BtreeCursorRestore(BtCursor*, पूर्णाङ्क*);
पूर्णाङ्क sqlite3BtreeDelete(BtCursor*, u8 flags);

/* Allowed flags for sqlite3BtreeDelete() and sqlite3BtreeInsert() */
#परिभाषय BTREE_SAVEPOSITION 0x02  /* Leave cursor pointing at NEXT or PREV */
#परिभाषय BTREE_AUXDELETE    0x04  /* not the primary delete operation */
#परिभाषय BTREE_APPEND       0x08  /* Insert is likely an append */
#परिभाषय BTREE_PREFORMAT    0x80  /* Inserted data is a preformated cell */

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
संरचना BtreePayload {
  ध्रुव शून्य *p_कुञ्चिका;       /* Key content for indexes.  NULL for tables */
  sqlite3_int64 n_कुञ्चिका;     /* Size of pKey for indexes.  PRIMARY KEY for tabs */
  ध्रुव शून्य *p_दत्त;      /* Data for tables. */
  sqlite3_मानम् *aMem;    /* First of nMem value in the unpacked pKey */
  u16 nMem;               /* Number of aMem[] value.  Might be zero */
  पूर्णाङ्क n_दत्त;              /* Size of pData.  0 if none. */
  पूर्णाङ्क nZero;              /* Extra zero data appended after pData,nData */
};

पूर्णाङ्क sqlite3_Btree_निवेशय(BtCursor*, ध्रुव BtreePayload *pPayload,
                       पूर्णाङ्क flags, पूर्णाङ्क seek_परिणाम);
पूर्णाङ्क sqlite3BtreeFirst(BtCursor*, पूर्णाङ्क *pRes);
पूर्णाङ्क sqlite3_Btree_Is_रिक्तानि(BtCursor *pCur, पूर्णाङ्क *pRes);
पूर्णाङ्क sqlite3_Btree_चरमः(BtCursor*, पूर्णाङ्क *pRes);
पूर्णाङ्क sqlite3_Btree_अग्र(BtCursor*, पूर्णाङ्क flags);
पूर्णाङ्क sqlite3BtreeEof(BtCursor*);
पूर्णाङ्क sqlite3BtreePrevious(BtCursor*, पूर्णाङ्क flags);
i64 sqlite3_Btree_Integer_कुञ्चिका(BtCursor*);
शून्य sqlite3BtreeCursorPin(BtCursor*);
शून्य sqlite3BtreeCursorUnpin(BtCursor*);
i64 sqlite3_Btree_विस्थापनम्(BtCursor*);
पूर्णाङ्क sqlite3BtreePayload(BtCursor*, u32 विस्थापनम्, u32 amt, शून्य*);
ध्रुव शून्य *sqlite3BtreePayloadFetch(BtCursor*, u32 *pAmt);
u32 sqlite3_Btree_Payload_आकार(BtCursor*);
sqlite3_int64 sqlite3_Btree_Max_लेखः_आकार(BtCursor*);

पूर्णाङ्क sqlite3BtreeIntegrityCheck(
  sqlite3 *db,  /* Database connection that is running the check */
  Btree *p,     /* The btree to be checked */
  Pgno *a_मूलम्,  /* An array of root pages numbers for individual trees */
  sqlite3_मानम् *aCnt,  /* OUT: entry counts for each btree in aRoot[] */
  पूर्णाङ्क n_मूलम्,    /* Number of entries in aRoot[] */
  पूर्णाङ्क mxErr,    /* Stop reporting errors after this many */
  पूर्णाङ्क *pnErr,   /* OUT: Write number of errors seen to this variable */
  वर्ण **pz_बहिर्गमनम्  /* OUT: Write the error message string here */
);
संरचना Pager *sqlite3BtreePager(Btree*);
i64 sqlite3_Btree_Row_गणनम्_Est(BtCursor*);

#यदि_अपरिभाषितम् SQLITE_OMIT_INCRBLOB
पूर्णाङ्क sqlite3BtreePayloadChecked(BtCursor*, u32 विस्थापनम्, u32 amt, शून्य*);
पूर्णाङ्क sqlite3_Btree_Put_दत्त(BtCursor*, u32 विस्थापनम्, u32 amt, शून्य*);
शून्य sqlite3BtreeIncrblobCursor(BtCursor *);
#यदि_अन्तः
शून्य sqlite3BtreeClearCursor(BtCursor *);
पूर्णाङ्क sqlite3_Btree_समुच्चयः_संस्करणम्(Btree *pBt, पूर्णाङ्क i_संस्करणम्);
पूर्णाङ्क sqlite3BtreeCursorHasHint(BtCursor*, अचिह्नित पूर्णाङ्क आवरणम्);
पूर्णाङ्क sqlite3BtreeIsReadonly(Btree *pBt);
पूर्णाङ्क sqlite3_शीर्षकम्_आकार_Btree(शून्य);

#यदि_परिभाषितम् SQLITE_DEBUG
sqlite3_uint64 sqlite3_Btree_Seek_गणनम्(Btree*);
#अन्यथा
# define sqlite3_Btree_Seek_गणनम्(X) 0
#यदि_अन्तः

#यदि_अपरिभाषितम् NDEBUG
पूर्णाङ्क sqlite3BtreeCursorIsValid(BtCursor*);
#यदि_अन्तः
पूर्णाङ्क sqlite3BtreeCursorIsValidNN(BtCursor*);

पूर्णाङ्क sqlite3_Btree_गणनम्(sqlite3*, BtCursor*, i64*);

#यदि_परिभाषितम् SQLITE_TEST
पूर्णाङ्क sqlite3BtreeCursorInfo(BtCursor*, पूर्णाङ्क*, पूर्णाङ्क);
शून्य sqlite3BtreeCursorList(Btree*);
#यदि_अन्तः

#यदि_अपरिभाषितम् SQLITE_OMIT_WAL
  पूर्णाङ्क sqlite3BtreeCheckpoint(Btree*, पूर्णाङ्क, पूर्णाङ्क *, पूर्णाङ्क *);
#यदि_अन्तः

पूर्णाङ्क sqlite3BtreeTransferRow(BtCursor*, BtCursor*, i64);

शून्य sqlite3BtreeClearCache(Btree*);

/*
** If we are not using shared cache, then there is no need to
** use mutexes to access the BtShared structures.  So make the
** Enter and Leave procedures no-ops.
*/
#यदि_अपरिभाषितम् SQLITE_OMIT_SHARED_CACHE
  शून्य sqlite3BtreeEnter(Btree*);
  शून्य sqlite3BtreeEnterAll(sqlite3*);
  पूर्णाङ्क sqlite3BtreeSharable(Btree*);
  शून्य sqlite3BtreeEnterCursor(BtCursor*);
  पूर्णाङ्क sqlite3_Btree_Connection_गणनम्(Btree*);
#अन्यथा
# define sqlite3BtreeEnter(X) 
# define sqlite3BtreeEnterAll(X)
# define sqlite3BtreeSharable(X) 0
# define sqlite3BtreeEnterCursor(X)
# define sqlite3_Btree_Connection_गणनम्(X) 1
#यदि_अन्तः

#यदि !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE
  शून्य sqlite3BtreeLeave(Btree*);
  शून्य sqlite3BtreeLeaveCursor(BtCursor*);
  शून्य sqlite3BtreeLeaveAll(sqlite3*);
#यदि_अपरिभाषितम् NDEBUG
  /* These routines are used inside assert() statements only. */
  पूर्णाङ्क sqlite3BtreeHoldsMutex(Btree*);
  पूर्णाङ्क sqlite3BtreeHoldsAllMutexes(sqlite3*);
  पूर्णाङ्क sqlite3SchemaMutexHeld(sqlite3*,पूर्णाङ्क,Schema*);
#यदि_अन्तः
#अन्यथा

# define sqlite3BtreeLeave(X)
# define sqlite3BtreeLeaveCursor(X)
# define sqlite3BtreeLeaveAll(X)

# define sqlite3BtreeHoldsMutex(X) 1
# define sqlite3BtreeHoldsAllMutexes(X) 1
# define sqlite3SchemaMutexHeld(X,Y,Z) 1
#यदि_अन्तः


#यदि_अन्तः /* SQLITE_BTREE_H */
