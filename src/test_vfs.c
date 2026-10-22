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
#ಯದಿ SQLITE_TEST          /* This file is used for testing only */

#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ Testvfs Testvfs;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ TestvfsShm TestvfsShm;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ TestvfsBuffer TestvfsBuffer;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ TestvfsFile TestvfsFile;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
ಸಂರಚನಾ TestvfsFile {
  sqlite3_file base;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#define tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

ಸಂರಚನಾ TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  ಧ್ರುವ ವರ್ಣ *zFilename;          /* Filename as passed to xOpen() */
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

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ TestFaultInject TestFaultInject;
ಸಂರಚನಾ TestFaultInject {
  ಪೂರ್ಣಾಙ್ಕ iCnt;                       /* Remaining calls before fault injection */
  ಪೂರ್ಣಾಙ್ಕ eFault;                     /* A FAULT_INJECT_* value */
  ಪೂರ್ಣಾಙ್ಕ nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
ಸಂರಚನಾ Testvfs {
  ವರ್ಣ *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  TestvfsBuffer *pBuffer;         /* List of shared buffers */
  ಪೂರ್ಣಾಙ್ಕ isNoshm;
  ಪೂರ್ಣಾಙ್ಕ isFullshm;

  ಪೂರ್ಣಾಙ್ಕ mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#ಯದಿ 0
  ಪೂರ್ಣಾಙ್ಕ iIoerrCnt;
  ಪೂರ್ಣಾಙ್ಕ ioerr;
  ಪೂರ್ಣಾಙ್ಕ nIoerrFail;
  ಪೂರ್ಣಾಙ್ಕ iFullCnt;
  ಪೂರ್ಣಾಙ್ಕ fullerr;
  ಪೂರ್ಣಾಙ್ಕ nFullFail;
#endif

  ಪೂರ್ಣಾಙ್ಕ iDevchar;
  ಪೂರ್ಣಾಙ್ಕ iSectorsize;
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
ಸಂರಚನಾ TestvfsBuffer {
  ವರ್ಣ *zFile;                    /* Associated file name */
  ಪೂರ್ಣಾಙ್ಕ pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  TestvfsBuffer *pNext;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->pAppData))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsRead(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsWrite(sqlite3_file*,ಧ್ರುವ ಶೂನ್ಯ*,ಪೂರ್ಣಾಙ್ಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsTruncate(sqlite3_file*, sqlite3_int64 size);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsSync(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsUnlock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsCheckReservedLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsFileControl(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsSectorSize(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsOpen(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *, sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ , ಪೂರ್ಣಾಙ್ಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsDelete(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ syncDir);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsAccess(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ flags, ಪೂರ್ಣಾಙ್ಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsFullPathname(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zName, ಪೂರ್ಣಾಙ್ಕ, ವರ್ಣ *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
ಸ್ಥಿರ ಶೂನ್ಯ *tvfsDlOpen(sqlite3_vfs*, ಧ್ರುವ ವರ್ಣ *zFilename);
ಸ್ಥಿರ ಶೂನ್ಯ tvfsDlError(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *z_Err_ಸಂದೇಶ);
ಸ್ಥಿರ ಶೂನ್ಯ (*tvfsDlSym(sqlite3_vfs*,ಶೂನ್ಯ*, ಧ್ರುವ ವರ್ಣ *z_ಚಿಹ್ನೆ))(ಶೂನ್ಯ);
ಸ್ಥಿರ ಶೂನ್ಯ tvfsDlClose(sqlite3_vfs*, ಶೂನ್ಯ*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsRandomness(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zOut);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsSleep(sqlite3_vfs*, ಪೂರ್ಣಾಙ್ಕ microseconds);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsCurrentTime(sqlite3_vfs*, ದ್ವಿಗುಣ*);

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsShmOpen(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsShmLock(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ , ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsShmMap(sqlite3_file*,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ,ಪೂರ್ಣಾಙ್ಕ, ಶೂನ್ಯ ಚಞ್ಚಲ **);
ಸ್ಥಿರ ಶೂನ್ಯ tvfsShmBarrier(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsShmUnmap(sqlite3_file*, ಪೂರ್ಣಾಙ್ಕ);

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsFetch(sqlite3_file*, sqlite3_int64, ಪೂರ್ಣಾಙ್ಕ, ಶೂನ್ಯ**);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsUnfetch(sqlite3_file*, sqlite3_int64, ಶೂನ್ಯ*);

ಸ್ಥಿರ sqlite3_io_methods tvfs_io_methods = {
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

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsResultCode(Testvfs *p, ಪೂರ್ಣಾಙ್ಕ *pRc){
  ಸಂರಚನಾ errcode {
    ಪೂರ್ಣಾಙ್ಕ eCode;
    ಧ್ರುವ ವರ್ಣ *zCode;
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

  ಧ್ರುವ ವರ್ಣ *z;
  ಪೂರ್ಣಾಙ್ಕ i;

  z = Tcl_GetStringResult(p->interp);
  ಪ್ರತಿ(i=0; i<ArraySize(aCode); i++){
    ಯದಿ( 0==strcmp(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }

  ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsInjectFault(TestFaultInject *p){
  ಪೂರ್ಣಾಙ್ಕ ret = 0;
  ಯದಿ( p->eFault ){
    p->iCnt--;
    ಯದಿ( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  ಪ್ರತ್ಯಾಗಮ ret;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsInjectIoerr(Testvfs *p){
  ಪ್ರತ್ಯಾಗಮ tvfsInjectFault(&p->ioerr_err);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsInjectFullerr(Testvfs *p){
  ಪ್ರತ್ಯಾಗಮ tvfsInjectFault(&p->full_err);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsInjectCantopenerr(Testvfs *p){
  ಪ್ರತ್ಯಾಗಮ tvfsInjectFault(&p->cantopen_err);
}


ಸ್ಥಿರ ಶೂನ್ಯ tvfsExecTcl(
  Testvfs *p, 
  ಧ್ರುವ ವರ್ಣ *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  ಪೂರ್ಣಾಙ್ಕ rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  ಪ್ರತಿಜ್ಞಾ( p->pScript );

  ಪ್ರತಿಜ್ಞಾ( zMethod );
  ಪ್ರತಿಜ್ಞಾ( p );
  ಪ್ರತಿಜ್ಞಾ( arg2==0 || arg1!=0 );
  ಪ್ರತಿಜ್ಞಾ( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ಎಣಿಕೆ(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  ಯದಿ( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  ಯದಿ( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  ಯದಿ( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  ಯದಿ( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  ಯದಿ( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_ResetResult(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ಯದಿ( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  ಯದಿ( pFd->pShmId ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(pFd->pShmId);
    pFd->pShmId = 0;
  }
  ಯದಿ( pFile->pMethods ){
    ckfree((ವರ್ಣ *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((ವರ್ಣ *)pFd);
  pTestfile->pFd = 0;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಙ್ಕ iAmt, 
  sqlite_int64 iOfst
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ಯದಿ( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  ಯದಿ( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Write data to an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsWrite(
  sqlite3_file *pFile, 
  ಧ್ರುವ ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಙ್ಕ iAmt, 
  sqlite_int64 iOfst
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ಯದಿ( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfsResultCode(p, &rc);
    ಯದಿ( rc<0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  }

  ಯದಿ( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  ಯದಿ( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Truncate an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ಯದಿ( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  
  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, size);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Sync an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsSync(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ flags){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ಯದಿ( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    ವರ್ಣ *zFlags = 0;

    ವಿಕಲ್ಪ( flags ){
      ಪಕ್ಷ SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_SYNC_FULL:
        zFlags = "full";
        ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        ಭಙ್ಗ;
      ಪಕ್ಷ SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        ಭಙ್ಗ;
      ಸ್ವಭಾವ:
        ಪ್ರತಿಜ್ಞಾ(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  ಯದಿ( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  TestvfsFd *p = tvfsGetFd(pFile);
  ಪ್ರತ್ಯಾಗಮ sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ಯದಿ( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    ವರ್ಣ zLock[30];
    sqlite3_snprintf(ಪರಿಮಾಣ(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  ಯದಿ( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_LOCK;
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ಯದಿ( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    ವರ್ಣ zLock[30];
    sqlite3_snprintf(ಪರಿಮಾಣ(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  ಯದಿ( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR_UNLOCK;
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsCheckReservedLock(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ಯದಿ( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಙ್ಕ op, ಶೂನ್ಯ *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ಯದಿ( op==SQLITE_FCNTL_PRAGMA ){
    ವರ್ಣ **argv = (ವರ್ಣ**)pArg;
    ಯದಿ( sqlite3_stricmp(argv[1],"error")==0 ){
      ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_ERROR;
      ಯದಿ( argv[2] ){
        ಧ್ರುವ ವರ್ಣ *z = argv[2];
        ಪೂರ್ಣಾಙ್ಕ x = atoi(z);
        ಯದಿ( x ){
          rc = x;
          ಯಾವತ್( sqlite3Isdigit(z[0]) ){ z++; }
          ಯಾವತ್( sqlite3Isspace(z[0]) ){ z++; }
        }
        ಯದಿ( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      ಪ್ರತ್ಯಾಗಮ rc;
    }
    ಯದಿ( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
    }
  }
  ಯದಿ( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    ಸಂರಚನಾ Fcntl {
      ಪೂರ್ಣಾಙ್ಕ iFnctl;
      ಧ್ರುವ ವರ್ಣ *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aF)/ಪರಿಮಾಣ(aF[0]); i++){
      ಯದಿ( op==aF[i].iFnctl ) ಭಙ್ಗ;
    }
    ಯದಿ( i<ಪರಿಮಾಣ(aF)/ಪರಿಮಾಣ(aF[0]) ){
      ಪೂರ್ಣಾಙ್ಕ rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfsResultCode(p, &rc);
      ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ (rc<0 ? SQLITE_OK : rc);
    }
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsSectorSize(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ಯದಿ( p->iSectorsize>=0 ){
    ಪ್ರತ್ಯಾಗಮ p->iSectorsize;
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3OsSectorSize(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ಯದಿ( p->iDevchar>=0 ){
    ಪ್ರತ್ಯಾಗಮ p->iDevchar;
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsOpen(
  sqlite3_vfs *pVfs,
  ಧ್ರುವ ವರ್ಣ *zName,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಙ್ಕ flags,
  ಪೂರ್ಣಾಙ್ಕ *pOutFlags
){
  ಪೂರ್ಣಾಙ್ಕ rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  pFd = (TestvfsFd *)ckalloc(ಪರಿಮಾಣ(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, ಪರಿಮಾಣ(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, ಪರಿಮಾಣ(TestvfsFile));
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
  ಯದಿ( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_ಎಣಿಕೆ(pArg);
    ಯದಿ( flags&SQLITE_OPEN_MAIN_DB ){
      ಧ್ರುವ ವರ್ಣ *z = &zName[strlen(zName)+1];
      ಯಾವತ್( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_ಎಣಿಕೆ(pArg);
    ಯದಿ( tvfsResultCode(p, &rc) ){
      ಯದಿ( rc!=SQLITE_OK ) ಪ್ರತ್ಯಾಗಮ rc;
    }ಅನ್ಯಥಾ{
      pId = Tcl_GetObjResult(p->interp);
    }
  }

  ಯದಿ( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR;
  ಯದಿ( tvfsInjectCantopenerr(p) ) ಪ್ರತ್ಯಾಗಮ SQLITE_CANTOPEN;
  ಯದಿ( tvfsInjectFullerr(p) ) ಪ್ರತ್ಯಾಗಮ SQLITE_FULL;

  ಯದಿ( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_ಎಣಿಕೆ(pId);
  pFd->pShmId = pId;
  Tcl_ResetResult(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  ಯದಿ( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    ಪೂರ್ಣಾಙ್ಕ nByte;

    ಯದಿ( pVfs->iVersion>1 ){
      nByte = ಪರಿಮಾಣ(sqlite3_io_methods);
    }ಅನ್ಯಥಾ{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    memcpy(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    ಯದಿ( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    ಯದಿ( pVfs->iVersion>1 && ((Testvfs *)pVfs->pAppData)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsDelete(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath, ಪೂರ್ಣಾಙ್ಕ dirSync){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  ಯದಿ( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  ಯದಿ( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  ಪ್ರತ್ಯಾಗಮ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsAccess(
  sqlite3_vfs *pVfs, 
  ಧ್ರುವ ವರ್ಣ *zPath, 
  ಪೂರ್ಣಾಙ್ಕ flags, 
  ಪೂರ್ಣಾಙ್ಕ *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  ಯದಿ( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    ಪೂರ್ಣಾಙ್ಕ rc;
    ವರ್ಣ *zArg = 0;
    ಯದಿ( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    ಯದಿ( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    ಯದಿ( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    ಯದಿ( tvfsResultCode(p, &rc) ){
      ಯದಿ( rc!=SQLITE_OK ) ಪ್ರತ್ಯಾಗಮ rc;
    }ಅನ್ಯಥಾ{
      Tcl_Interp *interp = p->interp;
      ಯದಿ( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_GetObjResult(interp), pResOut) ){
        ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  ಧ್ರುವ ವರ್ಣ *zPath, 
  ಪೂರ್ಣಾಙ್ಕ nOut, 
  ವರ್ಣ *zOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  ಯದಿ( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    ಪೂರ್ಣಾಙ್ಕ rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    ಯದಿ( tvfsResultCode(p, &rc) ){
      ಯದಿ( rc!=SQLITE_OK ) ಪ್ರತ್ಯಾಗಮ rc;
    }
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *tvfsDlOpen(sqlite3_vfs *pVfs, ಧ್ರುವ ವರ್ಣ *zPath){
  ಪ್ರತ್ಯಾಗಮ sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ tvfsDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *z_Err_ಸಂದೇಶ){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_ಸಂದೇಶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ (*tvfsDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *p, ಧ್ರುವ ವರ್ಣ *zSym))(ಶೂನ್ಯ){
  ಪ್ರತ್ಯಾಗಮ sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ tvfsDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nByte, ವರ್ಣ *zBufOut){
  ಪ್ರತ್ಯಾಗಮ sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಙ್ಕ nMicro){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  ಯದಿ( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsCurrentTime(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *pTimeOut){
  ಪ್ರತ್ಯಾಗಮ PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;             /* Return code */
  TestvfsBuffer *pBuffer;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->pAppData;
  ಪ್ರತಿಜ್ಞಾ( 0==p->isFullshm );
  ಪ್ರತಿಜ್ಞಾ( pFd->pShmId && pFd->pShm==0 && pFd->pNext==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_ResetResult(p->interp);
  ಯದಿ( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    ಯದಿ( tvfsResultCode(p, &rc) ){
      ಯದಿ( rc!=SQLITE_OK ) ಪ್ರತ್ಯಾಗಮ rc;
    }
  }

  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
  ಯದಿ( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  ಪ್ರತಿ(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
    ಯದಿ( 0==strcmp(pFd->zFilename, pBuffer->zFile) ) ಭಙ್ಗ;
  }
  ಯದಿ( !pBuffer ){
    ಪೂರ್ಣಾಙ್ಕ szName = (ಪೂರ್ಣಾಙ್ಕ)strlen(pFd->zFilename);
    ಪೂರ್ಣಾಙ್ಕ nByte = ಪರಿಮಾಣ(TestvfsBuffer) + szName + 1;
    pBuffer = (TestvfsBuffer *)ckalloc(nByte);
    memset(pBuffer, 0, nByte);
    pBuffer->zFile = (ವರ್ಣ *)&pBuffer[1];
    memcpy(pBuffer->zFile, pFd->zFilename, szName+1);
    pBuffer->pNext = p->pBuffer;
    p->pBuffer = pBuffer;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->pNext = pBuffer->pFile;
  pBuffer->pFile = pFd;
  pFd->pShm = pBuffer;
  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಶೂನ್ಯ tvfsAllocPage(TestvfsBuffer *p, ಪೂರ್ಣಾಙ್ಕ iPage, ಪೂರ್ಣಾಙ್ಕ pgsz){
  ಪ್ರತಿಜ್ಞಾ( iPage<TESTVFS_MAX_PAGES );
  ಯದಿ( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    memset(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  ಪೂರ್ಣಾಙ್ಕ iPage,                      /* Page to retrieve */
  ಪೂರ್ಣಾಙ್ಕ pgsz,                       /* Size of pages */
  ಪೂರ್ಣಾಙ್ಕ isWrite,                    /* True to extend file if necessary */
  ಶೂನ್ಯ ಚಞ್ಚಲ **pp              /* OUT: Mapped memory */
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  ಯದಿ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ಪ್ರತ್ಯಾಗಮ pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  ಯದಿ( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    ಯದಿ( rc!=SQLITE_OK ){
      ಪ್ರತ್ಯಾಗಮ rc;
    }
  }

  ಯದಿ( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_ಎಣಿಕೆ(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(iPage));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfsResultCode(p, &rc);
    Tcl_Decr_Ref_ಎಣಿಕೆ(pArg);
  }
  ಯದಿ( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ಯದಿ( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  ಯದಿ( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (ಶೂನ್ಯ ಚಞ್ಚಲ *)pFd->pShm->aPage[iPage];
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsShmLock(
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಙ್ಕ ofst,
  ಪೂರ್ಣಾಙ್ಕ n,
  ಪೂರ್ಣಾಙ್ಕ flags
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  ಪೂರ್ಣಾಙ್ಕ nLock;
  ವರ್ಣ zLock[80];

  ಯದಿ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ಪ್ರತ್ಯಾಗಮ pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  ಯದಿ( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(ಪರಿಮಾಣ(zLock), zLock, "%d %d", ofst, n);
    nLock = (ಪೂರ್ಣಾಙ್ಕ)strlen(zLock);
    ಯದಿ( flags & SQLITE_SHM_LOCK ){
      strcpy(&zLock[nLock], " lock");
    }ಅನ್ಯಥಾ{
      strcpy(&zLock[nLock], " unlock");
    }
    nLock += (ಪೂರ್ಣಾಙ್ಕ)strlen(&zLock[nLock]);
    ಯದಿ( flags & SQLITE_SHM_SHARED ){
      strcpy(&zLock[nLock], " shared");
    }ಅನ್ಯಥಾ{
      strcpy(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  ಯದಿ( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ಯದಿ( rc==SQLITE_OK ){
    ಪೂರ್ಣಾಙ್ಕ isLock = (flags & SQLITE_SHM_LOCK);
    ಪೂರ್ಣಾಙ್ಕ isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    ಯದಿ( isLock ){
      TestvfsFd *p2;
      ಪ್ರತಿ(p2=pFd->pShm->pFile; p2; p2=p2->pNext){
        ಯದಿ( p2==pFd ) ಅನುವರ್ತ;
        ಯದಿ( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          ಭಙ್ಗ;
        }
      }
      ಯದಿ( rc==SQLITE_OK ){
        ಯದಿ( isExcl )  pFd->excllock |= mask;
        ಯದಿ( !isExcl ) pFd->sharedlock |= mask;
      }
    }ಅನ್ಯಥಾ{
      ಯದಿ( isExcl )  pFd->excllock &= (~mask);
      ಯದಿ( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಶೂನ್ಯ tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  ಯದಿ( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    ಧ್ರುವ ವರ್ಣ *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  ಯದಿ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    ಪ್ರತ್ಯಾಗಮ;
  }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsShmUnmap(
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಙ್ಕ deleteFlag
){
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  TestvfsBuffer *pBuffer = pFd->pShm;
  TestvfsFd **ppFd;

  ಯದಿ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ಪ್ರತ್ಯಾಗಮ pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  ಯದಿ( !pBuffer ) ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  ಪ್ರತಿಜ್ಞಾ( pFd->pShmId && pFd->pShm );

  ಯದಿ( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }

  ಪ್ರತಿ(ppFd=&pBuffer->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->pNext));
  ಪ್ರತಿಜ್ಞಾ( (*ppFd)==pFd );
  *ppFd = pFd->pNext;
  pFd->pNext = 0;

  ಯದಿ( pBuffer->pFile==0 ){
    ಪೂರ್ಣಾಙ್ಕ i;
    TestvfsBuffer **pp;
    ಪ್ರತಿ(pp=&p->pBuffer; *pp!=pBuffer; pp=&((*pp)->pNext));
    *pp = (*pp)->pNext;
    ಪ್ರತಿ(i=0; pBuffer->aPage[i]; i++){
      ckfree((ವರ್ಣ *)pBuffer->aPage[i]);
    }
    ckfree((ವರ್ಣ *)pBuffer);
  }
  pFd->pShm = 0;

  ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    ಪೂರ್ಣಾಙ್ಕ iAmt, 
    ಶೂನ್ಯ **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  ಪ್ರತ್ಯಾಗಮ sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, ಶೂನ್ಯ *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  ಪ್ರತ್ಯಾಗಮ sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI testvfs_obj_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  ಗಣನಾ DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  ಸಂರಚನಾ TestvfsSubcmd {
    ವರ್ಣ *zName;
    ಗಣನಾ DB_enum eCmd;
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
  ಪೂರ್ಣಾಙ್ಕ i;
  
  ಯದಿ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "SUBCOMMAND ...");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( Tcl_GetIndexFromObjStruct(
        interp, objv[1], aSubcmd, ಪರಿಮಾಣ(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  Tcl_ResetResult(interp);

  ವಿಕಲ್ಪ( aSubcmd[i].eCmd ){
    ಪಕ್ಷ CMD_SHM: {
      Tcl_Obj *pObj;
      ಪೂರ್ಣಾಙ್ಕ rc;
      TestvfsBuffer *pBuffer;
      ವರ್ಣ *zName;
      ಯದಿ( objc!=3 && objc!=4 ){
        Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "FILE ?VALUE?");
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      ಯದಿ( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), ರಿಕ್ತ);
        ckfree(zName);
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಪ್ರತಿ(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
        ಯದಿ( 0==strcmp(pBuffer->zFile, zName) ) ಭಙ್ಗ;
      }
      ckfree(zName);
      ಯದಿ( !pBuffer ){
        Tcl_AppendResult(interp, "no such file: ", Tcl_GetString(objv[2]), ರಿಕ್ತ);
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಯದಿ( objc==4 ){
        Tcl_Size n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        ಪೂರ್ಣಾಙ್ಕ pgsz = pBuffer->pgsz;
        ಯದಿ( pgsz==0 ) pgsz = 65536;
        ಪ್ರತಿ(i=0; i*pgsz<(ಪೂರ್ಣಾಙ್ಕ)n; i++){
          ಪೂರ್ಣಾಙ್ಕ nByte = pgsz;
          tvfsAllocPage(pBuffer, i, pgsz);
          ಯದಿ( n-i*pgsz<pgsz ){
            nByte = (ಪೂರ್ಣಾಙ್ಕ)n;
          }
          memcpy(pBuffer->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      ಪ್ರತಿ(i=0; pBuffer->aPage[i]; i++){
        ಪೂರ್ಣಾಙ್ಕ pgsz = pBuffer->pgsz;
        ಯದಿ( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(pBuffer->aPage[i], pgsz));
      }
      Tcl_SetObjResult(interp, pObj);
      ಭಙ್ಗ;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    ಪಕ್ಷ CMD_FILTER: {
      ಸ್ಥಿರ ಸಂರಚನಾ VfsMethod {
        ವರ್ಣ *zName;
        ಪೂರ್ಣಾಙ್ಕ mask;
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
      ಪೂರ್ಣಾಙ್ಕ mask = 0;
      ಯದಿ( objc!=3 ){
        Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "LIST");
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಯದಿ( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      Tcl_ResetResult(interp);
      ಪ್ರತಿ(i=0; i<(ಪೂರ್ಣಾಙ್ಕ)nElem; i++){
        ಪೂರ್ಣಾಙ್ಕ iMethod;
        ವರ್ಣ *zElem = Tcl_GetString(apElem[i]);
        ಪ್ರತಿ(iMethod=0; iMethod<ArraySize(vfsmethod); iMethod++){
          ಯದಿ( strcmp(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            ಭಙ್ಗ;
          }
        }
        ಯದಿ( iMethod==ArraySize(vfsmethod) ){
          Tcl_AppendResult(interp, "unknown method: ", zElem, ರಿಕ್ತ);
          ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
        }
      }
      p->mask = mask;
      ಭಙ್ಗ;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    ಪಕ್ಷ CMD_SCRIPT: {
      ಯದಿ( objc==3 ){
        Tcl_Size nByte;
        ಯದಿ( p->pScript ){
          Tcl_Decr_Ref_ಎಣಿಕೆ(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        ಯದಿ( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_ಎಣಿಕೆ(p->pScript);
        }
      }ಅನ್ಯಥಾ ಯದಿ( objc!=2 ){
        Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "?SCRIPT?");
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }

      Tcl_ResetResult(interp);
      ಯದಿ( p->pScript ) Tcl_SetObjResult(interp, p->pScript);

      ಭಙ್ಗ;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    ಪಕ್ಷ CMD_CANTOPENERR:
    ಪಕ್ಷ CMD_IOERR:
    ಪಕ್ಷ CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      ಪೂರ್ಣಾಙ್ಕ iRet;

      ವಿಕಲ್ಪ( aSubcmd[i].eCmd ){
        ಪಕ್ಷ CMD_IOERR: pTest = &p->ioerr_err; ಭಙ್ಗ;
        ಪಕ್ಷ CMD_FULLERR: pTest = &p->full_err; ಭಙ್ಗ;
        ಪಕ್ಷ CMD_CANTOPENERR: pTest = &p->cantopen_err; ಭಙ್ಗ;
        ಸ್ವಭಾವ: ಪ್ರತಿಜ್ಞಾ(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      ಯದಿ( objc==4 ){
        ಪೂರ್ಣಾಙ್ಕ iCnt, iPersist;
        ಯದಿ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }ಅನ್ಯಥಾ ಯದಿ( objc!=2 ){
        Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "?CNT PERSIST?");
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(iRet));
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ CMD_DEVCHAR: {
      ಸಂರಚನಾ DeviceFlag {
        ವರ್ಣ *zName;
        ಪೂರ್ಣಾಙ್ಕ i_ಮೌಲ್ಯ;
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
      ಪೂರ್ಣಾಙ್ಕ iFlag;

      ಯದಿ( objc>3 ){
        Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "?ATTR-LIST?");
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಯದಿ( objc==3 ){
        ಪೂರ್ಣಾಙ್ಕ j;
        ಪೂರ್ಣಾಙ್ಕ iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_Size nFlags = 0;

        ಯದಿ( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
        }

        ಪ್ರತಿ(j=0; j<(ಪೂರ್ಣಾಙ್ಕ)nFlags; j++){
          ಪೂರ್ಣಾಙ್ಕ idx = 0;
          ಯದಿ( Tcl_GetIndexFromObjStruct(interp, flags[j], aFlag, 
                ಪರಿಮಾಣ(aFlag[0]), "flag", 0, &idx) 
          ){
            ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
          }
          ಯದಿ( aFlag[idx].i_ಮೌಲ್ಯ<0 && nFlags>1 ){
            Tcl_AppendResult(interp, "bad flags: ", Tcl_GetString(objv[2]), ರಿಕ್ತ);
            ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
          }
          iNew |= aFlag[idx].i_ಮೌಲ್ಯ;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      ಪ್ರತಿ(iFlag=0; iFlag<ಪರಿಮಾಣ(aFlag)/ಪರಿಮಾಣ(aFlag[0]); iFlag++){
        ಯದಿ( p->iDevchar & aFlag[iFlag].i_ಮೌಲ್ಯ ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_SetObjResult(interp, pRet);

      ಭಙ್ಗ;
    }

    ಪಕ್ಷ CMD_SECTORSIZE: {
      ಯದಿ( objc>3 ){
        Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, "?VALUE?");
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಯದಿ( objc==3 ){
        ಪೂರ್ಣಾಙ್ಕ iNew = 0;
        ಯದಿ( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(p->iSectorsize));
      ಭಙ್ಗ;
    }
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_TCLAPI testvfs_obj_del(ClientData cd){
  Testvfs *p = (Testvfs *)cd;
  ಯದಿ( p->pScript ) Tcl_Decr_Ref_ಎಣಿಕೆ(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, ಪರಿಮಾಣ(sqlite3_vfs));
  ckfree((ವರ್ಣ *)p->pVfs);
  memset(p, 0, ಪರಿಮಾಣ(Testvfs));
  ckfree((ವರ್ಣ *)p);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI testvfs_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರ sqlite3_vfs tvfs_vfs = {
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
#ಅನ್ಯಥಾ
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
  ವರ್ಣ *zVfs;
  ಪೂರ್ಣಾಙ್ಕ nByte;                      /* Bytes of space to allocate at p */

  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ isNoshm = 0;                /* True if -noshm is passed */
  ಪೂರ್ಣಾಙ್ಕ isFullshm = 0;              /* True if -fullshm is passed */
  ಪೂರ್ಣಾಙ್ಕ isDefault = 0;              /* True if -default is passed */
  ಪೂರ್ಣಾಙ್ಕ szOsFile = 0;               /* Value passed to -szosfile */
  ಪೂರ್ಣಾಙ್ಕ mxPathname = -1;            /* Value passed to -mxpathname */
  ಪೂರ್ಣಾಙ್ಕ iVersion = 3;               /* Value passed to -iversion */

  ಯದಿ( objc<2 || 0!=(objc%2) ) ಗಚ್ಛ bad_args;
  ಪ್ರತಿ(i=2; i<objc; i += 2){
    Tcl_Size nSwitch;
    ವರ್ಣ *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    ಯದಿ( nSwitch>2 && 0==strncmp("-noshm", zSwitch, nSwitch) ){
      ಯದಿ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಯದಿ( isNoshm ) isFullshm = 0;
    }
    ಅನ್ಯಥಾ ಯದಿ( nSwitch>2 && 0==strncmp("-default", zSwitch, nSwitch) ){
      ಯದಿ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
    }
    ಅನ್ಯಥಾ ಯದಿ( nSwitch>2 && 0==strncmp("-szosfile", zSwitch, nSwitch) ){
      ಯದಿ( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
    }
    ಅನ್ಯಥಾ ಯದಿ( nSwitch>2 && 0==strncmp("-mxpathname", zSwitch, nSwitch) ){
      ಯದಿ( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
    }
    ಅನ್ಯಥಾ ಯದಿ( nSwitch>2 && 0==strncmp("-iversion", zSwitch, nSwitch) ){
      ಯದಿ( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
    }
    ಅನ್ಯಥಾ ಯದಿ( nSwitch>2 && 0==strncmp("-fullshm", zSwitch, nSwitch) ){
      ಯದಿ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      ಯದಿ( isFullshm ) isNoshm = 0;
    }
    ಅನ್ಯಥಾ{
      ಗಚ್ಛ bad_args;
    }
  }

  ಯದಿ( szOsFile<ಪರಿಮಾಣ(TestvfsFile) ){
    szOsFile = ಪರಿಮಾಣ(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = ಪರಿಮಾಣ(Testvfs) + (ಪೂರ್ಣಾಙ್ಕ)strlen(zVfs)+1;
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

  p->zName = (ವರ್ಣ *)&p[1];
  memcpy(p->zName, zVfs, strlen(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(ಪರಿಮಾಣ(sqlite3_vfs));
  memcpy(pVfs, &tvfs_vfs, ಪರಿಮಾಣ(sqlite3_vfs));
  pVfs->pAppData = (ಶೂನ್ಯ *)p;
  pVfs->iVersion = iVersion;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  ಯದಿ( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  ಪ್ರತ್ಯಾಗಮ TCL_OK;

 bad_args:
  Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp *interp, ಧ್ರುವ ವರ್ಣ *zA, sqlite3 **ppDb);
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_vfs_shmlock(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಧ್ರುವ ವರ್ಣ *azArg1[] = {"shared", "exclusive", 0};
  ಧ್ರುವ ವರ್ಣ *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಧ್ರುವ ವರ್ಣ *zDbname = 0;
  ಪೂರ್ಣಾಙ್ಕ iArg1 = 0;
  ಪೂರ್ಣಾಙ್ಕ iArg2 = 0;
  ಪೂರ್ಣಾಙ್ಕ iOffset = 0;
  ಪೂರ್ಣಾಙ್ಕ n = 0;
  sqlite3_file *pFd;

  ಯದಿ( objc!=7 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIndexFromObj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_GetIndexFromObj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (ಶೂನ್ಯ*)&pFd);
  ಯದಿ( pFd==0 ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_vfs_set_readmark(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  ಪೂರ್ಣಾಙ್ಕ rc = SQLITE_OK;
  ಧ್ರುವ ವರ್ಣ *zDbname = 0;
  ಪೂರ್ಣಾಙ್ಕ iSlot = 0;
  ಪೂರ್ಣಾಙ್ಕ iVal = -1;
  sqlite3_file *pFd;
  ಶೂನ್ಯ ಚಞ್ಚಲ *pShm = 0;
  u32 *aShm;
  ಪೂರ್ಣಾಙ್ಕ iOff;

  ಯದಿ( objc!=4 && objc!=5 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (ಶೂನ್ಯ*)&pFd);
  ಯದಿ( pFd==0 ){
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( pShm==0 ){
    Tcl_AppendResult(interp, "*-shm is not yet mapped", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  ಯದಿ( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(aShm[iOff]));

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಪೂರ್ಣಾಙ್ಕ Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

#endif
