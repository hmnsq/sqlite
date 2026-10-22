/*
** 2011-07-09
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains code for the VdbeSorter object, used in concert with
** a VdbeCursor to sort large numbers of keys for CREATE INDEX statements
** or by SELECT statements with ORDER BY clauses that cannot be satisfied
** using indexes and without LIMIT clauses.
**
** The VdbeSorter object implements a multi-threaded external merge sort
** algorithm that is efficient even if the number of elements being sorted
** exceeds the available memory.
**
** Here is the (internal, non-API) interface between this module and the
** rest of the SQLite system:
**
**    sqlite3VdbeSorterInit()       Create a new VdbeSorter object.
**
**    sqlite3VdbeSorterWrite()      Add a single new row to the VdbeSorter
**                                  object.  The row is a binary blob in the
**                                  OP_MakeRecord format that contains both
**                                  the ORDER BY key columns and result columns
**                                  in the case of a SELECT w/ ORDER BY, or
**                                  the complete record for an index entry
**                                  in the case of a CREATE INDEX.
**
**    sqlite3VdbeSorterRewind()     Sort all content previously added.
**                                  Position the read cursor on the
**                                  first sorted element.
**
**    sqlite3VdbeSorterNext()       Advance the read cursor to the next sorted
**                                  element.
**
**    sqlite3VdbeSorterRowkey()     Return the complete binary blob for the
**                                  row currently under the read cursor.
**
**    sqlite3VdbeSorterCompare()    Compare the binary blob for the row
**                                  currently under the read cursor against
**                                  another binary blob X and report if
**                                  X is strictly less than the read cursor.
**                                  Used to enforce uniqueness in a
**                                  CREATE UNIQUE INDEX statement.
**
**    sqlite3VdbeSorterClose()      Close the VdbeSorter object and reclaim
**                                  all resources.
**
**    sqlite3VdbeSorterReset()      Refurbish the VdbeSorter for reuse.  This
**                                  is like Close() followed by Init() only
**                                  much faster.
**
** The interfaces above must be called in a particular order.  Write() can
** only occur in between Init()/Reset() and Rewind().  Next(), Rowkey(), and
** Compare() can only occur in between Rewind() and Close()/Reset(). i.e.
**
**   Init()
**   for each record: Write()
**   Rewind()
**     Rowkey()/Compare()
**   Next()
**   Close()
**
** Algorithm:
**
** Records passed to the sorter via calls to Write() are initially held
** unsorted in main memory. Assuming the amount of memory used never exceeds
** a threshold, when Rewind() is called the set of records is sorted using
** an in-memory merge sort. In this case, no temporary files are required
** and subsequent calls to Rowkey(), Next() and Compare() read records
** directly from main memory.
**
** If the amount of space used to store records in main memory exceeds the
** threshold, then the set of records currently in memory are sorted and
** written to a temporary file in "Packed Memory Array" (PMA) format.
** A PMA created at this point is known as a "level-0 PMA". Higher levels
** of PMAs may be created by merging existing PMAs together - for example
** merging two or more level-0 PMAs together creates a level-1 PMA.
**
** The threshold for the amount of main memory to use before flushing
** records to a PMA is roughly the same as the limit configured for the
** page-cache of the main database. Specifically, the threshold is set to
** the value returned by "PRAGMA main.page_size" multiplied by
** that returned by "PRAGMA main.cache_size", in bytes.
**
** If the sorter is running in single-threaded mode, then all PMAs generated
** are appended to a single temporary file. Or, if the sorter is running in
** multi-threaded mode then up to (N+1) temporary files may be opened, where
** N is the configured number of worker threads. In this case, instead of
** sorting the records and writing the PMA to a temporary file itself, the
** calling thread usually launches a worker thread to do so. Except, if
** there are already N worker threads running, the main thread does the work
** itself.
**
** The sorter is running in multi-threaded mode if (a) the library was built
** with pre-processor symbol SQLITE_MAX_WORKER_THREADS set to a value greater
** than zero, and (b) worker threads have been enabled at runtime by calling
** "PRAGMA threads=N" with some value of N greater than 0.
**
** When Rewind() is called, any data remaining in memory is flushed to a
** final PMA. So at this point the data is stored in some number of sorted
** PMAs within temporary files on disk.
**
** If there are fewer than SORTER_MAX_MERGE_COUNT PMAs in total and the
** sorter is running in single-threaded mode, then these PMAs are merged
** incrementally as keys are retrieved from the sorter by the VDBE.  The
** MergeEngine object, described in further detail below, performs this
** merge.
**
** Or, if running in multi-threaded mode, then a background thread is
** launched to merge the existing PMAs. Once the background thread has
** merged T bytes of data into a single sorted PMA, the main thread
** begins reading keys from that PMA while the background thread proceeds
** with merging the next T bytes of data. And so on.
**
** Parameter T is set to half the value of the memory threshold used
** by Write() above to determine when to create a new PMA.
**
** If there are more than SORTER_MAX_MERGE_COUNT PMAs in total when
** Rewind() is called, then a hierarchy of incremental-merges is used.
** First, T bytes of data from the first SORTER_MAX_MERGE_COUNT PMAs on
** disk are merged together. Then T bytes of data from the second set, and
** so on, such that no operation ever merges more than SORTER_MAX_MERGE_COUNT
** PMAs at a time. This done is to improve locality.
**
** If running in multi-threaded mode and there are more than
** SORTER_MAX_MERGE_COUNT PMAs on disk when Rewind() is called, then more
** than one background thread may be created. Specifically, there may be
** one background thread for each temporary file on disk, and one background
** thread to merge the output of each of the others to a single PMA for
** the main thread to read from.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "vdbeInt.h"

/*
** If SQLITE_DEBUG_SORTER_THREADS is defined, this module outputs various
** messages to stderr that may be helpful in understanding the performance
** characteristics of the sorter in multi-threaded mode.
*/
#ਜੇ 0
# define SQLITE_DEBUG_SORTER_ਧਾਗੇ 1
#ਜੇ_ਅੰਤ

/*
** Hard-coded maximum amount of data to accumulate in memory before flushing
** to a level 0 PMA. The purpose of this limit is to prevent various integer
** overflows. 512MiB.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SQLITE_MAX_PMASZ    (1<<29)

/*
** Private objects used by the sorter
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਮਿਲਾਓ_Engine ਮਿਲਾਓ_Engine;     /* Merge PMAs together */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ PmaReader PmaReader;         /* Incrementally read one PMA */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ PmaWriter PmaWriter;         /* Incrementally write one PMA */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Sorter_ਰਿਕਾਰਡ Sorter_ਰਿਕਾਰਡ;   /* A record being sorted */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਛਾਂਟੋ_Subtask ਛਾਂਟੋ_Subtask;     /* A sub-task in the sort process */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ SorterFile SorterFile;       /* Temporary file object wrapper */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ SorterList SorterList;       /* In-memory list of records */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ IncrMerger IncrMerger;       /* Read & merge multiple PMAs */

/*
** A container for a temp file handle and the current amount of data
** stored in the file.
*/
ਬਣਤਰ SorterFile {
  sqlite3_file *pFd;              /* File handle */
  i64 iEof;                       /* Bytes of data stored in pFd */
};

/*
** An in-memory list of objects to be sorted.
**
** If aMemory==0 then each object is allocated separately and the objects
** are connected using SorterRecord.u.pNext.  If aMemory!=0 then all objects
** are stored in the aMemory[] bulk memory, one right after the other, and
** are connected using SorterRecord.u.iNext.
*/
ਬਣਤਰ SorterList {
  Sorter_ਰਿਕਾਰਡ *pList;            /* Linked list of records */
  u8 *a_ਸਮ੍ਰਿਤੀ;                    /* If non-NULL, bulk memory to hold pList */
  i64 szPMA;                      /* Size of pList as PMA in bytes */
};

/*
** The MergeEngine object is used to combine two or more smaller PMAs into
** one big PMA using a merge operation.  Separate PMAs all need to be
** combined into one big PMA in order to be able to step through the sorted
** records in order.
**
** The aReadr[] array contains a PmaReader object for each of the PMAs being
** merged.  An aReadr[] object either points to a valid key or else is at EOF.
** ("EOF" means "End Of File".  When aReadr[] is at EOF there is no more data.)
** For the purposes of the paragraphs below, we assume that the array is
** actually N elements in size, where N is the smallest power of 2 greater
** to or equal to the number of PMAs being merged. The extra aReadr[] elements
** are treated as if they are empty (always at EOF).
**
** The aTree[] array is also N elements in size. The value of N is stored in
** the MergeEngine.nTree variable.
**
** The final (N/2) elements of aTree[] contain the results of comparing
** pairs of PMA keys together. Element i contains the result of
** comparing aReadr[2*i-N] and aReadr[2*i-N+1]. Whichever key is smaller, the
** aTree element is set to the index of it.
**
** For the purposes of this comparison, EOF is considered greater than any
** other key value. If the keys are equal (only possible with two EOF
** values), it doesn't matter which index is stored.
**
** The (N/4) elements of aTree[] that precede the final (N/2) described
** above contains the index of the smallest of each block of 4 PmaReaders
** And so on. So that aTree[1] contains the index of the PmaReader that
** currently points to the smallest key value. aTree[0] is unused.
**
** Example:
**
**     aReadr[0] -> Banana
**     aReadr[1] -> Feijoa
**     aReadr[2] -> Elderberry
**     aReadr[3] -> Currant
**     aReadr[4] -> Grapefruit
**     aReadr[5] -> Apple
**     aReadr[6] -> Durian
**     aReadr[7] -> EOF
**
**     aTree[] = { X, 5   0, 5    0, 3, 5, 6 }
**
** The current element is "Apple" (the value of the key indicated by
** PmaReader 5). When the Next() operation is invoked, PmaReader 5 will
** be advanced to the next key in its segment. Say the next key is
** "Eggplant":
**
**     aReadr[5] -> Eggplant
**
** The contents of aTree[] are updated first by comparing the new PmaReader
** 5 key to the current key of PmaReader 4 (still "Grapefruit"). The PmaReader
** 5 value is still smaller, so aTree[6] is set to 5. And so on up the tree.
** The value of PmaReader 6 - "Durian" - is now smaller than that of PmaReader
** 5, so aTree[3] is set to 6. Key 0 is smaller than key 6 (Banana<Durian),
** so the value written into element 1 of the array is 0. As follows:
**
**     aTree[] = { X, 0   0, 6    0, 3, 5, 6 }
**
** In other words, each time we advance to the next sorter element, log2(N)
** key comparison operations are required, where N is the number of segments
** being merged (rounded up to the next power of 2).
*/
ਬਣਤਰ ਮਿਲਾਓ_Engine {
  ਪੂਰਨ n_ਰੁੱਖ;                 /* Used size of aTree/aReadr (power of 2) */
  ਛਾਂਟੋ_Subtask *pTask;        /* Used by this thread only */
  ਪੂਰਨ *a_ਰੁੱਖ;                /* Current state of incremental merge */
  PmaReader *aReadr;         /* Array of PmaReaders to merge data from */
};

/*
** This object represents a single thread of control in a sort operation.
** Exactly VdbeSorter.nTask instances of this object are allocated
** as part of each VdbeSorter object. Instances are never allocated any
** other way. VdbeSorter.nTask is set to the number of worker threads allowed
** (see SQLITE_CONFIG_WORKER_THREADS) plus one (the main thread).  Thus for
** single-threaded operation, there is exactly one instance of this object
** and for multi-threaded operation there are two or more instances.
**
** Essentially, this structure contains all those fields of the VdbeSorter
** structure for which each thread requires a separate instance. For example,
** each thread requeries its own UnpackedRecord object to unpack records in
** as part of comparison operations.
**
** Before a background thread is launched, variable bDone is set to 0. Then,
** right before it exits, the thread itself sets bDone to 1. This is used for
** two purposes:
**
**   1. When flushing the contents of memory to a level-0 PMA on disk, to
**      attempt to select a SortSubtask for which there is not already an
**      active background thread (since doing so causes the main thread
**      to block until it finishes).
**
**   2. If SQLITE_DEBUG_SORTER_THREADS is defined, to determine if a call
**      to sqlite3ThreadJoin() is likely to block. Cases that are likely to
**      block provoke debugging output.
**
** In both cases, the effects of the main thread seeing (bDone==0) even
** after the thread has finished are not dire. So we don't worry about
** memory barriers and such here.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ (*SorterCompare)(ਛਾਂਟੋ_Subtask*,ਪੂਰਨ*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ);
ਬਣਤਰ ਛਾਂਟੋ_Subtask {
  SQLiteThread *pThread;          /* Background thread, if any */
  ਪੂਰਨ b_ਪੂਰਾ;                      /* Set if thread is finished but not joined */
  ਪੂਰਨ nPMA;                       /* Number of PMAs currently in file */
  VdbeSorter *pSorter;            /* Sorter that owns this sub-task */
  Unpacked_ਰਿਕਾਰਡ *pUnpacked;      /* Space to unpack a record */
  SorterList list;                /* List for thread to write to a PMA */
  SorterCompare xCompare;         /* Compare function to use */
  SorterFile file;                /* Temp file for level-0 PMAs */
  SorterFile file2;               /* Space for other PMAs */
  u64 nSpill;                     /* Total bytes written by this task */
};


/*
** Main sorter structure. A single instance of this is allocated for each
** sorter cursor created by the VDBE.
**
** mxKeysize:
**   As records are added to the sorter by calls to sqlite3VdbeSorterWrite(),
**   this variable is updated so as to be set to the size on disk of the
**   largest record in the sorter.
*/
ਬਣਤਰ VdbeSorter {
  ਪੂਰਨ mn_Pma_ਮਾਪ;                  /* Minimum PMA size, in bytes */
  ਪੂਰਨ mx_Pma_ਮਾਪ;                  /* Maximum PMA size, in bytes.  0==no limit */
  ਪੂਰਨ mxKeysize;                  /* Largest serialized key seen so far */
  ਪੂਰਨ pgsz;                       /* Main database page size */
  PmaReader *pReader;             /* Readr data from here after Rewind() */
  ਮਿਲਾਓ_Engine *pMerger;           /* Or here, if bUseThreads==0 */
  sqlite3 *db;                    /* Database connection */
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info;              /* How to compare records */
  Unpacked_ਰਿਕਾਰਡ *pUnpacked;      /* Used by VdbeSorterCompare() */
  SorterList list;                /* List of in-memory records */
  ਪੂਰਨ i_ਸਮ੍ਰਿਤੀ;                    /* Offset of free space in list.aMemory */
  ਪੂਰਨ n_ਸਮ੍ਰਿਤੀ;                    /* Size of list.aMemory allocation in bytes */
  u8 bUsePMA;                     /* True if one or more PMAs created */
  u8 b_Use_ਧਾਗੇ;                 /* True to use background threads */
  u8 iPrev;                       /* Previous thread used to flush PMA */
  u8 nTask;                       /* Size of aTask[] array */
  u8 type_ਨਕਾਬ;
  ਛਾਂਟੋ_Subtask aTask[FLEXARRAY];   /* One or more subtasks */
};

/* Size (in bytes) of a VdbeSorter object that works with N or fewer subtasks */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SZ_VDBESORTER(N)  (offsetof(VdbeSorter,aTask)+(N)*ਆਕਾਰ(ਛਾਂਟੋ_Subtask))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SORTER_TYPE_INTEGER 0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SORTER_TYPE_ਪਾਠ    0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SORTER_TYPE_REAL    0x04

/*
** An instance of the following object is used to read records out of a
** PMA, in sorted order.  The next key to be read is cached in nKey/aKey.
** aKey might point into aMap or into aBuffer.  If neither of those locations
** contain a contiguous representation of the key, then aAlloc is allocated
** and the key is copied into aAlloc and aKey is made to point to aAlloc.
**
** pFd==0 at EOF.
*/
ਬਣਤਰ PmaReader {
  i64 iReadOff;               /* Current read offset */
  i64 iEof;                   /* 1 byte past EOF for this PmaReader */
  ਪੂਰਨ nAlloc;                 /* Bytes of space at aAlloc */
  ਪੂਰਨ n_ਕੁੰਜੀ;                   /* Number of bytes in key */
  sqlite3_file *pFd;          /* File handle we are reading from */
  u8 *aAlloc;                 /* Space for aKey if aBuffer and pMap wont work */
  u8 *a_ਕੁੰਜੀ;                   /* Pointer to current key */
  u8 *a_ਬਫਰ;                /* Current read buffer */
  ਪੂਰਨ n_ਬਫਰ;                /* Size of read buffer in bytes */
  u8 *aMap;                   /* Pointer to mapping of entire file */
  IncrMerger *pIncr;          /* Incremental merger */
};

/*
** Normally, a PmaReader object iterates through an existing PMA stored
** within a temp file. However, if the PmaReader.pIncr variable points to
** an object of the following type, it may be used to iterate/merge through
** multiple PMAs simultaneously.
**
** There are two types of IncrMerger object - single (bUseThread==0) and
** multi-threaded (bUseThread==1).
**
** A multi-threaded IncrMerger object uses two temporary files - aFile[0]
** and aFile[1]. Neither file is allowed to grow to more than mxSz bytes in
** size. When the IncrMerger is initialized, it reads enough data from
** pMerger to populate aFile[0]. It then sets variables within the
** corresponding PmaReader object to read from that file and kicks off
** a background thread to populate aFile[1] with the next mxSz bytes of
** sorted record data from pMerger.
**
** When the PmaReader reaches the end of aFile[0], it blocks until the
** background thread has finished populating aFile[1]. It then exchanges
** the contents of the aFile[0] and aFile[1] variables within this structure,
** sets the PmaReader fields to read from the new aFile[0] and kicks off
** another background thread to populate the new aFile[1]. And so on, until
** the contents of pMerger are exhausted.
**
** A single-threaded IncrMerger does not open any temporary files of its
** own. Instead, it has exclusive access to mxSz bytes of space beginning
** at offset iStartOff of file pTask->file2. And instead of using a
** background thread to prepare data for the PmaReader, with a single
** threaded IncrMerger the allocate part of pTask->file2 is "refilled" with
** keys from pMerger by the calling thread whenever the PmaReader runs out
** of data.
*/
ਬਣਤਰ IncrMerger {
  ਛਾਂਟੋ_Subtask *pTask;             /* Task that owns this merger */
  ਮਿਲਾਓ_Engine *pMerger;           /* Merge engine thread reads data from */
  i64 i_ਸ਼ੁਰੂ_Off;                  /* Offset to start writing file at */
  ਪੂਰਨ mxSz;                       /* Maximum bytes of data to store */
  ਪੂਰਨ bEof;                       /* Set to true when merge is finished */
  ਪੂਰਨ bUseThread;                 /* True to use a bg thread for this object */
  SorterFile aFile[2];            /* aFile[0] for reading, [1] for writing */
};

