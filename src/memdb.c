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
#అన్తర్భావ "sqliteInt.h"
#నిర్వచితం_కాకపోతే SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
రకనిర్వచనం నిర్మాణం sqlite3_vfs MemVfs;
రకనిర్వచనం నిర్మాణం MemFile MemFile;
రకనిర్వచనం నిర్మాణం MemStore MemStore;

/* Access to a lower-level VFS that (might) implement dynamic loading,
** access to randomness, etc.
*/
#నిర్వచించు ORIGVFS(p) ((sqlite3_vfs*)((p)->p_App_దత్తాంశం))

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
నిర్మాణం MemStore {
  sqlite3_int64 sz;               /* Size of the file */
  sqlite3_int64 szAlloc;          /* Space allocated to aData */
  sqlite3_int64 szMax;            /* Maximum allowed size of the file */
  గుర్తులేని అక్షరం *a_దత్తాంశం;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  పూర్ణాంకం nMmap;                      /* Number of memory mapped pages */
  గుర్తులేని mFlags;                /* Flags */
  పూర్ణాంకం n_Rd_తాళం;                    /* Number of readers */
  పూర్ణాంకం n_Wr_తాళం;                    /* Number of writers.  (Always 0 or 1) */
  పూర్ణాంకం nRef;                       /* Number of users of this MemStore */
  అక్షరం *zFName;                   /* The filename for shared stores */
};

/* An open file */
నిర్మాణం MemFile {
  sqlite3_file పునాది;              /* IO methods */
  MemStore *pStore;               /* The storage */
  పూర్ణాంకం e_తాళం;                      /* Most recent lock against this file */
};

/*
** File-scope variables for holding the memdb files that are accessible
** to multiple database connections in separate threads.
**
** Must hold SQLITE_MUTEX_STATIC_VFS1 to access any part of this object.
*/
స్థిర నిర్మాణం MemFS {
  పూర్ణాంకం nMemStore;                  /* Number of shared MemStore objects */
  MemStore **apMemStore;          /* Array of all shared MemStore objects */
} memdb_g;

