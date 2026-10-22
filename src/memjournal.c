/*
** 2008 October 7
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
** This file contains code use to implement an in-memory rollback journal.
** The in-memory rollback journal is used to journal transactions for
** ":memory:" databases and when the journal_mode=MEMORY pragma is used.
**
** Update:  The in-memory journal is also used to temporarily cache
** smaller journals that are not critical for power-loss recovery.
** For example, statement journals that are not too big will be held
** entirely in memory, thus reducing the number of file I/O calls, and
** more importantly, reducing temporary file creation events.  If these
** journals become too large for memory, they are spilled to disk.  But
** in the common case, they are usually small and no file I/O needs to
** occur.
*/
#include "sqliteInt.h"

/* Forward references to internal structures */
typedef sa_nracanaa MemJournal MemJournal;
typedef sa_nracanaa FilePoint FilePoint;
typedef sa_nracanaa FileChunk FileChunk;

/*
** The rollback journal is composed of a linked list of these structures.
**
** The zChunk array is always at least 8 bytes in size - usually much more.
** Its actual size is stored in the MemJournal.nChunkSize variable.
*/
sa_nracanaa FileChunk {
  FileChunk *pNext;               /* Next chunk in the journal */
  u8 zChunk[8];                   /* Content of this chunk */
};

/*
** By default, allocate this many bytes of memory for each FileChunk object.
*/
#define MEMJOURNAL_DFLT_FILECHUNKSIZE 1024

/*
** For chunk size nChunkSize, return the number of bytes that should
** be allocated for each FileChunk structure.
*/
#define fileChunkSize(nChunkSize) (parimaa_na(FileChunk) + ((nChunkSize)-8))

/*
** An instance of this object serves as a cursor into the rollback journal.
** The cursor can be either for reading or writing.
*/
sa_nracanaa FilePoint {
  sqlite3_int64 iOffset;          /* Offset from the beginning of the file */
  FileChunk *pChunk;              /* Specific chunk into which cursor points */
};

/*
** This structure is a subclass of sqlite3_file. Each open memory-journal
** is an instance of this class.
*/
sa_nracanaa MemJournal {
  const sqlite3_io_methods *pMethod; /* Parent class. MUST BE FIRST */
  ကိန်းပြည့် nChunkSize;                 /* In-memory chunk-size */

  ကိန်းပြည့် nSpill;                     /* Bytes of data before flushing */
  FileChunk *pFirst;              /* Head of in-memory chunk-list */
  FilePoint endpoint;             /* Pointer to the end of the file */
  FilePoint readpoint;            /* Pointer to the end of the last xRead() */

  ကိန်းပြည့် flags;                      /* xOpen flags */
  sqlite3_vfs *pVfs;              /* The "real" underlying VFS */
  const var_na *zJournal;           /* Name of the journal file */
};

/*
** Read data from the in-memory journal file.  This is the implementation
** of the sqlite3_vfs.xRead method.
*/
static ကိန်းပြည့် memjrnlRead(
  sqlite3_file *pJfd,    /* The journal file from which to read */
  void *zBuf,            /* Put the results here */
  ကိန်းပြည့် iAmt,              /* Number of bytes to read */
  sqlite_int64 iOfst     /* Begin reading at this offset */
){
  MemJournal *p = (MemJournal *)pJfd;
  u8 *zOut = zBuf;
  ကိန်းပြည့် nRead = iAmt;
  ကိန်းပြည့် iChunkOffset;
  FileChunk *pChunk;

  အကယ်၍( (iAmt+iOfst)>p->endpoint.iOffset ){
    ပြန်ပေး SQLITE_IOERR_SHORT_READ;
  }
  assert( p->readpoint.iOffset==0 || p->readpoint.pChunk!=0 );
  အကယ်၍( p->readpoint.iOffset!=iOfst || iOfst==0 ){
    sqlite3_int64 iOff = 0;
    အတွက်(pChunk=p->pFirst; 
        ALWAYS(pChunk) && (iOff+p->nChunkSize)<=iOfst;
        pChunk=pChunk->pNext
    ){
      iOff += p->nChunkSize;
    }
  }မဟုတ်ပါက{
    pChunk = p->readpoint.pChunk;
    assert( pChunk!=0 );
  }

  iChunkOffset = (ကိန်းပြည့်)(iOfst%p->nChunkSize);
  do {
    ကိန်းပြည့် iSpace = p->nChunkSize - iChunkOffset;
    ကိန်းပြည့် nCopy = MIN(nRead, (p->nChunkSize - iChunkOffset));
    memcpy(zOut, (u8*)pChunk->zChunk + iChunkOffset, nCopy);
    zOut += nCopy;
    nRead -= iSpace;
    iChunkOffset = 0;
  } နေစဉ်( nRead>=0 && (pChunk=pChunk->pNext)!=0 && nRead>0 );
  p->readpoint.iOffset = pChunk ? iOfst+iAmt : 0;
  p->readpoint.pChunk = pChunk;

  ပြန်ပေး SQLITE_OK;
}