/*
** An instance of this object is used for writing a PMA.
**
** The PMA is written one record at a time.  Each record is of an arbitrary
** size.  But I/O is more efficient if it occurs in page-sized blocks where
** each block is aligned on a page boundary.  This object caches writes to
** the PMA so that aligned, page-size blocks are written.
*/
ਬਣਤਰ PmaWriter {
  ਪੂਰਨ eFWErr;                     /* Non-zero if in an error state */
  u8 *a_ਬਫਰ;                    /* Pointer to write buffer */
  ਪੂਰਨ n_ਬਫਰ;                    /* Size of write buffer in bytes */
  ਪੂਰਨ i_Buf_ਸ਼ੁਰੂ;                  /* First byte of buffer to write */
  ਪੂਰਨ i_Buf_ਸਿਰਾ;                    /* Last byte of buffer to write */
  i64 iWriteOff;                  /* Offset of start of buffer in file */
  sqlite3_file *pFd;              /* File handle to write to */
  u64 nPmaSpill;                  /* Total number of bytes written */
};

/*
** This object is the header on a single record while that record is being
** held in memory and prior to being written out as part of a PMA.
**
** How the linked list is connected depends on how memory is being managed
** by this module. If using a separate allocation for each in-memory record
** (VdbeSorter.list.aMemory==0), then the list is always connected using the
** SorterRecord.u.pNext pointers.
**
** Or, if using the single large allocation method (VdbeSorter.list.aMemory!=0),
** then while records are being accumulated the list is linked using the
** SorterRecord.u.iNext offset. This is because the aMemory[] array may
** be sqlite3Realloc()ed while records are being accumulated. Once the VM
** has finished passing records to the sorter, or when the in-memory buffer
** is full, the list is sorted. As part of the sorting process, it is
** converted to use the SorterRecord.u.pNext pointers. See function
** vdbeSorterSort() for details.
*/
ਬਣਤਰ Sorter_ਰਿਕਾਰਡ {
  ਪੂਰਨ nVal;                       /* Size of the record in bytes */
  ਸੰਘ {
    Sorter_ਰਿਕਾਰਡ *p_ਅਗਲਾ;          /* Pointer to next record in list */
    ਪੂਰਨ i_ਅਗਲਾ;                    /* Offset within aMemory of next record */
  } u;
  /* The data for the record immediately follows this header */
};

/* Return a pointer to the buffer containing the record data for SorterRecord
** object p. Should be used as if:
**
**   void *SRVAL(SorterRecord *p) { return (void*)&p[1]; }
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SRVAL(p) ((ਖਾਲੀ*)((Sorter_ਰਿਕਾਰਡ*)(p) + 1))


/* Maximum number of PMAs that a single MergeEngine can merge */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ 16

ਸਥਿਰ ਪੂਰਨ vdbe_Incr_ਅਦਲਾ_ਬਦਲੀ(IncrMerger*);
ਸਥਿਰ ਖਾਲੀ vdbeIncrFree(IncrMerger *);

/*
** Free all memory belonging to the PmaReader object passed as the
** argument. All structure fields are set to zero before returning.
*/
ਸਥਿਰ ਖਾਲੀ vdbePmaReaderClear(PmaReader *pReadr){
  sqlite3_free(pReadr->aAlloc);
  sqlite3_free(pReadr->a_ਬਫਰ);
  ਜੇ( pReadr->aMap ) sqlite3OsUnfetch(pReadr->pFd, 0, pReadr->aMap);
  vdbeIncrFree(pReadr->pIncr);
  memset(pReadr, 0, ਆਕਾਰ(PmaReader));
}

/*
** Read the next nByte bytes of data from the PMA p.
** If successful, set *ppOut to point to a buffer containing the data
** and return SQLITE_OK. Otherwise, if an error occurs, return an SQLite
** error code.
**
** The buffer returned in *ppOut is only valid until the
** next call to this function.
*/
ਸਥਿਰ ਪੂਰਨ vdbePmaReadBlob(
  PmaReader *p,                   /* PmaReader from which to take the blob */
  ਪੂਰਨ nByte,                      /* Bytes of data to read */
  u8 **pp_ਬਾਹਰਲਾ                      /* OUT: Pointer to buffer containing data */
){
  ਪੂਰਨ iBuf;                       /* Offset within buffer to read from */
  ਪੂਰਨ nAvail;                     /* Bytes of data available in buffer */

  ਜੇ( p->aMap ){
    *pp_ਬਾਹਰਲਾ = &p->aMap[p->iReadOff];
    p->iReadOff += nByte;
    ਵਾਪਸ SQLITE_OK;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( p->a_ਬਫਰ );

  /* If there is no more data to be read from the buffer, read the next
  ** p->nBuffer bytes of data from the file into it. Or, if there are less
  ** than p->nBuffer bytes remaining in the PMA, read all remaining data.  */
  iBuf = p->iReadOff % p->n_ਬਫਰ;
  ਜੇ( iBuf==0 ){
    ਪੂਰਨ nRead;                    /* Bytes to read from disk */
    ਪੂਰਨ rc;                       /* sqlite3OsRead() return code */

    /* Determine how many bytes of data to read. */
    ਜੇ( (p->iEof - p->iReadOff) > (i64)p->n_ਬਫਰ ){
      nRead = p->n_ਬਫਰ;
    }ਨਹੀਂਤਾਂ{
      nRead = (ਪੂਰਨ)(p->iEof - p->iReadOff);
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( nRead>0 );

    /* Readr data from the file. Return early if an error occurs. */
    rc = sqlite3OsRead(p->pFd, p->a_ਬਫਰ, nRead, p->iReadOff);
    ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_IOERR_SHORT_READ );
    ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
  }
  nAvail = p->n_ਬਫਰ - iBuf;

  ਜੇ( nByte<=nAvail ){
    /* The requested data is available in the in-memory buffer. In this
    ** case there is no need to make a copy of the data, just return a
    ** pointer into the buffer to the caller.  */
    *pp_ਬਾਹਰਲਾ = &p->a_ਬਫਰ[iBuf];
    p->iReadOff += nByte;
  }ਨਹੀਂਤਾਂ{
    /* The requested data is not all available in the in-memory buffer.
    ** In this case, allocate space at p->aAlloc[] to copy the requested
    ** range into. Then return a copy of pointer p->aAlloc to the caller.  */
    ਪੂਰਨ nRem;                     /* Bytes remaining to copy */

    /* Extend the p->aAlloc[] allocation if required. */
    ਜੇ( p->nAlloc<nByte ){
      u8 *aNew;
      sqlite3_int64 nNew = MAX(128, 2*(sqlite3_int64)p->nAlloc);
      ਜਦੋਂ( nByte>nNew ) nNew = nNew*2;
      aNew = sqlite3Realloc(p->aAlloc, nNew);
      ਜੇ( !aNew ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
      p->nAlloc = nNew;
      p->aAlloc = aNew;
    }

    /* Copy as much data as is available in the buffer into the start of
    ** p->aAlloc[].  */
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(p->aAlloc, &p->a_ਬਫਰ[iBuf], nAvail);
    p->iReadOff += nAvail;
    nRem = nByte - nAvail;

    /* The following loop copies up to p->nBuffer bytes per iteration into
    ** the p->aAlloc[] buffer.  */
    ਜਦੋਂ( nRem>0 ){
      ਪੂਰਨ rc;                     /* vdbePmaReadBlob() return code */
      ਪੂਰਨ nCopy;                  /* Number of bytes to copy */
      u8 *a_ਅਗਲਾ = 0;              /* Pointer to buffer to copy data from */

      nCopy = nRem;
      ਜੇ( nRem>p->n_ਬਫਰ ) nCopy = p->n_ਬਫਰ;
      rc = vdbePmaReadBlob(p, nCopy, &a_ਅਗਲਾ);
      ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
      ਪ੍ਰਤਿਜ੍ਞਾ( a_ਅਗਲਾ!=p->aAlloc );
      ਪ੍ਰਤਿਜ੍ਞਾ( a_ਅਗਲਾ!=0 );
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p->aAlloc[nByte - nRem], a_ਅਗਲਾ, nCopy);
      nRem -= nCopy;
    }

    *pp_ਬਾਹਰਲਾ = p->aAlloc;
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** Read a varint from the stream of data accessed by p. Set *pnOut to
** the value read.
*/
ਸਥਿਰ ਪੂਰਨ vdbePmaReadVarint(PmaReader *p, u64 *pn_ਬਾਹਰਲਾ){
  ਪੂਰਨ iBuf;

  ਜੇ( p->aMap ){
    p->iReadOff += sqlite3GetVarint(&p->aMap[p->iReadOff], pn_ਬਾਹਰਲਾ);
  }ਨਹੀਂਤਾਂ{
    iBuf = p->iReadOff % p->n_ਬਫਰ;
    ਜੇ( iBuf && (p->n_ਬਫਰ-iBuf)>=9 ){
      p->iReadOff += sqlite3GetVarint(&p->a_ਬਫਰ[iBuf], pn_ਬਾਹਰਲਾ);
    }ਨਹੀਂਤਾਂ{
      u8 aVarint[16], *a;
      ਪੂਰਨ i = 0, rc;
      ਕਰੋ{
        rc = vdbePmaReadBlob(p, 1, &a);
        ਜੇ( rc ) ਵਾਪਸ rc;
        aVarint[(i++)&0xf] = a[0];
      }ਜਦੋਂ( (a[0]&0x80)!=0 );
      sqlite3GetVarint(aVarint, pn_ਬਾਹਰਲਾ);
    }
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** Attempt to memory map file pFile. If successful, set *pp to point to the
** new mapping and return SQLITE_OK. If the mapping is not attempted
** (because the file is too large or the VFS layer is configured not to use
** mmap), return SQLITE_OK and set *pp to NULL.
**
** Or, if an error occurs, return an SQLite error code. The final value of
** *pp is undefined in this case.
*/
ਸਥਿਰ ਪੂਰਨ vdbeSorterMapFile(ਛਾਂਟੋ_Subtask *pTask, SorterFile *pFile, u8 **pp){
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( pFile->iEof<=(i64)(pTask->pSorter->db->nMaxSorterMmap) ){
    sqlite3_file *pFd = pFile->pFd;
    ਜੇ( pFd->pMethods->i_ਸੰਸਕਰਣ>=3 ){
      rc = sqlite3OsFetch(pFd, 0, (ਪੂਰਨ)pFile->iEof, (ਖਾਲੀ**)pp);
      testcase( rc!=SQLITE_OK );
    }
  }
  ਵਾਪਸ rc;
}

/*
** Attach PmaReader pReadr to file pFile (if it is not already attached to
** that file) and seek it to offset iOff within the file.  Return SQLITE_OK
** if successful, or an SQLite error code if an error occurs.
*/
ਸਥਿਰ ਪੂਰਨ vdbePmaReaderSeek(
  ਛਾਂਟੋ_Subtask *pTask,             /* Task context */
  PmaReader *pReadr,              /* Reader whose cursor is to be moved */
  SorterFile *pFile,              /* Sorter file to read from */
  i64 iOff                        /* Offset in pFile */
){
  ਪੂਰਨ rc = SQLITE_OK;

  ਪ੍ਰਤਿਜ੍ਞਾ( pReadr->pIncr==0 || pReadr->pIncr->bEof==0 );

  ਜੇ( sqlite3FaultSim(201) ) ਵਾਪਸ SQLITE_IOERR_READ;
  ਜੇ( pReadr->aMap ){
    sqlite3OsUnfetch(pReadr->pFd, 0, pReadr->aMap);
    pReadr->aMap = 0;
  }
  pReadr->iReadOff = iOff;
  pReadr->iEof = pFile->iEof;
  pReadr->pFd = pFile->pFd;

  rc = vdbeSorterMapFile(pTask, pFile, &pReadr->aMap);
  ਜੇ( rc==SQLITE_OK && pReadr->aMap==0 ){
    ਪੂਰਨ pgsz = pTask->pSorter->pgsz;
    ਪੂਰਨ iBuf = pReadr->iReadOff % pgsz;
    ਜੇ( pReadr->a_ਬਫਰ==0 ){
      pReadr->a_ਬਫਰ = (u8*)sqlite3Malloc(pgsz);
      ਜੇ( pReadr->a_ਬਫਰ==0 ) rc = SQLITE_NOMEM_BKPT;
      pReadr->n_ਬਫਰ = pgsz;
    }
    ਜੇ( rc==SQLITE_OK && iBuf ){
      ਪੂਰਨ nRead = pgsz - iBuf;
      ਜੇ( (pReadr->iReadOff + nRead) > pReadr->iEof ){
        nRead = (ਪੂਰਨ)(pReadr->iEof - pReadr->iReadOff);
      }
      rc = sqlite3OsRead(
          pReadr->pFd, &pReadr->a_ਬਫਰ[iBuf], nRead, pReadr->iReadOff
      );
      testcase( rc!=SQLITE_OK );
    }
  }

  ਵਾਪਸ rc;
}

/*
** Advance PmaReader pReadr to the next key in its PMA. Return SQLITE_OK if
** no error occurs, or an SQLite error code if one does.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Pma_Reader_ਅਗਲਾ(PmaReader *pReadr){
  ਪੂਰਨ rc = SQLITE_OK;             /* Return Code */
  u64 nRec = 0;                   /* Size of record in bytes */


  ਜੇ( pReadr->iReadOff>=pReadr->iEof ){
    IncrMerger *pIncr = pReadr->pIncr;
    ਪੂਰਨ bEof = 1;
    ਜੇ( pIncr ){
      rc = vdbe_Incr_ਅਦਲਾ_ਬਦਲੀ(pIncr);
      ਜੇ( rc==SQLITE_OK && pIncr->bEof==0 ){
        rc = vdbePmaReaderSeek(
            pIncr->pTask, pReadr, &pIncr->aFile[0], pIncr->i_ਸ਼ੁਰੂ_Off
        );
        bEof = 0;
      }
    }

    ਜੇ( bEof ){
      /* This is an EOF condition */
      vdbePmaReaderClear(pReadr);
      testcase( rc!=SQLITE_OK );
      ਵਾਪਸ rc;
    }
  }

  ਜੇ( rc==SQLITE_OK ){
    rc = vdbePmaReadVarint(pReadr, &nRec);
  }
  ਜੇ( rc==SQLITE_OK ){
    pReadr->n_ਕੁੰਜੀ = (ਪੂਰਨ)nRec;
    rc = vdbePmaReadBlob(pReadr, (ਪੂਰਨ)nRec, &pReadr->a_ਕੁੰਜੀ);
    testcase( rc!=SQLITE_OK );
  }

  ਵਾਪਸ rc;
}

/*
** Initialize PmaReader pReadr to scan through the PMA stored in file pFile
** starting at offset iStart and ending at offset iEof-1. This function
** leaves the PmaReader pointing to the first key in the PMA (or EOF if the
** PMA is empty).
**
** If the pnByte parameter is NULL, then it is assumed that the file
** contains a single PMA, and that that PMA omits the initial length varint.
*/
ਸਥਿਰ ਪੂਰਨ vdbePmaReaderInit(
  ਛਾਂਟੋ_Subtask *pTask,             /* Task context */
  SorterFile *pFile,              /* Sorter file to read from */
  i64 i_ਸ਼ੁਰੂ,                     /* Start offset in pFile */
  PmaReader *pReadr,              /* PmaReader to populate */
  i64 *pnByte                     /* IN/OUT: Increment this value by PMA size */
){
  ਪੂਰਨ rc;

  ਪ੍ਰਤਿਜ੍ਞਾ( pFile->iEof>i_ਸ਼ੁਰੂ );
  ਪ੍ਰਤਿਜ੍ਞਾ( pReadr->aAlloc==0 && pReadr->nAlloc==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pReadr->a_ਬਫਰ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pReadr->aMap==0 );

  rc = vdbePmaReaderSeek(pTask, pReadr, pFile, i_ਸ਼ੁਰੂ);
  ਜੇ( rc==SQLITE_OK ){
    u64 nByte = 0;                 /* Size of PMA in bytes */
    rc = vdbePmaReadVarint(pReadr, &nByte);
    pReadr->iEof = pReadr->iReadOff + nByte;
    *pnByte += nByte;
  }

  ਜੇ( rc==SQLITE_OK ){
    rc = vdbe_Pma_Reader_ਅਗਲਾ(pReadr);
  }
  ਵਾਪਸ rc;
}

/*
** A version of vdbeSorterCompare() that assumes that it has already been
** determined that the first field of key1 is equal to the first field of
** key2.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Sorter_Compare_ਪੂਛ(
  ਛਾਂਟੋ_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ਪੂਰਨ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ਅਚਲ ਖਾਲੀ *pKey1, ਪੂਰਨ nKey1,   /* Left side of comparison */
  ਅਚਲ ਖਾਲੀ *pKey2, ਪੂਰਨ nKey2    /* Right side of comparison */
){
  Unpacked_ਰਿਕਾਰਡ *r2 = pTask->pUnpacked;
  ਜੇ( *pbKey2Cached==0 ){
    sqlite3_Vdbe_ਰਿਕਾਰਡ_Unpack(nKey2, pKey2, r2);
    *pbKey2Cached = 1;
  }
  ਵਾਪਸ sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare_With_Skip(nKey1, pKey1, r2, 1);
}

/*
** Compare key1 (buffer pKey1, size nKey1 bytes) with key2 (buffer pKey2,
** size nKey2 bytes). Use (pTask->pKeyInfo) for the collation sequences
** used by the comparison. Return the result of the comparison.
**
** If IN/OUT parameter *pbKey2Cached is true when this function is called,
** it is assumed that (pTask->pUnpacked) contains the unpacked version
** of key2. If it is false, (pTask->pUnpacked) is populated with the unpacked
** version of key2 and *pbKey2Cached set to true before returning.
**
** If an OOM error is encountered, (pTask->pUnpacked->error_rc) is set
** to SQLITE_NOMEM.
*/
ਸਥਿਰ ਪੂਰਨ vdbeSorterCompare(
  ਛਾਂਟੋ_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ਪੂਰਨ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ਅਚਲ ਖਾਲੀ *pKey1, ਪੂਰਨ nKey1,   /* Left side of comparison */
  ਅਚਲ ਖਾਲੀ *pKey2, ਪੂਰਨ nKey2    /* Right side of comparison */
){
  Unpacked_ਰਿਕਾਰਡ *r2 = pTask->pUnpacked;
  ਜੇ( !*pbKey2Cached ){
    sqlite3_Vdbe_ਰਿਕਾਰਡ_Unpack(nKey2, pKey2, r2);
    *pbKey2Cached = 1;
  }
  ਵਾਪਸ sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare(nKey1, pKey1, r2);
}

/*
** A specially optimized version of vdbeSorterCompare() that assumes that
** the first field of each key is a TEXT value and that the collation
** sequence to compare them with is BINARY.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Sorter_Compare_ਪਾਠ(
  ਛਾਂਟੋ_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ਪੂਰਨ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ਅਚਲ ਖਾਲੀ *pKey1, ਪੂਰਨ nKey1,   /* Left side of comparison */
  ਅਚਲ ਖਾਲੀ *pKey2, ਪੂਰਨ nKey2    /* Right side of comparison */
){
  ਅਚਲ u8 * ਅਚਲ p1 = (ਅਚਲ u8 * ਅਚਲ)pKey1;
  ਅਚਲ u8 * ਅਚਲ p2 = (ਅਚਲ u8 * ਅਚਲ)pKey2;
  ਅਚਲ u8 * ਅਚਲ v1 = &p1[ p1[0] ];   /* Pointer to value 1 */
  ਅਚਲ u8 * ਅਚਲ v2 = &p2[ p2[0] ];   /* Pointer to value 2 */

  ਪੂਰਨ n1;
  ਪੂਰਨ n2;
  ਪੂਰਨ res;

  getVarint32NR(&p1[1], n1);
  getVarint32NR(&p2[1], n2);
  res = memcmp(v1, v2, (MIN(n1, n2) - 13)/2);
  ਜੇ( res==0 ){
    res = n1 - n2;
  }

  ਜੇ( res==0 ){
    ਜੇ( pTask->pSorter->p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ>1 ){
      res = vdbe_Sorter_Compare_ਪੂਛ(
          pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2
      );
    }
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pTask->pSorter->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( !(pTask->pSorter->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[0]&KEYINFO_ਕ੍ਰਮ_BIGNULL) );
    ਜੇ( pTask->pSorter->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[0] ){
      res = res * -1;
    }
  }

  ਵਾਪਸ res;
}

/*
** A specially optimized version of vdbeSorterCompare() that assumes that
** the first field of each key is an INTEGER value.
*/
ਸਥਿਰ ਪੂਰਨ vdbeSorterCompareInt(
  ਛਾਂਟੋ_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ਪੂਰਨ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ਅਚਲ ਖਾਲੀ *pKey1, ਪੂਰਨ nKey1,   /* Left side of comparison */
  ਅਚਲ ਖਾਲੀ *pKey2, ਪੂਰਨ nKey2    /* Right side of comparison */
){
  ਅਚਲ u8 * ਅਚਲ p1 = (ਅਚਲ u8 * ਅਚਲ)pKey1;
  ਅਚਲ u8 * ਅਚਲ p2 = (ਅਚਲ u8 * ਅਚਲ)pKey2;
  ਅਚਲ ਪੂਰਨ s1 = p1[1];                 /* Left hand serial type */
  ਅਚਲ ਪੂਰਨ s2 = p2[1];                 /* Right hand serial type */
  ਅਚਲ u8 * ਅਚਲ v1 = &p1[ p1[0] ];   /* Pointer to value 1 */
  ਅਚਲ u8 * ਅਚਲ v2 = &p2[ p2[0] ];   /* Pointer to value 2 */
  ਪੂਰਨ res;                              /* Return value */

  ਪ੍ਰਤਿਜ੍ਞਾ( (s1>0 && s1<7) || s1==8 || s1==9 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (s2>0 && s2<7) || s2==8 || s2==9 );

  ਜੇ( s1==s2 ){
    /* The two values have the same sign. Compare using memcmp(). */
    ਸਥਿਰ ਅਚਲ u8 aLen[] = {0, 1, 2, 3, 4, 6, 8, 0, 0, 0 };
    ਅਚਲ u8 n = aLen[s1];
    ਪੂਰਨ i;
    res = 0;
    ਲਈ(i=0; i<n; i++){
      ਜੇ( (res = v1[i] - v2[i])!=0 ){
        ਜੇ( ((v1[0] ^ v2[0]) & 0x80)!=0 ){
          res = v1[0] & 0x80 ? -1 : +1;
        }
        ਰੋਕੋ;
      }
    }
  }ਨਹੀਂਤਾਂ ਜੇ( s1>7 && s2>7 ){
    res = s1 - s2;
  }ਨਹੀਂਤਾਂ{
    ਜੇ( s2>7 ){
      res = +1;
    }ਨਹੀਂਤਾਂ ਜੇ( s1>7 ){
      res = -1;
    }ਨਹੀਂਤਾਂ{
      res = s1 - s2;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( res!=0 );

    ਜੇ( res>0 ){
      ਜੇ( *v1 & 0x80 ) res = -1;
    }ਨਹੀਂਤਾਂ{
      ਜੇ( *v2 & 0x80 ) res = +1;
    }
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( pTask->pSorter->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags!=0 );
  ਜੇ( res==0 ){
    ਜੇ( pTask->pSorter->p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ>1 ){
      res = vdbe_Sorter_Compare_ਪੂਛ(
          pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2
      );
    }
  }ਨਹੀਂਤਾਂ ਜੇ( pTask->pSorter->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[0] ){
    ਪ੍ਰਤਿਜ੍ਞਾ( !(pTask->pSorter->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[0]&KEYINFO_ਕ੍ਰਮ_BIGNULL) );
    res = res * -1;
  }

  ਵਾਪਸ res;
}

/* Helper function for vdbeSorterCompareReal().
**
** The first elements of both pKey1 and pKey2 have been decoded into double
** values r1 and r2.  Do the comparison between those keys and return the
** result.  If r1==r2, break the tie with a comparison of subsequent elements
** from each key.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Sorter_ਅੰਤ_Real_Compare(
  ਛਾਂਟੋ_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ਪੂਰਨ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ਅਚਲ ਖਾਲੀ *pKey1, ਪੂਰਨ nKey1,   /* Left side of comparison */
  ਅਚਲ ਖਾਲੀ *pKey2, ਪੂਰਨ nKey2,   /* Right side of comparison */
  ਦੁੱਗਣਾ r1,                      /* REAL value of first element of pKey1 */
  ਦੁੱਗਣਾ r2                       /* REAL value of first element of pKey2 */
){
  ਪੂਰਨ res;
  ਜੇ( r1<r2 ){
    res = -1;
  }ਨਹੀਂਤਾਂ ਜੇ( r1>r2 ){
    res = +1;
  }ਨਹੀਂਤਾਂ{
    res = 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pTask->pSorter->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags!=0 );
  ਜੇ( res==0 ){
    ਜੇ( pTask->pSorter->p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ>1 ){
      res = vdbe_Sorter_Compare_ਪੂਛ(
          pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2
      );
    }
  }ਨਹੀਂਤਾਂ ਜੇ( pTask->pSorter->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[0] ){
    ਪ੍ਰਤਿਜ੍ਞਾ( !(pTask->pSorter->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[0]&KEYINFO_ਕ੍ਰਮ_BIGNULL) );
    res = res * -1;
  }
  ਵਾਪਸ res;
}

/* Helper function for vdbeSorterCompareReal().
**
** Buffer p[] is a record where the first term is guaranteed to be either
** a floating-point value, or an integer stand-in for a floating point
** value (a MEM_IntReal).  Whatever its format, extract the value and
** return it.
*/
ਸਥਿਰ ਦੁੱਗਣਾ vdbeSorterGetReal(ਅਚਲ u8 *p){
  ਦੁੱਗਣਾ r;                    /* the return value */

  ਪ੍ਰਤਿਜ੍ਞਾ( p[0]<0x80 );         /* 1-byte headers: nAllField<13 */
  ਪ੍ਰਤਿਜ੍ਞਾ( p[1]>0 && p[1]<10 ); /* first fields proven numeric */

  ਜੇ( p[1]==7 ){
    u64 x = sqlite3Get8byte(p + p[0]);
    ਅਦਲਾ_ਬਦਲੀ_Mixed_Endian_Float(x);
    ਪ੍ਰਤਿਜ੍ਞਾ( !IsNaN(x) );
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&r, &x, ਆਕਾਰ(r));
  }ਨਹੀਂਤਾਂ{
    Mem m;
    m.u.i = 0;
    sqlite3VdbeSerialGet(p + p[0], p[1], &m);
    ਪ੍ਰਤਿਜ੍ਞਾ( m.flags==MEM_Int );
    r = (ਦੁੱਗਣਾ)m.u.i;
  }
  ਵਾਪਸ r;
}

/* Helper function for vdbeSorterCompareReal()
**
** This routine handles the case of comparing two floating-point values
** where one or both of the floating-point are represented by integers.
** In other words, where one both is an MEM_RealInt.
**
** This subroutine is factored out from vdbeSorterCompareReal() for
** efficiency.  If inlined into vdbeSorterCompareReal(), this routine
** will use extra stack space and consume CPU cycles setting up and
** breaking down that stack space, even if in the common case where
** this path is not used.
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ vdbeSorterCompareRealInt(
  ਛਾਂਟੋ_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ਪੂਰਨ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ਅਚਲ ਖਾਲੀ *pKey1, ਪੂਰਨ nKey1,   /* Left side of comparison */
  ਅਚਲ ਖਾਲੀ *pKey2, ਪੂਰਨ nKey2    /* Right side of comparison */
){
  ਅਚਲ u8 * ਅਚਲ p1 = (ਅਚਲ u8 * ਅਚਲ)pKey1;
  ਅਚਲ u8 * ਅਚਲ p2 = (ਅਚਲ u8 * ਅਚਲ)pKey2;
  ਜੇ( p1[1]==6 || p2[1]==6 ){
    /* 64-bit integer values cannot be represented exactly by a double so
    ** must be handled by the generalized comparison function. */
    ਵਾਪਸ vdbeSorterCompare(pTask,
       pbKey2Cached, pKey1,nKey1, pKey2,nKey2);
  }ਨਹੀਂਤਾਂ{
    ਦੁੱਗਣਾ r1 = vdbeSorterGetReal(p1);
    ਦੁੱਗਣਾ r2 = vdbeSorterGetReal(p2);
    ਵਾਪਸ vdbe_Sorter_ਅੰਤ_Real_Compare(pTask,pbKey2Cached,
                  pKey1,nKey1,pKey2,nKey2,r1,r2);
  }
}

/*
** Comparison function optimized for the case where the first term
** of both keys are either MEM_Real or MEM_RealInt.
**
** See also vdbeSorterCompareInt() for MEM_Int values and
** vdbeSorterCompareText() for MEM_Str values.  The general
** case is vdbeSorterCompare() which handles anything, but is slower.
*/
ਸਥਿਰ ਪੂਰਨ vdbeSorterCompareReal(
  ਛਾਂਟੋ_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ਪੂਰਨ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ਅਚਲ ਖਾਲੀ *pKey1, ਪੂਰਨ nKey1,   /* Left side of comparison */
  ਅਚਲ ਖਾਲੀ *pKey2, ਪੂਰਨ nKey2    /* Right side of comparison */
){
  ਅਚਲ u8*ਅਚਲ p1 = (ਅਚਲ u8*ਅਚਲ)pKey1;  /* Left key record */
  ਅਚਲ u8*ਅਚਲ p2 = (ਅਚਲ u8*ਅਚਲ)pKey2;  /* Right key record */
  u64 x;                   /* A real value stored as an integer */
  ਦੁੱਗਣਾ r1;               /* First element of pKey1 */
  ਦੁੱਗਣਾ r2;               /* First element of pKey2 */

  ਪ੍ਰਤਿਜ੍ਞਾ( p1[0]<0x80 && p2[0]<0x80 );  /* 1-byte headers: nAllField<13 */
  ਪ੍ਰਤਿਜ੍ਞਾ( p1[1]>0 && p1[1]<10 );       /* first field guaranteed numeric */
  ਪ੍ਰਤਿਜ੍ਞਾ( p2[1]>0 && p2[1]<10 );       /* first field guaranteed numeric */

  ਜੇ( p1[1]!=7 || p2[1]!=7 ){
    /* One or both floating point values are stored as INTEGER.  This might
    ** be because of the MEM_RealInt encoding.  Try to optimize that case. */
    ਵਾਪਸ vdbeSorterCompareRealInt(pTask,
       pbKey2Cached, pKey1,nKey1, pKey2,nKey2
    );
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p1[0]<=nKey1-8 && p2[0]<=nKey2-8 );

  x = sqlite3Get8byte(p1 + *p1);
  ਅਦਲਾ_ਬਦਲੀ_Mixed_Endian_Float(x);
  ਪ੍ਰਤਿਜ੍ਞਾ( !IsNaN(x) );
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&r1, &x, ਆਕਾਰ(r1));
  x = sqlite3Get8byte(p2 + *p2);
  ਅਦਲਾ_ਬਦਲੀ_Mixed_Endian_Float(x);
  ਪ੍ਰਤਿਜ੍ਞਾ( !IsNaN(x) );
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&r2, &x, ਆਕਾਰ(r2));
  ਵਾਪਸ vdbe_Sorter_ਅੰਤ_Real_Compare(pTask,
      pbKey2Cached, pKey1,nKey1,  pKey2,nKey2, r1, r2
  );
}

