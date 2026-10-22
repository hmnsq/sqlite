/*
** 2010 February 1
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains the implementation of a write-ahead log (WAL) used in
** "journal_mode=WAL" mode.
**
** WRITE-AHEAD LOG (WAL) FILE FORMAT
**
** A WAL file consists of a header followed by zero or more "frames".
** Each frame records the revised content of a single page from the
** database file.  All changes to the database are recorded by writing
** frames into the WAL.  Transactions commit when a frame is written that
** contains a commit marker.  A single WAL can and usually does record
** multiple transactions.  Periodically, the content of the WAL is
** transferred back into the database file in an operation called a
** "checkpoint".
**
** A single WAL file can be used multiple times.  In other words, the
** WAL can fill up with frames and then be checkpointed and then new
** frames can overwrite the old ones.  A WAL always grows from beginning
** toward the end.  Checksums and counters attached to each frame are
** used to determine which frames within the WAL are valid and which
** are leftovers from prior checkpoints.
**
** The WAL header is 32 bytes in size and consists of the following eight
** big-endian 32-bit unsigned integer values:
**
**     0: Magic number.  0x377f0682 or 0x377f0683
**     4: File format version.  Currently 3007000
**     8: Database page size.  Example: 1024
**    12: Checkpoint sequence number
**    16: Salt-1, random integer incremented with each checkpoint
**    20: Salt-2, a different random integer changing with each ckpt
**    24: Checksum-1 (first part of checksum for first 24 bytes of header).
**    28: Checksum-2 (second part of checksum for first 24 bytes of header).
**
** Immediately following the wal-header are zero or more frames. Each
** frame consists of a 24-byte frame-header followed by <page-size> bytes
** of page data. The frame-header is six big-endian 32-bit unsigned
** integer values, as follows:
**
**     0: Page number.
**     4: For commit records, the size of the database image in pages
**        after the commit. For all other records, zero.
**     8: Salt-1 (copied from the header)
**    12: Salt-2 (copied from the header)
**    16: Checksum-1.
**    20: Checksum-2.
**
** A frame is considered valid if and only if the following conditions are
** true:
**
**    (1) The salt-1 and salt-2 values in the frame-header match
**        salt values in the wal-header
**
**    (2) The checksum values in the final 8 bytes of the frame-header
**        exactly match the checksum computed consecutively on the
**        WAL header and the first 8 bytes and the content of all frames
**        up to and including the current frame.
**
** The checksum is computed using 32-bit big-endian integers if the
** magic number in the first 4 bytes of the WAL is 0x377f0683 and it
** is computed using little-endian if the magic number is 0x377f0682.
** The checksum values are always stored in the frame header in a
** big-endian format regardless of which byte order is used to compute
** the checksum.  The checksum is computed by interpreting the input as
** an even number of unsigned 32-bit integers: x[0] through x[N].  The
** algorithm used for the checksum is as follows:
**
**   for i from 0 to n-1 step 2:
**     s0 += x[i] + s1;
**     s1 += x[i+1] + s0;
**   endfor
**
** Note that s0 and s1 are both weighted checksums using fibonacci weights
** in reverse order (the largest fibonacci weight occurs on the first element
** of the sequence being summed.)  The s1 value spans all 32-bit
** terms of the sequence whereas s0 omits the final term.
**
** On a checkpoint, the WAL is first VFS.xSync-ed, then valid content of the
** WAL is transferred into the database, then the database is VFS.xSync-ed.
** The VFS.xSync operations serve as write barriers - all writes launched
** before the xSync must complete before any write that launches after the
** xSync begins.
**
** After each checkpoint, the salt-1 value is incremented and the salt-2
** value is randomized.  This prevents old and new frames in the WAL from
** being considered valid at the same time and being checkpointing together
** following a crash.
**
** READER ALGORITHM
**
** To read a page from the database (call it page number P), a reader
** first checks the WAL to see if it contains page P.  If so, then the
** last valid instance of page P that is a followed by a commit frame
** or is a commit frame itself becomes the value read.  If the WAL
** contains no copies of page P that are valid and which are a commit
** frame or are followed by a commit frame, then page P is read from
** the database file.
**
** To start a read transaction, the reader records the index of the last
** valid frame in the WAL.  The reader uses this recorded "mxFrame" value
** for all subsequent read operations.  New transactions can be appended
** to the WAL, but as long as the reader uses its original mxFrame value
** and ignores the newly appended content, it will see a consistent snapshot
** of the database from a single point in time.  This technique allows
** multiple concurrent readers to view different versions of the database
** content simultaneously.
**
** The reader algorithm in the previous paragraphs works correctly, but
** because frames for page P can appear anywhere within the WAL, the
** reader has to scan the entire WAL looking for page P frames.  If the
** WAL is large (multiple megabytes is typical) that scan can be slow,
** and read performance suffers.  To overcome this problem, a separate
** data structure called the wal-index is maintained to expedite the
** search for frames of a particular page.
**
** WAL-INDEX FORMAT
**
** Conceptually, the wal-index is shared memory, though VFS implementations
** might choose to implement the wal-index using a mmapped file.  Because
** the wal-index is shared memory, SQLite does not support journal_mode=WAL
** on a network filesystem.  All users of the database must be able to
** share memory.
**
** In the default unix and windows implementation, the wal-index is a mmapped
** file whose name is the database name with a "-shm" suffix added.  For that
** reason, the wal-index is sometimes called the "shm" file.
**
** The wal-index is transient.  After a crash, the wal-index can (and should
** be) reconstructed from the original WAL file.  In fact, the VFS is required
** to either truncate or zero the header of the wal-index when the last
** connection to it closes.  Because the wal-index is transient, it can
** use an architecture-specific format; it does not have to be cross-platform.
** Hence, unlike the database and WAL file formats which store all values
** as big endian, the wal-index can store multi-byte values in the native
** byte order of the host computer.
**
** The purpose of the wal-index is to answer this question quickly:  Given
** a page number P and a maximum frame index M, return the index of the
** last frame in the wal before frame M for page P in the WAL, or return
** NULL if there are no frames for page P in the WAL prior to M.
**
** The wal-index consists of a header region, followed by an one or
** more index blocks.
**
** The wal-index header contains the total number of frames within the WAL
** in the mxFrame field.
**
** Each index block except for the first contains information on
** HASHTABLE_NPAGE frames. The first index block contains information on
** HASHTABLE_NPAGE_ONE frames. The values of HASHTABLE_NPAGE_ONE and
** HASHTABLE_NPAGE are selected so that together the wal-index header and
** first index block are the same size as all other index blocks in the
** wal-index.  The values are:
**
**   HASHTABLE_NPAGE      4096
**   HASHTABLE_NPAGE_ONE  4062
**
** Each index block contains two sections, a page-mapping that contains the
** database page number associated with each wal frame, and a hash-table
** that allows readers to query an index block for a specific page number.
** The page-mapping is an array of HASHTABLE_NPAGE (or HASHTABLE_NPAGE_ONE
** for the first index block) 32-bit page numbers. The first entry in the
** first index-block contains the database page number corresponding to the
** first frame in the WAL file. The first entry in the second index block
** in the WAL file corresponds to the (HASHTABLE_NPAGE_ONE+1)th frame in
** the log, and so on.
**
** The last index block in a wal-index usually contains less than the full
** complement of HASHTABLE_NPAGE (or HASHTABLE_NPAGE_ONE) page-numbers,
** depending on the contents of the WAL file. This does not change the
** allocated size of the page-mapping array - the page-mapping array merely
** contains unused entries.
**
** Even without using the hash table, the last frame for page P
** can be found by scanning the page-mapping sections of each index block
** starting with the last index block and moving toward the first, and
** within each index block, starting at the end and moving toward the
** beginning.  The first entry that equals P corresponds to the frame
** holding the content for that page.
**
** The hash table consists of HASHTABLE_NSLOT 16-bit unsigned integers.
** HASHTABLE_NSLOT = 2*HASHTABLE_NPAGE, and there is one entry in the
** hash table for each page number in the mapping section, so the hash
** table is never more than half full.  The expected number of collisions
** prior to finding a match is 1.  Each entry of the hash table is an
** 1-based index of an entry in the mapping section of the same
** index block.   Let K be the 1-based index of the largest entry in
** the mapping section.  (For index blocks other than the last, K will
** always be exactly HASHTABLE_NPAGE (4096) and for the last index block
** K will be (mxFrame%HASHTABLE_NPAGE).)  Unused slots of the hash table
** contain a value of 0.
**
** To look for page P in the hash table, first compute a hash iKey on
** P as follows:
**
**      iKey = (P * 383) % HASHTABLE_NSLOT
**
** Then start scanning entries of the hash table, starting with iKey
** (wrapping around to the beginning when the end of the hash table is
** reached) until an unused hash slot is found. Let the first unused slot
** be at index iUnused.  (iUnused might be less than iKey if there was
** wrap-around.) Because the hash table is never more than half full,
** the search is guaranteed to eventually hit an unused entry.  Let
** iMax be the value between iKey and iUnused, closest to iUnused,
** where aHash[iMax]==P.  If there is no iMax entry (if there exists
** no hash slot such that aHash[i]==p) then page P is not in the
** current index block.  Otherwise the iMax-th mapping entry of the
** current index block corresponds to the last entry that references
** page P.
**
** A hash search begins with the last index block and moves toward the
** first index block, looking for entries corresponding to page P.  On
** average, only two or three slots in each index block need to be
** examined in order to either find the last entry for page P, or to
** establish that no such entry exists in the block.  Each index block
** holds over 4000 entries.  So two or three index blocks are sufficient
** to cover a typical 10 megabyte WAL file, assuming 1K pages.  8 or 10
** comparisons (on average) suffice to either locate a frame in the
** WAL or to establish that the frame does not exist in the WAL.  This
** is much faster than scanning the entire 10MB WAL.
**
** Note that entries are added in order of increasing K.  Hence, one
** reader might be using some value K0 and a second reader that started
** at a later time (after additional transactions were added to the WAL
** and to the wal-index) might be using a different value K1, where K1>K0.
** Both readers can use the same hash table and mapping section to get
** the correct result.  There may be entries in the hash table with
** K>K0 but to the first reader, those entries will appear to be unused
** slots in the hash table and so the first reader will get an answer as
** if no values greater than K0 had ever been inserted into the hash table
** in the first place - which is what reader one wants.  Meanwhile, the
** second reader using K1 will see additional values that were inserted
** later, which is exactly what reader two wants.
**
** When a rollback occurs, the value of K is decreased. Hash table entries
** that correspond to frames greater than the new K value are removed
** from the hash table at this point.
*/
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WAL

#అన్తర్భావ "wal.h"

/*
** Trace output macros
*/
#ఒకవేళ defined(SQLITE_TEST) && defined(SQLITE_DEBUG)
పూర్ణాంకం sqlite3WalTrace = 0;
# define WALTRACE(X)  ఒకవేళ(sqlite3WalTrace) sqlite3DebugPrintf X
#లేకపోతే
# define WALTRACE(X)
#అయితే_ముగింపు

/*
** The maximum (and only) versions of the wal and wal-index formats
** that may be interpreted by this version of SQLite.
**
** If a client begins recovering a WAL file and finds that (a) the checksum
** values in the wal-header are correct and (b) the version field is not
** WAL_MAX_VERSION, recovery fails and SQLite returns SQLITE_CANTOPEN.
**
** Similarly, if a client successfully reads a wal-index header (i.e. the
** checksum test is successful) and finds that the version field is not
** WALINDEX_MAX_VERSION, then no read-transaction is opened and SQLite
** returns SQLITE_CANTOPEN.
*/
#నిర్వచించు WAL_MAX_సంస్కరణ      3007000
#నిర్వచించు WALINDEX_MAX_సంస్కరణ 3007000

/*
** Index numbers for various locking bytes.   WAL_NREADER is the number
** of available reader locks and should be at least 3.  The default
** is SQLITE_SHM_NLOCK==8 and  WAL_NREADER==5.
**
** Technically, the various VFSes are free to implement these locks however
** they see fit.  However, compatibility is encouraged so that VFSes can
** interoperate.  The standard implementation used on both unix and windows
** is for the index number to indicate a byte offset into the
** WalCkptInfo.aLock[] array in the wal-index header.  In other words, all
** locks are on the shm file.  The WALINDEX_LOCK_OFFSET constant (which
** should be 120) is the location in the shm file for the first locking
** byte.
*/
#నిర్వచించు WAL_WRITE_తాళం         0
#నిర్వచించు WAL_ALL_BUT_WRITE      1
#నిర్వచించు WAL_CKPT_తాళం          1
#నిర్వచించు WAL_RECOVER_తాళం       2
#నిర్వచించు WAL_READ_తాళం(I)       (3+(I))
#నిర్వచించు WAL_NREADER            (SQLITE_SHM_NLOCK-3)


/* Object declarations */
రకనిర్వచనం నిర్మాణం Wal_సూచీ_Hdr Wal_సూచీ_Hdr;
రకనిర్వచనం నిర్మాణం WalIterator WalIterator;
రకనిర్వచనం నిర్మాణం WalCkptInfo WalCkptInfo;


/*
** The following object holds a copy of the wal-index header content.
**
** The actual header in the wal-index consists of two copies of this
** object followed by one instance of the WalCkptInfo object.
** For all versions of SQLite through 3.10.0 and probably beyond,
** the locking bytes (WalCkptInfo.aLock) start at offset 120 and
** the total header size is 136 bytes.
**
** The szPage value can be any power of 2 between 512 and 32768, inclusive.
** Or it can be 1 to represent a 65536-byte page.  The latter case was
** added in 3.7.1 when support for 64K pages was added.
*/
నిర్మాణం Wal_సూచీ_Hdr {
  u32 i_సంస్కరణ;                   /* Wal-index version */
  u32 వాడని;                     /* Unused (padding) field */
  u32 iChange;                    /* Counter incremented each transaction */
  u8 isInit;                      /* 1 when initialized */
  u8 big_చివర_Cksum;                 /* True if checksums in WAL are big-endian */
  u16 sz_పుట;                     /* Database page size in bytes. 1==64K */
  u32 mx_చట్రం;                    /* Index of last valid frame in the WAL */
  u32 n_పుట;                      /* Size of database in pages */
  u32 a_చట్రం_Cksum[2];             /* Checksum of last frame in log */
  u32 aSalt[2];                   /* Two salt values copied from WAL header */
  u32 aCksum[2];                  /* Checksum over all prior fields */
};

/*
** A copy of the following object occurs in the wal-index immediately
** following the second copy of the WalIndexHdr.  This object stores
** information used by checkpoint.
**
** nBackfill is the number of frames in the WAL that have been written
** back into the database. (We call the act of moving content from WAL to
** database "backfilling".)  The nBackfill number is never greater than
** WalIndexHdr.mxFrame.  nBackfill can only be increased by threads
** holding the WAL_CKPT_LOCK lock (which includes a recovery thread).
** However, a WAL_WRITE_LOCK thread can move the value of nBackfill from
** mxFrame back to zero when the WAL is reset.
**
** nBackfillAttempted is the largest value of nBackfill that a checkpoint
** has attempted to achieve.  Normally nBackfill==nBackfillAtempted, however
** the nBackfillAttempted is set before any backfilling is done and the
** nBackfill is only set after all backfilling completes.  So if a checkpoint
** crashes, nBackfillAttempted might be larger than nBackfill.  The
** WalIndexHdr.mxFrame must never be less than nBackfillAttempted.
**
** The aLock[] field is a set of bytes used for locking.  These bytes should
** never be read or written.
**
** There is one entry in aReadMark[] for each reader lock.  If a reader
** holds read-lock K, then the value in aReadMark[K] is no greater than
** the mxFrame for that reader.  The value READMARK_NOT_USED (0xffffffff)
** for any aReadMark[] means that entry is unused.  aReadMark[0] is
** a special case; its value is never used and it exists as a place-holder
** to avoid having to offset aReadMark[] indexes by one.  Readers holding
** WAL_READ_LOCK(0) always ignore the entire WAL and read all content
** directly from the database.
**
** The value of aReadMark[K] may only be changed by a thread that
** is holding an exclusive lock on WAL_READ_LOCK(K).  Thus, the value of
** aReadMark[K] cannot changed while there is a reader is using that mark
** since the reader will be holding a shared lock on WAL_READ_LOCK(K).
**
** The checkpointer may only transfer frames from WAL to database where
** the frame numbers are less than or equal to every aReadMark[] that is
** in use (that is, every aReadMark[j] for which there is a corresponding
** WAL_READ_LOCK(j)).  New readers (usually) pick the aReadMark[] with the
** largest value and will increase an unused aReadMark[] to mxFrame if there
** is not already an aReadMark[] equal to mxFrame.  The exception to the
** previous sentence is when nBackfill equals mxFrame (meaning that everything
** in the WAL has been backfilled into the database) then new readers
** will choose aReadMark[0] which has value 0 and hence such reader will
** get all their all content directly from the database file and ignore
** the WAL.
**
** Writers normally append new frames to the end of the WAL.  However,
** if nBackfill equals mxFrame (meaning that all WAL content has been
** written back into the database) and if no readers are using the WAL
** (in other words, if there are no WAL_READ_LOCK(i) where i>0) then
** the writer will first "reset" the WAL back to the beginning and start
** writing new content beginning at frame 1.
**
** We assume that 32-bit loads are atomic and so no locks are needed in
** order to read from any aReadMark[] entries.
*/
నిర్మాణం WalCkptInfo {
  u32 nBackfill;                  /* Number of WAL frames backfilled into DB */
  u32 aReadMark[WAL_NREADER];     /* Reader marks */
  u8 a_తాళం[SQLITE_SHM_NLOCK];     /* Reserved space for locks */
  u32 nBackfillAttempted;         /* WAL frames perhaps written, or maybe not */
  u32 notUsed0;                   /* Available for future enhancements */
};
#నిర్వచించు READMARK_NOT_వాడిన  0xffffffff

/*
** This is a schematic view of the complete 136-byte header of the
** wal-index file (also known as the -shm file):
**
**      +-----------------------------+
**   0: | iVersion                    | \
**      +-----------------------------+  |
**   4: | (unused padding)            |  |
**      +-----------------------------+  |
**   8: | iChange                     |  |
**      +-------+-------+-------------+  |
**  12: | bInit |  bBig |   szPage    |  |
**      +-------+-------+-------------+  |
**  16: | mxFrame                     |  |  First copy of the
**      +-----------------------------+  |  WalIndexHdr object
**  20: | nPage                       |  |
**      +-----------------------------+  |
**  24: | aFrameCksum                 |  |
**      |                             |  |
**      +-----------------------------+  |
**  32: | aSalt                       |  |
**      |                             |  |
**      +-----------------------------+  |
**  40: | aCksum                      |  |
**      |                             | /
**      +-----------------------------+
**  48: | iVersion                    | \
**      +-----------------------------+  |
**  52: | (unused padding)            |  |
**      +-----------------------------+  |
**  56: | iChange                     |  |
**      +-------+-------+-------------+  |
**  60: | bInit |  bBig |   szPage    |  |
**      +-------+-------+-------------+  |  Second copy of the
**  64: | mxFrame                     |  |  WalIndexHdr
**      +-----------------------------+  |
**  68: | nPage                       |  |
**      +-----------------------------+  |
**  72: | aFrameCksum                 |  |
**      |                             |  |
**      +-----------------------------+  |
**  80: | aSalt                       |  |
**      |                             |  |
**      +-----------------------------+  |
**  88: | aCksum                      |  |
**      |                             | /
**      +-----------------------------+
**  96: | nBackfill                   |
**      +-----------------------------+
** 100: | 5 read marks                |
**      |                             |
**      |                             |
**      |                             |
**      |                             |
**      +-------+-------+------+------+
** 120: | Write | Ckpt  | Rcvr | Rd0  | \
**      +-------+-------+------+------+  ) 8 lock bytes
**      | Read1 | Read2 | Rd3  | Rd4  | /
**      +-------+-------+------+------+
** 128: | nBackfillAttempted          |
**      +-----------------------------+
** 132: | (unused padding)            |
**      +-----------------------------+
*/

/* A block of WALINDEX_LOCK_RESERVED bytes beginning at
** WALINDEX_LOCK_OFFSET is reserved for locks. Since some systems
** only support mandatory file-locks, we do not read or write data
** from the region of the file on which locks are applied.
*/
#నిర్వచించు WALINDEX_తాళం_స్థానభ్రంశం (పరిమాణం(Wal_సూచీ_Hdr)*2+offsetof(WalCkptInfo,a_తాళం))
#నిర్వచించు WALINDEX_HDR_కొలత    (పరిమాణం(Wal_సూచీ_Hdr)*2+పరిమాణం(WalCkptInfo))

/* Size of header before each frame in wal */
#నిర్వచించు WAL_చట్రం_HDRSIZE 24

/* Size of write ahead log header, including checksum. */
#నిర్వచించు WAL_HDRSIZE 32

/* WAL magic value. Either this value, or the same value with the least
** significant bit also set (WAL_MAGIC | 0x00000001) is stored in 32-bit
** big-endian format in the first 4 bytes of a WAL file.
**
** If the LSB is set, then the checksums for each frame within the WAL
** file are calculated by treating all data as an array of 32-bit
** big-endian words. Otherwise, they are calculated by interpreting
** all data as 32-bit little-endian words.
*/
#నిర్వచించు WAL_MAGIC 0x377f0682

/*
** Return the offset of frame iFrame in the write-ahead log file,
** assuming a database page size of szPage bytes. The offset returned
** is to the start of the write-ahead log frame-header.
*/
#నిర్వచించు wal_చట్రం_స్థానభ్రంశం(i_చట్రం, sz_పుట) (                               \
  WAL_HDRSIZE + ((i_చట్రం)-1)*(i64)((sz_పుట)+WAL_చట్రం_HDRSIZE)         \
)

/*
** An open write-ahead log file is represented by an instance of the
** following object.
**
** writeLock:
**   This is usually set to 1 whenever the WRITER lock is held. However,
**   if it is set to 2, then the WRITER lock is held but must be released
**   by walHandleException() if a SEH exception is thrown.
*/
నిర్మాణం Wal {
  sqlite3_vfs *pVfs;         /* The VFS used to create pDbFd */
  sqlite3_file *pDbFd;       /* File handle for the database file */
  sqlite3_file *pWalFd;      /* File handle for WAL file */
  u32 iCallback;             /* Value to pass to log callback (or 0) */
  i64 mx_Wal_కొలత;             /* Truncate WAL to this size upon reset */
  పూర్ణాంకం n_Wi_దత్తాంశం;               /* Size of array apWiData */
  పూర్ణాంకం sz_మొదటి_ఖండం;          /* Size of first block written to WAL file */
  అస్థిర u32 **ap_Wi_దత్తాంశం;   /* Pointer to wal-index content in memory */
  u32 sz_పుట;                /* Database page size */
  i16 read_తాళం;              /* Which read lock is being held.  -1 for none */
  u8 syncFlags;              /* Flags to use to sync header writes */
  u8 exclusive_బహుళకం;          /* Non-zero if connection is in exclusive mode */
  u8 write_తాళం;              /* True if in a write transaction */
  u8 ckpt_తాళం;               /* True if holding a checkpoint lock */
  u8 readOnly;               /* WAL_RDWR, WAL_RDONLY, or WAL_SHM_RDONLY */
  u8 truncateOnCommit;       /* True to truncate WAL file on commit */
  u8 sync_శీర్షిక;             /* Fsync the WAL header if true */
  u8 pad_గమ్యం_Sector_Boundary;    /* Pad transactions out to the next sector */
  u8 bShmUnreliable;         /* SHM content is read-only and unreliable */
  Wal_సూచీ_Hdr hdr;           /* Wal-index header for current transaction */
  u32 min_చట్రం;              /* Ignore wal frames before this one */
  u32 iReCksum;              /* On commit, recalculate checksums from here */
  స్థిరరాశి అక్షరం *zWalName;      /* Name of WAL file */
  u32 nCkpt;                 /* Checkpoint sequence counter in the wal-header */
#నిర్వచితమైతే SQLITE_USE_SEH
  u32 తాళం_ముసుగు;              /* Mask of locks held */
  శూన్య *pFree;               /* Pointer to sqlite3_free() if exception thrown */
  u32 *p_Wi_విలువ;             /* Value to write into apWiData[iWiPg] */
  పూర్ణాంకం iWiPg;                 /* Write pWiValue into apWiData[iWiPg] */
  పూర్ణాంకం iSysErrno;             /* System error code following exception */
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_DEBUG
  పూర్ణాంకం nSehTry;               /* Number of nested SEH_TRY{} blocks */
  u8 తాళం_Error;              /* True if a locking error has occurred */
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_SNAPSHOT
  Wal_సూచీ_Hdr *pSnapshot;    /* Start transaction here if not NULL */
  పూర్ణాంకం bGetSnapshot;          /* Transaction opened for sqlite3_get_snapshot() */
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  sqlite3 *db;
#అయితే_ముగింపు
};

/*
** Candidate values for Wal.exclusiveMode.
*/
#నిర్వచించు WAL_సాధారణ_బహుళకం     0
#నిర్వచించు WAL_EXCLUSIVE_బహుళకం  1
#నిర్వచించు WAL_HEAPMEMORY_బహుళకం 2

/*
** Possible values for WAL.readOnly
*/
#నిర్వచించు WAL_RDWR        0    /* Normal read/write connection */
#నిర్వచించు WAL_RDONLY      1    /* The WAL file is readonly */
#నిర్వచించు WAL_SHM_RDONLY  2    /* The SHM file is readonly */

/*
** Each page of the wal-index mapping contains a hash-table made up of
** an array of HASHTABLE_NSLOT elements of the following type.
*/
రకనిర్వచనం u16 ht_స్థానం;

/*
** This structure is used to implement an iterator that loops through
** all frames in the WAL in database page order. Where two or more frames
** correspond to the same database page, the iterator visits only the
** frame most recently written to the WAL (in other words, the frame with
** the largest index).
**
** The internals of this structure are only accessed by:
**
**   walIteratorInit() - Create a new iterator,
**   walIteratorNext() - Step an iterator,
**   walIteratorFree() - Free an iterator.
**
** This functionality is used by the checkpoint code (see walCheckpoint()).
**
** There are two possible algorithms:
**
**   WALITER-1     Load all pages mentioned in the WAL file and sort them,
**                 then hand them back out in sorted order as requested by
**                 walIteratorNext().  nPagemap==0 for this algorithm.
**
**   WALITER-2     Create a map of all database file pages and record the
**                 last WAL file frame that updates that page.  Then walk
**                 through the map in acsending order and return all
**                 non-zero entries. nPagemap is positive for this algorithm.
**
** Either algorithm should work for any database and WAL file, however
** algorithm WALITER-1 is faster and uses less memory when the database is
** larger than the WAL file and algorithm WALITER-2 is faster and uses
** less memory when the WAL file is larger than the database, modulo a
** constant of proportionality.  The walIteratorInit() routine selects
** the best algorithm for each situation.  WALITER-2 was added for
** SQLite 3.54.0.  Prior to that, WALITER-1 was used always.
** 
** For algorithm WALITER-2, nPagemap stores the number of pages in the
** database file, nSegment==1, and aSegment[0].aPgno[] is the page map array
** that is nPagemap+1 entries in length.  The value of aSegment[0].aPgno[PGNO]
** is the frame number of the last (newest) entry for page PGNO in the WAL
** file, or 0 if PGNO is not in the WAL file.  (Aside: There is no page
** number 0 in a well-formed database, so the memory allocation is arranged
** such that aSegment[0].aPgno[0] and aSegment[0].iZero are aliases for the
** same four bits of memory.  This saves us from having to use -1 and +1
** offsets to translate between 1-based SQLite page numbers and 0-based
** C-language index numbers.  tag-20260903-1)
*/
నిర్మాణం WalIterator {
  u32 i_పూర్వ;                     /* Last result returned from the iterator */
  పూర్ణాంకం nSegment;                   /* Number of entries in aSegment[] */
  u32 nPagemap;                   /* Non-zero in "page-map" mode */
  నిర్మాణం WalSegment {
    పూర్ణాంకం i_తదుపరి;                    /* Next slot in aIndex[] not yet returned */
    ht_స్థానం *a_సూచీ;              /* i0, i1, i2... such that aPgno[iN] ascend */
    u32 *aPgno;                   /* Array of page numbers. */
    పూర్ణాంకం n_నమోదు;                   /* Nr. of entries in aPgno[] and aIndex[] */
    పూర్ణాంకం iZero;                    /* Frame number associated with aPgno[0] */
  } aSegment[FLEXARRAY];          /* One for every 32KB page in the wal-index */
};

/* Size (in bytes) of a WalIterator object suitable for N or fewer segments */
#నిర్వచించు SZ_WALITERATOR(N)  \
     (offsetof(WalIterator,aSegment)+(N)*పరిమాణం(నిర్మాణం WalSegment))


/*
** Define the parameters of the hash tables in the wal-index file. There
** is a hash-table following every HASHTABLE_NPAGE page numbers in the
** wal-index.
**
** Changing any of these constants will alter the wal-index format and
** create incompatibilities.
*/
#నిర్వచించు HASHTABLE_NPAGE      4096                 /* Must be power of 2 */
#నిర్వచించు HASHTABLE_HASH_1     383                  /* Should be prime */
#నిర్వచించు HASHTABLE_NSLOT      (HASHTABLE_NPAGE*2)  /* Must be a power of 2 */

/*
** The block of page numbers associated with the first hash-table in a
** wal-index is smaller than usual. This is so that there is a complete
** hash-table on each aligned 32KB page of the wal-index.
*/
#నిర్వచించు HASHTABLE_NPAGE_ONE  (HASHTABLE_NPAGE - (WALINDEX_HDR_కొలత/పరిమాణం(u32)))

/* The wal-index is divided into pages of WALINDEX_PGSZ bytes each. */
#నిర్వచించు WALINDEX_PGSZ   (                                         \
    పరిమాణం(ht_స్థానం)*HASHTABLE_NSLOT + HASHTABLE_NPAGE*పరిమాణం(u32) \
)

