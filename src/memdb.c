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
#include "sqliteInt.h"
#ifndef SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
typedef sa_nracanaa sqlite3_vfs MemVfs;
typedef sa_nracanaa MemFile MemFile;
typedef sa_nracanaa MemStore MemStore;

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
sa_nracanaa MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  unsigned var_na *aData;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  පූර්ණ_සංඛ්‍යා nMmap;                      /* Number of memory mapped pages */
  unsigned mFlags;                /* Flags */
  පූර්ණ_සංඛ්‍යා nRdLock;                    /* Number of readers */
  පූර්ණ_සංඛ්‍යා nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  පූර්ණ_සංඛ්‍යා nRef;                       /* Number of users of this MemStore */
  var_na *zFName;                   /* The filename for shared stores */
};

/* An open file */
sa_nracanaa MemFile {
  sqlite3_file base;              /* IO methods */
  MemStore *pStore;               /* The storage */
  පූර්ණ_සංඛ්‍යා eLock;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
static sa_nracanaa MemFS {
  පූර්ණ_සංඛ්‍යා nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
static පූර්ණ_සංඛ්‍යා memdbClose(sqlite3_file*);
static පූර්ණ_සංඛ්‍යා memdbRead(sqlite3_file*, void*, පූර්ණ_සංඛ්‍යා iAmt, sqlite3_int64 iOfst);
static පූර්ණ_සංඛ්‍යා memdbWrite(sqlite3_file*,const void*,පූර්ණ_සංඛ්‍යා iAmt, sqlite3_int64 iOfst);
static පූර්ණ_සංඛ්‍යා memdbTruncate(sqlite3_file*, sqlite3_int64 size);
static පූර්ණ_සංඛ්‍යා memdbSync(sqlite3_file*, පූර්ණ_සංඛ්‍යා flags);
static පූර්ණ_සංඛ්‍යා memdbFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static පූර්ණ_සංඛ්‍යා memdbLock(sqlite3_file*, පූර්ණ_සංඛ්‍යා);
static පූර්ණ_සංඛ්‍යා memdbUnlock(sqlite3_file*, පූර්ණ_සංඛ්‍යා);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
static පූර්ණ_සංඛ්‍යා memdbFileControl(sqlite3_file*, පූර්ණ_සංඛ්‍යා op, void *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
static පූර්ණ_සංඛ්‍යා memdbDeviceCharacteristics(sqlite3_file*);
static පූර්ණ_සංඛ්‍යා memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, පූර්ණ_සංඛ්‍යා iAmt, void **pp);
static පූර්ණ_සංඛ්‍යා memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, void *p);

/*
** Methods for MemVfs
*/
static පූර්ණ_සංඛ්‍යා memdbOpen(sqlite3_vfs*, const var_na *, sqlite3_file*, පූර්ණ_සංඛ්‍යා , පූර්ණ_සංඛ්‍යා *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
static පූර්ණ_සංඛ්‍යා memdbAccess(sqlite3_vfs*, const var_na *zName, පූර්ණ_සංඛ්‍යා flags, පූර්ණ_සංඛ්‍යා *);
static පූර්ණ_සංඛ්‍යා memdbFullPathname(sqlite3_vfs*, const var_na *zName, පූර්ණ_සංඛ්‍යා, var_na *zOut);
static void *memdbDlOpen(sqlite3_vfs*, const var_na *zFilename);
static void memdbDlError(sqlite3_vfs*, පූර්ණ_සංඛ්‍යා nByte, var_na *zErrMsg);
static void (*memdbDlSym(sqlite3_vfs *pVfs, void *p, const var_na*zSym))(void);
static void memdbDlClose(sqlite3_vfs*, void*);
static පූර්ණ_සංඛ්‍යා memdbRandomness(sqlite3_vfs*, පූර්ණ_සංඛ්‍යා nByte, var_na *zOut);
static පූර්ණ_සංඛ්‍යා memdbSleep(sqlite3_vfs*, පූර්ණ_සංඛ්‍යා microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
static පූර්ණ_සංඛ්‍යා memdbGetLastError(sqlite3_vfs*, පූර්ණ_සංඛ්‍යා, var_na *);
static පූර්ණ_සංඛ්‍යා memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

static sqlite3_vfs memdb_vfs = {
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

static const sqlite3_io_methods memdb_io_methods = {
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
#නම් defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
static void memdbEnter(MemStore *p){
  UNUSED_PARAMETER(p);
}
static void memdbLeave(MemStore *p){
  UNUSED_PARAMETER(p);
}
#නැතහොත්
static void memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
static void memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#endif



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
static පූර්ණ_සංඛ්‍යා memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  නම්( p->zFName ){
    පූර්ණ_සංඛ්‍යා i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    සඳහා(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      නම්( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        නම්( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          නම්( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        bha_nga;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }නැතහොත්{
    memdbEnter(p);
  }
  p->nRef--;
  නම්( p->nRef<=0 ){
    නම්( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->aData);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }නැතහොත්{
    memdbLeave(p);
  }
  ආපසු SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
static පූර්ණ_සංඛ්‍යා memdbRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  පූර්ණ_සංඛ්‍යා iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  නම්( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    නම්( iOfst<p->sz ) memcpy(zBuf, p->aData+iOfst, p->sz - iOfst);
    memdbLeave(p);
    ආපසු SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, p->aData+iOfst, iAmt);
  memdbLeave(p);
  ආපසු SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
static පූර්ණ_සංඛ්‍යා memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  unsigned var_na *pNew;
  නම්( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    ආපසු SQLITE_FULL;
  }
  නම්( newSz>p->szMax ){
    ආපසු SQLITE_FULL;
  }
  newSz *= 2;
  නම්( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->aData, newSz);
  නම්( pNew==0 ) ආපසු SQLITE_IOERR_NOMEM;
  p->aData = pNew;
  p->szAlloc = newSz;
  ආපසු SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
static පූර්ණ_සංඛ්‍යා memdbWrite(
  sqlite3_file *pFile,
  const void *z,
  පූර්ණ_සංඛ්‍යා iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  නම්( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    ආපසු SQLITE_IOERR_WRITE;
  }
  නම්( iOfst+iAmt>p->sz ){
    පූර්ණ_සංඛ්‍යා rc;
    නම්( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      ආපසු rc;
    }
    නම්( iOfst>p->sz ) memset(p->aData+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  memcpy(p->aData+iOfst, z, iAmt);
  memdbLeave(p);
  ආපසු SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
static පූර්ණ_සංඛ්‍යා memdbTruncate(sqlite3_file *pFile, sqlite_int64 size){
  MemStore *p = ((MemFile*)pFile)->pStore;
  පූර්ණ_සංඛ්‍යා rc = SQLITE_OK;
  memdbEnter(p);
  නම්( size>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }නැතහොත්{
    p->sz = size; 
  }
  memdbLeave(p);
  ආපසු rc;
}

/*
** Sync an memdb-file.
*/
static පූර්ණ_සංඛ්‍යා memdbSync(sqlite3_file *pFile, පූර්ණ_සංඛ්‍යා flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  ආපසු SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
static පූර්ණ_සංඛ්‍යා memdbFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *pSize = p->sz;
  memdbLeave(p);
  ආපසු SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
static පූර්ණ_සංඛ්‍යා memdbLock(sqlite3_file *pFile, පූර්ණ_සංඛ්‍යා eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  පූර්ණ_සංඛ්‍යා rc = SQLITE_OK;
  නම්( eLock<=pThis->eLock ) ආපසු SQLITE_OK;
  memdbEnter(p);

  assert( p->nWrLock==0 || p->nWrLock==1 );
  assert( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  assert( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  නම්( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }නැතහොත්{
    switch( eLock ){
      pak_sa SQLITE_LOCK_SHARED: {
        assert( pThis->eLock==SQLITE_LOCK_NONE );
        නම්( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }නැතහොත්{
          p->nRdLock++;
        }
        bha_nga;
      };
  
      pak_sa SQLITE_LOCK_RESERVED:
      pak_sa SQLITE_LOCK_PENDING: {
        assert( pThis->eLock>=SQLITE_LOCK_SHARED );
        නම්( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          නම්( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }නැතහොත්{
            p->nWrLock = 1;
          }
        }
        bha_nga;
      }
  
      default: {
        assert(  eLock==SQLITE_LOCK_EXCLUSIVE );
        assert( pThis->eLock>=SQLITE_LOCK_SHARED );
        නම්( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }නැතහොත් නම්( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        bha_nga;
      }
    }
  }
  නම්( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  ආපසු rc;
}

/*
** Unlock an memdb-file.
*/
static පූර්ණ_සංඛ්‍යා memdbUnlock(sqlite3_file *pFile, පූර්ණ_සංඛ්‍යා eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  නම්( eLock>=pThis->eLock ) ආපසු SQLITE_OK;
  memdbEnter(p);

  assert( eLock==SQLITE_LOCK_SHARED || eLock==SQLITE_LOCK_NONE );
  නම්( eLock==SQLITE_LOCK_SHARED ){
    නම්( ALWAYS(pThis->eLock>SQLITE_LOCK_SHARED) ){
      p->nWrLock--;
    }
  }නැතහොත්{
    නම්( pThis->eLock>SQLITE_LOCK_SHARED ){
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  ආපසු SQLITE_OK;
}

#නම් 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
static පූර්ණ_සංඛ්‍යා memdbCheckReservedLock(sqlite3_file *pFile, පූර්ණ_සංඛ්‍යා *pResOut){
  *pResOut = 0;
  ආපසු SQLITE_OK;
}
#endif


/*
** File control method. For custom operations on an memdb-file.
*/
static පූර්ණ_සංඛ්‍යා memdbFileControl(sqlite3_file *pFile, පූර්ණ_සංඛ්‍යා op, void *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  පූර්ණ_සංඛ්‍යා rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  නම්( op==SQLITE_FCNTL_VFSNAME ){
    *(var_na**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->aData, p->sz);
    rc = SQLITE_OK;
  }
  නම්( op==SQLITE_FCNTL_SIZE_LIMIT ){
    sqlite3_int64 iLimit = *(sqlite3_int64*)pArg;
    නම්( iLimit<p->sz ){
      නම්( iLimit<0 ){
        iLimit = p->szMax;
      }නැතහොත්{
        iLimit = p->sz;
      }
    }
    p->szMax = iLimit;
    *(sqlite3_int64*)pArg = iLimit;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  ආපසු rc;
}

#නම් 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
static පූර්ණ_සංඛ්‍යා memdbSectorSize(sqlite3_file *pFile){
  ආපසු 1024;
}
#endif

/*
** Return the device characteristic flags supported by an memdb-file.
*/
static පූර්ණ_සංඛ්‍යා memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  ආපසු SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
static පූර්ණ_සංඛ්‍යා memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  පූර්ණ_සංඛ්‍යා iAmt,
  void **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  නම්( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }නැතහොත්{
    p->nMmap++;
    *pp = (void*)(p->aData + iOfst);
  }
  memdbLeave(p);
  ආපසු SQLITE_OK;
}

/* Release a memory-mapped page */
static පූර්ණ_සංඛ්‍යා memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, void *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  ආපසු SQLITE_OK;
}

/*
** Open an mem file handle.
*/
static පූර්ණ_සංඛ්‍යා memdbOpen(
  sqlite3_vfs *pVfs,
  const var_na *zName,
  sqlite3_file *pFd,
  පූර්ණ_සංඛ්‍යා flags,
  පූර්ණ_සංඛ්‍යා *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  පූර්ණ_සංඛ්‍යා szName;
  UNUSED_PARAMETER(pVfs);

  memset(pFile, 0, parimaa_na(*pFile));
  szName = sqlite3Strlen30(zName);
  නම්( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    පූර්ණ_සංඛ්‍යා i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    සඳහා(i=0; i<memdb_g.nMemStore; i++){
      නම්( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        bha_nga;
      }
    }
    නම්( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( parimaa_na(*p) + (i64)szName + 3 );
      නම්( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        ආපසු SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             parimaa_na(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      නම්( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ආපසු SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, parimaa_na(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mxMemdbSize;
      p->zFName = (var_na*)&p[1];
      memcpy(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      නම්( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ආපසු SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }නැතහොත්{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }නැතහොත්{
    p = sqlite3Malloc( parimaa_na(*p) );
    නම්( p==0 ){
      ආපසු SQLITE_NOMEM;
    }
    memset(p, 0, parimaa_na(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mxMemdbSize;
  }
  pFile->pStore = p;
  නම්( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  ආපසු SQLITE_OK;
}

#නම් 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static පූර්ණ_සංඛ්‍යා memdbDelete(sqlite3_vfs *pVfs, const var_na *zPath, පූර්ණ_සංඛ්‍යා dirSync){
  ආපසු SQLITE_IOERR_DELETE;
}
#endif

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
static පූර්ණ_සංඛ්‍යා memdbAccess(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  පූර්ණ_සංඛ්‍යා flags, 
  පූර්ණ_සංඛ්‍යා *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  ආපසු SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
static පූර්ණ_සංඛ්‍යා memdbFullPathname(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  පූර්ණ_සංඛ්‍යා nOut, 
  var_na *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  ආපසු SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *memdbDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  ආපසු ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void memdbDlError(sqlite3_vfs *pVfs, පූර්ණ_සංඛ්‍යා nByte, var_na *zErrMsg){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*memdbDlSym(sqlite3_vfs *pVfs, void *p, const var_na *zSym))(void){
  ආපසු ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
static void memdbDlClose(sqlite3_vfs *pVfs, void *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
static පූර්ණ_සංඛ්‍යා memdbRandomness(sqlite3_vfs *pVfs, පූර්ණ_සංඛ්‍යා nByte, var_na *zBufOut){
  ආපසු ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static පූර්ණ_සංඛ්‍යා memdbSleep(sqlite3_vfs *pVfs, පූර්ණ_සංඛ්‍යා nMicro){
  ආපසු ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#නම් 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static පූර්ණ_සංඛ්‍යා memdbCurrentTime(sqlite3_vfs *pVfs, dviguu_na *pTimeOut){
  ආපසු ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#endif

static පූර්ණ_සංඛ්‍යා memdbGetLastError(sqlite3_vfs *pVfs, පූර්ණ_සංඛ්‍යා a, var_na *b){
  ආපසු ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
static පූර්ණ_සංඛ්‍යා memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  ආපසු ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
static MemFile *memdbFromDbSchema(sqlite3 *db, const var_na *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  පූර්ණ_සංඛ්‍යා rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  නම්( rc ) ආපසු 0;
  නම්( p->base.pMethods!=&memdb_io_methods ) ආපසු 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  නම්( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  ආපසු p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
unsigned var_na *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  const var_na *zSchema,      /* Which database within the connection */
  sqlite3_int64 *piSize,    /* Write size here, if not NULL */
  unsigned පූර්ණ_සංඛ්‍යා mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  පූර්ණ_සංඛ්‍යා iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  පූර්ණ_සංඛ්‍යා szPage = 0;
  sqlite3_stmt *pStmt = 0;
  unsigned var_na *pOut = 0;
  var_na *zSql;
  පූර්ණ_සංඛ්‍යා rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  නම්( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ආපසු 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  නම්( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  නම්( piSize ) *piSize = -1;
  නම්( iDb<0 ) goto serialize_out;
  නම්( p ){
    MemStore *pStore = p->pStore;
    assert( pStore->pMutex==0 );
    නම්( piSize ) *piSize = pStore->sz;
    නම්( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->aData;
    }නැතහොත්{
      pOut = sqlite3_malloc64( pStore->sz );
      නම්( pOut ) memcpy(pOut, pStore->aData, pStore->sz);
    }
    goto serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  නම්( pBt==0 ) goto serialize_out;
  szPage = sqlite3BtreeGetPageSize(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  නම්( rc ) goto serialize_out;
  rc = sqlite3_step(pStmt);
  නම්( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    නම්( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      නම්( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    නම්( piSize ) *piSize = sz;
    නම්( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }නැතහොත්{
      pOut = sqlite3_malloc64( sz );
      නම්( pOut ){
        පූර්ණ_සංඛ්‍යා nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        පූර්ණ_සංඛ්‍යා pgno;
        සඳහා(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          unsigned var_na *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          නම්( rc==SQLITE_OK ){
            memcpy(pTo, sqlite3PagerGetData(pPage), szPage);
          }නැතහොත් නම්( pgno==PENDING_BYTE/szPage+1 ){
            memset(pTo, 0, szPage);
          }නැතහොත්{
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
  ආපසු pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
පූර්ණ_සංඛ්‍යා sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  const var_na *zSchema,    /* Which DB to reopen with the deserialization */
  unsigned var_na *pData,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  unsigned mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  var_na *zSql;
  sqlite3_stmt *pStmt = 0;
  පූර්ණ_සංඛ්‍යා rc;
  පූර්ණ_සංඛ්‍යා iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  නම්( !sqlite3SafetyCheckOk(db) ){
    ආපසු SQLITE_MISUSE_BKPT;
  }
  නම්( szDb<0 ) ආපසු SQLITE_MISUSE_BKPT;
  නම්( szBuf<0 ) ආපසු SQLITE_MISUSE_BKPT;
#endif

  sqlite3_mutex_enter(db->mutex);
  නම්( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  නම්( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    goto end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  නම්( zSql==0 ){
    rc = SQLITE_NOMEM;
  }නැතහොත්{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  නම්( rc ) goto end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  නම්( rc!=SQLITE_OK ){
    goto end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  නම්( p==0 ){
    rc = SQLITE_ERROR;
  }නැතහොත්{
    MemStore *pStore = p->pStore;
    pStore->aData = pData;
    pData = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    නම්( pStore->szMax<sqlite3GlobalConfig.mxMemdbSize ){
      pStore->szMax = sqlite3GlobalConfig.mxMemdbSize;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  නම්( pData && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(pData);
  }
  sqlite3_mutex_leave(db->mutex);
  ආපසු rc;
}

/*
** Return true if the VFS is the memvfs.
*/
පූර්ණ_සංඛ්‍යා sqlite3IsMemdb(const sqlite3_vfs *pVfs){
  ආපසු pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
පූර්ණ_සංඛ්‍යා sqlite3MemdbInit(void){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  unsigned පූර්ණ_සංඛ්‍යා sz;
  නම්( NEVER(pLower==0) ) ආපසු SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.pAppData = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  නම්( sz<parimaa_na(MemFile) ) sz = parimaa_na(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  ආපසු sqlite3_vfs_register(&memdb_vfs, 0);
}
#endif /* SQLITE_OMIT_DESERIALIZE */