/*
** Initialize the temporary index cursor just opened as a sorter cursor.
**
** Usually, the sorter module uses the value of (pCsr->pKeyInfo->nKeyField)
** to determine the number of fields that should be compared from the
** records being sorted. However, if the value passed as argument nField
** is non-zero and the sorter is able to guarantee a stable sort, nField
** is used instead. This is used when sorting records for a CREATE INDEX
** statement. In this case, keys are always delivered to the sorter in
** order of the primary key, which happens to be make up the final part
** of the records being sorted. So if the sort is stable, there is never
** any reason to compare PK fields and they can be ignored for a small
** performance boost.
**
** The sorter can guarantee a stable sort when running in single-threaded
** mode, but not in multi-threaded mode.
**
** SQLITE_OK is returned if successful, or an SQLite error code otherwise.
*/
ਪੂਰਨ sqlite3VdbeSorterInit(
  sqlite3 *db,                    /* Database connection (for malloc()) */
  ਪੂਰਨ n_ਖੇਤਰ,                     /* Number of key fields in each record */
  VdbeCursor *pCsr                /* Cursor that holds the new sorter */
){
  ਪੂਰਨ pgsz;                       /* Page size of main database */
  ਪੂਰਨ i;                          /* Used to iterate through aTask[] */
  VdbeSorter *pSorter;            /* The new sorter */
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info;              /* Copy of pCsr->pKeyInfo with db==0 */
  ਪੂਰਨ sz_ਕੁੰਜੀ_Info;                  /* Size of pCsr->pKeyInfo in bytes */
  i64 sz;                         /* Size of pSorter in bytes */
  ਪੂਰਨ rc = SQLITE_OK;
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ==0
# define n_ਕਾਮਾ 0
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ n_ਕਾਮਾ;
#ਜੇ_ਅੰਤ

  /* Initialize the upper limit on the number of worker threads */
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
  ਜੇ( sqlite3_ਆਰਜ਼ੀ_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ(db) || sqlite3GlobalConfig.bCoreMutex==0 ){
    n_ਕਾਮਾ = 0;
  }ਨਹੀਂਤਾਂ{
    n_ਕਾਮਾ = db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਕਾਮਾ_ਧਾਗੇ];
  }
#ਜੇ_ਅੰਤ

  /* Do not allow the total number of threads (main thread + all workers)
  ** to exceed the maximum merge count */
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>=SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ
  ਜੇ( n_ਕਾਮਾ>=SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ ){
    n_ਕਾਮਾ = SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ-1;
  }
#ਜੇ_ਅੰਤ

  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->p_ਕੁੰਜੀ_Info );
  ਪ੍ਰਤਿਜ੍ਞਾ( !pCsr->isEphemeral );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->eCurType==CURTYPE_SORTER );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਆਕਾਰ(ਕੁੰਜੀ_Info) + UMXV(pCsr->p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ)*ਆਕਾਰ(CollSeq*)
               < 0x7fffffff );
  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ<=pCsr->p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ );
  sz_ਕੁੰਜੀ_Info = SZ_KEYINFO(pCsr->p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ);
  sz = SZ_VDBESORTER(n_ਕਾਮਾ+1);

  pSorter = (VdbeSorter*)sqlite3DbMallocZero(db, sz + sz_ਕੁੰਜੀ_Info);
  pCsr->uc.pSorter = pSorter;
  ਜੇ( pSorter==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }ਨਹੀਂਤਾਂ{
    Btree *pBt = db->aDb[0].pBt;
    pSorter->p_ਕੁੰਜੀ_Info = p_ਕੁੰਜੀ_Info = (ਕੁੰਜੀ_Info*)((u8*)pSorter + sz);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਕੁੰਜੀ_Info, pCsr->p_ਕੁੰਜੀ_Info, sz_ਕੁੰਜੀ_Info);
    p_ਕੁੰਜੀ_Info->db = 0;
    ਜੇ( n_ਖੇਤਰ && n_ਕਾਮਾ==0 ){
      p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ = n_ਖੇਤਰ;
      ਪ੍ਰਤਿਜ੍ਞਾ( n_ਖੇਤਰ<=pCsr->p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ );
    }
    /* It is OK that pKeyInfo reuses the aSortFlags field from pCsr->pKeyInfo,
    ** since the pCsr->pKeyInfo->aSortFlags[] array is invariant and lives
    ** longer that pSorter. */
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags==pCsr->p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags );
    sqlite3BtreeEnter(pBt);
    pSorter->pgsz = pgsz = sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(pBt);
    sqlite3BtreeLeave(pBt);
    pSorter->nTask = n_ਕਾਮਾ + 1;
    pSorter->iPrev = (u8)(n_ਕਾਮਾ - 1);
    pSorter->b_Use_ਧਾਗੇ = (pSorter->nTask>1);
    pSorter->db = db;
    ਲਈ(i=0; i<pSorter->nTask; i++){
      ਛਾਂਟੋ_Subtask *pTask = &pSorter->aTask[i];
      pTask->pSorter = pSorter;
    }

    ਜੇ( !sqlite3_ਆਰਜ਼ੀ_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ(db) ){
      i64 mxCache;                /* Cache size in bytes*/
      u32 szPma = sqlite3GlobalConfig.szPma;
      pSorter->mn_Pma_ਮਾਪ = szPma * pgsz;

      mxCache = db->aDb[0].pSchema->cache_ਮਾਪ;
      ਜੇ( mxCache<0 ){
        /* A negative cache-size value C indicates that the cache is abs(C)
        ** KiB in size.  */
        mxCache = mxCache * -1024;
      }ਨਹੀਂਤਾਂ{
        mxCache = mxCache * pgsz;
      }
      mxCache = MIN(mxCache, SQLITE_MAX_PMASZ);
      pSorter->mx_Pma_ਮਾਪ = MAX(pSorter->mn_Pma_ਮਾਪ, (ਪੂਰਨ)mxCache);

      /* Avoid large memory allocations if the application has requested
      ** SQLITE_CONFIG_SMALL_MALLOC. */
      ਜੇ( sqlite3GlobalConfig.bSmallMalloc==0 ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pSorter->i_ਸਮ੍ਰਿਤੀ==0 );
        pSorter->n_ਸਮ੍ਰਿਤੀ = pgsz;
        pSorter->list.a_ਸਮ੍ਰਿਤੀ = (u8*)sqlite3Malloc(pgsz);
        ਜੇ( !pSorter->list.a_ਸਮ੍ਰਿਤੀ ) rc = SQLITE_NOMEM_BKPT;
      }
    }

    ਜੇ( p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ<13
     && (p_ਕੁੰਜੀ_Info->aColl[0]==0 || p_ਕੁੰਜੀ_Info->aColl[0]==db->pDfltColl)
     && (p_ਕੁੰਜੀ_Info->a_ਛਾਂਟੋ_Flags[0] & KEYINFO_ਕ੍ਰਮ_BIGNULL)==0
    ){
      pSorter->type_ਨਕਾਬ = SORTER_TYPE_INTEGER|SORTER_TYPE_ਪਾਠ|SORTER_TYPE_REAL;
    }
  }

  ਵਾਪਸ rc;
}
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ n_ਕਾਮਾ   /* Defined at the top of this function */

