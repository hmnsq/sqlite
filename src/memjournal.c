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
#समावेश "sqliteInt.h"

/* Forward references to internal structures */
प्रकार काष्ठा MemJournal MemJournal;
प्रकार काष्ठा FilePoint FilePoint;
प्रकार काष्ठा FileChunk FileChunk;

/*
** The rollback journal is composed of a linked list of these structures.
**
** The zChunk array is always at least 8 bytes in size - usually much more.
** Its actual size is stored in the MemJournal.nChunkSize variable.
*/
काष्ठा FileChunk {
  FileChunk *p_अग्र;               /* Next chunk in the journal */
  u8 zChunk[8];                   /* Content of this chunk */
};

/*
** By default, allocate this many bytes of memory for each FileChunk object.
*/
#घोषणा MEMJOURNAL_DFLT_FILECHUNKSIZE 1024

/*
** For chunk size nChunkSize, return the number of bytes that should
** be allocated for each FileChunk structure.
*/
#घोषणा file_Chunk_आकार(n_Chunk_आकार) (माप(FileChunk) + ((n_Chunk_आकार)-8))

/*
** An instance of this object serves as a cursor into the rollback journal.
** The cursor can be either for reading or writing.
*/
काष्ठा FilePoint {
  sqlite3_int64 iOffset;          /* Offset from the beginning of the file */
  FileChunk *pChunk;              /* Specific chunk into which cursor points */
};

/*
** This structure is a subclass of sqlite3_file. Each open memory-journal
** is an instance of this class.
*/
काष्ठा MemJournal {
  स्थिर sqlite3_io_methods *pMethod; /* Parent class. MUST BE FIRST */
  पूर्णांक n_Chunk_आकार;                 /* In-memory chunk-size */

  पूर्णांक nSpill;                     /* Bytes of data before flushing */
  FileChunk *pFirst;              /* Head of in-memory chunk-list */
  FilePoint endpoint;             /* Pointer to the end of the file */
  FilePoint readpoint;            /* Pointer to the end of the last xRead() */

  पूर्णांक flags;                      /* xOpen flags */
  sqlite3_vfs *pVfs;              /* The "real" underlying VFS */
  स्थिर अक्षर *zJournal;           /* Name of the journal file */
};

/*
** Read data from the in-memory journal file.  This is the implementation
** of the sqlite3_vfs.xRead method.
*/
अटल पूर्णांक memjrnlRead(
  sqlite3_file *pJfd,    /* The journal file from which to read */
  व्योम *zBuf,            /* Put the results here */
  पूर्णांक iAmt,              /* Number of bytes to read */
  sqlite_int64 iOfst     /* Begin reading at this offset */
){
  MemJournal *p = (MemJournal *)pJfd;
  u8 *zOut = zBuf;
  पूर्णांक nRead = iAmt;
  पूर्णांक iChunkOffset;
  FileChunk *pChunk;

  अगर( (iAmt+iOfst)>p->endpoint.iOffset ){
    वापस SQLITE_IOERR_SHORT_READ;
  }
  निश्चित( p->readpoint.iOffset==0 || p->readpoint.pChunk!=0 );
  अगर( p->readpoint.iOffset!=iOfst || iOfst==0 ){
    sqlite3_int64 iOff = 0;
    क्रम(pChunk=p->pFirst; 
        ALWAYS(pChunk) && (iOff+p->n_Chunk_आकार)<=iOfst;
        pChunk=pChunk->p_अग्र
    ){
      iOff += p->n_Chunk_आकार;
    }
  }अन्यथा{
    pChunk = p->readpoint.pChunk;
    निश्चित( pChunk!=0 );
  }

  iChunkOffset = (पूर्णांक)(iOfst%p->n_Chunk_आकार);
  करो {
    पूर्णांक iSpace = p->n_Chunk_आकार - iChunkOffset;
    पूर्णांक nCopy = MIN(nRead, (p->n_Chunk_आकार - iChunkOffset));
    स_नकल(zOut, (u8*)pChunk->zChunk + iChunkOffset, nCopy);
    zOut += nCopy;
    nRead -= iSpace;
    iChunkOffset = 0;
  } जबतक( nRead>=0 && (pChunk=pChunk->p_अग्र)!=0 && nRead>0 );
  p->readpoint.iOffset = pChunk ? iOfst+iAmt : 0;
  p->readpoint.pChunk = pChunk;

  वापस SQLITE_OK;
}

