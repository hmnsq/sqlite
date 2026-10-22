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
#ضمن "sqliteInt.h"

/* Forward references to internal structures */
تعريفنوع بنية MemJournal MemJournal;
تعريفنوع بنية File_نقطو File_نقطو;
تعريفنوع بنية FileChunk FileChunk;

/*
** The rollback journal is composed of a linked list of these structures.
**
** The zChunk array is always at least 8 bytes in size - usually much more.
** Its actual size is stored in the MemJournal.nChunkSize variable.
*/
بنية FileChunk {
  FileChunk *p_ايندڙ;               /* Next chunk in the journal */
  u8 zChunk[8];                   /* Content of this chunk */
};

/*
** By default, allocate this many bytes of memory for each FileChunk object.
*/
#وصف_ڪريو MEMJOURNAL_DFLT_FILECHUNKSIZE 1024

/*
** For chunk size nChunkSize, return the number of bytes that should
** be allocated for each FileChunk structure.
*/
#وصف_ڪريو file_Chunk_سائيز(n_Chunk_سائيز) (حجم(FileChunk) + ((n_Chunk_سائيز)-8))

/*
** An instance of this object serves as a cursor into the rollback journal.
** The cursor can be either for reading or writing.
*/
بنية File_نقطو {
  sqlite3_int64 i_وٿي;          /* Offset from the beginning of the file */
  FileChunk *pChunk;              /* Specific chunk into which cursor points */
};

/*
** This structure is a subclass of sqlite3_file. Each open memory-journal
** is an instance of this class.
*/
بنية MemJournal {
  ثابت sqlite3_io_methods *p_طريقو; /* Parent class. MUST BE FIRST */
  صحيح n_Chunk_سائيز;                 /* In-memory chunk-size */

  صحيح nSpill;                     /* Bytes of data before flushing */
  FileChunk *p_پهريون;              /* Head of in-memory chunk-list */
  File_نقطو endpoint;             /* Pointer to the end of the file */
  File_نقطو readpoint;            /* Pointer to the end of the last xRead() */

  صحيح flags;                      /* xOpen flags */
  sqlite3_vfs *pVfs;              /* The "real" underlying VFS */
  ثابت حرف *zJournal;           /* Name of the journal file */
};

/*
** Read data from the in-memory journal file.  This is the implementation
** of the sqlite3_vfs.xRead method.
*/
ساكن صحيح memjrnlRead(
  sqlite3_file *pJfd,    /* The journal file from which to read */
  فراغ *zBuf,            /* Put the results here */
  صحيح iAmt,              /* Number of bytes to read */
  sqlite_int64 iOfst     /* Begin reading at this offset */
){
  MemJournal *p = (MemJournal *)pJfd;
  u8 *z_ٻاهريون = zBuf;
  صحيح nRead = iAmt;
  صحيح i_Chunk_وٿي;
  FileChunk *pChunk;

  إذا( (iAmt+iOfst)>p->endpoint.i_وٿي ){
    أرجع SQLITE_IOERR_SHORT_READ;
  }
  أكد( p->readpoint.i_وٿي==0 || p->readpoint.pChunk!=0 );
  إذا( p->readpoint.i_وٿي!=iOfst || iOfst==0 ){
    sqlite3_int64 iOff = 0;
    لكل(pChunk=p->p_پهريون; 
        ALWAYS(pChunk) && (iOff+p->n_Chunk_سائيز)<=iOfst;
        pChunk=pChunk->p_ايندڙ
    ){
      iOff += p->n_Chunk_سائيز;
    }
  }وإلا{
    pChunk = p->readpoint.pChunk;
    أكد( pChunk!=0 );
  }

  i_Chunk_وٿي = (صحيح)(iOfst%p->n_Chunk_سائيز);
  افعل {
    صحيح iSpace = p->n_Chunk_سائيز - i_Chunk_وٿي;
    صحيح nCopy = MIN(nRead, (p->n_Chunk_سائيز - i_Chunk_وٿي));
    يادگيري_نقل(z_ٻاهريون, (u8*)pChunk->zChunk + i_Chunk_وٿي, nCopy);
    z_ٻاهريون += nCopy;
    nRead -= iSpace;
    i_Chunk_وٿي = 0;
  } طالما( nRead>=0 && (pChunk=pChunk->p_ايندڙ)!=0 && nRead>0 );
  p->readpoint.i_وٿي = pChunk ? iOfst+iAmt : 0;
  p->readpoint.pChunk = pChunk;

  أرجع SQLITE_OK;
}

