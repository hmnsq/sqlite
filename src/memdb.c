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
#अन्तर्भाव "sqliteInt.h"
#ifndef SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
प्रकारनिर्देश संरचना sqlite3_vfs MemVfs;
प्रकारनिर्देश संरचना MemFile MemFile;
प्रकारनिर्देश संरचना MemStore MemStore;

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
संरचना MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  अचिन्हित वर्ण *aData;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  पूर्णाङ्क nMmap;                      /* Number of memory mapped pages */
  अचिन्हित mFlags;                /* Flags */
  पूर्णाङ्क nRdLock;                    /* Number of readers */
  पूर्णाङ्क nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  पूर्णाङ्क nRef;                       /* Number of users of this MemStore */
  वर्ण *zFName;                   /* The filename for shared stores */
};

/* An open file */
संरचना MemFile {
  sqlite3_file base;              /* IO methods */
  MemStore *pStore;               /* The storage */
  पूर्णाङ्क eLock;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
स्थिर संरचना MemFS {
  पूर्णाङ्क nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
स्थिर पूर्णाङ्क memdbClose(sqlite3_file*);
स्थिर पूर्णाङ्क memdbRead(sqlite3_file*, शून्य*, पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क memdbWrite(sqlite3_file*,अचर शून्य*,पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क memdbTruncate(sqlite3_file*, sqlite3_int64 size);
स्थिर पूर्णाङ्क memdbSync(sqlite3_file*, पूर्णाङ्क flags);
स्थिर पूर्णाङ्क memdbFileSize(sqlite3_file*, sqlite3_int64 *pSize);
स्थिर पूर्णाङ्क memdbLock(sqlite3_file*, पूर्णाङ्क);
स्थिर पूर्णाङ्क memdbUnlock(sqlite3_file*, पूर्णाङ्क);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
स्थिर पूर्णाङ्क memdbFileControl(sqlite3_file*, पूर्णाङ्क op, शून्य *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
स्थिर पूर्णाङ्क memdbDeviceCharacteristics(sqlite3_file*);
स्थिर पूर्णाङ्क memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, पूर्णाङ्क iAmt, शून्य **pp);
स्थिर पूर्णाङ्क memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, शून्य *p);

/*
** Methods for MemVfs
*/
स्थिर पूर्णाङ्क memdbOpen(sqlite3_vfs*, अचर वर्ण *, sqlite3_file*, पूर्णाङ्क , पूर्णाङ्क *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
स्थिर पूर्णाङ्क memdbAccess(sqlite3_vfs*, अचर वर्ण *zName, पूर्णाङ्क flags, पूर्णाङ्क *);
स्थिर पूर्णाङ्क memdbFullPathname(sqlite3_vfs*, अचर वर्ण *zName, पूर्णाङ्क, वर्ण *zOut);
स्थिर शून्य *memdbDlOpen(sqlite3_vfs*, अचर वर्ण *zFilename);
स्थिर शून्य memdbDlError(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देश);
स्थिर शून्य (*memdbDlSym(sqlite3_vfs *pVfs, शून्य *p, अचर वर्ण*zSym))(शून्य);
स्थिर शून्य memdbDlClose(sqlite3_vfs*, शून्य*);
स्थिर पूर्णाङ्क memdbRandomness(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *zOut);
स्थिर पूर्णाङ्क memdbSleep(sqlite3_vfs*, पूर्णाङ्क microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
स्थिर पूर्णाङ्क memdbGetLastError(sqlite3_vfs*, पूर्णाङ्क, वर्ण *);
स्थिर पूर्णाङ्क memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

स्थिर sqlite3_vfs memdb_vfs = {
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

स्थिर अचर sqlite3_io_methods memdb_io_methods = {
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
#यदि defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
स्थिर शून्य memdbEnter(MemStore *p){
  UNUSED_PARAMETER(p);
}
स्थिर शून्य memdbLeave(MemStore *p){
  UNUSED_PARAMETER(p);
}
#अन्यथा
स्थिर शून्य memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
स्थिर शून्य memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#endif



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
स्थिर पूर्णाङ्क memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  यदि( p->zFName ){
    पूर्णाङ्क i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    लागि(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      यदि( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        यदि( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          यदि( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        रोक;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }अन्यथा{
    memdbEnter(p);
  }
  p->nRef--;
  यदि( p->nRef<=0 ){
    यदि( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->aData);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }अन्यथा{
    memdbLeave(p);
  }
  फर्काऊ SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
स्थिर पूर्णाङ्क memdbRead(
  sqlite3_file *pFile, 
  शून्य *zBuf, 
  पूर्णाङ्क iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  यदि( iOfst+iAmt>p->sz ){
    स_रखो(zBuf, 0, iAmt);
    यदि( iOfst<p->sz ) स_नकल(zBuf, p->aData+iOfst, p->sz - iOfst);
    memdbLeave(p);
    फर्काऊ SQLITE_IOERR_SHORT_READ;
  }
  स_नकल(zBuf, p->aData+iOfst, iAmt);
  memdbLeave(p);
  फर्काऊ SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
स्थिर पूर्णाङ्क memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  अचिन्हित वर्ण *pNew;
  यदि( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    फर्काऊ SQLITE_FULL;
  }
  यदि( newSz>p->szMax ){
    फर्काऊ SQLITE_FULL;
  }
  newSz *= 2;
  यदि( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->aData, newSz);
  यदि( pNew==0 ) फर्काऊ SQLITE_IOERR_NOMEM;
  p->aData = pNew;
  p->szAlloc = newSz;
  फर्काऊ SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
स्थिर पूर्णाङ्क memdbWrite(
  sqlite3_file *pFile,
  अचर शून्य *z,
  पूर्णाङ्क iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  यदि( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    फर्काऊ SQLITE_IOERR_WRITE;
  }
  यदि( iOfst+iAmt>p->sz ){
    पूर्णाङ्क rc;
    यदि( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      फर्काऊ rc;
    }
    यदि( iOfst>p->sz ) स_रखो(p->aData+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  स_नकल(p->aData+iOfst, z, iAmt);
  memdbLeave(p);
  फर्काऊ SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
स्थिर पूर्णाङ्क memdbTruncate(sqlite3_file *pFile, sqlite_int64 size){
  MemStore *p = ((MemFile*)pFile)->pStore;
  पूर्णाङ्क rc = SQLITE_OK;
  memdbEnter(p);
  यदि( size>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }अन्यथा{
    p->sz = size; 
  }
  memdbLeave(p);
  फर्काऊ rc;
}

/*
** Sync an memdb-file.
*/
स्थिर पूर्णाङ्क memdbSync(sqlite3_file *pFile, पूर्णाङ्क flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  फर्काऊ SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
स्थिर पूर्णाङ्क memdbFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *pSize = p->sz;
  memdbLeave(p);
  फर्काऊ SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
स्थिर पूर्णाङ्क memdbLock(sqlite3_file *pFile, पूर्णाङ्क eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  पूर्णाङ्क rc = SQLITE_OK;
  यदि( eLock<=pThis->eLock ) फर्काऊ SQLITE_OK;
  memdbEnter(p);

  प्रतिज्ञा( p->nWrLock==0 || p->nWrLock==1 );
  प्रतिज्ञा( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  प्रतिज्ञा( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  यदि( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }अन्यथा{
    विकल्प( eLock ){
      पक्ष SQLITE_LOCK_SHARED: {
        प्रतिज्ञा( pThis->eLock==SQLITE_LOCK_NONE );
        यदि( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }अन्यथा{
          p->nRdLock++;
        }
        रोक;
      };
  
      पक्ष SQLITE_LOCK_RESERVED:
      पक्ष SQLITE_LOCK_PENDING: {
        प्रतिज्ञा( pThis->eLock>=SQLITE_LOCK_SHARED );
        यदि( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          यदि( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }अन्यथा{
            p->nWrLock = 1;
          }
        }
        रोक;
      }
  
      स्वभाव: {
        प्रतिज्ञा(  eLock==SQLITE_LOCK_EXCLUSIVE );
        प्रतिज्ञा( pThis->eLock>=SQLITE_LOCK_SHARED );
        यदि( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }अन्यथा यदि( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        रोक;
      }
    }
  }
  यदि( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  फर्काऊ rc;
}

/*
** Unlock an memdb-file.
*/
स्थिर पूर्णाङ्क memdbUnlock(sqlite3_file *pFile, पूर्णाङ्क eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  यदि( eLock>=pThis->eLock ) फर्काऊ SQLITE_OK;
  memdbEnter(p);

  प्रतिज्ञा( eLock==SQLITE_LOCK_SHARED || eLock==SQLITE_LOCK_NONE );
  यदि( eLock==SQLITE_LOCK_SHARED ){
    यदि( ALWAYS(pThis->eLock>SQLITE_LOCK_SHARED) ){
      p->nWrLock--;
    }
  }अन्यथा{
    यदि( pThis->eLock>SQLITE_LOCK_SHARED ){
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  फर्काऊ SQLITE_OK;
}

#यदि 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
स्थिर पूर्णाङ्क memdbCheckReservedLock(sqlite3_file *pFile, पूर्णाङ्क *pResOut){
  *pResOut = 0;
  फर्काऊ SQLITE_OK;
}
#endif


/*
** File control method. For custom operations on an memdb-file.
*/
स्थिर पूर्णाङ्क memdbFileControl(sqlite3_file *pFile, पूर्णाङ्क op, शून्य *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  पूर्णाङ्क rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  यदि( op==SQLITE_FCNTL_VFSNAME ){
    *(वर्ण**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->aData, p->sz);
    rc = SQLITE_OK;
  }
  यदि( op==SQLITE_FCNTL_SIZE_LIMIT ){
    sqlite3_int64 iLimit = *(sqlite3_int64*)pArg;
    यदि( iLimit<p->sz ){
      यदि( iLimit<0 ){
        iLimit = p->szMax;
      }अन्यथा{
        iLimit = p->sz;
      }
    }
    p->szMax = iLimit;
    *(sqlite3_int64*)pArg = iLimit;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  फर्काऊ rc;
}

#यदि 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
स्थिर पूर्णाङ्क memdbSectorSize(sqlite3_file *pFile){
  फर्काऊ 1024;
}
#endif

/*
** Return the device characteristic flags supported by an memdb-file.
*/
स्थिर पूर्णाङ्क memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  फर्काऊ SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
स्थिर पूर्णाङ्क memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  पूर्णाङ्क iAmt,
  शून्य **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  यदि( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }अन्यथा{
    p->nMmap++;
    *pp = (शून्य*)(p->aData + iOfst);
  }
  memdbLeave(p);
  फर्काऊ SQLITE_OK;
}

/* Release a memory-mapped page */
स्थिर पूर्णाङ्क memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, शून्य *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  फर्काऊ SQLITE_OK;
}

/*
** Open an mem file handle.
*/
स्थिर पूर्णाङ्क memdbOpen(
  sqlite3_vfs *pVfs,
  अचर वर्ण *zName,
  sqlite3_file *pFd,
  पूर्णाङ्क flags,
  पूर्णाङ्क *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  पूर्णाङ्क szName;
  UNUSED_PARAMETER(pVfs);

  स_रखो(pFile, 0, परिमाण(*pFile));
  szName = sqlite3Strlen30(zName);
  यदि( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    पूर्णाङ्क i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    लागि(i=0; i<memdb_g.nMemStore; i++){
      यदि( म_भेद(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        रोक;
      }
    }
    यदि( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( परिमाण(*p) + (i64)szName + 3 );
      यदि( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        फर्काऊ SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             परिमाण(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      यदि( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        फर्काऊ SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      स_रखो(p, 0, परिमाण(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mxMemdbSize;
      p->zFName = (वर्ण*)&p[1];
      स_नकल(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      यदि( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        फर्काऊ SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }अन्यथा{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }अन्यथा{
    p = sqlite3Malloc( परिमाण(*p) );
    यदि( p==0 ){
      फर्काऊ SQLITE_NOMEM;
    }
    स_रखो(p, 0, परिमाण(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mxMemdbSize;
  }
  pFile->pStore = p;
  यदि( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  फर्काऊ SQLITE_OK;
}

#यदि 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
स्थिर पूर्णाङ्क memdbDelete(sqlite3_vfs *pVfs, अचर वर्ण *zPath, पूर्णाङ्क dirSync){
  फर्काऊ SQLITE_IOERR_DELETE;
}
#endif

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
स्थिर पूर्णाङ्क memdbAccess(
  sqlite3_vfs *pVfs, 
  अचर वर्ण *zPath, 
  पूर्णाङ्क flags, 
  पूर्णाङ्क *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  फर्काऊ SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
स्थिर पूर्णाङ्क memdbFullPathname(
  sqlite3_vfs *pVfs, 
  अचर वर्ण *zPath, 
  पूर्णाङ्क nOut, 
  वर्ण *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  फर्काऊ SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
स्थिर शून्य *memdbDlOpen(sqlite3_vfs *pVfs, अचर वर्ण *zPath){
  फर्काऊ ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
स्थिर शून्य memdbDlError(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देश){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_सन्देश);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
स्थिर शून्य (*memdbDlSym(sqlite3_vfs *pVfs, शून्य *p, अचर वर्ण *zSym))(शून्य){
  फर्काऊ ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
स्थिर शून्य memdbDlClose(sqlite3_vfs *pVfs, शून्य *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
स्थिर पूर्णाङ्क memdbRandomness(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *zBufOut){
  फर्काऊ ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
स्थिर पूर्णाङ्क memdbSleep(sqlite3_vfs *pVfs, पूर्णाङ्क nMicro){
  फर्काऊ ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#यदि 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
स्थिर पूर्णाङ्क memdbCurrentTime(sqlite3_vfs *pVfs, द्विगुण *pTimeOut){
  फर्काऊ ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#endif

स्थिर पूर्णाङ्क memdbGetLastError(sqlite3_vfs *pVfs, पूर्णाङ्क a, वर्ण *b){
  फर्काऊ ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
स्थिर पूर्णाङ्क memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  फर्काऊ ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
स्थिर MemFile *memdbFromDbSchema(sqlite3 *db, अचर वर्ण *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  पूर्णाङ्क rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  यदि( rc ) फर्काऊ 0;
  यदि( p->base.pMethods!=&memdb_io_methods ) फर्काऊ 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  यदि( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  फर्काऊ p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
अचिन्हित वर्ण *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  अचर वर्ण *zSchema,      /* Which database within the connection */
  sqlite3_int64 *piSize,    /* Write size here, if not NULL */
  अचिन्हित पूर्णाङ्क mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  पूर्णाङ्क iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  पूर्णाङ्क szPage = 0;
  sqlite3_stmt *pStmt = 0;
  अचिन्हित वर्ण *pOut = 0;
  वर्ण *zSql;
  पूर्णाङ्क rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  यदि( !sqlite3SafetyCheckOk(db) ){
    (शून्य)SQLITE_MISUSE_BKPT;
    फर्काऊ 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  यदि( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  यदि( piSize ) *piSize = -1;
  यदि( iDb<0 ) गच्छ serialize_out;
  यदि( p ){
    MemStore *pStore = p->pStore;
    प्रतिज्ञा( pStore->pMutex==0 );
    यदि( piSize ) *piSize = pStore->sz;
    यदि( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->aData;
    }अन्यथा{
      pOut = sqlite3_malloc64( pStore->sz );
      यदि( pOut ) स_नकल(pOut, pStore->aData, pStore->sz);
    }
    गच्छ serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  यदि( pBt==0 ) गच्छ serialize_out;
  szPage = sqlite3BtreeGetPageSize(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  यदि( rc ) गच्छ serialize_out;
  rc = sqlite3_step(pStmt);
  यदि( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    यदि( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      यदि( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    यदि( piSize ) *piSize = sz;
    यदि( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }अन्यथा{
      pOut = sqlite3_malloc64( sz );
      यदि( pOut ){
        पूर्णाङ्क nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        पूर्णाङ्क pgno;
        लागि(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          अचिन्हित वर्ण *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          यदि( rc==SQLITE_OK ){
            स_नकल(pTo, sqlite3PagerGetData(pPage), szPage);
          }अन्यथा यदि( pgno==PENDING_BYTE/szPage+1 ){
            स_रखो(pTo, 0, szPage);
          }अन्यथा{
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
  फर्काऊ pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
पूर्णाङ्क sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  अचर वर्ण *zSchema,    /* Which DB to reopen with the deserialization */
  अचिन्हित वर्ण *pData,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  अचिन्हित mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  वर्ण *zSql;
  sqlite3_stmt *pStmt = 0;
  पूर्णाङ्क rc;
  पूर्णाङ्क iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  यदि( !sqlite3SafetyCheckOk(db) ){
    फर्काऊ SQLITE_MISUSE_BKPT;
  }
  यदि( szDb<0 ) फर्काऊ SQLITE_MISUSE_BKPT;
  यदि( szBuf<0 ) फर्काऊ SQLITE_MISUSE_BKPT;
#endif

  sqlite3_mutex_enter(db->mutex);
  यदि( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  यदि( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    गच्छ end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  यदि( zSql==0 ){
    rc = SQLITE_NOMEM;
  }अन्यथा{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  यदि( rc ) गच्छ end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  यदि( rc!=SQLITE_OK ){
    गच्छ end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  यदि( p==0 ){
    rc = SQLITE_ERROR;
  }अन्यथा{
    MemStore *pStore = p->pStore;
    pStore->aData = pData;
    pData = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    यदि( pStore->szMax<sqlite3GlobalConfig.mxMemdbSize ){
      pStore->szMax = sqlite3GlobalConfig.mxMemdbSize;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  यदि( pData && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(pData);
  }
  sqlite3_mutex_leave(db->mutex);
  फर्काऊ rc;
}

/*
** Return true if the VFS is the memvfs.
*/
पूर्णाङ्क sqlite3IsMemdb(अचर sqlite3_vfs *pVfs){
  फर्काऊ pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
पूर्णाङ्क sqlite3MemdbInit(शून्य){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  अचिन्हित पूर्णाङ्क sz;
  यदि( NEVER(pLower==0) ) फर्काऊ SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.pAppData = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  यदि( sz<परिमाण(MemFile) ) sz = परिमाण(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  फर्काऊ sqlite3_vfs_register(&memdb_vfs, 0);
}
#endif /* SQLITE_OMIT_DESERIALIZE */
