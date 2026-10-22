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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "vdbeInt.h"

/*
** If SQLITE_DEBUG_SORTER_THREADS is defined, this module outputs various
** messages to stderr that may be helpful in understanding the performance
** characteristics of the sorter in multi-threaded mode.
*/
#ఒకవేళ 0
# define SQLITE_DEBUG_SORTER_దారాలు 1
#అయితే_ముగింపు

/*
** Hard-coded maximum amount of data to accumulate in memory before flushing
** to a level 0 PMA. The purpose of this limit is to prevent various integer
** overflows. 512MiB.
*/
#నిర్వచించు SQLITE_MAX_PMASZ    (1<<29)

/*
** Private objects used by the sorter
*/
రకనిర్వచనం నిర్మాణం విలీనం_Engine విలీనం_Engine;     /* Merge PMAs together */
రకనిర్వచనం నిర్మాణం PmaReader PmaReader;         /* Incrementally read one PMA */
రకనిర్వచనం నిర్మాణం PmaWriter PmaWriter;         /* Incrementally write one PMA */
రకనిర్వచనం నిర్మాణం Sorter_రికార్డు Sorter_రికార్డు;   /* A record being sorted */
రకనిర్వచనం నిర్మాణం క్రమీకరించు_Subtask క్రమీకరించు_Subtask;     /* A sub-task in the sort process */
రకనిర్వచనం నిర్మాణం SorterFile SorterFile;       /* Temporary file object wrapper */
రకనిర్వచనం నిర్మాణం SorterList SorterList;       /* In-memory list of records */
రకనిర్వచనం నిర్మాణం IncrMerger IncrMerger;       /* Read & merge multiple PMAs */

