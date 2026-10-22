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
#ਜੇ SQLITE_TEST          /* This file is used for testing only */

#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Testvfs Testvfs;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ TestvfsShm TestvfsShm;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Testvfs_ਬਫਰ Testvfs_ਬਫਰ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ TestvfsFile TestvfsFile;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
ਬਣਤਰ TestvfsFile {
  sqlite3_file ਅਧਾਰ;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

ਬਣਤਰ TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  ਅਚਲ ਅੱਖਰ *zFilename;          /* Filename as passed to xOpen() */
  sqlite3_file *pReal;            /* The real, underlying file descriptor */
  Tcl_Obj *pShmId;                /* Shared memory id for Tcl callbacks */

  Testvfs_ਬਫਰ *pShm;            /* Shared memory buffer */
  u32 excllock;                   /* Mask of exclusive locks */
  u32 sharedlock;                 /* Mask of shared locks */
  TestvfsFd *p_ਅਗਲਾ;               /* Next handle opened on the same file */
};


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FAULT_INJECT_NONE       0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FAULT_INJECT_TRANSIENT  1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FAULT_INJECT_PERSISTENT 2

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ TestFaultInject TestFaultInject;
ਬਣਤਰ TestFaultInject {
  ਪੂਰਨ iCnt;                       /* Remaining calls before fault injection */
  ਪੂਰਨ eFault;                     /* A FAULT_INJECT_* value */
  ਪੂਰਨ nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
ਬਣਤਰ Testvfs {
  ਅੱਖਰ *zName;                    /* Name of this VFS */
  sqlite3_vfs *p_ਮਾਪਾ;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  Testvfs_ਬਫਰ *p_ਬਫਰ;         /* List of shared buffers */
  ਪੂਰਨ isNoshm;
  ਪੂਰਨ isFullshm;

  ਪੂਰਨ ਨਕਾਬ;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject ਭਰਿਆ_err;
  TestFaultInject cantopen_err;

#ਜੇ 0
  ਪੂਰਨ iIoerrCnt;
  ਪੂਰਨ ioerr;
  ਪੂਰਨ nIoerrFail;
  ਪੂਰਨ i_ਭਰਿਆ_Cnt;
  ਪੂਰਨ fullerr;
  ਪੂਰਨ n_ਭਰਿਆ_Fail;
#ਜੇ_ਅੰਤ

  ਪੂਰਨ iDevchar;
  ਪੂਰਨ iSectorsize;
};

/*
** The Testvfs.mask variable is set to a combination of the following.
** If a bit is clear in Testvfs.mask, then calls made by SQLite to the 
** corresponding VFS method is ignored for purposes of:
**
**   + Simulating IO errors, and
**   + Invoking the Tcl callback script.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_SHMOPEN_ਨਕਾਬ      0x00000001
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_SHMLOCK_ਨਕਾਬ      0x00000010
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_SHMMAP_ਨਕਾਬ       0x00000020
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_SHMBARRIER_ਨਕਾਬ   0x00000040
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_SHMCLOSE_ਨਕਾਬ     0x00000080

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_OPEN_ਨਕਾਬ         0x00000100
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_SYNC_ਨਕਾਬ         0x00000200
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_DELETE_ਨਕਾਬ       0x00000400
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_CLOSE_ਨਕਾਬ        0x00000800
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_WRITE_ਨਕਾਬ        0x00001000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_TRUNCATE_ਨਕਾਬ     0x00002000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_ACCESS_ਨਕਾਬ       0x00004000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_FULLPATHNAME_ਨਕਾਬ 0x00008000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_READ_ਨਕਾਬ         0x00010000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_UNLOCK_ਨਕਾਬ       0x00020000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_ਤਾਲਾ_ਨਕਾਬ         0x00040000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_CKLOCK_ਨਕਾਬ       0x00080000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_FCNTL_ਨਕਾਬ        0x00100000
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_SLEEP_ਨਕਾਬ        0x00200000

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_ALL_ਨਕਾਬ          0x003FFFFF


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_MAX_ਪੰਨੇ 1024

/*
** A shared-memory buffer. There is one of these objects for each shared
** memory region opened by clients. If two clients open the same file,
** there are two TestvfsFile structures but only one TestvfsBuffer structure.
*/
ਬਣਤਰ Testvfs_ਬਫਰ {
  ਅੱਖਰ *zFile;                    /* Associated file name */
  ਪੂਰਨ pgsz;                       /* Page size */
  u8 *a_ਪੰਨਾ[TESTVFS_MAX_ਪੰਨੇ];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  Testvfs_ਬਫਰ *p_ਅਗਲਾ;           /* Next in linked list of all buffers */
};


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PARENTVFS(x) (((Testvfs *)((x)->p_App_ਅੰਕੜੇ))->p_ਮਾਪਾ)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ TESTVFS_MAX_ਦਲੀਲਾਂ 12


/*
** Method declarations for TestvfsFile.
*/
ਸਥਿਰ ਪੂਰਨ tvfsClose(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ tvfsRead(sqlite3_file*, ਖਾਲੀ*, ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ tvfsWrite(sqlite3_file*,ਅਚਲ ਖਾਲੀ*,ਪੂਰਨ iAmt, sqlite3_int64 iOfst);
ਸਥਿਰ ਪੂਰਨ tvfsTruncate(sqlite3_file*, sqlite3_int64 ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ tvfsSync(sqlite3_file*, ਪੂਰਨ flags);
ਸਥਿਰ ਪੂਰਨ tvfs_File_ਮਾਪ(sqlite3_file*, sqlite3_int64 *p_ਮਾਪ);
ਸਥਿਰ ਪੂਰਨ tvfs_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ tvfsUnlock(sqlite3_file*, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ tvfs_Check_Reserved_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ tvfsFileControl(sqlite3_file*, ਪੂਰਨ op, ਖਾਲੀ *pArg);
ਸਥਿਰ ਪੂਰਨ tvfs_Sector_ਮਾਪ(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
ਸਥਿਰ ਪੂਰਨ tvfsOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *, sqlite3_file*, ਪੂਰਨ , ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ tvfsDelete(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ syncDir);
ਸਥਿਰ ਪੂਰਨ tvfsAccess(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ flags, ਪੂਰਨ *);
ਸਥਿਰ ਪੂਰਨ tvfs_ਭਰਿਆ_Pathname(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zName, ਪੂਰਨ, ਅੱਖਰ *z_ਬਾਹਰਲਾ);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
ਸਥਿਰ ਖਾਲੀ *tvfsDlOpen(sqlite3_vfs*, ਅਚਲ ਅੱਖਰ *zFilename);
ਸਥਿਰ ਖਾਲੀ tvfsDlError(sqlite3_vfs*, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ);
ਸਥਿਰ ਖਾਲੀ (*tvfsDlSym(sqlite3_vfs*,ਖਾਲੀ*, ਅਚਲ ਅੱਖਰ *z_ਚਿੰਨ੍ਹ))(ਖਾਲੀ);
ਸਥਿਰ ਖਾਲੀ tvfsDlClose(sqlite3_vfs*, ਖਾਲੀ*);
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_LOAD_EXTENSION */
ਸਥਿਰ ਪੂਰਨ tvfsRandomness(sqlite3_vfs*, ਪੂਰਨ nByte, ਅੱਖਰ *z_ਬਾਹਰਲਾ);
ਸਥਿਰ ਪੂਰਨ tvfsSleep(sqlite3_vfs*, ਪੂਰਨ microseconds);
ਸਥਿਰ ਪੂਰਨ tvfs_Current_ਸਮਾਂ(sqlite3_vfs*, ਦੁੱਗਣਾ*);

ਸਥਿਰ ਪੂਰਨ tvfsShmOpen(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ tvfs_Shm_ਤਾਲਾ(sqlite3_file*, ਪੂਰਨ , ਪੂਰਨ, ਪੂਰਨ);
ਸਥਿਰ ਪੂਰਨ tvfsShmMap(sqlite3_file*,ਪੂਰਨ,ਪੂਰਨ,ਪੂਰਨ, ਖਾਲੀ ਅਸਥਿਰ **);
ਸਥਿਰ ਖਾਲੀ tvfsShmBarrier(sqlite3_file*);
ਸਥਿਰ ਪੂਰਨ tvfsShmUnmap(sqlite3_file*, ਪੂਰਨ);

ਸਥਿਰ ਪੂਰਨ tvfsFetch(sqlite3_file*, sqlite3_int64, ਪੂਰਨ, ਖਾਲੀ**);
ਸਥਿਰ ਪੂਰਨ tvfsUnfetch(sqlite3_file*, sqlite3_int64, ਖਾਲੀ*);

ਸਥਿਰ sqlite3_io_methods tvfs_io_methods = {
  3,                              /* iVersion */
  tvfsClose,                      /* xClose */
  tvfsRead,                       /* xRead */
  tvfsWrite,                      /* xWrite */
  tvfsTruncate,                   /* xTruncate */
  tvfsSync,                       /* xSync */
  tvfs_File_ਮਾਪ,                   /* xFileSize */
  tvfs_ਤਾਲਾ,                       /* xLock */
  tvfsUnlock,                     /* xUnlock */
  tvfs_Check_Reserved_ਤਾਲਾ,          /* xCheckReservedLock */
  tvfsFileControl,                /* xFileControl */
  tvfs_Sector_ਮਾਪ,                 /* xSectorSize */
  tvfsDeviceCharacteristics,      /* xDeviceCharacteristics */
  tvfsShmMap,                     /* xShmMap */
  tvfs_Shm_ਤਾਲਾ,                    /* xShmLock */
  tvfsShmBarrier,                 /* xShmBarrier */
  tvfsShmUnmap,                   /* xShmUnmap */
  tvfsFetch,
  tvfsUnfetch
};

ਸਥਿਰ ਪੂਰਨ tvfs_ਨਤੀਜਾ_ਕੋਡ(Testvfs *p, ਪੂਰਨ *pRc){
  ਬਣਤਰ errcode {
    ਪੂਰਨ e_ਕੋਡ;
    ਅਚਲ ਅੱਖਰ *z_ਕੋਡ;
  } a_ਕੋਡ[] = {
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

  ਅਚਲ ਅੱਖਰ *z;
  ਪੂਰਨ i;

  z = Tcl_Get_String_ਨਤੀਜਾ(p->interp);
  ਲਈ(i=0; i<Array_ਮਾਪ(a_ਕੋਡ); i++){
    ਜੇ( 0==strcmp(z, a_ਕੋਡ[i].z_ਕੋਡ) ){
      *pRc = a_ਕੋਡ[i].e_ਕੋਡ;
      ਵਾਪਸ 1;
    }
  }

  ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ tvfsInjectFault(TestFaultInject *p){
  ਪੂਰਨ ret = 0;
  ਜੇ( p->eFault ){
    p->iCnt--;
    ਜੇ( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  ਵਾਪਸ ret;
}


ਸਥਿਰ ਪੂਰਨ tvfsInjectIoerr(Testvfs *p){
  ਵਾਪਸ tvfsInjectFault(&p->ioerr_err);
}

ਸਥਿਰ ਪੂਰਨ tvfsInjectFullerr(Testvfs *p){
  ਵਾਪਸ tvfsInjectFault(&p->ਭਰਿਆ_err);
}
ਸਥਿਰ ਪੂਰਨ tvfsInjectCantopenerr(Testvfs *p){
  ਵਾਪਸ tvfsInjectFault(&p->cantopen_err);
}


ਸਥਿਰ ਖਾਲੀ tvfsExecTcl(
  Testvfs *p, 
  ਅਚਲ ਅੱਖਰ *z_ਵਿਧੀ,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  ਪੂਰਨ rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pScript );

  ਪ੍ਰਤਿਜ੍ਞਾ( z_ਵਿਧੀ );
  ਪ੍ਰਤਿਜ੍ਞਾ( p );
  ਪ੍ਰਤਿਜ੍ਞਾ( arg2==0 || arg1!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ਗਿਣਤੀ(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(z_ਵਿਧੀ, -1));
  ਜੇ( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  ਜੇ( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  ਜੇ( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  ਜੇ( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  ਜੇ( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_Reset_ਨਤੀਜਾ(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ਅੰਕੜੇ;

  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_CLOSE_ਨਕਾਬ ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  ਜੇ( pFd->pShmId ){
    Tcl_Decr_Ref_ਗਿਣਤੀ(pFd->pShmId);
    pFd->pShmId = 0;
  }
  ਜੇ( pFile->pMethods ){
    ckfree((ਅੱਖਰ *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((ਅੱਖਰ *)pFd);
  pTestfile->pFd = 0;
  ਵਾਪਸ SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ tvfsRead(
  sqlite3_file *pFile, 
  ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  ਪੂਰਨ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ਅੰਕੜੇ;
  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_READ_ਨਕਾਬ ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_ਨਤੀਜਾ_ਕੋਡ(p, &rc);
  }
  ਜੇ( rc==SQLITE_OK && p->ਨਕਾਬ&TESTVFS_READ_ਨਕਾਬ && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  ਵਾਪਸ rc;
}

/*
** Write data to an tvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ tvfsWrite(
  sqlite3_file *pFile, 
  ਅਚਲ ਖਾਲੀ *zBuf, 
  ਪੂਰਨ iAmt, 
  sqlite_int64 iOfst
){
  ਪੂਰਨ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ਅੰਕੜੇ;

  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_WRITE_ਨਕਾਬ ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfs_ਨਤੀਜਾ_ਕੋਡ(p, &rc);
    ਜੇ( rc<0 ) ਵਾਪਸ SQLITE_OK;
  }

  ਜੇ( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_ਭਰਿਆ;
  }
  ਜੇ( rc==SQLITE_OK && p->ਨਕਾਬ&TESTVFS_WRITE_ਨਕਾਬ && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  ਵਾਪਸ rc;
}

/*
** Truncate an tvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ tvfsTruncate(sqlite3_file *pFile, sqlite_int64 ਮਾਪ){
  ਪੂਰਨ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ਅੰਕੜੇ;

  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_TRUNCATE_ਨਕਾਬ ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_ਨਤੀਜਾ_ਕੋਡ(p, &rc);
  }
  
  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, ਮਾਪ);
  }
  ਵਾਪਸ rc;
}

/*
** Sync an tvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ tvfsSync(sqlite3_file *pFile, ਪੂਰਨ flags){
  ਪੂਰਨ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ਅੰਕੜੇ;

  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_SYNC_ਨਕਾਬ ){
    ਅੱਖਰ *zFlags = 0;

    ਚੋਣ( flags ){
      ਹਾਲਤ SQLITE_SYNC_ਸਾਧਾਰਨ:
        zFlags = "normal";
        ਰੋਕੋ;
      ਹਾਲਤ SQLITE_SYNC_ਭਰਿਆ:
        zFlags = "full";
        ਰੋਕੋ;
      ਹਾਲਤ SQLITE_SYNC_ਸਾਧਾਰਨ|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        ਰੋਕੋ;
      ਹਾਲਤ SQLITE_SYNC_ਭਰਿਆ|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        ਰੋਕੋ;
      ਮੂਲ:
        ਪ੍ਰਤਿਜ੍ਞਾ(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfs_ਨਤੀਜਾ_ਕੋਡ(p, &rc);
  }

  ਜੇ( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_ਭਰਿਆ;

  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  ਵਾਪਸ rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ tvfs_File_ਮਾਪ(sqlite3_file *pFile, sqlite_int64 *p_ਮਾਪ){
  TestvfsFd *p = tvfsGetFd(pFile);
  ਵਾਪਸ sqlite3_Os_File_ਮਾਪ(p->pReal, p_ਮਾਪ);
}

/*
** Lock an tvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ tvfs_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ਅੰਕੜੇ;
  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_ਤਾਲਾ_ਨਕਾਬ ){
    ਅੱਖਰ z_ਤਾਲਾ[30];
    sqlite3_snprintf(ਆਕਾਰ(z_ਤਾਲਾ),z_ਤਾਲਾ,"%d",e_ਤਾਲਾ);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(z_ਤਾਲਾ, -1), 0, 0);
  }
  ਜੇ( p->ਨਕਾਬ&TESTVFS_ਤਾਲਾ_ਨਕਾਬ && tvfsInjectIoerr(p) ){
    ਵਾਪਸ SQLITE_IOERR_ਤਾਲਾ;
  }
  ਵਾਪਸ sqlite3_Os_ਤਾਲਾ(pFd->pReal, e_ਤਾਲਾ);
}

/*
** Unlock an tvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ tvfsUnlock(sqlite3_file *pFile, ਪੂਰਨ e_ਤਾਲਾ){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ਅੰਕੜੇ;
  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_UNLOCK_ਨਕਾਬ ){
    ਅੱਖਰ z_ਤਾਲਾ[30];
    sqlite3_snprintf(ਆਕਾਰ(z_ਤਾਲਾ),z_ਤਾਲਾ,"%d",e_ਤਾਲਾ);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(z_ਤਾਲਾ, -1), 0, 0);
  }
  ਜੇ( p->ਨਕਾਬ&TESTVFS_UNLOCK_ਨਕਾਬ && tvfsInjectIoerr(p) ){
    ਵਾਪਸ SQLITE_IOERR_UNLOCK;
  }
  ਵਾਪਸ sqlite3OsUnlock(pFd->pReal, e_ਤਾਲਾ);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ tvfs_Check_Reserved_ਤਾਲਾ(sqlite3_file *pFile, ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ਅੰਕੜੇ;
  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_CKLOCK_ਨਕਾਬ ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  ਵਾਪਸ sqlite3_Os_Check_Reserved_ਤਾਲਾ(pFd->pReal, p_Res_ਬਾਹਰਲਾ);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ tvfsFileControl(sqlite3_file *pFile, ਪੂਰਨ op, ਖਾਲੀ *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ਅੰਕੜੇ;
  ਜੇ( op==SQLITE_FCNTL_PRAGMA ){
    ਅੱਖਰ **argv = (ਅੱਖਰ**)pArg;
    ਜੇ( sqlite3_stricmp(argv[1],"error")==0 ){
      ਪੂਰਨ rc = SQLITE_ERROR;
      ਜੇ( argv[2] ){
        ਅਚਲ ਅੱਖਰ *z = argv[2];
        ਪੂਰਨ x = atoi(z);
        ਜੇ( x ){
          rc = x;
          ਜਦੋਂ( sqlite3Isdigit(z[0]) ){ z++; }
          ਜਦੋਂ( sqlite3Isspace(z[0]) ){ z++; }
        }
        ਜੇ( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      ਵਾਪਸ rc;
    }
    ਜੇ( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      ਵਾਪਸ SQLITE_OK;
    }
  }
  ਜੇ( p->pScript && (p->ਨਕਾਬ&TESTVFS_FCNTL_ਨਕਾਬ) ){
    ਬਣਤਰ Fcntl {
      ਪੂਰਨ iFnctl;
      ਅਚਲ ਅੱਖਰ *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    ਪੂਰਨ i;
    ਲਈ(i=0; i<ਆਕਾਰ(aF)/ਆਕਾਰ(aF[0]); i++){
      ਜੇ( op==aF[i].iFnctl ) ਰੋਕੋ;
    }
    ਜੇ( i<ਆਕਾਰ(aF)/ਆਕਾਰ(aF[0]) ){
      ਪੂਰਨ rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfs_ਨਤੀਜਾ_ਕੋਡ(p, &rc);
      ਜੇ( rc ) ਵਾਪਸ (rc<0 ? SQLITE_OK : rc);
    }
  }
  ਵਾਪਸ sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ tvfs_Sector_ਮਾਪ(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ਅੰਕੜੇ;
  ਜੇ( p->iSectorsize>=0 ){
    ਵਾਪਸ p->iSectorsize;
  }
  ਵਾਪਸ sqlite3_Os_Sector_ਮਾਪ(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
ਸਥਿਰ ਪੂਰਨ tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ਅੰਕੜੇ;
  ਜੇ( p->iDevchar>=0 ){
    ਵਾਪਸ p->iDevchar;
  }
  ਵਾਪਸ sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
ਸਥਿਰ ਪੂਰਨ tvfsOpen(
  sqlite3_vfs *pVfs,
  ਅਚਲ ਅੱਖਰ *zName,
  sqlite3_file *pFile,
  ਪੂਰਨ flags,
  ਪੂਰਨ *p_ਬਾਹਰਲਾ_Flags
){
  ਪੂਰਨ rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->p_App_ਅੰਕੜੇ;

  pFd = (TestvfsFd *)ckalloc(ਆਕਾਰ(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, ਆਕਾਰ(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, ਆਕਾਰ(TestvfsFile));
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
  Tcl_Reset_ਨਤੀਜਾ(p->interp);
  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_OPEN_ਨਕਾਬ ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_ਗਿਣਤੀ(pArg);
    ਜੇ( flags&SQLITE_OPEN_MAIN_DB ){
      ਅਚਲ ਅੱਖਰ *z = &zName[ਲੜੀ_ਲੰਬਾਈ(zName)+1];
      ਜਦੋਂ( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += ਲੜੀ_ਲੰਬਾਈ(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += ਲੜੀ_ਲੰਬਾਈ(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_ਗਿਣਤੀ(pArg);
    ਜੇ( tvfs_ਨਤੀਜਾ_ਕੋਡ(p, &rc) ){
      ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
    }ਨਹੀਂਤਾਂ{
      pId = Tcl_Get_Obj_ਨਤੀਜਾ(p->interp);
    }
  }

  ਜੇ( (p->ਨਕਾਬ&TESTVFS_OPEN_ਨਕਾਬ) &&  tvfsInjectIoerr(p) ) ਵਾਪਸ SQLITE_IOERR;
  ਜੇ( tvfsInjectCantopenerr(p) ) ਵਾਪਸ SQLITE_CANTOPEN;
  ਜੇ( tvfsInjectFullerr(p) ) ਵਾਪਸ SQLITE_ਭਰਿਆ;

  ਜੇ( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_ਗਿਣਤੀ(pId);
  pFd->pShmId = pId;
  Tcl_Reset_ਨਤੀਜਾ(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, p_ਬਾਹਰਲਾ_Flags);
  ਜੇ( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    ਪੂਰਨ nByte;

    ਜੇ( pVfs->i_ਸੰਸਕਰਣ>1 ){
      nByte = ਆਕਾਰ(sqlite3_io_methods);
    }ਨਹੀਂਤਾਂ{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pMethods, &tvfs_io_methods, nByte);
    pMethods->i_ਸੰਸਕਰਣ = pFd->pReal->pMethods->i_ਸੰਸਕਰਣ;
    ਜੇ( pMethods->i_ਸੰਸਕਰਣ>pVfs->i_ਸੰਸਕਰਣ ){
      pMethods->i_ਸੰਸਕਰਣ = pVfs->i_ਸੰਸਕਰਣ;
    }
    ਜੇ( pVfs->i_ਸੰਸਕਰਣ>1 && ((Testvfs *)pVfs->p_App_ਅੰਕੜੇ)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->x_Shm_ਤਾਲਾ = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  ਵਾਪਸ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ਸਥਿਰ ਪੂਰਨ tvfsDelete(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, ਪੂਰਨ dirSync){
  ਪੂਰਨ rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->p_App_ਅੰਕੜੇ;

  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_DELETE_ਨਕਾਬ ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(z_ਰਸਤਾ, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfs_ਨਤੀਜਾ_ਕੋਡ(p, &rc);
  }
  ਜੇ( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), z_ਰਸਤਾ, dirSync);
  }
  ਵਾਪਸ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ਸਥਿਰ ਪੂਰਨ tvfsAccess(
  sqlite3_vfs *pVfs, 
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, 
  ਪੂਰਨ flags, 
  ਪੂਰਨ *p_Res_ਬਾਹਰਲਾ
){
  Testvfs *p = (Testvfs *)pVfs->p_App_ਅੰਕੜੇ;
  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_ACCESS_ਨਕਾਬ ){
    ਪੂਰਨ rc;
    ਅੱਖਰ *zArg = 0;
    ਜੇ( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    ਜੇ( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    ਜੇ( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(z_ਰਸਤਾ, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    ਜੇ( tvfs_ਨਤੀਜਾ_ਕੋਡ(p, &rc) ){
      ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
    }ਨਹੀਂਤਾਂ{
      Tcl_Interp *interp = p->interp;
      ਜੇ( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_Get_Obj_ਨਤੀਜਾ(interp), p_Res_ਬਾਹਰਲਾ) ){
        ਵਾਪਸ SQLITE_OK;
      }
    }
  }
  ਵਾਪਸ sqlite3OsAccess(PARENTVFS(pVfs), z_ਰਸਤਾ, flags, p_Res_ਬਾਹਰਲਾ);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ਸਥਿਰ ਪੂਰਨ tvfs_ਭਰਿਆ_Pathname(
  sqlite3_vfs *pVfs, 
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ, 
  ਪੂਰਨ n_ਬਾਹਰਲਾ, 
  ਅੱਖਰ *z_ਬਾਹਰਲਾ
){
  Testvfs *p = (Testvfs *)pVfs->p_App_ਅੰਕੜੇ;
  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_FULLPATHNAME_ਨਕਾਬ ){
    ਪੂਰਨ rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(z_ਰਸਤਾ, -1), 0, 0, 0);
    ਜੇ( tvfs_ਨਤੀਜਾ_ਕੋਡ(p, &rc) ){
      ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
    }
  }
  ਵਾਪਸ sqlite3_Os_ਭਰਿਆ_Pathname(PARENTVFS(pVfs), z_ਰਸਤਾ, n_ਬਾਹਰਲਾ, z_ਬਾਹਰਲਾ);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ਸਥਿਰ ਖਾਲੀ *tvfsDlOpen(sqlite3_vfs *pVfs, ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ){
  ਵਾਪਸ sqlite3OsDlOpen(PARENTVFS(pVfs), z_ਰਸਤਾ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ਸਥਿਰ ਖਾਲੀ tvfsDlError(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Err_ਸੁਨੇਹਾ){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_ਸੁਨੇਹਾ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ਸਥਿਰ ਖਾਲੀ (*tvfsDlSym(sqlite3_vfs *pVfs, ਖਾਲੀ *p, ਅਚਲ ਅੱਖਰ *zSym))(ਖਾਲੀ){
  ਵਾਪਸ sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ਸਥਿਰ ਖਾਲੀ tvfsDlClose(sqlite3_vfs *pVfs, ਖਾਲੀ *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ਸਥਿਰ ਪੂਰਨ tvfsRandomness(sqlite3_vfs *pVfs, ਪੂਰਨ nByte, ਅੱਖਰ *z_Buf_ਬਾਹਰਲਾ){
  ਵਾਪਸ sqlite3OsRandomness(PARENTVFS(pVfs), nByte, z_Buf_ਬਾਹਰਲਾ);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ਸਥਿਰ ਪੂਰਨ tvfsSleep(sqlite3_vfs *pVfs, ਪੂਰਨ nMicro){
  Testvfs *p = (Testvfs *)pVfs->p_App_ਅੰਕੜੇ;
  ਜੇ( p->pScript && (p->ਨਕਾਬ&TESTVFS_SLEEP_ਨਕਾਬ) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  ਵਾਪਸ sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ਸਥਿਰ ਪੂਰਨ tvfs_Current_ਸਮਾਂ(sqlite3_vfs *pVfs, ਦੁੱਗਣਾ *p_ਸਮਾਂ_ਬਾਹਰਲਾ){
  ਵਾਪਸ PARENTVFS(pVfs)->x_Current_ਸਮਾਂ(PARENTVFS(pVfs), p_ਸਮਾਂ_ਬਾਹਰਲਾ);
}

ਸਥਿਰ ਪੂਰਨ tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  ਪੂਰਨ rc = SQLITE_OK;             /* Return code */
  Testvfs_ਬਫਰ *p_ਬਫਰ;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->p_App_ਅੰਕੜੇ;
  ਪ੍ਰਤਿਜ੍ਞਾ( 0==p->isFullshm );
  ਪ੍ਰਤਿਜ੍ਞਾ( pFd->pShmId && pFd->pShm==0 && pFd->p_ਅਗਲਾ==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_Reset_ਨਤੀਜਾ(p->interp);
  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_SHMOPEN_ਨਕਾਬ ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    ਜੇ( tvfs_ਨਤੀਜਾ_ਕੋਡ(p, &rc) ){
      ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
    }
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK );
  ਜੇ( p->ਨਕਾਬ&TESTVFS_SHMOPEN_ਨਕਾਬ && tvfsInjectIoerr(p) ){
    ਵਾਪਸ SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  ਲਈ(p_ਬਫਰ=p->p_ਬਫਰ; p_ਬਫਰ; p_ਬਫਰ=p_ਬਫਰ->p_ਅਗਲਾ){
    ਜੇ( 0==strcmp(pFd->zFilename, p_ਬਫਰ->zFile) ) ਰੋਕੋ;
  }
  ਜੇ( !p_ਬਫਰ ){
    ਪੂਰਨ szName = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(pFd->zFilename);
    ਪੂਰਨ nByte = ਆਕਾਰ(Testvfs_ਬਫਰ) + szName + 1;
    p_ਬਫਰ = (Testvfs_ਬਫਰ *)ckalloc(nByte);
    memset(p_ਬਫਰ, 0, nByte);
    p_ਬਫਰ->zFile = (ਅੱਖਰ *)&p_ਬਫਰ[1];
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਬਫਰ->zFile, pFd->zFilename, szName+1);
    p_ਬਫਰ->p_ਅਗਲਾ = p->p_ਬਫਰ;
    p->p_ਬਫਰ = p_ਬਫਰ;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->p_ਅਗਲਾ = p_ਬਫਰ->pFile;
  p_ਬਫਰ->pFile = pFd;
  pFd->pShm = p_ਬਫਰ;
  ਵਾਪਸ rc;
}

ਸਥਿਰ ਖਾਲੀ tvfs_Alloc_ਪੰਨਾ(Testvfs_ਬਫਰ *p, ਪੂਰਨ i_ਪੰਨਾ, ਪੂਰਨ pgsz){
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਪੰਨਾ<TESTVFS_MAX_ਪੰਨੇ );
  ਜੇ( p->a_ਪੰਨਾ[i_ਪੰਨਾ]==0 ){
    p->a_ਪੰਨਾ[i_ਪੰਨਾ] = (u8 *)ckalloc(pgsz);
    memset(p->a_ਪੰਨਾ[i_ਪੰਨਾ], 0, pgsz);
    p->pgsz = pgsz;
  }
}

ਸਥਿਰ ਪੂਰਨ tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  ਪੂਰਨ i_ਪੰਨਾ,                      /* Page to retrieve */
  ਪੂਰਨ pgsz,                       /* Size of pages */
  ਪੂਰਨ isWrite,                    /* True to extend file if necessary */
  ਖਾਲੀ ਅਸਥਿਰ **pp              /* OUT: Mapped memory */
){
  ਪੂਰਨ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_ਅੰਕੜੇ);

  ਜੇ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ਵਾਪਸ pReal->pMethods->xShmMap(pReal, i_ਪੰਨਾ, pgsz, isWrite, pp);
  }

  ਜੇ( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    ਜੇ( rc!=SQLITE_OK ){
      ਵਾਪਸ rc;
    }
  }

  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_SHMMAP_ਨਕਾਬ ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_ਗਿਣਤੀ(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(i_ਪੰਨਾ));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfs_ਨਤੀਜਾ_ਕੋਡ(p, &rc);
    Tcl_Decr_Ref_ਗਿਣਤੀ(pArg);
  }
  ਜੇ( rc==SQLITE_OK && p->ਨਕਾਬ&TESTVFS_SHMMAP_ਨਕਾਬ && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ਜੇ( rc==SQLITE_OK && isWrite && !pFd->pShm->a_ਪੰਨਾ[i_ਪੰਨਾ] ){
    tvfs_Alloc_ਪੰਨਾ(pFd->pShm, i_ਪੰਨਾ, pgsz);
  }
  ਜੇ( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (ਖਾਲੀ ਅਸਥਿਰ *)pFd->pShm->a_ਪੰਨਾ[i_ਪੰਨਾ];
  }

  ਵਾਪਸ rc;
}


ਸਥਿਰ ਪੂਰਨ tvfs_Shm_ਤਾਲਾ(
  sqlite3_file *pFile,
  ਪੂਰਨ ofst,
  ਪੂਰਨ n,
  ਪੂਰਨ flags
){
  ਪੂਰਨ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_ਅੰਕੜੇ);
  ਪੂਰਨ n_ਤਾਲਾ;
  ਅੱਖਰ z_ਤਾਲਾ[80];

  ਜੇ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ਵਾਪਸ pReal->pMethods->x_Shm_ਤਾਲਾ(pReal, ofst, n, flags);
  }

  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_SHMLOCK_ਨਕਾਬ ){
    sqlite3_snprintf(ਆਕਾਰ(z_ਤਾਲਾ), z_ਤਾਲਾ, "%d %d", ofst, n);
    n_ਤਾਲਾ = (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(z_ਤਾਲਾ);
    ਜੇ( flags & SQLITE_SHM_ਤਾਲਾ ){
      strcpy(&z_ਤਾਲਾ[n_ਤਾਲਾ], " lock");
    }ਨਹੀਂਤਾਂ{
      strcpy(&z_ਤਾਲਾ[n_ਤਾਲਾ], " unlock");
    }
    n_ਤਾਲਾ += (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(&z_ਤਾਲਾ[n_ਤਾਲਾ]);
    ਜੇ( flags & SQLITE_SHM_SHARED ){
      strcpy(&z_ਤਾਲਾ[n_ਤਾਲਾ], " shared");
    }ਨਹੀਂਤਾਂ{
      strcpy(&z_ਤਾਲਾ[n_ਤਾਲਾ], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(z_ਤਾਲਾ, -1), 0
    );
    tvfs_ਨਤੀਜਾ_ਕੋਡ(p, &rc);
  }

  ਜੇ( rc==SQLITE_OK && p->ਨਕਾਬ&TESTVFS_SHMLOCK_ਨਕਾਬ && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ਜੇ( rc==SQLITE_OK ){
    ਪੂਰਨ is_ਤਾਲਾ = (flags & SQLITE_SHM_ਤਾਲਾ);
    ਪੂਰਨ isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 ਨਕਾਬ = (((1<<n)-1) << ofst);
    ਜੇ( is_ਤਾਲਾ ){
      TestvfsFd *p2;
      ਲਈ(p2=pFd->pShm->pFile; p2; p2=p2->p_ਅਗਲਾ){
        ਜੇ( p2==pFd ) ਜਾਰੀ;
        ਜੇ( (p2->excllock&ਨਕਾਬ) || (isExcl && p2->sharedlock&ਨਕਾਬ) ){
          rc = SQLITE_BUSY;
          ਰੋਕੋ;
        }
      }
      ਜੇ( rc==SQLITE_OK ){
        ਜੇ( isExcl )  pFd->excllock |= ਨਕਾਬ;
        ਜੇ( !isExcl ) pFd->sharedlock |= ਨਕਾਬ;
      }
    }ਨਹੀਂਤਾਂ{
      ਜੇ( isExcl )  pFd->excllock &= (~ਨਕਾਬ);
      ਜੇ( !isExcl ) pFd->sharedlock &= (~ਨਕਾਬ);
    }
  }

  ਵਾਪਸ rc;
}

ਸਥਿਰ ਖਾਲੀ tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_ਅੰਕੜੇ);

  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_SHMBARRIER_ਨਕਾਬ ){
    ਅਚਲ ਅੱਖਰ *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  ਜੇ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    ਵਾਪਸ;
  }
}

ਸਥਿਰ ਪੂਰਨ tvfsShmUnmap(
  sqlite3_file *pFile,
  ਪੂਰਨ deleteFlag
){
  ਪੂਰਨ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_ਅੰਕੜੇ);
  Testvfs_ਬਫਰ *p_ਬਫਰ = pFd->pShm;
  TestvfsFd **ppFd;

  ਜੇ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ਵਾਪਸ pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  ਜੇ( !p_ਬਫਰ ) ਵਾਪਸ SQLITE_OK;
  ਪ੍ਰਤਿਜ੍ਞਾ( pFd->pShmId && pFd->pShm );

  ਜੇ( p->pScript && p->ਨਕਾਬ&TESTVFS_SHMCLOSE_ਨਕਾਬ ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfs_ਨਤੀਜਾ_ਕੋਡ(p, &rc);
  }

  ਲਈ(ppFd=&p_ਬਫਰ->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->p_ਅਗਲਾ));
  ਪ੍ਰਤਿਜ੍ਞਾ( (*ppFd)==pFd );
  *ppFd = pFd->p_ਅਗਲਾ;
  pFd->p_ਅਗਲਾ = 0;

  ਜੇ( p_ਬਫਰ->pFile==0 ){
    ਪੂਰਨ i;
    Testvfs_ਬਫਰ **pp;
    ਲਈ(pp=&p->p_ਬਫਰ; *pp!=p_ਬਫਰ; pp=&((*pp)->p_ਅਗਲਾ));
    *pp = (*pp)->p_ਅਗਲਾ;
    ਲਈ(i=0; p_ਬਫਰ->a_ਪੰਨਾ[i]; i++){
      ckfree((ਅੱਖਰ *)p_ਬਫਰ->a_ਪੰਨਾ[i]);
    }
    ckfree((ਅੱਖਰ *)p_ਬਫਰ);
  }
  pFd->pShm = 0;

  ਵਾਪਸ rc;
}

ਸਥਿਰ ਪੂਰਨ tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    ਪੂਰਨ iAmt, 
    ਖਾਲੀ **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  ਵਾਪਸ sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

ਸਥਿਰ ਪੂਰਨ tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, ਖਾਲੀ *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  ਵਾਪਸ sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI testvfs_obj_cmd(
  Client_ਅੰਕੜੇ cd,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  ਗਿਣਤੀ DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  ਬਣਤਰ TestvfsSubcmd {
    ਅੱਖਰ *zName;
    ਗਿਣਤੀ DB_enum eCmd;
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
  ਪੂਰਨ i;
  
  ਜੇ( objc<2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "SUBCOMMAND ...");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj_Struct(
        interp, objv[1], aSubcmd, ਆਕਾਰ(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    ਵਾਪਸ TCL_ERROR;
  }
  Tcl_Reset_ਨਤੀਜਾ(interp);

  ਚੋਣ( aSubcmd[i].eCmd ){
    ਹਾਲਤ CMD_SHM: {
      Tcl_Obj *pObj;
      ਪੂਰਨ rc;
      Testvfs_ਬਫਰ *p_ਬਫਰ;
      ਅੱਖਰ *zName;
      ਜੇ( objc!=3 && objc!=4 ){
        Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "FILE ?VALUE?");
        ਵਾਪਸ TCL_ERROR;
      }
      zName = ckalloc(p->p_ਮਾਪਾ->mxPathname);
      rc = p->p_ਮਾਪਾ->x_ਭਰਿਆ_Pathname(
          p->p_ਮਾਪਾ, Tcl_GetString(objv[2]), 
          p->p_ਮਾਪਾ->mxPathname, zName
      );
      ਜੇ( rc!=SQLITE_OK ){
        Tcl_Append_ਨਤੀਜਾ(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), ਨਿਰਾ);
        ckfree(zName);
        ਵਾਪਸ TCL_ERROR;
      }
      ਲਈ(p_ਬਫਰ=p->p_ਬਫਰ; p_ਬਫਰ; p_ਬਫਰ=p_ਬਫਰ->p_ਅਗਲਾ){
        ਜੇ( 0==strcmp(p_ਬਫਰ->zFile, zName) ) ਰੋਕੋ;
      }
      ckfree(zName);
      ਜੇ( !p_ਬਫਰ ){
        Tcl_Append_ਨਤੀਜਾ(interp, "no such file: ", Tcl_GetString(objv[2]), ਨਿਰਾ);
        ਵਾਪਸ TCL_ERROR;
      }
      ਜੇ( objc==4 ){
        Tcl_ਮਾਪ n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        ਪੂਰਨ pgsz = p_ਬਫਰ->pgsz;
        ਜੇ( pgsz==0 ) pgsz = 65536;
        ਲਈ(i=0; i*pgsz<(ਪੂਰਨ)n; i++){
          ਪੂਰਨ nByte = pgsz;
          tvfs_Alloc_ਪੰਨਾ(p_ਬਫਰ, i, pgsz);
          ਜੇ( n-i*pgsz<pgsz ){
            nByte = (ਪੂਰਨ)n;
          }
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਬਫਰ->a_ਪੰਨਾ[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      ਲਈ(i=0; p_ਬਫਰ->a_ਪੰਨਾ[i]; i++){
        ਪੂਰਨ pgsz = p_ਬਫਰ->pgsz;
        ਜੇ( pgsz==0 ) pgsz = 65536;
        Tcl_Append_Obj_ਮੰਜ਼ਿਲ_Obj(pObj, Tcl_NewByteArrayObj(p_ਬਫਰ->a_ਪੰਨਾ[i], pgsz));
      }
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pObj);
      ਰੋਕੋ;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    ਹਾਲਤ CMD_FILTER: {
      ਸਥਿਰ ਬਣਤਰ Vfs_ਵਿਧੀ {
        ਅੱਖਰ *zName;
        ਪੂਰਨ ਨਕਾਬ;
      } vfsmethod [] = {
        { "xShmOpen",           TESTVFS_SHMOPEN_ਨਕਾਬ },
        { "xShmLock",           TESTVFS_SHMLOCK_ਨਕਾਬ },
        { "xShmBarrier",        TESTVFS_SHMBARRIER_ਨਕਾਬ },
        { "xShmUnmap",          TESTVFS_SHMCLOSE_ਨਕਾਬ },
        { "xShmMap",            TESTVFS_SHMMAP_ਨਕਾਬ },
        { "xSync",              TESTVFS_SYNC_ਨਕਾਬ },
        { "xDelete",            TESTVFS_DELETE_ਨਕਾਬ },
        { "xWrite",             TESTVFS_WRITE_ਨਕਾਬ },
        { "xRead",              TESTVFS_READ_ਨਕਾਬ },
        { "xTruncate",          TESTVFS_TRUNCATE_ਨਕਾਬ },
        { "xOpen",              TESTVFS_OPEN_ਨਕਾਬ },
        { "xClose",             TESTVFS_CLOSE_ਨਕਾਬ },
        { "xAccess",            TESTVFS_ACCESS_ਨਕਾਬ },
        { "xFullPathname",      TESTVFS_FULLPATHNAME_ਨਕਾਬ },
        { "xUnlock",            TESTVFS_UNLOCK_ਨਕਾਬ },
        { "xLock",              TESTVFS_ਤਾਲਾ_ਨਕਾਬ },
        { "xCheckReservedLock", TESTVFS_CKLOCK_ਨਕਾਬ },
        { "xFileControl",       TESTVFS_FCNTL_ਨਕਾਬ },
        { "xSleep",             TESTVFS_SLEEP_ਨਕਾਬ },
      };
      Tcl_Obj **apElem = 0;
      Tcl_ਮਾਪ nElem = 0;
      ਪੂਰਨ ਨਕਾਬ = 0;
      ਜੇ( objc!=3 ){
        Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "LIST");
        ਵਾਪਸ TCL_ERROR;
      }
      ਜੇ( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        ਵਾਪਸ TCL_ERROR;
      }
      Tcl_Reset_ਨਤੀਜਾ(interp);
      ਲਈ(i=0; i<(ਪੂਰਨ)nElem; i++){
        ਪੂਰਨ i_ਵਿਧੀ;
        ਅੱਖਰ *zElem = Tcl_GetString(apElem[i]);
        ਲਈ(i_ਵਿਧੀ=0; i_ਵਿਧੀ<Array_ਮਾਪ(vfsmethod); i_ਵਿਧੀ++){
          ਜੇ( strcmp(zElem, vfsmethod[i_ਵਿਧੀ].zName)==0 ){
            ਨਕਾਬ |= vfsmethod[i_ਵਿਧੀ].ਨਕਾਬ;
            ਰੋਕੋ;
          }
        }
        ਜੇ( i_ਵਿਧੀ==Array_ਮਾਪ(vfsmethod) ){
          Tcl_Append_ਨਤੀਜਾ(interp, "unknown method: ", zElem, ਨਿਰਾ);
          ਵਾਪਸ TCL_ERROR;
        }
      }
      p->ਨਕਾਬ = ਨਕਾਬ;
      ਰੋਕੋ;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    ਹਾਲਤ CMD_SCRIPT: {
      ਜੇ( objc==3 ){
        Tcl_ਮਾਪ nByte;
        ਜੇ( p->pScript ){
          Tcl_Decr_Ref_ਗਿਣਤੀ(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        ਜੇ( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_ਗਿਣਤੀ(p->pScript);
        }
      }ਨਹੀਂਤਾਂ ਜੇ( objc!=2 ){
        Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?SCRIPT?");
        ਵਾਪਸ TCL_ERROR;
      }

      Tcl_Reset_ਨਤੀਜਾ(interp);
      ਜੇ( p->pScript ) Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, p->pScript);

      ਰੋਕੋ;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    ਹਾਲਤ CMD_CANTOPENERR:
    ਹਾਲਤ CMD_IOERR:
    ਹਾਲਤ CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      ਪੂਰਨ iRet;

      ਚੋਣ( aSubcmd[i].eCmd ){
        ਹਾਲਤ CMD_IOERR: pTest = &p->ioerr_err; ਰੋਕੋ;
        ਹਾਲਤ CMD_FULLERR: pTest = &p->ਭਰਿਆ_err; ਰੋਕੋ;
        ਹਾਲਤ CMD_CANTOPENERR: pTest = &p->cantopen_err; ਰੋਕੋ;
        ਮੂਲ: ਪ੍ਰਤਿਜ੍ਞਾ(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      ਜੇ( objc==4 ){
        ਪੂਰਨ iCnt, iPersist;
        ਜੇ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          ਵਾਪਸ TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }ਨਹੀਂਤਾਂ ਜੇ( objc!=2 ){
        Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?CNT PERSIST?");
        ਵਾਪਸ TCL_ERROR;
      }
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(iRet));
      ਰੋਕੋ;
    }

    ਹਾਲਤ CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      ਰੋਕੋ;
    }

    ਹਾਲਤ CMD_DEVCHAR: {
      ਬਣਤਰ DeviceFlag {
        ਅੱਖਰ *zName;
        ਪੂਰਨ i_ਮੁੱਲ;
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
        { "safe_append",           SQLITE_IOCAP_ਸੁਰੱਖਿਅਤ_APPEND           },
        { "undeletable_when_open", SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN },
        { "powersafe_overwrite",   SQLITE_IOCAP_POWERSAFE_OVERWRITE   },
        { "immutable",             SQLITE_IOCAP_IMMUTABLE             },
        { 0, 0 }
      };
      Tcl_Obj *pRet;
      ਪੂਰਨ iFlag;

      ਜੇ( objc>3 ){
        Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?ATTR-LIST?");
        ਵਾਪਸ TCL_ERROR;
      }
      ਜੇ( objc==3 ){
        ਪੂਰਨ j;
        ਪੂਰਨ iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_ਮਾਪ nFlags = 0;

        ਜੇ( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          ਵਾਪਸ TCL_ERROR;
        }

        ਲਈ(j=0; j<(ਪੂਰਨ)nFlags; j++){
          ਪੂਰਨ idx = 0;
          ਜੇ( Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj_Struct(interp, flags[j], aFlag, 
                ਆਕਾਰ(aFlag[0]), "flag", 0, &idx) 
          ){
            ਵਾਪਸ TCL_ERROR;
          }
          ਜੇ( aFlag[idx].i_ਮੁੱਲ<0 && nFlags>1 ){
            Tcl_Append_ਨਤੀਜਾ(interp, "bad flags: ", Tcl_GetString(objv[2]), ਨਿਰਾ);
            ਵਾਪਸ TCL_ERROR;
          }
          iNew |= aFlag[idx].i_ਮੁੱਲ;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      ਲਈ(iFlag=0; iFlag<ਆਕਾਰ(aFlag)/ਆਕਾਰ(aFlag[0]); iFlag++){
        ਜੇ( p->iDevchar & aFlag[iFlag].i_ਮੁੱਲ ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, pRet);

      ਰੋਕੋ;
    }

    ਹਾਲਤ CMD_SECTORSIZE: {
      ਜੇ( objc>3 ){
        Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, "?VALUE?");
        ਵਾਪਸ TCL_ERROR;
      }
      ਜੇ( objc==3 ){
        ਪੂਰਨ iNew = 0;
        ਜੇ( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          ਵਾਪਸ TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(p->iSectorsize));
      ਰੋਕੋ;
    }
  }

  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਖਾਲੀ SQLITE_TCLAPI testvfs_obj_del(Client_ਅੰਕੜੇ cd){
  Testvfs *p = (Testvfs *)cd;
  ਜੇ( p->pScript ) Tcl_Decr_Ref_ਗਿਣਤੀ(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, ਆਕਾਰ(sqlite3_vfs));
  ckfree((ਅੱਖਰ *)p->pVfs);
  memset(p, 0, ਆਕਾਰ(Testvfs));
  ckfree((ਅੱਖਰ *)p);
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
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI testvfs_cmd(
  Client_ਅੰਕੜੇ cd,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਸਥਿਰ sqlite3_vfs tvfs_vfs = {
    3,                            /* iVersion */
    0,                            /* szOsFile */
    0,                            /* mxPathname */
    0,                            /* pNext */
    0,                            /* zName */
    0,                            /* pAppData */
    tvfsOpen,                     /* xOpen */
    tvfsDelete,                   /* xDelete */
    tvfsAccess,                   /* xAccess */
    tvfs_ਭਰਿਆ_Pathname,             /* xFullPathname */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_ਲੱਦੋ_EXTENSION
    tvfsDlOpen,                   /* xDlOpen */
    tvfsDlError,                  /* xDlError */
    tvfsDlSym,                    /* xDlSym */
    tvfsDlClose,                  /* xDlClose */
#ਨਹੀਂਤਾਂ
    0,                            /* xDlOpen */
    0,                            /* xDlError */
    0,                            /* xDlSym */
    0,                            /* xDlClose */
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_LOAD_EXTENSION */
    tvfsRandomness,               /* xRandomness */
    tvfsSleep,                    /* xSleep */
    tvfs_Current_ਸਮਾਂ,              /* xCurrentTime */
    0,                            /* xGetLastError */
    0,                            /* xCurrentTimeInt64 */
    0,                            /* xSetSystemCall */
    0,                            /* xGetSystemCall */
    0,                            /* xNextSystemCall */
  };

  Testvfs *p;                     /* New object */
  sqlite3_vfs *pVfs;              /* New VFS */
  ਅੱਖਰ *zVfs;
  ਪੂਰਨ nByte;                      /* Bytes of space to allocate at p */

  ਪੂਰਨ i;
  ਪੂਰਨ isNoshm = 0;                /* True if -noshm is passed */
  ਪੂਰਨ isFullshm = 0;              /* True if -fullshm is passed */
  ਪੂਰਨ isDefault = 0;              /* True if -default is passed */
  ਪੂਰਨ szOsFile = 0;               /* Value passed to -szosfile */
  ਪੂਰਨ mxPathname = -1;            /* Value passed to -mxpathname */
  ਪੂਰਨ i_ਸੰਸਕਰਣ = 3;               /* Value passed to -iversion */

  ਜੇ( objc<2 || 0!=(objc%2) ) ਜਾਓ bad_ਦਲੀਲਾਂ;
  ਲਈ(i=2; i<objc; i += 2){
    Tcl_ਮਾਪ nSwitch;
    ਅੱਖਰ *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    ਜੇ( nSwitch>2 && 0==ਲੜੀ_ਤੁਲਨਾ("-noshm", zSwitch, nSwitch) ){
      ਜੇ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        ਵਾਪਸ TCL_ERROR;
      }
      ਜੇ( isNoshm ) isFullshm = 0;
    }
    ਨਹੀਂਤਾਂ ਜੇ( nSwitch>2 && 0==ਲੜੀ_ਤੁਲਨਾ("-default", zSwitch, nSwitch) ){
      ਜੇ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        ਵਾਪਸ TCL_ERROR;
      }
    }
    ਨਹੀਂਤਾਂ ਜੇ( nSwitch>2 && 0==ਲੜੀ_ਤੁਲਨਾ("-szosfile", zSwitch, nSwitch) ){
      ਜੇ( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        ਵਾਪਸ TCL_ERROR;
      }
    }
    ਨਹੀਂਤਾਂ ਜੇ( nSwitch>2 && 0==ਲੜੀ_ਤੁਲਨਾ("-mxpathname", zSwitch, nSwitch) ){
      ਜੇ( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        ਵਾਪਸ TCL_ERROR;
      }
    }
    ਨਹੀਂਤਾਂ ਜੇ( nSwitch>2 && 0==ਲੜੀ_ਤੁਲਨਾ("-iversion", zSwitch, nSwitch) ){
      ਜੇ( Tcl_GetIntFromObj(interp, objv[i+1], &i_ਸੰਸਕਰਣ) ){
        ਵਾਪਸ TCL_ERROR;
      }
    }
    ਨਹੀਂਤਾਂ ਜੇ( nSwitch>2 && 0==ਲੜੀ_ਤੁਲਨਾ("-fullshm", zSwitch, nSwitch) ){
      ਜੇ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        ਵਾਪਸ TCL_ERROR;
      }
      ਜੇ( isFullshm ) isNoshm = 0;
    }
    ਨਹੀਂਤਾਂ{
      ਜਾਓ bad_ਦਲੀਲਾਂ;
    }
  }

  ਜੇ( szOsFile<ਆਕਾਰ(TestvfsFile) ){
    szOsFile = ਆਕਾਰ(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = ਆਕਾਰ(Testvfs) + (ਪੂਰਨ)ਲੜੀ_ਲੰਬਾਈ(zVfs)+1;
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
  p->p_ਮਾਪਾ = sqlite3_vfs_ਲੱਭੋ(0);
  p->interp = interp;

  p->zName = (ਅੱਖਰ *)&p[1];
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(p->zName, zVfs, ਲੜੀ_ਲੰਬਾਈ(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(ਆਕਾਰ(sqlite3_vfs));
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(pVfs, &tvfs_vfs, ਆਕਾਰ(sqlite3_vfs));
  pVfs->p_App_ਅੰਕੜੇ = (ਖਾਲੀ *)p;
  pVfs->i_ਸੰਸਕਰਣ = i_ਸੰਸਕਰਣ;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->p_ਮਾਪਾ->mxPathname;
  ਜੇ( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->ਨਕਾਬ = TESTVFS_ALL_ਨਕਾਬ;

  sqlite3_vfs_ਰਜਿਸਟਰ(pVfs, isDefault);

  ਵਾਪਸ TCL_OK;

 bad_ਦਲੀਲਾਂ:
  Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  ਵਾਪਸ TCL_ERROR;
}

ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp *interp, ਅਚਲ ਅੱਖਰ *zA, sqlite3 **ppDb);
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_vfs_shmlock(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਅਚਲ ਅੱਖਰ *azArg1[] = {"shared", "exclusive", 0};
  ਅਚਲ ਅੱਖਰ *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  ਪੂਰਨ rc = SQLITE_OK;
  ਅਚਲ ਅੱਖਰ *zDbname = 0;
  ਪੂਰਨ iArg1 = 0;
  ਪੂਰਨ iArg2 = 0;
  ਪੂਰਨ i_ਵਿੱਥ = 0;
  ਪੂਰਨ n = 0;
  sqlite3_file *pFd;

  ਜੇ( objc!=7 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    ਵਾਪਸ TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &i_ਵਿੱਥ)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    ਵਾਪਸ TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_ਸੂਚਕ, (ਖਾਲੀ*)&pFd);
  ਜੇ( pFd==0 ){
    ਵਾਪਸ TCL_ERROR;
  }
  rc = pFd->pMethods->x_Shm_ਤਾਲਾ(pFd, i_ਵਿੱਥ, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_ਤਾਲਾ : SQLITE_SHM_UNLOCK)
  );
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_vfs_ਸੈੱਟ_readmark(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  ਪੂਰਨ rc = SQLITE_OK;
  ਅਚਲ ਅੱਖਰ *zDbname = 0;
  ਪੂਰਨ i_ਥਾਂ = 0;
  ਪੂਰਨ iVal = -1;
  sqlite3_file *pFd;
  ਖਾਲੀ ਅਸਥਿਰ *pShm = 0;
  u32 *aShm;
  ਪੂਰਨ iOff;

  ਜੇ( objc!=4 && objc!=5 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    ਵਾਪਸ TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &i_ਥਾਂ)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    ਵਾਪਸ TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_ਸੂਚਕ, (ਖਾਲੀ*)&pFd);
  ਜੇ( pFd==0 ){
    ਵਾਪਸ TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( pShm==0 ){
    Tcl_Append_ਨਤੀਜਾ(interp, "*-shm is not yet mapped", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+i_ਥਾਂ;

  ਜੇ( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(aShm[iOff]));

  ਵਾਪਸ TCL_OK;
}

ਪੂਰਨ Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_ਸੈੱਟ_readmark, 0, 0);
  ਵਾਪਸ TCL_OK;
}

#ਜੇ_ਅੰਤ