/*
** Free the list of FileChunk structures headed at MemJournal.pFirst.
*/
ساكن فراغ memjrnlFreeChunks(FileChunk *p_پهريون){
  FileChunk *pIter;
  FileChunk *p_ايندڙ;
  لكل(pIter=p_پهريون; pIter; pIter=p_ايندڙ){
    p_ايندڙ = pIter->p_ايندڙ;
    sqlite3_free(pIter);
  } 
}

/*
** Flush the contents of memory to a real file on disk.
*/
ساكن صحيح memjrnlCreateFile(MemJournal *p){
  صحيح rc;
  sqlite3_file *pReal = (sqlite3_file*)p;
  MemJournal copy = *p;

  memset(p, 0, حجم(MemJournal));
  rc = sqlite3OsOpen(copy.pVfs, copy.zJournal, pReal, copy.flags, 0);
  إذا( rc==SQLITE_OK ){
    صحيح nChunk = copy.n_Chunk_سائيز;
    i64 iOff = 0;
    FileChunk *pIter;
    لكل(pIter=copy.p_پهريون; pIter; pIter=pIter->p_ايندڙ){
      إذا( iOff + nChunk > copy.endpoint.i_وٿي ){
        nChunk = copy.endpoint.i_وٿي - iOff;
      }
      rc = sqlite3OsWrite(pReal, (u8*)pIter->zChunk, nChunk, iOff);
      إذا( rc ) اكسر;
      iOff += nChunk;
    }
    إذا( rc==SQLITE_OK ){
      /* No error has occurred. Free the in-memory buffers. */
      memjrnlFreeChunks(copy.p_پهريون);
    }
  }
  إذا( rc!=SQLITE_OK ){
    /* If an error occurred while creating or writing to the file, restore
    ** the original before returning. This way, SQLite uses the in-memory
    ** journal data to roll back changes made to the internal page-cache
    ** before this function was called.  */
    sqlite3OsClose(pReal);
    *p = copy;
  }
  أرجع rc;
}


/* Forward reference */
ساكن صحيح memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 سائيز);

