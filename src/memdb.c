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
#شامل "sqliteInt.h"
#ifndef SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
تعریفنوع ساختار sqlite3_vfs MemVfs;
تعریفنوع ساختار MemFile MemFile;
تعریفنوع ساختار MemStore MemStore;

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
ساختار MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  unsigned نویسه *aData;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  صحیح nMmap;                      /* Number of memory mapped pages */
  unsigned mFlags;                /* Flags */
  صحیح nRdLock;                    /* Number of readers */
  صحیح nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  صحیح nRef;                       /* Number of users of this MemStore */
  نویسه *zFName;                   /* The filename for shared stores */
};

/* An open file */
ساختار MemFile {
  sqlite3_file base;              /* IO methods */
  MemStore *pStore;               /* The storage */
  صحیح eLock;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
ایستا ساختار MemFS {
  صحیح nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
ایستا صحیح memdbClose(sqlite3_file*);
ایستا صحیح memdbRead(sqlite3_file*, تهی*, صحیح iAmt, sqlite3_int64 iOfst);
ایستا صحیح memdbWrite(sqlite3_file*,ثابت تهی*,صحیح iAmt, sqlite3_int64 iOfst);
ایستا صحیح memdbTruncate(sqlite3_file*, sqlite3_int64 size);
ایستا صحیح memdbSync(sqlite3_file*, صحیح flags);
ایستا صحیح memdbFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ایستا صحیح memdbLock(sqlite3_file*, صحیح);
ایستا صحیح memdbUnlock(sqlite3_file*, صحیح);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
ایستا صحیح memdbFileControl(sqlite3_file*, صحیح op, تهی *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
ایستا صحیح memdbDeviceCharacteristics(sqlite3_file*);
ایستا صحیح memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, صحیح iAmt, تهی **pp);
ایستا صحیح memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, تهی *p);

/*
** Methods for MemVfs
*/
ایستا صحیح memdbOpen(sqlite3_vfs*, ثابت نویسه *, sqlite3_file*, صحیح , صحیح *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
ایستا صحیح memdbAccess(sqlite3_vfs*, ثابت نویسه *zName, صحیح flags, صحیح *);
ایستا صحیح memdbFullPathname(sqlite3_vfs*, ثابت نویسه *zName, صحیح, نویسه *zOut);
ایستا تهی *memdbDlOpen(sqlite3_vfs*, ثابت نویسه *zFilename);
ایستا تهی memdbDlError(sqlite3_vfs*, صحیح nByte, نویسه *z_Err_پیام);
ایستا تهی (*memdbDlSym(sqlite3_vfs *pVfs, تهی *p, ثابت نویسه*zSym))(تهی);
ایستا تهی memdbDlClose(sqlite3_vfs*, تهی*);
ایستا صحیح memdbRandomness(sqlite3_vfs*, صحیح nByte, نویسه *zOut);
ایستا صحیح memdbSleep(sqlite3_vfs*, صحیح microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
ایستا صحیح memdbGetLastError(sqlite3_vfs*, صحیح, نویسه *);
ایستا صحیح memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

ایستا sqlite3_vfs memdb_vfs = {
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

ایستا ثابت sqlite3_io_methods memdb_io_methods = {
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
#اگر defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
ایستا تهی memdbEnter(MemStore *p){
  UNUSED_PARAMETER(p);
}
ایستا تهی memdbLeave(MemStore *p){
  UNUSED_PARAMETER(p);
}
#وگرنه
ایستا تهی memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
ایستا تهی memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#endif



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
ایستا صحیح memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  اگر( p->zFName ){
    صحیح i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    برای(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      اگر( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        اگر( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          اگر( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        بشکن;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }وگرنه{
    memdbEnter(p);
  }
  p->nRef--;
  اگر( p->nRef<=0 ){
    اگر( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->aData);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }وگرنه{
    memdbLeave(p);
  }
  بازگشت SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
ایستا صحیح memdbRead(
  sqlite3_file *pFile, 
  تهی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  اگر( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    اگر( iOfst<p->sz ) memcpy(zBuf, p->aData+iOfst, p->sz - iOfst);
    memdbLeave(p);
    بازگشت SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, p->aData+iOfst, iAmt);
  memdbLeave(p);
  بازگشت SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
ایستا صحیح memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  unsigned نویسه *pNew;
  اگر( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    بازگشت SQLITE_FULL;
  }
  اگر( newSz>p->szMax ){
    بازگشت SQLITE_FULL;
  }
  newSz *= 2;
  اگر( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->aData, newSz);
  اگر( pNew==0 ) بازگشت SQLITE_IOERR_NOMEM;
  p->aData = pNew;
  p->szAlloc = newSz;
  بازگشت SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
ایستا صحیح memdbWrite(
  sqlite3_file *pFile,
  ثابت تهی *z,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  اگر( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    بازگشت SQLITE_IOERR_WRITE;
  }
  اگر( iOfst+iAmt>p->sz ){
    صحیح rc;
    اگر( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      بازگشت rc;
    }
    اگر( iOfst>p->sz ) memset(p->aData+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  memcpy(p->aData+iOfst, z, iAmt);
  memdbLeave(p);
  بازگشت SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
ایستا صحیح memdbTruncate(sqlite3_file *pFile, sqlite_int64 size){
  MemStore *p = ((MemFile*)pFile)->pStore;
  صحیح rc = SQLITE_OK;
  memdbEnter(p);
  اگر( size>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }وگرنه{
    p->sz = size; 
  }
  memdbLeave(p);
  بازگشت rc;
}

/*
** Sync an memdb-file.
*/
ایستا صحیح memdbSync(sqlite3_file *pFile, صحیح flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  بازگشت SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
ایستا صحیح memdbFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *pSize = p->sz;
  memdbLeave(p);
  بازگشت SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
ایستا صحیح memdbLock(sqlite3_file *pFile, صحیح eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  صحیح rc = SQLITE_OK;
  اگر( eLock<=pThis->eLock ) بازگشت SQLITE_OK;
  memdbEnter(p);

  أكد( p->nWrLock==0 || p->nWrLock==1 );
  أكد( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  أكد( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  اگر( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }وگرنه{
    انتخاب( eLock ){
      حالت SQLITE_LOCK_SHARED: {
        أكد( pThis->eLock==SQLITE_LOCK_NONE );
        اگر( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }وگرنه{
          p->nRdLock++;
        }
        بشکن;
      };
  
      حالت SQLITE_LOCK_RESERVED:
      حالت SQLITE_LOCK_PENDING: {
        أكد( pThis->eLock>=SQLITE_LOCK_SHARED );
        اگر( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          اگر( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }وگرنه{
            p->nWrLock = 1;
          }
        }
        بشکن;
      }
  
      پیشفرض: {
        أكد(  eLock==SQLITE_LOCK_EXCLUSIVE );
        أكد( pThis->eLock>=SQLITE_LOCK_SHARED );
        اگر( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }وگرنه اگر( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        بشکن;
      }
    }
  }
  اگر( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  بازگشت rc;
}

/*
** Unlock an memdb-file.
*/
ایستا صحیح memdbUnlock(sqlite3_file *pFile, صحیح eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  اگر( eLock>=pThis->eLock ) بازگشت SQLITE_OK;
  memdbEnter(p);

  أكد( eLock==SQLITE_LOCK_SHARED || eLock==SQLITE_LOCK_NONE );
  اگر( eLock==SQLITE_LOCK_SHARED ){
    اگر( ALWAYS(pThis->eLock>SQLITE_LOCK_SHARED) ){
      p->nWrLock--;
    }
  }وگرنه{
    اگر( pThis->eLock>SQLITE_LOCK_SHARED ){
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  بازگشت SQLITE_OK;
}

#اگر 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
ایستا صحیح memdbCheckReservedLock(sqlite3_file *pFile, صحیح *pResOut){
  *pResOut = 0;
  بازگشت SQLITE_OK;
}
#endif


/*
** File control method. For custom operations on an memdb-file.
*/
ایستا صحیح memdbFileControl(sqlite3_file *pFile, صحیح op, تهی *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  صحیح rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  اگر( op==SQLITE_FCNTL_VFSNAME ){
    *(نویسه**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->aData, p->sz);
    rc = SQLITE_OK;
  }
  اگر( op==SQLITE_FCNTL_SIZE_LIMIT ){
    sqlite3_int64 iLimit = *(sqlite3_int64*)pArg;
    اگر( iLimit<p->sz ){
      اگر( iLimit<0 ){
        iLimit = p->szMax;
      }وگرنه{
        iLimit = p->sz;
      }
    }
    p->szMax = iLimit;
    *(sqlite3_int64*)pArg = iLimit;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  بازگشت rc;
}

#اگر 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
ایستا صحیح memdbSectorSize(sqlite3_file *pFile){
  بازگشت 1024;
}
#endif

/*
** Return the device characteristic flags supported by an memdb-file.
*/
ایستا صحیح memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  بازگشت SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
ایستا صحیح memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  صحیح iAmt,
  تهی **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  اگر( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }وگرنه{
    p->nMmap++;
    *pp = (تهی*)(p->aData + iOfst);
  }
  memdbLeave(p);
  بازگشت SQLITE_OK;
}

/* Release a memory-mapped page */
ایستا صحیح memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, تهی *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  بازگشت SQLITE_OK;
}

/*
** Open an mem file handle.
*/
ایستا صحیح memdbOpen(
  sqlite3_vfs *pVfs,
  ثابت نویسه *zName,
  sqlite3_file *pFd,
  صحیح flags,
  صحیح *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  صحیح szName;
  UNUSED_PARAMETER(pVfs);

  memset(pFile, 0, اندازه(*pFile));
  szName = sqlite3Strlen30(zName);
  اگر( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    صحیح i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    برای(i=0; i<memdb_g.nMemStore; i++){
      اگر( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        بشکن;
      }
    }
    اگر( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( اندازه(*p) + (i64)szName + 3 );
      اگر( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        بازگشت SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             اندازه(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      اگر( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        بازگشت SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, اندازه(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mxMemdbSize;
      p->zFName = (نویسه*)&p[1];
      memcpy(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      اگر( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        بازگشت SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }وگرنه{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }وگرنه{
    p = sqlite3Malloc( اندازه(*p) );
    اگر( p==0 ){
      بازگشت SQLITE_NOMEM;
    }
    memset(p, 0, اندازه(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mxMemdbSize;
  }
  pFile->pStore = p;
  اگر( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  بازگشت SQLITE_OK;
}

#اگر 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ایستا صحیح memdbDelete(sqlite3_vfs *pVfs, ثابت نویسه *zPath, صحیح dirSync){
  بازگشت SQLITE_IOERR_DELETE;
}
#endif

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
ایستا صحیح memdbAccess(
  sqlite3_vfs *pVfs, 
  ثابت نویسه *zPath, 
  صحیح flags, 
  صحیح *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  بازگشت SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ایستا صحیح memdbFullPathname(
  sqlite3_vfs *pVfs, 
  ثابت نویسه *zPath, 
  صحیح nOut, 
  نویسه *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  بازگشت SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ایستا تهی *memdbDlOpen(sqlite3_vfs *pVfs, ثابت نویسه *zPath){
  بازگشت ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ایستا تهی memdbDlError(sqlite3_vfs *pVfs, صحیح nByte, نویسه *z_Err_پیام){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_پیام);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ایستا تهی (*memdbDlSym(sqlite3_vfs *pVfs, تهی *p, ثابت نویسه *zSym))(تهی){
  بازگشت ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ایستا تهی memdbDlClose(sqlite3_vfs *pVfs, تهی *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ایستا صحیح memdbRandomness(sqlite3_vfs *pVfs, صحیح nByte, نویسه *zBufOut){
  بازگشت ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ایستا صحیح memdbSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  بازگشت ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#اگر 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ایستا صحیح memdbCurrentTime(sqlite3_vfs *pVfs, دوگانه *pTimeOut){
  بازگشت ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#endif

ایستا صحیح memdbGetLastError(sqlite3_vfs *pVfs, صحیح a, نویسه *b){
  بازگشت ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
ایستا صحیح memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  بازگشت ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
ایستا MemFile *memdbFromDbSchema(sqlite3 *db, ثابت نویسه *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  صحیح rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  اگر( rc ) بازگشت 0;
  اگر( p->base.pMethods!=&memdb_io_methods ) بازگشت 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  اگر( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  بازگشت p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
unsigned نویسه *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  ثابت نویسه *zSchema,      /* Which database within the connection */
  sqlite3_int64 *piSize,    /* Write size here, if not NULL */
  unsigned صحیح mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  صحیح iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  صحیح szPage = 0;
  sqlite3_stmt *pStmt = 0;
  unsigned نویسه *pOut = 0;
  نویسه *zSql;
  صحیح rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (تهی)SQLITE_MISUSE_BKPT;
    بازگشت 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  اگر( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  اگر( piSize ) *piSize = -1;
  اگر( iDb<0 ) برو serialize_out;
  اگر( p ){
    MemStore *pStore = p->pStore;
    أكد( pStore->pMutex==0 );
    اگر( piSize ) *piSize = pStore->sz;
    اگر( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->aData;
    }وگرنه{
      pOut = sqlite3_malloc64( pStore->sz );
      اگر( pOut ) memcpy(pOut, pStore->aData, pStore->sz);
    }
    برو serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  اگر( pBt==0 ) برو serialize_out;
  szPage = sqlite3BtreeGetPageSize(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  اگر( rc ) برو serialize_out;
  rc = sqlite3_step(pStmt);
  اگر( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    اگر( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      اگر( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    اگر( piSize ) *piSize = sz;
    اگر( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }وگرنه{
      pOut = sqlite3_malloc64( sz );
      اگر( pOut ){
        صحیح nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        صحیح pgno;
        برای(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          unsigned نویسه *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          اگر( rc==SQLITE_OK ){
            memcpy(pTo, sqlite3PagerGetData(pPage), szPage);
          }وگرنه اگر( pgno==PENDING_BYTE/szPage+1 ){
            memset(pTo, 0, szPage);
          }وگرنه{
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
  بازگشت pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
صحیح sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  ثابت نویسه *zSchema,    /* Which DB to reopen with the deserialization */
  unsigned نویسه *pData,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  unsigned mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  نویسه *zSql;
  sqlite3_stmt *pStmt = 0;
  صحیح rc;
  صحیح iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    بازگشت SQLITE_MISUSE_BKPT;
  }
  اگر( szDb<0 ) بازگشت SQLITE_MISUSE_BKPT;
  اگر( szBuf<0 ) بازگشت SQLITE_MISUSE_BKPT;
#endif

  sqlite3_mutex_enter(db->mutex);
  اگر( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  اگر( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    برو end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  اگر( zSql==0 ){
    rc = SQLITE_NOMEM;
  }وگرنه{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  اگر( rc ) برو end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  اگر( rc!=SQLITE_OK ){
    برو end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  اگر( p==0 ){
    rc = SQLITE_ERROR;
  }وگرنه{
    MemStore *pStore = p->pStore;
    pStore->aData = pData;
    pData = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    اگر( pStore->szMax<sqlite3GlobalConfig.mxMemdbSize ){
      pStore->szMax = sqlite3GlobalConfig.mxMemdbSize;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  اگر( pData && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(pData);
  }
  sqlite3_mutex_leave(db->mutex);
  بازگشت rc;
}

/*
** Return true if the VFS is the memvfs.
*/
صحیح sqlite3IsMemdb(ثابت sqlite3_vfs *pVfs){
  بازگشت pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
صحیح sqlite3MemdbInit(تهی){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  unsigned صحیح sz;
  اگر( NEVER(pLower==0) ) بازگشت SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.pAppData = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  اگر( sz<اندازه(MemFile) ) sz = اندازه(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  بازگشت sqlite3_vfs_register(&memdb_vfs, 0);
}
#endif /* SQLITE_OMIT_DESERIALIZE */
