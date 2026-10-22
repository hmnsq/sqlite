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
#అన్తర్భావ "sqliteInt.h"

/* Forward references to internal structures */
రకనిర్వచనం నిర్మాణం MemJournal MemJournal;
రకనిర్వచనం నిర్మాణం File_బిందువు File_బిందువు;
రకనిర్వచనం నిర్మాణం FileChunk FileChunk;

/*
** The rollback journal is composed of a linked list of these structures.
**
** The zChunk array is always at least 8 bytes in size - usually much more.
** Its actual size is stored in the MemJournal.nChunkSize variable.
*/
నిర్మాణం FileChunk {
  FileChunk *p_తదుపరి;               /* Next chunk in the journal */
  u8 zChunk[8];                   /* Content of this chunk */
};

/*
** By default, allocate this many bytes of memory for each FileChunk object.
*/
#నిర్వచించు MEMJOURNAL_DFLT_FILECHUNKSIZE 1024

/*
** For chunk size nChunkSize, return the number of bytes that should
** be allocated for each FileChunk structure.
*/
#నిర్వచించు file_Chunk_కొలత(n_Chunk_కొలత) (పరిమాణం(FileChunk) + ((n_Chunk_కొలత)-8))

/*
** An instance of this object serves as a cursor into the rollback journal.
** The cursor can be either for reading or writing.
*/
నిర్మాణం File_బిందువు {
  sqlite3_int64 i_స్థానభ్రంశం;          /* Offset from the beginning of the file */
  FileChunk *pChunk;              /* Specific chunk into which cursor points */
};

/*
** This structure is a subclass of sqlite3_file. Each open memory-journal
** is an instance of this class.
*/
నిర్మాణం MemJournal {
  స్థిరరాశి sqlite3_io_methods *p_పద్ధతి; /* Parent class. MUST BE FIRST */
  పూర్ణాంకం n_Chunk_కొలత;                 /* In-memory chunk-size */

  పూర్ణాంకం nSpill;                     /* Bytes of data before flushing */
  FileChunk *p_మొదటి;              /* Head of in-memory chunk-list */
  File_బిందువు endpoint;             /* Pointer to the end of the file */
  File_బిందువు readpoint;            /* Pointer to the end of the last xRead() */

  పూర్ణాంకం flags;                      /* xOpen flags */
  sqlite3_vfs *pVfs;              /* The "real" underlying VFS */
  స్థిరరాశి అక్షరం *zJournal;           /* Name of the journal file */
};

/*
** Read data from the in-memory journal file.  This is the implementation
** of the sqlite3_vfs.xRead method.
*/
స్థిర పూర్ణాంకం memjrnlRead(
  sqlite3_file *pJfd,    /* The journal file from which to read */
  శూన్య *zBuf,            /* Put the results here */
  పూర్ణాంకం iAmt,              /* Number of bytes to read */
  sqlite_int64 iOfst     /* Begin reading at this offset */
){
  MemJournal *p = (MemJournal *)pJfd;
  u8 *z_బహిర్గతం = zBuf;
  పూర్ణాంకం nRead = iAmt;
  పూర్ణాంకం i_Chunk_స్థానభ్రంశం;
  FileChunk *pChunk;

  ఒకవేళ( (iAmt+iOfst)>p->endpoint.i_స్థానభ్రంశం ){
    తిరిగి SQLITE_IOERR_SHORT_READ;
  }
  ప్రతిజ్ఞా( p->readpoint.i_స్థానభ్రంశం==0 || p->readpoint.pChunk!=0 );
  ఒకవేళ( p->readpoint.i_స్థానభ్రంశం!=iOfst || iOfst==0 ){
    sqlite3_int64 iOff = 0;
    కొరకు(pChunk=p->p_మొదటి; 
        ALWAYS(pChunk) && (iOff+p->n_Chunk_కొలత)<=iOfst;
        pChunk=pChunk->p_తదుపరి
    ){
      iOff += p->n_Chunk_కొలత;
    }
  }లేకపోతే{
    pChunk = p->readpoint.pChunk;
    ప్రతిజ్ఞా( pChunk!=0 );
  }

  i_Chunk_స్థానభ్రంశం = (పూర్ణాంకం)(iOfst%p->n_Chunk_కొలత);
  చేయి {
    పూర్ణాంకం iSpace = p->n_Chunk_కొలత - i_Chunk_స్థానభ్రంశం;
    పూర్ణాంకం nCopy = MIN(nRead, (p->n_Chunk_కొలత - i_Chunk_స్థానభ్రంశం));
    జ్ఞాపకం_నకలు(z_బహిర్గతం, (u8*)pChunk->zChunk + i_Chunk_స్థానభ్రంశం, nCopy);
    z_బహిర్గతం += nCopy;
    nRead -= iSpace;
    i_Chunk_స్థానభ్రంశం = 0;
  } అయితే( nRead>=0 && (pChunk=pChunk->p_తదుపరి)!=0 && nRead>0 );
  p->readpoint.i_స్థానభ్రంశం = pChunk ? iOfst+iAmt : 0;
  p->readpoint.pChunk = pChunk;

  తిరిగి SQLITE_OK;
}

