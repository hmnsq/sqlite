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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/* Forward references to internal structures */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ MemJournal MemJournal;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ File_ਬਿੰਦੂ File_ਬਿੰਦੂ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ FileChunk FileChunk;

/*
** The rollback journal is composed of a linked list of these structures.
**
** The zChunk array is always at least 8 bytes in size - usually much more.
** Its actual size is stored in the MemJournal.nChunkSize variable.
*/
ਬਣਤਰ FileChunk {
  FileChunk *p_ਅਗਲਾ;               /* Next chunk in the journal */
  u8 zChunk[8];                   /* Content of this chunk */
};

/*
** By default, allocate this many bytes of memory for each FileChunk object.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MEMJOURNAL_DFLT_FILECHUNKSIZE 1024

/*
** For chunk size nChunkSize, return the number of bytes that should
** be allocated for each FileChunk structure.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ file_Chunk_ਮਾਪ(n_Chunk_ਮਾਪ) (ਆਕਾਰ(FileChunk) + ((n_Chunk_ਮਾਪ)-8))

/*
** An instance of this object serves as a cursor into the rollback journal.
** The cursor can be either for reading or writing.
*/
ਬਣਤਰ File_ਬਿੰਦੂ {
  sqlite3_int64 i_ਵਿੱਥ;          /* Offset from the beginning of the file */
  FileChunk *pChunk;              /* Specific chunk into which cursor points */
};

/*
** This structure is a subclass of sqlite3_file. Each open memory-journal
** is an instance of this class.
*/
ਬਣਤਰ MemJournal {
  ਅਚਲ sqlite3_io_methods *p_ਵਿਧੀ; /* Parent class. MUST BE FIRST */
  ਪੂਰਨ n_Chunk_ਮਾਪ;                 /* In-memory chunk-size */

  ਪੂਰਨ nSpill;                     /* Bytes of data before flushing */
  FileChunk *p_ਪਹਿਲਾ;              /* Head of in-memory chunk-list */
  File_ਬਿੰਦੂ endpoint;             /* Pointer to the end of the file */
  File_ਬਿੰਦੂ readpoint;            /* Pointer to the end of the last xRead() */

  ਪੂਰਨ flags;                      /* xOpen flags */
  sqlite3_vfs *pVfs;              /* The "real" underlying VFS */
  ਅਚਲ ਅੱਖਰ *zJournal;           /* Name of the journal file */
};

/*
** Read data from the in-memory journal file.  This is the implementation
** of the sqlite3_vfs.xRead method.
*/
ਸਥਿਰ ਪੂਰਨ memjrnlRead(
  sqlite3_file *pJfd,    /* The journal file from which to read */
  ਖਾਲੀ *zBuf,            /* Put the results here */
  ਪੂਰਨ iAmt,              /* Number of bytes to read */
  sqlite_int64 iOfst     /* Begin reading at this offset */
){
  MemJournal *p = (MemJournal *)pJfd;
  u8 *z_ਬਾਹਰਲਾ = zBuf;
  ਪੂਰਨ nRead = iAmt;
  ਪੂਰਨ i_Chunk_ਵਿੱਥ;
  FileChunk *pChunk;

  ਜੇ( (iAmt+iOfst)>p->endpoint.i_ਵਿੱਥ ){
    ਵਾਪਸ SQLITE_IOERR_SHORT_READ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p->readpoint.i_ਵਿੱਥ==0 || p->readpoint.pChunk!=0 );
  ਜੇ( p->readpoint.i_ਵਿੱਥ!=iOfst || iOfst==0 ){
    sqlite3_int64 iOff = 0;
    ਲਈ(pChunk=p->p_ਪਹਿਲਾ; 
        ALWAYS(pChunk) && (iOff+p->n_Chunk_ਮਾਪ)<=iOfst;
        pChunk=pChunk->p_ਅਗਲਾ
    ){
      iOff += p->n_Chunk_ਮਾਪ;
    }
  }ਨਹੀਂਤਾਂ{
    pChunk = p->readpoint.pChunk;
    ਪ੍ਰਤਿਜ੍ਞਾ( pChunk!=0 );
  }

  i_Chunk_ਵਿੱਥ = (ਪੂਰਨ)(iOfst%p->n_Chunk_ਮਾਪ);
  ਕਰੋ {
    ਪੂਰਨ iSpace = p->n_Chunk_ਮਾਪ - i_Chunk_ਵਿੱਥ;
    ਪੂਰਨ nCopy = MIN(nRead, (p->n_Chunk_ਮਾਪ - i_Chunk_ਵਿੱਥ));
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(z_ਬਾਹਰਲਾ, (u8*)pChunk->zChunk + i_Chunk_ਵਿੱਥ, nCopy);
    z_ਬਾਹਰਲਾ += nCopy;
    nRead -= iSpace;
    i_Chunk_ਵਿੱਥ = 0;
  } ਜਦੋਂ( nRead>=0 && (pChunk=pChunk->p_ਅਗਲਾ)!=0 && nRead>0 );
  p->readpoint.i_ਵਿੱਥ = pChunk ? iOfst+iAmt : 0;
  p->readpoint.pChunk = pChunk;

  ਵਾਪਸ SQLITE_OK;
}

