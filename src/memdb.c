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
  ຈຳນວນເຕັມ nMmap;                      /* Number of memory mapped pages */
  unsigned mFlags;                /* Flags */
  ຈຳນວນເຕັມ nRdLock;                    /* Number of readers */
  ຈຳນວນເຕັມ nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  ຈຳນວນເຕັມ nRef;                       /* Number of users of this MemStore */
  var_na *zFName;                   /* The filename for shared stores */
};

/* An open file */
sa_nracanaa MemFile {
  sqlite3_file base;              /* IO methods */
  MemStore *pStore;               /* The storage */
  ຈຳນວນເຕັມ eLock;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
static sa_nracanaa MemFS {
  ຈຳນວນເຕັມ nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
static ຈຳນວນເຕັມ memdbClose(sqlite3_file*);
static ຈຳນວນເຕັມ memdbRead(sqlite3_file*, void*, ຈຳນວນເຕັມ iAmt, sqlite3_int64 iOfst);
static ຈຳນວນເຕັມ memdbWrite(sqlite3_file*,const void*,ຈຳນວນເຕັມ iAmt, sqlite3_int64 iOfst);
static ຈຳນວນເຕັມ memdbTruncate(sqlite3_file*, sqlite3_int64 size);
static ຈຳນວນເຕັມ memdbSync(sqlite3_file*, ຈຳນວນເຕັມ flags);
static ຈຳນວນເຕັມ memdbFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static ຈຳນວນເຕັມ memdbLock(sqlite3_file*, ຈຳນວນເຕັມ);
static ຈຳນວນເຕັມ memdbUnlock(sqlite3_file*, ຈຳນວນເຕັມ);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
static ຈຳນວນເຕັມ memdbFileControl(sqlite3_file*, ຈຳນວນເຕັມ op, void *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
static ຈຳນວນເຕັມ memdbDeviceCharacteristics(sqlite3_file*);
static ຈຳນວນເຕັມ memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, ຈຳນວນເຕັມ iAmt, void **pp);
static ຈຳນວນເຕັມ memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, void *p);

/*
** Methods for MemVfs
*/
static ຈຳນວນເຕັມ memdbOpen(sqlite3_vfs*, const var_na *, sqlite3_file*, ຈຳນວນເຕັມ , ຈຳນວນເຕັມ *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
static ຈຳນວນເຕັມ memdbAccess(sqlite3_vfs*, const var_na *zName, ຈຳນວນເຕັມ flags, ຈຳນວນເຕັມ *);
static ຈຳນວນເຕັມ memdbFullPathname(sqlite3_vfs*, const var_na *zName, ຈຳນວນເຕັມ, var_na *zOut);
static void *memdbDlOpen(sqlite3_vfs*, const var_na *zFilename);
static void memdbDlError(sqlite3_vfs*, ຈຳນວນເຕັມ nByte, var_na *zErrMsg);
static void (*memdbDlSym(sqlite3_vfs *pVfs, void *p, const var_na*zSym))(void);
static void memdbDlClose(sqlite3_vfs*, void*);
static ຈຳນວນເຕັມ memdbRandomness(sqlite3_vfs*, ຈຳນວນເຕັມ nByte, var_na *zOut);
static ຈຳນວນເຕັມ memdbSleep(sqlite3_vfs*, ຈຳນວນເຕັມ microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
static ຈຳນວນເຕັມ memdbGetLastError(sqlite3_vfs*, ຈຳນວນເຕັມ, var_na *);
static ຈຳນວນເຕັມ memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

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
#ຖ້າ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
static void memdbEnter(MemStore *p){
  UNUSED_PARAMETER(p);
}
static void memdbLeave(MemStore *p){
  UNUSED_PARAMETER(p);
}
#ບໍ່ດັ່ງນັ້ນ
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
static ຈຳນວນເຕັມ memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ຖ້າ( p->zFName ){
    ຈຳນວນເຕັມ i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    ສຳລັບ(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      ຖ້າ( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        ຖ້າ( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          ຖ້າ( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        bha_nga;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }ບໍ່ດັ່ງນັ້ນ{
    memdbEnter(p);
  }
  p->nRef--;
  ຖ້າ( p->nRef<=0 ){
    ຖ້າ( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->aData);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }ບໍ່ດັ່ງນັ້ນ{
    memdbLeave(p);
  }
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
static ຈຳນວນເຕັມ memdbRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  ຈຳນວນເຕັມ iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ຖ້າ( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    ຖ້າ( iOfst<p->sz ) memcpy(zBuf, p->aData+iOfst, p->sz - iOfst);
    memdbLeave(p);
    ສົ່ງຄືນ SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, p->aData+iOfst, iAmt);
  memdbLeave(p);
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
static ຈຳນວນເຕັມ memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  unsigned var_na *pNew;
  ຖ້າ( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    ສົ່ງຄືນ SQLITE_FULL;
  }
  ຖ້າ( newSz>p->szMax ){
    ສົ່ງຄືນ SQLITE_FULL;
  }
  newSz *= 2;
  ຖ້າ( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->aData, newSz);
  ຖ້າ( pNew==0 ) ສົ່ງຄືນ SQLITE_IOERR_NOMEM;
  p->aData = pNew;
  p->szAlloc = newSz;
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
static ຈຳນວນເຕັມ memdbWrite(
  sqlite3_file *pFile,
  const void *z,
  ຈຳນວນເຕັມ iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ຖ້າ( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    ສົ່ງຄືນ SQLITE_IOERR_WRITE;
  }
  ຖ້າ( iOfst+iAmt>p->sz ){
    ຈຳນວນເຕັມ rc;
    ຖ້າ( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      ສົ່ງຄືນ rc;
    }
    ຖ້າ( iOfst>p->sz ) memset(p->aData+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  memcpy(p->aData+iOfst, z, iAmt);
  memdbLeave(p);
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
static ຈຳນວນເຕັມ memdbTruncate(sqlite3_file *pFile, sqlite_int64 size){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ຈຳນວນເຕັມ rc = SQLITE_OK;
  memdbEnter(p);
  ຖ້າ( size>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }ບໍ່ດັ່ງນັ້ນ{
    p->sz = size; 
  }
  memdbLeave(p);
  ສົ່ງຄືນ rc;
}

/*
** Sync an memdb-file.
*/
static ຈຳນວນເຕັມ memdbSync(sqlite3_file *pFile, ຈຳນວນເຕັມ flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
static ຈຳນວນເຕັມ memdbFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *pSize = p->sz;
  memdbLeave(p);
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
static ຈຳນວນເຕັມ memdbLock(sqlite3_file *pFile, ຈຳນວນເຕັມ eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  ຈຳນວນເຕັມ rc = SQLITE_OK;
  ຖ້າ( eLock<=pThis->eLock ) ສົ່ງຄືນ SQLITE_OK;
  memdbEnter(p);

  assert( p->nWrLock==0 || p->nWrLock==1 );
  assert( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  assert( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  ຖ້າ( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }ບໍ່ດັ່ງນັ້ນ{
    switch( eLock ){
      pak_sa SQLITE_LOCK_SHARED: {
        assert( pThis->eLock==SQLITE_LOCK_NONE );
        ຖ້າ( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }ບໍ່ດັ່ງນັ້ນ{
          p->nRdLock++;
        }
        bha_nga;
      };
  
      pak_sa SQLITE_LOCK_RESERVED:
      pak_sa SQLITE_LOCK_PENDING: {
        assert( pThis->eLock>=SQLITE_LOCK_SHARED );
        ຖ້າ( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          ຖ້າ( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }ບໍ່ດັ່ງນັ້ນ{
            p->nWrLock = 1;
          }
        }
        bha_nga;
      }
  
      default: {
        assert(  eLock==SQLITE_LOCK_EXCLUSIVE );
        assert( pThis->eLock>=SQLITE_LOCK_SHARED );
        ຖ້າ( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }ບໍ່ດັ່ງນັ້ນ ຖ້າ( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        bha_nga;
      }
    }
  }
  ຖ້າ( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  ສົ່ງຄືນ rc;
}

/*
** Unlock an memdb-file.
*/
static ຈຳນວນເຕັມ memdbUnlock(sqlite3_file *pFile, ຈຳນວນເຕັມ eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  ຖ້າ( eLock>=pThis->eLock ) ສົ່ງຄືນ SQLITE_OK;
  memdbEnter(p);

  assert( eLock==SQLITE_LOCK_SHARED || eLock==SQLITE_LOCK_NONE );
  ຖ້າ( eLock==SQLITE_LOCK_SHARED ){
    ຖ້າ( ALWAYS(pThis->eLock>SQLITE_LOCK_SHARED) ){
      p->nWrLock--;
    }
  }ບໍ່ດັ່ງນັ້ນ{
    ຖ້າ( pThis->eLock>SQLITE_LOCK_SHARED ){
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  ສົ່ງຄືນ SQLITE_OK;
}

#ຖ້າ 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
static ຈຳນວນເຕັມ memdbCheckReservedLock(sqlite3_file *pFile, ຈຳນວນເຕັມ *pResOut){
  *pResOut = 0;
  ສົ່ງຄືນ SQLITE_OK;
}
#endif


/*
** File control method. For custom operations on an memdb-file.
*/
static ຈຳນວນເຕັມ memdbFileControl(sqlite3_file *pFile, ຈຳນວນເຕັມ op, void *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ຈຳນວນເຕັມ rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  ຖ້າ( op==SQLITE_FCNTL_VFSNAME ){
    *(var_na**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->aData, p->sz);
    rc = SQLITE_OK;
  }
  ຖ້າ( op==SQLITE_FCNTL_SIZE_LIMIT ){
    sqlite3_int64 iLimit = *(sqlite3_int64*)pArg;
    ຖ້າ( iLimit<p->sz ){
      ຖ້າ( iLimit<0 ){
        iLimit = p->szMax;
      }ບໍ່ດັ່ງນັ້ນ{
        iLimit = p->sz;
      }
    }
    p->szMax = iLimit;
    *(sqlite3_int64*)pArg = iLimit;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  ສົ່ງຄືນ rc;
}

#ຖ້າ 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
static ຈຳນວນເຕັມ memdbSectorSize(sqlite3_file *pFile){
  ສົ່ງຄືນ 1024;
}
#endif

/*
** Return the device characteristic flags supported by an memdb-file.
*/
static ຈຳນວນເຕັມ memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  ສົ່ງຄືນ SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
static ຈຳນວນເຕັມ memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  ຈຳນວນເຕັມ iAmt,
  void **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ຖ້າ( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }ບໍ່ດັ່ງນັ້ນ{
    p->nMmap++;
    *pp = (void*)(p->aData + iOfst);
  }
  memdbLeave(p);
  ສົ່ງຄືນ SQLITE_OK;
}

/* Release a memory-mapped page */
static ຈຳນວນເຕັມ memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, void *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Open an mem file handle.
*/
static ຈຳນວນເຕັມ memdbOpen(
  sqlite3_vfs *pVfs,
  const var_na *zName,
  sqlite3_file *pFd,
  ຈຳນວນເຕັມ flags,
  ຈຳນວນເຕັມ *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  ຈຳນວນເຕັມ szName;
  UNUSED_PARAMETER(pVfs);

  memset(pFile, 0, parimaa_na(*pFile));
  szName = sqlite3Strlen30(zName);
  ຖ້າ( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    ຈຳນວນເຕັມ i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    ສຳລັບ(i=0; i<memdb_g.nMemStore; i++){
      ຖ້າ( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        bha_nga;
      }
    }
    ຖ້າ( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( parimaa_na(*p) + (i64)szName + 3 );
      ຖ້າ( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        ສົ່ງຄືນ SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             parimaa_na(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      ຖ້າ( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ສົ່ງຄືນ SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, parimaa_na(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mxMemdbSize;
      p->zFName = (var_na*)&p[1];
      memcpy(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      ຖ້າ( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ສົ່ງຄືນ SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }ບໍ່ດັ່ງນັ້ນ{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }ບໍ່ດັ່ງນັ້ນ{
    p = sqlite3Malloc( parimaa_na(*p) );
    ຖ້າ( p==0 ){
      ສົ່ງຄືນ SQLITE_NOMEM;
    }
    memset(p, 0, parimaa_na(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mxMemdbSize;
  }
  pFile->pStore = p;
  ຖ້າ( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  ສົ່ງຄືນ SQLITE_OK;
}

#ຖ້າ 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static ຈຳນວນເຕັມ memdbDelete(sqlite3_vfs *pVfs, const var_na *zPath, ຈຳນວນເຕັມ dirSync){
  ສົ່ງຄືນ SQLITE_IOERR_DELETE;
}
#endif

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
static ຈຳນວນເຕັມ memdbAccess(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  ຈຳນວນເຕັມ flags, 
  ຈຳນວນເຕັມ *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
static ຈຳນວນເຕັມ memdbFullPathname(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  ຈຳນວນເຕັມ nOut, 
  var_na *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  ສົ່ງຄືນ SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *memdbDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  ສົ່ງຄືນ ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void memdbDlError(sqlite3_vfs *pVfs, ຈຳນວນເຕັມ nByte, var_na *zErrMsg){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*memdbDlSym(sqlite3_vfs *pVfs, void *p, const var_na *zSym))(void){
  ສົ່ງຄືນ ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
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
static ຈຳນວນເຕັມ memdbRandomness(sqlite3_vfs *pVfs, ຈຳນວນເຕັມ nByte, var_na *zBufOut){
  ສົ່ງຄືນ ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static ຈຳນວນເຕັມ memdbSleep(sqlite3_vfs *pVfs, ຈຳນວນເຕັມ nMicro){
  ສົ່ງຄືນ ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#ຖ້າ 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static ຈຳນວນເຕັມ memdbCurrentTime(sqlite3_vfs *pVfs, dviguu_na *pTimeOut){
  ສົ່ງຄືນ ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#endif

static ຈຳນວນເຕັມ memdbGetLastError(sqlite3_vfs *pVfs, ຈຳນວນເຕັມ a, var_na *b){
  ສົ່ງຄືນ ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
static ຈຳນວນເຕັມ memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  ສົ່ງຄືນ ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
static MemFile *memdbFromDbSchema(sqlite3 *db, const var_na *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  ຈຳນວນເຕັມ rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  ຖ້າ( rc ) ສົ່ງຄືນ 0;
  ຖ້າ( p->base.pMethods!=&memdb_io_methods ) ສົ່ງຄືນ 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  ຖ້າ( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  ສົ່ງຄືນ p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
unsigned var_na *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  const var_na *zSchema,      /* Which database within the connection */
  sqlite3_int64 *piSize,    /* Write size here, if not NULL */
  unsigned ຈຳນວນເຕັມ mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  ຈຳນວນເຕັມ iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  ຈຳນວນເຕັມ szPage = 0;
  sqlite3_stmt *pStmt = 0;
  unsigned var_na *pOut = 0;
  var_na *zSql;
  ຈຳນວນເຕັມ rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  ຖ້າ( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    ສົ່ງຄືນ 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  ຖ້າ( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  ຖ້າ( piSize ) *piSize = -1;
  ຖ້າ( iDb<0 ) goto serialize_out;
  ຖ້າ( p ){
    MemStore *pStore = p->pStore;
    assert( pStore->pMutex==0 );
    ຖ້າ( piSize ) *piSize = pStore->sz;
    ຖ້າ( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->aData;
    }ບໍ່ດັ່ງນັ້ນ{
      pOut = sqlite3_malloc64( pStore->sz );
      ຖ້າ( pOut ) memcpy(pOut, pStore->aData, pStore->sz);
    }
    goto serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  ຖ້າ( pBt==0 ) goto serialize_out;
  szPage = sqlite3BtreeGetPageSize(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  ຖ້າ( rc ) goto serialize_out;
  rc = sqlite3_step(pStmt);
  ຖ້າ( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    ຖ້າ( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      ຖ້າ( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    ຖ້າ( piSize ) *piSize = sz;
    ຖ້າ( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }ບໍ່ດັ່ງນັ້ນ{
      pOut = sqlite3_malloc64( sz );
      ຖ້າ( pOut ){
        ຈຳນວນເຕັມ nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        ຈຳນວນເຕັມ pgno;
        ສຳລັບ(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          unsigned var_na *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          ຖ້າ( rc==SQLITE_OK ){
            memcpy(pTo, sqlite3PagerGetData(pPage), szPage);
          }ບໍ່ດັ່ງນັ້ນ ຖ້າ( pgno==PENDING_BYTE/szPage+1 ){
            memset(pTo, 0, szPage);
          }ບໍ່ດັ່ງນັ້ນ{
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
  ສົ່ງຄືນ pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
ຈຳນວນເຕັມ sqlite3_deserialize(
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
  ຈຳນວນເຕັມ rc;
  ຈຳນວນເຕັມ iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  ຖ້າ( !sqlite3SafetyCheckOk(db) ){
    ສົ່ງຄືນ SQLITE_MISUSE_BKPT;
  }
  ຖ້າ( szDb<0 ) ສົ່ງຄືນ SQLITE_MISUSE_BKPT;
  ຖ້າ( szBuf<0 ) ສົ່ງຄືນ SQLITE_MISUSE_BKPT;
#endif

  sqlite3_mutex_enter(db->mutex);
  ຖ້າ( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  ຖ້າ( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    goto end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  ຖ້າ( zSql==0 ){
    rc = SQLITE_NOMEM;
  }ບໍ່ດັ່ງນັ້ນ{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  ຖ້າ( rc ) goto end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  ຖ້າ( rc!=SQLITE_OK ){
    goto end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  ຖ້າ( p==0 ){
    rc = SQLITE_ERROR;
  }ບໍ່ດັ່ງນັ້ນ{
    MemStore *pStore = p->pStore;
    pStore->aData = pData;
    pData = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    ຖ້າ( pStore->szMax<sqlite3GlobalConfig.mxMemdbSize ){
      pStore->szMax = sqlite3GlobalConfig.mxMemdbSize;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  ຖ້າ( pData && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(pData);
  }
  sqlite3_mutex_leave(db->mutex);
  ສົ່ງຄືນ rc;
}

/*
** Return true if the VFS is the memvfs.
*/
ຈຳນວນເຕັມ sqlite3IsMemdb(const sqlite3_vfs *pVfs){
  ສົ່ງຄືນ pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
ຈຳນວນເຕັມ sqlite3MemdbInit(void){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  unsigned ຈຳນວນເຕັມ sz;
  ຖ້າ( NEVER(pLower==0) ) ສົ່ງຄືນ SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.pAppData = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  ຖ້າ( sz<parimaa_na(MemFile) ) sz = parimaa_na(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  ສົ່ງຄືນ sqlite3_vfs_register(&memdb_vfs, 0);
}
#endif /* SQLITE_OMIT_DESERIALIZE */