/*
** Free the list of FileChunk structures headed at MemJournal.pFirst.
*/
static void memjrnlFreeChunks(FileChunk *pFirst){
  FileChunk *pIter;
  FileChunk *pNext;
  အတွက်(pIter=pFirst; pIter; pIter=pNext){
    pNext = pIter->pNext;
    sqlite3_free(pIter);
  } 
}

/*
** Flush the contents of memory to a real file on disk.
*/
static ကိန်းပြည့် memjrnlCreateFile(MemJournal *p){
  ကိန်းပြည့် rc;
  sqlite3_file *pReal = (sqlite3_file*)p;
  MemJournal copy = *p;

  memset(p, 0, parimaa_na(MemJournal));
  rc = sqlite3OsOpen(copy.pVfs, copy.zJournal, pReal, copy.flags, 0);
  အကယ်၍( rc==SQLITE_OK ){
    ကိန်းပြည့် nChunk = copy.nChunkSize;
    i64 iOff = 0;
    FileChunk *pIter;
    အတွက်(pIter=copy.pFirst; pIter; pIter=pIter->pNext){
      အကယ်၍( iOff + nChunk > copy.endpoint.iOffset ){
        nChunk = copy.endpoint.iOffset - iOff;
      }
      rc = sqlite3OsWrite(pReal, (u8*)pIter->zChunk, nChunk, iOff);
      အကယ်၍( rc ) bha_nga;
      iOff += nChunk;
    }
    အကယ်၍( rc==SQLITE_OK ){
      /* No error has occurred. Free the in-memory buffers. */
      memjrnlFreeChunks(copy.pFirst);
    }
  }
  အကယ်၍( rc!=SQLITE_OK ){
    /* If an error occurred while creating or writing to the file, restore
    ** the original before returning. This way, SQLite uses the in-memory
    ** journal data to roll back changes made to the internal page-cache
    ** before this function was called.  */
    sqlite3OsClose(pReal);
    *p = copy;
  }
  ပြန်ပေး rc;
}


/* Forward reference */
static ကိန်းပြည့် memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 size);

/*
** Write data to the file.
*/
static ကိန်းပြည့် memjrnlWrite(
  sqlite3_file *pJfd,    /* The journal file into which to write */
  const void *zBuf,      /* Take data to be written from here */
  ကိန်းပြည့် iAmt,              /* Number of bytes to write */
  sqlite_int64 iOfst     /* Begin writing at this offset into the file */
){
  MemJournal *p = (MemJournal *)pJfd;
  ကိန်းပြည့် nWrite = iAmt;
  u8 *zWrite = (u8 *)zBuf;

  /* If the file should be created now, create it and write the new data
  ** into the file on disk. */
  အကယ်၍( p->nSpill>0 && (iAmt+iOfst)>p->nSpill ){
    ကိန်းပြည့် rc = memjrnlCreateFile(p);
    အကယ်၍( rc==SQLITE_OK ){
      rc = sqlite3OsWrite(pJfd, zBuf, iAmt, iOfst);
    }
    ပြန်ပေး rc;
  }

  /* If the contents of this write should be stored in memory */
  မဟုတ်ပါက{
    /* An in-memory journal file should only ever be appended to. Random
    ** access writes are not required. The only exception to this is when
    ** the in-memory journal is being used by a connection using the
    ** atomic-write optimization. In this case the first 28 bytes of the
    ** journal file may be written as part of committing the transaction. */
    assert( iOfst<=p->endpoint.iOffset );
    အကယ်၍( iOfst>0 && iOfst!=p->endpoint.iOffset ){
      memjrnlTruncate(pJfd, iOfst);
    }
    အကယ်၍( iOfst==0 && p->pFirst ){
      assert( p->nChunkSize>iAmt );
      memcpy((u8*)p->pFirst->zChunk, zBuf, iAmt);
    }မဟုတ်ပါက{
      နေစဉ်( nWrite>0 ){
        FileChunk *pChunk = p->endpoint.pChunk;
        ကိန်းပြည့် iChunkOffset = (ကိန်းပြည့်)(p->endpoint.iOffset%p->nChunkSize);
        ကိန်းပြည့် iSpace = MIN(nWrite, p->nChunkSize - iChunkOffset);

        assert( pChunk!=0 || iChunkOffset==0 );
        အကယ်၍( iChunkOffset==0 ){
          /* New chunk is required to extend the file. */
          FileChunk *pNew = sqlite3_malloc(fileChunkSize(p->nChunkSize));
          အကယ်၍( !pNew ){
            ပြန်ပေး SQLITE_IOERR_NOMEM_BKPT;
          }
          pNew->pNext = 0;
          အကယ်၍( pChunk ){
            assert( p->pFirst );
            pChunk->pNext = pNew;
          }မဟုတ်ပါက{
            assert( !p->pFirst );
            p->pFirst = pNew;
          }
          pChunk = p->endpoint.pChunk = pNew;
        }

        assert( pChunk!=0 );
        memcpy((u8*)pChunk->zChunk + iChunkOffset, zWrite, iSpace);
        zWrite += iSpace;
        nWrite -= iSpace;
        p->endpoint.iOffset += iSpace;
      }
    }
  }

  ပြန်ပေး SQLITE_OK;
}