/*
** Free the list of FileChunk structures headed at MemJournal.pFirst.
*/
స్థిర శూన్య memjrnlFreeChunks(FileChunk *p_మొదటి){
  FileChunk *pIter;
  FileChunk *p_తదుపరి;
  కొరకు(pIter=p_మొదటి; pIter; pIter=p_తదుపరి){
    p_తదుపరి = pIter->p_తదుపరి;
    sqlite3_free(pIter);
  } 
}

/*
** Flush the contents of memory to a real file on disk.
*/
స్థిర పూర్ణాంకం memjrnlCreateFile(MemJournal *p){
  పూర్ణాంకం rc;
  sqlite3_file *pReal = (sqlite3_file*)p;
  MemJournal copy = *p;

  memset(p, 0, పరిమాణం(MemJournal));
  rc = sqlite3OsOpen(copy.pVfs, copy.zJournal, pReal, copy.flags, 0);
  ఒకవేళ( rc==SQLITE_OK ){
    పూర్ణాంకం nChunk = copy.n_Chunk_కొలత;
    i64 iOff = 0;
    FileChunk *pIter;
    కొరకు(pIter=copy.p_మొదటి; pIter; pIter=pIter->p_తదుపరి){
      ఒకవేళ( iOff + nChunk > copy.endpoint.i_స్థానభ్రంశం ){
        nChunk = copy.endpoint.i_స్థానభ్రంశం - iOff;
      }
      rc = sqlite3OsWrite(pReal, (u8*)pIter->zChunk, nChunk, iOff);
      ఒకవేళ( rc ) ఆపు;
      iOff += nChunk;
    }
    ఒకవేళ( rc==SQLITE_OK ){
      /* No error has occurred. Free the in-memory buffers. */
      memjrnlFreeChunks(copy.p_మొదటి);
    }
  }
  ఒకవేళ( rc!=SQLITE_OK ){
    /* If an error occurred while creating or writing to the file, restore
    ** the original before returning. This way, SQLite uses the in-memory
    ** journal data to roll back changes made to the internal page-cache
    ** before this function was called.  */
    sqlite3OsClose(pReal);
    *p = copy;
  }
  తిరిగి rc;
}


/* Forward reference */
స్థిర పూర్ణాంకం memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 కొలత);

