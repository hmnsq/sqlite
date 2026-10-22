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
#ఒకవేళ SQLITE_TEST          /* This file is used for testing only */

#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ "tclsqlite.h"

రకనిర్వచనం నిర్మాణం Testvfs Testvfs;
రకనిర్వచనం నిర్మాణం TestvfsShm TestvfsShm;
రకనిర్వచనం నిర్మాణం Testvfs_బఫరు Testvfs_బఫరు;
రకనిర్వచనం నిర్మాణం TestvfsFile TestvfsFile;
రకనిర్వచనం నిర్మాణం TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
నిర్మాణం TestvfsFile {
  sqlite3_file పునాది;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#నిర్వచించు tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

నిర్మాణం TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  స్థిరరాశి అక్షరం *zFilename;          /* Filename as passed to xOpen() */
  sqlite3_file *pReal;            /* The real, underlying file descriptor */
  Tcl_Obj *pShmId;                /* Shared memory id for Tcl callbacks */

  Testvfs_బఫరు *pShm;            /* Shared memory buffer */
  u32 excllock;                   /* Mask of exclusive locks */
  u32 sharedlock;                 /* Mask of shared locks */
  TestvfsFd *p_తదుపరి;               /* Next handle opened on the same file */
};


#నిర్వచించు FAULT_INJECT_NONE       0
#నిర్వచించు FAULT_INJECT_TRANSIENT  1
#నిర్వచించు FAULT_INJECT_PERSISTENT 2

