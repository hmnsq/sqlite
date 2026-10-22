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
#شامل "sqliteInt.h"
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
قسمتعریف ساخت sqlite3_vfs MemVfs;
قسمتعریف ساخت MemFile MemFile;
قسمتعریف ساخت MemStore MemStore;

/* Access to a lower-level VFS that (might) implement dynamic loading,
** access to randomness, etc.
*/
#تعریف_کرو ORIGVFS(p) ((sqlite3_vfs*)((p)->p_App_ڈیٹا))

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
ساخت MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  غیرعلامتی حرف *a_ڈیٹا;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  صحیح nMmap;                      /* Number of memory mapped pages */
  غیرعلامتی mFlags;                /* Flags */
  صحیح n_Rd_تالا;                    /* Number of readers */
  صحیح n_Wr_تالا;                    /* Number of writers.  (Always 0 or 1) */
  صحیح nRef;                       /* Number of users of this MemStore */
  حرف *zFName;                   /* The filename for shared stores */
};

/* An open file */
ساخت MemFile {
  sqlite3_file بنیاد;              /* IO methods */
  MemStore *pStore;               /* The storage */
  صحیح e_تالا;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
ساکن ساخت MemFS {
  صحیح nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
ساکن صحیح memdbClose(sqlite3_file*);
ساکن صحیح memdbRead(sqlite3_file*, خالی*, صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح memdbWrite(sqlite3_file*,ثابت خالی*,صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح memdbTruncate(sqlite3_file*, sqlite3_int64 سائز);
ساکن صحیح memdbSync(sqlite3_file*, صحیح flags);
ساکن صحیح memdb_File_سائز(sqlite3_file*, sqlite3_int64 *p_سائز);
ساکن صحیح memdb_تالا(sqlite3_file*, صحیح);
ساکن صحیح memdbUnlock(sqlite3_file*, صحیح);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
ساکن صحیح memdbFileControl(sqlite3_file*, صحیح op, خالی *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
ساکن صحیح memdbDeviceCharacteristics(sqlite3_file*);
ساکن صحیح memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, صحیح iAmt, خالی **pp);
ساکن صحیح memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, خالی *p);

/*
** Methods for MemVfs
*/
ساکن صحیح memdbOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحیح , صحیح *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
ساکن صحیح memdbAccess(sqlite3_vfs*, ثابت حرف *zName, صحیح flags, صحیح *);
ساکن صحیح memdb_بھرا_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحیح, حرف *z_برآمد);
ساکن خالی *memdbDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساکن خالی memdbDlError(sqlite3_vfs*, صحیح nByte, حرف *z_Err_پیغام);
ساکن خالی (*memdbDlSym(sqlite3_vfs *pVfs, خالی *p, ثابت حرف*zSym))(خالی);
ساکن خالی memdbDlClose(sqlite3_vfs*, خالی*);
ساکن صحیح memdbRandomness(sqlite3_vfs*, صحیح nByte, حرف *z_برآمد);
ساکن صحیح memdbSleep(sqlite3_vfs*, صحیح microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
ساکن صحیح memdb_Get_آخری_Error(sqlite3_vfs*, صحیح, حرف *);
ساکن صحیح memdb_Current_وقت_Int64(sqlite3_vfs*, sqlite3_int64*);

ساکن sqlite3_vfs memdb_vfs = {
  2,                           /* iVersion */
  0,                           /* szOsFile (set when registered) */
  1024,                        /* mxPathname */
  0,                           /* pNext */
  "memdb",                     /* zName */
  0,                           /* pAppData (set when registered) */ 
  memdbOpen,                   /* xOpen */
  0, /* memdbDelete, */        /* xDelete */
  memdbAccess,                 /* xAccess */
  memdb_بھرا_Pathname,           /* xFullPathname */
  memdbDlOpen,                 /* xDlOpen */
  memdbDlError,                /* xDlError */
  memdbDlSym,                  /* xDlSym */
  memdbDlClose,                /* xDlClose */
  memdbRandomness,             /* xRandomness */
  memdbSleep,                  /* xSleep */
  0, /* memdbCurrentTime, */   /* xCurrentTime */
  memdb_Get_آخری_Error,           /* xGetLastError */
  memdb_Current_وقت_Int64,       /* xCurrentTimeInt64 */
  0,                           /* xSetSystemCall */
  0,                           /* xGetSystemCall */
  0,                           /* xNextSystemCall */
};

ساکن ثابت sqlite3_io_methods memdb_io_methods = {
  3,                              /* iVersion */
  memdbClose,                      /* xClose */
  memdbRead,                       /* xRead */
  memdbWrite,                      /* xWrite */
  memdbTruncate,                   /* xTruncate */
  memdbSync,                       /* xSync */
  memdb_File_سائز,                   /* xFileSize */
  memdb_تالا,                       /* xLock */
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
#اگر defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
ساکن خالی memdbEnter(MemStore *p){
  غیر_مستعمل_PARAMETER(p);
}
ساکن خالی memdbLeave(MemStore *p){
  غیر_مستعمل_PARAMETER(p);
}
#ورنہ
ساکن خالی memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
ساکن خالی memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#اگر_ختم



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
ساکن صحیح memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  اگر( p->zFName ){
    صحیح i;
#اگر_غیر_تعریف_شدہ SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#اگر_ختم
    sqlite3_mutex_enter(pVfsMutex);
    برائے(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      اگر( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        اگر( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          اگر( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        توڑو;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }ورنہ{
    memdbEnter(p);
  }
  p->nRef--;
  اگر( p->nRef<=0 ){
    اگر( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->a_ڈیٹا);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }ورنہ{
    memdbLeave(p);
  }
  واپس SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
ساکن صحیح memdbRead(
  sqlite3_file *pFile, 
  خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  اگر( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    اگر( iOfst<p->sz ) حافظہ_نقل(zBuf, p->a_ڈیٹا+iOfst, p->sz - iOfst);
    memdbLeave(p);
    واپس SQLITE_IOERR_SHORT_READ;
  }
  حافظہ_نقل(zBuf, p->a_ڈیٹا+iOfst, iAmt);
  memdbLeave(p);
  واپس SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
ساکن صحیح memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  غیرعلامتی حرف *pNew;
  اگر( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    واپس SQLITE_بھرا;
  }
  اگر( newSz>p->szMax ){
    واپس SQLITE_بھرا;
  }
  newSz *= 2;
  اگر( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->a_ڈیٹا, newSz);
  اگر( pNew==0 ) واپس SQLITE_IOERR_NOMEM;
  p->a_ڈیٹا = pNew;
  p->szAlloc = newSz;
  واپس SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
ساکن صحیح memdbWrite(
  sqlite3_file *pFile,
  ثابت خالی *z,
  صحیح iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  اگر( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    واپس SQLITE_IOERR_WRITE;
  }
  اگر( iOfst+iAmt>p->sz ){
    صحیح rc;
    اگر( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      واپس rc;
    }
    اگر( iOfst>p->sz ) memset(p->a_ڈیٹا+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  حافظہ_نقل(p->a_ڈیٹا+iOfst, z, iAmt);
  memdbLeave(p);
  واپس SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
ساکن صحیح memdbTruncate(sqlite3_file *pFile, sqlite_int64 سائز){
  MemStore *p = ((MemFile*)pFile)->pStore;
  صحیح rc = SQLITE_OK;
  memdbEnter(p);
  اگر( سائز>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }ورنہ{
    p->sz = سائز; 
  }
  memdbLeave(p);
  واپس rc;
}

/*
** Sync an memdb-file.
*/
ساکن صحیح memdbSync(sqlite3_file *pFile, صحیح flags){
  غیر_مستعمل_PARAMETER(pFile);
  غیر_مستعمل_PARAMETER(flags);
  واپس SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
ساکن صحیح memdb_File_سائز(sqlite3_file *pFile, sqlite_int64 *p_سائز){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *p_سائز = p->sz;
  memdbLeave(p);
  واپس SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
ساکن صحیح memdb_تالا(sqlite3_file *pFile, صحیح e_تالا){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  صحیح rc = SQLITE_OK;
  اگر( e_تالا<=pThis->e_تالا ) واپس SQLITE_OK;
  memdbEnter(p);

  أكد( p->n_Wr_تالا==0 || p->n_Wr_تالا==1 );
  أكد( pThis->e_تالا<=SQLITE_تالا_SHARED || p->n_Wr_تالا==1 );
  أكد( pThis->e_تالا==SQLITE_تالا_NONE || p->n_Rd_تالا>=1 );

  اگر( e_تالا>SQLITE_تالا_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }ورنہ{
    انتخاب( e_تالا ){
      حالت SQLITE_تالا_SHARED: {
        أكد( pThis->e_تالا==SQLITE_تالا_NONE );
        اگر( p->n_Wr_تالا>0 ){
          rc = SQLITE_BUSY;
        }ورنہ{
          p->n_Rd_تالا++;
        }
        توڑو;
      };
  
      حالت SQLITE_تالا_RESERVED:
      حالت SQLITE_تالا_PENDING: {
        أكد( pThis->e_تالا>=SQLITE_تالا_SHARED );
        اگر( ALWAYS(pThis->e_تالا==SQLITE_تالا_SHARED) ){
          اگر( p->n_Wr_تالا>0 ){
            rc = SQLITE_BUSY;
          }ورنہ{
            p->n_Wr_تالا = 1;
          }
        }
        توڑو;
      }
  
      طےشدہ: {
        أكد(  e_تالا==SQLITE_تالا_EXCLUSIVE );
        أكد( pThis->e_تالا>=SQLITE_تالا_SHARED );
        اگر( p->n_Rd_تالا>1 ){
          rc = SQLITE_BUSY;
        }ورنہ اگر( pThis->e_تالا==SQLITE_تالا_SHARED ){
          p->n_Wr_تالا = 1;
        }
        توڑو;
      }
    }
  }
  اگر( rc==SQLITE_OK ) pThis->e_تالا = e_تالا;
  memdbLeave(p);
  واپس rc;
}

/*
** Unlock an memdb-file.
*/
ساکن صحیح memdbUnlock(sqlite3_file *pFile, صحیح e_تالا){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  اگر( e_تالا>=pThis->e_تالا ) واپس SQLITE_OK;
  memdbEnter(p);

  أكد( e_تالا==SQLITE_تالا_SHARED || e_تالا==SQLITE_تالا_NONE );
  اگر( e_تالا==SQLITE_تالا_SHARED ){
    اگر( ALWAYS(pThis->e_تالا>SQLITE_تالا_SHARED) ){
      p->n_Wr_تالا--;
    }
  }ورنہ{
    اگر( pThis->e_تالا>SQLITE_تالا_SHARED ){
      p->n_Wr_تالا--;
    }
    p->n_Rd_تالا--;
  }

  pThis->e_تالا = e_تالا;
  memdbLeave(p);
  واپس SQLITE_OK;
}

#اگر 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
ساکن صحیح memdb_Check_Reserved_تالا(sqlite3_file *pFile, صحیح *p_Res_برآمد){
  *p_Res_برآمد = 0;
  واپس SQLITE_OK;
}
#اگر_ختم


/*
** File control method. For custom operations on an memdb-file.
*/
ساکن صحیح memdbFileControl(sqlite3_file *pFile, صحیح op, خالی *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  صحیح rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  اگر( op==SQLITE_FCNTL_VFSNAME ){
    *(حرف**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->a_ڈیٹا, p->sz);
    rc = SQLITE_OK;
  }
  اگر( op==SQLITE_FCNTL_سائز_حد_بندی ){
    sqlite3_int64 i_حد_بندی = *(sqlite3_int64*)pArg;
    اگر( i_حد_بندی<p->sz ){
      اگر( i_حد_بندی<0 ){
        i_حد_بندی = p->szMax;
      }ورنہ{
        i_حد_بندی = p->sz;
      }
    }
    p->szMax = i_حد_بندی;
    *(sqlite3_int64*)pArg = i_حد_بندی;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  واپس rc;
}

#اگر 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
ساکن صحیح memdb_Sector_سائز(sqlite3_file *pFile){
  واپس 1024;
}
#اگر_ختم

/*
** Return the device characteristic flags supported by an memdb-file.
*/
ساکن صحیح memdbDeviceCharacteristics(sqlite3_file *pFile){
  غیر_مستعمل_PARAMETER(pFile);
  واپس SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_محفوظ_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
ساکن صحیح memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  صحیح iAmt,
  خالی **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  اگر( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }ورنہ{
    p->nMmap++;
    *pp = (خالی*)(p->a_ڈیٹا + iOfst);
  }
  memdbLeave(p);
  واپس SQLITE_OK;
}

/* Release a memory-mapped page */
ساکن صحیح memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, خالی *p_صفحہ){
  MemStore *p = ((MemFile*)pFile)->pStore;
  غیر_مستعمل_PARAMETER(iOfst);
  غیر_مستعمل_PARAMETER(p_صفحہ);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  واپس SQLITE_OK;
}

/*
** Open an mem file handle.
*/
ساکن صحیح memdbOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFd,
  صحیح flags,
  صحیح *p_برآمد_Flags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  صحیح szName;
  غیر_مستعمل_PARAMETER(pVfs);

  memset(pFile, 0, حجم(*pFile));
  szName = sqlite3Strlen30(zName);
  اگر( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    صحیح i;
#اگر_غیر_تعریف_شدہ SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#اگر_ختم
    sqlite3_mutex_enter(pVfsMutex);
    برائے(i=0; i<memdb_g.nMemStore; i++){
      اگر( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        توڑو;
      }
    }
    اگر( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( حجم(*p) + (i64)szName + 3 );
      اگر( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        واپس SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             حجم(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      اگر( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        واپس SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, حجم(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mx_Memdb_سائز;
      p->zFName = (حرف*)&p[1];
      حافظہ_نقل(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      اگر( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        واپس SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }ورنہ{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }ورنہ{
    p = sqlite3Malloc( حجم(*p) );
    اگر( p==0 ){
      واپس SQLITE_NOMEM;
    }
    memset(p, 0, حجم(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mx_Memdb_سائز;
  }
  pFile->pStore = p;
  اگر( p_برآمد_Flags!=0 ){
    *p_برآمد_Flags = flags | SQLITE_OPEN_حافظہ;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  واپس SQLITE_OK;
}

#اگر 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساکن صحیح memdbDelete(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ, صحیح dirSync){
  واپس SQLITE_IOERR_DELETE;
}
#اگر_ختم

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
ساکن صحیح memdbAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_راستہ, 
  صحیح flags, 
  صحیح *p_Res_برآمد
){
  غیر_مستعمل_PARAMETER(pVfs);
  غیر_مستعمل_PARAMETER(z_راستہ);
  غیر_مستعمل_PARAMETER(flags);
  *p_Res_برآمد = 0;
  واپس SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
ساکن صحیح memdb_بھرا_Pathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_راستہ, 
  صحیح n_برآمد, 
  حرف *z_برآمد
){
  غیر_مستعمل_PARAMETER(pVfs);
  sqlite3_snprintf(n_برآمد, z_برآمد, "%s", z_راستہ);
  واپس SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
ساکن خالی *memdbDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ){
  واپس ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), z_راستہ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساکن خالی memdbDlError(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Err_پیغام){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_پیغام);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ساکن خالی (*memdbDlSym(sqlite3_vfs *pVfs, خالی *p, ثابت حرف *zSym))(خالی){
  واپس ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ساکن خالی memdbDlClose(sqlite3_vfs *pVfs, خالی *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساکن صحیح memdbRandomness(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Buf_برآمد){
  واپس ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, z_Buf_برآمد);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساکن صحیح memdbSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  واپس ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#اگر 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساکن صحیح memdb_Current_وقت(sqlite3_vfs *pVfs, دوہرا *p_وقت_برآمد){
  واپس ORIGVFS(pVfs)->x_Current_وقت(ORIGVFS(pVfs), p_وقت_برآمد);
}
#اگر_ختم

ساکن صحیح memdb_Get_آخری_Error(sqlite3_vfs *pVfs, صحیح a, حرف *b){
  واپس ORIGVFS(pVfs)->x_Get_آخری_Error(ORIGVFS(pVfs), a, b);
}
ساکن صحیح memdb_Current_وقت_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  واپس ORIGVFS(pVfs)->x_Current_وقت_Int64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
ساکن MemFile *memdbFromDbSchema(sqlite3 *db, ثابت حرف *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  صحیح rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_اشارہ_گر, &p);
  اگر( rc ) واپس 0;
  اگر( p->بنیاد.pMethods!=&memdb_io_methods ) واپس 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  اگر( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  واپس p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
غیرعلامتی حرف *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  ثابت حرف *zSchema,      /* Which database within the connection */
  sqlite3_int64 *pi_سائز,    /* Write size here, if not NULL */
  غیرعلامتی صحیح mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  صحیح iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  صحیح sz_صفحہ = 0;
  sqlite3_stmt *pStmt = 0;
  غیرعلامتی حرف *p_برآمد = 0;
  حرف *zSql;
  صحیح rc;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    (خالی)SQLITE_MISUSE_BKPT;
    واپس 0;
  }
#اگر_ختم
  sqlite3_mutex_enter(db->mutex);

  اگر( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3_ڈھونڈو_Db_Name(db, zSchema);
  اگر( pi_سائز ) *pi_سائز = -1;
  اگر( iDb<0 ) جاؤ serialize_برآمد;
  اگر( p ){
    MemStore *pStore = p->pStore;
    أكد( pStore->pMutex==0 );
    اگر( pi_سائز ) *pi_سائز = pStore->sz;
    اگر( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_برآمد = pStore->a_ڈیٹا;
    }ورنہ{
      p_برآمد = sqlite3_malloc64( pStore->sz );
      اگر( p_برآمد ) حافظہ_نقل(p_برآمد, pStore->a_ڈیٹا, pStore->sz);
    }
    جاؤ serialize_برآمد;
  }
  pBt = db->aDb[iDb].pBt;
  اگر( pBt==0 ) جاؤ serialize_برآمد;
  sz_صفحہ = sqlite3_Btree_Get_صفحہ_سائز(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  اگر( rc ) جاؤ serialize_برآمد;
  rc = sqlite3_قدم(pStmt);
  اگر( rc==SQLITE_صف ){
    sz = sqlite3_عمود_int64(pStmt, 0)*sz_صفحہ;
    اگر( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_قدم(pStmt);
      اگر( rc==SQLITE_صف ){
        sz = sqlite3_عمود_int64(pStmt, 0)*sz_صفحہ;
      }
    }
    اگر( pi_سائز ) *pi_سائز = sz;
    اگر( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_برآمد = 0;
    }ورنہ{
      p_برآمد = sqlite3_malloc64( sz );
      اگر( p_برآمد ){
        صحیح n_صفحہ = sqlite3_عمود_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        صحیح pgno;
        برائے(pgno=1; pgno<=n_صفحہ; pgno++){
          Db_صفحہ *p_صفحہ = 0;
          غیرعلامتی حرف *p_مقصد = p_برآمد + sz_صفحہ*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (Db_صفحہ**)&p_صفحہ, 0);
          اگر( rc==SQLITE_OK ){
            حافظہ_نقل(p_مقصد, sqlite3_Pager_Get_ڈیٹا(p_صفحہ), sz_صفحہ);
          }ورنہ اگر( pgno==PENDING_BYTE/sz_صفحہ+1 ){
            memset(p_مقصد, 0, sz_صفحہ);
          }ورنہ{
            sqlite3_free(p_برآمد);
            p_برآمد = 0;
            n_صفحہ = 0;            
          }
          sqlite3PagerUnref(p_صفحہ);       
        }
      }
    }
  }
  sqlite3_finalize(pStmt);

 serialize_برآمد:
  sqlite3_mutex_leave(db->mutex);
  واپس p_برآمد;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
صحیح sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  ثابت حرف *zSchema,    /* Which DB to reopen with the deserialization */
  غیرعلامتی حرف *p_ڈیٹا,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  غیرعلامتی mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  حرف *zSql;
  sqlite3_stmt *pStmt = 0;
  صحیح rc;
  صحیح iDb;

#اگر_تعریف_شدہ SQLITE_ENABLE_API_ARMOR
  اگر( !sqlite3SafetyCheckOk(db) ){
    واپس SQLITE_MISUSE_BKPT;
  }
  اگر( szDb<0 ) واپس SQLITE_MISUSE_BKPT;
  اگر( szBuf<0 ) واپس SQLITE_MISUSE_BKPT;
#اگر_ختم

  sqlite3_mutex_enter(db->mutex);
  اگر( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3_ڈھونڈو_Db_Name(db, zSchema);
  testcase( iDb==1 );
  اگر( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    جاؤ انتہا_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  اگر( zSql==0 ){
    rc = SQLITE_NOMEM;
  }ورنہ{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  اگر( rc ) جاؤ انتہا_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_قدم(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  اگر( rc!=SQLITE_OK ){
    جاؤ انتہا_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  اگر( p==0 ){
    rc = SQLITE_ERROR;
  }ورنہ{
    MemStore *pStore = p->pStore;
    pStore->a_ڈیٹا = p_ڈیٹا;
    p_ڈیٹا = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    اگر( pStore->szMax<sqlite3GlobalConfig.mx_Memdb_سائز ){
      pStore->szMax = sqlite3GlobalConfig.mx_Memdb_سائز;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

انتہا_deserialize:
  اگر( p_ڈیٹا && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(p_ڈیٹا);
  }
  sqlite3_mutex_leave(db->mutex);
  واپس rc;
}

/*
** Return true if the VFS is the memvfs.
*/
صحیح sqlite3IsMemdb(ثابت sqlite3_vfs *pVfs){
  واپس pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
صحیح sqlite3MemdbInit(خالی){
  sqlite3_vfs *p_زیریں = sqlite3_vfs_ڈھونڈو(0);
  غیرعلامتی صحیح sz;
  اگر( NEVER(p_زیریں==0) ) واپس SQLITE_ERROR;
  sz = p_زیریں->szOsFile;
  memdb_vfs.p_App_ڈیٹا = p_زیریں;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  اگر( sz<حجم(MemFile) ) sz = حجم(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  واپس sqlite3_vfs_رجسٹر(&memdb_vfs, 0);
}
#اگر_ختم /* SQLITE_OMIT_DESERIALIZE */