/*
** Write data to the file.
*/
ساكن صحيح memjrnlWrite(
  sqlite3_file *pJfd,    /* The journal file into which to write */
  ثابت فراغ *zBuf,      /* Take data to be written from here */
  صحيح iAmt,              /* Number of bytes to write */
  sqlite_int64 iOfst     /* Begin writing at this offset into the file */
){
  MemJournal *p = (MemJournal *)pJfd;
  صحيح nWrite = iAmt;
  u8 *zWrite = (u8 *)zBuf;

  /* If the file should be created now, create it and write the new data
  ** into the file on disk. */
  إذا( p->nSpill>0 && (iAmt+iOfst)>p->nSpill ){
    صحيح rc = memjrnlCreateFile(p);
    إذا( rc==SQLITE_OK ){
      rc = sqlite3OsWrite(pJfd, zBuf, iAmt, iOfst);
    }
    أرجع rc;
  }

  /* If the contents of this write should be stored in memory */
  وإلا{
    /* An in-memory journal file should only ever be appended to. Random
    ** access writes are not required. The only exception to this is when
    ** the in-memory journal is being used by a connection using the
    ** atomic-write optimization. In this case the first 28 bytes of the
    ** journal file may be written as part of committing the transaction. */
    أكد( iOfst<=p->endpoint.i_وٿي );
    إذا( iOfst>0 && iOfst!=p->endpoint.i_وٿي ){
      memjrnlTruncate(pJfd, iOfst);
    }
    إذا( iOfst==0 && p->p_پهريون ){
      أكد( p->n_Chunk_سائيز>iAmt );
      يادگيري_نقل((u8*)p->p_پهريون->zChunk, zBuf, iAmt);
    }وإلا{
      طالما( nWrite>0 ){
        FileChunk *pChunk = p->endpoint.pChunk;
        صحيح i_Chunk_وٿي = (صحيح)(p->endpoint.i_وٿي%p->n_Chunk_سائيز);
        صحيح iSpace = MIN(nWrite, p->n_Chunk_سائيز - i_Chunk_وٿي);

        أكد( pChunk!=0 || i_Chunk_وٿي==0 );
        إذا( i_Chunk_وٿي==0 ){
          /* New chunk is required to extend the file. */
          FileChunk *pNew = sqlite3_malloc(file_Chunk_سائيز(p->n_Chunk_سائيز));
          إذا( !pNew ){
            أرجع SQLITE_IOERR_NOMEM_BKPT;
          }
          pNew->p_ايندڙ = 0;
          إذا( pChunk ){
            أكد( p->p_پهريون );
            pChunk->p_ايندڙ = pNew;
          }وإلا{
            أكد( !p->p_پهريون );
            p->p_پهريون = pNew;
          }
          pChunk = p->endpoint.pChunk = pNew;
        }

        أكد( pChunk!=0 );
        يادگيري_نقل((u8*)pChunk->zChunk + i_Chunk_وٿي, zWrite, iSpace);
        zWrite += iSpace;
        nWrite -= iSpace;
        p->endpoint.i_وٿي += iSpace;
      }
    }
  }

  أرجع SQLITE_OK;
}

/*
** Truncate the in-memory file.
*/
ساكن صحيح memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 سائيز){
  MemJournal *p = (MemJournal *)pJfd;
  أكد( p->endpoint.pChunk==0 || p->endpoint.pChunk->p_ايندڙ==0 );
  إذا( سائيز<p->endpoint.i_وٿي ){
    FileChunk *pIter = 0;
    إذا( سائيز==0 ){
      memjrnlFreeChunks(p->p_پهريون);
      p->p_پهريون = 0;
    }وإلا{
      i64 iOff = p->n_Chunk_سائيز;
      لكل(pIter=p->p_پهريون; ALWAYS(pIter) && iOff<سائيز; pIter=pIter->p_ايندڙ){
        iOff += p->n_Chunk_سائيز;
      }
      إذا( ALWAYS(pIter) ){
        memjrnlFreeChunks(pIter->p_ايندڙ);
        pIter->p_ايندڙ = 0;
      }
    }

    p->endpoint.pChunk = pIter;
    p->endpoint.i_وٿي = سائيز;
    p->readpoint.pChunk = 0;
    p->readpoint.i_وٿي = 0;
  }
  أرجع SQLITE_OK;
}

/*
** Close the file.
*/
ساكن صحيح memjrnlClose(sqlite3_file *pJfd){
  MemJournal *p = (MemJournal *)pJfd;
  memjrnlFreeChunks(p->p_پهريون);
  أرجع SQLITE_OK;
}

/*
** Sync the file.
**
** If the real file has been created, call its xSync method. Otherwise, 
** syncing an in-memory journal is a no-op. 
*/
ساكن صحيح memjrnlSync(sqlite3_file *pJfd, صحيح flags){
  اڻ_استعمال_ٿيل_PARAMETER2(pJfd, flags);
  أرجع SQLITE_OK;
}

/*
** Query the size of the file in bytes.
*/
ساكن صحيح memjrnl_File_سائيز(sqlite3_file *pJfd, sqlite_int64 *p_سائيز){
  MemJournal *p = (MemJournal *)pJfd;
  *p_سائيز = (sqlite_int64) p->endpoint.i_وٿي;
  أرجع SQLITE_OK;
}

