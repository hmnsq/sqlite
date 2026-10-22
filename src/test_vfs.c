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
تعريفنوع بنية Testvfs_میانگیر Testvfs_میانگیر;
تعريفنوع بنية TestvfsFile TestvfsFile;
تعريفنوع بنية TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
بنية TestvfsFile {
  sqlite3_file پایه;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#تعریف_کن tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

بنية TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  ثابت حرف *zFilename;          /* Filename as passed to xOpen() */
  sqlite3_file *pReal;            /* The real, underlying file descriptor */
  Tcl_Obj *pShmId;                /* Shared memory id for Tcl callbacks */

  Testvfs_میانگیر *pShm;            /* Shared memory buffer */
  u32 excllock;                   /* Mask of exclusive locks */
  u32 sharedlock;                 /* Mask of shared locks */
  TestvfsFd *p_بعدی;               /* Next handle opened on the same file */
};


#تعریف_کن FAULT_INJECT_NONE       0
#تعریف_کن FAULT_INJECT_TRANSIENT  1
#تعریف_کن FAULT_INJECT_PERSISTENT 2

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
  sqlite3_vfs *p_والد;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  Testvfs_میانگیر *p_میانگیر;         /* List of shared buffers */
  صحيح isNoshm;
  صحيح isFullshm;

  صحيح ماسک;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject پر_err;
  TestFaultInject cantopen_err;

#إذا 0
  صحيح iIoerrCnt;
  صحيح ioerr;
  صحيح nIoerrFail;
  صحيح i_پر_Cnt;
  صحيح fullerr;
  صحيح n_پر_Fail;
#پایان_اگر

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
#تعریف_کن TESTVFS_SHMOPEN_ماسک      0x00000001
#تعریف_کن TESTVFS_SHMLOCK_ماسک      0x00000010
#تعریف_کن TESTVFS_SHMMAP_ماسک       0x00000020
#تعریف_کن TESTVFS_SHMBARRIER_ماسک   0x00000040
#تعریف_کن TESTVFS_SHMCLOSE_ماسک     0x00000080

#تعریف_کن TESTVFS_OPEN_ماسک         0x00000100
#تعریف_کن TESTVFS_SYNC_ماسک         0x00000200
#تعریف_کن TESTVFS_DELETE_ماسک       0x00000400
#تعریف_کن TESTVFS_CLOSE_ماسک        0x00000800
#تعریف_کن TESTVFS_WRITE_ماسک        0x00001000
#تعریف_کن TESTVFS_TRUNCATE_ماسک     0x00002000
#تعریف_کن TESTVFS_ACCESS_ماسک       0x00004000
#تعریف_کن TESTVFS_FULLPATHNAME_ماسک 0x00008000
#تعریف_کن TESTVFS_READ_ماسک         0x00010000
#تعریف_کن TESTVFS_UNLOCK_ماسک       0x00020000
#تعریف_کن TESTVFS_قفل_ماسک         0x00040000
#تعریف_کن TESTVFS_CKLOCK_ماسک       0x00080000
#تعریف_کن TESTVFS_FCNTL_ماسک        0x00100000
#تعریف_کن TESTVFS_SLEEP_ماسک        0x00200000

#تعریف_کن TESTVFS_ALL_ماسک          0x003FFFFF


#تعریف_کن TESTVFS_MAX_صفحات 1024

/*
** A shared-memory buffer. There is one of these objects for each shared
** memory region opened by clients. If two clients open the same file,
** there are two TestvfsFile structures but only one TestvfsBuffer structure.
*/
بنية Testvfs_میانگیر {
  حرف *zFile;                    /* Associated file name */
  صحيح pgsz;                       /* Page size */
  u8 *a_صفحه[TESTVFS_MAX_صفحات];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  Testvfs_میانگیر *p_بعدی;           /* Next in linked list of all buffers */
};


#تعریف_کن PARENTVFS(x) (((Testvfs *)((x)->p_App_داده))->p_والد)

#تعریف_کن TESTVFS_MAX_آرگومانها 12


