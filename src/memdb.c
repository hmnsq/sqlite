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
#অন্তর্ভাব "sqliteInt.h"
#ifndef SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
প্রকারনির্দেশ সংরচনা sqlite3_vfs MemVfs;
প্রকারনির্দেশ সংরচনা MemFile MemFile;
প্রকারনির্দেশ সংরচনা MemStore MemStore;

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
সংরচনা MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  unsigned বর্ণ *aData;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  পূর্ণাঙ্ক nMmap;                      /* Number of memory mapped pages */
  unsigned mFlags;                /* Flags */
  পূর্ণাঙ্ক nRdLock;                    /* Number of readers */
  পূর্ণাঙ্ক nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  পূর্ণাঙ্ক nRef;                       /* Number of users of this MemStore */
  বর্ণ *zFName;                   /* The filename for shared stores */
};

/* An open file */
সংরচনা MemFile {
  sqlite3_file base;              /* IO methods */
  MemStore *pStore;               /* The storage */
  পূর্ণাঙ্ক eLock;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
স্থির সংরচনা MemFS {
  পূর্ণাঙ্ক nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
স্থির পূর্ণাঙ্ক memdbClose(sqlite3_file*);
স্থির পূর্ণাঙ্ক memdbRead(sqlite3_file*, শূন্য*, পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক memdbWrite(sqlite3_file*,ধ্রুব শূন্য*,পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক memdbTruncate(sqlite3_file*, sqlite3_int64 size);
স্থির পূর্ণাঙ্ক memdbSync(sqlite3_file*, পূর্ণাঙ্ক flags);
স্থির পূর্ণাঙ্ক memdbFileSize(sqlite3_file*, sqlite3_int64 *pSize);
স্থির পূর্ণাঙ্ক memdbLock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক memdbUnlock(sqlite3_file*, পূর্ণাঙ্ক);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
স্থির পূর্ণাঙ্ক memdbFileControl(sqlite3_file*, পূর্ণাঙ্ক op, শূন্য *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
স্থির পূর্ণাঙ্ক memdbDeviceCharacteristics(sqlite3_file*);
স্থির পূর্ণাঙ্ক memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, পূর্ণাঙ্ক iAmt, শূন্য **pp);
স্থির পূর্ণাঙ্ক memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, শূন্য *p);

/*
** Methods for MemVfs
*/
স্থির পূর্ণাঙ্ক memdbOpen(sqlite3_vfs*, ধ্রুব বর্ণ *, sqlite3_file*, পূর্ণাঙ্ক , পূর্ণাঙ্ক *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
স্থির পূর্ণাঙ্ক memdbAccess(sqlite3_vfs*, ধ্রুব বর্ণ *zName, পূর্ণাঙ্ক flags, পূর্ণাঙ্ক *);
স্থির পূর্ণাঙ্ক memdbFullPathname(sqlite3_vfs*, ধ্রুব বর্ণ *zName, পূর্ণাঙ্ক, বর্ণ *zOut);
স্থির শূন্য *memdbDlOpen(sqlite3_vfs*, ধ্রুব বর্ণ *zFilename);
স্থির শূন্য memdbDlError(sqlite3_vfs*, পূর্ণাঙ্ক nByte, বর্ণ *z_Err_বার্তা);
স্থির শূন্য (*memdbDlSym(sqlite3_vfs *pVfs, শূন্য *p, ধ্রুব বর্ণ*zSym))(শূন্য);
স্থির শূন্য memdbDlClose(sqlite3_vfs*, শূন্য*);
স্থির পূর্ণাঙ্ক memdbRandomness(sqlite3_vfs*, পূর্ণাঙ্ক nByte, বর্ণ *zOut);
স্থির পূর্ণাঙ্ক memdbSleep(sqlite3_vfs*, পূর্ণাঙ্ক microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
স্থির পূর্ণাঙ্ক memdbGetLastError(sqlite3_vfs*, পূর্ণাঙ্ক, বর্ণ *);
স্থির পূর্ণাঙ্ক memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

স্থির sqlite3_vfs memdb_vfs = {
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

স্থির ধ্রুব sqlite3_io_methods memdb_io_methods = {
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
#যদি defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
স্থির শূন্য memdbEnter(MemStore *p){
  UNUSED_PARAMETER(p);
}
স্থির শূন্য memdbLeave(MemStore *p){
  UNUSED_PARAMETER(p);
}
#অন্যথা
স্থির শূন্য memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
স্থির শূন্য memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#endif



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
স্থির পূর্ণাঙ্ক memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  যদি( p->zFName ){
    পূর্ণাঙ্ক i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    প্রতি(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      যদি( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        যদি( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          যদি( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        ভঙ্গ;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }অন্যথা{
    memdbEnter(p);
  }
  p->nRef--;
  যদি( p->nRef<=0 ){
    যদি( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->aData);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }অন্যথা{
    memdbLeave(p);
  }
  প্রত্যাগম SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
স্থির পূর্ণাঙ্ক memdbRead(
  sqlite3_file *pFile, 
  শূন্য *zBuf, 
  পূর্ণাঙ্ক iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  যদি( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    যদি( iOfst<p->sz ) memcpy(zBuf, p->aData+iOfst, p->sz - iOfst);
    memdbLeave(p);
    প্রত্যাগম SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, p->aData+iOfst, iAmt);
  memdbLeave(p);
  প্রত্যাগম SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
স্থির পূর্ণাঙ্ক memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  unsigned বর্ণ *pNew;
  যদি( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    প্রত্যাগম SQLITE_FULL;
  }
  যদি( newSz>p->szMax ){
    প্রত্যাগম SQLITE_FULL;
  }
  newSz *= 2;
  যদি( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->aData, newSz);
  যদি( pNew==0 ) প্রত্যাগম SQLITE_IOERR_NOMEM;
  p->aData = pNew;
  p->szAlloc = newSz;
  প্রত্যাগম SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
স্থির পূর্ণাঙ্ক memdbWrite(
  sqlite3_file *pFile,
  ধ্রুব শূন্য *z,
  পূর্ণাঙ্ক iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  যদি( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    প্রত্যাগম SQLITE_IOERR_WRITE;
  }
  যদি( iOfst+iAmt>p->sz ){
    পূর্ণাঙ্ক rc;
    যদি( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      প্রত্যাগম rc;
    }
    যদি( iOfst>p->sz ) memset(p->aData+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  memcpy(p->aData+iOfst, z, iAmt);
  memdbLeave(p);
  প্রত্যাগম SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
স্থির পূর্ণাঙ্ক memdbTruncate(sqlite3_file *pFile, sqlite_int64 size){
  MemStore *p = ((MemFile*)pFile)->pStore;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  memdbEnter(p);
  যদি( size>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }অন্যথা{
    p->sz = size; 
  }
  memdbLeave(p);
  প্রত্যাগম rc;
}

/*
** Sync an memdb-file.
*/
স্থির পূর্ণাঙ্ক memdbSync(sqlite3_file *pFile, পূর্ণাঙ্ক flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  প্রত্যাগম SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
স্থির পূর্ণাঙ্ক memdbFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *pSize = p->sz;
  memdbLeave(p);
  প্রত্যাগম SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
স্থির পূর্ণাঙ্ক memdbLock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  যদি( eLock<=pThis->eLock ) প্রত্যাগম SQLITE_OK;
  memdbEnter(p);

  প্রতিজ্ঞা( p->nWrLock==0 || p->nWrLock==1 );
  প্রতিজ্ঞা( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  প্রতিজ্ঞা( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  যদি( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }অন্যথা{
    বিকল্প( eLock ){
      পক্ষ SQLITE_LOCK_SHARED: {
        প্রতিজ্ঞা( pThis->eLock==SQLITE_LOCK_NONE );
        যদি( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }অন্যথা{
          p->nRdLock++;
        }
        ভঙ্গ;
      };
  
      পক্ষ SQLITE_LOCK_RESERVED:
      পক্ষ SQLITE_LOCK_PENDING: {
        প্রতিজ্ঞা( pThis->eLock>=SQLITE_LOCK_SHARED );
        যদি( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          যদি( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }অন্যথা{
            p->nWrLock = 1;
          }
        }
        ভঙ্গ;
      }
  
      স্বভাব: {
        প্রতিজ্ঞা(  eLock==SQLITE_LOCK_EXCLUSIVE );
        প্রতিজ্ঞা( pThis->eLock>=SQLITE_LOCK_SHARED );
        যদি( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }অন্যথা যদি( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        ভঙ্গ;
      }
    }
  }
  যদি( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  প্রত্যাগম rc;
}

/*
** Unlock an memdb-file.
*/
স্থির পূর্ণাঙ্ক memdbUnlock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  যদি( eLock>=pThis->eLock ) প্রত্যাগম SQLITE_OK;
  memdbEnter(p);

  প্রতিজ্ঞা( eLock==SQLITE_LOCK_SHARED || eLock==SQLITE_LOCK_NONE );
  যদি( eLock==SQLITE_LOCK_SHARED ){
    যদি( ALWAYS(pThis->eLock>SQLITE_LOCK_SHARED) ){
      p->nWrLock--;
    }
  }অন্যথা{
    যদি( pThis->eLock>SQLITE_LOCK_SHARED ){
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  প্রত্যাগম SQLITE_OK;
}

#যদি 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
স্থির পূর্ণাঙ্ক memdbCheckReservedLock(sqlite3_file *pFile, পূর্ণাঙ্ক *pResOut){
  *pResOut = 0;
  প্রত্যাগম SQLITE_OK;
}
#endif


/*
** File control method. For custom operations on an memdb-file.
*/
স্থির পূর্ণাঙ্ক memdbFileControl(sqlite3_file *pFile, পূর্ণাঙ্ক op, শূন্য *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  পূর্ণাঙ্ক rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  যদি( op==SQLITE_FCNTL_VFSNAME ){
    *(বর্ণ**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->aData, p->sz);
    rc = SQLITE_OK;
  }
  যদি( op==SQLITE_FCNTL_SIZE_LIMIT ){
    sqlite3_int64 iLimit = *(sqlite3_int64*)pArg;
    যদি( iLimit<p->sz ){
      যদি( iLimit<0 ){
        iLimit = p->szMax;
      }অন্যথা{
        iLimit = p->sz;
      }
    }
    p->szMax = iLimit;
    *(sqlite3_int64*)pArg = iLimit;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  প্রত্যাগম rc;
}

#যদি 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
স্থির পূর্ণাঙ্ক memdbSectorSize(sqlite3_file *pFile){
  প্রত্যাগম 1024;
}
#endif

/*
** Return the device characteristic flags supported by an memdb-file.
*/
স্থির পূর্ণাঙ্ক memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  প্রত্যাগম SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
স্থির পূর্ণাঙ্ক memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  পূর্ণাঙ্ক iAmt,
  শূন্য **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  যদি( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }অন্যথা{
    p->nMmap++;
    *pp = (শূন্য*)(p->aData + iOfst);
  }
  memdbLeave(p);
  প্রত্যাগম SQLITE_OK;
}

/* Release a memory-mapped page */
স্থির পূর্ণাঙ্ক memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, শূন্য *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  প্রত্যাগম SQLITE_OK;
}

/*
** Open an mem file handle.
*/
স্থির পূর্ণাঙ্ক memdbOpen(
  sqlite3_vfs *pVfs,
  ধ্রুব বর্ণ *zName,
  sqlite3_file *pFd,
  পূর্ণাঙ্ক flags,
  পূর্ণাঙ্ক *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  পূর্ণাঙ্ক szName;
  UNUSED_PARAMETER(pVfs);

  memset(pFile, 0, পরিমাণ(*pFile));
  szName = sqlite3Strlen30(zName);
  যদি( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    পূর্ণাঙ্ক i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    প্রতি(i=0; i<memdb_g.nMemStore; i++){
      যদি( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        ভঙ্গ;
      }
    }
    যদি( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( পরিমাণ(*p) + (i64)szName + 3 );
      যদি( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        প্রত্যাগম SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             পরিমাণ(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      যদি( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        প্রত্যাগম SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, পরিমাণ(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mxMemdbSize;
      p->zFName = (বর্ণ*)&p[1];
      memcpy(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      যদি( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        প্রত্যাগম SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }অন্যথা{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }অন্যথা{
    p = sqlite3Malloc( পরিমাণ(*p) );
    যদি( p==0 ){
      প্রত্যাগম SQLITE_NOMEM;
    }
    memset(p, 0, পরিমাণ(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mxMemdbSize;
  }
  pFile->pStore = p;
  যদি( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  প্রত্যাগম SQLITE_OK;
}

#যদি 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
স্থির পূর্ণাঙ্ক memdbDelete(sqlite3_vfs *pVfs, ধ্রুব বর্ণ *zPath, পূর্ণাঙ্ক dirSync){
  প্রত্যাগম SQLITE_IOERR_DELETE;
}
#endif

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
স্থির পূর্ণাঙ্ক memdbAccess(
  sqlite3_vfs *pVfs, 
  ধ্রুব বর্ণ *zPath, 
  পূর্ণাঙ্ক flags, 
  পূর্ণাঙ্ক *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  প্রত্যাগম SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
স্থির পূর্ণাঙ্ক memdbFullPathname(
  sqlite3_vfs *pVfs, 
  ধ্রুব বর্ণ *zPath, 
  পূর্ণাঙ্ক nOut, 
  বর্ণ *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  প্রত্যাগম SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
স্থির শূন্য *memdbDlOpen(sqlite3_vfs *pVfs, ধ্রুব বর্ণ *zPath){
  প্রত্যাগম ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
স্থির শূন্য memdbDlError(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, বর্ণ *z_Err_বার্তা){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_বার্তা);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
স্থির শূন্য (*memdbDlSym(sqlite3_vfs *pVfs, শূন্য *p, ধ্রুব বর্ণ *zSym))(শূন্য){
  প্রত্যাগম ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
স্থির শূন্য memdbDlClose(sqlite3_vfs *pVfs, শূন্য *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
স্থির পূর্ণাঙ্ক memdbRandomness(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, বর্ণ *zBufOut){
  প্রত্যাগম ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
স্থির পূর্ণাঙ্ক memdbSleep(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nMicro){
  প্রত্যাগম ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#যদি 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
স্থির পূর্ণাঙ্ক memdbCurrentTime(sqlite3_vfs *pVfs, দ্বিগুণ *pTimeOut){
  প্রত্যাগম ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#endif

স্থির পূর্ণাঙ্ক memdbGetLastError(sqlite3_vfs *pVfs, পূর্ণাঙ্ক a, বর্ণ *b){
  প্রত্যাগম ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
স্থির পূর্ণাঙ্ক memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  প্রত্যাগম ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
স্থির MemFile *memdbFromDbSchema(sqlite3 *db, ধ্রুব বর্ণ *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  পূর্ণাঙ্ক rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  যদি( rc ) প্রত্যাগম 0;
  যদি( p->base.pMethods!=&memdb_io_methods ) প্রত্যাগম 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  যদি( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  প্রত্যাগম p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
unsigned বর্ণ *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  ধ্রুব বর্ণ *zSchema,      /* Which database within the connection */
  sqlite3_int64 *piSize,    /* Write size here, if not NULL */
  unsigned পূর্ণাঙ্ক mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  পূর্ণাঙ্ক iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  পূর্ণাঙ্ক szPage = 0;
  sqlite3_stmt *pStmt = 0;
  unsigned বর্ণ *pOut = 0;
  বর্ণ *zSql;
  পূর্ণাঙ্ক rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) ){
    (শূন্য)SQLITE_MISUSE_BKPT;
    প্রত্যাগম 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  যদি( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  যদি( piSize ) *piSize = -1;
  যদি( iDb<0 ) গচ্ছ serialize_out;
  যদি( p ){
    MemStore *pStore = p->pStore;
    প্রতিজ্ঞা( pStore->pMutex==0 );
    যদি( piSize ) *piSize = pStore->sz;
    যদি( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->aData;
    }অন্যথা{
      pOut = sqlite3_malloc64( pStore->sz );
      যদি( pOut ) memcpy(pOut, pStore->aData, pStore->sz);
    }
    গচ্ছ serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  যদি( pBt==0 ) গচ্ছ serialize_out;
  szPage = sqlite3BtreeGetPageSize(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  যদি( rc ) গচ্ছ serialize_out;
  rc = sqlite3_step(pStmt);
  যদি( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    যদি( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      যদি( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    যদি( piSize ) *piSize = sz;
    যদি( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }অন্যথা{
      pOut = sqlite3_malloc64( sz );
      যদি( pOut ){
        পূর্ণাঙ্ক nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        পূর্ণাঙ্ক pgno;
        প্রতি(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          unsigned বর্ণ *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          যদি( rc==SQLITE_OK ){
            memcpy(pTo, sqlite3PagerGetData(pPage), szPage);
          }অন্যথা যদি( pgno==PENDING_BYTE/szPage+1 ){
            memset(pTo, 0, szPage);
          }অন্যথা{
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
  প্রত্যাগম pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
পূর্ণাঙ্ক sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  ধ্রুব বর্ণ *zSchema,    /* Which DB to reopen with the deserialization */
  unsigned বর্ণ *pData,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  unsigned mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  বর্ণ *zSql;
  sqlite3_stmt *pStmt = 0;
  পূর্ণাঙ্ক rc;
  পূর্ণাঙ্ক iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) ){
    প্রত্যাগম SQLITE_MISUSE_BKPT;
  }
  যদি( szDb<0 ) প্রত্যাগম SQLITE_MISUSE_BKPT;
  যদি( szBuf<0 ) প্রত্যাগম SQLITE_MISUSE_BKPT;
#endif

  sqlite3_mutex_enter(db->mutex);
  যদি( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  যদি( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    গচ্ছ end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  যদি( zSql==0 ){
    rc = SQLITE_NOMEM;
  }অন্যথা{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  যদি( rc ) গচ্ছ end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  যদি( rc!=SQLITE_OK ){
    গচ্ছ end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  যদি( p==0 ){
    rc = SQLITE_ERROR;
  }অন্যথা{
    MemStore *pStore = p->pStore;
    pStore->aData = pData;
    pData = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    যদি( pStore->szMax<sqlite3GlobalConfig.mxMemdbSize ){
      pStore->szMax = sqlite3GlobalConfig.mxMemdbSize;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  যদি( pData && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(pData);
  }
  sqlite3_mutex_leave(db->mutex);
  প্রত্যাগম rc;
}

/*
** Return true if the VFS is the memvfs.
*/
পূর্ণাঙ্ক sqlite3IsMemdb(ধ্রুব sqlite3_vfs *pVfs){
  প্রত্যাগম pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
পূর্ণাঙ্ক sqlite3MemdbInit(শূন্য){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  unsigned পূর্ণাঙ্ক sz;
  যদি( NEVER(pLower==0) ) প্রত্যাগম SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.pAppData = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  যদি( sz<পরিমাণ(MemFile) ) sz = পরিমাণ(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  প্রত্যাগম sqlite3_vfs_register(&memdb_vfs, 0);
}
#endif /* SQLITE_OMIT_DESERIALIZE */