/*
** Free the list of sorted records starting at pRecord.
*/
ਸਥਿਰ ਖਾਲੀ vdbe_Sorter_ਰਿਕਾਰਡ_Free(sqlite3 *db, Sorter_ਰਿਕਾਰਡ *p_ਰਿਕਾਰਡ){
  Sorter_ਰਿਕਾਰਡ *p;
  Sorter_ਰਿਕਾਰਡ *p_ਅਗਲਾ;
  ਲਈ(p=p_ਰਿਕਾਰਡ; p; p=p_ਅਗਲਾ){
    p_ਅਗਲਾ = p->u.p_ਅਗਲਾ;
    sqlite3DbFree(db, p);
  }
}

/*
** Free all resources owned by the object indicated by argument pTask. All
** fields of *pTask are zeroed before returning.
*/
ਸਥਿਰ ਖਾਲੀ vdbe_ਛਾਂਟੋ_Subtask_Cleanup(sqlite3 *db, ਛਾਂਟੋ_Subtask *pTask){
  sqlite3DbFree(db, pTask->pUnpacked);
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
  /* pTask->list.aMemory can only be non-zero if it was handed memory
  ** from the main thread.  That only occurs SQLITE_MAX_WORKER_THREADS>0 */
  ਜੇ( pTask->list.a_ਸਮ੍ਰਿਤੀ ){
    sqlite3_free(pTask->list.a_ਸਮ੍ਰਿਤੀ);
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
  {
    ਪ੍ਰਤਿਜ੍ਞਾ( pTask->list.a_ਸਮ੍ਰਿਤੀ==0 );
    vdbe_Sorter_ਰਿਕਾਰਡ_Free(0, pTask->list.pList);
  }
  ਜੇ( pTask->file.pFd ){
    sqlite3OsCloseFree(pTask->file.pFd);
  }
  ਜੇ( pTask->file2.pFd ){
    sqlite3OsCloseFree(pTask->file2.pFd);
  }
  memset(pTask, 0, ਆਕਾਰ(ਛਾਂਟੋ_Subtask));
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG_SORTER_ਧਾਗੇ
ਸਥਿਰ ਖਾਲੀ vdbeSorterWorkDebug(ਛਾਂਟੋ_Subtask *pTask, ਅਚਲ ਅੱਖਰ *zEvent){
  i64 t;
  ਪੂਰਨ iTask = (pTask - pTask->pSorter->aTask);
  sqlite3_Os_Current_ਸਮਾਂ_Int64(pTask->pSorter->db->pVfs, &t);
  fprintf(stderr, "%lld:%d %s\n", t, iTask, zEvent);
}
ਸਥਿਰ ਖਾਲੀ vdbeSorterRewindDebug(ਅਚਲ ਅੱਖਰ *zEvent){
  i64 t = 0;
  sqlite3_vfs *pVfs = sqlite3_vfs_ਲੱਭੋ(0);
  ਜੇ( ALWAYS(pVfs) ) sqlite3_Os_Current_ਸਮਾਂ_Int64(pVfs, &t);
  fprintf(stderr, "%lld:X %s\n", t, zEvent);
}
ਸਥਿਰ ਖਾਲੀ vdbeSorterPopulateDebug(
  ਛਾਂਟੋ_Subtask *pTask,
  ਅਚਲ ਅੱਖਰ *zEvent
){
  i64 t;
  ਪੂਰਨ iTask = (pTask - pTask->pSorter->aTask);
  sqlite3_Os_Current_ਸਮਾਂ_Int64(pTask->pSorter->db->pVfs, &t);
  fprintf(stderr, "%lld:bg%d %s\n", t, iTask, zEvent);
}
ਸਥਿਰ ਖਾਲੀ vdbe_Sorter_ਖੰਡ_Debug(
  ਛਾਂਟੋ_Subtask *pTask,
  ਪੂਰਨ bBlocked,
  ਅਚਲ ਅੱਖਰ *zEvent
){
  ਜੇ( bBlocked ){
    i64 t;
    sqlite3_Os_Current_ਸਮਾਂ_Int64(pTask->pSorter->db->pVfs, &t);
    fprintf(stderr, "%lld:main %s\n", t, zEvent);
  }
}
#ਨਹੀਂਤਾਂ
# define vdbeSorterWorkDebug(x,y)
# define vdbeSorterRewindDebug(y)
# define vdbeSorterPopulateDebug(x,y)
# define vdbe_Sorter_ਖੰਡ_Debug(x,y,z)
#ਜੇ_ਅੰਤ

#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
/*
** Join thread pTask->thread.
*/
ਸਥਿਰ ਪੂਰਨ vdbeSorterJoinThread(ਛਾਂਟੋ_Subtask *pTask){
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( pTask->pThread ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG_SORTER_ਧਾਗੇ
    ਪੂਰਨ b_ਪੂਰਾ = pTask->b_ਪੂਰਾ;
#ਜੇ_ਅੰਤ
    ਖਾਲੀ *pRet = SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(SQLITE_ERROR);
    vdbe_Sorter_ਖੰਡ_Debug(pTask, !b_ਪੂਰਾ, "enter");
    (ਖਾਲੀ)sqlite3ThreadJoin(pTask->pThread, &pRet);
    vdbe_Sorter_ਖੰਡ_Debug(pTask, !b_ਪੂਰਾ, "exit");
    rc = SQLITE_PTR_ਮੰਜ਼ਿਲ_INT(pRet);
    ਪ੍ਰਤਿਜ੍ਞਾ( pTask->b_ਪੂਰਾ==1 );
    pTask->b_ਪੂਰਾ = 0;
    pTask->pThread = 0;
  }
  ਵਾਪਸ rc;
}

/*
** Launch a background thread to run xTask(pIn).
*/
ਸਥਿਰ ਪੂਰਨ vdbeSorterCreateThread(
  ਛਾਂਟੋ_Subtask *pTask,             /* Thread will use this task object */
  ਖਾਲੀ *(*xTask)(ਖਾਲੀ*),          /* Routine to run in a separate thread */
  ਖਾਲੀ *p_ਵਿੱਚ                       /* Argument passed into xTask() */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( pTask->pThread==0 && pTask->b_ਪੂਰਾ==0 );
  ਵਾਪਸ sqlite3ThreadCreate(&pTask->pThread, xTask, p_ਵਿੱਚ);
}

/*
** Join all outstanding threads launched by SorterWrite() to create
** level-0 PMAs.
*/
ਸਥਿਰ ਪੂਰਨ vdbeSorterJoinAll(VdbeSorter *pSorter, ਪੂਰਨ rcin){
  ਪੂਰਨ rc = rcin;
  ਪੂਰਨ i;

  /* This function is always called by the main user thread.
  **
  ** If this function is being called after SorterRewind() has been called,
  ** it is possible that thread pSorter->aTask[pSorter->nTask-1].pThread
  ** is currently attempt to join one of the other threads. To avoid a race
  ** condition where this thread also attempts to join the same object, join
  ** thread pSorter->aTask[pSorter->nTask-1].pThread first. */
  ਲਈ(i=pSorter->nTask-1; i>=0; i--){
    ਛਾਂਟੋ_Subtask *pTask = &pSorter->aTask[i];
    ਪੂਰਨ rc2 = vdbeSorterJoinThread(pTask);
    ਜੇ( rc==SQLITE_OK ) rc = rc2;
  }
  ਵਾਪਸ rc;
}
#ਨਹੀਂਤਾਂ
# define vdbeSorterJoinAll(x,rcin) (rcin)
# define vdbeSorterJoinThread(pTask) SQLITE_OK
#ਜੇ_ਅੰਤ

/*
** Allocate a new MergeEngine object capable of handling up to
** nReader PmaReader inputs.
**
** nReader is automatically rounded up to the next power of two.
** nReader may not exceed SORTER_MAX_MERGE_COUNT even after rounding up.
*/
ਸਥਿਰ ਮਿਲਾਓ_Engine *vdbe_ਮਿਲਾਓ_Engine_New(ਪੂਰਨ nReader){
  ਪੂਰਨ N = 2;                      /* Smallest power of two >= nReader */
  i64 nByte;                      /* Total bytes of space to allocate */
  ਮਿਲਾਓ_Engine *pNew;              /* Pointer to allocated object to return */

  ਪ੍ਰਤਿਜ੍ਞਾ( nReader<=SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ );

  ਜਦੋਂ( N<nReader ) N += N;
  nByte = ਆਕਾਰ(ਮਿਲਾਓ_Engine) + N * (ਆਕਾਰ(ਪੂਰਨ) + ਆਕਾਰ(PmaReader));

  pNew = sqlite3FaultSim(100) ? 0 : (ਮਿਲਾਓ_Engine*)sqlite3MallocZero(nByte);
  ਜੇ( pNew ){
    pNew->n_ਰੁੱਖ = N;
    pNew->pTask = 0;
    pNew->aReadr = (PmaReader*)&pNew[1];
    pNew->a_ਰੁੱਖ = (ਪੂਰਨ*)&pNew->aReadr[N];
  }
  ਵਾਪਸ pNew;
}

/*
** Free the MergeEngine object passed as the only argument.
*/
ਸਥਿਰ ਖਾਲੀ vdbe_ਮਿਲਾਓ_Engine_Free(ਮਿਲਾਓ_Engine *pMerger){
  ਪੂਰਨ i;
  ਜੇ( pMerger ){
    ਲਈ(i=0; i<pMerger->n_ਰੁੱਖ; i++){
      vdbePmaReaderClear(&pMerger->aReadr[i]);
    }
  }
  sqlite3_free(pMerger);
}

/*
** Free all resources associated with the IncrMerger object indicated by
** the first argument.
*/
ਸਥਿਰ ਖਾਲੀ vdbeIncrFree(IncrMerger *pIncr){
  ਜੇ( pIncr ){
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
    ਜੇ( pIncr->bUseThread ){
      vdbeSorterJoinThread(pIncr->pTask);
      ਜੇ( pIncr->aFile[0].pFd ) sqlite3OsCloseFree(pIncr->aFile[0].pFd);
      ਜੇ( pIncr->aFile[1].pFd ) sqlite3OsCloseFree(pIncr->aFile[1].pFd);
    }
#ਜੇ_ਅੰਤ
    vdbe_ਮਿਲਾਓ_Engine_Free(pIncr->pMerger);
    sqlite3_free(pIncr);
  }
}

/*
** Reset a sorting cursor back to its original empty state.
*/
ਖਾਲੀ sqlite3VdbeSorterReset(sqlite3 *db, VdbeSorter *pSorter){
  ਪੂਰਨ i;
  (ਖਾਲੀ)vdbeSorterJoinAll(pSorter, SQLITE_OK);
  ਪ੍ਰਤਿਜ੍ਞਾ( pSorter->b_Use_ਧਾਗੇ || pSorter->pReader==0 );
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
  ਜੇ( pSorter->pReader ){
    vdbePmaReaderClear(pSorter->pReader);
    sqlite3DbFree(db, pSorter->pReader);
    pSorter->pReader = 0;
  }
#ਜੇ_ਅੰਤ
  vdbe_ਮਿਲਾਓ_Engine_Free(pSorter->pMerger);
  pSorter->pMerger = 0;
  ਲਈ(i=0; i<pSorter->nTask; i++){
    ਛਾਂਟੋ_Subtask *pTask = &pSorter->aTask[i];
    vdbe_ਛਾਂਟੋ_Subtask_Cleanup(db, pTask);
    pTask->pSorter = pSorter;
  }
  ਜੇ( pSorter->list.a_ਸਮ੍ਰਿਤੀ==0 ){
    vdbe_Sorter_ਰਿਕਾਰਡ_Free(0, pSorter->list.pList);
  }
  pSorter->list.pList = 0;
  pSorter->list.szPMA = 0;
  pSorter->bUsePMA = 0;
  pSorter->i_ਸਮ੍ਰਿਤੀ = 0;
  pSorter->mxKeysize = 0;
  sqlite3DbFree(db, pSorter->pUnpacked);
  pSorter->pUnpacked = 0;
}

/*
** Free any cursor components allocated by sqlite3VdbeSorterXXX routines.
*/
ਖਾਲੀ sqlite3VdbeSorterClose(sqlite3 *db, VdbeCursor *pCsr){
  VdbeSorter *pSorter;
  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  ਜੇ( pSorter ){
    /* Increment db->nSpill by the total number of bytes of data written
    ** to temp files by this sort operation.  */
    ਪੂਰਨ ii;
    ਲਈ(ii=0; ii<pSorter->nTask; ii++){
      db->nSpill += pSorter->aTask[ii].nSpill;
    }
    sqlite3VdbeSorterReset(db, pSorter);
    sqlite3_free(pSorter->list.a_ਸਮ੍ਰਿਤੀ);
    sqlite3DbFree(db, pSorter);
    pCsr->uc.pSorter = 0;
  }
}

#ਜੇ SQLITE_MAX_MMAP_ਮਾਪ>0
/*
** The first argument is a file-handle open on a temporary file. The file
** is guaranteed to be nByte bytes or smaller in size. This function
** attempts to extend the file to nByte bytes in size and to ensure that
** the VFS has memory mapped it.
**
** Whether or not the file does end up memory mapped of course depends on
** the specific VFS implementation.
*/
ਸਥਿਰ ਖਾਲੀ vdbeSorterExtendFile(sqlite3 *db, sqlite3_file *pFd, i64 nByte){
  ਜੇ( nByte<=(i64)(db->nMaxSorterMmap) && pFd->pMethods->i_ਸੰਸਕਰਣ>=3 ){
    ਖਾਲੀ *p = 0;
    ਪੂਰਨ chunksize = 4*1024;
    sqlite3OsFileControlHint(pFd, SQLITE_FCNTL_CHUNK_ਮਾਪ, &chunksize);
    sqlite3OsFileControlHint(pFd, SQLITE_FCNTL_ਮਾਪ_HINT, &nByte);
    sqlite3OsFetch(pFd, 0, (ਪੂਰਨ)nByte, &p);
    ਜੇ( p ) sqlite3OsUnfetch(pFd, 0, p);
  }
}
#ਨਹੀਂਤਾਂ
# define vdbeSorterExtendFile(x,y,z)
#ਜੇ_ਅੰਤ

/*
** Allocate space for a file-handle and open a temporary file. If successful,
** set *ppFd to point to the malloc'd file-handle and return SQLITE_OK.
** Otherwise, set *ppFd to 0 and return an SQLite error code.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Sorter_Open_ਆਰਜ਼ੀ_File(
  sqlite3 *db,                    /* Database handle doing sort */
  i64 nExtend,                    /* Attempt to extend file to this size */
  sqlite3_file **ppFd
){
  ਪੂਰਨ rc;
  ਜੇ( sqlite3FaultSim(202) ) ਵਾਪਸ SQLITE_IOERR_ACCESS;
  rc = sqlite3OsOpenMalloc(db->pVfs, 0, ppFd,
      SQLITE_OPEN_ਆਰਜ਼ੀ_JOURNAL |
      SQLITE_OPEN_READWRITE    | SQLITE_OPEN_CREATE |
      SQLITE_OPEN_EXCLUSIVE    | SQLITE_OPEN_DELETEONCLOSE, &rc
  );
  ਜੇ( rc==SQLITE_OK ){
    i64 max = SQLITE_MAX_MMAP_ਮਾਪ;
    sqlite3OsFileControlHint(*ppFd, SQLITE_FCNTL_MMAP_ਮਾਪ, (ਖਾਲੀ*)&max);
    ਜੇ( nExtend>0 ){
      vdbeSorterExtendFile(db, *ppFd, nExtend);
    }
  }
  ਵਾਪਸ rc;
}

/*
** If it has not already been allocated, allocate the UnpackedRecord
** structure at pTask->pUnpacked. Return SQLITE_OK if successful (or
** if no allocation was required), or SQLITE_NOMEM otherwise.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_ਛਾਂਟੋ_Alloc_Unpacked(ਛਾਂਟੋ_Subtask *pTask){
  ਜੇ( pTask->pUnpacked==0 ){
    pTask->pUnpacked = sqlite3_Vdbe_Alloc_Unpacked_ਰਿਕਾਰਡ(pTask->pSorter->p_ਕੁੰਜੀ_Info);
    ਜੇ( pTask->pUnpacked==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
    pTask->pUnpacked->n_ਖੇਤਰ = pTask->pSorter->p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ;
    pTask->pUnpacked->err_ਕੋਡ = 0;
  }
  ਵਾਪਸ SQLITE_OK;
}


/*
** Merge the two sorted lists p1 and p2 into a single list.
*/
ਸਥਿਰ Sorter_ਰਿਕਾਰਡ *vdbe_Sorter_ਮਿਲਾਓ(
  ਛਾਂਟੋ_Subtask *pTask,             /* Calling thread context */
  Sorter_ਰਿਕਾਰਡ *p1,               /* First list to merge */
  Sorter_ਰਿਕਾਰਡ *p2                /* Second list to merge */
){
  Sorter_ਰਿਕਾਰਡ *pFinal = 0;
  Sorter_ਰਿਕਾਰਡ **pp = &pFinal;
  ਪੂਰਨ bCached = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( p1!=0 && p2!=0 );
  ਲਈ(;;){
    ਪੂਰਨ res;
    res = pTask->xCompare(
        pTask, &bCached, SRVAL(p1), p1->nVal, SRVAL(p2), p2->nVal
    );

    ਜੇ( res<=0 ){
      *pp = p1;
      pp = &p1->u.p_ਅਗਲਾ;
      p1 = p1->u.p_ਅਗਲਾ;
      ਜੇ( p1==0 ){
        *pp = p2;
        ਰੋਕੋ;
      }
    }ਨਹੀਂਤਾਂ{
      *pp = p2;
      pp = &p2->u.p_ਅਗਲਾ;
      p2 = p2->u.p_ਅਗਲਾ;
      bCached = 0;
      ਜੇ( p2==0 ){
        *pp = p1;
        ਰੋਕੋ;
      }
    }
  }
  ਵਾਪਸ pFinal;
}

/*
** Return the SorterCompare function to compare values collected by the
** sorter object passed as the only argument.
*/
ਸਥਿਰ SorterCompare vdbeSorterGetCompare(VdbeSorter *p){
  ਜੇ( p->type_ਨਕਾਬ & SORTER_TYPE_INTEGER ){
    ਵਾਪਸ vdbeSorterCompareInt;
  }ਨਹੀਂਤਾਂ ਜੇ( p->type_ਨਕਾਬ & SORTER_TYPE_ਪਾਠ ){
    ਵਾਪਸ vdbe_Sorter_Compare_ਪਾਠ;
  }ਨਹੀਂਤਾਂ ਜੇ( p->type_ਨਕਾਬ & SORTER_TYPE_REAL ){
    ਵਾਪਸ vdbeSorterCompareReal;
  }
  ਵਾਪਸ vdbeSorterCompare;
}

/*
** Sort the linked list of records headed at pTask->pList. Return
** SQLITE_OK if successful, or an SQLite error code (i.e. SQLITE_NOMEM) if
** an error occurs.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Sorter_ਛਾਂਟੋ(ਛਾਂਟੋ_Subtask *pTask, SorterList *pList){
  ਪੂਰਨ i;
  Sorter_ਰਿਕਾਰਡ *p;
  ਪੂਰਨ rc;
  Sorter_ਰਿਕਾਰਡ *a_ਥਾਂ[64];

  rc = vdbe_ਛਾਂਟੋ_Alloc_Unpacked(pTask);
  ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;

  p = pList->pList;
  pTask->xCompare = vdbeSorterGetCompare(pTask->pSorter);
  memset(a_ਥਾਂ, 0, ਆਕਾਰ(a_ਥਾਂ));

  ਜਦੋਂ( p ){
    Sorter_ਰਿਕਾਰਡ *p_ਅਗਲਾ;
    ਜੇ( pList->a_ਸਮ੍ਰਿਤੀ ){
      ਜੇ( (u8*)p==pList->a_ਸਮ੍ਰਿਤੀ ){
        p_ਅਗਲਾ = 0;
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( p->u.i_ਅਗਲਾ<sqlite3_Malloc_ਮਾਪ(pList->a_ਸਮ੍ਰਿਤੀ) );
        p_ਅਗਲਾ = (Sorter_ਰਿਕਾਰਡ*)&pList->a_ਸਮ੍ਰਿਤੀ[p->u.i_ਅਗਲਾ];
      }
    }ਨਹੀਂਤਾਂ{
      p_ਅਗਲਾ = p->u.p_ਅਗਲਾ;
    }

    p->u.p_ਅਗਲਾ = 0;
    ਲਈ(i=0; a_ਥਾਂ[i]; i++){
      p = vdbe_Sorter_ਮਿਲਾਓ(pTask, p, a_ਥਾਂ[i]);
      /* ,--Each aSlot[] holds twice as much as the previous. So we cannot use
      ** |  up all 64 aSlots[] with only a 64-bit address space.
      ** v                                                                */
      ਪ੍ਰਤਿਜ੍ਞਾ( i<Array_ਮਾਪ(a_ਥਾਂ) );
      a_ਥਾਂ[i] = 0;
    }
    a_ਥਾਂ[i] = p;
    p = p_ਅਗਲਾ;
  }

  p = 0;
  ਲਈ(i=0; i<Array_ਮਾਪ(a_ਥਾਂ); i++){
    ਜੇ( a_ਥਾਂ[i]==0 ) ਜਾਰੀ;
    p = p ? vdbe_Sorter_ਮਿਲਾਓ(pTask, p, a_ਥਾਂ[i]) : a_ਥਾਂ[i];
  }
  pList->pList = p;

  ਪ੍ਰਤਿਜ੍ਞਾ( pTask->pUnpacked->err_ਕੋਡ==SQLITE_OK
       || pTask->pUnpacked->err_ਕੋਡ==SQLITE_NOMEM
  );
  ਵਾਪਸ pTask->pUnpacked->err_ਕੋਡ;
}