/*
** Truncate the in-memory file.
*/
static ကိန်းပြည့် memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 size){
  MemJournal *p = (MemJournal *)pJfd;
  assert( p->endpoint.pChunk==0 || p->endpoint.pChunk->pNext==0 );
  အကယ်၍( size<p->endpoint.iOffset ){
    FileChunk *pIter = 0;
    အကယ်၍( size==0 ){
      memjrnlFreeChunks(p->pFirst);
      p->pFirst = 0;
    }မဟုတ်ပါက{
      i64 iOff = p->nChunkSize;
      အတွက်(pIter=p->pFirst; ALWAYS(pIter) && iOff<size; pIter=pIter->pNext){
        iOff += p->nChunkSize;
      }
      အကယ်၍( ALWAYS(pIter) ){
        memjrnlFreeChunks(pIter->pNext);
        pIter->pNext = 0;
      }
    }

    p->endpoint.pChunk = pIter;
    p->endpoint.iOffset = size;
    p->readpoint.pChunk = 0;
    p->readpoint.iOffset = 0;
  }
  ပြန်ပေး SQLITE_OK;
}

/*
** Close the file.
*/
static ကိန်းပြည့် memjrnlClose(sqlite3_file *pJfd){
  MemJournal *p = (MemJournal *)pJfd;
  memjrnlFreeChunks(p->pFirst);
  ပြန်ပေး SQLITE_OK;
}

/*
** Sync the file.
**
** If the real file has been created, call its xSync method. Otherwise, 
** syncing an in-memory journal is a no-op. 
*/
static ကိန်းပြည့် memjrnlSync(sqlite3_file *pJfd, ကိန်းပြည့် flags){
  UNUSED_PARAMETER2(pJfd, flags);
  ပြန်ပေး SQLITE_OK;
}

/*
** Query the size of the file in bytes.
*/
static ကိန်းပြည့် memjrnlFileSize(sqlite3_file *pJfd, sqlite_int64 *pSize){
  MemJournal *p = (MemJournal *)pJfd;
  *pSize = (sqlite_int64) p->endpoint.iOffset;
  ပြန်ပေး SQLITE_OK;
}

/*
** Table of methods for MemJournal sqlite3_file object.
*/
static const sa_nracanaa sqlite3_io_methods MemJournalMethods = {
  1,                /* iVersion */
  memjrnlClose,     /* xClose */
  memjrnlRead,      /* xRead */
  memjrnlWrite,     /* xWrite */
  memjrnlTruncate,  /* xTruncate */
  memjrnlSync,      /* xSync */
  memjrnlFileSize,  /* xFileSize */
  0,                /* xLock */
  0,                /* xUnlock */
  0,                /* xCheckReservedLock */
  0,                /* xFileControl */
  0,                /* xSectorSize */
  0,                /* xDeviceCharacteristics */
  0,                /* xShmMap */
  0,                /* xShmLock */
  0,                /* xShmBarrier */
  0,                /* xShmUnmap */
  0,                /* xFetch */
  0                 /* xUnfetch */
};