/*
** Write data to the file.
*/
స్థిర పూర్ణాంకం memjrnlWrite(
  sqlite3_file *pJfd,    /* The journal file into which to write */
  స్థిరరాశి శూన్య *zBuf,      /* Take data to be written from here */
  పూర్ణాంకం iAmt,              /* Number of bytes to write */
  sqlite_int64 iOfst     /* Begin writing at this offset into the file */
){
  MemJournal *p = (MemJournal *)pJfd;
  పూర్ణాంకం nWrite = iAmt;
  u8 *zWrite = (u8 *)zBuf;

  /* If the file should be created now, create it and write the new data
  ** into the file on disk. */
  ఒకవేళ( p->nSpill>0 && (iAmt+iOfst)>p->nSpill ){
    పూర్ణాంకం rc = memjrnlCreateFile(p);
    ఒకవేళ( rc==SQLITE_OK ){
      rc = sqlite3OsWrite(pJfd, zBuf, iAmt, iOfst);
    }
    తిరిగి rc;
  }

  /* If the contents of this write should be stored in memory */
  లేకపోతే{
    /* An in-memory journal file should only ever be appended to. Random
    ** access writes are not required. The only exception to this is when
    ** the in-memory journal is being used by a connection using the
    ** atomic-write optimization. In this case the first 28 bytes of the
    ** journal file may be written as part of committing the transaction. */
    ప్రతిజ్ఞా( iOfst<=p->endpoint.i_స్థానభ్రంశం );
    ఒకవేళ( iOfst>0 && iOfst!=p->endpoint.i_స్థానభ్రంశం ){
      memjrnlTruncate(pJfd, iOfst);
    }
    ఒకవేళ( iOfst==0 && p->p_మొదటి ){
      ప్రతిజ్ఞా( p->n_Chunk_కొలత>iAmt );
      జ్ఞాపకం_నకలు((u8*)p->p_మొదటి->zChunk, zBuf, iAmt);
    }లేకపోతే{
      అయితే( nWrite>0 ){
        FileChunk *pChunk = p->endpoint.pChunk;
        పూర్ణాంకం i_Chunk_స్థానభ్రంశం = (పూర్ణాంకం)(p->endpoint.i_స్థానభ్రంశం%p->n_Chunk_కొలత);
        పూర్ణాంకం iSpace = MIN(nWrite, p->n_Chunk_కొలత - i_Chunk_స్థానభ్రంశం);

        ప్రతిజ్ఞా( pChunk!=0 || i_Chunk_స్థానభ్రంశం==0 );
        ఒకవేళ( i_Chunk_స్థానభ్రంశం==0 ){
          /* New chunk is required to extend the file. */
          FileChunk *pNew = sqlite3_malloc(file_Chunk_కొలత(p->n_Chunk_కొలత));
          ఒకవేళ( !pNew ){
            తిరిగి SQLITE_IOERR_NOMEM_BKPT;
          }
          pNew->p_తదుపరి = 0;
          ఒకవేళ( pChunk ){
            ప్రతిజ్ఞా( p->p_మొదటి );
            pChunk->p_తదుపరి = pNew;
          }లేకపోతే{
            ప్రతిజ్ఞా( !p->p_మొదటి );
            p->p_మొదటి = pNew;
          }
          pChunk = p->endpoint.pChunk = pNew;
        }

        ప్రతిజ్ఞా( pChunk!=0 );
        జ్ఞాపకం_నకలు((u8*)pChunk->zChunk + i_Chunk_స్థానభ్రంశం, zWrite, iSpace);
        zWrite += iSpace;
        nWrite -= iSpace;
        p->endpoint.i_స్థానభ్రంశం += iSpace;
      }
    }
  }

  తిరిగి SQLITE_OK;
}

/*
** Truncate the in-memory file.
*/
స్థిర పూర్ణాంకం memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 కొలత){
  MemJournal *p = (MemJournal *)pJfd;
  ప్రతిజ్ఞా( p->endpoint.pChunk==0 || p->endpoint.pChunk->p_తదుపరి==0 );
  ఒకవేళ( కొలత<p->endpoint.i_స్థానభ్రంశం ){
    FileChunk *pIter = 0;
    ఒకవేళ( కొలత==0 ){
      memjrnlFreeChunks(p->p_మొదటి);
      p->p_మొదటి = 0;
    }లేకపోతే{
      i64 iOff = p->n_Chunk_కొలత;
      కొరకు(pIter=p->p_మొదటి; ALWAYS(pIter) && iOff<కొలత; pIter=pIter->p_తదుపరి){
        iOff += p->n_Chunk_కొలత;
      }
      ఒకవేళ( ALWAYS(pIter) ){
        memjrnlFreeChunks(pIter->p_తదుపరి);
        pIter->p_తదుపరి = 0;
      }
    }

    p->endpoint.pChunk = pIter;
    p->endpoint.i_స్థానభ్రంశం = కొలత;
    p->readpoint.pChunk = 0;
    p->readpoint.i_స్థానభ్రంశం = 0;
  }
  తిరిగి SQLITE_OK;
}

/*
** Close the file.
*/
స్థిర పూర్ణాంకం memjrnlClose(sqlite3_file *pJfd){
  MemJournal *p = (MemJournal *)pJfd;
  memjrnlFreeChunks(p->p_మొదటి);
  తిరిగి SQLITE_OK;
}

/*
** Sync the file.
**
** If the real file has been created, call its xSync method. Otherwise, 
** syncing an in-memory journal is a no-op. 
*/
స్థిర పూర్ణాంకం memjrnlSync(sqlite3_file *pJfd, పూర్ణాంకం flags){
  వాడని_PARAMETER2(pJfd, flags);
  తిరిగి SQLITE_OK;
}

/*
** Query the size of the file in bytes.
*/
స్థిర పూర్ణాంకం memjrnl_File_కొలత(sqlite3_file *pJfd, sqlite_int64 *p_కొలత){
  MemJournal *p = (MemJournal *)pJfd;
  *p_కొలత = (sqlite_int64) p->endpoint.i_స్థానభ్రంశం;
  తిరిగి SQLITE_OK;
}