/*
** Free the list of FileChunk structures headed at MemJournal.pFirst.
*/
अटल व्योम memjrnlFreeChunks(FileChunk *pFirst){
  FileChunk *pIter;
  FileChunk *p_अग्र;
  क्रम(pIter=pFirst; pIter; pIter=p_अग्र){
    p_अग्र = pIter->p_अग्र;
    sqlite3_free(pIter);
  } 
}

/*
** Flush the contents of memory to a real file on disk.
*/
अटल पूर्णांक memjrnlCreateFile(MemJournal *p){
  पूर्णांक rc;
  sqlite3_file *pReal = (sqlite3_file*)p;
  MemJournal copy = *p;

  स_रखो(p, 0, माप(MemJournal));
  rc = sqlite3OsOpen(copy.pVfs, copy.zJournal, pReal, copy.flags, 0);
  अगर( rc==SQLITE_OK ){
    पूर्णांक nChunk = copy.n_Chunk_आकार;
    i64 iOff = 0;
    FileChunk *pIter;
    क्रम(pIter=copy.pFirst; pIter; pIter=pIter->p_अग्र){
      अगर( iOff + nChunk > copy.endpoint.iOffset ){
        nChunk = copy.endpoint.iOffset - iOff;
      }
      rc = sqlite3OsWrite(pReal, (u8*)pIter->zChunk, nChunk, iOff);
      अगर( rc ) अवरोध;
      iOff += nChunk;
    }
    अगर( rc==SQLITE_OK ){
      /* No error has occurred. Free the in-memory buffers. */
      memjrnlFreeChunks(copy.pFirst);
    }
  }
  अगर( rc!=SQLITE_OK ){
    /* If an error occurred while creating or writing to the file, restore
    ** the original before returning. This way, SQLite uses the in-memory
    ** journal data to roll back changes made to the internal page-cache
    ** before this function was called.  */
    sqlite3OsClose(pReal);
    *p = copy;
  }
  वापस rc;
}


/* Forward reference */
अटल पूर्णांक memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 आकार);

