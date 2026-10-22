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
#अगर_अघोषित SQLITE_BTREE_H
#घोषणा SQLITE_BTREE_H

/* TODO: This definition is just included so other modules compile. It
** needs to be revisited.
*/
#घोषणा SQLITE_N_BTREE_META 16

/*
** If defined as non-zero, auto-vacuum is enabled by default. Otherwise
** it must be turned on for each database using "PRAGMA auto_vacuum = 1".
*/
#अगर_अघोषित SQLITE_DEFAULT_AUTOVACUUM
  #घोषणा SQLITE_DEFAULT_AUTOVACUUM 0
#पूर्ण_अगर

#घोषणा BTREE_AUTOVACUUM_NONE 0        /* Do not do auto-vacuum */
#घोषणा BTREE_AUTOVACUUM_FULL 1        /* Do full auto-vacuum */
#घोषणा BTREE_AUTOVACUUM_INCR 2        /* Incremental vacuum */

/*
** Forward declarations of structure
*/
प्रकार काष्ठा Btree Btree;
प्रकार काष्ठा BtCursor BtCursor;
प्रकार काष्ठा BtShared BtShared;
प्रकार काष्ठा BtreePayload BtreePayload;


पूर्णांक sqlite3BtreeOpen(
  sqlite3_vfs *pVfs,       /* VFS to use with this b-tree */
  स्थिर अक्षर *zFilename,   /* Name of database file to open */
  sqlite3 *db,             /* Associated database connection */
  Btree **ppBtree,         /* Return open Btree* here */
  पूर्णांक flags,               /* Flags */
  पूर्णांक vfsFlags             /* Flags passed through to VFS open */
);

/* The flags parameter to sqlite3BtreeOpen can be the bitwise or of the
** following values.
**
** NOTE:  These values must match the corresponding PAGER_ values in
** pager.h.
*/
#घोषणा BTREE_OMIT_JOURNAL  1  /* Do not create or use a rollback journal */
#घोषणा BTREE_MEMORY        2  /* This is an in-memory DB */
#घोषणा BTREE_SINGLE        4  /* The file contains at most 1 b-tree */
#घोषणा BTREE_UNORDERED     8  /* Use of a hash implementation is OK */

पूर्णांक sqlite3BtreeClose(Btree*);
पूर्णांक sqlite3_Btree_Set_Cache_आकार(Btree*,पूर्णांक);
पूर्णांक sqlite3_Btree_Set_Spill_आकार(Btree*,पूर्णांक);
#अगर SQLITE_MAX_MMAP_आकार>0
  पूर्णांक sqlite3BtreeSetMmapLimit(Btree*,sqlite3_int64);
#पूर्ण_अगर
पूर्णांक sqlite3BtreeSetPagerFlags(Btree*,अचिन्हित);
पूर्णांक sqlite3_Btree_Set_Page_आकार(Btree *p, पूर्णांक nPagesize, पूर्णांक nReserve, पूर्णांक eFix);
पूर्णांक sqlite3_Btree_Get_Page_आकार(Btree*);
Pgno sqlite3_Btree_Max_Page_गिनती(Btree*,Pgno);
Pgno sqlite3BtreeLastPage(Btree*);
पूर्णांक sqlite3BtreeSecureDelete(Btree*,पूर्णांक);
पूर्णांक sqlite3BtreeGetRequestedReserve(Btree*);
पूर्णांक sqlite3BtreeGetReserveNoMutex(Btree *p);
पूर्णांक sqlite3BtreeSetAutoVacuum(Btree *, पूर्णांक);
पूर्णांक sqlite3BtreeGetAutoVacuum(Btree *);
पूर्णांक sqlite3BtreeBeginTrans(Btree*,पूर्णांक,पूर्णांक*);
पूर्णांक sqlite3BtreeCommitPhaseOne(Btree*, स्थिर अक्षर*);
पूर्णांक sqlite3BtreeCommitPhaseTwo(Btree*, पूर्णांक);
पूर्णांक sqlite3BtreeCommit(Btree*);
पूर्णांक sqlite3BtreeRollback(Btree*,पूर्णांक,पूर्णांक);
पूर्णांक sqlite3BtreeBeginStmt(Btree*,पूर्णांक);
पूर्णांक sqlite3BtreeCreateTable(Btree*, Pgno*, पूर्णांक flags);
पूर्णांक sqlite3BtreeTxnState(Btree*);
पूर्णांक sqlite3BtreeIsInBackup(Btree*);

