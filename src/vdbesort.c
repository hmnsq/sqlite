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
#ضمن "sqliteInt.h"
#ضمن "vdbeInt.h"

/*
** If SQLITE_DEBUG_SORTER_THREADS is defined, this module outputs various
** messages to stderr that may be helpful in understanding the performance
** characteristics of the sorter in multi-threaded mode.
*/
#إذا 0
# define SQLITE_DEBUG_SORTER_THREADS 1
#endif

/*
** Hard-coded maximum amount of data to accumulate in memory before flushing
** to a level 0 PMA. The purpose of this limit is to prevent various integer
** overflows. 512MiB.
*/
#define SQLITE_MAX_PMASZ    (1<<29)

/*
** Private objects used by the sorter
*/
تعريفنوع بنية MergeEngine MergeEngine;     /* Merge PMAs together */
تعريفنوع بنية PmaReader PmaReader;         /* Incrementally read one PMA */
تعريفنوع بنية PmaWriter PmaWriter;         /* Incrementally write one PMA */
تعريفنوع بنية SorterRecord SorterRecord;   /* A record being sorted */
تعريفنوع بنية SortSubtask SortSubtask;     /* A sub-task in the sort process */
تعريفنوع بنية SorterFile SorterFile;       /* Temporary file object wrapper */
تعريفنوع بنية SorterList SorterList;       /* In-memory list of records */
تعريفنوع بنية IncrMerger IncrMerger;       /* Read & merge multiple PMAs */