/*
** Table of methods for MemJournal sqlite3_file object.
*/
ساكن ثابت بنية sqlite3_io_methods MemJournalMethods = {
  1,                /* iVersion */
  memjrnlClose,     /* xClose */
  memjrnlRead,      /* xRead */
  memjrnlWrite,     /* xWrite */
  memjrnlTruncate,  /* xTruncate */
  memjrnlSync,      /* xSync */
  memjrnl_File_سائيز,  /* xFileSize */
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
صحيح sqlite3JournalOpen(
  sqlite3_vfs *pVfs,         /* The VFS to use for actual file I/O */
  ثابت حرف *zName,         /* Name of the journal file */
  sqlite3_file *pJfd,        /* Preallocated, blank file handle */
  صحيح flags,                 /* Opening flags */
  صحيح nSpill                 /* Bytes buffered before opening the file */
){
  MemJournal *p = (MemJournal*)pJfd;

  أكد( zName || nSpill<0 || (flags & SQLITE_OPEN_EXCLUSIVE) );

  /* Zero the file-handle object. If nSpill was passed zero, initialize
  ** it using the sqlite3OsOpen() function of the underlying VFS. In this
  ** case none of the code in this module is executed as a result of calls
  ** made on the journal file-handle.  */
  memset(p, 0, حجم(MemJournal));
  إذا( nSpill==0 ){
    أرجع sqlite3OsOpen(pVfs, zName, pJfd, flags, 0);
  }

  إذا( nSpill>0 ){
    p->n_Chunk_سائيز = nSpill;
  }وإلا{
    p->n_Chunk_سائيز = 8 + MEMJOURNAL_DFLT_FILECHUNKSIZE - حجم(FileChunk);
    أكد( MEMJOURNAL_DFLT_FILECHUNKSIZE==file_Chunk_سائيز(p->n_Chunk_سائيز) );
  }

  pJfd->pMethods = (ثابت sqlite3_io_methods*)&MemJournalMethods;
  p->nSpill = nSpill;
  p->flags = flags;
  p->zJournal = zName;
  p->pVfs = pVfs;
  أرجع SQLITE_OK;
}

/*
** Open an in-memory journal file.
*/
فراغ sqlite3MemJournalOpen(sqlite3_file *pJfd){
  sqlite3JournalOpen(0, 0, pJfd, 0, -1);
}

#إذا defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
/*
** If the argument p points to a MemJournal structure that is not an 
** in-memory-only journal file (i.e. is one that was opened with a +ve
** nSpill parameter or as SQLITE_OPEN_MAIN_JOURNAL), and the underlying 
** file has not yet been created, create it now.
*/
صحيح sqlite3JournalCreate(sqlite3_file *pJfd){
  صحيح rc = SQLITE_OK;
  MemJournal *p = (MemJournal*)pJfd;
  إذا( pJfd->pMethods==&MemJournalMethods && (
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_ATOMIC_WRITE
     p->nSpill>0
#وإلا
     /* While this appears to not be possible without ATOMIC_WRITE, the
     ** paths are complex, so it seems prudent to leave the test in as
     ** a NEVER(), in case our analysis is subtly flawed. */
     NEVER(p->nSpill>0)
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_BATCH_ATOMIC_WRITE
     || (p->flags & SQLITE_OPEN_MAIN_JOURNAL)
#جيڪڏهن_پورو
  )){
    rc = memjrnlCreateFile(p);
  }
  أرجع rc;
}
#جيڪڏهن_پورو

/*
** The file-handle passed as the only argument is open on a journal file.
** Return true if this "journal file" is currently stored in heap memory,
** or false otherwise.
*/
صحيح sqlite3_Journal_Is_اندريون_يادگيري(sqlite3_file *p){
  أرجع p->pMethods==&MemJournalMethods;
}

/* 
** Return the number of bytes required to store a JournalFile that uses vfs
** pVfs to create the underlying on-disk files.
*/
صحيح sqlite3_Journal_سائيز(sqlite3_vfs *pVfs){
  أرجع MAX(pVfs->szOsFile, (صحيح)حجم(MemJournal));
}