/*
** Structured Exception Handling (SEH) is a Windows-specific technique
** for catching exceptions raised while accessing memory-mapped files.
**
** The -DSQLITE_USE_SEH compile-time option means to use SEH to catch and
** deal with system-level errors that arise during WAL -shm file processing.
** Without this compile-time option, any system-level faults that appear
** while accessing the memory-mapped -shm file will cause a process-wide
** signal to be deliver, which will more than likely cause the entire
** process to exit.
*/
#నిర్వచితమైతే SQLITE_USE_SEH
#అన్తర్భావ <Windows.h>

/* Beginning of a block of code in which an exception might occur */
# define SEH_TRY    __try { \
   ప్రతిజ్ఞా( walAssertLockmask(pWal) && pWal->nSehTry==0 ); \
   VVA_ONLY(pWal->nSehTry++);

/* The end of a block of code in which an exception might occur */
# define SEH_EXCEPT(X) \
   VVA_ONLY(pWal->nSehTry--); \
   ప్రతిజ్ఞా( pWal->nSehTry==0 ); \
   } __except( sehExceptionFilter(pWal, Get_Exception_సంకేతం(), GetExceptionInformation() ) ){ X }

/* Simulate a memory-mapping fault in the -shm file for testing purposes */
# define SEH_INJECT_FAULT sehInjectFault(pWal) 

/*
** The second argument is the return value of GetExceptionCode() for the 
** current exception. Return EXCEPTION_EXECUTE_HANDLER if the exception code
** indicates that the exception may have been caused by accessing the *-shm 
** file mapping. Or EXCEPTION_CONTINUE_SEARCH otherwise.
*/
స్థిర పూర్ణాంకం sehExceptionFilter(Wal *pWal, పూర్ణాంకం e_సంకేతం, EXCEPTION_సూచికలు *p){
  VVA_ONLY(pWal->nSehTry--);
  ఒకవేళ( e_సంకేతం==EXCEPTION_లోపల_పుట_ERROR ){
    ఒకవేళ( p && p->Exception_రికార్డు && p->Exception_రికార్డు->సంఖ్య_Parameters>=3 ){
      /* From MSDN: For this type of exception, the first element of the
      ** ExceptionInformation[] array is a read-write flag - 0 if the exception
      ** was thrown while reading, 1 if while writing. The second element is
      ** the virtual address being accessed. The "third array element specifies
      ** the underlying NTSTATUS code that resulted in the exception". */
      pWal->iSysErrno = (పూర్ణాంకం)p->Exception_రికార్డు->ExceptionInformation[2];
    }
    తిరిగి EXCEPTION_EXECUTE_HANDLER;
  }
  తిరిగి EXCEPTION_CONTINUE_శోధన;
}

/*
** If one is configured, invoke the xTestCallback callback with 650 as
** the argument. If it returns true, throw the same exception that is
** thrown by the system if the *-shm file mapping is accessed after it
** has been invalidated.
*/
స్థిర శూన్య sehInjectFault(Wal *pWal){
  పూర్ణాంకం res;
  ప్రతిజ్ఞా( pWal->nSehTry>0 );

  res = sqlite3FaultSim(650);
  ఒకవేళ( res!=0 ){
    ULONG_PTR aArg[3];
    aArg[0] = 0;
    aArg[1] = 0;
    aArg[2] = (ULONG_PTR)res;
    RaiseException(EXCEPTION_లోపల_పుట_ERROR, 0, 3, (స్థిరరాశి ULONG_PTR*)aArg);
  }
}

/*
** There are two ways to use this macro. To set a pointer to be freed
** if an exception is thrown:
**
**   SEH_FREE_ON_ERROR(0, pPtr);
**
** and to cancel the same:
**
**   SEH_FREE_ON_ERROR(pPtr, 0);
**
** In the first case, there must not already be a pointer registered to
** be freed. In the second case, pPtr must be the registered pointer.
*/
#నిర్వచించు SEH_FREE_ON_ERROR(X,Y) \
  ప్రతిజ్ఞా( (X==0 || Y==0) && pWal->pFree==X ); pWal->pFree = Y

/*
** There are two ways to use this macro. To arrange for pWal->apWiData[iPg]
** to be set to pValue if an exception is thrown:
**
**   SEH_SET_ON_ERROR(iPg, pValue);
**
** and to cancel the same:
**
**   SEH_SET_ON_ERROR(0, 0);
*/
#నిర్వచించు SEH_సమితి_ON_ERROR(X,Y)  pWal->iWiPg = X; pWal->p_Wi_విలువ = Y

#లేకపోతే
# define SEH_TRY          VVA_ONLY(pWal->nSehTry++);
# define SEH_EXCEPT(X)    VVA_ONLY(pWal->nSehTry--); ప్రతిజ్ఞా( pWal->nSehTry==0 );
# define SEH_INJECT_FAULT ప్రతిజ్ఞా( pWal->nSehTry>0 );
# define SEH_FREE_ON_ERROR(X,Y)
# define SEH_సమితి_ON_ERROR(X,Y)
#అయితే_ముగింపు /* ifdef SQLITE_USE_SEH */


/*
** Obtain a pointer to the iPage'th page of the wal-index. The wal-index
** is broken into pages of WALINDEX_PGSZ bytes. Wal-index pages are
** numbered from zero.
**
** If the wal-index is currently smaller the iPage pages then the size
** of the wal-index might be increased, but only if it is safe to do
** so.  It is safe to enlarge the wal-index if pWal->writeLock is true
** or pWal->exclusiveMode==WAL_HEAPMEMORY_MODE.
**
** Three possible result scenarios:
**
**   (1)  rc==SQLITE_OK    and *ppPage==Requested-Wal-Index-Page
**   (2)  rc>=SQLITE_ERROR and *ppPage==NULL
**   (3)  rc==SQLITE_OK    and *ppPage==NULL  // only if iPage==0
**
** Scenario (3) can only occur when pWal->writeLock is false and iPage==0
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం wal_సూచీ_పుట_Realloc(
  Wal *pWal,               /* The WAL context */
  పూర్ణాంకం i_పుట,               /* The page we seek */
  అస్థిర u32 **pp_పుట    /* Write the page pointer here */
){
  పూర్ణాంకం rc = SQLITE_OK;

  /* Enlarge the pWal->apWiData[] array if required */
  ఒకవేళ( pWal->n_Wi_దత్తాంశం<=i_పుట ){
    sqlite3_int64 nByte = పరిమాణం(u32*)*(1+(i64)i_పుట);
    అస్థిర u32 **apNew;
    apNew = (అస్థిర u32 **)sqlite3Realloc((శూన్య *)pWal->ap_Wi_దత్తాంశం, nByte);
    ఒకవేళ( !apNew ){
      *pp_పుట = 0;
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    memset((శూన్య*)&apNew[pWal->n_Wi_దత్తాంశం], 0,
           పరిమాణం(u32*)*(i_పుట+1-pWal->n_Wi_దత్తాంశం));
    pWal->ap_Wi_దత్తాంశం = apNew;
    pWal->n_Wi_దత్తాంశం = i_పుట+1;
  }

  /* Request a pointer to the required page from the VFS */
  ప్రతిజ్ఞా( pWal->ap_Wi_దత్తాంశం[i_పుట]==0 );
  ఒకవేళ( pWal->exclusive_బహుళకం==WAL_HEAPMEMORY_బహుళకం ){
    pWal->ap_Wi_దత్తాంశం[i_పుట] = (u32 అస్థిర *)sqlite3MallocZero(WALINDEX_PGSZ);
    ఒకవేళ( !pWal->ap_Wi_దత్తాంశం[i_పుట] ) rc = SQLITE_NOMEM_BKPT;
  }లేకపోతే{
    rc = sqlite3OsShmMap(pWal->pDbFd, i_పుట, WALINDEX_PGSZ,
        pWal->write_తాళం, (శూన్య అస్థిర **)&pWal->ap_Wi_దత్తాంశం[i_పుట]
    );
    ప్రతిజ్ఞా( pWal->ap_Wi_దత్తాంశం[i_పుట]!=0
         || rc!=SQLITE_OK
         || (pWal->write_తాళం==0 && i_పుట==0) );
    testcase( pWal->ap_Wi_దత్తాంశం[i_పుట]==0 && rc==SQLITE_OK );
    ఒకవేళ( rc==SQLITE_OK ){
      ఒకవేళ( i_పుట>0 && sqlite3FaultSim(600) ) rc = SQLITE_NOMEM;
    }లేకపోతే ఒకవేళ( (rc&0xff)==SQLITE_READONLY ){
      pWal->readOnly |= WAL_SHM_RDONLY;
      ఒకవేళ( rc==SQLITE_READONLY ){
        rc = SQLITE_OK;
      }
    }
  }

  *pp_పుట = pWal->ap_Wi_దత్తాంశం[i_పుట];
  ప్రతిజ్ఞా( i_పుట==0 || *pp_పుట || rc!=SQLITE_OK );
  తిరిగి rc;
}
స్థిర పూర్ణాంకం wal_సూచీ_పుట(
  Wal *pWal,               /* The WAL context */
  పూర్ణాంకం i_పుట,               /* The page we seek */
  అస్థిర u32 **pp_పుట    /* Write the page pointer here */
){
  SEH_INJECT_FAULT;
  ఒకవేళ( pWal->n_Wi_దత్తాంశం<=i_పుట || (*pp_పుట = pWal->ap_Wi_దత్తాంశం[i_పుట])==0 ){
    తిరిగి wal_సూచీ_పుట_Realloc(pWal, i_పుట, pp_పుట);
  }
  తిరిగి SQLITE_OK;
}

/*
** Return a pointer to the WalCkptInfo structure in the wal-index.
*/
స్థిర అస్థిర WalCkptInfo *walCkptInfo(Wal *pWal){
  ప్రతిజ్ఞా( pWal->n_Wi_దత్తాంశం>0 && pWal->ap_Wi_దత్తాంశం[0] );
  SEH_INJECT_FAULT;
  తిరిగి (అస్థిర WalCkptInfo*)&(pWal->ap_Wi_దత్తాంశం[0][పరిమాణం(Wal_సూచీ_Hdr)/2]);
}

/*
** Return a pointer to the WalIndexHdr structure in the wal-index.
*/
స్థిర అస్థిర Wal_సూచీ_Hdr *wal_సూచీ_Hdr(Wal *pWal){
  ప్రతిజ్ఞా( pWal->n_Wi_దత్తాంశం>0 && pWal->ap_Wi_దత్తాంశం[0] );
  SEH_INJECT_FAULT;
  తిరిగి (అస్థిర Wal_సూచీ_Hdr*)pWal->ap_Wi_దత్తాంశం[0];
}

/*
** The argument to this macro must be of type u32. On a little-endian
** architecture, it returns the u32 value that results from interpreting
** the 4 bytes as a big-endian value. On a big-endian architecture, it
** returns the value that would be produced by interpreting the 4 bytes
** of the input value as a little-endian integer.
*/
#నిర్వచించు BYTESWAP32(x) ( \
    (((x)&0x000000FF)<<24) + (((x)&0x0000FF00)<<8)  \
  + (((x)&0x00FF0000)>>8)  + (((x)&0xFF000000)>>24) \
)

/*
** Generate or extend an 8 byte checksum based on the data in
** array aByte[] and the initial values of aIn[0] and aIn[1] (or
** initial values of 0 and 0 if aIn==NULL).
**
** The checksum is written back into aOut[] before returning.
**
** nByte must be a positive multiple of 8.
*/
స్థిర శూన్య wal_Checksum_బైట్లు(
  పూర్ణాంకం nativeCksum, /* True for native byte-order, false for non-native */
  u8 *a,           /* Content to be checksummed */
  పూర్ణాంకం nByte,       /* Bytes of content in a[].  Must be a multiple of 8. */
  స్థిరరాశి u32 *a_లోపల,  /* Initial checksum value input */
  u32 *a_బహిర్గతం        /* OUT: Final checksum value output */
){
  u32 s1, s2;
  u32 *a_దత్తాంశం = (u32 *)a;
  u32 *a_చివర = (u32 *)&a[nByte];

  ఒకవేళ( a_లోపల ){
    s1 = a_లోపల[0];
    s2 = a_లోపల[1];
  }లేకపోతే{
    s1 = s2 = 0;
  }

  /* nByte is a multiple of 8 between 8 and 65536 */
  ప్రతిజ్ఞా( nByte>=8 && (nByte&7)==0 && nByte<=65536 );

  ఒకవేళ( !nativeCksum ){
    చేయి {
      s1 += BYTESWAP32(a_దత్తాంశం[0]) + s2;
      s2 += BYTESWAP32(a_దత్తాంశం[1]) + s1;
      a_దత్తాంశం += 2;
    }అయితే( a_దత్తాంశం<a_చివర );
  }లేకపోతే ఒకవేళ( nByte%64==0 ){
    చేయి {
      s1 += *a_దత్తాంశం++ + s2;
      s2 += *a_దత్తాంశం++ + s1;
      s1 += *a_దత్తాంశం++ + s2;
      s2 += *a_దత్తాంశం++ + s1;
      s1 += *a_దత్తాంశం++ + s2;
      s2 += *a_దత్తాంశం++ + s1;
      s1 += *a_దత్తాంశం++ + s2;
      s2 += *a_దత్తాంశం++ + s1;
      s1 += *a_దత్తాంశం++ + s2;
      s2 += *a_దత్తాంశం++ + s1;
      s1 += *a_దత్తాంశం++ + s2;
      s2 += *a_దత్తాంశం++ + s1;
      s1 += *a_దత్తాంశం++ + s2;
      s2 += *a_దత్తాంశం++ + s1;
      s1 += *a_దత్తాంశం++ + s2;
      s2 += *a_దత్తాంశం++ + s1;
    }అయితే( a_దత్తాంశం<a_చివర );
  }లేకపోతే{
    చేయి {
      s1 += *a_దత్తాంశం++ + s2;
      s2 += *a_దత్తాంశం++ + s1;
    }అయితే( a_దత్తాంశం<a_చివర );
  }
  ప్రతిజ్ఞా( a_దత్తాంశం==a_చివర );

  a_బహిర్గతం[0] = s1;
  a_బహిర్గతం[1] = s2;
}

/*
** If there is the possibility of concurrent access to the SHM file
** from multiple threads and/or processes, then do a memory barrier.
*/
స్థిర శూన్య walShmBarrier(Wal *pWal){
  ఒకవేళ( pWal->exclusive_బహుళకం!=WAL_HEAPMEMORY_బహుళకం ){
    sqlite3OsShmBarrier(pWal->pDbFd);
  }
}

/*
** Add the SQLITE_NO_TSAN as part of the return-type of a function
** definition as a hint that the function contains constructs that
** might give false-positive TSAN warnings.
**
** See tag-20200519-1.
*/
#ఒకవేళ defined(__clang__) && !defined(SQLITE_NO_TSAN)
# define SQLITE_NO_TSAN __attribute__((no_sanitize_thread))
#లేకపోతే
# define SQLITE_NO_TSAN
#అయితే_ముగింపు

/*
** Write the header information in pWal->hdr into the wal-index.
**
** The checksum on pWal->hdr is updated before it is written.
*/
స్థిర SQLITE_NO_TSAN శూన్య wal_సూచీ_Write_Hdr(Wal *pWal){
  అస్థిర Wal_సూచీ_Hdr *aHdr = wal_సూచీ_Hdr(pWal);
  స్థిరరాశి పూర్ణాంకం nCksum = offsetof(Wal_సూచీ_Hdr, aCksum);

  ప్రతిజ్ఞా( pWal->write_తాళం );
  pWal->hdr.isInit = 1;
  pWal->hdr.i_సంస్కరణ = WALINDEX_MAX_సంస్కరణ;
  wal_Checksum_బైట్లు(1, (u8*)&pWal->hdr, nCksum, 0, pWal->hdr.aCksum);
  /* Possible TSAN false-positive.  See tag-20200519-1 */
  జ్ఞాపకం_నకలు((శూన్య*)&aHdr[1], (స్థిరరాశి శూన్య*)&pWal->hdr, పరిమాణం(Wal_సూచీ_Hdr));
  walShmBarrier(pWal);
  జ్ఞాపకం_నకలు((శూన్య*)&aHdr[0], (స్థిరరాశి శూన్య*)&pWal->hdr, పరిమాణం(Wal_సూచీ_Hdr));
}

/*
** This function encodes a single frame header and writes it to a buffer
** supplied by the caller. A frame-header is made up of a series of
** 4-byte big-endian integers, as follows:
**
**     0: Page number.
**     4: For commit records, the size of the database image in pages
**        after the commit. For all other records, zero.
**     8: Salt-1 (copied from the wal-header)
**    12: Salt-2 (copied from the wal-header)
**    16: Checksum-1.
**    20: Checksum-2.
*/
స్థిర శూన్య wal_Encode_చట్రం(
  Wal *pWal,                      /* The write-ahead log */
  u32 i_పుట,                      /* Database page number for frame */
  u32 nTruncate,                  /* New db size (or 0 for non-commit frames) */
  u8 *a_దత్తాంశం,                      /* Pointer to page data */
  u8 *a_చట్రం                      /* OUT: Write encoded frame here */
){
  పూర్ణాంకం nativeCksum;                /* True for native byte-order checksums */
  u32 *aCksum = pWal->hdr.a_చట్రం_Cksum;
  ప్రతిజ్ఞా( WAL_చట్రం_HDRSIZE==24 );
  sqlite3Put4byte(&a_చట్రం[0], i_పుట);
  sqlite3Put4byte(&a_చట్రం[4], nTruncate);
  ఒకవేళ( pWal->iReCksum==0 ){
    జ్ఞాపకం_నకలు(&a_చట్రం[8], pWal->hdr.aSalt, 8);

    nativeCksum = (pWal->hdr.big_చివర_Cksum==SQLITE_BIGENDIAN);
    wal_Checksum_బైట్లు(nativeCksum, a_చట్రం, 8, aCksum, aCksum);
    wal_Checksum_బైట్లు(nativeCksum, a_దత్తాంశం, pWal->sz_పుట, aCksum, aCksum);

    sqlite3Put4byte(&a_చట్రం[16], aCksum[0]);
    sqlite3Put4byte(&a_చట్రం[20], aCksum[1]);
  }లేకపోతే{
    memset(&a_చట్రం[8], 0, 16);
  }
}

/*
** Check to see if the frame with header in aFrame[] and content
** in aData[] is valid.  If it is a valid frame, fill *piPage and
** *pnTruncate and return true.  Return if the frame is not valid.
*/
స్థిర పూర్ణాంకం wal_Decode_చట్రం(
  Wal *pWal,                      /* The write-ahead log */
  u32 *pi_పుట,                    /* OUT: Database page number for frame */
  u32 *pnTruncate,                /* OUT: New db size (or 0 if not commit) */
  u8 *a_దత్తాంశం,                      /* Pointer to page data (for checksum) */
  u8 *a_చట్రం                      /* Frame data */
){
  పూర్ణాంకం nativeCksum;                /* True for native byte-order checksums */
  u32 *aCksum = pWal->hdr.a_చట్రం_Cksum;
  u32 pgno;                       /* Page number of the frame */
  ప్రతిజ్ఞా( WAL_చట్రం_HDRSIZE==24 );

  /* A frame is only valid if the salt values in the frame-header
  ** match the salt values in the wal-header.
  */
  ఒకవేళ( memcmp(&pWal->hdr.aSalt, &a_చట్రం[8], 8)!=0 ){
    తిరిగి 0;
  }

  /* A frame is only valid if the page number is greater than zero.
  */
  pgno = sqlite3Get4byte(&a_చట్రం[0]);
  ఒకవేళ( pgno==0 ){
    తిరిగి 0;
  }

  /* Need a valid page size
  */
  ఒకవేళ( !pWal->sz_పుట ){
    తిరిగి 0;
  }

  /* A frame is only valid if a checksum of the WAL header,
  ** all prior frames, the first 16 bytes of this frame-header,
  ** and the frame-data matches the checksum in the last 8
  ** bytes of this frame-header.
  */
  nativeCksum = (pWal->hdr.big_చివర_Cksum==SQLITE_BIGENDIAN);
  wal_Checksum_బైట్లు(nativeCksum, a_చట్రం, 8, aCksum, aCksum);
  wal_Checksum_బైట్లు(nativeCksum, a_దత్తాంశం, pWal->sz_పుట, aCksum, aCksum);
  ఒకవేళ( aCksum[0]!=sqlite3Get4byte(&a_చట్రం[16])
   || aCksum[1]!=sqlite3Get4byte(&a_చట్రం[20])
  ){
    /* Checksum failed. */
    తిరిగి 0;
  }

  /* If we reach this point, the frame is valid.  Return the page number
  ** and the new database size.
  */
  *pi_పుట = pgno;
  *pnTruncate = sqlite3Get4byte(&a_చట్రం[4]);
  తిరిగి 1;
}


#ఒకవేళ defined(SQLITE_TEST) && defined(SQLITE_DEBUG)
/*
** Names of locks.  This routine is used to provide debugging output and is not
** a part of an ordinary build.
*/
స్థిర స్థిరరాశి అక్షరం *wal_తాళం_Name(పూర్ణాంకం తాళం_Idx){
  ఒకవేళ( తాళం_Idx==WAL_WRITE_తాళం ){
    తిరిగి "WRITE-LOCK";
  }లేకపోతే ఒకవేళ( తాళం_Idx==WAL_CKPT_తాళం ){
    తిరిగి "CKPT-LOCK";
  }లేకపోతే ఒకవేళ( తాళం_Idx==WAL_RECOVER_తాళం ){
    తిరిగి "RECOVER-LOCK";
  }లేకపోతే{
    స్థిర అక్షరం zName[15];
    sqlite3_snprintf(పరిమాణం(zName), zName, "READ-LOCK[%d]",
                     తాళం_Idx-WAL_READ_తాళం(0));
    తిరిగి zName;
  }
}
#అయితే_ముగింపు /*defined(SQLITE_TEST) || defined(SQLITE_DEBUG) */


/*
** Set or release locks on the WAL.  Locks are either shared or exclusive.
** A lock cannot be moved directly between shared and exclusive - it must go
** through the unlocked state first.
**
** In locking_mode=EXCLUSIVE, all of these routines become no-ops.
*/
స్థిర పూర్ణాంకం wal_తాళం_Shared(Wal *pWal, పూర్ణాంకం తాళం_Idx){
  పూర్ణాంకం rc;
  ఒకవేళ( pWal->exclusive_బహుళకం ) తిరిగి SQLITE_OK;
  rc = sqlite3_Os_Shm_తాళం(pWal->pDbFd, తాళం_Idx, 1,
                        SQLITE_SHM_తాళం | SQLITE_SHM_SHARED);
  WALTRACE(("WAL%p: acquire SHARED-%s %s\n", pWal,
            wal_తాళం_Name(తాళం_Idx), rc ? "failed" : "ok"));
  VVA_ONLY( pWal->తాళం_Error = (u8)(rc!=SQLITE_OK && (rc&0xFF)!=SQLITE_BUSY); )
#నిర్వచితమైతే SQLITE_USE_SEH
  ఒకవేళ( rc==SQLITE_OK ) pWal->తాళం_ముసుగు |= (1 << తాళం_Idx);
#అయితే_ముగింపు
  తిరిగి rc;
}
స్థిర శూన్య walUnlockShared(Wal *pWal, పూర్ణాంకం తాళం_Idx){
  ఒకవేళ( pWal->exclusive_బహుళకం ) తిరిగి;
  (శూన్య)sqlite3_Os_Shm_తాళం(pWal->pDbFd, తాళం_Idx, 1,
                         SQLITE_SHM_UNLOCK | SQLITE_SHM_SHARED);
#నిర్వచితమైతే SQLITE_USE_SEH
  pWal->తాళం_ముసుగు &= ~(1 << తాళం_Idx);
#అయితే_ముగింపు
  WALTRACE(("WAL%p: release SHARED-%s\n", pWal, wal_తాళం_Name(తాళం_Idx)));
}
స్థిర పూర్ణాంకం wal_తాళం_Exclusive(Wal *pWal, పూర్ణాంకం తాళం_Idx, పూర్ణాంకం n){
  పూర్ణాంకం rc;
  ఒకవేళ( pWal->exclusive_బహుళకం ) తిరిగి SQLITE_OK;
  rc = sqlite3_Os_Shm_తాళం(pWal->pDbFd, తాళం_Idx, n,
                        SQLITE_SHM_తాళం | SQLITE_SHM_EXCLUSIVE);
  WALTRACE(("WAL%p: acquire EXCLUSIVE-%s cnt=%d %s\n", pWal,
            wal_తాళం_Name(తాళం_Idx), n, rc ? "failed" : "ok"));
  VVA_ONLY( pWal->తాళం_Error = (u8)(rc!=SQLITE_OK && (rc&0xFF)!=SQLITE_BUSY); )
#నిర్వచితమైతే SQLITE_USE_SEH
  ఒకవేళ( rc==SQLITE_OK ){
    pWal->తాళం_ముసుగు |= (((1<<n)-1) << (SQLITE_SHM_NLOCK+తాళం_Idx));
  }
#అయితే_ముగింపు
  తిరిగి rc;
}
స్థిర శూన్య walUnlockExclusive(Wal *pWal, పూర్ణాంకం తాళం_Idx, పూర్ణాంకం n){
  ఒకవేళ( pWal->exclusive_బహుళకం ) తిరిగి;
  (శూన్య)sqlite3_Os_Shm_తాళం(pWal->pDbFd, తాళం_Idx, n,
                         SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE);
#నిర్వచితమైతే SQLITE_USE_SEH
  pWal->తాళం_ముసుగు &= ~(((1<<n)-1) << (SQLITE_SHM_NLOCK+తాళం_Idx));
#అయితే_ముగింపు
  WALTRACE(("WAL%p: release EXCLUSIVE-%s cnt=%d\n", pWal,
             wal_తాళం_Name(తాళం_Idx), n));
}

/*
** Compute a hash on a page number.  The resulting hash value must land
** between 0 and (HASHTABLE_NSLOT-1).  The walNextHash() function advances
** the hash to the next value in the event of a collision.
*/
స్థిర పూర్ణాంకం walHash(u32 i_పుట){
  ప్రతిజ్ఞా( i_పుట>0 );
  ప్రతిజ్ఞా( (HASHTABLE_NSLOT & (HASHTABLE_NSLOT-1))==0 );
  తిరిగి (i_పుట*HASHTABLE_HASH_1) & (HASHTABLE_NSLOT-1);
}
స్థిర పూర్ణాంకం wal_తదుపరి_Hash(పూర్ణాంకం i_పూర్వ_Hash){
  తిరిగి (i_పూర్వ_Hash+1)&(HASHTABLE_NSLOT-1);
}

/*
** An instance of the WalHashLoc object is used to describe the location
** of a page hash table in the wal-index.  This becomes the return value
** from walHashGet().
*/
రకనిర్వచనం నిర్మాణం WalHashLoc WalHashLoc;
నిర్మాణం WalHashLoc {
  అస్థిర ht_స్థానం *aHash;  /* Start of the wal-index hash table */
  అస్థిర u32 *aPgno;      /* aPgno[1] is the page of first frame indexed */
  u32 iZero;                /* One less than the frame number of first indexed*/
};

/*
** Return pointers to the hash table and page number array stored on
** page iHash of the wal-index. The wal-index is broken into 32KB pages
** numbered starting from 0.
**
** Set output variable pLoc->aHash to point to the start of the hash table
** in the wal-index file. Set pLoc->iZero to one less than the frame
** number of the first frame indexed by this hash table. If a
** slot in the hash table is set to N, it refers to frame number
** (pLoc->iZero+N) in the log.
**
** Finally, set pLoc->aPgno so that pLoc->aPgno[0] is the page number of the
** first frame indexed by the hash table, frame (pLoc->iZero).
*/
స్థిర పూర్ణాంకం walHashGet(
  Wal *pWal,                      /* WAL handle */
  పూర్ణాంకం iHash,                      /* Find the iHash'th table */
  WalHashLoc *pLoc                /* OUT: Hash table location */
){
  పూర్ణాంకం rc;                         /* Return code */

  rc = wal_సూచీ_పుట(pWal, iHash, &pLoc->aPgno);
  ప్రతిజ్ఞా( rc==SQLITE_OK || iHash>0 );

  ఒకవేళ( pLoc->aPgno ){
    pLoc->aHash = (అస్థిర ht_స్థానం *)&pLoc->aPgno[HASHTABLE_NPAGE];
    ఒకవేళ( iHash==0 ){
      pLoc->aPgno = &pLoc->aPgno[WALINDEX_HDR_కొలత/పరిమాణం(u32)];
      pLoc->iZero = 0;
    }లేకపోతే{
      pLoc->iZero = HASHTABLE_NPAGE_ONE + (iHash-1)*HASHTABLE_NPAGE;
    }
  }లేకపోతే ఒకవేళ( NEVER(rc==SQLITE_OK) ){
    rc = SQLITE_ERROR;
  }
  తిరిగి rc;
}

/*
** Return the number of the wal-index page that contains the hash-table
** and page-number array that contain entries corresponding to WAL frame
** iFrame. The wal-index is broken up into 32KB pages. Wal-index pages
** are numbered starting from 0.
*/
స్థిర పూర్ణాంకం wal_చట్రం_పుట(u32 i_చట్రం){
  పూర్ణాంకం iHash = (i_చట్రం+HASHTABLE_NPAGE-HASHTABLE_NPAGE_ONE-1) / HASHTABLE_NPAGE;
  ప్రతిజ్ఞా( (iHash==0 || i_చట్రం>HASHTABLE_NPAGE_ONE)
       && (iHash>=1 || i_చట్రం<=HASHTABLE_NPAGE_ONE)
       && (iHash<=1 || i_చట్రం>(HASHTABLE_NPAGE_ONE+HASHTABLE_NPAGE))
       && (iHash>=2 || i_చట్రం<=HASHTABLE_NPAGE_ONE+HASHTABLE_NPAGE)
       && (iHash<=2 || i_చట్రం>(HASHTABLE_NPAGE_ONE+2*HASHTABLE_NPAGE))
  );
  ప్రతిజ్ఞా( iHash>=0 );
  తిరిగి iHash;
}

