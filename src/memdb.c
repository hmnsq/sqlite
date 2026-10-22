/*
** 2016-09-07
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file implements an in-memory VFS. A database is held as a contiguous
** block of memory.
**
** This file also implements interface sqlite3_serialize() and
** sqlite3_deserialize().
*/
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ifndef SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ sqlite3_vfs MemVfs;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ MemFile MemFile;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ MemStore MemStore;

/* Access to a lower-level VFS that (might) implement dynamic loading,
** access to randomness, etc.
*/
#define ORIGVFS(p) ((sqlite3_vfs*)((p)->pAppData))

/* Storage for a memdb file.
**
** An memdb object can be shared or separate.  Shared memdb objects can be
** used by more than one database connection.  Mutexes are used by shared
** memdb objects to coordinate access.  Separate memdb objects are only
** connected to a single database connection and do not require additional
** mutexes.
**
** Shared memdb objects have .zFName!=0 and .pMutex!=0.  They are created
** using "file:/name?vfs=memdb".  The first character of the name must be
** "/" or else the object will be a separate memdb object.  All shared
** memdb objects are stored in memdb_g.apMemStore[] in an arbitrary order.
**
** Separate memdb objects are created using a name that does not begin
** with "/" or using sqlite3_deserialize().
**
** Access rules for shared MemStore objects:
**
**   *  .zFName is initialized when the object is created and afterwards
**      is unchanged until the object is destroyed.  So it can be accessed
**      at any time as long as we know the object is not being destroyed,
**      which means while either the SQLITE_MUTEX_STATIC_VFS1 or
**      .pMutex is held or the object is not part of memdb_g.apMemStore[].
**
**   *  Can .pMutex can only be changed while holding the 
**      SQLITE_MUTEX_STATIC_VFS1 mutex or while the object is not part
**      of memdb_g.apMemStore[].
**
**   *  Other fields can only be changed while holding the .pMutex mutex
**      or when the .nRef is less than zero and the object is not part of
**      memdb_g.apMemStore[].
**
**   *  The .aData pointer has the added requirement that it can can only
**      be changed (for resizing) when nMmap is zero.
**      
*/
ಸಂರಚನಾ MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  unsigned ವರ್ಣ *aData;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  ಪೂರ್ಣಾಙ್ಕ nMmap;                      /* Number of memory mapped pages */
  unsigned mFlags;                /* Flags */
  ಪೂರ್ಣಾಙ್ಕ nRdLock;                    /* Number of readers */
  ಪೂರ್ಣಾಙ್ಕ nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  ಪೂರ್ಣಾಙ್ಕ nRef;                       /* Number of users of this MemStore */
  ವರ್ಣ *zFName;                   /* The filename for shared stores */
};

