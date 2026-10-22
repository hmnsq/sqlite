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
#अन्तर्भाव "sqliteInt.h"

/* Forward references to internal structures */
प्रकारनिर्देश संरचना MemJournal MemJournal;
प्रकारनिर्देश संरचना FilePoint FilePoint;
प्रकारनिर्देश संरचना FileChunk FileChunk;

/*
** The rollback journal is composed of a linked list of these structures.
**
** The zChunk array is always at least 8 bytes in size - usually much more.
** Its actual size is stored in the MemJournal.nChunkSize variable.
*/
संरचना FileChunk {
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
#define fileChunkSize(nChunkSize) (परिमाण(FileChunk) + ((nChunkSize)-8))

/*
** An instance of this object serves as a cursor into the rollback journal.
** The cursor can be either for reading or writing.
*/
संरचना FilePoint {
  sqlite3_int64 iOffset;          /* Offset from the beginning of the file */
  FileChunk *pChunk;              /* Specific chunk into which cursor points */
};

/*
** This structure is a subclass of sqlite3_file. Each open memory-journal
** is an instance of this class.
*/
संरचना MemJournal {
  ध्रुव sqlite3_io_methods *pMethod; /* Parent class. MUST BE FIRST */
  पूर्णाङ्क nChunkSize;                 /* In-memory chunk-size */

  पूर्णाङ्क nSpill;                     /* Bytes of data before flushing */
  FileChunk *pFirst;              /* Head of in-memory chunk-list */
  FilePoint endpoint;             /* Pointer to the end of the file */
  FilePoint readpoint;            /* Pointer to the end of the last xRead() */

  पूर्णाङ्क flags;                      /* xOpen flags */
  sqlite3_vfs *pVfs;              /* The "real" underlying VFS */
  ध्रुव वर्ण *zJournal;           /* Name of the journal file */
};

/*
** Read data from the in-memory journal file.  This is the implementation
** of the sqlite3_vfs.xRead method.
*/
स्थिर पूर्णाङ्क memjrnlRead(
  sqlite3_file *pJfd,    /* The journal file from which to read */
  शून्य *zBuf,            /* Put the results here */
  पूर्णाङ्क iAmt,              /* Number of bytes to read */
  sqlite_int64 iOfst     /* Begin reading at this offset */
){
  MemJournal *p = (MemJournal *)pJfd;
  u8 *zOut = zBuf;
  पूर्णाङ्क nRead = iAmt;
  पूर्णाङ्क iChunkOffset;
  FileChunk *pChunk;

  यदि( (iAmt+iOfst)>p->endpoint.iOffset ){
    प्रत्यागम SQLITE_IOERR_SHORT_READ;
  }
  प्रतिज्ञा( p->readpoint.iOffset==0 || p->readpoint.pChunk!=0 );
  यदि( p->readpoint.iOffset!=iOfst || iOfst==0 ){
    sqlite3_int64 iOff = 0;
    प्रति(pChunk=p->pFirst; 
        ALWAYS(pChunk) && (iOff+p->nChunkSize)<=iOfst;
        pChunk=pChunk->pNext
    ){
      iOff += p->nChunkSize;
    }
  }अन्यथा{
    pChunk = p->readpoint.pChunk;
    प्रतिज्ञा( pChunk!=0 );
  }

  iChunkOffset = (पूर्णाङ्क)(iOfst%p->nChunkSize);
  कुरु {
    पूर्णाङ्क iSpace = p->nChunkSize - iChunkOffset;
    पूर्णाङ्क nCopy = MIN(nRead, (p->nChunkSize - iChunkOffset));
    स_नकल(zOut, (u8*)pChunk->zChunk + iChunkOffset, nCopy);
    zOut += nCopy;
    nRead -= iSpace;
    iChunkOffset = 0;
  } यावत्( nRead>=0 && (pChunk=pChunk->pNext)!=0 && nRead>0 );
  p->readpoint.iOffset = pChunk ? iOfst+iAmt : 0;
  p->readpoint.pChunk = pChunk;

  प्रत्यागम SQLITE_OK;
}

/*
** Free the list of FileChunk structures headed at MemJournal.pFirst.
*/
स्थिर शून्य memjrnlFreeChunks(FileChunk *pFirst){
  FileChunk *pIter;
  FileChunk *pNext;
  प्रति(pIter=pFirst; pIter; pIter=pNext){
    pNext = pIter->pNext;
    sqlite3_free(pIter);
  } 
}

/*
** Flush the contents of memory to a real file on disk.
*/
स्थिर पूर्णाङ्क memjrnlCreateFile(MemJournal *p){
  पूर्णाङ्क rc;
  sqlite3_file *pReal = (sqlite3_file*)p;
  MemJournal copy = *p;

  स_रखो(p, 0, परिमाण(MemJournal));
  rc = sqlite3OsOpen(copy.pVfs, copy.zJournal, pReal, copy.flags, 0);
  यदि( rc==SQLITE_OK ){
    पूर्णाङ्क nChunk = copy.nChunkSize;
    i64 iOff = 0;
    FileChunk *pIter;
    प्रति(pIter=copy.pFirst; pIter; pIter=pIter->pNext){
      यदि( iOff + nChunk > copy.endpoint.iOffset ){
        nChunk = copy.endpoint.iOffset - iOff;
      }
      rc = sqlite3OsWrite(pReal, (u8*)pIter->zChunk, nChunk, iOff);
      यदि( rc ) भङ्ग;
      iOff += nChunk;
    }
    यदि( rc==SQLITE_OK ){
      /* No error has occurred. Free the in-memory buffers. */
      memjrnlFreeChunks(copy.pFirst);
    }
  }
  यदि( rc!=SQLITE_OK ){
    /* If an error occurred while creating or writing to the file, restore
    ** the original before returning. This way, SQLite uses the in-memory
    ** journal data to roll back changes made to the internal page-cache
    ** before this function was called.  */
    sqlite3OsClose(pReal);
    *p = copy;
  }
  प्रत्यागम rc;
}


/* Forward reference */
स्थिर पूर्णाङ्क memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 size);

