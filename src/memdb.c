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
#यदि_अपरिभाषितम् SQLITE_OMIT_DESERIALIZE

/*
** Forward declaration of objects used by this utility
*/
प्रकारनिर्देश संरचना sqlite3_vfs MemVfs;
प्रकारनिर्देश संरचना MemFile MemFile;
प्रकारनिर्देश संरचना MemStore MemStore;

/* Access to a lower-level VFS that (might) implement dynamic loading,
** access to randomness, etc.
*/
#परिभाषय ORIGVFS(p) ((sqlite3_vfs*)((p)->p_App_दत्त))

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
  अचिह्नित वर्ण *a_दत्त;           /* content of the file */
  sqlite3_mutex *pMutex;          /* Used by shared stores only */
  पूर्णाङ्क nMmap;                      /* Number of memory mapped pages */
  अचिह्नित mFlags;                /* Flags */
  पूर्णाङ्क n_Rd_तालकम्;                    /* Number of readers */
  पूर्णाङ्क n_Wr_तालकम्;                    /* Number of writers.  (Always 0 or 1) */
  पूर्णाङ्क nRef;                       /* Number of users of this MemStore */
  वर्ण *zFName;                   /* The filename for shared stores */
};

/* An open file */
संरचना MemFile {
  sqlite3_file आधारः;              /* IO methods */
  MemStore *pStore;               /* The storage */
  पूर्णाङ्क e_तालकम्;                      /* Most recent lock against this file */
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
स्थिर पूर्णाङ्क memdbWrite(sqlite3_file*,ध्रुव शून्य*,पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क memdbTruncate(sqlite3_file*, sqlite3_int64 आकार);
स्थिर पूर्णाङ्क memdbSync(sqlite3_file*, पूर्णाङ्क flags);
स्थिर पूर्णाङ्क memdb_File_आकार(sqlite3_file*, sqlite3_int64 *p_आकार);
स्थिर पूर्णाङ्क memdb_तालकम्(sqlite3_file*, पूर्णाङ्क);
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
स्थिर पूर्णाङ्क memdbOpen(sqlite3_vfs*, ध्रुव वर्ण *, sqlite3_file*, पूर्णाङ्क , पूर्णाङ्क *);
/* static int memdbDelete(sqlite3_vfs*, const char *zName, int syncDir); */
स्थिर पूर्णाङ्क memdbAccess(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क flags, पूर्णाङ्क *);
स्थिर पूर्णाङ्क memdb_पूर्णम्_Pathname(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क, वर्ण *z_बहिर्गमनम्);
स्थिर शून्य *memdbDlOpen(sqlite3_vfs*, ध्रुव वर्ण *zFilename);
स्थिर शून्य memdbDlError(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देशः);
स्थिर शून्य (*memdbDlSym(sqlite3_vfs *pVfs, शून्य *p, ध्रुव वर्ण*zSym))(शून्य);
स्थिर शून्य memdbDlClose(sqlite3_vfs*, शून्य*);
स्थिर पूर्णाङ्क memdbRandomness(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *z_बहिर्गमनम्);
स्थिर पूर्णाङ्क memdbSleep(sqlite3_vfs*, पूर्णाङ्क microseconds);
/* static int memdbCurrentTime(sqlite3_vfs*, double*); */
स्थिर पूर्णाङ्क memdb_Get_चरमः_Error(sqlite3_vfs*, पूर्णाङ्क, वर्ण *);
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
  memdb_पूर्णम्_Pathname,           /* xFullPathname */
  memdbDlOpen,                 /* xDlOpen */
  memdbDlError,                /* xDlError */
  memdbDlSym,                  /* xDlSym */
  memdbDlClose,                /* xDlClose */
  memdbRandomness,             /* xRandomness */
  memdbSleep,                  /* xSleep */
  0, /* memdbCurrentTime, */   /* xCurrentTime */
  memdb_Get_चरमः_Error,           /* xGetLastError */
  memdbCurrentTimeInt64,       /* xCurrentTimeInt64 */
  0,                           /* xSetSystemCall */
  0,                           /* xGetSystemCall */
  0,                           /* xNextSystemCall */
};

स्थिर ध्रुव sqlite3_io_methods memdb_io_methods = {
  3,                              /* iVersion */
  memdbClose,                      /* xClose */
  memdbRead,                       /* xRead */
  memdbWrite,                      /* xWrite */
  memdbTruncate,                   /* xTruncate */
  memdbSync,                       /* xSync */
  memdb_File_आकार,                   /* xFileSize */
  memdb_तालकम्,                       /* xLock */
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
  अप्रयुक्तम्_PARAMETER(p);
}
स्थिर शून्य memdbLeave(MemStore *p){
  अप्रयुक्तम्_PARAMETER(p);
}
#अन्यथा
स्थिर शून्य memdbEnter(MemStore *p){
  sqlite3_mutex_enter(p->pMutex);
}
स्थिर शून्य memdbLeave(MemStore *p){
  sqlite3_mutex_leave(p->pMutex);
}
#यदि_अन्तः



/*
** Close an memdb-file.
** Free the underlying MemStore object when its refcount drops to zero
** or less.
*/
स्थिर पूर्णाङ्क memdbClose(sqlite3_file *pFile){
  MemStore *p = ((MemFile*)pFile)->pStore;
  यदि( p->zFName ){
    पूर्णाङ्क i;
#यदि_अपरिभाषितम् SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#यदि_अन्तः
    sqlite3_mutex_enter(pVfsMutex);
    प्रति(i=0; ALWAYS(i<memdb_g.nMemStore); i++){
      यदि( memdb_g.apMemStore[i]==p ){
        memdbEnter(p);
        यदि( p->nRef==1 ){
          memdb_g.apMemStore[i] = memdb_g.apMemStore[--memdb_g.nMemStore];
          यदि( memdb_g.nMemStore==0 ){
            sqlite3_free(memdb_g.apMemStore);
            memdb_g.apMemStore = 0;
          }
        }
        भङ्ग;
      }
    }
    sqlite3_mutex_leave(pVfsMutex);
  }अन्यथा{
    memdbEnter(p);
  }
  p->nRef--;
  यदि( p->nRef<=0 ){
    यदि( p->mFlags & SQLITE_DESERIALIZE_FREEONCLOSE ){
      sqlite3_free(p->a_दत्त);
    }
    memdbLeave(p);
    sqlite3_mutex_free(p->pMutex);
    sqlite3_free(p);
  }अन्यथा{
    memdbLeave(p);
  }
  प्रत्यागम SQLITE_OK;
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
    स्मृतिं_पूरय(zBuf, 0, iAmt);
    यदि( iOfst<p->sz ) स्मृतिं_प्रतिलिख(zBuf, p->a_दत्त+iOfst, p->sz - iOfst);
    memdbLeave(p);
    प्रत्यागम SQLITE_IOERR_SHORT_READ;
  }
  स्मृतिं_प्रतिलिख(zBuf, p->a_दत्त+iOfst, iAmt);
  memdbLeave(p);
  प्रत्यागम SQLITE_OK;
}