/* 
** Open a journal file. 
**
** The behaviour of the journal file depends on the value of parameter 
** nSpill. If nSpill is 0, then the journal file is always create and 
** accessed using the underlying VFS. If nSpill is less than zero, then
** all content is always stored in main-memory. Finally, if nSpill is a
** positive value, then the journal file is initially created in-memory
** but may be flushed to disk later on. In this case the journal file is
** flushed to disk either when it grows larger than nSpill bytes in size,
** or when sqlite3JournalCreate() is called.
*/
ကိန်းပြည့် sqlite3JournalOpen(
  sqlite3_vfs *pVfs,         /* The VFS to use for actual file I/O */
  const var_na *zName,         /* Name of the journal file */
  sqlite3_file *pJfd,        /* Preallocated, blank file handle */
  ကိန်းပြည့် flags,                 /* Opening flags */
  ကိန်းပြည့် nSpill                 /* Bytes buffered before opening the file */
){
  MemJournal *p = (MemJournal*)pJfd;

  assert( zName || nSpill<0 || (flags & SQLITE_OPEN_EXCLUSIVE) );

  /* Zero the file-handle object. If nSpill was passed zero, initialize
  ** it using the sqlite3OsOpen() function of the underlying VFS. In this
  ** case none of the code in this module is executed as a result of calls
  ** made on the journal file-handle.  */
  memset(p, 0, parimaa_na(MemJournal));
  အကယ်၍( nSpill==0 ){
    ပြန်ပေး sqlite3OsOpen(pVfs, zName, pJfd, flags, 0);
  }

  အကယ်၍( nSpill>0 ){
    p->nChunkSize = nSpill;
  }မဟုတ်ပါက{
    p->nChunkSize = 8 + MEMJOURNAL_DFLT_FILECHUNKSIZE - parimaa_na(FileChunk);
    assert( MEMJOURNAL_DFLT_FILECHUNKSIZE==fileChunkSize(p->nChunkSize) );
  }

  pJfd->pMethods = (const sqlite3_io_methods*)&MemJournalMethods;
  p->nSpill = nSpill;
  p->flags = flags;
  p->zJournal = zName;
  p->pVfs = pVfs;
  ပြန်ပေး SQLITE_OK;
}

/*
** Open an in-memory journal file.
*/
void sqlite3MemJournalOpen(sqlite3_file *pJfd){
  sqlite3JournalOpen(0, 0, pJfd, 0, -1);
}

#အကယ်၍ defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
/*
** If the argument p points to a MemJournal structure that is not an 
** in-memory-only journal file (i.e. is one that was opened with a +ve
** nSpill parameter or as SQLITE_OPEN_MAIN_JOURNAL), and the underlying 
** file has not yet been created, create it now.
*/
ကိန်းပြည့် sqlite3JournalCreate(sqlite3_file *pJfd){
  ကိန်းပြည့် rc = SQLITE_OK;
  MemJournal *p = (MemJournal*)pJfd;
  အကယ်၍( pJfd->pMethods==&MemJournalMethods && (
#ifdef SQLITE_ENABLE_ATOMIC_WRITE
     p->nSpill>0
#မဟုတ်ပါက
     /* While this appears to not be possible without ATOMIC_WRITE, the
     ** paths are complex, so it seems prudent to leave the test in as
     ** a NEVER(), in case our analysis is subtly flawed. */
     NEVER(p->nSpill>0)
#endif
#ifdef SQLITE_ENABLE_BATCH_ATOMIC_WRITE
     || (p->flags & SQLITE_OPEN_MAIN_JOURNAL)
#endif
  )){
    rc = memjrnlCreateFile(p);
  }
  ပြန်ပေး rc;
}
#endif

/*
** The file-handle passed as the only argument is open on a journal file.
** Return true if this "journal file" is currently stored in heap memory,
** or false otherwise.
*/
ကိန်းပြည့် sqlite3JournalIsInMemory(sqlite3_file *p){
  ပြန်ပေး p->pMethods==&MemJournalMethods;
}

/* 
** Return the number of bytes required to store a JournalFile that uses vfs
** pVfs to create the underlying on-disk files.
*/
ကိန်းပြည့် sqlite3JournalSize(sqlite3_vfs *pVfs){
  ပြန်ပေး MAX(pVfs->szOsFile, (ကိန်းပြည့်)parimaa_na(MemJournal));
}