/*
** Table of methods for MemJournal sqlite3_file object.
*/
స్థిర స్థిరరాశి నిర్మాణం sqlite3_io_methods MemJournalMethods = {
  1,                /* iVersion */
  memjrnlClose,     /* xClose */
  memjrnlRead,      /* xRead */
  memjrnlWrite,     /* xWrite */
  memjrnlTruncate,  /* xTruncate */
  memjrnlSync,      /* xSync */
  memjrnl_File_కొలత,  /* xFileSize */
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
పూర్ణాంకం sqlite3JournalOpen(
  sqlite3_vfs *pVfs,         /* The VFS to use for actual file I/O */
  స్థిరరాశి అక్షరం *zName,         /* Name of the journal file */
  sqlite3_file *pJfd,        /* Preallocated, blank file handle */
  పూర్ణాంకం flags,                 /* Opening flags */
  పూర్ణాంకం nSpill                 /* Bytes buffered before opening the file */
){
  MemJournal *p = (MemJournal*)pJfd;

  ప్రతిజ్ఞా( zName || nSpill<0 || (flags & SQLITE_OPEN_EXCLUSIVE) );

  /* Zero the file-handle object. If nSpill was passed zero, initialize
  ** it using the sqlite3OsOpen() function of the underlying VFS. In this
  ** case none of the code in this module is executed as a result of calls
  ** made on the journal file-handle.  */
  memset(p, 0, పరిమాణం(MemJournal));
  ఒకవేళ( nSpill==0 ){
    తిరిగి sqlite3OsOpen(pVfs, zName, pJfd, flags, 0);
  }

  ఒకవేళ( nSpill>0 ){
    p->n_Chunk_కొలత = nSpill;
  }లేకపోతే{
    p->n_Chunk_కొలత = 8 + MEMJOURNAL_DFLT_FILECHUNKSIZE - పరిమాణం(FileChunk);
    ప్రతిజ్ఞా( MEMJOURNAL_DFLT_FILECHUNKSIZE==file_Chunk_కొలత(p->n_Chunk_కొలత) );
  }

  pJfd->pMethods = (స్థిరరాశి sqlite3_io_methods*)&MemJournalMethods;
  p->nSpill = nSpill;
  p->flags = flags;
  p->zJournal = zName;
  p->pVfs = pVfs;
  తిరిగి SQLITE_OK;
}

/*
** Open an in-memory journal file.
*/
శూన్య sqlite3MemJournalOpen(sqlite3_file *pJfd){
  sqlite3JournalOpen(0, 0, pJfd, 0, -1);
}

#ఒకవేళ defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
/*
** If the argument p points to a MemJournal structure that is not an 
** in-memory-only journal file (i.e. is one that was opened with a +ve
** nSpill parameter or as SQLITE_OPEN_MAIN_JOURNAL), and the underlying 
** file has not yet been created, create it now.
*/
పూర్ణాంకం sqlite3JournalCreate(sqlite3_file *pJfd){
  పూర్ణాంకం rc = SQLITE_OK;
  MemJournal *p = (MemJournal*)pJfd;
  ఒకవేళ( pJfd->pMethods==&MemJournalMethods && (
#నిర్వచితమైతే SQLITE_ENABLE_ATOMIC_WRITE
     p->nSpill>0
#లేకపోతే
     /* While this appears to not be possible without ATOMIC_WRITE, the
     ** paths are complex, so it seems prudent to leave the test in as
     ** a NEVER(), in case our analysis is subtly flawed. */
     NEVER(p->nSpill>0)
#అయితే_ముగింపు
#నిర్వచితమైతే SQLITE_ENABLE_BATCH_ATOMIC_WRITE
     || (p->flags & SQLITE_OPEN_MAIN_JOURNAL)
#అయితే_ముగింపు
  )){
    rc = memjrnlCreateFile(p);
  }
  తిరిగి rc;
}
#అయితే_ముగింపు

/*
** The file-handle passed as the only argument is open on a journal file.
** Return true if this "journal file" is currently stored in heap memory,
** or false otherwise.
*/
పూర్ణాంకం sqlite3_Journal_Is_లోపల_జ్ఞాపకం(sqlite3_file *p){
  తిరిగి p->pMethods==&MemJournalMethods;
}

/* 
** Return the number of bytes required to store a JournalFile that uses vfs
** pVfs to create the underlying on-disk files.
*/
పూర్ణాంకం sqlite3_Journal_కొలత(sqlite3_vfs *pVfs){
  తిరిగి MAX(pVfs->szOsFile, (పూర్ణాంకం)పరిమాణం(MemJournal));
}
