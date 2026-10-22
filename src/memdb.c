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
#അന്തര്ഭാവ "sqliteInt.h"
#ifndef SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
തരനിർവചനം ഘടന sqlite3_vfs MemVfs;
തരനിർവചനം ഘടന MemFile MemFile;
തരനിർവചനം ഘടന MemStore MemStore;

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
ഘടന MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  unsigned അക്ഷരം *aData;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  പൂർണ്ണസംഖ്യ nMmap;                      /* Number of memory mapped pages */
  unsigned mFlags;                /* Flags */
  പൂർണ്ണസംഖ്യ nRdLock;                    /* Number of readers */
  പൂർണ്ണസംഖ്യ nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  പൂർണ്ണസംഖ്യ nRef;                       /* Number of users of this MemStore */
  അക്ഷരം *zFName;                   /* The filename for shared stores */
};

/* An open file */
ഘടന MemFile {
  sqlite3_file base;              /* IO methods */
  MemStore *pStore;               /* The storage */
  പൂർണ്ണസംഖ്യ eLock;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
സ്ഥിതം ഘടന MemFS {
  പൂർണ്ണസംഖ്യ nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbClose(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbRead(sqlite3_file*, ശൂന്യം*, പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbWrite(sqlite3_file*,സ്ഥിരം ശൂന്യം*,പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbTruncate(sqlite3_file*, sqlite3_int64 size);
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbSync(sqlite3_file*, പൂർണ്ണസംഖ്യ flags);
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbFileSize(sqlite3_file*, sqlite3_int64 *pSize);
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbLock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbUnlock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbFileControl(sqlite3_file*, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbDeviceCharacteristics(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, പൂർണ്ണസംഖ്യ iAmt, ശൂന്യം **pp);
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, ശൂന്യം *p);

/*
** Methods for MemVfs
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbOpen(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *, sqlite3_file*, പൂർണ്ണസംഖ്യ , പൂർണ്ണസംഖ്യ *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbAccess(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ flags, പൂർണ്ണസംഖ്യ *);
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbFullPathname(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ, അക്ഷരം *zOut);
സ്ഥിതം ശൂന്യം *memdbDlOpen(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zFilename);
സ്ഥിതം ശൂന്യം memdbDlError(sqlite3_vfs*, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *z_Err_സന്ദേശം);
സ്ഥിതം ശൂന്യം (*memdbDlSym(sqlite3_vfs *pVfs, ശൂന്യം *p, സ്ഥിരം അക്ഷരം*zSym))(ശൂന്യം);
സ്ഥിതം ശൂന്യം memdbDlClose(sqlite3_vfs*, ശൂന്യം*);
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbRandomness(sqlite3_vfs*, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zOut);
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbSleep(sqlite3_vfs*, പൂർണ്ണസംഖ്യ microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbGetLastError(sqlite3_vfs*, പൂർണ്ണസംഖ്യ, അക്ഷരം *);
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

സ്ഥിതം sqlite3_vfs memdb_vfs = {
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

സ്ഥിതം സ്ഥിരം sqlite3_io_methods memdb_io_methods = {
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
#എങ്കിൽ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
സ്ഥിതം ശൂന്യം memdbEnter(MemStore *p){
  UNUSED_PARAMETER(p);
}
സ്ഥിതം ശൂന്യം memdbLeave(MemStore *p){
  UNUSED_PARAMETER(p);
}
#അല്ലെങ്കിൽ
സ്ഥിതം ശൂന്യം memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
സ്ഥിതം ശൂന്യം memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#endif



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  എങ്കിൽ( p->zFName ){
    പൂർണ്ണസംഖ്യ i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    വേണ്ടി(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      എങ്കിൽ( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        എങ്കിൽ( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          എങ്കിൽ( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        നിർത്തുക;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }അല്ലെങ്കിൽ{
    memdbEnter(p);
  }
  p->nRef--;
  എങ്കിൽ( p->nRef<=0 ){
    എങ്കിൽ( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->aData);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }അല്ലെങ്കിൽ{
    memdbLeave(p);
  }
  തിരികെ SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbRead(
  sqlite3_file *pFile, 
  ശൂന്യം *zBuf, 
  പൂർണ്ണസംഖ്യ iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  എങ്കിൽ( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    എങ്കിൽ( iOfst<p->sz ) memcpy(zBuf, p->aData+iOfst, p->sz - iOfst);
    memdbLeave(p);
    തിരികെ SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, p->aData+iOfst, iAmt);
  memdbLeave(p);
  തിരികെ SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  unsigned അക്ഷരം *pNew;
  എങ്കിൽ( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    തിരികെ SQLITE_FULL;
  }
  എങ്കിൽ( newSz>p->szMax ){
    തിരികെ SQLITE_FULL;
  }
  newSz *= 2;
  എങ്കിൽ( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->aData, newSz);
  എങ്കിൽ( pNew==0 ) തിരികെ SQLITE_IOERR_NOMEM;
  p->aData = pNew;
  p->szAlloc = newSz;
  തിരികെ SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbWrite(
  sqlite3_file *pFile,
  സ്ഥിരം ശൂന്യം *z,
  പൂർണ്ണസംഖ്യ iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  എങ്കിൽ( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    തിരികെ SQLITE_IOERR_WRITE;
  }
  എങ്കിൽ( iOfst+iAmt>p->sz ){
    പൂർണ്ണസംഖ്യ rc;
    എങ്കിൽ( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      തിരികെ rc;
    }
    എങ്കിൽ( iOfst>p->sz ) memset(p->aData+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  memcpy(p->aData+iOfst, z, iAmt);
  memdbLeave(p);
  തിരികെ SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbTruncate(sqlite3_file *pFile, sqlite_int64 size){
  MemStore *p = ((MemFile*)pFile)->pStore;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  memdbEnter(p);
  എങ്കിൽ( size>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }അല്ലെങ്കിൽ{
    p->sz = size; 
  }
  memdbLeave(p);
  തിരികെ rc;
}

/*
** Sync an memdb-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbSync(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  തിരികെ SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *pSize = p->sz;
  memdbLeave(p);
  തിരികെ SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  എങ്കിൽ( eLock<=pThis->eLock ) തിരികെ SQLITE_OK;
  memdbEnter(p);

  പ്രതിജ്ഞാ( p->nWrLock==0 || p->nWrLock==1 );
  പ്രതിജ്ഞാ( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  പ്രതിജ്ഞാ( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  എങ്കിൽ( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }അല്ലെങ്കിൽ{
    തിരഞ്ഞെടുപ്പ്( eLock ){
      അവസ്ഥ SQLITE_LOCK_SHARED: {
        പ്രതിജ്ഞാ( pThis->eLock==SQLITE_LOCK_NONE );
        എങ്കിൽ( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }അല്ലെങ്കിൽ{
          p->nRdLock++;
        }
        നിർത്തുക;
      };
  
      അവസ്ഥ SQLITE_LOCK_RESERVED:
      അവസ്ഥ SQLITE_LOCK_PENDING: {
        പ്രതിജ്ഞാ( pThis->eLock>=SQLITE_LOCK_SHARED );
        എങ്കിൽ( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          എങ്കിൽ( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }അല്ലെങ്കിൽ{
            p->nWrLock = 1;
          }
        }
        നിർത്തുക;
      }
  
      സ്ഥിരസ്ഥിതി: {
        പ്രതിജ്ഞാ(  eLock==SQLITE_LOCK_EXCLUSIVE );
        പ്രതിജ്ഞാ( pThis->eLock>=SQLITE_LOCK_SHARED );
        എങ്കിൽ( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }അല്ലെങ്കിൽ എങ്കിൽ( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        നിർത്തുക;
      }
    }
  }
  എങ്കിൽ( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  തിരികെ rc;
}

/*
** Unlock an memdb-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbUnlock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  എങ്കിൽ( eLock>=pThis->eLock ) തിരികെ SQLITE_OK;
  memdbEnter(p);

  പ്രതിജ്ഞാ( eLock==SQLITE_LOCK_SHARED || eLock==SQLITE_LOCK_NONE );
  എങ്കിൽ( eLock==SQLITE_LOCK_SHARED ){
    എങ്കിൽ( ALWAYS(pThis->eLock>SQLITE_LOCK_SHARED) ){
      p->nWrLock--;
    }
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( pThis->eLock>SQLITE_LOCK_SHARED ){
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  തിരികെ SQLITE_OK;
}

#എങ്കിൽ 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbCheckReservedLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ *pResOut){
  *pResOut = 0;
  തിരികെ SQLITE_OK;
}
#endif


/*
** File control method. For custom operations on an memdb-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbFileControl(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  പൂർണ്ണസംഖ്യ rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  എങ്കിൽ( op==SQLITE_FCNTL_VFSNAME ){
    *(അക്ഷരം**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->aData, p->sz);
    rc = SQLITE_OK;
  }
  എങ്കിൽ( op==SQLITE_FCNTL_SIZE_LIMIT ){
    sqlite3_int64 iLimit = *(sqlite3_int64*)pArg;
    എങ്കിൽ( iLimit<p->sz ){
      എങ്കിൽ( iLimit<0 ){
        iLimit = p->szMax;
      }അല്ലെങ്കിൽ{
        iLimit = p->sz;
      }
    }
    p->szMax = iLimit;
    *(sqlite3_int64*)pArg = iLimit;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  തിരികെ rc;
}

#എങ്കിൽ 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbSectorSize(sqlite3_file *pFile){
  തിരികെ 1024;
}
#endif

/*
** Return the device characteristic flags supported by an memdb-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  തിരികെ SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  പൂർണ്ണസംഖ്യ iAmt,
  ശൂന്യം **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  എങ്കിൽ( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }അല്ലെങ്കിൽ{
    p->nMmap++;
    *pp = (ശൂന്യം*)(p->aData + iOfst);
  }
  memdbLeave(p);
  തിരികെ SQLITE_OK;
}

/* Release a memory-mapped page */
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, ശൂന്യം *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  തിരികെ SQLITE_OK;
}

/*
** Open an mem file handle.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbOpen(
  sqlite3_vfs *pVfs,
  സ്ഥിരം അക്ഷരം *zName,
  sqlite3_file *pFd,
  പൂർണ്ണസംഖ്യ flags,
  പൂർണ്ണസംഖ്യ *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  പൂർണ്ണസംഖ്യ szName;
  UNUSED_PARAMETER(pVfs);

  memset(pFile, 0, വലുപ്പം(*pFile));
  szName = sqlite3Strlen30(zName);
  എങ്കിൽ( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    പൂർണ്ണസംഖ്യ i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    വേണ്ടി(i=0; i<memdb_g.nMemStore; i++){
      എങ്കിൽ( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        നിർത്തുക;
      }
    }
    എങ്കിൽ( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( വലുപ്പം(*p) + (i64)szName + 3 );
      എങ്കിൽ( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        തിരികെ SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             വലുപ്പം(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      എങ്കിൽ( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        തിരികെ SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, വലുപ്പം(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mxMemdbSize;
      p->zFName = (അക്ഷരം*)&p[1];
      memcpy(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      എങ്കിൽ( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        തിരികെ SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }അല്ലെങ്കിൽ{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }അല്ലെങ്കിൽ{
    p = sqlite3Malloc( വലുപ്പം(*p) );
    എങ്കിൽ( p==0 ){
      തിരികെ SQLITE_NOMEM;
    }
    memset(p, 0, വലുപ്പം(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mxMemdbSize;
  }
  pFile->pStore = p;
  എങ്കിൽ( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  തിരികെ SQLITE_OK;
}

#എങ്കിൽ 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbDelete(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath, പൂർണ്ണസംഖ്യ dirSync){
  തിരികെ SQLITE_IOERR_DELETE;
}
#endif

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbAccess(
  sqlite3_vfs *pVfs, 
  സ്ഥിരം അക്ഷരം *zPath, 
  പൂർണ്ണസംഖ്യ flags, 
  പൂർണ്ണസംഖ്യ *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  തിരികെ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbFullPathname(
  sqlite3_vfs *pVfs, 
  സ്ഥിരം അക്ഷരം *zPath, 
  പൂർണ്ണസംഖ്യ nOut, 
  അക്ഷരം *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  തിരികെ SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
സ്ഥിതം ശൂന്യം *memdbDlOpen(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath){
  തിരികെ ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
സ്ഥിതം ശൂന്യം memdbDlError(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *z_Err_സന്ദേശം){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_സന്ദേശം);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
സ്ഥിതം ശൂന്യം (*memdbDlSym(sqlite3_vfs *pVfs, ശൂന്യം *p, സ്ഥിരം അക്ഷരം *zSym))(ശൂന്യം){
  തിരികെ ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
സ്ഥിതം ശൂന്യം memdbDlClose(sqlite3_vfs *pVfs, ശൂന്യം *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbRandomness(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zBufOut){
  തിരികെ ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbSleep(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nMicro){
  തിരികെ ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#എങ്കിൽ 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbCurrentTime(sqlite3_vfs *pVfs, ഇരട്ട *pTimeOut){
  തിരികെ ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ memdbGetLastError(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ a, അക്ഷരം *b){
  തിരികെ ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  തിരികെ ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
സ്ഥിതം MemFile *memdbFromDbSchema(sqlite3 *db, സ്ഥിരം അക്ഷരം *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  പൂർണ്ണസംഖ്യ rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  എങ്കിൽ( rc ) തിരികെ 0;
  എങ്കിൽ( p->base.pMethods!=&memdb_io_methods ) തിരികെ 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  എങ്കിൽ( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  തിരികെ p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
unsigned അക്ഷരം *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  സ്ഥിരം അക്ഷരം *zSchema,      /* Which database within the connection */
  sqlite3_int64 *piSize,    /* Write size here, if not NULL */
  unsigned പൂർണ്ണസംഖ്യ mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  പൂർണ്ണസംഖ്യ iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  പൂർണ്ണസംഖ്യ szPage = 0;
  sqlite3_stmt *pStmt = 0;
  unsigned അക്ഷരം *pOut = 0;
  അക്ഷരം *zSql;
  പൂർണ്ണസംഖ്യ rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( !sqlite3SafetyCheckOk(db) ){
    (ശൂന്യം)SQLITE_MISUSE_BKPT;
    തിരികെ 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  എങ്കിൽ( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  എങ്കിൽ( piSize ) *piSize = -1;
  എങ്കിൽ( iDb<0 ) പോകുക serialize_out;
  എങ്കിൽ( p ){
    MemStore *pStore = p->pStore;
    പ്രതിജ്ഞാ( pStore->pMutex==0 );
    എങ്കിൽ( piSize ) *piSize = pStore->sz;
    എങ്കിൽ( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->aData;
    }അല്ലെങ്കിൽ{
      pOut = sqlite3_malloc64( pStore->sz );
      എങ്കിൽ( pOut ) memcpy(pOut, pStore->aData, pStore->sz);
    }
    പോകുക serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  എങ്കിൽ( pBt==0 ) പോകുക serialize_out;
  szPage = sqlite3BtreeGetPageSize(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  എങ്കിൽ( rc ) പോകുക serialize_out;
  rc = sqlite3_step(pStmt);
  എങ്കിൽ( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    എങ്കിൽ( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      എങ്കിൽ( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    എങ്കിൽ( piSize ) *piSize = sz;
    എങ്കിൽ( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }അല്ലെങ്കിൽ{
      pOut = sqlite3_malloc64( sz );
      എങ്കിൽ( pOut ){
        പൂർണ്ണസംഖ്യ nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        പൂർണ്ണസംഖ്യ pgno;
        വേണ്ടി(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          unsigned അക്ഷരം *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          എങ്കിൽ( rc==SQLITE_OK ){
            memcpy(pTo, sqlite3PagerGetData(pPage), szPage);
          }അല്ലെങ്കിൽ എങ്കിൽ( pgno==PENDING_BYTE/szPage+1 ){
            memset(pTo, 0, szPage);
          }അല്ലെങ്കിൽ{
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
  തിരികെ pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
പൂർണ്ണസംഖ്യ sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  സ്ഥിരം അക്ഷരം *zSchema,    /* Which DB to reopen with the deserialization */
  unsigned അക്ഷരം *pData,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  unsigned mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  അക്ഷരം *zSql;
  sqlite3_stmt *pStmt = 0;
  പൂർണ്ണസംഖ്യ rc;
  പൂർണ്ണസംഖ്യ iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  എങ്കിൽ( !sqlite3SafetyCheckOk(db) ){
    തിരികെ SQLITE_MISUSE_BKPT;
  }
  എങ്കിൽ( szDb<0 ) തിരികെ SQLITE_MISUSE_BKPT;
  എങ്കിൽ( szBuf<0 ) തിരികെ SQLITE_MISUSE_BKPT;
#endif

  sqlite3_mutex_enter(db->mutex);
  എങ്കിൽ( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  എങ്കിൽ( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    പോകുക end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  എങ്കിൽ( zSql==0 ){
    rc = SQLITE_NOMEM;
  }അല്ലെങ്കിൽ{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  എങ്കിൽ( rc ) പോകുക end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  എങ്കിൽ( rc!=SQLITE_OK ){
    പോകുക end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  എങ്കിൽ( p==0 ){
    rc = SQLITE_ERROR;
  }അല്ലെങ്കിൽ{
    MemStore *pStore = p->pStore;
    pStore->aData = pData;
    pData = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    എങ്കിൽ( pStore->szMax<sqlite3GlobalConfig.mxMemdbSize ){
      pStore->szMax = sqlite3GlobalConfig.mxMemdbSize;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  എങ്കിൽ( pData && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(pData);
  }
  sqlite3_mutex_leave(db->mutex);
  തിരികെ rc;
}

/*
** Return true if the VFS is the memvfs.
*/
പൂർണ്ണസംഖ്യ sqlite3IsMemdb(സ്ഥിരം sqlite3_vfs *pVfs){
  തിരികെ pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
പൂർണ്ണസംഖ്യ sqlite3MemdbInit(ശൂന്യം){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  unsigned പൂർണ്ണസംഖ്യ sz;
  എങ്കിൽ( NEVER(pLower==0) ) തിരികെ SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.pAppData = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  എങ്കിൽ( sz<വലുപ്പം(MemFile) ) sz = വലുപ്പം(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  തിരികെ sqlite3_vfs_register(&memdb_vfs, 0);
}
#endif /* SQLITE_OMIT_DESERIALIZE */