/*
** Write data to the file.
*/
स्थिर पूर्णाङ्क memjrnlWrite(
  sqlite3_file *pJfd,    /* The journal file into which to write */
  ध्रुव शून्य *zBuf,      /* Take data to be written from here */
  पूर्णाङ्क iAmt,              /* Number of bytes to write */
  sqlite_int64 iOfst     /* Begin writing at this offset into the file */
){
  MemJournal *p = (MemJournal *)pJfd;
  पूर्णाङ्क nWrite = iAmt;
  u8 *zWrite = (u8 *)zBuf;

  /* If the file should be created now, create it and write the new data
  ** into the file on disk. */
  यदि( p->nSpill>0 && (iAmt+iOfst)>p->nSpill ){
    पूर्णाङ्क rc = memjrnlCreateFile(p);
    यदि( rc==SQLITE_OK ){
      rc = sqlite3OsWrite(pJfd, zBuf, iAmt, iOfst);
    }
    प्रत्यागम rc;
  }

  /* If the contents of this write should be stored in memory */
  अन्यथा{
    /* An in-memory journal file should only ever be appended to. Random
    ** access writes are not required. The only exception to this is when
    ** the in-memory journal is being used by a connection using the
    ** atomic-write optimization. In this case the first 28 bytes of the
    ** journal file may be written as part of committing the transaction. */
    प्रतिज्ञा( iOfst<=p->endpoint.iOffset );
    यदि( iOfst>0 && iOfst!=p->endpoint.iOffset ){
      memjrnlTruncate(pJfd, iOfst);
    }
    यदि( iOfst==0 && p->pFirst ){
      प्रतिज्ञा( p->nChunkSize>iAmt );
      स_नकल((u8*)p->pFirst->zChunk, zBuf, iAmt);
    }अन्यथा{
      यावत्( nWrite>0 ){
        FileChunk *pChunk = p->endpoint.pChunk;
        पूर्णाङ्क iChunkOffset = (पूर्णाङ्क)(p->endpoint.iOffset%p->nChunkSize);
        पूर्णाङ्क iSpace = MIN(nWrite, p->nChunkSize - iChunkOffset);

        प्रतिज्ञा( pChunk!=0 || iChunkOffset==0 );
        यदि( iChunkOffset==0 ){
          /* New chunk is required to extend the file. */
          FileChunk *pNew = sqlite3_malloc(fileChunkSize(p->nChunkSize));
          यदि( !pNew ){
            प्रत्यागम SQLITE_IOERR_NOMEM_BKPT;
          }
          pNew->pNext = 0;
          यदि( pChunk ){
            प्रतिज्ञा( p->pFirst );
            pChunk->pNext = pNew;
          }अन्यथा{
            प्रतिज्ञा( !p->pFirst );
            p->pFirst = pNew;
          }
          pChunk = p->endpoint.pChunk = pNew;
        }

        प्रतिज्ञा( pChunk!=0 );
        स_नकल((u8*)pChunk->zChunk + iChunkOffset, zWrite, iSpace);
        zWrite += iSpace;
        nWrite -= iSpace;
        p->endpoint.iOffset += iSpace;
      }
    }
  }

  प्रत्यागम SQLITE_OK;
}

/*
** Truncate the in-memory file.
*/
स्थिर पूर्णाङ्क memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 size){
  MemJournal *p = (MemJournal *)pJfd;
  प्रतिज्ञा( p->endpoint.pChunk==0 || p->endpoint.pChunk->pNext==0 );
  यदि( size<p->endpoint.iOffset ){
    FileChunk *pIter = 0;
    यदि( size==0 ){
      memjrnlFreeChunks(p->pFirst);
      p->pFirst = 0;
    }अन्यथा{
      i64 iOff = p->nChunkSize;
      प्रति(pIter=p->pFirst; ALWAYS(pIter) && iOff<size; pIter=pIter->pNext){
        iOff += p->nChunkSize;
      }
      यदि( ALWAYS(pIter) ){
        memjrnlFreeChunks(pIter->pNext);
        pIter->pNext = 0;
      }
    }

    p->endpoint.pChunk = pIter;
    p->endpoint.iOffset = size;
    p->readpoint.pChunk = 0;
    p->readpoint.iOffset = 0;
  }
  प्रत्यागम SQLITE_OK;
}