/*
** Return the page number associated with frame iFrame in this WAL.
*/
స్థిర u32 wal_చట్రం_Pgno(Wal *pWal, u32 i_చట్రం){
  పూర్ణాంకం iHash = wal_చట్రం_పుట(i_చట్రం);
  SEH_INJECT_FAULT;
  ఒకవేళ( iHash==0 ){
    తిరిగి pWal->ap_Wi_దత్తాంశం[0][WALINDEX_HDR_కొలత/పరిమాణం(u32) + i_చట్రం - 1];
  }
  తిరిగి pWal->ap_Wi_దత్తాంశం[iHash][(i_చట్రం-1-HASHTABLE_NPAGE_ONE)%HASHTABLE_NPAGE];
}

/*
** Remove entries from the hash table that point to WAL slots greater
** than pWal->hdr.mxFrame.
**
** This function is called whenever pWal->hdr.mxFrame is decreased due
** to a rollback or savepoint.
**
** At most only the hash table containing pWal->hdr.mxFrame needs to be
** updated.  Any later hash tables will be automatically cleared when
** pWal->hdr.mxFrame advances to the point where those hash tables are
** actually needed.
*/
స్థిర శూన్య walCleanupHash(Wal *pWal){
  WalHashLoc sLoc;                /* Hash table location */
  పూర్ణాంకం i_హద్దు = 0;                 /* Zero values greater than this */
  పూర్ణాంకం nByte;                      /* Number of bytes to zero in aPgno[] */
  పూర్ణాంకం i;                          /* Used to iterate through aHash[] */

  ప్రతిజ్ఞా( pWal->write_తాళం );
  testcase( pWal->hdr.mx_చట్రం==HASHTABLE_NPAGE_ONE-1 );
  testcase( pWal->hdr.mx_చట్రం==HASHTABLE_NPAGE_ONE );
  testcase( pWal->hdr.mx_చట్రం==HASHTABLE_NPAGE_ONE+1 );

  ఒకవేళ( pWal->hdr.mx_చట్రం==0 ) తిరిగి;

  /* Obtain pointers to the hash-table and page-number array containing
  ** the entry that corresponds to frame pWal->hdr.mxFrame. It is guaranteed
  ** that the page said hash-table and array reside on is already mapped.(1)
  */
  ప్రతిజ్ఞా( pWal->n_Wi_దత్తాంశం>wal_చట్రం_పుట(pWal->hdr.mx_చట్రం) );
  ప్రతిజ్ఞా( pWal->ap_Wi_దత్తాంశం[wal_చట్రం_పుట(pWal->hdr.mx_చట్రం)] );
  i = walHashGet(pWal, wal_చట్రం_పుట(pWal->hdr.mx_చట్రం), &sLoc);
  ఒకవేళ( NEVER(i) ) తిరిగి; /* Defense-in-depth, in case (1) above is wrong */

  /* Zero all hash-table entries that correspond to frame numbers greater
  ** than pWal->hdr.mxFrame.
  */
  i_హద్దు = pWal->hdr.mx_చట్రం - sLoc.iZero;
  ప్రతిజ్ఞా( i_హద్దు>0 );
  కొరకు(i=0; i<HASHTABLE_NSLOT; i++){
    ఒకవేళ( sLoc.aHash[i]>i_హద్దు ){
      sLoc.aHash[i] = 0;
    }
  }

  /* Zero the entries in the aPgno array that correspond to frames with
  ** frame numbers greater than pWal->hdr.mxFrame.
  */
  nByte = (పూర్ణాంకం)((అక్షరం *)sLoc.aHash - (అక్షరం *)&sLoc.aPgno[i_హద్దు]);
  ప్రతిజ్ఞా( nByte>=0 );
  memset((శూన్య *)&sLoc.aPgno[i_హద్దు], 0, nByte);

#నిర్వచితమైతే SQLITE_ENABLE_EXPENSIVE_ASSERT
  /* Verify that the every entry in the mapping region is still reachable
  ** via the hash table even after the cleanup.
  */
  ఒకవేళ( i_హద్దు ){
    పూర్ణాంకం j;           /* Loop counter */
    పూర్ణాంకం i_కీలకం;        /* Hash key */
    కొరకు(j=0; j<i_హద్దు; j++){
      కొరకు(i_కీలకం=walHash(sLoc.aPgno[j]);sLoc.aHash[i_కీలకం];i_కీలకం=wal_తదుపరి_Hash(i_కీలకం)){
        ఒకవేళ( sLoc.aHash[i_కీలకం]==j+1 ) ఆపు;
      }
      ప్రతిజ్ఞా( sLoc.aHash[i_కీలకం]==j+1 );
    }
  }
#అయితే_ముగింపు /* SQLITE_ENABLE_EXPENSIVE_ASSERT */
}


/*
** Set an entry in the wal-index that will map database page number
** pPage into WAL frame iFrame.
*/
స్థిర పూర్ణాంకం wal_సూచీ_Append(Wal *pWal, u32 i_చట్రం, u32 i_పుట){
  పూర్ణాంకం rc;                         /* Return code */
  WalHashLoc sLoc;                /* Wal-index hash table location */

  rc = walHashGet(pWal, wal_చట్రం_పుట(i_చట్రం), &sLoc);

  /* Assuming the wal-index file was successfully mapped, populate the
  ** page number array and hash table entry.
  */
  ఒకవేళ( rc==SQLITE_OK ){
    పూర్ణాంకం i_కీలకం;                     /* Hash table key */
    పూర్ణాంకం idx;                      /* Value to write to hash-table slot */
    పూర్ణాంకం nCollide;                 /* Number of hash collisions */

    idx = i_చట్రం - sLoc.iZero;
    ప్రతిజ్ఞా( idx <= HASHTABLE_NSLOT/2 + 1 );

    /* If this is the first entry to be added to this hash-table, zero the
    ** entire hash table and aPgno[] array before proceeding.
    */
    ఒకవేళ( idx==1 ){
      పూర్ణాంకం nByte = (పూర్ణాంకం)((u8*)&sLoc.aHash[HASHTABLE_NSLOT] - (u8*)sLoc.aPgno);
      ప్రతిజ్ఞా( nByte>=0 );
      memset((శూన్య*)sLoc.aPgno, 0, nByte);
    }

    /* If the entry in aPgno[] is already set, then the previous writer
    ** must have exited unexpectedly in the middle of a transaction (after
    ** writing one or more dirty pages to the WAL to free up memory).
    ** Remove the remnants of that writers uncommitted transaction from
    ** the hash-table before writing any new entries.
    */
    ఒకవేళ( sLoc.aPgno[idx-1] ){
      walCleanupHash(pWal);
      ప్రతిజ్ఞా( !sLoc.aPgno[idx-1] );
    }

    /* Write the aPgno[] array entry and the hash-table slot. */
    nCollide = idx;
    కొరకు(i_కీలకం=walHash(i_పుట); sLoc.aHash[i_కీలకం]; i_కీలకం=wal_తదుపరి_Hash(i_కీలకం)){
      ఒకవేళ( (nCollide--)==0 ) తిరిగి SQLITE_CORRUPT_BKPT;
    }
    sLoc.aPgno[(idx-1)&(HASHTABLE_NPAGE-1)] = i_పుట;
    AtomicStore(&sLoc.aHash[i_కీలకం], (ht_స్థానం)idx);

#నిర్వచితమైతే SQLITE_ENABLE_EXPENSIVE_ASSERT
    /* Verify that the number of entries in the hash table exactly equals
    ** the number of entries in the mapping region.
    */
    {
      పూర్ణాంకం i;           /* Loop counter */
      పూర్ణాంకం n_నమోదు = 0;  /* Number of entries in the hash table */
      కొరకు(i=0; i<HASHTABLE_NSLOT; i++){ ఒకవేళ( sLoc.aHash[i] ) n_నమోదు++; }
      ప్రతిజ్ఞా( n_నమోదు==idx );
    }

    /* Verify that the every entry in the mapping region is reachable
    ** via the hash table.  This turns out to be a really, really expensive
    ** thing to check, so only do this occasionally - not on every
    ** iteration.
    */
    ఒకవేళ( (idx&0x3ff)==0 ){
      పూర్ణాంకం i;           /* Loop counter */
      కొరకు(i=0; i<idx; i++){
        కొరకు(i_కీలకం=walHash(sLoc.aPgno[i]);
            sLoc.aHash[i_కీలకం];
            i_కీలకం=wal_తదుపరి_Hash(i_కీలకం)){
          ఒకవేళ( sLoc.aHash[i_కీలకం]==i+1 ) ఆపు;
        }
        ప్రతిజ్ఞా( sLoc.aHash[i_కీలకం]==i+1 );
      }
    }
#అయితే_ముగింపు /* SQLITE_ENABLE_EXPENSIVE_ASSERT */
  }

  తిరిగి rc;
}


/*
** Recover the wal-index by reading the write-ahead log file.
**
** This routine first tries to establish an exclusive lock on the
** wal-index to prevent other threads/processes from doing anything
** with the WAL or wal-index while recovery is running.  The
** WAL_RECOVER_LOCK is also held so that other threads will know
** that this thread is running recovery.  If unable to establish
** the necessary locks, this routine returns SQLITE_BUSY.
*/
స్థిర పూర్ణాంకం wal_సూచీ_Recover(Wal *pWal){
  పూర్ణాంకం rc;                         /* Return Code */
  i64 n_కొలత;                      /* Size of log file */
  u32 a_చట్రం_Cksum[2] = {0, 0};
  పూర్ణాంకం i_తాళం;                      /* Lock offset to lock for checkpoint */

  /* Obtain an exclusive lock on all byte in the locking range not already
  ** locked by the caller. The caller is guaranteed to have locked the
  ** WAL_WRITE_LOCK byte, and may have also locked the WAL_CKPT_LOCK byte.
  ** If successful, the same bytes that are locked here are unlocked before
  ** this function returns.
  */
  ప్రతిజ్ఞా( pWal->ckpt_తాళం==1 || pWal->ckpt_తాళం==0 );
  ప్రతిజ్ఞా( WAL_ALL_BUT_WRITE==WAL_WRITE_తాళం+1 );
  ప్రతిజ్ఞా( WAL_CKPT_తాళం==WAL_ALL_BUT_WRITE );
  ప్రతిజ్ఞా( pWal->write_తాళం );
  i_తాళం = WAL_ALL_BUT_WRITE + pWal->ckpt_తాళం;
  rc = wal_తాళం_Exclusive(pWal, i_తాళం, WAL_READ_తాళం(0)-i_తాళం);
  ఒకవేళ( rc ){
    తిరిగి rc;
  }

  WALTRACE(("WAL%p: recovery begin...\n", pWal));

  memset(&pWal->hdr, 0, పరిమాణం(Wal_సూచీ_Hdr));

  rc = sqlite3_Os_File_కొలత(pWal->pWalFd, &n_కొలత);
  ఒకవేళ( rc!=SQLITE_OK ){
    వెళ్ళు recovery_error;
  }

  ఒకవేళ( n_కొలత>WAL_HDRSIZE ){
    u8 aBuf[WAL_HDRSIZE];         /* Buffer to load WAL header into */
    u32 *a_వ్యక్తిగత = 0;            /* Heap copy of *-shm hash being populated */
    u8 *a_చట్రం = 0;               /* Malloc'd buffer to load entire frame */
    పూర్ణాంకం sz_చట్రం;                  /* Number of bytes in buffer aFrame[] */
    u8 *a_దత్తాంశం;                    /* Pointer to data part of aFrame buffer */
    పూర్ణాంకం sz_పుట;                   /* Page size according to the log */
    u32 magic;                    /* Magic value read from WAL header */
    u32 సంస్కరణ_2;                  /* Magic value read from WAL header */
    పూర్ణాంకం isValid;                  /* True if this frame is valid */
    u32 iPg;                      /* Current 32KB wal-index page */
    u32 i_చివరి_చట్రం;               /* Last frame in wal, based on nSize alone */

    /* Read in the WAL header. */
    rc = sqlite3OsRead(pWal->pWalFd, aBuf, WAL_HDRSIZE, 0);
    ఒకవేళ( rc!=SQLITE_OK ){
      వెళ్ళు recovery_error;
    }

    /* If the database page size is not a power of two, or is greater than
    ** SQLITE_MAX_PAGE_SIZE, conclude that the WAL file contains no valid
    ** data. Similarly, if the 'magic' value is invalid, ignore the whole
    ** WAL file.
    */
    magic = sqlite3Get4byte(&aBuf[0]);
    sz_పుట = sqlite3Get4byte(&aBuf[8]);
    ఒకవేళ( (magic&0xFFFFFFFE)!=WAL_MAGIC
     || sz_పుట&(sz_పుట-1)
     || sz_పుట>SQLITE_MAX_పుట_కొలత
     || sz_పుట<512
    ){
      వెళ్ళు ముగిసిన;
    }
    pWal->hdr.big_చివర_Cksum = (u8)(magic&0x00000001);
    pWal->sz_పుట = sz_పుట;
    pWal->nCkpt = sqlite3Get4byte(&aBuf[12]);
    జ్ఞాపకం_నకలు(&pWal->hdr.aSalt, &aBuf[16], 8);

    /* Verify that the WAL header checksum is correct */
    wal_Checksum_బైట్లు(pWal->hdr.big_చివర_Cksum==SQLITE_BIGENDIAN,
        aBuf, WAL_HDRSIZE-2*4, 0, pWal->hdr.a_చట్రం_Cksum
    );
    ఒకవేళ( pWal->hdr.a_చట్రం_Cksum[0]!=sqlite3Get4byte(&aBuf[24])
     || pWal->hdr.a_చట్రం_Cksum[1]!=sqlite3Get4byte(&aBuf[28])
    ){
      వెళ్ళు ముగిసిన;
    }

    /* Verify that the version number on the WAL format is one that
    ** are able to understand */
    సంస్కరణ_2 = sqlite3Get4byte(&aBuf[4]);
    ఒకవేళ( సంస్కరణ_2!=WAL_MAX_సంస్కరణ ){
      rc = SQLITE_CANTOPEN_BKPT;
      వెళ్ళు ముగిసిన;
    }

    /* Malloc a buffer to read frames into. */
    sz_చట్రం = sz_పుట + WAL_చట్రం_HDRSIZE;
    a_చట్రం = (u8 *)sqlite3_malloc64(sz_చట్రం + WALINDEX_PGSZ);
    SEH_FREE_ON_ERROR(0, a_చట్రం);
    ఒకవేళ( !a_చట్రం ){
      rc = SQLITE_NOMEM_BKPT;
      వెళ్ళు recovery_error;
    }
    a_దత్తాంశం = &a_చట్రం[WAL_చట్రం_HDRSIZE];
    a_వ్యక్తిగత = (u32*)&a_దత్తాంశం[sz_పుట];

    /* Read all frames from the log file. */
    i_చివరి_చట్రం = (n_కొలత - WAL_HDRSIZE) / sz_చట్రం;
    కొరకు(iPg=0; iPg<=(u32)wal_చట్రం_పుట(i_చివరి_చట్రం); iPg++){
      u32 *aShare;
      u32 i_చట్రం;                 /* Index of last frame read */
      u32 i_చివరి = MIN(i_చివరి_చట్రం, HASHTABLE_NPAGE_ONE+iPg*HASHTABLE_NPAGE);
      u32 i_మొదటి = 1 + (iPg==0?0:HASHTABLE_NPAGE_ONE+(iPg-1)*HASHTABLE_NPAGE);
      u32 nHdr, nHdr32;
      rc = wal_సూచీ_పుట(pWal, iPg, (అస్థిర u32**)&aShare);
      ప్రతిజ్ఞా( aShare!=0 || rc!=SQLITE_OK );
      ఒకవేళ( aShare==0 ) ఆపు;
      SEH_సమితి_ON_ERROR(iPg, aShare);
      pWal->ap_Wi_దత్తాంశం[iPg] = a_వ్యక్తిగత;

      కొరకు(i_చట్రం=i_మొదటి; i_చట్రం<=i_చివరి; i_చట్రం++){
        i64 i_స్థానభ్రంశం = wal_చట్రం_స్థానభ్రంశం(i_చట్రం, sz_పుట);
        u32 pgno;                 /* Database page number for frame */
        u32 nTruncate;            /* dbsize field from frame header */

        /* Read and decode the next log frame. */
        rc = sqlite3OsRead(pWal->pWalFd, a_చట్రం, sz_చట్రం, i_స్థానభ్రంశం);
        ఒకవేళ( rc!=SQLITE_OK ) ఆపు;
        isValid = wal_Decode_చట్రం(pWal, &pgno, &nTruncate, a_దత్తాంశం, a_చట్రం);
        ఒకవేళ( !isValid ) ఆపు;
        rc = wal_సూచీ_Append(pWal, i_చట్రం, pgno);
        ఒకవేళ( NEVER(rc!=SQLITE_OK) ) ఆపు;

        /* If nTruncate is non-zero, this is a commit record. */
        ఒకవేళ( nTruncate ){
          pWal->hdr.mx_చట్రం = i_చట్రం;
          pWal->hdr.n_పుట = nTruncate;
          pWal->hdr.sz_పుట = (u16)((sz_పుట&0xff00) | (sz_పుట>>16));
          testcase( sz_పుట<=32768 );
          testcase( sz_పుట>=65536 );
          a_చట్రం_Cksum[0] = pWal->hdr.a_చట్రం_Cksum[0];
          a_చట్రం_Cksum[1] = pWal->hdr.a_చట్రం_Cksum[1];
        }
      }
      pWal->ap_Wi_దత్తాంశం[iPg] = aShare;
      SEH_సమితి_ON_ERROR(0,0);
      nHdr = (iPg==0 ? WALINDEX_HDR_కొలత : 0);
      nHdr32 = nHdr / పరిమాణం(u32);
#నిర్వచితం_కాకపోతే SQLITE_SAFER_WALINDEX_RECOVERY
      /* Memcpy() should work fine here, on all reasonable implementations.
      ** Technically, memcpy() might change the destination to some
      ** intermediate value before setting to the final value, and that might
      ** cause a concurrent reader to malfunction.  Memcpy() is allowed to
      ** do that, according to the spec, but no memcpy() implementation that
      ** we know of actually does that, which is why we say that memcpy()
      ** is safe for this.  Memcpy() is certainly a lot faster.
      */
      జ్ఞాపకం_నకలు(&aShare[nHdr32], &a_వ్యక్తిగత[nHdr32], WALINDEX_PGSZ-nHdr);
#లేకపోతే
      /* In the event that some platform is found for which memcpy()
      ** changes the destination to some intermediate value before
      ** setting the final value, this alternative copy routine is
      ** provided.
      */
      {
        పూర్ణాంకం i;
        కొరకు(i=nHdr32; i<WALINDEX_PGSZ/పరిమాణం(u32); i++){
          ఒకవేళ( aShare[i]!=a_వ్యక్తిగత[i] ){
            /* Atomic memory operations are not required here because if
            ** the value needs to be changed, that means it is not being
            ** accessed concurrently. */
            aShare[i] = a_వ్యక్తిగత[i];
          }
        }
      }
#అయితే_ముగింపు
      SEH_INJECT_FAULT;
      ఒకవేళ( i_చట్రం<=i_చివరి ) ఆపు;
    }

    SEH_FREE_ON_ERROR(a_చట్రం, 0);
    sqlite3_free(a_చట్రం);
  }

ముగిసిన:
  ఒకవేళ( rc==SQLITE_OK ){
    అస్థిర WalCkptInfo *pInfo;
    పూర్ణాంకం i;
    pWal->hdr.a_చట్రం_Cksum[0] = a_చట్రం_Cksum[0];
    pWal->hdr.a_చట్రం_Cksum[1] = a_చట్రం_Cksum[1];
    wal_సూచీ_Write_Hdr(pWal);

    /* Reset the checkpoint-header. This is safe because this thread is
    ** currently holding locks that exclude all other writers and
    ** checkpointers. Then set the values of read-mark slots 1 through N.
    */
    pInfo = walCkptInfo(pWal);
    pInfo->nBackfill = 0;
    pInfo->nBackfillAttempted = pWal->hdr.mx_చట్రం;
    pInfo->aReadMark[0] = 0;
    కొరకు(i=1; i<WAL_NREADER; i++){
      rc = wal_తాళం_Exclusive(pWal, WAL_READ_తాళం(i), 1);
      ఒకవేళ( rc==SQLITE_OK ){
        ఒకవేళ( i==1 && pWal->hdr.mx_చట్రం ){
          pInfo->aReadMark[i] = pWal->hdr.mx_చట్రం;
        }లేకపోతే{
          pInfo->aReadMark[i] = READMARK_NOT_వాడిన;
        }
        SEH_INJECT_FAULT;
        walUnlockExclusive(pWal, WAL_READ_తాళం(i), 1);
      }లేకపోతే ఒకవేళ( rc!=SQLITE_BUSY ){
        వెళ్ళు recovery_error;
      }
    }

    /* If more than one frame was recovered from the log file, report an
    ** event via sqlite3_log(). This is to help with identifying performance
    ** problems caused by applications routinely shutting down without
    ** checkpointing the log file.
    */
    ఒకవేళ( pWal->hdr.n_పుట ){
      sqlite3_log(SQLITE_NOTICE_RECOVER_WAL,
          "recovered %d frames from WAL file %s",
          pWal->hdr.mx_చట్రం, pWal->zWalName
      );
    }
  }

recovery_error:
  WALTRACE(("WAL%p: recovery %s\n", pWal, rc ? "failed" : "ok"));
  walUnlockExclusive(pWal, i_తాళం, WAL_READ_తాళం(0)-i_తాళం);
  తిరిగి rc;
}

/*
** Close an open wal-index.
*/
స్థిర శూన్య wal_సూచీ_Close(Wal *pWal, పూర్ణాంకం isDelete){
  ఒకవేళ( pWal->exclusive_బహుళకం==WAL_HEAPMEMORY_బహుళకం || pWal->bShmUnreliable ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<pWal->n_Wi_దత్తాంశం; i++){
      sqlite3_free((శూన్య *)pWal->ap_Wi_దత్తాంశం[i]);
      pWal->ap_Wi_దత్తాంశం[i] = 0;
    }
  }
  ఒకవేళ( pWal->exclusive_బహుళకం!=WAL_HEAPMEMORY_బహుళకం ){
    sqlite3OsShmUnmap(pWal->pDbFd, isDelete);
  }
}

/*
** Open a connection to the WAL file zWalName. The database file must
** already be opened on connection pDbFd. The buffer that zWalName points
** to must remain valid for the lifetime of the returned Wal* handle.
**
** A SHARED lock should be held on the database file when this function
** is called. The purpose of this SHARED lock is to prevent any other
** client from unlinking the WAL or wal-index file. If another process
** were to do this just after this client opened one of these files, the
** system would be badly broken.
**
** If the log file is successfully opened, SQLITE_OK is returned and
** *ppWal is set to point to a new WAL handle. If an error occurs,
** an SQLite error code is returned and *ppWal is left unmodified.
*/
పూర్ణాంకం sqlite3WalOpen(
  sqlite3_vfs *pVfs,              /* vfs module to open wal and wal-index */
  sqlite3_file *pDbFd,            /* The open database file */
  స్థిరరాశి అక్షరం *zWalName,           /* Name of the WAL file */
  పూర్ణాంకం bNoShm,                     /* True to run in heap-memory mode */
  i64 mx_Wal_కొలత,                  /* Truncate WAL to this size on reset */
  Wal **ppWal                     /* OUT: Allocated Wal handle */
){
  పూర్ణాంకం rc;                         /* Return Code */
  Wal *pRet;                      /* Object to allocate and return */
  పూర్ణాంకం flags;                      /* Flags passed to OsOpen() */

  ప్రతిజ్ఞా( zWalName && zWalName[0] );
  ప్రతిజ్ఞా( pDbFd );

  /* Verify the values of various constants.  Any changes to the values
  ** of these constants would result in an incompatible on-disk format
  ** for the -shm file.  Any change that causes one of these asserts to
  ** fail is a backward compatibility problem, even if the change otherwise
  ** works.
  **
  ** This table also serves as a helpful cross-reference when trying to
  ** interpret hex dumps of the -shm file.
  */
  ప్రతిజ్ఞా(    48 ==  పరిమాణం(Wal_సూచీ_Hdr)  );
  ప్రతిజ్ఞా(    40 ==  పరిమాణం(WalCkptInfo)  );
  ప్రతిజ్ఞా(   120 ==  WALINDEX_తాళం_స్థానభ్రంశం );
  ప్రతిజ్ఞా(   136 ==  WALINDEX_HDR_కొలత    );
  ప్రతిజ్ఞా(  4096 ==  HASHTABLE_NPAGE      );
  ప్రతిజ్ఞా(  4062 ==  HASHTABLE_NPAGE_ONE  );
  ప్రతిజ్ఞా(  8192 ==  HASHTABLE_NSLOT      );
  ప్రతిజ్ఞా(   383 ==  HASHTABLE_HASH_1     );
  ప్రతిజ్ఞా( 32768 ==  WALINDEX_PGSZ        );
  ప్రతిజ్ఞా(     8 ==  SQLITE_SHM_NLOCK     );
  ప్రతిజ్ఞా(     5 ==  WAL_NREADER          );
  ప్రతిజ్ఞా(    24 ==  WAL_చట్రం_HDRSIZE    );
  ప్రతిజ్ఞా(    32 ==  WAL_HDRSIZE          );
  ప్రతిజ్ఞా(   120 ==  WALINDEX_తాళం_స్థానభ్రంశం + WAL_WRITE_తాళం   );
  ప్రతిజ్ఞా(   121 ==  WALINDEX_తాళం_స్థానభ్రంశం + WAL_CKPT_తాళం    );
  ప్రతిజ్ఞా(   122 ==  WALINDEX_తాళం_స్థానభ్రంశం + WAL_RECOVER_తాళం );
  ప్రతిజ్ఞా(   123 ==  WALINDEX_తాళం_స్థానభ్రంశం + WAL_READ_తాళం(0) );
  ప్రతిజ్ఞా(   124 ==  WALINDEX_తాళం_స్థానభ్రంశం + WAL_READ_తాళం(1) );
  ప్రతిజ్ఞా(   125 ==  WALINDEX_తాళం_స్థానభ్రంశం + WAL_READ_తాళం(2) );
  ప్రతిజ్ఞా(   126 ==  WALINDEX_తాళం_స్థానభ్రంశం + WAL_READ_తాళం(3) );
  ప్రతిజ్ఞా(   127 ==  WALINDEX_తాళం_స్థానభ్రంశం + WAL_READ_తాళం(4) );

  /* In the amalgamation, the os_unix.c and os_win.c source files come before
  ** this source file.  Verify that the #defines of the locking byte offsets
  ** in os_unix.c and os_win.c agree with the WALINDEX_LOCK_OFFSET value.
  ** For that matter, if the lock offset ever changes from its initial design
  ** value of 120, we need to know that so there is an assert() to check it.
  */
#నిర్వచితమైతే WIN_SHM_పునాది
  ప్రతిజ్ఞా( WIN_SHM_పునాది==WALINDEX_తాళం_స్థానభ్రంశం );
#అయితే_ముగింపు
#నిర్వచితమైతే UNIX_SHM_పునాది
  ప్రతిజ్ఞా( UNIX_SHM_పునాది==WALINDEX_తాళం_స్థానభ్రంశం );
#అయితే_ముగింపు


  /* Allocate an instance of struct Wal to return. */
  *ppWal = 0;
  pRet = (Wal*)sqlite3MallocZero(పరిమాణం(Wal) + pVfs->szOsFile);
  ఒకవేళ( !pRet ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }

  pRet->pVfs = pVfs;
  pRet->pWalFd = (sqlite3_file *)&pRet[1];
  pRet->pDbFd = pDbFd;
  pRet->read_తాళం = -1;
  pRet->mx_Wal_కొలత = mx_Wal_కొలత;
  pRet->zWalName = zWalName;
  pRet->sync_శీర్షిక = 1;
  pRet->pad_గమ్యం_Sector_Boundary = 1;
  pRet->exclusive_బహుళకం = (bNoShm ? WAL_HEAPMEMORY_బహుళకం: WAL_సాధారణ_బహుళకం);

  /* Open file handle on the write-ahead log file. */
  flags = (SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_WAL);
  rc = sqlite3OsOpen(pVfs, zWalName, pRet->pWalFd, flags, &flags);
  ఒకవేళ( rc==SQLITE_OK && flags&SQLITE_OPEN_READONLY ){
    pRet->readOnly = WAL_RDONLY;
  }

  ఒకవేళ( rc!=SQLITE_OK ){
    wal_సూచీ_Close(pRet, 0);
    sqlite3OsClose(pRet->pWalFd);
    sqlite3_free(pRet);
  }లేకపోతే{
    పూర్ణాంకం iDC = sqlite3OsDeviceCharacteristics(pDbFd);
    ఒకవేళ( iDC & SQLITE_IOCAP_SEQUENTIAL ){ pRet->sync_శీర్షిక = 0; }
    ఒకవేళ( iDC & SQLITE_IOCAP_POWERSAFE_OVERWRITE ){
      pRet->pad_గమ్యం_Sector_Boundary = 0;
    }
    *ppWal = pRet;
    WALTRACE(("WAL%d: opened\n", pRet));
  }
  తిరిగి rc;
}

/*
** Change the size to which the WAL file is truncated on each reset.
*/
శూన్య sqlite3_Wal_హద్దు(Wal *pWal, i64 i_హద్దు){
  ఒకవేళ( pWal ) pWal->mx_Wal_కొలత = i_హద్దు;
}