/*
** Free the list of FileChunk structures headed at MemJournal.pFirst.
*/
ਸਥਿਰ ਖਾਲੀ memjrnlFreeChunks(FileChunk *p_ਪਹਿਲਾ){
  FileChunk *pIter;
  FileChunk *p_ਅਗਲਾ;
  ਲਈ(pIter=p_ਪਹਿਲਾ; pIter; pIter=p_ਅਗਲਾ){
    p_ਅਗਲਾ = pIter->p_ਅਗਲਾ;
    sqlite3_free(pIter);
  } 
}

/*
** Flush the contents of memory to a real file on disk.
*/
ਸਥਿਰ ਪੂਰਨ memjrnlCreateFile(MemJournal *p){
  ਪੂਰਨ rc;
  sqlite3_file *pReal = (sqlite3_file*)p;
  MemJournal copy = *p;

  memset(p, 0, ਆਕਾਰ(MemJournal));
  rc = sqlite3OsOpen(copy.pVfs, copy.zJournal, pReal, copy.flags, 0);
  ਜੇ( rc==SQLITE_OK ){
    ਪੂਰਨ nChunk = copy.n_Chunk_ਮਾਪ;
    i64 iOff = 0;
    FileChunk *pIter;
    ਲਈ(pIter=copy.p_ਪਹਿਲਾ; pIter; pIter=pIter->p_ਅਗਲਾ){
      ਜੇ( iOff + nChunk > copy.endpoint.i_ਵਿੱਥ ){
        nChunk = copy.endpoint.i_ਵਿੱਥ - iOff;
      }
      rc = sqlite3OsWrite(pReal, (u8*)pIter->zChunk, nChunk, iOff);
      ਜੇ( rc ) ਰੋਕੋ;
      iOff += nChunk;
    }
    ਜੇ( rc==SQLITE_OK ){
      /* No error has occurred. Free the in-memory buffers. */
      memjrnlFreeChunks(copy.p_ਪਹਿਲਾ);
    }
  }
  ਜੇ( rc!=SQLITE_OK ){
    /* If an error occurred while creating or writing to the file, restore
    ** the original before returning. This way, SQLite uses the in-memory
    ** journal data to roll back changes made to the internal page-cache
    ** before this function was called.  */
    sqlite3OsClose(pReal);
    *p = copy;
  }
  ਵਾਪਸ rc;
}


/* Forward reference */
ਸਥਿਰ ਪੂਰਨ memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 ਮਾਪ);

