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
#എങ്കിൽ SQLITE_TEST          /* This file is used for testing only */

#അന്തര്ഭാവ "sqlite3.h"
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"

തരനിർവചനം ഘടന Testvfs Testvfs;
തരനിർവചനം ഘടന TestvfsShm TestvfsShm;
തരനിർവചനം ഘടന TestvfsBuffer TestvfsBuffer;
തരനിർവചനം ഘടന TestvfsFile TestvfsFile;
തരനിർവചനം ഘടന TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
ഘടന TestvfsFile {
  sqlite3_file base;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#define tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

ഘടന TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  സ്ഥിരം അക്ഷരം *zFilename;          /* Filename as passed to xOpen() */
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

തരനിർവചനം ഘടന TestFaultInject TestFaultInject;
ഘടന TestFaultInject {
  പൂർണ്ണസംഖ്യ iCnt;                       /* Remaining calls before fault injection */
  പൂർണ്ണസംഖ്യ eFault;                     /* A FAULT_INJECT_* value */
  പൂർണ്ണസംഖ്യ nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
ഘടന Testvfs {
  അക്ഷരം *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  TestvfsBuffer *pBuffer;         /* List of shared buffers */
  പൂർണ്ണസംഖ്യ isNoshm;
  പൂർണ്ണസംഖ്യ isFullshm;

  പൂർണ്ണസംഖ്യ mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#എങ്കിൽ 0
  പൂർണ്ണസംഖ്യ iIoerrCnt;
  പൂർണ്ണസംഖ്യ ioerr;
  പൂർണ്ണസംഖ്യ nIoerrFail;
  പൂർണ്ണസംഖ്യ iFullCnt;
  പൂർണ്ണസംഖ്യ fullerr;
  പൂർണ്ണസംഖ്യ nFullFail;
#endif

  പൂർണ്ണസംഖ്യ iDevchar;
  പൂർണ്ണസംഖ്യ iSectorsize;
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
ഘടന TestvfsBuffer {
  അക്ഷരം *zFile;                    /* Associated file name */
  പൂർണ്ണസംഖ്യ pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  TestvfsBuffer *pNext;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->pAppData))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsClose(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsRead(sqlite3_file*, ശൂന്യം*, പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsWrite(sqlite3_file*,സ്ഥിരം ശൂന്യം*,പൂർണ്ണസംഖ്യ iAmt, sqlite3_int64 iOfst);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsTruncate(sqlite3_file*, sqlite3_int64 size);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsSync(sqlite3_file*, പൂർണ്ണസംഖ്യ flags);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsLock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsUnlock(sqlite3_file*, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsCheckReservedLock(sqlite3_file*, പൂർണ്ണസംഖ്യ *);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsFileControl(sqlite3_file*, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsSectorSize(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsOpen(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *, sqlite3_file*, പൂർണ്ണസംഖ്യ , പൂർണ്ണസംഖ്യ *);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsDelete(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ syncDir);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsAccess(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ flags, പൂർണ്ണസംഖ്യ *);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsFullPathname(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zName, പൂർണ്ണസംഖ്യ, അക്ഷരം *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
സ്ഥിതം ശൂന്യം *tvfsDlOpen(sqlite3_vfs*, സ്ഥിരം അക്ഷരം *zFilename);
സ്ഥിതം ശൂന്യം tvfsDlError(sqlite3_vfs*, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *z_Err_സന്ദേശം);
സ്ഥിതം ശൂന്യം (*tvfsDlSym(sqlite3_vfs*,ശൂന്യം*, സ്ഥിരം അക്ഷരം *z_ചിഹ്നം))(ശൂന്യം);
സ്ഥിതം ശൂന്യം tvfsDlClose(sqlite3_vfs*, ശൂന്യം*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsRandomness(sqlite3_vfs*, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zOut);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsSleep(sqlite3_vfs*, പൂർണ്ണസംഖ്യ microseconds);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsCurrentTime(sqlite3_vfs*, ഇരട്ട*);

സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsShmOpen(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsShmLock(sqlite3_file*, പൂർണ്ണസംഖ്യ , പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsShmMap(sqlite3_file*,പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ,പൂർണ്ണസംഖ്യ, ശൂന്യം അസ്ഥിര **);
സ്ഥിതം ശൂന്യം tvfsShmBarrier(sqlite3_file*);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsShmUnmap(sqlite3_file*, പൂർണ്ണസംഖ്യ);

സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsFetch(sqlite3_file*, sqlite3_int64, പൂർണ്ണസംഖ്യ, ശൂന്യം**);
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsUnfetch(sqlite3_file*, sqlite3_int64, ശൂന്യം*);

സ്ഥിതം sqlite3_io_methods tvfs_io_methods = {
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

സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsResultCode(Testvfs *p, പൂർണ്ണസംഖ്യ *pRc){
  ഘടന errcode {
    പൂർണ്ണസംഖ്യ eCode;
    സ്ഥിരം അക്ഷരം *zCode;
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

  സ്ഥിരം അക്ഷരം *z;
  പൂർണ്ണസംഖ്യ i;

  z = Tcl_GetStringResult(p->interp);
  വേണ്ടി(i=0; i<ArraySize(aCode); i++){
    എങ്കിൽ( 0==strcmp(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      തിരികെ 1;
    }
  }

  തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsInjectFault(TestFaultInject *p){
  പൂർണ്ണസംഖ്യ ret = 0;
  എങ്കിൽ( p->eFault ){
    p->iCnt--;
    എങ്കിൽ( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  തിരികെ ret;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsInjectIoerr(Testvfs *p){
  തിരികെ tvfsInjectFault(&p->ioerr_err);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsInjectFullerr(Testvfs *p){
  തിരികെ tvfsInjectFault(&p->full_err);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsInjectCantopenerr(Testvfs *p){
  തിരികെ tvfsInjectFault(&p->cantopen_err);
}


സ്ഥിതം ശൂന്യം tvfsExecTcl(
  Testvfs *p, 
  സ്ഥിരം അക്ഷരം *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  പൂർണ്ണസംഖ്യ rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  പ്രതിജ്ഞാ( p->pScript );

  പ്രതിജ്ഞാ( zMethod );
  പ്രതിജ്ഞാ( p );
  പ്രതിജ്ഞാ( arg2==0 || arg1!=0 );
  പ്രതിജ്ഞാ( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_എണ്ണം(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  എങ്കിൽ( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  എങ്കിൽ( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  എങ്കിൽ( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  എങ്കിൽ( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  എങ്കിൽ( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_ResetResult(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  എങ്കിൽ( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  എങ്കിൽ( pFd->pShmId ){
    Tcl_Decr_Ref_എണ്ണം(pFd->pShmId);
    pFd->pShmId = 0;
  }
  എങ്കിൽ( pFile->pMethods ){
    ckfree((അക്ഷരം *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((അക്ഷരം *)pFd);
  pTestfile->pFd = 0;
  തിരികെ SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsRead(
  sqlite3_file *pFile, 
  ശൂന്യം *zBuf, 
  പൂർണ്ണസംഖ്യ iAmt, 
  sqlite_int64 iOfst
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  എങ്കിൽ( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  എങ്കിൽ( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  എങ്കിൽ( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  തിരികെ rc;
}

/*
** Write data to an tvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsWrite(
  sqlite3_file *pFile, 
  സ്ഥിരം ശൂന്യം *zBuf, 
  പൂർണ്ണസംഖ്യ iAmt, 
  sqlite_int64 iOfst
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  എങ്കിൽ( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfsResultCode(p, &rc);
    എങ്കിൽ( rc<0 ) തിരികെ SQLITE_OK;
  }

  എങ്കിൽ( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  എങ്കിൽ( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  എങ്കിൽ( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  തിരികെ rc;
}

/*
** Truncate an tvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  എങ്കിൽ( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  
  എങ്കിൽ( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, size);
  }
  തിരികെ rc;
}

/*
** Sync an tvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsSync(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ flags){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  എങ്കിൽ( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    അക്ഷരം *zFlags = 0;

    തിരഞ്ഞെടുപ്പ്( flags ){
      അവസ്ഥ SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        നിർത്തുക;
      അവസ്ഥ SQLITE_SYNC_FULL:
        zFlags = "full";
        നിർത്തുക;
      അവസ്ഥ SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        നിർത്തുക;
      അവസ്ഥ SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        നിർത്തുക;
      സ്ഥിരസ്ഥിതി:
        പ്രതിജ്ഞാ(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  എങ്കിൽ( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  എങ്കിൽ( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  തിരികെ rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  TestvfsFd *p = tvfsGetFd(pFile);
  തിരികെ sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an tvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  എങ്കിൽ( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    അക്ഷരം zLock[30];
    sqlite3_snprintf(വലുപ്പം(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  എങ്കിൽ( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    തിരികെ SQLITE_IOERR_LOCK;
  }
  തിരികെ sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsUnlock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  എങ്കിൽ( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    അക്ഷരം zLock[30];
    sqlite3_snprintf(വലുപ്പം(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  എങ്കിൽ( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    തിരികെ SQLITE_IOERR_UNLOCK;
  }
  തിരികെ sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsCheckReservedLock(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  എങ്കിൽ( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  തിരികെ sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsFileControl(sqlite3_file *pFile, പൂർണ്ണസംഖ്യ op, ശൂന്യം *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  എങ്കിൽ( op==SQLITE_FCNTL_PRAGMA ){
    അക്ഷരം **argv = (അക്ഷരം**)pArg;
    എങ്കിൽ( sqlite3_stricmp(argv[1],"error")==0 ){
      പൂർണ്ണസംഖ്യ rc = SQLITE_ERROR;
      എങ്കിൽ( argv[2] ){
        സ്ഥിരം അക്ഷരം *z = argv[2];
        പൂർണ്ണസംഖ്യ x = atoi(z);
        എങ്കിൽ( x ){
          rc = x;
          ആകുമ്പോൾ( sqlite3Isdigit(z[0]) ){ z++; }
          ആകുമ്പോൾ( sqlite3Isspace(z[0]) ){ z++; }
        }
        എങ്കിൽ( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      തിരികെ rc;
    }
    എങ്കിൽ( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      തിരികെ SQLITE_OK;
    }
  }
  എങ്കിൽ( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    ഘടന Fcntl {
      പൂർണ്ണസംഖ്യ iFnctl;
      സ്ഥിരം അക്ഷരം *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി(i=0; i<വലുപ്പം(aF)/വലുപ്പം(aF[0]); i++){
      എങ്കിൽ( op==aF[i].iFnctl ) നിർത്തുക;
    }
    എങ്കിൽ( i<വലുപ്പം(aF)/വലുപ്പം(aF[0]) ){
      പൂർണ്ണസംഖ്യ rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfsResultCode(p, &rc);
      എങ്കിൽ( rc ) തിരികെ (rc<0 ? SQLITE_OK : rc);
    }
  }
  തിരികെ sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsSectorSize(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  എങ്കിൽ( p->iSectorsize>=0 ){
    തിരികെ p->iSectorsize;
  }
  തിരികെ sqlite3OsSectorSize(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  എങ്കിൽ( p->iDevchar>=0 ){
    തിരികെ p->iDevchar;
  }
  തിരികെ sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsOpen(
  sqlite3_vfs *pVfs,
  സ്ഥിരം അക്ഷരം *zName,
  sqlite3_file *pFile,
  പൂർണ്ണസംഖ്യ flags,
  പൂർണ്ണസംഖ്യ *pOutFlags
){
  പൂർണ്ണസംഖ്യ rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  pFd = (TestvfsFd *)ckalloc(വലുപ്പം(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, വലുപ്പം(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, വലുപ്പം(TestvfsFile));
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
  എങ്കിൽ( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_എണ്ണം(pArg);
    എങ്കിൽ( flags&SQLITE_OPEN_MAIN_DB ){
      സ്ഥിരം അക്ഷരം *z = &zName[strlen(zName)+1];
      ആകുമ്പോൾ( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_എണ്ണം(pArg);
    എങ്കിൽ( tvfsResultCode(p, &rc) ){
      എങ്കിൽ( rc!=SQLITE_OK ) തിരികെ rc;
    }അല്ലെങ്കിൽ{
      pId = Tcl_GetObjResult(p->interp);
    }
  }

  എങ്കിൽ( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) തിരികെ SQLITE_IOERR;
  എങ്കിൽ( tvfsInjectCantopenerr(p) ) തിരികെ SQLITE_CANTOPEN;
  എങ്കിൽ( tvfsInjectFullerr(p) ) തിരികെ SQLITE_FULL;

  എങ്കിൽ( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_എണ്ണം(pId);
  pFd->pShmId = pId;
  Tcl_ResetResult(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  എങ്കിൽ( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    പൂർണ്ണസംഖ്യ nByte;

    എങ്കിൽ( pVfs->iVersion>1 ){
      nByte = വലുപ്പം(sqlite3_io_methods);
    }അല്ലെങ്കിൽ{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    memcpy(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    എങ്കിൽ( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    എങ്കിൽ( pVfs->iVersion>1 && ((Testvfs *)pVfs->pAppData)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  തിരികെ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsDelete(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath, പൂർണ്ണസംഖ്യ dirSync){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  എങ്കിൽ( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  എങ്കിൽ( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  തിരികെ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsAccess(
  sqlite3_vfs *pVfs, 
  സ്ഥിരം അക്ഷരം *zPath, 
  പൂർണ്ണസംഖ്യ flags, 
  പൂർണ്ണസംഖ്യ *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  എങ്കിൽ( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    പൂർണ്ണസംഖ്യ rc;
    അക്ഷരം *zArg = 0;
    എങ്കിൽ( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    എങ്കിൽ( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    എങ്കിൽ( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    എങ്കിൽ( tvfsResultCode(p, &rc) ){
      എങ്കിൽ( rc!=SQLITE_OK ) തിരികെ rc;
    }അല്ലെങ്കിൽ{
      Tcl_Interp *interp = p->interp;
      എങ്കിൽ( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_GetObjResult(interp), pResOut) ){
        തിരികെ SQLITE_OK;
      }
    }
  }
  തിരികെ sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  സ്ഥിരം അക്ഷരം *zPath, 
  പൂർണ്ണസംഖ്യ nOut, 
  അക്ഷരം *zOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  എങ്കിൽ( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    പൂർണ്ണസംഖ്യ rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    എങ്കിൽ( tvfsResultCode(p, &rc) ){
      എങ്കിൽ( rc!=SQLITE_OK ) തിരികെ rc;
    }
  }
  തിരികെ sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
സ്ഥിതം ശൂന്യം *tvfsDlOpen(sqlite3_vfs *pVfs, സ്ഥിരം അക്ഷരം *zPath){
  തിരികെ sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
സ്ഥിതം ശൂന്യം tvfsDlError(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *z_Err_സന്ദേശം){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_സന്ദേശം);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
സ്ഥിതം ശൂന്യം (*tvfsDlSym(sqlite3_vfs *pVfs, ശൂന്യം *p, സ്ഥിരം അക്ഷരം *zSym))(ശൂന്യം){
  തിരികെ sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
സ്ഥിതം ശൂന്യം tvfsDlClose(sqlite3_vfs *pVfs, ശൂന്യം *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsRandomness(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nByte, അക്ഷരം *zBufOut){
  തിരികെ sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsSleep(sqlite3_vfs *pVfs, പൂർണ്ണസംഖ്യ nMicro){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  എങ്കിൽ( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  തിരികെ sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsCurrentTime(sqlite3_vfs *pVfs, ഇരട്ട *pTimeOut){
  തിരികെ PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;             /* Return code */
  TestvfsBuffer *pBuffer;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->pAppData;
  പ്രതിജ്ഞാ( 0==p->isFullshm );
  പ്രതിജ്ഞാ( pFd->pShmId && pFd->pShm==0 && pFd->pNext==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_ResetResult(p->interp);
  എങ്കിൽ( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    എങ്കിൽ( tvfsResultCode(p, &rc) ){
      എങ്കിൽ( rc!=SQLITE_OK ) തിരികെ rc;
    }
  }

  പ്രതിജ്ഞാ( rc==SQLITE_OK );
  എങ്കിൽ( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    തിരികെ SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  വേണ്ടി(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
    എങ്കിൽ( 0==strcmp(pFd->zFilename, pBuffer->zFile) ) നിർത്തുക;
  }
  എങ്കിൽ( !pBuffer ){
    പൂർണ്ണസംഖ്യ szName = (പൂർണ്ണസംഖ്യ)strlen(pFd->zFilename);
    പൂർണ്ണസംഖ്യ nByte = വലുപ്പം(TestvfsBuffer) + szName + 1;
    pBuffer = (TestvfsBuffer *)ckalloc(nByte);
    memset(pBuffer, 0, nByte);
    pBuffer->zFile = (അക്ഷരം *)&pBuffer[1];
    memcpy(pBuffer->zFile, pFd->zFilename, szName+1);
    pBuffer->pNext = p->pBuffer;
    p->pBuffer = pBuffer;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->pNext = pBuffer->pFile;
  pBuffer->pFile = pFd;
  pFd->pShm = pBuffer;
  തിരികെ rc;
}

സ്ഥിതം ശൂന്യം tvfsAllocPage(TestvfsBuffer *p, പൂർണ്ണസംഖ്യ iPage, പൂർണ്ണസംഖ്യ pgsz){
  പ്രതിജ്ഞാ( iPage<TESTVFS_MAX_PAGES );
  എങ്കിൽ( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    memset(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  പൂർണ്ണസംഖ്യ iPage,                      /* Page to retrieve */
  പൂർണ്ണസംഖ്യ pgsz,                       /* Size of pages */
  പൂർണ്ണസംഖ്യ isWrite,                    /* True to extend file if necessary */
  ശൂന്യം അസ്ഥിര **pp              /* OUT: Mapped memory */
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  എങ്കിൽ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    തിരികെ pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  എങ്കിൽ( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    എങ്കിൽ( rc!=SQLITE_OK ){
      തിരികെ rc;
    }
  }

  എങ്കിൽ( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_എണ്ണം(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(iPage));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfsResultCode(p, &rc);
    Tcl_Decr_Ref_എണ്ണം(pArg);
  }
  എങ്കിൽ( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  എങ്കിൽ( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  എങ്കിൽ( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (ശൂന്യം അസ്ഥിര *)pFd->pShm->aPage[iPage];
  }

  തിരികെ rc;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsShmLock(
  sqlite3_file *pFile,
  പൂർണ്ണസംഖ്യ ofst,
  പൂർണ്ണസംഖ്യ n,
  പൂർണ്ണസംഖ്യ flags
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  പൂർണ്ണസംഖ്യ nLock;
  അക്ഷരം zLock[80];

  എങ്കിൽ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    തിരികെ pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  എങ്കിൽ( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(വലുപ്പം(zLock), zLock, "%d %d", ofst, n);
    nLock = (പൂർണ്ണസംഖ്യ)strlen(zLock);
    എങ്കിൽ( flags & SQLITE_SHM_LOCK ){
      strcpy(&zLock[nLock], " lock");
    }അല്ലെങ്കിൽ{
      strcpy(&zLock[nLock], " unlock");
    }
    nLock += (പൂർണ്ണസംഖ്യ)strlen(&zLock[nLock]);
    എങ്കിൽ( flags & SQLITE_SHM_SHARED ){
      strcpy(&zLock[nLock], " shared");
    }അല്ലെങ്കിൽ{
      strcpy(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  എങ്കിൽ( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  എങ്കിൽ( rc==SQLITE_OK ){
    പൂർണ്ണസംഖ്യ isLock = (flags & SQLITE_SHM_LOCK);
    പൂർണ്ണസംഖ്യ isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    എങ്കിൽ( isLock ){
      TestvfsFd *p2;
      വേണ്ടി(p2=pFd->pShm->pFile; p2; p2=p2->pNext){
        എങ്കിൽ( p2==pFd ) തുടരുക;
        എങ്കിൽ( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          നിർത്തുക;
        }
      }
      എങ്കിൽ( rc==SQLITE_OK ){
        എങ്കിൽ( isExcl )  pFd->excllock |= mask;
        എങ്കിൽ( !isExcl ) pFd->sharedlock |= mask;
      }
    }അല്ലെങ്കിൽ{
      എങ്കിൽ( isExcl )  pFd->excllock &= (~mask);
      എങ്കിൽ( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  തിരികെ rc;
}

സ്ഥിതം ശൂന്യം tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  എങ്കിൽ( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    സ്ഥിരം അക്ഷരം *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  എങ്കിൽ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    തിരികെ;
  }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsShmUnmap(
  sqlite3_file *pFile,
  പൂർണ്ണസംഖ്യ deleteFlag
){
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  TestvfsBuffer *pBuffer = pFd->pShm;
  TestvfsFd **ppFd;

  എങ്കിൽ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    തിരികെ pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  എങ്കിൽ( !pBuffer ) തിരികെ SQLITE_OK;
  പ്രതിജ്ഞാ( pFd->pShmId && pFd->pShm );

  എങ്കിൽ( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }

  വേണ്ടി(ppFd=&pBuffer->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->pNext));
  പ്രതിജ്ഞാ( (*ppFd)==pFd );
  *ppFd = pFd->pNext;
  pFd->pNext = 0;

  എങ്കിൽ( pBuffer->pFile==0 ){
    പൂർണ്ണസംഖ്യ i;
    TestvfsBuffer **pp;
    വേണ്ടി(pp=&p->pBuffer; *pp!=pBuffer; pp=&((*pp)->pNext));
    *pp = (*pp)->pNext;
    വേണ്ടി(i=0; pBuffer->aPage[i]; i++){
      ckfree((അക്ഷരം *)pBuffer->aPage[i]);
    }
    ckfree((അക്ഷരം *)pBuffer);
  }
  pFd->pShm = 0;

  തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    പൂർണ്ണസംഖ്യ iAmt, 
    ശൂന്യം **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  തിരികെ sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, ശൂന്യം *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  തിരികെ sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI testvfs_obj_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  എണ്ണൽ DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  ഘടന TestvfsSubcmd {
    അക്ഷരം *zName;
    എണ്ണൽ DB_enum eCmd;
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
  പൂർണ്ണസംഖ്യ i;
  
  എങ്കിൽ( objc<2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "SUBCOMMAND ...");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( Tcl_GetIndexFromObjStruct(
        interp, objv[1], aSubcmd, വലുപ്പം(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    തിരികെ TCL_ERROR;
  }
  Tcl_ResetResult(interp);

  തിരഞ്ഞെടുപ്പ്( aSubcmd[i].eCmd ){
    അവസ്ഥ CMD_SHM: {
      Tcl_Obj *pObj;
      പൂർണ്ണസംഖ്യ rc;
      TestvfsBuffer *pBuffer;
      അക്ഷരം *zName;
      എങ്കിൽ( objc!=3 && objc!=4 ){
        Tcl_Wrong_അക്കം_Args(interp, 2, objv, "FILE ?VALUE?");
        തിരികെ TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      എങ്കിൽ( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), ശൂന്യ);
        ckfree(zName);
        തിരികെ TCL_ERROR;
      }
      വേണ്ടി(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
        എങ്കിൽ( 0==strcmp(pBuffer->zFile, zName) ) നിർത്തുക;
      }
      ckfree(zName);
      എങ്കിൽ( !pBuffer ){
        Tcl_AppendResult(interp, "no such file: ", Tcl_GetString(objv[2]), ശൂന്യ);
        തിരികെ TCL_ERROR;
      }
      എങ്കിൽ( objc==4 ){
        Tcl_Size n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        പൂർണ്ണസംഖ്യ pgsz = pBuffer->pgsz;
        എങ്കിൽ( pgsz==0 ) pgsz = 65536;
        വേണ്ടി(i=0; i*pgsz<(പൂർണ്ണസംഖ്യ)n; i++){
          പൂർണ്ണസംഖ്യ nByte = pgsz;
          tvfsAllocPage(pBuffer, i, pgsz);
          എങ്കിൽ( n-i*pgsz<pgsz ){
            nByte = (പൂർണ്ണസംഖ്യ)n;
          }
          memcpy(pBuffer->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      വേണ്ടി(i=0; pBuffer->aPage[i]; i++){
        പൂർണ്ണസംഖ്യ pgsz = pBuffer->pgsz;
        എങ്കിൽ( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(pBuffer->aPage[i], pgsz));
      }
      Tcl_SetObjResult(interp, pObj);
      നിർത്തുക;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    അവസ്ഥ CMD_FILTER: {
      സ്ഥിതം ഘടന VfsMethod {
        അക്ഷരം *zName;
        പൂർണ്ണസംഖ്യ mask;
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
      പൂർണ്ണസംഖ്യ mask = 0;
      എങ്കിൽ( objc!=3 ){
        Tcl_Wrong_അക്കം_Args(interp, 2, objv, "LIST");
        തിരികെ TCL_ERROR;
      }
      എങ്കിൽ( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        തിരികെ TCL_ERROR;
      }
      Tcl_ResetResult(interp);
      വേണ്ടി(i=0; i<(പൂർണ്ണസംഖ്യ)nElem; i++){
        പൂർണ്ണസംഖ്യ iMethod;
        അക്ഷരം *zElem = Tcl_GetString(apElem[i]);
        വേണ്ടി(iMethod=0; iMethod<ArraySize(vfsmethod); iMethod++){
          എങ്കിൽ( strcmp(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            നിർത്തുക;
          }
        }
        എങ്കിൽ( iMethod==ArraySize(vfsmethod) ){
          Tcl_AppendResult(interp, "unknown method: ", zElem, ശൂന്യ);
          തിരികെ TCL_ERROR;
        }
      }
      p->mask = mask;
      നിർത്തുക;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    അവസ്ഥ CMD_SCRIPT: {
      എങ്കിൽ( objc==3 ){
        Tcl_Size nByte;
        എങ്കിൽ( p->pScript ){
          Tcl_Decr_Ref_എണ്ണം(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        എങ്കിൽ( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_എണ്ണം(p->pScript);
        }
      }അല്ലെങ്കിൽ എങ്കിൽ( objc!=2 ){
        Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?SCRIPT?");
        തിരികെ TCL_ERROR;
      }

      Tcl_ResetResult(interp);
      എങ്കിൽ( p->pScript ) Tcl_SetObjResult(interp, p->pScript);

      നിർത്തുക;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    അവസ്ഥ CMD_CANTOPENERR:
    അവസ്ഥ CMD_IOERR:
    അവസ്ഥ CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      പൂർണ്ണസംഖ്യ iRet;

      തിരഞ്ഞെടുപ്പ്( aSubcmd[i].eCmd ){
        അവസ്ഥ CMD_IOERR: pTest = &p->ioerr_err; നിർത്തുക;
        അവസ്ഥ CMD_FULLERR: pTest = &p->full_err; നിർത്തുക;
        അവസ്ഥ CMD_CANTOPENERR: pTest = &p->cantopen_err; നിർത്തുക;
        സ്ഥിരസ്ഥിതി: പ്രതിജ്ഞാ(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      എങ്കിൽ( objc==4 ){
        പൂർണ്ണസംഖ്യ iCnt, iPersist;
        എങ്കിൽ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          തിരികെ TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }അല്ലെങ്കിൽ എങ്കിൽ( objc!=2 ){
        Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?CNT PERSIST?");
        തിരികെ TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(iRet));
      നിർത്തുക;
    }

    അവസ്ഥ CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      നിർത്തുക;
    }

    അവസ്ഥ CMD_DEVCHAR: {
      ഘടന DeviceFlag {
        അക്ഷരം *zName;
        പൂർണ്ണസംഖ്യ i_മൂല്യം;
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
      പൂർണ്ണസംഖ്യ iFlag;

      എങ്കിൽ( objc>3 ){
        Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?ATTR-LIST?");
        തിരികെ TCL_ERROR;
      }
      എങ്കിൽ( objc==3 ){
        പൂർണ്ണസംഖ്യ j;
        പൂർണ്ണസംഖ്യ iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_Size nFlags = 0;

        എങ്കിൽ( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          തിരികെ TCL_ERROR;
        }

        വേണ്ടി(j=0; j<(പൂർണ്ണസംഖ്യ)nFlags; j++){
          പൂർണ്ണസംഖ്യ idx = 0;
          എങ്കിൽ( Tcl_GetIndexFromObjStruct(interp, flags[j], aFlag, 
                വലുപ്പം(aFlag[0]), "flag", 0, &idx) 
          ){
            തിരികെ TCL_ERROR;
          }
          എങ്കിൽ( aFlag[idx].i_മൂല്യം<0 && nFlags>1 ){
            Tcl_AppendResult(interp, "bad flags: ", Tcl_GetString(objv[2]), ശൂന്യ);
            തിരികെ TCL_ERROR;
          }
          iNew |= aFlag[idx].i_മൂല്യം;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      വേണ്ടി(iFlag=0; iFlag<വലുപ്പം(aFlag)/വലുപ്പം(aFlag[0]); iFlag++){
        എങ്കിൽ( p->iDevchar & aFlag[iFlag].i_മൂല്യം ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_SetObjResult(interp, pRet);

      നിർത്തുക;
    }

    അവസ്ഥ CMD_SECTORSIZE: {
      എങ്കിൽ( objc>3 ){
        Tcl_Wrong_അക്കം_Args(interp, 2, objv, "?VALUE?");
        തിരികെ TCL_ERROR;
      }
      എങ്കിൽ( objc==3 ){
        പൂർണ്ണസംഖ്യ iNew = 0;
        എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          തിരികെ TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(p->iSectorsize));
      നിർത്തുക;
    }
  }

  തിരികെ TCL_OK;
}

സ്ഥിതം ശൂന്യം SQLITE_TCLAPI testvfs_obj_del(ClientData cd){
  Testvfs *p = (Testvfs *)cd;
  എങ്കിൽ( p->pScript ) Tcl_Decr_Ref_എണ്ണം(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, വലുപ്പം(sqlite3_vfs));
  ckfree((അക്ഷരം *)p->pVfs);
  memset(p, 0, വലുപ്പം(Testvfs));
  ckfree((അക്ഷരം *)p);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI testvfs_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  സ്ഥിതം sqlite3_vfs tvfs_vfs = {
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
#അല്ലെങ്കിൽ
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
  അക്ഷരം *zVfs;
  പൂർണ്ണസംഖ്യ nByte;                      /* Bytes of space to allocate at p */

  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ isNoshm = 0;                /* True if -noshm is passed */
  പൂർണ്ണസംഖ്യ isFullshm = 0;              /* True if -fullshm is passed */
  പൂർണ്ണസംഖ്യ isDefault = 0;              /* True if -default is passed */
  പൂർണ്ണസംഖ്യ szOsFile = 0;               /* Value passed to -szosfile */
  പൂർണ്ണസംഖ്യ mxPathname = -1;            /* Value passed to -mxpathname */
  പൂർണ്ണസംഖ്യ iVersion = 3;               /* Value passed to -iversion */

  എങ്കിൽ( objc<2 || 0!=(objc%2) ) പോകുക bad_args;
  വേണ്ടി(i=2; i<objc; i += 2){
    Tcl_Size nSwitch;
    അക്ഷരം *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    എങ്കിൽ( nSwitch>2 && 0==strncmp("-noshm", zSwitch, nSwitch) ){
      എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        തിരികെ TCL_ERROR;
      }
      എങ്കിൽ( isNoshm ) isFullshm = 0;
    }
    അല്ലെങ്കിൽ എങ്കിൽ( nSwitch>2 && 0==strncmp("-default", zSwitch, nSwitch) ){
      എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        തിരികെ TCL_ERROR;
      }
    }
    അല്ലെങ്കിൽ എങ്കിൽ( nSwitch>2 && 0==strncmp("-szosfile", zSwitch, nSwitch) ){
      എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        തിരികെ TCL_ERROR;
      }
    }
    അല്ലെങ്കിൽ എങ്കിൽ( nSwitch>2 && 0==strncmp("-mxpathname", zSwitch, nSwitch) ){
      എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        തിരികെ TCL_ERROR;
      }
    }
    അല്ലെങ്കിൽ എങ്കിൽ( nSwitch>2 && 0==strncmp("-iversion", zSwitch, nSwitch) ){
      എങ്കിൽ( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        തിരികെ TCL_ERROR;
      }
    }
    അല്ലെങ്കിൽ എങ്കിൽ( nSwitch>2 && 0==strncmp("-fullshm", zSwitch, nSwitch) ){
      എങ്കിൽ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        തിരികെ TCL_ERROR;
      }
      എങ്കിൽ( isFullshm ) isNoshm = 0;
    }
    അല്ലെങ്കിൽ{
      പോകുക bad_args;
    }
  }

  എങ്കിൽ( szOsFile<വലുപ്പം(TestvfsFile) ){
    szOsFile = വലുപ്പം(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = വലുപ്പം(Testvfs) + (പൂർണ്ണസംഖ്യ)strlen(zVfs)+1;
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

  p->zName = (അക്ഷരം *)&p[1];
  memcpy(p->zName, zVfs, strlen(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(വലുപ്പം(sqlite3_vfs));
  memcpy(pVfs, &tvfs_vfs, വലുപ്പം(sqlite3_vfs));
  pVfs->pAppData = (ശൂന്യം *)p;
  pVfs->iVersion = iVersion;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  എങ്കിൽ( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  തിരികെ TCL_OK;

 bad_args:
  Tcl_Wrong_അക്കം_Args(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  തിരികെ TCL_ERROR;
}

ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp *interp, സ്ഥിരം അക്ഷരം *zA, sqlite3 **ppDb);
ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_vfs_shmlock(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  സ്ഥിരം അക്ഷരം *azArg1[] = {"shared", "exclusive", 0};
  സ്ഥിരം അക്ഷരം *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  സ്ഥിരം അക്ഷരം *zDbname = 0;
  പൂർണ്ണസംഖ്യ iArg1 = 0;
  പൂർണ്ണസംഖ്യ iArg2 = 0;
  പൂർണ്ണസംഖ്യ iOffset = 0;
  പൂർണ്ണസംഖ്യ n = 0;
  sqlite3_file *pFd;

  എങ്കിൽ( objc!=7 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    തിരികെ TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIndexFromObj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_GetIndexFromObj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    തിരികെ TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (ശൂന്യം*)&pFd);
  എങ്കിൽ( pFd==0 ){
    തിരികെ TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_vfs_set_readmark(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  പൂർണ്ണസംഖ്യ rc = SQLITE_OK;
  സ്ഥിരം അക്ഷരം *zDbname = 0;
  പൂർണ്ണസംഖ്യ iSlot = 0;
  പൂർണ്ണസംഖ്യ iVal = -1;
  sqlite3_file *pFd;
  ശൂന്യം അസ്ഥിര *pShm = 0;
  u32 *aShm;
  പൂർണ്ണസംഖ്യ iOff;

  എങ്കിൽ( objc!=4 && objc!=5 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    തിരികെ TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    തിരികെ TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (ശൂന്യം*)&pFd);
  എങ്കിൽ( pFd==0 ){
    തിരികെ TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( pShm==0 ){
    Tcl_AppendResult(interp, "*-shm is not yet mapped", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  എങ്കിൽ( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(aShm[iOff]));

  തിരികെ TCL_OK;
}

പൂർണ്ണസംഖ്യ Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  തിരികെ TCL_OK;
}

#endif
