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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ sqlite3_vfs MemVfs;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ MemFile MemFile;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ MemStore MemStore;

/* Access to a lower-level VFS that (might) implement dynamic loading,
** access to randomness, etc.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ORIGVFS(p) ((sqlite3_vfs*)((p)->p_App_ਅੰਕੜੇ))

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
ਬਣਤਰ MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *a_ਅੰਕੜੇ;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  ਪੂਰਨ nMmap;                      /* Number of memory mapped pages */
  ਬਿਨਾਨਿਸ਼ਾਨੀ mFlags;                /* Flags */
  ਪੂਰਨ n_Rd_ਤਾਲਾ;                    /* Number of readers */
  ਪੂਰਨ n_Wr_ਤਾਲਾ;                    /* Number of writers.  (Always 0 or 1) */
  ਪੂਰਨ nRef;                       /* Number of users of this MemStore */
  ਅੱਖਰ *zFName;                   /* The filename for shared stores */
};

/* An open file */
ਬਣਤਰ MemFile {
  sqlite3_file ਅਧਾਰ;              /* IO methods */
  MemStore *pStore;               /* The storage */
  ਪੂਰਨ e_ਤਾਲਾ;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
ਸਥਿਰ ਬਣਤਰ MemFS {
  ਪੂਰਨ nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
ਸਥਿਰ ਪੂਰਨ memdbClose(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ memdbRead(sqlite3_file*, ਖਾਲੀ*, ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ memdbWrite(sqlite3_file*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ memdbTruncate(sqlite3_file*, sqlite3_int64 ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ memdbSync(sqlite3_file*, ਪੂਰਨ flags);
ਸਥਿਰ ਪੂਰਨ memdb_File_ਮਾਪ(sqlite3_file*, sqlite3_int64 *p_ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ memdb_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ memdbUnlock(sqlite3_file*, ਪੂਰਨ);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
ਸਥਿਰ ਪੂਰਨ memdbFileControl(sqlite3_file*, ਪੂਰਨ op, ਖਾਲੀ *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
ਸਥਿਰ ਪੂਰਨ memdbDeviceCharacteristics(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, ਪੂਰਨ iAmt, ਖਾਲੀ **pp);
ਸਥਿਰ ਪੂਰਨ memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, ਖਾਲੀ *p);

/*
** Methods for MemVfs
*/
ਸਥਿਰ ਪੂਰਨ memdbOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *, sqlite3_file*, ਪੂਰਨ , ਪੂਰਨ *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
ਸਥਿਰ ਪੂਰਨ memdbAccess(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ flags, ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ memdb_ਭਰਿਆ_Pathname(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ, ਅੱਖਰ *z_ਬਾਹਰਲਾ);
ਸਥਿਰ ਖਾਲੀ *memdbDlOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zFilename);
ਸਥਿਰ ਖਾਲੀ memdbDlError(sqlite3_vfs*, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ);
ਸਥਿਰ ਖਾਲੀ (*memdbDlSym(sqlite3_vfs *pVfs, ਖਾਲੀ *p, ਅਚਲ ਅੱਖਰ*zSym))(ਖਾਲੀ);
ਸਥਿਰ ਖਾਲੀ memdbDlClose(sqlite3_vfs*, ਖਾਲੀ*);
ਸਥਿਰ ਪੂਰਨ memdbRandomness(sqlite3_vfs*, ਪੂਰਨ nByte, ਅੱਖਰ *z_ਬਾਹਰਲਾ);
ਸਥਿਰ ਪੂਰਨ memdbSleep(sqlite3_vfs*, ਪੂਰਨ microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
ਸਥਿਰ ਪੂਰਨ memdb_Get_ਆਖਰੀ_Error(sqlite3_vfs*, ਪੂਰਨ, ਅੱਖਰ *);
ਸਥਿਰ ਪੂਰਨ memdb_Current_ਸਮਾਂ_Int64(sqlite3_vfs*, sqlite3_int64*);

ਸਥਿਰ sqlite3_vfs memdb_vfs = {
  2,                           /* iVersion */
  0,                           /* szOsFile (set when registered) */
  1024,                        /* mxPathname */
  0,                           /* pNext */
  "memdb",                     /* zName */
  0,                           /* pAppData (set when registered) */ 
  memdbOpen,                   /* xOpen */
  0, /* memdbDelete, */        /* xDelete */
  memdbAccess,                 /* xAccess */
  memdb_ਭਰਿਆ_Pathname,           /* xFullPathname */
  memdbDlOpen,                 /* xDlOpen */
  memdbDlError,                /* xDlError */
  memdbDlSym,                  /* xDlSym */
  memdbDlClose,                /* xDlClose */
  memdbRandomness,             /* xRandomness */
  memdbSleep,                  /* xSleep */
  0, /* memdbCurrentTime, */   /* xCurrentTime */
  memdb_Get_ਆਖਰੀ_Error,           /* xGetLastError */
  memdb_Current_ਸਮਾਂ_Int64,       /* xCurrentTimeInt64 */
  0,                           /* xSetSystemCall */
  0,                           /* xGetSystemCall */
  0,                           /* xNextSystemCall */
};

ਸਥਿਰ ਅਚਲ sqlite3_io_methods memdb_io_methods = {
  3,                              /* iVersion */
  memdbClose,                      /* xClose */
  memdbRead,                       /* xRead */
  memdbWrite,                      /* xWrite */
  memdbTruncate,                   /* xTruncate */
  memdbSync,                       /* xSync */
  memdb_File_ਮਾਪ,                   /* xFileSize */
  memdb_ਤਾਲਾ,                       /* xLock */
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
#ਜੇ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
ਸਥਿਰ ਖਾਲੀ memdbEnter(MemStore *p){
  ਅਣਵਰਤਿਆ_PARAMETER(p);
}
ਸਥਿਰ ਖਾਲੀ memdbLeave(MemStore *p){
  ਅਣਵਰਤਿਆ_PARAMETER(p);
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
ਸਥਿਰ ਖਾਲੀ memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#ਜੇ_ਅੰਤ



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
ਸਥਿਰ ਪੂਰਨ memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ਜੇ( p->zFName ){
    ਪੂਰਨ i;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#ਜੇ_ਅੰਤ
    sqlite3_mutex_enter(pVfsMutex);
    ਲਈ(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      ਜੇ( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        ਜੇ( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          ਜੇ( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        ਰੋਕੋ;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }ਨਹੀਂਤਾਂ{
    memdbEnter(p);
  }
  p->nRef--;
  ਜੇ( p->nRef<=0 ){
    ਜੇ( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->a_ਅੰਕੜੇ);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }ਨਹੀਂਤਾਂ{
    memdbLeave(p);
  }
  ਵਾਪਸ SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
ਸਥਿਰ ਪੂਰਨ memdbRead(
  sqlite3_file *pFile, 
  ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ਜੇ( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    ਜੇ( iOfst<p->sz ) ਸਮ੍ਰਿਤੀ_ਨਕਲ(zBuf, p->a_ਅੰਕੜੇ+iOfst, p->sz - iOfst);
    memdbLeave(p);
    ਵਾਪਸ SQLITE_IOERR_SHORT_READ;
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(zBuf, p->a_ਅੰਕੜੇ+iOfst, iAmt);
  memdbLeave(p);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
ਸਥਿਰ ਪੂਰਨ memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *pNew;
  ਜੇ( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    ਵਾਪਸ SQLITE_ਭਰਿਆ;
  }
  ਜੇ( newSz>p->szMax ){
    ਵਾਪਸ SQLITE_ਭਰਿਆ;
  }
  newSz *= 2;
  ਜੇ( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->a_ਅੰਕੜੇ, newSz);
  ਜੇ( pNew==0 ) ਵਾਪਸ SQLITE_IOERR_NOMEM;
  p->a_ਅੰਕੜੇ = pNew;
  p->szAlloc = newSz;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
ਸਥਿਰ ਪੂਰਨ memdbWrite(
  sqlite3_file *pFile,
  ਅਚਲ ਖਾਲੀ *z,
  ਪੂਰਨ iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ਜੇ( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    ਵਾਪਸ SQLITE_IOERR_WRITE;
  }
  ਜੇ( iOfst+iAmt>p->sz ){
    ਪੂਰਨ rc;
    ਜੇ( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      ਵਾਪਸ rc;
    }
    ਜੇ( iOfst>p->sz ) memset(p->a_ਅੰਕੜੇ+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p->a_ਅੰਕੜੇ+iOfst, z, iAmt);
  memdbLeave(p);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
ਸਥਿਰ ਪੂਰਨ memdbTruncate(sqlite3_file *pFile, sqlite_int64 ਮਾਪ){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ਪੂਰਨ rc = SQLITE_OK;
  memdbEnter(p);
  ਜੇ( ਮਾਪ>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }ਨਹੀਂਤਾਂ{
    p->sz = ਮਾਪ; 
  }
  memdbLeave(p);
  ਵਾਪਸ rc;
}

/*
** Sync an memdb-file.
*/
ਸਥਿਰ ਪੂਰਨ memdbSync(sqlite3_file *pFile, ਪੂਰਨ flags){
  ਅਣਵਰਤਿਆ_PARAMETER(pFile);
  ਅਣਵਰਤਿਆ_PARAMETER(flags);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
ਸਥਿਰ ਪੂਰਨ memdb_File_ਮਾਪ(sqlite3_file *pFile, sqlite_int64 *p_ਮਾਪ){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *p_ਮਾਪ = p->sz;
  memdbLeave(p);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
ਸਥਿਰ ਪੂਰਨ memdb_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  ਪੂਰਨ rc = SQLITE_OK;
  ਜੇ( e_ਤਾਲਾ<=pThis->e_ਤਾਲਾ ) ਵਾਪਸ SQLITE_OK;
  memdbEnter(p);

  ਪ੍ਰਤਿਜ੍ਞਾ( p->n_Wr_ਤਾਲਾ==0 || p->n_Wr_ਤਾਲਾ==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pThis->e_ਤਾਲਾ<=SQLITE_ਤਾਲਾ_SHARED || p->n_Wr_ਤਾਲਾ==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pThis->e_ਤਾਲਾ==SQLITE_ਤਾਲਾ_NONE || p->n_Rd_ਤਾਲਾ>=1 );

  ਜੇ( e_ਤਾਲਾ>SQLITE_ਤਾਲਾ_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }ਨਹੀਂਤਾਂ{
    ਚੋਣ( e_ਤਾਲਾ ){
      ਹਾਲਤ SQLITE_ਤਾਲਾ_SHARED: {
        ਪ੍ਰਤਿਜ੍ਞਾ( pThis->e_ਤਾਲਾ==SQLITE_ਤਾਲਾ_NONE );
        ਜੇ( p->n_Wr_ਤਾਲਾ>0 ){
          rc = SQLITE_BUSY;
        }ਨਹੀਂਤਾਂ{
          p->n_Rd_ਤਾਲਾ++;
        }
        ਰੋਕੋ;
      };
  
      ਹਾਲਤ SQLITE_ਤਾਲਾ_RESERVED:
      ਹਾਲਤ SQLITE_ਤਾਲਾ_PENDING: {
        ਪ੍ਰਤਿਜ੍ਞਾ( pThis->e_ਤਾਲਾ>=SQLITE_ਤਾਲਾ_SHARED );
        ਜੇ( ALWAYS(pThis->e_ਤਾਲਾ==SQLITE_ਤਾਲਾ_SHARED) ){
          ਜੇ( p->n_Wr_ਤਾਲਾ>0 ){
            rc = SQLITE_BUSY;
          }ਨਹੀਂਤਾਂ{
            p->n_Wr_ਤਾਲਾ = 1;
          }
        }
        ਰੋਕੋ;
      }
  
      ਮੂਲ: {
        ਪ੍ਰਤਿਜ੍ਞਾ(  e_ਤਾਲਾ==SQLITE_ਤਾਲਾ_EXCLUSIVE );
        ਪ੍ਰਤਿਜ੍ਞਾ( pThis->e_ਤਾਲਾ>=SQLITE_ਤਾਲਾ_SHARED );
        ਜੇ( p->n_Rd_ਤਾਲਾ>1 ){
          rc = SQLITE_BUSY;
        }ਨਹੀਂਤਾਂ ਜੇ( pThis->e_ਤਾਲਾ==SQLITE_ਤਾਲਾ_SHARED ){
          p->n_Wr_ਤਾਲਾ = 1;
        }
        ਰੋਕੋ;
      }
    }
  }
  ਜੇ( rc==SQLITE_OK ) pThis->e_ਤਾਲਾ = e_ਤਾਲਾ;
  memdbLeave(p);
  ਵਾਪਸ rc;
}

/*
** Unlock an memdb-file.
*/
ਸਥਿਰ ਪੂਰਨ memdbUnlock(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  ਜੇ( e_ਤਾਲਾ>=pThis->e_ਤਾਲਾ ) ਵਾਪਸ SQLITE_OK;
  memdbEnter(p);

  ਪ੍ਰਤਿਜ੍ਞਾ( e_ਤਾਲਾ==SQLITE_ਤਾਲਾ_SHARED || e_ਤਾਲਾ==SQLITE_ਤਾਲਾ_NONE );
  ਜੇ( e_ਤਾਲਾ==SQLITE_ਤਾਲਾ_SHARED ){
    ਜੇ( ALWAYS(pThis->e_ਤਾਲਾ>SQLITE_ਤਾਲਾ_SHARED) ){
      p->n_Wr_ਤਾਲਾ--;
    }
  }ਨਹੀਂਤਾਂ{
    ਜੇ( pThis->e_ਤਾਲਾ>SQLITE_ਤਾਲਾ_SHARED ){
      p->n_Wr_ਤਾਲਾ--;
    }
    p->n_Rd_ਤਾਲਾ--;
  }

  pThis->e_ਤਾਲਾ = e_ਤਾਲਾ;
  memdbLeave(p);
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
ਸਥਿਰ ਪੂਰਨ memdb_Check_Reserved_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  *p_Res_ਬਾਹਰਲਾ = 0;
  ਵਾਪਸ SQLITE_OK;
}
#ਜੇ_ਅੰਤ


/*
** File control method. For custom operations on an memdb-file.
*/
ਸਥਿਰ ਪੂਰਨ memdbFileControl(sqlite3_file *pFile, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ਪੂਰਨ rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  ਜੇ( op==SQLITE_FCNTL_VFSNAME ){
    *(ਅੱਖਰ**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->a_ਅੰਕੜੇ, p->sz);
    rc = SQLITE_OK;
  }
  ਜੇ( op==SQLITE_FCNTL_ਮਾਪ_ਹੱਦਬੰਦੀ ){
    sqlite3_int64 i_ਹੱਦਬੰਦੀ = *(sqlite3_int64*)pArg;
    ਜੇ( i_ਹੱਦਬੰਦੀ<p->sz ){
      ਜੇ( i_ਹੱਦਬੰਦੀ<0 ){
        i_ਹੱਦਬੰਦੀ = p->szMax;
      }ਨਹੀਂਤਾਂ{
        i_ਹੱਦਬੰਦੀ = p->sz;
      }
    }
    p->szMax = i_ਹੱਦਬੰਦੀ;
    *(sqlite3_int64*)pArg = i_ਹੱਦਬੰਦੀ;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  ਵਾਪਸ rc;
}

#ਜੇ 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
ਸਥਿਰ ਪੂਰਨ memdb_Sector_ਮਾਪ(sqlite3_file *pFile){
  ਵਾਪਸ 1024;
}
#ਜੇ_ਅੰਤ

/*
** Return the device characteristic flags supported by an memdb-file.
*/
ਸਥਿਰ ਪੂਰਨ memdbDeviceCharacteristics(sqlite3_file *pFile){
  ਅਣਵਰਤਿਆ_PARAMETER(pFile);
  ਵਾਪਸ SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_ਸੁਰੱਖਿਅਤ_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
ਸਥਿਰ ਪੂਰਨ memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  ਪੂਰਨ iAmt,
  ਖਾਲੀ **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ਜੇ( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }ਨਹੀਂਤਾਂ{
    p->nMmap++;
    *pp = (ਖਾਲੀ*)(p->a_ਅੰਕੜੇ + iOfst);
  }
  memdbLeave(p);
  ਵਾਪਸ SQLITE_OK;
}

/* Release a memory-mapped page */
ਸਥਿਰ ਪੂਰਨ memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, ਖਾਲੀ *p_ਪੰਨਾ){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ਅਣਵਰਤਿਆ_PARAMETER(iOfst);
  ਅਣਵਰਤਿਆ_PARAMETER(p_ਪੰਨਾ);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Open an mem file handle.
*/
ਸਥਿਰ ਪੂਰਨ memdbOpen(
  sqlite3_vfs *pVfs,
  ਅਚਲ ਅੱਖਰ *zName,
  sqlite3_file *pFd,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  ਪੂਰਨ szName;
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);

  memset(pFile, 0, ਆਕਾਰ(*pFile));
  szName = sqlite3Strlen30(zName);
  ਜੇ( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    ਪੂਰਨ i;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#ਜੇ_ਅੰਤ
    sqlite3_mutex_enter(pVfsMutex);
    ਲਈ(i=0; i<memdb_g.nMemStore; i++){
      ਜੇ( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        ਰੋਕੋ;
      }
    }
    ਜੇ( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( ਆਕਾਰ(*p) + (i64)szName + 3 );
      ਜੇ( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        ਵਾਪਸ SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             ਆਕਾਰ(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      ਜੇ( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ਵਾਪਸ SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, ਆਕਾਰ(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mx_Memdb_ਮਾਪ;
      p->zFName = (ਅੱਖਰ*)&p[1];
      ਸਮ੍ਰਿਤੀ_ਨਕਲ(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      ਜੇ( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ਵਾਪਸ SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }ਨਹੀਂਤਾਂ{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }ਨਹੀਂਤਾਂ{
    p = sqlite3Malloc( ਆਕਾਰ(*p) );
    ਜੇ( p==0 ){
      ਵਾਪਸ SQLITE_NOMEM;
    }
    memset(p, 0, ਆਕਾਰ(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mx_Memdb_ਮਾਪ;
  }
  pFile->pStore = p;
  ਜੇ( p_ਬਾਹਰਲਾ_Flags!=0 ){
    *p_ਬਾਹਰਲਾ_Flags = flags | SQLITE_OPEN_ਸਮ੍ਰਿਤੀ;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  ਵਾਪਸ SQLITE_OK;
}

#ਜੇ 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ਸਥਿਰ ਪੂਰਨ memdbDelete(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਪੂਰਨ dirSync){
  ਵਾਪਸ SQLITE_IOERR_DELETE;
}
#ਜੇ_ਅੰਤ

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
ਸਥਿਰ ਪੂਰਨ memdbAccess(
  sqlite3_vfs *pVfs, 
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, 
  ਪੂਰਨ flags, 
  ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ
){
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  ਅਣਵਰਤਿਆ_PARAMETER(z_ਰਸਤਾ);
  ਅਣਵਰਤਿਆ_PARAMETER(flags);
  *p_Res_ਬਾਹਰਲਾ = 0;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ਸਥਿਰ ਪੂਰਨ memdb_ਭਰਿਆ_Pathname(
  sqlite3_vfs *pVfs, 
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, 
  ਪੂਰਨ n_ਬਾਹਰਲਾ, 
  ਅੱਖਰ *z_ਬਾਹਰਲਾ
){
  ਅਣਵਰਤਿਆ_PARAMETER(pVfs);
  sqlite3_snprintf(n_ਬਾਹਰਲਾ, z_ਬਾਹਰਲਾ, "%s", z_ਰਸਤਾ);
  ਵਾਪਸ SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ਸਥਿਰ ਖਾਲੀ *memdbDlOpen(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ){
  ਵਾਪਸ ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), z_ਰਸਤਾ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ਸਥਿਰ ਖਾਲੀ memdbDlError(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_ਸੁਨੇਹਾ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ਸਥਿਰ ਖਾਲੀ (*memdbDlSym(sqlite3_vfs *pVfs, ਖਾਲੀ *p, ਅਚਲ ਅੱਖਰ *zSym))(ਖਾਲੀ){
  ਵਾਪਸ ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ਸਥਿਰ ਖਾਲੀ memdbDlClose(sqlite3_vfs *pVfs, ਖਾਲੀ *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ਸਥਿਰ ਪੂਰਨ memdbRandomness(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Buf_ਬਾਹਰਲਾ){
  ਵਾਪਸ ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, z_Buf_ਬਾਹਰਲਾ);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ਸਥਿਰ ਪੂਰਨ memdbSleep(sqlite3_vfs *pVfs, ਪੂਰਨ nMicro){
  ਵਾਪਸ ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#ਜੇ 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ਸਥਿਰ ਪੂਰਨ memdb_Current_ਸਮਾਂ(sqlite3_vfs *pVfs, ਦੁੱਗਣਾ *p_ਸਮਾਂ_ਬਾਹਰਲਾ){
  ਵਾਪਸ ORIGVFS(pVfs)->x_Current_ਸਮਾਂ(ORIGVFS(pVfs), p_ਸਮਾਂ_ਬਾਹਰਲਾ);
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ memdb_Get_ਆਖਰੀ_Error(sqlite3_vfs *pVfs, ਪੂਰਨ a, ਅੱਖਰ *b){
  ਵਾਪਸ ORIGVFS(pVfs)->x_Get_ਆਖਰੀ_Error(ORIGVFS(pVfs), a, b);
}
ਸਥਿਰ ਪੂਰਨ memdb_Current_ਸਮਾਂ_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  ਵਾਪਸ ORIGVFS(pVfs)->x_Current_ਸਮਾਂ_Int64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
ਸਥਿਰ MemFile *memdbFromDbSchema(sqlite3 *db, ਅਚਲ ਅੱਖਰ *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  ਪੂਰਨ rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_ਸੂਚਕ, &p);
  ਜੇ( rc ) ਵਾਪਸ 0;
  ਜੇ( p->ਅਧਾਰ.pMethods!=&memdb_io_methods ) ਵਾਪਸ 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  ਜੇ( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  ਵਾਪਸ p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  ਅਚਲ ਅੱਖਰ *zSchema,      /* Which database within the connection */
  sqlite3_int64 *pi_ਮਾਪ,    /* Write size here, if not NULL */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  ਪੂਰਨ iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  ਪੂਰਨ sz_ਪੰਨਾ = 0;
  sqlite3_stmt *pStmt = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p_ਬਾਹਰਲਾ = 0;
  ਅੱਖਰ *zSql;
  ਪੂਰਨ rc;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    (ਖਾਲੀ)SQLITE_MISUSE_BKPT;
    ਵਾਪਸ 0;
  }
#ਜੇ_ਅੰਤ
  sqlite3_mutex_enter(db->mutex);

  ਜੇ( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3_ਲੱਭੋ_Db_Name(db, zSchema);
  ਜੇ( pi_ਮਾਪ ) *pi_ਮਾਪ = -1;
  ਜੇ( iDb<0 ) ਜਾਓ serialize_ਬਾਹਰਲਾ;
  ਜੇ( p ){
    MemStore *pStore = p->pStore;
    ਪ੍ਰਤਿਜ੍ਞਾ( pStore->pMutex==0 );
    ਜੇ( pi_ਮਾਪ ) *pi_ਮਾਪ = pStore->sz;
    ਜੇ( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_ਬਾਹਰਲਾ = pStore->a_ਅੰਕੜੇ;
    }ਨਹੀਂਤਾਂ{
      p_ਬਾਹਰਲਾ = sqlite3_malloc64( pStore->sz );
      ਜੇ( p_ਬਾਹਰਲਾ ) ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਬਾਹਰਲਾ, pStore->a_ਅੰਕੜੇ, pStore->sz);
    }
    ਜਾਓ serialize_ਬਾਹਰਲਾ;
  }
  pBt = db->aDb[iDb].pBt;
  ਜੇ( pBt==0 ) ਜਾਓ serialize_ਬਾਹਰਲਾ;
  sz_ਪੰਨਾ = sqlite3_Btree_Get_ਪੰਨਾ_ਮਾਪ(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  ਜੇ( rc ) ਜਾਓ serialize_ਬਾਹਰਲਾ;
  rc = sqlite3_ਕਦਮ(pStmt);
  ਜੇ( rc==SQLITE_ਪੰਕਤੀ ){
    sz = sqlite3_ਕਾਲਮ_int64(pStmt, 0)*sz_ਪੰਨਾ;
    ਜੇ( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_ਕਦਮ(pStmt);
      ਜੇ( rc==SQLITE_ਪੰਕਤੀ ){
        sz = sqlite3_ਕਾਲਮ_int64(pStmt, 0)*sz_ਪੰਨਾ;
      }
    }
    ਜੇ( pi_ਮਾਪ ) *pi_ਮਾਪ = sz;
    ਜੇ( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_ਬਾਹਰਲਾ = 0;
    }ਨਹੀਂਤਾਂ{
      p_ਬਾਹਰਲਾ = sqlite3_malloc64( sz );
      ਜੇ( p_ਬਾਹਰਲਾ ){
        ਪੂਰਨ n_ਪੰਨਾ = sqlite3_ਕਾਲਮ_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        ਪੂਰਨ pgno;
        ਲਈ(pgno=1; pgno<=n_ਪੰਨਾ; pgno++){
          Db_ਪੰਨਾ *p_ਪੰਨਾ = 0;
          ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p_ਮੰਜ਼ਿਲ = p_ਬਾਹਰਲਾ + sz_ਪੰਨਾ*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (Db_ਪੰਨਾ**)&p_ਪੰਨਾ, 0);
          ਜੇ( rc==SQLITE_OK ){
            ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਮੰਜ਼ਿਲ, sqlite3_Pager_Get_ਅੰਕੜੇ(p_ਪੰਨਾ), sz_ਪੰਨਾ);
          }ਨਹੀਂਤਾਂ ਜੇ( pgno==PENDING_BYTE/sz_ਪੰਨਾ+1 ){
            memset(p_ਮੰਜ਼ਿਲ, 0, sz_ਪੰਨਾ);
          }ਨਹੀਂਤਾਂ{
            sqlite3_free(p_ਬਾਹਰਲਾ);
            p_ਬਾਹਰਲਾ = 0;
            n_ਪੰਨਾ = 0;            
          }
          sqlite3PagerUnref(p_ਪੰਨਾ);       
        }
      }
    }
  }
  sqlite3_finalize(pStmt);

 serialize_ਬਾਹਰਲਾ:
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ p_ਬਾਹਰਲਾ;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
ਪੂਰਨ sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  ਅਚਲ ਅੱਖਰ *zSchema,    /* Which DB to reopen with the deserialization */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p_ਅੰਕੜੇ,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  ਬਿਨਾਨਿਸ਼ਾਨੀ mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  ਅੱਖਰ *zSql;
  sqlite3_stmt *pStmt = 0;
  ਪੂਰਨ rc;
  ਪੂਰਨ iDb;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_API_ARMOR
  ਜੇ( !sqlite3SafetyCheckOk(db) ){
    ਵਾਪਸ SQLITE_MISUSE_BKPT;
  }
  ਜੇ( szDb<0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
  ਜੇ( szBuf<0 ) ਵਾਪਸ SQLITE_MISUSE_BKPT;
#ਜੇ_ਅੰਤ

  sqlite3_mutex_enter(db->mutex);
  ਜੇ( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3_ਲੱਭੋ_Db_Name(db, zSchema);
  testcase( iDb==1 );
  ਜੇ( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    ਜਾਓ ਸਿਰਾ_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  ਜੇ( zSql==0 ){
    rc = SQLITE_NOMEM;
  }ਨਹੀਂਤਾਂ{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  ਜੇ( rc ) ਜਾਓ ਸਿਰਾ_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_ਕਦਮ(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  ਜੇ( rc!=SQLITE_OK ){
    ਜਾਓ ਸਿਰਾ_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  ਜੇ( p==0 ){
    rc = SQLITE_ERROR;
  }ਨਹੀਂਤਾਂ{
    MemStore *pStore = p->pStore;
    pStore->a_ਅੰਕੜੇ = p_ਅੰਕੜੇ;
    p_ਅੰਕੜੇ = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    ਜੇ( pStore->szMax<sqlite3GlobalConfig.mx_Memdb_ਮਾਪ ){
      pStore->szMax = sqlite3GlobalConfig.mx_Memdb_ਮਾਪ;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

ਸਿਰਾ_deserialize:
  ਜੇ( p_ਅੰਕੜੇ && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(p_ਅੰਕੜੇ);
  }
  sqlite3_mutex_leave(db->mutex);
  ਵਾਪਸ rc;
}

/*
** Return true if the VFS is the memvfs.
*/
ਪੂਰਨ sqlite3IsMemdb(ਅਚਲ sqlite3_vfs *pVfs){
  ਵਾਪਸ pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
ਪੂਰਨ sqlite3MemdbInit(ਖਾਲੀ){
  sqlite3_vfs *p_ਹੇਠਲਾ = sqlite3_vfs_ਲੱਭੋ(0);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ sz;
  ਜੇ( NEVER(p_ਹੇਠਲਾ==0) ) ਵਾਪਸ SQLITE_ERROR;
  sz = p_ਹੇਠਲਾ->szOsFile;
  memdb_vfs.p_App_ਅੰਕੜੇ = p_ਹੇਠਲਾ;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  ਜੇ( sz<ਆਕਾਰ(MemFile) ) sz = ਆਕਾਰ(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  ਵਾਪਸ sqlite3_vfs_ਰਜਿਸਟਰ(&memdb_vfs, 0);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_DESERIALIZE */
