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
#અન્તર્ભાવ "sqliteInt.h"

/* Forward references to internal structures */
પ્રકારવ્યાખ્યા સંરચના MemJournal MemJournal;
પ્રકારવ્યાખ્યા સંરચના FilePoint FilePoint;
પ્રકારવ્યાખ્યા સંરચના FileChunk FileChunk;

/*
** The rollback journal is composed of a linked list of these structures.
**
** The zChunk array is always at least 8 bytes in size - usually much more.
** Its actual size is stored in the MemJournal.nChunkSize variable.
*/
સંરચના FileChunk {
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
#define fileChunkSize(nChunkSize) (કદ(FileChunk) + ((nChunkSize)-8))

/*
** An instance of this object serves as a cursor into the rollback journal.
** The cursor can be either for reading or writing.
*/
સંરચના FilePoint {
  sqlite3_int64 iOffset;          /* Offset from the beginning of the file */
  FileChunk *pChunk;              /* Specific chunk into which cursor points */
};

/*
** This structure is a subclass of sqlite3_file. Each open memory-journal
** is an instance of this class.
*/
સંરચના MemJournal {
  અચલ sqlite3_io_methods *pMethod; /* Parent class. MUST BE FIRST */
  પૂર્ણાંક nChunkSize;                 /* In-memory chunk-size */

  પૂર્ણાંક nSpill;                     /* Bytes of data before flushing */
  FileChunk *pFirst;              /* Head of in-memory chunk-list */
  FilePoint endpoint;             /* Pointer to the end of the file */
  FilePoint readpoint;            /* Pointer to the end of the last xRead() */

  પૂર્ણાંક flags;                      /* xOpen flags */
  sqlite3_vfs *pVfs;              /* The "real" underlying VFS */
  અચલ અક્ષર *zJournal;           /* Name of the journal file */
};

/*
** Read data from the in-memory journal file.  This is the implementation
** of the sqlite3_vfs.xRead method.
*/
સ્થિર પૂર્ણાંક memjrnlRead(
  sqlite3_file *pJfd,    /* The journal file from which to read */
  શૂન્ય *zBuf,            /* Put the results here */
  પૂર્ણાંક iAmt,              /* Number of bytes to read */
  sqlite_int64 iOfst     /* Begin reading at this offset */
){
  MemJournal *p = (MemJournal *)pJfd;
  u8 *zOut = zBuf;
  પૂર્ણાંક nRead = iAmt;
  પૂર્ણાંક iChunkOffset;
  FileChunk *pChunk;

  જો( (iAmt+iOfst)>p->endpoint.iOffset ){
    પાછા SQLITE_IOERR_SHORT_READ;
  }
  પ્રતિજ્ઞા( p->readpoint.iOffset==0 || p->readpoint.pChunk!=0 );
  જો( p->readpoint.iOffset!=iOfst || iOfst==0 ){
    sqlite3_int64 iOff = 0;
    માટે(pChunk=p->pFirst; 
        ALWAYS(pChunk) && (iOff+p->nChunkSize)<=iOfst;
        pChunk=pChunk->pNext
    ){
      iOff += p->nChunkSize;
    }
  }નહીંતર{
    pChunk = p->readpoint.pChunk;
    પ્રતિજ્ઞા( pChunk!=0 );
  }

  iChunkOffset = (પૂર્ણાંક)(iOfst%p->nChunkSize);
  કરો {
    પૂર્ણાંક iSpace = p->nChunkSize - iChunkOffset;
    પૂર્ણાંક nCopy = MIN(nRead, (p->nChunkSize - iChunkOffset));
    memcpy(zOut, (u8*)pChunk->zChunk + iChunkOffset, nCopy);
    zOut += nCopy;
    nRead -= iSpace;
    iChunkOffset = 0;
  } જ્યારે( nRead>=0 && (pChunk=pChunk->pNext)!=0 && nRead>0 );
  p->readpoint.iOffset = pChunk ? iOfst+iAmt : 0;
  p->readpoint.pChunk = pChunk;

  પાછા SQLITE_OK;
}

/*
** Free the list of FileChunk structures headed at MemJournal.pFirst.
*/
સ્થિર શૂન્ય memjrnlFreeChunks(FileChunk *pFirst){
  FileChunk *pIter;
  FileChunk *pNext;
  માટે(pIter=pFirst; pIter; pIter=pNext){
    pNext = pIter->pNext;
    sqlite3_free(pIter);
  } 
}

/*
** Flush the contents of memory to a real file on disk.
*/
સ્થિર પૂર્ણાંક memjrnlCreateFile(MemJournal *p){
  પૂર્ણાંક rc;
  sqlite3_file *pReal = (sqlite3_file*)p;
  MemJournal copy = *p;

  memset(p, 0, કદ(MemJournal));
  rc = sqlite3OsOpen(copy.pVfs, copy.zJournal, pReal, copy.flags, 0);
  જો( rc==SQLITE_OK ){
    પૂર્ણાંક nChunk = copy.nChunkSize;
    i64 iOff = 0;
    FileChunk *pIter;
    માટે(pIter=copy.pFirst; pIter; pIter=pIter->pNext){
      જો( iOff + nChunk > copy.endpoint.iOffset ){
        nChunk = copy.endpoint.iOffset - iOff;
      }
      rc = sqlite3OsWrite(pReal, (u8*)pIter->zChunk, nChunk, iOff);
      જો( rc ) તોડો;
      iOff += nChunk;
    }
    જો( rc==SQLITE_OK ){
      /* No error has occurred. Free the in-memory buffers. */
      memjrnlFreeChunks(copy.pFirst);
    }
  }
  જો( rc!=SQLITE_OK ){
    /* If an error occurred while creating or writing to the file, restore
    ** the original before returning. This way, SQLite uses the in-memory
    ** journal data to roll back changes made to the internal page-cache
    ** before this function was called.  */
    sqlite3OsClose(pReal);
    *p = copy;
  }
  પાછા rc;
}


/* Forward reference */
સ્થિર પૂર્ણાંક memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 size);