व्योम *sqlite3BtreeSchema(Btree *, पूर्णांक, व्योम(*)(व्योम *));
पूर्णांक sqlite3BtreeSchemaLocked(Btree *pBtree);
#अगर_अघोषित SQLITE_OMIT_SHARED_CACHE
पूर्णांक sqlite3BtreeLockTable(Btree *pBtree, पूर्णांक iTab, u8 isWriteLock);
#पूर्ण_अगर

/* Savepoints are named, nestable SQL transactions mostly implemented */ 
/* in vdbe.c and pager.c See https://sqlite.org/lang_savepoint.html */
पूर्णांक sqlite3BtreeSavepoint(Btree *, पूर्णांक, पूर्णांक);

/* "Checkpoint" only refers to WAL. See https://sqlite.org/wal.html#ckpt */
#अगर_अघोषित SQLITE_OMIT_WAL
  पूर्णांक sqlite3BtreeCheckpoint(Btree*, पूर्णांक, पूर्णांक *, पूर्णांक *);  
#पूर्ण_अगर

स्थिर अक्षर *sqlite3BtreeGetFilename(Btree *);
स्थिर अक्षर *sqlite3BtreeGetJournalname(Btree *);
पूर्णांक sqlite3BtreeCopyFile(Btree*, Btree*);

पूर्णांक sqlite3BtreeIncrVacuum(Btree *);

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
#घोषणा BTREE_INTKEY     1    /* Table has only 64-bit signed integer keys */
#घोषणा BTREE_BLOBKEY    2    /* Table has keys only - no data */

पूर्णांक sqlite3BtreeDropTable(Btree*, पूर्णांक, पूर्णांक*);
पूर्णांक sqlite3BtreeClearTable(Btree*, पूर्णांक, i64*);
पूर्णांक sqlite3BtreeClearTableOfCursor(BtCursor*);
पूर्णांक sqlite3BtreeTripAllCursors(Btree*, पूर्णांक, पूर्णांक);

व्योम sqlite3BtreeGetMeta(Btree *pBtree, पूर्णांक idx, u32 *p_मूल्य);
पूर्णांक sqlite3BtreeUpdateMeta(Btree*, पूर्णांक idx, u32 मूल्य);

पूर्णांक sqlite3BtreeNewDb(Btree *p);

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
#घोषणा BTREE_FREE_PAGE_गिनती     0
#घोषणा BTREE_SCHEMA_VERSION      1
#घोषणा BTREE_FILE_FORMAT         2
#घोषणा BTREE_DEFAULT_CACHE_आकार  3
#घोषणा BTREE_LARGEST_मूल_PAGE   4
#घोषणा BTREE_TEXT_ENCODING       5
#घोषणा BTREE_USER_VERSION        6
#घोषणा BTREE_INCR_VACUUM         7
#घोषणा BTREE_APPLICATION_ID      8
#घोषणा BTREE_तथ्य_VERSION        15  /* A virtual meta-value */

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
#घोषणा BTREE_HINT_RANGE 0       /* Range constraints on queries */
#घोषणा BTREE_HINT_TABLECURSOR 1 /* Table csr associated with this index csr */

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
#घोषणा BTREE_BULKLOAD 0x00000001  /* Used to full index in sorted order */
#घोषणा BTREE_SEEK_EQ  0x00000002  /* EQ seeks only - no range seeks */

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
#घोषणा BTREE_WRCSR     0x00000004     /* read-write cursor */
#घोषणा BTREE_FORDELETE 0x00000008     /* Cursor is for seek/delete only */

पूर्णांक sqlite3BtreeCursor(
  Btree*,                              /* BTree containing table to open */
  Pgno iTable,                         /* Index of root page */
  पूर्णांक wrFlag,                          /* 1 for writing.  0 for read-only */
  काष्ठा कुंजी_Info*,                     /* First argument to compare function */
  BtCursor *pCursor                    /* Space to write cursor structure */
);
BtCursor *sqlite3BtreeFakeValidCursor(व्योम);
पूर्णांक sqlite3_Btree_Cursor_आकार(व्योम);
#अगर_घोषित SQLITE_DEBUG
पूर्णांक sqlite3BtreeClosesWithCursor(Btree*,BtCursor*);
#पूर्ण_अगर
व्योम sqlite3BtreeCursorZero(BtCursor*);
व्योम sqlite3BtreeCursorHintFlags(BtCursor*, अचिन्हित);
#अगर_घोषित SQLITE_ENABLE_CURSOR_HINTS
व्योम sqlite3BtreeCursorHint(BtCursor*, पूर्णांक, ...);
 #अगर_घोषित SQLITE_DEBUG
 BtCursor *sqlite3BtreeCursorHintTblCsr(BtCursor*);
 #पूर्ण_अगर