/*
** Write data to the file.
*/
अटल पूर्णांक memjrnlWrite(
  sqlite3_file *pJfd,    /* The journal file into which to write */
  स्थिर व्योम *zBuf,      /* Take data to be written from here */
  पूर्णांक iAmt,              /* Number of bytes to write */
  sqlite_int64 iOfst     /* Begin writing at this offset into the file */
){
  MemJournal *p = (MemJournal *)pJfd;
  पूर्णांक nWrite = iAmt;
  u8 *zWrite = (u8 *)zBuf;

  /* If the file should be created now, create it and write the new data
  ** into the file on disk. */
  अगर( p->nSpill>0 && (iAmt+iOfst)>p->nSpill ){
    पूर्णांक rc = memjrnlCreateFile(p);
    अगर( rc==SQLITE_OK ){
      rc = sqlite3OsWrite(pJfd, zBuf, iAmt, iOfst);
    }
    वापस rc;
  }

  /* If the contents of this write should be stored in memory */
  अन्यथा{
    /* An in-memory journal file should only ever be appended to. Random
    ** access writes are not required. The only exception to this is when
    ** the in-memory journal is being used by a connection using the
    ** atomic-write optimization. In this case the first 28 bytes of the
    ** journal file may be written as part of committing the transaction. */
    निश्चित( iOfst<=p->endpoint.iOffset );
    अगर( iOfst>0 && iOfst!=p->endpoint.iOffset ){
      memjrnlTruncate(pJfd, iOfst);
    }
    अगर( iOfst==0 && p->pFirst ){
      निश्चित( p->n_Chunk_आकार>iAmt );
      स_नकल((u8*)p->pFirst->zChunk, zBuf, iAmt);
    }अन्यथा{
      जबतक( nWrite>0 ){
        FileChunk *pChunk = p->endpoint.pChunk;
        पूर्णांक iChunkOffset = (पूर्णांक)(p->endpoint.iOffset%p->n_Chunk_आकार);
        पूर्णांक iSpace = MIN(nWrite, p->n_Chunk_आकार - iChunkOffset);

        निश्चित( pChunk!=0 || iChunkOffset==0 );
        अगर( iChunkOffset==0 ){
          /* New chunk is required to extend the file. */
          FileChunk *pNew = sqlite3_malloc(file_Chunk_आकार(p->n_Chunk_आकार));
          अगर( !pNew ){
            वापस SQLITE_IOERR_NOMEM_BKPT;
          }
          pNew->p_अग्र = 0;
          अगर( pChunk ){
            निश्चित( p->pFirst );
            pChunk->p_अग्र = pNew;
          }अन्यथा{
            निश्चित( !p->pFirst );
            p->pFirst = pNew;
          }
          pChunk = p->endpoint.pChunk = pNew;
        }

        निश्चित( pChunk!=0 );
        स_नकल((u8*)pChunk->zChunk + iChunkOffset, zWrite, iSpace);
        zWrite += iSpace;
        nWrite -= iSpace;
        p->endpoint.iOffset += iSpace;
      }
    }
  }

  वापस SQLITE_OK;
}

/*
** Truncate the in-memory file.
*/
अटल पूर्णांक memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 आकार){
  MemJournal *p = (MemJournal *)pJfd;
  निश्चित( p->endpoint.pChunk==0 || p->endpoint.pChunk->p_अग्र==0 );
  अगर( आकार<p->endpoint.iOffset ){
    FileChunk *pIter = 0;
    अगर( आकार==0 ){
      memjrnlFreeChunks(p->pFirst);
      p->pFirst = 0;
    }अन्यथा{
      i64 iOff = p->n_Chunk_आकार;
      क्रम(pIter=p->pFirst; ALWAYS(pIter) && iOff<आकार; pIter=pIter->p_अग्र){
        iOff += p->n_Chunk_आकार;
      }
      अगर( ALWAYS(pIter) ){
        memjrnlFreeChunks(pIter->p_अग्र);
        pIter->p_अग्र = 0;
      }
    }

    p->endpoint.pChunk = pIter;
    p->endpoint.iOffset = आकार;
    p->readpoint.pChunk = 0;
    p->readpoint.iOffset = 0;
  }
  वापस SQLITE_OK;
}

/*
** Close the file.
*/
अटल पूर्णांक memjrnlClose(sqlite3_file *pJfd){
  MemJournal *p = (MemJournal *)pJfd;
  memjrnlFreeChunks(p->pFirst);
  वापस SQLITE_OK;
}

/*
** Sync the file.
**
** If the real file has been created, call its xSync method. Otherwise, 
** syncing an in-memory journal is a no-op. 
*/
अटल पूर्णांक memjrnlSync(sqlite3_file *pJfd, पूर्णांक flags){
  UNUSED_PARAMETER2(pJfd, flags);
  वापस SQLITE_OK;
}

/*
** Query the size of the file in bytes.
*/
अटल पूर्णांक memjrnl_File_आकार(sqlite3_file *pJfd, sqlite_int64 *p_आकार){
  MemJournal *p = (MemJournal *)pJfd;
  *p_आकार = (sqlite_int64) p->endpoint.iOffset;
  वापस SQLITE_OK;
}

