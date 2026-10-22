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
#எனில் SQLITE_TEST          /* This file is used for testing only */

#அந்தர்பாவ "sqlite3.h"
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"

வகைவரை கட்டமைப்பு Testvfs Testvfs;
வகைவரை கட்டமைப்பு TestvfsShm TestvfsShm;
வகைவரை கட்டமைப்பு TestvfsBuffer TestvfsBuffer;
வகைவரை கட்டமைப்பு TestvfsFile TestvfsFile;
வகைவரை கட்டமைப்பு TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
கட்டமைப்பு TestvfsFile {
  sqlite3_file base;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#define tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

கட்டமைப்பு TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  மாறிலி எழுத்து *zFilename;          /* Filename as passed to xOpen() */
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

வகைவரை கட்டமைப்பு TestFaultInject TestFaultInject;
கட்டமைப்பு TestFaultInject {
  முழு iCnt;                       /* Remaining calls before fault injection */
  முழு eFault;                     /* A FAULT_INJECT_* value */
  முழு nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
கட்டமைப்பு Testvfs {
  எழுத்து *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  TestvfsBuffer *pBuffer;         /* List of shared buffers */
  முழு isNoshm;
  முழு isFullshm;

  முழு mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#எனில் 0
  முழு iIoerrCnt;
  முழு ioerr;
  முழு nIoerrFail;
  முழு iFullCnt;
  முழு fullerr;
  முழு nFullFail;
#endif

  முழு iDevchar;
  முழு iSectorsize;
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
கட்டமைப்பு TestvfsBuffer {
  எழுத்து *zFile;                    /* Associated file name */
  முழு pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  TestvfsBuffer *pNext;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->pAppData))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
நிலையான முழு tvfsClose(sqlite3_file*);
நிலையான முழு tvfsRead(sqlite3_file*, வெறுமை*, முழு iAmt, sqlite3_int64 iOfst);
நிலையான முழு tvfsWrite(sqlite3_file*,மாறிலி வெறுமை*,முழு iAmt, sqlite3_int64 iOfst);
நிலையான முழு tvfsTruncate(sqlite3_file*, sqlite3_int64 size);
நிலையான முழு tvfsSync(sqlite3_file*, முழு flags);
நிலையான முழு tvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
நிலையான முழு tvfsLock(sqlite3_file*, முழு);
நிலையான முழு tvfsUnlock(sqlite3_file*, முழு);
நிலையான முழு tvfsCheckReservedLock(sqlite3_file*, முழு *);
நிலையான முழு tvfsFileControl(sqlite3_file*, முழு op, வெறுமை *pArg);
நிலையான முழு tvfsSectorSize(sqlite3_file*);
நிலையான முழு tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
நிலையான முழு tvfsOpen(sqlite3_vfs*, மாறிலி எழுத்து *, sqlite3_file*, முழு , முழு *);
நிலையான முழு tvfsDelete(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு syncDir);
நிலையான முழு tvfsAccess(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு flags, முழு *);
நிலையான முழு tvfsFullPathname(sqlite3_vfs*, மாறிலி எழுத்து *zName, முழு, எழுத்து *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
நிலையான வெறுமை *tvfsDlOpen(sqlite3_vfs*, மாறிலி எழுத்து *zFilename);
நிலையான வெறுமை tvfsDlError(sqlite3_vfs*, முழு nByte, எழுத்து *z_Err_செய்தி);
நிலையான வெறுமை (*tvfsDlSym(sqlite3_vfs*,வெறுமை*, மாறிலி எழுத்து *z_குறியீடு))(வெறுமை);
நிலையான வெறுமை tvfsDlClose(sqlite3_vfs*, வெறுமை*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
நிலையான முழு tvfsRandomness(sqlite3_vfs*, முழு nByte, எழுத்து *zOut);
நிலையான முழு tvfsSleep(sqlite3_vfs*, முழு microseconds);
நிலையான முழு tvfsCurrentTime(sqlite3_vfs*, இரட்டை*);

நிலையான முழு tvfsShmOpen(sqlite3_file*);
நிலையான முழு tvfsShmLock(sqlite3_file*, முழு , முழு, முழு);
நிலையான முழு tvfsShmMap(sqlite3_file*,முழு,முழு,முழு, வெறுமை நிலையற்ற **);
நிலையான வெறுமை tvfsShmBarrier(sqlite3_file*);
நிலையான முழு tvfsShmUnmap(sqlite3_file*, முழு);

நிலையான முழு tvfsFetch(sqlite3_file*, sqlite3_int64, முழு, வெறுமை**);
நிலையான முழு tvfsUnfetch(sqlite3_file*, sqlite3_int64, வெறுமை*);

நிலையான sqlite3_io_methods tvfs_io_methods = {
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

நிலையான முழு tvfsResultCode(Testvfs *p, முழு *pRc){
  கட்டமைப்பு errcode {
    முழு eCode;
    மாறிலி எழுத்து *zCode;
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

  மாறிலி எழுத்து *z;
  முழு i;

  z = Tcl_GetStringResult(p->interp);
  க்கு(i=0; i<ArraySize(aCode); i++){
    எனில்( 0==strcmp(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      திரும்பு 1;
    }
  }

  திரும்பு 0;
}

நிலையான முழு tvfsInjectFault(TestFaultInject *p){
  முழு ret = 0;
  எனில்( p->eFault ){
    p->iCnt--;
    எனில்( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  திரும்பு ret;
}


நிலையான முழு tvfsInjectIoerr(Testvfs *p){
  திரும்பு tvfsInjectFault(&p->ioerr_err);
}

நிலையான முழு tvfsInjectFullerr(Testvfs *p){
  திரும்பு tvfsInjectFault(&p->full_err);
}
நிலையான முழு tvfsInjectCantopenerr(Testvfs *p){
  திரும்பு tvfsInjectFault(&p->cantopen_err);
}


நிலையான வெறுமை tvfsExecTcl(
  Testvfs *p, 
  மாறிலி எழுத்து *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  முழு rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  ப்ரதிஜ்ஞா( p->pScript );

  ப்ரதிஜ்ஞா( zMethod );
  ப்ரதிஜ்ஞா( p );
  ப்ரதிஜ்ஞா( arg2==0 || arg1!=0 );
  ப்ரதிஜ்ஞா( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_மொத்தம்(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  எனில்( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  எனில்( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  எனில்( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  எனில்( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  எனில்( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_ResetResult(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
நிலையான முழு tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  எனில்( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  எனில்( pFd->pShmId ){
    Tcl_Decr_Ref_மொத்தம்(pFd->pShmId);
    pFd->pShmId = 0;
  }
  எனில்( pFile->pMethods ){
    ckfree((எழுத்து *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((எழுத்து *)pFd);
  pTestfile->pFd = 0;
  திரும்பு SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
நிலையான முழு tvfsRead(
  sqlite3_file *pFile, 
  வெறுமை *zBuf, 
  முழு iAmt, 
  sqlite_int64 iOfst
){
  முழு rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  எனில்( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  எனில்( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  எனில்( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  திரும்பு rc;
}

/*
** Write data to an tvfs-file.
*/
நிலையான முழு tvfsWrite(
  sqlite3_file *pFile, 
  மாறிலி வெறுமை *zBuf, 
  முழு iAmt, 
  sqlite_int64 iOfst
){
  முழு rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  எனில்( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfsResultCode(p, &rc);
    எனில்( rc<0 ) திரும்பு SQLITE_OK;
  }

  எனில்( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  எனில்( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  எனில்( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  திரும்பு rc;
}

/*
** Truncate an tvfs-file.
*/
நிலையான முழு tvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  முழு rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  எனில்( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  
  எனில்( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, size);
  }
  திரும்பு rc;
}

/*
** Sync an tvfs-file.
*/
நிலையான முழு tvfsSync(sqlite3_file *pFile, முழு flags){
  முழு rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  எனில்( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    எழுத்து *zFlags = 0;

    தேர்வு( flags ){
      நிலை SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        நிறுத்து;
      நிலை SQLITE_SYNC_FULL:
        zFlags = "full";
        நிறுத்து;
      நிலை SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        நிறுத்து;
      நிலை SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        நிறுத்து;
      இயல்பு:
        ப்ரதிஜ்ஞா(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  எனில்( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  எனில்( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  திரும்பு rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
நிலையான முழு tvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  TestvfsFd *p = tvfsGetFd(pFile);
  திரும்பு sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an tvfs-file.
*/
நிலையான முழு tvfsLock(sqlite3_file *pFile, முழு eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  எனில்( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    எழுத்து zLock[30];
    sqlite3_snprintf(அளவு(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  எனில்( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    திரும்பு SQLITE_IOERR_LOCK;
  }
  திரும்பு sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
நிலையான முழு tvfsUnlock(sqlite3_file *pFile, முழு eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  எனில்( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    எழுத்து zLock[30];
    sqlite3_snprintf(அளவு(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  எனில்( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    திரும்பு SQLITE_IOERR_UNLOCK;
  }
  திரும்பு sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
நிலையான முழு tvfsCheckReservedLock(sqlite3_file *pFile, முழு *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  எனில்( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  திரும்பு sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
நிலையான முழு tvfsFileControl(sqlite3_file *pFile, முழு op, வெறுமை *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  எனில்( op==SQLITE_FCNTL_PRAGMA ){
    எழுத்து **argv = (எழுத்து**)pArg;
    எனில்( sqlite3_stricmp(argv[1],"error")==0 ){
      முழு rc = SQLITE_ERROR;
      எனில்( argv[2] ){
        மாறிலி எழுத்து *z = argv[2];
        முழு x = atoi(z);
        எனில்( x ){
          rc = x;
          போது( sqlite3Isdigit(z[0]) ){ z++; }
          போது( sqlite3Isspace(z[0]) ){ z++; }
        }
        எனில்( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      திரும்பு rc;
    }
    எனில்( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      திரும்பு SQLITE_OK;
    }
  }
  எனில்( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    கட்டமைப்பு Fcntl {
      முழு iFnctl;
      மாறிலி எழுத்து *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    முழு i;
    க்கு(i=0; i<அளவு(aF)/அளவு(aF[0]); i++){
      எனில்( op==aF[i].iFnctl ) நிறுத்து;
    }
    எனில்( i<அளவு(aF)/அளவு(aF[0]) ){
      முழு rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfsResultCode(p, &rc);
      எனில்( rc ) திரும்பு (rc<0 ? SQLITE_OK : rc);
    }
  }
  திரும்பு sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
நிலையான முழு tvfsSectorSize(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  எனில்( p->iSectorsize>=0 ){
    திரும்பு p->iSectorsize;
  }
  திரும்பு sqlite3OsSectorSize(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
நிலையான முழு tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  எனில்( p->iDevchar>=0 ){
    திரும்பு p->iDevchar;
  }
  திரும்பு sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
நிலையான முழு tvfsOpen(
  sqlite3_vfs *pVfs,
  மாறிலி எழுத்து *zName,
  sqlite3_file *pFile,
  முழு flags,
  முழு *pOutFlags
){
  முழு rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  pFd = (TestvfsFd *)ckalloc(அளவு(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, அளவு(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, அளவு(TestvfsFile));
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
  எனில்( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_மொத்தம்(pArg);
    எனில்( flags&SQLITE_OPEN_MAIN_DB ){
      மாறிலி எழுத்து *z = &zName[strlen(zName)+1];
      போது( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_மொத்தம்(pArg);
    எனில்( tvfsResultCode(p, &rc) ){
      எனில்( rc!=SQLITE_OK ) திரும்பு rc;
    }இல்லையெனில்{
      pId = Tcl_GetObjResult(p->interp);
    }
  }

  எனில்( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) திரும்பு SQLITE_IOERR;
  எனில்( tvfsInjectCantopenerr(p) ) திரும்பு SQLITE_CANTOPEN;
  எனில்( tvfsInjectFullerr(p) ) திரும்பு SQLITE_FULL;

  எனில்( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_மொத்தம்(pId);
  pFd->pShmId = pId;
  Tcl_ResetResult(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  எனில்( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    முழு nByte;

    எனில்( pVfs->iVersion>1 ){
      nByte = அளவு(sqlite3_io_methods);
    }இல்லையெனில்{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    memcpy(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    எனில்( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    எனில்( pVfs->iVersion>1 && ((Testvfs *)pVfs->pAppData)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  திரும்பு rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
நிலையான முழு tvfsDelete(sqlite3_vfs *pVfs, மாறிலி எழுத்து *zPath, முழு dirSync){
  முழு rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  எனில்( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  எனில்( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  திரும்பு rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
நிலையான முழு tvfsAccess(
  sqlite3_vfs *pVfs, 
  மாறிலி எழுத்து *zPath, 
  முழு flags, 
  முழு *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  எனில்( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    முழு rc;
    எழுத்து *zArg = 0;
    எனில்( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    எனில்( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    எனில்( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    எனில்( tvfsResultCode(p, &rc) ){
      எனில்( rc!=SQLITE_OK ) திரும்பு rc;
    }இல்லையெனில்{
      Tcl_Interp *interp = p->interp;
      எனில்( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_GetObjResult(interp), pResOut) ){
        திரும்பு SQLITE_OK;
      }
    }
  }
  திரும்பு sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
நிலையான முழு tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  மாறிலி எழுத்து *zPath, 
  முழு nOut, 
  எழுத்து *zOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  எனில்( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    முழு rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    எனில்( tvfsResultCode(p, &rc) ){
      எனில்( rc!=SQLITE_OK ) திரும்பு rc;
    }
  }
  திரும்பு sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
நிலையான வெறுமை *tvfsDlOpen(sqlite3_vfs *pVfs, மாறிலி எழுத்து *zPath){
  திரும்பு sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
நிலையான வெறுமை tvfsDlError(sqlite3_vfs *pVfs, முழு nByte, எழுத்து *z_Err_செய்தி){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_செய்தி);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
நிலையான வெறுமை (*tvfsDlSym(sqlite3_vfs *pVfs, வெறுமை *p, மாறிலி எழுத்து *zSym))(வெறுமை){
  திரும்பு sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
நிலையான வெறுமை tvfsDlClose(sqlite3_vfs *pVfs, வெறுமை *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
நிலையான முழு tvfsRandomness(sqlite3_vfs *pVfs, முழு nByte, எழுத்து *zBufOut){
  திரும்பு sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
நிலையான முழு tvfsSleep(sqlite3_vfs *pVfs, முழு nMicro){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  எனில்( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  திரும்பு sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
நிலையான முழு tvfsCurrentTime(sqlite3_vfs *pVfs, இரட்டை *pTimeOut){
  திரும்பு PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

நிலையான முழு tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  முழு rc = SQLITE_OK;             /* Return code */
  TestvfsBuffer *pBuffer;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->pAppData;
  ப்ரதிஜ்ஞா( 0==p->isFullshm );
  ப்ரதிஜ்ஞா( pFd->pShmId && pFd->pShm==0 && pFd->pNext==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_ResetResult(p->interp);
  எனில்( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    எனில்( tvfsResultCode(p, &rc) ){
      எனில்( rc!=SQLITE_OK ) திரும்பு rc;
    }
  }

  ப்ரதிஜ்ஞா( rc==SQLITE_OK );
  எனில்( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    திரும்பு SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  க்கு(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
    எனில்( 0==strcmp(pFd->zFilename, pBuffer->zFile) ) நிறுத்து;
  }
  எனில்( !pBuffer ){
    முழு szName = (முழு)strlen(pFd->zFilename);
    முழு nByte = அளவு(TestvfsBuffer) + szName + 1;
    pBuffer = (TestvfsBuffer *)ckalloc(nByte);
    memset(pBuffer, 0, nByte);
    pBuffer->zFile = (எழுத்து *)&pBuffer[1];
    memcpy(pBuffer->zFile, pFd->zFilename, szName+1);
    pBuffer->pNext = p->pBuffer;
    p->pBuffer = pBuffer;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->pNext = pBuffer->pFile;
  pBuffer->pFile = pFd;
  pFd->pShm = pBuffer;
  திரும்பு rc;
}

நிலையான வெறுமை tvfsAllocPage(TestvfsBuffer *p, முழு iPage, முழு pgsz){
  ப்ரதிஜ்ஞா( iPage<TESTVFS_MAX_PAGES );
  எனில்( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    memset(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

நிலையான முழு tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  முழு iPage,                      /* Page to retrieve */
  முழு pgsz,                       /* Size of pages */
  முழு isWrite,                    /* True to extend file if necessary */
  வெறுமை நிலையற்ற **pp              /* OUT: Mapped memory */
){
  முழு rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  எனில்( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    திரும்பு pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  எனில்( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    எனில்( rc!=SQLITE_OK ){
      திரும்பு rc;
    }
  }

  எனில்( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_மொத்தம்(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(iPage));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfsResultCode(p, &rc);
    Tcl_Decr_Ref_மொத்தம்(pArg);
  }
  எனில்( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  எனில்( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  எனில்( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (வெறுமை நிலையற்ற *)pFd->pShm->aPage[iPage];
  }

  திரும்பு rc;
}


நிலையான முழு tvfsShmLock(
  sqlite3_file *pFile,
  முழு ofst,
  முழு n,
  முழு flags
){
  முழு rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  முழு nLock;
  எழுத்து zLock[80];

  எனில்( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    திரும்பு pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  எனில்( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(அளவு(zLock), zLock, "%d %d", ofst, n);
    nLock = (முழு)strlen(zLock);
    எனில்( flags & SQLITE_SHM_LOCK ){
      strcpy(&zLock[nLock], " lock");
    }இல்லையெனில்{
      strcpy(&zLock[nLock], " unlock");
    }
    nLock += (முழு)strlen(&zLock[nLock]);
    எனில்( flags & SQLITE_SHM_SHARED ){
      strcpy(&zLock[nLock], " shared");
    }இல்லையெனில்{
      strcpy(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  எனில்( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  எனில்( rc==SQLITE_OK ){
    முழு isLock = (flags & SQLITE_SHM_LOCK);
    முழு isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    எனில்( isLock ){
      TestvfsFd *p2;
      க்கு(p2=pFd->pShm->pFile; p2; p2=p2->pNext){
        எனில்( p2==pFd ) தொடர்;
        எனில்( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          நிறுத்து;
        }
      }
      எனில்( rc==SQLITE_OK ){
        எனில்( isExcl )  pFd->excllock |= mask;
        எனில்( !isExcl ) pFd->sharedlock |= mask;
      }
    }இல்லையெனில்{
      எனில்( isExcl )  pFd->excllock &= (~mask);
      எனில்( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  திரும்பு rc;
}

நிலையான வெறுமை tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  எனில்( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    மாறிலி எழுத்து *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  எனில்( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    திரும்பு;
  }
}

நிலையான முழு tvfsShmUnmap(
  sqlite3_file *pFile,
  முழு deleteFlag
){
  முழு rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  TestvfsBuffer *pBuffer = pFd->pShm;
  TestvfsFd **ppFd;

  எனில்( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    திரும்பு pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  எனில்( !pBuffer ) திரும்பு SQLITE_OK;
  ப்ரதிஜ்ஞா( pFd->pShmId && pFd->pShm );

  எனில்( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }

  க்கு(ppFd=&pBuffer->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->pNext));
  ப்ரதிஜ்ஞா( (*ppFd)==pFd );
  *ppFd = pFd->pNext;
  pFd->pNext = 0;

  எனில்( pBuffer->pFile==0 ){
    முழு i;
    TestvfsBuffer **pp;
    க்கு(pp=&p->pBuffer; *pp!=pBuffer; pp=&((*pp)->pNext));
    *pp = (*pp)->pNext;
    க்கு(i=0; pBuffer->aPage[i]; i++){
      ckfree((எழுத்து *)pBuffer->aPage[i]);
    }
    ckfree((எழுத்து *)pBuffer);
  }
  pFd->pShm = 0;

  திரும்பு rc;
}

நிலையான முழு tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    முழு iAmt, 
    வெறுமை **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  திரும்பு sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

நிலையான முழு tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, வெறுமை *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  திரும்பு sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

நிலையான முழு SQLITE_TCLAPI testvfs_obj_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  எண்ணிக்கை DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  கட்டமைப்பு TestvfsSubcmd {
    எழுத்து *zName;
    எண்ணிக்கை DB_enum eCmd;
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
  முழு i;
  
  எனில்( objc<2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "SUBCOMMAND ...");
    திரும்பு TCL_ERROR;
  }
  எனில்( Tcl_GetIndexFromObjStruct(
        interp, objv[1], aSubcmd, அளவு(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    திரும்பு TCL_ERROR;
  }
  Tcl_ResetResult(interp);

  தேர்வு( aSubcmd[i].eCmd ){
    நிலை CMD_SHM: {
      Tcl_Obj *pObj;
      முழு rc;
      TestvfsBuffer *pBuffer;
      எழுத்து *zName;
      எனில்( objc!=3 && objc!=4 ){
        Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "FILE ?VALUE?");
        திரும்பு TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      எனில்( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), வெற்று);
        ckfree(zName);
        திரும்பு TCL_ERROR;
      }
      க்கு(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
        எனில்( 0==strcmp(pBuffer->zFile, zName) ) நிறுத்து;
      }
      ckfree(zName);
      எனில்( !pBuffer ){
        Tcl_AppendResult(interp, "no such file: ", Tcl_GetString(objv[2]), வெற்று);
        திரும்பு TCL_ERROR;
      }
      எனில்( objc==4 ){
        Tcl_Size n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        முழு pgsz = pBuffer->pgsz;
        எனில்( pgsz==0 ) pgsz = 65536;
        க்கு(i=0; i*pgsz<(முழு)n; i++){
          முழு nByte = pgsz;
          tvfsAllocPage(pBuffer, i, pgsz);
          எனில்( n-i*pgsz<pgsz ){
            nByte = (முழு)n;
          }
          memcpy(pBuffer->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      க்கு(i=0; pBuffer->aPage[i]; i++){
        முழு pgsz = pBuffer->pgsz;
        எனில்( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(pBuffer->aPage[i], pgsz));
      }
      Tcl_SetObjResult(interp, pObj);
      நிறுத்து;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    நிலை CMD_FILTER: {
      நிலையான கட்டமைப்பு VfsMethod {
        எழுத்து *zName;
        முழு mask;
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
      முழு mask = 0;
      எனில்( objc!=3 ){
        Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "LIST");
        திரும்பு TCL_ERROR;
      }
      எனில்( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        திரும்பு TCL_ERROR;
      }
      Tcl_ResetResult(interp);
      க்கு(i=0; i<(முழு)nElem; i++){
        முழு iMethod;
        எழுத்து *zElem = Tcl_GetString(apElem[i]);
        க்கு(iMethod=0; iMethod<ArraySize(vfsmethod); iMethod++){
          எனில்( strcmp(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            நிறுத்து;
          }
        }
        எனில்( iMethod==ArraySize(vfsmethod) ){
          Tcl_AppendResult(interp, "unknown method: ", zElem, வெற்று);
          திரும்பு TCL_ERROR;
        }
      }
      p->mask = mask;
      நிறுத்து;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    நிலை CMD_SCRIPT: {
      எனில்( objc==3 ){
        Tcl_Size nByte;
        எனில்( p->pScript ){
          Tcl_Decr_Ref_மொத்தம்(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        எனில்( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_மொத்தம்(p->pScript);
        }
      }இல்லையெனில் எனில்( objc!=2 ){
        Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?SCRIPT?");
        திரும்பு TCL_ERROR;
      }

      Tcl_ResetResult(interp);
      எனில்( p->pScript ) Tcl_SetObjResult(interp, p->pScript);

      நிறுத்து;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    நிலை CMD_CANTOPENERR:
    நிலை CMD_IOERR:
    நிலை CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      முழு iRet;

      தேர்வு( aSubcmd[i].eCmd ){
        நிலை CMD_IOERR: pTest = &p->ioerr_err; நிறுத்து;
        நிலை CMD_FULLERR: pTest = &p->full_err; நிறுத்து;
        நிலை CMD_CANTOPENERR: pTest = &p->cantopen_err; நிறுத்து;
        இயல்பு: ப்ரதிஜ்ஞா(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      எனில்( objc==4 ){
        முழு iCnt, iPersist;
        எனில்( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          திரும்பு TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }இல்லையெனில் எனில்( objc!=2 ){
        Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?CNT PERSIST?");
        திரும்பு TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(iRet));
      நிறுத்து;
    }

    நிலை CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      நிறுத்து;
    }

    நிலை CMD_DEVCHAR: {
      கட்டமைப்பு DeviceFlag {
        எழுத்து *zName;
        முழு i_மதிப்பு;
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
      முழு iFlag;

      எனில்( objc>3 ){
        Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?ATTR-LIST?");
        திரும்பு TCL_ERROR;
      }
      எனில்( objc==3 ){
        முழு j;
        முழு iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_Size nFlags = 0;

        எனில்( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          திரும்பு TCL_ERROR;
        }

        க்கு(j=0; j<(முழு)nFlags; j++){
          முழு idx = 0;
          எனில்( Tcl_GetIndexFromObjStruct(interp, flags[j], aFlag, 
                அளவு(aFlag[0]), "flag", 0, &idx) 
          ){
            திரும்பு TCL_ERROR;
          }
          எனில்( aFlag[idx].i_மதிப்பு<0 && nFlags>1 ){
            Tcl_AppendResult(interp, "bad flags: ", Tcl_GetString(objv[2]), வெற்று);
            திரும்பு TCL_ERROR;
          }
          iNew |= aFlag[idx].i_மதிப்பு;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      க்கு(iFlag=0; iFlag<அளவு(aFlag)/அளவு(aFlag[0]); iFlag++){
        எனில்( p->iDevchar & aFlag[iFlag].i_மதிப்பு ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_SetObjResult(interp, pRet);

      நிறுத்து;
    }

    நிலை CMD_SECTORSIZE: {
      எனில்( objc>3 ){
        Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, "?VALUE?");
        திரும்பு TCL_ERROR;
      }
      எனில்( objc==3 ){
        முழு iNew = 0;
        எனில்( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          திரும்பு TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(p->iSectorsize));
      நிறுத்து;
    }
  }

  திரும்பு TCL_OK;
}

நிலையான வெறுமை SQLITE_TCLAPI testvfs_obj_del(ClientData cd){
  Testvfs *p = (Testvfs *)cd;
  எனில்( p->pScript ) Tcl_Decr_Ref_மொத்தம்(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, அளவு(sqlite3_vfs));
  ckfree((எழுத்து *)p->pVfs);
  memset(p, 0, அளவு(Testvfs));
  ckfree((எழுத்து *)p);
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
நிலையான முழு SQLITE_TCLAPI testvfs_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  நிலையான sqlite3_vfs tvfs_vfs = {
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
#இல்லையெனில்
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
  எழுத்து *zVfs;
  முழு nByte;                      /* Bytes of space to allocate at p */

  முழு i;
  முழு isNoshm = 0;                /* True if -noshm is passed */
  முழு isFullshm = 0;              /* True if -fullshm is passed */
  முழு isDefault = 0;              /* True if -default is passed */
  முழு szOsFile = 0;               /* Value passed to -szosfile */
  முழு mxPathname = -1;            /* Value passed to -mxpathname */
  முழு iVersion = 3;               /* Value passed to -iversion */

  எனில்( objc<2 || 0!=(objc%2) ) செல் bad_args;
  க்கு(i=2; i<objc; i += 2){
    Tcl_Size nSwitch;
    எழுத்து *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    எனில்( nSwitch>2 && 0==strncmp("-noshm", zSwitch, nSwitch) ){
      எனில்( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        திரும்பு TCL_ERROR;
      }
      எனில்( isNoshm ) isFullshm = 0;
    }
    இல்லையெனில் எனில்( nSwitch>2 && 0==strncmp("-default", zSwitch, nSwitch) ){
      எனில்( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        திரும்பு TCL_ERROR;
      }
    }
    இல்லையெனில் எனில்( nSwitch>2 && 0==strncmp("-szosfile", zSwitch, nSwitch) ){
      எனில்( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        திரும்பு TCL_ERROR;
      }
    }
    இல்லையெனில் எனில்( nSwitch>2 && 0==strncmp("-mxpathname", zSwitch, nSwitch) ){
      எனில்( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        திரும்பு TCL_ERROR;
      }
    }
    இல்லையெனில் எனில்( nSwitch>2 && 0==strncmp("-iversion", zSwitch, nSwitch) ){
      எனில்( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        திரும்பு TCL_ERROR;
      }
    }
    இல்லையெனில் எனில்( nSwitch>2 && 0==strncmp("-fullshm", zSwitch, nSwitch) ){
      எனில்( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        திரும்பு TCL_ERROR;
      }
      எனில்( isFullshm ) isNoshm = 0;
    }
    இல்லையெனில்{
      செல் bad_args;
    }
  }

  எனில்( szOsFile<அளவு(TestvfsFile) ){
    szOsFile = அளவு(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = அளவு(Testvfs) + (முழு)strlen(zVfs)+1;
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

  p->zName = (எழுத்து *)&p[1];
  memcpy(p->zName, zVfs, strlen(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(அளவு(sqlite3_vfs));
  memcpy(pVfs, &tvfs_vfs, அளவு(sqlite3_vfs));
  pVfs->pAppData = (வெறுமை *)p;
  pVfs->iVersion = iVersion;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  எனில்( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  திரும்பு TCL_OK;

 bad_args:
  Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  திரும்பு TCL_ERROR;
}

வெளி முழு getDbPointer(Tcl_Interp *interp, மாறிலி எழுத்து *zA, sqlite3 **ppDb);
வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
நிலையான முழு SQLITE_TCLAPI test_vfs_shmlock(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  மாறிலி எழுத்து *azArg1[] = {"shared", "exclusive", 0};
  மாறிலி எழுத்து *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  முழு rc = SQLITE_OK;
  மாறிலி எழுத்து *zDbname = 0;
  முழு iArg1 = 0;
  முழு iArg2 = 0;
  முழு iOffset = 0;
  முழு n = 0;
  sqlite3_file *pFd;

  எனில்( objc!=7 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    திரும்பு TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIndexFromObj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_GetIndexFromObj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    திரும்பு TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (வெறுமை*)&pFd);
  எனில்( pFd==0 ){
    திரும்பு TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_vfs_set_readmark(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  முழு rc = SQLITE_OK;
  மாறிலி எழுத்து *zDbname = 0;
  முழு iSlot = 0;
  முழு iVal = -1;
  sqlite3_file *pFd;
  வெறுமை நிலையற்ற *pShm = 0;
  u32 *aShm;
  முழு iOff;

  எனில்( objc!=4 && objc!=5 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    திரும்பு TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    திரும்பு TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (வெறுமை*)&pFd);
  எனில்( pFd==0 ){
    திரும்பு TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  எனில்( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    திரும்பு TCL_ERROR;
  }
  எனில்( pShm==0 ){
    Tcl_AppendResult(interp, "*-shm is not yet mapped", வெற்று);
    திரும்பு TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  எனில்( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(aShm[iOff]));

  திரும்பு TCL_OK;
}

முழு Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  திரும்பு TCL_OK;
}

#endif
