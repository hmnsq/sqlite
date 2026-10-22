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
#যদি SQLITE_TEST          /* This file is used for testing only */

#অন্তর্ভাৱ "sqlite3.h"
#অন্তর্ভাৱ "sqliteInt.h"
#অন্তর্ভাৱ "tclsqlite.h"

প্রকারনির্দেশ সংরচনা Testvfs Testvfs;
প্রকারনির্দেশ সংরচনা TestvfsShm TestvfsShm;
প্রকারনির্দেশ সংরচনা TestvfsBuffer TestvfsBuffer;
প্রকারনির্দেশ সংরচনা TestvfsFile TestvfsFile;
প্রকারনির্দেশ সংরচনা TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
সংরচনা TestvfsFile {
  sqlite3_file base;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#define tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

সংরচনা TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  ধ্রুৱ ৱর্ণ *zFilename;          /* Filename as passed to xOpen() */
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

প্রকারনির্দেশ সংরচনা TestFaultInject TestFaultInject;
সংরচনা TestFaultInject {
  পূর্ণাঙ্ক iCnt;                       /* Remaining calls before fault injection */
  পূর্ণাঙ্ক eFault;                     /* A FAULT_INJECT_* value */
  পূর্ণাঙ্ক nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
সংরচনা Testvfs {
  ৱর্ণ *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  TestvfsBuffer *pBuffer;         /* List of shared buffers */
  পূর্ণাঙ্ক isNoshm;
  পূর্ণাঙ্ক isFullshm;

  পূর্ণাঙ্ক mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#যদি 0
  পূর্ণাঙ্ক iIoerrCnt;
  পূর্ণাঙ্ক ioerr;
  পূর্ণাঙ্ক nIoerrFail;
  পূর্ণাঙ্ক iFullCnt;
  পূর্ণাঙ্ক fullerr;
  পূর্ণাঙ্ক nFullFail;
#endif

  পূর্ণাঙ্ক iDevchar;
  পূর্ণাঙ্ক iSectorsize;
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
সংরচনা TestvfsBuffer {
  ৱর্ণ *zFile;                    /* Associated file name */
  পূর্ণাঙ্ক pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  TestvfsBuffer *pNext;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->pAppData))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
স্থির পূর্ণাঙ্ক tvfsClose(sqlite3_file*);
স্থির পূর্ণাঙ্ক tvfsRead(sqlite3_file*, শূন্য*, পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক tvfsWrite(sqlite3_file*,ধ্রুৱ শূন্য*,পূর্ণাঙ্ক iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণাঙ্ক tvfsTruncate(sqlite3_file*, sqlite3_int64 size);
স্থির পূর্ণাঙ্ক tvfsSync(sqlite3_file*, পূর্ণাঙ্ক flags);
স্থির পূর্ণাঙ্ক tvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
স্থির পূর্ণাঙ্ক tvfsLock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক tvfsUnlock(sqlite3_file*, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক tvfsCheckReservedLock(sqlite3_file*, পূর্ণাঙ্ক *);
স্থির পূর্ণাঙ্ক tvfsFileControl(sqlite3_file*, পূর্ণাঙ্ক op, শূন্য *pArg);
স্থির পূর্ণাঙ্ক tvfsSectorSize(sqlite3_file*);
স্থির পূর্ণাঙ্ক tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
স্থির পূর্ণাঙ্ক tvfsOpen(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *, sqlite3_file*, পূর্ণাঙ্ক , পূর্ণাঙ্ক *);
স্থির পূর্ণাঙ্ক tvfsDelete(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *zName, পূর্ণাঙ্ক syncDir);
স্থির পূর্ণাঙ্ক tvfsAccess(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *zName, পূর্ণাঙ্ক flags, পূর্ণাঙ্ক *);
স্থির পূর্ণাঙ্ক tvfsFullPathname(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *zName, পূর্ণাঙ্ক, ৱর্ণ *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
স্থির শূন্য *tvfsDlOpen(sqlite3_vfs*, ধ্রুৱ ৱর্ণ *zFilename);
স্থির শূন্য tvfsDlError(sqlite3_vfs*, পূর্ণাঙ্ক nByte, ৱর্ণ *z_Err_বাৰ্তা);
স্থির শূন্য (*tvfsDlSym(sqlite3_vfs*,শূন্য*, ধ্রুৱ ৱর্ণ *z_প্ৰতীক))(শূন্য);
স্থির শূন্য tvfsDlClose(sqlite3_vfs*, শূন্য*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
স্থির পূর্ণাঙ্ক tvfsRandomness(sqlite3_vfs*, পূর্ণাঙ্ক nByte, ৱর্ণ *zOut);
স্থির পূর্ণাঙ্ক tvfsSleep(sqlite3_vfs*, পূর্ণাঙ্ক microseconds);
স্থির পূর্ণাঙ্ক tvfsCurrentTime(sqlite3_vfs*, দ্ৱিগুণ*);

স্থির পূর্ণাঙ্ক tvfsShmOpen(sqlite3_file*);
স্থির পূর্ণাঙ্ক tvfsShmLock(sqlite3_file*, পূর্ণাঙ্ক , পূর্ণাঙ্ক, পূর্ণাঙ্ক);
স্থির পূর্ণাঙ্ক tvfsShmMap(sqlite3_file*,পূর্ণাঙ্ক,পূর্ণাঙ্ক,পূর্ণাঙ্ক, শূন্য চঞ্চল **);
স্থির শূন্য tvfsShmBarrier(sqlite3_file*);
স্থির পূর্ণাঙ্ক tvfsShmUnmap(sqlite3_file*, পূর্ণাঙ্ক);

স্থির পূর্ণাঙ্ক tvfsFetch(sqlite3_file*, sqlite3_int64, পূর্ণাঙ্ক, শূন্য**);
স্থির পূর্ণাঙ্ক tvfsUnfetch(sqlite3_file*, sqlite3_int64, শূন্য*);

স্থির sqlite3_io_methods tvfs_io_methods = {
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

স্থির পূর্ণাঙ্ক tvfsResultCode(Testvfs *p, পূর্ণাঙ্ক *pRc){
  সংরচনা errcode {
    পূর্ণাঙ্ক eCode;
    ধ্রুৱ ৱর্ণ *zCode;
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

  ধ্রুৱ ৱর্ণ *z;
  পূর্ণাঙ্ক i;

  z = Tcl_GetStringResult(p->interp);
  বাবে(i=0; i<ArraySize(aCode); i++){
    যদি( 0==strcmp(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      ঘূৰাই 1;
    }
  }

  ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক tvfsInjectFault(TestFaultInject *p){
  পূর্ণাঙ্ক ret = 0;
  যদি( p->eFault ){
    p->iCnt--;
    যদি( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  ঘূৰাই ret;
}


স্থির পূর্ণাঙ্ক tvfsInjectIoerr(Testvfs *p){
  ঘূৰাই tvfsInjectFault(&p->ioerr_err);
}

স্থির পূর্ণাঙ্ক tvfsInjectFullerr(Testvfs *p){
  ঘূৰাই tvfsInjectFault(&p->full_err);
}
স্থির পূর্ণাঙ্ক tvfsInjectCantopenerr(Testvfs *p){
  ঘূৰাই tvfsInjectFault(&p->cantopen_err);
}


স্থির শূন্য tvfsExecTcl(
  Testvfs *p, 
  ধ্রুৱ ৱর্ণ *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  পূর্ণাঙ্ক rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  প্রতিজ্ঞা( p->pScript );

  প্রতিজ্ঞা( zMethod );
  প্রতিজ্ঞা( p );
  প্রতিজ্ঞা( arg2==0 || arg1!=0 );
  প্রতিজ্ঞা( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_গুনতি(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  যদি( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  যদি( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  যদি( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  যদি( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  যদি( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_ResetResult(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
স্থির পূর্ণাঙ্ক tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  যদি( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  যদি( pFd->pShmId ){
    Tcl_Decr_Ref_গুনতি(pFd->pShmId);
    pFd->pShmId = 0;
  }
  যদি( pFile->pMethods ){
    ckfree((ৱর্ণ *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((ৱর্ণ *)pFd);
  pTestfile->pFd = 0;
  ঘূৰাই SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
স্থির পূর্ণাঙ্ক tvfsRead(
  sqlite3_file *pFile, 
  শূন্য *zBuf, 
  পূর্ণাঙ্ক iAmt, 
  sqlite_int64 iOfst
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  যদি( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  যদি( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  যদি( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  ঘূৰাই rc;
}

/*
** Write data to an tvfs-file.
*/
স্থির পূর্ণাঙ্ক tvfsWrite(
  sqlite3_file *pFile, 
  ধ্রুৱ শূন্য *zBuf, 
  পূর্ণাঙ্ক iAmt, 
  sqlite_int64 iOfst
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  যদি( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfsResultCode(p, &rc);
    যদি( rc<0 ) ঘূৰাই SQLITE_OK;
  }

  যদি( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  যদি( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  যদি( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  ঘূৰাই rc;
}

/*
** Truncate an tvfs-file.
*/
স্থির পূর্ণাঙ্ক tvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  যদি( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  
  যদি( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, size);
  }
  ঘূৰাই rc;
}

/*
** Sync an tvfs-file.
*/
স্থির পূর্ণাঙ্ক tvfsSync(sqlite3_file *pFile, পূর্ণাঙ্ক flags){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  যদি( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    ৱর্ণ *zFlags = 0;

    ৱিকল্প( flags ){
      পক্ষ SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        ভঙ্গ;
      পক্ষ SQLITE_SYNC_FULL:
        zFlags = "full";
        ভঙ্গ;
      পক্ষ SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        ভঙ্গ;
      পক্ষ SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        ভঙ্গ;
      স্ৱভাৱ:
        প্রতিজ্ঞা(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  যদি( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  যদি( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  ঘূৰাই rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
স্থির পূর্ণাঙ্ক tvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  TestvfsFd *p = tvfsGetFd(pFile);
  ঘূৰাই sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an tvfs-file.
*/
স্থির পূর্ণাঙ্ক tvfsLock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  যদি( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    ৱর্ণ zLock[30];
    sqlite3_snprintf(পরিমাণ(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  যদি( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    ঘূৰাই SQLITE_IOERR_LOCK;
  }
  ঘূৰাই sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
স্থির পূর্ণাঙ্ক tvfsUnlock(sqlite3_file *pFile, পূর্ণাঙ্ক eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  যদি( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    ৱর্ণ zLock[30];
    sqlite3_snprintf(পরিমাণ(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  যদি( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    ঘূৰাই SQLITE_IOERR_UNLOCK;
  }
  ঘূৰাই sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
স্থির পূর্ণাঙ্ক tvfsCheckReservedLock(sqlite3_file *pFile, পূর্ণাঙ্ক *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  যদি( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  ঘূৰাই sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
স্থির পূর্ণাঙ্ক tvfsFileControl(sqlite3_file *pFile, পূর্ণাঙ্ক op, শূন্য *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  যদি( op==SQLITE_FCNTL_PRAGMA ){
    ৱর্ণ **argv = (ৱর্ণ**)pArg;
    যদি( sqlite3_stricmp(argv[1],"error")==0 ){
      পূর্ণাঙ্ক rc = SQLITE_ERROR;
      যদি( argv[2] ){
        ধ্রুৱ ৱর্ণ *z = argv[2];
        পূর্ণাঙ্ক x = atoi(z);
        যদি( x ){
          rc = x;
          যেতিয়ালৈ( sqlite3Isdigit(z[0]) ){ z++; }
          যেতিয়ালৈ( sqlite3Isspace(z[0]) ){ z++; }
        }
        যদি( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      ঘূৰাই rc;
    }
    যদি( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      ঘূৰাই SQLITE_OK;
    }
  }
  যদি( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    সংরচনা Fcntl {
      পূর্ণাঙ্ক iFnctl;
      ধ্রুৱ ৱর্ণ *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    পূর্ণাঙ্ক i;
    বাবে(i=0; i<পরিমাণ(aF)/পরিমাণ(aF[0]); i++){
      যদি( op==aF[i].iFnctl ) ভঙ্গ;
    }
    যদি( i<পরিমাণ(aF)/পরিমাণ(aF[0]) ){
      পূর্ণাঙ্ক rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfsResultCode(p, &rc);
      যদি( rc ) ঘূৰাই (rc<0 ? SQLITE_OK : rc);
    }
  }
  ঘূৰাই sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
স্থির পূর্ণাঙ্ক tvfsSectorSize(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  যদি( p->iSectorsize>=0 ){
    ঘূৰাই p->iSectorsize;
  }
  ঘূৰাই sqlite3OsSectorSize(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
স্থির পূর্ণাঙ্ক tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  যদি( p->iDevchar>=0 ){
    ঘূৰাই p->iDevchar;
  }
  ঘূৰাই sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
স্থির পূর্ণাঙ্ক tvfsOpen(
  sqlite3_vfs *pVfs,
  ধ্রুৱ ৱর্ণ *zName,
  sqlite3_file *pFile,
  পূর্ণাঙ্ক flags,
  পূর্ণাঙ্ক *pOutFlags
){
  পূর্ণাঙ্ক rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  pFd = (TestvfsFd *)ckalloc(পরিমাণ(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, পরিমাণ(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, পরিমাণ(TestvfsFile));
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
  যদি( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_গুনতি(pArg);
    যদি( flags&SQLITE_OPEN_MAIN_DB ){
      ধ্রুৱ ৱর্ণ *z = &zName[strlen(zName)+1];
      যেতিয়ালৈ( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_গুনতি(pArg);
    যদি( tvfsResultCode(p, &rc) ){
      যদি( rc!=SQLITE_OK ) ঘূৰাই rc;
    }অন্যথা{
      pId = Tcl_GetObjResult(p->interp);
    }
  }

  যদি( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) ঘূৰাই SQLITE_IOERR;
  যদি( tvfsInjectCantopenerr(p) ) ঘূৰাই SQLITE_CANTOPEN;
  যদি( tvfsInjectFullerr(p) ) ঘূৰাই SQLITE_FULL;

  যদি( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_গুনতি(pId);
  pFd->pShmId = pId;
  Tcl_ResetResult(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  যদি( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    পূর্ণাঙ্ক nByte;

    যদি( pVfs->iVersion>1 ){
      nByte = পরিমাণ(sqlite3_io_methods);
    }অন্যথা{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    memcpy(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    যদি( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    যদি( pVfs->iVersion>1 && ((Testvfs *)pVfs->pAppData)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  ঘূৰাই rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
স্থির পূর্ণাঙ্ক tvfsDelete(sqlite3_vfs *pVfs, ধ্রুৱ ৱর্ণ *zPath, পূর্ণাঙ্ক dirSync){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  যদি( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  যদি( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  ঘূৰাই rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
স্থির পূর্ণাঙ্ক tvfsAccess(
  sqlite3_vfs *pVfs, 
  ধ্রুৱ ৱর্ণ *zPath, 
  পূর্ণাঙ্ক flags, 
  পূর্ণাঙ্ক *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  যদি( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    পূর্ণাঙ্ক rc;
    ৱর্ণ *zArg = 0;
    যদি( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    যদি( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    যদি( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    যদি( tvfsResultCode(p, &rc) ){
      যদি( rc!=SQLITE_OK ) ঘূৰাই rc;
    }অন্যথা{
      Tcl_Interp *interp = p->interp;
      যদি( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_GetObjResult(interp), pResOut) ){
        ঘূৰাই SQLITE_OK;
      }
    }
  }
  ঘূৰাই sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
স্থির পূর্ণাঙ্ক tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  ধ্রুৱ ৱর্ণ *zPath, 
  পূর্ণাঙ্ক nOut, 
  ৱর্ণ *zOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  যদি( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    পূর্ণাঙ্ক rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    যদি( tvfsResultCode(p, &rc) ){
      যদি( rc!=SQLITE_OK ) ঘূৰাই rc;
    }
  }
  ঘূৰাই sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
স্থির শূন্য *tvfsDlOpen(sqlite3_vfs *pVfs, ধ্রুৱ ৱর্ণ *zPath){
  ঘূৰাই sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
স্থির শূন্য tvfsDlError(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, ৱর্ণ *z_Err_বাৰ্তা){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_বাৰ্তা);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
স্থির শূন্য (*tvfsDlSym(sqlite3_vfs *pVfs, শূন্য *p, ধ্রুৱ ৱর্ণ *zSym))(শূন্য){
  ঘূৰাই sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
স্থির শূন্য tvfsDlClose(sqlite3_vfs *pVfs, শূন্য *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
স্থির পূর্ণাঙ্ক tvfsRandomness(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nByte, ৱর্ণ *zBufOut){
  ঘূৰাই sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
স্থির পূর্ণাঙ্ক tvfsSleep(sqlite3_vfs *pVfs, পূর্ণাঙ্ক nMicro){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  যদি( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  ঘূৰাই sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
স্থির পূর্ণাঙ্ক tvfsCurrentTime(sqlite3_vfs *pVfs, দ্ৱিগুণ *pTimeOut){
  ঘূৰাই PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

স্থির পূর্ণাঙ্ক tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  পূর্ণাঙ্ক rc = SQLITE_OK;             /* Return code */
  TestvfsBuffer *pBuffer;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->pAppData;
  প্রতিজ্ঞা( 0==p->isFullshm );
  প্রতিজ্ঞা( pFd->pShmId && pFd->pShm==0 && pFd->pNext==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_ResetResult(p->interp);
  যদি( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    যদি( tvfsResultCode(p, &rc) ){
      যদি( rc!=SQLITE_OK ) ঘূৰাই rc;
    }
  }

  প্রতিজ্ঞা( rc==SQLITE_OK );
  যদি( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    ঘূৰাই SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  বাবে(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
    যদি( 0==strcmp(pFd->zFilename, pBuffer->zFile) ) ভঙ্গ;
  }
  যদি( !pBuffer ){
    পূর্ণাঙ্ক szName = (পূর্ণাঙ্ক)strlen(pFd->zFilename);
    পূর্ণাঙ্ক nByte = পরিমাণ(TestvfsBuffer) + szName + 1;
    pBuffer = (TestvfsBuffer *)ckalloc(nByte);
    memset(pBuffer, 0, nByte);
    pBuffer->zFile = (ৱর্ণ *)&pBuffer[1];
    memcpy(pBuffer->zFile, pFd->zFilename, szName+1);
    pBuffer->pNext = p->pBuffer;
    p->pBuffer = pBuffer;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->pNext = pBuffer->pFile;
  pBuffer->pFile = pFd;
  pFd->pShm = pBuffer;
  ঘূৰাই rc;
}

স্থির শূন্য tvfsAllocPage(TestvfsBuffer *p, পূর্ণাঙ্ক iPage, পূর্ণাঙ্ক pgsz){
  প্রতিজ্ঞা( iPage<TESTVFS_MAX_PAGES );
  যদি( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    memset(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

স্থির পূর্ণাঙ্ক tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  পূর্ণাঙ্ক iPage,                      /* Page to retrieve */
  পূর্ণাঙ্ক pgsz,                       /* Size of pages */
  পূর্ণাঙ্ক isWrite,                    /* True to extend file if necessary */
  শূন্য চঞ্চল **pp              /* OUT: Mapped memory */
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  যদি( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ঘূৰাই pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  যদি( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    যদি( rc!=SQLITE_OK ){
      ঘূৰাই rc;
    }
  }

  যদি( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_গুনতি(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(iPage));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfsResultCode(p, &rc);
    Tcl_Decr_Ref_গুনতি(pArg);
  }
  যদি( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  যদি( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  যদি( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (শূন্য চঞ্চল *)pFd->pShm->aPage[iPage];
  }

  ঘূৰাই rc;
}


স্থির পূর্ণাঙ্ক tvfsShmLock(
  sqlite3_file *pFile,
  পূর্ণাঙ্ক ofst,
  পূর্ণাঙ্ক n,
  পূর্ণাঙ্ক flags
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  পূর্ণাঙ্ক nLock;
  ৱর্ণ zLock[80];

  যদি( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ঘূৰাই pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  যদি( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(পরিমাণ(zLock), zLock, "%d %d", ofst, n);
    nLock = (পূর্ণাঙ্ক)strlen(zLock);
    যদি( flags & SQLITE_SHM_LOCK ){
      strcpy(&zLock[nLock], " lock");
    }অন্যথা{
      strcpy(&zLock[nLock], " unlock");
    }
    nLock += (পূর্ণাঙ্ক)strlen(&zLock[nLock]);
    যদি( flags & SQLITE_SHM_SHARED ){
      strcpy(&zLock[nLock], " shared");
    }অন্যথা{
      strcpy(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  যদি( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  যদি( rc==SQLITE_OK ){
    পূর্ণাঙ্ক isLock = (flags & SQLITE_SHM_LOCK);
    পূর্ণাঙ্ক isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    যদি( isLock ){
      TestvfsFd *p2;
      বাবে(p2=pFd->pShm->pFile; p2; p2=p2->pNext){
        যদি( p2==pFd ) অনুৱর্ত;
        যদি( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          ভঙ্গ;
        }
      }
      যদি( rc==SQLITE_OK ){
        যদি( isExcl )  pFd->excllock |= mask;
        যদি( !isExcl ) pFd->sharedlock |= mask;
      }
    }অন্যথা{
      যদি( isExcl )  pFd->excllock &= (~mask);
      যদি( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  ঘূৰাই rc;
}

স্থির শূন্য tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  যদি( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    ধ্রুৱ ৱর্ণ *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  যদি( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    ঘূৰাই;
  }
}

স্থির পূর্ণাঙ্ক tvfsShmUnmap(
  sqlite3_file *pFile,
  পূর্ণাঙ্ক deleteFlag
){
  পূর্ণাঙ্ক rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  TestvfsBuffer *pBuffer = pFd->pShm;
  TestvfsFd **ppFd;

  যদি( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ঘূৰাই pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  যদি( !pBuffer ) ঘূৰাই SQLITE_OK;
  প্রতিজ্ঞা( pFd->pShmId && pFd->pShm );

  যদি( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }

  বাবে(ppFd=&pBuffer->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->pNext));
  প্রতিজ্ঞা( (*ppFd)==pFd );
  *ppFd = pFd->pNext;
  pFd->pNext = 0;

  যদি( pBuffer->pFile==0 ){
    পূর্ণাঙ্ক i;
    TestvfsBuffer **pp;
    বাবে(pp=&p->pBuffer; *pp!=pBuffer; pp=&((*pp)->pNext));
    *pp = (*pp)->pNext;
    বাবে(i=0; pBuffer->aPage[i]; i++){
      ckfree((ৱর্ণ *)pBuffer->aPage[i]);
    }
    ckfree((ৱর্ণ *)pBuffer);
  }
  pFd->pShm = 0;

  ঘূৰাই rc;
}

স্থির পূর্ণাঙ্ক tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    পূর্ণাঙ্ক iAmt, 
    শূন্য **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  ঘূৰাই sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

স্থির পূর্ণাঙ্ক tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, শূন্য *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  ঘূৰাই sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI testvfs_obj_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  গণনা DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  সংরচনা TestvfsSubcmd {
    ৱর্ণ *zName;
    গণনা DB_enum eCmd;
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
  পূর্ণাঙ্ক i;
  
  যদি( objc<2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SUBCOMMAND ...");
    ঘূৰাই TCL_ERROR;
  }
  যদি( Tcl_GetIndexFromObjStruct(
        interp, objv[1], aSubcmd, পরিমাণ(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    ঘূৰাই TCL_ERROR;
  }
  Tcl_ResetResult(interp);

  ৱিকল্প( aSubcmd[i].eCmd ){
    পক্ষ CMD_SHM: {
      Tcl_Obj *pObj;
      পূর্ণাঙ্ক rc;
      TestvfsBuffer *pBuffer;
      ৱর্ণ *zName;
      যদি( objc!=3 && objc!=4 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "FILE ?VALUE?");
        ঘূৰাই TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      যদি( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), রিক্ত);
        ckfree(zName);
        ঘূৰাই TCL_ERROR;
      }
      বাবে(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
        যদি( 0==strcmp(pBuffer->zFile, zName) ) ভঙ্গ;
      }
      ckfree(zName);
      যদি( !pBuffer ){
        Tcl_AppendResult(interp, "no such file: ", Tcl_GetString(objv[2]), রিক্ত);
        ঘূৰাই TCL_ERROR;
      }
      যদি( objc==4 ){
        Tcl_Size n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        পূর্ণাঙ্ক pgsz = pBuffer->pgsz;
        যদি( pgsz==0 ) pgsz = 65536;
        বাবে(i=0; i*pgsz<(পূর্ণাঙ্ক)n; i++){
          পূর্ণাঙ্ক nByte = pgsz;
          tvfsAllocPage(pBuffer, i, pgsz);
          যদি( n-i*pgsz<pgsz ){
            nByte = (পূর্ণাঙ্ক)n;
          }
          memcpy(pBuffer->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      বাবে(i=0; pBuffer->aPage[i]; i++){
        পূর্ণাঙ্ক pgsz = pBuffer->pgsz;
        যদি( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(pBuffer->aPage[i], pgsz));
      }
      Tcl_SetObjResult(interp, pObj);
      ভঙ্গ;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    পক্ষ CMD_FILTER: {
      স্থির সংরচনা VfsMethod {
        ৱর্ণ *zName;
        পূর্ণাঙ্ক mask;
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
      পূর্ণাঙ্ক mask = 0;
      যদি( objc!=3 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "LIST");
        ঘূৰাই TCL_ERROR;
      }
      যদি( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        ঘূৰাই TCL_ERROR;
      }
      Tcl_ResetResult(interp);
      বাবে(i=0; i<(পূর্ণাঙ্ক)nElem; i++){
        পূর্ণাঙ্ক iMethod;
        ৱর্ণ *zElem = Tcl_GetString(apElem[i]);
        বাবে(iMethod=0; iMethod<ArraySize(vfsmethod); iMethod++){
          যদি( strcmp(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            ভঙ্গ;
          }
        }
        যদি( iMethod==ArraySize(vfsmethod) ){
          Tcl_AppendResult(interp, "unknown method: ", zElem, রিক্ত);
          ঘূৰাই TCL_ERROR;
        }
      }
      p->mask = mask;
      ভঙ্গ;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    পক্ষ CMD_SCRIPT: {
      যদি( objc==3 ){
        Tcl_Size nByte;
        যদি( p->pScript ){
          Tcl_Decr_Ref_গুনতি(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        যদি( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_গুনতি(p->pScript);
        }
      }অন্যথা যদি( objc!=2 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?SCRIPT?");
        ঘূৰাই TCL_ERROR;
      }

      Tcl_ResetResult(interp);
      যদি( p->pScript ) Tcl_SetObjResult(interp, p->pScript);

      ভঙ্গ;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    পক্ষ CMD_CANTOPENERR:
    পক্ষ CMD_IOERR:
    পক্ষ CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      পূর্ণাঙ্ক iRet;

      ৱিকল্প( aSubcmd[i].eCmd ){
        পক্ষ CMD_IOERR: pTest = &p->ioerr_err; ভঙ্গ;
        পক্ষ CMD_FULLERR: pTest = &p->full_err; ভঙ্গ;
        পক্ষ CMD_CANTOPENERR: pTest = &p->cantopen_err; ভঙ্গ;
        স্ৱভাৱ: প্রতিজ্ঞা(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      যদি( objc==4 ){
        পূর্ণাঙ্ক iCnt, iPersist;
        যদি( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          ঘূৰাই TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }অন্যথা যদি( objc!=2 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?CNT PERSIST?");
        ঘূৰাই TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(iRet));
      ভঙ্গ;
    }

    পক্ষ CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      ভঙ্গ;
    }

    পক্ষ CMD_DEVCHAR: {
      সংরচনা DeviceFlag {
        ৱর্ণ *zName;
        পূর্ণাঙ্ক i_মান;
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
      পূর্ণাঙ্ক iFlag;

      যদি( objc>3 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?ATTR-LIST?");
        ঘূৰাই TCL_ERROR;
      }
      যদি( objc==3 ){
        পূর্ণাঙ্ক j;
        পূর্ণাঙ্ক iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_Size nFlags = 0;

        যদি( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          ঘূৰাই TCL_ERROR;
        }

        বাবে(j=0; j<(পূর্ণাঙ্ক)nFlags; j++){
          পূর্ণাঙ্ক idx = 0;
          যদি( Tcl_GetIndexFromObjStruct(interp, flags[j], aFlag, 
                পরিমাণ(aFlag[0]), "flag", 0, &idx) 
          ){
            ঘূৰাই TCL_ERROR;
          }
          যদি( aFlag[idx].i_মান<0 && nFlags>1 ){
            Tcl_AppendResult(interp, "bad flags: ", Tcl_GetString(objv[2]), রিক্ত);
            ঘূৰাই TCL_ERROR;
          }
          iNew |= aFlag[idx].i_মান;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      বাবে(iFlag=0; iFlag<পরিমাণ(aFlag)/পরিমাণ(aFlag[0]); iFlag++){
        যদি( p->iDevchar & aFlag[iFlag].i_মান ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_SetObjResult(interp, pRet);

      ভঙ্গ;
    }

    পক্ষ CMD_SECTORSIZE: {
      যদি( objc>3 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?VALUE?");
        ঘূৰাই TCL_ERROR;
      }
      যদি( objc==3 ){
        পূর্ণাঙ্ক iNew = 0;
        যদি( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          ঘূৰাই TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(p->iSectorsize));
      ভঙ্গ;
    }
  }

  ঘূৰাই TCL_OK;
}

স্থির শূন্য SQLITE_TCLAPI testvfs_obj_del(ClientData cd){
  Testvfs *p = (Testvfs *)cd;
  যদি( p->pScript ) Tcl_Decr_Ref_গুনতি(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, পরিমাণ(sqlite3_vfs));
  ckfree((ৱর্ণ *)p->pVfs);
  memset(p, 0, পরিমাণ(Testvfs));
  ckfree((ৱর্ণ *)p);
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
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI testvfs_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  স্থির sqlite3_vfs tvfs_vfs = {
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
#অন্যথা
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
  ৱর্ণ *zVfs;
  পূর্ণাঙ্ক nByte;                      /* Bytes of space to allocate at p */

  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক isNoshm = 0;                /* True if -noshm is passed */
  পূর্ণাঙ্ক isFullshm = 0;              /* True if -fullshm is passed */
  পূর্ণাঙ্ক isDefault = 0;              /* True if -default is passed */
  পূর্ণাঙ্ক szOsFile = 0;               /* Value passed to -szosfile */
  পূর্ণাঙ্ক mxPathname = -1;            /* Value passed to -mxpathname */
  পূর্ণাঙ্ক iVersion = 3;               /* Value passed to -iversion */

  যদি( objc<2 || 0!=(objc%2) ) গচ্ছ bad_args;
  বাবে(i=2; i<objc; i += 2){
    Tcl_Size nSwitch;
    ৱর্ণ *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    যদি( nSwitch>2 && 0==strncmp("-noshm", zSwitch, nSwitch) ){
      যদি( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        ঘূৰাই TCL_ERROR;
      }
      যদি( isNoshm ) isFullshm = 0;
    }
    অন্যথা যদি( nSwitch>2 && 0==strncmp("-default", zSwitch, nSwitch) ){
      যদি( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        ঘূৰাই TCL_ERROR;
      }
    }
    অন্যথা যদি( nSwitch>2 && 0==strncmp("-szosfile", zSwitch, nSwitch) ){
      যদি( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        ঘূৰাই TCL_ERROR;
      }
    }
    অন্যথা যদি( nSwitch>2 && 0==strncmp("-mxpathname", zSwitch, nSwitch) ){
      যদি( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        ঘূৰাই TCL_ERROR;
      }
    }
    অন্যথা যদি( nSwitch>2 && 0==strncmp("-iversion", zSwitch, nSwitch) ){
      যদি( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        ঘূৰাই TCL_ERROR;
      }
    }
    অন্যথা যদি( nSwitch>2 && 0==strncmp("-fullshm", zSwitch, nSwitch) ){
      যদি( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        ঘূৰাই TCL_ERROR;
      }
      যদি( isFullshm ) isNoshm = 0;
    }
    অন্যথা{
      গচ্ছ bad_args;
    }
  }

  যদি( szOsFile<পরিমাণ(TestvfsFile) ){
    szOsFile = পরিমাণ(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = পরিমাণ(Testvfs) + (পূর্ণাঙ্ক)strlen(zVfs)+1;
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

  p->zName = (ৱর্ণ *)&p[1];
  memcpy(p->zName, zVfs, strlen(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(পরিমাণ(sqlite3_vfs));
  memcpy(pVfs, &tvfs_vfs, পরিমাণ(sqlite3_vfs));
  pVfs->pAppData = (শূন্য *)p;
  pVfs->iVersion = iVersion;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  যদি( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  ঘূৰাই TCL_OK;

 bad_args:
  Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  ঘূৰাই TCL_ERROR;
}

বাহ্য পূর্ণাঙ্ক getDbPointer(Tcl_Interp *interp, ধ্রুৱ ৱর্ণ *zA, sqlite3 **ppDb);
বাহ্য ধ্রুৱ ৱর্ণ *sqlite3ErrName(পূর্ণাঙ্ক);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_vfs_shmlock(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  ধ্রুৱ ৱর্ণ *azArg1[] = {"shared", "exclusive", 0};
  ধ্রুৱ ৱর্ণ *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  ধ্রুৱ ৱর্ণ *zDbname = 0;
  পূর্ণাঙ্ক iArg1 = 0;
  পূর্ণাঙ্ক iArg2 = 0;
  পূর্ণাঙ্ক iOffset = 0;
  পূর্ণাঙ্ক n = 0;
  sqlite3_file *pFd;

  যদি( objc!=7 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    ঘূৰাই TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIndexFromObj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_GetIndexFromObj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    ঘূৰাই TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (শূন্য*)&pFd);
  যদি( pFd==0 ){
    ঘূৰাই TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_vfs_set_readmark(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  পূর্ণাঙ্ক rc = SQLITE_OK;
  ধ্রুৱ ৱর্ণ *zDbname = 0;
  পূর্ণাঙ্ক iSlot = 0;
  পূর্ণাঙ্ক iVal = -1;
  sqlite3_file *pFd;
  শূন্য চঞ্চল *pShm = 0;
  u32 *aShm;
  পূর্ণাঙ্ক iOff;

  যদি( objc!=4 && objc!=5 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    ঘূৰাই TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    ঘূৰাই TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (শূন্য*)&pFd);
  যদি( pFd==0 ){
    ঘূৰাই TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  যদি( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ঘূৰাই TCL_ERROR;
  }
  যদি( pShm==0 ){
    Tcl_AppendResult(interp, "*-shm is not yet mapped", রিক্ত);
    ঘূৰাই TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  যদি( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(aShm[iOff]));

  ঘূৰাই TCL_OK;
}

পূর্ণাঙ্ক Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  ঘূৰাই TCL_OK;
}

#endif
