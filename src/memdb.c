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
प्रकारव्याख्या संरचना sqlite3_vfs MemVfs;
प्रकारव्याख्या संरचना MemFile MemFile;
प्रकारव्याख्या संरचना MemStore MemStore;

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
  अचिन्हित अक्षर *aData;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  पूर्णांक nMmap;                      /* Number of memory mapped pages */
  अचिन्हित mFlags;                /* Flags */
  पूर्णांक nRdLock;                    /* Number of readers */
  पूर्णांक nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  पूर्णांक nRef;                       /* Number of users of this MemStore */
  अक्षर *zFName;                   /* The filename for shared stores */
};

/* An open file */
संरचना MemFile {
  sqlite3_file base;              /* IO methods */
  MemStore *pStore;               /* The storage */
  पूर्णांक eLock;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
स्थिर संरचना MemFS {
  पूर्णांक nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
स्थिर पूर्णांक memdbClose(sqlite3_file*);
स्थिर पूर्णांक memdbRead(sqlite3_file*, शून्य*, पूर्णांक iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णांक memdbWrite(sqlite3_file*,स्थिरांक शून्य*,पूर्णांक iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णांक memdbTruncate(sqlite3_file*, sqlite3_int64 size);
स्थिर पूर्णांक memdbSync(sqlite3_file*, पूर्णांक flags);
स्थिर पूर्णांक memdbFileSize(sqlite3_file*, sqlite3_int64 *pSize);
स्थिर पूर्णांक memdbLock(sqlite3_file*, पूर्णांक);
स्थिर पूर्णांक memdbUnlock(sqlite3_file*, पूर्णांक);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
स्थिर पूर्णांक memdbFileControl(sqlite3_file*, पूर्णांक op, शून्य *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
स्थिर पूर्णांक memdbDeviceCharacteristics(sqlite3_file*);
स्थिर पूर्णांक memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, पूर्णांक iAmt, शून्य **pp);
स्थिर पूर्णांक memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, शून्य *p);

/*
** Methods for MemVfs
*/
स्थिर पूर्णांक memdbOpen(sqlite3_vfs*, स्थिरांक अक्षर *, sqlite3_file*, पूर्णांक , पूर्णांक *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
स्थिर पूर्णांक memdbAccess(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक flags, पूर्णांक *);
स्थिर पूर्णांक memdbFullPathname(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक, अक्षर *zOut);
स्थिर शून्य *memdbDlOpen(sqlite3_vfs*, स्थिरांक अक्षर *zFilename);
स्थिर शून्य memdbDlError(sqlite3_vfs*, पूर्णांक nByte, अक्षर *z_Err_संदेश);
स्थिर शून्य (*memdbDlSym(sqlite3_vfs *pVfs, शून्य *p, स्थिरांक अक्षर*zSym))(शून्य);
स्थिर शून्य memdbDlClose(sqlite3_vfs*, शून्य*);
स्थिर पूर्णांक memdbRandomness(sqlite3_vfs*, पूर्णांक nByte, अक्षर *zOut);
स्थिर पूर्णांक memdbSleep(sqlite3_vfs*, पूर्णांक microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
स्थिर पूर्णांक memdbGetLastError(sqlite3_vfs*, पूर्णांक, अक्षर *);
स्थिर पूर्णांक memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

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

स्थिर स्थिरांक sqlite3_io_methods memdb_io_methods = {
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
#जर defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
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
स्थिर पूर्णांक memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  जर( p->zFName ){
    पूर्णांक i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    साठी(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      जर( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        जर( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          जर( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        थांबा;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }अन्यथा{
    memdbEnter(p);
  }
  p->nRef--;
  जर( p->nRef<=0 ){
    जर( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->aData);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }अन्यथा{
    memdbLeave(p);
  }
  परत SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
स्थिर पूर्णांक memdbRead(
  sqlite3_file *pFile, 
  शून्य *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  जर( iOfst+iAmt>p->sz ){
    स_रखो(zBuf, 0, iAmt);
    जर( iOfst<p->sz ) स_नकल(zBuf, p->aData+iOfst, p->sz - iOfst);
    memdbLeave(p);
    परत SQLITE_IOERR_SHORT_READ;
  }
  स_नकल(zBuf, p->aData+iOfst, iAmt);
  memdbLeave(p);
  परत SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
स्थिर पूर्णांक memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  अचिन्हित अक्षर *pNew;
  जर( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    परत SQLITE_FULL;
  }
  जर( newSz>p->szMax ){
    परत SQLITE_FULL;
  }
  newSz *= 2;
  जर( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->aData, newSz);
  जर( pNew==0 ) परत SQLITE_IOERR_NOMEM;
  p->aData = pNew;
  p->szAlloc = newSz;
  परत SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
स्थिर पूर्णांक memdbWrite(
  sqlite3_file *pFile,
  स्थिरांक शून्य *z,
  पूर्णांक iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  जर( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    परत SQLITE_IOERR_WRITE;
  }
  जर( iOfst+iAmt>p->sz ){
    पूर्णांक rc;
    जर( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      परत rc;
    }
    जर( iOfst>p->sz ) स_रखो(p->aData+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  स_नकल(p->aData+iOfst, z, iAmt);
  memdbLeave(p);
  परत SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
स्थिर पूर्णांक memdbTruncate(sqlite3_file *pFile, sqlite_int64 size){
  MemStore *p = ((MemFile*)pFile)->pStore;
  पूर्णांक rc = SQLITE_OK;
  memdbEnter(p);
  जर( size>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }अन्यथा{
    p->sz = size; 
  }
  memdbLeave(p);
  परत rc;
}

/*
** Sync an memdb-file.
*/
स्थिर पूर्णांक memdbSync(sqlite3_file *pFile, पूर्णांक flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  परत SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
स्थिर पूर्णांक memdbFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *pSize = p->sz;
  memdbLeave(p);
  परत SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
स्थिर पूर्णांक memdbLock(sqlite3_file *pFile, पूर्णांक eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  पूर्णांक rc = SQLITE_OK;
  जर( eLock<=pThis->eLock ) परत SQLITE_OK;
  memdbEnter(p);

  प्रतिज्ञा( p->nWrLock==0 || p->nWrLock==1 );
  प्रतिज्ञा( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  प्रतिज्ञा( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  जर( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }अन्यथा{
    निवड( eLock ){
      स्थिती SQLITE_LOCK_SHARED: {
        प्रतिज्ञा( pThis->eLock==SQLITE_LOCK_NONE );
        जर( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }अन्यथा{
          p->nRdLock++;
        }
        थांबा;
      };
  
      स्थिती SQLITE_LOCK_RESERVED:
      स्थिती SQLITE_LOCK_PENDING: {
        प्रतिज्ञा( pThis->eLock>=SQLITE_LOCK_SHARED );
        जर( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          जर( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }अन्यथा{
            p->nWrLock = 1;
          }
        }
        थांबा;
      }
  
      पूर्वनिर्धारित: {
        प्रतिज्ञा(  eLock==SQLITE_LOCK_EXCLUSIVE );
        प्रतिज्ञा( pThis->eLock>=SQLITE_LOCK_SHARED );
        जर( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }अन्यथा जर( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        थांबा;
      }
    }
  }
  जर( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  परत rc;
}

/*
** Unlock an memdb-file.
*/
स्थिर पूर्णांक memdbUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  जर( eLock>=pThis->eLock ) परत SQLITE_OK;
  memdbEnter(p);

  प्रतिज्ञा( eLock==SQLITE_LOCK_SHARED || eLock==SQLITE_LOCK_NONE );
  जर( eLock==SQLITE_LOCK_SHARED ){
    जर( ALWAYS(pThis->eLock>SQLITE_LOCK_SHARED) ){
      p->nWrLock--;
    }
  }अन्यथा{
    जर( pThis->eLock>SQLITE_LOCK_SHARED ){
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  परत SQLITE_OK;
}

#जर 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
स्थिर पूर्णांक memdbCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  *pResOut = 0;
  परत SQLITE_OK;
}
#endif


/*
** File control method. For custom operations on an memdb-file.
*/
स्थिर पूर्णांक memdbFileControl(sqlite3_file *pFile, पूर्णांक op, शून्य *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  पूर्णांक rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  जर( op==SQLITE_FCNTL_VFSNAME ){
    *(अक्षर**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->aData, p->sz);
    rc = SQLITE_OK;
  }
  जर( op==SQLITE_FCNTL_SIZE_LIMIT ){
    sqlite3_int64 iLimit = *(sqlite3_int64*)pArg;
    जर( iLimit<p->sz ){
      जर( iLimit<0 ){
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
  परत rc;
}

#जर 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
स्थिर पूर्णांक memdbSectorSize(sqlite3_file *pFile){
  परत 1024;
}
#endif

/*
** Return the device characteristic flags supported by an memdb-file.
*/
स्थिर पूर्णांक memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  परत SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
स्थिर पूर्णांक memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  पूर्णांक iAmt,
  शून्य **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  जर( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }अन्यथा{
    p->nMmap++;
    *pp = (शून्य*)(p->aData + iOfst);
  }
  memdbLeave(p);
  परत SQLITE_OK;
}

/* Release a memory-mapped page */
स्थिर पूर्णांक memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, शून्य *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  परत SQLITE_OK;
}

/*
** Open an mem file handle.
*/
स्थिर पूर्णांक memdbOpen(
  sqlite3_vfs *pVfs,
  स्थिरांक अक्षर *zName,
  sqlite3_file *pFd,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  पूर्णांक szName;
  UNUSED_PARAMETER(pVfs);

  स_रखो(pFile, 0, आकार(*pFile));
  szName = sqlite3Strlen30(zName);
  जर( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    पूर्णांक i;
#ifndef SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#endif
    sqlite3_mutex_enter(pVfsMutex);
    साठी(i=0; i<memdb_g.nMemStore; i++){
      जर( म_भेद(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        थांबा;
      }
    }
    जर( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( आकार(*p) + (i64)szName + 3 );
      जर( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        परत SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             आकार(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      जर( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        परत SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      स_रखो(p, 0, आकार(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mxMemdbSize;
      p->zFName = (अक्षर*)&p[1];
      स_नकल(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      जर( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        परत SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }अन्यथा{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }अन्यथा{
    p = sqlite3Malloc( आकार(*p) );
    जर( p==0 ){
      परत SQLITE_NOMEM;
    }
    स_रखो(p, 0, आकार(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mxMemdbSize;
  }
  pFile->pStore = p;
  जर( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  परत SQLITE_OK;
}

#जर 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
स्थिर पूर्णांक memdbDelete(sqlite3_vfs *pVfs, स्थिरांक अक्षर *zPath, पूर्णांक dirSync){
  परत SQLITE_IOERR_DELETE;
}
#endif

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
स्थिर पूर्णांक memdbAccess(
  sqlite3_vfs *pVfs, 
  स्थिरांक अक्षर *zPath, 
  पूर्णांक flags, 
  पूर्णांक *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  परत SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
स्थिर पूर्णांक memdbFullPathname(
  sqlite3_vfs *pVfs, 
  स्थिरांक अक्षर *zPath, 
  पूर्णांक nOut, 
  अक्षर *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  परत SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
स्थिर शून्य *memdbDlOpen(sqlite3_vfs *pVfs, स्थिरांक अक्षर *zPath){
  परत ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
स्थिर शून्य memdbDlError(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *z_Err_संदेश){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_संदेश);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
स्थिर शून्य (*memdbDlSym(sqlite3_vfs *pVfs, शून्य *p, स्थिरांक अक्षर *zSym))(शून्य){
  परत ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
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
स्थिर पूर्णांक memdbRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  परत ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
स्थिर पूर्णांक memdbSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  परत ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#जर 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
स्थिर पूर्णांक memdbCurrentTime(sqlite3_vfs *pVfs, द्विगुण *pTimeOut){
  परत ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#endif

स्थिर पूर्णांक memdbGetLastError(sqlite3_vfs *pVfs, पूर्णांक a, अक्षर *b){
  परत ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
स्थिर पूर्णांक memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  परत ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
स्थिर MemFile *memdbFromDbSchema(sqlite3 *db, स्थिरांक अक्षर *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  पूर्णांक rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  जर( rc ) परत 0;
  जर( p->base.pMethods!=&memdb_io_methods ) परत 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  जर( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  परत p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
अचिन्हित अक्षर *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  स्थिरांक अक्षर *zSchema,      /* Which database within the connection */
  sqlite3_int64 *piSize,    /* Write size here, if not NULL */
  अचिन्हित पूर्णांक mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  पूर्णांक iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  पूर्णांक szPage = 0;
  sqlite3_stmt *pStmt = 0;
  अचिन्हित अक्षर *pOut = 0;
  अक्षर *zSql;
  पूर्णांक rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  जर( !sqlite3SafetyCheckOk(db) ){
    (शून्य)SQLITE_MISUSE_BKPT;
    परत 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);

  जर( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  जर( piSize ) *piSize = -1;
  जर( iDb<0 ) जा serialize_out;
  जर( p ){
    MemStore *pStore = p->pStore;
    प्रतिज्ञा( pStore->pMutex==0 );
    जर( piSize ) *piSize = pStore->sz;
    जर( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->aData;
    }अन्यथा{
      pOut = sqlite3_malloc64( pStore->sz );
      जर( pOut ) स_नकल(pOut, pStore->aData, pStore->sz);
    }
    जा serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  जर( pBt==0 ) जा serialize_out;
  szPage = sqlite3BtreeGetPageSize(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  जर( rc ) जा serialize_out;
  rc = sqlite3_step(pStmt);
  जर( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    जर( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      जर( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    जर( piSize ) *piSize = sz;
    जर( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }अन्यथा{
      pOut = sqlite3_malloc64( sz );
      जर( pOut ){
        पूर्णांक nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        पूर्णांक pgno;
        साठी(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          अचिन्हित अक्षर *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          जर( rc==SQLITE_OK ){
            स_नकल(pTo, sqlite3PagerGetData(pPage), szPage);
          }अन्यथा जर( pgno==PENDING_BYTE/szPage+1 ){
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
  परत pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
पूर्णांक sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  स्थिरांक अक्षर *zSchema,    /* Which DB to reopen with the deserialization */
  अचिन्हित अक्षर *pData,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  अचिन्हित mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  अक्षर *zSql;
  sqlite3_stmt *pStmt = 0;
  पूर्णांक rc;
  पूर्णांक iDb;

#ifdef SQLITE_ENABLE_API_ARMOR
  जर( !sqlite3SafetyCheckOk(db) ){
    परत SQLITE_MISUSE_BKPT;
  }
  जर( szDb<0 ) परत SQLITE_MISUSE_BKPT;
  जर( szBuf<0 ) परत SQLITE_MISUSE_BKPT;
#endif

  sqlite3_mutex_enter(db->mutex);
  जर( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  जर( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    जा end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  जर( zSql==0 ){
    rc = SQLITE_NOMEM;
  }अन्यथा{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  जर( rc ) जा end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  जर( rc!=SQLITE_OK ){
    जा end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  जर( p==0 ){
    rc = SQLITE_ERROR;
  }अन्यथा{
    MemStore *pStore = p->pStore;
    pStore->aData = pData;
    pData = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    जर( pStore->szMax<sqlite3GlobalConfig.mxMemdbSize ){
      pStore->szMax = sqlite3GlobalConfig.mxMemdbSize;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  जर( pData && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(pData);
  }
  sqlite3_mutex_leave(db->mutex);
  परत rc;
}

/*
** Return true if the VFS is the memvfs.
*/
पूर्णांक sqlite3IsMemdb(स्थिरांक sqlite3_vfs *pVfs){
  परत pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
पूर्णांक sqlite3MemdbInit(शून्य){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  अचिन्हित पूर्णांक sz;
  जर( NEVER(pLower==0) ) परत SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.pAppData = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  जर( sz<आकार(MemFile) ) sz = आकार(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  परत sqlite3_vfs_register(&memdb_vfs, 0);
}
#endif /* SQLITE_OMIT_DESERIALIZE */