రకనిర్వచనం నిర్మాణం TestFaultInject TestFaultInject;
నిర్మాణం TestFaultInject {
  పూర్ణాంకం iCnt;                       /* Remaining calls before fault injection */
  పూర్ణాంకం eFault;                     /* A FAULT_INJECT_* value */
  పూర్ణాంకం nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
నిర్మాణం Testvfs {
  అక్షరం *zName;                    /* Name of this VFS */
  sqlite3_vfs *p_మాతృ;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  Testvfs_బఫరు *p_బఫరు;         /* List of shared buffers */
  పూర్ణాంకం isNoshm;
  పూర్ణాంకం isFullshm;

  పూర్ణాంకం ముసుగు;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject నిండిన_err;
  TestFaultInject cantopen_err;

#ఒకవేళ 0
  పూర్ణాంకం iIoerrCnt;
  పూర్ణాంకం ioerr;
  పూర్ణాంకం nIoerrFail;
  పూర్ణాంకం i_నిండిన_Cnt;
  పూర్ణాంకం fullerr;
  పూర్ణాంకం n_నిండిన_Fail;
#అయితే_ముగింపు

  పూర్ణాంకం iDevchar;
  పూర్ణాంకం iSectorsize;
};

/*
** The Testvfs.mask variable is set to a combination of the following.
** If a bit is clear in Testvfs.mask, then calls made by SQLite to the 
** corresponding VFS method is ignored for purposes of:
**
**   + Simulating IO errors, and
**   + Invoking the Tcl callback script.
*/
#నిర్వచించు TESTVFS_SHMOPEN_ముసుగు      0x00000001
#నిర్వచించు TESTVFS_SHMLOCK_ముసుగు      0x00000010
#నిర్వచించు TESTVFS_SHMMAP_ముసుగు       0x00000020
#నిర్వచించు TESTVFS_SHMBARRIER_ముసుగు   0x00000040
#నిర్వచించు TESTVFS_SHMCLOSE_ముసుగు     0x00000080

#నిర్వచించు TESTVFS_OPEN_ముసుగు         0x00000100
#నిర్వచించు TESTVFS_SYNC_ముసుగు         0x00000200
#నిర్వచించు TESTVFS_DELETE_ముసుగు       0x00000400
#నిర్వచించు TESTVFS_CLOSE_ముసుగు        0x00000800
#నిర్వచించు TESTVFS_WRITE_ముసుగు        0x00001000
#నిర్వచించు TESTVFS_TRUNCATE_ముసుగు     0x00002000
#నిర్వచించు TESTVFS_ACCESS_ముసుగు       0x00004000
#నిర్వచించు TESTVFS_FULLPATHNAME_ముసుగు 0x00008000
#నిర్వచించు TESTVFS_READ_ముసుగు         0x00010000
#నిర్వచించు TESTVFS_UNLOCK_ముసుగు       0x00020000
#నిర్వచించు TESTVFS_తాళం_ముసుగు         0x00040000
#నిర్వచించు TESTVFS_CKLOCK_ముసుగు       0x00080000
#నిర్వచించు TESTVFS_FCNTL_ముసుగు        0x00100000
#నిర్వచించు TESTVFS_SLEEP_ముసుగు        0x00200000

#నిర్వచించు TESTVFS_ALL_ముసుగు          0x003FFFFF


#నిర్వచించు TESTVFS_MAX_పుటలు 1024

/*
** A shared-memory buffer. There is one of these objects for each shared
** memory region opened by clients. If two clients open the same file,
** there are two TestvfsFile structures but only one TestvfsBuffer structure.
*/
నిర్మాణం Testvfs_బఫరు {
  అక్షరం *zFile;                    /* Associated file name */
  పూర్ణాంకం pgsz;                       /* Page size */
  u8 *a_పుట[TESTVFS_MAX_పుటలు];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  Testvfs_బఫరు *p_తదుపరి;           /* Next in linked list of all buffers */
};


#నిర్వచించు PARENTVFS(x) (((Testvfs *)((x)->p_App_దత్తాంశం))->p_మాతృ)

#నిర్వచించు TESTVFS_MAX_వాదనలు 12


/*
** Method declarations for TestvfsFile.
*/
స్థిర పూర్ణాంకం tvfsClose(sqlite3_file*);
స్థిర పూర్ణాంకం tvfsRead(sqlite3_file*, శూన్య*, పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం tvfsWrite(sqlite3_file*,స్థిరరాశి శూన్య*,పూర్ణాంకం iAmt, sqlite3_int64 iOfst);
స్థిర పూర్ణాంకం tvfsTruncate(sqlite3_file*, sqlite3_int64 కొలత);
స్థిర పూర్ణాంకం tvfsSync(sqlite3_file*, పూర్ణాంకం flags);
స్థిర పూర్ణాంకం tvfs_File_కొలత(sqlite3_file*, sqlite3_int64 *p_కొలత);
స్థిర పూర్ణాంకం tvfs_తాళం(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం tvfsUnlock(sqlite3_file*, పూర్ణాంకం);
స్థిర పూర్ణాంకం tvfs_Check_Reserved_తాళం(sqlite3_file*, పూర్ణాంకం *);
స్థిర పూర్ణాంకం tvfsFileControl(sqlite3_file*, పూర్ణాంకం op, శూన్య *pArg);
స్థిర పూర్ణాంకం tvfs_Sector_కొలత(sqlite3_file*);
స్థిర పూర్ణాంకం tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
స్థిర పూర్ణాంకం tvfsOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *, sqlite3_file*, పూర్ణాంకం , పూర్ణాంకం *);
స్థిర పూర్ణాంకం tvfsDelete(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం syncDir);
స్థిర పూర్ణాంకం tvfsAccess(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం flags, పూర్ణాంకం *);
స్థిర పూర్ణాంకం tvfs_నిండిన_Pathname(sqlite3_vfs*, స్థిరరాశి అక్షరం *zName, పూర్ణాంకం, అక్షరం *z_బహిర్గతం);
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
స్థిర శూన్య *tvfsDlOpen(sqlite3_vfs*, స్థిరరాశి అక్షరం *zFilename);
స్థిర శూన్య tvfsDlError(sqlite3_vfs*, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం);
స్థిర శూన్య (*tvfsDlSym(sqlite3_vfs*,శూన్య*, స్థిరరాశి అక్షరం *z_చిహ్నం))(శూన్య);
స్థిర శూన్య tvfsDlClose(sqlite3_vfs*, శూన్య*);
#అయితే_ముగింపు /* SQLITE_OMIT_LOAD_EXTENSION */
స్థిర పూర్ణాంకం tvfsRandomness(sqlite3_vfs*, పూర్ణాంకం nByte, అక్షరం *z_బహిర్గతం);
స్థిర పూర్ణాంకం tvfsSleep(sqlite3_vfs*, పూర్ణాంకం microseconds);
స్థిర పూర్ణాంకం tvfs_Current_సమయం(sqlite3_vfs*, ద్విగుణ*);

స్థిర పూర్ణాంకం tvfsShmOpen(sqlite3_file*);
స్థిర పూర్ణాంకం tvfs_Shm_తాళం(sqlite3_file*, పూర్ణాంకం , పూర్ణాంకం, పూర్ణాంకం);
స్థిర పూర్ణాంకం tvfsShmMap(sqlite3_file*,పూర్ణాంకం,పూర్ణాంకం,పూర్ణాంకం, శూన్య అస్థిర **);
స్థిర శూన్య tvfsShmBarrier(sqlite3_file*);
స్థిర పూర్ణాంకం tvfsShmUnmap(sqlite3_file*, పూర్ణాంకం);

స్థిర పూర్ణాంకం tvfsFetch(sqlite3_file*, sqlite3_int64, పూర్ణాంకం, శూన్య**);
స్థిర పూర్ణాంకం tvfsUnfetch(sqlite3_file*, sqlite3_int64, శూన్య*);

స్థిర sqlite3_io_methods tvfs_io_methods = {
  3,                              /* iVersion */
  tvfsClose,                      /* xClose */
  tvfsRead,                       /* xRead */
  tvfsWrite,                      /* xWrite */
  tvfsTruncate,                   /* xTruncate */
  tvfsSync,                       /* xSync */
  tvfs_File_కొలత,                   /* xFileSize */
  tvfs_తాళం,                       /* xLock */
  tvfsUnlock,                     /* xUnlock */
  tvfs_Check_Reserved_తాళం,          /* xCheckReservedLock */
  tvfsFileControl,                /* xFileControl */
  tvfs_Sector_కొలత,                 /* xSectorSize */
  tvfsDeviceCharacteristics,      /* xDeviceCharacteristics */
  tvfsShmMap,                     /* xShmMap */
  tvfs_Shm_తాళం,                    /* xShmLock */
  tvfsShmBarrier,                 /* xShmBarrier */
  tvfsShmUnmap,                   /* xShmUnmap */
  tvfsFetch,
  tvfsUnfetch
};

స్థిర పూర్ణాంకం tvfs_ఫలితం_సంకేతం(Testvfs *p, పూర్ణాంకం *pRc){
  నిర్మాణం errcode {
    పూర్ణాంకం e_సంకేతం;
    స్థిరరాశి అక్షరం *z_సంకేతం;
  } a_సంకేతం[] = {
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

  స్థిరరాశి అక్షరం *z;
  పూర్ణాంకం i;

  z = Tcl_Get_String_ఫలితం(p->interp);
  కొరకు(i=0; i<Array_కొలత(a_సంకేతం); i++){
    ఒకవేళ( 0==strcmp(z, a_సంకేతం[i].z_సంకేతం) ){
      *pRc = a_సంకేతం[i].e_సంకేతం;
      తిరిగి 1;
    }
  }

  తిరిగి 0;
}

స్థిర పూర్ణాంకం tvfsInjectFault(TestFaultInject *p){
  పూర్ణాంకం ret = 0;
  ఒకవేళ( p->eFault ){
    p->iCnt--;
    ఒకవేళ( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  తిరిగి ret;
}


స్థిర పూర్ణాంకం tvfsInjectIoerr(Testvfs *p){
  తిరిగి tvfsInjectFault(&p->ioerr_err);
}

స్థిర పూర్ణాంకం tvfsInjectFullerr(Testvfs *p){
  తిరిగి tvfsInjectFault(&p->నిండిన_err);
}
స్థిర పూర్ణాంకం tvfsInjectCantopenerr(Testvfs *p){
  తిరిగి tvfsInjectFault(&p->cantopen_err);
}


స్థిర శూన్య tvfsExecTcl(
  Testvfs *p, 
  స్థిరరాశి అక్షరం *z_పద్ధతి,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  పూర్ణాంకం rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  ప్రతిజ్ఞా( p->pScript );

  ప్రతిజ్ఞా( z_పద్ధతి );
  ప్రతిజ్ఞా( p );
  ప్రతిజ్ఞా( arg2==0 || arg1!=0 );
  ప్రతిజ్ఞా( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_లెక్క(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(z_పద్ధతి, -1));
  ఒకవేళ( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  ఒకవేళ( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  ఒకవేళ( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  ఒకవేళ( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  ఒకవేళ( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_Reset_ఫలితం(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
స్థిర పూర్ణాంకం tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_దత్తాంశం;

  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_CLOSE_ముసుగు ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  ఒకవేళ( pFd->pShmId ){
    Tcl_Decr_Ref_లెక్క(pFd->pShmId);
    pFd->pShmId = 0;
  }
  ఒకవేళ( pFile->pMethods ){
    ckfree((అక్షరం *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((అక్షరం *)pFd);
  pTestfile->pFd = 0;
  తిరిగి SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
స్థిర పూర్ణాంకం tvfsRead(
  sqlite3_file *pFile, 
  శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  పూర్ణాంకం rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_దత్తాంశం;
  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_READ_ముసుగు ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_ఫలితం_సంకేతం(p, &rc);
  }
  ఒకవేళ( rc==SQLITE_OK && p->ముసుగు&TESTVFS_READ_ముసుగు && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  తిరిగి rc;
}

/*
** Write data to an tvfs-file.
*/
స్థిర పూర్ణాంకం tvfsWrite(
  sqlite3_file *pFile, 
  స్థిరరాశి శూన్య *zBuf, 
  పూర్ణాంకం iAmt, 
  sqlite_int64 iOfst
){
  పూర్ణాంకం rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_దత్తాంశం;

  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_WRITE_ముసుగు ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfs_ఫలితం_సంకేతం(p, &rc);
    ఒకవేళ( rc<0 ) తిరిగి SQLITE_OK;
  }

  ఒకవేళ( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_నిండిన;
  }
  ఒకవేళ( rc==SQLITE_OK && p->ముసుగు&TESTVFS_WRITE_ముసుగు && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  తిరిగి rc;
}

/*
** Truncate an tvfs-file.
*/
స్థిర పూర్ణాంకం tvfsTruncate(sqlite3_file *pFile, sqlite_int64 కొలత){
  పూర్ణాంకం rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_దత్తాంశం;

  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_TRUNCATE_ముసుగు ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_ఫలితం_సంకేతం(p, &rc);
  }
  
  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, కొలత);
  }
  తిరిగి rc;
}

/*
** Sync an tvfs-file.
*/
స్థిర పూర్ణాంకం tvfsSync(sqlite3_file *pFile, పూర్ణాంకం flags){
  పూర్ణాంకం rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_దత్తాంశం;

  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_SYNC_ముసుగు ){
    అక్షరం *zFlags = 0;

    ఎంపిక( flags ){
      స్థితి SQLITE_SYNC_సాధారణ:
        zFlags = "normal";
        ఆపు;
      స్థితి SQLITE_SYNC_నిండిన:
        zFlags = "full";
        ఆపు;
      స్థితి SQLITE_SYNC_సాధారణ|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        ఆపు;
      స్థితి SQLITE_SYNC_నిండిన|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        ఆపు;
      అప్రమేయ:
        ప్రతిజ్ఞా(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfs_ఫలితం_సంకేతం(p, &rc);
  }

  ఒకవేళ( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_నిండిన;

  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  తిరిగి rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
స్థిర పూర్ణాంకం tvfs_File_కొలత(sqlite3_file *pFile, sqlite_int64 *p_కొలత){
  TestvfsFd *p = tvfsGetFd(pFile);
  తిరిగి sqlite3_Os_File_కొలత(p->pReal, p_కొలత);
}

/*
** Lock an tvfs-file.
*/
స్థిర పూర్ణాంకం tvfs_తాళం(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_దత్తాంశం;
  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_తాళం_ముసుగు ){
    అక్షరం z_తాళం[30];
    sqlite3_snprintf(పరిమాణం(z_తాళం),z_తాళం,"%d",e_తాళం);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(z_తాళం, -1), 0, 0);
  }
  ఒకవేళ( p->ముసుగు&TESTVFS_తాళం_ముసుగు && tvfsInjectIoerr(p) ){
    తిరిగి SQLITE_IOERR_తాళం;
  }
  తిరిగి sqlite3_Os_తాళం(pFd->pReal, e_తాళం);
}

/*
** Unlock an tvfs-file.
*/
స్థిర పూర్ణాంకం tvfsUnlock(sqlite3_file *pFile, పూర్ణాంకం e_తాళం){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_దత్తాంశం;
  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_UNLOCK_ముసుగు ){
    అక్షరం z_తాళం[30];
    sqlite3_snprintf(పరిమాణం(z_తాళం),z_తాళం,"%d",e_తాళం);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(z_తాళం, -1), 0, 0);
  }
  ఒకవేళ( p->ముసుగు&TESTVFS_UNLOCK_ముసుగు && tvfsInjectIoerr(p) ){
    తిరిగి SQLITE_IOERR_UNLOCK;
  }
  తిరిగి sqlite3OsUnlock(pFd->pReal, e_తాళం);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
స్థిర పూర్ణాంకం tvfs_Check_Reserved_తాళం(sqlite3_file *pFile, పూర్ణాంకం *p_Res_బహిర్గతం){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_దత్తాంశం;
  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_CKLOCK_ముసుగు ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  తిరిగి sqlite3_Os_Check_Reserved_తాళం(pFd->pReal, p_Res_బహిర్గతం);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
స్థిర పూర్ణాంకం tvfsFileControl(sqlite3_file *pFile, పూర్ణాంకం op, శూన్య *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_దత్తాంశం;
  ఒకవేళ( op==SQLITE_FCNTL_PRAGMA ){
    అక్షరం **argv = (అక్షరం**)pArg;
    ఒకవేళ( sqlite3_stricmp(argv[1],"error")==0 ){
      పూర్ణాంకం rc = SQLITE_ERROR;
      ఒకవేళ( argv[2] ){
        స్థిరరాశి అక్షరం *z = argv[2];
        పూర్ణాంకం x = atoi(z);
        ఒకవేళ( x ){
          rc = x;
          అయితే( sqlite3Isdigit(z[0]) ){ z++; }
          అయితే( sqlite3Isspace(z[0]) ){ z++; }
        }
        ఒకవేళ( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      తిరిగి rc;
    }
    ఒకవేళ( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      తిరిగి SQLITE_OK;
    }
  }
  ఒకవేళ( p->pScript && (p->ముసుగు&TESTVFS_FCNTL_ముసుగు) ){
    నిర్మాణం Fcntl {
      పూర్ణాంకం iFnctl;
      స్థిరరాశి అక్షరం *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    పూర్ణాంకం i;
    కొరకు(i=0; i<పరిమాణం(aF)/పరిమాణం(aF[0]); i++){
      ఒకవేళ( op==aF[i].iFnctl ) ఆపు;
    }
    ఒకవేళ( i<పరిమాణం(aF)/పరిమాణం(aF[0]) ){
      పూర్ణాంకం rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfs_ఫలితం_సంకేతం(p, &rc);
      ఒకవేళ( rc ) తిరిగి (rc<0 ? SQLITE_OK : rc);
    }
  }
  తిరిగి sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
స్థిర పూర్ణాంకం tvfs_Sector_కొలత(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_దత్తాంశం;
  ఒకవేళ( p->iSectorsize>=0 ){
    తిరిగి p->iSectorsize;
  }
  తిరిగి sqlite3_Os_Sector_కొలత(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
స్థిర పూర్ణాంకం tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_దత్తాంశం;
  ఒకవేళ( p->iDevchar>=0 ){
    తిరిగి p->iDevchar;
  }
  తిరిగి sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
స్థిర పూర్ణాంకం tvfsOpen(
  sqlite3_vfs *pVfs,
  స్థిరరాశి అక్షరం *zName,
  sqlite3_file *pFile,
  పూర్ణాంకం flags,
  పూర్ణాంకం *p_బహిర్గతం_Flags
){
  పూర్ణాంకం rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->p_App_దత్తాంశం;

  pFd = (TestvfsFd *)ckalloc(పరిమాణం(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, పరిమాణం(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, పరిమాణం(TestvfsFile));
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
  Tcl_Reset_ఫలితం(p->interp);
  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_OPEN_ముసుగు ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_లెక్క(pArg);
    ఒకవేళ( flags&SQLITE_OPEN_MAIN_DB ){
      స్థిరరాశి అక్షరం *z = &zName[అక్షరశ్రేణి_పొడవు(zName)+1];
      అయితే( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += అక్షరశ్రేణి_పొడవు(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += అక్షరశ్రేణి_పొడవు(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_లెక్క(pArg);
    ఒకవేళ( tvfs_ఫలితం_సంకేతం(p, &rc) ){
      ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
    }లేకపోతే{
      pId = Tcl_Get_Obj_ఫలితం(p->interp);
    }
  }

  ఒకవేళ( (p->ముసుగు&TESTVFS_OPEN_ముసుగు) &&  tvfsInjectIoerr(p) ) తిరిగి SQLITE_IOERR;
  ఒకవేళ( tvfsInjectCantopenerr(p) ) తిరిగి SQLITE_CANTOPEN;
  ఒకవేళ( tvfsInjectFullerr(p) ) తిరిగి SQLITE_నిండిన;

  ఒకవేళ( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_లెక్క(pId);
  pFd->pShmId = pId;
  Tcl_Reset_ఫలితం(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, p_బహిర్గతం_Flags);
  ఒకవేళ( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    పూర్ణాంకం nByte;

    ఒకవేళ( pVfs->i_సంస్కరణ>1 ){
      nByte = పరిమాణం(sqlite3_io_methods);
    }లేకపోతే{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    జ్ఞాపకం_నకలు(pMethods, &tvfs_io_methods, nByte);
    pMethods->i_సంస్కరణ = pFd->pReal->pMethods->i_సంస్కరణ;
    ఒకవేళ( pMethods->i_సంస్కరణ>pVfs->i_సంస్కరణ ){
      pMethods->i_సంస్కరణ = pVfs->i_సంస్కరణ;
    }
    ఒకవేళ( pVfs->i_సంస్కరణ>1 && ((Testvfs *)pVfs->p_App_దత్తాంశం)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->x_Shm_తాళం = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  తిరిగి rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
స్థిర పూర్ణాంకం tvfsDelete(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం, పూర్ణాంకం dirSync){
  పూర్ణాంకం rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->p_App_దత్తాంశం;

  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_DELETE_ముసుగు ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(z_మార్గం, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfs_ఫలితం_సంకేతం(p, &rc);
  }
  ఒకవేళ( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), z_మార్గం, dirSync);
  }
  తిరిగి rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
స్థిర పూర్ణాంకం tvfsAccess(
  sqlite3_vfs *pVfs, 
  స్థిరరాశి అక్షరం *z_మార్గం, 
  పూర్ణాంకం flags, 
  పూర్ణాంకం *p_Res_బహిర్గతం
){
  Testvfs *p = (Testvfs *)pVfs->p_App_దత్తాంశం;
  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_ACCESS_ముసుగు ){
    పూర్ణాంకం rc;
    అక్షరం *zArg = 0;
    ఒకవేళ( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    ఒకవేళ( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    ఒకవేళ( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(z_మార్గం, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    ఒకవేళ( tvfs_ఫలితం_సంకేతం(p, &rc) ){
      ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
    }లేకపోతే{
      Tcl_Interp *interp = p->interp;
      ఒకవేళ( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_Get_Obj_ఫలితం(interp), p_Res_బహిర్గతం) ){
        తిరిగి SQLITE_OK;
      }
    }
  }
  తిరిగి sqlite3OsAccess(PARENTVFS(pVfs), z_మార్గం, flags, p_Res_బహిర్గతం);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
స్థిర పూర్ణాంకం tvfs_నిండిన_Pathname(
  sqlite3_vfs *pVfs, 
  స్థిరరాశి అక్షరం *z_మార్గం, 
  పూర్ణాంకం n_బహిర్గతం, 
  అక్షరం *z_బహిర్గతం
){
  Testvfs *p = (Testvfs *)pVfs->p_App_దత్తాంశం;
  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_FULLPATHNAME_ముసుగు ){
    పూర్ణాంకం rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(z_మార్గం, -1), 0, 0, 0);
    ఒకవేళ( tvfs_ఫలితం_సంకేతం(p, &rc) ){
      ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
    }
  }
  తిరిగి sqlite3_Os_నిండిన_Pathname(PARENTVFS(pVfs), z_మార్గం, n_బహిర్గతం, z_బహిర్గతం);
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
స్థిర శూన్య *tvfsDlOpen(sqlite3_vfs *pVfs, స్థిరరాశి అక్షరం *z_మార్గం){
  తిరిగి sqlite3OsDlOpen(PARENTVFS(pVfs), z_మార్గం);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
స్థిర శూన్య tvfsDlError(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Err_సందేశం){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_సందేశం);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
స్థిర శూన్య (*tvfsDlSym(sqlite3_vfs *pVfs, శూన్య *p, స్థిరరాశి అక్షరం *zSym))(శూన్య){
  తిరిగి sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
స్థిర శూన్య tvfsDlClose(sqlite3_vfs *pVfs, శూన్య *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#అయితే_ముగింపు /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
స్థిర పూర్ణాంకం tvfsRandomness(sqlite3_vfs *pVfs, పూర్ణాంకం nByte, అక్షరం *z_Buf_బహిర్గతం){
  తిరిగి sqlite3OsRandomness(PARENTVFS(pVfs), nByte, z_Buf_బహిర్గతం);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
స్థిర పూర్ణాంకం tvfsSleep(sqlite3_vfs *pVfs, పూర్ణాంకం nMicro){
  Testvfs *p = (Testvfs *)pVfs->p_App_దత్తాంశం;
  ఒకవేళ( p->pScript && (p->ముసుగు&TESTVFS_SLEEP_ముసుగు) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  తిరిగి sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
స్థిర పూర్ణాంకం tvfs_Current_సమయం(sqlite3_vfs *pVfs, ద్విగుణ *p_సమయం_బహిర్గతం){
  తిరిగి PARENTVFS(pVfs)->x_Current_సమయం(PARENTVFS(pVfs), p_సమయం_బహిర్గతం);
}

స్థిర పూర్ణాంకం tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  పూర్ణాంకం rc = SQLITE_OK;             /* Return code */
  Testvfs_బఫరు *p_బఫరు;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->p_App_దత్తాంశం;
  ప్రతిజ్ఞా( 0==p->isFullshm );
  ప్రతిజ్ఞా( pFd->pShmId && pFd->pShm==0 && pFd->p_తదుపరి==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_Reset_ఫలితం(p->interp);
  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_SHMOPEN_ముసుగు ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    ఒకవేళ( tvfs_ఫలితం_సంకేతం(p, &rc) ){
      ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
    }
  }

  ప్రతిజ్ఞా( rc==SQLITE_OK );
  ఒకవేళ( p->ముసుగు&TESTVFS_SHMOPEN_ముసుగు && tvfsInjectIoerr(p) ){
    తిరిగి SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  కొరకు(p_బఫరు=p->p_బఫరు; p_బఫరు; p_బఫరు=p_బఫరు->p_తదుపరి){
    ఒకవేళ( 0==strcmp(pFd->zFilename, p_బఫరు->zFile) ) ఆపు;
  }
  ఒకవేళ( !p_బఫరు ){
    పూర్ణాంకం szName = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(pFd->zFilename);
    పూర్ణాంకం nByte = పరిమాణం(Testvfs_బఫరు) + szName + 1;
    p_బఫరు = (Testvfs_బఫరు *)ckalloc(nByte);
    memset(p_బఫరు, 0, nByte);
    p_బఫరు->zFile = (అక్షరం *)&p_బఫరు[1];
    జ్ఞాపకం_నకలు(p_బఫరు->zFile, pFd->zFilename, szName+1);
    p_బఫరు->p_తదుపరి = p->p_బఫరు;
    p->p_బఫరు = p_బఫరు;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->p_తదుపరి = p_బఫరు->pFile;
  p_బఫరు->pFile = pFd;
  pFd->pShm = p_బఫరు;
  తిరిగి rc;
}

స్థిర శూన్య tvfs_Alloc_పుట(Testvfs_బఫరు *p, పూర్ణాంకం i_పుట, పూర్ణాంకం pgsz){
  ప్రతిజ్ఞా( i_పుట<TESTVFS_MAX_పుటలు );
  ఒకవేళ( p->a_పుట[i_పుట]==0 ){
    p->a_పుట[i_పుట] = (u8 *)ckalloc(pgsz);
    memset(p->a_పుట[i_పుట], 0, pgsz);
    p->pgsz = pgsz;
  }
}

స్థిర పూర్ణాంకం tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  పూర్ణాంకం i_పుట,                      /* Page to retrieve */
  పూర్ణాంకం pgsz,                       /* Size of pages */
  పూర్ణాంకం isWrite,                    /* True to extend file if necessary */
  శూన్య అస్థిర **pp              /* OUT: Mapped memory */
){
  పూర్ణాంకం rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_దత్తాంశం);

  ఒకవేళ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    తిరిగి pReal->pMethods->xShmMap(pReal, i_పుట, pgsz, isWrite, pp);
  }

  ఒకవేళ( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    ఒకవేళ( rc!=SQLITE_OK ){
      తిరిగి rc;
    }
  }

  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_SHMMAP_ముసుగు ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_లెక్క(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(i_పుట));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfs_ఫలితం_సంకేతం(p, &rc);
    Tcl_Decr_Ref_లెక్క(pArg);
  }
  ఒకవేళ( rc==SQLITE_OK && p->ముసుగు&TESTVFS_SHMMAP_ముసుగు && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ఒకవేళ( rc==SQLITE_OK && isWrite && !pFd->pShm->a_పుట[i_పుట] ){
    tvfs_Alloc_పుట(pFd->pShm, i_పుట, pgsz);
  }
  ఒకవేళ( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (శూన్య అస్థిర *)pFd->pShm->a_పుట[i_పుట];
  }

  తిరిగి rc;
}


స్థిర పూర్ణాంకం tvfs_Shm_తాళం(
  sqlite3_file *pFile,
  పూర్ణాంకం ofst,
  పూర్ణాంకం n,
  పూర్ణాంకం flags
){
  పూర్ణాంకం rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_దత్తాంశం);
  పూర్ణాంకం n_తాళం;
  అక్షరం z_తాళం[80];

  ఒకవేళ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    తిరిగి pReal->pMethods->x_Shm_తాళం(pReal, ofst, n, flags);
  }

  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_SHMLOCK_ముసుగు ){
    sqlite3_snprintf(పరిమాణం(z_తాళం), z_తాళం, "%d %d", ofst, n);
    n_తాళం = (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(z_తాళం);
    ఒకవేళ( flags & SQLITE_SHM_తాళం ){
      strcpy(&z_తాళం[n_తాళం], " lock");
    }లేకపోతే{
      strcpy(&z_తాళం[n_తాళం], " unlock");
    }
    n_తాళం += (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(&z_తాళం[n_తాళం]);
    ఒకవేళ( flags & SQLITE_SHM_SHARED ){
      strcpy(&z_తాళం[n_తాళం], " shared");
    }లేకపోతే{
      strcpy(&z_తాళం[n_తాళం], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(z_తాళం, -1), 0
    );
    tvfs_ఫలితం_సంకేతం(p, &rc);
  }

  ఒకవేళ( rc==SQLITE_OK && p->ముసుగు&TESTVFS_SHMLOCK_ముసుగు && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ఒకవేళ( rc==SQLITE_OK ){
    పూర్ణాంకం is_తాళం = (flags & SQLITE_SHM_తాళం);
    పూర్ణాంకం isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 ముసుగు = (((1<<n)-1) << ofst);
    ఒకవేళ( is_తాళం ){
      TestvfsFd *p2;
      కొరకు(p2=pFd->pShm->pFile; p2; p2=p2->p_తదుపరి){
        ఒకవేళ( p2==pFd ) కొనసాగు;
        ఒకవేళ( (p2->excllock&ముసుగు) || (isExcl && p2->sharedlock&ముసుగు) ){
          rc = SQLITE_BUSY;
          ఆపు;
        }
      }
      ఒకవేళ( rc==SQLITE_OK ){
        ఒకవేళ( isExcl )  pFd->excllock |= ముసుగు;
        ఒకవేళ( !isExcl ) pFd->sharedlock |= ముసుగు;
      }
    }లేకపోతే{
      ఒకవేళ( isExcl )  pFd->excllock &= (~ముసుగు);
      ఒకవేళ( !isExcl ) pFd->sharedlock &= (~ముసుగు);
    }
  }

  తిరిగి rc;
}

స్థిర శూన్య tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_దత్తాంశం);

  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_SHMBARRIER_ముసుగు ){
    స్థిరరాశి అక్షరం *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  ఒకవేళ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    తిరిగి;
  }
}

స్థిర పూర్ణాంకం tvfsShmUnmap(
  sqlite3_file *pFile,
  పూర్ణాంకం deleteFlag
){
  పూర్ణాంకం rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_దత్తాంశం);
  Testvfs_బఫరు *p_బఫరు = pFd->pShm;
  TestvfsFd **ppFd;

  ఒకవేళ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    తిరిగి pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  ఒకవేళ( !p_బఫరు ) తిరిగి SQLITE_OK;
  ప్రతిజ్ఞా( pFd->pShmId && pFd->pShm );

  ఒకవేళ( p->pScript && p->ముసుగు&TESTVFS_SHMCLOSE_ముసుగు ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfs_ఫలితం_సంకేతం(p, &rc);
  }

  కొరకు(ppFd=&p_బఫరు->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->p_తదుపరి));
  ప్రతిజ్ఞా( (*ppFd)==pFd );
  *ppFd = pFd->p_తదుపరి;
  pFd->p_తదుపరి = 0;

  ఒకవేళ( p_బఫరు->pFile==0 ){
    పూర్ణాంకం i;
    Testvfs_బఫరు **pp;
    కొరకు(pp=&p->p_బఫరు; *pp!=p_బఫరు; pp=&((*pp)->p_తదుపరి));
    *pp = (*pp)->p_తదుపరి;
    కొరకు(i=0; p_బఫరు->a_పుట[i]; i++){
      ckfree((అక్షరం *)p_బఫరు->a_పుట[i]);
    }
    ckfree((అక్షరం *)p_బఫరు);
  }
  pFd->pShm = 0;

  తిరిగి rc;
}

స్థిర పూర్ణాంకం tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    పూర్ణాంకం iAmt, 
    శూన్య **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  తిరిగి sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

స్థిర పూర్ణాంకం tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, శూన్య *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  తిరిగి sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI testvfs_obj_cmd(
  Client_దత్తాంశం cd,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  గణన DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  నిర్మాణం TestvfsSubcmd {
    అక్షరం *zName;
    గణన DB_enum eCmd;
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
  పూర్ణాంకం i;
  
  ఒకవేళ( objc<2 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "SUBCOMMAND ...");
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( Tcl_Get_సూచీ_From_Obj_Struct(
        interp, objv[1], aSubcmd, పరిమాణం(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    తిరిగి TCL_ERROR;
  }
  Tcl_Reset_ఫలితం(interp);

  ఎంపిక( aSubcmd[i].eCmd ){
    స్థితి CMD_SHM: {
      Tcl_Obj *pObj;
      పూర్ణాంకం rc;
      Testvfs_బఫరు *p_బఫరు;
      అక్షరం *zName;
      ఒకవేళ( objc!=3 && objc!=4 ){
        Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "FILE ?VALUE?");
        తిరిగి TCL_ERROR;
      }
      zName = ckalloc(p->p_మాతృ->mxPathname);
      rc = p->p_మాతృ->x_నిండిన_Pathname(
          p->p_మాతృ, Tcl_GetString(objv[2]), 
          p->p_మాతృ->mxPathname, zName
      );
      ఒకవేళ( rc!=SQLITE_OK ){
        Tcl_Append_ఫలితం(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), ఖాళీ);
        ckfree(zName);
        తిరిగి TCL_ERROR;
      }
      కొరకు(p_బఫరు=p->p_బఫరు; p_బఫరు; p_బఫరు=p_బఫరు->p_తదుపరి){
        ఒకవేళ( 0==strcmp(p_బఫరు->zFile, zName) ) ఆపు;
      }
      ckfree(zName);
      ఒకవేళ( !p_బఫరు ){
        Tcl_Append_ఫలితం(interp, "no such file: ", Tcl_GetString(objv[2]), ఖాళీ);
        తిరిగి TCL_ERROR;
      }
      ఒకవేళ( objc==4 ){
        Tcl_కొలత n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        పూర్ణాంకం pgsz = p_బఫరు->pgsz;
        ఒకవేళ( pgsz==0 ) pgsz = 65536;
        కొరకు(i=0; i*pgsz<(పూర్ణాంకం)n; i++){
          పూర్ణాంకం nByte = pgsz;
          tvfs_Alloc_పుట(p_బఫరు, i, pgsz);
          ఒకవేళ( n-i*pgsz<pgsz ){
            nByte = (పూర్ణాంకం)n;
          }
          జ్ఞాపకం_నకలు(p_బఫరు->a_పుట[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      కొరకు(i=0; p_బఫరు->a_పుట[i]; i++){
        పూర్ణాంకం pgsz = p_బఫరు->pgsz;
        ఒకవేళ( pgsz==0 ) pgsz = 65536;
        Tcl_Append_Obj_గమ్యం_Obj(pObj, Tcl_NewByteArrayObj(p_బఫరు->a_పుట[i], pgsz));
      }
      Tcl_సమితి_Obj_ఫలితం(interp, pObj);
      ఆపు;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    స్థితి CMD_FILTER: {
      స్థిర నిర్మాణం Vfs_పద్ధతి {
        అక్షరం *zName;
        పూర్ణాంకం ముసుగు;
      } vfsmethod [] = {
        { "xShmOpen",           TESTVFS_SHMOPEN_ముసుగు },
        { "xShmLock",           TESTVFS_SHMLOCK_ముసుగు },
        { "xShmBarrier",        TESTVFS_SHMBARRIER_ముసుగు },
        { "xShmUnmap",          TESTVFS_SHMCLOSE_ముసుగు },
        { "xShmMap",            TESTVFS_SHMMAP_ముసుగు },
        { "xSync",              TESTVFS_SYNC_ముసుగు },
        { "xDelete",            TESTVFS_DELETE_ముసుగు },
        { "xWrite",             TESTVFS_WRITE_ముసుగు },
        { "xRead",              TESTVFS_READ_ముసుగు },
        { "xTruncate",          TESTVFS_TRUNCATE_ముసుగు },
        { "xOpen",              TESTVFS_OPEN_ముసుగు },
        { "xClose",             TESTVFS_CLOSE_ముసుగు },
        { "xAccess",            TESTVFS_ACCESS_ముసుగు },
        { "xFullPathname",      TESTVFS_FULLPATHNAME_ముసుగు },
        { "xUnlock",            TESTVFS_UNLOCK_ముసుగు },
        { "xLock",              TESTVFS_తాళం_ముసుగు },
        { "xCheckReservedLock", TESTVFS_CKLOCK_ముసుగు },
        { "xFileControl",       TESTVFS_FCNTL_ముసుగు },
        { "xSleep",             TESTVFS_SLEEP_ముసుగు },
      };
      Tcl_Obj **apElem = 0;
      Tcl_కొలత nElem = 0;
      పూర్ణాంకం ముసుగు = 0;
      ఒకవేళ( objc!=3 ){
        Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "LIST");
        తిరిగి TCL_ERROR;
      }
      ఒకవేళ( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        తిరిగి TCL_ERROR;
      }
      Tcl_Reset_ఫలితం(interp);
      కొరకు(i=0; i<(పూర్ణాంకం)nElem; i++){
        పూర్ణాంకం i_పద్ధతి;
        అక్షరం *zElem = Tcl_GetString(apElem[i]);
        కొరకు(i_పద్ధతి=0; i_పద్ధతి<Array_కొలత(vfsmethod); i_పద్ధతి++){
          ఒకవేళ( strcmp(zElem, vfsmethod[i_పద్ధతి].zName)==0 ){
            ముసుగు |= vfsmethod[i_పద్ధతి].ముసుగు;
            ఆపు;
          }
        }
        ఒకవేళ( i_పద్ధతి==Array_కొలత(vfsmethod) ){
          Tcl_Append_ఫలితం(interp, "unknown method: ", zElem, ఖాళీ);
          తిరిగి TCL_ERROR;
        }
      }
      p->ముసుగు = ముసుగు;
      ఆపు;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    స్థితి CMD_SCRIPT: {
      ఒకవేళ( objc==3 ){
        Tcl_కొలత nByte;
        ఒకవేళ( p->pScript ){
          Tcl_Decr_Ref_లెక్క(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        ఒకవేళ( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_లెక్క(p->pScript);
        }
      }లేకపోతే ఒకవేళ( objc!=2 ){
        Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?SCRIPT?");
        తిరిగి TCL_ERROR;
      }

      Tcl_Reset_ఫలితం(interp);
      ఒకవేళ( p->pScript ) Tcl_సమితి_Obj_ఫలితం(interp, p->pScript);

      ఆపు;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    స్థితి CMD_CANTOPENERR:
    స్థితి CMD_IOERR:
    స్థితి CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      పూర్ణాంకం iRet;

      ఎంపిక( aSubcmd[i].eCmd ){
        స్థితి CMD_IOERR: pTest = &p->ioerr_err; ఆపు;
        స్థితి CMD_FULLERR: pTest = &p->నిండిన_err; ఆపు;
        స్థితి CMD_CANTOPENERR: pTest = &p->cantopen_err; ఆపు;
        అప్రమేయ: ప్రతిజ్ఞా(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      ఒకవేళ( objc==4 ){
        పూర్ణాంకం iCnt, iPersist;
        ఒకవేళ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          తిరిగి TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }లేకపోతే ఒకవేళ( objc!=2 ){
        Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?CNT PERSIST?");
        తిరిగి TCL_ERROR;
      }
      Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(iRet));
      ఆపు;
    }

    స్థితి CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      ఆపు;
    }

    స్థితి CMD_DEVCHAR: {
      నిర్మాణం DeviceFlag {
        అక్షరం *zName;
        పూర్ణాంకం i_విలువ;
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
        { "safe_append",           SQLITE_IOCAP_సురక్షిత_APPEND           },
        { "undeletable_when_open", SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN },
        { "powersafe_overwrite",   SQLITE_IOCAP_POWERSAFE_OVERWRITE   },
        { "immutable",             SQLITE_IOCAP_IMMUTABLE             },
        { 0, 0 }
      };
      Tcl_Obj *pRet;
      పూర్ణాంకం iFlag;

      ఒకవేళ( objc>3 ){
        Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?ATTR-LIST?");
        తిరిగి TCL_ERROR;
      }
      ఒకవేళ( objc==3 ){
        పూర్ణాంకం j;
        పూర్ణాంకం iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_కొలత nFlags = 0;

        ఒకవేళ( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          తిరిగి TCL_ERROR;
        }

        కొరకు(j=0; j<(పూర్ణాంకం)nFlags; j++){
          పూర్ణాంకం idx = 0;
          ఒకవేళ( Tcl_Get_సూచీ_From_Obj_Struct(interp, flags[j], aFlag, 
                పరిమాణం(aFlag[0]), "flag", 0, &idx) 
          ){
            తిరిగి TCL_ERROR;
          }
          ఒకవేళ( aFlag[idx].i_విలువ<0 && nFlags>1 ){
            Tcl_Append_ఫలితం(interp, "bad flags: ", Tcl_GetString(objv[2]), ఖాళీ);
            తిరిగి TCL_ERROR;
          }
          iNew |= aFlag[idx].i_విలువ;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      కొరకు(iFlag=0; iFlag<పరిమాణం(aFlag)/పరిమాణం(aFlag[0]); iFlag++){
        ఒకవేళ( p->iDevchar & aFlag[iFlag].i_విలువ ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_సమితి_Obj_ఫలితం(interp, pRet);

      ఆపు;
    }

    స్థితి CMD_SECTORSIZE: {
      ఒకవేళ( objc>3 ){
        Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, "?VALUE?");
        తిరిగి TCL_ERROR;
      }
      ఒకవేళ( objc==3 ){
        పూర్ణాంకం iNew = 0;
        ఒకవేళ( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          తిరిగి TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(p->iSectorsize));
      ఆపు;
    }
  }

  తిరిగి TCL_OK;
}

స్థిర శూన్య SQLITE_TCLAPI testvfs_obj_del(Client_దత్తాంశం cd){
  Testvfs *p = (Testvfs *)cd;
  ఒకవేళ( p->pScript ) Tcl_Decr_Ref_లెక్క(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, పరిమాణం(sqlite3_vfs));
  ckfree((అక్షరం *)p->pVfs);
  memset(p, 0, పరిమాణం(Testvfs));
  ckfree((అక్షరం *)p);
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
స్థిర పూర్ణాంకం SQLITE_TCLAPI testvfs_cmd(
  Client_దత్తాంశం cd,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  స్థిర sqlite3_vfs tvfs_vfs = {
    3,                            /* iVersion */
    0,                            /* szOsFile */
    0,                            /* mxPathname */
    0,                            /* pNext */
    0,                            /* zName */
    0,                            /* pAppData */
    tvfsOpen,                     /* xOpen */
    tvfsDelete,                   /* xDelete */
    tvfsAccess,                   /* xAccess */
    tvfs_నిండిన_Pathname,             /* xFullPathname */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_ఎక్కించు_EXTENSION
    tvfsDlOpen,                   /* xDlOpen */
    tvfsDlError,                  /* xDlError */
    tvfsDlSym,                    /* xDlSym */
    tvfsDlClose,                  /* xDlClose */
#లేకపోతే
    0,                            /* xDlOpen */
    0,                            /* xDlError */
    0,                            /* xDlSym */
    0,                            /* xDlClose */
#అయితే_ముగింపు /* SQLITE_OMIT_LOAD_EXTENSION */
    tvfsRandomness,               /* xRandomness */
    tvfsSleep,                    /* xSleep */
    tvfs_Current_సమయం,              /* xCurrentTime */
    0,                            /* xGetLastError */
    0,                            /* xCurrentTimeInt64 */
    0,                            /* xSetSystemCall */
    0,                            /* xGetSystemCall */
    0,                            /* xNextSystemCall */
  };

  Testvfs *p;                     /* New object */
  sqlite3_vfs *pVfs;              /* New VFS */
  అక్షరం *zVfs;
  పూర్ణాంకం nByte;                      /* Bytes of space to allocate at p */

  పూర్ణాంకం i;
  పూర్ణాంకం isNoshm = 0;                /* True if -noshm is passed */
  పూర్ణాంకం isFullshm = 0;              /* True if -fullshm is passed */
  పూర్ణాంకం isDefault = 0;              /* True if -default is passed */
  పూర్ణాంకం szOsFile = 0;               /* Value passed to -szosfile */
  పూర్ణాంకం mxPathname = -1;            /* Value passed to -mxpathname */
  పూర్ణాంకం i_సంస్కరణ = 3;               /* Value passed to -iversion */

  ఒకవేళ( objc<2 || 0!=(objc%2) ) వెళ్ళు bad_వాదనలు;
  కొరకు(i=2; i<objc; i += 2){
    Tcl_కొలత nSwitch;
    అక్షరం *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    ఒకవేళ( nSwitch>2 && 0==అక్షరశ్రేణి_పోలిక("-noshm", zSwitch, nSwitch) ){
      ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        తిరిగి TCL_ERROR;
      }
      ఒకవేళ( isNoshm ) isFullshm = 0;
    }
    లేకపోతే ఒకవేళ( nSwitch>2 && 0==అక్షరశ్రేణి_పోలిక("-default", zSwitch, nSwitch) ){
      ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        తిరిగి TCL_ERROR;
      }
    }
    లేకపోతే ఒకవేళ( nSwitch>2 && 0==అక్షరశ్రేణి_పోలిక("-szosfile", zSwitch, nSwitch) ){
      ఒకవేళ( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        తిరిగి TCL_ERROR;
      }
    }
    లేకపోతే ఒకవేళ( nSwitch>2 && 0==అక్షరశ్రేణి_పోలిక("-mxpathname", zSwitch, nSwitch) ){
      ఒకవేళ( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        తిరిగి TCL_ERROR;
      }
    }
    లేకపోతే ఒకవేళ( nSwitch>2 && 0==అక్షరశ్రేణి_పోలిక("-iversion", zSwitch, nSwitch) ){
      ఒకవేళ( Tcl_GetIntFromObj(interp, objv[i+1], &i_సంస్కరణ) ){
        తిరిగి TCL_ERROR;
      }
    }
    లేకపోతే ఒకవేళ( nSwitch>2 && 0==అక్షరశ్రేణి_పోలిక("-fullshm", zSwitch, nSwitch) ){
      ఒకవేళ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        తిరిగి TCL_ERROR;
      }
      ఒకవేళ( isFullshm ) isNoshm = 0;
    }
    లేకపోతే{
      వెళ్ళు bad_వాదనలు;
    }
  }

  ఒకవేళ( szOsFile<పరిమాణం(TestvfsFile) ){
    szOsFile = పరిమాణం(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = పరిమాణం(Testvfs) + (పూర్ణాంకం)అక్షరశ్రేణి_పొడవు(zVfs)+1;
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
  p->p_మాతృ = sqlite3_vfs_కనుగొను(0);
  p->interp = interp;

  p->zName = (అక్షరం *)&p[1];
  జ్ఞాపకం_నకలు(p->zName, zVfs, అక్షరశ్రేణి_పొడవు(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(పరిమాణం(sqlite3_vfs));
  జ్ఞాపకం_నకలు(pVfs, &tvfs_vfs, పరిమాణం(sqlite3_vfs));
  pVfs->p_App_దత్తాంశం = (శూన్య *)p;
  pVfs->i_సంస్కరణ = i_సంస్కరణ;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->p_మాతృ->mxPathname;
  ఒకవేళ( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->ముసుగు = TESTVFS_ALL_ముసుగు;

  sqlite3_vfs_రిజిస్టరు(pVfs, isDefault);

  తిరిగి TCL_OK;

 bad_వాదనలు:
  Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  తిరిగి TCL_ERROR;
}

బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp *interp, స్థిరరాశి అక్షరం *zA, sqlite3 **ppDb);
బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI test_vfs_shmlock(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  స్థిరరాశి అక్షరం *azArg1[] = {"shared", "exclusive", 0};
  స్థిరరాశి అక్షరం *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  పూర్ణాంకం rc = SQLITE_OK;
  స్థిరరాశి అక్షరం *zDbname = 0;
  పూర్ణాంకం iArg1 = 0;
  పూర్ణాంకం iArg2 = 0;
  పూర్ణాంకం i_స్థానభ్రంశం = 0;
  పూర్ణాంకం n = 0;
  sqlite3_file *pFd;

  ఒకవేళ( objc!=7 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    తిరిగి TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_Get_సూచీ_From_Obj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_Get_సూచీ_From_Obj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &i_స్థానభ్రంశం)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    తిరిగి TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_సూచిక, (శూన్య*)&pFd);
  ఒకవేళ( pFd==0 ){
    తిరిగి TCL_ERROR;
  }
  rc = pFd->pMethods->x_Shm_తాళం(pFd, i_స్థానభ్రంశం, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_తాళం : SQLITE_SHM_UNLOCK)
  );
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI test_vfs_సమితి_readmark(
  శూన్య * client_దత్తాంశం,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  పూర్ణాంకం rc = SQLITE_OK;
  స్థిరరాశి అక్షరం *zDbname = 0;
  పూర్ణాంకం i_స్థానం = 0;
  పూర్ణాంకం iVal = -1;
  sqlite3_file *pFd;
  శూన్య అస్థిర *pShm = 0;
  u32 *aShm;
  పూర్ణాంకం iOff;

  ఒకవేళ( objc!=4 && objc!=5 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    తిరిగి TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ఒకవేళ( get_Db_సూచిక(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &i_స్థానం)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    తిరిగి TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_సూచిక, (శూన్య*)&pFd);
  ఒకవేళ( pFd==0 ){
    తిరిగి TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  ఒకవేళ( rc!=SQLITE_OK ){
    Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    తిరిగి TCL_ERROR;
  }
  ఒకవేళ( pShm==0 ){
    Tcl_Append_ఫలితం(interp, "*-shm is not yet mapped", ఖాళీ);
    తిరిగి TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+i_స్థానం;

  ఒకవేళ( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(aShm[iOff]));

  తిరిగి TCL_OK;
}

పూర్ణాంకం Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_సమితి_readmark, 0, 0);
  తిరిగి TCL_OK;
}

#అయితే_ముగింపు