/*
** Write data to the file.
*/
ਸਥਿਰ ਪੂਰਨ memjrnlWrite(
  sqlite3_file *pJfd,    /* The journal file into which to write */
  ਅਚਲ ਖਾਲੀ *zBuf,      /* Take data to be written from here */
  ਪੂਰਨ iAmt,              /* Number of bytes to write */
  sqlite_int64 iOfst     /* Begin writing at this offset into the file */
){
  MemJournal *p = (MemJournal *)pJfd;
  ਪੂਰਨ nWrite = iAmt;
  u8 *zWrite = (u8 *)zBuf;

  /* If the file should be created now, create it and write the new data
  ** into the file on disk. */
  ਜੇ( p->nSpill>0 && (iAmt+iOfst)>p->nSpill ){
    ਪੂਰਨ rc = memjrnlCreateFile(p);
    ਜੇ( rc==SQLITE_OK ){
      rc = sqlite3OsWrite(pJfd, zBuf, iAmt, iOfst);
    }
    ਵਾਪਸ rc;
  }

  /* If the contents of this write should be stored in memory */
  ਨਹੀਂਤਾਂ{
    /* An in-memory journal file should only ever be appended to. Random
    ** access writes are not required. The only exception to this is when
    ** the in-memory journal is being used by a connection using the
    ** atomic-write optimization. In this case the first 28 bytes of the
    ** journal file may be written as part of committing the transaction. */
    ਪ੍ਰਤਿਜ੍ਞਾ( iOfst<=p->endpoint.i_ਵਿੱਥ );
    ਜੇ( iOfst>0 && iOfst!=p->endpoint.i_ਵਿੱਥ ){
      memjrnlTruncate(pJfd, iOfst);
    }
    ਜੇ( iOfst==0 && p->p_ਪਹਿਲਾ ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p->n_Chunk_ਮਾਪ>iAmt );
      ਸਮ੍ਰਿਤੀ_ਨਕਲ((u8*)p->p_ਪਹਿਲਾ->zChunk, zBuf, iAmt);
    }ਨਹੀਂਤਾਂ{
      ਜਦੋਂ( nWrite>0 ){
        FileChunk *pChunk = p->endpoint.pChunk;
        ਪੂਰਨ i_Chunk_ਵਿੱਥ = (ਪੂਰਨ)(p->endpoint.i_ਵਿੱਥ%p->n_Chunk_ਮਾਪ);
        ਪੂਰਨ iSpace = MIN(nWrite, p->n_Chunk_ਮਾਪ - i_Chunk_ਵਿੱਥ);

        ਪ੍ਰਤਿਜ੍ਞਾ( pChunk!=0 || i_Chunk_ਵਿੱਥ==0 );
        ਜੇ( i_Chunk_ਵਿੱਥ==0 ){
          /* New chunk is required to extend the file. */
          FileChunk *pNew = sqlite3_malloc(file_Chunk_ਮਾਪ(p->n_Chunk_ਮਾਪ));
          ਜੇ( !pNew ){
            ਵਾਪਸ SQLITE_IOERR_NOMEM_BKPT;
          }
          pNew->p_ਅਗਲਾ = 0;
          ਜੇ( pChunk ){
            ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਪਹਿਲਾ );
            pChunk->p_ਅਗਲਾ = pNew;
          }ਨਹੀਂਤਾਂ{
            ਪ੍ਰਤਿਜ੍ਞਾ( !p->p_ਪਹਿਲਾ );
            p->p_ਪਹਿਲਾ = pNew;
          }
          pChunk = p->endpoint.pChunk = pNew;
        }

        ਪ੍ਰਤਿਜ੍ਞਾ( pChunk!=0 );
        ਸਮ੍ਰਿਤੀ_ਨਕਲ((u8*)pChunk->zChunk + i_Chunk_ਵਿੱਥ, zWrite, iSpace);
        zWrite += iSpace;
        nWrite -= iSpace;
        p->endpoint.i_ਵਿੱਥ += iSpace;
      }
    }
  }

  ਵਾਪਸ SQLITE_OK;
}