/*
** Table of methods for MemJournal sqlite3_file object.
*/
अटल स्थिर काष्ठा sqlite3_io_methods MemJournalMethods = {
  1,                /* iVersion */
  memjrnlClose,     /* xClose */
  memjrnlRead,      /* xRead */
  memjrnlWrite,     /* xWrite */
  memjrnlTruncate,  /* xTruncate */
  memjrnlSync,      /* xSync */
  memjrnl_File_आकार,  /* xFileSize */
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
पूर्णांक sqlite3JournalOpen(
  sqlite3_vfs *pVfs,         /* The VFS to use for actual file I/O */
  स्थिर अक्षर *zName,         /* Name of the journal file */
  sqlite3_file *pJfd,        /* Preallocated, blank file handle */
  पूर्णांक flags,                 /* Opening flags */
  पूर्णांक nSpill                 /* Bytes buffered before opening the file */
){
  MemJournal *p = (MemJournal*)pJfd;

  निश्चित( zName || nSpill<0 || (flags & SQLITE_OPEN_EXCLUSIVE) );

  /* Zero the file-handle object. If nSpill was passed zero, initialize
  ** it using the sqlite3OsOpen() function of the underlying VFS. In this
  ** case none of the code in this module is executed as a result of calls
  ** made on the journal file-handle.  */
  स_रखो(p, 0, माप(MemJournal));
  अगर( nSpill==0 ){
    वापस sqlite3OsOpen(pVfs, zName, pJfd, flags, 0);
  }

  अगर( nSpill>0 ){
    p->n_Chunk_आकार = nSpill;
  }अन्यथा{
    p->n_Chunk_आकार = 8 + MEMJOURNAL_DFLT_FILECHUNKSIZE - माप(FileChunk);
    निश्चित( MEMJOURNAL_DFLT_FILECHUNKSIZE==file_Chunk_आकार(p->n_Chunk_आकार) );
  }

  pJfd->pMethods = (स्थिर sqlite3_io_methods*)&MemJournalMethods;
  p->nSpill = nSpill;
  p->flags = flags;
  p->zJournal = zName;
  p->pVfs = pVfs;
  वापस SQLITE_OK;
}

/*
** Open an in-memory journal file.
*/
व्योम sqlite3MemJournalOpen(sqlite3_file *pJfd){
  sqlite3JournalOpen(0, 0, pJfd, 0, -1);
}

#अगर defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
/*
** If the argument p points to a MemJournal structure that is not an 
** in-memory-only journal file (i.e. is one that was opened with a +ve
** nSpill parameter or as SQLITE_OPEN_MAIN_JOURNAL), and the underlying 
** file has not yet been created, create it now.
*/
पूर्णांक sqlite3JournalCreate(sqlite3_file *pJfd){
  पूर्णांक rc = SQLITE_OK;
  MemJournal *p = (MemJournal*)pJfd;
  अगर( pJfd->pMethods==&MemJournalMethods && (
#अगर_घोषित SQLITE_ENABLE_ATOMIC_WRITE
     p->nSpill>0
#अन्यथा
     /* While this appears to not be possible without ATOMIC_WRITE, the
     ** paths are complex, so it seems prudent to leave the test in as
     ** a NEVER(), in case our analysis is subtly flawed. */
     NEVER(p->nSpill>0)
#पूर्ण_अगर
#अगर_घोषित SQLITE_ENABLE_BATCH_ATOMIC_WRITE
     || (p->flags & SQLITE_OPEN_MAIN_JOURNAL)
#पूर्ण_अगर
  )){
    rc = memjrnlCreateFile(p);
  }
  वापस rc;
}
#पूर्ण_अगर

/*
** The file-handle passed as the only argument is open on a journal file.
** Return true if this "journal file" is currently stored in heap memory,
** or false otherwise.
*/
पूर्णांक sqlite3JournalIsInMemory(sqlite3_file *p){
  वापस p->pMethods==&MemJournalMethods;
}

/* 
** Return the number of bytes required to store a JournalFile that uses vfs
** pVfs to create the underlying on-disk files.
*/
पूर्णांक sqlite3_Journal_आकार(sqlite3_vfs *pVfs){
  वापस MAX(pVfs->szOsFile, (पूर्णांक)माप(MemJournal));
}
