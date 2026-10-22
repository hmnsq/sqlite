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
#إذا SQLITE_TEST          /* This file is used for testing only */

#ضمن "sqlite3.h"
#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"

تعريفنوع بنية Testvfs Testvfs;
تعريفنوع بنية TestvfsShm TestvfsShm;
تعريفنوع بنية TestvfsBuffer TestvfsBuffer;
تعريفنوع بنية TestvfsFile TestvfsFile;
تعريفنوع بنية TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
بنية TestvfsFile {
  sqlite3_file base;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#define tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

بنية TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  ثابت حرف *zFilename;          /* Filename as passed to xOpen() */
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

تعريفنوع بنية TestFaultInject TestFaultInject;
بنية TestFaultInject {
  صحيح iCnt;                       /* Remaining calls before fault injection */
  صحيح eFault;                     /* A FAULT_INJECT_* value */
  صحيح nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
بنية Testvfs {
  حرف *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  TestvfsBuffer *pBuffer;         /* List of shared buffers */
  صحيح isNoshm;
  صحيح isFullshm;

  صحيح mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#إذا 0
  صحيح iIoerrCnt;
  صحيح ioerr;
  صحيح nIoerrFail;
  صحيح iFullCnt;
  صحيح fullerr;
  صحيح nFullFail;
#endif

  صحيح iDevchar;
  صحيح iSectorsize;
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
بنية TestvfsBuffer {
  حرف *zFile;                    /* Associated file name */
  صحيح pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  TestvfsBuffer *pNext;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->pAppData))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
ساكن صحيح tvfsClose(sqlite3_file*);
ساكن صحيح tvfsRead(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح tvfsWrite(sqlite3_file*,ثابت فراغ*,صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح tvfsTruncate(sqlite3_file*, sqlite3_int64 size);
ساكن صحيح tvfsSync(sqlite3_file*, صحيح flags);
ساكن صحيح tvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ساكن صحيح tvfsLock(sqlite3_file*, صحيح);
ساكن صحيح tvfsUnlock(sqlite3_file*, صحيح);
ساكن صحيح tvfsCheckReservedLock(sqlite3_file*, صحيح *);
ساكن صحيح tvfsFileControl(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح tvfsSectorSize(sqlite3_file*);
ساكن صحيح tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
ساكن صحيح tvfsOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحيح , صحيح *);
ساكن صحيح tvfsDelete(sqlite3_vfs*, ثابت حرف *zName, صحيح syncDir);
ساكن صحيح tvfsAccess(sqlite3_vfs*, ثابت حرف *zName, صحيح flags, صحيح *);
ساكن صحيح tvfsFullPathname(sqlite3_vfs*, ثابت حرف *zName, صحيح, حرف *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
ساكن فراغ *tvfsDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن فراغ tvfsDlError(sqlite3_vfs*, صحيح nByte, حرف *z_Err_پەیام);
ساكن فراغ (*tvfsDlSym(sqlite3_vfs*,فراغ*, ثابت حرف *z_هێما))(فراغ);
ساكن فراغ tvfsDlClose(sqlite3_vfs*, فراغ*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
ساكن صحيح tvfsRandomness(sqlite3_vfs*, صحيح nByte, حرف *zOut);
ساكن صحيح tvfsSleep(sqlite3_vfs*, صحيح microseconds);
ساكن صحيح tvfsCurrentTime(sqlite3_vfs*, مزدوج*);

ساكن صحيح tvfsShmOpen(sqlite3_file*);
ساكن صحيح tvfsShmLock(sqlite3_file*, صحيح , صحيح, صحيح);
ساكن صحيح tvfsShmMap(sqlite3_file*,صحيح,صحيح,صحيح, فراغ متطاير **);
ساكن فراغ tvfsShmBarrier(sqlite3_file*);
ساكن صحيح tvfsShmUnmap(sqlite3_file*, صحيح);

ساكن صحيح tvfsFetch(sqlite3_file*, sqlite3_int64, صحيح, فراغ**);
ساكن صحيح tvfsUnfetch(sqlite3_file*, sqlite3_int64, فراغ*);

ساكن sqlite3_io_methods tvfs_io_methods = {
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

ساكن صحيح tvfsResultCode(Testvfs *p, صحيح *pRc){
  بنية errcode {
    صحيح eCode;
    ثابت حرف *zCode;
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

  ثابت حرف *z;
  صحيح i;

  z = Tcl_GetStringResult(p->interp);
  لكل(i=0; i<ArraySize(aCode); i++){
    إذا( 0==strcmp(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      أرجع 1;
    }
  }

  أرجع 0;
}

ساكن صحيح tvfsInjectFault(TestFaultInject *p){
  صحيح ret = 0;
  إذا( p->eFault ){
    p->iCnt--;
    إذا( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  أرجع ret;
}


ساكن صحيح tvfsInjectIoerr(Testvfs *p){
  أرجع tvfsInjectFault(&p->ioerr_err);
}

ساكن صحيح tvfsInjectFullerr(Testvfs *p){
  أرجع tvfsInjectFault(&p->full_err);
}
ساكن صحيح tvfsInjectCantopenerr(Testvfs *p){
  أرجع tvfsInjectFault(&p->cantopen_err);
}


ساكن فراغ tvfsExecTcl(
  Testvfs *p, 
  ثابت حرف *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  صحيح rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  أكد( p->pScript );

  أكد( zMethod );
  أكد( p );
  أكد( arg2==0 || arg1!=0 );
  أكد( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ژماردن(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  إذا( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  إذا( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  إذا( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  إذا( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  إذا( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_ResetResult(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
ساكن صحيح tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  إذا( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  إذا( pFd->pShmId ){
    Tcl_Decr_Ref_ژماردن(pFd->pShmId);
    pFd->pShmId = 0;
  }
  إذا( pFile->pMethods ){
    ckfree((حرف *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((حرف *)pFd);
  pTestfile->pFd = 0;
  أرجع SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
ساكن صحيح tvfsRead(
  sqlite3_file *pFile, 
  فراغ *zBuf, 
  صحيح iAmt, 
  sqlite_int64 iOfst
){
  صحيح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  إذا( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  إذا( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  إذا( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  أرجع rc;
}

/*
** Write data to an tvfs-file.
*/
ساكن صحيح tvfsWrite(
  sqlite3_file *pFile, 
  ثابت فراغ *zBuf, 
  صحيح iAmt, 
  sqlite_int64 iOfst
){
  صحيح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  إذا( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfsResultCode(p, &rc);
    إذا( rc<0 ) أرجع SQLITE_OK;
  }

  إذا( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  إذا( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  إذا( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  أرجع rc;
}

/*
** Truncate an tvfs-file.
*/
ساكن صحيح tvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  صحيح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  إذا( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  
  إذا( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, size);
  }
  أرجع rc;
}

/*
** Sync an tvfs-file.
*/
ساكن صحيح tvfsSync(sqlite3_file *pFile, صحيح flags){
  صحيح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  إذا( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    حرف *zFlags = 0;

    بدل( flags ){
      حالة SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        اكسر;
      حالة SQLITE_SYNC_FULL:
        zFlags = "full";
        اكسر;
      حالة SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        اكسر;
      حالة SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        اكسر;
      افتراضي:
        أكد(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  إذا( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  إذا( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  أرجع rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
ساكن صحيح tvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  TestvfsFd *p = tvfsGetFd(pFile);
  أرجع sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an tvfs-file.
*/
ساكن صحيح tvfsLock(sqlite3_file *pFile, صحيح eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  إذا( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    حرف zLock[30];
    sqlite3_snprintf(حجم(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  إذا( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    أرجع SQLITE_IOERR_LOCK;
  }
  أرجع sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
ساكن صحيح tvfsUnlock(sqlite3_file *pFile, صحيح eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  إذا( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    حرف zLock[30];
    sqlite3_snprintf(حجم(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  إذا( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    أرجع SQLITE_IOERR_UNLOCK;
  }
  أرجع sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
ساكن صحيح tvfsCheckReservedLock(sqlite3_file *pFile, صحيح *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  إذا( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  أرجع sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
ساكن صحيح tvfsFileControl(sqlite3_file *pFile, صحيح op, فراغ *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  إذا( op==SQLITE_FCNTL_PRAGMA ){
    حرف **argv = (حرف**)pArg;
    إذا( sqlite3_stricmp(argv[1],"error")==0 ){
      صحيح rc = SQLITE_ERROR;
      إذا( argv[2] ){
        ثابت حرف *z = argv[2];
        صحيح x = atoi(z);
        إذا( x ){
          rc = x;
          طالما( sqlite3Isdigit(z[0]) ){ z++; }
          طالما( sqlite3Isspace(z[0]) ){ z++; }
        }
        إذا( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      أرجع rc;
    }
    إذا( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      أرجع SQLITE_OK;
    }
  }
  إذا( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    بنية Fcntl {
      صحيح iFnctl;
      ثابت حرف *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    صحيح i;
    لكل(i=0; i<حجم(aF)/حجم(aF[0]); i++){
      إذا( op==aF[i].iFnctl ) اكسر;
    }
    إذا( i<حجم(aF)/حجم(aF[0]) ){
      صحيح rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfsResultCode(p, &rc);
      إذا( rc ) أرجع (rc<0 ? SQLITE_OK : rc);
    }
  }
  أرجع sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
ساكن صحيح tvfsSectorSize(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  إذا( p->iSectorsize>=0 ){
    أرجع p->iSectorsize;
  }
  أرجع sqlite3OsSectorSize(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
ساكن صحيح tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  إذا( p->iDevchar>=0 ){
    أرجع p->iDevchar;
  }
  أرجع sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
ساكن صحيح tvfsOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحيح flags,
  صحيح *pOutFlags
){
  صحيح rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  pFd = (TestvfsFd *)ckalloc(حجم(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, حجم(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, حجم(TestvfsFile));
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
  إذا( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_ژماردن(pArg);
    إذا( flags&SQLITE_OPEN_MAIN_DB ){
      ثابت حرف *z = &zName[strlen(zName)+1];
      طالما( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_ژماردن(pArg);
    إذا( tvfsResultCode(p, &rc) ){
      إذا( rc!=SQLITE_OK ) أرجع rc;
    }وإلا{
      pId = Tcl_GetObjResult(p->interp);
    }
  }

  إذا( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) أرجع SQLITE_IOERR;
  إذا( tvfsInjectCantopenerr(p) ) أرجع SQLITE_CANTOPEN;
  إذا( tvfsInjectFullerr(p) ) أرجع SQLITE_FULL;

  إذا( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_ژماردن(pId);
  pFd->pShmId = pId;
  Tcl_ResetResult(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  إذا( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    صحيح nByte;

    إذا( pVfs->iVersion>1 ){
      nByte = حجم(sqlite3_io_methods);
    }وإلا{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    memcpy(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    إذا( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    إذا( pVfs->iVersion>1 && ((Testvfs *)pVfs->pAppData)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  أرجع rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساكن صحيح tvfsDelete(sqlite3_vfs *pVfs, ثابت حرف *zPath, صحيح dirSync){
  صحيح rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  إذا( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  إذا( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  أرجع rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساكن صحيح tvfsAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *zPath, 
  صحيح flags, 
  صحيح *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  إذا( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    صحيح rc;
    حرف *zArg = 0;
    إذا( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    إذا( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    إذا( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    إذا( tvfsResultCode(p, &rc) ){
      إذا( rc!=SQLITE_OK ) أرجع rc;
    }وإلا{
      Tcl_Interp *interp = p->interp;
      إذا( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_GetObjResult(interp), pResOut) ){
        أرجع SQLITE_OK;
      }
    }
  }
  أرجع sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *zPath, 
  صحيح nOut, 
  حرف *zOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  إذا( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    صحيح rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    إذا( tvfsResultCode(p, &rc) ){
      إذا( rc!=SQLITE_OK ) أرجع rc;
    }
  }
  أرجع sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *tvfsDlOpen(sqlite3_vfs *pVfs, ثابت حرف *zPath){
  أرجع sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساكن فراغ tvfsDlError(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Err_پەیام){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_پەیام);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ساكن فراغ (*tvfsDlSym(sqlite3_vfs *pVfs, فراغ *p, ثابت حرف *zSym))(فراغ){
  أرجع sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ساكن فراغ tvfsDlClose(sqlite3_vfs *pVfs, فراغ *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساكن صحيح tvfsRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *zBufOut){
  أرجع sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساكن صحيح tvfsSleep(sqlite3_vfs *pVfs, صحيح nMicro){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  إذا( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  أرجع sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساكن صحيح tvfsCurrentTime(sqlite3_vfs *pVfs, مزدوج *pTimeOut){
  أرجع PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

ساكن صحيح tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  صحيح rc = SQLITE_OK;             /* Return code */
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
  إذا( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    إذا( tvfsResultCode(p, &rc) ){
      إذا( rc!=SQLITE_OK ) أرجع rc;
    }
  }

  أكد( rc==SQLITE_OK );
  إذا( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    أرجع SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  لكل(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
    إذا( 0==strcmp(pFd->zFilename, pBuffer->zFile) ) اكسر;
  }
  إذا( !pBuffer ){
    صحيح szName = (صحيح)strlen(pFd->zFilename);
    صحيح nByte = حجم(TestvfsBuffer) + szName + 1;
    pBuffer = (TestvfsBuffer *)ckalloc(nByte);
    memset(pBuffer, 0, nByte);
    pBuffer->zFile = (حرف *)&pBuffer[1];
    memcpy(pBuffer->zFile, pFd->zFilename, szName+1);
    pBuffer->pNext = p->pBuffer;
    p->pBuffer = pBuffer;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->pNext = pBuffer->pFile;
  pBuffer->pFile = pFd;
  pFd->pShm = pBuffer;
  أرجع rc;
}

ساكن فراغ tvfsAllocPage(TestvfsBuffer *p, صحيح iPage, صحيح pgsz){
  أكد( iPage<TESTVFS_MAX_PAGES );
  إذا( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    memset(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

ساكن صحيح tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  صحيح iPage,                      /* Page to retrieve */
  صحيح pgsz,                       /* Size of pages */
  صحيح isWrite,                    /* True to extend file if necessary */
  فراغ متطاير **pp              /* OUT: Mapped memory */
){
  صحيح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  إذا( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    أرجع pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  إذا( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
  }

  إذا( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_ژماردن(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(iPage));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfsResultCode(p, &rc);
    Tcl_Decr_Ref_ژماردن(pArg);
  }
  إذا( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  إذا( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  إذا( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (فراغ متطاير *)pFd->pShm->aPage[iPage];
  }

  أرجع rc;
}


ساكن صحيح tvfsShmLock(
  sqlite3_file *pFile,
  صحيح ofst,
  صحيح n,
  صحيح flags
){
  صحيح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  صحيح nLock;
  حرف zLock[80];

  إذا( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    أرجع pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  إذا( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(حجم(zLock), zLock, "%d %d", ofst, n);
    nLock = (صحيح)strlen(zLock);
    إذا( flags & SQLITE_SHM_LOCK ){
      strcpy(&zLock[nLock], " lock");
    }وإلا{
      strcpy(&zLock[nLock], " unlock");
    }
    nLock += (صحيح)strlen(&zLock[nLock]);
    إذا( flags & SQLITE_SHM_SHARED ){
      strcpy(&zLock[nLock], " shared");
    }وإلا{
      strcpy(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  إذا( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  إذا( rc==SQLITE_OK ){
    صحيح isLock = (flags & SQLITE_SHM_LOCK);
    صحيح isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    إذا( isLock ){
      TestvfsFd *p2;
      لكل(p2=pFd->pShm->pFile; p2; p2=p2->pNext){
        إذا( p2==pFd ) استمر;
        إذا( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          اكسر;
        }
      }
      إذا( rc==SQLITE_OK ){
        إذا( isExcl )  pFd->excllock |= mask;
        إذا( !isExcl ) pFd->sharedlock |= mask;
      }
    }وإلا{
      إذا( isExcl )  pFd->excllock &= (~mask);
      إذا( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  أرجع rc;
}

ساكن فراغ tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  إذا( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    ثابت حرف *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  إذا( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    أرجع;
  }
}

ساكن صحيح tvfsShmUnmap(
  sqlite3_file *pFile,
  صحيح deleteFlag
){
  صحيح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  TestvfsBuffer *pBuffer = pFd->pShm;
  TestvfsFd **ppFd;

  إذا( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    أرجع pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  إذا( !pBuffer ) أرجع SQLITE_OK;
  أكد( pFd->pShmId && pFd->pShm );

  إذا( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }

  لكل(ppFd=&pBuffer->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->pNext));
  أكد( (*ppFd)==pFd );
  *ppFd = pFd->pNext;
  pFd->pNext = 0;

  إذا( pBuffer->pFile==0 ){
    صحيح i;
    TestvfsBuffer **pp;
    لكل(pp=&p->pBuffer; *pp!=pBuffer; pp=&((*pp)->pNext));
    *pp = (*pp)->pNext;
    لكل(i=0; pBuffer->aPage[i]; i++){
      ckfree((حرف *)pBuffer->aPage[i]);
    }
    ckfree((حرف *)pBuffer);
  }
  pFd->pShm = 0;

  أرجع rc;
}

ساكن صحيح tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    صحيح iAmt, 
    فراغ **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  أرجع sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

ساكن صحيح tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, فراغ *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  أرجع sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

ساكن صحيح SQLITE_TCLAPI testvfs_obj_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  تعداد DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  بنية TestvfsSubcmd {
    حرف *zName;
    تعداد DB_enum eCmd;
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
  صحيح i;
  
  إذا( objc<2 ){
    Tcl_Wrong_ژمارە_Args(interp, 1, objv, "SUBCOMMAND ...");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_GetIndexFromObjStruct(
        interp, objv[1], aSubcmd, حجم(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    أرجع TCL_ERROR;
  }
  Tcl_ResetResult(interp);

  بدل( aSubcmd[i].eCmd ){
    حالة CMD_SHM: {
      Tcl_Obj *pObj;
      صحيح rc;
      TestvfsBuffer *pBuffer;
      حرف *zName;
      إذا( objc!=3 && objc!=4 ){
        Tcl_Wrong_ژمارە_Args(interp, 2, objv, "FILE ?VALUE?");
        أرجع TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      إذا( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), عدم);
        ckfree(zName);
        أرجع TCL_ERROR;
      }
      لكل(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
        إذا( 0==strcmp(pBuffer->zFile, zName) ) اكسر;
      }
      ckfree(zName);
      إذا( !pBuffer ){
        Tcl_AppendResult(interp, "no such file: ", Tcl_GetString(objv[2]), عدم);
        أرجع TCL_ERROR;
      }
      إذا( objc==4 ){
        Tcl_Size n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        صحيح pgsz = pBuffer->pgsz;
        إذا( pgsz==0 ) pgsz = 65536;
        لكل(i=0; i*pgsz<(صحيح)n; i++){
          صحيح nByte = pgsz;
          tvfsAllocPage(pBuffer, i, pgsz);
          إذا( n-i*pgsz<pgsz ){
            nByte = (صحيح)n;
          }
          memcpy(pBuffer->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      لكل(i=0; pBuffer->aPage[i]; i++){
        صحيح pgsz = pBuffer->pgsz;
        إذا( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(pBuffer->aPage[i], pgsz));
      }
      Tcl_SetObjResult(interp, pObj);
      اكسر;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    حالة CMD_FILTER: {
      ساكن بنية VfsMethod {
        حرف *zName;
        صحيح mask;
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
      صحيح mask = 0;
      إذا( objc!=3 ){
        Tcl_Wrong_ژمارە_Args(interp, 2, objv, "LIST");
        أرجع TCL_ERROR;
      }
      إذا( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        أرجع TCL_ERROR;
      }
      Tcl_ResetResult(interp);
      لكل(i=0; i<(صحيح)nElem; i++){
        صحيح iMethod;
        حرف *zElem = Tcl_GetString(apElem[i]);
        لكل(iMethod=0; iMethod<ArraySize(vfsmethod); iMethod++){
          إذا( strcmp(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            اكسر;
          }
        }
        إذا( iMethod==ArraySize(vfsmethod) ){
          Tcl_AppendResult(interp, "unknown method: ", zElem, عدم);
          أرجع TCL_ERROR;
        }
      }
      p->mask = mask;
      اكسر;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    حالة CMD_SCRIPT: {
      إذا( objc==3 ){
        Tcl_Size nByte;
        إذا( p->pScript ){
          Tcl_Decr_Ref_ژماردن(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        إذا( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_ژماردن(p->pScript);
        }
      }وإلا إذا( objc!=2 ){
        Tcl_Wrong_ژمارە_Args(interp, 2, objv, "?SCRIPT?");
        أرجع TCL_ERROR;
      }

      Tcl_ResetResult(interp);
      إذا( p->pScript ) Tcl_SetObjResult(interp, p->pScript);

      اكسر;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    حالة CMD_CANTOPENERR:
    حالة CMD_IOERR:
    حالة CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      صحيح iRet;

      بدل( aSubcmd[i].eCmd ){
        حالة CMD_IOERR: pTest = &p->ioerr_err; اكسر;
        حالة CMD_FULLERR: pTest = &p->full_err; اكسر;
        حالة CMD_CANTOPENERR: pTest = &p->cantopen_err; اكسر;
        افتراضي: أكد(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      إذا( objc==4 ){
        صحيح iCnt, iPersist;
        إذا( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          أرجع TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }وإلا إذا( objc!=2 ){
        Tcl_Wrong_ژمارە_Args(interp, 2, objv, "?CNT PERSIST?");
        أرجع TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(iRet));
      اكسر;
    }

    حالة CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      اكسر;
    }

    حالة CMD_DEVCHAR: {
      بنية DeviceFlag {
        حرف *zName;
        صحيح i_بەها;
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
      صحيح iFlag;

      إذا( objc>3 ){
        Tcl_Wrong_ژمارە_Args(interp, 2, objv, "?ATTR-LIST?");
        أرجع TCL_ERROR;
      }
      إذا( objc==3 ){
        صحيح j;
        صحيح iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_Size nFlags = 0;

        إذا( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          أرجع TCL_ERROR;
        }

        لكل(j=0; j<(صحيح)nFlags; j++){
          صحيح idx = 0;
          إذا( Tcl_GetIndexFromObjStruct(interp, flags[j], aFlag, 
                حجم(aFlag[0]), "flag", 0, &idx) 
          ){
            أرجع TCL_ERROR;
          }
          إذا( aFlag[idx].i_بەها<0 && nFlags>1 ){
            Tcl_AppendResult(interp, "bad flags: ", Tcl_GetString(objv[2]), عدم);
            أرجع TCL_ERROR;
          }
          iNew |= aFlag[idx].i_بەها;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      لكل(iFlag=0; iFlag<حجم(aFlag)/حجم(aFlag[0]); iFlag++){
        إذا( p->iDevchar & aFlag[iFlag].i_بەها ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_SetObjResult(interp, pRet);

      اكسر;
    }

    حالة CMD_SECTORSIZE: {
      إذا( objc>3 ){
        Tcl_Wrong_ژمارە_Args(interp, 2, objv, "?VALUE?");
        أرجع TCL_ERROR;
      }
      إذا( objc==3 ){
        صحيح iNew = 0;
        إذا( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          أرجع TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(p->iSectorsize));
      اكسر;
    }
  }

  أرجع TCL_OK;
}

ساكن فراغ SQLITE_TCLAPI testvfs_obj_del(ClientData cd){
  Testvfs *p = (Testvfs *)cd;
  إذا( p->pScript ) Tcl_Decr_Ref_ژماردن(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, حجم(sqlite3_vfs));
  ckfree((حرف *)p->pVfs);
  memset(p, 0, حجم(Testvfs));
  ckfree((حرف *)p);
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
ساكن صحيح SQLITE_TCLAPI testvfs_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  ساكن sqlite3_vfs tvfs_vfs = {
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
#وإلا
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
  حرف *zVfs;
  صحيح nByte;                      /* Bytes of space to allocate at p */

  صحيح i;
  صحيح isNoshm = 0;                /* True if -noshm is passed */
  صحيح isFullshm = 0;              /* True if -fullshm is passed */
  صحيح isDefault = 0;              /* True if -default is passed */
  صحيح szOsFile = 0;               /* Value passed to -szosfile */
  صحيح mxPathname = -1;            /* Value passed to -mxpathname */
  صحيح iVersion = 3;               /* Value passed to -iversion */

  إذا( objc<2 || 0!=(objc%2) ) اذهب bad_args;
  لكل(i=2; i<objc; i += 2){
    Tcl_Size nSwitch;
    حرف *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    إذا( nSwitch>2 && 0==strncmp("-noshm", zSwitch, nSwitch) ){
      إذا( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        أرجع TCL_ERROR;
      }
      إذا( isNoshm ) isFullshm = 0;
    }
    وإلا إذا( nSwitch>2 && 0==strncmp("-default", zSwitch, nSwitch) ){
      إذا( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        أرجع TCL_ERROR;
      }
    }
    وإلا إذا( nSwitch>2 && 0==strncmp("-szosfile", zSwitch, nSwitch) ){
      إذا( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        أرجع TCL_ERROR;
      }
    }
    وإلا إذا( nSwitch>2 && 0==strncmp("-mxpathname", zSwitch, nSwitch) ){
      إذا( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        أرجع TCL_ERROR;
      }
    }
    وإلا إذا( nSwitch>2 && 0==strncmp("-iversion", zSwitch, nSwitch) ){
      إذا( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        أرجع TCL_ERROR;
      }
    }
    وإلا إذا( nSwitch>2 && 0==strncmp("-fullshm", zSwitch, nSwitch) ){
      إذا( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        أرجع TCL_ERROR;
      }
      إذا( isFullshm ) isNoshm = 0;
    }
    وإلا{
      اذهب bad_args;
    }
  }

  إذا( szOsFile<حجم(TestvfsFile) ){
    szOsFile = حجم(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = حجم(Testvfs) + (صحيح)strlen(zVfs)+1;
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

  p->zName = (حرف *)&p[1];
  memcpy(p->zName, zVfs, strlen(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(حجم(sqlite3_vfs));
  memcpy(pVfs, &tvfs_vfs, حجم(sqlite3_vfs));
  pVfs->pAppData = (فراغ *)p;
  pVfs->iVersion = iVersion;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  إذا( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  أرجع TCL_OK;

 bad_args:
  Tcl_Wrong_ژمارە_Args(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  أرجع TCL_ERROR;
}

خارجي صحيح getDbPointer(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجي ثابت حرف *sqlite3ErrName(صحيح);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
ساكن صحيح SQLITE_TCLAPI test_vfs_shmlock(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *azArg1[] = {"shared", "exclusive", 0};
  ثابت حرف *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  صحيح rc = SQLITE_OK;
  ثابت حرف *zDbname = 0;
  صحيح iArg1 = 0;
  صحيح iArg2 = 0;
  صحيح iOffset = 0;
  صحيح n = 0;
  sqlite3_file *pFd;

  إذا( objc!=7 ){
    Tcl_Wrong_ژمارە_Args(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    أرجع TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  إذا( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIndexFromObj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_GetIndexFromObj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    أرجع TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (فراغ*)&pFd);
  إذا( pFd==0 ){
    أرجع TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_vfs_set_readmark(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  صحيح rc = SQLITE_OK;
  ثابت حرف *zDbname = 0;
  صحيح iSlot = 0;
  صحيح iVal = -1;
  sqlite3_file *pFd;
  فراغ متطاير *pShm = 0;
  u32 *aShm;
  صحيح iOff;

  إذا( objc!=4 && objc!=5 ){
    Tcl_Wrong_ژمارە_Args(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    أرجع TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  إذا( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    أرجع TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (فراغ*)&pFd);
  إذا( pFd==0 ){
    أرجع TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  إذا( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    أرجع TCL_ERROR;
  }
  إذا( pShm==0 ){
    Tcl_AppendResult(interp, "*-shm is not yet mapped", عدم);
    أرجع TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  إذا( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(aShm[iOff]));

  أرجع TCL_OK;
}

صحيح Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  أرجع TCL_OK;
}

#endif