/*
** Find the smallest page number out of all pages held in the WAL that
** has not been returned by any prior invocation of this method on the
** same WalIterator object.   Write into *piFrame the frame index where
** that page was last written into the WAL.  Write into *piPage the page
** number.
**
** Return 0 on success.  If there are no pages in the WAL with a page
** number larger than *piPage, then return 1.
*/
స్థిర పూర్ణాంకం wal_Iterator_తదుపరి(
  WalIterator *p,               /* Iterator */
  u32 *pi_పుట,                  /* OUT: The page number of the next page */
  u32 *pi_చట్రం                  /* OUT: Wal frame index of next page */
){
  u32 iRet = 0xFFFFFFFF;        /* 0xffffffff is never a valid page number */

  ఒకవేళ( p->nPagemap ){
    /* Algorithm WALITER-2 */
    అయితే( p->i_పూర్వ<p->nPagemap ){
      p->i_పూర్వ++;
      ఒకవేళ( p->aSegment[0].aPgno[p->i_పూర్వ] ){
        iRet = p->i_పూర్వ;
        *pi_చట్రం = p->aSegment[0].aPgno[p->i_పూర్వ];
        ఆపు;
      }
    }
  }లేకపోతే{
    /* Algorithm WALITER-1 */
    u32 iMin;                     /* Result pgno must be greater than iMin */
    పూర్ణాంకం i;                        /* For looping through segments */

    iMin = p->i_పూర్వ;
    ప్రతిజ్ఞా( iMin<0xffffffff );
    కొరకు(i=p->nSegment-1; i>=0; i--){
      నిర్మాణం WalSegment *pSegment = &p->aSegment[i];
      అయితే( pSegment->i_తదుపరి<pSegment->n_నమోదు ){
        u32 iPg = pSegment->aPgno[pSegment->a_సూచీ[pSegment->i_తదుపరి]];
        ఒకవేళ( iPg>iMin ){
          ఒకవేళ( iPg<iRet ){
            iRet = iPg;
            *pi_చట్రం = pSegment->iZero + pSegment->a_సూచీ[pSegment->i_తదుపరి];
          }
          ఆపు;
        }
        pSegment->i_తదుపరి++;
      }
    }
  }

  *pi_పుట = p->i_పూర్వ = iRet;
  తిరిగి (iRet==0xFFFFFFFF);
}

/*
** This function merges two sorted lists into a single sorted list.
**
** aLeft[] and aRight[] are arrays of indices.  The sort key is
** aContent[aLeft[]] and aContent[aRight[]].  Upon entry, the following
** is guaranteed for all J<K:
**
**        aContent[aLeft[J]] < aContent[aLeft[K]]
**        aContent[aRight[J]] < aContent[aRight[K]]
**
** This routine overwrites aRight[] with a new (probably longer) sequence
** of indices such that the aRight[] contains every index that appears in
** either aLeft[] or the old aRight[] and such that the second condition
** above is still met.
**
** The aContent[aLeft[X]] values will be unique for all X.  And the
** aContent[aRight[X]] values will be unique too.  But there might be
** one or more combinations of X and Y such that
**
**      aLeft[X]!=aRight[Y]  &&  aContent[aLeft[X]] == aContent[aRight[Y]]
**
** When that happens, omit the aLeft[X] and use the aRight[Y] index.
*/
స్థిర శూన్య wal_విలీనం(
  స్థిరరాశి u32 *aContent,            /* Pages in wal - keys for the sort */
  ht_స్థానం *a_ఎడమ,                 /* IN: Left hand input list */
  పూర్ణాంకం n_ఎడమ,                      /* IN: Elements in array *paLeft */
  ht_స్థానం **pa_కుడి,              /* IN/OUT: Right hand input list */
  పూర్ణాంకం *pn_కుడి,                   /* IN/OUT: Elements in *paRight */
  ht_స్థానం *aTmp                   /* Temporary buffer */
){
  పూర్ణాంకం i_ఎడమ = 0;                  /* Current index in aLeft */
  పూర్ణాంకం i_కుడి = 0;                 /* Current index in aRight */
  పూర్ణాంకం i_బహిర్గతం = 0;                   /* Current index in output buffer */
  పూర్ణాంకం n_కుడి = *pn_కుడి;
  ht_స్థానం *a_కుడి = *pa_కుడి;

  ప్రతిజ్ఞా( n_ఎడమ>0 && n_కుడి>0 );
  అయితే( i_కుడి<n_కుడి || i_ఎడమ<n_ఎడమ ){
    ht_స్థానం logpage;
    Pgno dbpage;

    ఒకవేళ( (i_ఎడమ<n_ఎడమ)
     && (i_కుడి>=n_కుడి || aContent[a_ఎడమ[i_ఎడమ]]<aContent[a_కుడి[i_కుడి]])
    ){
      logpage = a_ఎడమ[i_ఎడమ++];
    }లేకపోతే{
      logpage = a_కుడి[i_కుడి++];
    }
    dbpage = aContent[logpage];

    aTmp[i_బహిర్గతం++] = logpage;
    ఒకవేళ( i_ఎడమ<n_ఎడమ && aContent[a_ఎడమ[i_ఎడమ]]==dbpage ) i_ఎడమ++;

    ప్రతిజ్ఞా( i_ఎడమ>=n_ఎడమ || aContent[a_ఎడమ[i_ఎడమ]]>dbpage );
    ప్రతిజ్ఞా( i_కుడి>=n_కుడి || aContent[a_కుడి[i_కుడి]]>dbpage );
  }

  *pa_కుడి = a_ఎడమ;
  *pn_కుడి = i_బహిర్గతం;
  జ్ఞాపకం_నకలు(a_ఎడమ, aTmp, పరిమాణం(aTmp[0])*i_బహిర్గతం);
}

/*
** Sort the elements in list aList using aContent[] as the sort key.
** Remove elements with duplicate keys, preferring to keep the
** larger aList[] values.
**
** The aList[] entries are indices into aContent[].  The values in
** aList[] are to be sorted so that for all J<K:
**
**      aContent[aList[J]] < aContent[aList[K]]
**
** For any X and Y such that
**
**      aContent[aList[X]] == aContent[aList[Y]]
**
** Keep the larger of the two values aList[X] and aList[Y] and discard
** the smaller.
*/
స్థిర శూన్య walMergesort(
  స్థిరరాశి u32 *aContent,            /* Pages in wal */
  ht_స్థానం *a_బఫరు,               /* Buffer of at least *pnList items to use */
  ht_స్థానం *aList,                 /* IN/OUT: List to sort */
  పూర్ణాంకం *pnList                     /* IN/OUT: Number of elements in aList[] */
){
  నిర్మాణం Sublist {
    పూర్ణాంకం nList;                    /* Number of elements in aList */
    ht_స్థానం *aList;               /* Pointer to sub-list content */
  };

  స్థిరరాశి పూర్ణాంకం nList = *pnList;      /* Size of input list */
  పూర్ణాంకం n_విలీనం = 0;                 /* Number of elements in list aMerge */
  ht_స్థానం *a_విలీనం = 0;            /* List to be merged */
  పూర్ణాంకం iList;                      /* Index into input list */
  u32 iSub = 0;                   /* Index into aSub array */
  నిర్మాణం Sublist aSub[13];        /* Array of sub-lists */

  memset(aSub, 0, పరిమాణం(aSub));
  ప్రతిజ్ఞా( nList<=HASHTABLE_NPAGE && nList>0 );
  ప్రతిజ్ఞా( HASHTABLE_NPAGE==(1<<(Array_కొలత(aSub)-1)) );

  కొరకు(iList=0; iList<nList; iList++){
    n_విలీనం = 1;
    a_విలీనం = &aList[iList];
    కొరకు(iSub=0; iList & (1<<iSub); iSub++){
      నిర్మాణం Sublist *p;
      ప్రతిజ్ఞా( iSub<Array_కొలత(aSub) );
      p = &aSub[iSub];
      ప్రతిజ్ఞా( p->aList && p->nList<=(1<<iSub) );
      ప్రతిజ్ఞా( p->aList==&aList[iList&~((2<<iSub)-1)] );
      wal_విలీనం(aContent, p->aList, p->nList, &a_విలీనం, &n_విలీనం, a_బఫరు);
    }
    aSub[iSub].aList = a_విలీనం;
    aSub[iSub].nList = n_విలీనం;
  }

  కొరకు(iSub++; iSub<Array_కొలత(aSub); iSub++){
    ఒకవేళ( nList & (1<<iSub) ){
      నిర్మాణం Sublist *p;
      ప్రతిజ్ఞా( iSub<Array_కొలత(aSub) );
      p = &aSub[iSub];
      ప్రతిజ్ఞా( p->nList<=(1<<iSub) );
      ప్రతిజ్ఞా( p->aList==&aList[nList&~((2<<iSub)-1)] );
      wal_విలీనం(aContent, p->aList, p->nList, &a_విలీనం, &n_విలీనం, a_బఫరు);
    }
  }
  ప్రతిజ్ఞా( a_విలీనం==aList );
  *pnList = n_విలీనం;

#నిర్వచితమైతే SQLITE_DEBUG
  {
    పూర్ణాంకం i;
    కొరకు(i=1; i<*pnList; i++){
      ప్రతిజ్ఞా( aContent[aList[i]] > aContent[aList[i-1]] );
    }
  }
#అయితే_ముగింపు
}

/*
** Free an iterator allocated by walIteratorInit().
*/
స్థిర శూన్య walIteratorFree(WalIterator *p){
  sqlite3_free(p);
}

