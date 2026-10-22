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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/* Forward references to internal structures */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ MemJournal MemJournal;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ File_ಬಿಂದು File_ಬಿಂದು;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ FileChunk FileChunk;

/*
** The rollback journal is composed of a linked list of these structures.
**
** The zChunk array is always at least 8 bytes in size - usually much more.
** Its actual size is stored in the MemJournal.nChunkSize variable.
*/
ರಚನೆ FileChunk {
  FileChunk *p_ಮುಂದಿನ;               /* Next chunk in the journal */
  u8 zChunk[8];                   /* Content of this chunk */
};

/*
** By default, allocate this many bytes of memory for each FileChunk object.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MEMJOURNAL_DFLT_FILECHUNKSIZE 1024

/*
** For chunk size nChunkSize, return the number of bytes that should
** be allocated for each FileChunk structure.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು file_Chunk_ಗಾತ್ರ(n_Chunk_ಗಾತ್ರ) (ಗಾತ್ರ(FileChunk) + ((n_Chunk_ಗಾತ್ರ)-8))

/*
** An instance of this object serves as a cursor into the rollback journal.
** The cursor can be either for reading or writing.
*/
ರಚನೆ File_ಬಿಂದು {
  sqlite3_int64 i_ಸ್ಥಾನಾಂತರ;          /* Offset from the beginning of the file */
  FileChunk *pChunk;              /* Specific chunk into which cursor points */
};

/*
** This structure is a subclass of sqlite3_file. Each open memory-journal
** is an instance of this class.
*/
ರಚನೆ MemJournal {
  ಸ್ಥಿರಾಂಕ sqlite3_io_methods *p_ವಿಧಾನ; /* Parent class. MUST BE FIRST */
  ಪೂರ್ಣಾಂಕ n_Chunk_ಗಾತ್ರ;                 /* In-memory chunk-size */

  ಪೂರ್ಣಾಂಕ nSpill;                     /* Bytes of data before flushing */
  FileChunk *p_ಮೊದಲ;              /* Head of in-memory chunk-list */
  File_ಬಿಂದು endpoint;             /* Pointer to the end of the file */
  File_ಬಿಂದು readpoint;            /* Pointer to the end of the last xRead() */

  ಪೂರ್ಣಾಂಕ flags;                      /* xOpen flags */
  sqlite3_vfs *pVfs;              /* The "real" underlying VFS */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zJournal;           /* Name of the journal file */
};