/*
** Method declarations for TestvfsFile.
*/
ساكن صحيح tvfsClose(sqlite3_file*);
ساكن صحيح tvfsRead(sqlite3_file*, فراغ*, صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح tvfsWrite(sqlite3_file*,ثابت فراغ*,صحيح iAmt, sqlite3_int64 iOfst);
ساكن صحيح tvfsTruncate(sqlite3_file*, sqlite3_int64 اندازه);
ساكن صحيح tvfsSync(sqlite3_file*, صحيح flags);
ساكن صحيح tvfs_File_اندازه(sqlite3_file*, sqlite3_int64 *p_اندازه);
ساكن صحيح tvfs_قفل(sqlite3_file*, صحيح);
ساكن صحيح tvfsUnlock(sqlite3_file*, صحيح);
ساكن صحيح tvfs_Check_Reserved_قفل(sqlite3_file*, صحيح *);
ساكن صحيح tvfsFileControl(sqlite3_file*, صحيح op, فراغ *pArg);
ساكن صحيح tvfs_Sector_اندازه(sqlite3_file*);
ساكن صحيح tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
ساكن صحيح tvfsOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحيح , صحيح *);
ساكن صحيح tvfsDelete(sqlite3_vfs*, ثابت حرف *zName, صحيح syncDir);
ساكن صحيح tvfsAccess(sqlite3_vfs*, ثابت حرف *zName, صحيح flags, صحيح *);
ساكن صحيح tvfs_پر_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحيح, حرف *z_بیرون);
#اگر_تعریف_نشده SQLITE_OMIT_بارگیری_EXTENSION
ساكن فراغ *tvfsDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساكن فراغ tvfsDlError(sqlite3_vfs*, صحيح nByte, حرف *z_Err_پیام);
ساكن فراغ (*tvfsDlSym(sqlite3_vfs*,فراغ*, ثابت حرف *z_نماد))(فراغ);
ساكن فراغ tvfsDlClose(sqlite3_vfs*, فراغ*);
#پایان_اگر /* SQLITE_OMIT_LOAD_EXTENSION */
ساكن صحيح tvfsRandomness(sqlite3_vfs*, صحيح nByte, حرف *z_بیرون);
ساكن صحيح tvfsSleep(sqlite3_vfs*, صحيح microseconds);
ساكن صحيح tvfs_Current_زمان(sqlite3_vfs*, مزدوج*);

ساكن صحيح tvfsShmOpen(sqlite3_file*);
ساكن صحيح tvfs_Shm_قفل(sqlite3_file*, صحيح , صحيح, صحيح);
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
  tvfs_File_اندازه,                   /* xFileSize */
  tvfs_قفل,                       /* xLock */
  tvfsUnlock,                     /* xUnlock */
  tvfs_Check_Reserved_قفل,          /* xCheckReservedLock */
  tvfsFileControl,                /* xFileControl */
  tvfs_Sector_اندازه,                 /* xSectorSize */
  tvfsDeviceCharacteristics,      /* xDeviceCharacteristics */
  tvfsShmMap,                     /* xShmMap */
  tvfs_Shm_قفل,                    /* xShmLock */
  tvfsShmBarrier,                 /* xShmBarrier */
  tvfsShmUnmap,                   /* xShmUnmap */
  tvfsFetch,
  tvfsUnfetch
};