/*
** Construct a WalInterator object that can be used to loop over all
** pages in the WAL following frame nBackfill in ascending order. Frames
** nBackfill or earlier may be included - excluding them is an optimization
** only. The caller must hold the checkpoint lock.
**
** On success, make *pp point to the newly allocated WalInterator object
** return SQLITE_OK. Otherwise, return an error code. If this routine
** returns an error, the value of *pp is undefined.
**
** The calling routine should invoke walIteratorFree() to destroy the
** WalIterator object when it has finished with it.
*/
స్థిర పూర్ణాంకం walIteratorInit(Wal *pWal, u32 nBackfill, WalIterator **pp){
  పూర్ణాంకం i_మొదటి_పుట;                 /* First 32KB *-shm page to visit */
  పూర్ణాంకం i_చివరి_పుట;                  /* Last 32KB *-shm page to visit */
  u32 iZero;                      /* Frame before first frame to visit */
  WalIterator *p;                 /* Return value */
  పూర్ణాంకం nSegment;                   /* Number of segments to merge */
  u32 i_చివరి;                      /* Last frame in log */
  sqlite3_int64 nByte;            /* Number of bytes to allocate */
  sqlite3_int64 nExtra;           /* Number of bytes to allocate */
  పూర్ణాంకం i;                          /* Iterator variable */
  ht_స్థానం *aTmp = 0;              /* Temp space used by merge-sort */
  పూర్ణాంకం rc = SQLITE_OK;             /* Return Code */
  పూర్ణాంకం eAlg;                       /* Which algorithm to use.  1 or 2 */

  /* This routine only runs while holding the checkpoint lock. And
  ** it only runs if there is actually content in the log (mxFrame>0).  */
  ప్రతిజ్ఞా( pWal->ckpt_తాళం && pWal->hdr.mx_చట్రం>0 );

  i_చివరి = pWal->hdr.mx_చట్రం;
  i_మొదటి_పుట = wal_చట్రం_పుట(nBackfill+1);
  i_చివరి_పుట = wal_చట్రం_పుట(i_చివరి);
  iZero = i_మొదటి_పుట ? (HASHTABLE_NPAGE_ONE+(i_మొదటి_పుట-1)*HASHTABLE_NPAGE) : 0;

  ఒకవేళ( (i_చివరి-iZero)*పరిమాణం(ht_స్థానం)>pWal->hdr.n_పుట*పరిమాణం(u32) ){
    /* Algorithm WALITER-2 */
    nSegment = 1;
    eAlg = 2;
    nByte = SZ_WALITERATOR(nSegment) + pWal->hdr.n_పుట * పరిమాణం(u32);
    nExtra = 0;
  }లేకపోతే{
    /* Algorithm WALITER-1 */
    nSegment = i_చివరి_పుట - i_మొదటి_పుట + 1;
    eAlg = 1;
    nByte = SZ_WALITERATOR(nSegment) + (i_చివరి-iZero)*పరిమాణం(ht_స్థానం);
    nExtra = పరిమాణం(ht_స్థానం) * (i_చివరి>HASHTABLE_NPAGE?HASHTABLE_NPAGE:i_చివరి);
  }

  /* Allocate space for the WalIterator object. */
  p = (WalIterator *)sqlite3_malloc64(nByte + nExtra);
  ఒకవేళ( !p ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  memset(p, 0, nByte);
  p->nSegment = nSegment;
  ఒకవేళ( eAlg==2 ){
    p->nPagemap = pWal->hdr.n_పుట;
    p->aSegment[0].aPgno = ((u32*)&p->aSegment[1]) - 1;
    ప్రతిజ్ఞా( (u8*)&(p->aSegment[0].aPgno[0]) == (u8*)&(p->aSegment[0].iZero) );
    /* ^-- verifies tag-20260903-1 */
  }లేకపోతే{
    aTmp = (ht_స్థానం*)&(((u8*)p)[nByte]);
  }
  SEH_FREE_ON_ERROR(0, p);
  కొరకు(i=i_మొదటి_పుట; rc==SQLITE_OK && i<=i_చివరి_పుట; i++){
    WalHashLoc sLoc;

    rc = walHashGet(pWal, i, &sLoc);
    ఒకవేళ( rc==SQLITE_OK ){
      పూర్ణాంకం j;                      /* Counter variable */
      పూర్ణాంకం n_నమోదు;                 /* Number of entries in this segment */
      ht_స్థానం *a_సూచీ;            /* Sorted index for this segment */

      ఒకవేళ( i==i_చివరి_పుట ){
        n_నమోదు = (పూర్ణాంకం)(i_చివరి - sLoc.iZero);
      }లేకపోతే{
        n_నమోదు = (పూర్ణాంకం)((u32*)sLoc.aHash - (u32*)sLoc.aPgno);
      }

      ఒకవేళ( eAlg==2 ){
        u32 *aPagemap = p->aSegment[0].aPgno;
        కొరకు(j=0; j<n_నమోదు; j++){
          u32 pgno = sLoc.aPgno[ j ];
          ఒకవేళ( pgno<=p->nPagemap ){
            aPagemap[pgno] = sLoc.iZero + j + 1;
          }
        }
      }లేకపోతే{
        ప్రతిజ్ఞా( i!=i_మొదటి_పుట || iZero==sLoc.iZero );
        a_సూచీ = &((ht_స్థానం *)&p->aSegment[p->nSegment])[sLoc.iZero - iZero];
        sLoc.iZero++;

        కొరకు(j=0; j<n_నమోదు; j++){
          a_సూచీ[j] = (ht_స్థానం)j;
        }
        walMergesort((u32 *)sLoc.aPgno, aTmp, a_సూచీ, &n_నమోదు);
        p->aSegment[i-i_మొదటి_పుట].iZero = sLoc.iZero;
        p->aSegment[i-i_మొదటి_పుట].n_నమోదు = n_నమోదు;
        p->aSegment[i-i_మొదటి_పుట].a_సూచీ = a_సూచీ;
        p->aSegment[i-i_మొదటి_పుట].aPgno = (u32 *)sLoc.aPgno;
      }
    }
  }
  ఒకవేళ( rc!=SQLITE_OK ){
    SEH_FREE_ON_ERROR(p, 0);
    walIteratorFree(p);
    p = 0;
  }
  *pp = p;
  తిరిగి rc;
}

#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT


/*
** Attempt to enable blocking locks that block for nMs ms. Return 1 if 
** blocking locks are successfully enabled, or 0 otherwise.
*/
స్థిర పూర్ణాంకం walEnableBlockingMs(Wal *pWal, పూర్ణాంకం nMs){
  పూర్ణాంకం rc = sqlite3OsFileControl(
      pWal->pDbFd, SQLITE_FCNTL_తాళం_TIMEOUT, (శూన్య*)&nMs
  );
  తిరిగి (rc==SQLITE_OK);
}

/*
** Attempt to enable blocking locks. Blocking locks are enabled only if (a)
** they are supported by the VFS, and (b) the database handle is configured
** with a busy-timeout. Return 1 if blocking locks are successfully enabled,
** or 0 otherwise.
*/
స్థిర పూర్ణాంకం walEnableBlocking(Wal *pWal){
  పూర్ణాంకం res = 0;
  ఒకవేళ( pWal->db ){
    పూర్ణాంకం tmout = pWal->db->setlkTimeout;
    ఒకవేళ( tmout ){
      res = walEnableBlockingMs(pWal, tmout);
    }
  }
  తిరిగి res;
}

/*
** Disable blocking locks.
*/
స్థిర శూన్య walDisableBlocking(Wal *pWal){
  పూర్ణాంకం tmout = 0;
  sqlite3OsFileControl(pWal->pDbFd, SQLITE_FCNTL_తాళం_TIMEOUT, (శూన్య*)&tmout);
}

/*
** If parameter bLock is true, attempt to enable blocking locks, take
** the WRITER lock, and then disable blocking locks. If blocking locks
** cannot be enabled, no attempt to obtain the WRITER lock is made. Return
** an SQLite error code if an error occurs, or SQLITE_OK otherwise. It is not
** an error if blocking locks can not be enabled.
**
** If the bLock parameter is false and the WRITER lock is held, release it.
*/
పూర్ణాంకం sqlite3_Wal_Write_తాళం(Wal *pWal, పూర్ణాంకం b_తాళం){
  పూర్ణాంకం rc = SQLITE_OK;
  ప్రతిజ్ఞా( pWal->read_తాళం<0 || b_తాళం==0 );
  ఒకవేళ( b_తాళం ){
    ప్రతిజ్ఞా( pWal->db );
    ఒకవేళ( walEnableBlocking(pWal) ){
      rc = wal_తాళం_Exclusive(pWal, WAL_WRITE_తాళం, 1);
      ఒకవేళ( rc==SQLITE_OK ){
        pWal->write_తాళం = 1;
      }
      walDisableBlocking(pWal);
    }
  }లేకపోతే ఒకవేళ( pWal->write_తాళం ){
    walUnlockExclusive(pWal, WAL_WRITE_తాళం, 1);
    pWal->write_తాళం = 0;
  }
  తిరిగి rc;
}

/*
** Set the database handle used to determine if blocking locks are required.
*/
శూన్య sqlite3WalDb(Wal *pWal, sqlite3 *db){
  pWal->db = db;
}

#లేకపోతే
# define walEnableBlocking(x) 0
# define walDisableBlocking(x)
# define walEnableBlockingMs(pWal, ms) 0
# define sqlite3WalDb(pWal, db)
#అయితే_ముగింపు   /* ifdef SQLITE_ENABLE_SETLK_TIMEOUT */


/*
** Attempt to obtain the exclusive WAL lock defined by parameters lockIdx and
** n. If the attempt fails and parameter xBusy is not NULL, then it is a
** busy-handler function. Invoke it and retry the lock until either the
** lock is successfully obtained or the busy-handler returns 0.
*/
స్థిర పూర్ణాంకం wal_Busy_తాళం(
  Wal *pWal,                      /* WAL connection */
  పూర్ణాంకం (*xBusy)(శూన్య*),            /* Function to call when busy */
  శూన్య *pBusyArg,                 /* Context argument for xBusyHandler */
  పూర్ణాంకం తాళం_Idx,                    /* Offset of first byte to lock */
  పూర్ణాంకం n                           /* Number of bytes to lock */
){
  పూర్ణాంకం rc;
  చేయి {
    rc = wal_తాళం_Exclusive(pWal, తాళం_Idx, n);
  }అయితే( xBusy && rc==SQLITE_BUSY && xBusy(pBusyArg) );
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  ఒకవేళ( rc==SQLITE_BUSY_TIMEOUT ){
    walDisableBlocking(pWal);
    rc = SQLITE_BUSY;
  }
#అయితే_ముగింపు
  తిరిగి rc;
}

/*
** The cache of the wal-index header must be valid to call this function.
** Return the page-size in bytes used by the database.
*/
స్థిర పూర్ణాంకం walPagesize(Wal *pWal){
  తిరిగి (pWal->hdr.sz_పుట&0xfe00) + ((pWal->hdr.sz_పుట&0x0001)<<16);
}

/*
** The following is guaranteed when this function is called:
**
**   a) the WRITER lock is held,
**   b) the entire log file has been checkpointed, and
**   c) any existing readers are reading exclusively from the database
**      file - there are no readers that may attempt to read a frame from
**      the log file.
**
** This function updates the shared-memory structures so that the next
** client to write to the database (which may be this one) does so by
** writing frames into the start of the log file.
**
** The value of parameter salt1 is used as the aSalt[1] value in the
** new wal-index header. It should be passed a pseudo-random value (i.e.
** one obtained from sqlite3_randomness()).
*/
స్థిర శూన్య walRestartHdr(Wal *pWal, u32 salt1){
  అస్థిర WalCkptInfo *pInfo = walCkptInfo(pWal);
  పూర్ణాంకం i;                          /* Loop counter */
  u32 *aSalt = pWal->hdr.aSalt;   /* Big-endian salt values */
  pWal->nCkpt++;
  pWal->hdr.mx_చట్రం = 0;
  sqlite3Put4byte((u8*)&aSalt[0], 1 + sqlite3Get4byte((u8*)&aSalt[0]));
  జ్ఞాపకం_నకలు(&pWal->hdr.aSalt[1], &salt1, 4);
  wal_సూచీ_Write_Hdr(pWal);
  AtomicStore(&pInfo->nBackfill, 0);
  pInfo->nBackfillAttempted = 0;
  pInfo->aReadMark[1] = 0;
  కొరకు(i=2; i<WAL_NREADER; i++) pInfo->aReadMark[i] = READMARK_NOT_వాడిన;
  ప్రతిజ్ఞా( pInfo->aReadMark[0]==0 );
}

/*
** Copy as much content as we can from the WAL back into the database file
** in response to an sqlite3_wal_checkpoint() request or the equivalent.
**
** The amount of information copies from WAL to database might be limited
** by active readers.  This routine will never overwrite a database page
** that a concurrent reader might be using.
**
** All I/O barrier operations (a.k.a fsyncs) occur in this routine when
** SQLite is in WAL-mode in synchronous=NORMAL.  That means that if
** checkpoints are always run by a background thread or background
** process, foreground threads will never block on a lengthy fsync call.
**
** Fsync is called on the WAL before writing content out of the WAL and
** into the database.  This ensures that if the new content is persistent
** in the WAL and can be recovered following a power-loss or hard reset.
**
** Fsync is also called on the database file if (and only if) the entire
** WAL content is copied into the database file.  This second fsync makes
** it safe to delete the WAL since the new content will persist in the
** database file.
**
** This routine uses and updates the nBackfill field of the wal-index header.
** This is the only routine that will increase the value of nBackfill.
** (A WAL reset or recovery will revert nBackfill to zero, but not increase
** its value.)
**
** The caller must be holding sufficient locks to ensure that no other
** checkpoint is running (in any other thread or process) at the same
** time.
*/
స్థిర పూర్ణాంకం walCheckpoint(
  Wal *pWal,                      /* Wal connection */
  sqlite3 *db,                    /* Check for interrupts on this handle */
  పూర్ణాంకం e_బహుళకం,                      /* One of PASSIVE, FULL or RESTART */
  పూర్ణాంకం (*xBusy)(శూన్య*),            /* Function to call when busy */
  శూన్య *pBusyArg,                 /* Context argument for xBusyHandler */
  పూర్ణాంకం sync_flags,                 /* Flags for OsSync() (or 0) */
  u8 *zBuf                        /* Temporary buffer to use */
){
  పూర్ణాంకం rc = SQLITE_OK;             /* Return code */
  పూర్ణాంకం sz_పుట;                     /* Database page-size */
  WalIterator *pIter = 0;         /* Wal iterator context */
  u32 iDbpage = 0;                /* Next database page to write */
  u32 i_చట్రం = 0;                 /* Wal frame containing data for iDbpage */
  u32 mx_సురక్షిత_చట్రం;                /* Max frame that can be backfilled */
  u32 mx_పుట;                     /* Max database page to write */
  పూర్ణాంకం i;                          /* Loop counter */
  అస్థిర WalCkptInfo *pInfo;    /* The checkpoint status information */

  sz_పుట = walPagesize(pWal);
  testcase( sz_పుట<=32768 );
  testcase( sz_పుట>=65536 );
  pInfo = walCkptInfo(pWal);
  ఒకవేళ( pInfo->nBackfill<pWal->hdr.mx_చట్రం ){

    /* EVIDENCE-OF: R-62920-47450 The busy-handler callback is never invoked
    ** in the SQLITE_CHECKPOINT_PASSIVE mode. */
    ప్రతిజ్ఞా( e_బహుళకం!=SQLITE_CHECKPOINT_PASSIVE || xBusy==0 );

    /* Compute in mxSafeFrame the index of the last frame of the WAL that is
    ** safe to write into the database.  Frames beyond mxSafeFrame might
    ** overwrite database pages that are in use by active readers and thus
    ** cannot be backfilled from the WAL.
    */
    mx_సురక్షిత_చట్రం = pWal->hdr.mx_చట్రం;
    mx_పుట = pWal->hdr.n_పుట;
    కొరకు(i=1; i<WAL_NREADER; i++){
      u32 y = Atomic_ఎక్కించు(pInfo->aReadMark+i); SEH_INJECT_FAULT;
      ఒకవేళ( mx_సురక్షిత_చట్రం>y ){
        ప్రతిజ్ఞా( y<=pWal->hdr.mx_చట్రం );
        rc = wal_Busy_తాళం(pWal, xBusy, pBusyArg, WAL_READ_తాళం(i), 1);
        ఒకవేళ( rc==SQLITE_OK ){
          u32 iMark = (i==1 ? mx_సురక్షిత_చట్రం : READMARK_NOT_వాడిన);
          AtomicStore(pInfo->aReadMark+i, iMark); SEH_INJECT_FAULT;
          walUnlockExclusive(pWal, WAL_READ_తాళం(i), 1);
        }లేకపోతే ఒకవేళ( rc==SQLITE_BUSY ){
          mx_సురక్షిత_చట్రం = y;
          xBusy = 0;
        }లేకపోతే{
          వెళ్ళు walcheckpoint_బహిర్గతం;
        }
      }
    }

    /* Allocate the iterator */
    ఒకవేళ( pInfo->nBackfill<mx_సురక్షిత_చట్రం ){
      rc = walIteratorInit(pWal, pInfo->nBackfill, &pIter);
      ప్రతిజ్ఞా( rc==SQLITE_OK || pIter==0 );
    }

    ఒకవేళ( pIter
     && (rc = wal_Busy_తాళం(pWal,xBusy,pBusyArg,WAL_READ_తాళం(0),1))==SQLITE_OK
    ){
      u32 nBackfill = pInfo->nBackfill;
      Wal_సూచీ_Hdr *p_సజీవ = (Wal_సూచీ_Hdr*)wal_సూచీ_Hdr(pWal);

      /* Now that read-lock slot 0 is locked, check that the wal has not been
      ** wrapped since the header was read for this checkpoint. If it was, then
      ** there was no work to do anyway.  In this case the
      ** (pInfo->nBackfill<pWal->hdr.mxFrame) test above only passed because
      ** pInfo->nBackfill had already been set to 0 by the writer that wrapped
      ** the wal file. It would also be dangerous to proceed, as there may be
      ** fewer than pWal->hdr.mxFrame valid frames in the wal file.  */
      పూర్ణాంకం bChg = memcmp(p_సజీవ->aSalt, pWal->hdr.aSalt, పరిమాణం(pWal->hdr.aSalt));
      ఒకవేళ( 0==bChg ){
        pInfo->nBackfillAttempted = mx_సురక్షిత_చట్రం; SEH_INJECT_FAULT;
  
        /* Sync the WAL to disk */
        rc = sqlite3OsSync(pWal->pWalFd, CKPT_SYNC_FLAGS(sync_flags));
  
        /* If the database may grow as a result of this checkpoint, hint
        ** about the eventual size of the db file to the VFS layer.
        */
        ఒకవేళ( rc==SQLITE_OK ){
          i64 nReq = ((i64)mx_పుట * sz_పుట);
          i64 n_కొలత;                    /* Current size of database file */
          sqlite3OsFileControl(pWal->pDbFd, SQLITE_FCNTL_CKPT_ప్రారంభం, 0);
          rc = sqlite3_Os_File_కొలత(pWal->pDbFd, &n_కొలత);
          ఒకవేళ( rc==SQLITE_OK && n_కొలత<nReq ){
            ఒకవేళ( (n_కొలత+65536+(i64)pWal->hdr.mx_చట్రం*sz_పుట)<nReq ){
              /* If the size of the final database is larger than the current
              ** database plus the amount of data in the wal file, plus the
              ** maximum size of the pending-byte page (65536 bytes), then
              ** must be corruption somewhere.  */
              rc = SQLITE_CORRUPT_BKPT;
            }లేకపోతే{
              sqlite3OsFileControlHint(
                  pWal->pDbFd, SQLITE_FCNTL_కొలత_HINT, &nReq);
            }
          }
  
        }
  
        /* Iterate through the contents of the WAL, copying data to the 
        ** db file */
        అయితే( rc==SQLITE_OK && 0==wal_Iterator_తదుపరి(pIter, &iDbpage, &i_చట్రం) ){
          i64 i_స్థానభ్రంశం;
          ప్రతిజ్ఞా( wal_చట్రం_Pgno(pWal, i_చట్రం)==iDbpage );
          SEH_INJECT_FAULT;
          ఒకవేళ( Atomic_ఎక్కించు(&db->u1.isInterrupted) ){
            rc = db->mallocFailed ? SQLITE_NOMEM_BKPT : SQLITE_అంతరాయం;
            ఆపు;
          }
          ఒకవేళ( i_చట్రం<=nBackfill || i_చట్రం>mx_సురక్షిత_చట్రం || iDbpage>mx_పుట ){
            కొనసాగు;
          }
          i_స్థానభ్రంశం = wal_చట్రం_స్థానభ్రంశం(i_చట్రం, sz_పుట) + WAL_చట్రం_HDRSIZE;
          /* testcase( IS_BIG_INT(iOffset) ); // requires a 4GiB WAL file */
          rc = sqlite3OsRead(pWal->pWalFd, zBuf, sz_పుట, i_స్థానభ్రంశం);
          ఒకవేళ( rc!=SQLITE_OK ) ఆపు;
          i_స్థానభ్రంశం = (iDbpage-1)*(i64)sz_పుట;
          testcase( IS_BIG_INT(i_స్థానభ్రంశం) );
          rc = sqlite3OsWrite(pWal->pDbFd, zBuf, sz_పుట, i_స్థానభ్రంశం);
          ఒకవేళ( rc!=SQLITE_OK ) ఆపు;
        }
        sqlite3OsFileControl(pWal->pDbFd, SQLITE_FCNTL_CKPT_పూర్తి, 0);
  
        /* If work was actually accomplished... */
        ఒకవేళ( rc==SQLITE_OK ){
          ఒకవేళ( mx_సురక్షిత_చట్రం==wal_సూచీ_Hdr(pWal)->mx_చట్రం ){
            i64 szDb = pWal->hdr.n_పుట*(i64)sz_పుట;
            testcase( IS_BIG_INT(szDb) );
            rc = sqlite3OsTruncate(pWal->pDbFd, szDb);
            ఒకవేళ( rc==SQLITE_OK ){
              rc = sqlite3OsSync(pWal->pDbFd, CKPT_SYNC_FLAGS(sync_flags));
            }
          }
          ఒకవేళ( rc==SQLITE_OK ){
            AtomicStore(&pInfo->nBackfill, mx_సురక్షిత_చట్రం); SEH_INJECT_FAULT;
          }
        }
      }

      /* Release the reader lock held while backfilling */
      walUnlockExclusive(pWal, WAL_READ_తాళం(0), 1);
    }

    ఒకవేళ( rc==SQLITE_BUSY ){
      /* Reset the return code so as not to report a checkpoint failure
      ** just because there are active readers.  */
      rc = SQLITE_OK;
    }
  }

  /* If this is an SQLITE_CHECKPOINT_RESTART or TRUNCATE operation, and the
  ** entire wal file has been copied into the database file, then block
  ** until all readers have finished using the wal file. This ensures that
  ** the next process to write to the database restarts the wal file.
  */
  ఒకవేళ( rc==SQLITE_OK && e_బహుళకం!=SQLITE_CHECKPOINT_PASSIVE ){
    ప్రతిజ్ఞా( pWal->write_తాళం );
    SEH_INJECT_FAULT;
    ఒకవేళ( pInfo->nBackfill<pWal->hdr.mx_చట్రం ){
      rc = SQLITE_BUSY;
    }లేకపోతే ఒకవేళ( e_బహుళకం>=SQLITE_CHECKPOINT_RESTART ){
      u32 salt1;
      sqlite3_randomness(4, &salt1);
      ప్రతిజ్ఞా( pInfo->nBackfill==pWal->hdr.mx_చట్రం );
      rc = wal_Busy_తాళం(pWal, xBusy, pBusyArg, WAL_READ_తాళం(1), WAL_NREADER-1);
      ఒకవేళ( rc==SQLITE_OK ){
        ఒకవేళ( e_బహుళకం==SQLITE_CHECKPOINT_TRUNCATE ){
          /* IMPLEMENTATION-OF: R-44699-57140 This mode works the same way as
          ** SQLITE_CHECKPOINT_RESTART with the addition that it also
          ** truncates the log file to zero bytes just prior to a
          ** successful return.
          **
          ** In theory, it might be safe to do this without updating the
          ** wal-index header in shared memory, as all subsequent reader or
          ** writer clients should see that the entire log file has been
          ** checkpointed and behave accordingly. This seems unsafe though,
          ** as it would leave the system in a state where the contents of
          ** the wal-index header do not match the contents of the
          ** file-system. To avoid this, update the wal-index header to
          ** indicate that the log file contains zero valid frames.  */
          walRestartHdr(pWal, salt1);
          rc = sqlite3OsTruncate(pWal->pWalFd, 0);
        }
        walUnlockExclusive(pWal, WAL_READ_తాళం(1), WAL_NREADER-1);
      }
    }
  }

 walcheckpoint_బహిర్గతం:
  SEH_FREE_ON_ERROR(pIter, 0);
  walIteratorFree(pIter);
  తిరిగి rc;
}

/*
** If the WAL file is currently larger than nMax bytes in size, truncate
** it to exactly nMax bytes. If an error occurs while doing so, ignore it.
*/
స్థిర శూన్య wal_హద్దు_కొలత(Wal *pWal, i64 nMax){
  i64 sz;
  పూర్ణాంకం rx;
  sqlite3BeginBenignMalloc();
  rx = sqlite3_Os_File_కొలత(pWal->pWalFd, &sz);
  ఒకవేళ( rx==SQLITE_OK && (sz > nMax ) ){
    rx = sqlite3OsTruncate(pWal->pWalFd, nMax);
  }
  sqlite3_చివర_Benign_Malloc();
  ఒకవేళ( rx ){
    sqlite3_log(rx, "cannot limit WAL size: %s", pWal->zWalName);
  }
}

#నిర్వచితమైతే SQLITE_USE_SEH
/*
** This is the "standard" exception handler used in a few places to handle 
** an exception thrown by reading from the *-shm mapping after it has become
** invalid in SQLITE_USE_SEH builds. It is used as follows:
**
**   SEH_TRY { ... }
**   SEH_EXCEPT( rc = walHandleException(pWal); )
**
** This function does three things:
**
**   1) Determines the locks that should be held, based on the contents of
**      the Wal.readLock, Wal.writeLock and Wal.ckptLock variables. All other
**      held locks are assumed to be transient locks that would have been
**      released had the exception not been thrown and are dropped.
**
**   2) Frees the pointer at Wal.pFree, if any, using sqlite3_free().
**
**   3) Set pWal->apWiData[pWal->iWiPg] to pWal->pWiValue if not NULL
**
**   4) Returns SQLITE_IOERR.
*/
స్థిర పూర్ణాంకం walHandleException(Wal *pWal){
  ఒకవేళ( pWal->exclusive_బహుళకం==0 ){
    స్థిర స్థిరరాశి పూర్ణాంకం S = 1;
    స్థిర స్థిరరాశి పూర్ణాంకం E = (1<<SQLITE_SHM_NLOCK);
    పూర్ణాంకం ii;
    u32 mUnlock;
    ఒకవేళ( pWal->write_తాళం==2 ) pWal->write_తాళం = 0;
    mUnlock = pWal->తాళం_ముసుగు & ~(
        (pWal->read_తాళం<0 ? 0 : (S << WAL_READ_తాళం(pWal->read_తాళం)))
        | (pWal->write_తాళం ? (E << WAL_WRITE_తాళం) : 0)
        | (pWal->ckpt_తాళం ? (E << WAL_CKPT_తాళం) : 0)
        );
    కొరకు(ii=0; ii<SQLITE_SHM_NLOCK; ii++){
      ఒకవేళ( (S<<ii) & mUnlock ) walUnlockShared(pWal, ii);
      ఒకవేళ( (E<<ii) & mUnlock ) walUnlockExclusive(pWal, ii, 1);
    }
  }
  sqlite3_free(pWal->pFree);
  pWal->pFree = 0;
  ఒకవేళ( pWal->p_Wi_విలువ ){
    pWal->ap_Wi_దత్తాంశం[pWal->iWiPg] = pWal->p_Wi_విలువ;
    pWal->p_Wi_విలువ = 0;
  }
  తిరిగి SQLITE_IOERR_లోపల_పుట;
}

/*
** Assert that the Wal.lockMask mask, which indicates the locks held
** by the connection, is consistent with the Wal.readLock, Wal.writeLock
** and Wal.ckptLock variables. To be used as:
**
**   assert( walAssertLockmask(pWal) );
*/
స్థిర పూర్ణాంకం walAssertLockmask(Wal *pWal){
  ఒకవేళ( pWal->exclusive_బహుళకం==0 ){
    స్థిర స్థిరరాశి పూర్ణాంకం S = 1;
    స్థిర స్థిరరాశి పూర్ణాంకం E = (1<<SQLITE_SHM_NLOCK);
    u32 mExpect = (
        (pWal->read_తాళం<0 ? 0 : (S << WAL_READ_తాళం(pWal->read_తాళం)))
      | (pWal->write_తాళం ? (E << WAL_WRITE_తాళం) : 0)
      | (pWal->ckpt_తాళం ? (E << WAL_CKPT_తాళం) : 0)
#నిర్వచితమైతే SQLITE_ENABLE_SNAPSHOT
      | (pWal->pSnapshot ? (pWal->తాళం_ముసుగు & (1 << WAL_CKPT_తాళం)) : 0)
#అయితే_ముగింపు
    );
    ప్రతిజ్ఞా( mExpect==pWal->తాళం_ముసుగు );
  }
  తిరిగి 1;
}

/*
** Return and zero the "system error" field set when an 
** EXCEPTION_IN_PAGE_ERROR exception is caught.
*/
పూర్ణాంకం sqlite3WalSystemErrno(Wal *pWal){
  పూర్ణాంకం iRet = 0;
  ఒకవేళ( pWal ){
    iRet = pWal->iSysErrno;
    pWal->iSysErrno = 0;
  }
  తిరిగి iRet;
}

#లేకపోతే
# define walAssertLockmask(x) 1
#అయితే_ముగింపు /* ifdef SQLITE_USE_SEH */

/*
** Close a connection to a log file.
*/
పూర్ణాంకం sqlite3WalClose(
  Wal *pWal,                      /* Wal to close */
  sqlite3 *db,                    /* For interrupt flag */
  పూర్ణాంకం sync_flags,                 /* Flags to pass to OsSync() (or 0) */
  పూర్ణాంకం nBuf,
  u8 *zBuf                        /* Buffer of at least nBuf bytes */
){
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( pWal ){
    పూర్ణాంకం isDelete = 0;             /* True to unlink wal and wal-index files */

    ప్రతిజ్ఞా( walAssertLockmask(pWal) );

    /* If an EXCLUSIVE lock can be obtained on the database file (using the
    ** ordinary, rollback-mode locking methods, this guarantees that the
    ** connection associated with this log file is the only connection to
    ** the database. In this case checkpoint the database and unlink both
    ** the wal and wal-index files.
    **
    ** The EXCLUSIVE lock is not released before returning.
    */
    ఒకవేళ( zBuf!=0
     && SQLITE_OK==(rc = sqlite3_Os_తాళం(pWal->pDbFd, SQLITE_తాళం_EXCLUSIVE))
    ){
      ఒకవేళ( pWal->exclusive_బహుళకం==WAL_సాధారణ_బహుళకం ){
        pWal->exclusive_బహుళకం = WAL_EXCLUSIVE_బహుళకం;
      }
      rc = sqlite3WalCheckpoint(pWal, db,
          SQLITE_CHECKPOINT_PASSIVE, 0, 0, sync_flags, nBuf, zBuf, 0, 0
      );
      ఒకవేళ( rc==SQLITE_OK ){
        పూర్ణాంకం bPersist = -1;
        sqlite3OsFileControlHint(
            pWal->pDbFd, SQLITE_FCNTL_PERSIST_WAL, &bPersist
        );
        ఒకవేళ( bPersist!=1 ){
          /* Try to delete the WAL file if the checkpoint completed and
          ** fsynced (rc==SQLITE_OK) and if we are not in persistent-wal
          ** mode (!bPersist) */
          isDelete = 1;
        }లేకపోతే ఒకవేళ( pWal->mx_Wal_కొలత>=0 ){
          /* Try to truncate the WAL file to zero bytes if the checkpoint
          ** completed and fsynced (rc==SQLITE_OK) and we are in persistent
          ** WAL mode (bPersist) and if the PRAGMA journal_size_limit is a
          ** non-negative value (pWal->mxWalSize>=0).  Note that we truncate
          ** to zero bytes as truncating to the journal_size_limit might
          ** leave a corrupt WAL file on disk. */
          wal_హద్దు_కొలత(pWal, 0);
        }
      }
    }

    wal_సూచీ_Close(pWal, isDelete);
    sqlite3OsClose(pWal->pWalFd);
    ఒకవేళ( isDelete ){
      sqlite3BeginBenignMalloc();
      sqlite3OsDelete(pWal->pVfs, pWal->zWalName, 0);
      sqlite3_చివర_Benign_Malloc();
    }
    WALTRACE(("WAL%p: closed\n", pWal));
    sqlite3_free((శూన్య *)pWal->ap_Wi_దత్తాంశం);
    sqlite3_free(pWal);
  }
  తిరిగి rc;
}

/*
** Try to read the wal-index header.  Return 0 on success and 1 if
** there is a problem.
**
** The wal-index is in shared memory.  Another thread or process might
** be writing the header at the same time this procedure is trying to
** read it, which might result in inconsistency.  A dirty read is detected
** by verifying that both copies of the header are the same and also by
** a checksum on the header.
**
** If and only if the read is consistent and the header is different from
** pWal->hdr, then pWal->hdr is updated to the content of the new header
** and *pChanged is set to 1.
**
** If the checksum cannot be verified return non-zero. If the header
** is read successfully and the checksum verified, return zero.
*/
స్థిర SQLITE_NO_TSAN పూర్ణాంకం wal_సూచీ_Try_Hdr(Wal *pWal, పూర్ణాంకం *p_మారింది){
  u32 aCksum[2];                  /* Checksum on the header content */
  Wal_సూచీ_Hdr h1, h2;             /* Two copies of the header content */
  Wal_సూచీ_Hdr అస్థిర *aHdr;     /* Header in shared memory */

  /* The first page of the wal-index must be mapped at this point. */
  ప్రతిజ్ఞా( pWal->n_Wi_దత్తాంశం>0 && pWal->ap_Wi_దత్తాంశం[0] );

  /* Read the header. This might happen concurrently with a write to the
  ** same area of shared memory on a different CPU in a SMP,
  ** meaning it is possible that an inconsistent snapshot is read
  ** from the file. If this happens, return non-zero.
  **
  ** tag-20200519-1:
  ** There are two copies of the header at the beginning of the wal-index.
  ** When reading, read [0] first then [1].  Writes are in the reverse order.
  ** Memory barriers are used to prevent the compiler or the hardware from
  ** reordering the reads and writes.  TSAN and similar tools can sometimes
  ** give false-positive warnings about these accesses because the tools do not
  ** account for the double-read and the memory barrier. The use of mutexes
  ** here would be problematic as the memory being accessed is potentially
  ** shared among multiple processes and not all mutex implementations work
  ** reliably in that environment.
  */
  aHdr = wal_సూచీ_Hdr(pWal);
  జ్ఞాపకం_నకలు(&h1, (శూన్య *)&aHdr[0], పరిమాణం(h1)); /* Possible TSAN false-positive */
  walShmBarrier(pWal);
  జ్ఞాపకం_నకలు(&h2, (శూన్య *)&aHdr[1], పరిమాణం(h2));

  ఒకవేళ( memcmp(&h1, &h2, పరిమాణం(h1))!=0 ){
    తిరిగి 1;   /* Dirty read */
  }
  ఒకవేళ( h1.isInit==0 ){
    తిరిగి 1;   /* Malformed header - probably all zeros */
  }
  wal_Checksum_బైట్లు(1, (u8*)&h1, పరిమాణం(h1)-పరిమాణం(h1.aCksum), 0, aCksum);
  ఒకవేళ( aCksum[0]!=h1.aCksum[0] || aCksum[1]!=h1.aCksum[1] ){
    తిరిగి 1;   /* Checksum does not match */
  }

  ఒకవేళ( memcmp(&pWal->hdr, &h1, పరిమాణం(Wal_సూచీ_Hdr)) ){
    *p_మారింది = 1;
    జ్ఞాపకం_నకలు(&pWal->hdr, &h1, పరిమాణం(Wal_సూచీ_Hdr));
    pWal->sz_పుట = (pWal->hdr.sz_పుట&0xfe00) + ((pWal->hdr.sz_పుట&0x0001)<<16);
    testcase( pWal->sz_పుట<=32768 );
    testcase( pWal->sz_పుట>=65536 );
  }

  /* The header was successfully read. Return zero. */
  తిరిగి 0;
}

/*
** This is the value that walTryBeginRead returns when it needs to
** be retried.
*/
#నిర్వచించు WAL_RETRY  (-1)

/*
** Read the wal-index header from the wal-index and into pWal->hdr.
** If the wal-header appears to be corrupt, try to reconstruct the
** wal-index from the WAL before returning.
**
** Set *pChanged to 1 if the wal-index header value in pWal->hdr is
** changed by this operation.  If pWal->hdr is unchanged, set *pChanged
** to 0.
**
** If the wal-index header is successfully read, return SQLITE_OK.
** Otherwise an SQLite error code.
*/
స్థిర పూర్ణాంకం wal_సూచీ_Read_Hdr(Wal *pWal, పూర్ణాంకం *p_మారింది){
  పూర్ణాంకం rc;                         /* Return code */
  పూర్ణాంకం badHdr;                     /* True if a header read failed */
  అస్థిర u32 *page0;            /* Chunk of wal-index containing header */

  /* Ensure that page 0 of the wal-index (the page that contains the
  ** wal-index header) is mapped. Return early if an error occurs here.
  */
  ప్రతిజ్ఞా( p_మారింది );
  rc = wal_సూచీ_పుట(pWal, 0, &page0);
  ఒకవేళ( rc!=SQLITE_OK ){
    ప్రతిజ్ఞా( rc!=SQLITE_READONLY ); /* READONLY changed to OK in walIndexPage */
    ఒకవేళ( rc==SQLITE_READONLY_CANTINIT ){
      /* The SQLITE_READONLY_CANTINIT return means that the shared-memory
      ** was openable but is not writable, and this thread is unable to
      ** confirm that another write-capable connection has the shared-memory
      ** open, and hence the content of the shared-memory is unreliable,
      ** since the shared-memory might be inconsistent with the WAL file
      ** and there is no writer on hand to fix it. */
      ప్రతిజ్ఞా( page0==0 );
      ప్రతిజ్ఞా( pWal->write_తాళం==0 );
      ప్రతిజ్ఞా( pWal->readOnly & WAL_SHM_RDONLY );
      pWal->bShmUnreliable = 1;
      pWal->exclusive_బహుళకం = WAL_HEAPMEMORY_బహుళకం;
      *p_మారింది = 1;
    }లేకపోతే{
      తిరిగి rc; /* Any other non-OK return is just an error */
    }
  }లేకపోతే{
    /* page0 can be NULL if the SHM is zero bytes in size and pWal->writeLock
    ** is zero, which prevents the SHM from growing */
    testcase( page0!=0 );
  }
  ప్రతిజ్ఞా( page0!=0 || pWal->write_తాళం==0 );

  /* If the first page of the wal-index has been mapped, try to read the
  ** wal-index header immediately, without holding any lock. This usually
  ** works, but may fail if the wal-index header is corrupt or currently
  ** being modified by another thread or process.
  */
  badHdr = (page0 ? wal_సూచీ_Try_Hdr(pWal, p_మారింది) : 1);

  /* If the first attempt failed, it might have been due to a race
  ** with a writer.  So get a WRITE lock and try again.
  */
  ఒకవేళ( badHdr ){
    ఒకవేళ( pWal->bShmUnreliable==0 && (pWal->readOnly & WAL_SHM_RDONLY) ){
      ఒకవేళ( SQLITE_OK==(rc = wal_తాళం_Shared(pWal, WAL_WRITE_తాళం)) ){
        walUnlockShared(pWal, WAL_WRITE_తాళం);
        rc = SQLITE_READONLY_RECOVERY;
      }
    }లేకపోతే{
      పూర్ణాంకం b_Write_తాళం = pWal->write_తాళం;
      ఒకవేళ( b_Write_తాళం 
       || SQLITE_OK==(rc = wal_తాళం_Exclusive(pWal, WAL_WRITE_తాళం, 1)) 
      ){
        /* If the write-lock was just obtained, set writeLock to 2 instead of
        ** the usual 1. This causes walIndexPage() to behave as if the 
        ** write-lock were held (so that it allocates new pages as required),
        ** and walHandleException() to unlock the write-lock if a SEH exception
        ** is thrown.  */
        ఒకవేళ( !b_Write_తాళం ) pWal->write_తాళం = 2;
        ఒకవేళ( SQLITE_OK==(rc = wal_సూచీ_పుట(pWal, 0, &page0)) ){
          badHdr = wal_సూచీ_Try_Hdr(pWal, p_మారింది);
          ఒకవేళ( badHdr ){
            /* If the wal-index header is still malformed even while holding
            ** a WRITE lock, it can only mean that the header is corrupted and
            ** needs to be reconstructed.  So run recovery to do exactly that.
            ** Disable blocking locks first.  */
            walDisableBlocking(pWal);
            rc = wal_సూచీ_Recover(pWal);
            *p_మారింది = 1;
          }
        }
        ఒకవేళ( b_Write_తాళం==0 ){
          pWal->write_తాళం = 0;
          walUnlockExclusive(pWal, WAL_WRITE_తాళం, 1);
        }
      }
    }
  }

  /* If the header is read successfully, check the version number to make
  ** sure the wal-index was not constructed with some future format that
  ** this version of SQLite cannot understand.
  */
  ఒకవేళ( badHdr==0 && pWal->hdr.i_సంస్కరణ!=WALINDEX_MAX_సంస్కరణ ){
    rc = SQLITE_CANTOPEN_BKPT;
  }
  ఒకవేళ( pWal->bShmUnreliable ){
    ఒకవేళ( rc!=SQLITE_OK ){
      wal_సూచీ_Close(pWal, 0);
      pWal->bShmUnreliable = 0;
      ప్రతిజ్ఞా( pWal->n_Wi_దత్తాంశం>0 && pWal->ap_Wi_దత్తాంశం[0]==0 );
      /* walIndexRecover() might have returned SHORT_READ if a concurrent
      ** writer truncated the WAL out from under it.  If that happens, it
      ** indicates that a writer has fixed the SHM file for us, so retry */
      ఒకవేళ( rc==SQLITE_IOERR_SHORT_READ ) rc = WAL_RETRY;
    }
    pWal->exclusive_బహుళకం = WAL_సాధారణ_బహుళకం;
  }

  తిరిగి rc;
}

/*
** Open a transaction in a connection where the shared-memory is read-only
** and where we cannot verify that there is a separate write-capable connection
** on hand to keep the shared-memory up-to-date with the WAL file.
**
** This can happen, for example, when the shared-memory is implemented by
** memory-mapping a *-shm file, where a prior writer has shut down and
** left the *-shm file on disk, and now the present connection is trying
** to use that database but lacks write permission on the *-shm file.
** Other scenarios are also possible, depending on the VFS implementation.
**
** Precondition:
**
**    The *-wal file has been read and an appropriate wal-index has been
**    constructed in pWal->apWiData[] using heap memory instead of shared
**    memory.
**
** If this function returns SQLITE_OK, then the read transaction has
** been successfully opened. In this case output variable (*pChanged)
** is set to true before returning if the caller should discard the
** contents of the page cache before proceeding. Or, if it returns
** WAL_RETRY, then the heap memory wal-index has been discarded and
** the caller should retry opening the read transaction from the
** beginning (including attempting to map the *-shm file).
**
** If an error occurs, an SQLite error code is returned.
*/
స్థిర పూర్ణాంకం walBeginShmUnreliable(Wal *pWal, పూర్ణాంకం *p_మారింది){
  i64 szWal;                      /* Size of wal file on disk in bytes */
  i64 i_స్థానభ్రంశం;                    /* Current offset when reading wal file */
  u8 aBuf[WAL_HDRSIZE];           /* Buffer to load WAL header into */
  u8 *a_చట్రం = 0;                 /* Malloc'd buffer to load entire frame */
  పూర్ణాంకం sz_చట్రం;                    /* Number of bytes in buffer aFrame[] */
  u8 *a_దత్తాంశం;                      /* Pointer to data part of aFrame buffer */
  అస్థిర శూన్య *pDummy;          /* Dummy argument for xShmMap */
  పూర్ణాంకం rc;                         /* Return code */
  u32 aSaveCksum[2];              /* Saved copy of pWal->hdr.aFrameCksum */

  ప్రతిజ్ఞా( pWal->bShmUnreliable );
  ప్రతిజ్ఞా( pWal->readOnly & WAL_SHM_RDONLY );
  ప్రతిజ్ఞా( pWal->n_Wi_దత్తాంశం>0 && pWal->ap_Wi_దత్తాంశం[0] );

  /* Take WAL_READ_LOCK(0). This has the effect of preventing any
  ** writers from running a checkpoint, but does not stop them
  ** from running recovery.  */
  rc = wal_తాళం_Shared(pWal, WAL_READ_తాళం(0));
  ఒకవేళ( rc!=SQLITE_OK ){
    ఒకవేళ( rc==SQLITE_BUSY ) rc = WAL_RETRY;
    వెళ్ళు begin_unreliable_shm_బహిర్గతం;
  }
  pWal->read_తాళం = 0;

  /* Check to see if a separate writer has attached to the shared-memory area,
  ** thus making the shared-memory "reliable" again.  Do this by invoking
  ** the xShmMap() routine of the VFS and looking to see if the return
  ** is SQLITE_READONLY instead of SQLITE_READONLY_CANTINIT.
  **
  ** If the shared-memory is now "reliable" return WAL_RETRY, which will
  ** cause the heap-memory WAL-index to be discarded and the actual
  ** shared memory to be used in its place.
  **
  ** This step is important because, even though this connection is holding
  ** the WAL_READ_LOCK(0) which prevents a checkpoint, a writer might
  ** have already checkpointed the WAL file and, while the current
  ** is active, wrap the WAL and start overwriting frames that this
  ** process wants to use.
  **
  ** Once sqlite3OsShmMap() has been called for an sqlite3_file and has
  ** returned any SQLITE_READONLY value, it must return only SQLITE_READONLY
  ** or SQLITE_READONLY_CANTINIT or some error for all subsequent invocations,
  ** even if some external agent does a "chmod" to make the shared-memory
  ** writable by us, until sqlite3OsShmUnmap() has been called.
  ** This is a requirement on the VFS implementation.
   */
  rc = sqlite3OsShmMap(pWal->pDbFd, 0, WALINDEX_PGSZ, 0, &pDummy);
  ప్రతిజ్ఞా( rc!=SQLITE_OK ); /* SQLITE_OK not possible for read-only connection */
  ఒకవేళ( rc!=SQLITE_READONLY_CANTINIT ){
    rc = (rc==SQLITE_READONLY ? WAL_RETRY : rc);
    వెళ్ళు begin_unreliable_shm_బహిర్గతం;
  }

  /* We reach this point only if the real shared-memory is still unreliable.
  ** Assume the in-memory WAL-index substitute is correct and load it
  ** into pWal->hdr.
  */
  జ్ఞాపకం_నకలు(&pWal->hdr, (శూన్య*)wal_సూచీ_Hdr(pWal), పరిమాణం(Wal_సూచీ_Hdr));

  /* Make sure some writer hasn't come in and changed the WAL file out
  ** from under us, then disconnected, while we were not looking.
  */
  rc = sqlite3_Os_File_కొలత(pWal->pWalFd, &szWal);
  ఒకవేళ( rc!=SQLITE_OK ){
    వెళ్ళు begin_unreliable_shm_బహిర్గతం;
  }
  ఒకవేళ( szWal<WAL_HDRSIZE ){
    /* If the wal file is too small to contain a wal-header and the
    ** wal-index header has mxFrame==0, then it must be safe to proceed
    ** reading the database file only. However, the page cache cannot
    ** be trusted, as a read/write connection may have connected, written
    ** the db, run a checkpoint, truncated the wal file and disconnected
    ** since this client's last read transaction.  */
    *p_మారింది = 1;
    rc = (pWal->hdr.mx_చట్రం==0 ? SQLITE_OK : WAL_RETRY);
    వెళ్ళు begin_unreliable_shm_బహిర్గతం;
  }

  /* Check the salt keys at the start of the wal file still match. */
  rc = sqlite3OsRead(pWal->pWalFd, aBuf, WAL_HDRSIZE, 0);
  ఒకవేళ( rc!=SQLITE_OK ){
    వెళ్ళు begin_unreliable_shm_బహిర్గతం;
  }
  ఒకవేళ( memcmp(&pWal->hdr.aSalt, &aBuf[16], 8) ){
    /* Some writer has wrapped the WAL file while we were not looking.
    ** Return WAL_RETRY which will cause the in-memory WAL-index to be
    ** rebuilt. */
    rc = WAL_RETRY;
    వెళ్ళు begin_unreliable_shm_బహిర్గతం;
  }

  /* Allocate a buffer to read frames into */
  ప్రతిజ్ఞా( (pWal->sz_పుట & (pWal->sz_పుట-1))==0 );
  ప్రతిజ్ఞా( (pWal->sz_పుట>=512 && pWal->sz_పుట<=65536) || pWal->sz_పుట==0 );
  sz_చట్రం = pWal->sz_పుట + WAL_చట్రం_HDRSIZE;
  a_చట్రం = (u8 *)sqlite3_malloc64(sz_చట్రం);
  ఒకవేళ( a_చట్రం==0 ){
    rc = SQLITE_NOMEM_BKPT;
    వెళ్ళు begin_unreliable_shm_బహిర్గతం;
  }
  a_దత్తాంశం = &a_చట్రం[WAL_చట్రం_HDRSIZE];

  /* Check to see if a complete transaction has been appended to the
  ** wal file since the heap-memory wal-index was created. If so, the
  ** heap-memory wal-index is discarded and WAL_RETRY returned to
  ** the caller.  */
  aSaveCksum[0] = pWal->hdr.a_చట్రం_Cksum[0];
  aSaveCksum[1] = pWal->hdr.a_చట్రం_Cksum[1];
  కొరకు(i_స్థానభ్రంశం=wal_చట్రం_స్థానభ్రంశం(pWal->hdr.mx_చట్రం+1, pWal->sz_పుట);
      i_స్థానభ్రంశం+sz_చట్రం<=szWal;
      i_స్థానభ్రంశం+=sz_చట్రం
  ){
    u32 pgno;                   /* Database page number for frame */
    u32 nTruncate;              /* dbsize field from frame header */

    /* Read and decode the next log frame. */
    rc = sqlite3OsRead(pWal->pWalFd, a_చట్రం, sz_చట్రం, i_స్థానభ్రంశం);
    ఒకవేళ( rc!=SQLITE_OK ) ఆపు;
    ఒకవేళ( !wal_Decode_చట్రం(pWal, &pgno, &nTruncate, a_దత్తాంశం, a_చట్రం) ) ఆపు;

    /* If nTruncate is non-zero, then a complete transaction has been
    ** appended to this wal file. Set rc to WAL_RETRY and break out of
    ** the loop.  */
    ఒకవేళ( nTruncate ){
      rc = WAL_RETRY;
      ఆపు;
    }
  }
  pWal->hdr.a_చట్రం_Cksum[0] = aSaveCksum[0];
  pWal->hdr.a_చట్రం_Cksum[1] = aSaveCksum[1];

 begin_unreliable_shm_బహిర్గతం:
  sqlite3_free(a_చట్రం);
  ఒకవేళ( rc!=SQLITE_OK ){
    పూర్ణాంకం i;
    కొరకు(i=0; i<pWal->n_Wi_దత్తాంశం; i++){
      sqlite3_free((శూన్య*)pWal->ap_Wi_దత్తాంశం[i]);
      pWal->ap_Wi_దత్తాంశం[i] = 0;
    }
    pWal->bShmUnreliable = 0;
    sqlite3_Wal_చివర_Read_Transaction(pWal);
    *p_మారింది = 1;
  }
  తిరిగి rc;
}

/*
** The final argument passed to walTryBeginRead() is of type (int*). The
** caller should invoke walTryBeginRead as follows:
**
**   int cnt = 0;
**   do {
**     rc = walTryBeginRead(..., &cnt);
**   }while( rc==WAL_RETRY );
**
** The final value of "cnt" is of no use to the caller. It is used by
** the implementation of walTryBeginRead() as follows:
**
**   + Each time walTryBeginRead() is called, it is incremented. Once
**     it reaches WAL_RETRY_PROTOCOL_LIMIT - indicating that walTryBeginRead()
**     has many times been invoked and failed with WAL_RETRY - walTryBeginRead()
**     returns SQLITE_PROTOCOL.
**
**   + If SQLITE_ENABLE_SETLK_TIMEOUT is defined and walTryBeginRead() failed
**     because a blocking lock timed out (SQLITE_BUSY_TIMEOUT from the OS
**     layer), the WAL_RETRY_BLOCKED_MASK bit is set in "cnt". In this case
**     the next invocation of walTryBeginRead() may omit an expected call to 
**     sqlite3OsSleep(). There has already been a delay when the previous call
**     waited on a lock.
*/
#నిర్వచించు WAL_RETRY_PROTOCOL_హద్దు 100
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
# define WAL_RETRY_BLOCKED_ముసుగు    0x10000000
#లేకపోతే
# define WAL_RETRY_BLOCKED_ముసుగు    0
#అయితే_ముగింపు

/*
** Attempt to start a read transaction.  This might fail due to a race or
** other transient condition.  When that happens, it returns WAL_RETRY to
** indicate to the caller that it is safe to retry immediately.
**
** On success return SQLITE_OK.  On a permanent failure (such an
** I/O error or an SQLITE_BUSY because another process is running
** recovery) return a positive error code.
**
** The useWal parameter is true to force the use of the WAL and disable
** the case where the WAL is bypassed because it has been completely
** checkpointed.  If useWal==0 then this routine calls walIndexReadHdr()
** to make a copy of the wal-index header into pWal->hdr.  If the
** wal-index header has changed, *pChanged is set to 1 (as an indication
** to the caller that the local page cache is obsolete and needs to be
** flushed.)  When useWal==1, the wal-index header is assumed to already
** be loaded and the pChanged parameter is unused.
**
** The caller must set the cnt parameter to the number of prior calls to
** this routine during the current read attempt that returned WAL_RETRY.
** This routine will start taking more aggressive measures to clear the
** race conditions after multiple WAL_RETRY returns, and after an excessive
** number of errors will ultimately return SQLITE_PROTOCOL.  The
** SQLITE_PROTOCOL return indicates that some other process has gone rogue
** and is not honoring the locking protocol.  There is a vanishingly small
** chance that SQLITE_PROTOCOL could be returned because of a run of really
** bad luck when there is lots of contention for the wal-index, but that
** possibility is so small that it can be safely neglected, we believe.
**
** On success, this routine obtains a read lock on
** WAL_READ_LOCK(pWal->readLock).  The pWal->readLock integer is
** in the range 0 <= pWal->readLock < WAL_NREADER.  If pWal->readLock==(-1)
** that means the Wal does not hold any read lock.  The reader must not
** access any database page that is modified by a WAL frame up to and
** including frame number aReadMark[pWal->readLock].  The reader will
** use WAL frames up to and including pWal->hdr.mxFrame if pWal->readLock>0
** Or if pWal->readLock==0, then the reader will ignore the WAL
** completely and get all content directly from the database file.
** If the useWal parameter is 1 then the WAL will never be ignored and
** this routine will always set pWal->readLock>0 on success.
** When the read transaction is completed, the caller must release the
** lock on WAL_READ_LOCK(pWal->readLock) and set pWal->readLock to -1.
**
** This routine uses the nBackfill and aReadMark[] fields of the header
** to select a particular WAL_READ_LOCK() that strives to let the
** checkpoint process do as much work as possible.  This routine might
** update values of the aReadMark[] array in the header, but if it does
** so it takes care to hold an exclusive lock on the corresponding
** WAL_READ_LOCK() while changing values.
*/
స్థిర పూర్ణాంకం walTryBeginRead(Wal *pWal, పూర్ణాంకం *p_మారింది, పూర్ణాంకం useWal, పూర్ణాంకం *pCnt){
  అస్థిర WalCkptInfo *pInfo;    /* Checkpoint information in wal-index */
  పూర్ణాంకం rc = SQLITE_OK;             /* Return code  */
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  పూర్ణాంకం n_ఖండం_Tmout = 0;
#అయితే_ముగింపు

  ప్రతిజ్ఞా( pWal->read_తాళం<0 );     /* Not currently locked */

  /* useWal may only be set for read/write connections */
  ప్రతిజ్ఞా( (pWal->readOnly & WAL_SHM_RDONLY)==0 || useWal==0 );

  /* Take steps to avoid spinning forever if there is a protocol error.
  **
  ** Circumstances that cause a RETRY should only last for the briefest
  ** instances of time.  No I/O or other system calls are done while the
  ** locks are held, so the locks should not be held for very long. But
  ** if we are unlucky, another process that is holding a lock might get
  ** paged out or take a page-fault that is time-consuming to resolve,
  ** during the few nanoseconds that it is holding the lock.  In that case,
  ** it might take longer than normal for the lock to free.
  **
  ** After 5 RETRYs, we begin calling sqlite3OsSleep().  The first few
  ** calls to sqlite3OsSleep() have a delay of 1 microsecond.  Really this
  ** is more of a scheduler yield than an actual delay.  But on the 10th
  ** an subsequent retries, the delays start becoming longer and longer,
  ** so that on the 100th (and last) RETRY we delay for 323 milliseconds.
  ** The total delay time before giving up is less than 10 seconds.
  */
  (*pCnt)++;
  ఒకవేళ( *pCnt>5 ){
    పూర్ణాంకం nDelay = 1;                      /* Pause time in microseconds */
    పూర్ణాంకం cnt = (*pCnt & ~WAL_RETRY_BLOCKED_ముసుగు);
    ఒకవేళ( cnt>WAL_RETRY_PROTOCOL_హద్దు ){
      VVA_ONLY( pWal->తాళం_Error = 1; )
      తిరిగి SQLITE_PROTOCOL;
    }
    ఒకవేళ( *pCnt>=10 ) nDelay = (cnt-9)*(cnt-9)*39;
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
    /* In SQLITE_ENABLE_SETLK_TIMEOUT builds, configure the file-descriptor
    ** to block for locks for approximately nDelay us. This affects three
    ** locks: (a) the shared lock taken on the DMS slot in os_unix.c (if
    ** using os_unix.c), (b) the WRITER lock taken in walIndexReadHdr() if the
    ** first attempted read fails, and (c) the shared lock taken on the 
    ** read-mark.  
    **
    ** If the previous call failed due to an SQLITE_BUSY_TIMEOUT error,
    ** then sleep for the minimum of 1us. The previous call already provided 
    ** an extra delay while it was blocking on the lock.
    */
    n_ఖండం_Tmout = (nDelay+998) / 1000;
    ఒకవేళ( !useWal && walEnableBlockingMs(pWal, n_ఖండం_Tmout) ){
      ఒకవేళ( *pCnt & WAL_RETRY_BLOCKED_ముసుగు ) nDelay = 1;
    }
#అయితే_ముగింపు
    sqlite3OsSleep(pWal->pVfs, nDelay);
    *pCnt &= ~WAL_RETRY_BLOCKED_ముసుగు;
  }

  ఒకవేళ( !useWal ){
    ప్రతిజ్ఞా( rc==SQLITE_OK );
    ఒకవేళ( pWal->bShmUnreliable==0 ){
      rc = wal_సూచీ_Read_Hdr(pWal, p_మారింది);
    }
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
    ఒకవేళ( rc==SQLITE_BUSY_TIMEOUT ){
      rc = SQLITE_BUSY;
      *pCnt |= WAL_RETRY_BLOCKED_ముసుగు;
    }
#అయితే_ముగింపు
    ఒకవేళ( rc==SQLITE_BUSY ){
      /* If there is not a recovery running in another thread or process
      ** then convert BUSY errors to WAL_RETRY.  If recovery is known to
      ** be running, convert BUSY to BUSY_RECOVERY.  There is a race here
      ** which might cause WAL_RETRY to be returned even if BUSY_RECOVERY
      ** would be technically correct.  But the race is benign since with
      ** WAL_RETRY this routine will be called again and will probably be
      ** right on the second iteration.
      */
      (శూన్య)walEnableBlocking(pWal);
      ఒకవేళ( pWal->ap_Wi_దత్తాంశం[0]==0 ){
        /* This branch is taken when the xShmMap() method returns SQLITE_BUSY.
        ** We assume this is a transient condition, so return WAL_RETRY. The
        ** xShmMap() implementation used by the default unix and win32 VFS
        ** modules may return SQLITE_BUSY due to a race condition in the
        ** code that determines whether or not the shared-memory region
        ** must be zeroed before the requested page is returned.
        */
        rc = WAL_RETRY;
      }లేకపోతే ఒకవేళ( SQLITE_OK==(rc = wal_తాళం_Shared(pWal, WAL_RECOVER_తాళం)) ){
        walUnlockShared(pWal, WAL_RECOVER_తాళం);
        rc = WAL_RETRY;
      }లేకపోతే ఒకవేళ( rc==SQLITE_BUSY ){
        rc = SQLITE_BUSY_RECOVERY;
      }
    }
    walDisableBlocking(pWal);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
    లేకపోతే ఒకవేళ( pWal->bShmUnreliable ){
      తిరిగి walBeginShmUnreliable(pWal, p_మారింది);
    }
  }

  ప్రతిజ్ఞా( pWal->n_Wi_దత్తాంశం>0 );
  ప్రతిజ్ఞా( pWal->ap_Wi_దత్తాంశం[0]!=0 );
  pInfo = walCkptInfo(pWal);
  SEH_INJECT_FAULT;
  {
    u32 mxReadMark;               /* Largest aReadMark[] value */
    పూర్ణాంకం mxI;                      /* Index of largest aReadMark[] value */
    పూర్ణాంకం i;                        /* Loop counter */
    u32 mx_చట్రం;                  /* Wal frame to lock to */
    ఒకవేళ( !useWal && Atomic_ఎక్కించు(&pInfo->nBackfill)==pWal->hdr.mx_చట్రం
#నిర్వచితమైతే SQLITE_ENABLE_SNAPSHOT
     && ((pWal->bGetSnapshot==0 && pWal->pSnapshot==0) || pWal->hdr.mx_చట్రం==0)
#అయితే_ముగింపు
    ){
      /* The WAL has been completely backfilled (or it is empty).
      ** and can be safely ignored.
      */
      rc = wal_తాళం_Shared(pWal, WAL_READ_తాళం(0));
      walShmBarrier(pWal);
      ఒకవేళ( rc==SQLITE_OK ){
        ఒకవేళ( memcmp((శూన్య *)wal_సూచీ_Hdr(pWal), &pWal->hdr,పరిమాణం(Wal_సూచీ_Hdr)) ){
          /* It is not safe to allow the reader to continue here if frames
          ** may have been appended to the log before READ_LOCK(0) was obtained.
          ** When holding READ_LOCK(0), the reader ignores the entire log file,
          ** which implies that the database file contains a trustworthy
          ** snapshot. Since holding READ_LOCK(0) prevents a checkpoint from
          ** happening, this is usually correct.
          **
          ** However, if frames have been appended to the log (or if the log
          ** is wrapped and written for that matter) before the READ_LOCK(0)
          ** is obtained, that is not necessarily true. A checkpointer may
          ** have started to backfill the appended frames but crashed before
          ** it finished. Leaving a corrupt image in the database file.
          */
          walUnlockShared(pWal, WAL_READ_తాళం(0));
          తిరిగి WAL_RETRY;
        }
        pWal->read_తాళం = 0;
        తిరిగి SQLITE_OK;
      }లేకపోతే ఒకవేళ( rc!=SQLITE_BUSY ){
        తిరిగి rc;
      }
    }
  
    /* If we get this far, it means that the reader will want to use
    ** the WAL to get at content from recent commits.  The job now is
    ** to select one of the aReadMark[] entries that is closest to
    ** but not exceeding pWal->hdr.mxFrame and lock that entry.
    */
    mxReadMark = 0;
    mxI = 0;
    mx_చట్రం = pWal->hdr.mx_చట్రం;
#నిర్వచితమైతే SQLITE_ENABLE_SNAPSHOT
    ఒకవేళ( pWal->pSnapshot && pWal->pSnapshot->mx_చట్రం<mx_చట్రం ){
      mx_చట్రం = pWal->pSnapshot->mx_చట్రం;
    }
#అయితే_ముగింపు
    కొరకు(i=1; i<WAL_NREADER; i++){
      u32 thisMark = Atomic_ఎక్కించు(pInfo->aReadMark+i); SEH_INJECT_FAULT;
      ఒకవేళ( mxReadMark<=thisMark && thisMark<=mx_చట్రం ){
        ప్రతిజ్ఞా( thisMark!=READMARK_NOT_వాడిన );
        mxReadMark = thisMark;
        mxI = i;
      }
    }
    ఒకవేళ( (pWal->readOnly & WAL_SHM_RDONLY)==0
     && (mxReadMark<mx_చట్రం || mxI==0)
    ){
      కొరకు(i=1; i<WAL_NREADER; i++){
        rc = wal_తాళం_Exclusive(pWal, WAL_READ_తాళం(i), 1);
        ఒకవేళ( rc==SQLITE_OK ){
          AtomicStore(pInfo->aReadMark+i,mx_చట్రం);
          mxReadMark = mx_చట్రం;
          mxI = i;
          walUnlockExclusive(pWal, WAL_READ_తాళం(i), 1);
          ఆపు;
        }లేకపోతే ఒకవేళ( rc!=SQLITE_BUSY ){
          తిరిగి rc;
        }
      }
    }
    ఒకవేళ( mxI==0 ){
      ప్రతిజ్ఞా( rc==SQLITE_BUSY || (pWal->readOnly & WAL_SHM_RDONLY)!=0 );
      తిరిగి rc==SQLITE_BUSY ? WAL_RETRY : SQLITE_READONLY_CANTINIT;
    }
  
    (శూన్య)walEnableBlockingMs(pWal, n_ఖండం_Tmout);
    rc = wal_తాళం_Shared(pWal, WAL_READ_తాళం(mxI));
    walDisableBlocking(pWal);
    ఒకవేళ( rc ){
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
      ఒకవేళ( rc==SQLITE_BUSY_TIMEOUT ){
        *pCnt |= WAL_RETRY_BLOCKED_ముసుగు;
      }
#లేకపోతే
      ప్రతిజ్ఞా( rc!=SQLITE_BUSY_TIMEOUT );
#అయితే_ముగింపు
      ప్రతిజ్ఞా((rc&0xFF)!=SQLITE_BUSY||rc==SQLITE_BUSY||rc==SQLITE_BUSY_TIMEOUT);
      తిరిగి (rc&0xFF)==SQLITE_BUSY ? WAL_RETRY : rc;
    }
    /* Now that the read-lock has been obtained, check that neither the
    ** value in the aReadMark[] array or the contents of the wal-index
    ** header have changed.
    **
    ** It is necessary to check that the wal-index header did not change
    ** between the time it was read and when the shared-lock was obtained
    ** on WAL_READ_LOCK(mxI) was obtained to account for the possibility
    ** that the log file may have been wrapped by a writer, or that frames
    ** that occur later in the log than pWal->hdr.mxFrame may have been
    ** copied into the database by a checkpointer. If either of these things
    ** happened, then reading the database with the current value of
    ** pWal->hdr.mxFrame risks reading a corrupted snapshot. So, retry
    ** instead.
    **
    ** Before checking that the live wal-index header has not changed
    ** since it was read, set Wal.minFrame to the first frame in the wal
    ** file that has not yet been checkpointed. This client will not need
    ** to read any frames earlier than minFrame from the wal file - they
    ** can be safely read directly from the database file.
    **
    ** Because a ShmBarrier() call is made between taking the copy of
    ** nBackfill and checking that the wal-header in shared-memory still
    ** matches the one cached in pWal->hdr, it is guaranteed that the
    ** checkpointer that set nBackfill was not working with a wal-index
    ** header newer than that cached in pWal->hdr. If it were, that could
    ** cause a problem. The checkpointer could omit to checkpoint
    ** a version of page X that lies before pWal->minFrame (call that version
    ** A) on the basis that there is a newer version (version B) of the same
    ** page later in the wal file. But if version B happens to like past
    ** frame pWal->hdr.mxFrame - then the client would incorrectly assume
    ** that it can read version A from the database file. However, since
    ** we can guarantee that the checkpointer that set nBackfill could not
    ** see any pages past pWal->hdr.mxFrame, this problem does not come up.
    */
    pWal->min_చట్రం = Atomic_ఎక్కించు(&pInfo->nBackfill)+1; SEH_INJECT_FAULT;
    walShmBarrier(pWal);
    ఒకవేళ( Atomic_ఎక్కించు(pInfo->aReadMark+mxI)!=mxReadMark
     || memcmp((శూన్య *)wal_సూచీ_Hdr(pWal), &pWal->hdr, పరిమాణం(Wal_సూచీ_Hdr))
    ){
      walUnlockShared(pWal, WAL_READ_తాళం(mxI));
      తిరిగి WAL_RETRY;
    }లేకపోతే{
      ప్రతిజ్ఞా( mxReadMark<=pWal->hdr.mx_చట్రం );
      pWal->read_తాళం = (i16)mxI;
    }
  }
  తిరిగి rc;
}

#నిర్వచితమైతే SQLITE_ENABLE_SNAPSHOT
/*
** This function does the work of sqlite3WalSnapshotRecover().
*/
స్థిర పూర్ణాంకం walSnapshotRecover(
  Wal *pWal,                      /* WAL handle */
  శూన్య *pBuf1,                    /* Temp buffer pWal->szPage bytes in size */
  శూన్య *pBuf2                     /* Temp buffer pWal->szPage bytes in size */
){
  పూర్ణాంకం sz_పుట = (పూర్ణాంకం)pWal->sz_పుట;
  పూర్ణాంకం rc;
  i64 szDb;                       /* Size of db file in bytes */

  rc = sqlite3_Os_File_కొలత(pWal->pDbFd, &szDb);
  ఒకవేళ( rc==SQLITE_OK ){
    అస్థిర WalCkptInfo *pInfo = walCkptInfo(pWal);
    u32 i = pInfo->nBackfillAttempted;
    కొరకు(i=pInfo->nBackfillAttempted; i>Atomic_ఎక్కించు(&pInfo->nBackfill); i--){
      WalHashLoc sLoc;          /* Hash table location */
      u32 pgno;                 /* Page number in db file */
      i64 iDbOff;               /* Offset of db file entry */
      i64 iWalOff;              /* Offset of wal file entry */

      rc = walHashGet(pWal, wal_చట్రం_పుట(i), &sLoc);
      ఒకవేళ( rc!=SQLITE_OK ) ఆపు;
      ప్రతిజ్ఞా( i - sLoc.iZero - 1 >=0 );
      pgno = sLoc.aPgno[i-sLoc.iZero-1];
      iDbOff = (i64)(pgno-1) * sz_పుట;

      ఒకవేళ( iDbOff+sz_పుట<=szDb ){
        iWalOff = wal_చట్రం_స్థానభ్రంశం(i, sz_పుట) + WAL_చట్రం_HDRSIZE;
        rc = sqlite3OsRead(pWal->pWalFd, pBuf1, sz_పుట, iWalOff);

        ఒకవేళ( rc==SQLITE_OK ){
          rc = sqlite3OsRead(pWal->pDbFd, pBuf2, sz_పుట, iDbOff);
        }

        ఒకవేళ( rc!=SQLITE_OK || 0==memcmp(pBuf1, pBuf2, sz_పుట) ){
          ఆపు;
        }
      }

      pInfo->nBackfillAttempted = i-1;
    }
  }

  తిరిగి rc;
}

/*
** Attempt to reduce the value of the WalCkptInfo.nBackfillAttempted
** variable so that older snapshots can be accessed. To do this, loop
** through all wal frames from nBackfillAttempted to (nBackfill+1),
** comparing their content to the corresponding page with the database
** file, if any. Set nBackfillAttempted to the frame number of the
** first frame for which the wal file content matches the db file.
**
** This is only really safe if the file-system is such that any page
** writes made by earlier checkpointers were atomic operations, which
** is not always true. It is also possible that nBackfillAttempted
** may be left set to a value larger than expected, if a wal frame
** contains content that duplicate of an earlier version of the same
** page.
**
** SQLITE_OK is returned if successful, or an SQLite error code if an
** error occurs. It is not an error if nBackfillAttempted cannot be
** decreased at all.
*/
పూర్ణాంకం sqlite3WalSnapshotRecover(Wal *pWal){
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( pWal->read_తాళం>=0 );
  rc = wal_తాళం_Exclusive(pWal, WAL_CKPT_తాళం, 1);
  ఒకవేళ( rc==SQLITE_OK ){
    శూన్య *pBuf1 = sqlite3_malloc(pWal->sz_పుట);
    శూన్య *pBuf2 = sqlite3_malloc(pWal->sz_పుట);
    ఒకవేళ( pBuf1==0 || pBuf2==0 ){
      rc = SQLITE_NOMEM;
    }లేకపోతే{
      pWal->ckpt_తాళం = 1;
      SEH_TRY {
        rc = walSnapshotRecover(pWal, pBuf1, pBuf2);
      }
      SEH_EXCEPT( rc = SQLITE_IOERR_లోపల_పుట; )
      pWal->ckpt_తాళం = 0;
    }

    sqlite3_free(pBuf1);
    sqlite3_free(pBuf2);
    walUnlockExclusive(pWal, WAL_CKPT_తాళం, 1);
  }

  తిరిగి rc;
}
#అయితే_ముగింపు /* SQLITE_ENABLE_SNAPSHOT */

/*
** This function does the work of sqlite3WalBeginReadTransaction() (see 
** below). That function simply calls this one inside an SEH_TRY{...} block.
*/
స్థిర పూర్ణాంకం walBeginReadTransaction(Wal *pWal, పూర్ణాంకం *p_మారింది){
  పూర్ణాంకం rc;                         /* Return code */
  పూర్ణాంకం cnt = 0;                    /* Number of TryBeginRead attempts */
#నిర్వచితమైతే SQLITE_ENABLE_SNAPSHOT
  పూర్ణాంకం ckpt_తాళం = 0;
  పూర్ణాంకం b_మారింది = 0;
  Wal_సూచీ_Hdr *pSnapshot = pWal->pSnapshot;
#అయితే_ముగింపు

  ప్రతిజ్ఞా( pWal->ckpt_తాళం==0 );
  ప్రతిజ్ఞా( pWal->nSehTry>0 );

#నిర్వచితమైతే SQLITE_ENABLE_SNAPSHOT
  ఒకవేళ( pSnapshot ){
    ఒకవేళ( memcmp(pSnapshot, &pWal->hdr, పరిమాణం(Wal_సూచీ_Hdr))!=0 ){
      b_మారింది = 1;
    }

    /* It is possible that there is a checkpointer thread running
    ** concurrent with this code. If this is the case, it may be that the
    ** checkpointer has already determined that it will checkpoint
    ** snapshot X, where X is later in the wal file than pSnapshot, but
    ** has not yet set the pInfo->nBackfillAttempted variable to indicate
    ** its intent. To avoid the race condition this leads to, ensure that
    ** there is no checkpointer process by taking a shared CKPT lock
    ** before checking pInfo->nBackfillAttempted.  */
    (శూన్య)walEnableBlocking(pWal);
    rc = wal_తాళం_Shared(pWal, WAL_CKPT_తాళం);
    walDisableBlocking(pWal);

    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
    ckpt_తాళం = 1;
  }
#అయితే_ముగింపు

  చేయి{
    rc = walTryBeginRead(pWal, p_మారింది, 0, &cnt);
  }అయితే( rc==WAL_RETRY );
  testcase( (rc&0xff)==SQLITE_BUSY );
  testcase( (rc&0xff)==SQLITE_IOERR );
  testcase( rc==SQLITE_PROTOCOL );
  testcase( rc==SQLITE_OK );

#నిర్వచితమైతే SQLITE_ENABLE_SNAPSHOT
  ఒకవేళ( rc==SQLITE_OK ){
    ఒకవేళ( pSnapshot && memcmp(pSnapshot, &pWal->hdr, పరిమాణం(Wal_సూచీ_Hdr))!=0 ){
      /* At this point the client has a lock on an aReadMark[] slot holding
      ** a value equal to or smaller than pSnapshot->mxFrame, but pWal->hdr
      ** is populated with the wal-index header corresponding to the head
      ** of the wal file. Verify that pSnapshot is still valid before
      ** continuing.  Reasons why pSnapshot might no longer be valid:
      **
      **    (1)  The WAL file has been reset since the snapshot was taken.
      **         In this case, the salt will have changed.
      **
      **    (2)  A checkpoint as been attempted that wrote frames past
      **         pSnapshot->mxFrame into the database file.  Note that the
      **         checkpoint need not have completed for this to cause problems.
      */
      అస్థిర WalCkptInfo *pInfo = walCkptInfo(pWal);

      ప్రతిజ్ఞా( pWal->read_తాళం>0 || pWal->hdr.mx_చట్రం==0 );
      ప్రతిజ్ఞా( pInfo->aReadMark[pWal->read_తాళం]<=pSnapshot->mx_చట్రం );

      /* Check that the wal file has not been wrapped. Assuming that it has
      ** not, also check that no checkpointer has attempted to checkpoint any
      ** frames beyond pSnapshot->mxFrame. If either of these conditions are
      ** true, return SQLITE_ERROR_SNAPSHOT. Otherwise, overwrite pWal->hdr
      ** with *pSnapshot and set *pChanged as appropriate for opening the
      ** snapshot.  */
      ఒకవేళ( !memcmp(pSnapshot->aSalt, pWal->hdr.aSalt, పరిమాణం(pWal->hdr.aSalt))
       && pSnapshot->mx_చట్రం>=pInfo->nBackfillAttempted
      ){
        ప్రతిజ్ఞా( pWal->read_తాళం>0 );
        జ్ఞాపకం_నకలు(&pWal->hdr, pSnapshot, పరిమాణం(Wal_సూచీ_Hdr));
        *p_మారింది = b_మారింది;
      }లేకపోతే{
        rc = SQLITE_ERROR_SNAPSHOT;
      }

      /* A client using a non-current snapshot may not ignore any frames
      ** from the start of the wal file. This is because, for a system
      ** where (minFrame < iSnapshot < maxFrame), a checkpointer may
      ** have omitted to checkpoint a frame earlier than minFrame in
      ** the file because there exists a frame after iSnapshot that
      ** is the same database page.  */
      pWal->min_చట్రం = 1;

      ఒకవేళ( rc!=SQLITE_OK ){
        sqlite3_Wal_చివర_Read_Transaction(pWal);
      }
    }
  }

  /* Release the shared CKPT lock obtained above. */
  ఒకవేళ( ckpt_తాళం ){
    ప్రతిజ్ఞా( pSnapshot );
    walUnlockShared(pWal, WAL_CKPT_తాళం);
  }
#అయితే_ముగింపు
  తిరిగి rc;
}

/*
** Begin a read transaction on the database.
**
** This routine used to be called sqlite3OpenSnapshot() and with good reason:
** it takes a snapshot of the state of the WAL and wal-index for the current
** instant in time.  The current thread will continue to use this snapshot.
** Other threads might append new content to the WAL and wal-index but
** that extra content is ignored by the current thread.
**
** If the database contents have changes since the previous read
** transaction, then *pChanged is set to 1 before returning.  The
** Pager layer will use this to know that its cache is stale and
** needs to be flushed.
*/
పూర్ణాంకం sqlite3WalBeginReadTransaction(Wal *pWal, పూర్ణాంకం *p_మారింది){
  పూర్ణాంకం rc;
  SEH_TRY {
    rc = walBeginReadTransaction(pWal, p_మారింది);
  }
  SEH_EXCEPT( rc = walHandleException(pWal); )
  తిరిగి rc;
}

/*
** Finish with a read transaction.  All this does is release the
** read-lock.
*/
శూన్య sqlite3_Wal_చివర_Read_Transaction(Wal *pWal){
#నిర్వచితం_కాకపోతే SQLITE_ENABLE_SETLK_TIMEOUT
  ప్రతిజ్ఞా( pWal->write_తాళం==0 || pWal->read_తాళం<0 );
#అయితే_ముగింపు
  ఒకవేళ( pWal->read_తాళం>=0 ){
    (శూన్య)sqlite3_Wal_చివర_Write_Transaction(pWal);
    walUnlockShared(pWal, WAL_READ_తాళం(pWal->read_తాళం));
    pWal->read_తాళం = -1;
  }
}

/*
** Search the wal file for page pgno. If found, set *piRead to the frame that
** contains the page. Otherwise, if pgno is not in the wal file, set *piRead
** to zero.
**
** Return SQLITE_OK if successful, or an error code if an error occurs. If an
** error does occur, the final value of *piRead is undefined.
*/
స్థిర పూర్ణాంకం wal_కనుగొను_చట్రం(
  Wal *pWal,                      /* WAL handle */
  Pgno pgno,                      /* Database page number to read data for */
  u32 *piRead                     /* OUT: Frame number (or zero) */
){
  u32 iRead = 0;                  /* If !=0, WAL frame to return data from */
  u32 i_చివరి = pWal->hdr.mx_చట్రం;  /* Last page in WAL for this reader */
  పూర్ణాంకం iHash;                      /* Used to loop through N hash tables */
  పూర్ణాంకం iMinHash;

  /* This routine is only be called from within a read transaction. */
  ప్రతిజ్ఞా( pWal->read_తాళం>=0 || pWal->తాళం_Error );

  /* If the "last page" field of the wal-index header snapshot is 0, then
  ** no data will be read from the wal under any circumstances. Return early
  ** in this case as an optimization.  Likewise, if pWal->readLock==0,
  ** then the WAL is ignored by the reader so return early, as if the
  ** WAL were empty.
  */
  ఒకవేళ( i_చివరి==0 || (pWal->read_తాళం==0 && pWal->bShmUnreliable==0) ){
    *piRead = 0;
    తిరిగి SQLITE_OK;
  }

  /* Search the hash table or tables for an entry matching page number
  ** pgno. Each iteration of the following for() loop searches one
  ** hash table (each hash table indexes up to HASHTABLE_NPAGE frames).
  **
  ** This code might run concurrently to the code in walIndexAppend()
  ** that adds entries to the wal-index (and possibly to this hash
  ** table). This means the value just read from the hash
  ** slot (aHash[iKey]) may have been added before or after the
  ** current read transaction was opened. Values added after the
  ** read transaction was opened may have been written incorrectly -
  ** i.e. these slots may contain garbage data. However, we assume
  ** that any slots written before the current read transaction was
  ** opened remain unmodified.
  **
  ** For the reasons above, the if(...) condition featured in the inner
  ** loop of the following block is more stringent that would be required
  ** if we had exclusive access to the hash-table:
  **
  **   (aPgno[iFrame]==pgno):
  **     This condition filters out normal hash-table collisions.
  **
  **   (iFrame<=iLast):
  **     This condition filters out entries that were added to the hash
  **     table after the current read-transaction had started.
  */
  iMinHash = wal_చట్రం_పుట(pWal->min_చట్రం);
  కొరకు(iHash=wal_చట్రం_పుట(i_చివరి); iHash>=iMinHash; iHash--){
    WalHashLoc sLoc;              /* Hash table location */
    పూర్ణాంకం i_కీలకం;                     /* Hash slot index */
    పూర్ణాంకం nCollide;                 /* Number of hash collisions remaining */
    పూర్ణాంకం rc;                       /* Error code */
    u32 iH;

    rc = walHashGet(pWal, iHash, &sLoc);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
    nCollide = HASHTABLE_NSLOT;
    i_కీలకం = walHash(pgno);
    SEH_INJECT_FAULT;
    అయితే( (iH = Atomic_ఎక్కించు(&sLoc.aHash[i_కీలకం]))!=0 ){
      u32 i_చట్రం = iH + sLoc.iZero;
      ఒకవేళ( i_చట్రం<=i_చివరి
       && i_చట్రం>=pWal->min_చట్రం
       && sLoc.aPgno[(iH-1)&(HASHTABLE_NPAGE-1)]==pgno
      ){
        ప్రతిజ్ఞా( i_చట్రం>iRead || CORRUPT_DB );
        iRead = i_చట్రం;
      }
      ఒకవేళ( (nCollide--)==0 ){
        *piRead = 0;
        తిరిగి SQLITE_CORRUPT_BKPT;
      }
      i_కీలకం = wal_తదుపరి_Hash(i_కీలకం);
    }
    ఒకవేళ( iRead ) ఆపు;
  }

#నిర్వచితమైతే SQLITE_ENABLE_EXPENSIVE_ASSERT
  /* If expensive assert() statements are available, do a linear search
  ** of the wal-index file content. Make sure the results agree with the
  ** result obtained using the hash indexes above.  */
  {
    u32 iRead2 = 0;
    u32 iTest;
    ప్రతిజ్ఞా( pWal->bShmUnreliable || pWal->min_చట్రం>0 );
    కొరకు(iTest=i_చివరి; iTest>=pWal->min_చట్రం && iTest>0; iTest--){
      ఒకవేళ( wal_చట్రం_Pgno(pWal, iTest)==pgno ){
        iRead2 = iTest;
        ఆపు;
      }
    }
    ప్రతిజ్ఞా( iRead==iRead2 );
  }
#అయితే_ముగింపు

  *piRead = iRead;
  తిరిగి SQLITE_OK;
}

/*
** Search the wal file for page pgno. If found, set *piRead to the frame that
** contains the page. Otherwise, if pgno is not in the wal file, set *piRead
** to zero.
**
** Return SQLITE_OK if successful, or an error code if an error occurs. If an
** error does occur, the final value of *piRead is undefined.
**
** The difference between this function and walFindFrame() is that this
** function wraps walFindFrame() in an SEH_TRY{...} block.
*/
పూర్ణాంకం sqlite3_Wal_కనుగొను_చట్రం(
  Wal *pWal,                      /* WAL handle */
  Pgno pgno,                      /* Database page number to read data for */
  u32 *piRead                     /* OUT: Frame number (or zero) */
){
  పూర్ణాంకం rc;
  SEH_TRY {
    rc = wal_కనుగొను_చట్రం(pWal, pgno, piRead);
  }
  SEH_EXCEPT( rc = SQLITE_IOERR_లోపల_పుట; )
  తిరిగి rc;
}

/*
** Read the contents of frame iRead from the wal file into buffer pOut
** (which is nOut bytes in size). Return SQLITE_OK if successful, or an
** error code otherwise.
*/
పూర్ణాంకం sqlite3_Wal_Read_చట్రం(
  Wal *pWal,                      /* WAL handle */
  u32 iRead,                      /* Frame to read */
  పూర్ణాంకం n_బహిర్గతం,                       /* Size of buffer pOut in bytes */
  u8 *p_బహిర్గతం                        /* Buffer to write page data to */
){
  పూర్ణాంకం sz;
  i64 i_స్థానభ్రంశం;
  sz = pWal->hdr.sz_పుట;
  sz = (sz&0xfe00) + ((sz&0x0001)<<16);
  ఒకవేళ( n_బహిర్గతం>sz ){
    memset(p_బహిర్గతం+sz, 0, n_బహిర్గతం-sz);
    n_బహిర్గతం = sz;
  }
  testcase( sz<=32768 );
  testcase( sz>=65536 );
  i_స్థానభ్రంశం = wal_చట్రం_స్థానభ్రంశం(iRead, sz) + WAL_చట్రం_HDRSIZE;
  /* testcase( IS_BIG_INT(iOffset) ); // requires a 4GiB WAL */
  తిరిగి sqlite3OsRead(pWal->pWalFd, p_బహిర్గతం, n_బహిర్గతం, i_స్థానభ్రంశం);
}

/*
** Return the size of the database in pages (or zero, if unknown).
*/
Pgno sqlite3WalDbsize(Wal *pWal){
  ఒకవేళ( pWal && ALWAYS(pWal->read_తాళం>=0) ){
    తిరిగి pWal->hdr.n_పుట;
  }
  తిరిగి 0;
}


/*
** This function starts a write transaction on the WAL.
**
** A read transaction must have already been started by a prior call
** to sqlite3WalBeginReadTransaction().
**
** If another thread or process has written into the database since
** the read transaction was started, then it is not possible for this
** thread to write as doing so would cause a fork.  So this routine
** returns SQLITE_BUSY in that case and no write transaction is started.
**
** There can only be a single writer active at a time.
*/
పూర్ణాంకం sqlite3WalBeginWriteTransaction(Wal *pWal){
  పూర్ణాంకం rc;

#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  /* If the write-lock is already held, then it was obtained before the
  ** read-transaction was even opened, making this call a no-op.
  ** Return early. */
  ఒకవేళ( pWal->write_తాళం ){
    ప్రతిజ్ఞా( !memcmp(&pWal->hdr,(శూన్య*)pWal->ap_Wi_దత్తాంశం[0],పరిమాణం(Wal_సూచీ_Hdr)) );
    తిరిగి SQLITE_OK;
  }
#అయితే_ముగింపు

  /* Cannot start a write transaction without first holding a read
  ** transaction. */
  ప్రతిజ్ఞా( pWal->read_తాళం>=0 );
  ప్రతిజ్ఞా( pWal->write_తాళం==0 && pWal->iReCksum==0 );

  ఒకవేళ( pWal->readOnly ){
    తిరిగి SQLITE_READONLY;
  }

  /* Only one writer allowed at a time.  Get the write lock.  Return
  ** SQLITE_BUSY if unable.
  */
  rc = wal_తాళం_Exclusive(pWal, WAL_WRITE_తాళం, 1);
  ఒకవేళ( rc ){
    తిరిగి rc;
  }
  pWal->write_తాళం = 1;

  /* If another connection has written to the database file since the
  ** time the read transaction on this connection was started, then
  ** the write is disallowed.
  */
  SEH_TRY {
    ఒకవేళ( memcmp(&pWal->hdr, (శూన్య *)wal_సూచీ_Hdr(pWal), పరిమాణం(Wal_సూచీ_Hdr))!=0 ){
      rc = SQLITE_BUSY_SNAPSHOT;
    }
  }
  SEH_EXCEPT( rc = SQLITE_IOERR_లోపల_పుట; )

  ఒకవేళ( rc!=SQLITE_OK ){
    walUnlockExclusive(pWal, WAL_WRITE_తాళం, 1);
    pWal->write_తాళం = 0;
  }
  తిరిగి rc;
}

/*
** End a write transaction.  The commit has already been done.  This
** routine merely releases the lock.
*/
పూర్ణాంకం sqlite3_Wal_చివర_Write_Transaction(Wal *pWal){
  ఒకవేళ( pWal->write_తాళం ){
    walUnlockExclusive(pWal, WAL_WRITE_తాళం, 1);
    pWal->write_తాళం = 0;
    pWal->iReCksum = 0;
    pWal->truncateOnCommit = 0;
  }
  తిరిగి SQLITE_OK;
}

/*
** If any data has been written (but not committed) to the log file, this
** function moves the write-pointer back to the start of the transaction.
**
** Additionally, the callback function is invoked for each frame written
** to the WAL since the start of the transaction. If the callback returns
** other than SQLITE_OK, it is not invoked again and the error code is
** returned to the caller.
**
** Otherwise, if the callback function does not return an error, this
** function returns SQLITE_OK.
*/
పూర్ణాంకం sqlite3WalUndo(Wal *pWal, పూర్ణాంకం (*xUndo)(శూన్య *, Pgno), శూన్య *pUndoCtx){
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( ALWAYS(pWal->write_తాళం) ){
    Pgno iMax = pWal->hdr.mx_చట్రం;
    Pgno i_చట్రం;

    SEH_TRY {
      /* Restore the clients cache of the wal-index header to the state it
      ** was in before the client began writing to the database.
      */
      జ్ఞాపకం_నకలు(&pWal->hdr, (శూన్య *)wal_సూచీ_Hdr(pWal), పరిమాణం(Wal_సూచీ_Hdr));

      కొరకు(i_చట్రం=pWal->hdr.mx_చట్రం+1;
          ALWAYS(rc==SQLITE_OK) && i_చట్రం<=iMax;
          i_చట్రం++
      ){
        /* This call cannot fail. Unless the page for which the page number
        ** is passed as the second argument is (a) in the cache and
        ** (b) has an outstanding reference, then xUndo is either a no-op
        ** (if (a) is false) or simply expels the page from the cache (if (b)
        ** is false).
        **
        ** If the upper layer is doing a rollback, it is guaranteed that there
        ** are no outstanding references to any page other than page 1. And
        ** page 1 is never written to the log until the transaction is
        ** committed. As a result, the call to xUndo may not fail.
        */
        ప్రతిజ్ఞా( wal_చట్రం_Pgno(pWal, i_చట్రం)!=1 );
        rc = xUndo(pUndoCtx, wal_చట్రం_Pgno(pWal, i_చట్రం));
      }
      ఒకవేళ( iMax!=pWal->hdr.mx_చట్రం ) walCleanupHash(pWal);
    }
    SEH_EXCEPT( rc = SQLITE_IOERR_లోపల_పుట; )
    pWal->iReCksum = 0;
  }
  తిరిగి rc;
}

/*
** Argument aWalData must point to an array of WAL_SAVEPOINT_NDATA u32
** values. This function populates the array with values required to
** "rollback" the write position of the WAL handle back to the current
** point in the event of a savepoint rollback (via WalSavepointUndo()).
*/
శూన్య sqlite3WalSavepoint(Wal *pWal, u32 *a_Wal_దత్తాంశం){
  ప్రతిజ్ఞా( pWal->write_తాళం );
  a_Wal_దత్తాంశం[0] = pWal->hdr.mx_చట్రం;
  a_Wal_దత్తాంశం[1] = pWal->hdr.a_చట్రం_Cksum[0];
  a_Wal_దత్తాంశం[2] = pWal->hdr.a_చట్రం_Cksum[1];
  a_Wal_దత్తాంశం[3] = pWal->nCkpt;
}

/*
** Move the write position of the WAL back to the point identified by
** the values in the aWalData[] array. aWalData must point to an array
** of WAL_SAVEPOINT_NDATA u32 values that has been previously populated
** by a call to WalSavepoint().
*/
పూర్ణాంకం sqlite3WalSavepointUndo(Wal *pWal, u32 *a_Wal_దత్తాంశం){
  పూర్ణాంకం rc = SQLITE_OK;

  ప్రతిజ్ఞా( pWal->write_తాళం );
  ప్రతిజ్ఞా( a_Wal_దత్తాంశం[3]!=pWal->nCkpt || a_Wal_దత్తాంశం[0]<=pWal->hdr.mx_చట్రం );

  ఒకవేళ( a_Wal_దత్తాంశం[3]!=pWal->nCkpt ){
    /* This savepoint was opened immediately after the write-transaction
    ** was started. Right after that, the writer decided to wrap around
    ** to the start of the log. Update the savepoint values to match.
    */
    a_Wal_దత్తాంశం[0] = 0;
    a_Wal_దత్తాంశం[3] = pWal->nCkpt;
  }

  ఒకవేళ( a_Wal_దత్తాంశం[0]<pWal->hdr.mx_చట్రం ){
    pWal->hdr.mx_చట్రం = a_Wal_దత్తాంశం[0];
    pWal->hdr.a_చట్రం_Cksum[0] = a_Wal_దత్తాంశం[1];
    pWal->hdr.a_చట్రం_Cksum[1] = a_Wal_దత్తాంశం[2];
    SEH_TRY {
      walCleanupHash(pWal);
    }
    SEH_EXCEPT( rc = SQLITE_IOERR_లోపల_పుట; )
    ఒకవేళ( pWal->iReCksum>pWal->hdr.mx_చట్రం ){
      pWal->iReCksum = 0;
    }
  }

  తిరిగి rc;
}

/*
** This function is called just before writing a set of frames to the log
** file (see sqlite3WalFrames()). It checks to see if, instead of appending
** to the current log file, it is possible to overwrite the start of the
** existing log file with the new frames (i.e. "reset" the log). If so,
** it sets pWal->hdr.mxFrame to 0. Otherwise, pWal->hdr.mxFrame is left
** unchanged.
**
** SQLITE_OK is returned if no error is encountered (regardless of whether
** or not pWal->hdr.mxFrame is modified). An SQLite error code is returned
** if an error occurs.
*/
స్థిర పూర్ణాంకం walRestartLog(Wal *pWal){
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం cnt;

  ఒకవేళ( pWal->read_తాళం==0 ){
    అస్థిర WalCkptInfo *pInfo = walCkptInfo(pWal);
    ప్రతిజ్ఞా( pInfo->nBackfill==pWal->hdr.mx_చట్రం );
    ఒకవేళ( pInfo->nBackfill>0 ){
      u32 salt1;
      sqlite3_randomness(4, &salt1);
      rc = wal_తాళం_Exclusive(pWal, WAL_READ_తాళం(1), WAL_NREADER-1);
      ఒకవేళ( rc==SQLITE_OK ){
        /* If all readers are using WAL_READ_LOCK(0) (in other words if no
        ** readers are currently using the WAL), then the transactions
        ** frames will overwrite the start of the existing log. Update the
        ** wal-index header to reflect this.
        **
        ** In theory it would be Ok to update the cache of the header only
        ** at this point. But updating the actual wal-index header is also
        ** safe and means there is no special case for sqlite3WalUndo()
        ** to handle if this transaction is rolled back.  */
        walRestartHdr(pWal, salt1);
        walUnlockExclusive(pWal, WAL_READ_తాళం(1), WAL_NREADER-1);
      }లేకపోతే ఒకవేళ( rc!=SQLITE_BUSY ){
        తిరిగి rc;
      }
    }
    walUnlockShared(pWal, WAL_READ_తాళం(0));
    pWal->read_తాళం = -1;
    cnt = 0;
    చేయి{
      పూర్ణాంకం not_వాడిన;
      rc = walTryBeginRead(pWal, &not_వాడిన, 1, &cnt);
    }అయితే( rc==WAL_RETRY );
    ప్రతిజ్ఞా( (rc&0xff)!=SQLITE_BUSY ); /* BUSY not possible when useWal==1 */
    testcase( (rc&0xff)==SQLITE_IOERR );
    testcase( rc==SQLITE_PROTOCOL );
    testcase( rc==SQLITE_OK );
  }
  తిరిగి rc;
}

/*
** Information about the current state of the WAL file and where
** the next fsync should occur - passed from sqlite3WalFrames() into
** walWriteToLog().
*/
రకనిర్వచనం నిర్మాణం WalWriter {
  Wal *pWal;                   /* The complete WAL information */
  sqlite3_file *pFd;           /* The WAL file to which we write */
  sqlite3_int64 i_Sync_బిందువు;    /* Fsync at this offset */
  పూర్ణాంకం syncFlags;               /* Flags for the fsync */
  పూర్ణాంకం sz_పుట;                  /* Size of one page */
} WalWriter;

/*
** Write iAmt bytes of content into the WAL file beginning at iOffset.
** Do a sync when crossing the p->iSyncPoint boundary.
**
** In other words, if iSyncPoint is in between iOffset and iOffset+iAmt,
** first write the part before iSyncPoint, then sync, then write the
** rest.
*/
స్థిర పూర్ణాంకం wal_Write_గమ్యం_Log(
  WalWriter *p,              /* WAL to write to */
  శూన్య *pContent,            /* Content to be written */
  పూర్ణాంకం iAmt,                  /* Number of bytes to write */
  sqlite3_int64 i_స్థానభ్రంశం      /* Start writing at this offset */
){
  పూర్ణాంకం rc;
  ఒకవేళ( i_స్థానభ్రంశం<p->i_Sync_బిందువు && i_స్థానభ్రంశం+iAmt>=p->i_Sync_బిందువు ){
    పూర్ణాంకం i_మొదటి_Amt = (పూర్ణాంకం)(p->i_Sync_బిందువు - i_స్థానభ్రంశం);
    rc = sqlite3OsWrite(p->pFd, pContent, i_మొదటి_Amt, i_స్థానభ్రంశం);
    ఒకవేళ( rc ) తిరిగి rc;
    i_స్థానభ్రంశం += i_మొదటి_Amt;
    iAmt -= i_మొదటి_Amt;
    pContent = (శూన్య*)(i_మొదటి_Amt + (అక్షరం*)pContent);
    ప్రతిజ్ఞా( WAL_SYNC_FLAGS(p->syncFlags)!=0 );
    rc = sqlite3OsSync(p->pFd, WAL_SYNC_FLAGS(p->syncFlags));
    ఒకవేళ( iAmt==0 || rc ) తిరిగి rc;
  }
  rc = sqlite3OsWrite(p->pFd, pContent, iAmt, i_స్థానభ్రంశం);
  తిరిగి rc;
}

/*
** Write out a single frame of the WAL
*/
స్థిర పూర్ణాంకం wal_Write_One_చట్రం(
  WalWriter *p,               /* Where to write the frame */
  PgHdr *p_పుట,               /* The page of the frame to be written */
  పూర్ణాంకం nTruncate,              /* The commit flag.  Usually 0.  >0 for commit */
  sqlite3_int64 i_స్థానభ్రంశం       /* Byte offset at which to write */
){
  పూర్ణాంకం rc;                         /* Result code from subfunctions */
  శూన్య *p_దత్తాంశం;                    /* Data actually written */
  u8 a_చట్రం[WAL_చట్రం_HDRSIZE];   /* Buffer to assemble frame-header in */
  p_దత్తాంశం = p_పుట->p_దత్తాంశం;
  wal_Encode_చట్రం(p->pWal, p_పుట->pgno, nTruncate, p_దత్తాంశం, a_చట్రం);
  rc = wal_Write_గమ్యం_Log(p, a_చట్రం, పరిమాణం(a_చట్రం), i_స్థానభ్రంశం);
  ఒకవేళ( rc ) తిరిగి rc;
  /* Write the page data */
  rc = wal_Write_గమ్యం_Log(p, p_దత్తాంశం, p->sz_పుట, i_స్థానభ్రంశం+పరిమాణం(a_చట్రం));
  తిరిగి rc;
}

/*
** This function is called as part of committing a transaction within which
** one or more frames have been overwritten. It updates the checksums for
** all frames written to the wal file by the current transaction starting
** with the earliest to have been overwritten.
**
** SQLITE_OK is returned if successful, or an SQLite error code otherwise.
*/
స్థిర పూర్ణాంకం walRewriteChecksums(Wal *pWal, u32 i_చివరి){
  స్థిరరాశి పూర్ణాంకం sz_పుట = pWal->sz_పుట;/* Database page size */
  పూర్ణాంకం rc = SQLITE_OK;             /* Return code */
  u8 *aBuf;                       /* Buffer to load data from wal file into */
  u8 a_చట్రం[WAL_చట్రం_HDRSIZE];   /* Buffer to assemble frame-headers in */
  u32 iRead;                      /* Next frame to read from wal file */
  i64 iCksumOff;

  aBuf = sqlite3_malloc(sz_పుట + WAL_చట్రం_HDRSIZE);
  ఒకవేళ( aBuf==0 ) తిరిగి SQLITE_NOMEM_BKPT;

  /* Find the checksum values to use as input for the recalculating the
  ** first checksum. If the first frame is frame 1 (implying that the current
  ** transaction restarted the wal file), these values must be read from the
  ** wal-file header. Otherwise, read them from the frame header of the
  ** previous frame.  */
  ప్రతిజ్ఞా( pWal->iReCksum>0 );
  ఒకవేళ( pWal->iReCksum==1 ){
    iCksumOff = 24;
  }లేకపోతే{
    iCksumOff = wal_చట్రం_స్థానభ్రంశం(pWal->iReCksum-1, sz_పుట) + 16;
  }
  rc = sqlite3OsRead(pWal->pWalFd, aBuf, పరిమాణం(u32)*2, iCksumOff);
  pWal->hdr.a_చట్రం_Cksum[0] = sqlite3Get4byte(aBuf);
  pWal->hdr.a_చట్రం_Cksum[1] = sqlite3Get4byte(&aBuf[పరిమాణం(u32)]);

  iRead = pWal->iReCksum;
  pWal->iReCksum = 0;
  కొరకు(; rc==SQLITE_OK && iRead<=i_చివరి; iRead++){
    i64 iOff = wal_చట్రం_స్థానభ్రంశం(iRead, sz_పుట);
    rc = sqlite3OsRead(pWal->pWalFd, aBuf, sz_పుట+WAL_చట్రం_HDRSIZE, iOff);
    ఒకవేళ( rc==SQLITE_OK ){
      u32 iPgno, n_Db_కొలత;
      iPgno = sqlite3Get4byte(aBuf);
      n_Db_కొలత = sqlite3Get4byte(&aBuf[4]);

      wal_Encode_చట్రం(pWal, iPgno, n_Db_కొలత, &aBuf[WAL_చట్రం_HDRSIZE], a_చట్రం);
      rc = sqlite3OsWrite(pWal->pWalFd, a_చట్రం, పరిమాణం(a_చట్రం), iOff);
    }
  }

  sqlite3_free(aBuf);
  తిరిగి rc;
}

/*
** Write a set of frames to the log. The caller must hold the write-lock
** on the log file (obtained using sqlite3WalBeginWriteTransaction()).
*/
స్థిర పూర్ణాంకం wal_చట్రాలు(
  Wal *pWal,                      /* Wal handle to write to */
  పూర్ణాంకం sz_పుట,                     /* Database page-size in bytes */
  PgHdr *pList,                   /* List of dirty pages to write */
  Pgno nTruncate,                 /* Database size after this commit */
  పూర్ణాంకం isCommit,                   /* True if this is a commit */
  పూర్ణాంకం sync_flags                  /* Flags to pass to OsSync() (or 0) */
){
  పూర్ణాంకం rc;                         /* Used to catch return codes */
  u32 i_చట్రం;                     /* Next frame address */
  PgHdr *p;                       /* Iterator to run through pList with. */
  PgHdr *p_చివరి = 0;               /* Last frame in list */
  పూర్ణాంకం nExtra = 0;                 /* Number of extra copies of last page */
  పూర్ణాంకం sz_చట్రం;                    /* The size of a single frame */
  i64 i_స్థానభ్రంశం;                    /* Next byte to write in WAL file */
  WalWriter w;                    /* The writer */
  u32 i_మొదటి = 0;                 /* First frame that may be overwritten */
  Wal_సూచీ_Hdr *p_సజీవ;             /* Pointer to shared header */

  ప్రతిజ్ఞా( pList );
  ప్రతిజ్ఞా( pWal->write_తాళం );

  /* If this frame set completes a transaction, then nTruncate>0.  If
  ** nTruncate==0 then this frame set does not complete the transaction. */
  ప్రతిజ్ఞా( (isCommit!=0)==(nTruncate!=0) );

#ఒకవేళ defined(SQLITE_TEST) && defined(SQLITE_DEBUG)
  { పూర్ణాంకం cnt; కొరకు(cnt=0, p=pList; p; p=p->pDirty, cnt++){}
    WALTRACE(("WAL%p: frame write begin. %d frames. mxFrame=%d. %s\n",
              pWal, cnt, pWal->hdr.mx_చట్రం, isCommit ? "Commit" : "Spill"));
  }
#అయితే_ముగింపు

  p_సజీవ = (Wal_సూచీ_Hdr*)wal_సూచీ_Hdr(pWal);
  ఒకవేళ( memcmp(&pWal->hdr, (శూన్య *)p_సజీవ, పరిమాణం(Wal_సూచీ_Hdr))!=0 ){
    i_మొదటి = p_సజీవ->mx_చట్రం+1;
  }

  /* See if it is possible to write these frames into the start of the
  ** log file, instead of appending to it at pWal->hdr.mxFrame.
  */
  ఒకవేళ( SQLITE_OK!=(rc = walRestartLog(pWal)) ){
    తిరిగి rc;
  }

  /* If this is the first frame written into the log, write the WAL
  ** header to the start of the WAL file. See comments at the top of
  ** this source file for a description of the WAL header format.
  */
  i_చట్రం = pWal->hdr.mx_చట్రం;
  ఒకవేళ( i_చట్రం==0 ){
    u8 aWalHdr[WAL_HDRSIZE];      /* Buffer to assemble wal-header in */
    u32 aCksum[2];                /* Checksum for wal-header */

    sqlite3Put4byte(&aWalHdr[0], (WAL_MAGIC | SQLITE_BIGENDIAN));
    sqlite3Put4byte(&aWalHdr[4], WAL_MAX_సంస్కరణ);
    sqlite3Put4byte(&aWalHdr[8], sz_పుట);
    sqlite3Put4byte(&aWalHdr[12], pWal->nCkpt);
    ఒకవేళ( pWal->nCkpt==0 ) sqlite3_randomness(8, pWal->hdr.aSalt);
    జ్ఞాపకం_నకలు(&aWalHdr[16], pWal->hdr.aSalt, 8);
    wal_Checksum_బైట్లు(1, aWalHdr, WAL_HDRSIZE-2*4, 0, aCksum);
    sqlite3Put4byte(&aWalHdr[24], aCksum[0]);
    sqlite3Put4byte(&aWalHdr[28], aCksum[1]);

    pWal->sz_పుట = sz_పుట;
    pWal->hdr.big_చివర_Cksum = SQLITE_BIGENDIAN;
    pWal->hdr.a_చట్రం_Cksum[0] = aCksum[0];
    pWal->hdr.a_చట్రం_Cksum[1] = aCksum[1];
    pWal->truncateOnCommit = 1;

    rc = sqlite3OsWrite(pWal->pWalFd, aWalHdr, పరిమాణం(aWalHdr), 0);
    WALTRACE(("WAL%p: wal-header write %s\n", pWal, rc ? "failed" : "ok"));
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }

    /* Sync the header (unless SQLITE_IOCAP_SEQUENTIAL is true or unless
    ** all syncing is turned off by PRAGMA synchronous=OFF).  Otherwise
    ** an out-of-order write following a WAL restart could result in
    ** database corruption.  See the ticket:
    **
    **     https://sqlite.org/src/info/ff5be73dee
    */
    ఒకవేళ( pWal->sync_శీర్షిక ){
      rc = sqlite3OsSync(pWal->pWalFd, CKPT_SYNC_FLAGS(sync_flags));
      ఒకవేళ( rc ) తిరిగి rc;
    }
  }
  ఒకవేళ( (పూర్ణాంకం)pWal->sz_పుట!=sz_పుట ){
    తిరిగి SQLITE_CORRUPT_BKPT;  /* TH3 test case: cov1/corrupt155.test */
  }

  /* Setup information needed to write frames into the WAL */
  w.pWal = pWal;
  w.pFd = pWal->pWalFd;
  w.i_Sync_బిందువు = 0;
  w.syncFlags = sync_flags;
  w.sz_పుట = sz_పుట;
  i_స్థానభ్రంశం = wal_చట్రం_స్థానభ్రంశం(i_చట్రం+1, sz_పుట);
  sz_చట్రం = sz_పుట + WAL_చట్రం_HDRSIZE;

  /* Write all frames into the log file exactly once */
  కొరకు(p=pList; p; p=p->pDirty){
    పూర్ణాంకం n_Db_కొలత;   /* 0 normally.  Positive == commit flag */

    /* Check if this page has already been written into the wal file by
    ** the current transaction. If so, overwrite the existing frame and
    ** set Wal.writeLock to WAL_WRITELOCK_RECKSUM - indicating that
    ** checksums must be recomputed when the transaction is committed.  */
    ఒకవేళ( i_మొదటి && (p->pDirty || isCommit==0) ){
      u32 iWrite = 0;
      VVA_ONLY(rc =) wal_కనుగొను_చట్రం(pWal, p->pgno, &iWrite);
      ప్రతిజ్ఞా( rc==SQLITE_OK || iWrite==0 );
      ఒకవేళ( iWrite>=i_మొదటి ){
        i64 iOff = wal_చట్రం_స్థానభ్రంశం(iWrite, sz_పుట) + WAL_చట్రం_HDRSIZE;
        శూన్య *p_దత్తాంశం;
        ఒకవేళ( pWal->iReCksum==0 || iWrite<pWal->iReCksum ){
          pWal->iReCksum = iWrite;
        }
        p_దత్తాంశం = p->p_దత్తాంశం;
        rc = sqlite3OsWrite(pWal->pWalFd, p_దత్తాంశం, sz_పుట, iOff);
        ఒకవేళ( rc ) తిరిగి rc;
        p->flags &= ~PGHDR_WAL_APPEND;
        కొనసాగు;
      }
    }

    i_చట్రం++;
    ప్రతిజ్ఞా( i_స్థానభ్రంశం==wal_చట్రం_స్థానభ్రంశం(i_చట్రం, sz_పుట) );
    n_Db_కొలత = (isCommit && p->pDirty==0) ? nTruncate : 0;
    rc = wal_Write_One_చట్రం(&w, p, n_Db_కొలత, i_స్థానభ్రంశం);
    ఒకవేళ( rc ) తిరిగి rc;
    p_చివరి = p;
    i_స్థానభ్రంశం += sz_చట్రం;
    p->flags |= PGHDR_WAL_APPEND;
  }

  /* Recalculate checksums within the wal file if required. */
  ఒకవేళ( isCommit && pWal->iReCksum ){
    rc = walRewriteChecksums(pWal, i_చట్రం);
    ఒకవేళ( rc ) తిరిగి rc;
  }

  /* If this is the end of a transaction, then we might need to pad
  ** the transaction and/or sync the WAL file.
  **
  ** Padding and syncing only occur if this set of frames complete a
  ** transaction and if PRAGMA synchronous=FULL.  If synchronous==NORMAL
  ** or synchronous==OFF, then no padding or syncing are needed.
  **
  ** If SQLITE_IOCAP_POWERSAFE_OVERWRITE is defined, then padding is not
  ** needed and only the sync is done.  If padding is needed, then the
  ** final frame is repeated (with its commit mark) until the next sector
  ** boundary is crossed.  Only the part of the WAL prior to the last
  ** sector boundary is synced; the part of the last frame that extends
  ** past the sector boundary is written after the sync.
  */
  ఒకవేళ( isCommit && WAL_SYNC_FLAGS(sync_flags)!=0 ){
    పూర్ణాంకం bSync = 1;
    ఒకవేళ( pWal->pad_గమ్యం_Sector_Boundary ){
      పూర్ణాంకం sector_కొలత = sqlite3_Sector_కొలత(pWal->pWalFd);
      w.i_Sync_బిందువు = ((i_స్థానభ్రంశం+sector_కొలత-1)/sector_కొలత)*sector_కొలత;
      bSync = (w.i_Sync_బిందువు==i_స్థానభ్రంశం);
      testcase( bSync );
      అయితే( i_స్థానభ్రంశం<w.i_Sync_బిందువు ){
        rc = wal_Write_One_చట్రం(&w, p_చివరి, nTruncate, i_స్థానభ్రంశం);
        ఒకవేళ( rc ) తిరిగి rc;
        i_స్థానభ్రంశం += sz_చట్రం;
        nExtra++;
        ప్రతిజ్ఞా( p_చివరి!=0 );
      }
    }
    ఒకవేళ( bSync ){
      ప్రతిజ్ఞా( rc==SQLITE_OK );
      rc = sqlite3OsSync(w.pFd, WAL_SYNC_FLAGS(sync_flags));
    }
  }

  /* If this frame set completes the first transaction in the WAL and
  ** if PRAGMA journal_size_limit is set, then truncate the WAL to the
  ** journal size limit, if possible.
  */
  ఒకవేళ( isCommit && pWal->truncateOnCommit && pWal->mx_Wal_కొలత>=0 ){
    i64 sz = pWal->mx_Wal_కొలత;
    ఒకవేళ( wal_చట్రం_స్థానభ్రంశం(i_చట్రం+nExtra+1, sz_పుట)>pWal->mx_Wal_కొలత ){
      sz = wal_చట్రం_స్థానభ్రంశం(i_చట్రం+nExtra+1, sz_పుట);
    }
    wal_హద్దు_కొలత(pWal, sz);
    pWal->truncateOnCommit = 0;
  }

  /* Append data to the wal-index. It is not necessary to lock the
  ** wal-index to do this as the SQLITE_SHM_WRITE lock held on the wal-index
  ** guarantees that there are no other writers, and no data that may
  ** be in use by existing readers is being overwritten.
  */
  i_చట్రం = pWal->hdr.mx_చట్రం;
  కొరకు(p=pList; p && rc==SQLITE_OK; p=p->pDirty){
    ఒకవేళ( (p->flags & PGHDR_WAL_APPEND)==0 ) కొనసాగు;
    i_చట్రం++;
    rc = wal_సూచీ_Append(pWal, i_చట్రం, p->pgno);
  }
  ప్రతిజ్ఞా( p_చివరి!=0 || nExtra==0 );
  అయితే( rc==SQLITE_OK && nExtra>0 ){
    i_చట్రం++;
    nExtra--;
    rc = wal_సూచీ_Append(pWal, i_చట్రం, p_చివరి->pgno);
  }

  ఒకవేళ( rc==SQLITE_OK ){
    /* Update the private copy of the header. */
    pWal->hdr.sz_పుట = (u16)((sz_పుట&0xff00) | (sz_పుట>>16));
    testcase( sz_పుట<=32768 );
    testcase( sz_పుట>=65536 );
    pWal->hdr.mx_చట్రం = i_చట్రం;
    ఒకవేళ( isCommit ){
      pWal->hdr.iChange++;
      pWal->hdr.n_పుట = nTruncate;
    }
    /* If this is a commit, update the wal-index header too. */
    ఒకవేళ( isCommit ){
      wal_సూచీ_Write_Hdr(pWal);
      pWal->iCallback = i_చట్రం;
    }
  }

  WALTRACE(("WAL%p: frame write %s\n", pWal, rc ? "failed" : "ok"));
  తిరిగి rc;
}

