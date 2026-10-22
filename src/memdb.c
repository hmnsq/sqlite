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
#ضمن "sqliteInt.h"
#ifndef SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
تعريفنوع بنية sqlite3_vfs MemVfs;
تعريفنوع بنية MemFile MemFile;
تعريفنوع بنية MemStore MemStore;

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
بنية MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  unsigned حرف *aData;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  صحيح nMmap;                      /* Number of memory mapped pages */
  unsigned mFlags;                /* Flags */
  صحيح nRdLock;                    /* Number of readers */
  صحيح nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  صحيح nRef;                       /* Number of users of this MemStore */
  حرف *zFName;                   /* The filename for shared stores */
};

/* An open file */
بنية MemFile {
  sqlite3_file base;              /* IO methods */
  MemStore *pStore;               /* The storage */
  صحيح eLock;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
ساكن بنية MemFS {
  صحيح nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
ساكن صحيح memdbClose(sqlite3_file*);
ساكن صحيح memdbRead(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح memdbWrite(sqlite3_file*,ثابت فراغ*,صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح memdbTruncate(sqlite3_file*, sqlite3_int64 size);
ساكن صحيح memdbSync(sqlite3_file*, صحيح flags);
ساكن صحيح memdbFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ساكن صحيح memdbLock(sqlite3_file*, صحيح);
ساكن صحيح memdbUnlock(sqlite3_file*, صحيح);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
ساكن صحيح memdbFileControl(sqlite3_file*, صحيح op, فراغ *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
ساكن صحيح memdbDeviceCharacteristics(sqlite3_file*);
ساكن صحيح memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, صحيح iAmt, فراغ **pp);
ساكن صحيح memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, فراغ *p);

/*
** Methods for MemVfs
*/
ساكن صحيح memdbOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحيح , صحيح *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
ساكن صحيح memdbAccess(sqlite3_vfs*, ثابت حرف *zName, صحيح flags, صحيح *);
ساكن صحيح memdbFullPathname(sqlite3_vfs*, ثابت حرف *zName, صحيح, حرف *zOut);
ساكن فراغ *memdbDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن فراغ memdbDlError(sqlite3_vfs*, صحيح nByte, حرف *zErrMsg);
ساكن فراغ (*memdbDlSym(sqlite3_vfs *pVfs, فراغ *p, ثابت حرف*zSym))(فراغ);
ساكن فراغ memdbDlClose(sqlite3_vfs*, فراغ*);
ساكن صحيح memdbRandomness(sqlite3_vfs*, صحيح nByte, حرف *zOut);
ساكن صحيح memdbSleep(sqlite3_vfs*, صحيح microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
ساكن صحيح memdbGetLastError(sqlite3_vfs*, صحيح, حرف *);
ساكن صحيح memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

ساكن sqlite3_vfs memdb_vfs = {
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

ساكن ثابت sqlite3_io_methods memdb_io_methods = {
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
#𐤀𐤌 defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
ساكن فراغ memdbEnter(MemStore *p){
  UNUSED_PARAMETER(p);
}
ساكن فراغ memdbLeave(MemStore *p){
  UNUSED_PARAMETER(p);
}
#وإلا
ساكن فراغ memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
ساكن فراغ memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#endif



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
ساكن صحيح memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  𐤀𐤌( p->zFName ){
    صحيح i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    لكل(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      𐤀𐤌( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        𐤀𐤌( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          𐤀𐤌( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        اكسر;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }وإلا{
    memdbEnter(p);
  }
  p->nRef--;
  𐤀𐤌( p->nRef<=0 ){
    𐤀𐤌( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->aData);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }وإلا{
    memdbLeave(p);
  }
  أرجع SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
ساكن صحيح memdbRead(
  sqlite3_file *pFile, 
  فراغ *zBuf, 
  صحيح iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  𐤀𐤌( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    𐤀𐤌( iOfst<p->sz ) memcpy(zBuf, p->aData+iOfst, p->sz - iOfst);
    memdbLeave(p);
    أرجع SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, p->aData+iOfst, iAmt);
  memdbLeave(p);
  أرجع SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
ساكن صحيح memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  unsigned حرف *pNew;
  𐤀𐤌( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    أرجع SQLITE_FULL;
  }
  𐤀𐤌( newSz>p->szMax ){
    أرجع SQLITE_FULL;
  }
  newSz *= 2;
  𐤀𐤌( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->aData, newSz);
  𐤀𐤌( pNew==0 ) أرجع SQLITE_IOERR_NOMEM;
  p->aData = pNew;
  p->szAlloc = newSz;
  أرجع SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
ساكن صحيح memdbWrite(
  sqlite3_file *pFile,
  ثابت فراغ *z,
  صحيح iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  𐤀𐤌( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    أرجع SQLITE_IOERR_WRITE;
  }
  𐤀𐤌( iOfst+iAmt>p->sz ){
    صحيح rc;
    𐤀𐤌( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      أرجع rc;
    }
    𐤀𐤌( iOfst>p->sz ) memset(p->aData+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  memcpy(p->aData+iOfst, z, iAmt);
  memdbLeave(p);
  أرجع SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
ساكن صحيح memdbTruncate(sqlite3_file *pFile, sqlite_int64 size){
  MemStore *p = ((MemFile*)pFile)->pStore;
  صحيح rc = SQLITE_OK;
  memdbEnter(p);
  𐤀𐤌( size>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }وإلا{
    p->sz = size; 
  }
  memdbLeave(p);
  أرجع rc;
}

/*
** Sync an memdb-file.
*/
ساكن صحيح memdbSync(sqlite3_file *pFile, صحيح flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  أرجع SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
ساكن صحيح memdbFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *pSize = p->sz;
  memdbLeave(p);
  أرجع SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
ساكن صحيح memdbLock(sqlite3_file *pFile, صحيح eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  صحيح rc = SQLITE_OK;
  𐤀𐤌( eLock<=pThis->eLock ) أرجع SQLITE_OK;
  memdbEnter(p);

  أكد( p->nWrLock==0 || p->nWrLock==1 );
  أكد( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  أكد( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  𐤀𐤌( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }وإلا{
    بدل( eLock ){
      حالة SQLITE_LOCK_SHARED: {
        أكد( pThis->eLock==SQLITE_LOCK_NONE );
        𐤀𐤌( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }وإلا{
          p->nRdLock++;
        }
        اكسر;
      };
  
      حالة SQLITE_LOCK_RESERVED:
      حالة SQLITE_LOCK_PENDING: {
        أكد( pThis->eLock>=SQLITE_LOCK_SHARED );
        𐤀𐤌( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          𐤀𐤌( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }وإلا{
            p->nWrLock = 1;
          }
        }
        اكسر;
      }
  
      افتراضي: {
        أكد(  eLock==SQLITE_LOCK_EXCLUSIVE );
        أكد( pThis->eLock>=SQLITE_LOCK_SHARED );
        𐤀𐤌( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }وإلا 𐤀𐤌( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        اكسر;
      }
    }
  }
  𐤀𐤌( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  أرجع rc;
}

/*
** Unlock an memdb-file.
*/
ساكن صحيح memdbUnlock(sqlite3_file *pFile, صحيح eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  𐤀𐤌( eLock>=pThis->eLock ) أرجع SQLITE_OK;
  memdbEnter(p);

  أكد( eLock==SQLITE_LOCK_SHARED || eLock==SQLITE_LOCK_NONE );
  𐤀𐤌( eLock==SQLITE_LOCK_SHARED ){
    𐤀𐤌( ALWAYS(pThis->eLock>SQLITE_LOCK_SHARED) ){
      p->nWrLock--;
    }
  }وإلا{
    𐤀𐤌( pThis->eLock>SQLITE_LOCK_SHARED ){
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  أرجع SQLITE_OK;
}

#𐤀𐤌 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
ساكن صحيح memdbCheckReservedLock(sqlite3_file *pFile, صحيح *pResOut){
  *pResOut = 0;
  أرجع SQLITE_OK;
}
#endif


/*
** File control method. For custom operations on an memdb-file.
*/
ساكن صحيح memdbFileControl(sqlite3_file *pFile, صحيح op, فراغ *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  صحيح rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  𐤀𐤌( op==SQLITE_FCNTL_VFSNAME ){
    *(حرف**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->aData, p->sz);
    rc = SQLITE_OK;
  }
  𐤀𐤌( op==SQLITE_FCNTL_SIZE_LIMIT ){
    sqlite3_int64 iLimit = *(sqlite3_int64*)pArg;
    𐤀𐤌( iLimit<p->sz ){
      𐤀𐤌( iLimit<0 ){
        iLimit = p->szMax;
      }وإلا{
        iLimit = p->sz;
      }
    }
    p->szMax = iLimit;
    *(sqlite3_int64*)pArg = iLimit;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  أرجع rc;
}

#𐤀𐤌 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
ساكن صحيح memdbSectorSize(sqlite3_file *pFile){
  أرجع 1024;
}
#endif

/*
** Return the device characteristic flags supported by an memdb-file.
*/
ساكن صحيح memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  أرجع SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
ساكن صحيح memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  صحيح iAmt,
  فراغ **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  𐤀𐤌( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }وإلا{
    p->nMmap++;
    *pp = (فراغ*)(p->aData + iOfst);
  }
  memdbLeave(p);
  أرجع SQLITE_OK;
}

/* Release a memory-mapped page */
ساكن صحيح memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, فراغ *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  أرجع SQLITE_OK;
}

/*
** Open an mem file handle.
*/
ساكن صحيح memdbOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFd,
  صحيح flags,
  صحيح *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  صحيح szName;
  UNUSED_PARAMETER(pVfs);

  memset(pFile, 0, حجم(*pFile));
  szName = sqlite3Strlen30(zName);
  𐤀𐤌( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    صحيح i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    لكل(i=0; i<memdb_g.nMemStore; i++){
      𐤀𐤌( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        اكسر;
      }
    }
    𐤀𐤌( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( حجم(*p) + (i64)szName + 3 );
      𐤀𐤌( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        أرجع SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             حجم(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      𐤀𐤌( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        أرجع SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, حجم(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mxMemdbSize;
      p->zFName = (حرف*)&p[1];
      memcpy(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      𐤀𐤌( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        أرجع SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }وإلا{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }وإلا{
    p = sqlite3Malloc( حجم(*p) );
    𐤀𐤌( p==0 ){
      أرجع SQLITE_NOMEM;
    }
    memset(p, 0, حجم(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mxMemdbSize;
  }
  pFile->pStore = p;
  𐤀𐤌( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  أرجع SQLITE_OK;
}

#𐤀𐤌 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساكن صحيح memdbDelete(sqlite3_vfs *pVfs, ثابت حرف *zPath, صحيح dirSync){
  أرجع SQLITE_IOERR_DELETE;
}
#endif

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
ساكن صحيح memdbAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *zPath, 
  صحيح flags, 
  صحيح *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  أرجع SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح memdbFullPathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *zPath, 
  صحيح nOut, 
  حرف *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  أرجع SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *memdbDlOpen(sqlite3_vfs *pVfs, ثابت حرف *zPath){
  أرجع ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساكن فراغ memdbDlError(sqlite3_vfs *pVfs, صحيح nByte, حرف *zErrMsg){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ساكن فراغ (*memdbDlSym(sqlite3_vfs *pVfs, فراغ *p, ثابت حرف *zSym))(فراغ){
  أرجع ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ساكن فراغ memdbDlClose(sqlite3_vfs *pVfs, فراغ *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساكن صحيح memdbRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *zBufOut){
  أرجع ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساكن صحيح memdbSleep(sqlite3_vfs *pVfs, صحيح nMicro){
  أرجع ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#𐤀𐤌 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساكن صحيح memdbCurrentTime(sqlite3_vfs *pVfs, مزدوج *pTimeOut){
  أرجع ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#endif

ساكن صحيح memdbGetLastError(sqlite3_vfs *pVfs, صحيح a, حرف *b){
  أرجع ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
ساكن صحيح memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  أرجع ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
ساكن MemFile *memdbFromDbSchema(sqlite3 *db, ثابت حرف *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  صحيح rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  𐤀𐤌( rc ) أرجع 0;
  𐤀𐤌( p->base.pMethods!=&memdb_io_methods ) أرجع 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  𐤀𐤌( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  أرجع p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
unsigned حرف *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  ثابت حرف *zSchema,      /* Which database within the connection */
  sqlite3_int64 *piSize,    /* Write size here, if not NULL */
  unsigned صحيح mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  صحيح iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  صحيح szPage = 0;
  sqlite3_stmt *pStmt = 0;
  unsigned حرف *pOut = 0;
  حرف *zSql;
  صحيح rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  𐤀𐤌( !sqlite3SafetyCheckOk(db) ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  𐤀𐤌( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  𐤀𐤌( piSize ) *piSize = -1;
  𐤀𐤌( iDb<0 ) اذهب serialize_out;
  𐤀𐤌( p ){
    MemStore *pStore = p->pStore;
    أكد( pStore->pMutex==0 );
    𐤀𐤌( piSize ) *piSize = pStore->sz;
    𐤀𐤌( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->aData;
    }وإلا{
      pOut = sqlite3_malloc64( pStore->sz );
      𐤀𐤌( pOut ) memcpy(pOut, pStore->aData, pStore->sz);
    }
    اذهب serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  𐤀𐤌( pBt==0 ) اذهب serialize_out;
  szPage = sqlite3BtreeGetPageSize(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  𐤀𐤌( rc ) اذهب serialize_out;
  rc = sqlite3_step(pStmt);
  𐤀𐤌( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    𐤀𐤌( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      𐤀𐤌( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    𐤀𐤌( piSize ) *piSize = sz;
    𐤀𐤌( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }وإلا{
      pOut = sqlite3_malloc64( sz );
      𐤀𐤌( pOut ){
        صحيح nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        صحيح pgno;
        لكل(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          unsigned حرف *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          𐤀𐤌( rc==SQLITE_OK ){
            memcpy(pTo, sqlite3PagerGetData(pPage), szPage);
          }وإلا 𐤀𐤌( pgno==PENDING_BYTE/szPage+1 ){
            memset(pTo, 0, szPage);
          }وإلا{
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
  أرجع pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
صحيح sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  ثابت حرف *zSchema,    /* Which DB to reopen with the deserialization */
  unsigned حرف *pData,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  unsigned mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  حرف *zSql;
  sqlite3_stmt *pStmt = 0;
  صحيح rc;
  صحيح iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  𐤀𐤌( !sqlite3SafetyCheckOk(db) ){
    أرجع SQLITE_MISUSE_BKPT;
  }
  𐤀𐤌( szDb<0 ) أرجع SQLITE_MISUSE_BKPT;
  𐤀𐤌( szBuf<0 ) أرجع SQLITE_MISUSE_BKPT;
#endif

  sqlite3_mutex_enter(db->mutex);
  𐤀𐤌( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  𐤀𐤌( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    اذهب end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  𐤀𐤌( zSql==0 ){
    rc = SQLITE_NOMEM;
  }وإلا{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  𐤀𐤌( rc ) اذهب end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  𐤀𐤌( rc!=SQLITE_OK ){
    اذهب end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  𐤀𐤌( p==0 ){
    rc = SQLITE_ERROR;
  }وإلا{
    MemStore *pStore = p->pStore;
    pStore->aData = pData;
    pData = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    𐤀𐤌( pStore->szMax<sqlite3GlobalConfig.mxMemdbSize ){
      pStore->szMax = sqlite3GlobalConfig.mxMemdbSize;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  𐤀𐤌( pData && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(pData);
  }
  sqlite3_mutex_leave(db->mutex);
  أرجع rc;
}

/*
** Return true if the VFS is the memvfs.
*/
صحيح sqlite3IsMemdb(ثابت sqlite3_vfs *pVfs){
  أرجع pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
صحيح sqlite3MemdbInit(فراغ){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  unsigned صحيح sz;
  𐤀𐤌( NEVER(pLower==0) ) أرجع SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.pAppData = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  𐤀𐤌( sz<حجم(MemFile) ) sz = حجم(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  أرجع sqlite3_vfs_register(&memdb_vfs, 0);
}
#endif /* SQLITE_OMIT_DESERIALIZE */