/*
** Try to enlarge the memory allocation to hold at least sz bytes
*/
स्थिर पूर्णाङ्क memdbEnlarge(MemStore *p, sqlite3_int64 newSz){
  अचिह्नित वर्ण *pNew;
  यदि( (p->mFlags & SQLITE_DESERIALIZE_RESIZEABLE)==0 || NEVER(p->nMmap>0) ){
    प्रत्यागम SQLITE_पूर्णम्;
  }
  यदि( newSz>p->szMax ){
    प्रत्यागम SQLITE_पूर्णम्;
  }
  newSz *= 2;
  यदि( newSz>p->szMax ) newSz = p->szMax;
  pNew = sqlite3Realloc(p->a_दत्त, newSz);
  यदि( pNew==0 ) प्रत्यागम SQLITE_IOERR_NOMEM;
  p->a_दत्त = pNew;
  p->szAlloc = newSz;
  प्रत्यागम SQLITE_OK;
}

/*
** Write data to an memdb-file.
*/
स्थिर पूर्णाङ्क memdbWrite(
  sqlite3_file *pFile,
  ध्रुव शून्य *z,
  पूर्णाङ्क iAmt,
  sqlite_int64 iOfst
){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  यदि( NEVER(p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    /* Can't happen: memdbLock() will return SQLITE_READONLY before
    ** reaching this point */
    memdbLeave(p);
    प्रत्यागम SQLITE_IOERR_WRITE;
  }
  यदि( iOfst+iAmt>p->sz ){
    पूर्णाङ्क rc;
    यदि( iOfst+iAmt>p->szAlloc
     && (rc = memdbEnlarge(p, iOfst+iAmt))!=SQLITE_OK
    ){
      memdbLeave(p);
      प्रत्यागम rc;
    }
    यदि( iOfst>p->sz ) स्मृतिं_पूरय(p->a_दत्त+p->sz, 0, iOfst-p->sz);
    p->sz = iOfst+iAmt;
  }
  स्मृतिं_प्रतिलिख(p->a_दत्त+iOfst, z, iAmt);
  memdbLeave(p);
  प्रत्यागम SQLITE_OK;
}

/*
** Truncate an memdb-file.
**
** In rollback mode (which is always the case for memdb, as it does not
** support WAL mode) the truncate() method is only used to reduce
** the size of a file, never to increase the size.
*/
स्थिर पूर्णाङ्क memdbTruncate(sqlite3_file *pFile, sqlite_int64 आकार){
  MemStore *p = ((MemFile*)pFile)->pStore;
  पूर्णाङ्क rc = SQLITE_OK;
  memdbEnter(p);
  यदि( आकार>p->sz ){
    /* This can only happen with a corrupt wal mode db */
    rc = SQLITE_CORRUPT;
  }अन्यथा{
    p->sz = आकार; 
  }
  memdbLeave(p);
  प्रत्यागम rc;
}

/*
** Sync an memdb-file.
*/
स्थिर पूर्णाङ्क memdbSync(sqlite3_file *pFile, पूर्णाङ्क flags){
  अप्रयुक्तम्_PARAMETER(pFile);
  अप्रयुक्तम्_PARAMETER(flags);
  प्रत्यागम SQLITE_OK;
}

/*
** Return the current file-size of an memdb-file.
*/
स्थिर पूर्णाङ्क memdb_File_आकार(sqlite3_file *pFile, sqlite_int64 *p_आकार){
  MemStore *p = ((MemFile*)pFile)->pStore;
  memdbEnter(p);
  *p_आकार = p->sz;
  memdbLeave(p);
  प्रत्यागम SQLITE_OK;
}

/*
** Lock an memdb-file.
*/
स्थिर पूर्णाङ्क memdb_तालकम्(sqlite3_file *pFile, पूर्णाङ्क e_तालकम्){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  पूर्णाङ्क rc = SQLITE_OK;
  यदि( e_तालकम्<=pThis->e_तालकम् ) प्रत्यागम SQLITE_OK;
  memdbEnter(p);

  प्रतिज्ञा( p->n_Wr_तालकम्==0 || p->n_Wr_तालकम्==1 );
  प्रतिज्ञा( pThis->e_तालकम्<=SQLITE_तालकम्_SHARED || p->n_Wr_तालकम्==1 );
  प्रतिज्ञा( pThis->e_तालकम्==SQLITE_तालकम्_NONE || p->n_Rd_तालकम्>=1 );

  यदि( e_तालकम्>SQLITE_तालकम्_SHARED && (p->mFlags & SQLITE_DESERIALIZE_READONLY) ){
    rc = SQLITE_READONLY;
  }अन्यथा{
    विकल्प( e_तालकम् ){
      पक्ष SQLITE_तालकम्_SHARED: {
        प्रतिज्ञा( pThis->e_तालकम्==SQLITE_तालकम्_NONE );
        यदि( p->n_Wr_तालकम्>0 ){
          rc = SQLITE_BUSY;
        }अन्यथा{
          p->n_Rd_तालकम्++;
        }
        भङ्ग;
      };
  
      पक्ष SQLITE_तालकम्_RESERVED:
      पक्ष SQLITE_तालकम्_PENDING: {
        प्रतिज्ञा( pThis->e_तालकम्>=SQLITE_तालकम्_SHARED );
        यदि( ALWAYS(pThis->e_तालकम्==SQLITE_तालकम्_SHARED) ){
          यदि( p->n_Wr_तालकम्>0 ){
            rc = SQLITE_BUSY;
          }अन्यथा{
            p->n_Wr_तालकम् = 1;
          }
        }
        भङ्ग;
      }
  
      स्वभाव: {
        प्रतिज्ञा(  e_तालकम्==SQLITE_तालकम्_EXCLUSIVE );
        प्रतिज्ञा( pThis->e_तालकम्>=SQLITE_तालकम्_SHARED );
        यदि( p->n_Rd_तालकम्>1 ){
          rc = SQLITE_BUSY;
        }अन्यथा यदि( pThis->e_तालकम्==SQLITE_तालकम्_SHARED ){
          p->n_Wr_तालकम् = 1;
        }
        भङ्ग;
      }
    }
  }
  यदि( rc==SQLITE_OK ) pThis->e_तालकम् = e_तालकम्;
  memdbLeave(p);
  प्रत्यागम rc;
}

/*
** Unlock an memdb-file.
*/
स्थिर पूर्णाङ्क memdbUnlock(sqlite3_file *pFile, पूर्णाङ्क e_तालकम्){
  MemFile *pThis = (MemFile*)pFile;
  MemStore *p = pThis->pStore;
  यदि( e_तालकम्>=pThis->e_तालकम् ) प्रत्यागम SQLITE_OK;
  memdbEnter(p);

  प्रतिज्ञा( e_तालकम्==SQLITE_तालकम्_SHARED || e_तालकम्==SQLITE_तालकम्_NONE );
  यदि( e_तालकम्==SQLITE_तालकम्_SHARED ){
    यदि( ALWAYS(pThis->e_तालकम्>SQLITE_तालकम्_SHARED) ){
      p->n_Wr_तालकम्--;
    }
  }अन्यथा{
    यदि( pThis->e_तालकम्>SQLITE_तालकम्_SHARED ){
      p->n_Wr_तालकम्--;
    }
    p->n_Rd_तालकम्--;
  }

  pThis->e_तालकम् = e_तालकम्;
  memdbLeave(p);
  प्रत्यागम SQLITE_OK;
}

#यदि 0
/*
** This interface is only used for crash recovery, which does not
** occur on an in-memory database.
*/
स्थिर पूर्णाङ्क memdb_Check_Reserved_तालकम्(sqlite3_file *pFile, पूर्णाङ्क *p_Res_बहिर्गमनम्){
  *p_Res_बहिर्गमनम् = 0;
  प्रत्यागम SQLITE_OK;
}
#यदि_अन्तः


/*
** File control method. For custom operations on an memdb-file.
*/
स्थिर पूर्णाङ्क memdbFileControl(sqlite3_file *pFile, पूर्णाङ्क op, शून्य *pArg){
  MemStore *p = ((MemFile*)pFile)->pStore;
  पूर्णाङ्क rc = SQLITE_NOTFOUND;
  memdbEnter(p);
  यदि( op==SQLITE_FCNTL_VFSNAME ){
    *(वर्ण**)pArg = sqlite3_mprintf("memdb(%p,%lld)", p->a_दत्त, p->sz);
    rc = SQLITE_OK;
  }
  यदि( op==SQLITE_FCNTL_आकार_सीमा ){
    sqlite3_int64 i_सीमा = *(sqlite3_int64*)pArg;
    यदि( i_सीमा<p->sz ){
      यदि( i_सीमा<0 ){
        i_सीमा = p->szMax;
      }अन्यथा{
        i_सीमा = p->sz;
      }
    }
    p->szMax = i_सीमा;
    *(sqlite3_int64*)pArg = i_सीमा;
    rc = SQLITE_OK;
  }
  memdbLeave(p);
  प्रत्यागम rc;
}

#यदि 0  /* Not used because of SQLITE_IOCAP_POWERSAFE_OVERWRITE */
/*
** Return the sector-size in bytes for an memdb-file.
*/
स्थिर पूर्णाङ्क memdb_Sector_आकार(sqlite3_file *pFile){
  प्रत्यागम 1024;
}
#यदि_अन्तः

/*
** Return the device characteristic flags supported by an memdb-file.
*/
स्थिर पूर्णाङ्क memdbDeviceCharacteristics(sqlite3_file *pFile){
  अप्रयुक्तम्_PARAMETER(pFile);
  प्रत्यागम SQLITE_IOCAP_ATOMIC | 
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
    *pp = (शून्य*)(p->a_दत्त + iOfst);
  }
  memdbLeave(p);
  प्रत्यागम SQLITE_OK;
}

