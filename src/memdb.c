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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ sqlite3_vfs MemVfs;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ MemFile MemFile;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ MemStore MemStore;

/* Access to a lower-level VFS that (might) implement dynamic loading,
** access to randomness, etc.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ORIGVFS(p) ((sqlite3_vfs*)((p)->p_App_ದತ್ತಾಂಶ))

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
ರಚನೆ MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *a_ದತ್ತಾಂಶ;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  ಪೂರ್ಣಾಂಕ nMmap;                      /* Number of memory mapped pages */
  ಚಿಹ್ನೆರಹಿತ mFlags;                /* Flags */
  ಪೂರ್ಣಾಂಕ n_Rd_ಬೀಗ;                    /* Number of readers */
  ಪೂರ್ಣಾಂಕ n_Wr_ಬೀಗ;                    /* Number of writers.  (Always 0 or 1) */
  ಪೂರ್ಣಾಂಕ nRef;                       /* Number of users of this MemStore */
  ಅಕ್ಷರ *zFName;                   /* The filename for shared stores */
};

/* An open file */
ರಚನೆ MemFile {
  sqlite3_file ತಳ;              /* IO methods */
  MemStore *pStore;               /* The storage */
  ಪೂರ್ಣಾಂಕ e_ಬೀಗ;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
ಸ್ಥಿರ ರಚನೆ MemFS {
  ಪೂರ್ಣಾಂಕ nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbRead(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbWrite(sqlite3_file*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbTruncate(sqlite3_file*, sqlite3_int64 ಗಾತ್ರ_2);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbSync(sqlite3_file*, ಪೂರ್ಣಾಂಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdb_File_ಗಾತ್ರ(sqlite3_file*, sqlite3_int64 *p_ಗಾತ್ರ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdb_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbUnlock(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbFileControl(sqlite3_file*, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbDeviceCharacteristics(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, ಪೂರ್ಣಾಂಕ iAmt, ಶೂನ್ಯ **pp);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, ಶೂನ್ಯ *p);

/*
** Methods for MemVfs
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, sqlite3_file*, ಪೂರ್ಣಾಂಕ , ಪೂರ್ಣಾಂಕ *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbAccess(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ flags, ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdb_ತುಂಬಿದ_Pathname(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಶೂನ್ಯ *memdbDlOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename);
ಸ್ಥಿರ ಶೂನ್ಯ memdbDlError(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ);
ಸ್ಥಿರ ಶೂನ್ಯ (*memdbDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*zSym))(ಶೂನ್ಯ);
ಸ್ಥಿರ ಶೂನ್ಯ memdbDlClose(sqlite3_vfs*, ಶೂನ್ಯ*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbRandomness(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbSleep(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdb_Get_ಕೊನೆಯ_Error(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdb_Current_ಸಮಯ_Int64(sqlite3_vfs*, sqlite3_int64*);

ಸ್ಥಿರ sqlite3_vfs memdb_vfs = {
  2,                           /* iVersion */
  0,                           /* szOsFile (set when registered) */
  1024,                        /* mxPathname */
  0,                           /* pNext */
  "memdb",                     /* zName */
  0,                           /* pAppData (set when registered) */ 
  memdbOpen,                   /* xOpen */
  0, /* memdbDelete, */        /* xDelete */
  memdbAccess,                 /* xAccess */
  memdb_ತುಂಬಿದ_Pathname,           /* xFullPathname */
  memdbDlOpen,                 /* xDlOpen */
  memdbDlError,                /* xDlError */
  memdbDlSym,                  /* xDlSym */
  memdbDlClose,                /* xDlClose */
  memdbRandomness,             /* xRandomness */
  memdbSleep,                  /* xSleep */
  0, /* memdbCurrentTime, */   /* xCurrentTime */
  memdb_Get_ಕೊನೆಯ_Error,           /* xGetLastError */
  memdb_Current_ಸಮಯ_Int64,       /* xCurrentTimeInt64 */
  0,                           /* xSetSystemCall */
  0,                           /* xGetSystemCall */
  0,                           /* xNextSystemCall */
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ sqlite3_io_methods memdb_io_methods = {
  3,                              /* iVersion */
  memdbClose,                      /* xClose */
  memdbRead,                       /* xRead */
  memdbWrite,                      /* xWrite */
  memdbTruncate,                   /* xTruncate */
  memdbSync,                       /* xSync */
  memdb_File_ಗಾತ್ರ,                   /* xFileSize */
  memdb_ಬೀಗ,                       /* xLock */
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
#ಆದರೆ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
ಸ್ಥಿರ ಶೂನ್ಯ memdbEnter(MemStore *p){
  ಬಳಸದ_PARAMETER(p);
}
ಸ್ಥಿರ ಶೂನ್ಯ memdbLeave(MemStore *p){
  ಬಳಸದ_PARAMETER(p);
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
ಸ್ಥಿರ ಶೂನ್ಯ memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#ಆದರೆ_ಅಂತ್ಯ



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ಆದರೆ( p->zFName ){
    ಪೂರ್ಣಾಂಕ i;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_mutex_enter(pVfsMutex);
    ಗಾಗಿ(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      ಆದರೆ( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        ಆದರೆ( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          ಆದರೆ( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        ನಿಲ್ಲಿಸು;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }ಇಲ್ಲದಿದ್ದರೆ{
    memdbEnter(p);
  }
  p->nRef--;
  ಆದರೆ( p->nRef<=0 ){
    ಆದರೆ( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->a_ದತ್ತಾಂಶ);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }ಇಲ್ಲದಿದ್ದರೆ{
    memdbLeave(p);
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ಆದರೆ( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    ಆದರೆ( iOfst<p->sz ) ಸ್ಮರಣೆ_ನಕಲು(zBuf, p->a_ದತ್ತಾಂಶ+iOfst, p->sz - iOfst);
    memdbLeave(p);
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_SHORT_READ;
  }
  ಸ್ಮರಣೆ_ನಕಲು(zBuf, p->a_ದತ್ತಾಂಶ+iOfst, iAmt);
  memdbLeave(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *pNew;
  ಆದರೆ( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    ಹಿಂತಿರುಗಿ SQLITE_ತುಂಬಿದ;
  }
  ಆದರೆ( newSz>p->szMax ){
    ಹಿಂತಿರುಗಿ SQLITE_ತುಂಬಿದ;
  }
  newSz *= 2;
  ಆದರೆ( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->a_ದತ್ತಾಂಶ, newSz);
  ಆದರೆ( pNew==0 ) ಹಿಂತಿರುಗಿ SQLITE_IOERR_NOMEM;
  p->a_ದತ್ತಾಂಶ = pNew;
  p->szAlloc = newSz;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbWrite(
  sqlite3_file *pFile,
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *z,
  ಪೂರ್ಣಾಂಕ iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ಆದರೆ( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_WRITE;
  }
  ಆದರೆ( iOfst+iAmt>p->sz ){
    ಪೂರ್ಣಾಂಕ rc;
    ಆದರೆ( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      ಹಿಂತಿರುಗಿ rc;
    }
    ಆದರೆ( iOfst>p->sz ) memset(p->a_ದತ್ತಾಂಶ+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  ಸ್ಮರಣೆ_ನಕಲು(p->a_ದತ್ತಾಂಶ+iOfst, z, iAmt);
  memdbLeave(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbTruncate(sqlite3_file *pFile, sqlite_int64 ಗಾತ್ರ_2){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  memdbEnter(p);
  ಆದರೆ( ಗಾತ್ರ_2>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->sz = ಗಾತ್ರ_2; 
  }
  memdbLeave(p);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Sync an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbSync(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ flags){
  ಬಳಸದ_PARAMETER(pFile);
  ಬಳಸದ_PARAMETER(flags);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdb_File_ಗಾತ್ರ(sqlite3_file *pFile, sqlite_int64 *p_ಗಾತ್ರ){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *p_ಗಾತ್ರ = p->sz;
  memdbLeave(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdb_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಆದರೆ( e_ಬೀಗ<=pThis->e_ಬೀಗ ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  memdbEnter(p);

  ಪ್ರತಿಜ್ಞಾ( p->n_Wr_ಬೀಗ==0 || p->n_Wr_ಬೀಗ==1 );
  ಪ್ರತಿಜ್ಞಾ( pThis->e_ಬೀಗ<=SQLITE_ಬೀಗ_SHARED || p->n_Wr_ಬೀಗ==1 );
  ಪ್ರತಿಜ್ಞಾ( pThis->e_ಬೀಗ==SQLITE_ಬೀಗ_NONE || p->n_Rd_ಬೀಗ>=1 );

  ಆದರೆ( e_ಬೀಗ>SQLITE_ಬೀಗ_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆಯ್ಕೆ( e_ಬೀಗ ){
      ಸ್ಥಿತಿ SQLITE_ಬೀಗ_SHARED: {
        ಪ್ರತಿಜ್ಞಾ( pThis->e_ಬೀಗ==SQLITE_ಬೀಗ_NONE );
        ಆದರೆ( p->n_Wr_ಬೀಗ>0 ){
          rc = SQLITE_BUSY;
        }ಇಲ್ಲದಿದ್ದರೆ{
          p->n_Rd_ಬೀಗ++;
        }
        ನಿಲ್ಲಿಸು;
      };
  
      ಸ್ಥಿತಿ SQLITE_ಬೀಗ_RESERVED:
      ಸ್ಥಿತಿ SQLITE_ಬೀಗ_PENDING: {
        ಪ್ರತಿಜ್ಞಾ( pThis->e_ಬೀಗ>=SQLITE_ಬೀಗ_SHARED );
        ಆದರೆ( ALWAYS(pThis->e_ಬೀಗ==SQLITE_ಬೀಗ_SHARED) ){
          ಆದರೆ( p->n_Wr_ಬೀಗ>0 ){
            rc = SQLITE_BUSY;
          }ಇಲ್ಲದಿದ್ದರೆ{
            p->n_Wr_ಬೀಗ = 1;
          }
        }
        ನಿಲ್ಲಿಸು;
      }
  
      ಪೂರ್ವನಿಯೋಜಿತ: {
        ಪ್ರತಿಜ್ಞಾ(  e_ಬೀಗ==SQLITE_ಬೀಗ_EXCLUSIVE );
        ಪ್ರತಿಜ್ಞಾ( pThis->e_ಬೀಗ>=SQLITE_ಬೀಗ_SHARED );
        ಆದರೆ( p->n_Rd_ಬೀಗ>1 ){
          rc = SQLITE_BUSY;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pThis->e_ಬೀಗ==SQLITE_ಬೀಗ_SHARED ){
          p->n_Wr_ಬೀಗ = 1;
        }
        ನಿಲ್ಲಿಸು;
      }
    }
  }
  ಆದರೆ( rc==SQLITE_OK ) pThis->e_ಬೀಗ = e_ಬೀಗ;
  memdbLeave(p);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Unlock an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  ಆದರೆ( e_ಬೀಗ>=pThis->e_ಬೀಗ ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  memdbEnter(p);

  ಪ್ರತಿಜ್ಞಾ( e_ಬೀಗ==SQLITE_ಬೀಗ_SHARED || e_ಬೀಗ==SQLITE_ಬೀಗ_NONE );
  ಆದರೆ( e_ಬೀಗ==SQLITE_ಬೀಗ_SHARED ){
    ಆದರೆ( ALWAYS(pThis->e_ಬೀಗ>SQLITE_ಬೀಗ_SHARED) ){
      p->n_Wr_ಬೀಗ--;
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( pThis->e_ಬೀಗ>SQLITE_ಬೀಗ_SHARED ){
      p->n_Wr_ಬೀಗ--;
    }
    p->n_Rd_ಬೀಗ--;
  }

  pThis->e_ಬೀಗ = e_ಬೀಗ;
  memdbLeave(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

#ಆದರೆ 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdb_Check_Reserved_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  *p_Res_ಹೊರಹೋಗು = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** File control method. For custom operations on an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  ಆದರೆ( op==SQLITE_FCNTL_VFSNAME ){
    *(ಅಕ್ಷರ**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->a_ದತ್ತಾಂಶ, p->sz);
    rc = SQLITE_OK;
  }
  ಆದರೆ( op==SQLITE_FCNTL_ಗಾತ್ರ_ಎಲ್ಲೆ ){
    sqlite3_int64 i_ಎಲ್ಲೆ = *(sqlite3_int64*)pArg;
    ಆದರೆ( i_ಎಲ್ಲೆ<p->sz ){
      ಆದರೆ( i_ಎಲ್ಲೆ<0 ){
        i_ಎಲ್ಲೆ = p->szMax;
      }ಇಲ್ಲದಿದ್ದರೆ{
        i_ಎಲ್ಲೆ = p->sz;
      }
    }
    p->szMax = i_ಎಲ್ಲೆ;
    *(sqlite3_int64*)pArg = i_ಎಲ್ಲೆ;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  ಹಿಂತಿರುಗಿ rc;
}

#ಆದರೆ 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdb_Sector_ಗಾತ್ರ(sqlite3_file *pFile){
  ಹಿಂತಿರುಗಿ 1024;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Return the device characteristic flags supported by an memdb-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbDeviceCharacteristics(sqlite3_file *pFile){
  ಬಳಸದ_PARAMETER(pFile);
  ಹಿಂತಿರುಗಿ SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_ಸುರಕ್ಷಿತ_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  ಪೂರ್ಣಾಂಕ iAmt,
  ಶೂನ್ಯ **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ಆದರೆ( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->nMmap++;
    *pp = (ಶೂನ್ಯ*)(p->a_ದತ್ತಾಂಶ + iOfst);
  }
  memdbLeave(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/* Release a memory-mapped page */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, ಶೂನ್ಯ *p_ಪುಟ){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ಬಳಸದ_PARAMETER(iOfst);
  ಬಳಸದ_PARAMETER(p_ಪುಟ);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Open an mem file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbOpen(
  sqlite3_vfs *pVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  sqlite3_file *pFd,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  ಪೂರ್ಣಾಂಕ szName;
  ಬಳಸದ_PARAMETER(pVfs);

  memset(pFile, 0, ಗಾತ್ರ(*pFile));
  szName = sqlite3Strlen30(zName);
  ಆದರೆ( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    ಪೂರ್ಣಾಂಕ i;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_mutex_enter(pVfsMutex);
    ಗಾಗಿ(i=0; i<memdb_g.nMemStore; i++){
      ಆದರೆ( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        ನಿಲ್ಲಿಸು;
      }
    }
    ಆದರೆ( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( ಗಾತ್ರ(*p) + (i64)szName + 3 );
      ಆದರೆ( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             ಗಾತ್ರ(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      ಆದರೆ( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, ಗಾತ್ರ(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mx_Memdb_ಗಾತ್ರ;
      p->zFName = (ಅಕ್ಷರ*)&p[1];
      ಸ್ಮರಣೆ_ನಕಲು(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      ಆದರೆ( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }ಇಲ್ಲದಿದ್ದರೆ{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }ಇಲ್ಲದಿದ್ದರೆ{
    p = sqlite3Malloc( ಗಾತ್ರ(*p) );
    ಆದರೆ( p==0 ){
      ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
    }
    memset(p, 0, ಗಾತ್ರ(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mx_Memdb_ಗಾತ್ರ;
  }
  pFile->pStore = p;
  ಆದರೆ( p_ಹೊರಹೋಗು_Flags!=0 ){
    *p_ಹೊರಹೋಗು_Flags = flags | SQLITE_OPEN_ಸ್ಮರಣೆ;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

#ಆದರೆ 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbDelete(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ dirSync){
  ಹಿಂತಿರುಗಿ SQLITE_IOERR_DELETE;
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbAccess(
  sqlite3_vfs *pVfs, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, 
  ಪೂರ್ಣಾಂಕ flags, 
  ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು
){
  ಬಳಸದ_PARAMETER(pVfs);
  ಬಳಸದ_PARAMETER(z_ಮಾರ್ಗ);
  ಬಳಸದ_PARAMETER(flags);
  *p_Res_ಹೊರಹೋಗು = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdb_ತುಂಬಿದ_Pathname(
  sqlite3_vfs *pVfs, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, 
  ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು, 
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು
){
  ಬಳಸದ_PARAMETER(pVfs);
  sqlite3_snprintf(n_ಹೊರಹೋಗು, z_ಹೊರಹೋಗು, "%s", z_ಮಾರ್ಗ);
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *memdbDlOpen(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ){
  ಹಿಂತಿರುಗಿ ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), z_ಮಾರ್ಗ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memdbDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_ಸಂದೇಶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ (*memdbDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSym))(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ memdbDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Buf_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, z_Buf_ಹೊರಹೋಗು);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdbSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nMicro){
  ಹಿಂತಿರುಗಿ ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#ಆದರೆ 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdb_Current_ಸಮಯ(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *p_ಸಮಯ_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ ORIGVFS(pVfs)->x_Current_ಸಮಯ(ORIGVFS(pVfs), p_ಸಮಯ_ಹೊರಹೋಗು);
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdb_Get_ಕೊನೆಯ_Error(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ a, ಅಕ್ಷರ *b){
  ಹಿಂತಿರುಗಿ ORIGVFS(pVfs)->x_Get_ಕೊನೆಯ_Error(ORIGVFS(pVfs), a, b);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ memdb_Current_ಸಮಯ_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  ಹಿಂತಿರುಗಿ ORIGVFS(pVfs)->x_Current_ಸಮಯ_Int64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
ಸ್ಥಿರ MemFile *memdbFromDbSchema(sqlite3 *db, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  ಪೂರ್ಣಾಂಕ rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_ಸೂಚಕ, &p);
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( p->ತಳ.pMethods!=&memdb_io_methods ) ಹಿಂತಿರುಗಿ 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  ಆದರೆ( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  ಹಿಂತಿರುಗಿ p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSchema,      /* Which database within the connection */
  sqlite3_int64 *pi_ಗಾತ್ರ,    /* Write size here, if not NULL */
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  ಪೂರ್ಣಾಂಕ iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  ಪೂರ್ಣಾಂಕ sz_ಪುಟ = 0;
  sqlite3_stmt *pStmt = 0;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p_ಹೊರಹೋಗು = 0;
  ಅಕ್ಷರ *zSql;
  ಪೂರ್ಣಾಂಕ rc;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    (ಶೂನ್ಯ)SQLITE_MISUSE_BKPT;
    ಹಿಂತಿರುಗಿ 0;
  }
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_mutex_enter(db->mutex);

  ಆದರೆ( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, zSchema);
  ಆದರೆ( pi_ಗಾತ್ರ ) *pi_ಗಾತ್ರ = -1;
  ಆದರೆ( iDb<0 ) ಹೋಗು serialize_ಹೊರಹೋಗು;
  ಆದರೆ( p ){
    MemStore *pStore = p->pStore;
    ಪ್ರತಿಜ್ಞಾ( pStore->pMutex==0 );
    ಆದರೆ( pi_ಗಾತ್ರ ) *pi_ಗಾತ್ರ = pStore->sz;
    ಆದರೆ( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_ಹೊರಹೋಗು = pStore->a_ದತ್ತಾಂಶ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಹೊರಹೋಗು = sqlite3_malloc64( pStore->sz );
      ಆದರೆ( p_ಹೊರಹೋಗು ) ಸ್ಮರಣೆ_ನಕಲು(p_ಹೊರಹೋಗು, pStore->a_ದತ್ತಾಂಶ, pStore->sz);
    }
    ಹೋಗು serialize_ಹೊರಹೋಗು;
  }
  pBt = db->aDb[iDb].pBt;
  ಆದರೆ( pBt==0 ) ಹೋಗು serialize_ಹೊರಹೋಗು;
  sz_ಪುಟ = sqlite3_Btree_Get_ಪುಟ_ಗಾತ್ರ(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  ಆದರೆ( rc ) ಹೋಗು serialize_ಹೊರಹೋಗು;
  rc = sqlite3_ಹೆಜ್ಜೆ(pStmt);
  ಆದರೆ( rc==SQLITE_ಅಡ್ಡಸಾಲು ){
    sz = sqlite3_ಸ್ತಂಭ_int64(pStmt, 0)*sz_ಪುಟ;
    ಆದರೆ( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_ಹೆಜ್ಜೆ(pStmt);
      ಆದರೆ( rc==SQLITE_ಅಡ್ಡಸಾಲು ){
        sz = sqlite3_ಸ್ತಂಭ_int64(pStmt, 0)*sz_ಪುಟ;
      }
    }
    ಆದರೆ( pi_ಗಾತ್ರ ) *pi_ಗಾತ್ರ = sz;
    ಆದರೆ( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_ಹೊರಹೋಗು = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಹೊರಹೋಗು = sqlite3_malloc64( sz );
      ಆದರೆ( p_ಹೊರಹೋಗು ){
        ಪೂರ್ಣಾಂಕ n_ಪುಟ = sqlite3_ಸ್ತಂಭ_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        ಪೂರ್ಣಾಂಕ pgno;
        ಗಾಗಿ(pgno=1; pgno<=n_ಪುಟ; pgno++){
          Db_ಪುಟ *p_ಪುಟ = 0;
          ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p_ಗಮ್ಯ = p_ಹೊರಹೋಗು + sz_ಪುಟ*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (Db_ಪುಟ**)&p_ಪುಟ, 0);
          ಆದರೆ( rc==SQLITE_OK ){
            ಸ್ಮರಣೆ_ನಕಲು(p_ಗಮ್ಯ, sqlite3_Pager_Get_ದತ್ತಾಂಶ(p_ಪುಟ), sz_ಪುಟ);
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pgno==PENDING_BYTE/sz_ಪುಟ+1 ){
            memset(p_ಗಮ್ಯ, 0, sz_ಪುಟ);
          }ಇಲ್ಲದಿದ್ದರೆ{
            sqlite3_free(p_ಹೊರಹೋಗು);
            p_ಹೊರಹೋಗು = 0;
            n_ಪುಟ = 0;            
          }
          sqlite3PagerUnref(p_ಪುಟ);       
        }
      }
    }
  }
  sqlite3_finalize(pStmt);

 serialize_ಹೊರಹೋಗು:
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ p_ಹೊರಹೋಗು;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
ಪೂರ್ಣಾಂಕ sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSchema,    /* Which DB to reopen with the deserialization */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p_ದತ್ತಾಂಶ,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  ಚಿಹ್ನೆರಹಿತ mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  ಅಕ್ಷರ *zSql;
  sqlite3_stmt *pStmt = 0;
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ iDb;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_API_ARMOR
  ಆದರೆ( !sqlite3SafetyCheckOk(db) ){
    ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  }
  ಆದರೆ( szDb<0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
  ಆದರೆ( szBuf<0 ) ಹಿಂತಿರುಗಿ SQLITE_MISUSE_BKPT;
#ಆದರೆ_ಅಂತ್ಯ

  sqlite3_mutex_enter(db->mutex);
  ಆದರೆ( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3_ಕಂಡುಹಿಡಿ_Db_Name(db, zSchema);
  testcase( iDb==1 );
  ಆದರೆ( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    ಹೋಗು ಕೊನೆ_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  ಆದರೆ( zSql==0 ){
    rc = SQLITE_NOMEM;
  }ಇಲ್ಲದಿದ್ದರೆ{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  ಆದರೆ( rc ) ಹೋಗು ಕೊನೆ_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_ಹೆಜ್ಜೆ(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  ಆದರೆ( rc!=SQLITE_OK ){
    ಹೋಗು ಕೊನೆ_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  ಆದರೆ( p==0 ){
    rc = SQLITE_ERROR;
  }ಇಲ್ಲದಿದ್ದರೆ{
    MemStore *pStore = p->pStore;
    pStore->a_ದತ್ತಾಂಶ = p_ದತ್ತಾಂಶ;
    p_ದತ್ತಾಂಶ = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    ಆದರೆ( pStore->szMax<sqlite3GlobalConfig.mx_Memdb_ಗಾತ್ರ ){
      pStore->szMax = sqlite3GlobalConfig.mx_Memdb_ಗಾತ್ರ;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

ಕೊನೆ_deserialize:
  ಆದರೆ( p_ದತ್ತಾಂಶ && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(p_ದತ್ತಾಂಶ);
  }
  sqlite3_mutex_leave(db->mutex);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return true if the VFS is the memvfs.
*/
ಪೂರ್ಣಾಂಕ sqlite3IsMemdb(ಸ್ಥಿರಾಂಕ sqlite3_vfs *pVfs){
  ಹಿಂತಿರುಗಿ pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
ಪೂರ್ಣಾಂಕ sqlite3MemdbInit(ಶೂನ್ಯ){
  sqlite3_vfs *p_ಅಧೋ = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ sz;
  ಆದರೆ( NEVER(p_ಅಧೋ==0) ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  sz = p_ಅಧೋ->szOsFile;
  memdb_vfs.p_App_ದತ್ತಾಂಶ = p_ಅಧೋ;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  ಆದರೆ( sz<ಗಾತ್ರ(MemFile) ) sz = ಗಾತ್ರ(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  ಹಿಂತಿರುಗಿ sqlite3_vfs_ರಿಜಿಸ್ಟರ್(&memdb_vfs, 0);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_DESERIALIZE */
