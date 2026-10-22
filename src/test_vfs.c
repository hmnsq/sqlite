/*
** 2010 May 05
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
** This file contains the implementation of the Tcl [testvfs] command,
** used to create SQLite VFS implementations with various properties and
** instrumentation to support testing SQLite.
**
**   testvfs VFSNAME ?OPTIONS?
**
** Available options are:
**
**   -noshm      BOOLEAN        (True to omit shm methods. Default false)
**   -default    BOOLEAN        (True to make the vfs default. Default false)
**   -szosfile   INTEGER        (Value for sqlite3_vfs.szOsFile)
**   -mxpathname INTEGER        (Value for sqlite3_vfs.mxPathname)
**   -iversion   INTEGER        (Value for sqlite3_vfs.iVersion)
*/
#اگر SQLITE_TEST          /* This file is used for testing only */

#شامل "sqlite3.h"
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"

تعریفنوع ساختار Testvfs Testvfs;
تعریفنوع ساختار TestvfsShm TestvfsShm;
تعریفنوع ساختار TestvfsBuffer TestvfsBuffer;
تعریفنوع ساختار TestvfsFile TestvfsFile;
تعریفنوع ساختار TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
ساختار TestvfsFile {
  sqlite3_file base;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#define tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

ساختار TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  ثابت نویسه *zFilename;          /* Filename as passed to xOpen() */
  sqlite3_file *pReal;            /* The real, underlying file descriptor */
  Tcl_Obj *pShmId;                /* Shared memory id for Tcl callbacks */

  TestvfsBuffer *pShm;            /* Shared memory buffer */
  u32 excllock;                   /* Mask of exclusive locks */
  u32 sharedlock;                 /* Mask of shared locks */
  TestvfsFd *pNext;               /* Next handle opened on the same file */
};


#define FAULT_INJECT_NONE       0
#define FAULT_INJECT_TRANSIENT  1
#define FAULT_INJECT_PERSISTENT 2

تعریفنوع ساختار TestFaultInject TestFaultInject;
ساختار TestFaultInject {
  صحیح iCnt;                       /* Remaining calls before fault injection */
  صحیح eFault;                     /* A FAULT_INJECT_* value */
  صحیح nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
ساختار Testvfs {
  نویسه *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  TestvfsBuffer *pBuffer;         /* List of shared buffers */
  صحیح isNoshm;
  صحیح isFullshm;

  صحیح mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#اگر 0
  صحیح iIoerrCnt;
  صحیح ioerr;
  صحیح nIoerrFail;
  صحیح iFullCnt;
  صحیح fullerr;
  صحیح nFullFail;
#endif

  صحیح iDevchar;
  صحیح iSectorsize;
};

/*
** The Testvfs.mask variable is set to a combination of the following.
** If a bit is clear in Testvfs.mask, then calls made by SQLite to the 
** corresponding VFS method is ignored for purposes of:
**
**   + Simulating IO errors, and
**   + Invoking the Tcl callback script.
*/
#define TESTVFS_SHMOPEN_MASK      0x00000001
#define TESTVFS_SHMLOCK_MASK      0x00000010
#define TESTVFS_SHMMAP_MASK       0x00000020
#define TESTVFS_SHMBARRIER_MASK   0x00000040
#define TESTVFS_SHMCLOSE_MASK     0x00000080

#define TESTVFS_OPEN_MASK         0x00000100
#define TESTVFS_SYNC_MASK         0x00000200
#define TESTVFS_DELETE_MASK       0x00000400
#define TESTVFS_CLOSE_MASK        0x00000800
#define TESTVFS_WRITE_MASK        0x00001000
#define TESTVFS_TRUNCATE_MASK     0x00002000
#define TESTVFS_ACCESS_MASK       0x00004000
#define TESTVFS_FULLPATHNAME_MASK 0x00008000
#define TESTVFS_READ_MASK         0x00010000
#define TESTVFS_UNLOCK_MASK       0x00020000
#define TESTVFS_LOCK_MASK         0x00040000
#define TESTVFS_CKLOCK_MASK       0x00080000
#define TESTVFS_FCNTL_MASK        0x00100000
#define TESTVFS_SLEEP_MASK        0x00200000

#define TESTVFS_ALL_MASK          0x003FFFFF


#define TESTVFS_MAX_PAGES 1024

/*
** A shared-memory buffer. There is one of these objects for each shared
** memory region opened by clients. If two clients open the same file,
** there are two TestvfsFile structures but only one TestvfsBuffer structure.
*/
ساختار TestvfsBuffer {
  نویسه *zFile;                    /* Associated file name */
  صحیح pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  TestvfsBuffer *pNext;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->pAppData))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