#पूर्ण_अगर

पूर्णांक sqlite3BtreeCloseCursor(BtCursor*);
पूर्णांक sqlite3BtreeTableMoveto(
  BtCursor*,
  i64 int_कुंजी,
  पूर्णांक bias,
  पूर्णांक *pRes
);
पूर्णांक sqlite3_Btree_सूचकांक_Moveto(
  BtCursor*,
  UnpackedRecord *p_Un_कुंजी,
  पूर्णांक *pRes
);
पूर्णांक sqlite3BtreeCursorHasMoved(BtCursor*);
पूर्णांक sqlite3BtreeCursorRestore(BtCursor*, पूर्णांक*);
पूर्णांक sqlite3BtreeDelete(BtCursor*, u8 flags);

/* Allowed flags for sqlite3BtreeDelete() and sqlite3BtreeInsert() */
#घोषणा BTREE_SAVEPOSITION 0x02  /* Leave cursor pointing at NEXT or PREV */
#घोषणा BTREE_AUXDELETE    0x04  /* not the primary delete operation */
#घोषणा BTREE_APPEND       0x08  /* Insert is likely an append */
#घोषणा BTREE_PREFORMAT    0x80  /* Inserted data is a preformated cell */

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
काष्ठा BtreePayload {
  स्थिर व्योम *p_कुंजी;       /* Key content for indexes.  NULL for tables */
  sqlite3_int64 n_कुंजी;     /* Size of pKey for indexes.  PRIMARY KEY for tabs */
  स्थिर व्योम *p_तथ्य;      /* Data for tables. */
  sqlite3_मूल्य *aMem;    /* First of nMem value in the unpacked pKey */
  u16 nMem;               /* Number of aMem[] value.  Might be zero */
  पूर्णांक n_तथ्य;              /* Size of pData.  0 if none. */
  पूर्णांक nZero;              /* Extra zero data appended after pData,nData */
};

पूर्णांक sqlite3_Btree_जोड़ो(BtCursor*, स्थिर BtreePayload *pPayload,
                       पूर्णांक flags, पूर्णांक seek_परिणाम);
पूर्णांक sqlite3BtreeFirst(BtCursor*, पूर्णांक *pRes);
पूर्णांक sqlite3BtreeIsEmpty(BtCursor *pCur, पूर्णांक *pRes);
पूर्णांक sqlite3BtreeLast(BtCursor*, पूर्णांक *pRes);
पूर्णांक sqlite3_Btree_अग्र(BtCursor*, पूर्णांक flags);
पूर्णांक sqlite3BtreeEof(BtCursor*);
पूर्णांक sqlite3BtreePrevious(BtCursor*, पूर्णांक flags);
i64 sqlite3_Btree_Integer_कुंजी(BtCursor*);
व्योम sqlite3BtreeCursorPin(BtCursor*);
व्योम sqlite3BtreeCursorUnpin(BtCursor*);
i64 sqlite3BtreeOffset(BtCursor*);
पूर्णांक sqlite3BtreePayload(BtCursor*, u32 offset, u32 amt, व्योम*);
स्थिर व्योम *sqlite3BtreePayloadFetch(BtCursor*, u32 *pAmt);
u32 sqlite3_Btree_Payload_आकार(BtCursor*);
sqlite3_int64 sqlite3_Btree_Max_Record_आकार(BtCursor*);

पूर्णांक sqlite3BtreeIntegrityCheck(
  sqlite3 *db,  /* Database connection that is running the check */
  Btree *p,     /* The btree to be checked */
  Pgno *a_मूल,  /* An array of root pages numbers for individual trees */
  sqlite3_मूल्य *aCnt,  /* OUT: entry counts for each btree in aRoot[] */
  पूर्णांक n_मूल,    /* Number of entries in aRoot[] */
  पूर्णांक mxErr,    /* Stop reporting errors after this many */
  पूर्णांक *pnErr,   /* OUT: Write number of errors seen to this variable */
  अक्षर **pzOut  /* OUT: Write the error message string here */
);
काष्ठा Pager *sqlite3BtreePager(Btree*);
i64 sqlite3_Btree_Row_गिनती_Est(BtCursor*);

