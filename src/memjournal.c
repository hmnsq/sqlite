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
#شامل "sqliteInt.h"

/* Forward references to internal structures */
قسمتعریف ساخت MemJournal MemJournal;
قسمتعریف ساخت File_نقطہ File_نقطہ;
قسمتعریف ساخت FileChunk FileChunk;

/*
** The rollback journal is composed of a linked list of these structures.
**
** The zChunk array is always at least 8 bytes in size - usually much more.
** Its actual size is stored in the MemJournal.nChunkSize variable.
*/
ساخت FileChunk {
  FileChunk *p_اگلا;               /* Next chunk in the journal */
  u8 zChunk[8];                   /* Content of this chunk */
};

/*
** By default, allocate this many bytes of memory for each FileChunk object.
*/
#تعریف_کرو MEMJOURNAL_DFLT_FILECHUNKSIZE 1024

/*
** For chunk size nChunkSize, return the number of bytes that should
** be allocated for each FileChunk structure.
*/
#تعریف_کرو file_Chunk_سائز(n_Chunk_سائز) (حجم(FileChunk) + ((n_Chunk_سائز)-8))

/*
** An instance of this object serves as a cursor into the rollback journal.
** The cursor can be either for reading or writing.
*/
ساخت File_نقطہ {
  sqlite3_int64 i_انحراف;          /* Offset from the beginning of the file */
  FileChunk *pChunk;              /* Specific chunk into which cursor points */
};

/*
** This structure is a subclass of sqlite3_file. Each open memory-journal
** is an instance of this class.
*/
ساخت MemJournal {
  ثابت sqlite3_io_methods *p_طریقہ; /* Parent class. MUST BE FIRST */
  صحیح n_Chunk_سائز;                 /* In-memory chunk-size */

  صحیح nSpill;                     /* Bytes of data before flushing */
  FileChunk *p_پہلا;              /* Head of in-memory chunk-list */
  File_نقطہ endpoint;             /* Pointer to the end of the file */
  File_نقطہ readpoint;            /* Pointer to the end of the last xRead() */

  صحیح flags;                      /* xOpen flags */
  sqlite3_vfs *pVfs;              /* The "real" underlying VFS */
  ثابت حرف *zJournal;           /* Name of the journal file */
};

/*
** Read data from the in-memory journal file.  This is the implementation
** of the sqlite3_vfs.xRead method.
*/
ساکن صحیح memjrnlRead(
  sqlite3_file *pJfd,    /* The journal file from which to read */
  خالی *zBuf,            /* Put the results here */
  صحیح iAmt,              /* Number of bytes to read */
  sqlite_int64 iOfst     /* Begin reading at this offset */
){
  MemJournal *p = (MemJournal *)pJfd;
  u8 *z_برآمد = zBuf;
  صحیح nRead = iAmt;
  صحیح i_Chunk_انحراف;
  FileChunk *pChunk;

  اگر( (iAmt+iOfst)>p->endpoint.i_انحراف ){
    واپس SQLITE_IOERR_SHORT_READ;
  }
  أكد( p->readpoint.i_انحراف==0 || p->readpoint.pChunk!=0 );
  اگر( p->readpoint.i_انحراف!=iOfst || iOfst==0 ){
    sqlite3_int64 iOff = 0;
    برائے(pChunk=p->p_پہلا; 
        ALWAYS(pChunk) && (iOff+p->n_Chunk_سائز)<=iOfst;
        pChunk=pChunk->p_اگلا
    ){
      iOff += p->n_Chunk_سائز;
    }
  }ورنہ{
    pChunk = p->readpoint.pChunk;
    أكد( pChunk!=0 );
  }

  i_Chunk_انحراف = (صحیح)(iOfst%p->n_Chunk_سائز);
  کرو {
    صحیح iSpace = p->n_Chunk_سائز - i_Chunk_انحراف;
    صحیح nCopy = MIN(nRead, (p->n_Chunk_سائز - i_Chunk_انحراف));
    حافظہ_نقل(z_برآمد, (u8*)pChunk->zChunk + i_Chunk_انحراف, nCopy);
    z_برآمد += nCopy;
    nRead -= iSpace;
    i_Chunk_انحراف = 0;
  } جب( nRead>=0 && (pChunk=pChunk->p_اگلا)!=0 && nRead>0 );
  p->readpoint.i_انحراف = pChunk ? iOfst+iAmt : 0;
  p->readpoint.pChunk = pChunk;

  واپس SQLITE_OK;
}