ایستا صحیح tvfsClose(sqlite3_file*);
ایستا صحیح tvfsRead(sqlite3_file*, تهی*, صحیح iAmt, sqlite3_int64 iOfst);
ایستا صحیح tvfsWrite(sqlite3_file*,ثابت تهی*,صحیح iAmt, sqlite3_int64 iOfst);
ایستا صحیح tvfsTruncate(sqlite3_file*, sqlite3_int64 size);
ایستا صحیح tvfsSync(sqlite3_file*, صحیح flags);
ایستا صحیح tvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ایستا صحیح tvfsLock(sqlite3_file*, صحیح);
ایستا صحیح tvfsUnlock(sqlite3_file*, صحیح);
ایستا صحیح tvfsCheckReservedLock(sqlite3_file*, صحیح *);
ایستا صحیح tvfsFileControl(sqlite3_file*, صحیح op, تهی *pArg);
ایستا صحیح tvfsSectorSize(sqlite3_file*);
ایستا صحیح tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
ایستا صحیح tvfsOpen(sqlite3_vfs*, ثابت نویسه *, sqlite3_file*, صحیح , صحیح *);
ایستا صحیح tvfsDelete(sqlite3_vfs*, ثابت نویسه *zName, صحیح syncDir);
ایستا صحیح tvfsAccess(sqlite3_vfs*, ثابت نویسه *zName, صحیح flags, صحیح *);
ایستا صحیح tvfsFullPathname(sqlite3_vfs*, ثابت نویسه *zName, صحیح, نویسه *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
ایستا تهی *tvfsDlOpen(sqlite3_vfs*, ثابت نویسه *zFilename);
ایستا تهی tvfsDlError(sqlite3_vfs*, صحیح nByte, نویسه *z_Err_پیام);
ایستا تهی (*tvfsDlSym(sqlite3_vfs*,تهی*, ثابت نویسه *z_نماد))(تهی);
ایستا تهی tvfsDlClose(sqlite3_vfs*, تهی*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
ایستا صحیح tvfsRandomness(sqlite3_vfs*, صحیح nByte, نویسه *zOut);
ایستا صحیح tvfsSleep(sqlite3_vfs*, صحیح microseconds);
ایستا صحیح tvfsCurrentTime(sqlite3_vfs*, دوگانه*);

ایستا صحیح tvfsShmOpen(sqlite3_file*);
ایستا صحیح tvfsShmLock(sqlite3_file*, صحیح , صحیح, صحیح);
ایستا صحیح tvfsShmMap(sqlite3_file*,صحیح,صحیح,صحیح, تهی فرار **);
ایستا تهی tvfsShmBarrier(sqlite3_file*);
ایستا صحیح tvfsShmUnmap(sqlite3_file*, صحیح);

ایستا صحیح tvfsFetch(sqlite3_file*, sqlite3_int64, صحیح, تهی**);
ایستا صحیح tvfsUnfetch(sqlite3_file*, sqlite3_int64, تهی*);

ایستا sqlite3_io_methods tvfs_io_methods = {
  3,                              /* iVersion */
  tvfsClose,                      /* xClose */
  tvfsRead,                       /* xRead */
  tvfsWrite,                      /* xWrite */
  tvfsTruncate,                   /* xTruncate */
  tvfsSync,                       /* xSync */
  tvfsFileSize,                   /* xFileSize */
  tvfsLock,                       /* xLock */
  tvfsUnlock,                     /* xUnlock */
  tvfsCheckReservedLock,          /* xCheckReservedLock */
  tvfsFileControl,                /* xFileControl */
  tvfsSectorSize,                 /* xSectorSize */
  tvfsDeviceCharacteristics,      /* xDeviceCharacteristics */
  tvfsShmMap,                     /* xShmMap */
  tvfsShmLock,                    /* xShmLock */
  tvfsShmBarrier,                 /* xShmBarrier */
  tvfsShmUnmap,                   /* xShmUnmap */
  tvfsFetch,
  tvfsUnfetch
};

ایستا صحیح tvfsResultCode(Testvfs *p, صحیح *pRc){
  ساختار errcode {
    صحیح eCode;
    ثابت نویسه *zCode;
  } aCode[] = {
    { SQLITE_OK,       "SQLITE_OK"     },
    { SQLITE_ERROR,    "SQLITE_ERROR"  },
    { SQLITE_IOERR,    "SQLITE_IOERR"  },
    { SQLITE_LOCKED,   "SQLITE_LOCKED" },
    { SQLITE_BUSY,     "SQLITE_BUSY"   },
    { SQLITE_READONLY, "SQLITE_READONLY"   },
    { SQLITE_READONLY_CANTINIT, "SQLITE_READONLY_CANTINIT"   },
    { SQLITE_NOTFOUND, "SQLITE_NOTFOUND"   },
    { -1,              "SQLITE_OMIT"   },
  };

  ثابت نویسه *z;
  صحیح i;

  z = Tcl_GetStringResult(p->interp);
  برای(i=0; i<ArraySize(aCode); i++){
    اگر( 0==strcmp(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      بازگشت 1;
    }
  }

  بازگشت 0;
}

ایستا صحیح tvfsInjectFault(TestFaultInject *p){
  صحیح ret = 0;
  اگر( p->eFault ){
    p->iCnt--;
    اگر( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  بازگشت ret;
}


ایستا صحیح tvfsInjectIoerr(Testvfs *p){
  بازگشت tvfsInjectFault(&p->ioerr_err);
}

ایستا صحیح tvfsInjectFullerr(Testvfs *p){
  بازگشت tvfsInjectFault(&p->full_err);
}
ایستا صحیح tvfsInjectCantopenerr(Testvfs *p){
  بازگشت tvfsInjectFault(&p->cantopen_err);
}


ایستا تهی tvfsExecTcl(
  Testvfs *p, 
  ثابت نویسه *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  صحیح rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  أكد( p->pScript );

  أكد( zMethod );
  أكد( p );
  أكد( arg2==0 || arg1!=0 );
  أكد( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_شمار(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  اگر( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  اگر( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  اگر( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  اگر( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  اگر( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_ResetResult(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
ایستا صحیح tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  اگر( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  اگر( pFd->pShmId ){
    Tcl_Decr_Ref_شمار(pFd->pShmId);
    pFd->pShmId = 0;
  }
  اگر( pFile->pMethods ){
    ckfree((نویسه *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((نویسه *)pFd);
  pTestfile->pFd = 0;
  بازگشت SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
ایستا صحیح tvfsRead(
  sqlite3_file *pFile, 
  تهی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  اگر( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  اگر( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  اگر( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  بازگشت rc;
}

/*
** Write data to an tvfs-file.
*/
ایستا صحیح tvfsWrite(
  sqlite3_file *pFile, 
  ثابت تهی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  اگر( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfsResultCode(p, &rc);
    اگر( rc<0 ) بازگشت SQLITE_OK;
  }

  اگر( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  اگر( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  اگر( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  بازگشت rc;
}

/*
** Truncate an tvfs-file.
*/
ایستا صحیح tvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  اگر( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  
  اگر( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, size);
  }
  بازگشت rc;
}

/*
** Sync an tvfs-file.
*/
ایستا صحیح tvfsSync(sqlite3_file *pFile, صحیح flags){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  اگر( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    نویسه *zFlags = 0;

    انتخاب( flags ){
      حالت SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        بشکن;
      حالت SQLITE_SYNC_FULL:
        zFlags = "full";
        بشکن;
      حالت SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        بشکن;
      حالت SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        بشکن;
      پیشفرض:
        أكد(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  اگر( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  اگر( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  بازگشت rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
ایستا صحیح tvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  TestvfsFd *p = tvfsGetFd(pFile);
  بازگشت sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an tvfs-file.
*/
ایستا صحیح tvfsLock(sqlite3_file *pFile, صحیح eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  اگر( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    نویسه zLock[30];
    sqlite3_snprintf(اندازه(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  اگر( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    بازگشت SQLITE_IOERR_LOCK;
  }
  بازگشت sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
ایستا صحیح tvfsUnlock(sqlite3_file *pFile, صحیح eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  اگر( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    نویسه zLock[30];
    sqlite3_snprintf(اندازه(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  اگر( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    بازگشت SQLITE_IOERR_UNLOCK;
  }
  بازگشت sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
ایستا صحیح tvfsCheckReservedLock(sqlite3_file *pFile, صحیح *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  اگر( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  بازگشت sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
ایستا صحیح tvfsFileControl(sqlite3_file *pFile, صحیح op, تهی *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  اگر( op==SQLITE_FCNTL_PRAGMA ){
    نویسه **argv = (نویسه**)pArg;
    اگر( sqlite3_stricmp(argv[1],"error")==0 ){
      صحیح rc = SQLITE_ERROR;
      اگر( argv[2] ){
        ثابت نویسه *z = argv[2];
        صحیح x = atoi(z);
        اگر( x ){
          rc = x;
          وقتی( sqlite3Isdigit(z[0]) ){ z++; }
          وقتی( sqlite3Isspace(z[0]) ){ z++; }
        }
        اگر( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      بازگشت rc;
    }
    اگر( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      بازگشت SQLITE_OK;
    }
  }
  اگر( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    ساختار Fcntl {
      صحیح iFnctl;
      ثابت نویسه *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    صحیح i;
    برای(i=0; i<اندازه(aF)/اندازه(aF[0]); i++){
      اگر( op==aF[i].iFnctl ) بشکن;
    }
    اگر( i<اندازه(aF)/اندازه(aF[0]) ){
      صحیح rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfsResultCode(p, &rc);
      اگر( rc ) بازگشت (rc<0 ? SQLITE_OK : rc);
    }
  }
  بازگشت sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
ایستا صحیح tvfsSectorSize(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  اگر( p->iSectorsize>=0 ){
    بازگشت p->iSectorsize;
  }
  بازگشت sqlite3OsSectorSize(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
ایستا صحیح tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  اگر( p->iDevchar>=0 ){
    بازگشت p->iDevchar;
  }
  بازگشت sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
ایستا صحیح tvfsOpen(
  sqlite3_vfs *pVfs,
  ثابت نویسه *zName,
  sqlite3_file *pFile,
  صحیح flags,
  صحیح *pOutFlags
){
  صحیح rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  pFd = (TestvfsFd *)ckalloc(اندازه(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, اندازه(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, اندازه(TestvfsFile));
  pTestfile->pFd = pFd;

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xOpen FILENAME KEY-VALUE-ARGS
  **
  ** If the script returns an SQLite error code other than SQLITE_OK, an
  ** error is returned to the caller. If it returns SQLITE_OK, the new
  ** connection is named "anon". Otherwise, the value returned by the
  ** script is used as the connection name.
  */
  Tcl_ResetResult(p->interp);
  اگر( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_شمار(pArg);
    اگر( flags&SQLITE_OPEN_MAIN_DB ){
      ثابت نویسه *z = &zName[strlen(zName)+1];
      وقتی( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_شمار(pArg);
    اگر( tvfsResultCode(p, &rc) ){
      اگر( rc!=SQLITE_OK ) بازگشت rc;
    }وگرنه{
      pId = Tcl_GetObjResult(p->interp);
    }
  }

  اگر( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) بازگشت SQLITE_IOERR;
  اگر( tvfsInjectCantopenerr(p) ) بازگشت SQLITE_CANTOPEN;
  اگر( tvfsInjectFullerr(p) ) بازگشت SQLITE_FULL;

  اگر( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_شمار(pId);
  pFd->pShmId = pId;
  Tcl_ResetResult(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  اگر( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    صحیح nByte;

    اگر( pVfs->iVersion>1 ){
      nByte = اندازه(sqlite3_io_methods);
    }وگرنه{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    memcpy(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    اگر( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    اگر( pVfs->iVersion>1 && ((Testvfs *)pVfs->pAppData)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  بازگشت rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ایستا صحیح tvfsDelete(sqlite3_vfs *pVfs, ثابت نویسه *zPath, صحیح dirSync){
  صحیح rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  اگر( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  اگر( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  بازگشت rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ایستا صحیح tvfsAccess(
  sqlite3_vfs *pVfs, 
  ثابت نویسه *zPath, 
  صحیح flags, 
  صحیح *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  اگر( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    صحیح rc;
    نویسه *zArg = 0;
    اگر( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    اگر( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    اگر( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    اگر( tvfsResultCode(p, &rc) ){
      اگر( rc!=SQLITE_OK ) بازگشت rc;
    }وگرنه{
      Tcl_Interp *interp = p->interp;
      اگر( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_GetObjResult(interp), pResOut) ){
        بازگشت SQLITE_OK;
      }
    }
  }
  بازگشت sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ایستا صحیح tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  ثابت نویسه *zPath, 
  صحیح nOut, 
  نویسه *zOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  اگر( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    صحیح rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    اگر( tvfsResultCode(p, &rc) ){
      اگر( rc!=SQLITE_OK ) بازگشت rc;
    }
  }
  بازگشت sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ایستا تهی *tvfsDlOpen(sqlite3_vfs *pVfs, ثابت نویسه *zPath){
  بازگشت sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ایستا تهی tvfsDlError(sqlite3_vfs *pVfs, صحیح nByte, نویسه *z_Err_پیام){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_پیام);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ایستا تهی (*tvfsDlSym(sqlite3_vfs *pVfs, تهی *p, ثابت نویسه *zSym))(تهی){
  بازگشت sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ایستا تهی tvfsDlClose(sqlite3_vfs *pVfs, تهی *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ایستا صحیح tvfsRandomness(sqlite3_vfs *pVfs, صحیح nByte, نویسه *zBufOut){
  بازگشت sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ایستا صحیح tvfsSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  اگر( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  بازگشت sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ایستا صحیح tvfsCurrentTime(sqlite3_vfs *pVfs, دوگانه *pTimeOut){
  بازگشت PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

ایستا صحیح tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  صحیح rc = SQLITE_OK;             /* Return code */
  TestvfsBuffer *pBuffer;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->pAppData;
  أكد( 0==p->isFullshm );
  أكد( pFd->pShmId && pFd->pShm==0 && pFd->pNext==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_ResetResult(p->interp);
  اگر( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    اگر( tvfsResultCode(p, &rc) ){
      اگر( rc!=SQLITE_OK ) بازگشت rc;
    }
  }

  أكد( rc==SQLITE_OK );
  اگر( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    بازگشت SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  برای(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
    اگر( 0==strcmp(pFd->zFilename, pBuffer->zFile) ) بشکن;
  }
  اگر( !pBuffer ){
    صحیح szName = (صحیح)strlen(pFd->zFilename);
    صحیح nByte = اندازه(TestvfsBuffer) + szName + 1;
    pBuffer = (TestvfsBuffer *)ckalloc(nByte);
    memset(pBuffer, 0, nByte);
    pBuffer->zFile = (نویسه *)&pBuffer[1];
    memcpy(pBuffer->zFile, pFd->zFilename, szName+1);
    pBuffer->pNext = p->pBuffer;
    p->pBuffer = pBuffer;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->pNext = pBuffer->pFile;
  pBuffer->pFile = pFd;
  pFd->pShm = pBuffer;
  بازگشت rc;
}

ایستا تهی tvfsAllocPage(TestvfsBuffer *p, صحیح iPage, صحیح pgsz){
  أكد( iPage<TESTVFS_MAX_PAGES );
  اگر( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    memset(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

ایستا صحیح tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  صحیح iPage,                      /* Page to retrieve */
  صحیح pgsz,                       /* Size of pages */
  صحیح isWrite,                    /* True to extend file if necessary */
  تهی فرار **pp              /* OUT: Mapped memory */
){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  اگر( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    بازگشت pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  اگر( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    اگر( rc!=SQLITE_OK ){
      بازگشت rc;
    }
  }

  اگر( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_شمار(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(iPage));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfsResultCode(p, &rc);
    Tcl_Decr_Ref_شمار(pArg);
  }
  اگر( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  اگر( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  اگر( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (تهی فرار *)pFd->pShm->aPage[iPage];
  }

  بازگشت rc;
}


ایستا صحیح tvfsShmLock(
  sqlite3_file *pFile,
  صحیح ofst,
  صحیح n,
  صحیح flags
){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  صحیح nLock;
  نویسه zLock[80];

  اگر( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    بازگشت pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  اگر( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(اندازه(zLock), zLock, "%d %d", ofst, n);
    nLock = (صحیح)strlen(zLock);
    اگر( flags & SQLITE_SHM_LOCK ){
      strcpy(&zLock[nLock], " lock");
    }وگرنه{
      strcpy(&zLock[nLock], " unlock");
    }
    nLock += (صحیح)strlen(&zLock[nLock]);
    اگر( flags & SQLITE_SHM_SHARED ){
      strcpy(&zLock[nLock], " shared");
    }وگرنه{
      strcpy(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  اگر( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  اگر( rc==SQLITE_OK ){
    صحیح isLock = (flags & SQLITE_SHM_LOCK);
    صحیح isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    اگر( isLock ){
      TestvfsFd *p2;
      برای(p2=pFd->pShm->pFile; p2; p2=p2->pNext){
        اگر( p2==pFd ) ادامه;
        اگر( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          بشکن;
        }
      }
      اگر( rc==SQLITE_OK ){
        اگر( isExcl )  pFd->excllock |= mask;
        اگر( !isExcl ) pFd->sharedlock |= mask;
      }
    }وگرنه{
      اگر( isExcl )  pFd->excllock &= (~mask);
      اگر( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  بازگشت rc;
}

ایستا تهی tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  اگر( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    ثابت نویسه *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  اگر( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    بازگشت;
  }
}

ایستا صحیح tvfsShmUnmap(
  sqlite3_file *pFile,
  صحیح deleteFlag
){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  TestvfsBuffer *pBuffer = pFd->pShm;
  TestvfsFd **ppFd;

  اگر( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    بازگشت pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  اگر( !pBuffer ) بازگشت SQLITE_OK;
  أكد( pFd->pShmId && pFd->pShm );

  اگر( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }

  برای(ppFd=&pBuffer->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->pNext));
  أكد( (*ppFd)==pFd );
  *ppFd = pFd->pNext;
  pFd->pNext = 0;

  اگر( pBuffer->pFile==0 ){
    صحیح i;
    TestvfsBuffer **pp;
    برای(pp=&p->pBuffer; *pp!=pBuffer; pp=&((*pp)->pNext));
    *pp = (*pp)->pNext;
    برای(i=0; pBuffer->aPage[i]; i++){
      ckfree((نویسه *)pBuffer->aPage[i]);
    }
    ckfree((نویسه *)pBuffer);
  }
  pFd->pShm = 0;

  بازگشت rc;
}

ایستا صحیح tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    صحیح iAmt, 
    تهی **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  بازگشت sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

ایستا صحیح tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, تهی *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  بازگشت sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

ایستا صحیح SQLITE_TCLAPI testvfs_obj_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  شمارش DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  ساختار TestvfsSubcmd {
    نویسه *zName;
    شمارش DB_enum eCmd;
  } aSubcmd[] = {
    { "shm",         CMD_SHM         },
    { "delete",      CMD_DELETE      },
    { "filter",      CMD_FILTER      },
    { "ioerr",       CMD_IOERR       },
    { "fullerr",     CMD_FULLERR     },
    { "cantopenerr", CMD_CANTOPENERR },
    { "script",      CMD_SCRIPT      },
    { "devchar",     CMD_DEVCHAR     },
    { "sectorsize",  CMD_SECTORSIZE  },
    { 0, 0 }
  };
  صحیح i;
  
  اگر( objc<2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "SUBCOMMAND ...");
    بازگشت TCL_ERROR;
  }
  اگر( Tcl_GetIndexFromObjStruct(
        interp, objv[1], aSubcmd, اندازه(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    بازگشت TCL_ERROR;
  }
  Tcl_ResetResult(interp);

  انتخاب( aSubcmd[i].eCmd ){
    حالت CMD_SHM: {
      Tcl_Obj *pObj;
      صحیح rc;
      TestvfsBuffer *pBuffer;
      نویسه *zName;
      اگر( objc!=3 && objc!=4 ){
        Tcl_Wrong_رقم_Args(interp, 2, objv, "FILE ?VALUE?");
        بازگشت TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      اگر( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), پوچ);
        ckfree(zName);
        بازگشت TCL_ERROR;
      }
      برای(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
        اگر( 0==strcmp(pBuffer->zFile, zName) ) بشکن;
      }
      ckfree(zName);
      اگر( !pBuffer ){
        Tcl_AppendResult(interp, "no such file: ", Tcl_GetString(objv[2]), پوچ);
        بازگشت TCL_ERROR;
      }
      اگر( objc==4 ){
        Tcl_Size n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        صحیح pgsz = pBuffer->pgsz;
        اگر( pgsz==0 ) pgsz = 65536;
        برای(i=0; i*pgsz<(صحیح)n; i++){
          صحیح nByte = pgsz;
          tvfsAllocPage(pBuffer, i, pgsz);
          اگر( n-i*pgsz<pgsz ){
            nByte = (صحیح)n;
          }
          memcpy(pBuffer->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      برای(i=0; pBuffer->aPage[i]; i++){
        صحیح pgsz = pBuffer->pgsz;
        اگر( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(pBuffer->aPage[i], pgsz));
      }
      Tcl_SetObjResult(interp, pObj);
      بشکن;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    حالت CMD_FILTER: {
      ایستا ساختار VfsMethod {
        نویسه *zName;
        صحیح mask;
      } vfsmethod [] = {
        { "xShmOpen",           TESTVFS_SHMOPEN_MASK },
        { "xShmLock",           TESTVFS_SHMLOCK_MASK },
        { "xShmBarrier",        TESTVFS_SHMBARRIER_MASK },
        { "xShmUnmap",          TESTVFS_SHMCLOSE_MASK },
        { "xShmMap",            TESTVFS_SHMMAP_MASK },
        { "xSync",              TESTVFS_SYNC_MASK },
        { "xDelete",            TESTVFS_DELETE_MASK },
        { "xWrite",             TESTVFS_WRITE_MASK },
        { "xRead",              TESTVFS_READ_MASK },
        { "xTruncate",          TESTVFS_TRUNCATE_MASK },
        { "xOpen",              TESTVFS_OPEN_MASK },
        { "xClose",             TESTVFS_CLOSE_MASK },
        { "xAccess",            TESTVFS_ACCESS_MASK },
        { "xFullPathname",      TESTVFS_FULLPATHNAME_MASK },
        { "xUnlock",            TESTVFS_UNLOCK_MASK },
        { "xLock",              TESTVFS_LOCK_MASK },
        { "xCheckReservedLock", TESTVFS_CKLOCK_MASK },
        { "xFileControl",       TESTVFS_FCNTL_MASK },
        { "xSleep",             TESTVFS_SLEEP_MASK },
      };
      Tcl_Obj **apElem = 0;
      Tcl_Size nElem = 0;
      صحیح mask = 0;
      اگر( objc!=3 ){
        Tcl_Wrong_رقم_Args(interp, 2, objv, "LIST");
        بازگشت TCL_ERROR;
      }
      اگر( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        بازگشت TCL_ERROR;
      }
      Tcl_ResetResult(interp);
      برای(i=0; i<(صحیح)nElem; i++){
        صحیح iMethod;
        نویسه *zElem = Tcl_GetString(apElem[i]);
        برای(iMethod=0; iMethod<ArraySize(vfsmethod); iMethod++){
          اگر( strcmp(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            بشکن;
          }
        }
        اگر( iMethod==ArraySize(vfsmethod) ){
          Tcl_AppendResult(interp, "unknown method: ", zElem, پوچ);
          بازگشت TCL_ERROR;
        }
      }
      p->mask = mask;
      بشکن;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    حالت CMD_SCRIPT: {
      اگر( objc==3 ){
        Tcl_Size nByte;
        اگر( p->pScript ){
          Tcl_Decr_Ref_شمار(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        اگر( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_شمار(p->pScript);
        }
      }وگرنه اگر( objc!=2 ){
        Tcl_Wrong_رقم_Args(interp, 2, objv, "?SCRIPT?");
        بازگشت TCL_ERROR;
      }

      Tcl_ResetResult(interp);
      اگر( p->pScript ) Tcl_SetObjResult(interp, p->pScript);

      بشکن;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    حالت CMD_CANTOPENERR:
    حالت CMD_IOERR:
    حالت CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      صحیح iRet;

      انتخاب( aSubcmd[i].eCmd ){
        حالت CMD_IOERR: pTest = &p->ioerr_err; بشکن;
        حالت CMD_FULLERR: pTest = &p->full_err; بشکن;
        حالت CMD_CANTOPENERR: pTest = &p->cantopen_err; بشکن;
        پیشفرض: أكد(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      اگر( objc==4 ){
        صحیح iCnt, iPersist;
        اگر( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          بازگشت TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }وگرنه اگر( objc!=2 ){
        Tcl_Wrong_رقم_Args(interp, 2, objv, "?CNT PERSIST?");
        بازگشت TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(iRet));
      بشکن;
    }

    حالت CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      بشکن;
    }

    حالت CMD_DEVCHAR: {
      ساختار DeviceFlag {
        نویسه *zName;
        صحیح i_مقدار;
      } aFlag[] = {
        { "default",               -1 },
        { "atomic",                SQLITE_IOCAP_ATOMIC                },
        { "atomic512",             SQLITE_IOCAP_ATOMIC512             },
        { "atomic1k",              SQLITE_IOCAP_ATOMIC1K              },
        { "atomic2k",              SQLITE_IOCAP_ATOMIC2K              },
        { "atomic4k",              SQLITE_IOCAP_ATOMIC4K              },
        { "atomic8k",              SQLITE_IOCAP_ATOMIC8K              },
        { "atomic16k",             SQLITE_IOCAP_ATOMIC16K             },
        { "atomic32k",             SQLITE_IOCAP_ATOMIC32K             },
        { "atomic64k",             SQLITE_IOCAP_ATOMIC64K             },
        { "sequential",            SQLITE_IOCAP_SEQUENTIAL            },
        { "safe_append",           SQLITE_IOCAP_SAFE_APPEND           },
        { "undeletable_when_open", SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN },
        { "powersafe_overwrite",   SQLITE_IOCAP_POWERSAFE_OVERWRITE   },
        { "immutable",             SQLITE_IOCAP_IMMUTABLE             },
        { 0, 0 }
      };
      Tcl_Obj *pRet;
      صحیح iFlag;

      اگر( objc>3 ){
        Tcl_Wrong_رقم_Args(interp, 2, objv, "?ATTR-LIST?");
        بازگشت TCL_ERROR;
      }
      اگر( objc==3 ){
        صحیح j;
        صحیح iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_Size nFlags = 0;

        اگر( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          بازگشت TCL_ERROR;
        }

        برای(j=0; j<(صحیح)nFlags; j++){
          صحیح idx = 0;
          اگر( Tcl_GetIndexFromObjStruct(interp, flags[j], aFlag, 
                اندازه(aFlag[0]), "flag", 0, &idx) 
          ){
            بازگشت TCL_ERROR;
          }
          اگر( aFlag[idx].i_مقدار<0 && nFlags>1 ){
            Tcl_AppendResult(interp, "bad flags: ", Tcl_GetString(objv[2]), پوچ);
            بازگشت TCL_ERROR;
          }
          iNew |= aFlag[idx].i_مقدار;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      برای(iFlag=0; iFlag<اندازه(aFlag)/اندازه(aFlag[0]); iFlag++){
        اگر( p->iDevchar & aFlag[iFlag].i_مقدار ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_SetObjResult(interp, pRet);

      بشکن;
    }

    حالت CMD_SECTORSIZE: {
      اگر( objc>3 ){
        Tcl_Wrong_رقم_Args(interp, 2, objv, "?VALUE?");
        بازگشت TCL_ERROR;
      }
      اگر( objc==3 ){
        صحیح iNew = 0;
        اگر( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          بازگشت TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(p->iSectorsize));
      بشکن;
    }
  }

  بازگشت TCL_OK;
}

ایستا تهی SQLITE_TCLAPI testvfs_obj_del(ClientData cd){
  Testvfs *p = (Testvfs *)cd;
  اگر( p->pScript ) Tcl_Decr_Ref_شمار(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, اندازه(sqlite3_vfs));
  ckfree((نویسه *)p->pVfs);
  memset(p, 0, اندازه(Testvfs));
  ckfree((نویسه *)p);
}

/*
** Usage:  testvfs VFSNAME ?SWITCHES?
**
** Switches are:
**
**   -noshm   BOOLEAN             (True to omit shm methods. Default false)
**   -default BOOLEAN             (True to make the vfs default. Default false)
**
** This command creates two things when it is invoked: an SQLite VFS, and
** a Tcl command. Both are named VFSNAME. The VFS is installed. It is not
** installed as the default VFS.
**
** The VFS passes all file I/O calls through to the underlying VFS.
**
** Whenever the xShmMap method of the VFS
** is invoked, the SCRIPT is executed as follows:
**
**   SCRIPT xShmMap    FILENAME ID
**
** The value returned by the invocation of SCRIPT above is interpreted as
** an SQLite error code and returned to SQLite. Either a symbolic 
** "SQLITE_OK" or numeric "0" value may be returned.
**
** The contents of the shared-memory buffer associated with a given file
** may be read and set using the following command:
**
**   VFSNAME shm FILENAME ?NEWVALUE?
**
** When the xShmLock method is invoked by SQLite, the following script is
** run:
**
**   SCRIPT xShmLock    FILENAME ID LOCK
**
** where LOCK is of the form "OFFSET NBYTE lock/unlock shared/exclusive"
*/
ایستا صحیح SQLITE_TCLAPI testvfs_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ایستا sqlite3_vfs tvfs_vfs = {
    3,                            /* iVersion */
    0,                            /* szOsFile */
    0,                            /* mxPathname */
    0,                            /* pNext */
    0,                            /* zName */
    0,                            /* pAppData */
    tvfsOpen,                     /* xOpen */
    tvfsDelete,                   /* xDelete */
    tvfsAccess,                   /* xAccess */
    tvfsFullPathname,             /* xFullPathname */
#ifndef SQLITE_OMIT_LOAD_EXTENSION
    tvfsDlOpen,                   /* xDlOpen */
    tvfsDlError,                  /* xDlError */
    tvfsDlSym,                    /* xDlSym */
    tvfsDlClose,                  /* xDlClose */
#وگرنه
    0,                            /* xDlOpen */
    0,                            /* xDlError */
    0,                            /* xDlSym */
    0,                            /* xDlClose */
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
    tvfsRandomness,               /* xRandomness */
    tvfsSleep,                    /* xSleep */
    tvfsCurrentTime,              /* xCurrentTime */
    0,                            /* xGetLastError */
    0,                            /* xCurrentTimeInt64 */
    0,                            /* xSetSystemCall */
    0,                            /* xGetSystemCall */
    0,                            /* xNextSystemCall */
  };

  Testvfs *p;                     /* New object */
  sqlite3_vfs *pVfs;              /* New VFS */
  نویسه *zVfs;
  صحیح nByte;                      /* Bytes of space to allocate at p */

  صحیح i;
  صحیح isNoshm = 0;                /* True if -noshm is passed */
  صحیح isFullshm = 0;              /* True if -fullshm is passed */
  صحیح isDefault = 0;              /* True if -default is passed */
  صحیح szOsFile = 0;               /* Value passed to -szosfile */
  صحیح mxPathname = -1;            /* Value passed to -mxpathname */
  صحیح iVersion = 3;               /* Value passed to -iversion */

  اگر( objc<2 || 0!=(objc%2) ) برو bad_args;
  برای(i=2; i<objc; i += 2){
    Tcl_Size nSwitch;
    نویسه *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    اگر( nSwitch>2 && 0==strncmp("-noshm", zSwitch, nSwitch) ){
      اگر( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        بازگشت TCL_ERROR;
      }
      اگر( isNoshm ) isFullshm = 0;
    }
    وگرنه اگر( nSwitch>2 && 0==strncmp("-default", zSwitch, nSwitch) ){
      اگر( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        بازگشت TCL_ERROR;
      }
    }
    وگرنه اگر( nSwitch>2 && 0==strncmp("-szosfile", zSwitch, nSwitch) ){
      اگر( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        بازگشت TCL_ERROR;
      }
    }
    وگرنه اگر( nSwitch>2 && 0==strncmp("-mxpathname", zSwitch, nSwitch) ){
      اگر( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        بازگشت TCL_ERROR;
      }
    }
    وگرنه اگر( nSwitch>2 && 0==strncmp("-iversion", zSwitch, nSwitch) ){
      اگر( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        بازگشت TCL_ERROR;
      }
    }
    وگرنه اگر( nSwitch>2 && 0==strncmp("-fullshm", zSwitch, nSwitch) ){
      اگر( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        بازگشت TCL_ERROR;
      }
      اگر( isFullshm ) isNoshm = 0;
    }
    وگرنه{
      برو bad_args;
    }
  }

  اگر( szOsFile<اندازه(TestvfsFile) ){
    szOsFile = اندازه(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = اندازه(Testvfs) + (صحیح)strlen(zVfs)+1;
  p = (Testvfs *)ckalloc(nByte);
  memset(p, 0, nByte);
  p->iDevchar = -1;
  p->iSectorsize = -1;

  /* Create the new object command before querying SQLite for a default VFS
  ** to use for 'real' IO operations. This is because creating the new VFS
  ** may delete an existing [testvfs] VFS of the same name. If such a VFS
  ** is currently the default, the new [testvfs] may end up calling the 
  ** methods of a deleted object.
  */
  Tcl_CreateObjCommand(interp, zVfs, testvfs_obj_cmd, p, testvfs_obj_del);
  p->pParent = sqlite3_vfs_find(0);
  p->interp = interp;

  p->zName = (نویسه *)&p[1];
  memcpy(p->zName, zVfs, strlen(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(اندازه(sqlite3_vfs));
  memcpy(pVfs, &tvfs_vfs, اندازه(sqlite3_vfs));
  pVfs->pAppData = (تهی *)p;
  pVfs->iVersion = iVersion;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  اگر( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  بازگشت TCL_OK;

 bad_args:
  Tcl_Wrong_رقم_Args(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  بازگشت TCL_ERROR;
}

بیرونی صحیح getDbPointer(Tcl_Interp *interp, ثابت نویسه *zA, sqlite3 **ppDb);
بیرونی ثابت نویسه *sqlite3ErrName(صحیح);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
ایستا صحیح SQLITE_TCLAPI test_vfs_shmlock(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت نویسه *azArg1[] = {"shared", "exclusive", 0};
  ثابت نویسه *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  صحیح rc = SQLITE_OK;
  ثابت نویسه *zDbname = 0;
  صحیح iArg1 = 0;
  صحیح iArg2 = 0;
  صحیح iOffset = 0;
  صحیح n = 0;
  sqlite3_file *pFd;

  اگر( objc!=7 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    بازگشت TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIndexFromObj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_GetIndexFromObj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    بازگشت TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (تهی*)&pFd);
  اگر( pFd==0 ){
    بازگشت TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_vfs_set_readmark(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  صحیح rc = SQLITE_OK;
  ثابت نویسه *zDbname = 0;
  صحیح iSlot = 0;
  صحیح iVal = -1;
  sqlite3_file *pFd;
  تهی فرار *pShm = 0;
  u32 *aShm;
  صحیح iOff;

  اگر( objc!=4 && objc!=5 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    بازگشت TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    بازگشت TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (تهی*)&pFd);
  اگر( pFd==0 ){
    بازگشت TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  اگر( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    بازگشت TCL_ERROR;
  }
  اگر( pShm==0 ){
    Tcl_AppendResult(interp, "*-shm is not yet mapped", پوچ);
    بازگشت TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  اگر( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(aShm[iOff]));

  بازگشت TCL_OK;
}

صحیح Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  بازگشت TCL_OK;
}

#endif
