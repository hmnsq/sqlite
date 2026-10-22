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
#અન્તર્ભાવ "sqliteInt.h"
#ifndef SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
પ્રકારવ્યાખ્યા સંરચના sqlite3_vfs MemVfs;
પ્રકારવ્યાખ્યા સંરચના MemFile MemFile;
પ્રકારવ્યાખ્યા સંરચના MemStore MemStore;

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
સંરચના MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  unsigned અક્ષર *aData;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  પૂર્ણાંક nMmap;                      /* Number of memory mapped pages */
  unsigned mFlags;                /* Flags */
  પૂર્ણાંક nRdLock;                    /* Number of readers */
  પૂર્ણાંક nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  પૂર્ણાંક nRef;                       /* Number of users of this MemStore */
  અક્ષર *zFName;                   /* The filename for shared stores */
};

/* An open file */
સંરચના MemFile {
  sqlite3_file base;              /* IO methods */
  MemStore *pStore;               /* The storage */
  પૂર્ણાંક eLock;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
સ્થિર સંરચના MemFS {
  પૂર્ણાંક nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
સ્થિર પૂર્ણાંક memdbClose(sqlite3_file*);
સ્થિર પૂર્ણાંક memdbRead(sqlite3_file*, શૂન્ય*, પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક memdbWrite(sqlite3_file*,અચલ શૂન્ય*,પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક memdbTruncate(sqlite3_file*, sqlite3_int64 size);
સ્થિર પૂર્ણાંક memdbSync(sqlite3_file*, પૂર્ણાંક flags);
સ્થિર પૂર્ણાંક memdbFileSize(sqlite3_file*, sqlite3_int64 *pSize);
સ્થિર પૂર્ણાંક memdbLock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક memdbUnlock(sqlite3_file*, પૂર્ણાંક);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
સ્થિર પૂર્ણાંક memdbFileControl(sqlite3_file*, પૂર્ણાંક op, શૂન્ય *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
સ્થિર પૂર્ણાંક memdbDeviceCharacteristics(sqlite3_file*);
સ્થિર પૂર્ણાંક memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, પૂર્ણાંક iAmt, શૂન્ય **pp);
સ્થિર પૂર્ણાંક memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, શૂન્ય *p);

/*
** Methods for MemVfs
*/
સ્થિર પૂર્ણાંક memdbOpen(sqlite3_vfs*, અચલ અક્ષર *, sqlite3_file*, પૂર્ણાંક , પૂર્ણાંક *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
સ્થિર પૂર્ણાંક memdbAccess(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક flags, પૂર્ણાંક *);
સ્થિર પૂર્ણાંક memdbFullPathname(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક, અક્ષર *zOut);
સ્થિર શૂન્ય *memdbDlOpen(sqlite3_vfs*, અચલ અક્ષર *zFilename);
સ્થિર શૂન્ય memdbDlError(sqlite3_vfs*, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ);
સ્થિર શૂન્ય (*memdbDlSym(sqlite3_vfs *pVfs, શૂન્ય *p, અચલ અક્ષર*zSym))(શૂન્ય);
સ્થિર શૂન્ય memdbDlClose(sqlite3_vfs*, શૂન્ય*);
સ્થિર પૂર્ણાંક memdbRandomness(sqlite3_vfs*, પૂર્ણાંક nByte, અક્ષર *zOut);
સ્થિર પૂર્ણાંક memdbSleep(sqlite3_vfs*, પૂર્ણાંક microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
સ્થિર પૂર્ણાંક memdbGetLastError(sqlite3_vfs*, પૂર્ણાંક, અક્ષર *);
સ્થિર પૂર્ણાંક memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

સ્થિર sqlite3_vfs memdb_vfs = {
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

સ્થિર અચલ sqlite3_io_methods memdb_io_methods = {
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
#જો defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
સ્થિર શૂન્ય memdbEnter(MemStore *p){
  UNUSED_PARAMETER(p);
}
સ્થિર શૂન્ય memdbLeave(MemStore *p){
  UNUSED_PARAMETER(p);
}
#નહીંતર
સ્થિર શૂન્ય memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
સ્થિર શૂન્ય memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#endif



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
સ્થિર પૂર્ણાંક memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  જો( p->zFName ){
    પૂર્ણાંક i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    માટે(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      જો( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        જો( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          જો( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        તોડો;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }નહીંતર{
    memdbEnter(p);
  }
  p->nRef--;
  જો( p->nRef<=0 ){
    જો( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->aData);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }નહીંતર{
    memdbLeave(p);
  }
  પાછા SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
સ્થિર પૂર્ણાંક memdbRead(
  sqlite3_file *pFile, 
  શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  જો( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    જો( iOfst<p->sz ) memcpy(zBuf, p->aData+iOfst, p->sz - iOfst);
    memdbLeave(p);
    પાછા SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, p->aData+iOfst, iAmt);
  memdbLeave(p);
  પાછા SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
સ્થિર પૂર્ણાંક memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  unsigned અક્ષર *pNew;
  જો( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    પાછા SQLITE_FULL;
  }
  જો( newSz>p->szMax ){
    પાછા SQLITE_FULL;
  }
  newSz *= 2;
  જો( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->aData, newSz);
  જો( pNew==0 ) પાછા SQLITE_IOERR_NOMEM;
  p->aData = pNew;
  p->szAlloc = newSz;
  પાછા SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
સ્થિર પૂર્ણાંક memdbWrite(
  sqlite3_file *pFile,
  અચલ શૂન્ય *z,
  પૂર્ણાંક iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  જો( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    પાછા SQLITE_IOERR_WRITE;
  }
  જો( iOfst+iAmt>p->sz ){
    પૂર્ણાંક rc;
    જો( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      પાછા rc;
    }
    જો( iOfst>p->sz ) memset(p->aData+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  memcpy(p->aData+iOfst, z, iAmt);
  memdbLeave(p);
  પાછા SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
સ્થિર પૂર્ણાંક memdbTruncate(sqlite3_file *pFile, sqlite_int64 size){
  MemStore *p = ((MemFile*)pFile)->pStore;
  પૂર્ણાંક rc = SQLITE_OK;
  memdbEnter(p);
  જો( size>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }નહીંતર{
    p->sz = size; 
  }
  memdbLeave(p);
  પાછા rc;
}

/*
** Sync an memdb-file.
*/
સ્થિર પૂર્ણાંક memdbSync(sqlite3_file *pFile, પૂર્ણાંક flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  પાછા SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
સ્થિર પૂર્ણાંક memdbFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *pSize = p->sz;
  memdbLeave(p);
  પાછા SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
સ્થિર પૂર્ણાંક memdbLock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  પૂર્ણાંક rc = SQLITE_OK;
  જો( eLock<=pThis->eLock ) પાછા SQLITE_OK;
  memdbEnter(p);

  પ્રતિજ્ઞા( p->nWrLock==0 || p->nWrLock==1 );
  પ્રતિજ્ઞા( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  પ્રતિજ્ઞા( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  જો( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }નહીંતર{
    પસંદગી( eLock ){
      સ્થિતિ SQLITE_LOCK_SHARED: {
        પ્રતિજ્ઞા( pThis->eLock==SQLITE_LOCK_NONE );
        જો( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }નહીંતર{
          p->nRdLock++;
        }
        તોડો;
      };
  
      સ્થિતિ SQLITE_LOCK_RESERVED:
      સ્થિતિ SQLITE_LOCK_PENDING: {
        પ્રતિજ્ઞા( pThis->eLock>=SQLITE_LOCK_SHARED );
        જો( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          જો( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }નહીંતર{
            p->nWrLock = 1;
          }
        }
        તોડો;
      }
  
      મૂળભૂત: {
        પ્રતિજ્ઞા(  eLock==SQLITE_LOCK_EXCLUSIVE );
        પ્રતિજ્ઞા( pThis->eLock>=SQLITE_LOCK_SHARED );
        જો( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }નહીંતર જો( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        તોડો;
      }
    }
  }
  જો( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  પાછા rc;
}

/*
** Unlock an memdb-file.
*/
સ્થિર પૂર્ણાંક memdbUnlock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  જો( eLock>=pThis->eLock ) પાછા SQLITE_OK;
  memdbEnter(p);

  પ્રતિજ્ઞા( eLock==SQLITE_LOCK_SHARED || eLock==SQLITE_LOCK_NONE );
  જો( eLock==SQLITE_LOCK_SHARED ){
    જો( ALWAYS(pThis->eLock>SQLITE_LOCK_SHARED) ){
      p->nWrLock--;
    }
  }નહીંતર{
    જો( pThis->eLock>SQLITE_LOCK_SHARED ){
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  પાછા SQLITE_OK;
}

#જો 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
સ્થિર પૂર્ણાંક memdbCheckReservedLock(sqlite3_file *pFile, પૂર્ણાંક *pResOut){
  *pResOut = 0;
  પાછા SQLITE_OK;
}
#endif


/*
** File control method. For custom operations on an memdb-file.
*/
સ્થિર પૂર્ણાંક memdbFileControl(sqlite3_file *pFile, પૂર્ણાંક op, શૂન્ય *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  પૂર્ણાંક rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  જો( op==SQLITE_FCNTL_VFSNAME ){
    *(અક્ષર**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->aData, p->sz);
    rc = SQLITE_OK;
  }
  જો( op==SQLITE_FCNTL_SIZE_LIMIT ){
    sqlite3_int64 iLimit = *(sqlite3_int64*)pArg;
    જો( iLimit<p->sz ){
      જો( iLimit<0 ){
        iLimit = p->szMax;
      }નહીંતર{
        iLimit = p->sz;
      }
    }
    p->szMax = iLimit;
    *(sqlite3_int64*)pArg = iLimit;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  પાછા rc;
}

#જો 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
સ્થિર પૂર્ણાંક memdbSectorSize(sqlite3_file *pFile){
  પાછા 1024;
}
#endif

/*
** Return the device characteristic flags supported by an memdb-file.
*/
સ્થિર પૂર્ણાંક memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  પાછા SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
સ્થિર પૂર્ણાંક memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  પૂર્ણાંક iAmt,
  શૂન્ય **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  જો( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }નહીંતર{
    p->nMmap++;
    *pp = (શૂન્ય*)(p->aData + iOfst);
  }
  memdbLeave(p);
  પાછા SQLITE_OK;
}

/* Release a memory-mapped page */
સ્થિર પૂર્ણાંક memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, શૂન્ય *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  પાછા SQLITE_OK;
}

/*
** Open an mem file handle.
*/
સ્થિર પૂર્ણાંક memdbOpen(
  sqlite3_vfs *pVfs,
  અચલ અક્ષર *zName,
  sqlite3_file *pFd,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  પૂર્ણાંક szName;
  UNUSED_PARAMETER(pVfs);

  memset(pFile, 0, કદ(*pFile));
  szName = sqlite3Strlen30(zName);
  જો( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    પૂર્ણાંક i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    માટે(i=0; i<memdb_g.nMemStore; i++){
      જો( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        તોડો;
      }
    }
    જો( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( કદ(*p) + (i64)szName + 3 );
      જો( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        પાછા SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             કદ(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      જો( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        પાછા SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, કદ(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mxMemdbSize;
      p->zFName = (અક્ષર*)&p[1];
      memcpy(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      જો( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        પાછા SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }નહીંતર{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }નહીંતર{
    p = sqlite3Malloc( કદ(*p) );
    જો( p==0 ){
      પાછા SQLITE_NOMEM;
    }
    memset(p, 0, કદ(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mxMemdbSize;
  }
  pFile->pStore = p;
  જો( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  પાછા SQLITE_OK;
}

#જો 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
સ્થિર પૂર્ણાંક memdbDelete(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath, પૂર્ણાંક dirSync){
  પાછા SQLITE_IOERR_DELETE;
}
#endif

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
સ્થિર પૂર્ણાંક memdbAccess(
  sqlite3_vfs *pVfs, 
  અચલ અક્ષર *zPath, 
  પૂર્ણાંક flags, 
  પૂર્ણાંક *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  પાછા SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
સ્થિર પૂર્ણાંક memdbFullPathname(
  sqlite3_vfs *pVfs, 
  અચલ અક્ષર *zPath, 
  પૂર્ણાંક nOut, 
  અક્ષર *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  પાછા SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
સ્થિર શૂન્ય *memdbDlOpen(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath){
  પાછા ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
સ્થિર શૂન્ય memdbDlError(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_સંદેશ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
સ્થિર શૂન્ય (*memdbDlSym(sqlite3_vfs *pVfs, શૂન્ય *p, અચલ અક્ષર *zSym))(શૂન્ય){
  પાછા ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
સ્થિર શૂન્ય memdbDlClose(sqlite3_vfs *pVfs, શૂન્ય *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
સ્થિર પૂર્ણાંક memdbRandomness(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *zBufOut){
  પાછા ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
સ્થિર પૂર્ણાંક memdbSleep(sqlite3_vfs *pVfs, પૂર્ણાંક nMicro){
  પાછા ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#જો 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
સ્થિર પૂર્ણાંક memdbCurrentTime(sqlite3_vfs *pVfs, દ્વિગુણ *pTimeOut){
  પાછા ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#endif

સ્થિર પૂર્ણાંક memdbGetLastError(sqlite3_vfs *pVfs, પૂર્ણાંક a, અક્ષર *b){
  પાછા ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
સ્થિર પૂર્ણાંક memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  પાછા ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
સ્થિર MemFile *memdbFromDbSchema(sqlite3 *db, અચલ અક્ષર *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  પૂર્ણાંક rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  જો( rc ) પાછા 0;
  જો( p->base.pMethods!=&memdb_io_methods ) પાછા 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  જો( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  પાછા p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
unsigned અક્ષર *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  અચલ અક્ષર *zSchema,      /* Which database within the connection */
  sqlite3_int64 *piSize,    /* Write size here, if not NULL */
  unsigned પૂર્ણાંક mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  પૂર્ણાંક iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  પૂર્ણાંક szPage = 0;
  sqlite3_stmt *pStmt = 0;
  unsigned અક્ષર *pOut = 0;
  અક્ષર *zSql;
  પૂર્ણાંક rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !sqlite3SafetyCheckOk(db) ){
    (શૂન્ય)SQLITE_MISUSE_BKPT;
    પાછા 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  જો( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  જો( piSize ) *piSize = -1;
  જો( iDb<0 ) જાઓ serialize_out;
  જો( p ){
    MemStore *pStore = p->pStore;
    પ્રતિજ્ઞા( pStore->pMutex==0 );
    જો( piSize ) *piSize = pStore->sz;
    જો( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->aData;
    }નહીંતર{
      pOut = sqlite3_malloc64( pStore->sz );
      જો( pOut ) memcpy(pOut, pStore->aData, pStore->sz);
    }
    જાઓ serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  જો( pBt==0 ) જાઓ serialize_out;
  szPage = sqlite3BtreeGetPageSize(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  જો( rc ) જાઓ serialize_out;
  rc = sqlite3_step(pStmt);
  જો( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    જો( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      જો( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    જો( piSize ) *piSize = sz;
    જો( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }નહીંતર{
      pOut = sqlite3_malloc64( sz );
      જો( pOut ){
        પૂર્ણાંક nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        પૂર્ણાંક pgno;
        માટે(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          unsigned અક્ષર *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          જો( rc==SQLITE_OK ){
            memcpy(pTo, sqlite3PagerGetData(pPage), szPage);
          }નહીંતર જો( pgno==PENDING_BYTE/szPage+1 ){
            memset(pTo, 0, szPage);
          }નહીંતર{
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
  પાછા pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
પૂર્ણાંક sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  અચલ અક્ષર *zSchema,    /* Which DB to reopen with the deserialization */
  unsigned અક્ષર *pData,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  unsigned mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  અક્ષર *zSql;
  sqlite3_stmt *pStmt = 0;
  પૂર્ણાંક rc;
  પૂર્ણાંક iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  જો( !sqlite3SafetyCheckOk(db) ){
    પાછા SQLITE_MISUSE_BKPT;
  }
  જો( szDb<0 ) પાછા SQLITE_MISUSE_BKPT;
  જો( szBuf<0 ) પાછા SQLITE_MISUSE_BKPT;
#endif

  sqlite3_mutex_enter(db->mutex);
  જો( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  જો( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    જાઓ end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  જો( zSql==0 ){
    rc = SQLITE_NOMEM;
  }નહીંતર{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  જો( rc ) જાઓ end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  જો( rc!=SQLITE_OK ){
    જાઓ end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  જો( p==0 ){
    rc = SQLITE_ERROR;
  }નહીંતર{
    MemStore *pStore = p->pStore;
    pStore->aData = pData;
    pData = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    જો( pStore->szMax<sqlite3GlobalConfig.mxMemdbSize ){
      pStore->szMax = sqlite3GlobalConfig.mxMemdbSize;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  જો( pData && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(pData);
  }
  sqlite3_mutex_leave(db->mutex);
  પાછા rc;
}

/*
** Return true if the VFS is the memvfs.
*/
પૂર્ણાંક sqlite3IsMemdb(અચલ sqlite3_vfs *pVfs){
  પાછા pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
પૂર્ણાંક sqlite3MemdbInit(શૂન્ય){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  unsigned પૂર્ણાંક sz;
  જો( NEVER(pLower==0) ) પાછા SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.pAppData = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  જો( sz<કદ(MemFile) ) sz = કદ(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  પાછા sqlite3_vfs_register(&memdb_vfs, 0);
}
#endif /* SQLITE_OMIT_DESERIALIZE */
