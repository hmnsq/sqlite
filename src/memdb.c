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
#جيڪڏهن_وصف_نه_ٿيل SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
تعريفنوع بنية sqlite3_vfs MemVfs;
تعريفنوع بنية MemFile MemFile;
تعريفنوع بنية MemStore MemStore;

/* Access to a lower-level VFS that (might) implement dynamic loading,
** access to randomness, etc.
*/
#وصف_ڪريو ORIGVFS(p) ((sqlite3_vfs*)((p)->p_App_معلومات))

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
  بنانشان حرف *a_معلومات;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  صحيح nMmap;                      /* Number of memory mapped pages */
  بنانشان mFlags;                /* Flags */
  صحيح n_Rd_تالو;                    /* Number of readers */
  صحيح n_Wr_تالو;                    /* Number of writers.  (Always 0 or 1) */
  صحيح nRef;                       /* Number of users of this MemStore */
  حرف *zFName;                   /* The filename for shared stores */
};

/* An open file */
بنية MemFile {
  sqlite3_file بنياد;              /* IO methods */
  MemStore *pStore;               /* The storage */
  صحيح e_تالو;                      /* Most recent lock against this file */
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
ساكن صحيح memdbTruncate(sqlite3_file*, sqlite3_int64 سائيز);
ساكن صحيح memdbSync(sqlite3_file*, صحيح flags);
ساكن صحيح memdb_File_سائيز(sqlite3_file*, sqlite3_int64 *p_سائيز);
ساكن صحيح memdb_تالو(sqlite3_file*, صحيح);
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
ساكن صحيح memdb_ڀريل_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحيح, حرف *z_ٻاهريون);
ساكن فراغ *memdbDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن فراغ memdbDlError(sqlite3_vfs*, صحيح nByte, حرف *z_Err_نياپو);
ساكن فراغ (*memdbDlSym(sqlite3_vfs *pVfs, فراغ *p, ثابت حرف*zSym))(فراغ);
ساكن فراغ memdbDlClose(sqlite3_vfs*, فراغ*);
ساكن صحيح memdbRandomness(sqlite3_vfs*, صحيح nByte, حرف *z_ٻاهريون);
ساكن صحيح memdbSleep(sqlite3_vfs*, صحيح microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
ساكن صحيح memdb_Get_آخري_Error(sqlite3_vfs*, صحيح, حرف *);
ساكن صحيح memdb_Current_وقت_Int64(sqlite3_vfs*, sqlite3_int64*);

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
  memdb_ڀريل_Pathname,           /* xFullPathname */
  memdbDlOpen,                 /* xDlOpen */
  memdbDlError,                /* xDlError */
  memdbDlSym,                  /* xDlSym */
  memdbDlClose,                /* xDlClose */
  memdbRandomness,             /* xRandomness */
  memdbSleep,                  /* xSleep */
  0, /* memdbCurrentTime, */   /* xCurrentTime */
  memdb_Get_آخري_Error,           /* xGetLastError */
  memdb_Current_وقت_Int64,       /* xCurrentTimeInt64 */
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
  memdb_File_سائيز,                   /* xFileSize */
  memdb_تالو,                       /* xLock */
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
  اڻ_استعمال_ٿيل_PARAMETER(p);
}
ساكن فراغ memdbLeave(MemStore *p){
  اڻ_استعمال_ٿيل_PARAMETER(p);
}
#وإلا
ساكن فراغ memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
ساكن فراغ memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#جيڪڏهن_پورو



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
ساكن صحيح memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  إذا( p->zFName ){
    صحيح i;
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#جيڪڏهن_پورو
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
      sqlite3_free(p->a_معلومات);
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
    إذا( iOfst<p->sz ) يادگيري_نقل(zBuf, p->a_معلومات+iOfst, p->sz - iOfst);
    memdbLeave(p);
    أرجع SQLITE_IOERR_SHORT_READ;
  }
  يادگيري_نقل(zBuf, p->a_معلومات+iOfst, iAmt);
  memdbLeave(p);
  أرجع SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