/*
** Initialize a PMA-writer object.
*/
ਸਥਿਰ ਖਾਲੀ vdbePmaWriterInit(
  sqlite3_file *pFd,              /* File handle to write to */
  PmaWriter *p,                   /* Object to populate */
  ਪੂਰਨ nBuf,                       /* Buffer size */
  i64 i_ਸ਼ੁਰੂ                      /* Offset of pFd to begin writing at */
){
  memset(p, 0, ਆਕਾਰ(PmaWriter));
  p->a_ਬਫਰ = (u8*)sqlite3Malloc(nBuf);
  ਜੇ( !p->a_ਬਫਰ ){
    p->eFWErr = SQLITE_NOMEM_BKPT;
  }ਨਹੀਂਤਾਂ{
    p->i_Buf_ਸਿਰਾ = p->i_Buf_ਸ਼ੁਰੂ = (i_ਸ਼ੁਰੂ % nBuf);
    p->iWriteOff = i_ਸ਼ੁਰੂ - p->i_Buf_ਸ਼ੁਰੂ;
    p->n_ਬਫਰ = nBuf;
    p->pFd = pFd;
  }
}

/*
** Write nData bytes of data to the PMA. Return SQLITE_OK
** if successful, or an SQLite error code if an error occurs.
*/
ਸਥਿਰ ਖਾਲੀ vdbePmaWriteBlob(PmaWriter *p, u8 *p_ਅੰਕੜੇ, ਪੂਰਨ n_ਅੰਕੜੇ){
  ਪੂਰਨ nRem = n_ਅੰਕੜੇ;
  ਜਦੋਂ( nRem>0 && p->eFWErr==0 ){
    ਪੂਰਨ nCopy = nRem;
    ਜੇ( nCopy>(p->n_ਬਫਰ - p->i_Buf_ਸਿਰਾ) ){
      nCopy = p->n_ਬਫਰ - p->i_Buf_ਸਿਰਾ;
    }

    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p->a_ਬਫਰ[p->i_Buf_ਸਿਰਾ], &p_ਅੰਕੜੇ[n_ਅੰਕੜੇ-nRem], nCopy);
    p->i_Buf_ਸਿਰਾ += nCopy;
    ਜੇ( p->i_Buf_ਸਿਰਾ==p->n_ਬਫਰ ){
      p->eFWErr = sqlite3OsWrite(p->pFd,
          &p->a_ਬਫਰ[p->i_Buf_ਸ਼ੁਰੂ], p->i_Buf_ਸਿਰਾ - p->i_Buf_ਸ਼ੁਰੂ,
          p->iWriteOff + p->i_Buf_ਸ਼ੁਰੂ
      );
      p->nPmaSpill += (p->i_Buf_ਸਿਰਾ - p->i_Buf_ਸ਼ੁਰੂ);
      p->i_Buf_ਸ਼ੁਰੂ = p->i_Buf_ਸਿਰਾ = 0;
      p->iWriteOff += p->n_ਬਫਰ;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( p->i_Buf_ਸਿਰਾ<p->n_ਬਫਰ );

    nRem -= nCopy;
  }
}

/*
** Flush any buffered data to disk and clean up the PMA-writer object.
** The results of using the PMA-writer after this call are undefined.
** Return SQLITE_OK if flushing the buffered data succeeds or is not
** required. Otherwise, return an SQLite error code.
**
** Before returning, set *piEof to the offset immediately following the
** last byte written to the file. Also, increment (*pnSpill) by the total
** number of bytes written to the file.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Pma_Writer_ਅੰਤ(PmaWriter *p, i64 *piEof, u64 *pnSpill){
  ਪੂਰਨ rc;
  ਜੇ( p->eFWErr==0 && ALWAYS(p->a_ਬਫਰ) && p->i_Buf_ਸਿਰਾ>p->i_Buf_ਸ਼ੁਰੂ ){
    p->eFWErr = sqlite3OsWrite(p->pFd,
        &p->a_ਬਫਰ[p->i_Buf_ਸ਼ੁਰੂ], p->i_Buf_ਸਿਰਾ - p->i_Buf_ਸ਼ੁਰੂ,
        p->iWriteOff + p->i_Buf_ਸ਼ੁਰੂ
    );
    p->nPmaSpill += (p->i_Buf_ਸਿਰਾ - p->i_Buf_ਸ਼ੁਰੂ);
  }
  *piEof = (p->iWriteOff + p->i_Buf_ਸਿਰਾ);
  *pnSpill += p->nPmaSpill;
  sqlite3_free(p->a_ਬਫਰ);
  rc = p->eFWErr;
  memset(p, 0, ਆਕਾਰ(PmaWriter));
  ਵਾਪਸ rc;
}

/*
** Write value iVal encoded as a varint to the PMA. Return
** SQLITE_OK if successful, or an SQLite error code if an error occurs.
*/
ਸਥਿਰ ਖਾਲੀ vdbePmaWriteVarint(PmaWriter *p, u64 iVal){
  ਪੂਰਨ nByte;
  u8 aByte[10];
  nByte = sqlite3PutVarint(aByte, iVal);
  vdbePmaWriteBlob(p, aByte, nByte);
}

/*
** Write the current contents of in-memory linked-list pList to a level-0
** PMA in the temp file belonging to sub-task pTask. Return SQLITE_OK if
** successful, or an SQLite error code otherwise.
**
** The format of a PMA is:
**
**     * A varint. This varint contains the total number of bytes of content
**       in the PMA (not including the varint itself).
**
**     * One or more records packed end-to-end in order of ascending keys.
**       Each record consists of a varint followed by a blob of data (the
**       key). The varint is the number of bytes in the blob of data.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Sorter_List_ਮੰਜ਼ਿਲ_PMA(ਛਾਂਟੋ_Subtask *pTask, SorterList *pList){
  sqlite3 *db = pTask->pSorter->db;
  ਪੂਰਨ rc = SQLITE_OK;             /* Return code */
  PmaWriter writer;               /* Object used to write to the file */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* Set iSz to the expected size of file pTask->file after writing the PMA.
  ** This is used by an assert() statement at the end of this function.  */
  i64 iSz = pList->szPMA + sqlite3VarintLen(pList->szPMA) + pTask->file.iEof;
#ਜੇ_ਅੰਤ

  vdbeSorterWorkDebug(pTask, "enter");
  memset(&writer, 0, ਆਕਾਰ(PmaWriter));
  ਪ੍ਰਤਿਜ੍ਞਾ( pList->szPMA>0 );

  /* If the first temporary PMA file has not been opened, open it now. */
  ਜੇ( pTask->file.pFd==0 ){
    rc = vdbe_Sorter_Open_ਆਰਜ਼ੀ_File(db, 0, &pTask->file.pFd);
    ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || pTask->file.pFd );
    ਪ੍ਰਤਿਜ੍ਞਾ( pTask->file.iEof==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pTask->nPMA==0 );
  }

  /* Try to get the file to memory map */
  ਜੇ( rc==SQLITE_OK ){
    vdbeSorterExtendFile(db, pTask->file.pFd, pTask->file.iEof+pList->szPMA+9);
  }

  /* Sort the list */
  ਜੇ( rc==SQLITE_OK ){
    rc = vdbe_Sorter_ਛਾਂਟੋ(pTask, pList);
  }

  ਜੇ( rc==SQLITE_OK ){
    Sorter_ਰਿਕਾਰਡ *p;
    Sorter_ਰਿਕਾਰਡ *p_ਅਗਲਾ = 0;

    vdbePmaWriterInit(pTask->file.pFd, &writer, pTask->pSorter->pgsz,
                      pTask->file.iEof);
    pTask->nPMA++;
    vdbePmaWriteVarint(&writer, pList->szPMA);
    ਲਈ(p=pList->pList; p; p=p_ਅਗਲਾ){
      p_ਅਗਲਾ = p->u.p_ਅਗਲਾ;
      vdbePmaWriteVarint(&writer, p->nVal);
      vdbePmaWriteBlob(&writer, SRVAL(p), p->nVal);
      ਜੇ( pList->a_ਸਮ੍ਰਿਤੀ==0 ) sqlite3_free(p);
    }
    pList->pList = p;
    rc = vdbe_Pma_Writer_ਅੰਤ(&writer, &pTask->file.iEof, &pTask->nSpill);
  }

  vdbeSorterWorkDebug(pTask, "exit");
  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || pList->pList==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || pTask->file.iEof==iSz );
  ਵਾਪਸ rc;
}

/*
** Advance the MergeEngine to its next entry.
** Set *pbEof to true there is no next entry because
** the MergeEngine has reached the end of all its inputs.
**
** Return SQLITE_OK if successful or an error code if an error occurs.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_ਮਿਲਾਓ_Engine_ਕਦਮ(
  ਮਿਲਾਓ_Engine *pMerger,      /* The merge engine to advance to the next row */
  ਪੂਰਨ *pbEof                 /* Set TRUE at EOF.  Set false for more content */
){
  ਪੂਰਨ rc;
  ਪੂਰਨ iPrev = pMerger->a_ਰੁੱਖ[1];/* Index of PmaReader to advance */
  ਛਾਂਟੋ_Subtask *pTask = pMerger->pTask;

  /* Advance the current PmaReader */
  rc = vdbe_Pma_Reader_ਅਗਲਾ(&pMerger->aReadr[iPrev]);

  /* Update contents of aTree[] */
  ਜੇ( rc==SQLITE_OK ){
    ਪੂਰਨ i;                      /* Index of aTree[] to recalculate */
    PmaReader *pReadr1;         /* First PmaReader to compare */
    PmaReader *pReadr2;         /* Second PmaReader to compare */
    ਪੂਰਨ bCached = 0;

    /* Find the first two PmaReaders to compare. The one that was just
    ** advanced (iPrev) and the one next to it in the array.  */
    pReadr1 = &pMerger->aReadr[(iPrev & 0xFFFE)];
    pReadr2 = &pMerger->aReadr[(iPrev | 0x0001)];

    ਲਈ(i=(pMerger->n_ਰੁੱਖ+iPrev)/2; i>0; i=i/2){
      /* Compare pReadr1 and pReadr2. Store the result in variable iRes. */
      ਪੂਰਨ iRes;
      ਜੇ( pReadr1->pFd==0 ){
        iRes = +1;
      }ਨਹੀਂਤਾਂ ਜੇ( pReadr2->pFd==0 ){
        iRes = -1;
      }ਨਹੀਂਤਾਂ{
        iRes = pTask->xCompare(pTask, &bCached,
            pReadr1->a_ਕੁੰਜੀ, pReadr1->n_ਕੁੰਜੀ, pReadr2->a_ਕੁੰਜੀ, pReadr2->n_ਕੁੰਜੀ
        );
      }

      /* If pReadr1 contained the smaller value, set aTree[i] to its index.
      ** Then set pReadr2 to the next PmaReader to compare to pReadr1. In this
      ** case there is no cache of pReadr2 in pTask->pUnpacked, so set
      ** pKey2 to point to the record belonging to pReadr2.
      **
      ** Alternatively, if pReadr2 contains the smaller of the two values,
      ** set aTree[i] to its index and update pReadr1. If vdbeSorterCompare()
      ** was actually called above, then pTask->pUnpacked now contains
      ** a value equivalent to pReadr2. So set pKey2 to NULL to prevent
      ** vdbeSorterCompare() from decoding pReadr2 again.
      **
      ** If the two values were equal, then the value from the oldest
      ** PMA should be considered smaller. The VdbeSorter.aReadr[] array
      ** is sorted from oldest to newest, so pReadr1 contains older values
      ** than pReadr2 iff (pReadr1<pReadr2).  */
      ਜੇ( iRes<0 || (iRes==0 && pReadr1<pReadr2) ){
        pMerger->a_ਰੁੱਖ[i] = (ਪੂਰਨ)(pReadr1 - pMerger->aReadr);
        pReadr2 = &pMerger->aReadr[ pMerger->a_ਰੁੱਖ[i ^ 0x0001] ];
        bCached = 0;
      }ਨਹੀਂਤਾਂ{
        ਜੇ( pReadr1->pFd ) bCached = 0;
        pMerger->a_ਰੁੱਖ[i] = (ਪੂਰਨ)(pReadr2 - pMerger->aReadr);
        pReadr1 = &pMerger->aReadr[ pMerger->a_ਰੁੱਖ[i ^ 0x0001] ];
      }
    }
    *pbEof = (pMerger->aReadr[pMerger->a_ਰੁੱਖ[1]].pFd==0);
  }

  ਵਾਪਸ (rc==SQLITE_OK ? pTask->pUnpacked->err_ਕੋਡ : rc);
}

