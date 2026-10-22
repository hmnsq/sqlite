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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "vdbeInt.h"

/*
** If SQLITE_DEBUG_SORTER_THREADS is defined, this module outputs various
** messages to stderr that may be helpful in understanding the performance
** characteristics of the sorter in multi-threaded mode.
*/
#ಆದರೆ 0
# define SQLITE_DEBUG_SORTER_ಎಳೆಗಳು 1
#ಆದರೆ_ಅಂತ್ಯ

/*
** Hard-coded maximum amount of data to accumulate in memory before flushing
** to a level 0 PMA. The purpose of this limit is to prevent various integer
** overflows. 512MiB.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SQLITE_MAX_PMASZ    (1<<29)

/*
** Private objects used by the sorter
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ವಿಲೀನ_Engine ವಿಲೀನ_Engine;     /* Merge PMAs together */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ PmaReader PmaReader;         /* Incrementally read one PMA */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ PmaWriter PmaWriter;         /* Incrementally write one PMA */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Sorter_ದಾಖಲೆ Sorter_ದಾಖಲೆ;   /* A record being sorted */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ವಿಂಗಡಿಸು_Subtask ವಿಂಗಡಿಸು_Subtask;     /* A sub-task in the sort process */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ SorterFile SorterFile;       /* Temporary file object wrapper */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ SorterList SorterList;       /* In-memory list of records */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ IncrMerger IncrMerger;       /* Read & merge multiple PMAs */