ساكن صحيح memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  بنانشان حرف *pNew;
  إذا( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    أرجع SQLITE_ڀريل;
  }
  إذا( newSz>p->szMax ){
    أرجع SQLITE_ڀريل;
  }
  newSz *= 2;
  إذا( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->a_معلومات, newSz);
  إذا( pNew==0 ) أرجع SQLITE_IOERR_NOMEM;
  p->a_معلومات = pNew;
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
    إذا( iOfst>p->sz ) memset(p->a_معلومات+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  يادگيري_نقل(p->a_معلومات+iOfst, z, iAmt);
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
ساكن صحيح memdbTruncate(sqlite3_file *pFile, sqlite_int64 سائيز){
  MemStore *p = ((MemFile*)pFile)->pStore;
  صحيح rc = SQLITE_OK;
  memdbEnter(p);
  إذا( سائيز>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }وإلا{
    p->sz = سائيز; 
  }
  memdbLeave(p);
  أرجع rc;
}

/*
** Sync an memdb-file.
*/
ساكن صحيح memdbSync(sqlite3_file *pFile, صحيح flags){
  اڻ_استعمال_ٿيل_PARAMETER(pFile);
  اڻ_استعمال_ٿيل_PARAMETER(flags);
  أرجع SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
ساكن صحيح memdb_File_سائيز(sqlite3_file *pFile, sqlite_int64 *p_سائيز){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *p_سائيز = p->sz;
  memdbLeave(p);
  أرجع SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
ساكن صحيح memdb_تالو(sqlite3_file *pFile, صحيح e_تالو){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  صحيح rc = SQLITE_OK;
  إذا( e_تالو<=pThis->e_تالو ) أرجع SQLITE_OK;
  memdbEnter(p);

  أكد( p->n_Wr_تالو==0 || p->n_Wr_تالو==1 );
  أكد( pThis->e_تالو<=SQLITE_تالو_SHARED || p->n_Wr_تالو==1 );
  أكد( pThis->e_تالو==SQLITE_تالو_NONE || p->n_Rd_تالو>=1 );

  إذا( e_تالو>SQLITE_تالو_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }وإلا{
    بدل( e_تالو ){
      حالة SQLITE_تالو_SHARED: {
        أكد( pThis->e_تالو==SQLITE_تالو_NONE );
        إذا( p->n_Wr_تالو>0 ){
          rc = SQLITE_BUSY;
        }وإلا{
          p->n_Rd_تالو++;
        }
        اكسر;
      };
  
      حالة SQLITE_تالو_RESERVED:
      حالة SQLITE_تالو_PENDING: {
        أكد( pThis->e_تالو>=SQLITE_تالو_SHARED );
        إذا( ALWAYS(pThis->e_تالو==SQLITE_تالو_SHARED) ){
          إذا( p->n_Wr_تالو>0 ){
            rc = SQLITE_BUSY;
          }وإلا{
            p->n_Wr_تالو = 1;
          }
        }
        اكسر;
      }
  
      افتراضي: {
        أكد(  e_تالو==SQLITE_تالو_EXCLUSIVE );
        أكد( pThis->e_تالو>=SQLITE_تالو_SHARED );
        إذا( p->n_Rd_تالو>1 ){
          rc = SQLITE_BUSY;
        }وإلا إذا( pThis->e_تالو==SQLITE_تالو_SHARED ){
          p->n_Wr_تالو = 1;
        }
        اكسر;
      }
    }
  }
  إذا( rc==SQLITE_OK ) pThis->e_تالو = e_تالو;
  memdbLeave(p);
  أرجع rc;
}

/*
** Unlock an memdb-file.
*/
ساكن صحيح memdbUnlock(sqlite3_file *pFile, صحيح e_تالو){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  إذا( e_تالو>=pThis->e_تالو ) أرجع SQLITE_OK;
  memdbEnter(p);

  أكد( e_تالو==SQLITE_تالو_SHARED || e_تالو==SQLITE_تالو_NONE );
  إذا( e_تالو==SQLITE_تالو_SHARED ){
    إذا( ALWAYS(pThis->e_تالو>SQLITE_تالو_SHARED) ){
      p->n_Wr_تالو--;
    }
  }وإلا{
    إذا( pThis->e_تالو>SQLITE_تالو_SHARED ){
      p->n_Wr_تالو--;
    }
    p->n_Rd_تالو--;
  }

  pThis->e_تالو = e_تالو;
  memdbLeave(p);
  أرجع SQLITE_OK;
}

#إذا 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
ساكن صحيح memdb_Check_Reserved_تالو(sqlite3_file *pFile, صحيح *p_Res_ٻاهريون){
  *p_Res_ٻاهريون = 0;
  أرجع SQLITE_OK;
}
#جيڪڏهن_پورو


/*
** File control method. For custom operations on an memdb-file.
*/
ساكن صحيح memdbFileControl(sqlite3_file *pFile, صحيح op, فراغ *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  صحيح rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  إذا( op==SQLITE_FCNTL_VFSNAME ){
    *(حرف**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->a_معلومات, p->sz);
    rc = SQLITE_OK;
  }
  إذا( op==SQLITE_FCNTL_سائيز_حد_بندي ){
    sqlite3_int64 i_حد_بندي = *(sqlite3_int64*)pArg;
    إذا( i_حد_بندي<p->sz ){
      إذا( i_حد_بندي<0 ){
        i_حد_بندي = p->szMax;
      }وإلا{
        i_حد_بندي = p->sz;
      }
    }
    p->szMax = i_حد_بندي;
    *(sqlite3_int64*)pArg = i_حد_بندي;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  أرجع rc;
}

#إذا 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
ساكن صحيح memdb_Sector_سائيز(sqlite3_file *pFile){
  أرجع 1024;
}
#جيڪڏهن_پورو

/*
** Return the device characteristic flags supported by an memdb-file.
*/
ساكن صحيح memdbDeviceCharacteristics(sqlite3_file *pFile){
  اڻ_استعمال_ٿيل_PARAMETER(pFile);
  أرجع SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_محفوظ_APPEND |
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
    *pp = (فراغ*)(p->a_معلومات + iOfst);
  }
  memdbLeave(p);
  أرجع SQLITE_OK;
}

/* Release a memory-mapped page */
ساكن صحيح memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, فراغ *p_صفحو){
  MemStore *p = ((MemFile*)pFile)->pStore;
  اڻ_استعمال_ٿيل_PARAMETER(iOfst);
  اڻ_استعمال_ٿيل_PARAMETER(p_صفحو);
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
  صحيح *p_ٻاهريون_Flags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  صحيح szName;
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);

  memset(pFile, 0, حجم(*pFile));
  szName = sqlite3Strlen30(zName);
  إذا( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    صحيح i;
#جيڪڏهن_وصف_نه_ٿيل SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#جيڪڏهن_پورو
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
      p->szMax = sqlite3GlobalConfig.mx_Memdb_سائيز;
      p->zFName = (حرف*)&p[1];
      يادگيري_نقل(p->zFName, zName, szName+1);
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
    p->szMax = sqlite3GlobalConfig.mx_Memdb_سائيز;
  }
  pFile->pStore = p;
  إذا( p_ٻاهريون_Flags!=0 ){
    *p_ٻاهريون_Flags = flags | SQLITE_OPEN_يادگيري;
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
ساكن صحيح memdbDelete(sqlite3_vfs *pVfs, ثابت حرف *z_رستو, صحيح dirSync){
  أرجع SQLITE_IOERR_DELETE;
}
#جيڪڏهن_پورو

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
ساكن صحيح memdbAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_رستو, 
  صحيح flags, 
  صحيح *p_Res_ٻاهريون
){
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);
  اڻ_استعمال_ٿيل_PARAMETER(z_رستو);
  اڻ_استعمال_ٿيل_PARAMETER(flags);
  *p_Res_ٻاهريون = 0;
  أرجع SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح memdb_ڀريل_Pathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_رستو, 
  صحيح n_ٻاهريون, 
  حرف *z_ٻاهريون
){
  اڻ_استعمال_ٿيل_PARAMETER(pVfs);
  sqlite3_snprintf(n_ٻاهريون, z_ٻاهريون, "%s", z_رستو);
  أرجع SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *memdbDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_رستو){
  أرجع ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), z_رستو);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساكن فراغ memdbDlError(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Err_نياپو){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_نياپو);
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
ساكن صحيح memdbRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Buf_ٻاهريون){
  أرجع ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, z_Buf_ٻاهريون);
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
ساكن صحيح memdb_Current_وقت(sqlite3_vfs *pVfs, مزدوج *p_وقت_ٻاهريون){
  أرجع ORIGVFS(pVfs)->x_Current_وقت(ORIGVFS(pVfs), p_وقت_ٻاهريون);
}
#جيڪڏهن_پورو