#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
/*
** The main routine for background threads that write level-0 PMAs.
*/
ਸਥਿਰ ਖਾਲੀ *vdbeSorterFlushThread(ਖਾਲੀ *pCtx){
  ਛਾਂਟੋ_Subtask *pTask = (ਛਾਂਟੋ_Subtask*)pCtx;
  ਪੂਰਨ rc;                         /* Return code */
  ਪ੍ਰਤਿਜ੍ਞਾ( pTask->b_ਪੂਰਾ==0 );
  rc = vdbe_Sorter_List_ਮੰਜ਼ਿਲ_PMA(pTask, &pTask->list);
  pTask->b_ਪੂਰਾ = 1;
  ਵਾਪਸ SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(rc);
}
#ਜੇ_ਅੰਤ /* SQLITE_MAX_WORKER_THREADS>0 */

/*
** Flush the current contents of VdbeSorter.list to a new PMA, possibly
** using a background thread.
*/
ਸਥਿਰ ਪੂਰਨ vdbeSorterFlushPMA(VdbeSorter *pSorter){
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ==0
  pSorter->bUsePMA = 1;
  ਵਾਪਸ vdbe_Sorter_List_ਮੰਜ਼ਿਲ_PMA(&pSorter->aTask[0], &pSorter->list);
#ਨਹੀਂਤਾਂ
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ i;
  ਛਾਂਟੋ_Subtask *pTask = 0;    /* Thread context used to create new PMA */
  ਪੂਰਨ n_ਕਾਮਾ = (pSorter->nTask-1);

  /* Set the flag to indicate that at least one PMA has been written.
  ** Or will be, anyhow.  */
  pSorter->bUsePMA = 1;

  /* Select a sub-task to sort and flush the current list of in-memory
  ** records to disk. If the sorter is running in multi-threaded mode,
  ** round-robin between the first (pSorter->nTask-1) tasks. Except, if
  ** the background thread from a sub-tasks previous turn is still running,
  ** skip it. If the first (pSorter->nTask-1) sub-tasks are all still busy,
  ** fall back to using the final sub-task. The first (pSorter->nTask-1)
  ** sub-tasks are preferred as they use background threads - the final
  ** sub-task uses the main thread. */
  ਲਈ(i=0; i<n_ਕਾਮਾ; i++){
    ਪੂਰਨ iTest = (pSorter->iPrev + i + 1) % n_ਕਾਮਾ;
    pTask = &pSorter->aTask[iTest];
    ਜੇ( pTask->b_ਪੂਰਾ ){
      rc = vdbeSorterJoinThread(pTask);
    }
    ਜੇ( rc!=SQLITE_OK || pTask->pThread==0 ) ਰੋਕੋ;
  }

  ਜੇ( rc==SQLITE_OK ){
    ਜੇ( i==n_ਕਾਮਾ ){
      /* Use the foreground thread for this operation */
      rc = vdbe_Sorter_List_ਮੰਜ਼ਿਲ_PMA(&pSorter->aTask[n_ਕਾਮਾ], &pSorter->list);
    }ਨਹੀਂਤਾਂ{
      /* Launch a background thread for this operation */
      u8 *aMem;
      ਖਾਲੀ *pCtx;

      ਪ੍ਰਤਿਜ੍ਞਾ( pTask!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pTask->pThread==0 && pTask->b_ਪੂਰਾ==0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pTask->list.pList==0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pTask->list.a_ਸਮ੍ਰਿਤੀ==0 || pSorter->list.a_ਸਮ੍ਰਿਤੀ!=0 );

      aMem = pTask->list.a_ਸਮ੍ਰਿਤੀ;
      pCtx = (ਖਾਲੀ*)pTask;
      pSorter->iPrev = (u8)(pTask - pSorter->aTask);
      pTask->list = pSorter->list;
      pSorter->list.pList = 0;
      pSorter->list.szPMA = 0;
      ਜੇ( aMem ){
        pSorter->list.a_ਸਮ੍ਰਿਤੀ = aMem;
        pSorter->n_ਸਮ੍ਰਿਤੀ = sqlite3_Malloc_ਮਾਪ(aMem);
      }ਨਹੀਂਤਾਂ ਜੇ( pSorter->list.a_ਸਮ੍ਰਿਤੀ ){
        pSorter->list.a_ਸਮ੍ਰਿਤੀ = sqlite3Malloc(pSorter->n_ਸਮ੍ਰਿਤੀ);
        ਜੇ( !pSorter->list.a_ਸਮ੍ਰਿਤੀ ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
      }

      rc = vdbeSorterCreateThread(pTask, vdbeSorterFlushThread, pCtx);
    }
  }

  ਵਾਪਸ rc;
#ਜੇ_ਅੰਤ /* SQLITE_MAX_WORKER_THREADS!=0 */
}

/*
** Add a record to the sorter.
*/
ਪੂਰਨ sqlite3VdbeSorterWrite(
  ਅਚਲ VdbeCursor *pCsr,         /* Sorter cursor */
  Mem *pVal                       /* Memory cell containing record */
){
  VdbeSorter *pSorter;
  ਪੂਰਨ rc = SQLITE_OK;             /* Return Code */
  Sorter_ਰਿਕਾਰਡ *pNew;             /* New list element */
  ਪੂਰਨ bFlush;                     /* True to flush contents of memory to PMA */
  i64 nReq;                       /* Bytes of memory required */
  i64 nPMA;                       /* Bytes of PMA space required */
  ਪੂਰਨ t;                          /* serial type of first record field */

  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  getVarint32NR((ਅਚਲ u8*)&pVal->z[1], t);
  ਜੇ( t>0 && t<10 ){
    ਜੇ( t==7 ){
      pSorter->type_ਨਕਾਬ &= SORTER_TYPE_REAL;
    }ਨਹੀਂਤਾਂ{
      pSorter->type_ਨਕਾਬ &= (SORTER_TYPE_INTEGER|SORTER_TYPE_REAL);
    }
  }ਨਹੀਂਤਾਂ ਜੇ( t>10 && (t & 0x01) ){
    pSorter->type_ਨਕਾਬ &= SORTER_TYPE_ਪਾਠ;
  }ਨਹੀਂਤਾਂ{
    pSorter->type_ਨਕਾਬ = 0;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( pSorter );

  /* Figure out whether or not the current contents of memory should be
  ** flushed to a PMA before continuing. If so, do so.
  **
  ** If using the single large allocation mode (pSorter->aMemory!=0), then
  ** flush the contents of memory to a new PMA if (a) at least one value is
  ** already in memory and (b) the new value will not fit in memory.
  **
  ** Or, if using separate allocations for each record, flush the contents
  ** of memory to a PMA if either of the following are true:
  **
  **   * The total memory allocated for the in-memory list is greater
  **     than (page-size * cache-size), or
  **
  **   * The total memory allocated for the in-memory list is greater
  **     than (page-size * 10) and sqlite3HeapNearlyFull() returns true.
  */
  nReq = pVal->n + ਆਕਾਰ(Sorter_ਰਿਕਾਰਡ);
  nPMA = pVal->n + sqlite3VarintLen(pVal->n);
  ਜੇ( pSorter->mx_Pma_ਮਾਪ ){
    ਜੇ( pSorter->list.a_ਸਮ੍ਰਿਤੀ ){
      bFlush = pSorter->i_ਸਮ੍ਰਿਤੀ && (pSorter->i_ਸਮ੍ਰਿਤੀ+nReq) > pSorter->mx_Pma_ਮਾਪ;
    }ਨਹੀਂਤਾਂ{
      bFlush = (
          (pSorter->list.szPMA > pSorter->mx_Pma_ਮਾਪ)
       || (pSorter->list.szPMA > pSorter->mn_Pma_ਮਾਪ && sqlite3_ਢੇਰ_Nearly_ਭਰਿਆ())
      );
    }
    ਜੇ( bFlush ){
      rc = vdbeSorterFlushPMA(pSorter);
      pSorter->list.szPMA = 0;
      pSorter->i_ਸਮ੍ਰਿਤੀ = 0;
      ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || pSorter->list.pList==0 );
    }
  }

  pSorter->list.szPMA += nPMA;
  ਜੇ( nPMA>pSorter->mxKeysize ){
    pSorter->mxKeysize = nPMA;
  }

  ਜੇ( pSorter->list.a_ਸਮ੍ਰਿਤੀ ){
    ਪੂਰਨ nMin = pSorter->i_ਸਮ੍ਰਿਤੀ + nReq;

    ਜੇ( nMin>pSorter->n_ਸਮ੍ਰਿਤੀ ){
      u8 *aNew;
      sqlite3_int64 nNew = 2 * (sqlite3_int64)pSorter->n_ਸਮ੍ਰਿਤੀ;
      ਪੂਰਨ iListOff = -1;
      ਜੇ( pSorter->list.pList ){
        iListOff = (u8*)pSorter->list.pList - pSorter->list.a_ਸਮ੍ਰਿਤੀ;
      }
      ਜਦੋਂ( nNew < nMin ) nNew = nNew*2;
      ਜੇ( nNew > pSorter->mx_Pma_ਮਾਪ ) nNew = pSorter->mx_Pma_ਮਾਪ;
      ਜੇ( nNew < nMin ) nNew = nMin;
      aNew = sqlite3Realloc(pSorter->list.a_ਸਮ੍ਰਿਤੀ, nNew);
      ਜੇ( !aNew ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
      ਜੇ( iListOff>=0 ){
        pSorter->list.pList = (Sorter_ਰਿਕਾਰਡ*)&aNew[iListOff];
      }
      pSorter->list.a_ਸਮ੍ਰਿਤੀ = aNew;
      pSorter->n_ਸਮ੍ਰਿਤੀ = nNew;
    }

    pNew = (Sorter_ਰਿਕਾਰਡ*)&pSorter->list.a_ਸਮ੍ਰਿਤੀ[pSorter->i_ਸਮ੍ਰਿਤੀ];
    pSorter->i_ਸਮ੍ਰਿਤੀ += ROUND8(nReq);
    ਜੇ( pSorter->list.pList ){
      pNew->u.i_ਅਗਲਾ = (ਪੂਰਨ)((u8*)(pSorter->list.pList) - pSorter->list.a_ਸਮ੍ਰਿਤੀ);
    }
  }ਨਹੀਂਤਾਂ{
    pNew = (Sorter_ਰਿਕਾਰਡ *)sqlite3Malloc(nReq);
    ਜੇ( pNew==0 ){
      ਵਾਪਸ SQLITE_NOMEM_BKPT;
    }
    pNew->u.p_ਅਗਲਾ = pSorter->list.pList;
  }

  ਸਮ੍ਰਿਤੀ_ਨਕਲ(SRVAL(pNew), pVal->z, pVal->n);
  pNew->nVal = pVal->n;
  pSorter->list.pList = pNew;

  ਵਾਪਸ rc;
}

/*
** Read keys from pIncr->pMerger and populate pIncr->aFile[1]. The format
** of the data stored in aFile[1] is the same as that used by regular PMAs,
** except that the number-of-bytes varint is omitted from the start.
*/
ਸਥਿਰ ਪੂਰਨ vdbeIncrPopulate(IncrMerger *pIncr){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ rc2;
  i64 i_ਸ਼ੁਰੂ = pIncr->i_ਸ਼ੁਰੂ_Off;
  SorterFile *p_ਬਾਹਰਲਾ = &pIncr->aFile[1];
  ਛਾਂਟੋ_Subtask *pTask = pIncr->pTask;
  ਮਿਲਾਓ_Engine *pMerger = pIncr->pMerger;
  PmaWriter writer;
  ਪ੍ਰਤਿਜ੍ਞਾ( pIncr->bEof==0 );

  vdbeSorterPopulateDebug(pTask, "enter");

  vdbePmaWriterInit(p_ਬਾਹਰਲਾ->pFd, &writer, pTask->pSorter->pgsz, i_ਸ਼ੁਰੂ);
  ਜਦੋਂ( rc==SQLITE_OK ){
    ਪੂਰਨ dummy;
    PmaReader *pReader = &pMerger->aReadr[ pMerger->a_ਰੁੱਖ[1] ];
    ਪੂਰਨ n_ਕੁੰਜੀ = pReader->n_ਕੁੰਜੀ;
    i64 iEof = writer.iWriteOff + writer.i_Buf_ਸਿਰਾ;

    /* Check if the output file is full or if the input has been exhausted.
    ** In either case exit the loop. */
    ਜੇ( pReader->pFd==0 ) ਰੋਕੋ;
    ਜੇ( (iEof + n_ਕੁੰਜੀ + sqlite3VarintLen(n_ਕੁੰਜੀ))>(i_ਸ਼ੁਰੂ + pIncr->mxSz) ) ਰੋਕੋ;

    /* Write the next key to the output. */
    vdbePmaWriteVarint(&writer, n_ਕੁੰਜੀ);
    vdbePmaWriteBlob(&writer, pReader->a_ਕੁੰਜੀ, n_ਕੁੰਜੀ);
    ਪ੍ਰਤਿਜ੍ਞਾ( pIncr->pMerger->pTask==pTask );
    rc = vdbe_ਮਿਲਾਓ_Engine_ਕਦਮ(pIncr->pMerger, &dummy);
  }

  rc2 = vdbe_Pma_Writer_ਅੰਤ(&writer, &p_ਬਾਹਰਲਾ->iEof, &pTask->nSpill);
  ਜੇ( rc==SQLITE_OK ) rc = rc2;
  vdbeSorterPopulateDebug(pTask, "exit");
  ਵਾਪਸ rc;
}

#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
/*
** The main routine for background threads that populate aFile[1] of
** multi-threaded IncrMerger objects.
*/
ਸਥਿਰ ਖਾਲੀ *vdbeIncrPopulateThread(ਖਾਲੀ *pCtx){
  IncrMerger *pIncr = (IncrMerger*)pCtx;
  ਖਾਲੀ *pRet = SQLITE_INT_ਮੰਜ਼ਿਲ_PTR( vdbeIncrPopulate(pIncr) );
  pIncr->pTask->b_ਪੂਰਾ = 1;
  ਵਾਪਸ pRet;
}

/*
** Launch a background thread to populate aFile[1] of pIncr.
*/
ਸਥਿਰ ਪੂਰਨ vdbeIncrBgPopulate(IncrMerger *pIncr){
  ਖਾਲੀ *p = (ਖਾਲੀ*)pIncr;
  ਪ੍ਰਤਿਜ੍ਞਾ( pIncr->bUseThread );
  ਵਾਪਸ vdbeSorterCreateThread(pIncr->pTask, vdbeIncrPopulateThread, p);
}
#ਜੇ_ਅੰਤ

/*
** This function is called when the PmaReader corresponding to pIncr has
** finished reading the contents of aFile[0]. Its purpose is to "refill"
** aFile[0] such that the PmaReader should start rereading it from the
** beginning.
**
** For single-threaded objects, this is accomplished by literally reading
** keys from pIncr->pMerger and repopulating aFile[0].
**
** For multi-threaded objects, all that is required is to wait until the
** background thread is finished (if it is not already) and then swap
** aFile[0] and aFile[1] in place. If the contents of pMerger have not
** been exhausted, this function also launches a new background thread
** to populate the new aFile[1].
**
** SQLITE_OK is returned on success, or an SQLite error code otherwise.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Incr_ਅਦਲਾ_ਬਦਲੀ(IncrMerger *pIncr){
  ਪੂਰਨ rc = SQLITE_OK;

#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
  ਜੇ( pIncr->bUseThread ){
    rc = vdbeSorterJoinThread(pIncr->pTask);

    ਜੇ( rc==SQLITE_OK ){
      SorterFile f0 = pIncr->aFile[0];
      pIncr->aFile[0] = pIncr->aFile[1];
      pIncr->aFile[1] = f0;
    }

    ਜੇ( rc==SQLITE_OK ){
      ਜੇ( pIncr->aFile[0].iEof==pIncr->i_ਸ਼ੁਰੂ_Off ){
        pIncr->bEof = 1;
      }ਨਹੀਂਤਾਂ{
        rc = vdbeIncrBgPopulate(pIncr);
      }
    }
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
  {
    rc = vdbeIncrPopulate(pIncr);
    pIncr->aFile[0] = pIncr->aFile[1];
    ਜੇ( pIncr->aFile[0].iEof==pIncr->i_ਸ਼ੁਰੂ_Off ){
      pIncr->bEof = 1;
    }
  }

  ਵਾਪਸ rc;
}

/*
** Allocate and return a new IncrMerger object to read data from pMerger.
**
** If an OOM condition is encountered, return NULL. In this case free the
** pMerger argument before returning.
*/
ਸਥਿਰ ਪੂਰਨ vdbeIncrMergerNew(
  ਛਾਂਟੋ_Subtask *pTask,     /* The thread that will be using the new IncrMerger */
  ਮਿਲਾਓ_Engine *pMerger,   /* The MergeEngine that the IncrMerger will control */
  IncrMerger **pp_ਬਾਹਰਲਾ      /* Write the new IncrMerger here */
){
  ਪੂਰਨ rc = SQLITE_OK;
  IncrMerger *pIncr = *pp_ਬਾਹਰਲਾ = (IncrMerger*)
       (sqlite3FaultSim(100) ? 0 : sqlite3MallocZero(ਆਕਾਰ(*pIncr)));
  ਜੇ( pIncr ){
    pIncr->pMerger = pMerger;
    pIncr->pTask = pTask;
    pIncr->mxSz = MAX(pTask->pSorter->mxKeysize+9,pTask->pSorter->mx_Pma_ਮਾਪ/2);
    pTask->file2.iEof += pIncr->mxSz;
  }ਨਹੀਂਤਾਂ{
    vdbe_ਮਿਲਾਓ_Engine_Free(pMerger);
    rc = SQLITE_NOMEM_BKPT;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( *pp_ਬਾਹਰਲਾ!=0 || rc!=SQLITE_OK );
  ਵਾਪਸ rc;
}

#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
/*
** Set the "use-threads" flag on object pIncr.
*/
ਸਥਿਰ ਖਾਲੀ vdbe_Incr_Merger_ਸੈੱਟ_ਧਾਗੇ(IncrMerger *pIncr){
  pIncr->bUseThread = 1;
  pIncr->pTask->file2.iEof -= pIncr->mxSz;
}
#ਜੇ_ਅੰਤ /* SQLITE_MAX_WORKER_THREADS>0 */



/*
** Recompute pMerger->aTree[iOut] by comparing the next keys on the
** two PmaReaders that feed that entry.  Neither of the PmaReaders
** are advanced.  This routine merely does the comparison.
*/
ਸਥਿਰ ਖਾਲੀ vdbe_ਮਿਲਾਓ_Engine_Compare(
  ਮਿਲਾਓ_Engine *pMerger,  /* Merge engine containing PmaReaders to compare */
  ਪੂਰਨ i_ਬਾਹਰਲਾ               /* Store the result in pMerger->aTree[iOut] */
){
  ਪੂਰਨ i1;
  ਪੂਰਨ i2;
  ਪੂਰਨ iRes;
  PmaReader *p1;
  PmaReader *p2;

  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਬਾਹਰਲਾ<pMerger->n_ਰੁੱਖ && i_ਬਾਹਰਲਾ>0 );

  ਜੇ( i_ਬਾਹਰਲਾ>=(pMerger->n_ਰੁੱਖ/2) ){
    i1 = (i_ਬਾਹਰਲਾ - pMerger->n_ਰੁੱਖ/2) * 2;
    i2 = i1 + 1;
  }ਨਹੀਂਤਾਂ{
    i1 = pMerger->a_ਰੁੱਖ[i_ਬਾਹਰਲਾ*2];
    i2 = pMerger->a_ਰੁੱਖ[i_ਬਾਹਰਲਾ*2+1];
  }

  p1 = &pMerger->aReadr[i1];
  p2 = &pMerger->aReadr[i2];

  ਜੇ( p1->pFd==0 ){
    iRes = i2;
  }ਨਹੀਂਤਾਂ ਜੇ( p2->pFd==0 ){
    iRes = i1;
  }ਨਹੀਂਤਾਂ{
    ਛਾਂਟੋ_Subtask *pTask = pMerger->pTask;
    ਪੂਰਨ bCached = 0;
    ਪੂਰਨ res;
    ਪ੍ਰਤਿਜ੍ਞਾ( pTask->pUnpacked!=0 );  /* from vdbeSortSubtaskMain() */
    res = pTask->xCompare(
        pTask, &bCached, p1->a_ਕੁੰਜੀ, p1->n_ਕੁੰਜੀ, p2->a_ਕੁੰਜੀ, p2->n_ਕੁੰਜੀ
    );
    ਜੇ( res<=0 ){
      iRes = i1;
    }ਨਹੀਂਤਾਂ{
      iRes = i2;
    }
  }

  pMerger->a_ਰੁੱਖ[i_ਬਾਹਰਲਾ] = iRes;
}

/*
** Allowed values for the eMode parameter to vdbeMergeEngineInit()
** and vdbePmaReaderIncrMergeInit().
**
** Only INCRINIT_NORMAL is valid in single-threaded builds (when
** SQLITE_MAX_WORKER_THREADS==0).  The other values are only used
** when there exists one or more separate worker threads.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INCRINIT_ਸਾਧਾਰਨ 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INCRINIT_TASK   1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INCRINIT_ਜੜ੍ਹ   2

/*
** Forward reference required as the vdbeIncrMergeInit() and
** vdbePmaReaderIncrInit() routines are called mutually recursively when
** building a merge tree.
*/
ਸਥਿਰ ਪੂਰਨ vdbePmaReaderIncrInit(PmaReader *pReadr, ਪੂਰਨ e_ਬਹੁਲਕ);

/*
** Initialize the MergeEngine object passed as the second argument. Once this
** function returns, the first key of merged data may be read from the
** MergeEngine object in the usual fashion.
**
** If argument eMode is INCRINIT_ROOT, then it is assumed that any IncrMerge
** objects attached to the PmaReader objects that the merger reads from have
** already been populated, but that they have not yet populated aFile[0] and
** set the PmaReader objects up to read from it. In this case all that is
** required is to call vdbePmaReaderNext() on each PmaReader to point it at
** its first key.
**
** Otherwise, if eMode is any value other than INCRINIT_ROOT, then use
** vdbePmaReaderIncrMergeInit() to initialize each PmaReader that feeds data
** to pMerger.
**
** SQLITE_OK is returned if successful, or an SQLite error code otherwise.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_ਮਿਲਾਓ_Engine_Init(
  ਛਾਂਟੋ_Subtask *pTask,             /* Thread that will run pMerger */
  ਮਿਲਾਓ_Engine *pMerger,           /* MergeEngine to initialize */
  ਪੂਰਨ e_ਬਹੁਲਕ                       /* One of the INCRINIT_XXX constants */
){
  ਪੂਰਨ rc = SQLITE_OK;             /* Return code */
  ਪੂਰਨ i;                          /* For looping over PmaReader objects */
  ਪੂਰਨ n_ਰੁੱਖ;                      /* Number of subtrees to merge */

  /* Failure to allocate the merge would have been detected prior to
  ** invoking this routine */
  ਪ੍ਰਤਿਜ੍ਞਾ( pMerger!=0 );

  /* eMode is always INCRINIT_NORMAL in single-threaded mode */
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0 || e_ਬਹੁਲਕ==INCRINIT_ਸਾਧਾਰਨ );

  /* Verify that the MergeEngine is assigned to a single thread */
  ਪ੍ਰਤਿਜ੍ਞਾ( pMerger->pTask==0 );
  pMerger->pTask = pTask;

  n_ਰੁੱਖ = pMerger->n_ਰੁੱਖ;
  ਲਈ(i=0; i<n_ਰੁੱਖ; i++){
    ਜੇ( SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0 && e_ਬਹੁਲਕ==INCRINIT_ਜੜ੍ਹ ){
      /* PmaReaders should be normally initialized in order, as if they are
      ** reading from the same temp file this makes for more linear file IO.
      ** However, in the INCRINIT_ROOT case, if PmaReader aReadr[nTask-1] is
      ** in use it will block the vdbePmaReaderNext() call while it uses
      ** the main thread to fill its buffer. So calling PmaReaderNext()
      ** on this PmaReader before any of the multi-threaded PmaReaders takes
      ** better advantage of multi-processor hardware. */
      rc = vdbe_Pma_Reader_ਅਗਲਾ(&pMerger->aReadr[n_ਰੁੱਖ-i-1]);
    }ਨਹੀਂਤਾਂ{
      rc = vdbePmaReaderIncrInit(&pMerger->aReadr[i], INCRINIT_ਸਾਧਾਰਨ);
    }
    ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
  }

  ਲਈ(i=pMerger->n_ਰੁੱਖ-1; i>0; i--){
    vdbe_ਮਿਲਾਓ_Engine_Compare(pMerger, i);
  }
  ਵਾਪਸ pTask->pUnpacked->err_ਕੋਡ;
}