/* Release a memory-mapped page */
स्थिर पूर्णाङ्क memdbUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, शून्य *pPage){
  MemStore *p = ((MemFile*)pFile)->pStore;
  अप्रयुक्तम्_PARAMETER(iOfst);
  अप्रयुक्तम्_PARAMETER(pPage);
  memdbEnter(p);
  p->nMmap--;
  memdbLeave(p);
  प्रत्यागम SQLITE_OK;
}

/*
** Open an mem file handle.
*/
स्थिर पूर्णाङ्क memdbOpen(
  sqlite3_vfs *pVfs,
  ध्रुव वर्ण *zName,
  sqlite3_file *pFd,
  पूर्णाङ्क flags,
  पूर्णाङ्क *p_बहिर्गमनम्_Flags
){
  MemFile *pFile = (MemFile*)pFd;
  MemStore *p = 0;
  पूर्णाङ्क szName;
  अप्रयुक्तम्_PARAMETER(pVfs);

  स्मृतिं_पूरय(pFile, 0, परिमाण(*pFile));
  szName = sqlite3Strlen30(zName);
  यदि( szName>1 && (zName[0]=='/' || zName[0]=='\\') ){
    पूर्णाङ्क i;
#यदि_अपरिभाषितम् SQLITE_MUTEX_OMIT
    sqlite3_mutex *pVfsMutex = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_VFS1);
#यदि_अन्तः
    sqlite3_mutex_enter(pVfsMutex);
    प्रति(i=0; i<memdb_g.nMemStore; i++){
      यदि( वाक्यं_तुलय(memdb_g.apMemStore[i]->zFName,zName)==0 ){
        p = memdb_g.apMemStore[i];
        भङ्ग;
      }
    }
    यदि( p==0 ){
      MemStore **apNew;
      p = sqlite3Malloc( परिमाण(*p) + (i64)szName + 3 );
      यदि( p==0 ){
        sqlite3_mutex_leave(pVfsMutex);
        प्रत्यागम SQLITE_NOMEM;
      }
      apNew = sqlite3Realloc(memdb_g.apMemStore,
                             परिमाण(apNew[0])*(1+(i64)memdb_g.nMemStore) );
      यदि( apNew==0 ){
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        प्रत्यागम SQLITE_NOMEM;
      }
      apNew[memdb_g.nMemStore++] = p;
      memdb_g.apMemStore = apNew;
      स्मृतिं_पूरय(p, 0, परिमाण(*p));
      p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE|SQLITE_DESERIALIZE_FREEONCLOSE;
      p->szMax = sqlite3GlobalConfig.mx_Memdb_आकार;
      p->zFName = (वर्ण*)&p[1];
      स्मृतिं_प्रतिलिख(p->zFName, zName, szName+1);
      p->pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
      यदि( p->pMutex==0 ){
        memdb_g.nMemStore--;
        sqlite3_free(p);
        sqlite3_mutex_leave(pVfsMutex);
        प्रत्यागम SQLITE_NOMEM;
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
      प्रत्यागम SQLITE_NOMEM;
    }
    स्मृतिं_पूरय(p, 0, परिमाण(*p));
    p->mFlags = SQLITE_DESERIALIZE_RESIZEABLE | SQLITE_DESERIALIZE_FREEONCLOSE;
    p->szMax = sqlite3GlobalConfig.mx_Memdb_आकार;
  }
  pFile->pStore = p;
  यदि( p_बहिर्गमनम्_Flags!=0 ){
    *p_बहिर्गमनम्_Flags = flags | SQLITE_OPEN_स्मृतिः;
  }
  pFd->pMethods = &memdb_io_methods;
  memdbLeave(p);
  प्रत्यागम SQLITE_OK;
}