/* An open file */
ಸಂರಚನಾ MemFile {
  sqlite3_file base;              /* IO methods */
  MemStore *pStore;               /* The storage */
  ಪೂರ್ಣಾಙ್ಕ eLock;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
ಸ್ಥಿರ ಸಂರಚನಾ MemFS {
  ಪೂರ್ಣಾಙ್ಕ nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbRead(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbWrite(sqlite3_file*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbTruncate(sqlite3_file*, sqlite3_int64 size);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbSync(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbUnlock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbFileControl(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbDeviceCharacteristics(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, ಪೂರ್ಣಾಙ್ಕ iAmt, ಶೂನ್ಯ **pp);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, ಶೂನ್ಯ *p);

/*
** Methods for MemVfs
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbOpen(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *, sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ , ಪೂರ್ಣಾಙ್ಕ *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbAccess(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ flags, ಪೂರ್ಣಾಙ್ಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbFullPathname(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ, ವರ್ಣ *zOut);
ಸ್ಥಿರ ಶೂನ್ಯ *memdbDlOpen(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zFilename);
ಸ್ಥಿರ ಶೂನ್ಯ memdbDlError(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *z_Err_ಸಂದೇಶ);
ಸ್ಥಿರ ಶೂನ್ಯ (*memdbDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *p, ಧ್ರುವ ವರ್ಣ*zSym))(ಶೂನ್ಯ);
ಸ್ಥಿರ ಶೂನ್ಯ memdbDlClose(sqlite3_vfs*, ಶೂನ್ಯ*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbRandomness(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zOut);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbSleep(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbGetLastError(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ, ವರ್ಣ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

ಸ್ಥಿರ sqlite3_vfs memdb_vfs = {
  2,                           /* iVersion */
  0,                           /* szOsFile (set when registered) */
  1024,                        /* mxPathname */
  0,                           /* pNext */
  "memdb",                     /* zName */
  0,                           /* pAppData (set when registered) */ 
  memdbOpen,                   /* xOpen */
  0, /* memdbDelete, */        /* xDelete */
  memdbAccess,                 /* xAccess */
  memdbFullPathname,           /* xFullPathname */
  memdbDlOpen,                 /* xDlOpen */
  memdbDlError,                /* xDlError */
  memdbDlSym,                  /* xDlSym */
  memdbDlClose,                /* xDlClose */
  memdbRandomness,             /* xRandomness */
  memdbSleep,                  /* xSleep */
  0, /* memdbCurrentTime, */   /* xCurrentTime */
  memdbGetLastError,           /* xGetLastError */
  memdbCurrentTimeInt64,       /* xCurrentTimeInt64 */
  0,                           /* xSetSystemCall */
  0,                           /* xGetSystemCall */
  0,                           /* xNextSystemCall */
};

ಸ್ಥಿರ ಧ್ರುವ sqlite3_io_methods memdb_io_methods = {
  3,                              /* iVersion */
  memdbClose,                      /* xClose */
  memdbRead,                       /* xRead */
  memdbWrite,                      /* xWrite */
  memdbTruncate,                   /* xTruncate */
  memdbSync,                       /* xSync */
  memdbFileSize,                   /* xFileSize */
  memdbLock,                       /* xLock */
  memdbUnlock,                     /* xUnlock */
  0, /* memdbCheckReservedLock, */ /* xCheckReservedLock */
  memdbFileControl,                /* xFileControl */
  0, /* memdbSectorSize,*/         /* xSectorSize */
  memdbDeviceCharacteristics,      /* xDeviceCharacteristics */
  0,                               /* xShmMap */
  0,                               /* xShmLock */
  0,                               /* xShmBarrier */
  0,                               /* xShmUnmap */
  memdbFetch,                      /* xFetch */
  memdbUnfetch                     /* xUnfetch */
};

/*
** Enter/leave the mutex on a MemStore
*/
#ಯದಿ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
ಸ್ಥಿರ ಶೂನ್ಯ memdbEnter(MemStore *p){
  UNUSED_PARAMETER(p);
}
ಸ್ಥಿರ ಶೂನ್ಯ memdbLeave(MemStore *p){
  UNUSED_PARAMETER(p);
}
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಶೂನ್ಯ memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
ಸ್ಥಿರ ಶೂನ್ಯ memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#endif



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ಯದಿ( p->zFName ){
    ಪೂರ್ಣಾಙ್ಕ i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    ಪ್ರತಿ(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      ಯದಿ( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        ಯದಿ( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          ಯದಿ( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        ಭಙ್ಗ;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }ಅನ್ಯಥಾ{
    memdbEnter(p);
  }
  p->nRef--;
  ಯದಿ( p->nRef<=0 ){
    ಯದಿ( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->aData);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }ಅನ್ಯಥಾ{
    memdbLeave(p);
  }
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಙ್ಕ iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ಯದಿ( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    ಯದಿ( iOfst<p->sz ) memcpy(zBuf, p->aData+iOfst, p->sz - iOfst);
    memdbLeave(p);
    ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, p->aData+iOfst, iAmt);
  memdbLeave(p);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  unsigned ವರ್ಣ *pNew;
  ಯದಿ( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_FULL;
  }
  ಯದಿ( newSz>p->szMax ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_FULL;
  }
  newSz *= 2;
  ಯದಿ( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->aData, newSz);
  ಯದಿ( pNew==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_NOMEM;
  p->aData = pNew;
  p->szAlloc = newSz;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbWrite(
  sqlite3_file *pFile,
  ಧ್ರುವ ಶೂನ್ಯ *z,
  ಪೂರ್ಣಾಙ್ಕ iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ಯದಿ( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_WRITE;
  }
  ಯದಿ( iOfst+iAmt>p->sz ){
    ಪೂರ್ಣಾಙ್ಕ rc;
    ಯದಿ( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      ಪ್ರತ್ಯಾಗಮ rc;
    }
    ಯದಿ( iOfst>p->sz ) memset(p->aData+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  memcpy(p->aData+iOfst, z, iAmt);
  memdbLeave(p);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbTruncate(sqlite3_file *pFile, sqlite_int64 size){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  memdbEnter(p);
  ಯದಿ( size>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }ಅನ್ಯಥಾ{
    p->sz = size; 
  }
  memdbLeave(p);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Sync an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbSync(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *pSize = p->sz;
  memdbLeave(p);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಯದಿ( eLock<=pThis->eLock ) ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  memdbEnter(p);

  ಪ್ರತಿಜ್ಞಾ( p->nWrLock==0 || p->nWrLock==1 );
  ಪ್ರತಿಜ್ಞಾ( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  ಪ್ರತಿಜ್ಞಾ( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  ಯದಿ( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }ಅನ್ಯಥಾ{
    ವಿಕಲ್ಪ( eLock ){
      ಪಕ್ಷ SQLITE_LOCK_SHARED: {
        ಪ್ರತಿಜ್ಞಾ( pThis->eLock==SQLITE_LOCK_NONE );
        ಯದಿ( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }ಅನ್ಯಥಾ{
          p->nRdLock++;
        }
        ಭಙ್ಗ;
      };
  
      ಪಕ್ಷ SQLITE_LOCK_RESERVED:
      ಪಕ್ಷ SQLITE_LOCK_PENDING: {
        ಪ್ರತಿಜ್ಞಾ( pThis->eLock>=SQLITE_LOCK_SHARED );
        ಯದಿ( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          ಯದಿ( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }ಅನ್ಯಥಾ{
            p->nWrLock = 1;
          }
        }
        ಭಙ್ಗ;
      }
  
      ಸ್ವಭಾವ: {
        ಪ್ರತಿಜ್ಞಾ(  eLock==SQLITE_LOCK_EXCLUSIVE );
        ಪ್ರತಿಜ್ಞಾ( pThis->eLock>=SQLITE_LOCK_SHARED );
        ಯದಿ( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }ಅನ್ಯಥಾ ಯದಿ( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        ಭಙ್ಗ;
      }
    }
  }
  ಯದಿ( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Unlock an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  ಯದಿ( eLock>=pThis->eLock ) ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  memdbEnter(p);

  ಪ್ರತಿಜ್ಞಾ( eLock==SQLITE_LOCK_SHARED || eLock==SQLITE_LOCK_NONE );
  ಯದಿ( eLock==SQLITE_LOCK_SHARED ){
    ಯದಿ( ALWAYS(pThis->eLock>SQLITE_LOCK_SHARED) ){
      p->nWrLock--;
    }
  }ಅನ್ಯಥಾ{
    ಯದಿ( pThis->eLock>SQLITE_LOCK_SHARED ){
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

#ಯದಿ 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbCheckReservedLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ *pResOut){
  *pResOut = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}
#endif


/*
** File control method. For custom operations on an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  ಯದಿ( op==SQLITE_FCNTL_VFSNAME ){
    *(ವರ್ಣ**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->aData, p->sz);
    rc = SQLITE_OK;
  }
  ಯದಿ( op==SQLITE_FCNTL_SIZE_LIMIT ){
    sqlite3_int64 iLimit = *(sqlite3_int64*)pArg;
    ಯದಿ( iLimit<p->sz ){
      ಯದಿ( iLimit<0 ){
        iLimit = p->szMax;
      }ಅನ್ಯಥಾ{
        iLimit = p->sz;
      }
    }
    p->szMax = iLimit;
    *(sqlite3_int64*)pArg = iLimit;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  ಪ್ರತ್ಯಾಗಮ rc;
}

#ಯದಿ 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbSectorSize(sqlite3_file *pFile){
  ಪ್ರತ್ಯಾಗಮ 1024;
}
#endif

/*
** Return the device characteristic flags supported by an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  ಪ್ರತ್ಯಾಗಮ SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  ಪೂರ್ಣಾಙ್ಕ iAmt,
  ಶೂನ್ಯ **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ಯದಿ( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }ಅನ್ಯಥಾ{
    p->nMmap++;
    *pp = (ಶೂನ್ಯ*)(p->aData + iOfst);
  }
  memdbLeave(p);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/* Release a memory-mapped page */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, ಶೂನ್ಯ *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Open an mem file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbOpen(
  sqlite3_vfs *pVfs,
  ಧ್ರುವ ವರ್ಣ *zName,
  sqlite3_file *pFd,
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಪೂರ್ಣಾಙ್ಕ *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  ಪೂರ್ಣಾಙ್ಕ szName;
  UNUSED_PARAMETER(pVfs);

  memset(pFile, 0, ಪರಿಮಾಣ(*pFile));
  szName = sqlite3Strlen30(zName);
  ಯದಿ( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    ಪೂರ್ಣಾಙ್ಕ i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    ಪ್ರತಿ(i=0; i<memdb_g.nMemStore; i++){
      ಯದಿ( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        ಭಙ್ಗ;
      }
    }
    ಯದಿ( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( ಪರಿಮಾಣ(*p) + (i64)szName + 3 );
      ಯದಿ( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             ಪರಿಮಾಣ(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      ಯದಿ( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, ಪರಿಮಾಣ(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mxMemdbSize;
      p->zFName = (ವರ್ಣ*)&p[1];
      memcpy(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      ಯದಿ( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }ಅನ್ಯಥಾ{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }ಅನ್ಯಥಾ{
    p = sqlite3Malloc( ಪರಿಮಾಣ(*p) );
    ಯದಿ( p==0 ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_NOMEM;
    }
    memset(p, 0, ಪರಿಮಾಣ(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mxMemdbSize;
  }
  pFile->pStore = p;
  ಯದಿ( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

#ಯದಿ 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbDelete(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath, ಪೂರ್ಣಾಙ್ಕ dirSync){
  ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_DELETE;
}
#endif

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbAccess(
  sqlite3_vfs *pVfs, 
  ಧ್ರುವ ವರ್ಣ *zPath, 
  ಪೂರ್ಣಾಙ್ಕ flags, 
  ಪೂರ್ಣಾಙ್ಕ *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbFullPathname(
  sqlite3_vfs *pVfs, 
  ಧ್ರುವ ವರ್ಣ *zPath, 
  ಪೂರ್ಣಾಙ್ಕ nOut, 
  ವರ್ಣ *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *memdbDlOpen(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath){
  ಪ್ರತ್ಯಾಗಮ ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memdbDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *z_Err_ಸಂದೇಶ){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_ಸಂದೇಶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ (*memdbDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *p, ಧ್ರುವ ವರ್ಣ *zSym))(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memdbDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zBufOut){
  ಪ್ರತ್ಯಾಗಮ ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nMicro){
  ಪ್ರತ್ಯಾಗಮ ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#ಯದಿ 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbCurrentTime(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *pTimeOut){
  ಪ್ರತ್ಯಾಗಮ ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbGetLastError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ a, ವರ್ಣ *b){
  ಪ್ರತ್ಯಾಗಮ ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  ಪ್ರತ್ಯಾಗಮ ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
ಸ್ಥಿರ MemFile *memdbFromDbSchema(sqlite3 *db, ಧ್ರುವ ವರ್ಣ *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  ಪೂರ್ಣಾಙ್ಕ rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( p->base.pMethods!=&memdb_io_methods ) ಪ್ರತ್ಯಾಗಮ 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  ಯದಿ( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  ಪ್ರತ್ಯಾಗಮ p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
unsigned ವರ್ಣ *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  ಧ್ರುವ ವರ್ಣ *zSchema,      /* Which database within the connection */
  sqlite3_int64 *piSize,    /* Write size here, if not NULL */
  unsigned ಪೂರ್ಣಾಙ್ಕ mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  ಪೂರ್ಣಾಙ್ಕ iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  ಪೂರ್ಣಾಙ್ಕ szPage = 0;
  sqlite3_stmt *pStmt = 0;
  unsigned ವರ್ಣ *pOut = 0;
  ವರ್ಣ *zSql;
  ಪೂರ್ಣಾಙ್ಕ rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಪ್ರತ್ಯಾಗಮ 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  ಯದಿ( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  ಯದಿ( piSize ) *piSize = -1;
  ಯದಿ( iDb<0 ) ಗಚ್ಛ serialize_out;
  ಯದಿ( p ){
    MemStore *pStore = p->pStore;
    ಪ್ರತಿಜ್ಞಾ( pStore->pMutex==0 );
    ಯದಿ( piSize ) *piSize = pStore->sz;
    ಯದಿ( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->aData;
    }ಅನ್ಯಥಾ{
      pOut = sqlite3_malloc64( pStore->sz );
      ಯದಿ( pOut ) memcpy(pOut, pStore->aData, pStore->sz);
    }
    ಗಚ್ಛ serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  ಯದಿ( pBt==0 ) ಗಚ್ಛ serialize_out;
  szPage = sqlite3BtreeGetPageSize(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  ಯದಿ( rc ) ಗಚ್ಛ serialize_out;
  rc = sqlite3_step(pStmt);
  ಯದಿ( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    ಯದಿ( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      ಯದಿ( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    ಯದಿ( piSize ) *piSize = sz;
    ಯದಿ( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }ಅನ್ಯಥಾ{
      pOut = sqlite3_malloc64( sz );
      ಯದಿ( pOut ){
        ಪೂರ್ಣಾಙ್ಕ nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        ಪೂರ್ಣಾಙ್ಕ pgno;
        ಪ್ರತಿ(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          unsigned ವರ್ಣ *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          ಯದಿ( rc==SQLITE_OK ){
            memcpy(pTo, sqlite3PagerGetData(pPage), szPage);
          }ಅನ್ಯಥಾ ಯದಿ( pgno==PENDING_BYTE/szPage+1 ){
            memset(pTo, 0, szPage);
          }ಅನ್ಯಥಾ{
            sqlite3_free(pOut);
            pOut = 0;
            nPage = 0;            
          }
          sqlite3PagerUnref(pPage);       
        }
      }
    }
  }
  sqlite3_finalize(pStmt);

 serialize_out:
  sqlite3_mutex_leave(db->mutex);
  ಪ್ರತ್ಯಾಗಮ pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  ಧ್ರುವ ವರ್ಣ *zSchema,    /* Which DB to reopen with the deserialization */
  unsigned ವರ್ಣ *pData,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  unsigned mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  ವರ್ಣ *zSql;
  sqlite3_stmt *pStmt = 0;
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಪೂರ್ಣಾಙ್ಕ iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  ಯದಿ( !sqlite3SafetyCheckOk(db) ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  }
  ಯದಿ( szDb<0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
  ಯದಿ( szBuf<0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_MISUSE_BKPT;
#endif

  sqlite3_mutex_enter(db->mutex);
  ಯದಿ( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  ಯದಿ( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    ಗಚ್ಛ end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  ಯದಿ( zSql==0 ){
    rc = SQLITE_NOMEM;
  }ಅನ್ಯಥಾ{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  ಯದಿ( rc ) ಗಚ್ಛ end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  ಯದಿ( rc!=SQLITE_OK ){
    ಗಚ್ಛ end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  ಯದಿ( p==0 ){
    rc = SQLITE_ERROR;
  }ಅನ್ಯಥಾ{
    MemStore *pStore = p->pStore;
    pStore->aData = pData;
    pData = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    ಯದಿ( pStore->szMax<sqlite3GlobalConfig.mxMemdbSize ){
      pStore->szMax = sqlite3GlobalConfig.mxMemdbSize;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  ಯದಿ( pData && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(pData);
  }
  sqlite3_mutex_leave(db->mutex);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Return true if the VFS is the memvfs.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3IsMemdb(ಧ್ರುವ sqlite3_vfs *pVfs){
  ಪ್ರತ್ಯಾಗಮ pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3MemdbInit(ಶೂನ್ಯ){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  unsigned ಪೂರ್ಣಾಙ್ಕ sz;
  ಯದಿ( NEVER(pLower==0) ) ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.pAppData = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  ಯದಿ( sz<ಪರಿಮಾಣ(MemFile) ) sz = ಪರಿಮಾಣ(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  ಪ್ರತ್ಯಾಗಮ sqlite3_vfs_register(&memdb_vfs, 0);
}
#endif /* SQLITE_OMIT_DESERIALIZE */