ساكن صحيح tvfs_نتیجه_کد(Testvfs *p, صحيح *pRc){
  بنية errcode {
    صحيح e_کد;
    ثابت حرف *z_کد;
  } a_کد[] = {
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

  z = Tcl_Get_String_نتیجه(p->interp);
  لكل(i=0; i<Array_اندازه(a_کد); i++){
    إذا( 0==strcmp(z, a_کد[i].z_کد) ){
      *pRc = a_کد[i].e_کد;
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
  أرجع tvfsInjectFault(&p->پر_err);
}
ساكن صحيح tvfsInjectCantopenerr(Testvfs *p){
  أرجع tvfsInjectFault(&p->cantopen_err);
}


ساكن فراغ tvfsExecTcl(
  Testvfs *p, 
  ثابت حرف *z_روش,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  صحيح rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  أكد( p->pScript );

  أكد( z_روش );
  أكد( p );
  أكد( arg2==0 || arg1!=0 );
  أكد( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_شمار(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(z_روش, -1));
  إذا( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  إذا( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  إذا( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  إذا( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  إذا( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_Reset_نتیجه(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
ساكن صحيح tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_داده;

  إذا( p->pScript && p->ماسک&TESTVFS_CLOSE_ماسک ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  إذا( pFd->pShmId ){
    Tcl_Decr_Ref_شمار(pFd->pShmId);
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
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_داده;
  إذا( p->pScript && p->ماسک&TESTVFS_READ_ماسک ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_نتیجه_کد(p, &rc);
  }
  إذا( rc==SQLITE_OK && p->ماسک&TESTVFS_READ_ماسک && tvfsInjectIoerr(p) ){
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
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_داده;

  إذا( p->pScript && p->ماسک&TESTVFS_WRITE_ماسک ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfs_نتیجه_کد(p, &rc);
    إذا( rc<0 ) أرجع SQLITE_OK;
  }

  إذا( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_پر;
  }
  إذا( rc==SQLITE_OK && p->ماسک&TESTVFS_WRITE_ماسک && tvfsInjectIoerr(p) ){
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
ساكن صحيح tvfsTruncate(sqlite3_file *pFile, sqlite_int64 اندازه){
  صحيح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_داده;

  إذا( p->pScript && p->ماسک&TESTVFS_TRUNCATE_ماسک ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_نتیجه_کد(p, &rc);
  }
  
  إذا( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, اندازه);
  }
  أرجع rc;
}

/*
** Sync an tvfs-file.
*/
ساكن صحيح tvfsSync(sqlite3_file *pFile, صحيح flags){
  صحيح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_داده;

  إذا( p->pScript && p->ماسک&TESTVFS_SYNC_ماسک ){
    حرف *zFlags = 0;

    بدل( flags ){
      حالة SQLITE_SYNC_نرمال:
        zFlags = "normal";
        اكسر;
      حالة SQLITE_SYNC_پر:
        zFlags = "full";
        اكسر;
      حالة SQLITE_SYNC_نرمال|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        اكسر;
      حالة SQLITE_SYNC_پر|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        اكسر;
      افتراضي:
        أكد(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfs_نتیجه_کد(p, &rc);
  }

  إذا( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_پر;

  إذا( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  أرجع rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
ساكن صحيح tvfs_File_اندازه(sqlite3_file *pFile, sqlite_int64 *p_اندازه){
  TestvfsFd *p = tvfsGetFd(pFile);
  أرجع sqlite3_Os_File_اندازه(p->pReal, p_اندازه);
}

/*
** Lock an tvfs-file.
*/
ساكن صحيح tvfs_قفل(sqlite3_file *pFile, صحيح e_قفل){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_داده;
  إذا( p->pScript && p->ماسک&TESTVFS_قفل_ماسک ){
    حرف z_قفل[30];
    sqlite3_snprintf(حجم(z_قفل),z_قفل,"%d",e_قفل);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(z_قفل, -1), 0, 0);
  }
  إذا( p->ماسک&TESTVFS_قفل_ماسک && tvfsInjectIoerr(p) ){
    أرجع SQLITE_IOERR_قفل;
  }
  أرجع sqlite3_Os_قفل(pFd->pReal, e_قفل);
}

/*
** Unlock an tvfs-file.
*/
ساكن صحيح tvfsUnlock(sqlite3_file *pFile, صحيح e_قفل){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_داده;
  إذا( p->pScript && p->ماسک&TESTVFS_UNLOCK_ماسک ){
    حرف z_قفل[30];
    sqlite3_snprintf(حجم(z_قفل),z_قفل,"%d",e_قفل);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(z_قفل, -1), 0, 0);
  }
  إذا( p->ماسک&TESTVFS_UNLOCK_ماسک && tvfsInjectIoerr(p) ){
    أرجع SQLITE_IOERR_UNLOCK;
  }
  أرجع sqlite3OsUnlock(pFd->pReal, e_قفل);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
ساكن صحيح tvfs_Check_Reserved_قفل(sqlite3_file *pFile, صحيح *p_Res_بیرون){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_داده;
  إذا( p->pScript && p->ماسک&TESTVFS_CKLOCK_ماسک ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  أرجع sqlite3_Os_Check_Reserved_قفل(pFd->pReal, p_Res_بیرون);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
ساكن صحيح tvfsFileControl(sqlite3_file *pFile, صحيح op, فراغ *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_داده;
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
  إذا( p->pScript && (p->ماسک&TESTVFS_FCNTL_ماسک) ){
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
      tvfs_نتیجه_کد(p, &rc);
      إذا( rc ) أرجع (rc<0 ? SQLITE_OK : rc);
    }
  }
  أرجع sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
ساكن صحيح tvfs_Sector_اندازه(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_داده;
  إذا( p->iSectorsize>=0 ){
    أرجع p->iSectorsize;
  }
  أرجع sqlite3_Os_Sector_اندازه(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
ساكن صحيح tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_داده;
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
  صحيح *p_بیرون_Flags
){
  صحيح rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->p_App_داده;

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
  Tcl_Reset_نتیجه(p->interp);
  إذا( p->pScript && p->ماسک&TESTVFS_OPEN_ماسک ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_شمار(pArg);
    إذا( flags&SQLITE_OPEN_MAIN_DB ){
      ثابت حرف *z = &zName[طول_رشته(zName)+1];
      طالما( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += طول_رشته(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += طول_رشته(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_شمار(pArg);
    إذا( tvfs_نتیجه_کد(p, &rc) ){
      إذا( rc!=SQLITE_OK ) أرجع rc;
    }وإلا{
      pId = Tcl_Get_Obj_نتیجه(p->interp);
    }
  }

  إذا( (p->ماسک&TESTVFS_OPEN_ماسک) &&  tvfsInjectIoerr(p) ) أرجع SQLITE_IOERR;
  إذا( tvfsInjectCantopenerr(p) ) أرجع SQLITE_CANTOPEN;
  إذا( tvfsInjectFullerr(p) ) أرجع SQLITE_پر;

  إذا( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_شمار(pId);
  pFd->pShmId = pId;
  Tcl_Reset_نتیجه(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, p_بیرون_Flags);
  إذا( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    صحيح nByte;

    إذا( pVfs->i_نسخه>1 ){
      nByte = حجم(sqlite3_io_methods);
    }وإلا{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    کپی_حافظه(pMethods, &tvfs_io_methods, nByte);
    pMethods->i_نسخه = pFd->pReal->pMethods->i_نسخه;
    إذا( pMethods->i_نسخه>pVfs->i_نسخه ){
      pMethods->i_نسخه = pVfs->i_نسخه;
    }
    إذا( pVfs->i_نسخه>1 && ((Testvfs *)pVfs->p_App_داده)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->x_Shm_قفل = 0;
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
ساكن صحيح tvfsDelete(sqlite3_vfs *pVfs, ثابت حرف *z_مسیر, صحيح dirSync){
  صحيح rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->p_App_داده;

  إذا( p->pScript && p->ماسک&TESTVFS_DELETE_ماسک ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(z_مسیر, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfs_نتیجه_کد(p, &rc);
  }
  إذا( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), z_مسیر, dirSync);
  }
  أرجع rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساكن صحيح tvfsAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_مسیر, 
  صحيح flags, 
  صحيح *p_Res_بیرون
){
  Testvfs *p = (Testvfs *)pVfs->p_App_داده;
  إذا( p->pScript && p->ماسک&TESTVFS_ACCESS_ماسک ){
    صحيح rc;
    حرف *zArg = 0;
    إذا( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    إذا( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    إذا( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(z_مسیر, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    إذا( tvfs_نتیجه_کد(p, &rc) ){
      إذا( rc!=SQLITE_OK ) أرجع rc;
    }وإلا{
      Tcl_Interp *interp = p->interp;
      إذا( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_Get_Obj_نتیجه(interp), p_Res_بیرون) ){
        أرجع SQLITE_OK;
      }
    }
  }
  أرجع sqlite3OsAccess(PARENTVFS(pVfs), z_مسیر, flags, p_Res_بیرون);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ساكن صحيح tvfs_پر_Pathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_مسیر, 
  صحيح n_بیرون, 
  حرف *z_بیرون
){
  Testvfs *p = (Testvfs *)pVfs->p_App_داده;
  إذا( p->pScript && p->ماسک&TESTVFS_FULLPATHNAME_ماسک ){
    صحيح rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(z_مسیر, -1), 0, 0, 0);
    إذا( tvfs_نتیجه_کد(p, &rc) ){
      إذا( rc!=SQLITE_OK ) أرجع rc;
    }
  }
  أرجع sqlite3_Os_پر_Pathname(PARENTVFS(pVfs), z_مسیر, n_بیرون, z_بیرون);
}

#اگر_تعریف_نشده SQLITE_OMIT_بارگیری_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ساكن فراغ *tvfsDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_مسیر){
  أرجع sqlite3OsDlOpen(PARENTVFS(pVfs), z_مسیر);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساكن فراغ tvfsDlError(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Err_پیام){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_پیام);
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
#پایان_اگر /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساكن صحيح tvfsRandomness(sqlite3_vfs *pVfs, صحيح nByte, حرف *z_Buf_بیرون){
  أرجع sqlite3OsRandomness(PARENTVFS(pVfs), nByte, z_Buf_بیرون);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساكن صحيح tvfsSleep(sqlite3_vfs *pVfs, صحيح nMicro){
  Testvfs *p = (Testvfs *)pVfs->p_App_داده;
  إذا( p->pScript && (p->ماسک&TESTVFS_SLEEP_ماسک) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  أرجع sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساكن صحيح tvfs_Current_زمان(sqlite3_vfs *pVfs, مزدوج *p_زمان_بیرون){
  أرجع PARENTVFS(pVfs)->x_Current_زمان(PARENTVFS(pVfs), p_زمان_بیرون);
}

ساكن صحيح tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  صحيح rc = SQLITE_OK;             /* Return code */
  Testvfs_میانگیر *p_میانگیر;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->p_App_داده;
  أكد( 0==p->isFullshm );
  أكد( pFd->pShmId && pFd->pShm==0 && pFd->p_بعدی==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_Reset_نتیجه(p->interp);
  إذا( p->pScript && p->ماسک&TESTVFS_SHMOPEN_ماسک ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    إذا( tvfs_نتیجه_کد(p, &rc) ){
      إذا( rc!=SQLITE_OK ) أرجع rc;
    }
  }

  أكد( rc==SQLITE_OK );
  إذا( p->ماسک&TESTVFS_SHMOPEN_ماسک && tvfsInjectIoerr(p) ){
    أرجع SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  لكل(p_میانگیر=p->p_میانگیر; p_میانگیر; p_میانگیر=p_میانگیر->p_بعدی){
    إذا( 0==strcmp(pFd->zFilename, p_میانگیر->zFile) ) اكسر;
  }
  إذا( !p_میانگیر ){
    صحيح szName = (صحيح)طول_رشته(pFd->zFilename);
    صحيح nByte = حجم(Testvfs_میانگیر) + szName + 1;
    p_میانگیر = (Testvfs_میانگیر *)ckalloc(nByte);
    memset(p_میانگیر, 0, nByte);
    p_میانگیر->zFile = (حرف *)&p_میانگیر[1];
    کپی_حافظه(p_میانگیر->zFile, pFd->zFilename, szName+1);
    p_میانگیر->p_بعدی = p->p_میانگیر;
    p->p_میانگیر = p_میانگیر;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->p_بعدی = p_میانگیر->pFile;
  p_میانگیر->pFile = pFd;
  pFd->pShm = p_میانگیر;
  أرجع rc;
}

ساكن فراغ tvfs_Alloc_صفحه(Testvfs_میانگیر *p, صحيح i_صفحه, صحيح pgsz){
  أكد( i_صفحه<TESTVFS_MAX_صفحات );
  إذا( p->a_صفحه[i_صفحه]==0 ){
    p->a_صفحه[i_صفحه] = (u8 *)ckalloc(pgsz);
    memset(p->a_صفحه[i_صفحه], 0, pgsz);
    p->pgsz = pgsz;
  }
}

ساكن صحيح tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  صحيح i_صفحه,                      /* Page to retrieve */
  صحيح pgsz,                       /* Size of pages */
  صحيح isWrite,                    /* True to extend file if necessary */
  فراغ متطاير **pp              /* OUT: Mapped memory */
){
  صحيح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_داده);

  إذا( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    أرجع pReal->pMethods->xShmMap(pReal, i_صفحه, pgsz, isWrite, pp);
  }

  إذا( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    إذا( rc!=SQLITE_OK ){
      أرجع rc;
    }
  }

  إذا( p->pScript && p->ماسک&TESTVFS_SHMMAP_ماسک ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_شمار(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(i_صفحه));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfs_نتیجه_کد(p, &rc);
    Tcl_Decr_Ref_شمار(pArg);
  }
  إذا( rc==SQLITE_OK && p->ماسک&TESTVFS_SHMMAP_ماسک && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  إذا( rc==SQLITE_OK && isWrite && !pFd->pShm->a_صفحه[i_صفحه] ){
    tvfs_Alloc_صفحه(pFd->pShm, i_صفحه, pgsz);
  }
  إذا( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (فراغ متطاير *)pFd->pShm->a_صفحه[i_صفحه];
  }

  أرجع rc;
}


ساكن صحيح tvfs_Shm_قفل(
  sqlite3_file *pFile,
  صحيح ofst,
  صحيح n,
  صحيح flags
){
  صحيح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_داده);
  صحيح n_قفل;
  حرف z_قفل[80];

  إذا( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    أرجع pReal->pMethods->x_Shm_قفل(pReal, ofst, n, flags);
  }

  إذا( p->pScript && p->ماسک&TESTVFS_SHMLOCK_ماسک ){
    sqlite3_snprintf(حجم(z_قفل), z_قفل, "%d %d", ofst, n);
    n_قفل = (صحيح)طول_رشته(z_قفل);
    إذا( flags & SQLITE_SHM_قفل ){
      strcpy(&z_قفل[n_قفل], " lock");
    }وإلا{
      strcpy(&z_قفل[n_قفل], " unlock");
    }
    n_قفل += (صحيح)طول_رشته(&z_قفل[n_قفل]);
    إذا( flags & SQLITE_SHM_SHARED ){
      strcpy(&z_قفل[n_قفل], " shared");
    }وإلا{
      strcpy(&z_قفل[n_قفل], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(z_قفل, -1), 0
    );
    tvfs_نتیجه_کد(p, &rc);
  }

  إذا( rc==SQLITE_OK && p->ماسک&TESTVFS_SHMLOCK_ماسک && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  إذا( rc==SQLITE_OK ){
    صحيح is_قفل = (flags & SQLITE_SHM_قفل);
    صحيح isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 ماسک = (((1<<n)-1) << ofst);
    إذا( is_قفل ){
      TestvfsFd *p2;
      لكل(p2=pFd->pShm->pFile; p2; p2=p2->p_بعدی){
        إذا( p2==pFd ) استمر;
        إذا( (p2->excllock&ماسک) || (isExcl && p2->sharedlock&ماسک) ){
          rc = SQLITE_BUSY;
          اكسر;
        }
      }
      إذا( rc==SQLITE_OK ){
        إذا( isExcl )  pFd->excllock |= ماسک;
        إذا( !isExcl ) pFd->sharedlock |= ماسک;
      }
    }وإلا{
      إذا( isExcl )  pFd->excllock &= (~ماسک);
      إذا( !isExcl ) pFd->sharedlock &= (~ماسک);
    }
  }

  أرجع rc;
}

ساكن فراغ tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_داده);

  إذا( p->pScript && p->ماسک&TESTVFS_SHMBARRIER_ماسک ){
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
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_داده);
  Testvfs_میانگیر *p_میانگیر = pFd->pShm;
  TestvfsFd **ppFd;

  إذا( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    أرجع pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  إذا( !p_میانگیر ) أرجع SQLITE_OK;
  أكد( pFd->pShmId && pFd->pShm );

  إذا( p->pScript && p->ماسک&TESTVFS_SHMCLOSE_ماسک ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfs_نتیجه_کد(p, &rc);
  }

  لكل(ppFd=&p_میانگیر->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->p_بعدی));
  أكد( (*ppFd)==pFd );
  *ppFd = pFd->p_بعدی;
  pFd->p_بعدی = 0;

  إذا( p_میانگیر->pFile==0 ){
    صحيح i;
    Testvfs_میانگیر **pp;
    لكل(pp=&p->p_میانگیر; *pp!=p_میانگیر; pp=&((*pp)->p_بعدی));
    *pp = (*pp)->p_بعدی;
    لكل(i=0; p_میانگیر->a_صفحه[i]; i++){
      ckfree((حرف *)p_میانگیر->a_صفحه[i]);
    }
    ckfree((حرف *)p_میانگیر);
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
  Client_داده cd,
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
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "SUBCOMMAND ...");
    أرجع TCL_ERROR;
  }
  إذا( Tcl_Get_شاخص_From_Obj_Struct(
        interp, objv[1], aSubcmd, حجم(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    أرجع TCL_ERROR;
  }
  Tcl_Reset_نتیجه(interp);

  بدل( aSubcmd[i].eCmd ){
    حالة CMD_SHM: {
      Tcl_Obj *pObj;
      صحيح rc;
      Testvfs_میانگیر *p_میانگیر;
      حرف *zName;
      إذا( objc!=3 && objc!=4 ){
        Tcl_Wrong_رقم_آرگومانها(interp, 2, objv, "FILE ?VALUE?");
        أرجع TCL_ERROR;
      }
      zName = ckalloc(p->p_والد->mxPathname);
      rc = p->p_والد->x_پر_Pathname(
          p->p_والد, Tcl_GetString(objv[2]), 
          p->p_والد->mxPathname, zName
      );
      إذا( rc!=SQLITE_OK ){
        Tcl_Append_نتیجه(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), عدم);
        ckfree(zName);
        أرجع TCL_ERROR;
      }
      لكل(p_میانگیر=p->p_میانگیر; p_میانگیر; p_میانگیر=p_میانگیر->p_بعدی){
        إذا( 0==strcmp(p_میانگیر->zFile, zName) ) اكسر;
      }
      ckfree(zName);
      إذا( !p_میانگیر ){
        Tcl_Append_نتیجه(interp, "no such file: ", Tcl_GetString(objv[2]), عدم);
        أرجع TCL_ERROR;
      }
      إذا( objc==4 ){
        Tcl_اندازه n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        صحيح pgsz = p_میانگیر->pgsz;
        إذا( pgsz==0 ) pgsz = 65536;
        لكل(i=0; i*pgsz<(صحيح)n; i++){
          صحيح nByte = pgsz;
          tvfs_Alloc_صفحه(p_میانگیر, i, pgsz);
          إذا( n-i*pgsz<pgsz ){
            nByte = (صحيح)n;
          }
          کپی_حافظه(p_میانگیر->a_صفحه[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      لكل(i=0; p_میانگیر->a_صفحه[i]; i++){
        صحيح pgsz = p_میانگیر->pgsz;
        إذا( pgsz==0 ) pgsz = 65536;
        Tcl_Append_Obj_مقصد_Obj(pObj, Tcl_NewByteArrayObj(p_میانگیر->a_صفحه[i], pgsz));
      }
      Tcl_مجموعه_Obj_نتیجه(interp, pObj);
      اكسر;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    حالة CMD_FILTER: {
      ساكن بنية Vfs_روش {
        حرف *zName;
        صحيح ماسک;
      } vfsmethod [] = {
        { "xShmOpen",           TESTVFS_SHMOPEN_ماسک },
        { "xShmLock",           TESTVFS_SHMLOCK_ماسک },
        { "xShmBarrier",        TESTVFS_SHMBARRIER_ماسک },
        { "xShmUnmap",          TESTVFS_SHMCLOSE_ماسک },
        { "xShmMap",            TESTVFS_SHMMAP_ماسک },
        { "xSync",              TESTVFS_SYNC_ماسک },
        { "xDelete",            TESTVFS_DELETE_ماسک },
        { "xWrite",             TESTVFS_WRITE_ماسک },
        { "xRead",              TESTVFS_READ_ماسک },
        { "xTruncate",          TESTVFS_TRUNCATE_ماسک },
        { "xOpen",              TESTVFS_OPEN_ماسک },
        { "xClose",             TESTVFS_CLOSE_ماسک },
        { "xAccess",            TESTVFS_ACCESS_ماسک },
        { "xFullPathname",      TESTVFS_FULLPATHNAME_ماسک },
        { "xUnlock",            TESTVFS_UNLOCK_ماسک },
        { "xLock",              TESTVFS_قفل_ماسک },
        { "xCheckReservedLock", TESTVFS_CKLOCK_ماسک },
        { "xFileControl",       TESTVFS_FCNTL_ماسک },
        { "xSleep",             TESTVFS_SLEEP_ماسک },
      };
      Tcl_Obj **apElem = 0;
      Tcl_اندازه nElem = 0;
      صحيح ماسک = 0;
      إذا( objc!=3 ){
        Tcl_Wrong_رقم_آرگومانها(interp, 2, objv, "LIST");
        أرجع TCL_ERROR;
      }
      إذا( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        أرجع TCL_ERROR;
      }
      Tcl_Reset_نتیجه(interp);
      لكل(i=0; i<(صحيح)nElem; i++){
        صحيح i_روش;
        حرف *zElem = Tcl_GetString(apElem[i]);
        لكل(i_روش=0; i_روش<Array_اندازه(vfsmethod); i_روش++){
          إذا( strcmp(zElem, vfsmethod[i_روش].zName)==0 ){
            ماسک |= vfsmethod[i_روش].ماسک;
            اكسر;
          }
        }
        إذا( i_روش==Array_اندازه(vfsmethod) ){
          Tcl_Append_نتیجه(interp, "unknown method: ", zElem, عدم);
          أرجع TCL_ERROR;
        }
      }
      p->ماسک = ماسک;
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
        Tcl_اندازه nByte;
        إذا( p->pScript ){
          Tcl_Decr_Ref_شمار(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        إذا( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_شمار(p->pScript);
        }
      }وإلا إذا( objc!=2 ){
        Tcl_Wrong_رقم_آرگومانها(interp, 2, objv, "?SCRIPT?");
        أرجع TCL_ERROR;
      }

      Tcl_Reset_نتیجه(interp);
      إذا( p->pScript ) Tcl_مجموعه_Obj_نتیجه(interp, p->pScript);

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
        حالة CMD_FULLERR: pTest = &p->پر_err; اكسر;
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
        Tcl_Wrong_رقم_آرگومانها(interp, 2, objv, "?CNT PERSIST?");
        أرجع TCL_ERROR;
      }
      Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewIntObj(iRet));
      اكسر;
    }

    حالة CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      اكسر;
    }

    حالة CMD_DEVCHAR: {
      بنية DeviceFlag {
        حرف *zName;
        صحيح i_مقدار;
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
        { "safe_append",           SQLITE_IOCAP_امن_APPEND           },
        { "undeletable_when_open", SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN },
        { "powersafe_overwrite",   SQLITE_IOCAP_POWERSAFE_OVERWRITE   },
        { "immutable",             SQLITE_IOCAP_IMMUTABLE             },
        { 0, 0 }
      };
      Tcl_Obj *pRet;
      صحيح iFlag;

      إذا( objc>3 ){
        Tcl_Wrong_رقم_آرگومانها(interp, 2, objv, "?ATTR-LIST?");
        أرجع TCL_ERROR;
      }
      إذا( objc==3 ){
        صحيح j;
        صحيح iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_اندازه nFlags = 0;

        إذا( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          أرجع TCL_ERROR;
        }

        لكل(j=0; j<(صحيح)nFlags; j++){
          صحيح idx = 0;
          إذا( Tcl_Get_شاخص_From_Obj_Struct(interp, flags[j], aFlag, 
                حجم(aFlag[0]), "flag", 0, &idx) 
          ){
            أرجع TCL_ERROR;
          }
          إذا( aFlag[idx].i_مقدار<0 && nFlags>1 ){
            Tcl_Append_نتیجه(interp, "bad flags: ", Tcl_GetString(objv[2]), عدم);
            أرجع TCL_ERROR;
          }
          iNew |= aFlag[idx].i_مقدار;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      لكل(iFlag=0; iFlag<حجم(aFlag)/حجم(aFlag[0]); iFlag++){
        إذا( p->iDevchar & aFlag[iFlag].i_مقدار ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_مجموعه_Obj_نتیجه(interp, pRet);

      اكسر;
    }

    حالة CMD_SECTORSIZE: {
      إذا( objc>3 ){
        Tcl_Wrong_رقم_آرگومانها(interp, 2, objv, "?VALUE?");
        أرجع TCL_ERROR;
      }
      إذا( objc==3 ){
        صحيح iNew = 0;
        إذا( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          أرجع TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewIntObj(p->iSectorsize));
      اكسر;
    }
  }

  أرجع TCL_OK;
}

ساكن فراغ SQLITE_TCLAPI testvfs_obj_del(Client_داده cd){
  Testvfs *p = (Testvfs *)cd;
  إذا( p->pScript ) Tcl_Decr_Ref_شمار(p->pScript);
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
  Client_داده cd,
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
    tvfs_پر_Pathname,             /* xFullPathname */
#اگر_تعریف_نشده SQLITE_OMIT_بارگیری_EXTENSION
    tvfsDlOpen,                   /* xDlOpen */
    tvfsDlError,                  /* xDlError */
    tvfsDlSym,                    /* xDlSym */
    tvfsDlClose,                  /* xDlClose */
#وإلا
    0,                            /* xDlOpen */
    0,                            /* xDlError */
    0,                            /* xDlSym */
    0,                            /* xDlClose */
#پایان_اگر /* SQLITE_OMIT_LOAD_EXTENSION */
    tvfsRandomness,               /* xRandomness */
    tvfsSleep,                    /* xSleep */
    tvfs_Current_زمان,              /* xCurrentTime */
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
  صحيح i_نسخه = 3;               /* Value passed to -iversion */

  إذا( objc<2 || 0!=(objc%2) ) اذهب bad_آرگومانها;
  لكل(i=2; i<objc; i += 2){
    Tcl_اندازه nSwitch;
    حرف *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    إذا( nSwitch>2 && 0==مقایسه_رشته("-noshm", zSwitch, nSwitch) ){
      إذا( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        أرجع TCL_ERROR;
      }
      إذا( isNoshm ) isFullshm = 0;
    }
    وإلا إذا( nSwitch>2 && 0==مقایسه_رشته("-default", zSwitch, nSwitch) ){
      إذا( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        أرجع TCL_ERROR;
      }
    }
    وإلا إذا( nSwitch>2 && 0==مقایسه_رشته("-szosfile", zSwitch, nSwitch) ){
      إذا( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        أرجع TCL_ERROR;
      }
    }
    وإلا إذا( nSwitch>2 && 0==مقایسه_رشته("-mxpathname", zSwitch, nSwitch) ){
      إذا( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        أرجع TCL_ERROR;
      }
    }
    وإلا إذا( nSwitch>2 && 0==مقایسه_رشته("-iversion", zSwitch, nSwitch) ){
      إذا( Tcl_GetIntFromObj(interp, objv[i+1], &i_نسخه) ){
        أرجع TCL_ERROR;
      }
    }
    وإلا إذا( nSwitch>2 && 0==مقایسه_رشته("-fullshm", zSwitch, nSwitch) ){
      إذا( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        أرجع TCL_ERROR;
      }
      إذا( isFullshm ) isNoshm = 0;
    }
    وإلا{
      اذهب bad_آرگومانها;
    }
  }

  إذا( szOsFile<حجم(TestvfsFile) ){
    szOsFile = حجم(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = حجم(Testvfs) + (صحيح)طول_رشته(zVfs)+1;
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
  p->p_والد = sqlite3_vfs_پیدا_کن(0);
  p->interp = interp;

  p->zName = (حرف *)&p[1];
  کپی_حافظه(p->zName, zVfs, طول_رشته(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(حجم(sqlite3_vfs));
  کپی_حافظه(pVfs, &tvfs_vfs, حجم(sqlite3_vfs));
  pVfs->p_App_داده = (فراغ *)p;
  pVfs->i_نسخه = i_نسخه;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->p_والد->mxPathname;
  إذا( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->ماسک = TESTVFS_ALL_ماسک;

  sqlite3_vfs_ثبات(pVfs, isDefault);

  أرجع TCL_OK;

 bad_آرگومانها:
  Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  أرجع TCL_ERROR;
}

خارجي صحيح get_Db_اشاره_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجي ثابت حرف *sqlite3ErrName(صحيح);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
ساكن صحيح SQLITE_TCLAPI test_vfs_shmlock(
  فراغ * client_داده,
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
  صحيح i_جابه_جایی = 0;
  صحيح n = 0;
  sqlite3_file *pFd;

  إذا( objc!=7 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    أرجع TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  إذا( get_Db_اشاره_گر(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_Get_شاخص_From_Obj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_Get_شاخص_From_Obj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &i_جابه_جایی)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    أرجع TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_اشاره_گر, (فراغ*)&pFd);
  إذا( pFd==0 ){
    أرجع TCL_ERROR;
  }
  rc = pFd->pMethods->x_Shm_قفل(pFd, i_جابه_جایی, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_قفل : SQLITE_SHM_UNLOCK)
  );
  Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_vfs_مجموعه_readmark(
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  صحيح rc = SQLITE_OK;
  ثابت حرف *zDbname = 0;
  صحيح i_جایگاه = 0;
  صحيح iVal = -1;
  sqlite3_file *pFd;
  فراغ متطاير *pShm = 0;
  u32 *aShm;
  صحيح iOff;

  إذا( objc!=4 && objc!=5 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    أرجع TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  إذا( get_Db_اشاره_گر(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &i_جایگاه)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    أرجع TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_اشاره_گر, (فراغ*)&pFd);
  إذا( pFd==0 ){
    أرجع TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  إذا( rc!=SQLITE_OK ){
    Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    أرجع TCL_ERROR;
  }
  إذا( pShm==0 ){
    Tcl_Append_نتیجه(interp, "*-shm is not yet mapped", عدم);
    أرجع TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+i_جایگاه;

  إذا( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewIntObj(aShm[iOff]));

  أرجع TCL_OK;
}

صحيح Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_مجموعه_readmark, 0, 0);
  أرجع TCL_OK;
}

#پایان_اگر
