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
প্রকারলেখ কাঠামো sqlite3_vfs MemVfs;
প্রকারলেখ কাঠামো MemFile MemFile;
প্রকারলেখ কাঠামো MemStore MemStore;

/* Access to a lower-level VFS that (might) implement dynamic loading,
** access to randomness, etc.
*/
#define ORIGVFS(p) ((sqlite3_vfs*)((p)->p_App_তথ্য))

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
কাঠামো MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  unsigned অক্ষর *a_তথ্য;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  পূর্ণসংখ্যা nMmap;                      /* Number of memory mapped pages */
  unsigned mFlags;                /* Flags */
  পূর্ণসংখ্যা nRdLock;                    /* Number of readers */
  পূর্ণসংখ্যা nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  পূর্ণসংখ্যা nRef;                       /* Number of users of this MemStore */
  অক্ষর *zFName;                   /* The filename for shared stores */
};

/* An open file */
কাঠামো MemFile {
  sqlite3_file base;              /* IO methods */
  MemStore *pStore;               /* The storage */
  পূর্ণসংখ্যা eLock;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
স্থির কাঠামো MemFS {
  পূর্ণসংখ্যা nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
স্থির পূর্ণসংখ্যা memdbClose(sqlite3_file*);
স্থির পূর্ণসংখ্যা memdbRead(sqlite3_file*, শূন্য*, পূর্ণসংখ্যা iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণসংখ্যা memdbWrite(sqlite3_file*,ধ্রুবক শূন্য*,পূর্ণসংখ্যা iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণসংখ্যা memdbTruncate(sqlite3_file*, sqlite3_int64 আকার_2);
স্থির পূর্ণসংখ্যা memdbSync(sqlite3_file*, পূর্ণসংখ্যা flags);
স্থির পূর্ণসংখ্যা memdb_File_আকার(sqlite3_file*, sqlite3_int64 *p_আকার);
স্থির পূর্ণসংখ্যা memdbLock(sqlite3_file*, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা memdbUnlock(sqlite3_file*, পূর্ণসংখ্যা);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
স্থির পূর্ণসংখ্যা memdbFileControl(sqlite3_file*, পূর্ণসংখ্যা op, শূন্য *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
স্থির পূর্ণসংখ্যা memdbDeviceCharacteristics(sqlite3_file*);
স্থির পূর্ণসংখ্যা memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, পূর্ণসংখ্যা iAmt, শূন্য **pp);
স্থির পূর্ণসংখ্যা memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, শূন্য *p);

/*
** Methods for MemVfs
*/
স্থির পূর্ণসংখ্যা memdbOpen(sqlite3_vfs*, ধ্রুবক অক্ষর *, sqlite3_file*, পূর্ণসংখ্যা , পূর্ণসংখ্যা *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
স্থির পূর্ণসংখ্যা memdbAccess(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা flags, পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা memdbFullPathname(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা, অক্ষর *zOut);
স্থির শূন্য *memdbDlOpen(sqlite3_vfs*, ধ্রুবক অক্ষর *zFilename);
স্থির শূন্য memdbDlError(sqlite3_vfs*, পূর্ণসংখ্যা nByte, অক্ষর *z_Err_বার্তা);
স্থির শূন্য (*memdbDlSym(sqlite3_vfs *pVfs, শূন্য *p, ধ্রুবক অক্ষর*zSym))(শূন্য);
স্থির শূন্য memdbDlClose(sqlite3_vfs*, শূন্য*);
স্থির পূর্ণসংখ্যা memdbRandomness(sqlite3_vfs*, পূর্ণসংখ্যা nByte, অক্ষর *zOut);
স্থির পূর্ণসংখ্যা memdbSleep(sqlite3_vfs*, পূর্ণসংখ্যা microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
স্থির পূর্ণসংখ্যা memdbGetLastError(sqlite3_vfs*, পূর্ণসংখ্যা, অক্ষর *);
স্থির পূর্ণসংখ্যা memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

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

স্থির ধ্রুবক sqlite3_io_methods memdb_io_methods = {
  3,                              /* iVersion */
  memdbClose,                      /* xClose */
  memdbRead,                       /* xRead */
  memdbWrite,                      /* xWrite */
  memdbTruncate,                   /* xTruncate */
  memdbSync,                       /* xSync */
  memdb_File_আকার,                   /* xFileSize */
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
স্থির পূর্ণসংখ্যা memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  যদি( p->zFName ){
    পূর্ণসংখ্যা i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    জন্য(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      যদি( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        যদি( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          যদি( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        বিরতি;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }অন্যথা{
    memdbEnter(p);
  }
  p->nRef--;
  যদি( p->nRef<=0 ){
    যদি( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->a_তথ্য);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }অন্যথা{
    memdbLeave(p);
  }
  ফেরত SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
স্থির পূর্ণসংখ্যা memdbRead(
  sqlite3_file *pFile, 
  শূন্য *zBuf, 
  পূর্ণসংখ্যা iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  যদি( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    যদি( iOfst<p->sz ) memcpy(zBuf, p->a_তথ্য+iOfst, p->sz - iOfst);
    memdbLeave(p);
    ফেরত SQLITE_IOERR_SHORT_READ;
  }
  memcpy(zBuf, p->a_তথ্য+iOfst, iAmt);
  memdbLeave(p);
  ফেরত SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
স্থির পূর্ণসংখ্যা memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  unsigned অক্ষর *pNew;
  যদি( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    ফেরত SQLITE_FULL;
  }
  যদি( newSz>p->szMax ){
    ফেরত SQLITE_FULL;
  }
  newSz *= 2;
  যদি( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->a_তথ্য, newSz);
  যদি( pNew==0 ) ফেরত SQLITE_IOERR_NOMEM;
  p->a_তথ্য = pNew;
  p->szAlloc = newSz;
  ফেরত SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
স্থির পূর্ণসংখ্যা memdbWrite(
  sqlite3_file *pFile,
  ধ্রুবক শূন্য *z,
  পূর্ণসংখ্যা iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  যদি( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    ফেরত SQLITE_IOERR_WRITE;
  }
  যদি( iOfst+iAmt>p->sz ){
    পূর্ণসংখ্যা rc;
    যদি( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      ফেরত rc;
    }
    যদি( iOfst>p->sz ) memset(p->a_তথ্য+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  memcpy(p->a_তথ্য+iOfst, z, iAmt);
  memdbLeave(p);
  ফেরত SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
স্থির পূর্ণসংখ্যা memdbTruncate(sqlite3_file *pFile, sqlite_int64 আকার_2){
  MemStore *p = ((MemFile*)pFile)->pStore;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  memdbEnter(p);
  যদি( আকার_2>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }অন্যথা{
    p->sz = আকার_2; 
  }
  memdbLeave(p);
  ফেরত rc;
}

/*
** Sync an memdb-file.
*/
স্থির পূর্ণসংখ্যা memdbSync(sqlite3_file *pFile, পূর্ণসংখ্যা flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  ফেরত SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
স্থির পূর্ণসংখ্যা memdb_File_আকার(sqlite3_file *pFile, sqlite_int64 *p_আকার){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *p_আকার = p->sz;
  memdbLeave(p);
  ফেরত SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
স্থির পূর্ণসংখ্যা memdbLock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  যদি( eLock<=pThis->eLock ) ফেরত SQLITE_OK;
  memdbEnter(p);

  প্রতিজ্ঞা( p->nWrLock==0 || p->nWrLock==1 );
  প্রতিজ্ঞা( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  প্রতিজ্ঞা( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  যদি( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }অন্যথা{
    নির্বাচন( eLock ){
      ক্ষেত্র SQLITE_LOCK_SHARED: {
        প্রতিজ্ঞা( pThis->eLock==SQLITE_LOCK_NONE );
        যদি( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }অন্যথা{
          p->nRdLock++;
        }
        বিরতি;
      };
  
      ক্ষেত্র SQLITE_LOCK_RESERVED:
      ক্ষেত্র SQLITE_LOCK_PENDING: {
        প্রতিজ্ঞা( pThis->eLock>=SQLITE_LOCK_SHARED );
        যদি( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          যদি( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }অন্যথা{
            p->nWrLock = 1;
          }
        }
        বিরতি;
      }
  
      পূর্বনির্ধারিত: {
        প্রতিজ্ঞা(  eLock==SQLITE_LOCK_EXCLUSIVE );
        প্রতিজ্ঞা( pThis->eLock>=SQLITE_LOCK_SHARED );
        যদি( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }অন্যথা যদি( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        বিরতি;
      }
    }
  }
  যদি( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  ফেরত rc;
}

/*
** Unlock an memdb-file.
*/
স্থির পূর্ণসংখ্যা memdbUnlock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  যদি( eLock>=pThis->eLock ) ফেরত SQLITE_OK;
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
  ফেরত SQLITE_OK;
}

#যদি 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
স্থির পূর্ণসংখ্যা memdbCheckReservedLock(sqlite3_file *pFile, পূর্ণসংখ্যা *pResOut){
  *pResOut = 0;
  ফেরত SQLITE_OK;
}
#endif


/*
** File control method. For custom operations on an memdb-file.
*/
স্থির পূর্ণসংখ্যা memdbFileControl(sqlite3_file *pFile, পূর্ণসংখ্যা op, শূন্য *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  পূর্ণসংখ্যা rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  যদি( op==SQLITE_FCNTL_VFSNAME ){
    *(অক্ষর**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->a_তথ্য, p->sz);
    rc = SQLITE_OK;
  }
  যদি( op==SQLITE_FCNTL_আকার_LIMIT ){
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
  ফেরত rc;
}

#যদি 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
স্থির পূর্ণসংখ্যা memdb_Sector_আকার(sqlite3_file *pFile){
  ফেরত 1024;
}
#endif

/*
** Return the device characteristic flags supported by an memdb-file.
*/
স্থির পূর্ণসংখ্যা memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  ফেরত SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
স্থির পূর্ণসংখ্যা memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  পূর্ণসংখ্যা iAmt,
  শূন্য **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  যদি( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }অন্যথা{
    p->nMmap++;
    *pp = (শূন্য*)(p->a_তথ্য + iOfst);
  }
  memdbLeave(p);
  ফেরত SQLITE_OK;
}

/* Release a memory-mapped page */
স্থির পূর্ণসংখ্যা memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, শূন্য *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  ফেরত SQLITE_OK;
}

/*
** Open an mem file handle.
*/
স্থির পূর্ণসংখ্যা memdbOpen(
  sqlite3_vfs *pVfs,
  ধ্রুবক অক্ষর *zName,
  sqlite3_file *pFd,
  পূর্ণসংখ্যা flags,
  পূর্ণসংখ্যা *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  পূর্ণসংখ্যা szName;
  UNUSED_PARAMETER(pVfs);

  memset(pFile, 0, আকার(*pFile));
  szName = sqlite3Strlen30(zName);
  যদি( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    পূর্ণসংখ্যা i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    জন্য(i=0; i<memdb_g.nMemStore; i++){
      যদি( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        বিরতি;
      }
    }
    যদি( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( আকার(*p) + (i64)szName + 3 );
      যদি( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        ফেরত SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             আকার(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      যদি( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ফেরত SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, আকার(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mx_Memdb_আকার;
      p->zFName = (অক্ষর*)&p[1];
      memcpy(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      যদি( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        ফেরত SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }অন্যথা{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }অন্যথা{
    p = sqlite3Malloc( আকার(*p) );
    যদি( p==0 ){
      ফেরত SQLITE_NOMEM;
    }
    memset(p, 0, আকার(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mx_Memdb_আকার;
  }
  pFile->pStore = p;
  যদি( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  ফেরত SQLITE_OK;
}

#যদি 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
স্থির পূর্ণসংখ্যা memdbDelete(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath, পূর্ণসংখ্যা dirSync){
  ফেরত SQLITE_IOERR_DELETE;
}
#endif

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
স্থির পূর্ণসংখ্যা memdbAccess(
  sqlite3_vfs *pVfs, 
  ধ্রুবক অক্ষর *zPath, 
  পূর্ণসংখ্যা flags, 
  পূর্ণসংখ্যা *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  ফেরত SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
স্থির পূর্ণসংখ্যা memdbFullPathname(
  sqlite3_vfs *pVfs, 
  ধ্রুবক অক্ষর *zPath, 
  পূর্ণসংখ্যা nOut, 
  অক্ষর *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  ফেরত SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
স্থির শূন্য *memdbDlOpen(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath){
  ফেরত ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
স্থির শূন্য memdbDlError(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nByte, অক্ষর *z_Err_বার্তা){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_বার্তা);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
স্থির শূন্য (*memdbDlSym(sqlite3_vfs *pVfs, শূন্য *p, ধ্রুবক অক্ষর *zSym))(শূন্য){
  ফেরত ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
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
স্থির পূর্ণসংখ্যা memdbRandomness(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nByte, অক্ষর *zBufOut){
  ফেরত ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
স্থির পূর্ণসংখ্যা memdbSleep(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nMicro){
  ফেরত ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#যদি 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
স্থির পূর্ণসংখ্যা memdbCurrentTime(sqlite3_vfs *pVfs, দ্বিগুণ *pTimeOut){
  ফেরত ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#endif

স্থির পূর্ণসংখ্যা memdbGetLastError(sqlite3_vfs *pVfs, পূর্ণসংখ্যা a, অক্ষর *b){
  ফেরত ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
স্থির পূর্ণসংখ্যা memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  ফেরত ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
স্থির MemFile *memdbFromDbSchema(sqlite3 *db, ধ্রুবক অক্ষর *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  পূর্ণসংখ্যা rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  যদি( rc ) ফেরত 0;
  যদি( p->base.pMethods!=&memdb_io_methods ) ফেরত 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  যদি( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  ফেরত p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
unsigned অক্ষর *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  ধ্রুবক অক্ষর *zSchema,      /* Which database within the connection */
  sqlite3_int64 *pi_আকার,    /* Write size here, if not NULL */
  unsigned পূর্ণসংখ্যা mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  পূর্ণসংখ্যা iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  পূর্ণসংখ্যা szPage = 0;
  sqlite3_stmt *pStmt = 0;
  unsigned অক্ষর *pOut = 0;
  অক্ষর *zSql;
  পূর্ণসংখ্যা rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) ){
    (শূন্য)SQLITE_MISUSE_BKPT;
    ফেরত 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  যদি( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  যদি( pi_আকার ) *pi_আকার = -1;
  যদি( iDb<0 ) যাও serialize_out;
  যদি( p ){
    MemStore *pStore = p->pStore;
    প্রতিজ্ঞা( pStore->pMutex==0 );
    যদি( pi_আকার ) *pi_আকার = pStore->sz;
    যদি( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->a_তথ্য;
    }অন্যথা{
      pOut = sqlite3_malloc64( pStore->sz );
      যদি( pOut ) memcpy(pOut, pStore->a_তথ্য, pStore->sz);
    }
    যাও serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  যদি( pBt==0 ) যাও serialize_out;
  szPage = sqlite3_Btree_Get_Page_আকার(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  যদি( rc ) যাও serialize_out;
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
    যদি( pi_আকার ) *pi_আকার = sz;
    যদি( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }অন্যথা{
      pOut = sqlite3_malloc64( sz );
      যদি( pOut ){
        পূর্ণসংখ্যা nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        পূর্ণসংখ্যা pgno;
        জন্য(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          unsigned অক্ষর *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          যদি( rc==SQLITE_OK ){
            memcpy(pTo, sqlite3_Pager_Get_তথ্য(pPage), szPage);
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
  ফেরত pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
পূর্ণসংখ্যা sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  ধ্রুবক অক্ষর *zSchema,    /* Which DB to reopen with the deserialization */
  unsigned অক্ষর *p_তথ্য,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  unsigned mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  অক্ষর *zSql;
  sqlite3_stmt *pStmt = 0;
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  যদি( !sqlite3SafetyCheckOk(db) ){
    ফেরত SQLITE_MISUSE_BKPT;
  }
  যদি( szDb<0 ) ফেরত SQLITE_MISUSE_BKPT;
  যদি( szBuf<0 ) ফেরত SQLITE_MISUSE_BKPT;
#endif

  sqlite3_mutex_enter(db->mutex);
  যদি( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  যদি( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    যাও end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  যদি( zSql==0 ){
    rc = SQLITE_NOMEM;
  }অন্যথা{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  যদি( rc ) যাও end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  যদি( rc!=SQLITE_OK ){
    যাও end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  যদি( p==0 ){
    rc = SQLITE_ERROR;
  }অন্যথা{
    MemStore *pStore = p->pStore;
    pStore->a_তথ্য = p_তথ্য;
    p_তথ্য = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    যদি( pStore->szMax<sqlite3GlobalConfig.mx_Memdb_আকার ){
      pStore->szMax = sqlite3GlobalConfig.mx_Memdb_আকার;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  যদি( p_তথ্য && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(p_তথ্য);
  }
  sqlite3_mutex_leave(db->mutex);
  ফেরত rc;
}

/*
** Return true if the VFS is the memvfs.
*/
পূর্ণসংখ্যা sqlite3IsMemdb(ধ্রুবক sqlite3_vfs *pVfs){
  ফেরত pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
পূর্ণসংখ্যা sqlite3MemdbInit(শূন্য){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  unsigned পূর্ণসংখ্যা sz;
  যদি( NEVER(pLower==0) ) ফেরত SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.p_App_তথ্য = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  যদি( sz<আকার(MemFile) ) sz = আকার(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  ফেরত sqlite3_vfs_register(&memdb_vfs, 0);
}
#endif /* SQLITE_OMIT_DESERIALIZE */