/*
** Methods for MemFile
*/
స్థిర పూర్ణాంకం memdbClose(sqlite3_file*);
స్థిర పూర్ణాంకం memdbRead(sqlite3_file*, శూన్య*, పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం memdbWrite(sqlite3_file*,స్థిరరాశి శూన్య*,పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం memdbTruncate(sqlite3_file*, sqlite3_int64 కొలత);
స్థిర పూర్ణాంకం memdbSync(sqlite3_file*, పూర్ణాంకం flags);
స్థిర పూర్ణాంకం memdb_File_కొలత(sqlite3_file*, sqlite3_int64 *p_కొలత);
స్థిర పూర్ణాంకం memdb_తాళం(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం memdbUnlock(sqlite3_file*, పూర్ణాంకం);
/* static int memdbCheckReservedLock(sqlite3_file*, int *pResOut);// not used */
స్థిర పూర్ణాంకం memdbFileControl(sqlite3_file*, పూర్ణాంకం op, శూన్య *pArg);
/* static int memdbSectorSize(sqlite3_file*); // not used */
స్థిర పూర్ణాంకం memdbDeviceCharacteristics(sqlite3_file*);
స్థిర పూర్ణాంకం memdbFetch(sqlite3_file*, sqlite3_int64 iOfst, పూర్ణాంకం iAmt, శూన్య **pp);
స్థిర పూర్ణాంకం memdbUnfetch(sqlite3_file*, sqlite3_int64 iOfst, శూన్య *p);

/*
** Methods for MemVfs
*/
స్థిర పూర్ణాంకం memdbOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *, sqlite3_file*, పూర్ణాంకం , పూర్ణాంకం *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
స్థిర పూర్ణాంకం memdbAccess(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం flags, పూర్ణాంకం *);
స్థిర పూర్ణాంకం memdb_నిండిన_Pathname(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం, అక్షరం *z_బహిర్గతం);
స్థిర శూన్య *memdbDlOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *zFilename);
స్థిర శూన్య memdbDlError(sqlite3_vfs*, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం);
స్థిర శూన్య (*memdbDlSym(sqlite3_vfs *pVfs, శూన్య *p, స్థిరరాశి అక్షరం*zSym))(శూన్య);
స్థిర శూన్య memdbDlClose(sqlite3_vfs*, శూన్య*);
స్థిర పూర్ణాంకం memdbRandomness(sqlite3_vfs*, పూర్ణాంకం nByte, అక్షరం *z_బహిర్గతం);
స్థిర పూర్ణాంకం memdbSleep(sqlite3_vfs*, పూర్ణాంకం microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
స్థిర పూర్ణాంకం memdb_Get_చివరి_Error(sqlite3_vfs*, పూర్ణాంకం, అక్షరం *);
స్థిర పూర్ణాంకం memdb_Current_సమయం_Int64(sqlite3_vfs*, sqlite3_int64*);

స్థిర sqlite3_vfs memdb_vfs = {
  2,                           /* iVersion */
  0,                           /* szOsFile (set when registered) */
  1024,                        /* mxPathname */
  0,                           /* pNext */
  "memdb",                     /* zName */
  0,                           /* pAppData (set when registered) */ 
  memdbOpen,                   /* xOpen */
  0, /* memdbDelete, */        /* xDelete */
  memdbAccess,                 /* xAccess */
  memdb_నిండిన_Pathname,           /* xFullPathname */
  memdbDlOpen,                 /* xDlOpen */
  memdbDlError,                /* xDlError */
  memdbDlSym,                  /* xDlSym */
  memdbDlClose,                /* xDlClose */
  memdbRandomness,             /* xRandomness */
  memdbSleep,                  /* xSleep */
  0, /* memdbCurrentTime, */   /* xCurrentTime */
  memdb_Get_చివరి_Error,           /* xGetLastError */
  memdb_Current_సమయం_Int64,       /* xCurrentTimeInt64 */
  0,                           /* xSetSystemCall */
  0,                           /* xGetSystemCall */
  0,                           /* xNextSystemCall */
};

స్థిర స్థిరరాశి sqlite3_io_methods memdb_io_methods = {
  3,                              /* iVersion */
  memdbClose,                      /* xClose */
  memdbRead,                       /* xRead */
  memdbWrite,                      /* xWrite */
  memdbTruncate,                   /* xTruncate */
  memdbSync,                       /* xSync */
  memdb_File_కొలత,                   /* xFileSize */
  memdb_తాళం,                       /* xLock */
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
#ఒకవేళ defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
స్థిర శూన్య memdbEnter(MemStore *p){
  వాడని_PARAMETER(p);
}
స్థిర శూన్య memdbLeave(MemStore *p){
  వాడని_PARAMETER(p);
}
#లేకపోతే
స్థిర శూన్య memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
స్థిర శూన్య memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#అయితే_ముగింపు



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
స్థిర పూర్ణాంకం memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  ఒకవేళ( p->zFName ){
    పూర్ణాంకం i;
#నిర్వచితం_కాకపోతే SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#అయితే_ముగింపు
    sqlite3_mutex_enter(pVfsMutex);
    కొరకు(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      ఒకవేళ( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        ఒకవేళ( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          ఒకవేళ( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        ఆపు;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }లేకపోతే{
    memdbEnter(p);
  }
  p->nRef--;
  ఒకవేళ( p->nRef<=0 ){
    ఒకవేళ( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->a_దత్తాంశం);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }లేకపోతే{
    memdbLeave(p);
  }
  తిరిగి SQLITE_OK;
}

/*
** Read data from an memdb-file.
*/
స్థిర పూర్ణాంకం memdbRead(
  sqlite3_file *pFile, 
  శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ఒకవేళ( iOfst+iAmt>p->sz ){
    memset(zBuf, 0, iAmt);
    ఒకవేళ( iOfst<p->sz ) జ్ఞాపకం_నకలు(zBuf, p->a_దత్తాంశం+iOfst, p->sz - iOfst);
    memdbLeave(p);
    తిరిగి SQLITE_IOERR_SHORT_READ;
  }
  జ్ఞాపకం_నకలు(zBuf, p->a_దత్తాంశం+iOfst, iAmt);
  memdbLeave(p);
  తిరిగి SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
స్థిర పూర్ణాంకం memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  గుర్తులేని అక్షరం *pNew;
  ఒకవేళ( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    తిరిగి SQLITE_నిండిన;
  }
  ఒకవేళ( newSz>p->szMax ){
    తిరిగి SQLITE_నిండిన;
  }
  newSz *= 2;
  ఒకవేళ( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->a_దత్తాంశం, newSz);
  ఒకవేళ( pNew==0 ) తిరిగి SQLITE_IOERR_NOMEM;
  p->a_దత్తాంశం = pNew;
  p->szAlloc = newSz;
  తిరిగి SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
స్థిర పూర్ణాంకం memdbWrite(
  sqlite3_file *pFile,
  స్థిరరాశి శూన్య *z,
  పూర్ణాంకం iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ఒకవేళ( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    తిరిగి SQLITE_IOERR_WRITE;
  }
  ఒకవేళ( iOfst+iAmt>p->sz ){
    పూర్ణాంకం rc;
    ఒకవేళ( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      తిరిగి rc;
    }
    ఒకవేళ( iOfst>p->sz ) memset(p->a_దత్తాంశం+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  జ్ఞాపకం_నకలు(p->a_దత్తాంశం+iOfst, z, iAmt);
  memdbLeave(p);
  తిరిగి SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
స్థిర పూర్ణాంకం memdbTruncate(sqlite3_file *pFile, sqlite_int64 కొలత){
  MemStore *p = ((MemFile*)pFile)->pStore;
  పూర్ణాంకం rc = SQLITE_OK;
  memdbEnter(p);
  ఒకవేళ( కొలత>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }లేకపోతే{
    p->sz = కొలత; 
  }
  memdbLeave(p);
  తిరిగి rc;
}

/*
** Sync an memdb-file.
*/
స్థిర పూర్ణాంకం memdbSync(sqlite3_file *pFile, పూర్ణాంకం flags){
  వాడని_PARAMETER(pFile);
  వాడని_PARAMETER(flags);
  తిరిగి SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
స్థిర పూర్ణాంకం memdb_File_కొలత(sqlite3_file *pFile, sqlite_int64 *p_కొలత){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *p_కొలత = p->sz;
  memdbLeave(p);
  తిరిగి SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
స్థిర పూర్ణాంకం memdb_తాళం(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  పూర్ణాంకం rc = SQLITE_OK;
  ఒకవేళ( e_తాళం<=pThis->e_తాళం ) తిరిగి SQLITE_OK;
  memdbEnter(p);

  ప్రతిజ్ఞా( p->n_Wr_తాళం==0 || p->n_Wr_తాళం==1 );
  ప్రతిజ్ఞా( pThis->e_తాళం<=SQLITE_తాళం_SHARED || p->n_Wr_తాళం==1 );
  ప్రతిజ్ఞా( pThis->e_తాళం==SQLITE_తాళం_NONE || p->n_Rd_తాళం>=1 );

  ఒకవేళ( e_తాళం>SQLITE_తాళం_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }లేకపోతే{
    ఎంపిక( e_తాళం ){
      స్థితి SQLITE_తాళం_SHARED: {
        ప్రతిజ్ఞా( pThis->e_తాళం==SQLITE_తాళం_NONE );
        ఒకవేళ( p->n_Wr_తాళం>0 ){
          rc = SQLITE_BUSY;
        }లేకపోతే{
          p->n_Rd_తాళం++;
        }
        ఆపు;
      };
  
      స్థితి SQLITE_తాళం_RESERVED:
      స్థితి SQLITE_తాళం_PENDING: {
        ప్రతిజ్ఞా( pThis->e_తాళం>=SQLITE_తాళం_SHARED );
        ఒకవేళ( ALWAYS(pThis->e_తాళం==SQLITE_తాళం_SHARED) ){
          ఒకవేళ( p->n_Wr_తాళం>0 ){
            rc = SQLITE_BUSY;
          }లేకపోతే{
            p->n_Wr_తాళం = 1;
          }
        }
        ఆపు;
      }
  
      అప్రమేయ: {
        ప్రతిజ్ఞా(  e_తాళం==SQLITE_తాళం_EXCLUSIVE );
        ప్రతిజ్ఞా( pThis->e_తాళం>=SQLITE_తాళం_SHARED );
        ఒకవేళ( p->n_Rd_తాళం>1 ){
          rc = SQLITE_BUSY;
        }లేకపోతే ఒకవేళ( pThis->e_తాళం==SQLITE_తాళం_SHARED ){
          p->n_Wr_తాళం = 1;
        }
        ఆపు;
      }
    }
  }
  ఒకవేళ( rc==SQLITE_OK ) pThis->e_తాళం = e_తాళం;
  memdbLeave(p);
  తిరిగి rc;
}

/*
** Unlock an memdb-file.
*/
స్థిర పూర్ణాంకం memdbUnlock(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  ఒకవేళ( e_తాళం>=pThis->e_తాళం ) తిరిగి SQLITE_OK;
  memdbEnter(p);

  ప్రతిజ్ఞా( e_తాళం==SQLITE_తాళం_SHARED || e_తాళం==SQLITE_తాళం_NONE );
  ఒకవేళ( e_తాళం==SQLITE_తాళం_SHARED ){
    ఒకవేళ( ALWAYS(pThis->e_తాళం>SQLITE_తాళం_SHARED) ){
      p->n_Wr_తాళం--;
    }
  }లేకపోతే{
    ఒకవేళ( pThis->e_తాళం>SQLITE_తాళం_SHARED ){
      p->n_Wr_తాళం--;
    }
    p->n_Rd_తాళం--;
  }

  pThis->e_తాళం = e_తాళం;
  memdbLeave(p);
  తిరిగి SQLITE_OK;
}

#ఒకవేళ 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
స్థిర పూర్ణాంకం memdb_Check_Reserved_తాళం(sqlite3_file *pFile, పూర్ణాంకం *p_Res_బహిర్గతం){
  *p_Res_బహిర్గతం = 0;
  తిరిగి SQLITE_OK;
}
#అయితే_ముగింపు


/*
** File control method. For custom operations on an memdb-file.
*/
స్థిర పూర్ణాంకం memdbFileControl(sqlite3_file *pFile, పూర్ణాంకం op, శూన్య *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  పూర్ణాంకం rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  ఒకవేళ( op==SQLITE_FCNTL_VFSNAME ){
    *(అక్షరం**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->a_దత్తాంశం, p->sz);
    rc = SQLITE_OK;
  }
  ఒకవేళ( op==SQLITE_FCNTL_కొలత_హద్దు ){
    sqlite3_int64 i_హద్దు = *(sqlite3_int64*)pArg;
    ఒకవేళ( i_హద్దు<p->sz ){
      ఒకవేళ( i_హద్దు<0 ){
        i_హద్దు = p->szMax;
      }లేకపోతే{
        i_హద్దు = p->sz;
      }
    }
    p->szMax = i_హద్దు;
    *(sqlite3_int64*)pArg = i_హద్దు;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  తిరిగి rc;
}

#ఒకవేళ 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
స్థిర పూర్ణాంకం memdb_Sector_కొలత(sqlite3_file *pFile){
  తిరిగి 1024;
}
#అయితే_ముగింపు

/*
** Return the device characteristic flags supported by an memdb-file.
*/
స్థిర పూర్ణాంకం memdbDeviceCharacteristics(sqlite3_file *pFile){
  వాడని_PARAMETER(pFile);
  తిరిగి SQLITE_IOCAP_ATOMIC | 
         SQLITE_IOCAP_POWERSAFE_OVERWRITE |
         SQLITE_IOCAP_సురక్షిత_APPEND |
         SQLITE_IOCAP_SEQUENTIAL;
}

/* Fetch a page of a memory-mapped file */
స్థిర పూర్ణాంకం memdbFetch(
  sqlite3_file *pFile,
  sqlite3_int64 iOfst,
  పూర్ణాంకం iAmt,
  శూన్య **pp
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  ఒకవేళ( iOfst+iAmt>p->sz || (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)!=0 ){
    *pp = 0;
  }లేకపోతే{
    p->nMmap++;
    *pp = (శూన్య*)(p->a_దత్తాంశం + iOfst);
  }
  memdbLeave(p);
  తిరిగి SQLITE_OK;
}

/* Release a memory-mapped page */
స్థిర పూర్ణాంకం memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, శూన్య *p_పుట){
  MemStore *p = ((MemFile*)pFile)->pStore;
  వాడని_PARAMETER(iOfst);
  వాడని_PARAMETER(p_పుట);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  తిరిగి SQLITE_OK;
}

/*
** Open an mem file handle.
*/
స్థిర పూర్ణాంకం memdbOpen(
  sqlite3_vfs *pVfs,
  స్థిరరాశి అక్షరం *zName,
  sqlite3_file *pFd,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_బహిర్గతం_Flags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  పూర్ణాంకం szName;
  వాడని_PARAMETER(pVfs);

  memset(pFile, 0, పరిమాణం(*pFile));
  szName = sqlite3Strlen30(zName);
  ఒకవేళ( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    పూర్ణాంకం i;
#నిర్వచితం_కాకపోతే SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#అయితే_ముగింపు
    sqlite3_mutex_enter(pVfsMutex);
    కొరకు(i=0; i<memdb_g.nMemStore; i++){
      ఒకవేళ( strcmp(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        ఆపు;
      }
    }
    ఒకవేళ( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( పరిమాణం(*p) + (i64)szName + 3 );
      ఒకవేళ( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        తిరిగి SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             పరిమాణం(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      ఒకవేళ( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        తిరిగి SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      memset(p, 0, పరిమాణం(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mx_Memdb_కొలత;
      p->zFName = (అక్షరం*)&p[1];
      జ్ఞాపకం_నకలు(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      ఒకవేళ( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        తిరిగి SQLITE_NOMEM;
      }
      p->nRef = 1;
      memdbEnter(p);
    }లేకపోతే{
      memdbEnter(p);
      p->nRef++;
    }
    sqlite3_mutex_leave(pVfsMutex);
  }లేకపోతే{
    p = sqlite3Malloc( పరిమాణం(*p) );
    ఒకవేళ( p==0 ){
      తిరిగి SQLITE_NOMEM;
    }
    memset(p, 0, పరిమాణం(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mx_Memdb_కొలత;
  }
  pFile->pStore = p;
  ఒకవేళ( p_బహిర్గతం_Flags!=0 ){
    *p_బహిర్గతం_Flags = flags | SQLITE_OPEN_జ్ఞాపకం;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  తిరిగి SQLITE_OK;
}

#ఒకవేళ 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
స్థిర పూర్ణాంకం memdbDelete(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం, పూర్ణాంకం dirSync){
  తిరిగి SQLITE_IOERR_DELETE;
}
#అయితే_ముగింపు

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
స్థిర పూర్ణాంకం memdbAccess(
  sqlite3_vfs *pVfs, 
  స్థిరరాశి అక్షరం *z_మార్గం, 
  పూర్ణాంకం flags, 
  పూర్ణాంకం *p_Res_బహిర్గతం
){
  వాడని_PARAMETER(pVfs);
  వాడని_PARAMETER(z_మార్గం);
  వాడని_PARAMETER(flags);
  *p_Res_బహిర్గతం = 0;
  తిరిగి SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
స్థిర పూర్ణాంకం memdb_నిండిన_Pathname(
  sqlite3_vfs *pVfs, 
  స్థిరరాశి అక్షరం *z_మార్గం, 
  పూర్ణాంకం n_బహిర్గతం, 
  అక్షరం *z_బహిర్గతం
){
  వాడని_PARAMETER(pVfs);
  sqlite3_snprintf(n_బహిర్గతం, z_బహిర్గతం, "%s", z_మార్గం);
  తిరిగి SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
స్థిర శూన్య *memdbDlOpen(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం){
  తిరిగి ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), z_మార్గం);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
స్థిర శూన్య memdbDlError(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_సందేశం);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
స్థిర శూన్య (*memdbDlSym(sqlite3_vfs *pVfs, శూన్య *p, స్థిరరాశి అక్షరం *zSym))(శూన్య){
  తిరిగి ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
స్థిర శూన్య memdbDlClose(sqlite3_vfs *pVfs, శూన్య *pHandle){
  ORIGVFS(pVfs)->xDlClose(ORIGVFS(pVfs), pHandle);
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
స్థిర పూర్ణాంకం memdbRandomness(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Buf_బహిర్గతం){
  తిరిగి ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, z_Buf_బహిర్గతం);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
స్థిర పూర్ణాంకం memdbSleep(sqlite3_vfs *pVfs, పూర్ణాంకం nMicro){
  తిరిగి ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#ఒకవేళ 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
స్థిర పూర్ణాంకం memdb_Current_సమయం(sqlite3_vfs *pVfs, ద్విగుణ *p_సమయం_బహిర్గతం){
  తిరిగి ORIGVFS(pVfs)->x_Current_సమయం(ORIGVFS(pVfs), p_సమయం_బహిర్గతం);
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం memdb_Get_చివరి_Error(sqlite3_vfs *pVfs, పూర్ణాంకం a, అక్షరం *b){
  తిరిగి ORIGVFS(pVfs)->x_Get_చివరి_Error(ORIGVFS(pVfs), a, b);
}
స్థిర పూర్ణాంకం memdb_Current_సమయం_Int64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  తిరిగి ORIGVFS(pVfs)->x_Current_సమయం_Int64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
స్థిర MemFile *memdbFromDbSchema(sqlite3 *db, స్థిరరాశి అక్షరం *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  పూర్ణాంకం rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_సూచిక, &p);
  ఒకవేళ( rc ) తిరిగి 0;
  ఒకవేళ( p->పునాది.pMethods!=&memdb_io_methods ) తిరిగి 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  ఒకవేళ( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  తిరిగి p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
గుర్తులేని అక్షరం *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  స్థిరరాశి అక్షరం *zSchema,      /* Which database within the connection */
  sqlite3_int64 *pi_కొలత,    /* Write size here, if not NULL */
  గుర్తులేని పూర్ణాంకం mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  పూర్ణాంకం iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  పూర్ణాంకం sz_పుట = 0;
  sqlite3_stmt *pStmt = 0;
  గుర్తులేని అక్షరం *p_బహిర్గతం = 0;
  అక్షరం *zSql;
  పూర్ణాంకం rc;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    (శూన్య)SQLITE_MISUSE_BKPT;
    తిరిగి 0;
  }
#అయితే_ముగింపు
  sqlite3_mutex_enter(db->mutex);

  ఒకవేళ( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3_కనుగొను_Db_Name(db, zSchema);
  ఒకవేళ( pi_కొలత ) *pi_కొలత = -1;
  ఒకవేళ( iDb<0 ) వెళ్ళు serialize_బహిర్గతం;
  ఒకవేళ( p ){
    MemStore *pStore = p->pStore;
    ప్రతిజ్ఞా( pStore->pMutex==0 );
    ఒకవేళ( pi_కొలత ) *pi_కొలత = pStore->sz;
    ఒకవేళ( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_బహిర్గతం = pStore->a_దత్తాంశం;
    }లేకపోతే{
      p_బహిర్గతం = sqlite3_malloc64( pStore->sz );
      ఒకవేళ( p_బహిర్గతం ) జ్ఞాపకం_నకలు(p_బహిర్గతం, pStore->a_దత్తాంశం, pStore->sz);
    }
    వెళ్ళు serialize_బహిర్గతం;
  }
  pBt = db->aDb[iDb].pBt;
  ఒకవేళ( pBt==0 ) వెళ్ళు serialize_బహిర్గతం;
  sz_పుట = sqlite3_Btree_Get_పుట_కొలత(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  ఒకవేళ( rc ) వెళ్ళు serialize_బహిర్గతం;
  rc = sqlite3_అడుగు(pStmt);
  ఒకవేళ( rc==SQLITE_అడ్డువరుస ){
    sz = sqlite3_స్తంభం_int64(pStmt, 0)*sz_పుట;
    ఒకవేళ( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_అడుగు(pStmt);
      ఒకవేళ( rc==SQLITE_అడ్డువరుస ){
        sz = sqlite3_స్తంభం_int64(pStmt, 0)*sz_పుట;
      }
    }
    ఒకవేళ( pi_కొలత ) *pi_కొలత = sz;
    ఒకవేళ( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_బహిర్గతం = 0;
    }లేకపోతే{
      p_బహిర్గతం = sqlite3_malloc64( sz );
      ఒకవేళ( p_బహిర్గతం ){
        పూర్ణాంకం n_పుట = sqlite3_స్తంభం_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        పూర్ణాంకం pgno;
        కొరకు(pgno=1; pgno<=n_పుట; pgno++){
          Db_పుట *p_పుట = 0;
          గుర్తులేని అక్షరం *p_గమ్యం = p_బహిర్గతం + sz_పుట*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (Db_పుట**)&p_పుట, 0);
          ఒకవేళ( rc==SQLITE_OK ){
            జ్ఞాపకం_నకలు(p_గమ్యం, sqlite3_Pager_Get_దత్తాంశం(p_పుట), sz_పుట);
          }లేకపోతే ఒకవేళ( pgno==PENDING_BYTE/sz_పుట+1 ){
            memset(p_గమ్యం, 0, sz_పుట);
          }లేకపోతే{
            sqlite3_free(p_బహిర్గతం);
            p_బహిర్గతం = 0;
            n_పుట = 0;            
          }
          sqlite3PagerUnref(p_పుట);       
        }
      }
    }
  }
  sqlite3_finalize(pStmt);

 serialize_బహిర్గతం:
  sqlite3_mutex_leave(db->mutex);
  తిరిగి p_బహిర్గతం;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
పూర్ణాంకం sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  స్థిరరాశి అక్షరం *zSchema,    /* Which DB to reopen with the deserialization */
  గుర్తులేని అక్షరం *p_దత్తాంశం,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  గుర్తులేని mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  అక్షరం *zSql;
  sqlite3_stmt *pStmt = 0;
  పూర్ణాంకం rc;
  పూర్ణాంకం iDb;

#నిర్వచితమైతే SQLITE_ENABLE_API_ARMOR
  ఒకవేళ( !sqlite3SafetyCheckOk(db) ){
    తిరిగి SQLITE_MISUSE_BKPT;
  }
  ఒకవేళ( szDb<0 ) తిరిగి SQLITE_MISUSE_BKPT;
  ఒకవేళ( szBuf<0 ) తిరిగి SQLITE_MISUSE_BKPT;
#అయితే_ముగింపు

  sqlite3_mutex_enter(db->mutex);
  ఒకవేళ( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3_కనుగొను_Db_Name(db, zSchema);
  testcase( iDb==1 );
  ఒకవేళ( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    వెళ్ళు చివర_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  ఒకవేళ( zSql==0 ){
    rc = SQLITE_NOMEM;
  }లేకపోతే{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  ఒకవేళ( rc ) వెళ్ళు చివర_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_అడుగు(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  ఒకవేళ( rc!=SQLITE_OK ){
    వెళ్ళు చివర_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  ఒకవేళ( p==0 ){
    rc = SQLITE_ERROR;
  }లేకపోతే{
    MemStore *pStore = p->pStore;
    pStore->a_దత్తాంశం = p_దత్తాంశం;
    p_దత్తాంశం = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    ఒకవేళ( pStore->szMax<sqlite3GlobalConfig.mx_Memdb_కొలత ){
      pStore->szMax = sqlite3GlobalConfig.mx_Memdb_కొలత;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

చివర_deserialize:
  ఒకవేళ( p_దత్తాంశం && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(p_దత్తాంశం);
  }
  sqlite3_mutex_leave(db->mutex);
  తిరిగి rc;
}

/*
** Return true if the VFS is the memvfs.
*/
పూర్ణాంకం sqlite3IsMemdb(స్థిరరాశి sqlite3_vfs *pVfs){
  తిరిగి pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
పూర్ణాంకం sqlite3MemdbInit(శూన్య){
  sqlite3_vfs *p_క్రింది = sqlite3_vfs_కనుగొను(0);
  గుర్తులేని పూర్ణాంకం sz;
  ఒకవేళ( NEVER(p_క్రింది==0) ) తిరిగి SQLITE_ERROR;
  sz = p_క్రింది->szOsFile;
  memdb_vfs.p_App_దత్తాంశం = p_క్రింది;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  ఒకవేళ( sz<పరిమాణం(MemFile) ) sz = పరిమాణం(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  తిరిగి sqlite3_vfs_రిజిస్టరు(&memdb_vfs, 0);
}
#అయితే_ముగింపు /* SQLITE_OMIT_DESERIALIZE */
