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
#اگر_تعریف_نشده SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
تعريفنوع بنية sqlite3_vfs MemVfs;
تعريفنوع بنية MemFile MemFile;
تعريفنوع بنية MemStore MemStore;

/* Access to a lower-level VFS that (might) implement dynamic loading,
** access to randomness, etc.
*/
#تعریف_کن ORIGVFS(p) ((sqlite3_vfs*)((p)->p_App_داده))

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
  بیعلامت حرف *a_داده;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  صحيح nMmap;                      /* Number of memory mapped pages */
  بیعلامت mFlags;                /* Flags */
  صحيح n_Rd_قفل;                    /* Number of readers */
  صحيح n_Wr_قفل;                    /* Number of writers.  (Always 0 or 1) */
  صحيح nRef;                       /* Number of users of this MemStore */
  حرف *zFName;                   /* The filename for shared stores */
};

/* An open file */
بنية MemFile {
  sqlite3_file پایه;              /* IO methods */
  MemStore *pStore;               /* The storage */
  صحيح e_قفل;                      /* Most recent lock against this file */
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
ساكن صحيح memdbTruncate(sqlite3_file*, sqlite3_int64 اندازه);
ساكن صحيح memdbSync(sqlite3_file*, صحيح flags);
ساكن صحيح memdb_File_اندازه(sqlite3_file*, sqlite3_int64 *p_اندازه);
ساكن صحيح memdb_قفل(sqlite3_file*, صحيح);
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
ساكن صحيح memdb_پر_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحيح, حرف *z_بیرون);
ساكن فراغ *memdbDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن فراغ memdbDlError(sqlite3_vfs*, صحيح nByte, حرف *z_Err_پیام);
ساكن فراغ (*memdbDlSym(sqlite3_vfs *pVfs, فراغ *p, ثابت حرف*zSym))(فراغ);
ساكن فراغ memdbDlClose(sqlite3_vfs*, فراغ*);
ساكن صحيح memdbRandomness(sqlite3_vfs*, صحيح nByte, حرف *z_بیرون);
ساكن صحيح memdbSleep(sqlite3_vfs*, صحيح microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
ساكن صحيح memdb_Get_آخرین_Error(sqlite3_vfs*, صحيح, حرف *);
ساكن صحيح memdb_Current_زمان_Int64(sqlite3_vfs*, sqlite3_int64*);

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
  memdb_پر_Pathname,           /* xFullPathname */
  memdbDlOpen,                 /* xDlOpen */
  memdbDlError,                /* xDlError */
  memdbDlSym,                  /* xDlSym */
  memdbDlClose,                /* xDlClose */
  memdbRandomness,             /* xRandomness */
  memdbSleep,                  /* xSleep */
  0, /* memdbCurrentTime, */   /* xCurrentTime */
  memdb_Get_آخرین_Error,           /* xGetLastError */
  memdb_Current_زمان_Int64,       /* xCurrentTimeInt64 */
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
  memdb_File_اندازه,                   /* xFileSize */
  memdb_قفل,                       /* xLock */
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
#إذا defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
ساكن فراغ memdbEnter(MemStore *p){
  استفادهنشده_PARAMETER(p);
}
ساكن فراغ memdbLeave(MemStore *p){
  استفادهنشده_PARAMETER(p);
}
#وإلا
ساكن فراغ memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
ساكن فراغ memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#پایان_اگر



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
ساكن صحيح memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  إذا( p->zFName ){
    صحيح i;
#اگر_تعریف_نشده SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#پایان_اگر
    sqlite3_mutex_enter(pVfsMutex);
    لكل(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      إذا( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        إذا( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          إذا( memdb_g.nMemStore==0 ){
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
  إذا( p->nRef<=0 ){
    إذا( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->a_داده);
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
  إذا( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    إذا( iOfst<p->sz ) کپی_حافظه(zBuf, p->a_داده+iOfst, p->sz - iOfst);
    memdbLeave(p);
    أرجع SQLITE_IOERR_SHORT_READ;
  }
  کپی_حافظه(zBuf, p->a_داده+iOfst, iAmt);
  memdbLeave(p);
  أرجع SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
ساكن صحيح memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  بیعلامت حرف *pNew;
  إذا( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    أرجع SQLITE_پر;
  }
  إذا( newSz>p->szMax ){
    أرجع SQLITE_پر;
  }
  newSz *= 2;
  إذا( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->a_داده, newSz);
  إذا( pNew==0 ) أرجع SQLITE_IOERR_NOMEM;
  p->a_داده = pNew;
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
  إذا( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    أرجع SQLITE_IOERR_WRITE;
  }
  إذا( iOfst+iAmt>p->sz ){
    صحيح rc;
    إذا( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      أرجع rc;
    }
    إذا( iOfst>p->sz ) memset(p->a_داده+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  کپی_حافظه(p->a_داده+iOfst, z, iAmt);
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
ساكن صحيح memdbTruncate(sqlite3_file *pFile, sqlite_int64 اندازه){
  MemStore *p = ((MemFile*)pFile)->pStore;
  صحيح rc = SQLITE_OK;
  memdbEnter(p);
  إذا( اندازه>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }وإلا{
    p->sz = اندازه; 
  }
  memdbLeave(p);
  أرجع rc;
}

/*
** Sync an memdb-file.
*/
ساكن صحيح memdbSync(sqlite3_file *pFile, صحيح flags){
  استفادهنشده_PARAMETER(pFile);
  استفادهنشده_PARAMETER(flags);
  أرجع SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
ساكن صحيح memdb_File_اندازه(sqlite3_file *pFile, sqlite_int64 *p_اندازه){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *p_اندازه = p->sz;
  memdbLeave(p);
  أرجع SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
ساكن صحيح memdb_قفل(sqlite3_file *pFile, صحيح e_قفل){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  صحيح rc = SQLITE_OK;
  إذا( e_قفل<=pThis->e_قفل ) أرجع SQLITE_OK;
  memdbEnter(p);

  أكد( p->n_Wr_قفل==0 || p->n_Wr_قفل==1 );
  أكد( pThis->e_قفل<=SQLITE_قفل_SHARED || p->n_Wr_قفل==1 );
  أكد( pThis->e_قفل==SQLITE_قفل_NONE || p->n_Rd_قفل>=1 );

  إذا( e_قفل>SQLITE_قفل_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }وإلا{
    بدل( e_قفل ){
      حالة SQLITE_قفل_SHARED: {
        أكد( pThis->e_قفل==SQLITE_قفل_NONE );
        إذا( p->n_Wr_قفل>0 ){
          rc = SQLITE_BUSY;
        }وإلا{
          p->n_Rd_قفل++;
        }
        اكسر;
      };
  
      حالة SQLITE_قفل_RESERVED:
      حالة SQLITE_قفل_PENDING: {
        أكد( pThis->e_قفل>=SQLITE_قفل_SHARED );
        إذا( ALWAYS(pThis->e_قفل==SQLITE_قفل_SHARED) ){
          إذا( p->n_Wr_قفل>0 ){
            rc = SQLITE_BUSY;
          }وإلا{
            p->n_Wr_قفل = 1;
          }
        }
        اكسر;
      }
  
      افتراضي: {
        أكد(  e_قفل==SQLITE_قفل_EXCLUSIVE );
        أكد( pThis->e_قفل>=SQLITE_قفل_SHARED );
        إذا( p->n_Rd_قفل>1 ){
          rc = SQLITE_BUSY;
        }وإلا إذا( pThis->e_قفل==SQLITE_قفل_SHARED ){
          p->n_Wr_قفل = 1;
        }
        اكسر;
      }
    }
  }
  إذا( rc==SQLITE_OK ) pThis->e_قفل = e_قفل;
  memdbLeave(p);
  أرجع rc;
}

/*
** Unlock an memdb-file.
*/
ساكن صحيح memdbUnlock(sqlite3_file *pFile, صحيح e_قفل){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  إذا( e_قفل>=pThis->e_قفل ) أرجع SQLITE_OK;
  memdbEnter(p);

  أكد( e_قفل==SQLITE_قفل_SHARED || e_قفل==SQLITE_قفل_NONE );
  إذا( e_قفل==SQLITE_قفل_SHARED ){
    إذا( ALWAYS(pThis->e_قفل>SQLITE_قفل_SHARED) ){
      p->n_Wr_قفل--;
    }
  }وإلا{
    إذا( pThis->e_قفل>SQLITE_قفل_SHARED ){
      p->n_Wr_قفل--;
    }
    p->n_Rd_قفل--;
  }

  pThis->e_قفل = e_قفل;
  memdbLeave(p);
  أرجع SQLITE_OK;
}

#إذا 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
ساكن صحيح memdb_Check_Reserved_قفل(sqlite3_file *pFile, صحيح *p_Res_بیرون){
  *p_Res_بیرون = 0;
  أرجع SQLITE_OK;
}
#پایان_اگر


/*
** File control method. For custom operations on an memdb-file.
*/
ساكن صحيح memdbFileControl(sqlite3_file *pFile, صحيح op, فراغ *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  صحيح rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  إذا( op==SQLITE_FCNTL_VFSNAME ){
    *(حرف**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->a_داده, p->sz);
    rc = SQLITE_OK;
  }
  إذا( op==SQLITE_FCNTL_اندازه_حد ){
    sqlite3_int64 i_حد = *(sqlite3_int64*)pArg;
    إذا( i_حد<p->sz ){
      إذا( i_حد<0 ){
        i_حد = p->szMax;
      }وإلا{
        i_حد = p->sz;
      }
    }
    p->szMax = i_حد;
    *(sqlite3_int64*)pArg = i_حد;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  أرجع rc;
}

#إذا 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
ساكن صحيح memdb_Sector_اندازه(sqlite3_file *pFile){
  أرجع 1024;
}
#پایان_اگر

/*
** Return the device characteristic flags supported by an memdb-file.
*/
ساكن صحيح memdbDeviceCharacteristics(sqlite3_file *pFile){
  استفادهنشده_PARAMETER(pFile);
  أرجع SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_امن_APPEND |
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
  إذا( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }وإلا{
    p->nMmap++;
    *pp = (فراغ*)(p->a_داده + iOfst);
  }
  memdbLeave(p);
  أرجع SQLITE_OK;
}

/* Release a memory-mapped page */
ساكن صحيح memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, فراغ *p_صفحه){
  MemStore *p = ((MemFile*)pFile)->pStore;
  استفادهنشده_PARAMETER(iOfst);
  استفادهنشده_PARAMETER(p_صفحه);
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
  صحيح *p_بیرون_Flags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  صحيح szName;
  استفادهنشده_PARAMETER(pVfs);

  memset(pFile, 0, حجم(*pFile));
  szName = sqlite3Strlen30(zName);
  إذا( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    صحيح i;
#اگر_تعریف_نشده SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#پایان_اگر
    sqlite3_mutex_enter(pVfsMutex);
    لكل(i=0; i<memdb_g.nMemStore; i++){
      إذا( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        اكسر;
      }
    }
    إذا( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( حجم(*p) + (i64)szName + 3 );
      إذا( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        أرجع SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             حجم(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      إذا( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        أرجع SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, حجم(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mx_Memdb_اندازه;
      p->zFName = (حرف*)&p[1];
      کپی_حافظه(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      إذا( p->pMutex==0 ){
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
    إذا( p==0 ){
      أرجع SQLITE_NOMEM;
    }
    memset(p, 0, حجم(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mx_Memdb_اندازه;
  }
  pFile->pStore = p;
  إذا( p_بیرون_Flags!=0 ){
    *p_بیرون_Flags = flags | SQLITE_OPEN_حافظه;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  أرجع SQLITE_OK;
}

#إذا 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساكن صحيح memdbDelete(sqlite3_vfs *pVfs, ثابت حرف *z_مسیر, صحيح dirSync){
  أرجع SQLITE_IOERR_DELETE;
}
#پایان_اگر

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
ساكن صحيح memdbAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_مسیر, 
  صحيح flags, 
  صحيح *p_Res_بیرون
){
  استفادهنشده_PARAMETER(pVfs);
  استفادهنشده_PARAMETER(z_مسیر);
  استفادهنشده_PARAMETER(flags);
  *p_Res_بیرون = 0;
  أرجع SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح memdb_پر_Pathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_مسیر, 
  صحيح n_بیرون, 
  حرف *z_بیرون
){
  استفادهنشده_PARAMETER(pVfs);
  sqlite3_snprintf(n_بیرون, z_بیرون, "%s", z_مسیر);
  أرجع SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *memdbDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_مسیر){
  أرجع ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), z_مسیر);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساكن فراغ memdbDlError(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Err_پیام){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_پیام);
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
ساكن صحيح memdbRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Buf_بیرون){
  أرجع ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, z_Buf_بیرون);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساكن صحيح memdbSleep(sqlite3_vfs *pVfs, صحيح nMicro){
  أرجع ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#إذا 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساكن صحيح memdb_Current_زمان(sqlite3_vfs *pVfs, مزدوج *p_زمان_بیرون){
  أرجع ORIGVFS(pVfs)->x_Current_زمان(ORIGVFS(pVfs), p_زمان_بیرون);
}
#پایان_اگر

ساكن صحيح memdb_Get_آخرین_Error(sqlite3_vfs *pVfs, صحيح a, حرف *b){
  أرجع ORIGVFS(pVfs)->x_Get_آخرین_Error(ORIGVFS(pVfs), a, b);
}
ساكن صحيح memdb_Current_زمان_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  أرجع ORIGVFS(pVfs)->x_Current_زمان_Int64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
ساكن MemFile *memdbFromDbSchema(sqlite3 *db, ثابت حرف *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  صحيح rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_اشاره_گر, &p);
  إذا( rc ) أرجع 0;
  إذا( p->پایه.pMethods!=&memdb_io_methods ) أرجع 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  إذا( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  أرجع p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
بیعلامت حرف *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  ثابت حرف *zSchema,      /* Which database within the connection */
  sqlite3_int64 *pi_اندازه,    /* Write size here, if not NULL */
  بیعلامت صحيح mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  صحيح iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  صحيح sz_صفحه = 0;
  sqlite3_stmt *pStmt = 0;
  بیعلامت حرف *p_بیرون = 0;
  حرف *zSql;
  صحيح rc;

#اگر_تعریف_شده SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#پایان_اگر
  sqlite3_mutex_enter(db->mutex);

  إذا( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3_پیدا_کن_Db_Name(db, zSchema);
  إذا( pi_اندازه ) *pi_اندازه = -1;
  إذا( iDb<0 ) اذهب serialize_بیرون;
  إذا( p ){
    MemStore *pStore = p->pStore;
    أكد( pStore->pMutex==0 );
    إذا( pi_اندازه ) *pi_اندازه = pStore->sz;
    إذا( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_بیرون = pStore->a_داده;
    }وإلا{
      p_بیرون = sqlite3_malloc64( pStore->sz );
      إذا( p_بیرون ) کپی_حافظه(p_بیرون, pStore->a_داده, pStore->sz);
    }
    اذهب serialize_بیرون;
  }
  pBt = db->aDb[iDb].pBt;
  إذا( pBt==0 ) اذهب serialize_بیرون;
  sz_صفحه = sqlite3_Btree_Get_صفحه_اندازه(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  إذا( rc ) اذهب serialize_بیرون;
  rc = sqlite3_گام(pStmt);
  إذا( rc==SQLITE_ردیف ){
    sz = sqlite3_ستونی_int64(pStmt, 0)*sz_صفحه;
    إذا( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_گام(pStmt);
      إذا( rc==SQLITE_ردیف ){
        sz = sqlite3_ستونی_int64(pStmt, 0)*sz_صفحه;
      }
    }
    إذا( pi_اندازه ) *pi_اندازه = sz;
    إذا( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_بیرون = 0;
    }وإلا{
      p_بیرون = sqlite3_malloc64( sz );
      إذا( p_بیرون ){
        صحيح n_صفحه = sqlite3_ستونی_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        صحيح pgno;
        لكل(pgno=1; pgno<=n_صفحه; pgno++){
          Db_صفحه *p_صفحه = 0;
          بیعلامت حرف *p_مقصد = p_بیرون + sz_صفحه*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (Db_صفحه**)&p_صفحه, 0);
          إذا( rc==SQLITE_OK ){
            کپی_حافظه(p_مقصد, sqlite3_Pager_Get_داده(p_صفحه), sz_صفحه);
          }وإلا إذا( pgno==PENDING_BYTE/sz_صفحه+1 ){
            memset(p_مقصد, 0, sz_صفحه);
          }وإلا{
            sqlite3_free(p_بیرون);
            p_بیرون = 0;
            n_صفحه = 0;            
          }
          sqlite3PagerUnref(p_صفحه);       
        }
      }
    }
  }
  sqlite3_finalize(pStmt);

 serialize_بیرون:
  sqlite3_mutex_leave(db->mutex);
  أرجع p_بیرون;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
صحيح sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  ثابت حرف *zSchema,    /* Which DB to reopen with the deserialization */
  بیعلامت حرف *p_داده,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  بیعلامت mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  حرف *zSql;
  sqlite3_stmt *pStmt = 0;
  صحيح rc;
  صحيح iDb;

#اگر_تعریف_شده SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) ){
    أرجع SQLITE_MISUSE_BKPT;
  }
  إذا( szDb<0 ) أرجع SQLITE_MISUSE_BKPT;
  إذا( szBuf<0 ) أرجع SQLITE_MISUSE_BKPT;
#پایان_اگر

  sqlite3_mutex_enter(db->mutex);
  إذا( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3_پیدا_کن_Db_Name(db, zSchema);
  testcase( iDb==1 );
  إذا( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    اذهب انتها_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  إذا( zSql==0 ){
    rc = SQLITE_NOMEM;
  }وإلا{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  إذا( rc ) اذهب انتها_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_گام(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  إذا( rc!=SQLITE_OK ){
    اذهب انتها_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  إذا( p==0 ){
    rc = SQLITE_ERROR;
  }وإلا{
    MemStore *pStore = p->pStore;
    pStore->a_داده = p_داده;
    p_داده = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    إذا( pStore->szMax<sqlite3GlobalConfig.mx_Memdb_اندازه ){
      pStore->szMax = sqlite3GlobalConfig.mx_Memdb_اندازه;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

انتها_deserialize:
  إذا( p_داده && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(p_داده);
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
  sqlite3_vfs *p_پایینی = sqlite3_vfs_پیدا_کن(0);
  بیعلامت صحيح sz;
  إذا( NEVER(p_پایینی==0) ) أرجع SQLITE_ERROR;
  sz = p_پایینی->szOsFile;
  memdb_vfs.p_App_داده = p_پایینی;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  إذا( sz<حجم(MemFile) ) sz = حجم(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  أرجع sqlite3_vfs_ثبات(&memdb_vfs, 0);
}
#پایان_اگر /* SQLITE_OMIT_DESERIALIZE */