/*
** Write data to the file.
*/
સ્થિર પૂર્ણાંક memjrnlWrite(
  sqlite3_file *pJfd,    /* The journal file into which to write */
  અચલ શૂન્ય *zBuf,      /* Take data to be written from here */
  પૂર્ણાંક iAmt,              /* Number of bytes to write */
  sqlite_int64 iOfst     /* Begin writing at this offset into the file */
){
  MemJournal *p = (MemJournal *)pJfd;
  પૂર્ણાંક nWrite = iAmt;
  u8 *zWrite = (u8 *)zBuf;

  /* If the file should be created now, create it and write the new data
  ** into the file on disk. */
  જો( p->nSpill>0 && (iAmt+iOfst)>p->nSpill ){
    પૂર્ણાંક rc = memjrnlCreateFile(p);
    જો( rc==SQLITE_OK ){
      rc = sqlite3OsWrite(pJfd, zBuf, iAmt, iOfst);
    }
    પાછા rc;
  }

  /* If the contents of this write should be stored in memory */
  નહીંતર{
    /* An in-memory journal file should only ever be appended to. Random
    ** access writes are not required. The only exception to this is when
    ** the in-memory journal is being used by a connection using the
    ** atomic-write optimization. In this case the first 28 bytes of the
    ** journal file may be written as part of committing the transaction. */
    પ્રતિજ્ઞા( iOfst<=p->endpoint.iOffset );
    જો( iOfst>0 && iOfst!=p->endpoint.iOffset ){
      memjrnlTruncate(pJfd, iOfst);
    }
    જો( iOfst==0 && p->pFirst ){
      પ્રતિજ્ઞા( p->nChunkSize>iAmt );
      memcpy((u8*)p->pFirst->zChunk, zBuf, iAmt);
    }નહીંતર{
      જ્યારે( nWrite>0 ){
        FileChunk *pChunk = p->endpoint.pChunk;
        પૂર્ણાંક iChunkOffset = (પૂર્ણાંક)(p->endpoint.iOffset%p->nChunkSize);
        પૂર્ણાંક iSpace = MIN(nWrite, p->nChunkSize - iChunkOffset);

        પ્રતિજ્ઞા( pChunk!=0 || iChunkOffset==0 );
        જો( iChunkOffset==0 ){
          /* New chunk is required to extend the file. */
          FileChunk *pNew = sqlite3_malloc(fileChunkSize(p->nChunkSize));
          જો( !pNew ){
            પાછા SQLITE_IOERR_NOMEM_BKPT;
          }
          pNew->pNext = 0;
          જો( pChunk ){
            પ્રતિજ્ઞા( p->pFirst );
            pChunk->pNext = pNew;
          }નહીંતર{
            પ્રતિજ્ઞા( !p->pFirst );
            p->pFirst = pNew;
          }
          pChunk = p->endpoint.pChunk = pNew;
        }

        પ્રતિજ્ઞા( pChunk!=0 );
        memcpy((u8*)pChunk->zChunk + iChunkOffset, zWrite, iSpace);
        zWrite += iSpace;
        nWrite -= iSpace;
        p->endpoint.iOffset += iSpace;
      }
    }
  }

  પાછા SQLITE_OK;
}

/*
** Truncate the in-memory file.
*/
સ્થિર પૂર્ણાંક memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 size){
  MemJournal *p = (MemJournal *)pJfd;
  પ્રતિજ્ઞા( p->endpoint.pChunk==0 || p->endpoint.pChunk->pNext==0 );
  જો( size<p->endpoint.iOffset ){
    FileChunk *pIter = 0;
    જો( size==0 ){
      memjrnlFreeChunks(p->pFirst);
      p->pFirst = 0;
    }નહીંતર{
      i64 iOff = p->nChunkSize;
      માટે(pIter=p->pFirst; ALWAYS(pIter) && iOff<size; pIter=pIter->pNext){
        iOff += p->nChunkSize;
      }
      જો( ALWAYS(pIter) ){
        memjrnlFreeChunks(pIter->pNext);
        pIter->pNext = 0;
      }
    }

    p->endpoint.pChunk = pIter;
    p->endpoint.iOffset = size;
    p->readpoint.pChunk = 0;
    p->readpoint.iOffset = 0;
  }
  પાછા SQLITE_OK;
}