/*
** Read data from the in-memory journal file.  This is the implementation
** of the sqlite3_vfs.xRead method.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memjrnlRead(
  sqlite3_file *pJfd,    /* The journal file from which to read */
  ಶೂನ್ಯ *zBuf,            /* Put the results here */
  ಪೂರ್ಣಾಂಕ iAmt,              /* Number of bytes to read */
  sqlite_int64 iOfst     /* Begin reading at this offset */
){
  MemJournal *p = (MemJournal *)pJfd;
  u8 *z_ಹೊರಹೋಗು = zBuf;
  ಪೂರ್ಣಾಂಕ nRead = iAmt;
  ಪೂರ್ಣಾಂಕ i_Chunk_ಸ್ಥಾನಾಂತರ;
  FileChunk *pChunk;

  ಆದರೆ( (iAmt+iOfst)>p->endpoint.i_ಸ್ಥಾನಾಂತರ ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_SHORT_READ;
  }
  ಪ್ರತಿಜ್ಞಾ( p->readpoint.i_ಸ್ಥಾನಾಂತರ==0 || p->readpoint.pChunk!=0 );
  ಆದರೆ( p->readpoint.i_ಸ್ಥಾನಾಂತರ!=iOfst || iOfst==0 ){
    sqlite3_int64 iOff = 0;
    ಗಾಗಿ(pChunk=p->p_ಮೊದಲ; 
        ALWAYS(pChunk) && (iOff+p->n_Chunk_ಗಾತ್ರ)<=iOfst;
        pChunk=pChunk->p_ಮುಂದಿನ
    ){
      iOff += p->n_Chunk_ಗಾತ್ರ;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    pChunk = p->readpoint.pChunk;
    ಪ್ರತಿಜ್ಞಾ( pChunk!=0 );
  }

  i_Chunk_ಸ್ಥಾನಾಂತರ = (ಪೂರ್ಣಾಂಕ)(iOfst%p->n_Chunk_ಗಾತ್ರ);
  ಮಾಡು {
    ಪೂರ್ಣಾಂಕ iSpace = p->n_Chunk_ಗಾತ್ರ - i_Chunk_ಸ್ಥಾನಾಂತರ;
    ಪೂರ್ಣಾಂಕ nCopy = MIN(nRead, (p->n_Chunk_ಗಾತ್ರ - i_Chunk_ಸ್ಥಾನಾಂತರ));
    ಸ್ಮರಣೆ_ನಕಲು(z_ಹೊರಹೋಗು, (u8*)pChunk->zChunk + i_Chunk_ಸ್ಥಾನಾಂತರ, nCopy);
    z_ಹೊರಹೋಗು += nCopy;
    nRead -= iSpace;
    i_Chunk_ಸ್ಥಾನಾಂತರ = 0;
  } ಆಗ( nRead>=0 && (pChunk=pChunk->p_ಮುಂದಿನ)!=0 && nRead>0 );
  p->readpoint.i_ಸ್ಥಾನಾಂತರ = pChunk ? iOfst+iAmt : 0;
  p->readpoint.pChunk = pChunk;

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Free the list of FileChunk structures headed at MemJournal.pFirst.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memjrnlFreeChunks(FileChunk *p_ಮೊದಲ){
  FileChunk *pIter;
  FileChunk *p_ಮುಂದಿನ;
  ಗಾಗಿ(pIter=p_ಮೊದಲ; pIter; pIter=p_ಮುಂದಿನ){
    p_ಮುಂದಿನ = pIter->p_ಮುಂದಿನ;
    sqlite3_free(pIter);
  } 
}

/*
** Flush the contents of memory to a real file on disk.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memjrnlCreateFile(MemJournal *p){
  ಪೂರ್ಣಾಂಕ rc;
  sqlite3_file *pReal = (sqlite3_file*)p;
  MemJournal copy = *p;

  memset(p, 0, ಗಾತ್ರ(MemJournal));
  rc = sqlite3OsOpen(copy.pVfs, copy.zJournal, pReal, copy.flags, 0);
  ಆದರೆ( rc==SQLITE_OK ){
    ಪೂರ್ಣಾಂಕ nChunk = copy.n_Chunk_ಗಾತ್ರ;
    i64 iOff = 0;
    FileChunk *pIter;
    ಗಾಗಿ(pIter=copy.p_ಮೊದಲ; pIter; pIter=pIter->p_ಮುಂದಿನ){
      ಆದರೆ( iOff + nChunk > copy.endpoint.i_ಸ್ಥಾನಾಂತರ ){
        nChunk = copy.endpoint.i_ಸ್ಥಾನಾಂತರ - iOff;
      }
      rc = sqlite3OsWrite(pReal, (u8*)pIter->zChunk, nChunk, iOff);
      ಆದರೆ( rc ) ನಿಲ್ಲಿಸು;
      iOff += nChunk;
    }
    ಆದರೆ( rc==SQLITE_OK ){
      /* No error has occurred. Free the in-memory buffers. */
      memjrnlFreeChunks(copy.p_ಮೊದಲ);
    }
  }
  ಆದರೆ( rc!=SQLITE_OK ){
    /* If an error occurred while creating or writing to the file, restore
    ** the original before returning. This way, SQLite uses the in-memory
    ** journal data to roll back changes made to the internal page-cache
    ** before this function was called.  */
    sqlite3OsClose(pReal);
    *p = copy;
  }
  ಹಿಂತಿರುಗಿ rc;
}


/* Forward reference */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 ಗಾತ್ರ_2);