/*
** Free the list of FileChunk structures headed at MemJournal.pFirst.
*/
ساکن خالی memjrnlFreeChunks(FileChunk *p_پہلا){
  FileChunk *pIter;
  FileChunk *p_اگلا;
  برائے(pIter=p_پہلا; pIter; pIter=p_اگلا){
    p_اگلا = pIter->p_اگلا;
    sqlite3_free(pIter);
  } 
}

/*
** Flush the contents of memory to a real file on disk.
*/
ساکن صحیح memjrnlCreateFile(MemJournal *p){
  صحیح rc;
  sqlite3_file *pReal = (sqlite3_file*)p;
  MemJournal copy = *p;

  memset(p, 0, حجم(MemJournal));
  rc = sqlite3OsOpen(copy.pVfs, copy.zJournal, pReal, copy.flags, 0);
  اگر( rc==SQLITE_OK ){
    صحیح nChunk = copy.n_Chunk_سائز;
    i64 iOff = 0;
    FileChunk *pIter;
    برائے(pIter=copy.p_پہلا; pIter; pIter=pIter->p_اگلا){
      اگر( iOff + nChunk > copy.endpoint.i_انحراف ){
        nChunk = copy.endpoint.i_انحراف - iOff;
      }
      rc = sqlite3OsWrite(pReal, (u8*)pIter->zChunk, nChunk, iOff);
      اگر( rc ) توڑو;
      iOff += nChunk;
    }
    اگر( rc==SQLITE_OK ){
      /* No error has occurred. Free the in-memory buffers. */
      memjrnlFreeChunks(copy.p_پہلا);
    }
  }
  اگر( rc!=SQLITE_OK ){
    /* If an error occurred while creating or writing to the file, restore
    ** the original before returning. This way, SQLite uses the in-memory
    ** journal data to roll back changes made to the internal page-cache
    ** before this function was called.  */
    sqlite3OsClose(pReal);
    *p = copy;
  }
  واپس rc;
}


/* Forward reference */
ساکن صحیح memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 سائز);

/*
** Write data to the file.
*/
ساکن صحیح memjrnlWrite(
  sqlite3_file *pJfd,    /* The journal file into which to write */
  ثابت خالی *zBuf,      /* Take data to be written from here */
  صحیح iAmt,              /* Number of bytes to write */
  sqlite_int64 iOfst     /* Begin writing at this offset into the file */
){
  MemJournal *p = (MemJournal *)pJfd;
  صحیح nWrite = iAmt;
  u8 *zWrite = (u8 *)zBuf;

  /* If the file should be created now, create it and write the new data
  ** into the file on disk. */
  اگر( p->nSpill>0 && (iAmt+iOfst)>p->nSpill ){
    صحیح rc = memjrnlCreateFile(p);
    اگر( rc==SQLITE_OK ){
      rc = sqlite3OsWrite(pJfd, zBuf, iAmt, iOfst);
    }
    واپس rc;
  }

  /* If the contents of this write should be stored in memory */
  ورنہ{
    /* An in-memory journal file should only ever be appended to. Random
    ** access writes are not required. The only exception to this is when
    ** the in-memory journal is being used by a connection using the
    ** atomic-write optimization. In this case the first 28 bytes of the
    ** journal file may be written as part of committing the transaction. */
    أكد( iOfst<=p->endpoint.i_انحراف );
    اگر( iOfst>0 && iOfst!=p->endpoint.i_انحراف ){
      memjrnlTruncate(pJfd, iOfst);
    }
    اگر( iOfst==0 && p->p_پہلا ){
      أكد( p->n_Chunk_سائز>iAmt );
      حافظہ_نقل((u8*)p->p_پہلا->zChunk, zBuf, iAmt);
    }ورنہ{
      جب( nWrite>0 ){
        FileChunk *pChunk = p->endpoint.pChunk;
        صحیح i_Chunk_انحراف = (صحیح)(p->endpoint.i_انحراف%p->n_Chunk_سائز);
        صحیح iSpace = MIN(nWrite, p->n_Chunk_سائز - i_Chunk_انحراف);

        أكد( pChunk!=0 || i_Chunk_انحراف==0 );
        اگر( i_Chunk_انحراف==0 ){
          /* New chunk is required to extend the file. */
          FileChunk *pNew = sqlite3_malloc(file_Chunk_سائز(p->n_Chunk_سائز));
          اگر( !pNew ){
            واپس SQLITE_IOERR_NOMEM_BKPT;
          }
          pNew->p_اگلا = 0;
          اگر( pChunk ){
            أكد( p->p_پہلا );
            pChunk->p_اگلا = pNew;
          }ورنہ{
            أكد( !p->p_پہلا );
            p->p_پہلا = pNew;
          }
          pChunk = p->endpoint.pChunk = pNew;
        }

        أكد( pChunk!=0 );
        حافظہ_نقل((u8*)pChunk->zChunk + i_Chunk_انحراف, zWrite, iSpace);
        zWrite += iSpace;
        nWrite -= iSpace;
        p->endpoint.i_انحراف += iSpace;
      }
    }
  }

  واپس SQLITE_OK;
}