/*
** The PmaReader passed as the first argument is guaranteed to be an
** incremental-reader (pReadr->pIncr!=0). This function serves to open
** and/or initialize the temp file related fields of the IncrMerge
** object at (pReadr->pIncr).
**
** If argument eMode is set to INCRINIT_NORMAL, then all PmaReaders
** in the sub-tree headed by pReadr are also initialized. Data is then
** loaded into the buffers belonging to pReadr and it is set to point to
** the first key in its range.
**
** If argument eMode is set to INCRINIT_TASK, then pReadr is guaranteed
** to be a multi-threaded PmaReader and this function is being called in a
** background thread. In this case all PmaReaders in the sub-tree are
** initialized as for INCRINIT_NORMAL and the aFile[1] buffer belonging to
** pReadr is populated. However, pReadr itself is not set up to point
** to its first key. A call to vdbePmaReaderNext() is still required to do
** that.
**
** The reason this function does not call vdbePmaReaderNext() immediately
** in the INCRINIT_TASK case is that vdbePmaReaderNext() assumes that it has
** to block on thread (pTask->thread) before accessing aFile[1]. But, since
** this entire function is being run by thread (pTask->thread), that will
** lead to the current background thread attempting to join itself.
**
** Finally, if argument eMode is set to INCRINIT_ROOT, it may be assumed
** that pReadr->pIncr is a multi-threaded IncrMerge objects, and that all
** child-trees have already been initialized using IncrInit(INCRINIT_TASK).
** In this case vdbePmaReaderNext() is called on all child PmaReaders and
** the current PmaReader set to point to the first key in its range.
**
** SQLITE_OK is returned if successful, or an SQLite error code otherwise.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Pma_Reader_Incr_ਮਿਲਾਓ_Init(PmaReader *pReadr, ਪੂਰਨ e_ਬਹੁਲਕ){
  ਪੂਰਨ rc = SQLITE_OK;
  IncrMerger *pIncr = pReadr->pIncr;
  ਛਾਂਟੋ_Subtask *pTask = pIncr->pTask;
  sqlite3 *db = pTask->pSorter->db;

  /* eMode is always INCRINIT_NORMAL in single-threaded mode */
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0 || e_ਬਹੁਲਕ==INCRINIT_ਸਾਧਾਰਨ );

  rc = vdbe_ਮਿਲਾਓ_Engine_Init(pTask, pIncr->pMerger, e_ਬਹੁਲਕ);

  /* Set up the required files for pIncr. A multi-threaded IncrMerge object
  ** requires two temp files to itself, whereas a single-threaded object
  ** only requires a region of pTask->file2. */
  ਜੇ( rc==SQLITE_OK ){
    ਪੂਰਨ mxSz = pIncr->mxSz;
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
    ਜੇ( pIncr->bUseThread ){
      rc = vdbe_Sorter_Open_ਆਰਜ਼ੀ_File(db, mxSz, &pIncr->aFile[0].pFd);
      ਜੇ( rc==SQLITE_OK ){
        rc = vdbe_Sorter_Open_ਆਰਜ਼ੀ_File(db, mxSz, &pIncr->aFile[1].pFd);
      }
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    /*if( !pIncr->bUseThread )*/{
      ਜੇ( pTask->file2.pFd==0 ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pTask->file2.iEof>0 );
        rc = vdbe_Sorter_Open_ਆਰਜ਼ੀ_File(db, pTask->file2.iEof, &pTask->file2.pFd);
        pTask->file2.iEof = 0;
      }
      ਜੇ( rc==SQLITE_OK ){
        pIncr->aFile[1].pFd = pTask->file2.pFd;
        pIncr->i_ਸ਼ੁਰੂ_Off = pTask->file2.iEof;
        pTask->file2.iEof += mxSz;
      }
    }
  }

#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
  ਜੇ( rc==SQLITE_OK && pIncr->bUseThread ){
    /* Use the current thread to populate aFile[1], even though this
    ** PmaReader is multi-threaded. If this is an INCRINIT_TASK object,
    ** then this function is already running in background thread
    ** pIncr->pTask->thread.
    **
    ** If this is the INCRINIT_ROOT object, then it is running in the
    ** main VDBE thread. But that is Ok, as that thread cannot return
    ** control to the VDBE or proceed with anything useful until the
    ** first results are ready from this merger object anyway.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( e_ਬਹੁਲਕ==INCRINIT_ਜੜ੍ਹ || e_ਬਹੁਲਕ==INCRINIT_TASK );
    rc = vdbeIncrPopulate(pIncr);
  }
#ਜੇ_ਅੰਤ

  ਜੇ( rc==SQLITE_OK && (SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ==0 || e_ਬਹੁਲਕ!=INCRINIT_TASK) ){
    rc = vdbe_Pma_Reader_ਅਗਲਾ(pReadr);
  }

  ਵਾਪਸ rc;
}

#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
/*
** The main routine for vdbePmaReaderIncrMergeInit() operations run in
** background threads.
*/
ਸਥਿਰ ਖਾਲੀ *vdbePmaReaderBgIncrInit(ਖਾਲੀ *pCtx){
  PmaReader *pReader = (PmaReader*)pCtx;
  ਖਾਲੀ *pRet = SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(
                  vdbe_Pma_Reader_Incr_ਮਿਲਾਓ_Init(pReader,INCRINIT_TASK)
               );
  pReader->pIncr->pTask->b_ਪੂਰਾ = 1;
  ਵਾਪਸ pRet;
}
#ਜੇ_ਅੰਤ

/*
** If the PmaReader passed as the first argument is not an incremental-reader
** (if pReadr->pIncr==0), then this function is a no-op. Otherwise, it invokes
** the vdbePmaReaderIncrMergeInit() function with the parameters passed to
** this routine to initialize the incremental merge.
**
** If the IncrMerger object is multi-threaded (IncrMerger.bUseThread==1),
** then a background thread is launched to call vdbePmaReaderIncrMergeInit().
** Or, if the IncrMerger is single threaded, the same function is called
** using the current thread.
*/
ਸਥਿਰ ਪੂਰਨ vdbePmaReaderIncrInit(PmaReader *pReadr, ਪੂਰਨ e_ਬਹੁਲਕ){
  IncrMerger *pIncr = pReadr->pIncr;   /* Incremental merger */
  ਪੂਰਨ rc = SQLITE_OK;                  /* Return code */
  ਜੇ( pIncr ){
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
    ਪ੍ਰਤਿਜ੍ਞਾ( pIncr->bUseThread==0 || e_ਬਹੁਲਕ==INCRINIT_TASK );
    ਜੇ( pIncr->bUseThread ){
      ਖਾਲੀ *pCtx = (ਖਾਲੀ*)pReadr;
      rc = vdbeSorterCreateThread(pIncr->pTask, vdbePmaReaderBgIncrInit, pCtx);
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      rc = vdbe_Pma_Reader_Incr_ਮਿਲਾਓ_Init(pReadr, e_ਬਹੁਲਕ);
    }
  }
  ਵਾਪਸ rc;
}

/*
** Allocate a new MergeEngine object to merge the contents of nPMA level-0
** PMAs from pTask->file. If no error occurs, set *ppOut to point to
** the new object and return SQLITE_OK. Or, if an error does occur, set *ppOut
** to NULL and return an SQLite error code.
**
** When this function is called, *piOffset is set to the offset of the
** first PMA to read from pTask->file. Assuming no error occurs, it is
** set to the offset immediately following the last byte of the last
** PMA before returning. If an error does occur, then the final value of
** *piOffset is undefined.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_ਮਿਲਾਓ_Engine_Level0(
  ਛਾਂਟੋ_Subtask *pTask,             /* Sorter task to read from */
  ਪੂਰਨ nPMA,                       /* Number of PMAs to read */
  i64 *pi_ਵਿੱਥ,                  /* IN/OUT: Readr offset in pTask->file */
  ਮਿਲਾਓ_Engine **pp_ਬਾਹਰਲਾ             /* OUT: New merge-engine */
){
  ਮਿਲਾਓ_Engine *pNew;              /* Merge engine to return */
  i64 iOff = *pi_ਵਿੱਥ;
  ਪੂਰਨ i;
  ਪੂਰਨ rc = SQLITE_OK;

  *pp_ਬਾਹਰਲਾ = pNew = vdbe_ਮਿਲਾਓ_Engine_New(nPMA);
  ਜੇ( pNew==0 ) rc = SQLITE_NOMEM_BKPT;

  ਲਈ(i=0; i<nPMA && rc==SQLITE_OK; i++){
    i64 nDummy = 0;
    PmaReader *pReadr = &pNew->aReadr[i];
    rc = vdbePmaReaderInit(pTask, &pTask->file, iOff, pReadr, &nDummy);
    iOff = pReadr->iEof;
  }

  ਜੇ( rc!=SQLITE_OK ){
    vdbe_ਮਿਲਾਓ_Engine_Free(pNew);
    *pp_ਬਾਹਰਲਾ = 0;
  }
  *pi_ਵਿੱਥ = iOff;
  ਵਾਪਸ rc;
}

/*
** Return the depth of a tree comprising nPMA PMAs, assuming a fanout of
** SORTER_MAX_MERGE_COUNT. The returned value does not include leaf nodes.
**
** i.e.
**
**   nPMA<=16    -> TreeDepth() == 0
**   nPMA<=256   -> TreeDepth() == 1
**   nPMA<=65536 -> TreeDepth() == 2
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Sorter_ਰੁੱਖ_ਡੂੰਘਾਈ(ਪੂਰਨ nPMA){
  ਪੂਰਨ n_ਡੂੰਘਾਈ = 0;
  i64 nDiv = SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ;
  ਜਦੋਂ( nDiv < (i64)nPMA ){
    nDiv = nDiv * SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ;
    n_ਡੂੰਘਾਈ++;
  }
  ਵਾਪਸ n_ਡੂੰਘਾਈ;
}

/*
** pRoot is the root of an incremental merge-tree with depth nDepth (according
** to vdbeSorterTreeDepth()). pLeaf is the iSeq'th leaf to be added to the
** tree, counting from zero. This function adds pLeaf to the tree.
**
** If successful, SQLITE_OK is returned. If an error occurs, an SQLite error
** code is returned and pLeaf is freed.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Sorter_ਜੋੜੋ_ਮੰਜ਼ਿਲ_ਰੁੱਖ(
  ਛਾਂਟੋ_Subtask *pTask,             /* Task context */
  ਪੂਰਨ n_ਡੂੰਘਾਈ,                     /* Depth of tree according to TreeDepth() */
  ਪੂਰਨ iSeq,                       /* Sequence number of leaf within tree */
  ਮਿਲਾਓ_Engine *p_ਜੜ੍ਹ,             /* Root of tree */
  ਮਿਲਾਓ_Engine *p_ਪੱਤੀ              /* Leaf to add to tree */
){
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ nDiv = 1;
  ਪੂਰਨ i;
  ਮਿਲਾਓ_Engine *p = p_ਜੜ੍ਹ;
  IncrMerger *pIncr;

  rc = vdbeIncrMergerNew(pTask, p_ਪੱਤੀ, &pIncr);

  ਲਈ(i=1; i<n_ਡੂੰਘਾਈ; i++){
    nDiv = nDiv * SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ;
  }

  ਲਈ(i=1; i<n_ਡੂੰਘਾਈ && rc==SQLITE_OK; i++){
    ਪੂਰਨ iIter = (iSeq / nDiv) % SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ;
    PmaReader *pReadr = &p->aReadr[iIter];

    ਜੇ( pReadr->pIncr==0 ){
      ਮਿਲਾਓ_Engine *pNew = vdbe_ਮਿਲਾਓ_Engine_New(SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ);
      ਜੇ( pNew==0 ){
        rc = SQLITE_NOMEM_BKPT;
      }ਨਹੀਂਤਾਂ{
        rc = vdbeIncrMergerNew(pTask, pNew, &pReadr->pIncr);
      }
    }
    ਜੇ( rc==SQLITE_OK ){
      p = pReadr->pIncr->pMerger;
      nDiv = nDiv / SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ;
    }
  }

  ਜੇ( rc==SQLITE_OK ){
    p->aReadr[iSeq % SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ].pIncr = pIncr;
  }ਨਹੀਂਤਾਂ{
    vdbeIncrFree(pIncr);
  }
  ਵਾਪਸ rc;
}