/*
** Close the file.
*/
સ્થિર પૂર્ણાંક memjrnlClose(sqlite3_file *pJfd){
  MemJournal *p = (MemJournal *)pJfd;
  memjrnlFreeChunks(p->pFirst);
  પાછા SQLITE_OK;
}

/*
** Sync the file.
**
** If the real file has been created, call its xSync method. Otherwise, 
** syncing an in-memory journal is a no-op. 
*/
સ્થિર પૂર્ણાંક memjrnlSync(sqlite3_file *pJfd, પૂર્ણાંક flags){
  UNUSED_PARAMETER2(pJfd, flags);
  પાછા SQLITE_OK;
}

/*
** Query the size of the file in bytes.
*/
સ્થિર પૂર્ણાંક memjrnlFileSize(sqlite3_file *pJfd, sqlite_int64 *pSize){
  MemJournal *p = (MemJournal *)pJfd;
  *pSize = (sqlite_int64) p->endpoint.iOffset;
  પાછા SQLITE_OK;
}

/*
** Table of methods for MemJournal sqlite3_file object.
*/
સ્થિર અચલ સંરચના sqlite3_io_methods MemJournalMethods = {
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
પૂર્ણાંક sqlite3JournalOpen(
  sqlite3_vfs *pVfs,         /* The VFS to use for actual file I/O */
  અચલ અક્ષર *zName,         /* Name of the journal file */
  sqlite3_file *pJfd,        /* Preallocated, blank file handle */
  પૂર્ણાંક flags,                 /* Opening flags */
  પૂર્ણાંક nSpill                 /* Bytes buffered before opening the file */
){
  MemJournal *p = (MemJournal*)pJfd;

  પ્રતિજ્ઞા( zName || nSpill<0 || (flags & SQLITE_OPEN_EXCLUSIVE) );

  /* Zero the file-handle object. If nSpill was passed zero, initialize
  ** it using the sqlite3OsOpen() function of the underlying VFS. In this
  ** case none of the code in this module is executed as a result of calls
  ** made on the journal file-handle.  */
  memset(p, 0, કદ(MemJournal));
  જો( nSpill==0 ){
    પાછા sqlite3OsOpen(pVfs, zName, pJfd, flags, 0);
  }

  જો( nSpill>0 ){
    p->nChunkSize = nSpill;
  }નહીંતર{
    p->nChunkSize = 8 + MEMJOURNAL_DFLT_FILECHUNKSIZE - કદ(FileChunk);
    પ્રતિજ્ઞા( MEMJOURNAL_DFLT_FILECHUNKSIZE==fileChunkSize(p->nChunkSize) );
  }

  pJfd->pMethods = (અચલ sqlite3_io_methods*)&MemJournalMethods;
  p->nSpill = nSpill;
  p->flags = flags;
  p->zJournal = zName;
  p->pVfs = pVfs;
  પાછા SQLITE_OK;
}

/*
** Open an in-memory journal file.
*/
શૂન્ય sqlite3MemJournalOpen(sqlite3_file *pJfd){
  sqlite3JournalOpen(0, 0, pJfd, 0, -1);
}

#જો defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
/*
** If the argument p points to a MemJournal structure that is not an 
** in-memory-only journal file (i.e. is one that was opened with a +ve
** nSpill parameter or as SQLITE_OPEN_MAIN_JOURNAL), and the underlying 
** file has not yet been created, create it now.
*/
પૂર્ણાંક sqlite3JournalCreate(sqlite3_file *pJfd){
  પૂર્ણાંક rc = SQLITE_OK;
  MemJournal *p = (MemJournal*)pJfd;
  જો( pJfd->pMethods==&MemJournalMethods && (
#ifdef SQLITE_ENABLE_ATOMIC_WRITE
     p->nSpill>0
#નહીંતર
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
  પાછા rc;
}
#endif

/*
** The file-handle passed as the only argument is open on a journal file.
** Return true if this "journal file" is currently stored in heap memory,
** or false otherwise.
*/
પૂર્ણાંક sqlite3JournalIsInMemory(sqlite3_file *p){
  પાછા p->pMethods==&MemJournalMethods;
}

/* 
** Return the number of bytes required to store a JournalFile that uses vfs
** pVfs to create the underlying on-disk files.
*/
પૂર્ણાંક sqlite3JournalSize(sqlite3_vfs *pVfs){
  પાછા MAX(pVfs->szOsFile, (પૂર્ણાંક)કદ(MemJournal));
}