/*
** Truncate the in-memory file.
*/
ساکن صحیح memjrnlTruncate(sqlite3_file *pJfd, sqlite_int64 سائز){
  MemJournal *p = (MemJournal *)pJfd;
  أكد( p->endpoint.pChunk==0 || p->endpoint.pChunk->p_اگلا==0 );
  اگر( سائز<p->endpoint.i_انحراف ){
    FileChunk *pIter = 0;
    اگر( سائز==0 ){
      memjrnlFreeChunks(p->p_پہلا);
      p->p_پہلا = 0;
    }ورنہ{
      i64 iOff = p->n_Chunk_سائز;
      برائے(pIter=p->p_پہلا; ALWAYS(pIter) && iOff<سائز; pIter=pIter->p_اگلا){
        iOff += p->n_Chunk_سائز;
      }
      اگر( ALWAYS(pIter) ){
        memjrnlFreeChunks(pIter->p_اگلا);
        pIter->p_اگلا = 0;
      }
    }

    p->endpoint.pChunk = pIter;
    p->endpoint.i_انحراف = سائز;
    p->readpoint.pChunk = 0;
    p->readpoint.i_انحراف = 0;
  }
  واپس SQLITE_OK;
}

/*
** Close the file.
*/
ساکن صحیح memjrnlClose(sqlite3_file *pJfd){
  MemJournal *p = (MemJournal *)pJfd;
  memjrnlFreeChunks(p->p_پہلا);
  واپس SQLITE_OK;
}

/*
** Sync the file.
**
** If the real file has been created, call its xSync method. Otherwise, 
** syncing an in-memory journal is a no-op. 
*/
ساکن صحیح memjrnlSync(sqlite3_file *pJfd, صحیح flags){
  غیر_مستعمل_PARAMETER2(pJfd, flags);
  واپس SQLITE_OK;
}

/*
** Query the size of the file in bytes.
*/
ساکن صحیح memjrnl_File_سائز(sqlite3_file *pJfd, sqlite_int64 *p_سائز){
  MemJournal *p = (MemJournal *)pJfd;
  *p_سائز = (sqlite_int64) p->endpoint.i_انحراف;
  واپس SQLITE_OK;
}

