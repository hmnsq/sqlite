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
#اگر SQLITE_TEST          /* This file is used for testing only */

#شامل "sqlite3.h"
#شامل "sqliteInt.h"
#شامل "tclsqlite.h"

قسمتعریف ساخت Testvfs Testvfs;
قسمتعریف ساخت TestvfsShm TestvfsShm;
قسمتعریف ساخت Testvfs_بفر Testvfs_بفر;
قسمتعریف ساخت TestvfsFile TestvfsFile;
قسمتعریف ساخت TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
ساخت TestvfsFile {
  sqlite3_file بنیاد;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#تعریف_کرو tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

ساخت TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  ثابت حرف *zFilename;          /* Filename as passed to xOpen() */
  sqlite3_file *pReal;            /* The real, underlying file descriptor */
  Tcl_Obj *pShmId;                /* Shared memory id for Tcl callbacks */

  Testvfs_بفر *pShm;            /* Shared memory buffer */
  u32 excllock;                   /* Mask of exclusive locks */
  u32 sharedlock;                 /* Mask of shared locks */
  TestvfsFd *p_اگلا;               /* Next handle opened on the same file */
};


#تعریف_کرو FAULT_INJECT_NONE       0
#تعریف_کرو FAULT_INJECT_TRANSIENT  1
#تعریف_کرو FAULT_INJECT_PERSISTENT 2

