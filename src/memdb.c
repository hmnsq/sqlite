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
#அந்தர்பாவ "sqliteInt.h"
#ifndef SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
வகைவரை கட்டமைப்பு sqlite3_vfs MemVfs;
வகைவரை கட்டமைப்பு MemFile MemFile;
வகைவரை கட்டமைப்பு MemStore MemStore;

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
கட்டமைப்பு MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  unsigned எழுத்து *aData;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  முழு nMmap;                      /* Number of memory mapped pages */
  unsigned mFlags;                /* Flags */
  முழு nRdLock;                    /* Number of readers */
  முழு nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  முழு nRef;                       /* Number of users of this MemStore */
  எழுத்து *zFName;                   /* The filename for shared stores */
};

/* An open file */
கட்டமைப்பு MemFile {
  sqlite3_file base;              /* IO methods */
  MemStore *pStore;               /* The storage */
  முழு eLock;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
நிலையான கட்டமைப்பு MemFS {
  முழு nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
நிலையான முழு memdbClose(sqlite3_file*);
நிலையான முழு memdbRead(sqlite3_file*, வெறுமை*, முழு iAmt, sqlite3_int64 iOfst);
நிலையான முழு memdbWrite(sqlite3_file*,மாறிலி வெறுமை*,முழு iAmt, sqlite3_int64 iOfst);
நிலையான முழு memdbTruncate(sqlite3_file*, sqlite3_int64 size);
நிலையான முழு memdbSync(sqlite3_file*, முழு flags);
நிலையான முழு memdbFileSize(sqlite3_file*, sqlite3_int64 *pSize);
நிலையான முழு memdbLock(sqlite3_file*, முழு);
நிலையான முழு memdbUnlock(sqlite3_file*, முழு);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
நிலையான முழு memdbFileControl(sqlite3_file*, முழு op, வெறுமை *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
நிலையான முழு memdbDeviceCharacteristics(sqlite3_file*);
நிலையான முழு memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, முழு iAmt, வெறுமை **pp);
நிலையான முழு memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, வெறுமை *p);

/*
** Methods for MemVfs
*/
நிலையான முழு memdbOpen(sqlite3_vfs*, மாறிலி எழுத்து *, sqlite3_file*, முழு , முழு *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
நிலையான முழு memdbAccess(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு flags, முழு *);
நிலையான முழு memdbFullPathname(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு, எழுத்து *zOut);
நிலையான வெறுமை *memdbDlOpen(sqlite3_vfs*, மாறிலி எழுத்து *zFilename);
நிலையான வெறுமை memdbDlError(sqlite3_vfs*, முழு nByte, எழுத்து *z_Err_செய்தி);
நிலையான வெறுமை (*memdbDlSym(sqlite3_vfs *pVfs, வெறுமை *p, மாறிலி எழுத்து*zSym))(வெறுமை);
நிலையான வெறுமை memdbDlClose(sqlite3_vfs*, வெறுமை*);
நிலையான முழு memdbRandomness(sqlite3_vfs*, முழு nByte, எழுத்து *zOut);
நிலையான முழு memdbSleep(sqlite3_vfs*, முழு microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
நிலையான முழு memdbGetLastError(sqlite3_vfs*, முழு, எழுத்து *);
நிலையான முழு memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

நிலையான sqlite3_vfs memdb_vfs = {
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

நிலையான மாறிலி sqlite3_io_methods memdb_io_methods = {
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
#எனில் defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
நிலையான வெறுமை memdbEnter(MemStore *p){
  UNUSED_PARAMETER(p);
}
நிலையான வெறுமை memdbLeave(MemStore *p){
  UNUSED_PARAMETER(p);
}
#இல்லையெனில்
நிலையான வெறுமை memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
நிலையான வெறுமை memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#endif



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
நிலையான முழு memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  எனில்( p->zFName ){
    முழு i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    க்கு(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      எனில்( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        எனில்( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          எனில்( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        நிறுத்து;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }இல்லையெனில்{
    memdbEnter(p);
  }
  p->nRef--;
  எனில்( p->nRef<=0 ){
    எனில்( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->aData);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }இல்லையெனில்{
    memdbLeave(p);
  }
  திரும்பு SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
நிலையான முழு memdbRead(
  sqlite3_file *pFile, 
  வெறுமை *zBuf, 
  முழு iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  எனில்( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    எனில்( iOfst<p->sz ) memcpy(zBuf, p->aData+iOfst, p->sz - iOfst);
    memdbLeave(p);
    திரும்பு SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, p->aData+iOfst, iAmt);
  memdbLeave(p);
  திரும்பு SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
நிலையான முழு memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  unsigned எழுத்து *pNew;
  எனில்( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    திரும்பு SQLITE_FULL;
  }
  எனில்( newSz>p->szMax ){
    திரும்பு SQLITE_FULL;
  }
  newSz *= 2;
  எனில்( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->aData, newSz);
  எனில்( pNew==0 ) திரும்பு SQLITE_IOERR_NOMEM;
  p->aData = pNew;
  p->szAlloc = newSz;
  திரும்பு SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
நிலையான முழு memdbWrite(
  sqlite3_file *pFile,
  மாறிலி வெறுமை *z,
  முழு iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  எனில்( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    திரும்பு SQLITE_IOERR_WRITE;
  }
  எனில்( iOfst+iAmt>p->sz ){
    முழு rc;
    எனில்( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      திரும்பு rc;
    }
    எனில்( iOfst>p->sz ) memset(p->aData+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  memcpy(p->aData+iOfst, z, iAmt);
  memdbLeave(p);
  திரும்பு SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
நிலையான முழு memdbTruncate(sqlite3_file *pFile, sqlite_int64 size){
  MemStore *p = ((MemFile*)pFile)->pStore;
  முழு rc = SQLITE_OK;
  memdbEnter(p);
  எனில்( size>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }இல்லையெனில்{
    p->sz = size; 
  }
  memdbLeave(p);
  திரும்பு rc;
}

/*
** Sync an memdb-file.
*/
நிலையான முழு memdbSync(sqlite3_file *pFile, முழு flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  திரும்பு SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
நிலையான முழு memdbFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *pSize = p->sz;
  memdbLeave(p);
  திரும்பு SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
நிலையான முழு memdbLock(sqlite3_file *pFile, முழு eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  முழு rc = SQLITE_OK;
  எனில்( eLock<=pThis->eLock ) திரும்பு SQLITE_OK;
  memdbEnter(p);

  ப்ரதிஜ்ஞா( p->nWrLock==0 || p->nWrLock==1 );
  ப்ரதிஜ்ஞா( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  ப்ரதிஜ்ஞா( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  எனில்( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }இல்லையெனில்{
    தேர்வு( eLock ){
      நிலை SQLITE_LOCK_SHARED: {
        ப்ரதிஜ்ஞா( pThis->eLock==SQLITE_LOCK_NONE );
        எனில்( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }இல்லையெனில்{
          p->nRdLock++;
        }
        நிறுத்து;
      };
  
      நிலை SQLITE_LOCK_RESERVED:
      நிலை SQLITE_LOCK_PENDING: {
        ப்ரதிஜ்ஞா( pThis->eLock>=SQLITE_LOCK_SHARED );
        எனில்( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          எனில்( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }இல்லையெனில்{
            p->nWrLock = 1;
          }
        }
        நிறுத்து;
      }
  
      இயல்பு: {
        ப்ரதிஜ்ஞா(  eLock==SQLITE_LOCK_EXCLUSIVE );
        ப்ரதிஜ்ஞா( pThis->eLock>=SQLITE_LOCK_SHARED );
        எனில்( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }இல்லையெனில் எனில்( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        நிறுத்து;
      }
    }
  }
  எனில்( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  திரும்பு rc;
}

/*
** Unlock an memdb-file.
*/
நிலையான முழு memdbUnlock(sqlite3_file *pFile, முழு eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  எனில்( eLock>=pThis->eLock ) திரும்பு SQLITE_OK;
  memdbEnter(p);

  ப்ரதிஜ்ஞா( eLock==SQLITE_LOCK_SHARED || eLock==SQLITE_LOCK_NONE );
  எனில்( eLock==SQLITE_LOCK_SHARED ){
    எனில்( ALWAYS(pThis->eLock>SQLITE_LOCK_SHARED) ){
      p->nWrLock--;
    }
  }இல்லையெனில்{
    எனில்( pThis->eLock>SQLITE_LOCK_SHARED ){
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  திரும்பு SQLITE_OK;
}

#எனில் 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
நிலையான முழு memdbCheckReservedLock(sqlite3_file *pFile, முழு *pResOut){
  *pResOut = 0;
  திரும்பு SQLITE_OK;
}
#endif


/*
** File control method. For custom operations on an memdb-file.
*/
நிலையான முழு memdbFileControl(sqlite3_file *pFile, முழு op, வெறுமை *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  முழு rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  எனில்( op==SQLITE_FCNTL_VFSNAME ){
    *(எழுத்து**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->aData, p->sz);
    rc = SQLITE_OK;
  }
  எனில்( op==SQLITE_FCNTL_SIZE_LIMIT ){
    sqlite3_int64 iLimit = *(sqlite3_int64*)pArg;
    எனில்( iLimit<p->sz ){
      எனில்( iLimit<0 ){
        iLimit = p->szMax;
      }இல்லையெனில்{
        iLimit = p->sz;
      }
    }
    p->szMax = iLimit;
    *(sqlite3_int64*)pArg = iLimit;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  திரும்பு rc;
}

#எனில் 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
நிலையான முழு memdbSectorSize(sqlite3_file *pFile){
  திரும்பு 1024;
}
#endif

/*
** Return the device characteristic flags supported by an memdb-file.
*/
நிலையான முழு memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  திரும்பு SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
நிலையான முழு memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  முழு iAmt,
  வெறுமை **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  எனில்( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }இல்லையெனில்{
    p->nMmap++;
    *pp = (வெறுமை*)(p->aData + iOfst);
  }
  memdbLeave(p);
  திரும்பு SQLITE_OK;
}

/* Release a memory-mapped page */
நிலையான முழு memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, வெறுமை *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  திரும்பு SQLITE_OK;
}

/*
** Open an mem file handle.
*/
நிலையான முழு memdbOpen(
  sqlite3_vfs *pVfs,
  மாறிலி எழுத்து *zName,
  sqlite3_file *pFd,
  முழு flags,
  முழு *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  முழு szName;
  UNUSED_PARAMETER(pVfs);

  memset(pFile, 0, அளவு(*pFile));
  szName = sqlite3Strlen30(zName);
  எனில்( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    முழு i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    க்கு(i=0; i<memdb_g.nMemStore; i++){
      எனில்( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        நிறுத்து;
      }
    }
    எனில்( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( அளவு(*p) + (i64)szName + 3 );
      எனில்( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        திரும்பு SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             அளவு(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      எனில்( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        திரும்பு SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, அளவு(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mxMemdbSize;
      p->zFName = (எழுத்து*)&p[1];
      memcpy(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      எனில்( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        திரும்பு SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }இல்லையெனில்{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }இல்லையெனில்{
    p = sqlite3Malloc( அளவு(*p) );
    எனில்( p==0 ){
      திரும்பு SQLITE_NOMEM;
    }
    memset(p, 0, அளவு(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mxMemdbSize;
  }
  pFile->pStore = p;
  எனில்( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  திரும்பு SQLITE_OK;
}

#எனில் 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
நிலையான முழு memdbDelete(sqlite3_vfs *pVfs, மாறிலி எழுத்து *zPath, முழு dirSync){
  திரும்பு SQLITE_IOERR_DELETE;
}
#endif

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
நிலையான முழு memdbAccess(
  sqlite3_vfs *pVfs, 
  மாறிலி எழுத்து *zPath, 
  முழு flags, 
  முழு *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  திரும்பு SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
நிலையான முழு memdbFullPathname(
  sqlite3_vfs *pVfs, 
  மாறிலி எழுத்து *zPath, 
  முழு nOut, 
  எழுத்து *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  திரும்பு SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
நிலையான வெறுமை *memdbDlOpen(sqlite3_vfs *pVfs, மாறிலி எழுத்து *zPath){
  திரும்பு ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
நிலையான வெறுமை memdbDlError(sqlite3_vfs *pVfs, முழு nByte, எழுத்து *z_Err_செய்தி){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_செய்தி);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
நிலையான வெறுமை (*memdbDlSym(sqlite3_vfs *pVfs, வெறுமை *p, மாறிலி எழுத்து *zSym))(வெறுமை){
  திரும்பு ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
நிலையான வெறுமை memdbDlClose(sqlite3_vfs *pVfs, வெறுமை *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
நிலையான முழு memdbRandomness(sqlite3_vfs *pVfs, முழு nByte, எழுத்து *zBufOut){
  திரும்பு ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
நிலையான முழு memdbSleep(sqlite3_vfs *pVfs, முழு nMicro){
  திரும்பு ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#எனில் 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
நிலையான முழு memdbCurrentTime(sqlite3_vfs *pVfs, இரட்டை *pTimeOut){
  திரும்பு ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#endif

நிலையான முழு memdbGetLastError(sqlite3_vfs *pVfs, முழு a, எழுத்து *b){
  திரும்பு ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
நிலையான முழு memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  திரும்பு ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
நிலையான MemFile *memdbFromDbSchema(sqlite3 *db, மாறிலி எழுத்து *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  முழு rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  எனில்( rc ) திரும்பு 0;
  எனில்( p->base.pMethods!=&memdb_io_methods ) திரும்பு 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  எனில்( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  திரும்பு p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
unsigned எழுத்து *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  மாறிலி எழுத்து *zSchema,      /* Which database within the connection */
  sqlite3_int64 *piSize,    /* Write size here, if not NULL */
  unsigned முழு mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  முழு iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  முழு szPage = 0;
  sqlite3_stmt *pStmt = 0;
  unsigned எழுத்து *pOut = 0;
  எழுத்து *zSql;
  முழு rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( !sqlite3SafetyCheckOk(db) ){
    (வெறுமை)SQLITE_MISUSE_BKPT;
    திரும்பு 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  எனில்( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  எனில்( piSize ) *piSize = -1;
  எனில்( iDb<0 ) செல் serialize_out;
  எனில்( p ){
    MemStore *pStore = p->pStore;
    ப்ரதிஜ்ஞா( pStore->pMutex==0 );
    எனில்( piSize ) *piSize = pStore->sz;
    எனில்( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->aData;
    }இல்லையெனில்{
      pOut = sqlite3_malloc64( pStore->sz );
      எனில்( pOut ) memcpy(pOut, pStore->aData, pStore->sz);
    }
    செல் serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  எனில்( pBt==0 ) செல் serialize_out;
  szPage = sqlite3BtreeGetPageSize(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  எனில்( rc ) செல் serialize_out;
  rc = sqlite3_step(pStmt);
  எனில்( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    எனில்( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      எனில்( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    எனில்( piSize ) *piSize = sz;
    எனில்( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }இல்லையெனில்{
      pOut = sqlite3_malloc64( sz );
      எனில்( pOut ){
        முழு nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        முழு pgno;
        க்கு(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          unsigned எழுத்து *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          எனில்( rc==SQLITE_OK ){
            memcpy(pTo, sqlite3PagerGetData(pPage), szPage);
          }இல்லையெனில் எனில்( pgno==PENDING_BYTE/szPage+1 ){
            memset(pTo, 0, szPage);
          }இல்லையெனில்{
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
  திரும்பு pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
முழு sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  மாறிலி எழுத்து *zSchema,    /* Which DB to reopen with the deserialization */
  unsigned எழுத்து *pData,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  unsigned mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  எழுத்து *zSql;
  sqlite3_stmt *pStmt = 0;
  முழு rc;
  முழு iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  எனில்( !sqlite3SafetyCheckOk(db) ){
    திரும்பு SQLITE_MISUSE_BKPT;
  }
  எனில்( szDb<0 ) திரும்பு SQLITE_MISUSE_BKPT;
  எனில்( szBuf<0 ) திரும்பு SQLITE_MISUSE_BKPT;
#endif

  sqlite3_mutex_enter(db->mutex);
  எனில்( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  எனில்( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    செல் end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  எனில்( zSql==0 ){
    rc = SQLITE_NOMEM;
  }இல்லையெனில்{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  எனில்( rc ) செல் end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  எனில்( rc!=SQLITE_OK ){
    செல் end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  எனில்( p==0 ){
    rc = SQLITE_ERROR;
  }இல்லையெனில்{
    MemStore *pStore = p->pStore;
    pStore->aData = pData;
    pData = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    எனில்( pStore->szMax<sqlite3GlobalConfig.mxMemdbSize ){
      pStore->szMax = sqlite3GlobalConfig.mxMemdbSize;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  எனில்( pData && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(pData);
  }
  sqlite3_mutex_leave(db->mutex);
  திரும்பு rc;
}

/*
** Return true if the VFS is the memvfs.
*/
முழு sqlite3IsMemdb(மாறிலி sqlite3_vfs *pVfs){
  திரும்பு pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
முழு sqlite3MemdbInit(வெறுமை){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  unsigned முழு sz;
  எனில்( NEVER(pLower==0) ) திரும்பு SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.pAppData = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  எனில்( sz<அளவு(MemFile) ) sz = அளவு(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  திரும்பு sqlite3_vfs_register(&memdb_vfs, 0);
}
#endif /* SQLITE_OMIT_DESERIALIZE */
