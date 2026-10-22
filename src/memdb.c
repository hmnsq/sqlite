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
#समावेश "sqliteInt.h"
#अगर_अघोषित SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
प्रकार काष्ठा sqlite3_vfs MemVfs;
प्रकार काष्ठा MemFile MemFile;
प्रकार काष्ठा MemStore MemStore;

/* Access to a lower-level VFS that (might) implement dynamic loading,
** access to randomness, etc.
*/
#घोषणा ORIGVFS(p) ((sqlite3_vfs*)((p)->p_App_तथ्य))

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
काष्ठा MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  अचिन्हित अक्षर *a_तथ्य;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  पूर्णांक nMmap;                      /* Number of memory mapped pages */
  अचिन्हित mFlags;                /* Flags */
  पूर्णांक nRdLock;                    /* Number of readers */
  पूर्णांक nWrLock;                    /* Number of writers.  (Always 0 or 1) */
  पूर्णांक nRef;                       /* Number of users of this MemStore */
  अक्षर *zFName;                   /* The filename for shared stores */
};

/* An open file */
काष्ठा MemFile {
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
अटल काष्ठा MemFS {
  पूर्णांक nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
अटल पूर्णांक memdbClose(sqlite3_file*);
अटल पूर्णांक memdbRead(sqlite3_file*, व्योम*, पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक memdbWrite(sqlite3_file*,स्थिर व्योम*,पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक memdbTruncate(sqlite3_file*, sqlite3_int64 आकार);
अटल पूर्णांक memdbSync(sqlite3_file*, पूर्णांक flags);
अटल पूर्णांक memdb_File_आकार(sqlite3_file*, sqlite3_int64 *p_आकार);
अटल पूर्णांक memdbLock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक memdbUnlock(sqlite3_file*, पूर्णांक);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
अटल पूर्णांक memdbFileControl(sqlite3_file*, पूर्णांक op, व्योम *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
अटल पूर्णांक memdbDeviceCharacteristics(sqlite3_file*);
अटल पूर्णांक memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, पूर्णांक iAmt, व्योम **pp);
अटल पूर्णांक memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, व्योम *p);

/*
** Methods for MemVfs
*/
अटल पूर्णांक memdbOpen(sqlite3_vfs*, स्थिर अक्षर *, sqlite3_file*, पूर्णांक , पूर्णांक *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
अटल पूर्णांक memdbAccess(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक flags, पूर्णांक *);
अटल पूर्णांक memdbFullPathname(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक, अक्षर *zOut);
अटल व्योम *memdbDlOpen(sqlite3_vfs*, स्थिर अक्षर *zFilename);
अटल व्योम memdbDlError(sqlite3_vfs*, पूर्णांक nByte, अक्षर *z_Err_संदेश);
अटल व्योम (*memdbDlSym(sqlite3_vfs *pVfs, व्योम *p, स्थिर अक्षर*zSym))(व्योम);
अटल व्योम memdbDlClose(sqlite3_vfs*, व्योम*);
अटल पूर्णांक memdbRandomness(sqlite3_vfs*, पूर्णांक nByte, अक्षर *zOut);
अटल पूर्णांक memdbSleep(sqlite3_vfs*, पूर्णांक microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
अटल पूर्णांक memdbGetLastError(sqlite3_vfs*, पूर्णांक, अक्षर *);
अटल पूर्णांक memdbCurrentTimeInt64(sqlite3_vfs*, sqlite3_int64*);

अटल sqlite3_vfs memdb_vfs = {
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

अटल स्थिर sqlite3_io_methods memdb_io_methods = {
  3,                              /* iVersion */
  memdbClose,                      /* xClose */
  memdbRead,                       /* xRead */
  memdbWrite,                      /* xWrite */
  memdbTruncate,                   /* xTruncate */
  memdbSync,                       /* xSync */
  memdb_File_आकार,                   /* xFileSize */
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
#अगर defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
अटल व्योम memdbEnter(MemStore *p){
  UNUSED_PARAMETER(p);
}
अटल व्योम memdbLeave(MemStore *p){
  UNUSED_PARAMETER(p);
}
#अन्यथा
अटल व्योम memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
अटल व्योम memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#पूर्ण_अगर



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
अटल पूर्णांक memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  अगर( p->zFName ){
    पूर्णांक i;
#अगर_अघोषित SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#पूर्ण_अगर
    sqlite3_mutex_enter(pVfsMutex);
    क्रम(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      अगर( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        अगर( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          अगर( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        अवरोध;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }अन्यथा{
    memdbEnter(p);
  }
  p->nRef--;
  अगर( p->nRef<=0 ){
    अगर( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->a_तथ्य);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }अन्यथा{
    memdbLeave(p);
  }
  वापस SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
अटल पूर्णांक memdbRead(
  sqlite3_file *pFile, 
  व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  अगर( iOfst+iAmt>p->sz ){
    स_रखो(zBuf, 0, iAmt);
    अगर( iOfst<p->sz ) स_नकल(zBuf, p->a_तथ्य+iOfst, p->sz - iOfst);
    memdbLeave(p);
    वापस SQLITE_IOERR_SHORT_READ;
  }
  स_नकल(zBuf, p->a_तथ्य+iOfst, iAmt);
  memdbLeave(p);
  वापस SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
अटल पूर्णांक memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  अचिन्हित अक्षर *pNew;
  अगर( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    वापस SQLITE_FULL;
  }
  अगर( newSz>p->szMax ){
    वापस SQLITE_FULL;
  }
  newSz *= 2;
  अगर( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->a_तथ्य, newSz);
  अगर( pNew==0 ) वापस SQLITE_IOERR_NOMEM;
  p->a_तथ्य = pNew;
  p->szAlloc = newSz;
  वापस SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
अटल पूर्णांक memdbWrite(
  sqlite3_file *pFile,
  स्थिर व्योम *z,
  पूर्णांक iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  अगर( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    वापस SQLITE_IOERR_WRITE;
  }
  अगर( iOfst+iAmt>p->sz ){
    पूर्णांक rc;
    अगर( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      वापस rc;
    }
    अगर( iOfst>p->sz ) स_रखो(p->a_तथ्य+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  स_नकल(p->a_तथ्य+iOfst, z, iAmt);
  memdbLeave(p);
  वापस SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
अटल पूर्णांक memdbTruncate(sqlite3_file *pFile, sqlite_int64 आकार){
  MemStore *p = ((MemFile*)pFile)->pStore;
  पूर्णांक rc = SQLITE_OK;
  memdbEnter(p);
  अगर( आकार>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }अन्यथा{
    p->sz = आकार; 
  }
  memdbLeave(p);
  वापस rc;
}

/*
** Sync an memdb-file.
*/
अटल पूर्णांक memdbSync(sqlite3_file *pFile, पूर्णांक flags){
  UNUSED_PARAMETER(pFile);
  UNUSED_PARAMETER(flags);
  वापस SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
अटल पूर्णांक memdb_File_आकार(sqlite3_file *pFile, sqlite_int64 *p_आकार){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *p_आकार = p->sz;
  memdbLeave(p);
  वापस SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
अटल पूर्णांक memdbLock(sqlite3_file *pFile, पूर्णांक eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  पूर्णांक rc = SQLITE_OK;
  अगर( eLock<=pThis->eLock ) वापस SQLITE_OK;
  memdbEnter(p);

  निश्चित( p->nWrLock==0 || p->nWrLock==1 );
  निश्चित( pThis->eLock<=SQLITE_LOCK_SHARED || p->nWrLock==1 );
  निश्चित( pThis->eLock==SQLITE_LOCK_NONE || p->nRdLock>=1 );

  अगर( eLock>SQLITE_LOCK_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }अन्यथा{
    चयन( eLock ){
      हाल SQLITE_LOCK_SHARED: {
        निश्चित( pThis->eLock==SQLITE_LOCK_NONE );
        अगर( p->nWrLock>0 ){
          rc = SQLITE_BUSY;
        }अन्यथा{
          p->nRdLock++;
        }
        अवरोध;
      };
  
      हाल SQLITE_LOCK_RESERVED:
      हाल SQLITE_LOCK_PENDING: {
        निश्चित( pThis->eLock>=SQLITE_LOCK_SHARED );
        अगर( ALWAYS(pThis->eLock==SQLITE_LOCK_SHARED) ){
          अगर( p->nWrLock>0 ){
            rc = SQLITE_BUSY;
          }अन्यथा{
            p->nWrLock = 1;
          }
        }
        अवरोध;
      }
  
      शेष: {
        निश्चित(  eLock==SQLITE_LOCK_EXCLUSIVE );
        निश्चित( pThis->eLock>=SQLITE_LOCK_SHARED );
        अगर( p->nRdLock>1 ){
          rc = SQLITE_BUSY;
        }अन्यथा अगर( pThis->eLock==SQLITE_LOCK_SHARED ){
          p->nWrLock = 1;
        }
        अवरोध;
      }
    }
  }
  अगर( rc==SQLITE_OK ) pThis->eLock = eLock;
  memdbLeave(p);
  वापस rc;
}

/*
** Unlock an memdb-file.
*/
अटल पूर्णांक memdbUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  अगर( eLock>=pThis->eLock ) वापस SQLITE_OK;
  memdbEnter(p);

  निश्चित( eLock==SQLITE_LOCK_SHARED || eLock==SQLITE_LOCK_NONE );
  अगर( eLock==SQLITE_LOCK_SHARED ){
    अगर( ALWAYS(pThis->eLock>SQLITE_LOCK_SHARED) ){
      p->nWrLock--;
    }
  }अन्यथा{
    अगर( pThis->eLock>SQLITE_LOCK_SHARED ){
      p->nWrLock--;
    }
    p->nRdLock--;
  }

  pThis->eLock = eLock;
  memdbLeave(p);
  वापस SQLITE_OK;
}

#अगर 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
अटल पूर्णांक memdbCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  *pResOut = 0;
  वापस SQLITE_OK;
}
#पूर्ण_अगर


/*
** File control method. For custom operations on an memdb-file.
*/
अटल पूर्णांक memdbFileControl(sqlite3_file *pFile, पूर्णांक op, व्योम *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  पूर्णांक rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  अगर( op==SQLITE_FCNTL_VFSNAME ){
    *(अक्षर**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->a_तथ्य, p->sz);
    rc = SQLITE_OK;
  }
  अगर( op==SQLITE_FCNTL_आकार_LIMIT ){
    sqlite3_int64 iLimit = *(sqlite3_int64*)pArg;
    अगर( iLimit<p->sz ){
      अगर( iLimit<0 ){
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
  वापस rc;
}

#अगर 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
अटल पूर्णांक memdb_Sector_आकार(sqlite3_file *pFile){
  वापस 1024;
}
#पूर्ण_अगर

/*
** Return the device characteristic flags supported by an memdb-file.
*/
अटल पूर्णांक memdbDeviceCharacteristics(sqlite3_file *pFile){
  UNUSED_PARAMETER(pFile);
  वापस SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_SAFE_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
अटल पूर्णांक memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  पूर्णांक iAmt,
  व्योम **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  अगर( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }अन्यथा{
    p->nMmap++;
    *pp = (व्योम*)(p->a_तथ्य + iOfst);
  }
  memdbLeave(p);
  वापस SQLITE_OK;
}

/* Release a memory-mapped page */
अटल पूर्णांक memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, व्योम *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  UNUSED_PARAMETER(iOfst);
  UNUSED_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  वापस SQLITE_OK;
}

/*
** Open an mem file handle.
*/
अटल पूर्णांक memdbOpen(
  sqlite3_vfs *pVfs,
  स्थिर अक्षर *zName,
  sqlite3_file *pFd,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  पूर्णांक szName;
  UNUSED_PARAMETER(pVfs);

  स_रखो(pFile, 0, माप(*pFile));
  szName = sqlite3Strlen30(zName);
  अगर( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    पूर्णांक i;
#अगर_अघोषित SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#पूर्ण_अगर
    sqlite3_mutex_enter(pVfsMutex);
    क्रम(i=0; i<memdb_g.nMemStore; i++){
      अगर( म_भेद(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        अवरोध;
      }
    }
    अगर( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( माप(*p) + (i64)szName + 3 );
      अगर( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        वापस SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             माप(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      अगर( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        वापस SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      स_रखो(p, 0, माप(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mx_Memdb_आकार;
      p->zFName = (अक्षर*)&p[1];
      स_नकल(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      अगर( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        वापस SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }अन्यथा{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }अन्यथा{
    p = sqlite3Malloc( माप(*p) );
    अगर( p==0 ){
      वापस SQLITE_NOMEM;
    }
    स_रखो(p, 0, माप(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mx_Memdb_आकार;
  }
  pFile->pStore = p;
  अगर( pOutFlags!=0 ){
    *pOutFlags = flags | SQLITE_OPEN_MEMORY;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  वापस SQLITE_OK;
}

#अगर 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
अटल पूर्णांक memdbDelete(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath, पूर्णांक dirSync){
  वापस SQLITE_IOERR_DELETE;
}
#पूर्ण_अगर

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
अटल पूर्णांक memdbAccess(
  sqlite3_vfs *pVfs, 
  स्थिर अक्षर *zPath, 
  पूर्णांक flags, 
  पूर्णांक *pResOut
){
  UNUSED_PARAMETER(pVfs);
  UNUSED_PARAMETER(zPath);
  UNUSED_PARAMETER(flags);
  *pResOut = 0;
  वापस SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
अटल पूर्णांक memdbFullPathname(
  sqlite3_vfs *pVfs, 
  स्थिर अक्षर *zPath, 
  पूर्णांक nOut, 
  अक्षर *zOut
){
  UNUSED_PARAMETER(pVfs);
  sqlite3_snprintf(nOut, zOut, "%s", zPath);
  वापस SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
अटल व्योम *memdbDlOpen(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath){
  वापस ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
अटल व्योम memdbDlError(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *z_Err_संदेश){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_संदेश);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
अटल व्योम (*memdbDlSym(sqlite3_vfs *pVfs, व्योम *p, स्थिर अक्षर *zSym))(व्योम){
  वापस ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
अटल व्योम memdbDlClose(sqlite3_vfs *pVfs, व्योम *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
अटल पूर्णांक memdbRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  वापस ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
अटल पूर्णांक memdbSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  वापस ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#अगर 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
अटल पूर्णांक memdbCurrentTime(sqlite3_vfs *pVfs, द्विगुन *pTimeOut){
  वापस ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), pTimeOut);
}
#पूर्ण_अगर

अटल पूर्णांक memdbGetLastError(sqlite3_vfs *pVfs, पूर्णांक a, अक्षर *b){
  वापस ORIGVFS(pVfs)->xGetLastError(ORIGVFS(pVfs), a, b);
}
अटल पूर्णांक memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  वापस ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
अटल MemFile *memdbFromDbSchema(sqlite3 *db, स्थिर अक्षर *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  पूर्णांक rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  अगर( rc ) वापस 0;
  अगर( p->base.pMethods!=&memdb_io_methods ) वापस 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  अगर( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  वापस p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
अचिन्हित अक्षर *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  स्थिर अक्षर *zSchema,      /* Which database within the connection */
  sqlite3_int64 *pi_आकार,    /* Write size here, if not NULL */
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

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    (व्योम)SQLITE_MISUSE_BKPT;
    वापस 0;
  }
#पूर्ण_अगर
  sqlite3_mutex_enter(db->mutex);

  अगर( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  अगर( pi_आकार ) *pi_आकार = -1;
  अगर( iDb<0 ) जाओ serialize_out;
  अगर( p ){
    MemStore *pStore = p->pStore;
    निश्चित( pStore->pMutex==0 );
    अगर( pi_आकार ) *pi_आकार = pStore->sz;
    अगर( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = pStore->a_तथ्य;
    }अन्यथा{
      pOut = sqlite3_malloc64( pStore->sz );
      अगर( pOut ) स_नकल(pOut, pStore->a_तथ्य, pStore->sz);
    }
    जाओ serialize_out;
  }
  pBt = db->aDb[iDb].pBt;
  अगर( pBt==0 ) जाओ serialize_out;
  szPage = sqlite3_Btree_Get_Page_आकार(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  अगर( rc ) जाओ serialize_out;
  rc = sqlite3_step(pStmt);
  अगर( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    अगर( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_step(pStmt);
      अगर( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    अगर( pi_आकार ) *pi_आकार = sz;
    अगर( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      pOut = 0;
    }अन्यथा{
      pOut = sqlite3_malloc64( sz );
      अगर( pOut ){
        पूर्णांक nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        पूर्णांक pgno;
        क्रम(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          अचिन्हित अक्षर *pTo = pOut + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          अगर( rc==SQLITE_OK ){
            स_नकल(pTo, sqlite3_Pager_Get_तथ्य(pPage), szPage);
          }अन्यथा अगर( pgno==PENDING_BYTE/szPage+1 ){
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
  वापस pOut;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
पूर्णांक sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  स्थिर अक्षर *zSchema,    /* Which DB to reopen with the deserialization */
  अचिन्हित अक्षर *p_तथ्य,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  अचिन्हित mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  अक्षर *zSql;
  sqlite3_stmt *pStmt = 0;
  पूर्णांक rc;
  पूर्णांक iDb;

#अगर_घोषित SQLITE_ENABLE_API_ARMOR
  अगर( !sqlite3SafetyCheckOk(db) ){
    वापस SQLITE_MISUSE_BKPT;
  }
  अगर( szDb<0 ) वापस SQLITE_MISUSE_BKPT;
  अगर( szBuf<0 ) वापस SQLITE_MISUSE_BKPT;
#पूर्ण_अगर

  sqlite3_mutex_enter(db->mutex);
  अगर( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  अगर( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    जाओ end_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  अगर( zSql==0 ){
    rc = SQLITE_NOMEM;
  }अन्यथा{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  अगर( rc ) जाओ end_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_step(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  अगर( rc!=SQLITE_OK ){
    जाओ end_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  अगर( p==0 ){
    rc = SQLITE_ERROR;
  }अन्यथा{
    MemStore *pStore = p->pStore;
    pStore->a_तथ्य = p_तथ्य;
    p_तथ्य = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    अगर( pStore->szMax<sqlite3GlobalConfig.mx_Memdb_आकार ){
      pStore->szMax = sqlite3GlobalConfig.mx_Memdb_आकार;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

end_deserialize:
  अगर( p_तथ्य && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(p_तथ्य);
  }
  sqlite3_mutex_leave(db->mutex);
  वापस rc;
}

/*
** Return true if the VFS is the memvfs.
*/
पूर्णांक sqlite3IsMemdb(स्थिर sqlite3_vfs *pVfs){
  वापस pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
पूर्णांक sqlite3MemdbInit(व्योम){
  sqlite3_vfs *pLower = sqlite3_vfs_find(0);
  अचिन्हित पूर्णांक sz;
  अगर( NEVER(pLower==0) ) वापस SQLITE_ERROR;
  sz = pLower->szOsFile;
  memdb_vfs.p_App_तथ्य = pLower;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  अगर( sz<माप(MemFile) ) sz = माप(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  वापस sqlite3_vfs_register(&memdb_vfs, 0);
}
#पूर्ण_अगर /* SQLITE_OMIT_DESERIALIZE */