/*
** Write data to the file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memjrnlWrite(
  sqlite3_file *pJfd,    /* The journal file into which to write */
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zBuf,      /* Take data to be written from here */
  ಪೂರ್ಣಾಂಕ iAmt,              /* Number of bytes to write */
  sqlite_int64 iOfst     /* Begin writing at this offset into the file */
){
  MemJournal *p = (MemJournal *)pJfd;
  ಪೂರ್ಣಾಂಕ nWrite = iAmt;
  u8 *zWrite = (u8 *)zBuf;

  /* If the file should be created now, create it and write the new data
  ** into the file on disk. */
  ಆದರೆ( p->nSpill>0 && (iAmt+iOfst)>p->nSpill ){
    ಪೂರ್ಣಾಂಕ rc = memjrnlCreateFile(p);
    ಆದರೆ( rc==SQLITE_OK ){
      rc = sqlite3OsWrite(pJfd, zBuf, iAmt, iOfst);
    }
    ಹಿಂತಿರುಗಿ rc;
  }

  /* If the contents of this write should be stored in memory */
  ಇಲ್ಲದಿದ್ದರೆ{
    /* An in-memory journal file should only ever be appended to. Random
    ** access writes are not required. The only exception to this is when
    ** the in-memory journal is being used by a connection using the
    ** atomic-write optimization. In this case the first 28 bytes of the
    ** journal file may be written as part of committing the transaction. */
    ಪ್ರತಿಜ್ಞಾ( iOfst<=p->endpoint.i_ಸ್ಥಾನಾಂತರ );
    ಆದರೆ( iOfst>0 && iOfst!=p->endpoint.i_ಸ್ಥಾನಾಂತರ ){
      memjrnlTruncate(pJfd, iOfst);
    }
    ಆದರೆ( iOfst==0 && p->p_ಮೊದಲ ){
      ಪ್ರತಿಜ್ಞಾ( p->n_Chunk_ಗಾತ್ರ>iAmt );
      ಸ್ಮರಣೆ_ನಕಲು((u8*)p->p_ಮೊದಲ->zChunk, zBuf, iAmt);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆಗ( nWrite>0 ){
        FileChunk *pChunk = p->endpoint.pChunk;
        ಪೂರ್ಣಾಂಕ i_Chunk_ಸ್ಥಾನಾಂತರ = (ಪೂರ್ಣಾಂಕ)(p->endpoint.i_ಸ್ಥಾನಾಂತರ%p->n_Chunk_ಗಾತ್ರ);
        ಪೂರ್ಣಾಂಕ iSpace = MIN(nWrite, p->n_Chunk_ಗಾತ್ರ - i_Chunk_ಸ್ಥಾನಾಂತರ);

        ಪ್ರತಿಜ್ಞಾ( pChunk!=0 || i_Chunk_ಸ್ಥಾನಾಂತರ==0 );
        ಆದರೆ( i_Chunk_ಸ್ಥಾನಾಂತರ==0 ){
          /* New chunk is required to extend the file. */
          FileChunk *pNew = sqlite3_malloc(file_Chunk_ಗಾತ್ರ(p->n_Chunk_ಗಾತ್ರ));
          ಆದರೆ( !pNew ){
            ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM_BKPT;
          }
          pNew->p_ಮುಂದಿನ = 0;
          ಆದರೆ( pChunk ){
            ಪ್ರತಿಜ್ಞಾ( p->p_ಮೊದಲ );
            pChunk->p_ಮುಂದಿನ = pNew;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಪ್ರತಿಜ್ಞಾ( !p->p_ಮೊದಲ );
            p->p_ಮೊದಲ = pNew;
          }
          pChunk = p->endpoint.pChunk = pNew;
        }

        ಪ್ರತಿಜ್ಞಾ( pChunk!=0 );
        ಸ್ಮರಣೆ_ನಕಲು((u8*)pChunk->zChunk + i_Chunk_ಸ್ಥಾನಾಂತರ, zWrite, iSpace);
        zWrite += iSpace;
        nWrite -= iSpace;
        p->endpoint.i_ಸ್ಥಾನಾಂತರ += iSpace;
      }
    }
  }

  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Truncate the in-memory file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 ಗಾತ್ರ_2){
  MemJournal *p = (MemJournal *)pJfd;
  ಪ್ರತಿಜ್ಞಾ( p->endpoint.pChunk==0 || p->endpoint.pChunk->p_ಮುಂದಿನ==0 );
  ಆದರೆ( ಗಾತ್ರ_2<p->endpoint.i_ಸ್ಥಾನಾಂತರ ){
    FileChunk *pIter = 0;
    ಆದರೆ( ಗಾತ್ರ_2==0 ){
      memjrnlFreeChunks(p->p_ಮೊದಲ);
      p->p_ಮೊದಲ = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      i64 iOff = p->n_Chunk_ಗಾತ್ರ;
      ಗಾಗಿ(pIter=p->p_ಮೊದಲ; ALWAYS(pIter) && iOff<ಗಾತ್ರ_2; pIter=pIter->p_ಮುಂದಿನ){
        iOff += p->n_Chunk_ಗಾತ್ರ;
      }
      ಆದರೆ( ALWAYS(pIter) ){
        memjrnlFreeChunks(pIter->p_ಮುಂದಿನ);
        pIter->p_ಮುಂದಿನ = 0;
      }
    }

    p->endpoint.pChunk = pIter;
    p->endpoint.i_ಸ್ಥಾನಾಂತರ = ಗಾತ್ರ_2;
    p->readpoint.pChunk = 0;
    p->readpoint.i_ಸ್ಥಾನಾಂತರ = 0;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Close the file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memjrnlClose(sqlite3_file *pJfd){
  MemJournal *p = (MemJournal *)pJfd;
  memjrnlFreeChunks(p->p_ಮೊದಲ);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Sync the file.
**
** If the real file has been created, call its xSync method. Otherwise, 
** syncing an in-memory journal is a no-op. 
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memjrnlSync(sqlite3_file *pJfd, ಪೂರ್ಣಾಂಕ flags){
  ಬಳಸದ_PARAMETER2(pJfd, flags);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Query the size of the file in bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memjrnl_File_ಗಾತ್ರ(sqlite3_file *pJfd, sqlite_int64 *p_ಗಾತ್ರ){
  MemJournal *p = (MemJournal *)pJfd;
  *p_ಗಾತ್ರ = (sqlite_int64) p->endpoint.i_ಸ್ಥಾನಾಂತರ;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Table of methods for MemJournal sqlite3_file object.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ sqlite3_io_methods MemJournalMethods = {
  1,                /* iVersion */
  memjrnlClose,     /* xClose */
  memjrnlRead,      /* xRead */
  memjrnlWrite,     /* xWrite */
  memjrnlTruncate,  /* xTruncate */
  memjrnlSync,      /* xSync */
  memjrnl_File_ಗಾತ್ರ,  /* xFileSize */
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
ಪೂರ್ಣಾಂಕ sqlite3JournalOpen(
  sqlite3_vfs *pVfs,         /* The VFS to use for actual file I/O */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,         /* Name of the journal file */
  sqlite3_file *pJfd,        /* Preallocated, blank file handle */
  ಪೂರ್ಣಾಂಕ flags,                 /* Opening flags */
  ಪೂರ್ಣಾಂಕ nSpill                 /* Bytes buffered before opening the file */
){
  MemJournal *p = (MemJournal*)pJfd;

  ಪ್ರತಿಜ್ಞಾ( zName || nSpill<0 || (flags & SQLITE_OPEN_EXCLUSIVE) );

  /* Zero the file-handle object. If nSpill was passed zero, initialize
  ** it using the sqlite3OsOpen() function of the underlying VFS. In this
  ** case none of the code in this module is executed as a result of calls
  ** made on the journal file-handle.  */
  memset(p, 0, ಗಾತ್ರ(MemJournal));
  ಆದರೆ( nSpill==0 ){
    ಹಿಂತಿರುಗಿ sqlite3OsOpen(pVfs, zName, pJfd, flags, 0);
  }

  ಆದರೆ( nSpill>0 ){
    p->n_Chunk_ಗಾತ್ರ = nSpill;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->n_Chunk_ಗಾತ್ರ = 8 + MEMJOURNAL_DFLT_FILECHUNKSIZE - ಗಾತ್ರ(FileChunk);
    ಪ್ರತಿಜ್ಞಾ( MEMJOURNAL_DFLT_FILECHUNKSIZE==file_Chunk_ಗಾತ್ರ(p->n_Chunk_ಗಾತ್ರ) );
  }

  pJfd->pMethods = (ಸ್ಥಿರಾಂಕ sqlite3_io_methods*)&MemJournalMethods;
  p->nSpill = nSpill;
  p->flags = flags;
  p->zJournal = zName;
  p->pVfs = pVfs;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Open an in-memory journal file.
*/
ಶೂನ್ಯ sqlite3MemJournalOpen(sqlite3_file *pJfd){
  sqlite3JournalOpen(0, 0, pJfd, 0, -1);
}

#ಆದರೆ defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
/*
** If the argument p points to a MemJournal structure that is not an 
** in-memory-only journal file (i.e. is one that was opened with a +ve
** nSpill parameter or as SQLITE_OPEN_MAIN_JOURNAL), and the underlying 
** file has not yet been created, create it now.
*/
ಪೂರ್ಣಾಂಕ sqlite3JournalCreate(sqlite3_file *pJfd){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  MemJournal *p = (MemJournal*)pJfd;
  ಆದರೆ( pJfd->pMethods==&MemJournalMethods && (
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ATOMIC_WRITE
     p->nSpill>0
#ಇಲ್ಲದಿದ್ದರೆ
     /* While this appears to not be possible without ATOMIC_WRITE, the
     ** paths are complex, so it seems prudent to leave the test in as
     ** a NEVER(), in case our analysis is subtly flawed. */
     NEVER(p->nSpill>0)
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_BATCH_ATOMIC_WRITE
     || (p->flags & SQLITE_OPEN_MAIN_JOURNAL)
#ಆದರೆ_ಅಂತ್ಯ
  )){
    rc = memjrnlCreateFile(p);
  }
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** The file-handle passed as the only argument is open on a journal file.
** Return true if this "journal file" is currently stored in heap memory,
** or false otherwise.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Journal_Is_ಒಳ_ಸ್ಮರಣೆ(sqlite3_file *p){
  ಹಿಂತಿರುಗಿ p->pMethods==&MemJournalMethods;
}

/* 
** Return the number of bytes required to store a JournalFile that uses vfs
** pVfs to create the underlying on-disk files.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Journal_ಗಾತ್ರ(sqlite3_vfs *pVfs){
  ಹಿಂತಿರುಗಿ MAX(pVfs->szOsFile, (ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(MemJournal));
}