/*
** A container for a temp file handle and the current amount of data
** stored in the file.
*/
నిర్మాణం SorterFile {
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
నిర్మాణం SorterList {
  Sorter_రికార్డు *pList;            /* Linked list of records */
  u8 *a_జ్ఞాపకం;                    /* If non-NULL, bulk memory to hold pList */
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
నిర్మాణం విలీనం_Engine {
  పూర్ణాంకం n_వృక్షం;                 /* Used size of aTree/aReadr (power of 2) */
  క్రమీకరించు_Subtask *pTask;        /* Used by this thread only */
  పూర్ణాంకం *a_వృక్షం;                /* Current state of incremental merge */
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
రకనిర్వచనం పూర్ణాంకం (*SorterCompare)(క్రమీకరించు_Subtask*,పూర్ణాంకం*,స్థిరరాశి శూన్య*,పూర్ణాంకం,స్థిరరాశి శూన్య*,పూర్ణాంకం);
నిర్మాణం క్రమీకరించు_Subtask {
  SQLiteThread *pThread;          /* Background thread, if any */
  పూర్ణాంకం b_పూర్తి;                      /* Set if thread is finished but not joined */
  పూర్ణాంకం nPMA;                       /* Number of PMAs currently in file */
  VdbeSorter *pSorter;            /* Sorter that owns this sub-task */
  Unpacked_రికార్డు *pUnpacked;      /* Space to unpack a record */
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
నిర్మాణం VdbeSorter {
  పూర్ణాంకం mn_Pma_కొలత;                  /* Minimum PMA size, in bytes */
  పూర్ణాంకం mx_Pma_కొలత;                  /* Maximum PMA size, in bytes.  0==no limit */
  పూర్ణాంకం mxKeysize;                  /* Largest serialized key seen so far */
  పూర్ణాంకం pgsz;                       /* Main database page size */
  PmaReader *pReader;             /* Readr data from here after Rewind() */
  విలీనం_Engine *pMerger;           /* Or here, if bUseThreads==0 */
  sqlite3 *db;                    /* Database connection */
  కీలకం_Info *p_కీలకం_Info;              /* How to compare records */
  Unpacked_రికార్డు *pUnpacked;      /* Used by VdbeSorterCompare() */
  SorterList list;                /* List of in-memory records */
  పూర్ణాంకం i_జ్ఞాపకం;                    /* Offset of free space in list.aMemory */
  పూర్ణాంకం n_జ్ఞాపకం;                    /* Size of list.aMemory allocation in bytes */
  u8 bUsePMA;                     /* True if one or more PMAs created */
  u8 b_Use_దారాలు;                 /* True to use background threads */
  u8 iPrev;                       /* Previous thread used to flush PMA */
  u8 nTask;                       /* Size of aTask[] array */
  u8 type_ముసుగు;
  క్రమీకరించు_Subtask aTask[FLEXARRAY];   /* One or more subtasks */
};

/* Size (in bytes) of a VdbeSorter object that works with N or fewer subtasks */
#నిర్వచించు SZ_VDBESORTER(N)  (offsetof(VdbeSorter,aTask)+(N)*పరిమాణం(క్రమీకరించు_Subtask))

#నిర్వచించు SORTER_TYPE_INTEGER 0x01
#నిర్వచించు SORTER_TYPE_పాఠ్యం    0x02
#నిర్వచించు SORTER_TYPE_REAL    0x04

/*
** An instance of the following object is used to read records out of a
** PMA, in sorted order.  The next key to be read is cached in nKey/aKey.
** aKey might point into aMap or into aBuffer.  If neither of those locations
** contain a contiguous representation of the key, then aAlloc is allocated
** and the key is copied into aAlloc and aKey is made to point to aAlloc.
**
** pFd==0 at EOF.
*/
నిర్మాణం PmaReader {
  i64 iReadOff;               /* Current read offset */
  i64 iEof;                   /* 1 byte past EOF for this PmaReader */
  పూర్ణాంకం nAlloc;                 /* Bytes of space at aAlloc */
  పూర్ణాంకం n_కీలకం;                   /* Number of bytes in key */
  sqlite3_file *pFd;          /* File handle we are reading from */
  u8 *aAlloc;                 /* Space for aKey if aBuffer and pMap wont work */
  u8 *a_కీలకం;                   /* Pointer to current key */
  u8 *a_బఫరు;                /* Current read buffer */
  పూర్ణాంకం n_బఫరు;                /* Size of read buffer in bytes */
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
నిర్మాణం IncrMerger {
  క్రమీకరించు_Subtask *pTask;             /* Task that owns this merger */
  విలీనం_Engine *pMerger;           /* Merge engine thread reads data from */
  i64 i_ప్రారంభం_Off;                  /* Offset to start writing file at */
  పూర్ణాంకం mxSz;                       /* Maximum bytes of data to store */
  పూర్ణాంకం bEof;                       /* Set to true when merge is finished */
  పూర్ణాంకం bUseThread;                 /* True to use a bg thread for this object */
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
నిర్మాణం PmaWriter {
  పూర్ణాంకం eFWErr;                     /* Non-zero if in an error state */
  u8 *a_బఫరు;                    /* Pointer to write buffer */
  పూర్ణాంకం n_బఫరు;                    /* Size of write buffer in bytes */
  పూర్ణాంకం i_Buf_ప్రారంభం;                  /* First byte of buffer to write */
  పూర్ణాంకం i_Buf_చివర;                    /* Last byte of buffer to write */
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
నిర్మాణం Sorter_రికార్డు {
  పూర్ణాంకం nVal;                       /* Size of the record in bytes */
  సంఘం {
    Sorter_రికార్డు *p_తదుపరి;          /* Pointer to next record in list */
    పూర్ణాంకం i_తదుపరి;                    /* Offset within aMemory of next record */
  } u;
  /* The data for the record immediately follows this header */
};

/* Return a pointer to the buffer containing the record data for SorterRecord
** object p. Should be used as if:
**
**   void *SRVAL(SorterRecord *p) { return (void*)&p[1]; }
*/
#నిర్వచించు SRVAL(p) ((శూన్య*)((Sorter_రికార్డు*)(p) + 1))


/* Maximum number of PMAs that a single MergeEngine can merge */
#నిర్వచించు SORTER_MAX_విలీనం_లెక్క 16

స్థిర పూర్ణాంకం vdbe_Incr_మార్పిడి(IncrMerger*);
స్థిర శూన్య vdbeIncrFree(IncrMerger *);

/*
** Free all memory belonging to the PmaReader object passed as the
** argument. All structure fields are set to zero before returning.
*/
స్థిర శూన్య vdbePmaReaderClear(PmaReader *pReadr){
  sqlite3_free(pReadr->aAlloc);
  sqlite3_free(pReadr->a_బఫరు);
  ఒకవేళ( pReadr->aMap ) sqlite3OsUnfetch(pReadr->pFd, 0, pReadr->aMap);
  vdbeIncrFree(pReadr->pIncr);
  memset(pReadr, 0, పరిమాణం(PmaReader));
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
స్థిర పూర్ణాంకం vdbePmaReadBlob(
  PmaReader *p,                   /* PmaReader from which to take the blob */
  పూర్ణాంకం nByte,                      /* Bytes of data to read */
  u8 **pp_బహిర్గతం                      /* OUT: Pointer to buffer containing data */
){
  పూర్ణాంకం iBuf;                       /* Offset within buffer to read from */
  పూర్ణాంకం nAvail;                     /* Bytes of data available in buffer */

  ఒకవేళ( p->aMap ){
    *pp_బహిర్గతం = &p->aMap[p->iReadOff];
    p->iReadOff += nByte;
    తిరిగి SQLITE_OK;
  }

  ప్రతిజ్ఞా( p->a_బఫరు );

  /* If there is no more data to be read from the buffer, read the next
  ** p->nBuffer bytes of data from the file into it. Or, if there are less
  ** than p->nBuffer bytes remaining in the PMA, read all remaining data.  */
  iBuf = p->iReadOff % p->n_బఫరు;
  ఒకవేళ( iBuf==0 ){
    పూర్ణాంకం nRead;                    /* Bytes to read from disk */
    పూర్ణాంకం rc;                       /* sqlite3OsRead() return code */

    /* Determine how many bytes of data to read. */
    ఒకవేళ( (p->iEof - p->iReadOff) > (i64)p->n_బఫరు ){
      nRead = p->n_బఫరు;
    }లేకపోతే{
      nRead = (పూర్ణాంకం)(p->iEof - p->iReadOff);
    }
    ప్రతిజ్ఞా( nRead>0 );

    /* Readr data from the file. Return early if an error occurs. */
    rc = sqlite3OsRead(p->pFd, p->a_బఫరు, nRead, p->iReadOff);
    ప్రతిజ్ఞా( rc!=SQLITE_IOERR_SHORT_READ );
    ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
  }
  nAvail = p->n_బఫరు - iBuf;

  ఒకవేళ( nByte<=nAvail ){
    /* The requested data is available in the in-memory buffer. In this
    ** case there is no need to make a copy of the data, just return a
    ** pointer into the buffer to the caller.  */
    *pp_బహిర్గతం = &p->a_బఫరు[iBuf];
    p->iReadOff += nByte;
  }లేకపోతే{
    /* The requested data is not all available in the in-memory buffer.
    ** In this case, allocate space at p->aAlloc[] to copy the requested
    ** range into. Then return a copy of pointer p->aAlloc to the caller.  */
    పూర్ణాంకం nRem;                     /* Bytes remaining to copy */

    /* Extend the p->aAlloc[] allocation if required. */
    ఒకవేళ( p->nAlloc<nByte ){
      u8 *aNew;
      sqlite3_int64 nNew = MAX(128, 2*(sqlite3_int64)p->nAlloc);
      అయితే( nByte>nNew ) nNew = nNew*2;
      aNew = sqlite3Realloc(p->aAlloc, nNew);
      ఒకవేళ( !aNew ) తిరిగి SQLITE_NOMEM_BKPT;
      p->nAlloc = nNew;
      p->aAlloc = aNew;
    }

    /* Copy as much data as is available in the buffer into the start of
    ** p->aAlloc[].  */
    జ్ఞాపకం_నకలు(p->aAlloc, &p->a_బఫరు[iBuf], nAvail);
    p->iReadOff += nAvail;
    nRem = nByte - nAvail;

    /* The following loop copies up to p->nBuffer bytes per iteration into
    ** the p->aAlloc[] buffer.  */
    అయితే( nRem>0 ){
      పూర్ణాంకం rc;                     /* vdbePmaReadBlob() return code */
      పూర్ణాంకం nCopy;                  /* Number of bytes to copy */
      u8 *a_తదుపరి = 0;              /* Pointer to buffer to copy data from */

      nCopy = nRem;
      ఒకవేళ( nRem>p->n_బఫరు ) nCopy = p->n_బఫరు;
      rc = vdbePmaReadBlob(p, nCopy, &a_తదుపరి);
      ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
      ప్రతిజ్ఞా( a_తదుపరి!=p->aAlloc );
      ప్రతిజ్ఞా( a_తదుపరి!=0 );
      జ్ఞాపకం_నకలు(&p->aAlloc[nByte - nRem], a_తదుపరి, nCopy);
      nRem -= nCopy;
    }

    *pp_బహిర్గతం = p->aAlloc;
  }

  తిరిగి SQLITE_OK;
}

/*
** Read a varint from the stream of data accessed by p. Set *pnOut to
** the value read.
*/
స్థిర పూర్ణాంకం vdbePmaReadVarint(PmaReader *p, u64 *pn_బహిర్గతం){
  పూర్ణాంకం iBuf;

  ఒకవేళ( p->aMap ){
    p->iReadOff += sqlite3GetVarint(&p->aMap[p->iReadOff], pn_బహిర్గతం);
  }లేకపోతే{
    iBuf = p->iReadOff % p->n_బఫరు;
    ఒకవేళ( iBuf && (p->n_బఫరు-iBuf)>=9 ){
      p->iReadOff += sqlite3GetVarint(&p->a_బఫరు[iBuf], pn_బహిర్గతం);
    }లేకపోతే{
      u8 aVarint[16], *a;
      పూర్ణాంకం i = 0, rc;
      చేయి{
        rc = vdbePmaReadBlob(p, 1, &a);
        ఒకవేళ( rc ) తిరిగి rc;
        aVarint[(i++)&0xf] = a[0];
      }అయితే( (a[0]&0x80)!=0 );
      sqlite3GetVarint(aVarint, pn_బహిర్గతం);
    }
  }

  తిరిగి SQLITE_OK;
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
స్థిర పూర్ణాంకం vdbeSorterMapFile(క్రమీకరించు_Subtask *pTask, SorterFile *pFile, u8 **pp){
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( pFile->iEof<=(i64)(pTask->pSorter->db->nMaxSorterMmap) ){
    sqlite3_file *pFd = pFile->pFd;
    ఒకవేళ( pFd->pMethods->i_సంస్కరణ>=3 ){
      rc = sqlite3OsFetch(pFd, 0, (పూర్ణాంకం)pFile->iEof, (శూన్య**)pp);
      testcase( rc!=SQLITE_OK );
    }
  }
  తిరిగి rc;
}

/*
** Attach PmaReader pReadr to file pFile (if it is not already attached to
** that file) and seek it to offset iOff within the file.  Return SQLITE_OK
** if successful, or an SQLite error code if an error occurs.
*/
స్థిర పూర్ణాంకం vdbePmaReaderSeek(
  క్రమీకరించు_Subtask *pTask,             /* Task context */
  PmaReader *pReadr,              /* Reader whose cursor is to be moved */
  SorterFile *pFile,              /* Sorter file to read from */
  i64 iOff                        /* Offset in pFile */
){
  పూర్ణాంకం rc = SQLITE_OK;

  ప్రతిజ్ఞా( pReadr->pIncr==0 || pReadr->pIncr->bEof==0 );

  ఒకవేళ( sqlite3FaultSim(201) ) తిరిగి SQLITE_IOERR_READ;
  ఒకవేళ( pReadr->aMap ){
    sqlite3OsUnfetch(pReadr->pFd, 0, pReadr->aMap);
    pReadr->aMap = 0;
  }
  pReadr->iReadOff = iOff;
  pReadr->iEof = pFile->iEof;
  pReadr->pFd = pFile->pFd;

  rc = vdbeSorterMapFile(pTask, pFile, &pReadr->aMap);
  ఒకవేళ( rc==SQLITE_OK && pReadr->aMap==0 ){
    పూర్ణాంకం pgsz = pTask->pSorter->pgsz;
    పూర్ణాంకం iBuf = pReadr->iReadOff % pgsz;
    ఒకవేళ( pReadr->a_బఫరు==0 ){
      pReadr->a_బఫరు = (u8*)sqlite3Malloc(pgsz);
      ఒకవేళ( pReadr->a_బఫరు==0 ) rc = SQLITE_NOMEM_BKPT;
      pReadr->n_బఫరు = pgsz;
    }
    ఒకవేళ( rc==SQLITE_OK && iBuf ){
      పూర్ణాంకం nRead = pgsz - iBuf;
      ఒకవేళ( (pReadr->iReadOff + nRead) > pReadr->iEof ){
        nRead = (పూర్ణాంకం)(pReadr->iEof - pReadr->iReadOff);
      }
      rc = sqlite3OsRead(
          pReadr->pFd, &pReadr->a_బఫరు[iBuf], nRead, pReadr->iReadOff
      );
      testcase( rc!=SQLITE_OK );
    }
  }

  తిరిగి rc;
}

/*
** Advance PmaReader pReadr to the next key in its PMA. Return SQLITE_OK if
** no error occurs, or an SQLite error code if one does.
*/
స్థిర పూర్ణాంకం vdbe_Pma_Reader_తదుపరి(PmaReader *pReadr){
  పూర్ణాంకం rc = SQLITE_OK;             /* Return Code */
  u64 nRec = 0;                   /* Size of record in bytes */


  ఒకవేళ( pReadr->iReadOff>=pReadr->iEof ){
    IncrMerger *pIncr = pReadr->pIncr;
    పూర్ణాంకం bEof = 1;
    ఒకవేళ( pIncr ){
      rc = vdbe_Incr_మార్పిడి(pIncr);
      ఒకవేళ( rc==SQLITE_OK && pIncr->bEof==0 ){
        rc = vdbePmaReaderSeek(
            pIncr->pTask, pReadr, &pIncr->aFile[0], pIncr->i_ప్రారంభం_Off
        );
        bEof = 0;
      }
    }

    ఒకవేళ( bEof ){
      /* This is an EOF condition */
      vdbePmaReaderClear(pReadr);
      testcase( rc!=SQLITE_OK );
      తిరిగి rc;
    }
  }

  ఒకవేళ( rc==SQLITE_OK ){
    rc = vdbePmaReadVarint(pReadr, &nRec);
  }
  ఒకవేళ( rc==SQLITE_OK ){
    pReadr->n_కీలకం = (పూర్ణాంకం)nRec;
    rc = vdbePmaReadBlob(pReadr, (పూర్ణాంకం)nRec, &pReadr->a_కీలకం);
    testcase( rc!=SQLITE_OK );
  }

  తిరిగి rc;
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
స్థిర పూర్ణాంకం vdbePmaReaderInit(
  క్రమీకరించు_Subtask *pTask,             /* Task context */
  SorterFile *pFile,              /* Sorter file to read from */
  i64 i_ప్రారంభం,                     /* Start offset in pFile */
  PmaReader *pReadr,              /* PmaReader to populate */
  i64 *pnByte                     /* IN/OUT: Increment this value by PMA size */
){
  పూర్ణాంకం rc;

  ప్రతిజ్ఞా( pFile->iEof>i_ప్రారంభం );
  ప్రతిజ్ఞా( pReadr->aAlloc==0 && pReadr->nAlloc==0 );
  ప్రతిజ్ఞా( pReadr->a_బఫరు==0 );
  ప్రతిజ్ఞా( pReadr->aMap==0 );

  rc = vdbePmaReaderSeek(pTask, pReadr, pFile, i_ప్రారంభం);
  ఒకవేళ( rc==SQLITE_OK ){
    u64 nByte = 0;                 /* Size of PMA in bytes */
    rc = vdbePmaReadVarint(pReadr, &nByte);
    pReadr->iEof = pReadr->iReadOff + nByte;
    *pnByte += nByte;
  }

  ఒకవేళ( rc==SQLITE_OK ){
    rc = vdbe_Pma_Reader_తదుపరి(pReadr);
  }
  తిరిగి rc;
}

/*
** A version of vdbeSorterCompare() that assumes that it has already been
** determined that the first field of key1 is equal to the first field of
** key2.
*/
స్థిర పూర్ణాంకం vdbe_Sorter_Compare_తోక(
  క్రమీకరించు_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  పూర్ణాంకం *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  స్థిరరాశి శూన్య *pKey1, పూర్ణాంకం nKey1,   /* Left side of comparison */
  స్థిరరాశి శూన్య *pKey2, పూర్ణాంకం nKey2    /* Right side of comparison */
){
  Unpacked_రికార్డు *r2 = pTask->pUnpacked;
  ఒకవేళ( *pbKey2Cached==0 ){
    sqlite3_Vdbe_రికార్డు_Unpack(nKey2, pKey2, r2);
    *pbKey2Cached = 1;
  }
  తిరిగి sqlite3_Vdbe_రికార్డు_Compare_With_Skip(nKey1, pKey1, r2, 1);
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
స్థిర పూర్ణాంకం vdbeSorterCompare(
  క్రమీకరించు_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  పూర్ణాంకం *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  స్థిరరాశి శూన్య *pKey1, పూర్ణాంకం nKey1,   /* Left side of comparison */
  స్థిరరాశి శూన్య *pKey2, పూర్ణాంకం nKey2    /* Right side of comparison */
){
  Unpacked_రికార్డు *r2 = pTask->pUnpacked;
  ఒకవేళ( !*pbKey2Cached ){
    sqlite3_Vdbe_రికార్డు_Unpack(nKey2, pKey2, r2);
    *pbKey2Cached = 1;
  }
  తిరిగి sqlite3_Vdbe_రికార్డు_Compare(nKey1, pKey1, r2);
}

/*
** A specially optimized version of vdbeSorterCompare() that assumes that
** the first field of each key is a TEXT value and that the collation
** sequence to compare them with is BINARY.
*/
స్థిర పూర్ణాంకం vdbe_Sorter_Compare_పాఠ్యం(
  క్రమీకరించు_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  పూర్ణాంకం *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  స్థిరరాశి శూన్య *pKey1, పూర్ణాంకం nKey1,   /* Left side of comparison */
  స్థిరరాశి శూన్య *pKey2, పూర్ణాంకం nKey2    /* Right side of comparison */
){
  స్థిరరాశి u8 * స్థిరరాశి p1 = (స్థిరరాశి u8 * స్థిరరాశి)pKey1;
  స్థిరరాశి u8 * స్థిరరాశి p2 = (స్థిరరాశి u8 * స్థిరరాశి)pKey2;
  స్థిరరాశి u8 * స్థిరరాశి v1 = &p1[ p1[0] ];   /* Pointer to value 1 */
  స్థిరరాశి u8 * స్థిరరాశి v2 = &p2[ p2[0] ];   /* Pointer to value 2 */

  పూర్ణాంకం n1;
  పూర్ణాంకం n2;
  పూర్ణాంకం res;

  getVarint32NR(&p1[1], n1);
  getVarint32NR(&p2[1], n2);
  res = memcmp(v1, v2, (MIN(n1, n2) - 13)/2);
  ఒకవేళ( res==0 ){
    res = n1 - n2;
  }

  ఒకవేళ( res==0 ){
    ఒకవేళ( pTask->pSorter->p_కీలకం_Info->n_కీలకం_క్షేత్రం>1 ){
      res = vdbe_Sorter_Compare_తోక(
          pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2
      );
    }
  }లేకపోతే{
    ప్రతిజ్ఞా( pTask->pSorter->p_కీలకం_Info->a_క్రమీకరించు_Flags!=0 );
    ప్రతిజ్ఞా( !(pTask->pSorter->p_కీలకం_Info->a_క్రమీకరించు_Flags[0]&KEYINFO_క్రమం_BIGNULL) );
    ఒకవేళ( pTask->pSorter->p_కీలకం_Info->a_క్రమీకరించు_Flags[0] ){
      res = res * -1;
    }
  }

  తిరిగి res;
}

/*
** A specially optimized version of vdbeSorterCompare() that assumes that
** the first field of each key is an INTEGER value.
*/
స్థిర పూర్ణాంకం vdbeSorterCompareInt(
  క్రమీకరించు_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  పూర్ణాంకం *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  స్థిరరాశి శూన్య *pKey1, పూర్ణాంకం nKey1,   /* Left side of comparison */
  స్థిరరాశి శూన్య *pKey2, పూర్ణాంకం nKey2    /* Right side of comparison */
){
  స్థిరరాశి u8 * స్థిరరాశి p1 = (స్థిరరాశి u8 * స్థిరరాశి)pKey1;
  స్థిరరాశి u8 * స్థిరరాశి p2 = (స్థిరరాశి u8 * స్థిరరాశి)pKey2;
  స్థిరరాశి పూర్ణాంకం s1 = p1[1];                 /* Left hand serial type */
  స్థిరరాశి పూర్ణాంకం s2 = p2[1];                 /* Right hand serial type */
  స్థిరరాశి u8 * స్థిరరాశి v1 = &p1[ p1[0] ];   /* Pointer to value 1 */
  స్థిరరాశి u8 * స్థిరరాశి v2 = &p2[ p2[0] ];   /* Pointer to value 2 */
  పూర్ణాంకం res;                              /* Return value */

  ప్రతిజ్ఞా( (s1>0 && s1<7) || s1==8 || s1==9 );
  ప్రతిజ్ఞా( (s2>0 && s2<7) || s2==8 || s2==9 );

  ఒకవేళ( s1==s2 ){
    /* The two values have the same sign. Compare using memcmp(). */
    స్థిర స్థిరరాశి u8 aLen[] = {0, 1, 2, 3, 4, 6, 8, 0, 0, 0 };
    స్థిరరాశి u8 n = aLen[s1];
    పూర్ణాంకం i;
    res = 0;
    కొరకు(i=0; i<n; i++){
      ఒకవేళ( (res = v1[i] - v2[i])!=0 ){
        ఒకవేళ( ((v1[0] ^ v2[0]) & 0x80)!=0 ){
          res = v1[0] & 0x80 ? -1 : +1;
        }
        ఆపు;
      }
    }
  }లేకపోతే ఒకవేళ( s1>7 && s2>7 ){
    res = s1 - s2;
  }లేకపోతే{
    ఒకవేళ( s2>7 ){
      res = +1;
    }లేకపోతే ఒకవేళ( s1>7 ){
      res = -1;
    }లేకపోతే{
      res = s1 - s2;
    }
    ప్రతిజ్ఞా( res!=0 );

    ఒకవేళ( res>0 ){
      ఒకవేళ( *v1 & 0x80 ) res = -1;
    }లేకపోతే{
      ఒకవేళ( *v2 & 0x80 ) res = +1;
    }
  }

  ప్రతిజ్ఞా( pTask->pSorter->p_కీలకం_Info->a_క్రమీకరించు_Flags!=0 );
  ఒకవేళ( res==0 ){
    ఒకవేళ( pTask->pSorter->p_కీలకం_Info->n_కీలకం_క్షేత్రం>1 ){
      res = vdbe_Sorter_Compare_తోక(
          pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2
      );
    }
  }లేకపోతే ఒకవేళ( pTask->pSorter->p_కీలకం_Info->a_క్రమీకరించు_Flags[0] ){
    ప్రతిజ్ఞా( !(pTask->pSorter->p_కీలకం_Info->a_క్రమీకరించు_Flags[0]&KEYINFO_క్రమం_BIGNULL) );
    res = res * -1;
  }

  తిరిగి res;
}

/* Helper function for vdbeSorterCompareReal().
**
** The first elements of both pKey1 and pKey2 have been decoded into double
** values r1 and r2.  Do the comparison between those keys and return the
** result.  If r1==r2, break the tie with a comparison of subsequent elements
** from each key.
*/
స్థిర పూర్ణాంకం vdbe_Sorter_ముగింపు_Real_Compare(
  క్రమీకరించు_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  పూర్ణాంకం *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  స్థిరరాశి శూన్య *pKey1, పూర్ణాంకం nKey1,   /* Left side of comparison */
  స్థిరరాశి శూన్య *pKey2, పూర్ణాంకం nKey2,   /* Right side of comparison */
  ద్విగుణ r1,                      /* REAL value of first element of pKey1 */
  ద్విగుణ r2                       /* REAL value of first element of pKey2 */
){
  పూర్ణాంకం res;
  ఒకవేళ( r1<r2 ){
    res = -1;
  }లేకపోతే ఒకవేళ( r1>r2 ){
    res = +1;
  }లేకపోతే{
    res = 0;
  }
  ప్రతిజ్ఞా( pTask->pSorter->p_కీలకం_Info->a_క్రమీకరించు_Flags!=0 );
  ఒకవేళ( res==0 ){
    ఒకవేళ( pTask->pSorter->p_కీలకం_Info->n_కీలకం_క్షేత్రం>1 ){
      res = vdbe_Sorter_Compare_తోక(
          pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2
      );
    }
  }లేకపోతే ఒకవేళ( pTask->pSorter->p_కీలకం_Info->a_క్రమీకరించు_Flags[0] ){
    ప్రతిజ్ఞా( !(pTask->pSorter->p_కీలకం_Info->a_క్రమీకరించు_Flags[0]&KEYINFO_క్రమం_BIGNULL) );
    res = res * -1;
  }
  తిరిగి res;
}

/* Helper function for vdbeSorterCompareReal().
**
** Buffer p[] is a record where the first term is guaranteed to be either
** a floating-point value, or an integer stand-in for a floating point
** value (a MEM_IntReal).  Whatever its format, extract the value and
** return it.
*/
స్థిర ద్విగుణ vdbeSorterGetReal(స్థిరరాశి u8 *p){
  ద్విగుణ r;                    /* the return value */

  ప్రతిజ్ఞా( p[0]<0x80 );         /* 1-byte headers: nAllField<13 */
  ప్రతిజ్ఞా( p[1]>0 && p[1]<10 ); /* first fields proven numeric */

  ఒకవేళ( p[1]==7 ){
    u64 x = sqlite3Get8byte(p + p[0]);
    మార్పిడి_Mixed_Endian_Float(x);
    ప్రతిజ్ఞా( !IsNaN(x) );
    జ్ఞాపకం_నకలు(&r, &x, పరిమాణం(r));
  }లేకపోతే{
    Mem m;
    m.u.i = 0;
    sqlite3VdbeSerialGet(p + p[0], p[1], &m);
    ప్రతిజ్ఞా( m.flags==MEM_Int );
    r = (ద్విగుణ)m.u.i;
  }
  తిరిగి r;
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
స్థిర SQLITE_NOINLINE పూర్ణాంకం vdbeSorterCompareRealInt(
  క్రమీకరించు_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  పూర్ణాంకం *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  స్థిరరాశి శూన్య *pKey1, పూర్ణాంకం nKey1,   /* Left side of comparison */
  స్థిరరాశి శూన్య *pKey2, పూర్ణాంకం nKey2    /* Right side of comparison */
){
  స్థిరరాశి u8 * స్థిరరాశి p1 = (స్థిరరాశి u8 * స్థిరరాశి)pKey1;
  స్థిరరాశి u8 * స్థిరరాశి p2 = (స్థిరరాశి u8 * స్థిరరాశి)pKey2;
  ఒకవేళ( p1[1]==6 || p2[1]==6 ){
    /* 64-bit integer values cannot be represented exactly by a double so
    ** must be handled by the generalized comparison function. */
    తిరిగి vdbeSorterCompare(pTask,
       pbKey2Cached, pKey1,nKey1, pKey2,nKey2);
  }లేకపోతే{
    ద్విగుణ r1 = vdbeSorterGetReal(p1);
    ద్విగుణ r2 = vdbeSorterGetReal(p2);
    తిరిగి vdbe_Sorter_ముగింపు_Real_Compare(pTask,pbKey2Cached,
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
స్థిర పూర్ణాంకం vdbeSorterCompareReal(
  క్రమీకరించు_Subtask *pTask,             /* Subtask context (for pKeyInfo) */
  పూర్ణాంకం *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  స్థిరరాశి శూన్య *pKey1, పూర్ణాంకం nKey1,   /* Left side of comparison */
  స్థిరరాశి శూన్య *pKey2, పూర్ణాంకం nKey2    /* Right side of comparison */
){
  స్థిరరాశి u8*స్థిరరాశి p1 = (స్థిరరాశి u8*స్థిరరాశి)pKey1;  /* Left key record */
  స్థిరరాశి u8*స్థిరరాశి p2 = (స్థిరరాశి u8*స్థిరరాశి)pKey2;  /* Right key record */
  u64 x;                   /* A real value stored as an integer */
  ద్విగుణ r1;               /* First element of pKey1 */
  ద్విగుణ r2;               /* First element of pKey2 */

  ప్రతిజ్ఞా( p1[0]<0x80 && p2[0]<0x80 );  /* 1-byte headers: nAllField<13 */
  ప్రతిజ్ఞా( p1[1]>0 && p1[1]<10 );       /* first field guaranteed numeric */
  ప్రతిజ్ఞా( p2[1]>0 && p2[1]<10 );       /* first field guaranteed numeric */

  ఒకవేళ( p1[1]!=7 || p2[1]!=7 ){
    /* One or both floating point values are stored as INTEGER.  This might
    ** be because of the MEM_RealInt encoding.  Try to optimize that case. */
    తిరిగి vdbeSorterCompareRealInt(pTask,
       pbKey2Cached, pKey1,nKey1, pKey2,nKey2
    );
  }
  ప్రతిజ్ఞా( p1[0]<=nKey1-8 && p2[0]<=nKey2-8 );

  x = sqlite3Get8byte(p1 + *p1);
  మార్పిడి_Mixed_Endian_Float(x);
  ప్రతిజ్ఞా( !IsNaN(x) );
  జ్ఞాపకం_నకలు(&r1, &x, పరిమాణం(r1));
  x = sqlite3Get8byte(p2 + *p2);
  మార్పిడి_Mixed_Endian_Float(x);
  ప్రతిజ్ఞా( !IsNaN(x) );
  జ్ఞాపకం_నకలు(&r2, &x, పరిమాణం(r2));
  తిరిగి vdbe_Sorter_ముగింపు_Real_Compare(pTask,
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
పూర్ణాంకం sqlite3VdbeSorterInit(
  sqlite3 *db,                    /* Database connection (for malloc()) */
  పూర్ణాంకం n_క్షేత్రం,                     /* Number of key fields in each record */
  VdbeCursor *pCsr                /* Cursor that holds the new sorter */
){
  పూర్ణాంకం pgsz;                       /* Page size of main database */
  పూర్ణాంకం i;                          /* Used to iterate through aTask[] */
  VdbeSorter *pSorter;            /* The new sorter */
  కీలకం_Info *p_కీలకం_Info;              /* Copy of pCsr->pKeyInfo with db==0 */
  పూర్ణాంకం sz_కీలకం_Info;                  /* Size of pCsr->pKeyInfo in bytes */
  i64 sz;                         /* Size of pSorter in bytes */
  పూర్ణాంకం rc = SQLITE_OK;
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు==0
# define n_కార్మికుడు 0
#లేకపోతే
  పూర్ణాంకం n_కార్మికుడు;
#అయితే_ముగింపు

  /* Initialize the upper limit on the number of worker threads */
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
  ఒకవేళ( sqlite3_తాత్కాలిక_లోపల_జ్ఞాపకం(db) || sqlite3GlobalConfig.bCoreMutex==0 ){
    n_కార్మికుడు = 0;
  }లేకపోతే{
    n_కార్మికుడు = db->a_హద్దు[SQLITE_హద్దు_కార్మికుడు_దారాలు];
  }
#అయితే_ముగింపు

  /* Do not allow the total number of threads (main thread + all workers)
  ** to exceed the maximum merge count */
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>=SORTER_MAX_విలీనం_లెక్క
  ఒకవేళ( n_కార్మికుడు>=SORTER_MAX_విలీనం_లెక్క ){
    n_కార్మికుడు = SORTER_MAX_విలీనం_లెక్క-1;
  }
#అయితే_ముగింపు

  ప్రతిజ్ఞా( pCsr->p_కీలకం_Info );
  ప్రతిజ్ఞా( !pCsr->isEphemeral );
  ప్రతిజ్ఞా( pCsr->eCurType==CURTYPE_SORTER );
  ప్రతిజ్ఞా( పరిమాణం(కీలకం_Info) + UMXV(pCsr->p_కీలకం_Info->n_కీలకం_క్షేత్రం)*పరిమాణం(CollSeq*)
               < 0x7fffffff );
  ప్రతిజ్ఞా( pCsr->p_కీలకం_Info->n_కీలకం_క్షేత్రం<=pCsr->p_కీలకం_Info->n_All_క్షేత్రం );
  sz_కీలకం_Info = SZ_KEYINFO(pCsr->p_కీలకం_Info->n_All_క్షేత్రం);
  sz = SZ_VDBESORTER(n_కార్మికుడు+1);

  pSorter = (VdbeSorter*)sqlite3DbMallocZero(db, sz + sz_కీలకం_Info);
  pCsr->uc.pSorter = pSorter;
  ఒకవేళ( pSorter==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }లేకపోతే{
    Btree *pBt = db->aDb[0].pBt;
    pSorter->p_కీలకం_Info = p_కీలకం_Info = (కీలకం_Info*)((u8*)pSorter + sz);
    జ్ఞాపకం_నకలు(p_కీలకం_Info, pCsr->p_కీలకం_Info, sz_కీలకం_Info);
    p_కీలకం_Info->db = 0;
    ఒకవేళ( n_క్షేత్రం && n_కార్మికుడు==0 ){
      p_కీలకం_Info->n_కీలకం_క్షేత్రం = n_క్షేత్రం;
      ప్రతిజ్ఞా( n_క్షేత్రం<=pCsr->p_కీలకం_Info->n_All_క్షేత్రం );
    }
    /* It is OK that pKeyInfo reuses the aSortFlags field from pCsr->pKeyInfo,
    ** since the pCsr->pKeyInfo->aSortFlags[] array is invariant and lives
    ** longer that pSorter. */
    ప్రతిజ్ఞా( p_కీలకం_Info->a_క్రమీకరించు_Flags==pCsr->p_కీలకం_Info->a_క్రమీకరించు_Flags );
    sqlite3BtreeEnter(pBt);
    pSorter->pgsz = pgsz = sqlite3_Btree_Get_పుట_కొలత(pBt);
    sqlite3BtreeLeave(pBt);
    pSorter->nTask = n_కార్మికుడు + 1;
    pSorter->iPrev = (u8)(n_కార్మికుడు - 1);
    pSorter->b_Use_దారాలు = (pSorter->nTask>1);
    pSorter->db = db;
    కొరకు(i=0; i<pSorter->nTask; i++){
      క్రమీకరించు_Subtask *pTask = &pSorter->aTask[i];
      pTask->pSorter = pSorter;
    }

    ఒకవేళ( !sqlite3_తాత్కాలిక_లోపల_జ్ఞాపకం(db) ){
      i64 mxCache;                /* Cache size in bytes*/
      u32 szPma = sqlite3GlobalConfig.szPma;
      pSorter->mn_Pma_కొలత = szPma * pgsz;

      mxCache = db->aDb[0].pSchema->cache_కొలత;
      ఒకవేళ( mxCache<0 ){
        /* A negative cache-size value C indicates that the cache is abs(C)
        ** KiB in size.  */
        mxCache = mxCache * -1024;
      }లేకపోతే{
        mxCache = mxCache * pgsz;
      }
      mxCache = MIN(mxCache, SQLITE_MAX_PMASZ);
      pSorter->mx_Pma_కొలత = MAX(pSorter->mn_Pma_కొలత, (పూర్ణాంకం)mxCache);

      /* Avoid large memory allocations if the application has requested
      ** SQLITE_CONFIG_SMALL_MALLOC. */
      ఒకవేళ( sqlite3GlobalConfig.bSmallMalloc==0 ){
        ప్రతిజ్ఞా( pSorter->i_జ్ఞాపకం==0 );
        pSorter->n_జ్ఞాపకం = pgsz;
        pSorter->list.a_జ్ఞాపకం = (u8*)sqlite3Malloc(pgsz);
        ఒకవేళ( !pSorter->list.a_జ్ఞాపకం ) rc = SQLITE_NOMEM_BKPT;
      }
    }

    ఒకవేళ( p_కీలకం_Info->n_All_క్షేత్రం<13
     && (p_కీలకం_Info->aColl[0]==0 || p_కీలకం_Info->aColl[0]==db->pDfltColl)
     && (p_కీలకం_Info->a_క్రమీకరించు_Flags[0] & KEYINFO_క్రమం_BIGNULL)==0
    ){
      pSorter->type_ముసుగు = SORTER_TYPE_INTEGER|SORTER_TYPE_పాఠ్యం|SORTER_TYPE_REAL;
    }
  }

  తిరిగి rc;
}
#నిర్వచనం_రద్దు n_కార్మికుడు   /* Defined at the top of this function */

/*
** Free the list of sorted records starting at pRecord.
*/
స్థిర శూన్య vdbe_Sorter_రికార్డు_Free(sqlite3 *db, Sorter_రికార్డు *p_రికార్డు){
  Sorter_రికార్డు *p;
  Sorter_రికార్డు *p_తదుపరి;
  కొరకు(p=p_రికార్డు; p; p=p_తదుపరి){
    p_తదుపరి = p->u.p_తదుపరి;
    sqlite3DbFree(db, p);
  }
}

/*
** Free all resources owned by the object indicated by argument pTask. All
** fields of *pTask are zeroed before returning.
*/
స్థిర శూన్య vdbe_క్రమీకరించు_Subtask_Cleanup(sqlite3 *db, క్రమీకరించు_Subtask *pTask){
  sqlite3DbFree(db, pTask->pUnpacked);
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
  /* pTask->list.aMemory can only be non-zero if it was handed memory
  ** from the main thread.  That only occurs SQLITE_MAX_WORKER_THREADS>0 */
  ఒకవేళ( pTask->list.a_జ్ఞాపకం ){
    sqlite3_free(pTask->list.a_జ్ఞాపకం);
  }లేకపోతే
#అయితే_ముగింపు
  {
    ప్రతిజ్ఞా( pTask->list.a_జ్ఞాపకం==0 );
    vdbe_Sorter_రికార్డు_Free(0, pTask->list.pList);
  }
  ఒకవేళ( pTask->file.pFd ){
    sqlite3OsCloseFree(pTask->file.pFd);
  }
  ఒకవేళ( pTask->file2.pFd ){
    sqlite3OsCloseFree(pTask->file2.pFd);
  }
  memset(pTask, 0, పరిమాణం(క్రమీకరించు_Subtask));
}

#నిర్వచితమైతే SQLITE_DEBUG_SORTER_దారాలు
స్థిర శూన్య vdbeSorterWorkDebug(క్రమీకరించు_Subtask *pTask, స్థిరరాశి అక్షరం *zEvent){
  i64 t;
  పూర్ణాంకం iTask = (pTask - pTask->pSorter->aTask);
  sqlite3_Os_Current_సమయం_Int64(pTask->pSorter->db->pVfs, &t);
  fprintf(stderr, "%lld:%d %s\n", t, iTask, zEvent);
}
స్థిర శూన్య vdbeSorterRewindDebug(స్థిరరాశి అక్షరం *zEvent){
  i64 t = 0;
  sqlite3_vfs *pVfs = sqlite3_vfs_కనుగొను(0);
  ఒకవేళ( ALWAYS(pVfs) ) sqlite3_Os_Current_సమయం_Int64(pVfs, &t);
  fprintf(stderr, "%lld:X %s\n", t, zEvent);
}
స్థిర శూన్య vdbeSorterPopulateDebug(
  క్రమీకరించు_Subtask *pTask,
  స్థిరరాశి అక్షరం *zEvent
){
  i64 t;
  పూర్ణాంకం iTask = (pTask - pTask->pSorter->aTask);
  sqlite3_Os_Current_సమయం_Int64(pTask->pSorter->db->pVfs, &t);
  fprintf(stderr, "%lld:bg%d %s\n", t, iTask, zEvent);
}
స్థిర శూన్య vdbe_Sorter_ఖండం_Debug(
  క్రమీకరించు_Subtask *pTask,
  పూర్ణాంకం bBlocked,
  స్థిరరాశి అక్షరం *zEvent
){
  ఒకవేళ( bBlocked ){
    i64 t;
    sqlite3_Os_Current_సమయం_Int64(pTask->pSorter->db->pVfs, &t);
    fprintf(stderr, "%lld:main %s\n", t, zEvent);
  }
}
#లేకపోతే
# define vdbeSorterWorkDebug(x,y)
# define vdbeSorterRewindDebug(y)
# define vdbeSorterPopulateDebug(x,y)
# define vdbe_Sorter_ఖండం_Debug(x,y,z)
#అయితే_ముగింపు

#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
/*
** Join thread pTask->thread.
*/
స్థిర పూర్ణాంకం vdbeSorterJoinThread(క్రమీకరించు_Subtask *pTask){
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( pTask->pThread ){
#నిర్వచితమైతే SQLITE_DEBUG_SORTER_దారాలు
    పూర్ణాంకం b_పూర్తి = pTask->b_పూర్తి;
#అయితే_ముగింపు
    శూన్య *pRet = SQLITE_INT_గమ్యం_PTR(SQLITE_ERROR);
    vdbe_Sorter_ఖండం_Debug(pTask, !b_పూర్తి, "enter");
    (శూన్య)sqlite3ThreadJoin(pTask->pThread, &pRet);
    vdbe_Sorter_ఖండం_Debug(pTask, !b_పూర్తి, "exit");
    rc = SQLITE_PTR_గమ్యం_INT(pRet);
    ప్రతిజ్ఞా( pTask->b_పూర్తి==1 );
    pTask->b_పూర్తి = 0;
    pTask->pThread = 0;
  }
  తిరిగి rc;
}

/*
** Launch a background thread to run xTask(pIn).
*/
స్థిర పూర్ణాంకం vdbeSorterCreateThread(
  క్రమీకరించు_Subtask *pTask,             /* Thread will use this task object */
  శూన్య *(*xTask)(శూన్య*),          /* Routine to run in a separate thread */
  శూన్య *p_లోపల                       /* Argument passed into xTask() */
){
  ప్రతిజ్ఞా( pTask->pThread==0 && pTask->b_పూర్తి==0 );
  తిరిగి sqlite3ThreadCreate(&pTask->pThread, xTask, p_లోపల);
}

/*
** Join all outstanding threads launched by SorterWrite() to create
** level-0 PMAs.
*/
స్థిర పూర్ణాంకం vdbeSorterJoinAll(VdbeSorter *pSorter, పూర్ణాంకం rcin){
  పూర్ణాంకం rc = rcin;
  పూర్ణాంకం i;

  /* This function is always called by the main user thread.
  **
  ** If this function is being called after SorterRewind() has been called,
  ** it is possible that thread pSorter->aTask[pSorter->nTask-1].pThread
  ** is currently attempt to join one of the other threads. To avoid a race
  ** condition where this thread also attempts to join the same object, join
  ** thread pSorter->aTask[pSorter->nTask-1].pThread first. */
  కొరకు(i=pSorter->nTask-1; i>=0; i--){
    క్రమీకరించు_Subtask *pTask = &pSorter->aTask[i];
    పూర్ణాంకం rc2 = vdbeSorterJoinThread(pTask);
    ఒకవేళ( rc==SQLITE_OK ) rc = rc2;
  }
  తిరిగి rc;
}
#లేకపోతే
# define vdbeSorterJoinAll(x,rcin) (rcin)
# define vdbeSorterJoinThread(pTask) SQLITE_OK
#అయితే_ముగింపు

/*
** Allocate a new MergeEngine object capable of handling up to
** nReader PmaReader inputs.
**
** nReader is automatically rounded up to the next power of two.
** nReader may not exceed SORTER_MAX_MERGE_COUNT even after rounding up.
*/
స్థిర విలీనం_Engine *vdbe_విలీనం_Engine_New(పూర్ణాంకం nReader){
  పూర్ణాంకం N = 2;                      /* Smallest power of two >= nReader */
  i64 nByte;                      /* Total bytes of space to allocate */
  విలీనం_Engine *pNew;              /* Pointer to allocated object to return */

  ప్రతిజ్ఞా( nReader<=SORTER_MAX_విలీనం_లెక్క );

  అయితే( N<nReader ) N += N;
  nByte = పరిమాణం(విలీనం_Engine) + N * (పరిమాణం(పూర్ణాంకం) + పరిమాణం(PmaReader));

  pNew = sqlite3FaultSim(100) ? 0 : (విలీనం_Engine*)sqlite3MallocZero(nByte);
  ఒకవేళ( pNew ){
    pNew->n_వృక్షం = N;
    pNew->pTask = 0;
    pNew->aReadr = (PmaReader*)&pNew[1];
    pNew->a_వృక్షం = (పూర్ణాంకం*)&pNew->aReadr[N];
  }
  తిరిగి pNew;
}

/*
** Free the MergeEngine object passed as the only argument.
*/
స్థిర శూన్య vdbe_విలీనం_Engine_Free(విలీనం_Engine *pMerger){
  పూర్ణాంకం i;
  ఒకవేళ( pMerger ){
    కొరకు(i=0; i<pMerger->n_వృక్షం; i++){
      vdbePmaReaderClear(&pMerger->aReadr[i]);
    }
  }
  sqlite3_free(pMerger);
}

/*
** Free all resources associated with the IncrMerger object indicated by
** the first argument.
*/
స్థిర శూన్య vdbeIncrFree(IncrMerger *pIncr){
  ఒకవేళ( pIncr ){
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
    ఒకవేళ( pIncr->bUseThread ){
      vdbeSorterJoinThread(pIncr->pTask);
      ఒకవేళ( pIncr->aFile[0].pFd ) sqlite3OsCloseFree(pIncr->aFile[0].pFd);
      ఒకవేళ( pIncr->aFile[1].pFd ) sqlite3OsCloseFree(pIncr->aFile[1].pFd);
    }
#అయితే_ముగింపు
    vdbe_విలీనం_Engine_Free(pIncr->pMerger);
    sqlite3_free(pIncr);
  }
}

/*
** Reset a sorting cursor back to its original empty state.
*/
శూన్య sqlite3VdbeSorterReset(sqlite3 *db, VdbeSorter *pSorter){
  పూర్ణాంకం i;
  (శూన్య)vdbeSorterJoinAll(pSorter, SQLITE_OK);
  ప్రతిజ్ఞా( pSorter->b_Use_దారాలు || pSorter->pReader==0 );
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
  ఒకవేళ( pSorter->pReader ){
    vdbePmaReaderClear(pSorter->pReader);
    sqlite3DbFree(db, pSorter->pReader);
    pSorter->pReader = 0;
  }
#అయితే_ముగింపు
  vdbe_విలీనం_Engine_Free(pSorter->pMerger);
  pSorter->pMerger = 0;
  కొరకు(i=0; i<pSorter->nTask; i++){
    క్రమీకరించు_Subtask *pTask = &pSorter->aTask[i];
    vdbe_క్రమీకరించు_Subtask_Cleanup(db, pTask);
    pTask->pSorter = pSorter;
  }
  ఒకవేళ( pSorter->list.a_జ్ఞాపకం==0 ){
    vdbe_Sorter_రికార్డు_Free(0, pSorter->list.pList);
  }
  pSorter->list.pList = 0;
  pSorter->list.szPMA = 0;
  pSorter->bUsePMA = 0;
  pSorter->i_జ్ఞాపకం = 0;
  pSorter->mxKeysize = 0;
  sqlite3DbFree(db, pSorter->pUnpacked);
  pSorter->pUnpacked = 0;
}

/*
** Free any cursor components allocated by sqlite3VdbeSorterXXX routines.
*/
శూన్య sqlite3VdbeSorterClose(sqlite3 *db, VdbeCursor *pCsr){
  VdbeSorter *pSorter;
  ప్రతిజ్ఞా( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  ఒకవేళ( pSorter ){
    /* Increment db->nSpill by the total number of bytes of data written
    ** to temp files by this sort operation.  */
    పూర్ణాంకం ii;
    కొరకు(ii=0; ii<pSorter->nTask; ii++){
      db->nSpill += pSorter->aTask[ii].nSpill;
    }
    sqlite3VdbeSorterReset(db, pSorter);
    sqlite3_free(pSorter->list.a_జ్ఞాపకం);
    sqlite3DbFree(db, pSorter);
    pCsr->uc.pSorter = 0;
  }
}

#ఒకవేళ SQLITE_MAX_MMAP_కొలత>0
/*
** The first argument is a file-handle open on a temporary file. The file
** is guaranteed to be nByte bytes or smaller in size. This function
** attempts to extend the file to nByte bytes in size and to ensure that
** the VFS has memory mapped it.
**
** Whether or not the file does end up memory mapped of course depends on
** the specific VFS implementation.
*/
స్థిర శూన్య vdbeSorterExtendFile(sqlite3 *db, sqlite3_file *pFd, i64 nByte){
  ఒకవేళ( nByte<=(i64)(db->nMaxSorterMmap) && pFd->pMethods->i_సంస్కరణ>=3 ){
    శూన్య *p = 0;
    పూర్ణాంకం chunksize = 4*1024;
    sqlite3OsFileControlHint(pFd, SQLITE_FCNTL_CHUNK_కొలత, &chunksize);
    sqlite3OsFileControlHint(pFd, SQLITE_FCNTL_కొలత_HINT, &nByte);
    sqlite3OsFetch(pFd, 0, (పూర్ణాంకం)nByte, &p);
    ఒకవేళ( p ) sqlite3OsUnfetch(pFd, 0, p);
  }
}
#లేకపోతే
# define vdbeSorterExtendFile(x,y,z)
#అయితే_ముగింపు

/*
** Allocate space for a file-handle and open a temporary file. If successful,
** set *ppFd to point to the malloc'd file-handle and return SQLITE_OK.
** Otherwise, set *ppFd to 0 and return an SQLite error code.
*/
స్థిర పూర్ణాంకం vdbe_Sorter_Open_తాత్కాలిక_File(
  sqlite3 *db,                    /* Database handle doing sort */
  i64 nExtend,                    /* Attempt to extend file to this size */
  sqlite3_file **ppFd
){
  పూర్ణాంకం rc;
  ఒకవేళ( sqlite3FaultSim(202) ) తిరిగి SQLITE_IOERR_ACCESS;
  rc = sqlite3OsOpenMalloc(db->pVfs, 0, ppFd,
      SQLITE_OPEN_తాత్కాలిక_JOURNAL |
      SQLITE_OPEN_READWRITE    | SQLITE_OPEN_CREATE |
      SQLITE_OPEN_EXCLUSIVE    | SQLITE_OPEN_DELETEONCLOSE, &rc
  );
  ఒకవేళ( rc==SQLITE_OK ){
    i64 max = SQLITE_MAX_MMAP_కొలత;
    sqlite3OsFileControlHint(*ppFd, SQLITE_FCNTL_MMAP_కొలత, (శూన్య*)&max);
    ఒకవేళ( nExtend>0 ){
      vdbeSorterExtendFile(db, *ppFd, nExtend);
    }
  }
  తిరిగి rc;
}

/*
** If it has not already been allocated, allocate the UnpackedRecord
** structure at pTask->pUnpacked. Return SQLITE_OK if successful (or
** if no allocation was required), or SQLITE_NOMEM otherwise.
*/
స్థిర పూర్ణాంకం vdbe_క్రమీకరించు_Alloc_Unpacked(క్రమీకరించు_Subtask *pTask){
  ఒకవేళ( pTask->pUnpacked==0 ){
    pTask->pUnpacked = sqlite3_Vdbe_Alloc_Unpacked_రికార్డు(pTask->pSorter->p_కీలకం_Info);
    ఒకవేళ( pTask->pUnpacked==0 ) తిరిగి SQLITE_NOMEM_BKPT;
    pTask->pUnpacked->n_క్షేత్రం = pTask->pSorter->p_కీలకం_Info->n_కీలకం_క్షేత్రం;
    pTask->pUnpacked->err_సంకేతం = 0;
  }
  తిరిగి SQLITE_OK;
}


/*
** Merge the two sorted lists p1 and p2 into a single list.
*/
స్థిర Sorter_రికార్డు *vdbe_Sorter_విలీనం(
  క్రమీకరించు_Subtask *pTask,             /* Calling thread context */
  Sorter_రికార్డు *p1,               /* First list to merge */
  Sorter_రికార్డు *p2                /* Second list to merge */
){
  Sorter_రికార్డు *pFinal = 0;
  Sorter_రికార్డు **pp = &pFinal;
  పూర్ణాంకం bCached = 0;

  ప్రతిజ్ఞా( p1!=0 && p2!=0 );
  కొరకు(;;){
    పూర్ణాంకం res;
    res = pTask->xCompare(
        pTask, &bCached, SRVAL(p1), p1->nVal, SRVAL(p2), p2->nVal
    );

    ఒకవేళ( res<=0 ){
      *pp = p1;
      pp = &p1->u.p_తదుపరి;
      p1 = p1->u.p_తదుపరి;
      ఒకవేళ( p1==0 ){
        *pp = p2;
        ఆపు;
      }
    }లేకపోతే{
      *pp = p2;
      pp = &p2->u.p_తదుపరి;
      p2 = p2->u.p_తదుపరి;
      bCached = 0;
      ఒకవేళ( p2==0 ){
        *pp = p1;
        ఆపు;
      }
    }
  }
  తిరిగి pFinal;
}

/*
** Return the SorterCompare function to compare values collected by the
** sorter object passed as the only argument.
*/
స్థిర SorterCompare vdbeSorterGetCompare(VdbeSorter *p){
  ఒకవేళ( p->type_ముసుగు & SORTER_TYPE_INTEGER ){
    తిరిగి vdbeSorterCompareInt;
  }లేకపోతే ఒకవేళ( p->type_ముసుగు & SORTER_TYPE_పాఠ్యం ){
    తిరిగి vdbe_Sorter_Compare_పాఠ్యం;
  }లేకపోతే ఒకవేళ( p->type_ముసుగు & SORTER_TYPE_REAL ){
    తిరిగి vdbeSorterCompareReal;
  }
  తిరిగి vdbeSorterCompare;
}

/*
** Sort the linked list of records headed at pTask->pList. Return
** SQLITE_OK if successful, or an SQLite error code (i.e. SQLITE_NOMEM) if
** an error occurs.
*/
స్థిర పూర్ణాంకం vdbe_Sorter_క్రమీకరించు(క్రమీకరించు_Subtask *pTask, SorterList *pList){
  పూర్ణాంకం i;
  Sorter_రికార్డు *p;
  పూర్ణాంకం rc;
  Sorter_రికార్డు *a_స్థానం[64];

  rc = vdbe_క్రమీకరించు_Alloc_Unpacked(pTask);
  ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;

  p = pList->pList;
  pTask->xCompare = vdbeSorterGetCompare(pTask->pSorter);
  memset(a_స్థానం, 0, పరిమాణం(a_స్థానం));

  అయితే( p ){
    Sorter_రికార్డు *p_తదుపరి;
    ఒకవేళ( pList->a_జ్ఞాపకం ){
      ఒకవేళ( (u8*)p==pList->a_జ్ఞాపకం ){
        p_తదుపరి = 0;
      }లేకపోతే{
        ప్రతిజ్ఞా( p->u.i_తదుపరి<sqlite3_Malloc_కొలత(pList->a_జ్ఞాపకం) );
        p_తదుపరి = (Sorter_రికార్డు*)&pList->a_జ్ఞాపకం[p->u.i_తదుపరి];
      }
    }లేకపోతే{
      p_తదుపరి = p->u.p_తదుపరి;
    }

    p->u.p_తదుపరి = 0;
    కొరకు(i=0; a_స్థానం[i]; i++){
      p = vdbe_Sorter_విలీనం(pTask, p, a_స్థానం[i]);
      /* ,--Each aSlot[] holds twice as much as the previous. So we cannot use
      ** |  up all 64 aSlots[] with only a 64-bit address space.
      ** v                                                                */
      ప్రతిజ్ఞా( i<Array_కొలత(a_స్థానం) );
      a_స్థానం[i] = 0;
    }
    a_స్థానం[i] = p;
    p = p_తదుపరి;
  }

  p = 0;
  కొరకు(i=0; i<Array_కొలత(a_స్థానం); i++){
    ఒకవేళ( a_స్థానం[i]==0 ) కొనసాగు;
    p = p ? vdbe_Sorter_విలీనం(pTask, p, a_స్థానం[i]) : a_స్థానం[i];
  }
  pList->pList = p;

  ప్రతిజ్ఞా( pTask->pUnpacked->err_సంకేతం==SQLITE_OK
       || pTask->pUnpacked->err_సంకేతం==SQLITE_NOMEM
  );
  తిరిగి pTask->pUnpacked->err_సంకేతం;
}

/*
** Initialize a PMA-writer object.
*/
స్థిర శూన్య vdbePmaWriterInit(
  sqlite3_file *pFd,              /* File handle to write to */
  PmaWriter *p,                   /* Object to populate */
  పూర్ణాంకం nBuf,                       /* Buffer size */
  i64 i_ప్రారంభం                      /* Offset of pFd to begin writing at */
){
  memset(p, 0, పరిమాణం(PmaWriter));
  p->a_బఫరు = (u8*)sqlite3Malloc(nBuf);
  ఒకవేళ( !p->a_బఫరు ){
    p->eFWErr = SQLITE_NOMEM_BKPT;
  }లేకపోతే{
    p->i_Buf_చివర = p->i_Buf_ప్రారంభం = (i_ప్రారంభం % nBuf);
    p->iWriteOff = i_ప్రారంభం - p->i_Buf_ప్రారంభం;
    p->n_బఫరు = nBuf;
    p->pFd = pFd;
  }
}

/*
** Write nData bytes of data to the PMA. Return SQLITE_OK
** if successful, or an SQLite error code if an error occurs.
*/
స్థిర శూన్య vdbePmaWriteBlob(PmaWriter *p, u8 *p_దత్తాంశం, పూర్ణాంకం n_దత్తాంశం){
  పూర్ణాంకం nRem = n_దత్తాంశం;
  అయితే( nRem>0 && p->eFWErr==0 ){
    పూర్ణాంకం nCopy = nRem;
    ఒకవేళ( nCopy>(p->n_బఫరు - p->i_Buf_చివర) ){
      nCopy = p->n_బఫరు - p->i_Buf_చివర;
    }

    జ్ఞాపకం_నకలు(&p->a_బఫరు[p->i_Buf_చివర], &p_దత్తాంశం[n_దత్తాంశం-nRem], nCopy);
    p->i_Buf_చివర += nCopy;
    ఒకవేళ( p->i_Buf_చివర==p->n_బఫరు ){
      p->eFWErr = sqlite3OsWrite(p->pFd,
          &p->a_బఫరు[p->i_Buf_ప్రారంభం], p->i_Buf_చివర - p->i_Buf_ప్రారంభం,
          p->iWriteOff + p->i_Buf_ప్రారంభం
      );
      p->nPmaSpill += (p->i_Buf_చివర - p->i_Buf_ప్రారంభం);
      p->i_Buf_ప్రారంభం = p->i_Buf_చివర = 0;
      p->iWriteOff += p->n_బఫరు;
    }
    ప్రతిజ్ఞా( p->i_Buf_చివర<p->n_బఫరు );

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
స్థిర పూర్ణాంకం vdbe_Pma_Writer_ముగింపు(PmaWriter *p, i64 *piEof, u64 *pnSpill){
  పూర్ణాంకం rc;
  ఒకవేళ( p->eFWErr==0 && ALWAYS(p->a_బఫరు) && p->i_Buf_చివర>p->i_Buf_ప్రారంభం ){
    p->eFWErr = sqlite3OsWrite(p->pFd,
        &p->a_బఫరు[p->i_Buf_ప్రారంభం], p->i_Buf_చివర - p->i_Buf_ప్రారంభం,
        p->iWriteOff + p->i_Buf_ప్రారంభం
    );
    p->nPmaSpill += (p->i_Buf_చివర - p->i_Buf_ప్రారంభం);
  }
  *piEof = (p->iWriteOff + p->i_Buf_చివర);
  *pnSpill += p->nPmaSpill;
  sqlite3_free(p->a_బఫరు);
  rc = p->eFWErr;
  memset(p, 0, పరిమాణం(PmaWriter));
  తిరిగి rc;
}

/*
** Write value iVal encoded as a varint to the PMA. Return
** SQLITE_OK if successful, or an SQLite error code if an error occurs.
*/
స్థిర శూన్య vdbePmaWriteVarint(PmaWriter *p, u64 iVal){
  పూర్ణాంకం nByte;
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
స్థిర పూర్ణాంకం vdbe_Sorter_List_గమ్యం_PMA(క్రమీకరించు_Subtask *pTask, SorterList *pList){
  sqlite3 *db = pTask->pSorter->db;
  పూర్ణాంకం rc = SQLITE_OK;             /* Return code */
  PmaWriter writer;               /* Object used to write to the file */

#నిర్వచితమైతే SQLITE_DEBUG
  /* Set iSz to the expected size of file pTask->file after writing the PMA.
  ** This is used by an assert() statement at the end of this function.  */
  i64 iSz = pList->szPMA + sqlite3VarintLen(pList->szPMA) + pTask->file.iEof;
#అయితే_ముగింపు

  vdbeSorterWorkDebug(pTask, "enter");
  memset(&writer, 0, పరిమాణం(PmaWriter));
  ప్రతిజ్ఞా( pList->szPMA>0 );

  /* If the first temporary PMA file has not been opened, open it now. */
  ఒకవేళ( pTask->file.pFd==0 ){
    rc = vdbe_Sorter_Open_తాత్కాలిక_File(db, 0, &pTask->file.pFd);
    ప్రతిజ్ఞా( rc!=SQLITE_OK || pTask->file.pFd );
    ప్రతిజ్ఞా( pTask->file.iEof==0 );
    ప్రతిజ్ఞా( pTask->nPMA==0 );
  }

  /* Try to get the file to memory map */
  ఒకవేళ( rc==SQLITE_OK ){
    vdbeSorterExtendFile(db, pTask->file.pFd, pTask->file.iEof+pList->szPMA+9);
  }

  /* Sort the list */
  ఒకవేళ( rc==SQLITE_OK ){
    rc = vdbe_Sorter_క్రమీకరించు(pTask, pList);
  }

  ఒకవేళ( rc==SQLITE_OK ){
    Sorter_రికార్డు *p;
    Sorter_రికార్డు *p_తదుపరి = 0;

    vdbePmaWriterInit(pTask->file.pFd, &writer, pTask->pSorter->pgsz,
                      pTask->file.iEof);
    pTask->nPMA++;
    vdbePmaWriteVarint(&writer, pList->szPMA);
    కొరకు(p=pList->pList; p; p=p_తదుపరి){
      p_తదుపరి = p->u.p_తదుపరి;
      vdbePmaWriteVarint(&writer, p->nVal);
      vdbePmaWriteBlob(&writer, SRVAL(p), p->nVal);
      ఒకవేళ( pList->a_జ్ఞాపకం==0 ) sqlite3_free(p);
    }
    pList->pList = p;
    rc = vdbe_Pma_Writer_ముగింపు(&writer, &pTask->file.iEof, &pTask->nSpill);
  }

  vdbeSorterWorkDebug(pTask, "exit");
  ప్రతిజ్ఞా( rc!=SQLITE_OK || pList->pList==0 );
  ప్రతిజ్ఞా( rc!=SQLITE_OK || pTask->file.iEof==iSz );
  తిరిగి rc;
}

/*
** Advance the MergeEngine to its next entry.
** Set *pbEof to true there is no next entry because
** the MergeEngine has reached the end of all its inputs.
**
** Return SQLITE_OK if successful or an error code if an error occurs.
*/
స్థిర పూర్ణాంకం vdbe_విలీనం_Engine_అడుగు(
  విలీనం_Engine *pMerger,      /* The merge engine to advance to the next row */
  పూర్ణాంకం *pbEof                 /* Set TRUE at EOF.  Set false for more content */
){
  పూర్ణాంకం rc;
  పూర్ణాంకం iPrev = pMerger->a_వృక్షం[1];/* Index of PmaReader to advance */
  క్రమీకరించు_Subtask *pTask = pMerger->pTask;

  /* Advance the current PmaReader */
  rc = vdbe_Pma_Reader_తదుపరి(&pMerger->aReadr[iPrev]);

  /* Update contents of aTree[] */
  ఒకవేళ( rc==SQLITE_OK ){
    పూర్ణాంకం i;                      /* Index of aTree[] to recalculate */
    PmaReader *pReadr1;         /* First PmaReader to compare */
    PmaReader *pReadr2;         /* Second PmaReader to compare */
    పూర్ణాంకం bCached = 0;

    /* Find the first two PmaReaders to compare. The one that was just
    ** advanced (iPrev) and the one next to it in the array.  */
    pReadr1 = &pMerger->aReadr[(iPrev & 0xFFFE)];
    pReadr2 = &pMerger->aReadr[(iPrev | 0x0001)];

    కొరకు(i=(pMerger->n_వృక్షం+iPrev)/2; i>0; i=i/2){
      /* Compare pReadr1 and pReadr2. Store the result in variable iRes. */
      పూర్ణాంకం iRes;
      ఒకవేళ( pReadr1->pFd==0 ){
        iRes = +1;
      }లేకపోతే ఒకవేళ( pReadr2->pFd==0 ){
        iRes = -1;
      }లేకపోతే{
        iRes = pTask->xCompare(pTask, &bCached,
            pReadr1->a_కీలకం, pReadr1->n_కీలకం, pReadr2->a_కీలకం, pReadr2->n_కీలకం
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
      ఒకవేళ( iRes<0 || (iRes==0 && pReadr1<pReadr2) ){
        pMerger->a_వృక్షం[i] = (పూర్ణాంకం)(pReadr1 - pMerger->aReadr);
        pReadr2 = &pMerger->aReadr[ pMerger->a_వృక్షం[i ^ 0x0001] ];
        bCached = 0;
      }లేకపోతే{
        ఒకవేళ( pReadr1->pFd ) bCached = 0;
        pMerger->a_వృక్షం[i] = (పూర్ణాంకం)(pReadr2 - pMerger->aReadr);
        pReadr1 = &pMerger->aReadr[ pMerger->a_వృక్షం[i ^ 0x0001] ];
      }
    }
    *pbEof = (pMerger->aReadr[pMerger->a_వృక్షం[1]].pFd==0);
  }

  తిరిగి (rc==SQLITE_OK ? pTask->pUnpacked->err_సంకేతం : rc);
}

#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
/*
** The main routine for background threads that write level-0 PMAs.
*/
స్థిర శూన్య *vdbeSorterFlushThread(శూన్య *pCtx){
  క్రమీకరించు_Subtask *pTask = (క్రమీకరించు_Subtask*)pCtx;
  పూర్ణాంకం rc;                         /* Return code */
  ప్రతిజ్ఞా( pTask->b_పూర్తి==0 );
  rc = vdbe_Sorter_List_గమ్యం_PMA(pTask, &pTask->list);
  pTask->b_పూర్తి = 1;
  తిరిగి SQLITE_INT_గమ్యం_PTR(rc);
}
#అయితే_ముగింపు /* SQLITE_MAX_WORKER_THREADS>0 */

/*
** Flush the current contents of VdbeSorter.list to a new PMA, possibly
** using a background thread.
*/
స్థిర పూర్ణాంకం vdbeSorterFlushPMA(VdbeSorter *pSorter){
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు==0
  pSorter->bUsePMA = 1;
  తిరిగి vdbe_Sorter_List_గమ్యం_PMA(&pSorter->aTask[0], &pSorter->list);
#లేకపోతే
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం i;
  క్రమీకరించు_Subtask *pTask = 0;    /* Thread context used to create new PMA */
  పూర్ణాంకం n_కార్మికుడు = (pSorter->nTask-1);

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
  కొరకు(i=0; i<n_కార్మికుడు; i++){
    పూర్ణాంకం iTest = (pSorter->iPrev + i + 1) % n_కార్మికుడు;
    pTask = &pSorter->aTask[iTest];
    ఒకవేళ( pTask->b_పూర్తి ){
      rc = vdbeSorterJoinThread(pTask);
    }
    ఒకవేళ( rc!=SQLITE_OK || pTask->pThread==0 ) ఆపు;
  }

  ఒకవేళ( rc==SQLITE_OK ){
    ఒకవేళ( i==n_కార్మికుడు ){
      /* Use the foreground thread for this operation */
      rc = vdbe_Sorter_List_గమ్యం_PMA(&pSorter->aTask[n_కార్మికుడు], &pSorter->list);
    }లేకపోతే{
      /* Launch a background thread for this operation */
      u8 *aMem;
      శూన్య *pCtx;

      ప్రతిజ్ఞా( pTask!=0 );
      ప్రతిజ్ఞా( pTask->pThread==0 && pTask->b_పూర్తి==0 );
      ప్రతిజ్ఞా( pTask->list.pList==0 );
      ప్రతిజ్ఞా( pTask->list.a_జ్ఞాపకం==0 || pSorter->list.a_జ్ఞాపకం!=0 );

      aMem = pTask->list.a_జ్ఞాపకం;
      pCtx = (శూన్య*)pTask;
      pSorter->iPrev = (u8)(pTask - pSorter->aTask);
      pTask->list = pSorter->list;
      pSorter->list.pList = 0;
      pSorter->list.szPMA = 0;
      ఒకవేళ( aMem ){
        pSorter->list.a_జ్ఞాపకం = aMem;
        pSorter->n_జ్ఞాపకం = sqlite3_Malloc_కొలత(aMem);
      }లేకపోతే ఒకవేళ( pSorter->list.a_జ్ఞాపకం ){
        pSorter->list.a_జ్ఞాపకం = sqlite3Malloc(pSorter->n_జ్ఞాపకం);
        ఒకవేళ( !pSorter->list.a_జ్ఞాపకం ) తిరిగి SQLITE_NOMEM_BKPT;
      }

      rc = vdbeSorterCreateThread(pTask, vdbeSorterFlushThread, pCtx);
    }
  }

  తిరిగి rc;
#అయితే_ముగింపు /* SQLITE_MAX_WORKER_THREADS!=0 */
}

/*
** Add a record to the sorter.
*/
పూర్ణాంకం sqlite3VdbeSorterWrite(
  స్థిరరాశి VdbeCursor *pCsr,         /* Sorter cursor */
  Mem *pVal                       /* Memory cell containing record */
){
  VdbeSorter *pSorter;
  పూర్ణాంకం rc = SQLITE_OK;             /* Return Code */
  Sorter_రికార్డు *pNew;             /* New list element */
  పూర్ణాంకం bFlush;                     /* True to flush contents of memory to PMA */
  i64 nReq;                       /* Bytes of memory required */
  i64 nPMA;                       /* Bytes of PMA space required */
  పూర్ణాంకం t;                          /* serial type of first record field */

  ప్రతిజ్ఞా( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  getVarint32NR((స్థిరరాశి u8*)&pVal->z[1], t);
  ఒకవేళ( t>0 && t<10 ){
    ఒకవేళ( t==7 ){
      pSorter->type_ముసుగు &= SORTER_TYPE_REAL;
    }లేకపోతే{
      pSorter->type_ముసుగు &= (SORTER_TYPE_INTEGER|SORTER_TYPE_REAL);
    }
  }లేకపోతే ఒకవేళ( t>10 && (t & 0x01) ){
    pSorter->type_ముసుగు &= SORTER_TYPE_పాఠ్యం;
  }లేకపోతే{
    pSorter->type_ముసుగు = 0;
  }

  ప్రతిజ్ఞా( pSorter );

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
  nReq = pVal->n + పరిమాణం(Sorter_రికార్డు);
  nPMA = pVal->n + sqlite3VarintLen(pVal->n);
  ఒకవేళ( pSorter->mx_Pma_కొలత ){
    ఒకవేళ( pSorter->list.a_జ్ఞాపకం ){
      bFlush = pSorter->i_జ్ఞాపకం && (pSorter->i_జ్ఞాపకం+nReq) > pSorter->mx_Pma_కొలత;
    }లేకపోతే{
      bFlush = (
          (pSorter->list.szPMA > pSorter->mx_Pma_కొలత)
       || (pSorter->list.szPMA > pSorter->mn_Pma_కొలత && sqlite3_కుప్ప_Nearly_నిండిన())
      );
    }
    ఒకవేళ( bFlush ){
      rc = vdbeSorterFlushPMA(pSorter);
      pSorter->list.szPMA = 0;
      pSorter->i_జ్ఞాపకం = 0;
      ప్రతిజ్ఞా( rc!=SQLITE_OK || pSorter->list.pList==0 );
    }
  }

  pSorter->list.szPMA += nPMA;
  ఒకవేళ( nPMA>pSorter->mxKeysize ){
    pSorter->mxKeysize = nPMA;
  }

  ఒకవేళ( pSorter->list.a_జ్ఞాపకం ){
    పూర్ణాంకం nMin = pSorter->i_జ్ఞాపకం + nReq;

    ఒకవేళ( nMin>pSorter->n_జ్ఞాపకం ){
      u8 *aNew;
      sqlite3_int64 nNew = 2 * (sqlite3_int64)pSorter->n_జ్ఞాపకం;
      పూర్ణాంకం iListOff = -1;
      ఒకవేళ( pSorter->list.pList ){
        iListOff = (u8*)pSorter->list.pList - pSorter->list.a_జ్ఞాపకం;
      }
      అయితే( nNew < nMin ) nNew = nNew*2;
      ఒకవేళ( nNew > pSorter->mx_Pma_కొలత ) nNew = pSorter->mx_Pma_కొలత;
      ఒకవేళ( nNew < nMin ) nNew = nMin;
      aNew = sqlite3Realloc(pSorter->list.a_జ్ఞాపకం, nNew);
      ఒకవేళ( !aNew ) తిరిగి SQLITE_NOMEM_BKPT;
      ఒకవేళ( iListOff>=0 ){
        pSorter->list.pList = (Sorter_రికార్డు*)&aNew[iListOff];
      }
      pSorter->list.a_జ్ఞాపకం = aNew;
      pSorter->n_జ్ఞాపకం = nNew;
    }

    pNew = (Sorter_రికార్డు*)&pSorter->list.a_జ్ఞాపకం[pSorter->i_జ్ఞాపకం];
    pSorter->i_జ్ఞాపకం += ROUND8(nReq);
    ఒకవేళ( pSorter->list.pList ){
      pNew->u.i_తదుపరి = (పూర్ణాంకం)((u8*)(pSorter->list.pList) - pSorter->list.a_జ్ఞాపకం);
    }
  }లేకపోతే{
    pNew = (Sorter_రికార్డు *)sqlite3Malloc(nReq);
    ఒకవేళ( pNew==0 ){
      తిరిగి SQLITE_NOMEM_BKPT;
    }
    pNew->u.p_తదుపరి = pSorter->list.pList;
  }

  జ్ఞాపకం_నకలు(SRVAL(pNew), pVal->z, pVal->n);
  pNew->nVal = pVal->n;
  pSorter->list.pList = pNew;

  తిరిగి rc;
}

/*
** Read keys from pIncr->pMerger and populate pIncr->aFile[1]. The format
** of the data stored in aFile[1] is the same as that used by regular PMAs,
** except that the number-of-bytes varint is omitted from the start.
*/
స్థిర పూర్ణాంకం vdbeIncrPopulate(IncrMerger *pIncr){
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం rc2;
  i64 i_ప్రారంభం = pIncr->i_ప్రారంభం_Off;
  SorterFile *p_బహిర్గతం = &pIncr->aFile[1];
  క్రమీకరించు_Subtask *pTask = pIncr->pTask;
  విలీనం_Engine *pMerger = pIncr->pMerger;
  PmaWriter writer;
  ప్రతిజ్ఞా( pIncr->bEof==0 );

  vdbeSorterPopulateDebug(pTask, "enter");

  vdbePmaWriterInit(p_బహిర్గతం->pFd, &writer, pTask->pSorter->pgsz, i_ప్రారంభం);
  అయితే( rc==SQLITE_OK ){
    పూర్ణాంకం dummy;
    PmaReader *pReader = &pMerger->aReadr[ pMerger->a_వృక్షం[1] ];
    పూర్ణాంకం n_కీలకం = pReader->n_కీలకం;
    i64 iEof = writer.iWriteOff + writer.i_Buf_చివర;

    /* Check if the output file is full or if the input has been exhausted.
    ** In either case exit the loop. */
    ఒకవేళ( pReader->pFd==0 ) ఆపు;
    ఒకవేళ( (iEof + n_కీలకం + sqlite3VarintLen(n_కీలకం))>(i_ప్రారంభం + pIncr->mxSz) ) ఆపు;

    /* Write the next key to the output. */
    vdbePmaWriteVarint(&writer, n_కీలకం);
    vdbePmaWriteBlob(&writer, pReader->a_కీలకం, n_కీలకం);
    ప్రతిజ్ఞా( pIncr->pMerger->pTask==pTask );
    rc = vdbe_విలీనం_Engine_అడుగు(pIncr->pMerger, &dummy);
  }

  rc2 = vdbe_Pma_Writer_ముగింపు(&writer, &p_బహిర్గతం->iEof, &pTask->nSpill);
  ఒకవేళ( rc==SQLITE_OK ) rc = rc2;
  vdbeSorterPopulateDebug(pTask, "exit");
  తిరిగి rc;
}

#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
/*
** The main routine for background threads that populate aFile[1] of
** multi-threaded IncrMerger objects.
*/
స్థిర శూన్య *vdbeIncrPopulateThread(శూన్య *pCtx){
  IncrMerger *pIncr = (IncrMerger*)pCtx;
  శూన్య *pRet = SQLITE_INT_గమ్యం_PTR( vdbeIncrPopulate(pIncr) );
  pIncr->pTask->b_పూర్తి = 1;
  తిరిగి pRet;
}

/*
** Launch a background thread to populate aFile[1] of pIncr.
*/
స్థిర పూర్ణాంకం vdbeIncrBgPopulate(IncrMerger *pIncr){
  శూన్య *p = (శూన్య*)pIncr;
  ప్రతిజ్ఞా( pIncr->bUseThread );
  తిరిగి vdbeSorterCreateThread(pIncr->pTask, vdbeIncrPopulateThread, p);
}
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం vdbe_Incr_మార్పిడి(IncrMerger *pIncr){
  పూర్ణాంకం rc = SQLITE_OK;

#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
  ఒకవేళ( pIncr->bUseThread ){
    rc = vdbeSorterJoinThread(pIncr->pTask);

    ఒకవేళ( rc==SQLITE_OK ){
      SorterFile f0 = pIncr->aFile[0];
      pIncr->aFile[0] = pIncr->aFile[1];
      pIncr->aFile[1] = f0;
    }

    ఒకవేళ( rc==SQLITE_OK ){
      ఒకవేళ( pIncr->aFile[0].iEof==pIncr->i_ప్రారంభం_Off ){
        pIncr->bEof = 1;
      }లేకపోతే{
        rc = vdbeIncrBgPopulate(pIncr);
      }
    }
  }లేకపోతే
#అయితే_ముగింపు
  {
    rc = vdbeIncrPopulate(pIncr);
    pIncr->aFile[0] = pIncr->aFile[1];
    ఒకవేళ( pIncr->aFile[0].iEof==pIncr->i_ప్రారంభం_Off ){
      pIncr->bEof = 1;
    }
  }

  తిరిగి rc;
}

/*
** Allocate and return a new IncrMerger object to read data from pMerger.
**
** If an OOM condition is encountered, return NULL. In this case free the
** pMerger argument before returning.
*/
స్థిర పూర్ణాంకం vdbeIncrMergerNew(
  క్రమీకరించు_Subtask *pTask,     /* The thread that will be using the new IncrMerger */
  విలీనం_Engine *pMerger,   /* The MergeEngine that the IncrMerger will control */
  IncrMerger **pp_బహిర్గతం      /* Write the new IncrMerger here */
){
  పూర్ణాంకం rc = SQLITE_OK;
  IncrMerger *pIncr = *pp_బహిర్గతం = (IncrMerger*)
       (sqlite3FaultSim(100) ? 0 : sqlite3MallocZero(పరిమాణం(*pIncr)));
  ఒకవేళ( pIncr ){
    pIncr->pMerger = pMerger;
    pIncr->pTask = pTask;
    pIncr->mxSz = MAX(pTask->pSorter->mxKeysize+9,pTask->pSorter->mx_Pma_కొలత/2);
    pTask->file2.iEof += pIncr->mxSz;
  }లేకపోతే{
    vdbe_విలీనం_Engine_Free(pMerger);
    rc = SQLITE_NOMEM_BKPT;
  }
  ప్రతిజ్ఞా( *pp_బహిర్గతం!=0 || rc!=SQLITE_OK );
  తిరిగి rc;
}

#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
/*
** Set the "use-threads" flag on object pIncr.
*/
స్థిర శూన్య vdbe_Incr_Merger_సమితి_దారాలు(IncrMerger *pIncr){
  pIncr->bUseThread = 1;
  pIncr->pTask->file2.iEof -= pIncr->mxSz;
}
#అయితే_ముగింపు /* SQLITE_MAX_WORKER_THREADS>0 */



/*
** Recompute pMerger->aTree[iOut] by comparing the next keys on the
** two PmaReaders that feed that entry.  Neither of the PmaReaders
** are advanced.  This routine merely does the comparison.
*/
స్థిర శూన్య vdbe_విలీనం_Engine_Compare(
  విలీనం_Engine *pMerger,  /* Merge engine containing PmaReaders to compare */
  పూర్ణాంకం i_బహిర్గతం               /* Store the result in pMerger->aTree[iOut] */
){
  పూర్ణాంకం i1;
  పూర్ణాంకం i2;
  పూర్ణాంకం iRes;
  PmaReader *p1;
  PmaReader *p2;

  ప్రతిజ్ఞా( i_బహిర్గతం<pMerger->n_వృక్షం && i_బహిర్గతం>0 );

  ఒకవేళ( i_బహిర్గతం>=(pMerger->n_వృక్షం/2) ){
    i1 = (i_బహిర్గతం - pMerger->n_వృక్షం/2) * 2;
    i2 = i1 + 1;
  }లేకపోతే{
    i1 = pMerger->a_వృక్షం[i_బహిర్గతం*2];
    i2 = pMerger->a_వృక్షం[i_బహిర్గతం*2+1];
  }

  p1 = &pMerger->aReadr[i1];
  p2 = &pMerger->aReadr[i2];

  ఒకవేళ( p1->pFd==0 ){
    iRes = i2;
  }లేకపోతే ఒకవేళ( p2->pFd==0 ){
    iRes = i1;
  }లేకపోతే{
    క్రమీకరించు_Subtask *pTask = pMerger->pTask;
    పూర్ణాంకం bCached = 0;
    పూర్ణాంకం res;
    ప్రతిజ్ఞా( pTask->pUnpacked!=0 );  /* from vdbeSortSubtaskMain() */
    res = pTask->xCompare(
        pTask, &bCached, p1->a_కీలకం, p1->n_కీలకం, p2->a_కీలకం, p2->n_కీలకం
    );
    ఒకవేళ( res<=0 ){
      iRes = i1;
    }లేకపోతే{
      iRes = i2;
    }
  }

  pMerger->a_వృక్షం[i_బహిర్గతం] = iRes;
}

/*
** Allowed values for the eMode parameter to vdbeMergeEngineInit()
** and vdbePmaReaderIncrMergeInit().
**
** Only INCRINIT_NORMAL is valid in single-threaded builds (when
** SQLITE_MAX_WORKER_THREADS==0).  The other values are only used
** when there exists one or more separate worker threads.
*/
#నిర్వచించు INCRINIT_సాధారణ 0
#నిర్వచించు INCRINIT_TASK   1
#నిర్వచించు INCRINIT_మూలం   2

/*
** Forward reference required as the vdbeIncrMergeInit() and
** vdbePmaReaderIncrInit() routines are called mutually recursively when
** building a merge tree.
*/
స్థిర పూర్ణాంకం vdbePmaReaderIncrInit(PmaReader *pReadr, పూర్ణాంకం e_బహుళకం);

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
స్థిర పూర్ణాంకం vdbe_విలీనం_Engine_Init(
  క్రమీకరించు_Subtask *pTask,             /* Thread that will run pMerger */
  విలీనం_Engine *pMerger,           /* MergeEngine to initialize */
  పూర్ణాంకం e_బహుళకం                       /* One of the INCRINIT_XXX constants */
){
  పూర్ణాంకం rc = SQLITE_OK;             /* Return code */
  పూర్ణాంకం i;                          /* For looping over PmaReader objects */
  పూర్ణాంకం n_వృక్షం;                      /* Number of subtrees to merge */

  /* Failure to allocate the merge would have been detected prior to
  ** invoking this routine */
  ప్రతిజ్ఞా( pMerger!=0 );

  /* eMode is always INCRINIT_NORMAL in single-threaded mode */
  ప్రతిజ్ఞా( SQLITE_MAX_కార్మికుడు_దారాలు>0 || e_బహుళకం==INCRINIT_సాధారణ );

  /* Verify that the MergeEngine is assigned to a single thread */
  ప్రతిజ్ఞా( pMerger->pTask==0 );
  pMerger->pTask = pTask;

  n_వృక్షం = pMerger->n_వృక్షం;
  కొరకు(i=0; i<n_వృక్షం; i++){
    ఒకవేళ( SQLITE_MAX_కార్మికుడు_దారాలు>0 && e_బహుళకం==INCRINIT_మూలం ){
      /* PmaReaders should be normally initialized in order, as if they are
      ** reading from the same temp file this makes for more linear file IO.
      ** However, in the INCRINIT_ROOT case, if PmaReader aReadr[nTask-1] is
      ** in use it will block the vdbePmaReaderNext() call while it uses
      ** the main thread to fill its buffer. So calling PmaReaderNext()
      ** on this PmaReader before any of the multi-threaded PmaReaders takes
      ** better advantage of multi-processor hardware. */
      rc = vdbe_Pma_Reader_తదుపరి(&pMerger->aReadr[n_వృక్షం-i-1]);
    }లేకపోతే{
      rc = vdbePmaReaderIncrInit(&pMerger->aReadr[i], INCRINIT_సాధారణ);
    }
    ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
  }

  కొరకు(i=pMerger->n_వృక్షం-1; i>0; i--){
    vdbe_విలీనం_Engine_Compare(pMerger, i);
  }
  తిరిగి pTask->pUnpacked->err_సంకేతం;
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
స్థిర పూర్ణాంకం vdbe_Pma_Reader_Incr_విలీనం_Init(PmaReader *pReadr, పూర్ణాంకం e_బహుళకం){
  పూర్ణాంకం rc = SQLITE_OK;
  IncrMerger *pIncr = pReadr->pIncr;
  క్రమీకరించు_Subtask *pTask = pIncr->pTask;
  sqlite3 *db = pTask->pSorter->db;

  /* eMode is always INCRINIT_NORMAL in single-threaded mode */
  ప్రతిజ్ఞా( SQLITE_MAX_కార్మికుడు_దారాలు>0 || e_బహుళకం==INCRINIT_సాధారణ );

  rc = vdbe_విలీనం_Engine_Init(pTask, pIncr->pMerger, e_బహుళకం);

  /* Set up the required files for pIncr. A multi-threaded IncrMerge object
  ** requires two temp files to itself, whereas a single-threaded object
  ** only requires a region of pTask->file2. */
  ఒకవేళ( rc==SQLITE_OK ){
    పూర్ణాంకం mxSz = pIncr->mxSz;
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
    ఒకవేళ( pIncr->bUseThread ){
      rc = vdbe_Sorter_Open_తాత్కాలిక_File(db, mxSz, &pIncr->aFile[0].pFd);
      ఒకవేళ( rc==SQLITE_OK ){
        rc = vdbe_Sorter_Open_తాత్కాలిక_File(db, mxSz, &pIncr->aFile[1].pFd);
      }
    }లేకపోతే
#అయితే_ముగింపు
    /*if( !pIncr->bUseThread )*/{
      ఒకవేళ( pTask->file2.pFd==0 ){
        ప్రతిజ్ఞా( pTask->file2.iEof>0 );
        rc = vdbe_Sorter_Open_తాత్కాలిక_File(db, pTask->file2.iEof, &pTask->file2.pFd);
        pTask->file2.iEof = 0;
      }
      ఒకవేళ( rc==SQLITE_OK ){
        pIncr->aFile[1].pFd = pTask->file2.pFd;
        pIncr->i_ప్రారంభం_Off = pTask->file2.iEof;
        pTask->file2.iEof += mxSz;
      }
    }
  }

#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
  ఒకవేళ( rc==SQLITE_OK && pIncr->bUseThread ){
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
    ప్రతిజ్ఞా( e_బహుళకం==INCRINIT_మూలం || e_బహుళకం==INCRINIT_TASK );
    rc = vdbeIncrPopulate(pIncr);
  }
#అయితే_ముగింపు

  ఒకవేళ( rc==SQLITE_OK && (SQLITE_MAX_కార్మికుడు_దారాలు==0 || e_బహుళకం!=INCRINIT_TASK) ){
    rc = vdbe_Pma_Reader_తదుపరి(pReadr);
  }

  తిరిగి rc;
}

#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
/*
** The main routine for vdbePmaReaderIncrMergeInit() operations run in
** background threads.
*/
స్థిర శూన్య *vdbePmaReaderBgIncrInit(శూన్య *pCtx){
  PmaReader *pReader = (PmaReader*)pCtx;
  శూన్య *pRet = SQLITE_INT_గమ్యం_PTR(
                  vdbe_Pma_Reader_Incr_విలీనం_Init(pReader,INCRINIT_TASK)
               );
  pReader->pIncr->pTask->b_పూర్తి = 1;
  తిరిగి pRet;
}
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం vdbePmaReaderIncrInit(PmaReader *pReadr, పూర్ణాంకం e_బహుళకం){
  IncrMerger *pIncr = pReadr->pIncr;   /* Incremental merger */
  పూర్ణాంకం rc = SQLITE_OK;                  /* Return code */
  ఒకవేళ( pIncr ){
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
    ప్రతిజ్ఞా( pIncr->bUseThread==0 || e_బహుళకం==INCRINIT_TASK );
    ఒకవేళ( pIncr->bUseThread ){
      శూన్య *pCtx = (శూన్య*)pReadr;
      rc = vdbeSorterCreateThread(pIncr->pTask, vdbePmaReaderBgIncrInit, pCtx);
    }లేకపోతే
#అయితే_ముగింపు
    {
      rc = vdbe_Pma_Reader_Incr_విలీనం_Init(pReadr, e_బహుళకం);
    }
  }
  తిరిగి rc;
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
స్థిర పూర్ణాంకం vdbe_విలీనం_Engine_Level0(
  క్రమీకరించు_Subtask *pTask,             /* Sorter task to read from */
  పూర్ణాంకం nPMA,                       /* Number of PMAs to read */
  i64 *pi_స్థానభ్రంశం,                  /* IN/OUT: Readr offset in pTask->file */
  విలీనం_Engine **pp_బహిర్గతం             /* OUT: New merge-engine */
){
  విలీనం_Engine *pNew;              /* Merge engine to return */
  i64 iOff = *pi_స్థానభ్రంశం;
  పూర్ణాంకం i;
  పూర్ణాంకం rc = SQLITE_OK;

  *pp_బహిర్గతం = pNew = vdbe_విలీనం_Engine_New(nPMA);
  ఒకవేళ( pNew==0 ) rc = SQLITE_NOMEM_BKPT;

  కొరకు(i=0; i<nPMA && rc==SQLITE_OK; i++){
    i64 nDummy = 0;
    PmaReader *pReadr = &pNew->aReadr[i];
    rc = vdbePmaReaderInit(pTask, &pTask->file, iOff, pReadr, &nDummy);
    iOff = pReadr->iEof;
  }

  ఒకవేళ( rc!=SQLITE_OK ){
    vdbe_విలీనం_Engine_Free(pNew);
    *pp_బహిర్గతం = 0;
  }
  *pi_స్థానభ్రంశం = iOff;
  తిరిగి rc;
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
స్థిర పూర్ణాంకం vdbe_Sorter_వృక్షం_లోతు(పూర్ణాంకం nPMA){
  పూర్ణాంకం n_లోతు = 0;
  i64 nDiv = SORTER_MAX_విలీనం_లెక్క;
  అయితే( nDiv < (i64)nPMA ){
    nDiv = nDiv * SORTER_MAX_విలీనం_లెక్క;
    n_లోతు++;
  }
  తిరిగి n_లోతు;
}

/*
** pRoot is the root of an incremental merge-tree with depth nDepth (according
** to vdbeSorterTreeDepth()). pLeaf is the iSeq'th leaf to be added to the
** tree, counting from zero. This function adds pLeaf to the tree.
**
** If successful, SQLITE_OK is returned. If an error occurs, an SQLite error
** code is returned and pLeaf is freed.
*/
స్థిర పూర్ణాంకం vdbe_Sorter_కూడు_గమ్యం_వృక్షం(
  క్రమీకరించు_Subtask *pTask,             /* Task context */
  పూర్ణాంకం n_లోతు,                     /* Depth of tree according to TreeDepth() */
  పూర్ణాంకం iSeq,                       /* Sequence number of leaf within tree */
  విలీనం_Engine *p_మూలం,             /* Root of tree */
  విలీనం_Engine *p_ఆకు              /* Leaf to add to tree */
){
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం nDiv = 1;
  పూర్ణాంకం i;
  విలీనం_Engine *p = p_మూలం;
  IncrMerger *pIncr;

  rc = vdbeIncrMergerNew(pTask, p_ఆకు, &pIncr);

  కొరకు(i=1; i<n_లోతు; i++){
    nDiv = nDiv * SORTER_MAX_విలీనం_లెక్క;
  }

  కొరకు(i=1; i<n_లోతు && rc==SQLITE_OK; i++){
    పూర్ణాంకం iIter = (iSeq / nDiv) % SORTER_MAX_విలీనం_లెక్క;
    PmaReader *pReadr = &p->aReadr[iIter];

    ఒకవేళ( pReadr->pIncr==0 ){
      విలీనం_Engine *pNew = vdbe_విలీనం_Engine_New(SORTER_MAX_విలీనం_లెక్క);
      ఒకవేళ( pNew==0 ){
        rc = SQLITE_NOMEM_BKPT;
      }లేకపోతే{
        rc = vdbeIncrMergerNew(pTask, pNew, &pReadr->pIncr);
      }
    }
    ఒకవేళ( rc==SQLITE_OK ){
      p = pReadr->pIncr->pMerger;
      nDiv = nDiv / SORTER_MAX_విలీనం_లెక్క;
    }
  }

  ఒకవేళ( rc==SQLITE_OK ){
    p->aReadr[iSeq % SORTER_MAX_విలీనం_లెక్క].pIncr = pIncr;
  }లేకపోతే{
    vdbeIncrFree(pIncr);
  }
  తిరిగి rc;
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
స్థిర పూర్ణాంకం vdbe_Sorter_విలీనం_వృక్షం_Build(
  VdbeSorter *pSorter,       /* The VDBE cursor that implements the sort */
  విలీనం_Engine **pp_బహిర్గతం        /* Write the MergeEngine here */
){
  విలీనం_Engine *pMain = 0;
  పూర్ణాంకం rc = SQLITE_OK;
  పూర్ణాంకం iTask;

#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
  /* If the sorter uses more than one task, then create the top-level
  ** MergeEngine here. This MergeEngine will read data from exactly
  ** one PmaReader per sub-task.  */
  ప్రతిజ్ఞా( pSorter->b_Use_దారాలు || pSorter->nTask==1 );
  ఒకవేళ( pSorter->nTask>1 ){
    pMain = vdbe_విలీనం_Engine_New(pSorter->nTask);
    ఒకవేళ( pMain==0 ) rc = SQLITE_NOMEM_BKPT;
  }
#అయితే_ముగింపు

  కొరకు(iTask=0; rc==SQLITE_OK && iTask<pSorter->nTask; iTask++){
    క్రమీకరించు_Subtask *pTask = &pSorter->aTask[iTask];
    ప్రతిజ్ఞా( pTask->nPMA>0 || SQLITE_MAX_కార్మికుడు_దారాలు>0 );
    ఒకవేళ( SQLITE_MAX_కార్మికుడు_దారాలు==0 || pTask->nPMA ){
      విలీనం_Engine *p_మూలం = 0;     /* Root node of tree for this task */
      పూర్ణాంకం n_లోతు = vdbe_Sorter_వృక్షం_లోతు(pTask->nPMA);
      i64 iReadOff = 0;

      ఒకవేళ( pTask->nPMA<=SORTER_MAX_విలీనం_లెక్క ){
        rc = vdbe_విలీనం_Engine_Level0(pTask, pTask->nPMA, &iReadOff, &p_మూలం);
      }లేకపోతే{
        పూర్ణాంకం i;
        పూర్ణాంకం iSeq = 0;
        p_మూలం = vdbe_విలీనం_Engine_New(SORTER_MAX_విలీనం_లెక్క);
        ఒకవేళ( p_మూలం==0 ) rc = SQLITE_NOMEM_BKPT;
        కొరకు(i=0; i<pTask->nPMA && rc==SQLITE_OK; i += SORTER_MAX_విలీనం_లెక్క){
          విలీనం_Engine *pMerger = 0; /* New level-0 PMA merger */
          పూర్ణాంకం nReader;              /* Number of level-0 PMAs to merge */

          nReader = MIN(pTask->nPMA - i, SORTER_MAX_విలీనం_లెక్క);
          rc = vdbe_విలీనం_Engine_Level0(pTask, nReader, &iReadOff, &pMerger);
          ఒకవేళ( rc==SQLITE_OK ){
            rc = vdbe_Sorter_కూడు_గమ్యం_వృక్షం(pTask, n_లోతు, iSeq++, p_మూలం, pMerger);
          }
        }
      }

      ఒకవేళ( rc==SQLITE_OK ){
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
        ఒకవేళ( pMain!=0 ){
          rc = vdbeIncrMergerNew(pTask, p_మూలం, &pMain->aReadr[iTask].pIncr);
        }లేకపోతే
#అయితే_ముగింపు
        {
          ప్రతిజ్ఞా( pMain==0 );
          pMain = p_మూలం;
        }
      }లేకపోతే{
        vdbe_విలీనం_Engine_Free(p_మూలం);
      }
    }
  }

  ఒకవేళ( rc!=SQLITE_OK ){
    vdbe_విలీనం_Engine_Free(pMain);
    pMain = 0;
  }
  *pp_బహిర్గతం = pMain;
  తిరిగి rc;
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
స్థిర పూర్ణాంకం vdbe_Sorter_అమరిక_విలీనం(VdbeSorter *pSorter){
  పూర్ణాంకం rc;                         /* Return code */
  క్రమీకరించు_Subtask *pTask0 = &pSorter->aTask[0];
  విలీనం_Engine *pMain = 0;
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు
  sqlite3 *db = pTask0->pSorter->db;
  పూర్ణాంకం i;
  SorterCompare xCompare = vdbeSorterGetCompare(pSorter);
  కొరకు(i=0; i<pSorter->nTask; i++){
    pSorter->aTask[i].xCompare = xCompare;
  }
#అయితే_ముగింపు

  rc = vdbe_Sorter_విలీనం_వృక్షం_Build(pSorter, &pMain);
  ఒకవేళ( rc==SQLITE_OK ){
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు
    ప్రతిజ్ఞా( pSorter->b_Use_దారాలు==0 || pSorter->nTask>1 );
    ఒకవేళ( pSorter->b_Use_దారాలు ){
      పూర్ణాంకం iTask;
      PmaReader *pReadr = 0;
      క్రమీకరించు_Subtask *p_చివరి = &pSorter->aTask[pSorter->nTask-1];
      rc = vdbe_క్రమీకరించు_Alloc_Unpacked(p_చివరి);
      ఒకవేళ( rc==SQLITE_OK ){
        pReadr = (PmaReader*)sqlite3DbMallocZero(db, పరిమాణం(PmaReader));
        pSorter->pReader = pReadr;
        ఒకవేళ( pReadr==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      ఒకవేళ( rc==SQLITE_OK ){
        rc = vdbeIncrMergerNew(p_చివరి, pMain, &pReadr->pIncr);
        ఒకవేళ( rc==SQLITE_OK ){
          vdbe_Incr_Merger_సమితి_దారాలు(pReadr->pIncr);
          కొరకు(iTask=0; iTask<(pSorter->nTask-1); iTask++){
            IncrMerger *pIncr;
            ఒకవేళ( (pIncr = pMain->aReadr[iTask].pIncr) ){
              vdbe_Incr_Merger_సమితి_దారాలు(pIncr);
              ప్రతిజ్ఞా( pIncr->pTask!=p_చివరి );
            }
          }
          కొరకు(iTask=0; rc==SQLITE_OK && iTask<pSorter->nTask; iTask++){
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
            ప్రతిజ్ఞా( p->pIncr==0 || (
                (p->pIncr->pTask==&pSorter->aTask[iTask])             /* a */
             && (iTask!=pSorter->nTask-1 || p->pIncr->bUseThread==0)  /* b */
            ));
            rc = vdbePmaReaderIncrInit(p, INCRINIT_TASK);
          }
        }
        pMain = 0;
      }
      ఒకవేళ( rc==SQLITE_OK ){
        rc = vdbe_Pma_Reader_Incr_విలీనం_Init(pReadr, INCRINIT_మూలం);
      }
    }లేకపోతే
#అయితే_ముగింపు
    {
      rc = vdbe_విలీనం_Engine_Init(pTask0, pMain, INCRINIT_సాధారణ);
      pSorter->pMerger = pMain;
      pMain = 0;
    }
  }

  ఒకవేళ( rc!=SQLITE_OK ){
    vdbe_విలీనం_Engine_Free(pMain);
  }
  తిరిగి rc;
}


/*
** Once the sorter has been populated by calls to sqlite3VdbeSorterWrite,
** this function is called to prepare for iterating through the records
** in sorted order.
*/
పూర్ణాంకం sqlite3VdbeSorterRewind(స్థిరరాశి VdbeCursor *pCsr, పూర్ణాంకం *pbEof){
  VdbeSorter *pSorter;
  పూర్ణాంకం rc = SQLITE_OK;             /* Return code */

  ప్రతిజ్ఞా( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  ప్రతిజ్ఞా( pSorter );

  /* If no data has been written to disk, then do not do so now. Instead,
  ** sort the VdbeSorter.pRecord list. The vdbe layer will read data directly
  ** from the in-memory list.  */
  ఒకవేళ( pSorter->bUsePMA==0 ){
    ఒకవేళ( pSorter->list.pList ){
      *pbEof = 0;
      rc = vdbe_Sorter_క్రమీకరించు(&pSorter->aTask[0], &pSorter->list);
    }లేకపోతే{
      *pbEof = 1;
    }
    తిరిగి rc;
  }

  /* Write the current in-memory list to a PMA. When the VdbeSorterWrite()
  ** function flushes the contents of memory to disk, it immediately always
  ** creates a new list consisting of a single key immediately afterwards.
  ** So the list is never empty at this point.  */
  ప్రతిజ్ఞా( pSorter->list.pList );
  rc = vdbeSorterFlushPMA(pSorter);

  /* Join all threads */
  rc = vdbeSorterJoinAll(pSorter, rc);

  vdbeSorterRewindDebug("rewind");

  /* Assuming no errors have occurred, set up a merger structure to
  ** incrementally read and merge all remaining PMAs.  */
  ప్రతిజ్ఞా( pSorter->pReader==0 );
  ఒకవేళ( rc==SQLITE_OK ){
    rc = vdbe_Sorter_అమరిక_విలీనం(pSorter);
    *pbEof = 0;
  }

  vdbeSorterRewindDebug("rewinddone");
  తిరిగి rc;
}

/*
** Advance to the next element in the sorter.  Return value:
**
**    SQLITE_OK     success
**    SQLITE_DONE   end of data
**    otherwise     some kind of error.
*/
పూర్ణాంకం sqlite3_Vdbe_Sorter_తదుపరి(sqlite3 *db, స్థిరరాశి VdbeCursor *pCsr){
  VdbeSorter *pSorter;
  పూర్ణాంకం rc;                         /* Return code */

  ప్రతిజ్ఞా( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  ప్రతిజ్ఞా( pSorter->bUsePMA || (pSorter->pReader==0 && pSorter->pMerger==0) );
  ఒకవేళ( pSorter->bUsePMA ){
    ప్రతిజ్ఞా( pSorter->pReader==0 || pSorter->pMerger==0 );
    ప్రతిజ్ఞా( pSorter->b_Use_దారాలు==0 || pSorter->pReader );
    ప్రతిజ్ఞా( pSorter->b_Use_దారాలు==1 || pSorter->pMerger );
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
    ఒకవేళ( pSorter->b_Use_దారాలు ){
      rc = vdbe_Pma_Reader_తదుపరి(pSorter->pReader);
      ఒకవేళ( rc==SQLITE_OK && pSorter->pReader->pFd==0 ) rc = SQLITE_పూర్తి;
    }లేకపోతే
#అయితే_ముగింపు
    /*if( !pSorter->bUseThreads )*/ {
      పూర్ణాంకం res = 0;
      ప్రతిజ్ఞా( pSorter->pMerger!=0 );
      ప్రతిజ్ఞా( pSorter->pMerger->pTask==(&pSorter->aTask[0]) );
      rc = vdbe_విలీనం_Engine_అడుగు(pSorter->pMerger, &res);
      ఒకవేళ( rc==SQLITE_OK && res ) rc = SQLITE_పూర్తి;
    }
  }లేకపోతే{
    Sorter_రికార్డు *pFree = pSorter->list.pList;
    pSorter->list.pList = pFree->u.p_తదుపరి;
    pFree->u.p_తదుపరి = 0;
    ఒకవేళ( pSorter->list.a_జ్ఞాపకం==0 ) vdbe_Sorter_రికార్డు_Free(db, pFree);
    rc = pSorter->list.pList ? SQLITE_OK : SQLITE_పూర్తి;
  }
  తిరిగి rc;
}

/*
** Return a pointer to a buffer owned by the sorter that contains the
** current key.
*/
స్థిర శూన్య *vdbeSorterRowkey(
  స్థిరరాశి VdbeSorter *pSorter,      /* Sorter object */
  పూర్ణాంకం *pn_కీలకం                      /* OUT: Size of current key in bytes */
){
  శూన్య *p_కీలకం;
  ఒకవేళ( pSorter->bUsePMA ){
    PmaReader *pReader;
#ఒకవేళ SQLITE_MAX_కార్మికుడు_దారాలు>0
    ఒకవేళ( pSorter->b_Use_దారాలు ){
      pReader = pSorter->pReader;
    }లేకపోతే
#అయితే_ముగింపు
    /*if( !pSorter->bUseThreads )*/{
      pReader = &pSorter->pMerger->aReadr[pSorter->pMerger->a_వృక్షం[1]];
    }
    *pn_కీలకం = pReader->n_కీలకం;
    p_కీలకం = pReader->a_కీలకం;
  }లేకపోతే{
    *pn_కీలకం = pSorter->list.pList->nVal;
    p_కీలకం = SRVAL(pSorter->list.pList);
  }
  తిరిగి p_కీలకం;
}

/*
** Copy the current sorter key into the memory cell pOut.
*/
పూర్ణాంకం sqlite3VdbeSorterRowkey(స్థిరరాశి VdbeCursor *pCsr, Mem *p_బహిర్గతం){
  VdbeSorter *pSorter;
  శూన్య *p_కీలకం; పూర్ణాంకం n_కీలకం;           /* Sorter key to copy into pOut */

  ప్రతిజ్ఞా( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  p_కీలకం = vdbeSorterRowkey(pSorter, &n_కీలకం);
  ఒకవేళ( sqlite3VdbeMemClearAndResize(p_బహిర్గతం, n_కీలకం) ){
    తిరిగి SQLITE_NOMEM_BKPT;
  }
  p_బహిర్గతం->n = n_కీలకం;
  Mem_సమితి_Type_Flag(p_బహిర్గతం, MEM_Blob);
  జ్ఞాపకం_నకలు(p_బహిర్గతం->z, p_కీలకం, n_కీలకం);

  తిరిగి SQLITE_OK;
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
పూర్ణాంకం sqlite3VdbeSorterCompare(
  స్థిరరాశి VdbeCursor *pCsr,         /* Sorter cursor */
  Mem *pVal,                      /* Value to compare to current sorter key */
  పూర్ణాంకం n_కీలకం_నిలువు,                    /* Compare this many columns */
  పూర్ణాంకం *pRes                       /* OUT: Result of comparison */
){
  VdbeSorter *pSorter;
  Unpacked_రికార్డు *r2;
  కీలకం_Info *p_కీలకం_Info;
  పూర్ణాంకం i;
  శూన్య *p_కీలకం; పూర్ణాంకం n_కీలకం;           /* Sorter key to compare pVal with */

  ప్రతిజ్ఞా( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  r2 = pSorter->pUnpacked;
  p_కీలకం_Info = pCsr->p_కీలకం_Info;
  ఒకవేళ( r2==0 ){
    r2 = pSorter->pUnpacked = sqlite3_Vdbe_Alloc_Unpacked_రికార్డు(p_కీలకం_Info);
    ఒకవేళ( r2==0 ) తిరిగి SQLITE_NOMEM_BKPT;
    r2->n_క్షేత్రం = n_కీలకం_నిలువు;
  }
  ప్రతిజ్ఞా( r2->n_క్షేత్రం==n_కీలకం_నిలువు );

  p_కీలకం = vdbeSorterRowkey(pSorter, &n_కీలకం);
  sqlite3_Vdbe_రికార్డు_Unpack(n_కీలకం, p_కీలకం, r2);
  కొరకు(i=0; i<n_కీలకం_నిలువు; i++){
    ఒకవేళ( r2->aMem[i].flags & MEM_Null ){
      *pRes = -1;
      తిరిగి SQLITE_OK;
    }
  }

  *pRes = sqlite3_Vdbe_రికార్డు_Compare(pVal->n, pVal->z, r2);
  తిరిగి SQLITE_OK;
}