#अगर_अघोषित SQLITE_OMIT_INCRBLOB
पूर्णांक sqlite3BtreePayloadChecked(BtCursor*, u32 offset, u32 amt, व्योम*);
पूर्णांक sqlite3_Btree_Put_तथ्य(BtCursor*, u32 offset, u32 amt, व्योम*);
व्योम sqlite3BtreeIncrblobCursor(BtCursor *);
#पूर्ण_अगर
व्योम sqlite3BtreeClearCursor(BtCursor *);
पूर्णांक sqlite3BtreeSetVersion(Btree *pBt, पूर्णांक iVersion);
पूर्णांक sqlite3BtreeCursorHasHint(BtCursor*, अचिन्हित पूर्णांक mask);
पूर्णांक sqlite3BtreeIsReadonly(Btree *pBt);
पूर्णांक sqlite3_Header_आकार_Btree(व्योम);

#अगर_घोषित SQLITE_DEBUG
sqlite3_uint64 sqlite3_Btree_Seek_गिनती(Btree*);
#अन्यथा
# define sqlite3_Btree_Seek_गिनती(X) 0
#पूर्ण_अगर

#अगर_अघोषित न_संशोधन
पूर्णांक sqlite3BtreeCursorIsValid(BtCursor*);
#पूर्ण_अगर
पूर्णांक sqlite3BtreeCursorIsValidNN(BtCursor*);

पूर्णांक sqlite3_Btree_गिनती(sqlite3*, BtCursor*, i64*);

#अगर_घोषित SQLITE_TEST
पूर्णांक sqlite3BtreeCursorInfo(BtCursor*, पूर्णांक*, पूर्णांक);
व्योम sqlite3BtreeCursorList(Btree*);
#पूर्ण_अगर

#अगर_अघोषित SQLITE_OMIT_WAL
  पूर्णांक sqlite3BtreeCheckpoint(Btree*, पूर्णांक, पूर्णांक *, पूर्णांक *);
#पूर्ण_अगर

पूर्णांक sqlite3BtreeTransferRow(BtCursor*, BtCursor*, i64);

व्योम sqlite3BtreeClearCache(Btree*);

/*
** If we are not using shared cache, then there is no need to
** use mutexes to access the BtShared structures.  So make the
** Enter and Leave procedures no-ops.
*/
#अगर_अघोषित SQLITE_OMIT_SHARED_CACHE
  व्योम sqlite3BtreeEnter(Btree*);
  व्योम sqlite3BtreeEnterAll(sqlite3*);
  पूर्णांक sqlite3BtreeSharable(Btree*);
  व्योम sqlite3BtreeEnterCursor(BtCursor*);
  पूर्णांक sqlite3_Btree_Connection_गिनती(Btree*);
#अन्यथा
# define sqlite3BtreeEnter(X) 
# define sqlite3BtreeEnterAll(X)
# define sqlite3BtreeSharable(X) 0
# define sqlite3BtreeEnterCursor(X)
# define sqlite3_Btree_Connection_गिनती(X) 1
#पूर्ण_अगर

#अगर !defined(SQLITE_OMIT_SHARED_CACHE) && SQLITE_THREADSAFE
  व्योम sqlite3BtreeLeave(Btree*);
  व्योम sqlite3BtreeLeaveCursor(BtCursor*);
  व्योम sqlite3BtreeLeaveAll(sqlite3*);
#अगर_अघोषित न_संशोधन
  /* These routines are used inside assert() statements only. */
  पूर्णांक sqlite3BtreeHoldsMutex(Btree*);
  पूर्णांक sqlite3BtreeHoldsAllMutexes(sqlite3*);
  पूर्णांक sqlite3SchemaMutexHeld(sqlite3*,पूर्णांक,Schema*);
#पूर्ण_अगर
#अन्यथा

# define sqlite3BtreeLeave(X)
# define sqlite3BtreeLeaveCursor(X)
# define sqlite3BtreeLeaveAll(X)

# define sqlite3BtreeHoldsMutex(X) 1
# define sqlite3BtreeHoldsAllMutexes(X) 1
# define sqlite3SchemaMutexHeld(X,Y,Z) 1
#पूर्ण_अगर


#पूर्ण_अगर /* SQLITE_BTREE_H */
