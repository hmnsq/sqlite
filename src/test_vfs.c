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
#ଯଦି SQLITE_TEST          /* This file is used for testing only */

#ଅନ୍ତର୍ଭାଵ "sqlite3.h"
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା Testvfs Testvfs;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା TestvfsShm TestvfsShm;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା TestvfsBuffer TestvfsBuffer;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା TestvfsFile TestvfsFile;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
ସଂରଚନା TestvfsFile {
  sqlite3_file base;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#define tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

ସଂରଚନା TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  ଅଚଳ ଅକ୍ଷର *zFilename;          /* Filename as passed to xOpen() */
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

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା TestFaultInject TestFaultInject;
ସଂରଚନା TestFaultInject {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCnt;                       /* Remaining calls before fault injection */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eFault;                     /* A FAULT_INJECT_* value */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
ସଂରଚନା Testvfs {
  ଅକ୍ଷର *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  TestvfsBuffer *pBuffer;         /* List of shared buffers */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isNoshm;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isFullshm;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#ଯଦି 0
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iIoerrCnt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ioerr;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nIoerrFail;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFullCnt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fullerr;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nFullFail;
#endif

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iDevchar;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSectorsize;
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
ସଂରଚନା TestvfsBuffer {
  ଅକ୍ଷର *zFile;                    /* Associated file name */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  TestvfsBuffer *pNext;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->pAppData))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsClose(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsRead(sqlite3_file*, ଶୂନ୍ୟ*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64 iOfst);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsWrite(sqlite3_file*,ଅଚଳ ଶୂନ୍ୟ*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, sqlite3_int64 iOfst);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsTruncate(sqlite3_file*, sqlite3_int64 size);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsSync(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsLock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsUnlock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsCheckReservedLock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsFileControl(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsSectorSize(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsOpen(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *, sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା , ପୂର୍ଣ୍ଣସଂଖ୍ୟା *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsDelete(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା syncDir);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsAccess(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsFullPathname(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zName, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅକ୍ଷର *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
ସ୍ଥିର ଶୂନ୍ୟ *tvfsDlOpen(sqlite3_vfs*, ଅଚଳ ଅକ୍ଷର *zFilename);
ସ୍ଥିର ଶୂନ୍ୟ tvfsDlError(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *z_Err_ସନ୍ଦେଶ);
ସ୍ଥିର ଶୂନ୍ୟ (*tvfsDlSym(sqlite3_vfs*,ଶୂନ୍ୟ*, ଅଚଳ ଅକ୍ଷର *z_ପ୍ରତୀକ))(ଶୂନ୍ୟ);
ସ୍ଥିର ଶୂନ୍ୟ tvfsDlClose(sqlite3_vfs*, ଶୂନ୍ୟ*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsRandomness(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *zOut);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsSleep(sqlite3_vfs*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା microseconds);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsCurrentTime(sqlite3_vfs*, ଦ୍ୱିଗୁଣ*);

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsShmOpen(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsShmLock(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା , ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsShmMap(sqlite3_file*,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା,ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଶୂନ୍ୟ ଅସ୍ଥିର **);
ସ୍ଥିର ଶୂନ୍ୟ tvfsShmBarrier(sqlite3_file*);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsShmUnmap(sqlite3_file*, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsFetch(sqlite3_file*, sqlite3_int64, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଶୂନ୍ୟ**);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsUnfetch(sqlite3_file*, sqlite3_int64, ଶୂନ୍ୟ*);

ସ୍ଥିର sqlite3_io_methods tvfs_io_methods = {
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

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsResultCode(Testvfs *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pRc){
  ସଂରଚନା errcode {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା eCode;
    ଅଚଳ ଅକ୍ଷର *zCode;
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

  ଅଚଳ ଅକ୍ଷର *z;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  z = Tcl_GetStringResult(p->interp);
  ପାଇଁ(i=0; i<ArraySize(aCode); i++){
    ଯଦି( 0==strcmp(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      ଫେରନ୍ତୁ 1;
    }
  }

  ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsInjectFault(TestFaultInject *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
  ଯଦି( p->eFault ){
    p->iCnt--;
    ଯଦି( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  ଫେରନ୍ତୁ ret;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsInjectIoerr(Testvfs *p){
  ଫେରନ୍ତୁ tvfsInjectFault(&p->ioerr_err);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsInjectFullerr(Testvfs *p){
  ଫେରନ୍ତୁ tvfsInjectFault(&p->full_err);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsInjectCantopenerr(Testvfs *p){
  ଫେରନ୍ତୁ tvfsInjectFault(&p->cantopen_err);
}


ସ୍ଥିର ଶୂନ୍ୟ tvfsExecTcl(
  Testvfs *p, 
  ଅଚଳ ଅକ୍ଷର *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  ପ୍ରତିଜ୍ଞା( p->pScript );

  ପ୍ରତିଜ୍ଞା( zMethod );
  ପ୍ରତିଜ୍ଞା( p );
  ପ୍ରତିଜ୍ଞା( arg2==0 || arg1!=0 );
  ପ୍ରତିଜ୍ଞା( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ଗଣନା(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  ଯଦି( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  ଯଦି( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  ଯଦି( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  ଯଦି( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  ଯଦି( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_ResetResult(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ଯଦି( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  ଯଦି( pFd->pShmId ){
    Tcl_Decr_Ref_ଗଣନା(pFd->pShmId);
    pFd->pShmId = 0;
  }
  ଯଦି( pFile->pMethods ){
    ckfree((ଅକ୍ଷର *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((ଅକ୍ଷର *)pFd);
  pTestfile->pFd = 0;
  ଫେରନ୍ତୁ SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsRead(
  sqlite3_file *pFile, 
  ଶୂନ୍ୟ *zBuf, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, 
  sqlite_int64 iOfst
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ଯଦି( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  ଯଦି( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  ଯଦି( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Write data to an tvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsWrite(
  sqlite3_file *pFile, 
  ଅଚଳ ଶୂନ୍ୟ *zBuf, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, 
  sqlite_int64 iOfst
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ଯଦି( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfsResultCode(p, &rc);
    ଯଦି( rc<0 ) ଫେରନ୍ତୁ SQLITE_OK;
  }

  ଯଦି( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  ଯଦି( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  ଯଦି( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Truncate an tvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ଯଦି( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  
  ଯଦି( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, size);
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Sync an tvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsSync(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ଯଦି( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    ଅକ୍ଷର *zFlags = 0;

    ବାଛ( flags ){
      ସ୍ଥିତି SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_SYNC_FULL:
        zFlags = "full";
        ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        ଭାଙ୍ଗ;
      ସ୍ଥିତି SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ପ୍ରତିଜ୍ଞା(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  ଯଦି( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  ଯଦି( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  ଫେରନ୍ତୁ rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  TestvfsFd *p = tvfsGetFd(pFile);
  ଫେରନ୍ତୁ sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an tvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ଯଦି( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    ଅକ୍ଷର zLock[30];
    sqlite3_snprintf(ଆକାର(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  ଯଦି( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    ଫେରନ୍ତୁ SQLITE_IOERR_LOCK;
  }
  ଫେରନ୍ତୁ sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsUnlock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ଯଦି( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    ଅକ୍ଷର zLock[30];
    sqlite3_snprintf(ଆକାର(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  ଯଦି( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    ଫେରନ୍ତୁ SQLITE_IOERR_UNLOCK;
  }
  ଫେରନ୍ତୁ sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsCheckReservedLock(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ଯଦି( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  ଫେରନ୍ତୁ sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsFileControl(sqlite3_file *pFile, ପୂର୍ଣ୍ଣସଂଖ୍ୟା op, ଶୂନ୍ୟ *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ଯଦି( op==SQLITE_FCNTL_PRAGMA ){
    ଅକ୍ଷର **argv = (ଅକ୍ଷର**)pArg;
    ଯଦି( sqlite3_stricmp(argv[1],"error")==0 ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_ERROR;
      ଯଦି( argv[2] ){
        ଅଚଳ ଅକ୍ଷର *z = argv[2];
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା x = atoi(z);
        ଯଦି( x ){
          rc = x;
          ଯେତେବେଳେ( sqlite3Isdigit(z[0]) ){ z++; }
          ଯେତେବେଳେ( sqlite3Isspace(z[0]) ){ z++; }
        }
        ଯଦି( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      ଫେରନ୍ତୁ rc;
    }
    ଯଦି( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      ଫେରନ୍ତୁ SQLITE_OK;
    }
  }
  ଯଦି( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    ସଂରଚନା Fcntl {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFnctl;
      ଅଚଳ ଅକ୍ଷର *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<ଆକାର(aF)/ଆକାର(aF[0]); i++){
      ଯଦି( op==aF[i].iFnctl ) ଭାଙ୍ଗ;
    }
    ଯଦି( i<ଆକାର(aF)/ଆକାର(aF[0]) ){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfsResultCode(p, &rc);
      ଯଦି( rc ) ଫେରନ୍ତୁ (rc<0 ? SQLITE_OK : rc);
    }
  }
  ଫେରନ୍ତୁ sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsSectorSize(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ଯଦି( p->iSectorsize>=0 ){
    ଫେରନ୍ତୁ p->iSectorsize;
  }
  ଫେରନ୍ତୁ sqlite3OsSectorSize(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ଯଦି( p->iDevchar>=0 ){
    ଫେରନ୍ତୁ p->iDevchar;
  }
  ଫେରନ୍ତୁ sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsOpen(
  sqlite3_vfs *pVfs,
  ଅଚଳ ଅକ୍ଷର *zName,
  sqlite3_file *pFile,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pOutFlags
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  pFd = (TestvfsFd *)ckalloc(ଆକାର(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, ଆକାର(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, ଆକାର(TestvfsFile));
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
  ଯଦି( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_ଗଣନା(pArg);
    ଯଦି( flags&SQLITE_OPEN_MAIN_DB ){
      ଅଚଳ ଅକ୍ଷର *z = &zName[strlen(zName)+1];
      ଯେତେବେଳେ( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_ଗଣନା(pArg);
    ଯଦି( tvfsResultCode(p, &rc) ){
      ଯଦି( rc!=SQLITE_OK ) ଫେରନ୍ତୁ rc;
    }ଅନ୍ୟଥା{
      pId = Tcl_GetObjResult(p->interp);
    }
  }

  ଯଦି( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) ଫେରନ୍ତୁ SQLITE_IOERR;
  ଯଦି( tvfsInjectCantopenerr(p) ) ଫେରନ୍ତୁ SQLITE_CANTOPEN;
  ଯଦି( tvfsInjectFullerr(p) ) ଫେରନ୍ତୁ SQLITE_FULL;

  ଯଦି( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_ଗଣନା(pId);
  pFd->pShmId = pId;
  Tcl_ResetResult(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  ଯଦି( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;

    ଯଦି( pVfs->iVersion>1 ){
      nByte = ଆକାର(sqlite3_io_methods);
    }ଅନ୍ୟଥା{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    memcpy(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    ଯଦି( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    ଯଦି( pVfs->iVersion>1 && ((Testvfs *)pVfs->pAppData)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  ଫେରନ୍ତୁ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsDelete(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zPath, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dirSync){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  ଯଦି( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  ଯଦି( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  ଫେରନ୍ତୁ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsAccess(
  sqlite3_vfs *pVfs, 
  ଅଚଳ ଅକ୍ଷର *zPath, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  ଯଦି( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    ଅକ୍ଷର *zArg = 0;
    ଯଦି( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    ଯଦି( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    ଯଦି( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    ଯଦି( tvfsResultCode(p, &rc) ){
      ଯଦି( rc!=SQLITE_OK ) ଫେରନ୍ତୁ rc;
    }ଅନ୍ୟଥା{
      Tcl_Interp *interp = p->interp;
      ଯଦି( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_GetObjResult(interp), pResOut) ){
        ଫେରନ୍ତୁ SQLITE_OK;
      }
    }
  }
  ଫେରନ୍ତୁ sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  ଅଚଳ ଅକ୍ଷର *zPath, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nOut, 
  ଅକ୍ଷର *zOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  ଯଦି( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    ଯଦି( tvfsResultCode(p, &rc) ){
      ଯଦି( rc!=SQLITE_OK ) ଫେରନ୍ତୁ rc;
    }
  }
  ଫେରନ୍ତୁ sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ସ୍ଥିର ଶୂନ୍ୟ *tvfsDlOpen(sqlite3_vfs *pVfs, ଅଚଳ ଅକ୍ଷର *zPath){
  ଫେରନ୍ତୁ sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ସ୍ଥିର ଶୂନ୍ୟ tvfsDlError(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *z_Err_ସନ୍ଦେଶ){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_ସନ୍ଦେଶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ସ୍ଥିର ଶୂନ୍ୟ (*tvfsDlSym(sqlite3_vfs *pVfs, ଶୂନ୍ୟ *p, ଅଚଳ ଅକ୍ଷର *zSym))(ଶୂନ୍ୟ){
  ଫେରନ୍ତୁ sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ସ୍ଥିର ଶୂନ୍ୟ tvfsDlClose(sqlite3_vfs *pVfs, ଶୂନ୍ୟ *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsRandomness(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte, ଅକ୍ଷର *zBufOut){
  ଫେରନ୍ତୁ sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsSleep(sqlite3_vfs *pVfs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nMicro){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  ଯଦି( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  ଫେରନ୍ତୁ sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsCurrentTime(sqlite3_vfs *pVfs, ଦ୍ୱିଗୁଣ *pTimeOut){
  ଫେରନ୍ତୁ PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;             /* Return code */
  TestvfsBuffer *pBuffer;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->pAppData;
  ପ୍ରତିଜ୍ଞା( 0==p->isFullshm );
  ପ୍ରତିଜ୍ଞା( pFd->pShmId && pFd->pShm==0 && pFd->pNext==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_ResetResult(p->interp);
  ଯଦି( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    ଯଦି( tvfsResultCode(p, &rc) ){
      ଯଦି( rc!=SQLITE_OK ) ଫେରନ୍ତୁ rc;
    }
  }

  ପ୍ରତିଜ୍ଞା( rc==SQLITE_OK );
  ଯଦି( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    ଫେରନ୍ତୁ SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  ପାଇଁ(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
    ଯଦି( 0==strcmp(pFd->zFilename, pBuffer->zFile) ) ଭାଙ୍ଗ;
  }
  ଯଦି( !pBuffer ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା szName = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(pFd->zFilename);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte = ଆକାର(TestvfsBuffer) + szName + 1;
    pBuffer = (TestvfsBuffer *)ckalloc(nByte);
    memset(pBuffer, 0, nByte);
    pBuffer->zFile = (ଅକ୍ଷର *)&pBuffer[1];
    memcpy(pBuffer->zFile, pFd->zFilename, szName+1);
    pBuffer->pNext = p->pBuffer;
    p->pBuffer = pBuffer;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->pNext = pBuffer->pFile;
  pBuffer->pFile = pFd;
  pFd->pShm = pBuffer;
  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ଶୂନ୍ୟ tvfsAllocPage(TestvfsBuffer *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iPage, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgsz){
  ପ୍ରତିଜ୍ଞା( iPage<TESTVFS_MAX_PAGES );
  ଯଦି( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    memset(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iPage,                      /* Page to retrieve */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgsz,                       /* Size of pages */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isWrite,                    /* True to extend file if necessary */
  ଶୂନ୍ୟ ଅସ୍ଥିର **pp              /* OUT: Mapped memory */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  ଯଦି( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ଫେରନ୍ତୁ pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  ଯଦି( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    ଯଦି( rc!=SQLITE_OK ){
      ଫେରନ୍ତୁ rc;
    }
  }

  ଯଦି( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_ଗଣନା(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(iPage));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfsResultCode(p, &rc);
    Tcl_Decr_Ref_ଗଣନା(pArg);
  }
  ଯଦି( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ଯଦି( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  ଯଦି( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (ଶୂନ୍ୟ ଅସ୍ଥିର *)pFd->pShm->aPage[iPage];
  }

  ଫେରନ୍ତୁ rc;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsShmLock(
  sqlite3_file *pFile,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ofst,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nLock;
  ଅକ୍ଷର zLock[80];

  ଯଦି( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ଫେରନ୍ତୁ pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  ଯଦି( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(ଆକାର(zLock), zLock, "%d %d", ofst, n);
    nLock = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zLock);
    ଯଦି( flags & SQLITE_SHM_LOCK ){
      strcpy(&zLock[nLock], " lock");
    }ଅନ୍ୟଥା{
      strcpy(&zLock[nLock], " unlock");
    }
    nLock += (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(&zLock[nLock]);
    ଯଦି( flags & SQLITE_SHM_SHARED ){
      strcpy(&zLock[nLock], " shared");
    }ଅନ୍ୟଥା{
      strcpy(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  ଯଦି( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ଯଦି( rc==SQLITE_OK ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା isLock = (flags & SQLITE_SHM_LOCK);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    ଯଦି( isLock ){
      TestvfsFd *p2;
      ପାଇଁ(p2=pFd->pShm->pFile; p2; p2=p2->pNext){
        ଯଦି( p2==pFd ) ଜାରି;
        ଯଦି( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          ଭାଙ୍ଗ;
        }
      }
      ଯଦି( rc==SQLITE_OK ){
        ଯଦି( isExcl )  pFd->excllock |= mask;
        ଯଦି( !isExcl ) pFd->sharedlock |= mask;
      }
    }ଅନ୍ୟଥା{
      ଯଦି( isExcl )  pFd->excllock &= (~mask);
      ଯଦି( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ଶୂନ୍ୟ tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  ଯଦି( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    ଅଚଳ ଅକ୍ଷର *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  ଯଦି( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    ଫେରନ୍ତୁ;
  }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsShmUnmap(
  sqlite3_file *pFile,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା deleteFlag
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  TestvfsBuffer *pBuffer = pFd->pShm;
  TestvfsFd **ppFd;

  ଯଦି( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ଫେରନ୍ତୁ pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  ଯଦି( !pBuffer ) ଫେରନ୍ତୁ SQLITE_OK;
  ପ୍ରତିଜ୍ଞା( pFd->pShmId && pFd->pShm );

  ଯଦି( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }

  ପାଇଁ(ppFd=&pBuffer->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->pNext));
  ପ୍ରତିଜ୍ଞା( (*ppFd)==pFd );
  *ppFd = pFd->pNext;
  pFd->pNext = 0;

  ଯଦି( pBuffer->pFile==0 ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    TestvfsBuffer **pp;
    ପାଇଁ(pp=&p->pBuffer; *pp!=pBuffer; pp=&((*pp)->pNext));
    *pp = (*pp)->pNext;
    ପାଇଁ(i=0; pBuffer->aPage[i]; i++){
      ckfree((ଅକ୍ଷର *)pBuffer->aPage[i]);
    }
    ckfree((ଅକ୍ଷର *)pBuffer);
  }
  pFd->pShm = 0;

  ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା iAmt, 
    ଶୂନ୍ୟ **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  ଫେରନ୍ତୁ sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, ଶୂନ୍ୟ *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  ଫେରନ୍ତୁ sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI testvfs_obj_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  ଗଣନା DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  ସଂରଚନା TestvfsSubcmd {
    ଅକ୍ଷର *zName;
    ଗଣନା DB_enum eCmd;
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
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  
  ଯଦି( objc<2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "SUBCOMMAND ...");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( Tcl_GetIndexFromObjStruct(
        interp, objv[1], aSubcmd, ଆକାର(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  Tcl_ResetResult(interp);

  ବାଛ( aSubcmd[i].eCmd ){
    ସ୍ଥିତି CMD_SHM: {
      Tcl_Obj *pObj;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
      TestvfsBuffer *pBuffer;
      ଅକ୍ଷର *zName;
      ଯଦି( objc!=3 && objc!=4 ){
        Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "FILE ?VALUE?");
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      ଯଦି( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), ଖାଲି);
        ckfree(zName);
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ପାଇଁ(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
        ଯଦି( 0==strcmp(pBuffer->zFile, zName) ) ଭାଙ୍ଗ;
      }
      ckfree(zName);
      ଯଦି( !pBuffer ){
        Tcl_AppendResult(interp, "no such file: ", Tcl_GetString(objv[2]), ଖାଲି);
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ଯଦି( objc==4 ){
        Tcl_Size n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgsz = pBuffer->pgsz;
        ଯଦି( pgsz==0 ) pgsz = 65536;
        ପାଇଁ(i=0; i*pgsz<(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n; i++){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte = pgsz;
          tvfsAllocPage(pBuffer, i, pgsz);
          ଯଦି( n-i*pgsz<pgsz ){
            nByte = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n;
          }
          memcpy(pBuffer->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      ପାଇଁ(i=0; pBuffer->aPage[i]; i++){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା pgsz = pBuffer->pgsz;
        ଯଦି( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(pBuffer->aPage[i], pgsz));
      }
      Tcl_SetObjResult(interp, pObj);
      ଭାଙ୍ଗ;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    ସ୍ଥିତି CMD_FILTER: {
      ସ୍ଥିର ସଂରଚନା VfsMethod {
        ଅକ୍ଷର *zName;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask;
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
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask = 0;
      ଯଦି( objc!=3 ){
        Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "LIST");
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ଯଦି( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      Tcl_ResetResult(interp);
      ପାଇଁ(i=0; i<(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nElem; i++){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iMethod;
        ଅକ୍ଷର *zElem = Tcl_GetString(apElem[i]);
        ପାଇଁ(iMethod=0; iMethod<ArraySize(vfsmethod); iMethod++){
          ଯଦି( strcmp(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            ଭାଙ୍ଗ;
          }
        }
        ଯଦି( iMethod==ArraySize(vfsmethod) ){
          Tcl_AppendResult(interp, "unknown method: ", zElem, ଖାଲି);
          ଫେରନ୍ତୁ TCL_ERROR;
        }
      }
      p->mask = mask;
      ଭାଙ୍ଗ;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    ସ୍ଥିତି CMD_SCRIPT: {
      ଯଦି( objc==3 ){
        Tcl_Size nByte;
        ଯଦି( p->pScript ){
          Tcl_Decr_Ref_ଗଣନା(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        ଯଦି( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_ଗଣନା(p->pScript);
        }
      }ଅନ୍ୟଥା ଯଦି( objc!=2 ){
        Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?SCRIPT?");
        ଫେରନ୍ତୁ TCL_ERROR;
      }

      Tcl_ResetResult(interp);
      ଯଦି( p->pScript ) Tcl_SetObjResult(interp, p->pScript);

      ଭାଙ୍ଗ;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    ସ୍ଥିତି CMD_CANTOPENERR:
    ସ୍ଥିତି CMD_IOERR:
    ସ୍ଥିତି CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iRet;

      ବାଛ( aSubcmd[i].eCmd ){
        ସ୍ଥିତି CMD_IOERR: pTest = &p->ioerr_err; ଭାଙ୍ଗ;
        ସ୍ଥିତି CMD_FULLERR: pTest = &p->full_err; ଭାଙ୍ଗ;
        ସ୍ଥିତି CMD_CANTOPENERR: pTest = &p->cantopen_err; ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ପ୍ରତିଜ୍ଞା(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      ଯଦି( objc==4 ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCnt, iPersist;
        ଯଦି( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          ଫେରନ୍ତୁ TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }ଅନ୍ୟଥା ଯଦି( objc!=2 ){
        Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?CNT PERSIST?");
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(iRet));
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି CMD_DEVCHAR: {
      ସଂରଚନା DeviceFlag {
        ଅକ୍ଷର *zName;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_ମୂଲ୍ୟ;
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
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iFlag;

      ଯଦି( objc>3 ){
        Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?ATTR-LIST?");
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ଯଦି( objc==3 ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_Size nFlags = 0;

        ଯଦି( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          ଫେରନ୍ତୁ TCL_ERROR;
        }

        ପାଇଁ(j=0; j<(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)nFlags; j++){
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx = 0;
          ଯଦି( Tcl_GetIndexFromObjStruct(interp, flags[j], aFlag, 
                ଆକାର(aFlag[0]), "flag", 0, &idx) 
          ){
            ଫେରନ୍ତୁ TCL_ERROR;
          }
          ଯଦି( aFlag[idx].i_ମୂଲ୍ୟ<0 && nFlags>1 ){
            Tcl_AppendResult(interp, "bad flags: ", Tcl_GetString(objv[2]), ଖାଲି);
            ଫେରନ୍ତୁ TCL_ERROR;
          }
          iNew |= aFlag[idx].i_ମୂଲ୍ୟ;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      ପାଇଁ(iFlag=0; iFlag<ଆକାର(aFlag)/ଆକାର(aFlag[0]); iFlag++){
        ଯଦି( p->iDevchar & aFlag[iFlag].i_ମୂଲ୍ୟ ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_SetObjResult(interp, pRet);

      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି CMD_SECTORSIZE: {
      ଯଦି( objc>3 ){
        Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, "?VALUE?");
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ଯଦି( objc==3 ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iNew = 0;
        ଯଦି( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          ଫେରନ୍ତୁ TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(p->iSectorsize));
      ଭାଙ୍ଗ;
    }
  }

  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ଶୂନ୍ୟ SQLITE_TCLAPI testvfs_obj_del(ClientData cd){
  Testvfs *p = (Testvfs *)cd;
  ଯଦି( p->pScript ) Tcl_Decr_Ref_ଗଣନା(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, ଆକାର(sqlite3_vfs));
  ckfree((ଅକ୍ଷର *)p->pVfs);
  memset(p, 0, ଆକାର(Testvfs));
  ckfree((ଅକ୍ଷର *)p);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI testvfs_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ସ୍ଥିର sqlite3_vfs tvfs_vfs = {
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
#ଅନ୍ୟଥା
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
  ଅକ୍ଷର *zVfs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nByte;                      /* Bytes of space to allocate at p */

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isNoshm = 0;                /* True if -noshm is passed */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isFullshm = 0;              /* True if -fullshm is passed */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isDefault = 0;              /* True if -default is passed */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା szOsFile = 0;               /* Value passed to -szosfile */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mxPathname = -1;            /* Value passed to -mxpathname */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iVersion = 3;               /* Value passed to -iversion */

  ଯଦି( objc<2 || 0!=(objc%2) ) ଯାଅ bad_args;
  ପାଇଁ(i=2; i<objc; i += 2){
    Tcl_Size nSwitch;
    ଅକ୍ଷର *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    ଯଦି( nSwitch>2 && 0==strncmp("-noshm", zSwitch, nSwitch) ){
      ଯଦି( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ଯଦି( isNoshm ) isFullshm = 0;
    }
    ଅନ୍ୟଥା ଯଦି( nSwitch>2 && 0==strncmp("-default", zSwitch, nSwitch) ){
      ଯଦି( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
    }
    ଅନ୍ୟଥା ଯଦି( nSwitch>2 && 0==strncmp("-szosfile", zSwitch, nSwitch) ){
      ଯଦି( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
    }
    ଅନ୍ୟଥା ଯଦି( nSwitch>2 && 0==strncmp("-mxpathname", zSwitch, nSwitch) ){
      ଯଦି( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
    }
    ଅନ୍ୟଥା ଯଦି( nSwitch>2 && 0==strncmp("-iversion", zSwitch, nSwitch) ){
      ଯଦି( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
    }
    ଅନ୍ୟଥା ଯଦି( nSwitch>2 && 0==strncmp("-fullshm", zSwitch, nSwitch) ){
      ଯଦି( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      ଯଦି( isFullshm ) isNoshm = 0;
    }
    ଅନ୍ୟଥା{
      ଯାଅ bad_args;
    }
  }

  ଯଦି( szOsFile<ଆକାର(TestvfsFile) ){
    szOsFile = ଆକାର(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = ଆକାର(Testvfs) + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)strlen(zVfs)+1;
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

  p->zName = (ଅକ୍ଷର *)&p[1];
  memcpy(p->zName, zVfs, strlen(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(ଆକାର(sqlite3_vfs));
  memcpy(pVfs, &tvfs_vfs, ଆକାର(sqlite3_vfs));
  pVfs->pAppData = (ଶୂନ୍ୟ *)p;
  pVfs->iVersion = iVersion;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  ଯଦି( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  ଫେରନ୍ତୁ TCL_OK;

 bad_args:
  Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  ଫେରନ୍ତୁ TCL_ERROR;
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp *interp, ଅଚଳ ଅକ୍ଷର *zA, sqlite3 **ppDb);
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_vfs_shmlock(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  ଅଚଳ ଅକ୍ଷର *azArg1[] = {"shared", "exclusive", 0};
  ଅଚଳ ଅକ୍ଷର *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ଅଚଳ ଅକ୍ଷର *zDbname = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iArg1 = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iArg2 = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOffset = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;
  sqlite3_file *pFd;

  ଯଦି( objc!=7 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIndexFromObj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_GetIndexFromObj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (ଶୂନ୍ୟ*)&pFd);
  ଯଦି( pFd==0 ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_vfs_set_readmark(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = SQLITE_OK;
  ଅଚଳ ଅକ୍ଷର *zDbname = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSlot = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iVal = -1;
  sqlite3_file *pFd;
  ଶୂନ୍ୟ ଅସ୍ଥିର *pShm = 0;
  u32 *aShm;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iOff;

  ଯଦି( objc!=4 && objc!=5 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (ଶୂନ୍ୟ*)&pFd);
  ଯଦି( pFd==0 ){
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( pShm==0 ){
    Tcl_AppendResult(interp, "*-shm is not yet mapped", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  ଯଦି( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(aShm[iOff]));

  ଫେରନ୍ତୁ TCL_OK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  ଫେରନ୍ତୁ TCL_OK;
}

#endif