/*
** Table of methods for MemJournal sqlite3_file object.
*/
ساکن ثابت ساخت sqlite3_io_methods MemJournalMethods = {
  1,                /* iVersion */
  memjrnlClose,     /* xClose */
  memjrnlRead,      /* xRead */
  memjrnlWrite,     /* xWrite */
  memjrnlTruncate,  /* xTruncate */
  memjrnlSync,      /* xSync */
  memjrnl_File_سائز,  /* xFileSize */
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
صحیح sqlite3JournalOpen(
  sqlite3_vfs *pVfs,         /* The VFS to use for actual file I/O */
  ثابت حرف *zName,         /* Name of the journal file */
  sqlite3_file *pJfd,        /* Preallocated, blank file handle */
  صحیح flags,                 /* Opening flags */
  صحیح nSpill                 /* Bytes buffered before opening the file */
){
  MemJournal *p = (MemJournal*)pJfd;

  أكد( zName || nSpill<0 || (flags & SQLITE_OPEN_EXCLUSIVE) );

  /* Zero the file-handle object. If nSpill was passed zero, initialize
  ** it using the sqlite3OsOpen() function of the underlying VFS. In this
  ** case none of the code in this module is executed as a result of calls
  ** made on the journal file-handle.  */
  memset(p, 0, حجم(MemJournal));
  اگر( nSpill==0 ){
    واپس sqlite3OsOpen(pVfs, zName, pJfd, flags, 0);
  }

  اگر( nSpill>0 ){
    p->n_Chunk_سائز = nSpill;
  }ورنہ{
    p->n_Chunk_سائز = 8 + MEMJOURNAL_DFLT_FILECHUNKSIZE - حجم(FileChunk);
    أكد( MEMJOURNAL_DFLT_FILECHUNKSIZE==file_Chunk_سائز(p->n_Chunk_سائز) );
  }

  pJfd->pMethods = (ثابت sqlite3_io_methods*)&MemJournalMethods;
  p->nSpill = nSpill;
  p->flags = flags;
  p->zJournal = zName;
  p->pVfs = pVfs;
  واپس SQLITE_OK;
}

/*
** Open an in-memory journal file.
*/
خالی sqlite3MemJournalOpen(sqlite3_file *pJfd){
  sqlite3JournalOpen(0, 0, pJfd, 0, -1);
}

#اگر defined(SQLITE_ENABLE_ATOMIC_WRITE) \
 || defined(SQLITE_ENABLE_BATCH_ATOMIC_WRITE)
/*
** If the argument p points to a MemJournal structure that is not an 
** in-memory-only journal file (i.e. is one that was opened with a +ve
** nSpill parameter or as SQLITE_OPEN_MAIN_JOURNAL), and the underlying 
** file has not yet been created, create it now.
*/
صحیح sqlite3JournalCreate(sqlite3_file *pJfd){
  صحیح rc = SQLITE_OK;
  MemJournal *p = (MemJournal*)pJfd;
  اگر( pJfd->pMethods==&MemJournalMethods && (
#اگر_تعریف_شدہ SQLITE_ENABLE_ATOMIC_WRITE
     p->nSpill>0
#ورنہ
     /* While this appears to not be possible without ATOMIC_WRITE, the
     ** paths are complex, so it seems prudent to leave the test in as
     ** a NEVER(), in case our analysis is subtly flawed. */
     NEVER(p->nSpill>0)
#اگر_ختم
#اگر_تعریف_شدہ SQLITE_ENABLE_BATCH_ATOMIC_WRITE
     || (p->flags & SQLITE_OPEN_MAIN_JOURNAL)
#اگر_ختم
  )){
    rc = memjrnlCreateFile(p);
  }
  واپس rc;
}
#اگر_ختم

/*
** The file-handle passed as the only argument is open on a journal file.
** Return true if this "journal file" is currently stored in heap memory,
** or false otherwise.
*/
صحیح sqlite3_Journal_Is_اندر_حافظہ(sqlite3_file *p){
  واپس p->pMethods==&MemJournalMethods;
}

/* 
** Return the number of bytes required to store a JournalFile that uses vfs
** pVfs to create the underlying on-disk files.
*/
صحیح sqlite3_Journal_سائز(sqlite3_vfs *pVfs){
  واپس MAX(pVfs->szOsFile, (صحیح)حجم(MemJournal));
}