/*
** Truncate the in-memory file.
*/
ਸਥਿਰ ਪੂਰਨ memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 ਮਾਪ){
  MemJournal *p = (MemJournal *)pJfd;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->endpoint.pChunk==0 || p->endpoint.pChunk->p_ਅਗਲਾ==0 );
  ਜੇ( ਮਾਪ<p->endpoint.i_ਵਿੱਥ ){
    FileChunk *pIter = 0;
    ਜੇ( ਮਾਪ==0 ){
      memjrnlFreeChunks(p->p_ਪਹਿਲਾ);
      p->p_ਪਹਿਲਾ = 0;
    }ਨਹੀਂਤਾਂ{
      i64 iOff = p->n_Chunk_ਮਾਪ;
      ਲਈ(pIter=p->p_ਪਹਿਲਾ; ALWAYS(pIter) && iOff<ਮਾਪ; pIter=pIter->p_ਅਗਲਾ){
        iOff += p->n_Chunk_ਮਾਪ;
      }
      ਜੇ( ALWAYS(pIter) ){
        memjrnlFreeChunks(pIter->p_ਅਗਲਾ);
        pIter->p_ਅਗਲਾ = 0;
      }
    }

    p->endpoint.pChunk = pIter;
    p->endpoint.i_ਵਿੱਥ = ਮਾਪ;
    p->readpoint.pChunk = 0;
    p->readpoint.i_ਵਿੱਥ = 0;
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Close the file.
*/
ਸਥਿਰ ਪੂਰਨ memjrnlClose(sqlite3_file *pJfd){
  MemJournal *p = (MemJournal *)pJfd;
  memjrnlFreeChunks(p->p_ਪਹਿਲਾ);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Sync the file.
**
** If the real file has been created, call its xSync method. Otherwise, 
** syncing an in-memory journal is a no-op. 
*/
ਸਥਿਰ ਪੂਰਨ memjrnlSync(sqlite3_file *pJfd, ਪੂਰਨ flags){
  ਅਣਵਰਤਿਆ_PARAMETER2(pJfd, flags);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Query the size of the file in bytes.
*/
ਸਥਿਰ ਪੂਰਨ memjrnl_File_ਮਾਪ(sqlite3_file *pJfd, sqlite_int64 *p_ਮਾਪ){
  MemJournal *p = (MemJournal *)pJfd;
  *p_ਮਾਪ = (sqlite_int64) p->endpoint.i_ਵਿੱਥ;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Table of methods for MemJournal sqlite3_file object.
*/
ਸਥਿਰ ਅਚਲ ਬਣਤਰ sqlite3_io_methods MemJournalMethods = {
  1,                /* iVersion */
  memjrnlClose,     /* xClose */
  memjrnlRead,      /* xRead */
  memjrnlWrite,     /* xWrite */
  memjrnlTruncate,  /* xTruncate */
  memjrnlSync,      /* xSync */
  memjrnl_File_ਮਾਪ,  /* xFileSize */
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
ਪੂਰਨ sqlite3JournalOpen(
  sqlite3_vfs *pVfs,         /* The VFS to use for actual file I/O */
  ਅਚਲ ਅੱਖਰ *zName,         /* Name of the journal file */
  sqlite3_file *pJfd,        /* Preallocated, blank file handle */
  ਪੂਰਨ flags,                 /* Opening flags */
  ਪੂਰਨ nSpill                 /* Bytes buffered before opening the file */
){
  MemJournal *p = (MemJournal*)pJfd;

  ਪ੍ਰਤਿਜ੍ਞਾ( zName || nSpill<0 || (flags & SQLITE_OPEN_EXCLUSIVE) );

  /* Zero the file-handle object. If nSpill was passed zero, initialize
  ** it using the sqlite3OsOpen() function of the underlying VFS. In this
  ** case none of the code in this module is executed as a result of calls
  ** made on the journal file-handle.  */
  memset(p, 0, ਆਕਾਰ(MemJournal));
  ਜੇ( nSpill==0 ){
    ਵਾਪਸ sqlite3OsOpen(pVfs, zName, pJfd, flags, 0);
  }

  ਜੇ( nSpill>0 ){
    p->n_Chunk_ਮਾਪ = nSpill;
  }ਨਹੀਂਤਾਂ{
    p->n_Chunk_ਮਾਪ = 8 + MEMJOURNAL_DFLT_FILECHUNKSIZE - ਆਕਾਰ(FileChunk);
    ਪ੍ਰਤਿਜ੍ਞਾ( MEMJOURNAL_DFLT_FILECHUNKSIZE==file_Chunk_ਮਾਪ(p->n_Chunk_ਮਾਪ) );
  }

  pJfd->pMethods = (ਅਚਲ sqlite3_io_methods*)&MemJournalMethods;
  p->nSpill = nSpill;
  p->flags = flags;
  p->zJournal = zName;
  p->pVfs = pVfs;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Open an in-memory journal file.
*/
ਖਾਲੀ sqlite3MemJournalOpen(sqlite3_file *pJfd){
  sqlite3JournalOpen(0, 0, pJfd, 0, -1);
}

#ਜੇ defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
/*
** If the argument p points to a MemJournal structure that is not an 
** in-memory-only journal file (i.e. is one that was opened with a +ve
** nSpill parameter or as SQLITE_OPEN_MAIN_JOURNAL), and the underlying 
** file has not yet been created, create it now.
*/
ਪੂਰਨ sqlite3JournalCreate(sqlite3_file *pJfd){
  ਪੂਰਨ rc = SQLITE_OK;
  MemJournal *p = (MemJournal*)pJfd;
  ਜੇ( pJfd->pMethods==&MemJournalMethods && (
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ATOMIC_WRITE
     p->nSpill>0
#ਨਹੀਂਤਾਂ
     /* While this appears to not be possible without ATOMIC_WRITE, the
     ** paths are complex, so it seems prudent to leave the test in as
     ** a NEVER(), in case our analysis is subtly flawed. */
     NEVER(p->nSpill>0)
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_BATCH_ATOMIC_WRITE
     || (p->flags & SQLITE_OPEN_MAIN_JOURNAL)
#ਜੇ_ਅੰਤ
  )){
    rc = memjrnlCreateFile(p);
  }
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ

/*
** The file-handle passed as the only argument is open on a journal file.
** Return true if this "journal file" is currently stored in heap memory,
** or false otherwise.
*/
ਪੂਰਨ sqlite3_Journal_Is_ਵਿੱਚ_ਸਮ੍ਰਿਤੀ(sqlite3_file *p){
  ਵਾਪਸ p->pMethods==&MemJournalMethods;
}

/* 
** Return the number of bytes required to store a JournalFile that uses vfs
** pVfs to create the underlying on-disk files.
*/
ਪੂਰਨ sqlite3_Journal_ਮਾਪ(sqlite3_vfs *pVfs){
  ਵਾਪਸ MAX(pVfs->szOsFile, (ਪੂਰਨ)ਆਕਾਰ(MemJournal));
}