/* 
** Write a set of frames to the log. The caller must hold the write-lock
** on the log file (obtained using sqlite3WalBeginWriteTransaction()).
**
** The difference between this function and walFrames() is that this
** function wraps walFrames() in an SEH_TRY{...} block.
*/
పూర్ణాంకం sqlite3_Wal_చట్రాలు(
  Wal *pWal,                      /* Wal handle to write to */
  పూర్ణాంకం sz_పుట,                     /* Database page-size in bytes */
  PgHdr *pList,                   /* List of dirty pages to write */
  Pgno nTruncate,                 /* Database size after this commit */
  పూర్ణాంకం isCommit,                   /* True if this is a commit */
  పూర్ణాంకం sync_flags                  /* Flags to pass to OsSync() (or 0) */
){
  పూర్ణాంకం rc;
  SEH_TRY {
    rc = wal_చట్రాలు(pWal, sz_పుట, pList, nTruncate, isCommit, sync_flags);
  }
  SEH_EXCEPT( rc = walHandleException(pWal); )
  తిరిగి rc;
}

/*
** This routine is called to implement sqlite3_wal_checkpoint() and
** related interfaces.
**
** Obtain a CHECKPOINT lock and then backfill as much information as
** we can from WAL into the database.
**
** If parameter xBusy is not NULL, it is a pointer to a busy-handler
** callback. In this case this function runs a blocking checkpoint.
*/
పూర్ణాంకం sqlite3WalCheckpoint(
  Wal *pWal,                      /* Wal connection */
  sqlite3 *db,                    /* Check this handle's interrupt flag */
  పూర్ణాంకం e_బహుళకం,                      /* PASSIVE, FULL, RESTART, or TRUNCATE */
  పూర్ణాంకం (*xBusy)(శూన్య*),            /* Function to call when busy */
  శూన్య *pBusyArg,                 /* Context argument for xBusyHandler */
  పూర్ణాంకం sync_flags,                 /* Flags to sync db file with (or 0) */
  పూర్ణాంకం nBuf,                       /* Size of temporary buffer */
  u8 *zBuf,                       /* Temporary buffer to use */
  పూర్ణాంకం *pnLog,                     /* OUT: Number of frames in WAL */
  పూర్ణాంకం *pnCkpt                     /* OUT: Number of backfilled frames in WAL */
){
  పూర్ణాంకం rc;                         /* Return code */
  పూర్ణాంకం is_మారింది = 0;              /* True if a new wal-index header is loaded */
  పూర్ణాంకం eMode2 = e_బహుళకం;             /* Mode to pass to walCheckpoint() */
  పూర్ణాంకం (*xBusy2)(శూన్య*) = xBusy;   /* Busy handler for eMode2 */

  ప్రతిజ్ఞా( pWal->ckpt_తాళం==0 );
  ప్రతిజ్ఞా( pWal->write_తాళం==0 );

  /* EVIDENCE-OF: R-62920-47450 The busy-handler callback is never invoked
  ** in the SQLITE_CHECKPOINT_PASSIVE mode. */
  ప్రతిజ్ఞా( SQLITE_CHECKPOINT_NOOP<SQLITE_CHECKPOINT_PASSIVE );
  ప్రతిజ్ఞా( e_బహుళకం>SQLITE_CHECKPOINT_PASSIVE || xBusy==0 );

  ఒకవేళ( pWal->readOnly ) తిరిగి SQLITE_READONLY;
  WALTRACE(("WAL%p: checkpoint begins\n", pWal));

  /* Enable blocking locks, if possible. */
  sqlite3WalDb(pWal, db);
  ఒకవేళ( xBusy2 ) (శూన్య)walEnableBlocking(pWal);

  /* IMPLEMENTATION-OF: R-62028-47212 All calls obtain an exclusive
  ** "checkpoint" lock on the database file.
  ** EVIDENCE-OF: R-10421-19736 If any other process is running a
  ** checkpoint operation at the same time, the lock cannot be obtained and
  ** SQLITE_BUSY is returned.
  ** EVIDENCE-OF: R-53820-33897 Even if there is a busy-handler configured,
  ** it will not be invoked in this case.
  */
  ఒకవేళ( e_బహుళకం!=SQLITE_CHECKPOINT_NOOP ){
    rc = wal_తాళం_Exclusive(pWal, WAL_CKPT_తాళం, 1);
    testcase( rc==SQLITE_BUSY );
    testcase( rc!=SQLITE_OK && xBusy2!=0 );
    ఒకవేళ( rc==SQLITE_OK ){
      pWal->ckpt_తాళం = 1;

      /* IMPLEMENTATION-OF: R-59782-36818 The SQLITE_CHECKPOINT_FULL, RESTART 
      ** and TRUNCATE modes also obtain the exclusive "writer" lock on the 
      ** database file.
      **
      ** EVIDENCE-OF: R-60642-04082 If the writer lock cannot be obtained
      ** immediately, and a busy-handler is configured, it is invoked and the
      ** writer lock retried until either the busy-handler returns 0 or the
      ** lock is successfully obtained.
      */
      ఒకవేళ( e_బహుళకం!=SQLITE_CHECKPOINT_PASSIVE ){
        rc = wal_Busy_తాళం(pWal, xBusy2, pBusyArg, WAL_WRITE_తాళం, 1);
        ఒకవేళ( rc==SQLITE_OK ){
          pWal->write_తాళం = 1;
        }లేకపోతే ఒకవేళ( rc==SQLITE_BUSY ){
          eMode2 = SQLITE_CHECKPOINT_PASSIVE;
          xBusy2 = 0;
          rc = SQLITE_OK;
        }
      }
    }
  }లేకపోతే{
    rc = SQLITE_OK;
  }


  /* Read the wal-index header. */
  SEH_TRY {
    ఒకవేళ( rc==SQLITE_OK ){
      /* For a passive checkpoint, do not re-enable blocking locks after
      ** reading the wal-index header. A passive checkpoint should not block 
      ** or invoke the busy handler. The only lock such a checkpoint may 
      ** attempt to obtain is a lock on a read-slot, and it should give up
      ** immediately and do a partial checkpoint if it cannot obtain it. */
      walDisableBlocking(pWal);
      rc = wal_సూచీ_Read_Hdr(pWal, &is_మారింది);
      ఒకవేళ( eMode2>SQLITE_CHECKPOINT_PASSIVE ) (శూన్య)walEnableBlocking(pWal);
      ఒకవేళ( is_మారింది && pWal->pDbFd->pMethods->i_సంస్కరణ>=3 ){
        sqlite3OsUnfetch(pWal->pDbFd, 0, 0);
      }
    }

    /* Copy data from the log to the database file. */
    ఒకవేళ( rc==SQLITE_OK ){
      sqlite3FaultSim(660);
      ఒకవేళ( pWal->hdr.mx_చట్రం && walPagesize(pWal)!=nBuf ){
        rc = SQLITE_CORRUPT_BKPT;
      }లేకపోతే ఒకవేళ( eMode2!=SQLITE_CHECKPOINT_NOOP ){
        rc = walCheckpoint(pWal, db, eMode2, xBusy2, pBusyArg, sync_flags,zBuf);
      }

      /* If no error occurred, set the output variables. */
      ఒకవేళ( rc==SQLITE_OK || rc==SQLITE_BUSY ){
        ఒకవేళ( pnLog ) *pnLog = (పూర్ణాంకం)pWal->hdr.mx_చట్రం;
        SEH_INJECT_FAULT;
        ఒకవేళ( pnCkpt ) *pnCkpt = (పూర్ణాంకం)(walCkptInfo(pWal)->nBackfill);
      }
    }
  }
  SEH_EXCEPT( rc = walHandleException(pWal); )

  ఒకవేళ( is_మారింది ){
    /* If a new wal-index header was loaded before the checkpoint was
    ** performed, then the pager-cache associated with pWal is now
    ** out of date. So zero the cached wal-index header to ensure that
    ** next time the pager opens a snapshot on this database it knows that
    ** the cache needs to be reset.
    */
    memset(&pWal->hdr, 0, పరిమాణం(Wal_సూచీ_Hdr));
  }

  walDisableBlocking(pWal);
  sqlite3WalDb(pWal, 0);

  /* Release the locks. */
  (శూన్య)sqlite3_Wal_చివర_Write_Transaction(pWal);
  ఒకవేళ( pWal->ckpt_తాళం ){
    walUnlockExclusive(pWal, WAL_CKPT_తాళం, 1);
    pWal->ckpt_తాళం = 0;
  }
  WALTRACE(("WAL%p: checkpoint %s\n", pWal, rc ? "failed" : "ok"));
#నిర్వచితమైతే SQLITE_ENABLE_SETLK_TIMEOUT
  ఒకవేళ( rc==SQLITE_BUSY_TIMEOUT ) rc = SQLITE_BUSY;
#అయితే_ముగింపు
  తిరిగి (rc==SQLITE_OK && e_బహుళకం!=eMode2 ? SQLITE_BUSY : rc);
}