/*
** Close the file.
*/
स्थिर पूर्णाङ्क memjrnlClose(sqlite3_file *pJfd){
  MemJournal *p = (MemJournal *)pJfd;
  memjrnlFreeChunks(p->pFirst);
  प्रत्यागम SQLITE_OK;
}

/*
** Sync the file.
**
** If the real file has been created, call its xSync method. Otherwise, 
** syncing an in-memory journal is a no-op. 
*/
स्थिर पूर्णाङ्क memjrnlSync(sqlite3_file *pJfd, पूर्णाङ्क flags){
  UNUSED_PARAMETER2(pJfd, flags);
  प्रत्यागम SQLITE_OK;
}

/*
** Query the size of the file in bytes.
*/
स्थिर पूर्णाङ्क memjrnlFileSize(sqlite3_file *pJfd, sqlite_int64 *pSize){
  MemJournal *p = (MemJournal *)pJfd;
  *pSize = (sqlite_int64) p->endpoint.iOffset;
  प्रत्यागम SQLITE_OK;
}

/*
** Table of methods for MemJournal sqlite3_file object.
*/
स्थिर ध्रुव संरचना sqlite3_io_methods MemJournalMethods = {
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
पूर्णाङ्क sqlite3JournalOpen(
  sqlite3_vfs *pVfs,         /* The VFS to use for actual file I/O */
  ध्रुव वर्ण *zName,         /* Name of the journal file */
  sqlite3_file *pJfd,        /* Preallocated, blank file handle */
  पूर्णाङ्क flags,                 /* Opening flags */
  पूर्णाङ्क nSpill                 /* Bytes buffered before opening the file */
){
  MemJournal *p = (MemJournal*)pJfd;

  प्रतिज्ञा( zName || nSpill<0 || (flags & SQLITE_OPEN_EXCLUSIVE) );

  /* Zero the file-handle object. If nSpill was passed zero, initialize
  ** it using the sqlite3OsOpen() function of the underlying VFS. In this
  ** case none of the code in this module is executed as a result of calls
  ** made on the journal file-handle.  */
  स_रखो(p, 0, परिमाण(MemJournal));
  यदि( nSpill==0 ){
    प्रत्यागम sqlite3OsOpen(pVfs, zName, pJfd, flags, 0);
  }

  यदि( nSpill>0 ){
    p->nChunkSize = nSpill;
  }अन्यथा{
    p->nChunkSize = 8 + MEMJOURNAL_DFLT_FILECHUNKSIZE - परिमाण(FileChunk);
    प्रतिज्ञा( MEMJOURNAL_DFLT_FILECHUNKSIZE==fileChunkSize(p->nChunkSize) );
  }

  pJfd->pMethods = (ध्रुव sqlite3_io_methods*)&MemJournalMethods;
  p->nSpill = nSpill;
  p->flags = flags;
  p->zJournal = zName;
  p->pVfs = pVfs;
  प्रत्यागम SQLITE_OK;
}

/*
** Open an in-memory journal file.
*/
शून्य sqlite3MemJournalOpen(sqlite3_file *pJfd){
  sqlite3JournalOpen(0, 0, pJfd, 0, -1);
}

#यदि defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
/*
** If the argument p points to a MemJournal structure that is not an 
** in-memory-only journal file (i.e. is one that was opened with a +ve
** nSpill parameter or as SQLITE_OPEN_MAIN_JOURNAL), and the underlying 
** file has not yet been created, create it now.
*/
पूर्णाङ्क sqlite3JournalCreate(sqlite3_file *pJfd){
  पूर्णाङ्क rc = SQLITE_OK;
  MemJournal *p = (MemJournal*)pJfd;
  यदि( pJfd->pMethods==&MemJournalMethods && (
#ifdef SQLITE_ENABLE_ATOMIC_WRITE
     p->nSpill>0
#अन्यथा
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
  प्रत्यागम rc;
}
#endif

/*
** The file-handle passed as the only argument is open on a journal file.
** Return true if this "journal file" is currently stored in heap memory,
** or false otherwise.
*/
पूर्णाङ्क sqlite3JournalIsInMemory(sqlite3_file *p){
  प्रत्यागम p->pMethods==&MemJournalMethods;
}

/* 
** Return the number of bytes required to store a JournalFile that uses vfs
** pVfs to create the underlying on-disk files.
*/
पूर्णाङ्क sqlite3JournalSize(sqlite3_vfs *pVfs){
  प्रत्यागम MAX(pVfs->szOsFile, (पूर्णाङ्क)परिमाण(MemJournal));
}