#यदि 0 /* Only used to delete rollback journals, super-journals, and WAL
      ** files, none of which exist in memdb.  So this routine is never used */
/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
स्थिर पूर्णाङ्क memdbDelete(sqlite3_vfs *pVfs, ध्रुव वर्ण *z_मार्गः, पूर्णाङ्क dirSync){
  प्रत्यागम SQLITE_IOERR_DELETE;
}
#यदि_अन्तः

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
**
** With memdb, no files ever exist on disk.  So always return false.
*/
स्थिर पूर्णाङ्क memdbAccess(
  sqlite3_vfs *pVfs, 
  ध्रुव वर्ण *z_मार्गः, 
  पूर्णाङ्क flags, 
  पूर्णाङ्क *p_Res_बहिर्गमनम्
){
  अप्रयुक्तम्_PARAMETER(pVfs);
  अप्रयुक्तम्_PARAMETER(z_मार्गः);
  अप्रयुक्तम्_PARAMETER(flags);
  *p_Res_बहिर्गमनम् = 0;
  प्रत्यागम SQLITE_OK;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (INST_MAX_PATHNAME+1) bytes.
*/
स्थिर पूर्णाङ्क memdb_पूर्णम्_Pathname(
  sqlite3_vfs *pVfs, 
  ध्रुव वर्ण *z_मार्गः, 
  पूर्णाङ्क n_बहिर्गमनम्, 
  वर्ण *z_बहिर्गमनम्
){
  अप्रयुक्तम्_PARAMETER(pVfs);
  sqlite3_snprintf(n_बहिर्गमनम्, z_बहिर्गमनम्, "%s", z_मार्गः);
  प्रत्यागम SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
स्थिर शून्य *memdbDlOpen(sqlite3_vfs *pVfs, ध्रुव वर्ण *z_मार्गः){
  प्रत्यागम ORIGVFS(pVfs)->xDlOpen(ORIGVFS(pVfs), z_मार्गः);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
स्थिर शून्य memdbDlError(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देशः){
  ORIGVFS(pVfs)->xDlError(ORIGVFS(pVfs), nByte, z_Err_सन्देशः);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
स्थिर शून्य (*memdbDlSym(sqlite3_vfs *pVfs, शून्य *p, ध्रुव वर्ण *zSym))(शून्य){
  प्रत्यागम ORIGVFS(pVfs)->xDlSym(ORIGVFS(pVfs), p, zSym);
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
स्थिर पूर्णाङ्क memdbRandomness(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *z_Buf_बहिर्गमनम्){
  प्रत्यागम ORIGVFS(pVfs)->xRandomness(ORIGVFS(pVfs), nByte, z_Buf_बहिर्गमनम्);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
स्थिर पूर्णाङ्क memdbSleep(sqlite3_vfs *pVfs, पूर्णाङ्क nMicro){
  प्रत्यागम ORIGVFS(pVfs)->xSleep(ORIGVFS(pVfs), nMicro);
}

#यदि 0  /* Never used.  Modern cores only call xCurrentTimeInt64() */
/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
स्थिर पूर्णाङ्क memdbCurrentTime(sqlite3_vfs *pVfs, द्विगुण *p_Time_बहिर्गमनम्){
  प्रत्यागम ORIGVFS(pVfs)->xCurrentTime(ORIGVFS(pVfs), p_Time_बहिर्गमनम्);
}
#यदि_अन्तः

स्थिर पूर्णाङ्क memdb_Get_चरमः_Error(sqlite3_vfs *pVfs, पूर्णाङ्क a, वर्ण *b){
  प्रत्यागम ORIGVFS(pVfs)->x_Get_चरमः_Error(ORIGVFS(pVfs), a, b);
}
स्थिर पूर्णाङ्क memdbCurrentTimeInt64(sqlite3_vfs *pVfs, sqlite3_int64 *p){
  प्रत्यागम ORIGVFS(pVfs)->xCurrentTimeInt64(ORIGVFS(pVfs), p);
}

/*
** Translate a database connection pointer and schema name into a
** MemFile pointer.
*/
स्थिर MemFile *memdbFromDbSchema(sqlite3 *db, ध्रुव वर्ण *zSchema){
  MemFile *p = 0;
  MemStore *pStore;
  पूर्णाङ्क rc = sqlite3_file_control(db, zSchema, SQLITE_FCNTL_FILE_POINTER, &p);
  यदि( rc ) प्रत्यागम 0;
  यदि( p->आधारः.pMethods!=&memdb_io_methods ) प्रत्यागम 0;
  pStore = p->pStore;
  memdbEnter(pStore);
  यदि( pStore->zFName!=0 ) p = 0;
  memdbLeave(pStore);
  प्रत्यागम p;
}

/*
** Return the serialization of a database.  Or return NULL if there
** is an OOM or I/O error.
*/
अचिह्नित वर्ण *sqlite3_serialize(
  sqlite3 *db,              /* The database connection */
  ध्रुव वर्ण *zSchema,      /* Which database within the connection */
  sqlite3_int64 *pi_आकार,    /* Write size here, if not NULL */
  अचिह्नित पूर्णाङ्क mFlags       /* Maybe SQLITE_SERIALIZE_NOCOPY */
){
  MemFile *p;
  पूर्णाङ्क iDb;
  Btree *pBt;
  sqlite3_int64 sz;
  पूर्णाङ्क szPage = 0;
  sqlite3_stmt *pStmt = 0;
  अचिह्नित वर्ण *p_बहिर्गमनम् = 0;
  वर्ण *zSql;
  पूर्णाङ्क rc;

#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( !sqlite3SafetyCheckOk(db) ){
    (शून्य)SQLITE_MISUSE_BKPT;
    प्रत्यागम 0;
  }
#यदि_अन्तः
  sqlite3_mutex_enter(db->mutex);

  यदि( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  p = memdbFromDbSchema(db, zSchema);
  iDb = sqlite3FindDbName(db, zSchema);
  यदि( pi_आकार ) *pi_आकार = -1;
  यदि( iDb<0 ) गच्छ serialize_बहिर्गमनम्;
  यदि( p ){
    MemStore *pStore = p->pStore;
    प्रतिज्ञा( pStore->pMutex==0 );
    यदि( pi_आकार ) *pi_आकार = pStore->sz;
    यदि( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_बहिर्गमनम् = pStore->a_दत्त;
    }अन्यथा{
      p_बहिर्गमनम् = sqlite3_malloc64( pStore->sz );
      यदि( p_बहिर्गमनम् ) स्मृतिं_प्रतिलिख(p_बहिर्गमनम्, pStore->a_दत्त, pStore->sz);
    }
    गच्छ serialize_बहिर्गमनम्;
  }
  pBt = db->aDb[iDb].pBt;
  यदि( pBt==0 ) गच्छ serialize_बहिर्गमनम्;
  szPage = sqlite3_Btree_Get_Page_आकार(pBt);
  zSql = sqlite3_mprintf("PRAGMA \"%w\".page_count", zSchema);
  rc = zSql ? sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0) : SQLITE_NOMEM;
  sqlite3_free(zSql);
  यदि( rc ) गच्छ serialize_बहिर्गमनम्;
  rc = sqlite3_चरणः(pStmt);
  यदि( rc==SQLITE_ROW ){
    sz = sqlite3_column_int64(pStmt, 0)*szPage;
    यदि( sz==0 ){
      sqlite3_reset(pStmt);
      sqlite3_exec(db, "BEGIN IMMEDIATE; COMMIT;", 0, 0, 0);
      rc = sqlite3_चरणः(pStmt);
      यदि( rc==SQLITE_ROW ){
        sz = sqlite3_column_int64(pStmt, 0)*szPage;
      }
    }
    यदि( pi_आकार ) *pi_आकार = sz;
    यदि( mFlags & SQLITE_SERIALIZE_NOCOPY ){
      p_बहिर्गमनम् = 0;
    }अन्यथा{
      p_बहिर्गमनम् = sqlite3_malloc64( sz );
      यदि( p_बहिर्गमनम् ){
        पूर्णाङ्क nPage = sqlite3_column_int(pStmt, 0);
        Pager *pPager = sqlite3BtreePager(pBt);
        पूर्णाङ्क pgno;
        प्रति(pgno=1; pgno<=nPage; pgno++){
          DbPage *pPage = 0;
          अचिह्नित वर्ण *p_गन्तव्यम् = p_बहिर्गमनम् + szPage*(sqlite3_int64)(pgno-1);
          rc = sqlite3PagerGet(pPager, pgno, (DbPage**)&pPage, 0);
          यदि( rc==SQLITE_OK ){
            स्मृतिं_प्रतिलिख(p_गन्तव्यम्, sqlite3_Pager_Get_दत्त(pPage), szPage);
          }अन्यथा यदि( pgno==PENDING_BYTE/szPage+1 ){
            स्मृतिं_पूरय(p_गन्तव्यम्, 0, szPage);
          }अन्यथा{
            sqlite3_free(p_बहिर्गमनम्);
            p_बहिर्गमनम् = 0;
            nPage = 0;            
          }
          sqlite3PagerUnref(pPage);       
        }
      }
    }
  }
  sqlite3_finalize(pStmt);

 serialize_बहिर्गमनम्:
  sqlite3_mutex_leave(db->mutex);
  प्रत्यागम p_बहिर्गमनम्;
}

/* Convert zSchema to a MemDB and initialize its content.
*/
पूर्णाङ्क sqlite3_deserialize(
  sqlite3 *db,            /* The database connection */
  ध्रुव वर्ण *zSchema,    /* Which DB to reopen with the deserialization */
  अचिह्नित वर्ण *p_दत्त,   /* The serialized database content */
  sqlite3_int64 szDb,     /* Number bytes in the deserialization */
  sqlite3_int64 szBuf,    /* Total size of buffer pData[] */
  अचिह्नित mFlags         /* Zero or more SQLITE_DESERIALIZE_* flags */
){
  MemFile *p;
  वर्ण *zSql;
  sqlite3_stmt *pStmt = 0;
  पूर्णाङ्क rc;
  पूर्णाङ्क iDb;

#यदि_परिभाषितम् SQLITE_ENABLE_API_ARMOR
  यदि( !sqlite3SafetyCheckOk(db) ){
    प्रत्यागम SQLITE_MISUSE_BKPT;
  }
  यदि( szDb<0 ) प्रत्यागम SQLITE_MISUSE_BKPT;
  यदि( szBuf<0 ) प्रत्यागम SQLITE_MISUSE_BKPT;
#यदि_अन्तः

  sqlite3_mutex_enter(db->mutex);
  यदि( zSchema==0 ) zSchema = db->aDb[0].zDbSName;
  iDb = sqlite3FindDbName(db, zSchema);
  testcase( iDb==1 );
  यदि( iDb<2 && iDb!=0 ){
    rc = SQLITE_ERROR;
    गच्छ अन्तः_deserialize;
  }
  zSql = sqlite3_mprintf("ATTACH x AS %Q", zSchema);
  यदि( zSql==0 ){
    rc = SQLITE_NOMEM;
  }अन्यथा{
    rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
    sqlite3_free(zSql);
  }
  यदि( rc ) गच्छ अन्तः_deserialize;
  db->init.iDb = (u8)iDb;
  db->init.reopenMemdb = 1;
  sqlite3_चरणः(pStmt);
  db->init.reopenMemdb = 0;
  rc = sqlite3_finalize(pStmt);
  यदि( rc!=SQLITE_OK ){
    गच्छ अन्तः_deserialize;
  }
  p = memdbFromDbSchema(db, zSchema);
  यदि( p==0 ){
    rc = SQLITE_ERROR;
  }अन्यथा{
    MemStore *pStore = p->pStore;
    pStore->a_दत्त = p_दत्त;
    p_दत्त = 0;
    pStore->sz = szDb;
    pStore->szAlloc = szBuf;
    pStore->szMax = szBuf;
    यदि( pStore->szMax<sqlite3GlobalConfig.mx_Memdb_आकार ){
      pStore->szMax = sqlite3GlobalConfig.mx_Memdb_आकार;
    }
    pStore->mFlags = mFlags;
    rc = SQLITE_OK;
  }

अन्तः_deserialize:
  यदि( p_दत्त && (mFlags & SQLITE_DESERIALIZE_FREEONCLOSE)!=0 ){
    sqlite3_free(p_दत्त);
  }
  sqlite3_mutex_leave(db->mutex);
  प्रत्यागम rc;
}

/*
** Return true if the VFS is the memvfs.
*/
पूर्णाङ्क sqlite3IsMemdb(ध्रुव sqlite3_vfs *pVfs){
  प्रत्यागम pVfs==&memdb_vfs;
}

/* 
** This routine is called when the extension is loaded.
** Register the new VFS.
*/
पूर्णाङ्क sqlite3MemdbInit(शून्य){
  sqlite3_vfs *p_अधस्तनम् = sqlite3_vfs_find(0);
  अचिह्नित पूर्णाङ्क sz;
  यदि( NEVER(p_अधस्तनम्==0) ) प्रत्यागम SQLITE_ERROR;
  sz = p_अधस्तनम्->szOsFile;
  memdb_vfs.p_App_दत्त = p_अधस्तनम्;
  /* The following conditional can only be true when compiled for
  ** Windows x86 and SQLITE_MAX_MMAP_SIZE=0.  We always leave
  ** it in, to be safe, but it is marked as NO_TEST since there
  ** is no way to reach it under most builds. */
  यदि( sz<परिमाण(MemFile) ) sz = परिमाण(MemFile); /*NO_TEST*/
  memdb_vfs.szOsFile = sz;
  प्रत्यागम sqlite3_vfs_पञ्जिकाकोशः(&memdb_vfs, 0);
}
#यदि_अन्तः /* SQLITE_OMIT_DESERIALIZE */
