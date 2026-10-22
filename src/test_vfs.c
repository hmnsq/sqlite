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

#অন্তর্ভাব "sqlite3.h"
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "tclsqlite.h"

প্রকারলেখ কাঠামো Testvfs Testvfs;
প্রকারলেখ কাঠামো TestvfsShm TestvfsShm;
প্রকারলেখ কাঠামো Testvfs_বাফার Testvfs_বাফার;
প্রকারলেখ কাঠামো TestvfsFile TestvfsFile;
প্রকারলেখ কাঠামো TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
কাঠামো TestvfsFile {
  sqlite3_file base;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#define tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

কাঠামো TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  ধ্রুবক অক্ষর *zFilename;          /* Filename as passed to xOpen() */
  sqlite3_file *pReal;            /* The real, underlying file descriptor */
  Tcl_Obj *pShmId;                /* Shared memory id for Tcl callbacks */

  Testvfs_বাফার *pShm;            /* Shared memory buffer */
  u32 excllock;                   /* Mask of exclusive locks */
  u32 sharedlock;                 /* Mask of shared locks */
  TestvfsFd *p_পরবর্তী;               /* Next handle opened on the same file */
};


#define FAULT_INJECT_NONE       0
#define FAULT_INJECT_TRANSIENT  1
#define FAULT_INJECT_PERSISTENT 2