قسمتعریف ساخت TestFaultInject TestFaultInject;
ساخت TestFaultInject {
  صحیح iCnt;                       /* Remaining calls before fault injection */
  صحیح eFault;                     /* A FAULT_INJECT_* value */
  صحیح nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
ساخت Testvfs {
  حرف *zName;                    /* Name of this VFS */
  sqlite3_vfs *p_والد;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  Testvfs_بفر *p_بفر;         /* List of shared buffers */
  صحیح isNoshm;
  صحیح isFullshm;

  صحیح نقاب;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject بھرا_err;
  TestFaultInject cantopen_err;

#اگر 0
  صحیح iIoerrCnt;
  صحیح ioerr;
  صحیح nIoerrFail;
  صحیح i_بھرا_Cnt;
  صحیح fullerr;
  صحیح n_بھرا_Fail;
#اگر_ختم

  صحیح iDevchar;
  صحیح iSectorsize;
};

/*
** The Testvfs.mask variable is set to a combination of the following.
** If a bit is clear in Testvfs.mask, then calls made by SQLite to the 
** corresponding VFS method is ignored for purposes of:
**
**   + Simulating IO errors, and
**   + Invoking the Tcl callback script.
*/
#تعریف_کرو TESTVFS_SHMOPEN_نقاب      0x00000001
#تعریف_کرو TESTVFS_SHMLOCK_نقاب      0x00000010
#تعریف_کرو TESTVFS_SHMMAP_نقاب       0x00000020
#تعریف_کرو TESTVFS_SHMBARRIER_نقاب   0x00000040
#تعریف_کرو TESTVFS_SHMCLOSE_نقاب     0x00000080

#تعریف_کرو TESTVFS_OPEN_نقاب         0x00000100
#تعریف_کرو TESTVFS_SYNC_نقاب         0x00000200
#تعریف_کرو TESTVFS_DELETE_نقاب       0x00000400
#تعریف_کرو TESTVFS_CLOSE_نقاب        0x00000800
#تعریف_کرو TESTVFS_WRITE_نقاب        0x00001000
#تعریف_کرو TESTVFS_TRUNCATE_نقاب     0x00002000
#تعریف_کرو TESTVFS_ACCESS_نقاب       0x00004000
#تعریف_کرو TESTVFS_FULLPATHNAME_نقاب 0x00008000
#تعریف_کرو TESTVFS_READ_نقاب         0x00010000
#تعریف_کرو TESTVFS_UNLOCK_نقاب       0x00020000
#تعریف_کرو TESTVFS_تالا_نقاب         0x00040000
#تعریف_کرو TESTVFS_CKLOCK_نقاب       0x00080000
#تعریف_کرو TESTVFS_FCNTL_نقاب        0x00100000
#تعریف_کرو TESTVFS_SLEEP_نقاب        0x00200000

#تعریف_کرو TESTVFS_ALL_نقاب          0x003FFFFF


#تعریف_کرو TESTVFS_MAX_صفحات 1024

/*
** A shared-memory buffer. There is one of these objects for each shared
** memory region opened by clients. If two clients open the same file,
** there are two TestvfsFile structures but only one TestvfsBuffer structure.
*/
ساخت Testvfs_بفر {
  حرف *zFile;                    /* Associated file name */
  صحیح pgsz;                       /* Page size */
  u8 *a_صفحہ[TESTVFS_MAX_صفحات];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  Testvfs_بفر *p_اگلا;           /* Next in linked list of all buffers */
};


#تعریف_کرو PARENTVFS(x) (((Testvfs *)((x)->p_App_ڈیٹا))->p_والد)

#تعریف_کرو TESTVFS_MAX_دلائل 12


/*
** Method declarations for TestvfsFile.
*/
ساکن صحیح tvfsClose(sqlite3_file*);
ساکن صحیح tvfsRead(sqlite3_file*, خالی*, صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح tvfsWrite(sqlite3_file*,ثابت خالی*,صحیح iAmt, sqlite3_int64 iOfst);
ساکن صحیح tvfsTruncate(sqlite3_file*, sqlite3_int64 سائز);
ساکن صحیح tvfsSync(sqlite3_file*, صحیح flags);
ساکن صحیح tvfs_File_سائز(sqlite3_file*, sqlite3_int64 *p_سائز);
ساکن صحیح tvfs_تالا(sqlite3_file*, صحیح);
ساکن صحیح tvfsUnlock(sqlite3_file*, صحیح);
ساکن صحیح tvfs_Check_Reserved_تالا(sqlite3_file*, صحیح *);
ساکن صحیح tvfsFileControl(sqlite3_file*, صحیح op, خالی *pArg);
ساکن صحیح tvfs_Sector_سائز(sqlite3_file*);
ساکن صحیح tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
ساکن صحیح tvfsOpen(sqlite3_vfs*, ثابت حرف *, sqlite3_file*, صحیح , صحیح *);
ساکن صحیح tvfsDelete(sqlite3_vfs*, ثابت حرف *zName, صحیح syncDir);
ساکن صحیح tvfsAccess(sqlite3_vfs*, ثابت حرف *zName, صحیح flags, صحیح *);
ساکن صحیح tvfs_بھرا_Pathname(sqlite3_vfs*, ثابت حرف *zName, صحیح, حرف *z_برآمد);
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
ساکن خالی *tvfsDlOpen(sqlite3_vfs*, ثابت حرف *zFilename);
ساکن خالی tvfsDlError(sqlite3_vfs*, صحیح nByte, حرف *z_Err_پیغام);
ساکن خالی (*tvfsDlSym(sqlite3_vfs*,خالی*, ثابت حرف *z_علامت))(خالی);
ساکن خالی tvfsDlClose(sqlite3_vfs*, خالی*);
#اگر_ختم /* SQLITE_OMIT_LOAD_EXTENSION */
ساکن صحیح tvfsRandomness(sqlite3_vfs*, صحیح nByte, حرف *z_برآمد);
ساکن صحیح tvfsSleep(sqlite3_vfs*, صحیح microseconds);
ساکن صحیح tvfs_Current_وقت(sqlite3_vfs*, دوہرا*);

ساکن صحیح tvfsShmOpen(sqlite3_file*);
ساکن صحیح tvfs_Shm_تالا(sqlite3_file*, صحیح , صحیح, صحیح);
ساکن صحیح tvfsShmMap(sqlite3_file*,صحیح,صحیح,صحیح, خالی متلون **);
ساکن خالی tvfsShmBarrier(sqlite3_file*);
ساکن صحیح tvfsShmUnmap(sqlite3_file*, صحیح);

ساکن صحیح tvfsFetch(sqlite3_file*, sqlite3_int64, صحیح, خالی**);
ساکن صحیح tvfsUnfetch(sqlite3_file*, sqlite3_int64, خالی*);

ساکن sqlite3_io_methods tvfs_io_methods = {
  3,                              /* iVersion */
  tvfsClose,                      /* xClose */
  tvfsRead,                       /* xRead */
  tvfsWrite,                      /* xWrite */
  tvfsTruncate,                   /* xTruncate */
  tvfsSync,                       /* xSync */
  tvfs_File_سائز,                   /* xFileSize */
  tvfs_تالا,                       /* xLock */
  tvfsUnlock,                     /* xUnlock */
  tvfs_Check_Reserved_تالا,          /* xCheckReservedLock */
  tvfsFileControl,                /* xFileControl */
  tvfs_Sector_سائز,                 /* xSectorSize */
  tvfsDeviceCharacteristics,      /* xDeviceCharacteristics */
  tvfsShmMap,                     /* xShmMap */
  tvfs_Shm_تالا,                    /* xShmLock */
  tvfsShmBarrier,                 /* xShmBarrier */
  tvfsShmUnmap,                   /* xShmUnmap */
  tvfsFetch,
  tvfsUnfetch
};

ساکن صحیح tvfs_نتیجہ_رمز(Testvfs *p, صحیح *pRc){
  ساخت errcode {
    صحیح e_رمز;
    ثابت حرف *z_رمز;
  } a_رمز[] = {
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
  صحیح i;

  z = Tcl_Get_String_نتیجہ(p->interp);
  برائے(i=0; i<Array_سائز(a_رمز); i++){
    اگر( 0==strcmp(z, a_رمز[i].z_رمز) ){
      *pRc = a_رمز[i].e_رمز;
      واپس 1;
    }
  }

  واپس 0;
}

ساکن صحیح tvfsInjectFault(TestFaultInject *p){
  صحیح ret = 0;
  اگر( p->eFault ){
    p->iCnt--;
    اگر( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  واپس ret;
}


ساکن صحیح tvfsInjectIoerr(Testvfs *p){
  واپس tvfsInjectFault(&p->ioerr_err);
}

ساکن صحیح tvfsInjectFullerr(Testvfs *p){
  واپس tvfsInjectFault(&p->بھرا_err);
}
ساکن صحیح tvfsInjectCantopenerr(Testvfs *p){
  واپس tvfsInjectFault(&p->cantopen_err);
}


ساکن خالی tvfsExecTcl(
  Testvfs *p, 
  ثابت حرف *z_طریقہ,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  صحیح rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  أكد( p->pScript );

  أكد( z_طریقہ );
  أكد( p );
  أكد( arg2==0 || arg1!=0 );
  أكد( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_گنتی(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(z_طریقہ, -1));
  اگر( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  اگر( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  اگر( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  اگر( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  اگر( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_Reset_نتیجہ(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
ساکن صحیح tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ڈیٹا;

  اگر( p->pScript && p->نقاب&TESTVFS_CLOSE_نقاب ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  اگر( pFd->pShmId ){
    Tcl_Decr_Ref_گنتی(pFd->pShmId);
    pFd->pShmId = 0;
  }
  اگر( pFile->pMethods ){
    ckfree((حرف *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((حرف *)pFd);
  pTestfile->pFd = 0;
  واپس SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
ساکن صحیح tvfsRead(
  sqlite3_file *pFile, 
  خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ڈیٹا;
  اگر( p->pScript && p->نقاب&TESTVFS_READ_نقاب ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_نتیجہ_رمز(p, &rc);
  }
  اگر( rc==SQLITE_OK && p->نقاب&TESTVFS_READ_نقاب && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  اگر( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  واپس rc;
}

/*
** Write data to an tvfs-file.
*/
ساکن صحیح tvfsWrite(
  sqlite3_file *pFile, 
  ثابت خالی *zBuf, 
  صحیح iAmt, 
  sqlite_int64 iOfst
){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ڈیٹا;

  اگر( p->pScript && p->نقاب&TESTVFS_WRITE_نقاب ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfs_نتیجہ_رمز(p, &rc);
    اگر( rc<0 ) واپس SQLITE_OK;
  }

  اگر( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_بھرا;
  }
  اگر( rc==SQLITE_OK && p->نقاب&TESTVFS_WRITE_نقاب && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  اگر( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  واپس rc;
}

/*
** Truncate an tvfs-file.
*/
ساکن صحیح tvfsTruncate(sqlite3_file *pFile, sqlite_int64 سائز){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ڈیٹا;

  اگر( p->pScript && p->نقاب&TESTVFS_TRUNCATE_نقاب ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_نتیجہ_رمز(p, &rc);
  }
  
  اگر( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, سائز);
  }
  واپس rc;
}

/*
** Sync an tvfs-file.
*/
ساکن صحیح tvfsSync(sqlite3_file *pFile, صحیح flags){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ڈیٹا;

  اگر( p->pScript && p->نقاب&TESTVFS_SYNC_نقاب ){
    حرف *zFlags = 0;

    انتخاب( flags ){
      حالت SQLITE_SYNC_معمول:
        zFlags = "normal";
        توڑو;
      حالت SQLITE_SYNC_بھرا:
        zFlags = "full";
        توڑو;
      حالت SQLITE_SYNC_معمول|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        توڑو;
      حالت SQLITE_SYNC_بھرا|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        توڑو;
      طےشدہ:
        أكد(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfs_نتیجہ_رمز(p, &rc);
  }

  اگر( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_بھرا;

  اگر( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  واپس rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
ساکن صحیح tvfs_File_سائز(sqlite3_file *pFile, sqlite_int64 *p_سائز){
  TestvfsFd *p = tvfsGetFd(pFile);
  واپس sqlite3_Os_File_سائز(p->pReal, p_سائز);
}

/*
** Lock an tvfs-file.
*/
ساکن صحیح tvfs_تالا(sqlite3_file *pFile, صحیح e_تالا){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ڈیٹا;
  اگر( p->pScript && p->نقاب&TESTVFS_تالا_نقاب ){
    حرف z_تالا[30];
    sqlite3_snprintf(حجم(z_تالا),z_تالا,"%d",e_تالا);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(z_تالا, -1), 0, 0);
  }
  اگر( p->نقاب&TESTVFS_تالا_نقاب && tvfsInjectIoerr(p) ){
    واپس SQLITE_IOERR_تالا;
  }
  واپس sqlite3_Os_تالا(pFd->pReal, e_تالا);
}

/*
** Unlock an tvfs-file.
*/
ساکن صحیح tvfsUnlock(sqlite3_file *pFile, صحیح e_تالا){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ڈیٹا;
  اگر( p->pScript && p->نقاب&TESTVFS_UNLOCK_نقاب ){
    حرف z_تالا[30];
    sqlite3_snprintf(حجم(z_تالا),z_تالا,"%d",e_تالا);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(z_تالا, -1), 0, 0);
  }
  اگر( p->نقاب&TESTVFS_UNLOCK_نقاب && tvfsInjectIoerr(p) ){
    واپس SQLITE_IOERR_UNLOCK;
  }
  واپس sqlite3OsUnlock(pFd->pReal, e_تالا);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
ساکن صحیح tvfs_Check_Reserved_تالا(sqlite3_file *pFile, صحیح *p_Res_برآمد){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ڈیٹا;
  اگر( p->pScript && p->نقاب&TESTVFS_CKLOCK_نقاب ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  واپس sqlite3_Os_Check_Reserved_تالا(pFd->pReal, p_Res_برآمد);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
ساکن صحیح tvfsFileControl(sqlite3_file *pFile, صحیح op, خالی *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ڈیٹا;
  اگر( op==SQLITE_FCNTL_PRAGMA ){
    حرف **argv = (حرف**)pArg;
    اگر( sqlite3_stricmp(argv[1],"error")==0 ){
      صحیح rc = SQLITE_ERROR;
      اگر( argv[2] ){
        ثابت حرف *z = argv[2];
        صحیح x = atoi(z);
        اگر( x ){
          rc = x;
          جب( sqlite3Isdigit(z[0]) ){ z++; }
          جب( sqlite3Isspace(z[0]) ){ z++; }
        }
        اگر( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      واپس rc;
    }
    اگر( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      واپس SQLITE_OK;
    }
  }
  اگر( p->pScript && (p->نقاب&TESTVFS_FCNTL_نقاب) ){
    ساخت Fcntl {
      صحیح iFnctl;
      ثابت حرف *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    صحیح i;
    برائے(i=0; i<حجم(aF)/حجم(aF[0]); i++){
      اگر( op==aF[i].iFnctl ) توڑو;
    }
    اگر( i<حجم(aF)/حجم(aF[0]) ){
      صحیح rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfs_نتیجہ_رمز(p, &rc);
      اگر( rc ) واپس (rc<0 ? SQLITE_OK : rc);
    }
  }
  واپس sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
ساکن صحیح tvfs_Sector_سائز(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ڈیٹا;
  اگر( p->iSectorsize>=0 ){
    واپس p->iSectorsize;
  }
  واپس sqlite3_Os_Sector_سائز(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
ساکن صحیح tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ڈیٹا;
  اگر( p->iDevchar>=0 ){
    واپس p->iDevchar;
  }
  واپس sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
ساکن صحیح tvfsOpen(
  sqlite3_vfs *pVfs,
  ثابت حرف *zName,
  sqlite3_file *pFile,
  صحیح flags,
  صحیح *p_برآمد_Flags
){
  صحیح rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->p_App_ڈیٹا;

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
  Tcl_Reset_نتیجہ(p->interp);
  اگر( p->pScript && p->نقاب&TESTVFS_OPEN_نقاب ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_گنتی(pArg);
    اگر( flags&SQLITE_OPEN_MAIN_DB ){
      ثابت حرف *z = &zName[لڑی_لمبائی(zName)+1];
      جب( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += لڑی_لمبائی(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += لڑی_لمبائی(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_گنتی(pArg);
    اگر( tvfs_نتیجہ_رمز(p, &rc) ){
      اگر( rc!=SQLITE_OK ) واپس rc;
    }ورنہ{
      pId = Tcl_Get_Obj_نتیجہ(p->interp);
    }
  }

  اگر( (p->نقاب&TESTVFS_OPEN_نقاب) &&  tvfsInjectIoerr(p) ) واپس SQLITE_IOERR;
  اگر( tvfsInjectCantopenerr(p) ) واپس SQLITE_CANTOPEN;
  اگر( tvfsInjectFullerr(p) ) واپس SQLITE_بھرا;

  اگر( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_گنتی(pId);
  pFd->pShmId = pId;
  Tcl_Reset_نتیجہ(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, p_برآمد_Flags);
  اگر( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    صحیح nByte;

    اگر( pVfs->i_نسخہ>1 ){
      nByte = حجم(sqlite3_io_methods);
    }ورنہ{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    حافظہ_نقل(pMethods, &tvfs_io_methods, nByte);
    pMethods->i_نسخہ = pFd->pReal->pMethods->i_نسخہ;
    اگر( pMethods->i_نسخہ>pVfs->i_نسخہ ){
      pMethods->i_نسخہ = pVfs->i_نسخہ;
    }
    اگر( pVfs->i_نسخہ>1 && ((Testvfs *)pVfs->p_App_ڈیٹا)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->x_Shm_تالا = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  واپس rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ساکن صحیح tvfsDelete(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ, صحیح dirSync){
  صحیح rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->p_App_ڈیٹا;

  اگر( p->pScript && p->نقاب&TESTVFS_DELETE_نقاب ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(z_راستہ, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfs_نتیجہ_رمز(p, &rc);
  }
  اگر( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), z_راستہ, dirSync);
  }
  واپس rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ساکن صحیح tvfsAccess(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_راستہ, 
  صحیح flags, 
  صحیح *p_Res_برآمد
){
  Testvfs *p = (Testvfs *)pVfs->p_App_ڈیٹا;
  اگر( p->pScript && p->نقاب&TESTVFS_ACCESS_نقاب ){
    صحیح rc;
    حرف *zArg = 0;
    اگر( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    اگر( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    اگر( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(z_راستہ, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    اگر( tvfs_نتیجہ_رمز(p, &rc) ){
      اگر( rc!=SQLITE_OK ) واپس rc;
    }ورنہ{
      Tcl_Interp *interp = p->interp;
      اگر( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_Get_Obj_نتیجہ(interp), p_Res_برآمد) ){
        واپس SQLITE_OK;
      }
    }
  }
  واپس sqlite3OsAccess(PARENTVFS(pVfs), z_راستہ, flags, p_Res_برآمد);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ساکن صحیح tvfs_بھرا_Pathname(
  sqlite3_vfs *pVfs, 
  ثابت حرف *z_راستہ, 
  صحیح n_برآمد, 
  حرف *z_برآمد
){
  Testvfs *p = (Testvfs *)pVfs->p_App_ڈیٹا;
  اگر( p->pScript && p->نقاب&TESTVFS_FULLPATHNAME_نقاب ){
    صحیح rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(z_راستہ, -1), 0, 0, 0);
    اگر( tvfs_نتیجہ_رمز(p, &rc) ){
      اگر( rc!=SQLITE_OK ) واپس rc;
    }
  }
  واپس sqlite3_Os_بھرا_Pathname(PARENTVFS(pVfs), z_راستہ, n_برآمد, z_برآمد);
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ساکن خالی *tvfsDlOpen(sqlite3_vfs *pVfs, ثابت حرف *z_راستہ){
  واپس sqlite3OsDlOpen(PARENTVFS(pVfs), z_راستہ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ساکن خالی tvfsDlError(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Err_پیغام){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_پیغام);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ساکن خالی (*tvfsDlSym(sqlite3_vfs *pVfs, خالی *p, ثابت حرف *zSym))(خالی){
  واپس sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ساکن خالی tvfsDlClose(sqlite3_vfs *pVfs, خالی *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#اگر_ختم /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ساکن صحیح tvfsRandomness(sqlite3_vfs *pVfs, صحیح nByte, حرف *z_Buf_برآمد){
  واپس sqlite3OsRandomness(PARENTVFS(pVfs), nByte, z_Buf_برآمد);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ساکن صحیح tvfsSleep(sqlite3_vfs *pVfs, صحیح nMicro){
  Testvfs *p = (Testvfs *)pVfs->p_App_ڈیٹا;
  اگر( p->pScript && (p->نقاب&TESTVFS_SLEEP_نقاب) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  واپس sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ساکن صحیح tvfs_Current_وقت(sqlite3_vfs *pVfs, دوہرا *p_وقت_برآمد){
  واپس PARENTVFS(pVfs)->x_Current_وقت(PARENTVFS(pVfs), p_وقت_برآمد);
}

ساکن صحیح tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  صحیح rc = SQLITE_OK;             /* Return code */
  Testvfs_بفر *p_بفر;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->p_App_ڈیٹا;
  أكد( 0==p->isFullshm );
  أكد( pFd->pShmId && pFd->pShm==0 && pFd->p_اگلا==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_Reset_نتیجہ(p->interp);
  اگر( p->pScript && p->نقاب&TESTVFS_SHMOPEN_نقاب ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    اگر( tvfs_نتیجہ_رمز(p, &rc) ){
      اگر( rc!=SQLITE_OK ) واپس rc;
    }
  }

  أكد( rc==SQLITE_OK );
  اگر( p->نقاب&TESTVFS_SHMOPEN_نقاب && tvfsInjectIoerr(p) ){
    واپس SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  برائے(p_بفر=p->p_بفر; p_بفر; p_بفر=p_بفر->p_اگلا){
    اگر( 0==strcmp(pFd->zFilename, p_بفر->zFile) ) توڑو;
  }
  اگر( !p_بفر ){
    صحیح szName = (صحیح)لڑی_لمبائی(pFd->zFilename);
    صحیح nByte = حجم(Testvfs_بفر) + szName + 1;
    p_بفر = (Testvfs_بفر *)ckalloc(nByte);
    memset(p_بفر, 0, nByte);
    p_بفر->zFile = (حرف *)&p_بفر[1];
    حافظہ_نقل(p_بفر->zFile, pFd->zFilename, szName+1);
    p_بفر->p_اگلا = p->p_بفر;
    p->p_بفر = p_بفر;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->p_اگلا = p_بفر->pFile;
  p_بفر->pFile = pFd;
  pFd->pShm = p_بفر;
  واپس rc;
}

ساکن خالی tvfs_Alloc_صفحہ(Testvfs_بفر *p, صحیح i_صفحہ, صحیح pgsz){
  أكد( i_صفحہ<TESTVFS_MAX_صفحات );
  اگر( p->a_صفحہ[i_صفحہ]==0 ){
    p->a_صفحہ[i_صفحہ] = (u8 *)ckalloc(pgsz);
    memset(p->a_صفحہ[i_صفحہ], 0, pgsz);
    p->pgsz = pgsz;
  }
}

ساکن صحیح tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  صحیح i_صفحہ,                      /* Page to retrieve */
  صحیح pgsz,                       /* Size of pages */
  صحیح isWrite,                    /* True to extend file if necessary */
  خالی متلون **pp              /* OUT: Mapped memory */
){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_ڈیٹا);

  اگر( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    واپس pReal->pMethods->xShmMap(pReal, i_صفحہ, pgsz, isWrite, pp);
  }

  اگر( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    اگر( rc!=SQLITE_OK ){
      واپس rc;
    }
  }

  اگر( p->pScript && p->نقاب&TESTVFS_SHMMAP_نقاب ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_گنتی(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(i_صفحہ));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfs_نتیجہ_رمز(p, &rc);
    Tcl_Decr_Ref_گنتی(pArg);
  }
  اگر( rc==SQLITE_OK && p->نقاب&TESTVFS_SHMMAP_نقاب && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  اگر( rc==SQLITE_OK && isWrite && !pFd->pShm->a_صفحہ[i_صفحہ] ){
    tvfs_Alloc_صفحہ(pFd->pShm, i_صفحہ, pgsz);
  }
  اگر( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (خالی متلون *)pFd->pShm->a_صفحہ[i_صفحہ];
  }

  واپس rc;
}


ساکن صحیح tvfs_Shm_تالا(
  sqlite3_file *pFile,
  صحیح ofst,
  صحیح n,
  صحیح flags
){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_ڈیٹا);
  صحیح n_تالا;
  حرف z_تالا[80];

  اگر( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    واپس pReal->pMethods->x_Shm_تالا(pReal, ofst, n, flags);
  }

  اگر( p->pScript && p->نقاب&TESTVFS_SHMLOCK_نقاب ){
    sqlite3_snprintf(حجم(z_تالا), z_تالا, "%d %d", ofst, n);
    n_تالا = (صحیح)لڑی_لمبائی(z_تالا);
    اگر( flags & SQLITE_SHM_تالا ){
      strcpy(&z_تالا[n_تالا], " lock");
    }ورنہ{
      strcpy(&z_تالا[n_تالا], " unlock");
    }
    n_تالا += (صحیح)لڑی_لمبائی(&z_تالا[n_تالا]);
    اگر( flags & SQLITE_SHM_SHARED ){
      strcpy(&z_تالا[n_تالا], " shared");
    }ورنہ{
      strcpy(&z_تالا[n_تالا], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(z_تالا, -1), 0
    );
    tvfs_نتیجہ_رمز(p, &rc);
  }

  اگر( rc==SQLITE_OK && p->نقاب&TESTVFS_SHMLOCK_نقاب && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  اگر( rc==SQLITE_OK ){
    صحیح is_تالا = (flags & SQLITE_SHM_تالا);
    صحیح isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 نقاب = (((1<<n)-1) << ofst);
    اگر( is_تالا ){
      TestvfsFd *p2;
      برائے(p2=pFd->pShm->pFile; p2; p2=p2->p_اگلا){
        اگر( p2==pFd ) جاری;
        اگر( (p2->excllock&نقاب) || (isExcl && p2->sharedlock&نقاب) ){
          rc = SQLITE_BUSY;
          توڑو;
        }
      }
      اگر( rc==SQLITE_OK ){
        اگر( isExcl )  pFd->excllock |= نقاب;
        اگر( !isExcl ) pFd->sharedlock |= نقاب;
      }
    }ورنہ{
      اگر( isExcl )  pFd->excllock &= (~نقاب);
      اگر( !isExcl ) pFd->sharedlock &= (~نقاب);
    }
  }

  واپس rc;
}

ساکن خالی tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_ڈیٹا);

  اگر( p->pScript && p->نقاب&TESTVFS_SHMBARRIER_نقاب ){
    ثابت حرف *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  اگر( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    واپس;
  }
}

ساکن صحیح tvfsShmUnmap(
  sqlite3_file *pFile,
  صحیح deleteFlag
){
  صحیح rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_ڈیٹا);
  Testvfs_بفر *p_بفر = pFd->pShm;
  TestvfsFd **ppFd;

  اگر( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    واپس pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  اگر( !p_بفر ) واپس SQLITE_OK;
  أكد( pFd->pShmId && pFd->pShm );

  اگر( p->pScript && p->نقاب&TESTVFS_SHMCLOSE_نقاب ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfs_نتیجہ_رمز(p, &rc);
  }

  برائے(ppFd=&p_بفر->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->p_اگلا));
  أكد( (*ppFd)==pFd );
  *ppFd = pFd->p_اگلا;
  pFd->p_اگلا = 0;

  اگر( p_بفر->pFile==0 ){
    صحیح i;
    Testvfs_بفر **pp;
    برائے(pp=&p->p_بفر; *pp!=p_بفر; pp=&((*pp)->p_اگلا));
    *pp = (*pp)->p_اگلا;
    برائے(i=0; p_بفر->a_صفحہ[i]; i++){
      ckfree((حرف *)p_بفر->a_صفحہ[i]);
    }
    ckfree((حرف *)p_بفر);
  }
  pFd->pShm = 0;

  واپس rc;
}

ساکن صحیح tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    صحیح iAmt, 
    خالی **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  واپس sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

ساکن صحیح tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, خالی *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  واپس sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

ساکن صحیح SQLITE_TCLAPI testvfs_obj_cmd(
  Client_ڈیٹا cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  شمار DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  ساخت TestvfsSubcmd {
    حرف *zName;
    شمار DB_enum eCmd;
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
  صحیح i;
  
  اگر( objc<2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "SUBCOMMAND ...");
    واپس TCL_ERROR;
  }
  اگر( Tcl_Get_اشاریہ_From_Obj_Struct(
        interp, objv[1], aSubcmd, حجم(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    واپس TCL_ERROR;
  }
  Tcl_Reset_نتیجہ(interp);

  انتخاب( aSubcmd[i].eCmd ){
    حالت CMD_SHM: {
      Tcl_Obj *pObj;
      صحیح rc;
      Testvfs_بفر *p_بفر;
      حرف *zName;
      اگر( objc!=3 && objc!=4 ){
        Tcl_Wrong_عدد_دلائل(interp, 2, objv, "FILE ?VALUE?");
        واپس TCL_ERROR;
      }
      zName = ckalloc(p->p_والد->mxPathname);
      rc = p->p_والد->x_بھرا_Pathname(
          p->p_والد, Tcl_GetString(objv[2]), 
          p->p_والد->mxPathname, zName
      );
      اگر( rc!=SQLITE_OK ){
        Tcl_Append_نتیجہ(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), تہی);
        ckfree(zName);
        واپس TCL_ERROR;
      }
      برائے(p_بفر=p->p_بفر; p_بفر; p_بفر=p_بفر->p_اگلا){
        اگر( 0==strcmp(p_بفر->zFile, zName) ) توڑو;
      }
      ckfree(zName);
      اگر( !p_بفر ){
        Tcl_Append_نتیجہ(interp, "no such file: ", Tcl_GetString(objv[2]), تہی);
        واپس TCL_ERROR;
      }
      اگر( objc==4 ){
        Tcl_سائز n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        صحیح pgsz = p_بفر->pgsz;
        اگر( pgsz==0 ) pgsz = 65536;
        برائے(i=0; i*pgsz<(صحیح)n; i++){
          صحیح nByte = pgsz;
          tvfs_Alloc_صفحہ(p_بفر, i, pgsz);
          اگر( n-i*pgsz<pgsz ){
            nByte = (صحیح)n;
          }
          حافظہ_نقل(p_بفر->a_صفحہ[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      برائے(i=0; p_بفر->a_صفحہ[i]; i++){
        صحیح pgsz = p_بفر->pgsz;
        اگر( pgsz==0 ) pgsz = 65536;
        Tcl_Append_Obj_مقصد_Obj(pObj, Tcl_NewByteArrayObj(p_بفر->a_صفحہ[i], pgsz));
      }
      Tcl_سیٹ_Obj_نتیجہ(interp, pObj);
      توڑو;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    حالت CMD_FILTER: {
      ساکن ساخت Vfs_طریقہ {
        حرف *zName;
        صحیح نقاب;
      } vfsmethod [] = {
        { "xShmOpen",           TESTVFS_SHMOPEN_نقاب },
        { "xShmLock",           TESTVFS_SHMLOCK_نقاب },
        { "xShmBarrier",        TESTVFS_SHMBARRIER_نقاب },
        { "xShmUnmap",          TESTVFS_SHMCLOSE_نقاب },
        { "xShmMap",            TESTVFS_SHMMAP_نقاب },
        { "xSync",              TESTVFS_SYNC_نقاب },
        { "xDelete",            TESTVFS_DELETE_نقاب },
        { "xWrite",             TESTVFS_WRITE_نقاب },
        { "xRead",              TESTVFS_READ_نقاب },
        { "xTruncate",          TESTVFS_TRUNCATE_نقاب },
        { "xOpen",              TESTVFS_OPEN_نقاب },
        { "xClose",             TESTVFS_CLOSE_نقاب },
        { "xAccess",            TESTVFS_ACCESS_نقاب },
        { "xFullPathname",      TESTVFS_FULLPATHNAME_نقاب },
        { "xUnlock",            TESTVFS_UNLOCK_نقاب },
        { "xLock",              TESTVFS_تالا_نقاب },
        { "xCheckReservedLock", TESTVFS_CKLOCK_نقاب },
        { "xFileControl",       TESTVFS_FCNTL_نقاب },
        { "xSleep",             TESTVFS_SLEEP_نقاب },
      };
      Tcl_Obj **apElem = 0;
      Tcl_سائز nElem = 0;
      صحیح نقاب = 0;
      اگر( objc!=3 ){
        Tcl_Wrong_عدد_دلائل(interp, 2, objv, "LIST");
        واپس TCL_ERROR;
      }
      اگر( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        واپس TCL_ERROR;
      }
      Tcl_Reset_نتیجہ(interp);
      برائے(i=0; i<(صحیح)nElem; i++){
        صحیح i_طریقہ;
        حرف *zElem = Tcl_GetString(apElem[i]);
        برائے(i_طریقہ=0; i_طریقہ<Array_سائز(vfsmethod); i_طریقہ++){
          اگر( strcmp(zElem, vfsmethod[i_طریقہ].zName)==0 ){
            نقاب |= vfsmethod[i_طریقہ].نقاب;
            توڑو;
          }
        }
        اگر( i_طریقہ==Array_سائز(vfsmethod) ){
          Tcl_Append_نتیجہ(interp, "unknown method: ", zElem, تہی);
          واپس TCL_ERROR;
        }
      }
      p->نقاب = نقاب;
      توڑو;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    حالت CMD_SCRIPT: {
      اگر( objc==3 ){
        Tcl_سائز nByte;
        اگر( p->pScript ){
          Tcl_Decr_Ref_گنتی(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        اگر( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_گنتی(p->pScript);
        }
      }ورنہ اگر( objc!=2 ){
        Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?SCRIPT?");
        واپس TCL_ERROR;
      }

      Tcl_Reset_نتیجہ(interp);
      اگر( p->pScript ) Tcl_سیٹ_Obj_نتیجہ(interp, p->pScript);

      توڑو;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    حالت CMD_CANTOPENERR:
    حالت CMD_IOERR:
    حالت CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      صحیح iRet;

      انتخاب( aSubcmd[i].eCmd ){
        حالت CMD_IOERR: pTest = &p->ioerr_err; توڑو;
        حالت CMD_FULLERR: pTest = &p->بھرا_err; توڑو;
        حالت CMD_CANTOPENERR: pTest = &p->cantopen_err; توڑو;
        طےشدہ: أكد(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      اگر( objc==4 ){
        صحیح iCnt, iPersist;
        اگر( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          واپس TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }ورنہ اگر( objc!=2 ){
        Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?CNT PERSIST?");
        واپس TCL_ERROR;
      }
      Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(iRet));
      توڑو;
    }

    حالت CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      توڑو;
    }

    حالت CMD_DEVCHAR: {
      ساخت DeviceFlag {
        حرف *zName;
        صحیح i_قدر;
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
        { "safe_append",           SQLITE_IOCAP_محفوظ_APPEND           },
        { "undeletable_when_open", SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN },
        { "powersafe_overwrite",   SQLITE_IOCAP_POWERSAFE_OVERWRITE   },
        { "immutable",             SQLITE_IOCAP_IMMUTABLE             },
        { 0, 0 }
      };
      Tcl_Obj *pRet;
      صحیح iFlag;

      اگر( objc>3 ){
        Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?ATTR-LIST?");
        واپس TCL_ERROR;
      }
      اگر( objc==3 ){
        صحیح j;
        صحیح iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_سائز nFlags = 0;

        اگر( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          واپس TCL_ERROR;
        }

        برائے(j=0; j<(صحیح)nFlags; j++){
          صحیح idx = 0;
          اگر( Tcl_Get_اشاریہ_From_Obj_Struct(interp, flags[j], aFlag, 
                حجم(aFlag[0]), "flag", 0, &idx) 
          ){
            واپس TCL_ERROR;
          }
          اگر( aFlag[idx].i_قدر<0 && nFlags>1 ){
            Tcl_Append_نتیجہ(interp, "bad flags: ", Tcl_GetString(objv[2]), تہی);
            واپس TCL_ERROR;
          }
          iNew |= aFlag[idx].i_قدر;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      برائے(iFlag=0; iFlag<حجم(aFlag)/حجم(aFlag[0]); iFlag++){
        اگر( p->iDevchar & aFlag[iFlag].i_قدر ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_سیٹ_Obj_نتیجہ(interp, pRet);

      توڑو;
    }

    حالت CMD_SECTORSIZE: {
      اگر( objc>3 ){
        Tcl_Wrong_عدد_دلائل(interp, 2, objv, "?VALUE?");
        واپس TCL_ERROR;
      }
      اگر( objc==3 ){
        صحیح iNew = 0;
        اگر( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          واپس TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(p->iSectorsize));
      توڑو;
    }
  }

  واپس TCL_OK;
}

ساکن خالی SQLITE_TCLAPI testvfs_obj_del(Client_ڈیٹا cd){
  Testvfs *p = (Testvfs *)cd;
  اگر( p->pScript ) Tcl_Decr_Ref_گنتی(p->pScript);
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
ساکن صحیح SQLITE_TCLAPI testvfs_cmd(
  Client_ڈیٹا cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ساکن sqlite3_vfs tvfs_vfs = {
    3,                            /* iVersion */
    0,                            /* szOsFile */
    0,                            /* mxPathname */
    0,                            /* pNext */
    0,                            /* zName */
    0,                            /* pAppData */
    tvfsOpen,                     /* xOpen */
    tvfsDelete,                   /* xDelete */
    tvfsAccess,                   /* xAccess */
    tvfs_بھرا_Pathname,             /* xFullPathname */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_بھرو_EXTENSION
    tvfsDlOpen,                   /* xDlOpen */
    tvfsDlError,                  /* xDlError */
    tvfsDlSym,                    /* xDlSym */
    tvfsDlClose,                  /* xDlClose */
#ورنہ
    0,                            /* xDlOpen */
    0,                            /* xDlError */
    0,                            /* xDlSym */
    0,                            /* xDlClose */
#اگر_ختم /* SQLITE_OMIT_LOAD_EXTENSION */
    tvfsRandomness,               /* xRandomness */
    tvfsSleep,                    /* xSleep */
    tvfs_Current_وقت,              /* xCurrentTime */
    0,                            /* xGetLastError */
    0,                            /* xCurrentTimeInt64 */
    0,                            /* xSetSystemCall */
    0,                            /* xGetSystemCall */
    0,                            /* xNextSystemCall */
  };

  Testvfs *p;                     /* New object */
  sqlite3_vfs *pVfs;              /* New VFS */
  حرف *zVfs;
  صحیح nByte;                      /* Bytes of space to allocate at p */

  صحیح i;
  صحیح isNoshm = 0;                /* True if -noshm is passed */
  صحیح isFullshm = 0;              /* True if -fullshm is passed */
  صحیح isDefault = 0;              /* True if -default is passed */
  صحیح szOsFile = 0;               /* Value passed to -szosfile */
  صحیح mxPathname = -1;            /* Value passed to -mxpathname */
  صحیح i_نسخہ = 3;               /* Value passed to -iversion */

  اگر( objc<2 || 0!=(objc%2) ) جاؤ bad_دلائل;
  برائے(i=2; i<objc; i += 2){
    Tcl_سائز nSwitch;
    حرف *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    اگر( nSwitch>2 && 0==لڑی_موازنہ("-noshm", zSwitch, nSwitch) ){
      اگر( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        واپس TCL_ERROR;
      }
      اگر( isNoshm ) isFullshm = 0;
    }
    ورنہ اگر( nSwitch>2 && 0==لڑی_موازنہ("-default", zSwitch, nSwitch) ){
      اگر( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        واپس TCL_ERROR;
      }
    }
    ورنہ اگر( nSwitch>2 && 0==لڑی_موازنہ("-szosfile", zSwitch, nSwitch) ){
      اگر( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        واپس TCL_ERROR;
      }
    }
    ورنہ اگر( nSwitch>2 && 0==لڑی_موازنہ("-mxpathname", zSwitch, nSwitch) ){
      اگر( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        واپس TCL_ERROR;
      }
    }
    ورنہ اگر( nSwitch>2 && 0==لڑی_موازنہ("-iversion", zSwitch, nSwitch) ){
      اگر( Tcl_GetIntFromObj(interp, objv[i+1], &i_نسخہ) ){
        واپس TCL_ERROR;
      }
    }
    ورنہ اگر( nSwitch>2 && 0==لڑی_موازنہ("-fullshm", zSwitch, nSwitch) ){
      اگر( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        واپس TCL_ERROR;
      }
      اگر( isFullshm ) isNoshm = 0;
    }
    ورنہ{
      جاؤ bad_دلائل;
    }
  }

  اگر( szOsFile<حجم(TestvfsFile) ){
    szOsFile = حجم(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = حجم(Testvfs) + (صحیح)لڑی_لمبائی(zVfs)+1;
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
  p->p_والد = sqlite3_vfs_ڈھونڈو(0);
  p->interp = interp;

  p->zName = (حرف *)&p[1];
  حافظہ_نقل(p->zName, zVfs, لڑی_لمبائی(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(حجم(sqlite3_vfs));
  حافظہ_نقل(pVfs, &tvfs_vfs, حجم(sqlite3_vfs));
  pVfs->p_App_ڈیٹا = (خالی *)p;
  pVfs->i_نسخہ = i_نسخہ;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->p_والد->mxPathname;
  اگر( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->نقاب = TESTVFS_ALL_نقاب;

  sqlite3_vfs_رجسٹر(pVfs, isDefault);

  واپس TCL_OK;

 bad_دلائل:
  Tcl_Wrong_عدد_دلائل(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  واپس TCL_ERROR;
}

خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجی ثابت حرف *sqlite3ErrName(صحیح);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
ساکن صحیح SQLITE_TCLAPI test_vfs_shmlock(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  ثابت حرف *azArg1[] = {"shared", "exclusive", 0};
  ثابت حرف *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  صحیح rc = SQLITE_OK;
  ثابت حرف *zDbname = 0;
  صحیح iArg1 = 0;
  صحیح iArg2 = 0;
  صحیح i_انحراف = 0;
  صحیح n = 0;
  sqlite3_file *pFd;

  اگر( objc!=7 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    واپس TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_Get_اشاریہ_From_Obj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_Get_اشاریہ_From_Obj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &i_انحراف)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    واپس TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_اشارہ_گر, (خالی*)&pFd);
  اگر( pFd==0 ){
    واپس TCL_ERROR;
  }
  rc = pFd->pMethods->x_Shm_تالا(pFd, i_انحراف, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_تالا : SQLITE_SHM_UNLOCK)
  );
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_vfs_سیٹ_readmark(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  صحیح rc = SQLITE_OK;
  ثابت حرف *zDbname = 0;
  صحیح i_خانہ = 0;
  صحیح iVal = -1;
  sqlite3_file *pFd;
  خالی متلون *pShm = 0;
  u32 *aShm;
  صحیح iOff;

  اگر( objc!=4 && objc!=5 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    واپس TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &i_خانہ)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    واپس TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_اشارہ_گر, (خالی*)&pFd);
  اگر( pFd==0 ){
    واپس TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  اگر( rc!=SQLITE_OK ){
    Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    واپس TCL_ERROR;
  }
  اگر( pShm==0 ){
    Tcl_Append_نتیجہ(interp, "*-shm is not yet mapped", تہی);
    واپس TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+i_خانہ;

  اگر( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(aShm[iOff]));

  واپس TCL_OK;
}

صحیح Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_سیٹ_readmark, 0, 0);
  واپس TCL_OK;
}

#اگر_ختم