/* Return the value to pass to a sqlite3_wal_hook callback, the
** number of frames in the WAL at the point of the last commit since
** sqlite3WalCallback() was called.  If no commits have occurred since
** the last call, then return 0.
*/
పూర్ణాంకం sqlite3WalCallback(Wal *pWal){
  u32 ret = 0;
  ఒకవేళ( pWal ){
    ret = pWal->iCallback;
    pWal->iCallback = 0;
  }
  తిరిగి (పూర్ణాంకం)ret;
}

/*
** This function is called to change the WAL subsystem into or out
** of locking_mode=EXCLUSIVE.
**
** If op is zero, then attempt to change from locking_mode=EXCLUSIVE
** into locking_mode=NORMAL.  This means that we must acquire a lock
** on the pWal->readLock byte.  If the WAL is already in locking_mode=NORMAL
** or if the acquisition of the lock fails, then return 0.  If the
** transition out of exclusive-mode is successful, return 1.  This
** operation must occur while the pager is still holding the exclusive
** lock on the main database file.
**
** If op is one, then change from locking_mode=NORMAL into
** locking_mode=EXCLUSIVE.  This means that the pWal->readLock must
** be released.  Return 1 if the transition is made and 0 if the
** WAL is already in exclusive-locking mode - meaning that this
** routine is a no-op.  The pager must already hold the exclusive lock
** on the main database file before invoking this operation.
**
** If op is negative, then do a dry-run of the op==1 case but do
** not actually change anything. The pager uses this to see if it
** should acquire the database exclusive lock prior to invoking
** the op==1 case.
*/
పూర్ణాంకం sqlite3_Wal_Exclusive_బహుళకం(Wal *pWal, పూర్ణాంకం op){
  పూర్ణాంకం rc;
  ప్రతిజ్ఞా( pWal->write_తాళం==0 );
  ప్రతిజ్ఞా( pWal->exclusive_బహుళకం!=WAL_HEAPMEMORY_బహుళకం || op==-1 );

  /* pWal->readLock is usually set, but might be -1 if there was a
  ** prior error while attempting to acquire are read-lock. This cannot
  ** happen if the connection is actually in exclusive mode (as no xShmLock
  ** locks are taken in this case). Nor should the pager attempt to
  ** upgrade to exclusive-mode following such an error.
  */
#నిర్వచితం_కాకపోతే SQLITE_USE_SEH
  ప్రతిజ్ఞా( pWal->read_తాళం>=0 || pWal->తాళం_Error );
#అయితే_ముగింపు
  ప్రతిజ్ఞా( pWal->read_తాళం>=0 || (op<=0 && pWal->exclusive_బహుళకం==0) );

  ఒకవేళ( op==0 ){
    ఒకవేళ( pWal->exclusive_బహుళకం!=WAL_సాధారణ_బహుళకం ){
      pWal->exclusive_బహుళకం = WAL_సాధారణ_బహుళకం;
      ఒకవేళ( wal_తాళం_Shared(pWal, WAL_READ_తాళం(pWal->read_తాళం))!=SQLITE_OK ){
        pWal->exclusive_బహుళకం = WAL_EXCLUSIVE_బహుళకం;
      }
      rc = pWal->exclusive_బహుళకం==WAL_సాధారణ_బహుళకం;
    }లేకపోతే{
      /* Already in locking_mode=NORMAL */
      rc = 0;
    }
  }లేకపోతే ఒకవేళ( op>0 ){
    ప్రతిజ్ఞా( pWal->exclusive_బహుళకం==WAL_సాధారణ_బహుళకం );
    ప్రతిజ్ఞా( pWal->read_తాళం>=0 );
    walUnlockShared(pWal, WAL_READ_తాళం(pWal->read_తాళం));
    pWal->exclusive_బహుళకం = WAL_EXCLUSIVE_బహుళకం;
    rc = 1;
  }లేకపోతే{
    rc = pWal->exclusive_బహుళకం==WAL_సాధారణ_బహుళకం;
  }
  తిరిగి rc;
}