/*
** A container for a temp file handle and the current amount of data
** stored in the file.
*/
بنية SorterFile {
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
بنية SorterList {
  SorterRecord *pList;            /* Linked list of records */
  u8 *aMemory;                    /* If non-NULL, bulk memory to hold pList */
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
بنية MergeEngine {
  صحيح n_ڤوکوق;                 /* Used size of aTree/aReadr (power of 2) */
  SortSubtask *pTask;        /* Used by this thread only */
  صحيح *a_ڤوکوق;                /* Current state of incremental merge */
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
تعريفنوع صحيح (*SorterCompare)(SortSubtask*,صحيح*,ثابت فراغ*,صحيح,ثابت فراغ*,صحيح);
بنية SortSubtask {
  SQLiteThread *pThread;          /* Background thread, if any */
  صحيح bDone;                      /* Set if thread is finished but not joined */
  صحيح nPMA;                       /* Number of PMAs currently in file */
  VdbeSorter *pSorter;            /* Sorter that owns this sub-task */
  UnpackedRecord *pUnpacked;      /* Space to unpack a record */
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
بنية VdbeSorter {
  صحيح mnPmaSize;                  /* Minimum PMA size, in bytes */
  صحيح mxPmaSize;                  /* Maximum PMA size, in bytes.  0==no limit */
  صحيح mxKeysize;                  /* Largest serialized key seen so far */
  صحيح pgsz;                       /* Main database page size */
  PmaReader *pReader;             /* Readr data from here after Rewind() */
  MergeEngine *pMerger;           /* Or here, if bUseThreads==0 */
  sqlite3 *db;                    /* Database connection */
  کونچي_Info *p_کونچي_Info;              /* How to compare records */
  UnpackedRecord *pUnpacked;      /* Used by VdbeSorterCompare() */
  SorterList list;                /* List of in-memory records */
  صحيح iMemory;                    /* Offset of free space in list.aMemory */
  صحيح nMemory;                    /* Size of list.aMemory allocation in bytes */
  u8 bUsePMA;                     /* True if one or more PMAs created */
  u8 bUseThreads;                 /* True to use background threads */
  u8 iPrev;                       /* Previous thread used to flush PMA */
  u8 nTask;                       /* Size of aTask[] array */
  u8 typeMask;
  SortSubtask aTask[FLEXARRAY];   /* One or more subtasks */
};

/* Size (in bytes) of a VdbeSorter object that works with N or fewer subtasks */
#define SZ_VDBESORTER(N)  (offsetof(VdbeSorter,aTask)+(N)*حجم(SortSubtask))

#define SORTER_TYPE_INTEGER 0x01
#define SORTER_TYPE_TEXT    0x02
#define SORTER_TYPE_REAL    0x04

/*
** An instance of the following object is used to read records out of a
** PMA, in sorted order.  The next key to be read is cached in nKey/aKey.
** aKey might point into aMap or into aBuffer.  If neither of those locations
** contain a contiguous representation of the key, then aAlloc is allocated
** and the key is copied into aAlloc and aKey is made to point to aAlloc.
**
** pFd==0 at EOF.
*/
بنية PmaReader {
  i64 iReadOff;               /* Current read offset */
  i64 iEof;                   /* 1 byte past EOF for this PmaReader */
  صحيح nAlloc;                 /* Bytes of space at aAlloc */
  صحيح n_کونچي;                   /* Number of bytes in key */
  sqlite3_file *pFd;          /* File handle we are reading from */
  u8 *aAlloc;                 /* Space for aKey if aBuffer and pMap wont work */
  u8 *a_کونچي;                   /* Pointer to current key */
  u8 *aBuffer;                /* Current read buffer */
  صحيح nBuffer;                /* Size of read buffer in bytes */
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
بنية IncrMerger {
  SortSubtask *pTask;             /* Task that owns this merger */
  MergeEngine *pMerger;           /* Merge engine thread reads data from */
  i64 iStartOff;                  /* Offset to start writing file at */
  صحيح mxSz;                       /* Maximum bytes of data to store */
  صحيح bEof;                       /* Set to true when merge is finished */
  صحيح bUseThread;                 /* True to use a bg thread for this object */
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
بنية PmaWriter {
  صحيح eFWErr;                     /* Non-zero if in an error state */
  u8 *aBuffer;                    /* Pointer to write buffer */
  صحيح nBuffer;                    /* Size of write buffer in bytes */
  صحيح iBufStart;                  /* First byte of buffer to write */
  صحيح iBufEnd;                    /* Last byte of buffer to write */
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
بنية SorterRecord {
  صحيح nVal;                       /* Size of the record in bytes */
  اتحاد {
    SorterRecord *pNext;          /* Pointer to next record in list */
    صحيح iNext;                    /* Offset within aMemory of next record */
  } u;
  /* The data for the record immediately follows this header */
};

/* Return a pointer to the buffer containing the record data for SorterRecord
** object p. Should be used as if:
**
**   void *SRVAL(SorterRecord *p) { return (void*)&p[1]; }
*/
#define SRVAL(p) ((فراغ*)((SorterRecord*)(p) + 1))


/* Maximum number of PMAs that a single MergeEngine can merge */
#define SORTER_MAX_MERGE_کيراءن 16

ساكن صحيح vdbeIncrSwap(IncrMerger*);
ساكن فراغ vdbeIncrFree(IncrMerger *);

/*
** Free all memory belonging to the PmaReader object passed as the
** argument. All structure fields are set to zero before returning.
*/
ساكن فراغ vdbePmaReaderClear(PmaReader *pReadr){
  sqlite3_free(pReadr->aAlloc);
  sqlite3_free(pReadr->aBuffer);
  إذا( pReadr->aMap ) sqlite3OsUnfetch(pReadr->pFd, 0, pReadr->aMap);
  vdbeIncrFree(pReadr->pIncr);
  memset(pReadr, 0, حجم(PmaReader));
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
ساكن صحيح vdbePmaReadBlob(
  PmaReader *p,                   /* PmaReader from which to take the blob */
  صحيح nByte,                      /* Bytes of data to read */
  u8 **ppOut                      /* OUT: Pointer to buffer containing data */
){
  صحيح iBuf;                       /* Offset within buffer to read from */
  صحيح nAvail;                     /* Bytes of data available in buffer */

  إذا( p->aMap ){
    *ppOut = &p->aMap[p->iReadOff];
    p->iReadOff += nByte;
    أرجع SQLITE_OK;
  }

  أكد( p->aBuffer );

  /* If there is no more data to be read from the buffer, read the next
  ** p->nBuffer bytes of data from the file into it. Or, if there are less
  ** than p->nBuffer bytes remaining in the PMA, read all remaining data.  */
  iBuf = p->iReadOff % p->nBuffer;
  إذا( iBuf==0 ){
    صحيح nRead;                    /* Bytes to read from disk */
    صحيح rc;                       /* sqlite3OsRead() return code */

    /* Determine how many bytes of data to read. */
    إذا( (p->iEof - p->iReadOff) > (i64)p->nBuffer ){
      nRead = p->nBuffer;
    }وإلا{
      nRead = (صحيح)(p->iEof - p->iReadOff);
    }
    أكد( nRead>0 );

    /* Readr data from the file. Return early if an error occurs. */
    rc = sqlite3OsRead(p->pFd, p->aBuffer, nRead, p->iReadOff);
    أكد( rc!=SQLITE_IOERR_SHORT_READ );
    إذا( rc!=SQLITE_OK ) أرجع rc;
  }
  nAvail = p->nBuffer - iBuf;

  إذا( nByte<=nAvail ){
    /* The requested data is available in the in-memory buffer. In this
    ** case there is no need to make a copy of the data, just return a
    ** pointer into the buffer to the caller.  */
    *ppOut = &p->aBuffer[iBuf];
    p->iReadOff += nByte;
  }وإلا{
    /* The requested data is not all available in the in-memory buffer.
    ** In this case, allocate space at p->aAlloc[] to copy the requested
    ** range into. Then return a copy of pointer p->aAlloc to the caller.  */
    صحيح nRem;                     /* Bytes remaining to copy */

    /* Extend the p->aAlloc[] allocation if required. */
    إذا( p->nAlloc<nByte ){
      u8 *aNew;
      sqlite3_int64 nNew = MAX(128, 2*(sqlite3_int64)p->nAlloc);
      طالما( nByte>nNew ) nNew = nNew*2;
      aNew = sqlite3Realloc(p->aAlloc, nNew);
      إذا( !aNew ) أرجع SQLITE_NOMEM_BKPT;
      p->nAlloc = nNew;
      p->aAlloc = aNew;
    }

    /* Copy as much data as is available in the buffer into the start of
    ** p->aAlloc[].  */
    memcpy(p->aAlloc, &p->aBuffer[iBuf], nAvail);
    p->iReadOff += nAvail;
    nRem = nByte - nAvail;

    /* The following loop copies up to p->nBuffer bytes per iteration into
    ** the p->aAlloc[] buffer.  */
    طالما( nRem>0 ){
      صحيح rc;                     /* vdbePmaReadBlob() return code */
      صحيح nCopy;                  /* Number of bytes to copy */
      u8 *aNext = 0;              /* Pointer to buffer to copy data from */

      nCopy = nRem;
      إذا( nRem>p->nBuffer ) nCopy = p->nBuffer;
      rc = vdbePmaReadBlob(p, nCopy, &aNext);
      إذا( rc!=SQLITE_OK ) أرجع rc;
      أكد( aNext!=p->aAlloc );
      أكد( aNext!=0 );
      memcpy(&p->aAlloc[nByte - nRem], aNext, nCopy);
      nRem -= nCopy;
    }

    *ppOut = p->aAlloc;
  }

  أرجع SQLITE_OK;
}

/*
** Read a varint from the stream of data accessed by p. Set *pnOut to
** the value read.
*/
ساكن صحيح vdbePmaReadVarint(PmaReader *p, u64 *pnOut){
  صحيح iBuf;

  إذا( p->aMap ){
    p->iReadOff += sqlite3GetVarint(&p->aMap[p->iReadOff], pnOut);
  }وإلا{
    iBuf = p->iReadOff % p->nBuffer;
    إذا( iBuf && (p->nBuffer-iBuf)>=9 ){
      p->iReadOff += sqlite3GetVarint(&p->aBuffer[iBuf], pnOut);
    }وإلا{
      u8 aVarint[16], *a;
      صحيح i = 0, rc;
      افعل{
        rc = vdbePmaReadBlob(p, 1, &a);
        إذا( rc ) أرجع rc;
        aVarint[(i++)&0xf] = a[0];
      }طالما( (a[0]&0x80)!=0 );
      sqlite3GetVarint(aVarint, pnOut);
    }
  }

  أرجع SQLITE_OK;
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
ساكن صحيح vdbeSorterMapFile(SortSubtask *pTask, SorterFile *pFile, u8 **pp){
  صحيح rc = SQLITE_OK;
  إذا( pFile->iEof<=(i64)(pTask->pSorter->db->nMaxSorterMmap) ){
    sqlite3_file *pFd = pFile->pFd;
    إذا( pFd->pMethods->iVersion>=3 ){
      rc = sqlite3OsFetch(pFd, 0, (صحيح)pFile->iEof, (فراغ**)pp);
      testcase( rc!=SQLITE_OK );
    }
  }
  أرجع rc;
}

/*
** Attach PmaReader pReadr to file pFile (if it is not already attached to
** that file) and seek it to offset iOff within the file.  Return SQLITE_OK
** if successful, or an SQLite error code if an error occurs.
*/
ساكن صحيح vdbePmaReaderSeek(
  SortSubtask *pTask,             /* Task context */
  PmaReader *pReadr,              /* Reader whose cursor is to be moved */
  SorterFile *pFile,              /* Sorter file to read from */
  i64 iOff                        /* Offset in pFile */
){
  صحيح rc = SQLITE_OK;

  أكد( pReadr->pIncr==0 || pReadr->pIncr->bEof==0 );

  إذا( sqlite3FaultSim(201) ) أرجع SQLITE_IOERR_READ;
  إذا( pReadr->aMap ){
    sqlite3OsUnfetch(pReadr->pFd, 0, pReadr->aMap);
    pReadr->aMap = 0;
  }
  pReadr->iReadOff = iOff;
  pReadr->iEof = pFile->iEof;
  pReadr->pFd = pFile->pFd;

  rc = vdbeSorterMapFile(pTask, pFile, &pReadr->aMap);
  إذا( rc==SQLITE_OK && pReadr->aMap==0 ){
    صحيح pgsz = pTask->pSorter->pgsz;
    صحيح iBuf = pReadr->iReadOff % pgsz;
    إذا( pReadr->aBuffer==0 ){
      pReadr->aBuffer = (u8*)sqlite3Malloc(pgsz);
      إذا( pReadr->aBuffer==0 ) rc = SQLITE_NOMEM_BKPT;
      pReadr->nBuffer = pgsz;
    }
    إذا( rc==SQLITE_OK && iBuf ){
      صحيح nRead = pgsz - iBuf;
      إذا( (pReadr->iReadOff + nRead) > pReadr->iEof ){
        nRead = (صحيح)(pReadr->iEof - pReadr->iReadOff);
      }
      rc = sqlite3OsRead(
          pReadr->pFd, &pReadr->aBuffer[iBuf], nRead, pReadr->iReadOff
      );
      testcase( rc!=SQLITE_OK );
    }
  }

  أرجع rc;
}

/*
** Advance PmaReader pReadr to the next key in its PMA. Return SQLITE_OK if
** no error occurs, or an SQLite error code if one does.
*/
ساكن صحيح vdbePmaReaderNext(PmaReader *pReadr){
  صحيح rc = SQLITE_OK;             /* Return Code */
  u64 nRec = 0;                   /* Size of record in bytes */


  إذا( pReadr->iReadOff>=pReadr->iEof ){
    IncrMerger *pIncr = pReadr->pIncr;
    صحيح bEof = 1;
    إذا( pIncr ){
      rc = vdbeIncrSwap(pIncr);
      إذا( rc==SQLITE_OK && pIncr->bEof==0 ){
        rc = vdbePmaReaderSeek(
            pIncr->pTask, pReadr, &pIncr->aFile[0], pIncr->iStartOff
        );
        bEof = 0;
      }
    }

    إذا( bEof ){
      /* This is an EOF condition */
      vdbePmaReaderClear(pReadr);
      testcase( rc!=SQLITE_OK );
      أرجع rc;
    }
  }

  إذا( rc==SQLITE_OK ){
    rc = vdbePmaReadVarint(pReadr, &nRec);
  }
  إذا( rc==SQLITE_OK ){
    pReadr->n_کونچي = (صحيح)nRec;
    rc = vdbePmaReadBlob(pReadr, (صحيح)nRec, &pReadr->a_کونچي);
    testcase( rc!=SQLITE_OK );
  }

  أرجع rc;
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
ساكن صحيح vdbePmaReaderInit(
  SortSubtask *pTask,             /* Task context */
  SorterFile *pFile,              /* Sorter file to read from */
  i64 iStart,                     /* Start offset in pFile */
  PmaReader *pReadr,              /* PmaReader to populate */
  i64 *pnByte                     /* IN/OUT: Increment this value by PMA size */
){
  صحيح rc;

  أكد( pFile->iEof>iStart );
  أكد( pReadr->aAlloc==0 && pReadr->nAlloc==0 );
  أكد( pReadr->aBuffer==0 );
  أكد( pReadr->aMap==0 );

  rc = vdbePmaReaderSeek(pTask, pReadr, pFile, iStart);
  إذا( rc==SQLITE_OK ){
    u64 nByte = 0;                 /* Size of PMA in bytes */
    rc = vdbePmaReadVarint(pReadr, &nByte);
    pReadr->iEof = pReadr->iReadOff + nByte;
    *pnByte += nByte;
  }

  إذا( rc==SQLITE_OK ){
    rc = vdbePmaReaderNext(pReadr);
  }
  أرجع rc;
}

/*
** A version of vdbeSorterCompare() that assumes that it has already been
** determined that the first field of key1 is equal to the first field of
** key2.
*/
ساكن صحيح vdbeSorterCompareTail(
  SortSubtask *pTask,             /* Subtask context (for pKeyInfo) */
  صحيح *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ثابت فراغ *pKey1, صحيح nKey1,   /* Left side of comparison */
  ثابت فراغ *pKey2, صحيح nKey2    /* Right side of comparison */
){
  UnpackedRecord *r2 = pTask->pUnpacked;
  إذا( *pbKey2Cached==0 ){
    sqlite3VdbeRecordUnpack(nKey2, pKey2, r2);
    *pbKey2Cached = 1;
  }
  أرجع sqlite3VdbeRecordCompareWithSkip(nKey1, pKey1, r2, 1);
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
ساكن صحيح vdbeSorterCompare(
  SortSubtask *pTask,             /* Subtask context (for pKeyInfo) */
  صحيح *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ثابت فراغ *pKey1, صحيح nKey1,   /* Left side of comparison */
  ثابت فراغ *pKey2, صحيح nKey2    /* Right side of comparison */
){
  UnpackedRecord *r2 = pTask->pUnpacked;
  إذا( !*pbKey2Cached ){
    sqlite3VdbeRecordUnpack(nKey2, pKey2, r2);
    *pbKey2Cached = 1;
  }
  أرجع sqlite3VdbeRecordCompare(nKey1, pKey1, r2);
}

/*
** A specially optimized version of vdbeSorterCompare() that assumes that
** the first field of each key is a TEXT value and that the collation
** sequence to compare them with is BINARY.
*/
ساكن صحيح vdbeSorterCompareText(
  SortSubtask *pTask,             /* Subtask context (for pKeyInfo) */
  صحيح *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ثابت فراغ *pKey1, صحيح nKey1,   /* Left side of comparison */
  ثابت فراغ *pKey2, صحيح nKey2    /* Right side of comparison */
){
  ثابت u8 * ثابت p1 = (ثابت u8 * ثابت)pKey1;
  ثابت u8 * ثابت p2 = (ثابت u8 * ثابت)pKey2;
  ثابت u8 * ثابت v1 = &p1[ p1[0] ];   /* Pointer to value 1 */
  ثابت u8 * ثابت v2 = &p2[ p2[0] ];   /* Pointer to value 2 */

  صحيح n1;
  صحيح n2;
  صحيح res;

  getVarint32NR(&p1[1], n1);
  getVarint32NR(&p2[1], n2);
  res = memcmp(v1, v2, (MIN(n1, n2) - 13)/2);
  إذا( res==0 ){
    res = n1 - n2;
  }

  إذا( res==0 ){
    إذا( pTask->pSorter->p_کونچي_Info->n_کونچي_Field>1 ){
      res = vdbeSorterCompareTail(
          pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2
      );
    }
  }وإلا{
    أكد( pTask->pSorter->p_کونچي_Info->aSortFlags!=0 );
    أكد( !(pTask->pSorter->p_کونچي_Info->aSortFlags[0]&KEYINFO_ORDER_BIGNULL) );
    إذا( pTask->pSorter->p_کونچي_Info->aSortFlags[0] ){
      res = res * -1;
    }
  }

  أرجع res;
}

/*
** A specially optimized version of vdbeSorterCompare() that assumes that
** the first field of each key is an INTEGER value.
*/
ساكن صحيح vdbeSorterCompareInt(
  SortSubtask *pTask,             /* Subtask context (for pKeyInfo) */
  صحيح *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ثابت فراغ *pKey1, صحيح nKey1,   /* Left side of comparison */
  ثابت فراغ *pKey2, صحيح nKey2    /* Right side of comparison */
){
  ثابت u8 * ثابت p1 = (ثابت u8 * ثابت)pKey1;
  ثابت u8 * ثابت p2 = (ثابت u8 * ثابت)pKey2;
  ثابت صحيح s1 = p1[1];                 /* Left hand serial type */
  ثابت صحيح s2 = p2[1];                 /* Right hand serial type */
  ثابت u8 * ثابت v1 = &p1[ p1[0] ];   /* Pointer to value 1 */
  ثابت u8 * ثابت v2 = &p2[ p2[0] ];   /* Pointer to value 2 */
  صحيح res;                              /* Return value */

  أكد( (s1>0 && s1<7) || s1==8 || s1==9 );
  أكد( (s2>0 && s2<7) || s2==8 || s2==9 );

  إذا( s1==s2 ){
    /* The two values have the same sign. Compare using memcmp(). */
    ساكن ثابت u8 aLen[] = {0, 1, 2, 3, 4, 6, 8, 0, 0, 0 };
    ثابت u8 n = aLen[s1];
    صحيح i;
    res = 0;
    لكل(i=0; i<n; i++){
      إذا( (res = v1[i] - v2[i])!=0 ){
        إذا( ((v1[0] ^ v2[0]) & 0x80)!=0 ){
          res = v1[0] & 0x80 ? -1 : +1;
        }
        اكسر;
      }
    }
  }وإلا إذا( s1>7 && s2>7 ){
    res = s1 - s2;
  }وإلا{
    إذا( s2>7 ){
      res = +1;
    }وإلا إذا( s1>7 ){
      res = -1;
    }وإلا{
      res = s1 - s2;
    }
    أكد( res!=0 );

    إذا( res>0 ){
      إذا( *v1 & 0x80 ) res = -1;
    }وإلا{
      إذا( *v2 & 0x80 ) res = +1;
    }
  }

  أكد( pTask->pSorter->p_کونچي_Info->aSortFlags!=0 );
  إذا( res==0 ){
    إذا( pTask->pSorter->p_کونچي_Info->n_کونچي_Field>1 ){
      res = vdbeSorterCompareTail(
          pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2
      );
    }
  }وإلا إذا( pTask->pSorter->p_کونچي_Info->aSortFlags[0] ){
    أكد( !(pTask->pSorter->p_کونچي_Info->aSortFlags[0]&KEYINFO_ORDER_BIGNULL) );
    res = res * -1;
  }

  أرجع res;
}

/* Helper function for vdbeSorterCompareReal().
**
** The first elements of both pKey1 and pKey2 have been decoded into double
** values r1 and r2.  Do the comparison between those keys and return the
** result.  If r1==r2, break the tie with a comparison of subsequent elements
** from each key.
*/
ساكن صحيح vdbeSorterFinishRealCompare(
  SortSubtask *pTask,             /* Subtask context (for pKeyInfo) */
  صحيح *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ثابت فراغ *pKey1, صحيح nKey1,   /* Left side of comparison */
  ثابت فراغ *pKey2, صحيح nKey2,   /* Right side of comparison */
  مزدوج r1,                      /* REAL value of first element of pKey1 */
  مزدوج r2                       /* REAL value of first element of pKey2 */
){
  صحيح res;
  إذا( r1<r2 ){
    res = -1;
  }وإلا إذا( r1>r2 ){
    res = +1;
  }وإلا{
    res = 0;
  }
  أكد( pTask->pSorter->p_کونچي_Info->aSortFlags!=0 );
  إذا( res==0 ){
    إذا( pTask->pSorter->p_کونچي_Info->n_کونچي_Field>1 ){
      res = vdbeSorterCompareTail(
          pTask, pbKey2Cached, pKey1, nKey1, pKey2, nKey2
      );
    }
  }وإلا إذا( pTask->pSorter->p_کونچي_Info->aSortFlags[0] ){
    أكد( !(pTask->pSorter->p_کونچي_Info->aSortFlags[0]&KEYINFO_ORDER_BIGNULL) );
    res = res * -1;
  }
  أرجع res;
}

/* Helper function for vdbeSorterCompareReal().
**
** Buffer p[] is a record where the first term is guaranteed to be either
** a floating-point value, or an integer stand-in for a floating point
** value (a MEM_IntReal).  Whatever its format, extract the value and
** return it.
*/
ساكن مزدوج vdbeSorterGetReal(ثابت u8 *p){
  مزدوج r;                    /* the return value */

  أكد( p[0]<0x80 );         /* 1-byte headers: nAllField<13 */
  أكد( p[1]>0 && p[1]<10 ); /* first fields proven numeric */

  إذا( p[1]==7 ){
    u64 x = sqlite3Get8byte(p + p[0]);
    swapMixedEndianFloat(x);
    أكد( !IsNaN(x) );
    memcpy(&r, &x, حجم(r));
  }وإلا{
    Mem m;
    m.u.i = 0;
    sqlite3VdbeSerialGet(p + p[0], p[1], &m);
    أكد( m.flags==MEM_Int );
    r = (مزدوج)m.u.i;
  }
  أرجع r;
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
ساكن SQLITE_NOINLINE صحيح vdbeSorterCompareRealInt(
  SortSubtask *pTask,             /* Subtask context (for pKeyInfo) */
  صحيح *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ثابت فراغ *pKey1, صحيح nKey1,   /* Left side of comparison */
  ثابت فراغ *pKey2, صحيح nKey2    /* Right side of comparison */
){
  ثابت u8 * ثابت p1 = (ثابت u8 * ثابت)pKey1;
  ثابت u8 * ثابت p2 = (ثابت u8 * ثابت)pKey2;
  إذا( p1[1]==6 || p2[1]==6 ){
    /* 64-bit integer values cannot be represented exactly by a double so
    ** must be handled by the generalized comparison function. */
    أرجع vdbeSorterCompare(pTask,
       pbKey2Cached, pKey1,nKey1, pKey2,nKey2);
  }وإلا{
    مزدوج r1 = vdbeSorterGetReal(p1);
    مزدوج r2 = vdbeSorterGetReal(p2);
    أرجع vdbeSorterFinishRealCompare(pTask,pbKey2Cached,
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
ساكن صحيح vdbeSorterCompareReal(
  SortSubtask *pTask,             /* Subtask context (for pKeyInfo) */
  صحيح *pbKey2Cached,              /* True if pTask->pUnpacked is pKey2 */
  ثابت فراغ *pKey1, صحيح nKey1,   /* Left side of comparison */
  ثابت فراغ *pKey2, صحيح nKey2    /* Right side of comparison */
){
  ثابت u8*ثابت p1 = (ثابت u8*ثابت)pKey1;  /* Left key record */
  ثابت u8*ثابت p2 = (ثابت u8*ثابت)pKey2;  /* Right key record */
  u64 x;                   /* A real value stored as an integer */
  مزدوج r1;               /* First element of pKey1 */
  مزدوج r2;               /* First element of pKey2 */

  أكد( p1[0]<0x80 && p2[0]<0x80 );  /* 1-byte headers: nAllField<13 */
  أكد( p1[1]>0 && p1[1]<10 );       /* first field guaranteed numeric */
  أكد( p2[1]>0 && p2[1]<10 );       /* first field guaranteed numeric */

  إذا( p1[1]!=7 || p2[1]!=7 ){
    /* One or both floating point values are stored as INTEGER.  This might
    ** be because of the MEM_RealInt encoding.  Try to optimize that case. */
    أرجع vdbeSorterCompareRealInt(pTask,
       pbKey2Cached, pKey1,nKey1, pKey2,nKey2
    );
  }
  أكد( p1[0]<=nKey1-8 && p2[0]<=nKey2-8 );

  x = sqlite3Get8byte(p1 + *p1);
  swapMixedEndianFloat(x);
  أكد( !IsNaN(x) );
  memcpy(&r1, &x, حجم(r1));
  x = sqlite3Get8byte(p2 + *p2);
  swapMixedEndianFloat(x);
  أكد( !IsNaN(x) );
  memcpy(&r2, &x, حجم(r2));
  أرجع vdbeSorterFinishRealCompare(pTask,
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
صحيح sqlite3VdbeSorterInit(
  sqlite3 *db,                    /* Database connection (for malloc()) */
  صحيح nField,                     /* Number of key fields in each record */
  VdbeCursor *pCsr                /* Cursor that holds the new sorter */
){
  صحيح pgsz;                       /* Page size of main database */
  صحيح i;                          /* Used to iterate through aTask[] */
  VdbeSorter *pSorter;            /* The new sorter */
  کونچي_Info *p_کونچي_Info;              /* Copy of pCsr->pKeyInfo with db==0 */
  صحيح sz_کونچي_Info;                  /* Size of pCsr->pKeyInfo in bytes */
  i64 sz;                         /* Size of pSorter in bytes */
  صحيح rc = SQLITE_OK;
#إذا SQLITE_MAX_WORKER_THREADS==0
# define nWorker 0
#وإلا
  صحيح nWorker;
#endif

  /* Initialize the upper limit on the number of worker threads */
#إذا SQLITE_MAX_WORKER_THREADS>0
  إذا( sqlite3TempInMemory(db) || sqlite3GlobalConfig.bCoreMutex==0 ){
    nWorker = 0;
  }وإلا{
    nWorker = db->aLimit[SQLITE_LIMIT_WORKER_THREADS];
  }
#endif

  /* Do not allow the total number of threads (main thread + all workers)
  ** to exceed the maximum merge count */
#إذا SQLITE_MAX_WORKER_THREADS>=SORTER_MAX_MERGE_کيراءن
  إذا( nWorker>=SORTER_MAX_MERGE_کيراءن ){
    nWorker = SORTER_MAX_MERGE_کيراءن-1;
  }
#endif

  أكد( pCsr->p_کونچي_Info );
  أكد( !pCsr->isEphemeral );
  أكد( pCsr->eCurType==CURTYPE_SORTER );
  أكد( حجم(کونچي_Info) + UMXV(pCsr->p_کونچي_Info->n_کونچي_Field)*حجم(CollSeq*)
               < 0x7fffffff );
  أكد( pCsr->p_کونچي_Info->n_کونچي_Field<=pCsr->p_کونچي_Info->nAllField );
  sz_کونچي_Info = SZ_KEYINFO(pCsr->p_کونچي_Info->nAllField);
  sz = SZ_VDBESORTER(nWorker+1);

  pSorter = (VdbeSorter*)sqlite3DbMallocZero(db, sz + sz_کونچي_Info);
  pCsr->uc.pSorter = pSorter;
  إذا( pSorter==0 ){
    rc = SQLITE_NOMEM_BKPT;
  }وإلا{
    Btree *pBt = db->aDb[0].pBt;
    pSorter->p_کونچي_Info = p_کونچي_Info = (کونچي_Info*)((u8*)pSorter + sz);
    memcpy(p_کونچي_Info, pCsr->p_کونچي_Info, sz_کونچي_Info);
    p_کونچي_Info->db = 0;
    إذا( nField && nWorker==0 ){
      p_کونچي_Info->n_کونچي_Field = nField;
      أكد( nField<=pCsr->p_کونچي_Info->nAllField );
    }
    /* It is OK that pKeyInfo reuses the aSortFlags field from pCsr->pKeyInfo,
    ** since the pCsr->pKeyInfo->aSortFlags[] array is invariant and lives
    ** longer that pSorter. */
    أكد( p_کونچي_Info->aSortFlags==pCsr->p_کونچي_Info->aSortFlags );
    sqlite3BtreeEnter(pBt);
    pSorter->pgsz = pgsz = sqlite3BtreeGetPageSize(pBt);
    sqlite3BtreeLeave(pBt);
    pSorter->nTask = nWorker + 1;
    pSorter->iPrev = (u8)(nWorker - 1);
    pSorter->bUseThreads = (pSorter->nTask>1);
    pSorter->db = db;
    لكل(i=0; i<pSorter->nTask; i++){
      SortSubtask *pTask = &pSorter->aTask[i];
      pTask->pSorter = pSorter;
    }

    إذا( !sqlite3TempInMemory(db) ){
      i64 mxCache;                /* Cache size in bytes*/
      u32 szPma = sqlite3GlobalConfig.szPma;
      pSorter->mnPmaSize = szPma * pgsz;

      mxCache = db->aDb[0].pSchema->cache_size;
      إذا( mxCache<0 ){
        /* A negative cache-size value C indicates that the cache is abs(C)
        ** KiB in size.  */
        mxCache = mxCache * -1024;
      }وإلا{
        mxCache = mxCache * pgsz;
      }
      mxCache = MIN(mxCache, SQLITE_MAX_PMASZ);
      pSorter->mxPmaSize = MAX(pSorter->mnPmaSize, (صحيح)mxCache);

      /* Avoid large memory allocations if the application has requested
      ** SQLITE_CONFIG_SMALL_MALLOC. */
      إذا( sqlite3GlobalConfig.bSmallMalloc==0 ){
        أكد( pSorter->iMemory==0 );
        pSorter->nMemory = pgsz;
        pSorter->list.aMemory = (u8*)sqlite3Malloc(pgsz);
        إذا( !pSorter->list.aMemory ) rc = SQLITE_NOMEM_BKPT;
      }
    }

    إذا( p_کونچي_Info->nAllField<13
     && (p_کونچي_Info->aColl[0]==0 || p_کونچي_Info->aColl[0]==db->pDfltColl)
     && (p_کونچي_Info->aSortFlags[0] & KEYINFO_ORDER_BIGNULL)==0
    ){
      pSorter->typeMask = SORTER_TYPE_INTEGER|SORTER_TYPE_TEXT|SORTER_TYPE_REAL;
    }
  }

  أرجع rc;
}
#undef nWorker   /* Defined at the top of this function */

/*
** Free the list of sorted records starting at pRecord.
*/
ساكن فراغ vdbeSorterRecordFree(sqlite3 *db, SorterRecord *pRecord){
  SorterRecord *p;
  SorterRecord *pNext;
  لكل(p=pRecord; p; p=pNext){
    pNext = p->u.pNext;
    sqlite3DbFree(db, p);
  }
}

/*
** Free all resources owned by the object indicated by argument pTask. All
** fields of *pTask are zeroed before returning.
*/
ساكن فراغ vdbeSortSubtaskCleanup(sqlite3 *db, SortSubtask *pTask){
  sqlite3DbFree(db, pTask->pUnpacked);
#إذا SQLITE_MAX_WORKER_THREADS>0
  /* pTask->list.aMemory can only be non-zero if it was handed memory
  ** from the main thread.  That only occurs SQLITE_MAX_WORKER_THREADS>0 */
  إذا( pTask->list.aMemory ){
    sqlite3_free(pTask->list.aMemory);
  }وإلا
#endif
  {
    أكد( pTask->list.aMemory==0 );
    vdbeSorterRecordFree(0, pTask->list.pList);
  }
  إذا( pTask->file.pFd ){
    sqlite3OsCloseFree(pTask->file.pFd);
  }
  إذا( pTask->file2.pFd ){
    sqlite3OsCloseFree(pTask->file2.pFd);
  }
  memset(pTask, 0, حجم(SortSubtask));
}

#ifdef SQLITE_DEBUG_SORTER_THREADS
ساكن فراغ vdbeSorterWorkDebug(SortSubtask *pTask, ثابت حرف *zEvent){
  i64 t;
  صحيح iTask = (pTask - pTask->pSorter->aTask);
  sqlite3OsCurrentTimeInt64(pTask->pSorter->db->pVfs, &t);
  fprintf(stderr, "%lld:%d %s\n", t, iTask, zEvent);
}
ساكن فراغ vdbeSorterRewindDebug(ثابت حرف *zEvent){
  i64 t = 0;
  sqlite3_vfs *pVfs = sqlite3_vfs_find(0);
  إذا( ALWAYS(pVfs) ) sqlite3OsCurrentTimeInt64(pVfs, &t);
  fprintf(stderr, "%lld:X %s\n", t, zEvent);
}
ساكن فراغ vdbeSorterPopulateDebug(
  SortSubtask *pTask,
  ثابت حرف *zEvent
){
  i64 t;
  صحيح iTask = (pTask - pTask->pSorter->aTask);
  sqlite3OsCurrentTimeInt64(pTask->pSorter->db->pVfs, &t);
  fprintf(stderr, "%lld:bg%d %s\n", t, iTask, zEvent);
}
ساكن فراغ vdbeSorterBlockDebug(
  SortSubtask *pTask,
  صحيح bBlocked,
  ثابت حرف *zEvent
){
  إذا( bBlocked ){
    i64 t;
    sqlite3OsCurrentTimeInt64(pTask->pSorter->db->pVfs, &t);
    fprintf(stderr, "%lld:main %s\n", t, zEvent);
  }
}
#وإلا
# define vdbeSorterWorkDebug(x,y)
# define vdbeSorterRewindDebug(y)
# define vdbeSorterPopulateDebug(x,y)
# define vdbeSorterBlockDebug(x,y,z)
#endif

#إذا SQLITE_MAX_WORKER_THREADS>0
/*
** Join thread pTask->thread.
*/
ساكن صحيح vdbeSorterJoinThread(SortSubtask *pTask){
  صحيح rc = SQLITE_OK;
  إذا( pTask->pThread ){
#ifdef SQLITE_DEBUG_SORTER_THREADS
    صحيح bDone = pTask->bDone;
#endif
    فراغ *pRet = SQLITE_INT_TO_PTR(SQLITE_ERROR);
    vdbeSorterBlockDebug(pTask, !bDone, "enter");
    (فراغ)sqlite3ThreadJoin(pTask->pThread, &pRet);
    vdbeSorterBlockDebug(pTask, !bDone, "exit");
    rc = SQLITE_PTR_TO_INT(pRet);
    أكد( pTask->bDone==1 );
    pTask->bDone = 0;
    pTask->pThread = 0;
  }
  أرجع rc;
}

/*
** Launch a background thread to run xTask(pIn).
*/
ساكن صحيح vdbeSorterCreateThread(
  SortSubtask *pTask,             /* Thread will use this task object */
  فراغ *(*xTask)(فراغ*),          /* Routine to run in a separate thread */
  فراغ *pIn                       /* Argument passed into xTask() */
){
  أكد( pTask->pThread==0 && pTask->bDone==0 );
  أرجع sqlite3ThreadCreate(&pTask->pThread, xTask, pIn);
}

/*
** Join all outstanding threads launched by SorterWrite() to create
** level-0 PMAs.
*/
ساكن صحيح vdbeSorterJoinAll(VdbeSorter *pSorter, صحيح rcin){
  صحيح rc = rcin;
  صحيح i;

  /* This function is always called by the main user thread.
  **
  ** If this function is being called after SorterRewind() has been called,
  ** it is possible that thread pSorter->aTask[pSorter->nTask-1].pThread
  ** is currently attempt to join one of the other threads. To avoid a race
  ** condition where this thread also attempts to join the same object, join
  ** thread pSorter->aTask[pSorter->nTask-1].pThread first. */
  لكل(i=pSorter->nTask-1; i>=0; i--){
    SortSubtask *pTask = &pSorter->aTask[i];
    صحيح rc2 = vdbeSorterJoinThread(pTask);
    إذا( rc==SQLITE_OK ) rc = rc2;
  }
  أرجع rc;
}
#وإلا
# define vdbeSorterJoinAll(x,rcin) (rcin)
# define vdbeSorterJoinThread(pTask) SQLITE_OK
#endif

/*
** Allocate a new MergeEngine object capable of handling up to
** nReader PmaReader inputs.
**
** nReader is automatically rounded up to the next power of two.
** nReader may not exceed SORTER_MAX_MERGE_COUNT even after rounding up.
*/
ساكن MergeEngine *vdbeMergeEngineNew(صحيح nReader){
  صحيح N = 2;                      /* Smallest power of two >= nReader */
  i64 nByte;                      /* Total bytes of space to allocate */
  MergeEngine *pNew;              /* Pointer to allocated object to return */

  أكد( nReader<=SORTER_MAX_MERGE_کيراءن );

  طالما( N<nReader ) N += N;
  nByte = حجم(MergeEngine) + N * (حجم(صحيح) + حجم(PmaReader));

  pNew = sqlite3FaultSim(100) ? 0 : (MergeEngine*)sqlite3MallocZero(nByte);
  إذا( pNew ){
    pNew->n_ڤوکوق = N;
    pNew->pTask = 0;
    pNew->aReadr = (PmaReader*)&pNew[1];
    pNew->a_ڤوکوق = (صحيح*)&pNew->aReadr[N];
  }
  أرجع pNew;
}

/*
** Free the MergeEngine object passed as the only argument.
*/
ساكن فراغ vdbeMergeEngineFree(MergeEngine *pMerger){
  صحيح i;
  إذا( pMerger ){
    لكل(i=0; i<pMerger->n_ڤوکوق; i++){
      vdbePmaReaderClear(&pMerger->aReadr[i]);
    }
  }
  sqlite3_free(pMerger);
}

/*
** Free all resources associated with the IncrMerger object indicated by
** the first argument.
*/
ساكن فراغ vdbeIncrFree(IncrMerger *pIncr){
  إذا( pIncr ){
#إذا SQLITE_MAX_WORKER_THREADS>0
    إذا( pIncr->bUseThread ){
      vdbeSorterJoinThread(pIncr->pTask);
      إذا( pIncr->aFile[0].pFd ) sqlite3OsCloseFree(pIncr->aFile[0].pFd);
      إذا( pIncr->aFile[1].pFd ) sqlite3OsCloseFree(pIncr->aFile[1].pFd);
    }
#endif
    vdbeMergeEngineFree(pIncr->pMerger);
    sqlite3_free(pIncr);
  }
}

/*
** Reset a sorting cursor back to its original empty state.
*/
فراغ sqlite3VdbeSorterReset(sqlite3 *db, VdbeSorter *pSorter){
  صحيح i;
  (فراغ)vdbeSorterJoinAll(pSorter, SQLITE_OK);
  أكد( pSorter->bUseThreads || pSorter->pReader==0 );
#إذا SQLITE_MAX_WORKER_THREADS>0
  إذا( pSorter->pReader ){
    vdbePmaReaderClear(pSorter->pReader);
    sqlite3DbFree(db, pSorter->pReader);
    pSorter->pReader = 0;
  }
#endif
  vdbeMergeEngineFree(pSorter->pMerger);
  pSorter->pMerger = 0;
  لكل(i=0; i<pSorter->nTask; i++){
    SortSubtask *pTask = &pSorter->aTask[i];
    vdbeSortSubtaskCleanup(db, pTask);
    pTask->pSorter = pSorter;
  }
  إذا( pSorter->list.aMemory==0 ){
    vdbeSorterRecordFree(0, pSorter->list.pList);
  }
  pSorter->list.pList = 0;
  pSorter->list.szPMA = 0;
  pSorter->bUsePMA = 0;
  pSorter->iMemory = 0;
  pSorter->mxKeysize = 0;
  sqlite3DbFree(db, pSorter->pUnpacked);
  pSorter->pUnpacked = 0;
}

/*
** Free any cursor components allocated by sqlite3VdbeSorterXXX routines.
*/
فراغ sqlite3VdbeSorterClose(sqlite3 *db, VdbeCursor *pCsr){
  VdbeSorter *pSorter;
  أكد( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  إذا( pSorter ){
    /* Increment db->nSpill by the total number of bytes of data written
    ** to temp files by this sort operation.  */
    صحيح ii;
    لكل(ii=0; ii<pSorter->nTask; ii++){
      db->nSpill += pSorter->aTask[ii].nSpill;
    }
    sqlite3VdbeSorterReset(db, pSorter);
    sqlite3_free(pSorter->list.aMemory);
    sqlite3DbFree(db, pSorter);
    pCsr->uc.pSorter = 0;
  }
}

#إذا SQLITE_MAX_MMAP_SIZE>0
/*
** The first argument is a file-handle open on a temporary file. The file
** is guaranteed to be nByte bytes or smaller in size. This function
** attempts to extend the file to nByte bytes in size and to ensure that
** the VFS has memory mapped it.
**
** Whether or not the file does end up memory mapped of course depends on
** the specific VFS implementation.
*/
ساكن فراغ vdbeSorterExtendFile(sqlite3 *db, sqlite3_file *pFd, i64 nByte){
  إذا( nByte<=(i64)(db->nMaxSorterMmap) && pFd->pMethods->iVersion>=3 ){
    فراغ *p = 0;
    صحيح chunksize = 4*1024;
    sqlite3OsFileControlHint(pFd, SQLITE_FCNTL_CHUNK_SIZE, &chunksize);
    sqlite3OsFileControlHint(pFd, SQLITE_FCNTL_SIZE_HINT, &nByte);
    sqlite3OsFetch(pFd, 0, (صحيح)nByte, &p);
    إذا( p ) sqlite3OsUnfetch(pFd, 0, p);
  }
}
#وإلا
# define vdbeSorterExtendFile(x,y,z)
#endif

/*
** Allocate space for a file-handle and open a temporary file. If successful,
** set *ppFd to point to the malloc'd file-handle and return SQLITE_OK.
** Otherwise, set *ppFd to 0 and return an SQLite error code.
*/
ساكن صحيح vdbeSorterOpenTempFile(
  sqlite3 *db,                    /* Database handle doing sort */
  i64 nExtend,                    /* Attempt to extend file to this size */
  sqlite3_file **ppFd
){
  صحيح rc;
  إذا( sqlite3FaultSim(202) ) أرجع SQLITE_IOERR_ACCESS;
  rc = sqlite3OsOpenMalloc(db->pVfs, 0, ppFd,
      SQLITE_OPEN_TEMP_JOURNAL |
      SQLITE_OPEN_READWRITE    | SQLITE_OPEN_CREATE |
      SQLITE_OPEN_EXCLUSIVE    | SQLITE_OPEN_DELETEONCLOSE, &rc
  );
  إذا( rc==SQLITE_OK ){
    i64 max = SQLITE_MAX_MMAP_SIZE;
    sqlite3OsFileControlHint(*ppFd, SQLITE_FCNTL_MMAP_SIZE, (فراغ*)&max);
    إذا( nExtend>0 ){
      vdbeSorterExtendFile(db, *ppFd, nExtend);
    }
  }
  أرجع rc;
}

/*
** If it has not already been allocated, allocate the UnpackedRecord
** structure at pTask->pUnpacked. Return SQLITE_OK if successful (or
** if no allocation was required), or SQLITE_NOMEM otherwise.
*/
ساكن صحيح vdbeSortAllocUnpacked(SortSubtask *pTask){
  إذا( pTask->pUnpacked==0 ){
    pTask->pUnpacked = sqlite3VdbeAllocUnpackedRecord(pTask->pSorter->p_کونچي_Info);
    إذا( pTask->pUnpacked==0 ) أرجع SQLITE_NOMEM_BKPT;
    pTask->pUnpacked->nField = pTask->pSorter->p_کونچي_Info->n_کونچي_Field;
    pTask->pUnpacked->errCode = 0;
  }
  أرجع SQLITE_OK;
}


/*
** Merge the two sorted lists p1 and p2 into a single list.
*/
ساكن SorterRecord *vdbeSorterMerge(
  SortSubtask *pTask,             /* Calling thread context */
  SorterRecord *p1,               /* First list to merge */
  SorterRecord *p2                /* Second list to merge */
){
  SorterRecord *pFinal = 0;
  SorterRecord **pp = &pFinal;
  صحيح bCached = 0;

  أكد( p1!=0 && p2!=0 );
  لكل(;;){
    صحيح res;
    res = pTask->xCompare(
        pTask, &bCached, SRVAL(p1), p1->nVal, SRVAL(p2), p2->nVal
    );

    إذا( res<=0 ){
      *pp = p1;
      pp = &p1->u.pNext;
      p1 = p1->u.pNext;
      إذا( p1==0 ){
        *pp = p2;
        اكسر;
      }
    }وإلا{
      *pp = p2;
      pp = &p2->u.pNext;
      p2 = p2->u.pNext;
      bCached = 0;
      إذا( p2==0 ){
        *pp = p1;
        اكسر;
      }
    }
  }
  أرجع pFinal;
}

/*
** Return the SorterCompare function to compare values collected by the
** sorter object passed as the only argument.
*/
ساكن SorterCompare vdbeSorterGetCompare(VdbeSorter *p){
  إذا( p->typeMask & SORTER_TYPE_INTEGER ){
    أرجع vdbeSorterCompareInt;
  }وإلا إذا( p->typeMask & SORTER_TYPE_TEXT ){
    أرجع vdbeSorterCompareText;
  }وإلا إذا( p->typeMask & SORTER_TYPE_REAL ){
    أرجع vdbeSorterCompareReal;
  }
  أرجع vdbeSorterCompare;
}

/*
** Sort the linked list of records headed at pTask->pList. Return
** SQLITE_OK if successful, or an SQLite error code (i.e. SQLITE_NOMEM) if
** an error occurs.
*/
ساكن صحيح vdbeSorterSort(SortSubtask *pTask, SorterList *pList){
  صحيح i;
  SorterRecord *p;
  صحيح rc;
  SorterRecord *aSlot[64];

  rc = vdbeSortAllocUnpacked(pTask);
  إذا( rc!=SQLITE_OK ) أرجع rc;

  p = pList->pList;
  pTask->xCompare = vdbeSorterGetCompare(pTask->pSorter);
  memset(aSlot, 0, حجم(aSlot));

  طالما( p ){
    SorterRecord *pNext;
    إذا( pList->aMemory ){
      إذا( (u8*)p==pList->aMemory ){
        pNext = 0;
      }وإلا{
        أكد( p->u.iNext<sqlite3MallocSize(pList->aMemory) );
        pNext = (SorterRecord*)&pList->aMemory[p->u.iNext];
      }
    }وإلا{
      pNext = p->u.pNext;
    }

    p->u.pNext = 0;
    لكل(i=0; aSlot[i]; i++){
      p = vdbeSorterMerge(pTask, p, aSlot[i]);
      /* ,--Each aSlot[] holds twice as much as the previous. So we cannot use
      ** |  up all 64 aSlots[] with only a 64-bit address space.
      ** v                                                                */
      أكد( i<ArraySize(aSlot) );
      aSlot[i] = 0;
    }
    aSlot[i] = p;
    p = pNext;
  }

  p = 0;
  لكل(i=0; i<ArraySize(aSlot); i++){
    إذا( aSlot[i]==0 ) استمر;
    p = p ? vdbeSorterMerge(pTask, p, aSlot[i]) : aSlot[i];
  }
  pList->pList = p;

  أكد( pTask->pUnpacked->errCode==SQLITE_OK
       || pTask->pUnpacked->errCode==SQLITE_NOMEM
  );
  أرجع pTask->pUnpacked->errCode;
}

/*
** Initialize a PMA-writer object.
*/
ساكن فراغ vdbePmaWriterInit(
  sqlite3_file *pFd,              /* File handle to write to */
  PmaWriter *p,                   /* Object to populate */
  صحيح nBuf,                       /* Buffer size */
  i64 iStart                      /* Offset of pFd to begin writing at */
){
  memset(p, 0, حجم(PmaWriter));
  p->aBuffer = (u8*)sqlite3Malloc(nBuf);
  إذا( !p->aBuffer ){
    p->eFWErr = SQLITE_NOMEM_BKPT;
  }وإلا{
    p->iBufEnd = p->iBufStart = (iStart % nBuf);
    p->iWriteOff = iStart - p->iBufStart;
    p->nBuffer = nBuf;
    p->pFd = pFd;
  }
}

/*
** Write nData bytes of data to the PMA. Return SQLITE_OK
** if successful, or an SQLite error code if an error occurs.
*/
ساكن فراغ vdbePmaWriteBlob(PmaWriter *p, u8 *pData, صحيح nData){
  صحيح nRem = nData;
  طالما( nRem>0 && p->eFWErr==0 ){
    صحيح nCopy = nRem;
    إذا( nCopy>(p->nBuffer - p->iBufEnd) ){
      nCopy = p->nBuffer - p->iBufEnd;
    }

    memcpy(&p->aBuffer[p->iBufEnd], &pData[nData-nRem], nCopy);
    p->iBufEnd += nCopy;
    إذا( p->iBufEnd==p->nBuffer ){
      p->eFWErr = sqlite3OsWrite(p->pFd,
          &p->aBuffer[p->iBufStart], p->iBufEnd - p->iBufStart,
          p->iWriteOff + p->iBufStart
      );
      p->nPmaSpill += (p->iBufEnd - p->iBufStart);
      p->iBufStart = p->iBufEnd = 0;
      p->iWriteOff += p->nBuffer;
    }
    أكد( p->iBufEnd<p->nBuffer );

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
ساكن صحيح vdbePmaWriterFinish(PmaWriter *p, i64 *piEof, u64 *pnSpill){
  صحيح rc;
  إذا( p->eFWErr==0 && ALWAYS(p->aBuffer) && p->iBufEnd>p->iBufStart ){
    p->eFWErr = sqlite3OsWrite(p->pFd,
        &p->aBuffer[p->iBufStart], p->iBufEnd - p->iBufStart,
        p->iWriteOff + p->iBufStart
    );
    p->nPmaSpill += (p->iBufEnd - p->iBufStart);
  }
  *piEof = (p->iWriteOff + p->iBufEnd);
  *pnSpill += p->nPmaSpill;
  sqlite3_free(p->aBuffer);
  rc = p->eFWErr;
  memset(p, 0, حجم(PmaWriter));
  أرجع rc;
}

/*
** Write value iVal encoded as a varint to the PMA. Return
** SQLITE_OK if successful, or an SQLite error code if an error occurs.
*/
ساكن فراغ vdbePmaWriteVarint(PmaWriter *p, u64 iVal){
  صحيح nByte;
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
ساكن صحيح vdbeSorterListToPMA(SortSubtask *pTask, SorterList *pList){
  sqlite3 *db = pTask->pSorter->db;
  صحيح rc = SQLITE_OK;             /* Return code */
  PmaWriter writer;               /* Object used to write to the file */

#ifdef SQLITE_DEBUG
  /* Set iSz to the expected size of file pTask->file after writing the PMA.
  ** This is used by an assert() statement at the end of this function.  */
  i64 iSz = pList->szPMA + sqlite3VarintLen(pList->szPMA) + pTask->file.iEof;
#endif

  vdbeSorterWorkDebug(pTask, "enter");
  memset(&writer, 0, حجم(PmaWriter));
  أكد( pList->szPMA>0 );

  /* If the first temporary PMA file has not been opened, open it now. */
  إذا( pTask->file.pFd==0 ){
    rc = vdbeSorterOpenTempFile(db, 0, &pTask->file.pFd);
    أكد( rc!=SQLITE_OK || pTask->file.pFd );
    أكد( pTask->file.iEof==0 );
    أكد( pTask->nPMA==0 );
  }

  /* Try to get the file to memory map */
  إذا( rc==SQLITE_OK ){
    vdbeSorterExtendFile(db, pTask->file.pFd, pTask->file.iEof+pList->szPMA+9);
  }

  /* Sort the list */
  إذا( rc==SQLITE_OK ){
    rc = vdbeSorterSort(pTask, pList);
  }

  إذا( rc==SQLITE_OK ){
    SorterRecord *p;
    SorterRecord *pNext = 0;

    vdbePmaWriterInit(pTask->file.pFd, &writer, pTask->pSorter->pgsz,
                      pTask->file.iEof);
    pTask->nPMA++;
    vdbePmaWriteVarint(&writer, pList->szPMA);
    لكل(p=pList->pList; p; p=pNext){
      pNext = p->u.pNext;
      vdbePmaWriteVarint(&writer, p->nVal);
      vdbePmaWriteBlob(&writer, SRVAL(p), p->nVal);
      إذا( pList->aMemory==0 ) sqlite3_free(p);
    }
    pList->pList = p;
    rc = vdbePmaWriterFinish(&writer, &pTask->file.iEof, &pTask->nSpill);
  }

  vdbeSorterWorkDebug(pTask, "exit");
  أكد( rc!=SQLITE_OK || pList->pList==0 );
  أكد( rc!=SQLITE_OK || pTask->file.iEof==iSz );
  أرجع rc;
}

/*
** Advance the MergeEngine to its next entry.
** Set *pbEof to true there is no next entry because
** the MergeEngine has reached the end of all its inputs.
**
** Return SQLITE_OK if successful or an error code if an error occurs.
*/
ساكن صحيح vdbeMergeEngineStep(
  MergeEngine *pMerger,      /* The merge engine to advance to the next row */
  صحيح *pbEof                 /* Set TRUE at EOF.  Set false for more content */
){
  صحيح rc;
  صحيح iPrev = pMerger->a_ڤوکوق[1];/* Index of PmaReader to advance */
  SortSubtask *pTask = pMerger->pTask;

  /* Advance the current PmaReader */
  rc = vdbePmaReaderNext(&pMerger->aReadr[iPrev]);

  /* Update contents of aTree[] */
  إذا( rc==SQLITE_OK ){
    صحيح i;                      /* Index of aTree[] to recalculate */
    PmaReader *pReadr1;         /* First PmaReader to compare */
    PmaReader *pReadr2;         /* Second PmaReader to compare */
    صحيح bCached = 0;

    /* Find the first two PmaReaders to compare. The one that was just
    ** advanced (iPrev) and the one next to it in the array.  */
    pReadr1 = &pMerger->aReadr[(iPrev & 0xFFFE)];
    pReadr2 = &pMerger->aReadr[(iPrev | 0x0001)];

    لكل(i=(pMerger->n_ڤوکوق+iPrev)/2; i>0; i=i/2){
      /* Compare pReadr1 and pReadr2. Store the result in variable iRes. */
      صحيح iRes;
      إذا( pReadr1->pFd==0 ){
        iRes = +1;
      }وإلا إذا( pReadr2->pFd==0 ){
        iRes = -1;
      }وإلا{
        iRes = pTask->xCompare(pTask, &bCached,
            pReadr1->a_کونچي, pReadr1->n_کونچي, pReadr2->a_کونچي, pReadr2->n_کونچي
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
      إذا( iRes<0 || (iRes==0 && pReadr1<pReadr2) ){
        pMerger->a_ڤوکوق[i] = (صحيح)(pReadr1 - pMerger->aReadr);
        pReadr2 = &pMerger->aReadr[ pMerger->a_ڤوکوق[i ^ 0x0001] ];
        bCached = 0;
      }وإلا{
        إذا( pReadr1->pFd ) bCached = 0;
        pMerger->a_ڤوکوق[i] = (صحيح)(pReadr2 - pMerger->aReadr);
        pReadr1 = &pMerger->aReadr[ pMerger->a_ڤوکوق[i ^ 0x0001] ];
      }
    }
    *pbEof = (pMerger->aReadr[pMerger->a_ڤوکوق[1]].pFd==0);
  }

  أرجع (rc==SQLITE_OK ? pTask->pUnpacked->errCode : rc);
}

#إذا SQLITE_MAX_WORKER_THREADS>0
/*
** The main routine for background threads that write level-0 PMAs.
*/
ساكن فراغ *vdbeSorterFlushThread(فراغ *pCtx){
  SortSubtask *pTask = (SortSubtask*)pCtx;
  صحيح rc;                         /* Return code */
  أكد( pTask->bDone==0 );
  rc = vdbeSorterListToPMA(pTask, &pTask->list);
  pTask->bDone = 1;
  أرجع SQLITE_INT_TO_PTR(rc);
}
#endif /* SQLITE_MAX_WORKER_THREADS>0 */

/*
** Flush the current contents of VdbeSorter.list to a new PMA, possibly
** using a background thread.
*/
ساكن صحيح vdbeSorterFlushPMA(VdbeSorter *pSorter){
#إذا SQLITE_MAX_WORKER_THREADS==0
  pSorter->bUsePMA = 1;
  أرجع vdbeSorterListToPMA(&pSorter->aTask[0], &pSorter->list);
#وإلا
  صحيح rc = SQLITE_OK;
  صحيح i;
  SortSubtask *pTask = 0;    /* Thread context used to create new PMA */
  صحيح nWorker = (pSorter->nTask-1);

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
  لكل(i=0; i<nWorker; i++){
    صحيح iTest = (pSorter->iPrev + i + 1) % nWorker;
    pTask = &pSorter->aTask[iTest];
    إذا( pTask->bDone ){
      rc = vdbeSorterJoinThread(pTask);
    }
    إذا( rc!=SQLITE_OK || pTask->pThread==0 ) اكسر;
  }

  إذا( rc==SQLITE_OK ){
    إذا( i==nWorker ){
      /* Use the foreground thread for this operation */
      rc = vdbeSorterListToPMA(&pSorter->aTask[nWorker], &pSorter->list);
    }وإلا{
      /* Launch a background thread for this operation */
      u8 *aMem;
      فراغ *pCtx;

      أكد( pTask!=0 );
      أكد( pTask->pThread==0 && pTask->bDone==0 );
      أكد( pTask->list.pList==0 );
      أكد( pTask->list.aMemory==0 || pSorter->list.aMemory!=0 );

      aMem = pTask->list.aMemory;
      pCtx = (فراغ*)pTask;
      pSorter->iPrev = (u8)(pTask - pSorter->aTask);
      pTask->list = pSorter->list;
      pSorter->list.pList = 0;
      pSorter->list.szPMA = 0;
      إذا( aMem ){
        pSorter->list.aMemory = aMem;
        pSorter->nMemory = sqlite3MallocSize(aMem);
      }وإلا إذا( pSorter->list.aMemory ){
        pSorter->list.aMemory = sqlite3Malloc(pSorter->nMemory);
        إذا( !pSorter->list.aMemory ) أرجع SQLITE_NOMEM_BKPT;
      }

      rc = vdbeSorterCreateThread(pTask, vdbeSorterFlushThread, pCtx);
    }
  }

  أرجع rc;
#endif /* SQLITE_MAX_WORKER_THREADS!=0 */
}

/*
** Add a record to the sorter.
*/
صحيح sqlite3VdbeSorterWrite(
  ثابت VdbeCursor *pCsr,         /* Sorter cursor */
  Mem *pVal                       /* Memory cell containing record */
){
  VdbeSorter *pSorter;
  صحيح rc = SQLITE_OK;             /* Return Code */
  SorterRecord *pNew;             /* New list element */
  صحيح bFlush;                     /* True to flush contents of memory to PMA */
  i64 nReq;                       /* Bytes of memory required */
  i64 nPMA;                       /* Bytes of PMA space required */
  صحيح t;                          /* serial type of first record field */

  أكد( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  getVarint32NR((ثابت u8*)&pVal->z[1], t);
  إذا( t>0 && t<10 ){
    إذا( t==7 ){
      pSorter->typeMask &= SORTER_TYPE_REAL;
    }وإلا{
      pSorter->typeMask &= (SORTER_TYPE_INTEGER|SORTER_TYPE_REAL);
    }
  }وإلا إذا( t>10 && (t & 0x01) ){
    pSorter->typeMask &= SORTER_TYPE_TEXT;
  }وإلا{
    pSorter->typeMask = 0;
  }

  أكد( pSorter );

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
  nReq = pVal->n + حجم(SorterRecord);
  nPMA = pVal->n + sqlite3VarintLen(pVal->n);
  إذا( pSorter->mxPmaSize ){
    إذا( pSorter->list.aMemory ){
      bFlush = pSorter->iMemory && (pSorter->iMemory+nReq) > pSorter->mxPmaSize;
    }وإلا{
      bFlush = (
          (pSorter->list.szPMA > pSorter->mxPmaSize)
       || (pSorter->list.szPMA > pSorter->mnPmaSize && sqlite3HeapNearlyFull())
      );
    }
    إذا( bFlush ){
      rc = vdbeSorterFlushPMA(pSorter);
      pSorter->list.szPMA = 0;
      pSorter->iMemory = 0;
      أكد( rc!=SQLITE_OK || pSorter->list.pList==0 );
    }
  }

  pSorter->list.szPMA += nPMA;
  إذا( nPMA>pSorter->mxKeysize ){
    pSorter->mxKeysize = nPMA;
  }

  إذا( pSorter->list.aMemory ){
    صحيح nMin = pSorter->iMemory + nReq;

    إذا( nMin>pSorter->nMemory ){
      u8 *aNew;
      sqlite3_int64 nNew = 2 * (sqlite3_int64)pSorter->nMemory;
      صحيح iListOff = -1;
      إذا( pSorter->list.pList ){
        iListOff = (u8*)pSorter->list.pList - pSorter->list.aMemory;
      }
      طالما( nNew < nMin ) nNew = nNew*2;
      إذا( nNew > pSorter->mxPmaSize ) nNew = pSorter->mxPmaSize;
      إذا( nNew < nMin ) nNew = nMin;
      aNew = sqlite3Realloc(pSorter->list.aMemory, nNew);
      إذا( !aNew ) أرجع SQLITE_NOMEM_BKPT;
      إذا( iListOff>=0 ){
        pSorter->list.pList = (SorterRecord*)&aNew[iListOff];
      }
      pSorter->list.aMemory = aNew;
      pSorter->nMemory = nNew;
    }

    pNew = (SorterRecord*)&pSorter->list.aMemory[pSorter->iMemory];
    pSorter->iMemory += ROUND8(nReq);
    إذا( pSorter->list.pList ){
      pNew->u.iNext = (صحيح)((u8*)(pSorter->list.pList) - pSorter->list.aMemory);
    }
  }وإلا{
    pNew = (SorterRecord *)sqlite3Malloc(nReq);
    إذا( pNew==0 ){
      أرجع SQLITE_NOMEM_BKPT;
    }
    pNew->u.pNext = pSorter->list.pList;
  }

  memcpy(SRVAL(pNew), pVal->z, pVal->n);
  pNew->nVal = pVal->n;
  pSorter->list.pList = pNew;

  أرجع rc;
}

/*
** Read keys from pIncr->pMerger and populate pIncr->aFile[1]. The format
** of the data stored in aFile[1] is the same as that used by regular PMAs,
** except that the number-of-bytes varint is omitted from the start.
*/
ساكن صحيح vdbeIncrPopulate(IncrMerger *pIncr){
  صحيح rc = SQLITE_OK;
  صحيح rc2;
  i64 iStart = pIncr->iStartOff;
  SorterFile *pOut = &pIncr->aFile[1];
  SortSubtask *pTask = pIncr->pTask;
  MergeEngine *pMerger = pIncr->pMerger;
  PmaWriter writer;
  أكد( pIncr->bEof==0 );

  vdbeSorterPopulateDebug(pTask, "enter");

  vdbePmaWriterInit(pOut->pFd, &writer, pTask->pSorter->pgsz, iStart);
  طالما( rc==SQLITE_OK ){
    صحيح dummy;
    PmaReader *pReader = &pMerger->aReadr[ pMerger->a_ڤوکوق[1] ];
    صحيح n_کونچي = pReader->n_کونچي;
    i64 iEof = writer.iWriteOff + writer.iBufEnd;

    /* Check if the output file is full or if the input has been exhausted.
    ** In either case exit the loop. */
    إذا( pReader->pFd==0 ) اكسر;
    إذا( (iEof + n_کونچي + sqlite3VarintLen(n_کونچي))>(iStart + pIncr->mxSz) ) اكسر;

    /* Write the next key to the output. */
    vdbePmaWriteVarint(&writer, n_کونچي);
    vdbePmaWriteBlob(&writer, pReader->a_کونچي, n_کونچي);
    أكد( pIncr->pMerger->pTask==pTask );
    rc = vdbeMergeEngineStep(pIncr->pMerger, &dummy);
  }

  rc2 = vdbePmaWriterFinish(&writer, &pOut->iEof, &pTask->nSpill);
  إذا( rc==SQLITE_OK ) rc = rc2;
  vdbeSorterPopulateDebug(pTask, "exit");
  أرجع rc;
}

#إذا SQLITE_MAX_WORKER_THREADS>0
/*
** The main routine for background threads that populate aFile[1] of
** multi-threaded IncrMerger objects.
*/
ساكن فراغ *vdbeIncrPopulateThread(فراغ *pCtx){
  IncrMerger *pIncr = (IncrMerger*)pCtx;
  فراغ *pRet = SQLITE_INT_TO_PTR( vdbeIncrPopulate(pIncr) );
  pIncr->pTask->bDone = 1;
  أرجع pRet;
}

/*
** Launch a background thread to populate aFile[1] of pIncr.
*/
ساكن صحيح vdbeIncrBgPopulate(IncrMerger *pIncr){
  فراغ *p = (فراغ*)pIncr;
  أكد( pIncr->bUseThread );
  أرجع vdbeSorterCreateThread(pIncr->pTask, vdbeIncrPopulateThread, p);
}
#endif

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
ساكن صحيح vdbeIncrSwap(IncrMerger *pIncr){
  صحيح rc = SQLITE_OK;

#إذا SQLITE_MAX_WORKER_THREADS>0
  إذا( pIncr->bUseThread ){
    rc = vdbeSorterJoinThread(pIncr->pTask);

    إذا( rc==SQLITE_OK ){
      SorterFile f0 = pIncr->aFile[0];
      pIncr->aFile[0] = pIncr->aFile[1];
      pIncr->aFile[1] = f0;
    }

    إذا( rc==SQLITE_OK ){
      إذا( pIncr->aFile[0].iEof==pIncr->iStartOff ){
        pIncr->bEof = 1;
      }وإلا{
        rc = vdbeIncrBgPopulate(pIncr);
      }
    }
  }وإلا
#endif
  {
    rc = vdbeIncrPopulate(pIncr);
    pIncr->aFile[0] = pIncr->aFile[1];
    إذا( pIncr->aFile[0].iEof==pIncr->iStartOff ){
      pIncr->bEof = 1;
    }
  }

  أرجع rc;
}

/*
** Allocate and return a new IncrMerger object to read data from pMerger.
**
** If an OOM condition is encountered, return NULL. In this case free the
** pMerger argument before returning.
*/
ساكن صحيح vdbeIncrMergerNew(
  SortSubtask *pTask,     /* The thread that will be using the new IncrMerger */
  MergeEngine *pMerger,   /* The MergeEngine that the IncrMerger will control */
  IncrMerger **ppOut      /* Write the new IncrMerger here */
){
  صحيح rc = SQLITE_OK;
  IncrMerger *pIncr = *ppOut = (IncrMerger*)
       (sqlite3FaultSim(100) ? 0 : sqlite3MallocZero(حجم(*pIncr)));
  إذا( pIncr ){
    pIncr->pMerger = pMerger;
    pIncr->pTask = pTask;
    pIncr->mxSz = MAX(pTask->pSorter->mxKeysize+9,pTask->pSorter->mxPmaSize/2);
    pTask->file2.iEof += pIncr->mxSz;
  }وإلا{
    vdbeMergeEngineFree(pMerger);
    rc = SQLITE_NOMEM_BKPT;
  }
  أكد( *ppOut!=0 || rc!=SQLITE_OK );
  أرجع rc;
}

#إذا SQLITE_MAX_WORKER_THREADS>0
/*
** Set the "use-threads" flag on object pIncr.
*/
ساكن فراغ vdbeIncrMergerSetThreads(IncrMerger *pIncr){
  pIncr->bUseThread = 1;
  pIncr->pTask->file2.iEof -= pIncr->mxSz;
}
#endif /* SQLITE_MAX_WORKER_THREADS>0 */



/*
** Recompute pMerger->aTree[iOut] by comparing the next keys on the
** two PmaReaders that feed that entry.  Neither of the PmaReaders
** are advanced.  This routine merely does the comparison.
*/
ساكن فراغ vdbeMergeEngineCompare(
  MergeEngine *pMerger,  /* Merge engine containing PmaReaders to compare */
  صحيح iOut               /* Store the result in pMerger->aTree[iOut] */
){
  صحيح i1;
  صحيح i2;
  صحيح iRes;
  PmaReader *p1;
  PmaReader *p2;

  أكد( iOut<pMerger->n_ڤوکوق && iOut>0 );

  إذا( iOut>=(pMerger->n_ڤوکوق/2) ){
    i1 = (iOut - pMerger->n_ڤوکوق/2) * 2;
    i2 = i1 + 1;
  }وإلا{
    i1 = pMerger->a_ڤوکوق[iOut*2];
    i2 = pMerger->a_ڤوکوق[iOut*2+1];
  }

  p1 = &pMerger->aReadr[i1];
  p2 = &pMerger->aReadr[i2];

  إذا( p1->pFd==0 ){
    iRes = i2;
  }وإلا إذا( p2->pFd==0 ){
    iRes = i1;
  }وإلا{
    SortSubtask *pTask = pMerger->pTask;
    صحيح bCached = 0;
    صحيح res;
    أكد( pTask->pUnpacked!=0 );  /* from vdbeSortSubtaskMain() */
    res = pTask->xCompare(
        pTask, &bCached, p1->a_کونچي, p1->n_کونچي, p2->a_کونچي, p2->n_کونچي
    );
    إذا( res<=0 ){
      iRes = i1;
    }وإلا{
      iRes = i2;
    }
  }

  pMerger->a_ڤوکوق[iOut] = iRes;
}

/*
** Allowed values for the eMode parameter to vdbeMergeEngineInit()
** and vdbePmaReaderIncrMergeInit().
**
** Only INCRINIT_NORMAL is valid in single-threaded builds (when
** SQLITE_MAX_WORKER_THREADS==0).  The other values are only used
** when there exists one or more separate worker threads.
*/
#define INCRINIT_NORMAL 0
#define INCRINIT_TASK   1
#define INCRINIT_اکر   2

/*
** Forward reference required as the vdbeIncrMergeInit() and
** vdbePmaReaderIncrInit() routines are called mutually recursively when
** building a merge tree.
*/
ساكن صحيح vdbePmaReaderIncrInit(PmaReader *pReadr, صحيح eMode);

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
ساكن صحيح vdbeMergeEngineInit(
  SortSubtask *pTask,             /* Thread that will run pMerger */
  MergeEngine *pMerger,           /* MergeEngine to initialize */
  صحيح eMode                       /* One of the INCRINIT_XXX constants */
){
  صحيح rc = SQLITE_OK;             /* Return code */
  صحيح i;                          /* For looping over PmaReader objects */
  صحيح n_ڤوکوق;                      /* Number of subtrees to merge */

  /* Failure to allocate the merge would have been detected prior to
  ** invoking this routine */
  أكد( pMerger!=0 );

  /* eMode is always INCRINIT_NORMAL in single-threaded mode */
  أكد( SQLITE_MAX_WORKER_THREADS>0 || eMode==INCRINIT_NORMAL );

  /* Verify that the MergeEngine is assigned to a single thread */
  أكد( pMerger->pTask==0 );
  pMerger->pTask = pTask;

  n_ڤوکوق = pMerger->n_ڤوکوق;
  لكل(i=0; i<n_ڤوکوق; i++){
    إذا( SQLITE_MAX_WORKER_THREADS>0 && eMode==INCRINIT_اکر ){
      /* PmaReaders should be normally initialized in order, as if they are
      ** reading from the same temp file this makes for more linear file IO.
      ** However, in the INCRINIT_ROOT case, if PmaReader aReadr[nTask-1] is
      ** in use it will block the vdbePmaReaderNext() call while it uses
      ** the main thread to fill its buffer. So calling PmaReaderNext()
      ** on this PmaReader before any of the multi-threaded PmaReaders takes
      ** better advantage of multi-processor hardware. */
      rc = vdbePmaReaderNext(&pMerger->aReadr[n_ڤوکوق-i-1]);
    }وإلا{
      rc = vdbePmaReaderIncrInit(&pMerger->aReadr[i], INCRINIT_NORMAL);
    }
    إذا( rc!=SQLITE_OK ) أرجع rc;
  }

  لكل(i=pMerger->n_ڤوکوق-1; i>0; i--){
    vdbeMergeEngineCompare(pMerger, i);
  }
  أرجع pTask->pUnpacked->errCode;
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
ساكن صحيح vdbePmaReaderIncrMergeInit(PmaReader *pReadr, صحيح eMode){
  صحيح rc = SQLITE_OK;
  IncrMerger *pIncr = pReadr->pIncr;
  SortSubtask *pTask = pIncr->pTask;
  sqlite3 *db = pTask->pSorter->db;

  /* eMode is always INCRINIT_NORMAL in single-threaded mode */
  أكد( SQLITE_MAX_WORKER_THREADS>0 || eMode==INCRINIT_NORMAL );

  rc = vdbeMergeEngineInit(pTask, pIncr->pMerger, eMode);

  /* Set up the required files for pIncr. A multi-threaded IncrMerge object
  ** requires two temp files to itself, whereas a single-threaded object
  ** only requires a region of pTask->file2. */
  إذا( rc==SQLITE_OK ){
    صحيح mxSz = pIncr->mxSz;
#إذا SQLITE_MAX_WORKER_THREADS>0
    إذا( pIncr->bUseThread ){
      rc = vdbeSorterOpenTempFile(db, mxSz, &pIncr->aFile[0].pFd);
      إذا( rc==SQLITE_OK ){
        rc = vdbeSorterOpenTempFile(db, mxSz, &pIncr->aFile[1].pFd);
      }
    }وإلا
#endif
    /*if( !pIncr->bUseThread )*/{
      إذا( pTask->file2.pFd==0 ){
        أكد( pTask->file2.iEof>0 );
        rc = vdbeSorterOpenTempFile(db, pTask->file2.iEof, &pTask->file2.pFd);
        pTask->file2.iEof = 0;
      }
      إذا( rc==SQLITE_OK ){
        pIncr->aFile[1].pFd = pTask->file2.pFd;
        pIncr->iStartOff = pTask->file2.iEof;
        pTask->file2.iEof += mxSz;
      }
    }
  }

#إذا SQLITE_MAX_WORKER_THREADS>0
  إذا( rc==SQLITE_OK && pIncr->bUseThread ){
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
    أكد( eMode==INCRINIT_اکر || eMode==INCRINIT_TASK );
    rc = vdbeIncrPopulate(pIncr);
  }
#endif

  إذا( rc==SQLITE_OK && (SQLITE_MAX_WORKER_THREADS==0 || eMode!=INCRINIT_TASK) ){
    rc = vdbePmaReaderNext(pReadr);
  }

  أرجع rc;
}

#إذا SQLITE_MAX_WORKER_THREADS>0
/*
** The main routine for vdbePmaReaderIncrMergeInit() operations run in
** background threads.
*/
ساكن فراغ *vdbePmaReaderBgIncrInit(فراغ *pCtx){
  PmaReader *pReader = (PmaReader*)pCtx;
  فراغ *pRet = SQLITE_INT_TO_PTR(
                  vdbePmaReaderIncrMergeInit(pReader,INCRINIT_TASK)
               );
  pReader->pIncr->pTask->bDone = 1;
  أرجع pRet;
}
#endif

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
ساكن صحيح vdbePmaReaderIncrInit(PmaReader *pReadr, صحيح eMode){
  IncrMerger *pIncr = pReadr->pIncr;   /* Incremental merger */
  صحيح rc = SQLITE_OK;                  /* Return code */
  إذا( pIncr ){
#إذا SQLITE_MAX_WORKER_THREADS>0
    أكد( pIncr->bUseThread==0 || eMode==INCRINIT_TASK );
    إذا( pIncr->bUseThread ){
      فراغ *pCtx = (فراغ*)pReadr;
      rc = vdbeSorterCreateThread(pIncr->pTask, vdbePmaReaderBgIncrInit, pCtx);
    }وإلا
#endif
    {
      rc = vdbePmaReaderIncrMergeInit(pReadr, eMode);
    }
  }
  أرجع rc;
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
ساكن صحيح vdbeMergeEngineLevel0(
  SortSubtask *pTask,             /* Sorter task to read from */
  صحيح nPMA,                       /* Number of PMAs to read */
  i64 *piOffset,                  /* IN/OUT: Readr offset in pTask->file */
  MergeEngine **ppOut             /* OUT: New merge-engine */
){
  MergeEngine *pNew;              /* Merge engine to return */
  i64 iOff = *piOffset;
  صحيح i;
  صحيح rc = SQLITE_OK;

  *ppOut = pNew = vdbeMergeEngineNew(nPMA);
  إذا( pNew==0 ) rc = SQLITE_NOMEM_BKPT;

  لكل(i=0; i<nPMA && rc==SQLITE_OK; i++){
    i64 nDummy = 0;
    PmaReader *pReadr = &pNew->aReadr[i];
    rc = vdbePmaReaderInit(pTask, &pTask->file, iOff, pReadr, &nDummy);
    iOff = pReadr->iEof;
  }

  إذا( rc!=SQLITE_OK ){
    vdbeMergeEngineFree(pNew);
    *ppOut = 0;
  }
  *piOffset = iOff;
  أرجع rc;
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
ساكن صحيح vdbe_Sorter_ڤوکوق_Depth(صحيح nPMA){
  صحيح nDepth = 0;
  i64 nDiv = SORTER_MAX_MERGE_کيراءن;
  طالما( nDiv < (i64)nPMA ){
    nDiv = nDiv * SORTER_MAX_MERGE_کيراءن;
    nDepth++;
  }
  أرجع nDepth;
}

/*
** pRoot is the root of an incremental merge-tree with depth nDepth (according
** to vdbeSorterTreeDepth()). pLeaf is the iSeq'th leaf to be added to the
** tree, counting from zero. This function adds pLeaf to the tree.
**
** If successful, SQLITE_OK is returned. If an error occurs, an SQLite error
** code is returned and pLeaf is freed.
*/
ساكن صحيح vdbe_Sorter_Add_To_ڤوکوق(
  SortSubtask *pTask,             /* Task context */
  صحيح nDepth,                     /* Depth of tree according to TreeDepth() */
  صحيح iSeq,                       /* Sequence number of leaf within tree */
  MergeEngine *p_اکر,             /* Root of tree */
  MergeEngine *pLeaf              /* Leaf to add to tree */
){
  صحيح rc = SQLITE_OK;
  صحيح nDiv = 1;
  صحيح i;
  MergeEngine *p = p_اکر;
  IncrMerger *pIncr;

  rc = vdbeIncrMergerNew(pTask, pLeaf, &pIncr);

  لكل(i=1; i<nDepth; i++){
    nDiv = nDiv * SORTER_MAX_MERGE_کيراءن;
  }

  لكل(i=1; i<nDepth && rc==SQLITE_OK; i++){
    صحيح iIter = (iSeq / nDiv) % SORTER_MAX_MERGE_کيراءن;
    PmaReader *pReadr = &p->aReadr[iIter];

    إذا( pReadr->pIncr==0 ){
      MergeEngine *pNew = vdbeMergeEngineNew(SORTER_MAX_MERGE_کيراءن);
      إذا( pNew==0 ){
        rc = SQLITE_NOMEM_BKPT;
      }وإلا{
        rc = vdbeIncrMergerNew(pTask, pNew, &pReadr->pIncr);
      }
    }
    إذا( rc==SQLITE_OK ){
      p = pReadr->pIncr->pMerger;
      nDiv = nDiv / SORTER_MAX_MERGE_کيراءن;
    }
  }

  إذا( rc==SQLITE_OK ){
    p->aReadr[iSeq % SORTER_MAX_MERGE_کيراءن].pIncr = pIncr;
  }وإلا{
    vdbeIncrFree(pIncr);
  }
  أرجع rc;
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
ساكن صحيح vdbe_Sorter_Merge_ڤوکوق_Build(
  VdbeSorter *pSorter,       /* The VDBE cursor that implements the sort */
  MergeEngine **ppOut        /* Write the MergeEngine here */
){
  MergeEngine *pMain = 0;
  صحيح rc = SQLITE_OK;
  صحيح iTask;

#إذا SQLITE_MAX_WORKER_THREADS>0
  /* If the sorter uses more than one task, then create the top-level
  ** MergeEngine here. This MergeEngine will read data from exactly
  ** one PmaReader per sub-task.  */
  أكد( pSorter->bUseThreads || pSorter->nTask==1 );
  إذا( pSorter->nTask>1 ){
    pMain = vdbeMergeEngineNew(pSorter->nTask);
    إذا( pMain==0 ) rc = SQLITE_NOMEM_BKPT;
  }
#endif

  لكل(iTask=0; rc==SQLITE_OK && iTask<pSorter->nTask; iTask++){
    SortSubtask *pTask = &pSorter->aTask[iTask];
    أكد( pTask->nPMA>0 || SQLITE_MAX_WORKER_THREADS>0 );
    إذا( SQLITE_MAX_WORKER_THREADS==0 || pTask->nPMA ){
      MergeEngine *p_اکر = 0;     /* Root node of tree for this task */
      صحيح nDepth = vdbe_Sorter_ڤوکوق_Depth(pTask->nPMA);
      i64 iReadOff = 0;

      إذا( pTask->nPMA<=SORTER_MAX_MERGE_کيراءن ){
        rc = vdbeMergeEngineLevel0(pTask, pTask->nPMA, &iReadOff, &p_اکر);
      }وإلا{
        صحيح i;
        صحيح iSeq = 0;
        p_اکر = vdbeMergeEngineNew(SORTER_MAX_MERGE_کيراءن);
        إذا( p_اکر==0 ) rc = SQLITE_NOMEM_BKPT;
        لكل(i=0; i<pTask->nPMA && rc==SQLITE_OK; i += SORTER_MAX_MERGE_کيراءن){
          MergeEngine *pMerger = 0; /* New level-0 PMA merger */
          صحيح nReader;              /* Number of level-0 PMAs to merge */

          nReader = MIN(pTask->nPMA - i, SORTER_MAX_MERGE_کيراءن);
          rc = vdbeMergeEngineLevel0(pTask, nReader, &iReadOff, &pMerger);
          إذا( rc==SQLITE_OK ){
            rc = vdbe_Sorter_Add_To_ڤوکوق(pTask, nDepth, iSeq++, p_اکر, pMerger);
          }
        }
      }

      إذا( rc==SQLITE_OK ){
#إذا SQLITE_MAX_WORKER_THREADS>0
        إذا( pMain!=0 ){
          rc = vdbeIncrMergerNew(pTask, p_اکر, &pMain->aReadr[iTask].pIncr);
        }وإلا
#endif
        {
          أكد( pMain==0 );
          pMain = p_اکر;
        }
      }وإلا{
        vdbeMergeEngineFree(p_اکر);
      }
    }
  }

  إذا( rc!=SQLITE_OK ){
    vdbeMergeEngineFree(pMain);
    pMain = 0;
  }
  *ppOut = pMain;
  أرجع rc;
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
ساكن صحيح vdbeSorterSetupMerge(VdbeSorter *pSorter){
  صحيح rc;                         /* Return code */
  SortSubtask *pTask0 = &pSorter->aTask[0];
  MergeEngine *pMain = 0;
#إذا SQLITE_MAX_WORKER_THREADS
  sqlite3 *db = pTask0->pSorter->db;
  صحيح i;
  SorterCompare xCompare = vdbeSorterGetCompare(pSorter);
  لكل(i=0; i<pSorter->nTask; i++){
    pSorter->aTask[i].xCompare = xCompare;
  }
#endif

  rc = vdbe_Sorter_Merge_ڤوکوق_Build(pSorter, &pMain);
  إذا( rc==SQLITE_OK ){
#إذا SQLITE_MAX_WORKER_THREADS
    أكد( pSorter->bUseThreads==0 || pSorter->nTask>1 );
    إذا( pSorter->bUseThreads ){
      صحيح iTask;
      PmaReader *pReadr = 0;
      SortSubtask *pLast = &pSorter->aTask[pSorter->nTask-1];
      rc = vdbeSortAllocUnpacked(pLast);
      إذا( rc==SQLITE_OK ){
        pReadr = (PmaReader*)sqlite3DbMallocZero(db, حجم(PmaReader));
        pSorter->pReader = pReadr;
        إذا( pReadr==0 ) rc = SQLITE_NOMEM_BKPT;
      }
      إذا( rc==SQLITE_OK ){
        rc = vdbeIncrMergerNew(pLast, pMain, &pReadr->pIncr);
        إذا( rc==SQLITE_OK ){
          vdbeIncrMergerSetThreads(pReadr->pIncr);
          لكل(iTask=0; iTask<(pSorter->nTask-1); iTask++){
            IncrMerger *pIncr;
            إذا( (pIncr = pMain->aReadr[iTask].pIncr) ){
              vdbeIncrMergerSetThreads(pIncr);
              أكد( pIncr->pTask!=pLast );
            }
          }
          لكل(iTask=0; rc==SQLITE_OK && iTask<pSorter->nTask; iTask++){
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
            أكد( p->pIncr==0 || (
                (p->pIncr->pTask==&pSorter->aTask[iTask])             /* a */
             && (iTask!=pSorter->nTask-1 || p->pIncr->bUseThread==0)  /* b */
            ));
            rc = vdbePmaReaderIncrInit(p, INCRINIT_TASK);
          }
        }
        pMain = 0;
      }
      إذا( rc==SQLITE_OK ){
        rc = vdbePmaReaderIncrMergeInit(pReadr, INCRINIT_اکر);
      }
    }وإلا
#endif
    {
      rc = vdbeMergeEngineInit(pTask0, pMain, INCRINIT_NORMAL);
      pSorter->pMerger = pMain;
      pMain = 0;
    }
  }

  إذا( rc!=SQLITE_OK ){
    vdbeMergeEngineFree(pMain);
  }
  أرجع rc;
}


/*
** Once the sorter has been populated by calls to sqlite3VdbeSorterWrite,
** this function is called to prepare for iterating through the records
** in sorted order.
*/
صحيح sqlite3VdbeSorterRewind(ثابت VdbeCursor *pCsr, صحيح *pbEof){
  VdbeSorter *pSorter;
  صحيح rc = SQLITE_OK;             /* Return code */

  أكد( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  أكد( pSorter );

  /* If no data has been written to disk, then do not do so now. Instead,
  ** sort the VdbeSorter.pRecord list. The vdbe layer will read data directly
  ** from the in-memory list.  */
  إذا( pSorter->bUsePMA==0 ){
    إذا( pSorter->list.pList ){
      *pbEof = 0;
      rc = vdbeSorterSort(&pSorter->aTask[0], &pSorter->list);
    }وإلا{
      *pbEof = 1;
    }
    أرجع rc;
  }

  /* Write the current in-memory list to a PMA. When the VdbeSorterWrite()
  ** function flushes the contents of memory to disk, it immediately always
  ** creates a new list consisting of a single key immediately afterwards.
  ** So the list is never empty at this point.  */
  أكد( pSorter->list.pList );
  rc = vdbeSorterFlushPMA(pSorter);

  /* Join all threads */
  rc = vdbeSorterJoinAll(pSorter, rc);

  vdbeSorterRewindDebug("rewind");

  /* Assuming no errors have occurred, set up a merger structure to
  ** incrementally read and merge all remaining PMAs.  */
  أكد( pSorter->pReader==0 );
  إذا( rc==SQLITE_OK ){
    rc = vdbeSorterSetupMerge(pSorter);
    *pbEof = 0;
  }

  vdbeSorterRewindDebug("rewinddone");
  أرجع rc;
}

/*
** Advance to the next element in the sorter.  Return value:
**
**    SQLITE_OK     success
**    SQLITE_DONE   end of data
**    otherwise     some kind of error.
*/
صحيح sqlite3VdbeSorterNext(sqlite3 *db, ثابت VdbeCursor *pCsr){
  VdbeSorter *pSorter;
  صحيح rc;                         /* Return code */

  أكد( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  أكد( pSorter->bUsePMA || (pSorter->pReader==0 && pSorter->pMerger==0) );
  إذا( pSorter->bUsePMA ){
    أكد( pSorter->pReader==0 || pSorter->pMerger==0 );
    أكد( pSorter->bUseThreads==0 || pSorter->pReader );
    أكد( pSorter->bUseThreads==1 || pSorter->pMerger );
#إذا SQLITE_MAX_WORKER_THREADS>0
    إذا( pSorter->bUseThreads ){
      rc = vdbePmaReaderNext(pSorter->pReader);
      إذا( rc==SQLITE_OK && pSorter->pReader->pFd==0 ) rc = SQLITE_DONE;
    }وإلا
#endif
    /*if( !pSorter->bUseThreads )*/ {
      صحيح res = 0;
      أكد( pSorter->pMerger!=0 );
      أكد( pSorter->pMerger->pTask==(&pSorter->aTask[0]) );
      rc = vdbeMergeEngineStep(pSorter->pMerger, &res);
      إذا( rc==SQLITE_OK && res ) rc = SQLITE_DONE;
    }
  }وإلا{
    SorterRecord *pFree = pSorter->list.pList;
    pSorter->list.pList = pFree->u.pNext;
    pFree->u.pNext = 0;
    إذا( pSorter->list.aMemory==0 ) vdbeSorterRecordFree(db, pFree);
    rc = pSorter->list.pList ? SQLITE_OK : SQLITE_DONE;
  }
  أرجع rc;
}

/*
** Return a pointer to a buffer owned by the sorter that contains the
** current key.
*/
ساكن فراغ *vdbeSorterRowkey(
  ثابت VdbeSorter *pSorter,      /* Sorter object */
  صحيح *pn_کونچي                      /* OUT: Size of current key in bytes */
){
  فراغ *p_کونچي;
  إذا( pSorter->bUsePMA ){
    PmaReader *pReader;
#إذا SQLITE_MAX_WORKER_THREADS>0
    إذا( pSorter->bUseThreads ){
      pReader = pSorter->pReader;
    }وإلا
#endif
    /*if( !pSorter->bUseThreads )*/{
      pReader = &pSorter->pMerger->aReadr[pSorter->pMerger->a_ڤوکوق[1]];
    }
    *pn_کونچي = pReader->n_کونچي;
    p_کونچي = pReader->a_کونچي;
  }وإلا{
    *pn_کونچي = pSorter->list.pList->nVal;
    p_کونچي = SRVAL(pSorter->list.pList);
  }
  أرجع p_کونچي;
}

/*
** Copy the current sorter key into the memory cell pOut.
*/
صحيح sqlite3VdbeSorterRowkey(ثابت VdbeCursor *pCsr, Mem *pOut){
  VdbeSorter *pSorter;
  فراغ *p_کونچي; صحيح n_کونچي;           /* Sorter key to copy into pOut */

  أكد( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  p_کونچي = vdbeSorterRowkey(pSorter, &n_کونچي);
  إذا( sqlite3VdbeMemClearAndResize(pOut, n_کونچي) ){
    أرجع SQLITE_NOMEM_BKPT;
  }
  pOut->n = n_کونچي;
  MemSetTypeFlag(pOut, MEM_Blob);
  memcpy(pOut->z, p_کونچي, n_کونچي);

  أرجع SQLITE_OK;
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
صحيح sqlite3VdbeSorterCompare(
  ثابت VdbeCursor *pCsr,         /* Sorter cursor */
  Mem *pVal,                      /* Value to compare to current sorter key */
  صحيح n_کونچي_Col,                    /* Compare this many columns */
  صحيح *pRes                       /* OUT: Result of comparison */
){
  VdbeSorter *pSorter;
  UnpackedRecord *r2;
  کونچي_Info *p_کونچي_Info;
  صحيح i;
  فراغ *p_کونچي; صحيح n_کونچي;           /* Sorter key to compare pVal with */

  أكد( pCsr->eCurType==CURTYPE_SORTER );
  pSorter = pCsr->uc.pSorter;
  r2 = pSorter->pUnpacked;
  p_کونچي_Info = pCsr->p_کونچي_Info;
  إذا( r2==0 ){
    r2 = pSorter->pUnpacked = sqlite3VdbeAllocUnpackedRecord(p_کونچي_Info);
    إذا( r2==0 ) أرجع SQLITE_NOMEM_BKPT;
    r2->nField = n_کونچي_Col;
  }
  أكد( r2->nField==n_کونچي_Col );

  p_کونچي = vdbeSorterRowkey(pSorter, &n_کونچي);
  sqlite3VdbeRecordUnpack(n_کونچي, p_کونچي, r2);
  لكل(i=0; i<n_کونچي_Col; i++){
    إذا( r2->aMem[i].flags & MEM_Null ){
      *pRes = -1;
      أرجع SQLITE_OK;
    }
  }

  *pRes = sqlite3VdbeRecordCompare(pVal->n, pVal->z, r2);
  أرجع SQLITE_OK;
}