/*
** A container for a temp file handle and the current amount of data
** stored in the file.
*/
ರಚನೆ SorterFile {
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
ರಚನೆ SorterList {
  Sorter_ದಾಖಲೆ *pList;            /* Linked list of records */
  u8 *a_ಸ್ಮರಣೆ;                    /* If non-NULL, bulk memory to hold pList */
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
ರಚನೆ ವಿಲೀನ_Engine {
  ಪೂರ್ಣಾಂಕ n_ವೃಕ್ಷ;                 /* Used size of aTree/aReadr (power of 2) */
  ವಿಂಗಡಿಸು_Subtask *pTask;        /* Used by this thread only */
  ಪೂರ್ಣಾಂಕ *a_ವೃಕ್ಷ;                /* Current state of incremental merge */
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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ (*SorterCompare)(ವಿಂಗಡಿಸು_Subtask*,ಪೂರ್ಣಾಂಕ*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ);
ರಚನೆ ವಿಂಗಡಿಸು_Subtask {
  SQLiteThread *pThread;          /* Background thread, if any */
  ಪೂರ್ಣಾಂಕ b_ಮುಗಿದ;                      /* Set if thread is finished but not joined */
  ಪೂರ್ಣಾಂಕ nPMA;                       /* Number of PMAs currently in file */
  VdbeSorter *pSorter;            /* Sorter that owns this sub-task */
  Unpacked_ದಾಖಲೆ *pUnpacked;      /* Space to unpack a record */
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
ರಚನೆ VdbeSorter {
  ಪೂರ್ಣಾಂಕ mn_Pma_ಗಾತ್ರ;                  /* Minimum PMA size, in bytes */
  ಪೂರ್ಣಾಂಕ mx_Pma_ಗಾತ್ರ;                  /* Maximum PMA size, in bytes.  0==no limit */
  ಪೂರ್ಣಾಂಕ mxKeysize;                  /* Largest serialized key seen so far */
  ಪೂರ್ಣಾಂಕ pgsz;                       /* Main database page size */
  PmaReader *pReader;             /* Readr data from here after Rewind() */
  ವಿಲೀನ_Engine *pMerger;           /* Or here, if bUseThreads==0 */
  sqlite3 *db;                    /* Database connection */
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;              /* How to compare records */
  Unpacked_ದಾಖಲೆ *pUnpacked;      /* Used by VdbeSorterCompare() */
  SorterList list;                /* List of in-memory records */
  ಪೂರ್ಣಾಂಕ i_ಸ್ಮರಣೆ;                    /* Offset of free space in list.aMemory */
  ಪೂರ್ಣಾಂಕ n_ಸ್ಮರಣೆ;                    /* Size of list.aMemory allocation in bytes */
  u8 bUsePMA;                     /* True if one or more PMAs created */
  u8 b_Use_ಎಳೆಗಳು;                 /* True to use background threads */
  u8 iPrev;                       /* Previous thread used to flush PMA */
  u8 nTask;                       /* Size of aTask[] array */
  u8 type_ಮುಖವಾಡ;
  ವಿಂಗಡಿಸು_Subtask aTask[FLEXARRAY];   /* One or more subtasks */
};

/* Size (in bytes) of a VdbeSorter object that works with N or fewer subtasks */
#ವ್ಯಾಖ್ಯಾನಿಸು SZ_VDBESORTER(N)  (offsetof(VdbeSorter,aTask)+(N)*ಗಾತ್ರ(ವಿಂಗಡಿಸು_Subtask))

#ವ್ಯಾಖ್ಯಾನಿಸು SORTER_TYPE_INTEGER 0x01
#ವ್ಯಾಖ್ಯಾನಿಸು SORTER_TYPE_ಪಠ್ಯ    0x02
#ವ್ಯಾಖ್ಯಾನಿಸು SORTER_TYPE_REAL    0x04

/*
** An instance of the following object is used to read records out of a
** PMA, in sorted order.  The next key to be read is cached in nKey/aKey.
** aKey might point into aMap or into aBuffer.  If neither of those locations
** contain a contiguous representation of the key, then aAlloc is allocated
** and the key is copied into aAlloc and aKey is made to point to aAlloc.
**
** pFd==0 at EOF.
*/
ರಚನೆ PmaReader {
  i64 iReadOff;               /* Current read offset */
  i64 iEof;                   /* 1 byte past EOF for this PmaReader */
  ಪೂರ್ಣಾಂಕ nAlloc;                 /* Bytes of space at aAlloc */
  ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ;                   /* Number of bytes in key */
  sqlite3_file *pFd;          /* File handle we are reading from */
  u8 *aAlloc;                 /* Space for aKey if aBuffer and pMap wont work */
  u8 *a_ಕೀಲಿ;                   /* Pointer to current key */
  u8 *a_ಬಫರ್;                /* Current read buffer */
  ಪೂರ್ಣಾಂಕ n_ಬಫರ್;                /* Size of read buffer in bytes */
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
ರಚನೆ IncrMerger {
  ವಿಂಗಡಿಸು_Subtask *pTask;             /* Task that owns this merger */
  ವಿಲೀನ_Engine *pMerger;           /* Merge engine thread reads data from */
  i64 i_ಆರಂಭ_Off;                  /* Offset to start writing file at */
  ಪೂರ್ಣಾಂಕ mxSz;                       /* Maximum bytes of data to store */
  ಪೂರ್ಣಾಂಕ bEof;                       /* Set to true when merge is finished */
  ಪೂರ್ಣಾಂಕ bUseThread;                 /* True to use a bg thread for this object */
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
ರಚನೆ PmaWriter {
  ಪೂರ್ಣಾಂಕ eFWErr;                     /* Non-zero if in an error state */
  u8 *a_ಬಫರ್;                    /* Pointer to write buffer */
  ಪೂರ್ಣಾಂಕ n_ಬಫರ್;                    /* Size of write buffer in bytes */
  ಪೂರ್ಣಾಂಕ i_Buf_ಆರಂಭ;                  /* First byte of buffer to write */
  ಪೂರ್ಣಾಂಕ i_Buf_ಕೊನೆ;                    /* Last byte of buffer to write */
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
ರಚನೆ Sorter_ದಾಖಲೆ {
  ಪೂರ್ಣಾಂಕ nVal;                       /* Size of the record in bytes */
  ಒಕ್ಕೂಟ {
    Sorter_ದಾಖಲೆ *p_ಮುಂದಿನ;          /* Pointer to next record in list */
    ಪೂರ್ಣಾಂಕ i_ಮುಂದಿನ;                    /* Offset within aMemory of next record */
  } u;
  /* The data for the record immediately follows this header */
};

/* Return a pointer to the buffer containing the record data for SorterRecord
** object p. Should be used as if:
**
**   void *SRVAL(SorterRecord *p) { return (void*)&p[1]; }
*/
#ವ್ಯಾಖ್ಯಾನಿಸು SRVAL(p) ((ಶೂನ್ಯ*)((Sorter_ದಾಖಲೆ*)(p) + 1))


/* Maximum number of PMAs that a single MergeEngine can merge */
#ವ್ಯಾಖ್ಯಾನಿಸು SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ 16

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Incr_ಅದಲುಬದಲು(IncrMerger*);
ಸ್ಥಿರ ಶೂನ್ಯ vdbeIncrFree(IncrMerger *);

/*
** Free all memory belonging to the PmaReader object passed as the
** argument. All structure fields are set to zero before returning.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbePmaReaderClear(PmaReader *pReadr){
  sqlite3_free(pReadr->aAlloc);
  sqlite3_free(pReadr->a_ಬಫರ್);
  ಆದರೆ( pReadr->aMap ) sqlite3OsUnfetch(pReadr->pFd, 0, pReadr->aMap);
  vdbeIncrFree(pReadr->pIncr);
  memset(pReadr, 0, ಗಾತ್ರ(PmaReader));
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbePmaReadBlob(
  PmaReader *p,                   /* PmaReader from which to take the blob */
  ಪೂರ್ಣಾಂಕ nByte,                      /* Bytes of data to read */
  u8 **pp_ಹೊರಹೋಗು                      /* OUT: Pointer to buffer containing data */
){
  ಪೂರ್ಣಾಂಕ iBuf;                       /* Offset within buffer to read from */
  ಪೂರ್ಣಾಂಕ nAvail;                     /* Bytes of data available in buffer */

  ಆದರೆ( p->aMap ){
    *pp_ಹೊರಹೋಗು = &p->aMap[p->iReadOff];
    p->iReadOff += nByte;
    ಹಿಂತಿರುಗಿ SQLITE_OK;
  }

  ಪ್ರತಿಜ್ಞಾ( p->a_ಬಫರ್ );

  /* If there is no more data to be read from the buffer, read the next
  ** p->nBuffer bytes of data from the file into it. Or, if there are less
  ** than p->nBuffer bytes remaining in the PMA, read all remaining data.  */
  iBuf = p->iReadOff % p->n_ಬಫರ್;
  ಆದರೆ( iBuf==0 ){
    ಪೂರ್ಣಾಂಕ nRead;                    /* Bytes to read from disk */
    ಪೂರ್ಣಾಂಕ rc;                       /* sqlite3OsRead() return code */

    /* Determine how many bytes of data to read. */
    ಆದರೆ( (p->iEof - p->iReadOff) > (i64)p->n_ಬಫರ್ ){
      nRead = p->n_ಬಫರ್;
    }ಇಲ್ಲದಿದ್ದರೆ{
      nRead = (ಪೂರ್ಣಾಂಕ)(p->iEof - p->iReadOff);
    }
    ಪ್ರತಿಜ್ಞಾ( nRead>0 );

    /* Readr data from the file. Return early if an error occurs. */
    rc = sqlite3OsRead(p->pFd, p->a_ಬಫರ್, nRead, p->iReadOff);
    ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_IOERR_SHORT_READ );
    ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;
  }
  nAvail = p->n_ಬಫರ್ - iBuf;

  ಆದರೆ( nByte<=nAvail ){
    /* The requested data is available in the in-memory buffer. In this
    ** case there is no need to make a copy of the data, just return a
    ** pointer into the buffer to the caller.  */
    *pp_ಹೊರಹೋಗು = &p->a_ಬಫರ್[iBuf];
    p->iReadOff += nByte;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* The requested data is not all available in the in-memory buffer.
    ** In this case, allocate space at p->aAlloc[] to copy the requested
    ** range into. Then return a copy of pointer p->aAlloc to the caller.  */
    ಪೂರ್ಣಾಂಕ nRem;                     /* Bytes remaining to copy */

    /* Extend the p->aAlloc[] allocation if required. */
    ಆದರೆ( p->nAlloc<nByte ){
      u8 *aNew;
      sqlite3_int64 nNew = MAX(128, 2*(sqlite3_int64)p->nAlloc);
      ಆಗ( nByte>nNew ) nNew = nNew*2;
      aNew = sqlite3Realloc(p->aAlloc, nNew);
      ಆದರೆ( !aNew ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
      p->nAlloc = nNew;
      p->aAlloc = aNew;
    }

    /* Copy as much data as is available in the buffer into the start of
    ** p->aAlloc[].  */
    ಸ್ಮರಣೆ_ನಕಲು(p->aAlloc, &p->a_ಬಫರ್[iBuf], nAvail);
    p->iReadOff += nAvail;
    nRem = nByte - nAvail;

    /* The following loop copies up to p->nBuffer bytes per iteration into
    ** the p->aAlloc[] buffer.  */
    ಆಗ( nRem>0 ){
      ಪೂರ್ಣಾಂಕ rc;                     /* vdbePmaReadBlob() return code */
      ಪೂರ್ಣಾಂಕ nCopy;                  /* Number of bytes to copy */
      u8 *a_ಮುಂದಿನ = 0;              /* Pointer to buffer to copy data from */

      nCopy = nRem;
      ಆದರೆ( nRem>p->n_ಬಫರ್ ) nCopy = p->n_ಬಫರ್;
      rc = vdbePmaReadBlob(p, nCopy, &a_ಮುಂದಿನ);
      ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;
      ಪ್ರತಿಜ್ಞಾ( a_ಮುಂದಿನ!=p->aAlloc );
      ಪ್ರತಿಜ್ಞಾ( a_ಮುಂದಿನ!=0 );
      ಸ್ಮರಣೆ_ನಕಲು(&p->aAlloc[nByte - nRem], a_ಮುಂದಿನ, nCopy);
      nRem -= nCopy;
    }

    *pp_ಹೊರಹೋಗು = p->aAlloc;
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Read a varint from the stream of data accessed by p. Set *pnOut to
** the value read.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbePmaReadVarint(PmaReader *p, u64 *pn_ಹೊರಹೋಗು){
  ಪೂರ್ಣಾಂಕ iBuf;

  ಆದರೆ( p->aMap ){
    p->iReadOff += sqlite3GetVarint(&p->aMap[p->iReadOff], pn_ಹೊರಹೋಗು);
  }ಇಲ್ಲದಿದ್ದರೆ{
    iBuf = p->iReadOff % p->n_ಬಫರ್;
    ಆದರೆ( iBuf && (p->n_ಬಫರ್-iBuf)>=9 ){
      p->iReadOff += sqlite3GetVarint(&p->a_ಬಫರ್[iBuf], pn_ಹೊರಹೋಗು);
    }ಇಲ್ಲದಿದ್ದರೆ{
      u8 aVarint[16], *a;
      ಪೂರ್ಣಾಂಕ i = 0, rc;
      ಮಾಡು{
        rc = vdbePmaReadBlob(p, 1, &a);
        ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc;
        aVarint[(i++)&0xf] = a[0];
      }ಆಗ( (a[0]&0x80)!=0 );
      sqlite3GetVarint(aVarint, pn_ಹೊರಹೋಗು);
    }
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeSorterMapFile(ವಿಂಗಡಿಸು_Subtask *pTask, SorterFile *pFile, u8 **pp){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( pFile->iEof<=(i64)(pTask->pSorter->db->nMaxSorterMmap) ){
    sqlite3_file *pFd = pFile->pFd;
    ಆದರೆ( pFd->pMethods->i_ಆವೃತ್ತಿ>=3 ){
      rc = sqlite3OsFetch(pFd, 0, (ಪೂರ್ಣಾಂಕ)pFile->iEof, (ಶೂನ್ಯ**)pp);
      testcase( rc!=SQLITE_OK );
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Attach PmaReader pReadr to file pFile (if it is not already attached to
** that file) and seek it to offset iOff within the file.  Return SQLITE_OK
** if successful, or an SQLite error code if an error occurs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbePmaReaderSeek(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Task context */
  PmaReader *pReadr,              /* Reader whose cursor is to be moved */
  SorterFile *pFile,              /* Sorter file to read from */
  i64 iOff                        /* Offset in pFile */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  ಪ್ರತಿಜ್ಞಾ( pReadr->pIncr==0 || pReadr->pIncr->bEof==0 );

  ಆದರೆ( sqlite3FaultSim(201) ) ಹಿಂತಿರುಗಿ SQLITE_IOERR_READ;
  ಆದರೆ( pReadr->aMap ){
    sqlite3OsUnfetch(pReadr->pFd, 0, pReadr->aMap);
    pReadr->aMap = 0;
  }
  pReadr->iReadOff = iOff;
  pReadr->iEof = pFile->iEof;
  pReadr->pFd = pFile->pFd;

  rc = vdbeSorterMapFile(pTask, pFile, &pReadr->aMap);
  ಆದರೆ( rc==SQLITE_OK && pReadr->aMap==0 ){
    ಪೂರ್ಣಾಂಕ pgsz = pTask->pSorter->pgsz;
    ಪೂರ್ಣಾಂಕ iBuf = pReadr->iReadOff % pgsz;
    ಆದರೆ( pReadr->a_ಬಫರ್==0 ){
      pReadr->a_ಬಫರ್ = (u8*)sqlite3Malloc(pgsz);
      ಆದರೆ( pReadr->a_ಬಫರ್==0 ) rc = SQLITE_NOMEM_BKPT;
      pReadr->n_ಬಫರ್ = pgsz;
    }
    ಆದರೆ( rc==SQLITE_OK && iBuf ){
      ಪೂರ್ಣಾಂಕ nRead = pgsz - iBuf;
      ಆದರೆ( (pReadr->iReadOff + nRead) > pReadr->iEof ){
        nRead = (ಪೂರ್ಣಾಂಕ)(pReadr->iEof - pReadr->iReadOff);
      }
      rc = sqlite3OsRead(
          pReadr->pFd, &pReadr->a_ಬಫರ್[iBuf], nRead, pReadr->iReadOff
      );
      testcase( rc!=SQLITE_OK );
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Advance PmaReader pReadr to the next key in its PMA. Return SQLITE_OK if
** no error occurs, or an SQLite error code if one does.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Pma_Reader_ಮುಂದಿನ(PmaReader *pReadr){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;             /* Return Code */
  u64 nRec = 0;                   /* Size of record in bytes */


  ಆದರೆ( pReadr->iReadOff>=pReadr->iEof ){
    IncrMerger *pIncr = pReadr->pIncr;
    ಪೂರ್ಣಾಂಕ bEof = 1;
    ಆದರೆ( pIncr ){
      rc = vdbe_Incr_ಅದಲುಬದಲು(pIncr);
      ಆದರೆ( rc==SQLITE_OK && pIncr->bEof==0 ){
        rc = vdbePmaReaderSeek(
            pIncr->pTask, pReadr, &pIncr->aFile[0], pIncr->i_ಆರಂಭ_Off
        );
        bEof = 0;
      }
    }

    ಆದರೆ( bEof ){
      /* This is an EOF condition */
      vdbePmaReaderClear(pReadr);
      testcase( rc!=SQLITE_OK );
      ಹಿಂತಿರುಗಿ rc;
    }
  }

  ಆದರೆ( rc==SQLITE_OK ){
    rc = vdbePmaReadVarint(pReadr, &nRec);
  }
  ಆದರೆ( rc==SQLITE_OK ){
    pReadr->n_ಕೀಲಿ = (ಪೂರ್ಣಾಂಕ)nRec;
    rc = vdbePmaReadBlob(pReadr, (ಪೂರ್ಣಾಂಕ)nRec, &pReadr->a_ಕೀಲಿ);
    testcase( rc!=SQLITE_OK );
  }

  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbePmaReaderInit(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Task context */
  SorterFile *pFile,              /* Sorter file to read from */
  i64 i_ಆರಂಭ,                     /* Start offset in pFile */
  PmaReader *pReadr,              /* PmaReader to populate */
  i64 *pnByte                     /* IN/OUT: Increment this value by PMA size */
){
  ಪೂರ್ಣಾಂಕ rc;

  ಪ್ರತಿಜ್ಞಾ( pFile->iEof>i_ಆರಂಭ );
  ಪ್ರತಿಜ್ಞಾ( pReadr->aAlloc==0 && pReadr->nAlloc==0 );
  ಪ್ರತಿಜ್ಞಾ( pReadr->a_ಬಫರ್==0 );
  ಪ್ರತಿಜ್ಞಾ( pReadr->aMap==0 );

  rc = vdbePmaReaderSeek(pTask, pReadr, pFile, i_ಆರಂಭ);
  ಆದರೆ( rc==SQLITE_OK ){
    u64 nByte = 0;                 /* Size of PMA in bytes */
    rc = vdbePmaReadVarint(pReadr, &nByte);
    pReadr->iEof = pReadr->iReadOff + nByte;
    *pnByte += nByte;
  }

  ಆದರೆ( rc==SQLITE_OK ){
    rc = vdbe_Pma_Reader_ಮುಂದಿನ(pReadr);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** A version of vdbeSorterCompare() that assumes that it has already been
** determined that the first field of key1 is equal to the first field of
** key2.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Sorter_Compare_ಬಾಲ(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ಪೂರ್ಣಾಂಕ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1, ಪೂರ್ಣಾಂಕ nKey1,   /* Left side of comparison */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey2, ಪೂರ್ಣಾಂಕ nKey2    /* Right side of comparison */
){
  Unpacked_ದಾಖಲೆ *r2 = pTask->pUnpacked;
  ಆದರೆ( *pbKey2Cached==0 ){
    sqlite3_Vdbe_ದಾಖಲೆ_Unpack(nKey2, pKey2, r2);
    *pbKey2Cached = 1;
  }
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ದಾಖಲೆ_Compare_With_Skip(nKey1, pKey1, r2, 1);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeSorterCompare(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ಪೂರ್ಣಾಂಕ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1, ಪೂರ್ಣಾಂಕ nKey1,   /* Left side of comparison */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey2, ಪೂರ್ಣಾಂಕ nKey2    /* Right side of comparison */
){
  Unpacked_ದಾಖಲೆ *r2 = pTask->pUnpacked;
  ಆದರೆ( !*pbKey2Cached ){
    sqlite3_Vdbe_ದಾಖಲೆ_Unpack(nKey2, pKey2, r2);
    *pbKey2Cached = 1;
  }
  ಹಿಂತಿರುಗಿ sqlite3_Vdbe_ದಾಖಲೆ_Compare(nKey1, pKey1, r2);
}

/*
** A specially optimized version of vdbeSorterCompare() that assumes that
** the first field of each key is a TEXT value and that the collation
** sequence to compare them with is BINARY.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Sorter_Compare_ಪಠ್ಯ(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ಪೂರ್ಣಾಂಕ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1, ಪೂರ್ಣಾಂಕ nKey1,   /* Left side of comparison */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey2, ಪೂರ್ಣಾಂಕ nKey2    /* Right side of comparison */
){
  ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ p1 = (ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ)pKey1;
  ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ p2 = (ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ)pKey2;
  ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ v1 = &p1[ p1[0] ];   /* Pointer to value 1 */
  ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ v2 = &p2[ p2[0] ];   /* Pointer to value 2 */

  ಪೂರ್ಣಾಂಕ n1;
  ಪೂರ್ಣಾಂಕ n2;
  ಪೂರ್ಣಾಂಕ res;

  getVarint32NR(&p1[1], n1);
  getVarint32NR(&p2[1], n2);
  res = memcmp(v1, v2, (MIN(n1, n2) - 13)/2);
  ಆದರೆ( res==0 ){
    res = n1 - n2;
  }

  ಆದರೆ( res==0 ){
    ಆದರೆ( pTask->pSorter->p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ>1 ){
      res = vdbe_Sorter_Compare_ಬಾಲ(
          pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2
      );
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pTask->pSorter->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags!=0 );
    ಪ್ರತಿಜ್ಞಾ( !(pTask->pSorter->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[0]&KEYINFO_ಕ್ರಮ_BIGNULL) );
    ಆದರೆ( pTask->pSorter->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[0] ){
      res = res * -1;
    }
  }

  ಹಿಂತಿರುಗಿ res;
}

/*
** A specially optimized version of vdbeSorterCompare() that assumes that
** the first field of each key is an INTEGER value.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeSorterCompareInt(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ಪೂರ್ಣಾಂಕ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1, ಪೂರ್ಣಾಂಕ nKey1,   /* Left side of comparison */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey2, ಪೂರ್ಣಾಂಕ nKey2    /* Right side of comparison */
){
  ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ p1 = (ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ)pKey1;
  ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ p2 = (ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ)pKey2;
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ s1 = p1[1];                 /* Left hand serial type */
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ s2 = p2[1];                 /* Right hand serial type */
  ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ v1 = &p1[ p1[0] ];   /* Pointer to value 1 */
  ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ v2 = &p2[ p2[0] ];   /* Pointer to value 2 */
  ಪೂರ್ಣಾಂಕ res;                              /* Return value */

  ಪ್ರತಿಜ್ಞಾ( (s1>0 && s1<7) || s1==8 || s1==9 );
  ಪ್ರತಿಜ್ಞಾ( (s2>0 && s2<7) || s2==8 || s2==9 );

  ಆದರೆ( s1==s2 ){
    /* The two values have the same sign. Compare using memcmp(). */
    ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ u8 aLen[] = {0, 1, 2, 3, 4, 6, 8, 0, 0, 0 };
    ಸ್ಥಿರಾಂಕ u8 n = aLen[s1];
    ಪೂರ್ಣಾಂಕ i;
    res = 0;
    ಗಾಗಿ(i=0; i<n; i++){
      ಆದರೆ( (res = v1[i] - v2[i])!=0 ){
        ಆದರೆ( ((v1[0] ^ v2[0]) & 0x80)!=0 ){
          res = v1[0] & 0x80 ? -1 : +1;
        }
        ನಿಲ್ಲಿಸು;
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( s1>7 && s2>7 ){
    res = s1 - s2;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( s2>7 ){
      res = +1;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( s1>7 ){
      res = -1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      res = s1 - s2;
    }
    ಪ್ರತಿಜ್ಞಾ( res!=0 );

    ಆದರೆ( res>0 ){
      ಆದರೆ( *v1 & 0x80 ) res = -1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( *v2 & 0x80 ) res = +1;
    }
  }

  ಪ್ರತಿಜ್ಞಾ( pTask->pSorter->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags!=0 );
  ಆದರೆ( res==0 ){
    ಆದರೆ( pTask->pSorter->p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ>1 ){
      res = vdbe_Sorter_Compare_ಬಾಲ(
          pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2
      );
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pTask->pSorter->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[0] ){
    ಪ್ರತಿಜ್ಞಾ( !(pTask->pSorter->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[0]&KEYINFO_ಕ್ರಮ_BIGNULL) );
    res = res * -1;
  }

  ಹಿಂತಿರುಗಿ res;
}

/* Helper function for vdbeSorterCompareReal().
**
** The first elements of both pKey1 and pKey2 have been decoded into double
** values r1 and r2.  Do the comparison between those keys and return the
** result.  If r1==r2, break the tie with a comparison of subsequent elements
** from each key.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Sorter_ಮುಕ್ತಾಯ_Real_Compare(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ಪೂರ್ಣಾಂಕ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1, ಪೂರ್ಣಾಂಕ nKey1,   /* Left side of comparison */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey2, ಪೂರ್ಣಾಂಕ nKey2,   /* Right side of comparison */
  ದ್ವಿಗುಣ r1,                      /* REAL value of first element of pKey1 */
  ದ್ವಿಗುಣ r2                       /* REAL value of first element of pKey2 */
){
  ಪೂರ್ಣಾಂಕ res;
  ಆದರೆ( r1<r2 ){
    res = -1;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( r1>r2 ){
    res = +1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    res = 0;
  }
  ಪ್ರತಿಜ್ಞಾ( pTask->pSorter->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags!=0 );
  ಆದರೆ( res==0 ){
    ಆದರೆ( pTask->pSorter->p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ>1 ){
      res = vdbe_Sorter_Compare_ಬಾಲ(
          pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2
      );
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pTask->pSorter->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[0] ){
    ಪ್ರತಿಜ್ಞಾ( !(pTask->pSorter->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[0]&KEYINFO_ಕ್ರಮ_BIGNULL) );
    res = res * -1;
  }
  ಹಿಂತಿರುಗಿ res;
}

/* Helper function for vdbeSorterCompareReal().
**
** Buffer p[] is a record where the first term is guaranteed to be either
** a floating-point value, or an integer stand-in for a floating point
** value (a MEM_IntReal).  Whatever its format, extract the value and
** return it.
*/
ಸ್ಥಿರ ದ್ವಿಗುಣ vdbeSorterGetReal(ಸ್ಥಿರಾಂಕ u8 *p){
  ದ್ವಿಗುಣ r;                    /* the return value */

  ಪ್ರತಿಜ್ಞಾ( p[0]<0x80 );         /* 1-byte headers: nAllField<13 */
  ಪ್ರತಿಜ್ಞಾ( p[1]>0 && p[1]<10 ); /* first fields proven numeric */

  ಆದರೆ( p[1]==7 ){
    u64 x = sqlite3Get8byte(p + p[0]);
    ಅದಲುಬದಲು_Mixed_Endian_Float(x);
    ಪ್ರತಿಜ್ಞಾ( !IsNaN(x) );
    ಸ್ಮರಣೆ_ನಕಲು(&r, &x, ಗಾತ್ರ(r));
  }ಇಲ್ಲದಿದ್ದರೆ{
    Mem m;
    m.u.i = 0;
    sqlite3VdbeSerialGet(p + p[0], p[1], &m);
    ಪ್ರತಿಜ್ಞಾ( m.flags==MEM_Int );
    r = (ದ್ವಿಗುಣ)m.u.i;
  }
  ಹಿಂತಿರುಗಿ r;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ vdbeSorterCompareRealInt(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ಪೂರ್ಣಾಂಕ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1, ಪೂರ್ಣಾಂಕ nKey1,   /* Left side of comparison */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey2, ಪೂರ್ಣಾಂಕ nKey2    /* Right side of comparison */
){
  ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ p1 = (ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ)pKey1;
  ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ p2 = (ಸ್ಥಿರಾಂಕ u8 * ಸ್ಥಿರಾಂಕ)pKey2;
  ಆದರೆ( p1[1]==6 || p2[1]==6 ){
    /* 64-bit integer values cannot be represented exactly by a double so
    ** must be handled by the generalized comparison function. */
    ಹಿಂತಿರುಗಿ vdbeSorterCompare(pTask,
       pbKey2Cached, pKey1,nKey1, pKey2,nKey2);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ದ್ವಿಗುಣ r1 = vdbeSorterGetReal(p1);
    ದ್ವಿಗುಣ r2 = vdbeSorterGetReal(p2);
    ಹಿಂತಿರುಗಿ vdbe_Sorter_ಮುಕ್ತಾಯ_Real_Compare(pTask,pbKey2Cached,
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeSorterCompareReal(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  ಪೂರ್ಣಾಂಕ *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey1, ಪೂರ್ಣಾಂಕ nKey1,   /* Left side of comparison */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *pKey2, ಪೂರ್ಣಾಂಕ nKey2    /* Right side of comparison */
){
  ಸ್ಥಿರಾಂಕ u8*ಸ್ಥಿರಾಂಕ p1 = (ಸ್ಥಿರಾಂಕ u8*ಸ್ಥಿರಾಂಕ)pKey1;  /* Left key record */
  ಸ್ಥಿರಾಂಕ u8*ಸ್ಥಿರಾಂಕ p2 = (ಸ್ಥಿರಾಂಕ u8*ಸ್ಥಿರಾಂಕ)pKey2;  /* Right key record */
  u64 x;                   /* A real value stored as an integer */
  ದ್ವಿಗುಣ r1;               /* First element of pKey1 */
  ದ್ವಿಗುಣ r2;               /* First element of pKey2 */

  ಪ್ರತಿಜ್ಞಾ( p1[0]<0x80 && p2[0]<0x80 );  /* 1-byte headers: nAllField<13 */
  ಪ್ರತಿಜ್ಞಾ( p1[1]>0 && p1[1]<10 );       /* first field guaranteed numeric */
  ಪ್ರತಿಜ್ಞಾ( p2[1]>0 && p2[1]<10 );       /* first field guaranteed numeric */

  ಆದರೆ( p1[1]!=7 || p2[1]!=7 ){
    /* One or both floating point values are stored as INTEGER.  This might
    ** be because of the MEM_RealInt encoding.  Try to optimize that case. */
    ಹಿಂತಿರುಗಿ vdbeSorterCompareRealInt(pTask,
       pbKey2Cached, pKey1,nKey1, pKey2,nKey2
    );
  }
  ಪ್ರತಿಜ್ಞಾ( p1[0]<=nKey1-8 && p2[0]<=nKey2-8 );

  x = sqlite3Get8byte(p1 + *p1);
  ಅದಲುಬದಲು_Mixed_Endian_Float(x);
  ಪ್ರತಿಜ್ಞಾ( !IsNaN(x) );
  ಸ್ಮರಣೆ_ನಕಲು(&r1, &x, ಗಾತ್ರ(r1));
  x = sqlite3Get8byte(p2 + *p2);
  ಅದಲುಬದಲು_Mixed_Endian_Float(x);
  ಪ್ರತಿಜ್ಞಾ( !IsNaN(x) );
  ಸ್ಮರಣೆ_ನಕಲು(&r2, &x, ಗಾತ್ರ(r2));
  ಹಿಂತಿರುಗಿ vdbe_Sorter_ಮುಕ್ತಾಯ_Real_Compare(pTask,
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
ಪೂರ್ಣಾಂಕ sqlite3VdbeSorterInit(
  sqlite3 *db,                    /* Database connection (for malloc()) */
  ಪೂರ್ಣಾಂಕ n_ಕ್ಷೇತ್ರ,                     /* Number of key fields in each record */
  VdbeCursor *pCsr                /* Cursor that holds the new sorter */
){
  ಪೂರ್ಣಾಂಕ pgsz;                       /* Page size of main database */
  ಪೂರ್ಣಾಂಕ i;                          /* Used to iterate through aTask[] */
  VdbeSorter *pSorter;            /* The new sorter */
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;              /* Copy of pCsr->pKeyInfo with db==0 */
  ಪೂರ್ಣಾಂಕ sz_ಕೀಲಿ_Info;                  /* Size of pCsr->pKeyInfo in bytes */
  i64 sz;                         /* Size of pSorter in bytes */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು==0
# define n_ಕೆಲಸಗಾರ 0
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ n_ಕೆಲಸಗಾರ;
#ಆದರೆ_ಅಂತ್ಯ

  /* Initialize the upper limit on the number of worker threads */
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
  ಆದರೆ( sqlite3_ತಾತ್ಕಾಲಿಕ_ಒಳ_ಸ್ಮರಣೆ(db) || sqlite3GlobalConfig.bCoreMutex==0 ){
    n_ಕೆಲಸಗಾರ = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    n_ಕೆಲಸಗಾರ = db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಕೆಲಸಗಾರ_ಎಳೆಗಳು];
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Do not allow the total number of threads (main thread + all workers)
  ** to exceed the maximum merge count */
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>=SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ
  ಆದರೆ( n_ಕೆಲಸಗಾರ>=SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ ){
    n_ಕೆಲಸಗಾರ = SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ-1;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( pCsr->p_ಕೀಲಿ_Info );
  ಪ್ರತಿಜ್ಞಾ( !pCsr->isEphemeral );
  ಪ್ರತಿಜ್ಞಾ( pCsr->eCurType==CURTYPE_SORTER );
  ಪ್ರತಿಜ್ಞಾ( ಗಾತ್ರ(ಕೀಲಿ_Info) + UMXV(pCsr->p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ)*ಗಾತ್ರ(CollSeq*)
               < 0x7fffffff );
  ಪ್ರತಿಜ್ಞಾ( pCsr->p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ<=pCsr->p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ );
  sz_ಕೀಲಿ_Info = SZ_KEYINFO(pCsr->p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ);
  sz = SZ_VDBESORTER(n_ಕೆಲಸಗಾರ+1);

  pSorter = (VdbeSorter*)sqlite3DbMallocZero(db, sz + sz_ಕೀಲಿ_Info);
  pCsr->uc.pSorter = pSorter;
  ಆದರೆ( pSorter==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    Btree *pBt = db->aDb[0].pBt;
    pSorter->p_ಕೀಲಿ_Info = p_ಕೀಲಿ_Info = (ಕೀಲಿ_Info*)((u8*)pSorter + sz);
    ಸ್ಮರಣೆ_ನಕಲು(p_ಕೀಲಿ_Info, pCsr->p_ಕೀಲಿ_Info, sz_ಕೀಲಿ_Info);
    p_ಕೀಲಿ_Info->db = 0;
    ಆದರೆ( n_ಕ್ಷೇತ್ರ && n_ಕೆಲಸಗಾರ==0 ){
      p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ = n_ಕ್ಷೇತ್ರ;
      ಪ್ರತಿಜ್ಞಾ( n_ಕ್ಷೇತ್ರ<=pCsr->p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ );
    }
    /* It is OK that pKeyInfo reuses the aSortFlags field from pCsr->pKeyInfo,
    ** since the pCsr->pKeyInfo->aSortFlags[] array is invariant and lives
    ** longer that pSorter. */
    ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags==pCsr->p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags );
    sqlite3BtreeEnter(pBt);
    pSorter->pgsz = pgsz = sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(pBt);
    sqlite3BtreeLeave(pBt);
    pSorter->nTask = n_ಕೆಲಸಗಾರ + 1;
    pSorter->iPrev = (u8)(n_ಕೆಲಸಗಾರ - 1);
    pSorter->b_Use_ಎಳೆಗಳು = (pSorter->nTask>1);
    pSorter->db = db;
    ಗಾಗಿ(i=0; i<pSorter->nTask; i++){
      ವಿಂಗಡಿಸು_Subtask *pTask = &pSorter->aTask[i];
      pTask->pSorter = pSorter;
    }

    ಆದರೆ( !sqlite3_ತಾತ್ಕಾಲಿಕ_ಒಳ_ಸ್ಮರಣೆ(db) ){
      i64 mxCache;                /* Cache size in bytes*/
      u32 szPma = sqlite3GlobalConfig.szPma;
      pSorter->mn_Pma_ಗಾತ್ರ = szPma * pgsz;

      mxCache = db->aDb[0].pSchema->cache_ಗಾತ್ರ;
      ಆದರೆ( mxCache<0 ){
        /* A negative cache-size value C indicates that the cache is abs(C)
        ** KiB in size.  */
        mxCache = mxCache * -1024;
      }ಇಲ್ಲದಿದ್ದರೆ{
        mxCache = mxCache * pgsz;
      }
      mxCache = MIN(mxCache, SQLITE_MAX_PMASZ);
      pSorter->mx_Pma_ಗಾತ್ರ = MAX(pSorter->mn_Pma_ಗಾತ್ರ, (ಪೂರ್ಣಾಂಕ)mxCache);

      /* Avoid large memory allocations if the application has requested
      ** SQLITE_CONFIG_SMALL_MALLOC. */
      ಆದರೆ( sqlite3GlobalConfig.bSmallMalloc==0 ){
        ಪ್ರತಿಜ್ಞಾ( pSorter->i_ಸ್ಮರಣೆ==0 );
        pSorter->n_ಸ್ಮರಣೆ = pgsz;
        pSorter->list.a_ಸ್ಮರಣೆ = (u8*)sqlite3Malloc(pgsz);
        ಆದರೆ( !pSorter->list.a_ಸ್ಮರಣೆ ) rc = SQLITE_NOMEM_BKPT;
      }
    }

    ಆದರೆ( p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ<13
     && (p_ಕೀಲಿ_Info->aColl[0]==0 || p_ಕೀಲಿ_Info->aColl[0]==db->pDfltColl)
     && (p_ಕೀಲಿ_Info->a_ವಿಂಗಡಿಸು_Flags[0] & KEYINFO_ಕ್ರಮ_BIGNULL)==0
    ){
      pSorter->type_ಮುಖವಾಡ = SORTER_TYPE_INTEGER|SORTER_TYPE_ಪಠ್ಯ|SORTER_TYPE_REAL;
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು n_ಕೆಲಸಗಾರ   /* Defined at the top of this function */

/*
** Free the list of sorted records starting at pRecord.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbe_Sorter_ದಾಖಲೆ_Free(sqlite3 *db, Sorter_ದಾಖಲೆ *p_ದಾಖಲೆ){
  Sorter_ದಾಖಲೆ *p;
  Sorter_ದಾಖಲೆ *p_ಮುಂದಿನ;
  ಗಾಗಿ(p=p_ದಾಖಲೆ; p; p=p_ಮುಂದಿನ){
    p_ಮುಂದಿನ = p->u.p_ಮುಂದಿನ;
    sqlite3DbFree(db, p);
  }
}

/*
** Free all resources owned by the object indicated by argument pTask. All
** fields of *pTask are zeroed before returning.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbe_ವಿಂಗಡಿಸು_Subtask_Cleanup(sqlite3 *db, ವಿಂಗಡಿಸು_Subtask *pTask){
  sqlite3DbFree(db, pTask->pUnpacked);
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
  /* pTask->list.aMemory can only be non-zero if it was handed memory
  ** from the main thread.  That only occurs SQLITE_MAX_WORKER_THREADS>0 */
  ಆದರೆ( pTask->list.a_ಸ್ಮರಣೆ ){
    sqlite3_free(pTask->list.a_ಸ್ಮರಣೆ);
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
  {
    ಪ್ರತಿಜ್ಞಾ( pTask->list.a_ಸ್ಮರಣೆ==0 );
    vdbe_Sorter_ದಾಖಲೆ_Free(0, pTask->list.pList);
  }
  ಆದರೆ( pTask->file.pFd ){
    sqlite3OsCloseFree(pTask->file.pFd);
  }
  ಆದರೆ( pTask->file2.pFd ){
    sqlite3OsCloseFree(pTask->file2.pFd);
  }
  memset(pTask, 0, ಗಾತ್ರ(ವಿಂಗಡಿಸು_Subtask));
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG_SORTER_ಎಳೆಗಳು
ಸ್ಥಿರ ಶೂನ್ಯ vdbeSorterWorkDebug(ವಿಂಗಡಿಸು_Subtask *pTask, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zEvent){
  i64 t;
  ಪೂರ್ಣಾಂಕ iTask = (pTask - pTask->pSorter->aTask);
  sqlite3_Os_Current_ಸಮಯ_Int64(pTask->pSorter->db->pVfs, &t);
  fprintf(stderr, "%lld:%d %s\n", t, iTask, zEvent);
}
ಸ್ಥಿರ ಶೂನ್ಯ vdbeSorterRewindDebug(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zEvent){
  i64 t = 0;
  sqlite3_vfs *pVfs = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  ಆದರೆ( ALWAYS(pVfs) ) sqlite3_Os_Current_ಸಮಯ_Int64(pVfs, &t);
  fprintf(stderr, "%lld:X %s\n", t, zEvent);
}
ಸ್ಥಿರ ಶೂನ್ಯ vdbeSorterPopulateDebug(
  ವಿಂಗಡಿಸು_Subtask *pTask,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zEvent
){
  i64 t;
  ಪೂರ್ಣಾಂಕ iTask = (pTask - pTask->pSorter->aTask);
  sqlite3_Os_Current_ಸಮಯ_Int64(pTask->pSorter->db->pVfs, &t);
  fprintf(stderr, "%lld:bg%d %s\n", t, iTask, zEvent);
}
ಸ್ಥಿರ ಶೂನ್ಯ vdbe_Sorter_ಖಂಡ_Debug(
  ವಿಂಗಡಿಸು_Subtask *pTask,
  ಪೂರ್ಣಾಂಕ bBlocked,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zEvent
){
  ಆದರೆ( bBlocked ){
    i64 t;
    sqlite3_Os_Current_ಸಮಯ_Int64(pTask->pSorter->db->pVfs, &t);
    fprintf(stderr, "%lld:main %s\n", t, zEvent);
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define vdbeSorterWorkDebug(x,y)
# define vdbeSorterRewindDebug(y)
# define vdbeSorterPopulateDebug(x,y)
# define vdbe_Sorter_ಖಂಡ_Debug(x,y,z)
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
/*
** Join thread pTask->thread.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeSorterJoinThread(ವಿಂಗಡಿಸು_Subtask *pTask){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( pTask->pThread ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG_SORTER_ಎಳೆಗಳು
    ಪೂರ್ಣಾಂಕ b_ಮುಗಿದ = pTask->b_ಮುಗಿದ;
#ಆದರೆ_ಅಂತ್ಯ
    ಶೂನ್ಯ *pRet = SQLITE_INT_ಗಮ್ಯ_PTR(SQLITE_ERROR);
    vdbe_Sorter_ಖಂಡ_Debug(pTask, !b_ಮುಗಿದ, "enter");
    (ಶೂನ್ಯ)sqlite3ThreadJoin(pTask->pThread, &pRet);
    vdbe_Sorter_ಖಂಡ_Debug(pTask, !b_ಮುಗಿದ, "exit");
    rc = SQLITE_PTR_ಗಮ್ಯ_INT(pRet);
    ಪ್ರತಿಜ್ಞಾ( pTask->b_ಮುಗಿದ==1 );
    pTask->b_ಮುಗಿದ = 0;
    pTask->pThread = 0;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Launch a background thread to run xTask(pIn).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeSorterCreateThread(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Thread will use this task object */
  ಶೂನ್ಯ *(*xTask)(ಶೂನ್ಯ*),          /* Routine to run in a separate thread */
  ಶೂನ್ಯ *p_ಒಳ                       /* Argument passed into xTask() */
){
  ಪ್ರತಿಜ್ಞಾ( pTask->pThread==0 && pTask->b_ಮುಗಿದ==0 );
  ಹಿಂತಿರುಗಿ sqlite3ThreadCreate(&pTask->pThread, xTask, p_ಒಳ);
}

/*
** Join all outstanding threads launched by SorterWrite() to create
** level-0 PMAs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeSorterJoinAll(VdbeSorter *pSorter, ಪೂರ್ಣಾಂಕ rcin){
  ಪೂರ್ಣಾಂಕ rc = rcin;
  ಪೂರ್ಣಾಂಕ i;

  /* This function is always called by the main user thread.
  **
  ** If this function is being called after SorterRewind() has been called,
  ** it is possible that thread pSorter->aTask[pSorter->nTask-1].pThread
  ** is currently attempt to join one of the other threads. To avoid a race
  ** condition where this thread also attempts to join the same object, join
  ** thread pSorter->aTask[pSorter->nTask-1].pThread first. */
  ಗಾಗಿ(i=pSorter->nTask-1; i>=0; i--){
    ವಿಂಗಡಿಸು_Subtask *pTask = &pSorter->aTask[i];
    ಪೂರ್ಣಾಂಕ rc2 = vdbeSorterJoinThread(pTask);
    ಆದರೆ( rc==SQLITE_OK ) rc = rc2;
  }
  ಹಿಂತಿರುಗಿ rc;
}
#ಇಲ್ಲದಿದ್ದರೆ
# define vdbeSorterJoinAll(x,rcin) (rcin)
# define vdbeSorterJoinThread(pTask) SQLITE_OK
#ಆದರೆ_ಅಂತ್ಯ

/*
** Allocate a new MergeEngine object capable of handling up to
** nReader PmaReader inputs.
**
** nReader is automatically rounded up to the next power of two.
** nReader may not exceed SORTER_MAX_MERGE_COUNT even after rounding up.
*/
ಸ್ಥಿರ ವಿಲೀನ_Engine *vdbe_ವಿಲೀನ_Engine_New(ಪೂರ್ಣಾಂಕ nReader){
  ಪೂರ್ಣಾಂಕ N = 2;                      /* Smallest power of two >= nReader */
  i64 nByte;                      /* Total bytes of space to allocate */
  ವಿಲೀನ_Engine *pNew;              /* Pointer to allocated object to return */

  ಪ್ರತಿಜ್ಞಾ( nReader<=SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ );

  ಆಗ( N<nReader ) N += N;
  nByte = ಗಾತ್ರ(ವಿಲೀನ_Engine) + N * (ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ) + ಗಾತ್ರ(PmaReader));

  pNew = sqlite3FaultSim(100) ? 0 : (ವಿಲೀನ_Engine*)sqlite3MallocZero(nByte);
  ಆದರೆ( pNew ){
    pNew->n_ವೃಕ್ಷ = N;
    pNew->pTask = 0;
    pNew->aReadr = (PmaReader*)&pNew[1];
    pNew->a_ವೃಕ್ಷ = (ಪೂರ್ಣಾಂಕ*)&pNew->aReadr[N];
  }
  ಹಿಂತಿರುಗಿ pNew;
}

/*
** Free the MergeEngine object passed as the only argument.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbe_ವಿಲೀನ_Engine_Free(ವಿಲೀನ_Engine *pMerger){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( pMerger ){
    ಗಾಗಿ(i=0; i<pMerger->n_ವೃಕ್ಷ; i++){
      vdbePmaReaderClear(&pMerger->aReadr[i]);
    }
  }
  sqlite3_free(pMerger);
}

/*
** Free all resources associated with the IncrMerger object indicated by
** the first argument.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbeIncrFree(IncrMerger *pIncr){
  ಆದರೆ( pIncr ){
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
    ಆದರೆ( pIncr->bUseThread ){
      vdbeSorterJoinThread(pIncr->pTask);
      ಆದರೆ( pIncr->aFile[0].pFd ) sqlite3OsCloseFree(pIncr->aFile[0].pFd);
      ಆದರೆ( pIncr->aFile[1].pFd ) sqlite3OsCloseFree(pIncr->aFile[1].pFd);
    }
#ಆದರೆ_ಅಂತ್ಯ
    vdbe_ವಿಲೀನ_Engine_Free(pIncr->pMerger);
    sqlite3_free(pIncr);
  }
}

/*
** Reset a sorting cursor back to its original empty state.
*/
ಶೂನ್ಯ sqlite3VdbeSorterReset(sqlite3 *db, VdbeSorter *pSorter){
  ಪೂರ್ಣಾಂಕ i;
  (ಶೂನ್ಯ)vdbeSorterJoinAll(pSorter, SQLITE_OK);
  ಪ್ರತಿಜ್ಞಾ( pSorter->b_Use_ಎಳೆಗಳು || pSorter->pReader==0 );
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
  ಆದರೆ( pSorter->pReader ){
    vdbePmaReaderClear(pSorter->pReader);
    sqlite3DbFree(db, pSorter->pReader);
    pSorter->pReader = 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  vdbe_ವಿಲೀನ_Engine_Free(pSorter->pMerger);
  pSorter->pMerger = 0;
  ಗಾಗಿ(i=0; i<pSorter->nTask; i++){
    ವಿಂಗಡಿಸು_Subtask *pTask = &pSorter->aTask[i];
    vdbe_ವಿಂಗಡಿಸು_Subtask_Cleanup(db, pTask);
    pTask->pSorter = pSorter;
  }
  ಆದರೆ( pSorter->list.a_ಸ್ಮರಣೆ==0 ){
    vdbe_Sorter_ದಾಖಲೆ_Free(0, pSorter->list.pList);
  }
  pSorter->list.pList = 0;
  pSorter->list.szPMA = 0;
  pSorter->bUsePMA = 0;
  pSorter->i_ಸ್ಮರಣೆ = 0;
  pSorter->mxKeysize = 0;
  sqlite3DbFree(db, pSorter->pUnpacked);
  pSorter->pUnpacked = 0;
}

/*
** Free any cursor components allocated by sqlite3VdbeSorterXXX routines.
*/
ಶೂನ್ಯ sqlite3VdbeSorterClose(sqlite3 *db, VdbeCursor *pCsr){
  VdbeSorter *pSorter;
  ಪ್ರತಿಜ್ಞಾ( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  ಆದರೆ( pSorter ){
    /* Increment db->nSpill by the total number of bytes of data written
    ** to temp files by this sort operation.  */
    ಪೂರ್ಣಾಂಕ ii;
    ಗಾಗಿ(ii=0; ii<pSorter->nTask; ii++){
      db->nSpill += pSorter->aTask[ii].nSpill;
    }
    sqlite3VdbeSorterReset(db, pSorter);
    sqlite3_free(pSorter->list.a_ಸ್ಮರಣೆ);
    sqlite3DbFree(db, pSorter);
    pCsr->uc.pSorter = 0;
  }
}

#ಆದರೆ SQLITE_MAX_MMAP_ಗಾತ್ರ>0
/*
** The first argument is a file-handle open on a temporary file. The file
** is guaranteed to be nByte bytes or smaller in size. This function
** attempts to extend the file to nByte bytes in size and to ensure that
** the VFS has memory mapped it.
**
** Whether or not the file does end up memory mapped of course depends on
** the specific VFS implementation.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbeSorterExtendFile(sqlite3 *db, sqlite3_file *pFd, i64 nByte){
  ಆದರೆ( nByte<=(i64)(db->nMaxSorterMmap) && pFd->pMethods->i_ಆವೃತ್ತಿ>=3 ){
    ಶೂನ್ಯ *p = 0;
    ಪೂರ್ಣಾಂಕ chunksize = 4*1024;
    sqlite3OsFileControlHint(pFd, SQLITE_FCNTL_CHUNK_ಗಾತ್ರ, &chunksize);
    sqlite3OsFileControlHint(pFd, SQLITE_FCNTL_ಗಾತ್ರ_HINT, &nByte);
    sqlite3OsFetch(pFd, 0, (ಪೂರ್ಣಾಂಕ)nByte, &p);
    ಆದರೆ( p ) sqlite3OsUnfetch(pFd, 0, p);
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define vdbeSorterExtendFile(x,y,z)
#ಆದರೆ_ಅಂತ್ಯ

/*
** Allocate space for a file-handle and open a temporary file. If successful,
** set *ppFd to point to the malloc'd file-handle and return SQLITE_OK.
** Otherwise, set *ppFd to 0 and return an SQLite error code.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Sorter_Open_ತಾತ್ಕಾಲಿಕ_File(
  sqlite3 *db,                    /* Database handle doing sort */
  i64 nExtend,                    /* Attempt to extend file to this size */
  sqlite3_file **ppFd
){
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( sqlite3FaultSim(202) ) ಹಿಂತಿರುಗಿ SQLITE_IOERR_ACCESS;
  rc = sqlite3OsOpenMalloc(db->pVfs, 0, ppFd,
      SQLITE_OPEN_ತಾತ್ಕಾಲಿಕ_JOURNAL |
      SQLITE_OPEN_READWRITE    | SQLITE_OPEN_CREATE |
      SQLITE_OPEN_EXCLUSIVE    | SQLITE_OPEN_DELETEONCLOSE, &rc
  );
  ಆದರೆ( rc==SQLITE_OK ){
    i64 max = SQLITE_MAX_MMAP_ಗಾತ್ರ;
    sqlite3OsFileControlHint(*ppFd, SQLITE_FCNTL_MMAP_ಗಾತ್ರ, (ಶೂನ್ಯ*)&max);
    ಆದರೆ( nExtend>0 ){
      vdbeSorterExtendFile(db, *ppFd, nExtend);
    }
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** If it has not already been allocated, allocate the UnpackedRecord
** structure at pTask->pUnpacked. Return SQLITE_OK if successful (or
** if no allocation was required), or SQLITE_NOMEM otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_ವಿಂಗಡಿಸು_Alloc_Unpacked(ವಿಂಗಡಿಸು_Subtask *pTask){
  ಆದರೆ( pTask->pUnpacked==0 ){
    pTask->pUnpacked = sqlite3_Vdbe_Alloc_Unpacked_ದಾಖಲೆ(pTask->pSorter->p_ಕೀಲಿ_Info);
    ಆದರೆ( pTask->pUnpacked==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    pTask->pUnpacked->n_ಕ್ಷೇತ್ರ = pTask->pSorter->p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ;
    pTask->pUnpacked->err_ಸಂಕೇತ = 0;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}


/*
** Merge the two sorted lists p1 and p2 into a single list.
*/
ಸ್ಥಿರ Sorter_ದಾಖಲೆ *vdbe_Sorter_ವಿಲೀನ(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Calling thread context */
  Sorter_ದಾಖಲೆ *p1,               /* First list to merge */
  Sorter_ದಾಖಲೆ *p2                /* Second list to merge */
){
  Sorter_ದಾಖಲೆ *pFinal = 0;
  Sorter_ದಾಖಲೆ **pp = &pFinal;
  ಪೂರ್ಣಾಂಕ bCached = 0;

  ಪ್ರತಿಜ್ಞಾ( p1!=0 && p2!=0 );
  ಗಾಗಿ(;;){
    ಪೂರ್ಣಾಂಕ res;
    res = pTask->xCompare(
        pTask, &bCached, SRVAL(p1), p1->nVal, SRVAL(p2), p2->nVal
    );

    ಆದರೆ( res<=0 ){
      *pp = p1;
      pp = &p1->u.p_ಮುಂದಿನ;
      p1 = p1->u.p_ಮುಂದಿನ;
      ಆದರೆ( p1==0 ){
        *pp = p2;
        ನಿಲ್ಲಿಸು;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      *pp = p2;
      pp = &p2->u.p_ಮುಂದಿನ;
      p2 = p2->u.p_ಮುಂದಿನ;
      bCached = 0;
      ಆದರೆ( p2==0 ){
        *pp = p1;
        ನಿಲ್ಲಿಸು;
      }
    }
  }
  ಹಿಂತಿರುಗಿ pFinal;
}

/*
** Return the SorterCompare function to compare values collected by the
** sorter object passed as the only argument.
*/
ಸ್ಥಿರ SorterCompare vdbeSorterGetCompare(VdbeSorter *p){
  ಆದರೆ( p->type_ಮುಖವಾಡ & SORTER_TYPE_INTEGER ){
    ಹಿಂತಿರುಗಿ vdbeSorterCompareInt;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->type_ಮುಖವಾಡ & SORTER_TYPE_ಪಠ್ಯ ){
    ಹಿಂತಿರುಗಿ vdbe_Sorter_Compare_ಪಠ್ಯ;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->type_ಮುಖವಾಡ & SORTER_TYPE_REAL ){
    ಹಿಂತಿರುಗಿ vdbeSorterCompareReal;
  }
  ಹಿಂತಿರುಗಿ vdbeSorterCompare;
}

/*
** Sort the linked list of records headed at pTask->pList. Return
** SQLITE_OK if successful, or an SQLite error code (i.e. SQLITE_NOMEM) if
** an error occurs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Sorter_ವಿಂಗಡಿಸು(ವಿಂಗಡಿಸು_Subtask *pTask, SorterList *pList){
  ಪೂರ್ಣಾಂಕ i;
  Sorter_ದಾಖಲೆ *p;
  ಪೂರ್ಣಾಂಕ rc;
  Sorter_ದಾಖಲೆ *a_ಸ್ಥಾನ[64];

  rc = vdbe_ವಿಂಗಡಿಸು_Alloc_Unpacked(pTask);
  ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;

  p = pList->pList;
  pTask->xCompare = vdbeSorterGetCompare(pTask->pSorter);
  memset(a_ಸ್ಥಾನ, 0, ಗಾತ್ರ(a_ಸ್ಥಾನ));

  ಆಗ( p ){
    Sorter_ದಾಖಲೆ *p_ಮುಂದಿನ;
    ಆದರೆ( pList->a_ಸ್ಮರಣೆ ){
      ಆದರೆ( (u8*)p==pList->a_ಸ್ಮರಣೆ ){
        p_ಮುಂದಿನ = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( p->u.i_ಮುಂದಿನ<sqlite3_Malloc_ಗಾತ್ರ(pList->a_ಸ್ಮರಣೆ) );
        p_ಮುಂದಿನ = (Sorter_ದಾಖಲೆ*)&pList->a_ಸ್ಮರಣೆ[p->u.i_ಮುಂದಿನ];
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಮುಂದಿನ = p->u.p_ಮುಂದಿನ;
    }

    p->u.p_ಮುಂದಿನ = 0;
    ಗಾಗಿ(i=0; a_ಸ್ಥಾನ[i]; i++){
      p = vdbe_Sorter_ವಿಲೀನ(pTask, p, a_ಸ್ಥಾನ[i]);
      /* ,--Each aSlot[] holds twice as much as the previous. So we cannot use
      ** |  up all 64 aSlots[] with only a 64-bit address space.
      ** v                                                                */
      ಪ್ರತಿಜ್ಞಾ( i<Array_ಗಾತ್ರ(a_ಸ್ಥಾನ) );
      a_ಸ್ಥಾನ[i] = 0;
    }
    a_ಸ್ಥಾನ[i] = p;
    p = p_ಮುಂದಿನ;
  }

  p = 0;
  ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(a_ಸ್ಥಾನ); i++){
    ಆದರೆ( a_ಸ್ಥಾನ[i]==0 ) ಮುಂದುವರಿ;
    p = p ? vdbe_Sorter_ವಿಲೀನ(pTask, p, a_ಸ್ಥಾನ[i]) : a_ಸ್ಥಾನ[i];
  }
  pList->pList = p;

  ಪ್ರತಿಜ್ಞಾ( pTask->pUnpacked->err_ಸಂಕೇತ==SQLITE_OK
       || pTask->pUnpacked->err_ಸಂಕೇತ==SQLITE_NOMEM
  );
  ಹಿಂತಿರುಗಿ pTask->pUnpacked->err_ಸಂಕೇತ;
}

/*
** Initialize a PMA-writer object.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbePmaWriterInit(
  sqlite3_file *pFd,              /* File handle to write to */
  PmaWriter *p,                   /* Object to populate */
  ಪೂರ್ಣಾಂಕ nBuf,                       /* Buffer size */
  i64 i_ಆರಂಭ                      /* Offset of pFd to begin writing at */
){
  memset(p, 0, ಗಾತ್ರ(PmaWriter));
  p->a_ಬಫರ್ = (u8*)sqlite3Malloc(nBuf);
  ಆದರೆ( !p->a_ಬಫರ್ ){
    p->eFWErr = SQLITE_NOMEM_BKPT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->i_Buf_ಕೊನೆ = p->i_Buf_ಆರಂಭ = (i_ಆರಂಭ % nBuf);
    p->iWriteOff = i_ಆರಂಭ - p->i_Buf_ಆರಂಭ;
    p->n_ಬಫರ್ = nBuf;
    p->pFd = pFd;
  }
}

/*
** Write nData bytes of data to the PMA. Return SQLITE_OK
** if successful, or an SQLite error code if an error occurs.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbePmaWriteBlob(PmaWriter *p, u8 *p_ದತ್ತಾಂಶ, ಪೂರ್ಣಾಂಕ n_ದತ್ತಾಂಶ){
  ಪೂರ್ಣಾಂಕ nRem = n_ದತ್ತಾಂಶ;
  ಆಗ( nRem>0 && p->eFWErr==0 ){
    ಪೂರ್ಣಾಂಕ nCopy = nRem;
    ಆದರೆ( nCopy>(p->n_ಬಫರ್ - p->i_Buf_ಕೊನೆ) ){
      nCopy = p->n_ಬಫರ್ - p->i_Buf_ಕೊನೆ;
    }

    ಸ್ಮರಣೆ_ನಕಲು(&p->a_ಬಫರ್[p->i_Buf_ಕೊನೆ], &p_ದತ್ತಾಂಶ[n_ದತ್ತಾಂಶ-nRem], nCopy);
    p->i_Buf_ಕೊನೆ += nCopy;
    ಆದರೆ( p->i_Buf_ಕೊನೆ==p->n_ಬಫರ್ ){
      p->eFWErr = sqlite3OsWrite(p->pFd,
          &p->a_ಬಫರ್[p->i_Buf_ಆರಂಭ], p->i_Buf_ಕೊನೆ - p->i_Buf_ಆರಂಭ,
          p->iWriteOff + p->i_Buf_ಆರಂಭ
      );
      p->nPmaSpill += (p->i_Buf_ಕೊನೆ - p->i_Buf_ಆರಂಭ);
      p->i_Buf_ಆರಂಭ = p->i_Buf_ಕೊನೆ = 0;
      p->iWriteOff += p->n_ಬಫರ್;
    }
    ಪ್ರತಿಜ್ಞಾ( p->i_Buf_ಕೊನೆ<p->n_ಬಫರ್ );

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Pma_Writer_ಮುಕ್ತಾಯ(PmaWriter *p, i64 *piEof, u64 *pnSpill){
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( p->eFWErr==0 && ALWAYS(p->a_ಬಫರ್) && p->i_Buf_ಕೊನೆ>p->i_Buf_ಆರಂಭ ){
    p->eFWErr = sqlite3OsWrite(p->pFd,
        &p->a_ಬಫರ್[p->i_Buf_ಆರಂಭ], p->i_Buf_ಕೊನೆ - p->i_Buf_ಆರಂಭ,
        p->iWriteOff + p->i_Buf_ಆರಂಭ
    );
    p->nPmaSpill += (p->i_Buf_ಕೊನೆ - p->i_Buf_ಆರಂಭ);
  }
  *piEof = (p->iWriteOff + p->i_Buf_ಕೊನೆ);
  *pnSpill += p->nPmaSpill;
  sqlite3_free(p->a_ಬಫರ್);
  rc = p->eFWErr;
  memset(p, 0, ಗಾತ್ರ(PmaWriter));
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Write value iVal encoded as a varint to the PMA. Return
** SQLITE_OK if successful, or an SQLite error code if an error occurs.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbePmaWriteVarint(PmaWriter *p, u64 iVal){
  ಪೂರ್ಣಾಂಕ nByte;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Sorter_List_ಗಮ್ಯ_PMA(ವಿಂಗಡಿಸು_Subtask *pTask, SorterList *pList){
  sqlite3 *db = pTask->pSorter->db;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;             /* Return code */
  PmaWriter writer;               /* Object used to write to the file */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* Set iSz to the expected size of file pTask->file after writing the PMA.
  ** This is used by an assert() statement at the end of this function.  */
  i64 iSz = pList->szPMA + sqlite3VarintLen(pList->szPMA) + pTask->file.iEof;
#ಆದರೆ_ಅಂತ್ಯ

  vdbeSorterWorkDebug(pTask, "enter");
  memset(&writer, 0, ಗಾತ್ರ(PmaWriter));
  ಪ್ರತಿಜ್ಞಾ( pList->szPMA>0 );

  /* If the first temporary PMA file has not been opened, open it now. */
  ಆದರೆ( pTask->file.pFd==0 ){
    rc = vdbe_Sorter_Open_ತಾತ್ಕಾಲಿಕ_File(db, 0, &pTask->file.pFd);
    ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || pTask->file.pFd );
    ಪ್ರತಿಜ್ಞಾ( pTask->file.iEof==0 );
    ಪ್ರತಿಜ್ಞಾ( pTask->nPMA==0 );
  }

  /* Try to get the file to memory map */
  ಆದರೆ( rc==SQLITE_OK ){
    vdbeSorterExtendFile(db, pTask->file.pFd, pTask->file.iEof+pList->szPMA+9);
  }

  /* Sort the list */
  ಆದರೆ( rc==SQLITE_OK ){
    rc = vdbe_Sorter_ವಿಂಗಡಿಸು(pTask, pList);
  }

  ಆದರೆ( rc==SQLITE_OK ){
    Sorter_ದಾಖಲೆ *p;
    Sorter_ದಾಖಲೆ *p_ಮುಂದಿನ = 0;

    vdbePmaWriterInit(pTask->file.pFd, &writer, pTask->pSorter->pgsz,
                      pTask->file.iEof);
    pTask->nPMA++;
    vdbePmaWriteVarint(&writer, pList->szPMA);
    ಗಾಗಿ(p=pList->pList; p; p=p_ಮುಂದಿನ){
      p_ಮುಂದಿನ = p->u.p_ಮುಂದಿನ;
      vdbePmaWriteVarint(&writer, p->nVal);
      vdbePmaWriteBlob(&writer, SRVAL(p), p->nVal);
      ಆದರೆ( pList->a_ಸ್ಮರಣೆ==0 ) sqlite3_free(p);
    }
    pList->pList = p;
    rc = vdbe_Pma_Writer_ಮುಕ್ತಾಯ(&writer, &pTask->file.iEof, &pTask->nSpill);
  }

  vdbeSorterWorkDebug(pTask, "exit");
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || pList->pList==0 );
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || pTask->file.iEof==iSz );
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Advance the MergeEngine to its next entry.
** Set *pbEof to true there is no next entry because
** the MergeEngine has reached the end of all its inputs.
**
** Return SQLITE_OK if successful or an error code if an error occurs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_ವಿಲೀನ_Engine_ಹೆಜ್ಜೆ(
  ವಿಲೀನ_Engine *pMerger,      /* The merge engine to advance to the next row */
  ಪೂರ್ಣಾಂಕ *pbEof                 /* Set TRUE at EOF.  Set false for more content */
){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ iPrev = pMerger->a_ವೃಕ್ಷ[1];/* Index of PmaReader to advance */
  ವಿಂಗಡಿಸು_Subtask *pTask = pMerger->pTask;

  /* Advance the current PmaReader */
  rc = vdbe_Pma_Reader_ಮುಂದಿನ(&pMerger->aReadr[iPrev]);

  /* Update contents of aTree[] */
  ಆದರೆ( rc==SQLITE_OK ){
    ಪೂರ್ಣಾಂಕ i;                      /* Index of aTree[] to recalculate */
    PmaReader *pReadr1;         /* First PmaReader to compare */
    PmaReader *pReadr2;         /* Second PmaReader to compare */
    ಪೂರ್ಣಾಂಕ bCached = 0;

    /* Find the first two PmaReaders to compare. The one that was just
    ** advanced (iPrev) and the one next to it in the array.  */
    pReadr1 = &pMerger->aReadr[(iPrev & 0xFFFE)];
    pReadr2 = &pMerger->aReadr[(iPrev | 0x0001)];

    ಗಾಗಿ(i=(pMerger->n_ವೃಕ್ಷ+iPrev)/2; i>0; i=i/2){
      /* Compare pReadr1 and pReadr2. Store the result in variable iRes. */
      ಪೂರ್ಣಾಂಕ iRes;
      ಆದರೆ( pReadr1->pFd==0 ){
        iRes = +1;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pReadr2->pFd==0 ){
        iRes = -1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        iRes = pTask->xCompare(pTask, &bCached,
            pReadr1->a_ಕೀಲಿ, pReadr1->n_ಕೀಲಿ, pReadr2->a_ಕೀಲಿ, pReadr2->n_ಕೀಲಿ
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
      ಆದರೆ( iRes<0 || (iRes==0 && pReadr1<pReadr2) ){
        pMerger->a_ವೃಕ್ಷ[i] = (ಪೂರ್ಣಾಂಕ)(pReadr1 - pMerger->aReadr);
        pReadr2 = &pMerger->aReadr[ pMerger->a_ವೃಕ್ಷ[i ^ 0x0001] ];
        bCached = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( pReadr1->pFd ) bCached = 0;
        pMerger->a_ವೃಕ್ಷ[i] = (ಪೂರ್ಣಾಂಕ)(pReadr2 - pMerger->aReadr);
        pReadr1 = &pMerger->aReadr[ pMerger->a_ವೃಕ್ಷ[i ^ 0x0001] ];
      }
    }
    *pbEof = (pMerger->aReadr[pMerger->a_ವೃಕ್ಷ[1]].pFd==0);
  }

  ಹಿಂತಿರುಗಿ (rc==SQLITE_OK ? pTask->pUnpacked->err_ಸಂಕೇತ : rc);
}

#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
/*
** The main routine for background threads that write level-0 PMAs.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *vdbeSorterFlushThread(ಶೂನ್ಯ *pCtx){
  ವಿಂಗಡಿಸು_Subtask *pTask = (ವಿಂಗಡಿಸು_Subtask*)pCtx;
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code */
  ಪ್ರತಿಜ್ಞಾ( pTask->b_ಮುಗಿದ==0 );
  rc = vdbe_Sorter_List_ಗಮ್ಯ_PMA(pTask, &pTask->list);
  pTask->b_ಮುಗಿದ = 1;
  ಹಿಂತಿರುಗಿ SQLITE_INT_ಗಮ್ಯ_PTR(rc);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_MAX_WORKER_THREADS>0 */

/*
** Flush the current contents of VdbeSorter.list to a new PMA, possibly
** using a background thread.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeSorterFlushPMA(VdbeSorter *pSorter){
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು==0
  pSorter->bUsePMA = 1;
  ಹಿಂತಿರುಗಿ vdbe_Sorter_List_ಗಮ್ಯ_PMA(&pSorter->aTask[0], &pSorter->list);
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ i;
  ವಿಂಗಡಿಸು_Subtask *pTask = 0;    /* Thread context used to create new PMA */
  ಪೂರ್ಣಾಂಕ n_ಕೆಲಸಗಾರ = (pSorter->nTask-1);

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
  ಗಾಗಿ(i=0; i<n_ಕೆಲಸಗಾರ; i++){
    ಪೂರ್ಣಾಂಕ iTest = (pSorter->iPrev + i + 1) % n_ಕೆಲಸಗಾರ;
    pTask = &pSorter->aTask[iTest];
    ಆದರೆ( pTask->b_ಮುಗಿದ ){
      rc = vdbeSorterJoinThread(pTask);
    }
    ಆದರೆ( rc!=SQLITE_OK || pTask->pThread==0 ) ನಿಲ್ಲಿಸು;
  }

  ಆದರೆ( rc==SQLITE_OK ){
    ಆದರೆ( i==n_ಕೆಲಸಗಾರ ){
      /* Use the foreground thread for this operation */
      rc = vdbe_Sorter_List_ಗಮ್ಯ_PMA(&pSorter->aTask[n_ಕೆಲಸಗಾರ], &pSorter->list);
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Launch a background thread for this operation */
      u8 *aMem;
      ಶೂನ್ಯ *pCtx;

      ಪ್ರತಿಜ್ಞಾ( pTask!=0 );
      ಪ್ರತಿಜ್ಞಾ( pTask->pThread==0 && pTask->b_ಮುಗಿದ==0 );
      ಪ್ರತಿಜ್ಞಾ( pTask->list.pList==0 );
      ಪ್ರತಿಜ್ಞಾ( pTask->list.a_ಸ್ಮರಣೆ==0 || pSorter->list.a_ಸ್ಮರಣೆ!=0 );

      aMem = pTask->list.a_ಸ್ಮರಣೆ;
      pCtx = (ಶೂನ್ಯ*)pTask;
      pSorter->iPrev = (u8)(pTask - pSorter->aTask);
      pTask->list = pSorter->list;
      pSorter->list.pList = 0;
      pSorter->list.szPMA = 0;
      ಆದರೆ( aMem ){
        pSorter->list.a_ಸ್ಮರಣೆ = aMem;
        pSorter->n_ಸ್ಮರಣೆ = sqlite3_Malloc_ಗಾತ್ರ(aMem);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pSorter->list.a_ಸ್ಮರಣೆ ){
        pSorter->list.a_ಸ್ಮರಣೆ = sqlite3Malloc(pSorter->n_ಸ್ಮರಣೆ);
        ಆದರೆ( !pSorter->list.a_ಸ್ಮರಣೆ ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
      }

      rc = vdbeSorterCreateThread(pTask, vdbeSorterFlushThread, pCtx);
    }
  }

  ಹಿಂತಿರುಗಿ rc;
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_MAX_WORKER_THREADS!=0 */
}

/*
** Add a record to the sorter.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeSorterWrite(
  ಸ್ಥಿರಾಂಕ VdbeCursor *pCsr,         /* Sorter cursor */
  Mem *pVal                       /* Memory cell containing record */
){
  VdbeSorter *pSorter;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;             /* Return Code */
  Sorter_ದಾಖಲೆ *pNew;             /* New list element */
  ಪೂರ್ಣಾಂಕ bFlush;                     /* True to flush contents of memory to PMA */
  i64 nReq;                       /* Bytes of memory required */
  i64 nPMA;                       /* Bytes of PMA space required */
  ಪೂರ್ಣಾಂಕ t;                          /* serial type of first record field */

  ಪ್ರತಿಜ್ಞಾ( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  getVarint32NR((ಸ್ಥಿರಾಂಕ u8*)&pVal->z[1], t);
  ಆದರೆ( t>0 && t<10 ){
    ಆದರೆ( t==7 ){
      pSorter->type_ಮುಖವಾಡ &= SORTER_TYPE_REAL;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pSorter->type_ಮುಖವಾಡ &= (SORTER_TYPE_INTEGER|SORTER_TYPE_REAL);
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( t>10 && (t & 0x01) ){
    pSorter->type_ಮುಖವಾಡ &= SORTER_TYPE_ಪಠ್ಯ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pSorter->type_ಮುಖವಾಡ = 0;
  }

  ಪ್ರತಿಜ್ಞಾ( pSorter );

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
  nReq = pVal->n + ಗಾತ್ರ(Sorter_ದಾಖಲೆ);
  nPMA = pVal->n + sqlite3VarintLen(pVal->n);
  ಆದರೆ( pSorter->mx_Pma_ಗಾತ್ರ ){
    ಆದರೆ( pSorter->list.a_ಸ್ಮರಣೆ ){
      bFlush = pSorter->i_ಸ್ಮರಣೆ && (pSorter->i_ಸ್ಮರಣೆ+nReq) > pSorter->mx_Pma_ಗಾತ್ರ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      bFlush = (
          (pSorter->list.szPMA > pSorter->mx_Pma_ಗಾತ್ರ)
       || (pSorter->list.szPMA > pSorter->mn_Pma_ಗಾತ್ರ && sqlite3_ರಾಶಿ_Nearly_ತುಂಬಿದ())
      );
    }
    ಆದರೆ( bFlush ){
      rc = vdbeSorterFlushPMA(pSorter);
      pSorter->list.szPMA = 0;
      pSorter->i_ಸ್ಮರಣೆ = 0;
      ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || pSorter->list.pList==0 );
    }
  }

  pSorter->list.szPMA += nPMA;
  ಆದರೆ( nPMA>pSorter->mxKeysize ){
    pSorter->mxKeysize = nPMA;
  }

  ಆದರೆ( pSorter->list.a_ಸ್ಮರಣೆ ){
    ಪೂರ್ಣಾಂಕ nMin = pSorter->i_ಸ್ಮರಣೆ + nReq;

    ಆದರೆ( nMin>pSorter->n_ಸ್ಮರಣೆ ){
      u8 *aNew;
      sqlite3_int64 nNew = 2 * (sqlite3_int64)pSorter->n_ಸ್ಮರಣೆ;
      ಪೂರ್ಣಾಂಕ iListOff = -1;
      ಆದರೆ( pSorter->list.pList ){
        iListOff = (u8*)pSorter->list.pList - pSorter->list.a_ಸ್ಮರಣೆ;
      }
      ಆಗ( nNew < nMin ) nNew = nNew*2;
      ಆದರೆ( nNew > pSorter->mx_Pma_ಗಾತ್ರ ) nNew = pSorter->mx_Pma_ಗಾತ್ರ;
      ಆದರೆ( nNew < nMin ) nNew = nMin;
      aNew = sqlite3Realloc(pSorter->list.a_ಸ್ಮರಣೆ, nNew);
      ಆದರೆ( !aNew ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
      ಆದರೆ( iListOff>=0 ){
        pSorter->list.pList = (Sorter_ದಾಖಲೆ*)&aNew[iListOff];
      }
      pSorter->list.a_ಸ್ಮರಣೆ = aNew;
      pSorter->n_ಸ್ಮರಣೆ = nNew;
    }

    pNew = (Sorter_ದಾಖಲೆ*)&pSorter->list.a_ಸ್ಮರಣೆ[pSorter->i_ಸ್ಮರಣೆ];
    pSorter->i_ಸ್ಮರಣೆ += ROUND8(nReq);
    ಆದರೆ( pSorter->list.pList ){
      pNew->u.i_ಮುಂದಿನ = (ಪೂರ್ಣಾಂಕ)((u8*)(pSorter->list.pList) - pSorter->list.a_ಸ್ಮರಣೆ);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    pNew = (Sorter_ದಾಖಲೆ *)sqlite3Malloc(nReq);
    ಆದರೆ( pNew==0 ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    }
    pNew->u.p_ಮುಂದಿನ = pSorter->list.pList;
  }

  ಸ್ಮರಣೆ_ನಕಲು(SRVAL(pNew), pVal->z, pVal->n);
  pNew->nVal = pVal->n;
  pSorter->list.pList = pNew;

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Read keys from pIncr->pMerger and populate pIncr->aFile[1]. The format
** of the data stored in aFile[1] is the same as that used by regular PMAs,
** except that the number-of-bytes varint is omitted from the start.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeIncrPopulate(IncrMerger *pIncr){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ rc2;
  i64 i_ಆರಂಭ = pIncr->i_ಆರಂಭ_Off;
  SorterFile *p_ಹೊರಹೋಗು = &pIncr->aFile[1];
  ವಿಂಗಡಿಸು_Subtask *pTask = pIncr->pTask;
  ವಿಲೀನ_Engine *pMerger = pIncr->pMerger;
  PmaWriter writer;
  ಪ್ರತಿಜ್ಞಾ( pIncr->bEof==0 );

  vdbeSorterPopulateDebug(pTask, "enter");

  vdbePmaWriterInit(p_ಹೊರಹೋಗು->pFd, &writer, pTask->pSorter->pgsz, i_ಆರಂಭ);
  ಆಗ( rc==SQLITE_OK ){
    ಪೂರ್ಣಾಂಕ dummy;
    PmaReader *pReader = &pMerger->aReadr[ pMerger->a_ವೃಕ್ಷ[1] ];
    ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ = pReader->n_ಕೀಲಿ;
    i64 iEof = writer.iWriteOff + writer.i_Buf_ಕೊನೆ;

    /* Check if the output file is full or if the input has been exhausted.
    ** In either case exit the loop. */
    ಆದರೆ( pReader->pFd==0 ) ನಿಲ್ಲಿಸು;
    ಆದರೆ( (iEof + n_ಕೀಲಿ + sqlite3VarintLen(n_ಕೀಲಿ))>(i_ಆರಂಭ + pIncr->mxSz) ) ನಿಲ್ಲಿಸು;

    /* Write the next key to the output. */
    vdbePmaWriteVarint(&writer, n_ಕೀಲಿ);
    vdbePmaWriteBlob(&writer, pReader->a_ಕೀಲಿ, n_ಕೀಲಿ);
    ಪ್ರತಿಜ್ಞಾ( pIncr->pMerger->pTask==pTask );
    rc = vdbe_ವಿಲೀನ_Engine_ಹೆಜ್ಜೆ(pIncr->pMerger, &dummy);
  }

  rc2 = vdbe_Pma_Writer_ಮುಕ್ತಾಯ(&writer, &p_ಹೊರಹೋಗು->iEof, &pTask->nSpill);
  ಆದರೆ( rc==SQLITE_OK ) rc = rc2;
  vdbeSorterPopulateDebug(pTask, "exit");
  ಹಿಂತಿರುಗಿ rc;
}

#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
/*
** The main routine for background threads that populate aFile[1] of
** multi-threaded IncrMerger objects.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *vdbeIncrPopulateThread(ಶೂನ್ಯ *pCtx){
  IncrMerger *pIncr = (IncrMerger*)pCtx;
  ಶೂನ್ಯ *pRet = SQLITE_INT_ಗಮ್ಯ_PTR( vdbeIncrPopulate(pIncr) );
  pIncr->pTask->b_ಮುಗಿದ = 1;
  ಹಿಂತಿರುಗಿ pRet;
}

/*
** Launch a background thread to populate aFile[1] of pIncr.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeIncrBgPopulate(IncrMerger *pIncr){
  ಶೂನ್ಯ *p = (ಶೂನ್ಯ*)pIncr;
  ಪ್ರತಿಜ್ಞಾ( pIncr->bUseThread );
  ಹಿಂತಿರುಗಿ vdbeSorterCreateThread(pIncr->pTask, vdbeIncrPopulateThread, p);
}
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Incr_ಅದಲುಬದಲು(IncrMerger *pIncr){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
  ಆದರೆ( pIncr->bUseThread ){
    rc = vdbeSorterJoinThread(pIncr->pTask);

    ಆದರೆ( rc==SQLITE_OK ){
      SorterFile f0 = pIncr->aFile[0];
      pIncr->aFile[0] = pIncr->aFile[1];
      pIncr->aFile[1] = f0;
    }

    ಆದರೆ( rc==SQLITE_OK ){
      ಆದರೆ( pIncr->aFile[0].iEof==pIncr->i_ಆರಂಭ_Off ){
        pIncr->bEof = 1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = vdbeIncrBgPopulate(pIncr);
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
  {
    rc = vdbeIncrPopulate(pIncr);
    pIncr->aFile[0] = pIncr->aFile[1];
    ಆದರೆ( pIncr->aFile[0].iEof==pIncr->i_ಆರಂಭ_Off ){
      pIncr->bEof = 1;
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Allocate and return a new IncrMerger object to read data from pMerger.
**
** If an OOM condition is encountered, return NULL. In this case free the
** pMerger argument before returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbeIncrMergerNew(
  ವಿಂಗಡಿಸು_Subtask *pTask,     /* The thread that will be using the new IncrMerger */
  ವಿಲೀನ_Engine *pMerger,   /* The MergeEngine that the IncrMerger will control */
  IncrMerger **pp_ಹೊರಹೋಗು      /* Write the new IncrMerger here */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  IncrMerger *pIncr = *pp_ಹೊರಹೋಗು = (IncrMerger*)
       (sqlite3FaultSim(100) ? 0 : sqlite3MallocZero(ಗಾತ್ರ(*pIncr)));
  ಆದರೆ( pIncr ){
    pIncr->pMerger = pMerger;
    pIncr->pTask = pTask;
    pIncr->mxSz = MAX(pTask->pSorter->mxKeysize+9,pTask->pSorter->mx_Pma_ಗಾತ್ರ/2);
    pTask->file2.iEof += pIncr->mxSz;
  }ಇಲ್ಲದಿದ್ದರೆ{
    vdbe_ವಿಲೀನ_Engine_Free(pMerger);
    rc = SQLITE_NOMEM_BKPT;
  }
  ಪ್ರತಿಜ್ಞಾ( *pp_ಹೊರಹೋಗು!=0 || rc!=SQLITE_OK );
  ಹಿಂತಿರುಗಿ rc;
}

#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
/*
** Set the "use-threads" flag on object pIncr.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbe_Incr_Merger_ಗಣ_ಎಳೆಗಳು(IncrMerger *pIncr){
  pIncr->bUseThread = 1;
  pIncr->pTask->file2.iEof -= pIncr->mxSz;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_MAX_WORKER_THREADS>0 */



/*
** Recompute pMerger->aTree[iOut] by comparing the next keys on the
** two PmaReaders that feed that entry.  Neither of the PmaReaders
** are advanced.  This routine merely does the comparison.
*/
ಸ್ಥಿರ ಶೂನ್ಯ vdbe_ವಿಲೀನ_Engine_Compare(
  ವಿಲೀನ_Engine *pMerger,  /* Merge engine containing PmaReaders to compare */
  ಪೂರ್ಣಾಂಕ i_ಹೊರಹೋಗು               /* Store the result in pMerger->aTree[iOut] */
){
  ಪೂರ್ಣಾಂಕ i1;
  ಪೂರ್ಣಾಂಕ i2;
  ಪೂರ್ಣಾಂಕ iRes;
  PmaReader *p1;
  PmaReader *p2;

  ಪ್ರತಿಜ್ಞಾ( i_ಹೊರಹೋಗು<pMerger->n_ವೃಕ್ಷ && i_ಹೊರಹೋಗು>0 );

  ಆದರೆ( i_ಹೊರಹೋಗು>=(pMerger->n_ವೃಕ್ಷ/2) ){
    i1 = (i_ಹೊರಹೋಗು - pMerger->n_ವೃಕ್ಷ/2) * 2;
    i2 = i1 + 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    i1 = pMerger->a_ವೃಕ್ಷ[i_ಹೊರಹೋಗು*2];
    i2 = pMerger->a_ವೃಕ್ಷ[i_ಹೊರಹೋಗು*2+1];
  }

  p1 = &pMerger->aReadr[i1];
  p2 = &pMerger->aReadr[i2];

  ಆದರೆ( p1->pFd==0 ){
    iRes = i2;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p2->pFd==0 ){
    iRes = i1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ವಿಂಗಡಿಸು_Subtask *pTask = pMerger->pTask;
    ಪೂರ್ಣಾಂಕ bCached = 0;
    ಪೂರ್ಣಾಂಕ res;
    ಪ್ರತಿಜ್ಞಾ( pTask->pUnpacked!=0 );  /* from vdbeSortSubtaskMain() */
    res = pTask->xCompare(
        pTask, &bCached, p1->a_ಕೀಲಿ, p1->n_ಕೀಲಿ, p2->a_ಕೀಲಿ, p2->n_ಕೀಲಿ
    );
    ಆದರೆ( res<=0 ){
      iRes = i1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      iRes = i2;
    }
  }

  pMerger->a_ವೃಕ್ಷ[i_ಹೊರಹೋಗು] = iRes;
}

/*
** Allowed values for the eMode parameter to vdbeMergeEngineInit()
** and vdbePmaReaderIncrMergeInit().
**
** Only INCRINIT_NORMAL is valid in single-threaded builds (when
** SQLITE_MAX_WORKER_THREADS==0).  The other values are only used
** when there exists one or more separate worker threads.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು INCRINIT_ಸಾಮಾನ್ಯ 0
#ವ್ಯಾಖ್ಯಾನಿಸು INCRINIT_TASK   1
#ವ್ಯಾಖ್ಯಾನಿಸು INCRINIT_ಬೇರು   2

/*
** Forward reference required as the vdbeIncrMergeInit() and
** vdbePmaReaderIncrInit() routines are called mutually recursively when
** building a merge tree.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbePmaReaderIncrInit(PmaReader *pReadr, ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ);

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_ವಿಲೀನ_Engine_Init(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Thread that will run pMerger */
  ವಿಲೀನ_Engine *pMerger,           /* MergeEngine to initialize */
  ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ                       /* One of the INCRINIT_XXX constants */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;             /* Return code */
  ಪೂರ್ಣಾಂಕ i;                          /* For looping over PmaReader objects */
  ಪೂರ್ಣಾಂಕ n_ವೃಕ್ಷ;                      /* Number of subtrees to merge */

  /* Failure to allocate the merge would have been detected prior to
  ** invoking this routine */
  ಪ್ರತಿಜ್ಞಾ( pMerger!=0 );

  /* eMode is always INCRINIT_NORMAL in single-threaded mode */
  ಪ್ರತಿಜ್ಞಾ( SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0 || e_ಬಹುಲಕ==INCRINIT_ಸಾಮಾನ್ಯ );

  /* Verify that the MergeEngine is assigned to a single thread */
  ಪ್ರತಿಜ್ಞಾ( pMerger->pTask==0 );
  pMerger->pTask = pTask;

  n_ವೃಕ್ಷ = pMerger->n_ವೃಕ್ಷ;
  ಗಾಗಿ(i=0; i<n_ವೃಕ್ಷ; i++){
    ಆದರೆ( SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0 && e_ಬಹುಲಕ==INCRINIT_ಬೇರು ){
      /* PmaReaders should be normally initialized in order, as if they are
      ** reading from the same temp file this makes for more linear file IO.
      ** However, in the INCRINIT_ROOT case, if PmaReader aReadr[nTask-1] is
      ** in use it will block the vdbePmaReaderNext() call while it uses
      ** the main thread to fill its buffer. So calling PmaReaderNext()
      ** on this PmaReader before any of the multi-threaded PmaReaders takes
      ** better advantage of multi-processor hardware. */
      rc = vdbe_Pma_Reader_ಮುಂದಿನ(&pMerger->aReadr[n_ವೃಕ್ಷ-i-1]);
    }ಇಲ್ಲದಿದ್ದರೆ{
      rc = vdbePmaReaderIncrInit(&pMerger->aReadr[i], INCRINIT_ಸಾಮಾನ್ಯ);
    }
    ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;
  }

  ಗಾಗಿ(i=pMerger->n_ವೃಕ್ಷ-1; i>0; i--){
    vdbe_ವಿಲೀನ_Engine_Compare(pMerger, i);
  }
  ಹಿಂತಿರುಗಿ pTask->pUnpacked->err_ಸಂಕೇತ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Pma_Reader_Incr_ವಿಲೀನ_Init(PmaReader *pReadr, ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  IncrMerger *pIncr = pReadr->pIncr;
  ವಿಂಗಡಿಸು_Subtask *pTask = pIncr->pTask;
  sqlite3 *db = pTask->pSorter->db;

  /* eMode is always INCRINIT_NORMAL in single-threaded mode */
  ಪ್ರತಿಜ್ಞಾ( SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0 || e_ಬಹುಲಕ==INCRINIT_ಸಾಮಾನ್ಯ );

  rc = vdbe_ವಿಲೀನ_Engine_Init(pTask, pIncr->pMerger, e_ಬಹುಲಕ);

  /* Set up the required files for pIncr. A multi-threaded IncrMerge object
  ** requires two temp files to itself, whereas a single-threaded object
  ** only requires a region of pTask->file2. */
  ಆದರೆ( rc==SQLITE_OK ){
    ಪೂರ್ಣಾಂಕ mxSz = pIncr->mxSz;
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
    ಆದರೆ( pIncr->bUseThread ){
      rc = vdbe_Sorter_Open_ತಾತ್ಕಾಲಿಕ_File(db, mxSz, &pIncr->aFile[0].pFd);
      ಆದರೆ( rc==SQLITE_OK ){
        rc = vdbe_Sorter_Open_ತಾತ್ಕಾಲಿಕ_File(db, mxSz, &pIncr->aFile[1].pFd);
      }
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    /*if( !pIncr->bUseThread )*/{
      ಆದರೆ( pTask->file2.pFd==0 ){
        ಪ್ರತಿಜ್ಞಾ( pTask->file2.iEof>0 );
        rc = vdbe_Sorter_Open_ತಾತ್ಕಾಲಿಕ_File(db, pTask->file2.iEof, &pTask->file2.pFd);
        pTask->file2.iEof = 0;
      }
      ಆದರೆ( rc==SQLITE_OK ){
        pIncr->aFile[1].pFd = pTask->file2.pFd;
        pIncr->i_ಆರಂಭ_Off = pTask->file2.iEof;
        pTask->file2.iEof += mxSz;
      }
    }
  }

#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
  ಆದರೆ( rc==SQLITE_OK && pIncr->bUseThread ){
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
    ಪ್ರತಿಜ್ಞಾ( e_ಬಹುಲಕ==INCRINIT_ಬೇರು || e_ಬಹುಲಕ==INCRINIT_TASK );
    rc = vdbeIncrPopulate(pIncr);
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( rc==SQLITE_OK && (SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು==0 || e_ಬಹುಲಕ!=INCRINIT_TASK) ){
    rc = vdbe_Pma_Reader_ಮುಂದಿನ(pReadr);
  }

  ಹಿಂತಿರುಗಿ rc;
}

#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
/*
** The main routine for vdbePmaReaderIncrMergeInit() operations run in
** background threads.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *vdbePmaReaderBgIncrInit(ಶೂನ್ಯ *pCtx){
  PmaReader *pReader = (PmaReader*)pCtx;
  ಶೂನ್ಯ *pRet = SQLITE_INT_ಗಮ್ಯ_PTR(
                  vdbe_Pma_Reader_Incr_ವಿಲೀನ_Init(pReader,INCRINIT_TASK)
               );
  pReader->pIncr->pTask->b_ಮುಗಿದ = 1;
  ಹಿಂತಿರುಗಿ pRet;
}
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbePmaReaderIncrInit(PmaReader *pReadr, ಪೂರ್ಣಾಂಕ e_ಬಹುಲಕ){
  IncrMerger *pIncr = pReadr->pIncr;   /* Incremental merger */
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;                  /* Return code */
  ಆದರೆ( pIncr ){
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
    ಪ್ರತಿಜ್ಞಾ( pIncr->bUseThread==0 || e_ಬಹುಲಕ==INCRINIT_TASK );
    ಆದರೆ( pIncr->bUseThread ){
      ಶೂನ್ಯ *pCtx = (ಶೂನ್ಯ*)pReadr;
      rc = vdbeSorterCreateThread(pIncr->pTask, vdbePmaReaderBgIncrInit, pCtx);
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
      rc = vdbe_Pma_Reader_Incr_ವಿಲೀನ_Init(pReadr, e_ಬಹುಲಕ);
    }
  }
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_ವಿಲೀನ_Engine_Level0(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Sorter task to read from */
  ಪೂರ್ಣಾಂಕ nPMA,                       /* Number of PMAs to read */
  i64 *pi_ಸ್ಥಾನಾಂತರ,                  /* IN/OUT: Readr offset in pTask->file */
  ವಿಲೀನ_Engine **pp_ಹೊರಹೋಗು             /* OUT: New merge-engine */
){
  ವಿಲೀನ_Engine *pNew;              /* Merge engine to return */
  i64 iOff = *pi_ಸ್ಥಾನಾಂತರ;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;

  *pp_ಹೊರಹೋಗು = pNew = vdbe_ವಿಲೀನ_Engine_New(nPMA);
  ಆದರೆ( pNew==0 ) rc = SQLITE_NOMEM_BKPT;

  ಗಾಗಿ(i=0; i<nPMA && rc==SQLITE_OK; i++){
    i64 nDummy = 0;
    PmaReader *pReadr = &pNew->aReadr[i];
    rc = vdbePmaReaderInit(pTask, &pTask->file, iOff, pReadr, &nDummy);
    iOff = pReadr->iEof;
  }

  ಆದರೆ( rc!=SQLITE_OK ){
    vdbe_ವಿಲೀನ_Engine_Free(pNew);
    *pp_ಹೊರಹೋಗು = 0;
  }
  *pi_ಸ್ಥಾನಾಂತರ = iOff;
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Sorter_ವೃಕ್ಷ_ಆಳ(ಪೂರ್ಣಾಂಕ nPMA){
  ಪೂರ್ಣಾಂಕ n_ಆಳ = 0;
  i64 nDiv = SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ;
  ಆಗ( nDiv < (i64)nPMA ){
    nDiv = nDiv * SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ;
    n_ಆಳ++;
  }
  ಹಿಂತಿರುಗಿ n_ಆಳ;
}

/*
** pRoot is the root of an incremental merge-tree with depth nDepth (according
** to vdbeSorterTreeDepth()). pLeaf is the iSeq'th leaf to be added to the
** tree, counting from zero. This function adds pLeaf to the tree.
**
** If successful, SQLITE_OK is returned. If an error occurs, an SQLite error
** code is returned and pLeaf is freed.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Sorter_ಕೂಡಿಸು_ಗಮ್ಯ_ವೃಕ್ಷ(
  ವಿಂಗಡಿಸು_Subtask *pTask,             /* Task context */
  ಪೂರ್ಣಾಂಕ n_ಆಳ,                     /* Depth of tree according to TreeDepth() */
  ಪೂರ್ಣಾಂಕ iSeq,                       /* Sequence number of leaf within tree */
  ವಿಲೀನ_Engine *p_ಬೇರು,             /* Root of tree */
  ವಿಲೀನ_Engine *p_ಎಲೆ              /* Leaf to add to tree */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ nDiv = 1;
  ಪೂರ್ಣಾಂಕ i;
  ವಿಲೀನ_Engine *p = p_ಬೇರು;
  IncrMerger *pIncr;

  rc = vdbeIncrMergerNew(pTask, p_ಎಲೆ, &pIncr);

  ಗಾಗಿ(i=1; i<n_ಆಳ; i++){
    nDiv = nDiv * SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ;
  }

  ಗಾಗಿ(i=1; i<n_ಆಳ && rc==SQLITE_OK; i++){
    ಪೂರ್ಣಾಂಕ iIter = (iSeq / nDiv) % SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ;
    PmaReader *pReadr = &p->aReadr[iIter];

    ಆದರೆ( pReadr->pIncr==0 ){
      ವಿಲೀನ_Engine *pNew = vdbe_ವಿಲೀನ_Engine_New(SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ);
      ಆದರೆ( pNew==0 ){
        rc = SQLITE_NOMEM_BKPT;
      }ಇಲ್ಲದಿದ್ದರೆ{
        rc = vdbeIncrMergerNew(pTask, pNew, &pReadr->pIncr);
      }
    }
    ಆದರೆ( rc==SQLITE_OK ){
      p = pReadr->pIncr->pMerger;
      nDiv = nDiv / SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ;
    }
  }

  ಆದರೆ( rc==SQLITE_OK ){
    p->aReadr[iSeq % SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ].pIncr = pIncr;
  }ಇಲ್ಲದಿದ್ದರೆ{
    vdbeIncrFree(pIncr);
  }
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Sorter_ವಿಲೀನ_ವೃಕ್ಷ_Build(
  VdbeSorter *pSorter,       /* The VDBE cursor that implements the sort */
  ವಿಲೀನ_Engine **pp_ಹೊರಹೋಗು        /* Write the MergeEngine here */
){
  ವಿಲೀನ_Engine *pMain = 0;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಪೂರ್ಣಾಂಕ iTask;

#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
  /* If the sorter uses more than one task, then create the top-level
  ** MergeEngine here. This MergeEngine will read data from exactly
  ** one PmaReader per sub-task.  */
  ಪ್ರತಿಜ್ಞಾ( pSorter->b_Use_ಎಳೆಗಳು || pSorter->nTask==1 );
  ಆದರೆ( pSorter->nTask>1 ){
    pMain = vdbe_ವಿಲೀನ_Engine_New(pSorter->nTask);
    ಆದರೆ( pMain==0 ) rc = SQLITE_NOMEM_BKPT;
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಗಾಗಿ(iTask=0; rc==SQLITE_OK && iTask<pSorter->nTask; iTask++){
    ವಿಂಗಡಿಸು_Subtask *pTask = &pSorter->aTask[iTask];
    ಪ್ರತಿಜ್ಞಾ( pTask->nPMA>0 || SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0 );
    ಆದರೆ( SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು==0 || pTask->nPMA ){
      ವಿಲೀನ_Engine *p_ಬೇರು = 0;     /* Root node of tree for this task */
      ಪೂರ್ಣಾಂಕ n_ಆಳ = vdbe_Sorter_ವೃಕ್ಷ_ಆಳ(pTask->nPMA);
      i64 iReadOff = 0;

      ಆದರೆ( pTask->nPMA<=SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ ){
        rc = vdbe_ವಿಲೀನ_Engine_Level0(pTask, pTask->nPMA, &iReadOff, &p_ಬೇರು);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ i;
        ಪೂರ್ಣಾಂಕ iSeq = 0;
        p_ಬೇರು = vdbe_ವಿಲೀನ_Engine_New(SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ);
        ಆದರೆ( p_ಬೇರು==0 ) rc = SQLITE_NOMEM_BKPT;
        ಗಾಗಿ(i=0; i<pTask->nPMA && rc==SQLITE_OK; i += SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ){
          ವಿಲೀನ_Engine *pMerger = 0; /* New level-0 PMA merger */
          ಪೂರ್ಣಾಂಕ nReader;              /* Number of level-0 PMAs to merge */

          nReader = MIN(pTask->nPMA - i, SORTER_MAX_ವಿಲೀನ_ಎಣಿಕೆ);
          rc = vdbe_ವಿಲೀನ_Engine_Level0(pTask, nReader, &iReadOff, &pMerger);
          ಆದರೆ( rc==SQLITE_OK ){
            rc = vdbe_Sorter_ಕೂಡಿಸು_ಗಮ್ಯ_ವೃಕ್ಷ(pTask, n_ಆಳ, iSeq++, p_ಬೇರು, pMerger);
          }
        }
      }

      ಆದರೆ( rc==SQLITE_OK ){
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
        ಆದರೆ( pMain!=0 ){
          rc = vdbeIncrMergerNew(pTask, p_ಬೇರು, &pMain->aReadr[iTask].pIncr);
        }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
        {
          ಪ್ರತಿಜ್ಞಾ( pMain==0 );
          pMain = p_ಬೇರು;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        vdbe_ವಿಲೀನ_Engine_Free(p_ಬೇರು);
      }
    }
  }

  ಆದರೆ( rc!=SQLITE_OK ){
    vdbe_ವಿಲೀನ_Engine_Free(pMain);
    pMain = 0;
  }
  *pp_ಹೊರಹೋಗು = pMain;
  ಹಿಂತಿರುಗಿ rc;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ vdbe_Sorter_ಸಿದ್ಧತೆ_ವಿಲೀನ(VdbeSorter *pSorter){
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code */
  ವಿಂಗಡಿಸು_Subtask *pTask0 = &pSorter->aTask[0];
  ವಿಲೀನ_Engine *pMain = 0;
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು
  sqlite3 *db = pTask0->pSorter->db;
  ಪೂರ್ಣಾಂಕ i;
  SorterCompare xCompare = vdbeSorterGetCompare(pSorter);
  ಗಾಗಿ(i=0; i<pSorter->nTask; i++){
    pSorter->aTask[i].xCompare = xCompare;
  }
#ಆದರೆ_ಅಂತ್ಯ

  rc = vdbe_Sorter_ವಿಲೀನ_ವೃಕ್ಷ_Build(pSorter, &pMain);
  ಆದರೆ( rc==SQLITE_OK ){
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು
    ಪ್ರತಿಜ್ಞಾ( pSorter->b_Use_ಎಳೆಗಳು==0 || pSorter->nTask>1 );
    ಆದರೆ( pSorter->b_Use_ಎಳೆಗಳು ){
      ಪೂರ್ಣಾಂಕ iTask;
      PmaReader *pReadr = 0;
      ವಿಂಗಡಿಸು_Subtask *p_ಕೊನೆಯ = &pSorter->aTask[pSorter->nTask-1];
      rc = vdbe_ವಿಂಗಡಿಸು_Alloc_Unpacked(p_ಕೊನೆಯ);
      ಆದರೆ( rc==SQLITE_OK ){
        pReadr = (PmaReader*)sqlite3DbMallocZero(db, ಗಾತ್ರ(PmaReader));
        pSorter->pReader = pReadr;
        ಆದರೆ( pReadr==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      ಆದರೆ( rc==SQLITE_OK ){
        rc = vdbeIncrMergerNew(p_ಕೊನೆಯ, pMain, &pReadr->pIncr);
        ಆದರೆ( rc==SQLITE_OK ){
          vdbe_Incr_Merger_ಗಣ_ಎಳೆಗಳು(pReadr->pIncr);
          ಗಾಗಿ(iTask=0; iTask<(pSorter->nTask-1); iTask++){
            IncrMerger *pIncr;
            ಆದರೆ( (pIncr = pMain->aReadr[iTask].pIncr) ){
              vdbe_Incr_Merger_ಗಣ_ಎಳೆಗಳು(pIncr);
              ಪ್ರತಿಜ್ಞಾ( pIncr->pTask!=p_ಕೊನೆಯ );
            }
          }
          ಗಾಗಿ(iTask=0; rc==SQLITE_OK && iTask<pSorter->nTask; iTask++){
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
            ಪ್ರತಿಜ್ಞಾ( p->pIncr==0 || (
                (p->pIncr->pTask==&pSorter->aTask[iTask])             /* a */
             && (iTask!=pSorter->nTask-1 || p->pIncr->bUseThread==0)  /* b */
            ));
            rc = vdbePmaReaderIncrInit(p, INCRINIT_TASK);
          }
        }
        pMain = 0;
      }
      ಆದರೆ( rc==SQLITE_OK ){
        rc = vdbe_Pma_Reader_Incr_ವಿಲೀನ_Init(pReadr, INCRINIT_ಬೇರು);
      }
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
      rc = vdbe_ವಿಲೀನ_Engine_Init(pTask0, pMain, INCRINIT_ಸಾಮಾನ್ಯ);
      pSorter->pMerger = pMain;
      pMain = 0;
    }
  }

  ಆದರೆ( rc!=SQLITE_OK ){
    vdbe_ವಿಲೀನ_Engine_Free(pMain);
  }
  ಹಿಂತಿರುಗಿ rc;
}


/*
** Once the sorter has been populated by calls to sqlite3VdbeSorterWrite,
** this function is called to prepare for iterating through the records
** in sorted order.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeSorterRewind(ಸ್ಥಿರಾಂಕ VdbeCursor *pCsr, ಪೂರ್ಣಾಂಕ *pbEof){
  VdbeSorter *pSorter;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;             /* Return code */

  ಪ್ರತಿಜ್ಞಾ( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  ಪ್ರತಿಜ್ಞಾ( pSorter );

  /* If no data has been written to disk, then do not do so now. Instead,
  ** sort the VdbeSorter.pRecord list. The vdbe layer will read data directly
  ** from the in-memory list.  */
  ಆದರೆ( pSorter->bUsePMA==0 ){
    ಆದರೆ( pSorter->list.pList ){
      *pbEof = 0;
      rc = vdbe_Sorter_ವಿಂಗಡಿಸು(&pSorter->aTask[0], &pSorter->list);
    }ಇಲ್ಲದಿದ್ದರೆ{
      *pbEof = 1;
    }
    ಹಿಂತಿರುಗಿ rc;
  }

  /* Write the current in-memory list to a PMA. When the VdbeSorterWrite()
  ** function flushes the contents of memory to disk, it immediately always
  ** creates a new list consisting of a single key immediately afterwards.
  ** So the list is never empty at this point.  */
  ಪ್ರತಿಜ್ಞಾ( pSorter->list.pList );
  rc = vdbeSorterFlushPMA(pSorter);

  /* Join all threads */
  rc = vdbeSorterJoinAll(pSorter, rc);

  vdbeSorterRewindDebug("rewind");

  /* Assuming no errors have occurred, set up a merger structure to
  ** incrementally read and merge all remaining PMAs.  */
  ಪ್ರತಿಜ್ಞಾ( pSorter->pReader==0 );
  ಆದರೆ( rc==SQLITE_OK ){
    rc = vdbe_Sorter_ಸಿದ್ಧತೆ_ವಿಲೀನ(pSorter);
    *pbEof = 0;
  }

  vdbeSorterRewindDebug("rewinddone");
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Advance to the next element in the sorter.  Return value:
**
**    SQLITE_OK     success
**    SQLITE_DONE   end of data
**    otherwise     some kind of error.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Vdbe_Sorter_ಮುಂದಿನ(sqlite3 *db, ಸ್ಥಿರಾಂಕ VdbeCursor *pCsr){
  VdbeSorter *pSorter;
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code */

  ಪ್ರತಿಜ್ಞಾ( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  ಪ್ರತಿಜ್ಞಾ( pSorter->bUsePMA || (pSorter->pReader==0 && pSorter->pMerger==0) );
  ಆದರೆ( pSorter->bUsePMA ){
    ಪ್ರತಿಜ್ಞಾ( pSorter->pReader==0 || pSorter->pMerger==0 );
    ಪ್ರತಿಜ್ಞಾ( pSorter->b_Use_ಎಳೆಗಳು==0 || pSorter->pReader );
    ಪ್ರತಿಜ್ಞಾ( pSorter->b_Use_ಎಳೆಗಳು==1 || pSorter->pMerger );
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
    ಆದರೆ( pSorter->b_Use_ಎಳೆಗಳು ){
      rc = vdbe_Pma_Reader_ಮುಂದಿನ(pSorter->pReader);
      ಆದರೆ( rc==SQLITE_OK && pSorter->pReader->pFd==0 ) rc = SQLITE_ಮುಗಿದ;
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    /*if( !pSorter->bUseThreads )*/ {
      ಪೂರ್ಣಾಂಕ res = 0;
      ಪ್ರತಿಜ್ಞಾ( pSorter->pMerger!=0 );
      ಪ್ರತಿಜ್ಞಾ( pSorter->pMerger->pTask==(&pSorter->aTask[0]) );
      rc = vdbe_ವಿಲೀನ_Engine_ಹೆಜ್ಜೆ(pSorter->pMerger, &res);
      ಆದರೆ( rc==SQLITE_OK && res ) rc = SQLITE_ಮುಗಿದ;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    Sorter_ದಾಖಲೆ *pFree = pSorter->list.pList;
    pSorter->list.pList = pFree->u.p_ಮುಂದಿನ;
    pFree->u.p_ಮುಂದಿನ = 0;
    ಆದರೆ( pSorter->list.a_ಸ್ಮರಣೆ==0 ) vdbe_Sorter_ದಾಖಲೆ_Free(db, pFree);
    rc = pSorter->list.pList ? SQLITE_OK : SQLITE_ಮುಗಿದ;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return a pointer to a buffer owned by the sorter that contains the
** current key.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *vdbeSorterRowkey(
  ಸ್ಥಿರಾಂಕ VdbeSorter *pSorter,      /* Sorter object */
  ಪೂರ್ಣಾಂಕ *pn_ಕೀಲಿ                      /* OUT: Size of current key in bytes */
){
  ಶೂನ್ಯ *p_ಕೀಲಿ;
  ಆದರೆ( pSorter->bUsePMA ){
    PmaReader *pReader;
#ಆದರೆ SQLITE_MAX_ಕೆಲಸಗಾರ_ಎಳೆಗಳು>0
    ಆದರೆ( pSorter->b_Use_ಎಳೆಗಳು ){
      pReader = pSorter->pReader;
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    /*if( !pSorter->bUseThreads )*/{
      pReader = &pSorter->pMerger->aReadr[pSorter->pMerger->a_ವೃಕ್ಷ[1]];
    }
    *pn_ಕೀಲಿ = pReader->n_ಕೀಲಿ;
    p_ಕೀಲಿ = pReader->a_ಕೀಲಿ;
  }ಇಲ್ಲದಿದ್ದರೆ{
    *pn_ಕೀಲಿ = pSorter->list.pList->nVal;
    p_ಕೀಲಿ = SRVAL(pSorter->list.pList);
  }
  ಹಿಂತಿರುಗಿ p_ಕೀಲಿ;
}

/*
** Copy the current sorter key into the memory cell pOut.
*/
ಪೂರ್ಣಾಂಕ sqlite3VdbeSorterRowkey(ಸ್ಥಿರಾಂಕ VdbeCursor *pCsr, Mem *p_ಹೊರಹೋಗು){
  VdbeSorter *pSorter;
  ಶೂನ್ಯ *p_ಕೀಲಿ; ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ;           /* Sorter key to copy into pOut */

  ಪ್ರತಿಜ್ಞಾ( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  p_ಕೀಲಿ = vdbeSorterRowkey(pSorter, &n_ಕೀಲಿ);
  ಆದರೆ( sqlite3VdbeMemClearAndResize(p_ಹೊರಹೋಗು, n_ಕೀಲಿ) ){
    ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
  }
  p_ಹೊರಹೋಗು->n = n_ಕೀಲಿ;
  Mem_ಗಣ_Type_Flag(p_ಹೊರಹೋಗು, MEM_Blob);
  ಸ್ಮರಣೆ_ನಕಲು(p_ಹೊರಹೋಗು->z, p_ಕೀಲಿ, n_ಕೀಲಿ);

  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಪೂರ್ಣಾಂಕ sqlite3VdbeSorterCompare(
  ಸ್ಥಿರಾಂಕ VdbeCursor *pCsr,         /* Sorter cursor */
  Mem *pVal,                      /* Value to compare to current sorter key */
  ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ_ಲಂಬಸಾಲು,                    /* Compare this many columns */
  ಪೂರ್ಣಾಂಕ *pRes                       /* OUT: Result of comparison */
){
  VdbeSorter *pSorter;
  Unpacked_ದಾಖಲೆ *r2;
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;
  ಪೂರ್ಣಾಂಕ i;
  ಶೂನ್ಯ *p_ಕೀಲಿ; ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ;           /* Sorter key to compare pVal with */

  ಪ್ರತಿಜ್ಞಾ( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  r2 = pSorter->pUnpacked;
  p_ಕೀಲಿ_Info = pCsr->p_ಕೀಲಿ_Info;
  ಆದರೆ( r2==0 ){
    r2 = pSorter->pUnpacked = sqlite3_Vdbe_Alloc_Unpacked_ದಾಖಲೆ(p_ಕೀಲಿ_Info);
    ಆದರೆ( r2==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
    r2->n_ಕ್ಷೇತ್ರ = n_ಕೀಲಿ_ಲಂಬಸಾಲು;
  }
  ಪ್ರತಿಜ್ಞಾ( r2->n_ಕ್ಷೇತ್ರ==n_ಕೀಲಿ_ಲಂಬಸಾಲು );

  p_ಕೀಲಿ = vdbeSorterRowkey(pSorter, &n_ಕೀಲಿ);
  sqlite3_Vdbe_ದಾಖಲೆ_Unpack(n_ಕೀಲಿ, p_ಕೀಲಿ, r2);
  ಗಾಗಿ(i=0; i<n_ಕೀಲಿ_ಲಂಬಸಾಲು; i++){
    ಆದರೆ( r2->aMem[i].flags & MEM_Null ){
      *pRes = -1;
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
  }

  *pRes = sqlite3_Vdbe_ದಾಖಲೆ_Compare(pVal->n, pVal->z, r2);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
