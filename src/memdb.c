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
  ཧྲིལ nMmap;                      /* Number of memory mapped pages */
  unsigned mFlags;                /* Flags */
  ཧྲིལ nRdLock;                    /* Number of readers */
  ཧྲིལ nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  ཧྲིལ nRef;                       /* Number of users of this MemStore */
  var_na *zFName;                   /* The filename for shared stores */
};

/* An open file */
sa_nracanaa MemFile {
  sqlite3_file base;              /* IO methods */
  MemStore *pStore;               /* The storage */
  ཧྲིལ eLock;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
static sa_nracanaa MemFS {
  ཧྲིལ nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
static ཧྲིལ memdbClose(sqlite3_file*);
static ཧྲིལ memdbRead(sqlite3_file*, void*, ཧྲིལ iAmt, sqlite3_int64 iOfst);
static ཧྲིལ memdbWrite(sqlite3_file*,const void*,ཧྲིལ iAmt, sqlite3_int64 iOfst);
static ཧྲིལ memdbTruncate(sqlite3_file*, sqlite3_int64 size);
static ཧྲིལ memdbSync(sqlite3_file*, ཧྲིལ flags);
static ཧྲིལ memdbFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static ཧྲིལ memdbLock(sqlite3_file*, ཧྲིལ);
static ཧྲིལ memdbUnlock(sqlite3_file*, ཧྲིལ);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
static ཧྲིལ memdbFileControl(sqlite3_file*, ཧྲིལ op, void *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
static ཧྲིལ memdbDeviceCharacteristics(sqlite3_file*);
static ཧྲིལ memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, ཧྲིལ iAmt, void **pp);
static ཧྲིལ memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, void *p);

/*
** Methods for MemVfs
*/
static ཧྲིལ memdbOpen(sqlite3_vfs*, const var_na *, sqlite3_file*, ཧྲིལ , ཧྲིལ *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
static ཧྲིལ memdbAccess(sqlite3_vfs*, const var_na *zName, ཧྲིལ flags, ཧྲིལ *);
static ཧྲིལ memdbFullPathname(sqlite3_vfs*, const var_na *zName, ཧྲིལ, var_na *zOut);
static void *memdbDlOpen(sqlite3_vfs*, const var_na *zFilename);
static void memdbDlError(sqlite3_vfs*, ཧྲིལ nByte, var_na *zErrMsg);
static void (*memdbDlSym(sqlite3_vfs *pVfs, void *p, const var_na*zSym))(void);
static void memdbDlClose(sqlite3_vfs*, void*);
static ཧྲིལ memdbRandomness(sqlite3_vfs*, ཧྲིལ nByte, var_na *zOut);
static ཧྲིལ memdbSleep(sqlite3_vfs*, ཧྲིལ microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
static ཧྲིལ memdbGetLastError(sqlite3_vfs*, ཧྲིལ, var_na *);
static ཧྲིལ memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

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
#གལ་སྲིད defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
static void memdbEnter(MemStore *p){
  UNUSED_PARAMETER(p);
}
static void memdbLeave(MemStore *p){
  UNUSED_PARAMETER(p);
}
#དེ་མིན
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
static ཧྲིལ memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  གལ་སྲིད( p->zFName ){
    ཧྲིལ i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    དོན་ལུ(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      གལ་སྲིད( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        གལ་སྲིད( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          གལ་སྲིད( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        bha_nga;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }དེ་མིན{
    memdbEnter(p);
  }
  p->nRef--;
  གལ་སྲིད( p->nRef<=0 ){
    གལ་སྲིད( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->aData);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }དེ་མིན{
    memdbLeave(p);
  }
  ལོག SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
static ཧྲིལ memdbRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  ཧྲིལ iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  གལ་སྲིད( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    གལ་སྲིད( iOfst<p->sz ) memcpy(zBuf, p->aData+iOfst, p->sz - iOfst);
    memdbLeave(p);
    ལོག SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, p->aData+iOfst, iAmt);
  memdbLeave(p);
  ལོག SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
static ཧྲིལ memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  unsigned var_na *pNew;
  གལ་སྲིད( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    ལོག SQLITE_FULL;
  }
  གལ་སྲིད( newSz>p->szMax ){
    ལོག SQLITE_FULL;
  }
  newSz *= 2;
  གལ་སྲིད( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->aData, newSz);
  གལ་སྲིད( pNew==0 ) ལོག SQLITE_IOERR_NOMEM;
  p->aData = pNew;
  p->szAlloc = newSz;
  ལོག SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
static ཧྲིལ memdbWrite(
  sqlite3_file *pFile,
  const void *z,
  ཧྲིལ iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  གལ་སྲིད( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    ལོག SQLITE_IOERR_WRITE;
  }
  གལ་སྲིད( iOfst+iAmt>p->sz ){
    ཧྲིལ rc;
    གལ་སྲིད( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      ལོག rc;
    }
    གལ་སྲིད( iOfst>p->sz ) memset(p->aData+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  memcpy(p->aData+iOfst, z, iAmt);
  memdbLeave(p);
  ལོག SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
static ཧྲིལ memdbTruncate(sqlite3_file *pFile, sqlite_int64 size){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ཧྲིལ rc = SQLITE_OK;
  memdbEnter(p);
  གལ་སྲིད( size>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }དེ་མིན{
    p->sz = size; 
  }
  memdbLeave(p);
  ལོག rc;
}

/*
** Sync an memdb-file.
*/
static ཧྲིལ memdbSync(sqlite3_file *pFile, ཧྲིལ flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  ལོག SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
static ཧྲིལ memdbFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *pSize = p->sz;
  memdbLeave(p);
  ལོག SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
static ཧྲིལ memdbLock(sqlite3_file *pFile, ཧྲིལ eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  ཧྲིལ rc = SQLITE_OK;
  གལ་སྲིད( eLock<=pThis->eLock ) ལོག SQLITE_OK;
  memdbEnter(p);

  assert( p->nWrLock==0 || p->nWrLock==1 );
  assert( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  assert( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  གལ་སྲིད( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }དེ་མིན{
    switch( eLock ){
      pak_sa SQLITE_LOCK_SHARED: {
        assert( pThis->eLock==SQLITE_LOCK_NONE );
        གལ་སྲིད( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }དེ་མིན{
          p->nRdLock++;
        }
        bha_nga;
      };
  
      pak_sa SQLITE_LOCK_RESERVED:
      pak_sa SQLITE_LOCK_PENDING: {
        assert( pThis->eLock>=SQLITE_LOCK_SHARED );
        གལ་སྲིད( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          གལ་སྲིད( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }དེ་མིན{
            p->nWrLock = 1;
          }
        }
        bha_nga;
      }
  
      default: {
        assert(  eLock==SQLITE_LOCK_EXCLUSIVE );
        assert( pThis->eLock>=SQLITE_LOCK_SHARED );
        གལ་སྲིད( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }དེ་མིན གལ་སྲིད( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        bha_nga;
      }
    }
  }
  གལ་སྲིད( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  ལོག rc;
}

/*
** Unlock an memdb-file.
*/
static ཧྲིལ memdbUnlock(sqlite3_file *pFile, ཧྲིལ eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  གལ་སྲིད( eLock>=pThis->eLock ) ལོག SQLITE_OK;
  memdbEnter(p);

  assert( eLock==SQLITE_LOCK_SHARED || eLock==SQLITE_LOCK_NONE );
  གལ་སྲིད( eLock==SQLITE_LOCK_SHARED ){
    གལ་སྲིད( ALWAYS(pThis->eLock>SQLITE_LOCK_SHARED) ){
      p->nWrLock--;
    }
  }དེ་མིན{
    གལ་སྲིད( pThis->eLock>SQLITE_LOCK_SHARED ){
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  ལོག SQLITE_OK;
}

#གལ་སྲིད 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
static ཧྲིལ memdbCheckReservedLock(sqlite3_file *pFile, ཧྲིལ *pResOut){
  *pResOut = 0;
  ལོག SQLITE_OK;
}
#endif


/*
** File control method. For custom operations on an memdb-file.
*/
static ཧྲིལ memdbFileControl(sqlite3_file *pFile, ཧྲིལ op, void *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ཧྲིལ rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  གལ་སྲིད( op==SQLITE_FCNTL_VFSNAME ){
    *(var_na**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->aData, p->sz);
    rc = SQLITE_OK;
  }
  གལ་སྲིད( op==SQLITE_FCNTL_SIZE_LIMIT ){
    sqlite3_int64 iLimit = *(sqlite3_int64*)pArg;
    གལ་སྲིད( iLimit<p->sz ){
      གལ་སྲིད( iLimit<0 ){
        iLimit = p->szMax;
      }དེ་མིན{
        iLimit = p->sz;
      }
    }
    p->szMax = iLimit;
    *(sqlite3_int64*)pArg = iLimit;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  ལོག rc;
}

#གལ་སྲིད 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
static ཧྲིལ memdbSectorSize(sqlite3_file *pFile){
  ལོག 1024;
}
#endif

/*
** Return the device characteristic flags supported by an memdb-file.
*/
static ཧྲིལ memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  ལོག SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
static ཧྲིལ memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  ཧྲིལ iAmt,
  void **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  གལ་སྲིད( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }དེ་མིན{
    p->nMmap++;
    *pp = (void*)(p->aData + iOfst);
  }
  memdbLeave(p);
  ལོག SQLITE_OK;
}

/* Release a memory-mapped page */
static ཧྲིལ memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, void *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  ལོག SQLITE_OK;
}

/*
** Open an mem file handle.
*/
static ཧྲིལ memdbOpen(
  sqlite3_vfs *pVfs,
  const var_na *zName,
  sqlite3_file *pFd,
  ཧྲིལ flags,
  ཧྲིལ *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  ཧྲིལ szName;
  UNUSED_PARAMETER(pVfs);

  memset(pFile, 0, parimaa_na(*pFile));
  szName = sqlite3Strlen30(zName);
  གལ་སྲིད( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    ཧྲིལ i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    དོན་ལུ(i=0; i<memdb_g.nMemStore; i++){
      གལ་སྲིད( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        bha_nga;
      }
    }
    གལ་སྲིད( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( parimaa_na(*p) + (i64)szName + 3 );
      གལ་སྲིད( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        ལོག SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             parimaa_na(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      གལ་སྲིད( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ལོག SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, parimaa_na(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mxMemdbSize;
      p->zFName = (var_na*)&p[1];
      memcpy(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      གལ་སྲིད( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ལོག SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }དེ་མིན{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }དེ་མིན{
    p = sqlite3Malloc( parimaa_na(*p) );
    གལ་སྲིད( p==0 ){
      ལོག SQLITE_NOMEM;
    }
    memset(p, 0, parimaa_na(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mxMemdbSize;
  }
  pFile->pStore = p;
  གལ་སྲིད( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  ལོག SQLITE_OK;
}

#གལ་སྲིད 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static ཧྲིལ memdbDelete(sqlite3_vfs *pVfs, const var_na *zPath, ཧྲིལ dirSync){
  ལོག SQLITE_IOERR_DELETE;
}
#endif

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
static ཧྲིལ memdbAccess(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  ཧྲིལ flags, 
  ཧྲིལ *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  ལོག SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
static ཧྲིལ memdbFullPathname(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  ཧྲིལ nOut, 
  var_na *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  ལོག SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *memdbDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  ལོག ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void memdbDlError(sqlite3_vfs *pVfs, ཧྲིལ nByte, var_na *zErrMsg){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*memdbDlSym(sqlite3_vfs *pVfs, void *p, const var_na *zSym))(void){
  ལོག ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
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
static ཧྲིལ memdbRandomness(sqlite3_vfs *pVfs, ཧྲིལ nByte, var_na *zBufOut){
  ལོག ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static ཧྲིལ memdbSleep(sqlite3_vfs *pVfs, ཧྲིལ nMicro){
  ལོག ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#གལ་སྲིད 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static ཧྲིལ memdbCurrentTime(sqlite3_vfs *pVfs, dviguu_na *pTimeOut){
  ལོག ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#endif

static ཧྲིལ memdbGetLastError(sqlite3_vfs *pVfs, ཧྲིལ a, var_na *b){
  ལོག ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
static ཧྲིལ memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  ལོག ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
static MemFile *memdbFromDbSchema(sqlite3 *db, const var_na *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  ཧྲིལ rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  གལ་སྲིད( rc ) ལོག 0;
  གལ་སྲིད( p->base.pMethods!=&memdb_io_methods ) ལོག 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  གལ་སྲིད( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  ལོག p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
unsigned var_na *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  const var_na *zSchema,      /* Which database within the connection */
  sqlite3_int64 *piSize,    /* Write size here, if not NULL */
  unsigned ཧྲིལ mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  ཧྲིལ iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  ཧྲིལ szPage = 0;
  sqlite3_stmt *pStmt = 0;
  unsigned var_na *pOut = 0;
  var_na *zSql;
  ཧྲིལ rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  གལ་སྲིད( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ལོག 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  གལ་སྲིད( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  གལ་སྲིད( piSize ) *piSize = -1;
  གལ་སྲིད( iDb<0 ) goto serialize_out;
  གལ་སྲིད( p ){
    MemStore *pStore = p->pStore;
    assert( pStore->pMutex==0 );
    གལ་སྲིད( piSize ) *piSize = pStore->sz;
    གལ་སྲིད( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->aData;
    }དེ་མིན{
      pOut = sqlite3_malloc64( pStore->sz );
      གལ་སྲིད( pOut ) memcpy(pOut, pStore->aData, pStore->sz);
    }
    goto serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  གལ་སྲིད( pBt==0 ) goto serialize_out;
  szPage = sqlite3BtreeGetPageSize(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  གལ་སྲིད( rc ) goto serialize_out;
  rc = sqlite3_step(pStmt);
  གལ་སྲིད( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    གལ་སྲིད( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      གལ་སྲིད( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    གལ་སྲིད( piSize ) *piSize = sz;
    གལ་སྲིད( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }དེ་མིན{
      pOut = sqlite3_malloc64( sz );
      གལ་སྲིད( pOut ){
        ཧྲིལ nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        ཧྲིལ pgno;
        དོན་ལུ(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          unsigned var_na *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          གལ་སྲིད( rc==SQLITE_OK ){
            memcpy(pTo, sqlite3PagerGetData(pPage), szPage);
          }དེ་མིན གལ་སྲིད( pgno==PENDING_BYTE/szPage+1 ){
            memset(pTo, 0, szPage);
          }དེ་མིན{
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
  ལོག pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
ཧྲིལ sqlite3_deserialize(
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
  ཧྲིལ rc;
  ཧྲིལ iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  གལ་སྲིད( !sqlite3SafetyCheckOk(db) ){
    ལོག SQLITE_MISUSE_BKPT;
  }
  གལ་སྲིད( szDb<0 ) ལོག SQLITE_MISUSE_BKPT;
  གལ་སྲིད( szBuf<0 ) ལོག SQLITE_MISUSE_BKPT;
#endif

  sqlite3_mutex_enter(db->mutex);
  གལ་སྲིད( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  གལ་སྲིད( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    goto end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  གལ་སྲིད( zSql==0 ){
    rc = SQLITE_NOMEM;
  }དེ་མིན{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  གལ་སྲིད( rc ) goto end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  གལ་སྲིད( rc!=SQLITE_OK ){
    goto end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  གལ་སྲིད( p==0 ){
    rc = SQLITE_ERROR;
  }དེ་མིན{
    MemStore *pStore = p->pStore;
    pStore->aData = pData;
    pData = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    གལ་སྲིད( pStore->szMax<sqlite3GlobalConfig.mxMemdbSize ){
      pStore->szMax = sqlite3GlobalConfig.mxMemdbSize;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  གལ་སྲིད( pData && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(pData);
  }
  sqlite3_mutex_leave(db->mutex);
  ལོག rc;
}

/*
** Return true if the VFS is the memvfs.
*/
ཧྲིལ sqlite3IsMemdb(const sqlite3_vfs *pVfs){
  ལོག pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
ཧྲིལ sqlite3MemdbInit(void){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  unsigned ཧྲིལ sz;
  གལ་སྲིད( NEVER(pLower==0) ) ལོག SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.pAppData = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  གལ་སྲིད( sz<parimaa_na(MemFile) ) sz = parimaa_na(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  ལོག sqlite3_vfs_register(&memdb_vfs, 0);
}
#endif /* SQLITE_OMIT_DESERIALIZE */