ساكن صحيح memdb_Get_آخري_Error(sqlite3_vfs *pVfs, صحيح a, حرف *b){
  أرجع ORIGVFS(pVfs)->x_Get_آخري_Error(ORIGVFS(pVfs), a, b);
}
ساكن صحيح memdb_Current_وقت_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  أرجع ORIGVFS(pVfs)->x_Current_وقت_Int64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
ساكن MemFile *memdbFromDbSchema(sqlite3 *db, ثابت حرف *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  صحيح rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_اشارو, &p);
  إذا( rc ) أرجع 0;
  إذا( p->بنياد.pMethods!=&memdb_io_methods ) أرجع 0;
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
بنانشان حرف *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  ثابت حرف *zSchema,      /* Which database within the connection */
  sqlite3_int64 *pi_سائيز,    /* Write size here, if not NULL */
  بنانشان صحيح mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  صحيح iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  صحيح sz_صفحو = 0;
  sqlite3_stmt *pStmt = 0;
  بنانشان حرف *p_ٻاهريون = 0;
  حرف *zSql;
  صحيح rc;

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) ){
    (فراغ)SQLITE_MISUSE_BKPT;
    أرجع 0;
  }
#جيڪڏهن_پورو
  sqlite3_mutex_enter(db->mutex);

  إذا( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3_ڳوليو_Db_Name(db, zSchema);
  إذا( pi_سائيز ) *pi_سائيز = -1;
  إذا( iDb<0 ) اذهب serialize_ٻاهريون;
  إذا( p ){
    MemStore *pStore = p->pStore;
    أكد( pStore->pMutex==0 );
    إذا( pi_سائيز ) *pi_سائيز = pStore->sz;
    إذا( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_ٻاهريون = pStore->a_معلومات;
    }وإلا{
      p_ٻاهريون = sqlite3_malloc64( pStore->sz );
      إذا( p_ٻاهريون ) يادگيري_نقل(p_ٻاهريون, pStore->a_معلومات, pStore->sz);
    }
    اذهب serialize_ٻاهريون;
  }
  pBt = db->aDb[iDb].pBt;
  إذا( pBt==0 ) اذهب serialize_ٻاهريون;
  sz_صفحو = sqlite3_Btree_Get_صفحو_سائيز(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  إذا( rc ) اذهب serialize_ٻاهريون;
  rc = sqlite3_قدم(pStmt);
  إذا( rc==SQLITE_صف ){
    sz = sqlite3_ڪالم_int64(pStmt, 0)*sz_صفحو;
    إذا( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_قدم(pStmt);
      إذا( rc==SQLITE_صف ){
        sz = sqlite3_ڪالم_int64(pStmt, 0)*sz_صفحو;
      }
    }
    إذا( pi_سائيز ) *pi_سائيز = sz;
    إذا( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_ٻاهريون = 0;
    }وإلا{
      p_ٻاهريون = sqlite3_malloc64( sz );
      إذا( p_ٻاهريون ){
        صحيح n_صفحو = sqlite3_ڪالم_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        صحيح pgno;
        لكل(pgno=1; pgno<=n_صفحو; pgno++){
          Db_صفحو *p_صفحو = 0;
          بنانشان حرف *p_منزل = p_ٻاهريون + sz_صفحو*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (Db_صفحو**)&p_صفحو, 0);
          إذا( rc==SQLITE_OK ){
            يادگيري_نقل(p_منزل, sqlite3_Pager_Get_معلومات(p_صفحو), sz_صفحو);
          }وإلا إذا( pgno==PENDING_BYTE/sz_صفحو+1 ){
            memset(p_منزل, 0, sz_صفحو);
          }وإلا{
            sqlite3_free(p_ٻاهريون);
            p_ٻاهريون = 0;
            n_صفحو = 0;            
          }
          sqlite3PagerUnref(p_صفحو);       
        }
      }
    }
  }
  sqlite3_finalize(pStmt);

 serialize_ٻاهريون:
  sqlite3_mutex_leave(db->mutex);
  أرجع p_ٻاهريون;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
صحيح sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  ثابت حرف *zSchema,    /* Which DB to reopen with the deserialization */
  بنانشان حرف *p_معلومات,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  بنانشان mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  حرف *zSql;
  sqlite3_stmt *pStmt = 0;
  صحيح rc;
  صحيح iDb;

#جيڪڏهن_وصف_ٿيل SQLITE_ENABLE_API_ARMOR
  إذا( !sqlite3SafetyCheckOk(db) ){
    أرجع SQLITE_MISUSE_BKPT;
  }
  إذا( szDb<0 ) أرجع SQLITE_MISUSE_BKPT;
  إذا( szBuf<0 ) أرجع SQLITE_MISUSE_BKPT;
#جيڪڏهن_پورو

  sqlite3_mutex_enter(db->mutex);
  إذا( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3_ڳوليو_Db_Name(db, zSchema);
  testcase( iDb==1 );
  إذا( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    اذهب پڇاڙي_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  إذا( zSql==0 ){
    rc = SQLITE_NOMEM;
  }وإلا{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  إذا( rc ) اذهب پڇاڙي_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_قدم(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  إذا( rc!=SQLITE_OK ){
    اذهب پڇاڙي_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  إذا( p==0 ){
    rc = SQLITE_ERROR;
  }وإلا{
    MemStore *pStore = p->pStore;
    pStore->a_معلومات = p_معلومات;
    p_معلومات = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    إذا( pStore->szMax<sqlite3GlobalConfig.mx_Memdb_سائيز ){
      pStore->szMax = sqlite3GlobalConfig.mx_Memdb_سائيز;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

پڇاڙي_deserialize:
  إذا( p_معلومات && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(p_معلومات);
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
  sqlite3_vfs *p_هيٺاهون = sqlite3_vfs_ڳوليو(0);
  بنانشان صحيح sz;
  إذا( NEVER(p_هيٺاهون==0) ) أرجع SQLITE_ERROR;
  sz = p_هيٺاهون->szOsFile;
  memdb_vfs.p_App_معلومات = p_هيٺاهون;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  إذا( sz<حجم(MemFile) ) sz = حجم(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  أرجع sqlite3_vfs_رجسٽر(&memdb_vfs, 0);
}
#جيڪڏهن_پورو /* SQLITE_OMIT_DESERIALIZE */