প্রকারলেখ কাঠামো TestFaultInject TestFaultInject;
কাঠামো TestFaultInject {
  পূর্ণসংখ্যা iCnt;                       /* Remaining calls before fault injection */
  পূর্ণসংখ্যা eFault;                     /* A FAULT_INJECT_* value */
  পূর্ণসংখ্যা nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
কাঠামো Testvfs {
  অক্ষর *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  Testvfs_বাফার *p_বাফার;         /* List of shared buffers */
  পূর্ণসংখ্যা isNoshm;
  পূর্ণসংখ্যা isFullshm;

  পূর্ণসংখ্যা mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#যদি 0
  পূর্ণসংখ্যা iIoerrCnt;
  পূর্ণসংখ্যা ioerr;
  পূর্ণসংখ্যা nIoerrFail;
  পূর্ণসংখ্যা iFullCnt;
  পূর্ণসংখ্যা fullerr;
  পূর্ণসংখ্যা nFullFail;
#endif

  পূর্ণসংখ্যা iDevchar;
  পূর্ণসংখ্যা iSectorsize;
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
কাঠামো Testvfs_বাফার {
  অক্ষর *zFile;                    /* Associated file name */
  পূর্ণসংখ্যা pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  Testvfs_বাফার *p_পরবর্তী;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->p_App_তথ্য))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
স্থির পূর্ণসংখ্যা tvfsClose(sqlite3_file*);
স্থির পূর্ণসংখ্যা tvfsRead(sqlite3_file*, শূন্য*, পূর্ণসংখ্যা iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণসংখ্যা tvfsWrite(sqlite3_file*,ধ্রুবক শূন্য*,পূর্ণসংখ্যা iAmt, sqlite3_int64 iOfst);
স্থির পূর্ণসংখ্যা tvfsTruncate(sqlite3_file*, sqlite3_int64 আকার_2);
স্থির পূর্ণসংখ্যা tvfsSync(sqlite3_file*, পূর্ণসংখ্যা flags);
স্থির পূর্ণসংখ্যা tvfs_File_আকার(sqlite3_file*, sqlite3_int64 *p_আকার);
স্থির পূর্ণসংখ্যা tvfsLock(sqlite3_file*, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা tvfsUnlock(sqlite3_file*, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা tvfsCheckReservedLock(sqlite3_file*, পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা tvfsFileControl(sqlite3_file*, পূর্ণসংখ্যা op, শূন্য *pArg);
স্থির পূর্ণসংখ্যা tvfs_Sector_আকার(sqlite3_file*);
স্থির পূর্ণসংখ্যা tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
স্থির পূর্ণসংখ্যা tvfsOpen(sqlite3_vfs*, ধ্রুবক অক্ষর *, sqlite3_file*, পূর্ণসংখ্যা , পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা tvfsDelete(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা syncDir);
স্থির পূর্ণসংখ্যা tvfsAccess(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা flags, পূর্ণসংখ্যা *);
স্থির পূর্ণসংখ্যা tvfsFullPathname(sqlite3_vfs*, ধ্রুবক অক্ষর *zName, পূর্ণসংখ্যা, অক্ষর *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
স্থির শূন্য *tvfsDlOpen(sqlite3_vfs*, ধ্রুবক অক্ষর *zFilename);
স্থির শূন্য tvfsDlError(sqlite3_vfs*, পূর্ণসংখ্যা nByte, অক্ষর *z_Err_বার্তা);
স্থির শূন্য (*tvfsDlSym(sqlite3_vfs*,শূন্য*, ধ্রুবক অক্ষর *z_প্রতীক))(শূন্য);
স্থির শূন্য tvfsDlClose(sqlite3_vfs*, শূন্য*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
স্থির পূর্ণসংখ্যা tvfsRandomness(sqlite3_vfs*, পূর্ণসংখ্যা nByte, অক্ষর *zOut);
স্থির পূর্ণসংখ্যা tvfsSleep(sqlite3_vfs*, পূর্ণসংখ্যা microseconds);
স্থির পূর্ণসংখ্যা tvfsCurrentTime(sqlite3_vfs*, দ্বিগুণ*);

স্থির পূর্ণসংখ্যা tvfsShmOpen(sqlite3_file*);
স্থির পূর্ণসংখ্যা tvfsShmLock(sqlite3_file*, পূর্ণসংখ্যা , পূর্ণসংখ্যা, পূর্ণসংখ্যা);
স্থির পূর্ণসংখ্যা tvfsShmMap(sqlite3_file*,পূর্ণসংখ্যা,পূর্ণসংখ্যা,পূর্ণসংখ্যা, শূন্য অস্থির **);
স্থির শূন্য tvfsShmBarrier(sqlite3_file*);
স্থির পূর্ণসংখ্যা tvfsShmUnmap(sqlite3_file*, পূর্ণসংখ্যা);

স্থির পূর্ণসংখ্যা tvfsFetch(sqlite3_file*, sqlite3_int64, পূর্ণসংখ্যা, শূন্য**);
স্থির পূর্ণসংখ্যা tvfsUnfetch(sqlite3_file*, sqlite3_int64, শূন্য*);

স্থির sqlite3_io_methods tvfs_io_methods = {
  3,                              /* iVersion */
  tvfsClose,                      /* xClose */
  tvfsRead,                       /* xRead */
  tvfsWrite,                      /* xWrite */
  tvfsTruncate,                   /* xTruncate */
  tvfsSync,                       /* xSync */
  tvfs_File_আকার,                   /* xFileSize */
  tvfsLock,                       /* xLock */
  tvfsUnlock,                     /* xUnlock */
  tvfsCheckReservedLock,          /* xCheckReservedLock */
  tvfsFileControl,                /* xFileControl */
  tvfs_Sector_আকার,                 /* xSectorSize */
  tvfsDeviceCharacteristics,      /* xDeviceCharacteristics */
  tvfsShmMap,                     /* xShmMap */
  tvfsShmLock,                    /* xShmLock */
  tvfsShmBarrier,                 /* xShmBarrier */
  tvfsShmUnmap,                   /* xShmUnmap */
  tvfsFetch,
  tvfsUnfetch
};

স্থির পূর্ণসংখ্যা tvfs_ফলাফল_Code(Testvfs *p, পূর্ণসংখ্যা *pRc){
  কাঠামো errcode {
    পূর্ণসংখ্যা eCode;
    ধ্রুবক অক্ষর *zCode;
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

  ধ্রুবক অক্ষর *z;
  পূর্ণসংখ্যা i;

  z = Tcl_Get_String_ফলাফল(p->interp);
  জন্য(i=0; i<Array_আকার(aCode); i++){
    যদি( 0==strcmp(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      ফেরত 1;
    }
  }

  ফেরত 0;
}

স্থির পূর্ণসংখ্যা tvfsInjectFault(TestFaultInject *p){
  পূর্ণসংখ্যা ret = 0;
  যদি( p->eFault ){
    p->iCnt--;
    যদি( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  ফেরত ret;
}


স্থির পূর্ণসংখ্যা tvfsInjectIoerr(Testvfs *p){
  ফেরত tvfsInjectFault(&p->ioerr_err);
}

স্থির পূর্ণসংখ্যা tvfsInjectFullerr(Testvfs *p){
  ফেরত tvfsInjectFault(&p->full_err);
}
স্থির পূর্ণসংখ্যা tvfsInjectCantopenerr(Testvfs *p){
  ফেরত tvfsInjectFault(&p->cantopen_err);
}


স্থির শূন্য tvfsExecTcl(
  Testvfs *p, 
  ধ্রুবক অক্ষর *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  পূর্ণসংখ্যা rc;                         /* Return code from Tcl_EvalObj() */
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
    Tcl_Reset_ফলাফল(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
স্থির পূর্ণসংখ্যা tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_তথ্য;

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
    ckfree((অক্ষর *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((অক্ষর *)pFd);
  pTestfile->pFd = 0;
  ফেরত SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
স্থির পূর্ণসংখ্যা tvfsRead(
  sqlite3_file *pFile, 
  শূন্য *zBuf, 
  পূর্ণসংখ্যা iAmt, 
  sqlite_int64 iOfst
){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_তথ্য;
  যদি( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_ফলাফল_Code(p, &rc);
  }
  যদি( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  যদি( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  ফেরত rc;
}

/*
** Write data to an tvfs-file.
*/
স্থির পূর্ণসংখ্যা tvfsWrite(
  sqlite3_file *pFile, 
  ধ্রুবক শূন্য *zBuf, 
  পূর্ণসংখ্যা iAmt, 
  sqlite_int64 iOfst
){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_তথ্য;

  যদি( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfs_ফলাফল_Code(p, &rc);
    যদি( rc<0 ) ফেরত SQLITE_OK;
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
  ফেরত rc;
}

/*
** Truncate an tvfs-file.
*/
স্থির পূর্ণসংখ্যা tvfsTruncate(sqlite3_file *pFile, sqlite_int64 আকার_2){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_তথ্য;

  যদি( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_ফলাফল_Code(p, &rc);
  }
  
  যদি( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, আকার_2);
  }
  ফেরত rc;
}

/*
** Sync an tvfs-file.
*/
স্থির পূর্ণসংখ্যা tvfsSync(sqlite3_file *pFile, পূর্ণসংখ্যা flags){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_তথ্য;

  যদি( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    অক্ষর *zFlags = 0;

    নির্বাচন( flags ){
      ক্ষেত্র SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        বিরতি;
      ক্ষেত্র SQLITE_SYNC_FULL:
        zFlags = "full";
        বিরতি;
      ক্ষেত্র SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        বিরতি;
      ক্ষেত্র SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        বিরতি;
      পূর্বনির্ধারিত:
        প্রতিজ্ঞা(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfs_ফলাফল_Code(p, &rc);
  }

  যদি( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  যদি( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  ফেরত rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
স্থির পূর্ণসংখ্যা tvfs_File_আকার(sqlite3_file *pFile, sqlite_int64 *p_আকার){
  TestvfsFd *p = tvfsGetFd(pFile);
  ফেরত sqlite3_Os_File_আকার(p->pReal, p_আকার);
}

/*
** Lock an tvfs-file.
*/
স্থির পূর্ণসংখ্যা tvfsLock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_তথ্য;
  যদি( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    অক্ষর zLock[30];
    sqlite3_snprintf(আকার(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  যদি( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    ফেরত SQLITE_IOERR_LOCK;
  }
  ফেরত sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
স্থির পূর্ণসংখ্যা tvfsUnlock(sqlite3_file *pFile, পূর্ণসংখ্যা eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_তথ্য;
  যদি( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    অক্ষর zLock[30];
    sqlite3_snprintf(আকার(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  যদি( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    ফেরত SQLITE_IOERR_UNLOCK;
  }
  ফেরত sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
স্থির পূর্ণসংখ্যা tvfsCheckReservedLock(sqlite3_file *pFile, পূর্ণসংখ্যা *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_তথ্য;
  যদি( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  ফেরত sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
স্থির পূর্ণসংখ্যা tvfsFileControl(sqlite3_file *pFile, পূর্ণসংখ্যা op, শূন্য *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_তথ্য;
  যদি( op==SQLITE_FCNTL_PRAGMA ){
    অক্ষর **argv = (অক্ষর**)pArg;
    যদি( sqlite3_stricmp(argv[1],"error")==0 ){
      পূর্ণসংখ্যা rc = SQLITE_ERROR;
      যদি( argv[2] ){
        ধ্রুবক অক্ষর *z = argv[2];
        পূর্ণসংখ্যা x = atoi(z);
        যদি( x ){
          rc = x;
          যখন( sqlite3Isdigit(z[0]) ){ z++; }
          যখন( sqlite3Isspace(z[0]) ){ z++; }
        }
        যদি( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      ফেরত rc;
    }
    যদি( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      ফেরত SQLITE_OK;
    }
  }
  যদি( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    কাঠামো Fcntl {
      পূর্ণসংখ্যা iFnctl;
      ধ্রুবক অক্ষর *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    পূর্ণসংখ্যা i;
    জন্য(i=0; i<আকার(aF)/আকার(aF[0]); i++){
      যদি( op==aF[i].iFnctl ) বিরতি;
    }
    যদি( i<আকার(aF)/আকার(aF[0]) ){
      পূর্ণসংখ্যা rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfs_ফলাফল_Code(p, &rc);
      যদি( rc ) ফেরত (rc<0 ? SQLITE_OK : rc);
    }
  }
  ফেরত sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
স্থির পূর্ণসংখ্যা tvfs_Sector_আকার(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_তথ্য;
  যদি( p->iSectorsize>=0 ){
    ফেরত p->iSectorsize;
  }
  ফেরত sqlite3_Os_Sector_আকার(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
স্থির পূর্ণসংখ্যা tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_তথ্য;
  যদি( p->iDevchar>=0 ){
    ফেরত p->iDevchar;
  }
  ফেরত sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
স্থির পূর্ণসংখ্যা tvfsOpen(
  sqlite3_vfs *pVfs,
  ধ্রুবক অক্ষর *zName,
  sqlite3_file *pFile,
  পূর্ণসংখ্যা flags,
  পূর্ণসংখ্যা *pOutFlags
){
  পূর্ণসংখ্যা rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->p_App_তথ্য;

  pFd = (TestvfsFd *)ckalloc(আকার(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, আকার(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, আকার(TestvfsFile));
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
  Tcl_Reset_ফলাফল(p->interp);
  যদি( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_গুনতি(pArg);
    যদি( flags&SQLITE_OPEN_MAIN_DB ){
      ধ্রুবক অক্ষর *z = &zName[strlen(zName)+1];
      যখন( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_গুনতি(pArg);
    যদি( tvfs_ফলাফল_Code(p, &rc) ){
      যদি( rc!=SQLITE_OK ) ফেরত rc;
    }অন্যথা{
      pId = Tcl_Get_Obj_ফলাফল(p->interp);
    }
  }

  যদি( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) ফেরত SQLITE_IOERR;
  যদি( tvfsInjectCantopenerr(p) ) ফেরত SQLITE_CANTOPEN;
  যদি( tvfsInjectFullerr(p) ) ফেরত SQLITE_FULL;

  যদি( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_গুনতি(pId);
  pFd->pShmId = pId;
  Tcl_Reset_ফলাফল(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  যদি( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    পূর্ণসংখ্যা nByte;

    যদি( pVfs->iVersion>1 ){
      nByte = আকার(sqlite3_io_methods);
    }অন্যথা{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    memcpy(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    যদি( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    যদি( pVfs->iVersion>1 && ((Testvfs *)pVfs->p_App_তথ্য)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  ফেরত rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
স্থির পূর্ণসংখ্যা tvfsDelete(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath, পূর্ণসংখ্যা dirSync){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->p_App_তথ্য;

  যদি( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfs_ফলাফল_Code(p, &rc);
  }
  যদি( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  ফেরত rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
স্থির পূর্ণসংখ্যা tvfsAccess(
  sqlite3_vfs *pVfs, 
  ধ্রুবক অক্ষর *zPath, 
  পূর্ণসংখ্যা flags, 
  পূর্ণসংখ্যা *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->p_App_তথ্য;
  যদি( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    পূর্ণসংখ্যা rc;
    অক্ষর *zArg = 0;
    যদি( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    যদি( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    যদি( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    যদি( tvfs_ফলাফল_Code(p, &rc) ){
      যদি( rc!=SQLITE_OK ) ফেরত rc;
    }অন্যথা{
      Tcl_Interp *interp = p->interp;
      যদি( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_Get_Obj_ফলাফল(interp), pResOut) ){
        ফেরত SQLITE_OK;
      }
    }
  }
  ফেরত sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
স্থির পূর্ণসংখ্যা tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  ধ্রুবক অক্ষর *zPath, 
  পূর্ণসংখ্যা nOut, 
  অক্ষর *zOut
){
  Testvfs *p = (Testvfs *)pVfs->p_App_তথ্য;
  যদি( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    পূর্ণসংখ্যা rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    যদি( tvfs_ফলাফল_Code(p, &rc) ){
      যদি( rc!=SQLITE_OK ) ফেরত rc;
    }
  }
  ফেরত sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
স্থির শূন্য *tvfsDlOpen(sqlite3_vfs *pVfs, ধ্রুবক অক্ষর *zPath){
  ফেরত sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
স্থির শূন্য tvfsDlError(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nByte, অক্ষর *z_Err_বার্তা){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_বার্তা);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
স্থির শূন্য (*tvfsDlSym(sqlite3_vfs *pVfs, শূন্য *p, ধ্রুবক অক্ষর *zSym))(শূন্য){
  ফেরত sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
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
স্থির পূর্ণসংখ্যা tvfsRandomness(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nByte, অক্ষর *zBufOut){
  ফেরত sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
স্থির পূর্ণসংখ্যা tvfsSleep(sqlite3_vfs *pVfs, পূর্ণসংখ্যা nMicro){
  Testvfs *p = (Testvfs *)pVfs->p_App_তথ্য;
  যদি( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  ফেরত sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
স্থির পূর্ণসংখ্যা tvfsCurrentTime(sqlite3_vfs *pVfs, দ্বিগুণ *pTimeOut){
  ফেরত PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

স্থির পূর্ণসংখ্যা tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  পূর্ণসংখ্যা rc = SQLITE_OK;             /* Return code */
  Testvfs_বাফার *p_বাফার;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->p_App_তথ্য;
  প্রতিজ্ঞা( 0==p->isFullshm );
  প্রতিজ্ঞা( pFd->pShmId && pFd->pShm==0 && pFd->p_পরবর্তী==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_Reset_ফলাফল(p->interp);
  যদি( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    যদি( tvfs_ফলাফল_Code(p, &rc) ){
      যদি( rc!=SQLITE_OK ) ফেরত rc;
    }
  }

  প্রতিজ্ঞা( rc==SQLITE_OK );
  যদি( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    ফেরত SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  জন্য(p_বাফার=p->p_বাফার; p_বাফার; p_বাফার=p_বাফার->p_পরবর্তী){
    যদি( 0==strcmp(pFd->zFilename, p_বাফার->zFile) ) বিরতি;
  }
  যদি( !p_বাফার ){
    পূর্ণসংখ্যা szName = (পূর্ণসংখ্যা)strlen(pFd->zFilename);
    পূর্ণসংখ্যা nByte = আকার(Testvfs_বাফার) + szName + 1;
    p_বাফার = (Testvfs_বাফার *)ckalloc(nByte);
    memset(p_বাফার, 0, nByte);
    p_বাফার->zFile = (অক্ষর *)&p_বাফার[1];
    memcpy(p_বাফার->zFile, pFd->zFilename, szName+1);
    p_বাফার->p_পরবর্তী = p->p_বাফার;
    p->p_বাফার = p_বাফার;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->p_পরবর্তী = p_বাফার->pFile;
  p_বাফার->pFile = pFd;
  pFd->pShm = p_বাফার;
  ফেরত rc;
}

স্থির শূন্য tvfsAllocPage(Testvfs_বাফার *p, পূর্ণসংখ্যা iPage, পূর্ণসংখ্যা pgsz){
  প্রতিজ্ঞা( iPage<TESTVFS_MAX_PAGES );
  যদি( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    memset(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

স্থির পূর্ণসংখ্যা tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  পূর্ণসংখ্যা iPage,                      /* Page to retrieve */
  পূর্ণসংখ্যা pgsz,                       /* Size of pages */
  পূর্ণসংখ্যা isWrite,                    /* True to extend file if necessary */
  শূন্য অস্থির **pp              /* OUT: Mapped memory */
){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_তথ্য);

  যদি( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ফেরত pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  যদি( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    যদি( rc!=SQLITE_OK ){
      ফেরত rc;
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
    tvfs_ফলাফল_Code(p, &rc);
    Tcl_Decr_Ref_গুনতি(pArg);
  }
  যদি( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  যদি( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  যদি( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (শূন্য অস্থির *)pFd->pShm->aPage[iPage];
  }

  ফেরত rc;
}


স্থির পূর্ণসংখ্যা tvfsShmLock(
  sqlite3_file *pFile,
  পূর্ণসংখ্যা ofst,
  পূর্ণসংখ্যা n,
  পূর্ণসংখ্যা flags
){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_তথ্য);
  পূর্ণসংখ্যা nLock;
  অক্ষর zLock[80];

  যদি( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ফেরত pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  যদি( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(আকার(zLock), zLock, "%d %d", ofst, n);
    nLock = (পূর্ণসংখ্যা)strlen(zLock);
    যদি( flags & SQLITE_SHM_LOCK ){
      strcpy(&zLock[nLock], " lock");
    }অন্যথা{
      strcpy(&zLock[nLock], " unlock");
    }
    nLock += (পূর্ণসংখ্যা)strlen(&zLock[nLock]);
    যদি( flags & SQLITE_SHM_SHARED ){
      strcpy(&zLock[nLock], " shared");
    }অন্যথা{
      strcpy(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfs_ফলাফল_Code(p, &rc);
  }

  যদি( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  যদি( rc==SQLITE_OK ){
    পূর্ণসংখ্যা isLock = (flags & SQLITE_SHM_LOCK);
    পূর্ণসংখ্যা isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    যদি( isLock ){
      TestvfsFd *p2;
      জন্য(p2=pFd->pShm->pFile; p2; p2=p2->p_পরবর্তী){
        যদি( p2==pFd ) চালু;
        যদি( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          বিরতি;
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

  ফেরত rc;
}

স্থির শূন্য tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_তথ্য);

  যদি( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    ধ্রুবক অক্ষর *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  যদি( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    ফেরত;
  }
}

স্থির পূর্ণসংখ্যা tvfsShmUnmap(
  sqlite3_file *pFile,
  পূর্ণসংখ্যা deleteFlag
){
  পূর্ণসংখ্যা rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_তথ্য);
  Testvfs_বাফার *p_বাফার = pFd->pShm;
  TestvfsFd **ppFd;

  যদি( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ফেরত pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  যদি( !p_বাফার ) ফেরত SQLITE_OK;
  প্রতিজ্ঞা( pFd->pShmId && pFd->pShm );

  যদি( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfs_ফলাফল_Code(p, &rc);
  }

  জন্য(ppFd=&p_বাফার->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->p_পরবর্তী));
  প্রতিজ্ঞা( (*ppFd)==pFd );
  *ppFd = pFd->p_পরবর্তী;
  pFd->p_পরবর্তী = 0;

  যদি( p_বাফার->pFile==0 ){
    পূর্ণসংখ্যা i;
    Testvfs_বাফার **pp;
    জন্য(pp=&p->p_বাফার; *pp!=p_বাফার; pp=&((*pp)->p_পরবর্তী));
    *pp = (*pp)->p_পরবর্তী;
    জন্য(i=0; p_বাফার->aPage[i]; i++){
      ckfree((অক্ষর *)p_বাফার->aPage[i]);
    }
    ckfree((অক্ষর *)p_বাফার);
  }
  pFd->pShm = 0;

  ফেরত rc;
}

স্থির পূর্ণসংখ্যা tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    পূর্ণসংখ্যা iAmt, 
    শূন্য **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  ফেরত sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

স্থির পূর্ণসংখ্যা tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, শূন্য *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  ফেরত sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI testvfs_obj_cmd(
  Client_তথ্য cd,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  গণনা DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  কাঠামো TestvfsSubcmd {
    অক্ষর *zName;
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
  পূর্ণসংখ্যা i;
  
  যদি( objc<2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "SUBCOMMAND ...");
    ফেরত TCL_ERROR;
  }
  যদি( Tcl_Get_সূচক_From_Obj_Struct(
        interp, objv[1], aSubcmd, আকার(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    ফেরত TCL_ERROR;
  }
  Tcl_Reset_ফলাফল(interp);

  নির্বাচন( aSubcmd[i].eCmd ){
    ক্ষেত্র CMD_SHM: {
      Tcl_Obj *pObj;
      পূর্ণসংখ্যা rc;
      Testvfs_বাফার *p_বাফার;
      অক্ষর *zName;
      যদি( objc!=3 && objc!=4 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "FILE ?VALUE?");
        ফেরত TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      যদি( rc!=SQLITE_OK ){
        Tcl_Append_ফলাফল(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), ফাঁকা);
        ckfree(zName);
        ফেরত TCL_ERROR;
      }
      জন্য(p_বাফার=p->p_বাফার; p_বাফার; p_বাফার=p_বাফার->p_পরবর্তী){
        যদি( 0==strcmp(p_বাফার->zFile, zName) ) বিরতি;
      }
      ckfree(zName);
      যদি( !p_বাফার ){
        Tcl_Append_ফলাফল(interp, "no such file: ", Tcl_GetString(objv[2]), ফাঁকা);
        ফেরত TCL_ERROR;
      }
      যদি( objc==4 ){
        Tcl_আকার n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        পূর্ণসংখ্যা pgsz = p_বাফার->pgsz;
        যদি( pgsz==0 ) pgsz = 65536;
        জন্য(i=0; i*pgsz<(পূর্ণসংখ্যা)n; i++){
          পূর্ণসংখ্যা nByte = pgsz;
          tvfsAllocPage(p_বাফার, i, pgsz);
          যদি( n-i*pgsz<pgsz ){
            nByte = (পূর্ণসংখ্যা)n;
          }
          memcpy(p_বাফার->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      জন্য(i=0; p_বাফার->aPage[i]; i++){
        পূর্ণসংখ্যা pgsz = p_বাফার->pgsz;
        যদি( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(p_বাফার->aPage[i], pgsz));
      }
      Tcl_Set_Obj_ফলাফল(interp, pObj);
      বিরতি;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    ক্ষেত্র CMD_FILTER: {
      স্থির কাঠামো VfsMethod {
        অক্ষর *zName;
        পূর্ণসংখ্যা mask;
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
      Tcl_আকার nElem = 0;
      পূর্ণসংখ্যা mask = 0;
      যদি( objc!=3 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "LIST");
        ফেরত TCL_ERROR;
      }
      যদি( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        ফেরত TCL_ERROR;
      }
      Tcl_Reset_ফলাফল(interp);
      জন্য(i=0; i<(পূর্ণসংখ্যা)nElem; i++){
        পূর্ণসংখ্যা iMethod;
        অক্ষর *zElem = Tcl_GetString(apElem[i]);
        জন্য(iMethod=0; iMethod<Array_আকার(vfsmethod); iMethod++){
          যদি( strcmp(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            বিরতি;
          }
        }
        যদি( iMethod==Array_আকার(vfsmethod) ){
          Tcl_Append_ফলাফল(interp, "unknown method: ", zElem, ফাঁকা);
          ফেরত TCL_ERROR;
        }
      }
      p->mask = mask;
      বিরতি;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    ক্ষেত্র CMD_SCRIPT: {
      যদি( objc==3 ){
        Tcl_আকার nByte;
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
        ফেরত TCL_ERROR;
      }

      Tcl_Reset_ফলাফল(interp);
      যদি( p->pScript ) Tcl_Set_Obj_ফলাফল(interp, p->pScript);

      বিরতি;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    ক্ষেত্র CMD_CANTOPENERR:
    ক্ষেত্র CMD_IOERR:
    ক্ষেত্র CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      পূর্ণসংখ্যা iRet;

      নির্বাচন( aSubcmd[i].eCmd ){
        ক্ষেত্র CMD_IOERR: pTest = &p->ioerr_err; বিরতি;
        ক্ষেত্র CMD_FULLERR: pTest = &p->full_err; বিরতি;
        ক্ষেত্র CMD_CANTOPENERR: pTest = &p->cantopen_err; বিরতি;
        পূর্বনির্ধারিত: প্রতিজ্ঞা(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      যদি( objc==4 ){
        পূর্ণসংখ্যা iCnt, iPersist;
        যদি( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          ফেরত TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }অন্যথা যদি( objc!=2 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?CNT PERSIST?");
        ফেরত TCL_ERROR;
      }
      Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(iRet));
      বিরতি;
    }

    ক্ষেত্র CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      বিরতি;
    }

    ক্ষেত্র CMD_DEVCHAR: {
      কাঠামো DeviceFlag {
        অক্ষর *zName;
        পূর্ণসংখ্যা i_মান;
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
      পূর্ণসংখ্যা iFlag;

      যদি( objc>3 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?ATTR-LIST?");
        ফেরত TCL_ERROR;
      }
      যদি( objc==3 ){
        পূর্ণসংখ্যা j;
        পূর্ণসংখ্যা iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_আকার nFlags = 0;

        যদি( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          ফেরত TCL_ERROR;
        }

        জন্য(j=0; j<(পূর্ণসংখ্যা)nFlags; j++){
          পূর্ণসংখ্যা idx = 0;
          যদি( Tcl_Get_সূচক_From_Obj_Struct(interp, flags[j], aFlag, 
                আকার(aFlag[0]), "flag", 0, &idx) 
          ){
            ফেরত TCL_ERROR;
          }
          যদি( aFlag[idx].i_মান<0 && nFlags>1 ){
            Tcl_Append_ফলাফল(interp, "bad flags: ", Tcl_GetString(objv[2]), ফাঁকা);
            ফেরত TCL_ERROR;
          }
          iNew |= aFlag[idx].i_মান;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      জন্য(iFlag=0; iFlag<আকার(aFlag)/আকার(aFlag[0]); iFlag++){
        যদি( p->iDevchar & aFlag[iFlag].i_মান ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_Set_Obj_ফলাফল(interp, pRet);

      বিরতি;
    }

    ক্ষেত্র CMD_SECTORSIZE: {
      যদি( objc>3 ){
        Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, "?VALUE?");
        ফেরত TCL_ERROR;
      }
      যদি( objc==3 ){
        পূর্ণসংখ্যা iNew = 0;
        যদি( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          ফেরত TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(p->iSectorsize));
      বিরতি;
    }
  }

  ফেরত TCL_OK;
}

স্থির শূন্য SQLITE_TCLAPI testvfs_obj_del(Client_তথ্য cd){
  Testvfs *p = (Testvfs *)cd;
  যদি( p->pScript ) Tcl_Decr_Ref_গুনতি(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, আকার(sqlite3_vfs));
  ckfree((অক্ষর *)p->pVfs);
  memset(p, 0, আকার(Testvfs));
  ckfree((অক্ষর *)p);
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
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI testvfs_cmd(
  Client_তথ্য cd,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
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
  অক্ষর *zVfs;
  পূর্ণসংখ্যা nByte;                      /* Bytes of space to allocate at p */

  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা isNoshm = 0;                /* True if -noshm is passed */
  পূর্ণসংখ্যা isFullshm = 0;              /* True if -fullshm is passed */
  পূর্ণসংখ্যা isDefault = 0;              /* True if -default is passed */
  পূর্ণসংখ্যা szOsFile = 0;               /* Value passed to -szosfile */
  পূর্ণসংখ্যা mxPathname = -1;            /* Value passed to -mxpathname */
  পূর্ণসংখ্যা iVersion = 3;               /* Value passed to -iversion */

  যদি( objc<2 || 0!=(objc%2) ) যাও bad_args;
  জন্য(i=2; i<objc; i += 2){
    Tcl_আকার nSwitch;
    অক্ষর *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    যদি( nSwitch>2 && 0==strncmp("-noshm", zSwitch, nSwitch) ){
      যদি( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        ফেরত TCL_ERROR;
      }
      যদি( isNoshm ) isFullshm = 0;
    }
    অন্যথা যদি( nSwitch>2 && 0==strncmp("-default", zSwitch, nSwitch) ){
      যদি( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        ফেরত TCL_ERROR;
      }
    }
    অন্যথা যদি( nSwitch>2 && 0==strncmp("-szosfile", zSwitch, nSwitch) ){
      যদি( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        ফেরত TCL_ERROR;
      }
    }
    অন্যথা যদি( nSwitch>2 && 0==strncmp("-mxpathname", zSwitch, nSwitch) ){
      যদি( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        ফেরত TCL_ERROR;
      }
    }
    অন্যথা যদি( nSwitch>2 && 0==strncmp("-iversion", zSwitch, nSwitch) ){
      যদি( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        ফেরত TCL_ERROR;
      }
    }
    অন্যথা যদি( nSwitch>2 && 0==strncmp("-fullshm", zSwitch, nSwitch) ){
      যদি( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        ফেরত TCL_ERROR;
      }
      যদি( isFullshm ) isNoshm = 0;
    }
    অন্যথা{
      যাও bad_args;
    }
  }

  যদি( szOsFile<আকার(TestvfsFile) ){
    szOsFile = আকার(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = আকার(Testvfs) + (পূর্ণসংখ্যা)strlen(zVfs)+1;
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

  p->zName = (অক্ষর *)&p[1];
  memcpy(p->zName, zVfs, strlen(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(আকার(sqlite3_vfs));
  memcpy(pVfs, &tvfs_vfs, আকার(sqlite3_vfs));
  pVfs->p_App_তথ্য = (শূন্য *)p;
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

  ফেরত TCL_OK;

 bad_args:
  Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  ফেরত TCL_ERROR;
}

বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp *interp, ধ্রুবক অক্ষর *zA, sqlite3 **ppDb);
বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrName(পূর্ণসংখ্যা);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_vfs_shmlock(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  ধ্রুবক অক্ষর *azArg1[] = {"shared", "exclusive", 0};
  ধ্রুবক অক্ষর *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  ধ্রুবক অক্ষর *zDbname = 0;
  পূর্ণসংখ্যা iArg1 = 0;
  পূর্ণসংখ্যা iArg2 = 0;
  পূর্ণসংখ্যা iOffset = 0;
  পূর্ণসংখ্যা n = 0;
  sqlite3_file *pFd;

  যদি( objc!=7 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    ফেরত TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_Get_সূচক_From_Obj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_Get_সূচক_From_Obj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    ফেরত TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (শূন্য*)&pFd);
  যদি( pFd==0 ){
    ফেরত TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI test_vfs_set_readmark(
  শূন্য * client_তথ্য,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  পূর্ণসংখ্যা rc = SQLITE_OK;
  ধ্রুবক অক্ষর *zDbname = 0;
  পূর্ণসংখ্যা iSlot = 0;
  পূর্ণসংখ্যা iVal = -1;
  sqlite3_file *pFd;
  শূন্য অস্থির *pShm = 0;
  u32 *aShm;
  পূর্ণসংখ্যা iOff;

  যদি( objc!=4 && objc!=5 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    ফেরত TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    ফেরত TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (শূন্য*)&pFd);
  যদি( pFd==0 ){
    ফেরত TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  যদি( rc!=SQLITE_OK ){
    Tcl_Set_Obj_ফলাফল(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ফেরত TCL_ERROR;
  }
  যদি( pShm==0 ){
    Tcl_Append_ফলাফল(interp, "*-shm is not yet mapped", ফাঁকা);
    ফেরত TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  যদি( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(aShm[iOff]));

  ফেরত TCL_OK;
}

পূর্ণসংখ্যা Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  ফেরত TCL_OK;
}

#endif