/*
** Return true if the argument is non-NULL and the WAL module is using
** heap-memory for the wal-index. Otherwise, if the argument is NULL or the
** WAL module is using shared-memory, return false.
*/
పూర్ణాంకం sqlite3_Wal_కుప్ప_జ్ఞాపకం(Wal *pWal){
  తిరిగి (pWal && pWal->exclusive_బహుళకం==WAL_HEAPMEMORY_బహుళకం );
}

#నిర్వచితమైతే SQLITE_ENABLE_SNAPSHOT
/* Create a snapshot object.  The content of a snapshot is opaque to
** every other subsystem, so the WAL module can put whatever it needs
** in the object.
*/
పూర్ణాంకం sqlite3WalSnapshotGet(Wal *pWal, sqlite3_snapshot **ppSnapshot){
  పూర్ణాంకం rc = SQLITE_OK;
  Wal_సూచీ_Hdr *pRet;
  స్థిర స్థిరరాశి u32 aZero[4] = { 0, 0, 0, 0 };

  ప్రతిజ్ఞా( pWal->read_తాళం>=0 && pWal->write_తాళం==0 );

  ఒకవేళ( memcmp(&pWal->hdr.a_చట్రం_Cksum[0],aZero,16)==0 ){
    *ppSnapshot = 0;
    తిరిగి SQLITE_ERROR;
  }
  pRet = (Wal_సూచీ_Hdr*)sqlite3_malloc(పరిమాణం(Wal_సూచీ_Hdr));
  ఒకవేళ( pRet==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }లేకపోతే{
    జ్ఞాపకం_నకలు(pRet, &pWal->hdr, పరిమాణం(Wal_సూచీ_Hdr));
    *ppSnapshot = (sqlite3_snapshot*)pRet;
  }

  తిరిగి rc;
}

/* Try to open on pSnapshot when the next read-transaction starts
*/
శూన్య sqlite3WalSnapshotOpen(
  Wal *pWal,
  sqlite3_snapshot *pSnapshot
){
  ఒకవేళ( pSnapshot && ((Wal_సూచీ_Hdr*)pSnapshot)->i_సంస్కరణ==0 ){
    /* iVersion==0 means that this is a call to sqlite3_snapshot_get().  In
    ** this case set the bGetSnapshot flag so that if the call to
    ** sqlite3_snapshot_get() is about to read transaction on this wal 
    ** file, it does not take read-lock 0 if the wal file has been completely
    ** checkpointed. Taking read-lock 0 would work, but then it would be
    ** possible for a subsequent writer to destroy the snapshot even while 
    ** this connection is holding its read-transaction open. This is contrary
    ** to user expectations, so we avoid it by not taking read-lock 0. */
    pWal->bGetSnapshot = 1;
  }లేకపోతే{
    pWal->pSnapshot = (Wal_సూచీ_Hdr*)pSnapshot;
    pWal->bGetSnapshot = 0;
  }
}

/*
** Return a +ve value if snapshot p1 is newer than p2. A -ve value if
** p1 is older than p2 and zero if p1 and p2 are the same snapshot.
*/
పూర్ణాంకం sqlite3_snapshot_cmp(sqlite3_snapshot *p1, sqlite3_snapshot *p2){
  Wal_సూచీ_Hdr *pHdr1 = (Wal_సూచీ_Hdr*)p1;
  Wal_సూచీ_Hdr *pHdr2 = (Wal_సూచీ_Hdr*)p2;

  /* aSalt[0] is a copy of the value stored in the wal file header. It
  ** is incremented each time the wal file is restarted.  */
  ఒకవేళ( pHdr1->aSalt[0]<pHdr2->aSalt[0] ) తిరిగి -1;
  ఒకవేళ( pHdr1->aSalt[0]>pHdr2->aSalt[0] ) తిరిగి +1;
  ఒకవేళ( pHdr1->mx_చట్రం<pHdr2->mx_చట్రం ) తిరిగి -1;
  ఒకవేళ( pHdr1->mx_చట్రం>pHdr2->mx_చట్రం ) తిరిగి +1;
  తిరిగి 0;
}

/*
** The caller currently has a read transaction open on the database.
** This function takes a SHARED lock on the CHECKPOINTER slot and then
** checks if the snapshot passed as the second argument is still
** available. If so, SQLITE_OK is returned.
**
** If the snapshot is not available, SQLITE_ERROR is returned. Or, if
** the CHECKPOINTER lock cannot be obtained, SQLITE_BUSY. If any error
** occurs (any value other than SQLITE_OK is returned), the CHECKPOINTER
** lock is released before returning.
*/
పూర్ణాంకం sqlite3WalSnapshotCheck(Wal *pWal, sqlite3_snapshot *pSnapshot){
  పూర్ణాంకం rc;
  SEH_TRY {
    rc = wal_తాళం_Shared(pWal, WAL_CKPT_తాళం);
    ఒకవేళ( rc==SQLITE_OK ){
      Wal_సూచీ_Hdr *pNew = (Wal_సూచీ_Hdr*)pSnapshot;
      ఒకవేళ( memcmp(pNew->aSalt, pWal->hdr.aSalt, పరిమాణం(pWal->hdr.aSalt))
       || pNew->mx_చట్రం<walCkptInfo(pWal)->nBackfillAttempted
      ){
        rc = SQLITE_ERROR_SNAPSHOT;
        walUnlockShared(pWal, WAL_CKPT_తాళం);
      }
    }
  }
  SEH_EXCEPT( rc = walHandleException(pWal); )
  తిరిగి rc;
}

/*
** Release a lock obtained by an earlier successful call to
** sqlite3WalSnapshotCheck().
*/
శూన్య sqlite3WalSnapshotUnlock(Wal *pWal){
  ప్రతిజ్ఞా( pWal );
  walUnlockShared(pWal, WAL_CKPT_తాళం);
}


#అయితే_ముగింపు /* SQLITE_ENABLE_SNAPSHOT */

#నిర్వచితమైతే SQLITE_ENABLE_ZIPVFS
/*
** If the argument is not NULL, it points to a Wal object that holds a
** read-lock. This function returns the database page-size if it is known,
** or zero if it is not (or if pWal is NULL).
*/
పూర్ణాంకం sqlite3WalFramesize(Wal *pWal){
  ప్రతిజ్ఞా( pWal==0 || pWal->read_తాళం>=0 );
  తిరిగి (pWal ? pWal->sz_పుట : 0);
}
#అయితే_ముగింపు

/* Return the sqlite3_file object for the WAL file
*/
sqlite3_file *sqlite3WalFile(Wal *pWal){
  తిరిగి pWal->pWalFd;
}

#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Helper function to sqlite3WalStat().
**
** Append JSON-5 structure elements onto describing the current state
** of pWal onto the end of pStr.  This function is for debugging and
** analysis only and is not included in production builds.
*/
స్థిర శూన్య wal_సూచీ_Hdr_Stat(అస్థిర Wal_సూచీ_Hdr *pHdr, sqlite3_str *pStr){
  sqlite3_str_appendf(pStr, "iVersion:%u",pHdr->i_సంస్కరణ);
  sqlite3_str_appendf(pStr, ",iChange:%u",pHdr->iChange);
  sqlite3_str_appendf(pStr, ",isInit:%u",pHdr->isInit);
  sqlite3_str_appendf(pStr, ",bigEndCksum:%u",pHdr->big_చివర_Cksum);
  sqlite3_str_appendf(pStr, ",szPage:%u",pHdr->sz_పుట);
  sqlite3_str_appendf(pStr, ",mxFrame:%u",pHdr->mx_చట్రం);
  sqlite3_str_appendf(pStr, ",nPage:%u",pHdr->n_పుట);
#ఒకవేళ 0  /* Not often needed.  Declutter. */
  sqlite3_str_appendf(pStr, ",aFrameCksum:[0x%08x,0x%08x]",
                      pHdr->a_చట్రం_Cksum[0], pHdr->a_చట్రం_Cksum[1]);
  sqlite3_str_appendf(pStr, ",aSalt:[0x%08x,0x%08x]",
                      pHdr->aSalt[0], pHdr->aSalt[1]);
  sqlite3_str_appendf(pStr, ",aCksum:[0x%08x,0x%08x]",
                      pHdr->aCksum[0], pHdr->aCksum[1]);
#అయితే_ముగింపు
}
#అయితే_ముగింపు /* SQLITE_DEBUG || SQLITE_ENABLE_WALSTAT */


#ఒకవేళ defined(SQLITE_DEBUG) || defined(SQLITE_ENABLE_WALSTAT)
/*
** Append JSON-5 structure elements onto describing the current state
** of pWal onto the end of pStr.  This function is for debugging and
** analysis only and is not included in production builds.
*/
శూన్య sqlite3WalStat(Wal *pWal, sqlite3_str *pStr){
  SEH_TRY {
    పూర్ణాంకం i;
    అస్థిర WalCkptInfo *pInfo = walCkptInfo(pWal);
    అస్థిర Wal_సూచీ_Hdr *pHdr = wal_సూచీ_Hdr(pWal);
    sqlite3_str_appendf(pStr, "hdr0:{");
    wal_సూచీ_Hdr_Stat(pHdr,pStr);
    sqlite3_str_appendf(pStr, "},ckpt:{");
    sqlite3_str_appendf(pStr, "nBackfill:%u",pInfo->nBackfill);
    sqlite3_str_appendf(pStr, ",aReadMark:[%u", pInfo->aReadMark[0]);
    కొరకు(i=1; i<WAL_NREADER; i++){
      sqlite3_str_appendf(pStr, ",%u", pInfo->aReadMark[i]);
    }
    sqlite3_str_appendf(pStr, "],nBackfillAttempted:%u}", 
                        pInfo->nBackfillAttempted);

    sqlite3_str_appendf(pStr, ",nWiData:%d", pWal->n_Wi_దత్తాంశం);
    sqlite3_str_appendf(pStr, ",readLock:%d", pWal->read_తాళం);
    sqlite3_str_appendf(pStr, ",minFrame:%u", pWal->min_చట్రం);
  }
  SEH_EXCEPT( ; )
}
#అయితే_ముగింపు /* SQLITE_DEBUG || SQLITE_ENABLE_WALSTAT */

#అయితే_ముగింపు /* #ifndef SQLITE_OMIT_WAL */