/*
** This function is called as part of a SorterRewind() operation on a sorter
** that has already written two or more level-0 PMAs to one or more temp
** files. It builds a tree of MergeEngine/IncrMerger/PmaReader objects that
** can be used to incrementally merge all PMAs on disk.
**
** If successful, SQLITE_OK is returned and *ppOut set to point to the
** MergeEngine object at the root of the tree before returning. Or, if an
** error occurs, an SQLite error code is returned and the final value
** of *ppOut is undefined.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Sorter_ਮਿਲਾਓ_ਰੁੱਖ_Build(
  VdbeSorter *pSorter,       /* The VDBE cursor that implements the sort */
  ਮਿਲਾਓ_Engine **pp_ਬਾਹਰਲਾ        /* Write the MergeEngine here */
){
  ਮਿਲਾਓ_Engine *pMain = 0;
  ਪੂਰਨ rc = SQLITE_OK;
  ਪੂਰਨ iTask;

#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
  /* If the sorter uses more than one task, then create the top-level
  ** MergeEngine here. This MergeEngine will read data from exactly
  ** one PmaReader per sub-task.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( pSorter->b_Use_ਧਾਗੇ || pSorter->nTask==1 );
  ਜੇ( pSorter->nTask>1 ){
    pMain = vdbe_ਮਿਲਾਓ_Engine_New(pSorter->nTask);
    ਜੇ( pMain==0 ) rc = SQLITE_NOMEM_BKPT;
  }
#ਜੇ_ਅੰਤ

  ਲਈ(iTask=0; rc==SQLITE_OK && iTask<pSorter->nTask; iTask++){
    ਛਾਂਟੋ_Subtask *pTask = &pSorter->aTask[iTask];
    ਪ੍ਰਤਿਜ੍ਞਾ( pTask->nPMA>0 || SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0 );
    ਜੇ( SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ==0 || pTask->nPMA ){
      ਮਿਲਾਓ_Engine *p_ਜੜ੍ਹ = 0;     /* Root node of tree for this task */
      ਪੂਰਨ n_ਡੂੰਘਾਈ = vdbe_Sorter_ਰੁੱਖ_ਡੂੰਘਾਈ(pTask->nPMA);
      i64 iReadOff = 0;

      ਜੇ( pTask->nPMA<=SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ ){
        rc = vdbe_ਮਿਲਾਓ_Engine_Level0(pTask, pTask->nPMA, &iReadOff, &p_ਜੜ੍ਹ);
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ i;
        ਪੂਰਨ iSeq = 0;
        p_ਜੜ੍ਹ = vdbe_ਮਿਲਾਓ_Engine_New(SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ);
        ਜੇ( p_ਜੜ੍ਹ==0 ) rc = SQLITE_NOMEM_BKPT;
        ਲਈ(i=0; i<pTask->nPMA && rc==SQLITE_OK; i += SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ){
          ਮਿਲਾਓ_Engine *pMerger = 0; /* New level-0 PMA merger */
          ਪੂਰਨ nReader;              /* Number of level-0 PMAs to merge */

          nReader = MIN(pTask->nPMA - i, SORTER_MAX_ਮਿਲਾਓ_ਗਿਣਤੀ);
          rc = vdbe_ਮਿਲਾਓ_Engine_Level0(pTask, nReader, &iReadOff, &pMerger);
          ਜੇ( rc==SQLITE_OK ){
            rc = vdbe_Sorter_ਜੋੜੋ_ਮੰਜ਼ਿਲ_ਰੁੱਖ(pTask, n_ਡੂੰਘਾਈ, iSeq++, p_ਜੜ੍ਹ, pMerger);
          }
        }
      }

      ਜੇ( rc==SQLITE_OK ){
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
        ਜੇ( pMain!=0 ){
          rc = vdbeIncrMergerNew(pTask, p_ਜੜ੍ਹ, &pMain->aReadr[iTask].pIncr);
        }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
        {
          ਪ੍ਰਤਿਜ੍ਞਾ( pMain==0 );
          pMain = p_ਜੜ੍ਹ;
        }
      }ਨਹੀਂਤਾਂ{
        vdbe_ਮਿਲਾਓ_Engine_Free(p_ਜੜ੍ਹ);
      }
    }
  }

  ਜੇ( rc!=SQLITE_OK ){
    vdbe_ਮਿਲਾਓ_Engine_Free(pMain);
    pMain = 0;
  }
  *pp_ਬਾਹਰਲਾ = pMain;
  ਵਾਪਸ rc;
}

/*
** This function is called as part of an sqlite3VdbeSorterRewind() operation
** on a sorter that has written two or more PMAs to temporary files. It sets
** up either VdbeSorter.pMerger (for single threaded sorters) or pReader
** (for multi-threaded sorters) so that it can be used to iterate through
** all records stored in the sorter.
**
** SQLITE_OK is returned if successful, or an SQLite error code otherwise.
*/
ਸਥਿਰ ਪੂਰਨ vdbe_Sorter_ਤਿਆਰੀ_ਮਿਲਾਓ(VdbeSorter *pSorter){
  ਪੂਰਨ rc;                         /* Return code */
  ਛਾਂਟੋ_Subtask *pTask0 = &pSorter->aTask[0];
  ਮਿਲਾਓ_Engine *pMain = 0;
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ
  sqlite3 *db = pTask0->pSorter->db;
  ਪੂਰਨ i;
  SorterCompare xCompare = vdbeSorterGetCompare(pSorter);
  ਲਈ(i=0; i<pSorter->nTask; i++){
    pSorter->aTask[i].xCompare = xCompare;
  }
#ਜੇ_ਅੰਤ

  rc = vdbe_Sorter_ਮਿਲਾਓ_ਰੁੱਖ_Build(pSorter, &pMain);
  ਜੇ( rc==SQLITE_OK ){
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ
    ਪ੍ਰਤਿਜ੍ਞਾ( pSorter->b_Use_ਧਾਗੇ==0 || pSorter->nTask>1 );
    ਜੇ( pSorter->b_Use_ਧਾਗੇ ){
      ਪੂਰਨ iTask;
      PmaReader *pReadr = 0;
      ਛਾਂਟੋ_Subtask *p_ਆਖਰੀ = &pSorter->aTask[pSorter->nTask-1];
      rc = vdbe_ਛਾਂਟੋ_Alloc_Unpacked(p_ਆਖਰੀ);
      ਜੇ( rc==SQLITE_OK ){
        pReadr = (PmaReader*)sqlite3DbMallocZero(db, ਆਕਾਰ(PmaReader));
        pSorter->pReader = pReadr;
        ਜੇ( pReadr==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      ਜੇ( rc==SQLITE_OK ){
        rc = vdbeIncrMergerNew(p_ਆਖਰੀ, pMain, &pReadr->pIncr);
        ਜੇ( rc==SQLITE_OK ){
          vdbe_Incr_Merger_ਸੈੱਟ_ਧਾਗੇ(pReadr->pIncr);
          ਲਈ(iTask=0; iTask<(pSorter->nTask-1); iTask++){
            IncrMerger *pIncr;
            ਜੇ( (pIncr = pMain->aReadr[iTask].pIncr) ){
              vdbe_Incr_Merger_ਸੈੱਟ_ਧਾਗੇ(pIncr);
              ਪ੍ਰਤਿਜ੍ਞਾ( pIncr->pTask!=p_ਆਖਰੀ );
            }
          }
          ਲਈ(iTask=0; rc==SQLITE_OK && iTask<pSorter->nTask; iTask++){
            /* Check that:
            **  
            **   a) The incremental merge object is configured to use the
            **      right task, and
            **   b) If it is using task (nTask-1), it is configured to run
            **      in single-threaded mode. This is important, as the
            **      root merge (INCRINIT_ROOT) will be using the same task
            **      object.
            */
            PmaReader *p = &pMain->aReadr[iTask];
            ਪ੍ਰਤਿਜ੍ਞਾ( p->pIncr==0 || (
                (p->pIncr->pTask==&pSorter->aTask[iTask])             /* a */
             && (iTask!=pSorter->nTask-1 || p->pIncr->bUseThread==0)  /* b */
            ));
            rc = vdbePmaReaderIncrInit(p, INCRINIT_TASK);
          }
        }
        pMain = 0;
      }
      ਜੇ( rc==SQLITE_OK ){
        rc = vdbe_Pma_Reader_Incr_ਮਿਲਾਓ_Init(pReadr, INCRINIT_ਜੜ੍ਹ);
      }
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      rc = vdbe_ਮਿਲਾਓ_Engine_Init(pTask0, pMain, INCRINIT_ਸਾਧਾਰਨ);
      pSorter->pMerger = pMain;
      pMain = 0;
    }
  }

  ਜੇ( rc!=SQLITE_OK ){
    vdbe_ਮਿਲਾਓ_Engine_Free(pMain);
  }
  ਵਾਪਸ rc;
}


/*
** Once the sorter has been populated by calls to sqlite3VdbeSorterWrite,
** this function is called to prepare for iterating through the records
** in sorted order.
*/
ਪੂਰਨ sqlite3VdbeSorterRewind(ਅਚਲ VdbeCursor *pCsr, ਪੂਰਨ *pbEof){
  VdbeSorter *pSorter;
  ਪੂਰਨ rc = SQLITE_OK;             /* Return code */

  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSorter );

  /* If no data has been written to disk, then do not do so now. Instead,
  ** sort the VdbeSorter.pRecord list. The vdbe layer will read data directly
  ** from the in-memory list.  */
  ਜੇ( pSorter->bUsePMA==0 ){
    ਜੇ( pSorter->list.pList ){
      *pbEof = 0;
      rc = vdbe_Sorter_ਛਾਂਟੋ(&pSorter->aTask[0], &pSorter->list);
    }ਨਹੀਂਤਾਂ{
      *pbEof = 1;
    }
    ਵਾਪਸ rc;
  }

  /* Write the current in-memory list to a PMA. When the VdbeSorterWrite()
  ** function flushes the contents of memory to disk, it immediately always
  ** creates a new list consisting of a single key immediately afterwards.
  ** So the list is never empty at this point.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( pSorter->list.pList );
  rc = vdbeSorterFlushPMA(pSorter);

  /* Join all threads */
  rc = vdbeSorterJoinAll(pSorter, rc);

  vdbeSorterRewindDebug("rewind");

  /* Assuming no errors have occurred, set up a merger structure to
  ** incrementally read and merge all remaining PMAs.  */
  ਪ੍ਰਤਿਜ੍ਞਾ( pSorter->pReader==0 );
  ਜੇ( rc==SQLITE_OK ){
    rc = vdbe_Sorter_ਤਿਆਰੀ_ਮਿਲਾਓ(pSorter);
    *pbEof = 0;
  }

  vdbeSorterRewindDebug("rewinddone");
  ਵਾਪਸ rc;
}

/*
** Advance to the next element in the sorter.  Return value:
**
**    SQLITE_OK     success
**    SQLITE_DONE   end of data
**    otherwise     some kind of error.
*/
ਪੂਰਨ sqlite3_Vdbe_Sorter_ਅਗਲਾ(sqlite3 *db, ਅਚਲ VdbeCursor *pCsr){
  VdbeSorter *pSorter;
  ਪੂਰਨ rc;                         /* Return code */

  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSorter->bUsePMA || (pSorter->pReader==0 && pSorter->pMerger==0) );
  ਜੇ( pSorter->bUsePMA ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pSorter->pReader==0 || pSorter->pMerger==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pSorter->b_Use_ਧਾਗੇ==0 || pSorter->pReader );
    ਪ੍ਰਤਿਜ੍ਞਾ( pSorter->b_Use_ਧਾਗੇ==1 || pSorter->pMerger );
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
    ਜੇ( pSorter->b_Use_ਧਾਗੇ ){
      rc = vdbe_Pma_Reader_ਅਗਲਾ(pSorter->pReader);
      ਜੇ( rc==SQLITE_OK && pSorter->pReader->pFd==0 ) rc = SQLITE_ਪੂਰਾ;
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    /*if( !pSorter->bUseThreads )*/ {
      ਪੂਰਨ res = 0;
      ਪ੍ਰਤਿਜ੍ਞਾ( pSorter->pMerger!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pSorter->pMerger->pTask==(&pSorter->aTask[0]) );
      rc = vdbe_ਮਿਲਾਓ_Engine_ਕਦਮ(pSorter->pMerger, &res);
      ਜੇ( rc==SQLITE_OK && res ) rc = SQLITE_ਪੂਰਾ;
    }
  }ਨਹੀਂਤਾਂ{
    Sorter_ਰਿਕਾਰਡ *pFree = pSorter->list.pList;
    pSorter->list.pList = pFree->u.p_ਅਗਲਾ;
    pFree->u.p_ਅਗਲਾ = 0;
    ਜੇ( pSorter->list.a_ਸਮ੍ਰਿਤੀ==0 ) vdbe_Sorter_ਰਿਕਾਰਡ_Free(db, pFree);
    rc = pSorter->list.pList ? SQLITE_OK : SQLITE_ਪੂਰਾ;
  }
  ਵਾਪਸ rc;
}

/*
** Return a pointer to a buffer owned by the sorter that contains the
** current key.
*/
ਸਥਿਰ ਖਾਲੀ *vdbeSorterRowkey(
  ਅਚਲ VdbeSorter *pSorter,      /* Sorter object */
  ਪੂਰਨ *pn_ਕੁੰਜੀ                      /* OUT: Size of current key in bytes */
){
  ਖਾਲੀ *p_ਕੁੰਜੀ;
  ਜੇ( pSorter->bUsePMA ){
    PmaReader *pReader;
#ਜੇ SQLITE_MAX_ਕਾਮਾ_ਧਾਗੇ>0
    ਜੇ( pSorter->b_Use_ਧਾਗੇ ){
      pReader = pSorter->pReader;
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    /*if( !pSorter->bUseThreads )*/{
      pReader = &pSorter->pMerger->aReadr[pSorter->pMerger->a_ਰੁੱਖ[1]];
    }
    *pn_ਕੁੰਜੀ = pReader->n_ਕੁੰਜੀ;
    p_ਕੁੰਜੀ = pReader->a_ਕੁੰਜੀ;
  }ਨਹੀਂਤਾਂ{
    *pn_ਕੁੰਜੀ = pSorter->list.pList->nVal;
    p_ਕੁੰਜੀ = SRVAL(pSorter->list.pList);
  }
  ਵਾਪਸ p_ਕੁੰਜੀ;
}

/*
** Copy the current sorter key into the memory cell pOut.
*/
ਪੂਰਨ sqlite3VdbeSorterRowkey(ਅਚਲ VdbeCursor *pCsr, Mem *p_ਬਾਹਰਲਾ){
  VdbeSorter *pSorter;
  ਖਾਲੀ *p_ਕੁੰਜੀ; ਪੂਰਨ n_ਕੁੰਜੀ;           /* Sorter key to copy into pOut */

  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  p_ਕੁੰਜੀ = vdbeSorterRowkey(pSorter, &n_ਕੁੰਜੀ);
  ਜੇ( sqlite3VdbeMemClearAndResize(p_ਬਾਹਰਲਾ, n_ਕੁੰਜੀ) ){
    ਵਾਪਸ SQLITE_NOMEM_BKPT;
  }
  p_ਬਾਹਰਲਾ->n = n_ਕੁੰਜੀ;
  Mem_ਸੈੱਟ_Type_Flag(p_ਬਾਹਰਲਾ, MEM_Blob);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਬਾਹਰਲਾ->z, p_ਕੁੰਜੀ, n_ਕੁੰਜੀ);

  ਵਾਪਸ SQLITE_OK;
}

/*
** Compare the key in memory cell pVal with the key that the sorter cursor
** passed as the first argument currently points to. For the purposes of
** the comparison, ignore the rowid field at the end of each record.
**
** If the sorter cursor key contains any NULL values, consider it to be
** less than pVal. Even if pVal also contains NULL values.
**
** If an error occurs, return an SQLite error code (i.e. SQLITE_NOMEM).
** Otherwise, set *pRes to a negative, zero or positive value if the
** key in pVal is smaller than, equal to or larger than the current sorter
** key.
**
** This routine forms the core of the OP_SorterCompare opcode, which in
** turn is used to verify uniqueness when constructing a UNIQUE INDEX.
*/
ਪੂਰਨ sqlite3VdbeSorterCompare(
  ਅਚਲ VdbeCursor *pCsr,         /* Sorter cursor */
  Mem *pVal,                      /* Value to compare to current sorter key */
  ਪੂਰਨ n_ਕੁੰਜੀ_ਖਾਨਾ,                    /* Compare this many columns */
  ਪੂਰਨ *pRes                       /* OUT: Result of comparison */
){
  VdbeSorter *pSorter;
  Unpacked_ਰਿਕਾਰਡ *r2;
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info;
  ਪੂਰਨ i;
  ਖਾਲੀ *p_ਕੁੰਜੀ; ਪੂਰਨ n_ਕੁੰਜੀ;           /* Sorter key to compare pVal with */

  ਪ੍ਰਤਿਜ੍ਞਾ( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  r2 = pSorter->pUnpacked;
  p_ਕੁੰਜੀ_Info = pCsr->p_ਕੁੰਜੀ_Info;
  ਜੇ( r2==0 ){
    r2 = pSorter->pUnpacked = sqlite3_Vdbe_Alloc_Unpacked_ਰਿਕਾਰਡ(p_ਕੁੰਜੀ_Info);
    ਜੇ( r2==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
    r2->n_ਖੇਤਰ = n_ਕੁੰਜੀ_ਖਾਨਾ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( r2->n_ਖੇਤਰ==n_ਕੁੰਜੀ_ਖਾਨਾ );

  p_ਕੁੰਜੀ = vdbeSorterRowkey(pSorter, &n_ਕੁੰਜੀ);
  sqlite3_Vdbe_ਰਿਕਾਰਡ_Unpack(n_ਕੁੰਜੀ, p_ਕੁੰਜੀ, r2);
  ਲਈ(i=0; i<n_ਕੁੰਜੀ_ਖਾਨਾ; i++){
    ਜੇ( r2->aMem[i].flags & MEM_Null ){
      *pRes = -1;
      ਵਾਪਸ SQLITE_OK;
    }
  }

  *pRes = sqlite3_Vdbe_ਰਿਕਾਰਡ_Compare(pVal->n, pVal->z, r2);
  ਵਾਪਸ SQLITE_OK;
}
