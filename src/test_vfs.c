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
#यदि SQLITE_TEST          /* This file is used for testing only */

#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"

प्रकारनिर्देश संरचना Testvfs Testvfs;
प्रकारनिर्देश संरचना TestvfsShm TestvfsShm;
प्रकारनिर्देश संरचना Testvfs_बफर Testvfs_बफर;
प्रकारनिर्देश संरचना TestvfsFile TestvfsFile;
प्रकारनिर्देश संरचना TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
संरचना TestvfsFile {
  sqlite3_file आधारः;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#परिभाषय tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

संरचना TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  ध्रुव वर्ण *zFilename;          /* Filename as passed to xOpen() */
  sqlite3_file *pReal;            /* The real, underlying file descriptor */
  Tcl_Obj *pShmId;                /* Shared memory id for Tcl callbacks */

  Testvfs_बफर *pShm;            /* Shared memory buffer */
  u32 excllock;                   /* Mask of exclusive locks */
  u32 sharedlock;                 /* Mask of shared locks */
  TestvfsFd *p_अग्र;               /* Next handle opened on the same file */
};


#परिभाषय FAULT_INJECT_NONE       0
#परिभाषय FAULT_INJECT_TRANSIENT  1
#परिभाषय FAULT_INJECT_PERSISTENT 2

प्रकारनिर्देश संरचना TestFaultInject TestFaultInject;
संरचना TestFaultInject {
  पूर्णाङ्क iCnt;                       /* Remaining calls before fault injection */
  पूर्णाङ्क eFault;                     /* A FAULT_INJECT_* value */
  पूर्णाङ्क nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
संरचना Testvfs {
  वर्ण *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  Testvfs_बफर *p_बफर;         /* List of shared buffers */
  पूर्णाङ्क isNoshm;
  पूर्णाङ्क isFullshm;

  पूर्णाङ्क आवरणम्;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject पूर्णम्_err;
  TestFaultInject cantopen_err;

#यदि 0
  पूर्णाङ्क iIoerrCnt;
  पूर्णाङ्क ioerr;
  पूर्णाङ्क nIoerrFail;
  पूर्णाङ्क i_पूर्णम्_Cnt;
  पूर्णाङ्क fullerr;
  पूर्णाङ्क n_पूर्णम्_Fail;
#यदि_अन्तः

  पूर्णाङ्क iDevchar;
  पूर्णाङ्क iSectorsize;
};

/*
** The Testvfs.mask variable is set to a combination of the following.
** If a bit is clear in Testvfs.mask, then calls made by SQLite to the 
** corresponding VFS method is ignored for purposes of:
**
**   + Simulating IO errors, and
**   + Invoking the Tcl callback script.
*/
#परिभाषय TESTVFS_SHMOPEN_आवरणम्      0x00000001
#परिभाषय TESTVFS_SHMLOCK_आवरणम्      0x00000010
#परिभाषय TESTVFS_SHMMAP_आवरणम्       0x00000020
#परिभाषय TESTVFS_SHMBARRIER_आवरणम्   0x00000040
#परिभाषय TESTVFS_SHMCLOSE_आवरणम्     0x00000080

#परिभाषय TESTVFS_OPEN_आवरणम्         0x00000100
#परिभाषय TESTVFS_SYNC_आवरणम्         0x00000200
#परिभाषय TESTVFS_DELETE_आवरणम्       0x00000400
#परिभाषय TESTVFS_CLOSE_आवरणम्        0x00000800
#परिभाषय TESTVFS_WRITE_आवरणम्        0x00001000
#परिभाषय TESTVFS_TRUNCATE_आवरणम्     0x00002000
#परिभाषय TESTVFS_ACCESS_आवरणम्       0x00004000
#परिभाषय TESTVFS_FULLPATHNAME_आवरणम् 0x00008000
#परिभाषय TESTVFS_READ_आवरणम्         0x00010000
#परिभाषय TESTVFS_UNLOCK_आवरणम्       0x00020000
#परिभाषय TESTVFS_तालकम्_आवरणम्         0x00040000
#परिभाषय TESTVFS_CKLOCK_आवरणम्       0x00080000
#परिभाषय TESTVFS_FCNTL_आवरणम्        0x00100000
#परिभाषय TESTVFS_SLEEP_आवरणम्        0x00200000

#परिभाषय TESTVFS_ALL_आवरणम्          0x003FFFFF


#परिभाषय TESTVFS_MAX_पृष्ठानि 1024

/*
** A shared-memory buffer. There is one of these objects for each shared
** memory region opened by clients. If two clients open the same file,
** there are two TestvfsFile structures but only one TestvfsBuffer structure.
*/
संरचना Testvfs_बफर {
  वर्ण *zFile;                    /* Associated file name */
  पूर्णाङ्क pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_पृष्ठानि];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  Testvfs_बफर *p_अग्र;           /* Next in linked list of all buffers */
};


#परिभाषय PARENTVFS(x) (((Testvfs *)((x)->p_App_दत्त))->pParent)

#परिभाषय TESTVFS_MAX_तर्काः 12


/*
** Method declarations for TestvfsFile.
*/
स्थिर पूर्णाङ्क tvfsClose(sqlite3_file*);
स्थिर पूर्णाङ्क tvfsRead(sqlite3_file*, शून्य*, पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क tvfsWrite(sqlite3_file*,ध्रुव शून्य*,पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क tvfsTruncate(sqlite3_file*, sqlite3_int64 आकार);
स्थिर पूर्णाङ्क tvfsSync(sqlite3_file*, पूर्णाङ्क flags);
स्थिर पूर्णाङ्क tvfs_File_आकार(sqlite3_file*, sqlite3_int64 *p_आकार);
स्थिर पूर्णाङ्क tvfs_तालकम्(sqlite3_file*, पूर्णाङ्क);
स्थिर पूर्णाङ्क tvfsUnlock(sqlite3_file*, पूर्णाङ्क);
स्थिर पूर्णाङ्क tvfs_Check_Reserved_तालकम्(sqlite3_file*, पूर्णाङ्क *);
स्थिर पूर्णाङ्क tvfsFileControl(sqlite3_file*, पूर्णाङ्क op, शून्य *pArg);
स्थिर पूर्णाङ्क tvfs_Sector_आकार(sqlite3_file*);
स्थिर पूर्णाङ्क tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
स्थिर पूर्णाङ्क tvfsOpen(sqlite3_vfs*, ध्रुव वर्ण *, sqlite3_file*, पूर्णाङ्क , पूर्णाङ्क *);
स्थिर पूर्णाङ्क tvfsDelete(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क syncDir);
स्थिर पूर्णाङ्क tvfsAccess(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क flags, पूर्णाङ्क *);
स्थिर पूर्णाङ्क tvfs_पूर्णम्_Pathname(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क, वर्ण *z_बहिर्गमनम्);
#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
स्थिर शून्य *tvfsDlOpen(sqlite3_vfs*, ध्रुव वर्ण *zFilename);
स्थिर शून्य tvfsDlError(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देशः);
स्थिर शून्य (*tvfsDlSym(sqlite3_vfs*,शून्य*, ध्रुव वर्ण *z_चिह्नम्))(शून्य);
स्थिर शून्य tvfsDlClose(sqlite3_vfs*, शून्य*);
#यदि_अन्तः /* SQLITE_OMIT_LOAD_EXTENSION */
स्थिर पूर्णाङ्क tvfsRandomness(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *z_बहिर्गमनम्);
स्थिर पूर्णाङ्क tvfsSleep(sqlite3_vfs*, पूर्णाङ्क microseconds);
स्थिर पूर्णाङ्क tvfsCurrentTime(sqlite3_vfs*, द्विगुण*);

स्थिर पूर्णाङ्क tvfsShmOpen(sqlite3_file*);
स्थिर पूर्णाङ्क tvfs_Shm_तालकम्(sqlite3_file*, पूर्णाङ्क , पूर्णाङ्क, पूर्णाङ्क);
स्थिर पूर्णाङ्क tvfsShmMap(sqlite3_file*,पूर्णाङ्क,पूर्णाङ्क,पूर्णाङ्क, शून्य चञ्चल **);
स्थिर शून्य tvfsShmBarrier(sqlite3_file*);
स्थिर पूर्णाङ्क tvfsShmUnmap(sqlite3_file*, पूर्णाङ्क);

स्थिर पूर्णाङ्क tvfsFetch(sqlite3_file*, sqlite3_int64, पूर्णाङ्क, शून्य**);
स्थिर पूर्णाङ्क tvfsUnfetch(sqlite3_file*, sqlite3_int64, शून्य*);

स्थिर sqlite3_io_methods tvfs_io_methods = {
  3,                              /* iVersion */
  tvfsClose,                      /* xClose */
  tvfsRead,                       /* xRead */
  tvfsWrite,                      /* xWrite */
  tvfsTruncate,                   /* xTruncate */
  tvfsSync,                       /* xSync */
  tvfs_File_आकार,                   /* xFileSize */
  tvfs_तालकम्,                       /* xLock */
  tvfsUnlock,                     /* xUnlock */
  tvfs_Check_Reserved_तालकम्,          /* xCheckReservedLock */
  tvfsFileControl,                /* xFileControl */
  tvfs_Sector_आकार,                 /* xSectorSize */
  tvfsDeviceCharacteristics,      /* xDeviceCharacteristics */
  tvfsShmMap,                     /* xShmMap */
  tvfs_Shm_तालकम्,                    /* xShmLock */
  tvfsShmBarrier,                 /* xShmBarrier */
  tvfsShmUnmap,                   /* xShmUnmap */
  tvfsFetch,
  tvfsUnfetch
};

स्थिर पूर्णाङ्क tvfs_परिणाम_संहिता(Testvfs *p, पूर्णाङ्क *pRc){
  संरचना errcode {
    पूर्णाङ्क e_संहिता;
    ध्रुव वर्ण *z_संहिता;
  } a_संहिता[] = {
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

  ध्रुव वर्ण *z;
  पूर्णाङ्क i;

  z = Tcl_Get_String_परिणाम(p->interp);
  प्रति(i=0; i<Array_आकार(a_संहिता); i++){
    यदि( 0==वाक्यं_तुलय(z, a_संहिता[i].z_संहिता) ){
      *pRc = a_संहिता[i].e_संहिता;
      प्रत्यागम 1;
    }
  }

  प्रत्यागम 0;
}

स्थिर पूर्णाङ्क tvfsInjectFault(TestFaultInject *p){
  पूर्णाङ्क ret = 0;
  यदि( p->eFault ){
    p->iCnt--;
    यदि( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  प्रत्यागम ret;
}


स्थिर पूर्णाङ्क tvfsInjectIoerr(Testvfs *p){
  प्रत्यागम tvfsInjectFault(&p->ioerr_err);
}

स्थिर पूर्णाङ्क tvfsInjectFullerr(Testvfs *p){
  प्रत्यागम tvfsInjectFault(&p->पूर्णम्_err);
}
स्थिर पूर्णाङ्क tvfsInjectCantopenerr(Testvfs *p){
  प्रत्यागम tvfsInjectFault(&p->cantopen_err);
}


स्थिर शून्य tvfsExecTcl(
  Testvfs *p, 
  ध्रुव वर्ण *z_विधानम्,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  पूर्णाङ्क rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  प्रतिज्ञा( p->pScript );

  प्रतिज्ञा( z_विधानम् );
  प्रतिज्ञा( p );
  प्रतिज्ञा( arg2==0 || arg1!=0 );
  प्रतिज्ञा( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_गणनम्(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(z_विधानम्, -1));
  यदि( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  यदि( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  यदि( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  यदि( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  यदि( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_Reset_परिणाम(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_दत्त;

  यदि( p->pScript && p->आवरणम्&TESTVFS_CLOSE_आवरणम् ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  यदि( pFd->pShmId ){
    Tcl_Decr_Ref_गणनम्(pFd->pShmId);
    pFd->pShmId = 0;
  }
  यदि( pFile->pMethods ){
    ckfree((वर्ण *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((वर्ण *)pFd);
  pTestfile->pFd = 0;
  प्रत्यागम SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsRead(
  sqlite3_file *pFile, 
  शून्य *zBuf, 
  पूर्णाङ्क iAmt, 
  sqlite_int64 iOfst
){
  पूर्णाङ्क rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_दत्त;
  यदि( p->pScript && p->आवरणम्&TESTVFS_READ_आवरणम् ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_परिणाम_संहिता(p, &rc);
  }
  यदि( rc==SQLITE_OK && p->आवरणम्&TESTVFS_READ_आवरणम् && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  यदि( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  प्रत्यागम rc;
}

/*
** Write data to an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsWrite(
  sqlite3_file *pFile, 
  ध्रुव शून्य *zBuf, 
  पूर्णाङ्क iAmt, 
  sqlite_int64 iOfst
){
  पूर्णाङ्क rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_दत्त;

  यदि( p->pScript && p->आवरणम्&TESTVFS_WRITE_आवरणम् ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfs_परिणाम_संहिता(p, &rc);
    यदि( rc<0 ) प्रत्यागम SQLITE_OK;
  }

  यदि( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_पूर्णम्;
  }
  यदि( rc==SQLITE_OK && p->आवरणम्&TESTVFS_WRITE_आवरणम् && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  यदि( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  प्रत्यागम rc;
}

/*
** Truncate an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsTruncate(sqlite3_file *pFile, sqlite_int64 आकार){
  पूर्णाङ्क rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_दत्त;

  यदि( p->pScript && p->आवरणम्&TESTVFS_TRUNCATE_आवरणम् ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_परिणाम_संहिता(p, &rc);
  }
  
  यदि( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, आकार);
  }
  प्रत्यागम rc;
}

/*
** Sync an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsSync(sqlite3_file *pFile, पूर्णाङ्क flags){
  पूर्णाङ्क rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_दत्त;

  यदि( p->pScript && p->आवरणम्&TESTVFS_SYNC_आवरणम् ){
    वर्ण *zFlags = 0;

    विकल्प( flags ){
      पक्ष SQLITE_SYNC_प्रसामान्यम्:
        zFlags = "normal";
        भङ्ग;
      पक्ष SQLITE_SYNC_पूर्णम्:
        zFlags = "full";
        भङ्ग;
      पक्ष SQLITE_SYNC_प्रसामान्यम्|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        भङ्ग;
      पक्ष SQLITE_SYNC_पूर्णम्|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        भङ्ग;
      स्वभाव:
        प्रतिज्ञा(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfs_परिणाम_संहिता(p, &rc);
  }

  यदि( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_पूर्णम्;

  यदि( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  प्रत्यागम rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfs_File_आकार(sqlite3_file *pFile, sqlite_int64 *p_आकार){
  TestvfsFd *p = tvfsGetFd(pFile);
  प्रत्यागम sqlite3_Os_File_आकार(p->pReal, p_आकार);
}

/*
** Lock an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfs_तालकम्(sqlite3_file *pFile, पूर्णाङ्क e_तालकम्){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_दत्त;
  यदि( p->pScript && p->आवरणम्&TESTVFS_तालकम्_आवरणम् ){
    वर्ण z_तालकम्[30];
    sqlite3_snprintf(परिमाण(z_तालकम्),z_तालकम्,"%d",e_तालकम्);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(z_तालकम्, -1), 0, 0);
  }
  यदि( p->आवरणम्&TESTVFS_तालकम्_आवरणम् && tvfsInjectIoerr(p) ){
    प्रत्यागम SQLITE_IOERR_तालकम्;
  }
  प्रत्यागम sqlite3_Os_तालकम्(pFd->pReal, e_तालकम्);
}

/*
** Unlock an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsUnlock(sqlite3_file *pFile, पूर्णाङ्क e_तालकम्){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_दत्त;
  यदि( p->pScript && p->आवरणम्&TESTVFS_UNLOCK_आवरणम् ){
    वर्ण z_तालकम्[30];
    sqlite3_snprintf(परिमाण(z_तालकम्),z_तालकम्,"%d",e_तालकम्);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(z_तालकम्, -1), 0, 0);
  }
  यदि( p->आवरणम्&TESTVFS_UNLOCK_आवरणम् && tvfsInjectIoerr(p) ){
    प्रत्यागम SQLITE_IOERR_UNLOCK;
  }
  प्रत्यागम sqlite3OsUnlock(pFd->pReal, e_तालकम्);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfs_Check_Reserved_तालकम्(sqlite3_file *pFile, पूर्णाङ्क *p_Res_बहिर्गमनम्){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_दत्त;
  यदि( p->pScript && p->आवरणम्&TESTVFS_CKLOCK_आवरणम् ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  प्रत्यागम sqlite3_Os_Check_Reserved_तालकम्(pFd->pReal, p_Res_बहिर्गमनम्);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsFileControl(sqlite3_file *pFile, पूर्णाङ्क op, शून्य *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_दत्त;
  यदि( op==SQLITE_FCNTL_PRAGMA ){
    वर्ण **argv = (वर्ण**)pArg;
    यदि( sqlite3_stricmp(argv[1],"error")==0 ){
      पूर्णाङ्क rc = SQLITE_ERROR;
      यदि( argv[2] ){
        ध्रुव वर्ण *z = argv[2];
        पूर्णाङ्क x = atoi(z);
        यदि( x ){
          rc = x;
          यावत्( sqlite3Isdigit(z[0]) ){ z++; }
          यावत्( sqlite3Isspace(z[0]) ){ z++; }
        }
        यदि( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      प्रत्यागम rc;
    }
    यदि( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      प्रत्यागम SQLITE_OK;
    }
  }
  यदि( p->pScript && (p->आवरणम्&TESTVFS_FCNTL_आवरणम्) ){
    संरचना Fcntl {
      पूर्णाङ्क iFnctl;
      ध्रुव वर्ण *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    पूर्णाङ्क i;
    प्रति(i=0; i<परिमाण(aF)/परिमाण(aF[0]); i++){
      यदि( op==aF[i].iFnctl ) भङ्ग;
    }
    यदि( i<परिमाण(aF)/परिमाण(aF[0]) ){
      पूर्णाङ्क rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfs_परिणाम_संहिता(p, &rc);
      यदि( rc ) प्रत्यागम (rc<0 ? SQLITE_OK : rc);
    }
  }
  प्रत्यागम sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfs_Sector_आकार(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_दत्त;
  यदि( p->iSectorsize>=0 ){
    प्रत्यागम p->iSectorsize;
  }
  प्रत्यागम sqlite3_Os_Sector_आकार(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_दत्त;
  यदि( p->iDevchar>=0 ){
    प्रत्यागम p->iDevchar;
  }
  प्रत्यागम sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
स्थिर पूर्णाङ्क tvfsOpen(
  sqlite3_vfs *pVfs,
  ध्रुव वर्ण *zName,
  sqlite3_file *pFile,
  पूर्णाङ्क flags,
  पूर्णाङ्क *p_बहिर्गमनम्_Flags
){
  पूर्णाङ्क rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->p_App_दत्त;

  pFd = (TestvfsFd *)ckalloc(परिमाण(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  स्मृतिं_पूरय(pFd, 0, परिमाण(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  स्मृतिं_पूरय(pTestfile, 0, परिमाण(TestvfsFile));
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
  Tcl_Reset_परिणाम(p->interp);
  यदि( p->pScript && p->आवरणम्&TESTVFS_OPEN_आवरणम् ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_गणनम्(pArg);
    यदि( flags&SQLITE_OPEN_MAIN_DB ){
      ध्रुव वर्ण *z = &zName[वाक्यदैर्घ्यम्(zName)+1];
      यावत्( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += वाक्यदैर्घ्यम्(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += वाक्यदैर्घ्यम्(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_गणनम्(pArg);
    यदि( tvfs_परिणाम_संहिता(p, &rc) ){
      यदि( rc!=SQLITE_OK ) प्रत्यागम rc;
    }अन्यथा{
      pId = Tcl_Get_Obj_परिणाम(p->interp);
    }
  }

  यदि( (p->आवरणम्&TESTVFS_OPEN_आवरणम्) &&  tvfsInjectIoerr(p) ) प्रत्यागम SQLITE_IOERR;
  यदि( tvfsInjectCantopenerr(p) ) प्रत्यागम SQLITE_CANTOPEN;
  यदि( tvfsInjectFullerr(p) ) प्रत्यागम SQLITE_पूर्णम्;

  यदि( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_गणनम्(pId);
  pFd->pShmId = pId;
  Tcl_Reset_परिणाम(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, p_बहिर्गमनम्_Flags);
  यदि( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    पूर्णाङ्क nByte;

    यदि( pVfs->i_संस्करणम्>1 ){
      nByte = परिमाण(sqlite3_io_methods);
    }अन्यथा{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    स्मृतिं_प्रतिलिख(pMethods, &tvfs_io_methods, nByte);
    pMethods->i_संस्करणम् = pFd->pReal->pMethods->i_संस्करणम्;
    यदि( pMethods->i_संस्करणम्>pVfs->i_संस्करणम् ){
      pMethods->i_संस्करणम् = pVfs->i_संस्करणम्;
    }
    यदि( pVfs->i_संस्करणम्>1 && ((Testvfs *)pVfs->p_App_दत्त)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->x_Shm_तालकम् = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  प्रत्यागम rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
स्थिर पूर्णाङ्क tvfsDelete(sqlite3_vfs *pVfs, ध्रुव वर्ण *z_मार्गः, पूर्णाङ्क dirSync){
  पूर्णाङ्क rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->p_App_दत्त;

  यदि( p->pScript && p->आवरणम्&TESTVFS_DELETE_आवरणम् ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(z_मार्गः, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfs_परिणाम_संहिता(p, &rc);
  }
  यदि( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), z_मार्गः, dirSync);
  }
  प्रत्यागम rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
स्थिर पूर्णाङ्क tvfsAccess(
  sqlite3_vfs *pVfs, 
  ध्रुव वर्ण *z_मार्गः, 
  पूर्णाङ्क flags, 
  पूर्णाङ्क *p_Res_बहिर्गमनम्
){
  Testvfs *p = (Testvfs *)pVfs->p_App_दत्त;
  यदि( p->pScript && p->आवरणम्&TESTVFS_ACCESS_आवरणम् ){
    पूर्णाङ्क rc;
    वर्ण *zArg = 0;
    यदि( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    यदि( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    यदि( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(z_मार्गः, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    यदि( tvfs_परिणाम_संहिता(p, &rc) ){
      यदि( rc!=SQLITE_OK ) प्रत्यागम rc;
    }अन्यथा{
      Tcl_Interp *interp = p->interp;
      यदि( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_Get_Obj_परिणाम(interp), p_Res_बहिर्गमनम्) ){
        प्रत्यागम SQLITE_OK;
      }
    }
  }
  प्रत्यागम sqlite3OsAccess(PARENTVFS(pVfs), z_मार्गः, flags, p_Res_बहिर्गमनम्);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
स्थिर पूर्णाङ्क tvfs_पूर्णम्_Pathname(
  sqlite3_vfs *pVfs, 
  ध्रुव वर्ण *z_मार्गः, 
  पूर्णाङ्क n_बहिर्गमनम्, 
  वर्ण *z_बहिर्गमनम्
){
  Testvfs *p = (Testvfs *)pVfs->p_App_दत्त;
  यदि( p->pScript && p->आवरणम्&TESTVFS_FULLPATHNAME_आवरणम् ){
    पूर्णाङ्क rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(z_मार्गः, -1), 0, 0, 0);
    यदि( tvfs_परिणाम_संहिता(p, &rc) ){
      यदि( rc!=SQLITE_OK ) प्रत्यागम rc;
    }
  }
  प्रत्यागम sqlite3_Os_पूर्णम्_Pathname(PARENTVFS(pVfs), z_मार्गः, n_बहिर्गमनम्, z_बहिर्गमनम्);
}

#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
स्थिर शून्य *tvfsDlOpen(sqlite3_vfs *pVfs, ध्रुव वर्ण *z_मार्गः){
  प्रत्यागम sqlite3OsDlOpen(PARENTVFS(pVfs), z_मार्गः);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
स्थिर शून्य tvfsDlError(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *z_Err_सन्देशः){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_सन्देशः);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
स्थिर शून्य (*tvfsDlSym(sqlite3_vfs *pVfs, शून्य *p, ध्रुव वर्ण *zSym))(शून्य){
  प्रत्यागम sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
स्थिर शून्य tvfsDlClose(sqlite3_vfs *pVfs, शून्य *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#यदि_अन्तः /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
स्थिर पूर्णाङ्क tvfsRandomness(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *z_Buf_बहिर्गमनम्){
  प्रत्यागम sqlite3OsRandomness(PARENTVFS(pVfs), nByte, z_Buf_बहिर्गमनम्);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
स्थिर पूर्णाङ्क tvfsSleep(sqlite3_vfs *pVfs, पूर्णाङ्क nMicro){
  Testvfs *p = (Testvfs *)pVfs->p_App_दत्त;
  यदि( p->pScript && (p->आवरणम्&TESTVFS_SLEEP_आवरणम्) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  प्रत्यागम sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
स्थिर पूर्णाङ्क tvfsCurrentTime(sqlite3_vfs *pVfs, द्विगुण *p_Time_बहिर्गमनम्){
  प्रत्यागम PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), p_Time_बहिर्गमनम्);
}

स्थिर पूर्णाङ्क tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  पूर्णाङ्क rc = SQLITE_OK;             /* Return code */
  Testvfs_बफर *p_बफर;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->p_App_दत्त;
  प्रतिज्ञा( 0==p->isFullshm );
  प्रतिज्ञा( pFd->pShmId && pFd->pShm==0 && pFd->p_अग्र==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_Reset_परिणाम(p->interp);
  यदि( p->pScript && p->आवरणम्&TESTVFS_SHMOPEN_आवरणम् ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    यदि( tvfs_परिणाम_संहिता(p, &rc) ){
      यदि( rc!=SQLITE_OK ) प्रत्यागम rc;
    }
  }

  प्रतिज्ञा( rc==SQLITE_OK );
  यदि( p->आवरणम्&TESTVFS_SHMOPEN_आवरणम् && tvfsInjectIoerr(p) ){
    प्रत्यागम SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  प्रति(p_बफर=p->p_बफर; p_बफर; p_बफर=p_बफर->p_अग्र){
    यदि( 0==वाक्यं_तुलय(pFd->zFilename, p_बफर->zFile) ) भङ्ग;
  }
  यदि( !p_बफर ){
    पूर्णाङ्क szName = (पूर्णाङ्क)वाक्यदैर्घ्यम्(pFd->zFilename);
    पूर्णाङ्क nByte = परिमाण(Testvfs_बफर) + szName + 1;
    p_बफर = (Testvfs_बफर *)ckalloc(nByte);
    स्मृतिं_पूरय(p_बफर, 0, nByte);
    p_बफर->zFile = (वर्ण *)&p_बफर[1];
    स्मृतिं_प्रतिलिख(p_बफर->zFile, pFd->zFilename, szName+1);
    p_बफर->p_अग्र = p->p_बफर;
    p->p_बफर = p_बफर;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->p_अग्र = p_बफर->pFile;
  p_बफर->pFile = pFd;
  pFd->pShm = p_बफर;
  प्रत्यागम rc;
}

स्थिर शून्य tvfsAllocPage(Testvfs_बफर *p, पूर्णाङ्क iPage, पूर्णाङ्क pgsz){
  प्रतिज्ञा( iPage<TESTVFS_MAX_पृष्ठानि );
  यदि( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    स्मृतिं_पूरय(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

स्थिर पूर्णाङ्क tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  पूर्णाङ्क iPage,                      /* Page to retrieve */
  पूर्णाङ्क pgsz,                       /* Size of pages */
  पूर्णाङ्क isWrite,                    /* True to extend file if necessary */
  शून्य चञ्चल **pp              /* OUT: Mapped memory */
){
  पूर्णाङ्क rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_दत्त);

  यदि( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    प्रत्यागम pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  यदि( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    यदि( rc!=SQLITE_OK ){
      प्रत्यागम rc;
    }
  }

  यदि( p->pScript && p->आवरणम्&TESTVFS_SHMMAP_आवरणम् ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_गणनम्(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(iPage));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfs_परिणाम_संहिता(p, &rc);
    Tcl_Decr_Ref_गणनम्(pArg);
  }
  यदि( rc==SQLITE_OK && p->आवरणम्&TESTVFS_SHMMAP_आवरणम् && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  यदि( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  यदि( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (शून्य चञ्चल *)pFd->pShm->aPage[iPage];
  }

  प्रत्यागम rc;
}


स्थिर पूर्णाङ्क tvfs_Shm_तालकम्(
  sqlite3_file *pFile,
  पूर्णाङ्क ofst,
  पूर्णाङ्क n,
  पूर्णाङ्क flags
){
  पूर्णाङ्क rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_दत्त);
  पूर्णाङ्क n_तालकम्;
  वर्ण z_तालकम्[80];

  यदि( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    प्रत्यागम pReal->pMethods->x_Shm_तालकम्(pReal, ofst, n, flags);
  }

  यदि( p->pScript && p->आवरणम्&TESTVFS_SHMLOCK_आवरणम् ){
    sqlite3_snprintf(परिमाण(z_तालकम्), z_तालकम्, "%d %d", ofst, n);
    n_तालकम् = (पूर्णाङ्क)वाक्यदैर्घ्यम्(z_तालकम्);
    यदि( flags & SQLITE_SHM_तालकम् ){
      वाक्यं_प्रतिलिख(&z_तालकम्[n_तालकम्], " lock");
    }अन्यथा{
      वाक्यं_प्रतिलिख(&z_तालकम्[n_तालकम्], " unlock");
    }
    n_तालकम् += (पूर्णाङ्क)वाक्यदैर्घ्यम्(&z_तालकम्[n_तालकम्]);
    यदि( flags & SQLITE_SHM_SHARED ){
      वाक्यं_प्रतिलिख(&z_तालकम्[n_तालकम्], " shared");
    }अन्यथा{
      वाक्यं_प्रतिलिख(&z_तालकम्[n_तालकम्], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(z_तालकम्, -1), 0
    );
    tvfs_परिणाम_संहिता(p, &rc);
  }

  यदि( rc==SQLITE_OK && p->आवरणम्&TESTVFS_SHMLOCK_आवरणम् && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  यदि( rc==SQLITE_OK ){
    पूर्णाङ्क is_तालकम् = (flags & SQLITE_SHM_तालकम्);
    पूर्णाङ्क isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 आवरणम् = (((1<<n)-1) << ofst);
    यदि( is_तालकम् ){
      TestvfsFd *p2;
      प्रति(p2=pFd->pShm->pFile; p2; p2=p2->p_अग्र){
        यदि( p2==pFd ) अनुवर्त;
        यदि( (p2->excllock&आवरणम्) || (isExcl && p2->sharedlock&आवरणम्) ){
          rc = SQLITE_BUSY;
          भङ्ग;
        }
      }
      यदि( rc==SQLITE_OK ){
        यदि( isExcl )  pFd->excllock |= आवरणम्;
        यदि( !isExcl ) pFd->sharedlock |= आवरणम्;
      }
    }अन्यथा{
      यदि( isExcl )  pFd->excllock &= (~आवरणम्);
      यदि( !isExcl ) pFd->sharedlock &= (~आवरणम्);
    }
  }

  प्रत्यागम rc;
}

स्थिर शून्य tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_दत्त);

  यदि( p->pScript && p->आवरणम्&TESTVFS_SHMBARRIER_आवरणम् ){
    ध्रुव वर्ण *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  यदि( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    प्रत्यागम;
  }
}

स्थिर पूर्णाङ्क tvfsShmUnmap(
  sqlite3_file *pFile,
  पूर्णाङ्क deleteFlag
){
  पूर्णाङ्क rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_दत्त);
  Testvfs_बफर *p_बफर = pFd->pShm;
  TestvfsFd **ppFd;

  यदि( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    प्रत्यागम pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  यदि( !p_बफर ) प्रत्यागम SQLITE_OK;
  प्रतिज्ञा( pFd->pShmId && pFd->pShm );

  यदि( p->pScript && p->आवरणम्&TESTVFS_SHMCLOSE_आवरणम् ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfs_परिणाम_संहिता(p, &rc);
  }

  प्रति(ppFd=&p_बफर->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->p_अग्र));
  प्रतिज्ञा( (*ppFd)==pFd );
  *ppFd = pFd->p_अग्र;
  pFd->p_अग्र = 0;

  यदि( p_बफर->pFile==0 ){
    पूर्णाङ्क i;
    Testvfs_बफर **pp;
    प्रति(pp=&p->p_बफर; *pp!=p_बफर; pp=&((*pp)->p_अग्र));
    *pp = (*pp)->p_अग्र;
    प्रति(i=0; p_बफर->aPage[i]; i++){
      ckfree((वर्ण *)p_बफर->aPage[i]);
    }
    ckfree((वर्ण *)p_बफर);
  }
  pFd->pShm = 0;

  प्रत्यागम rc;
}

स्थिर पूर्णाङ्क tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    पूर्णाङ्क iAmt, 
    शून्य **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  प्रत्यागम sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

स्थिर पूर्णाङ्क tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, शून्य *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  प्रत्यागम sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI testvfs_obj_cmd(
  Client_दत्त cd,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  गणना DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  संरचना TestvfsSubcmd {
    वर्ण *zName;
    गणना DB_enum eCmd;
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
  पूर्णाङ्क i;
  
  यदि( objc<2 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "SUBCOMMAND ...");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_Get_सूचकांक_From_Obj_Struct(
        interp, objv[1], aSubcmd, परिमाण(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    प्रत्यागम TCL_ERROR;
  }
  Tcl_Reset_परिणाम(interp);

  विकल्प( aSubcmd[i].eCmd ){
    पक्ष CMD_SHM: {
      Tcl_Obj *pObj;
      पूर्णाङ्क rc;
      Testvfs_बफर *p_बफर;
      वर्ण *zName;
      यदि( objc!=3 && objc!=4 ){
        Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "FILE ?VALUE?");
        प्रत्यागम TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->x_पूर्णम्_Pathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      यदि( rc!=SQLITE_OK ){
        Tcl_Append_परिणाम(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), रिक्त);
        ckfree(zName);
        प्रत्यागम TCL_ERROR;
      }
      प्रति(p_बफर=p->p_बफर; p_बफर; p_बफर=p_बफर->p_अग्र){
        यदि( 0==वाक्यं_तुलय(p_बफर->zFile, zName) ) भङ्ग;
      }
      ckfree(zName);
      यदि( !p_बफर ){
        Tcl_Append_परिणाम(interp, "no such file: ", Tcl_GetString(objv[2]), रिक्त);
        प्रत्यागम TCL_ERROR;
      }
      यदि( objc==4 ){
        Tcl_आकार n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        पूर्णाङ्क pgsz = p_बफर->pgsz;
        यदि( pgsz==0 ) pgsz = 65536;
        प्रति(i=0; i*pgsz<(पूर्णाङ्क)n; i++){
          पूर्णाङ्क nByte = pgsz;
          tvfsAllocPage(p_बफर, i, pgsz);
          यदि( n-i*pgsz<pgsz ){
            nByte = (पूर्णाङ्क)n;
          }
          स्मृतिं_प्रतिलिख(p_बफर->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      प्रति(i=0; p_बफर->aPage[i]; i++){
        पूर्णाङ्क pgsz = p_बफर->pgsz;
        यदि( pgsz==0 ) pgsz = 65536;
        Tcl_Append_Obj_गन्तव्यम्_Obj(pObj, Tcl_NewByteArrayObj(p_बफर->aPage[i], pgsz));
      }
      Tcl_समुच्चयः_Obj_परिणाम(interp, pObj);
      भङ्ग;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    पक्ष CMD_FILTER: {
      स्थिर संरचना Vfs_विधानम् {
        वर्ण *zName;
        पूर्णाङ्क आवरणम्;
      } vfsmethod [] = {
        { "xShmOpen",           TESTVFS_SHMOPEN_आवरणम् },
        { "xShmLock",           TESTVFS_SHMLOCK_आवरणम् },
        { "xShmBarrier",        TESTVFS_SHMBARRIER_आवरणम् },
        { "xShmUnmap",          TESTVFS_SHMCLOSE_आवरणम् },
        { "xShmMap",            TESTVFS_SHMMAP_आवरणम् },
        { "xSync",              TESTVFS_SYNC_आवरणम् },
        { "xDelete",            TESTVFS_DELETE_आवरणम् },
        { "xWrite",             TESTVFS_WRITE_आवरणम् },
        { "xRead",              TESTVFS_READ_आवरणम् },
        { "xTruncate",          TESTVFS_TRUNCATE_आवरणम् },
        { "xOpen",              TESTVFS_OPEN_आवरणम् },
        { "xClose",             TESTVFS_CLOSE_आवरणम् },
        { "xAccess",            TESTVFS_ACCESS_आवरणम् },
        { "xFullPathname",      TESTVFS_FULLPATHNAME_आवरणम् },
        { "xUnlock",            TESTVFS_UNLOCK_आवरणम् },
        { "xLock",              TESTVFS_तालकम्_आवरणम् },
        { "xCheckReservedLock", TESTVFS_CKLOCK_आवरणम् },
        { "xFileControl",       TESTVFS_FCNTL_आवरणम् },
        { "xSleep",             TESTVFS_SLEEP_आवरणम् },
      };
      Tcl_Obj **apElem = 0;
      Tcl_आकार nElem = 0;
      पूर्णाङ्क आवरणम् = 0;
      यदि( objc!=3 ){
        Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "LIST");
        प्रत्यागम TCL_ERROR;
      }
      यदि( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        प्रत्यागम TCL_ERROR;
      }
      Tcl_Reset_परिणाम(interp);
      प्रति(i=0; i<(पूर्णाङ्क)nElem; i++){
        पूर्णाङ्क i_विधानम्;
        वर्ण *zElem = Tcl_GetString(apElem[i]);
        प्रति(i_विधानम्=0; i_विधानम्<Array_आकार(vfsmethod); i_विधानम्++){
          यदि( वाक्यं_तुलय(zElem, vfsmethod[i_विधानम्].zName)==0 ){
            आवरणम् |= vfsmethod[i_विधानम्].आवरणम्;
            भङ्ग;
          }
        }
        यदि( i_विधानम्==Array_आकार(vfsmethod) ){
          Tcl_Append_परिणाम(interp, "unknown method: ", zElem, रिक्त);
          प्रत्यागम TCL_ERROR;
        }
      }
      p->आवरणम् = आवरणम्;
      भङ्ग;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    पक्ष CMD_SCRIPT: {
      यदि( objc==3 ){
        Tcl_आकार nByte;
        यदि( p->pScript ){
          Tcl_Decr_Ref_गणनम्(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        यदि( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_गणनम्(p->pScript);
        }
      }अन्यथा यदि( objc!=2 ){
        Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?SCRIPT?");
        प्रत्यागम TCL_ERROR;
      }

      Tcl_Reset_परिणाम(interp);
      यदि( p->pScript ) Tcl_समुच्चयः_Obj_परिणाम(interp, p->pScript);

      भङ्ग;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    पक्ष CMD_CANTOPENERR:
    पक्ष CMD_IOERR:
    पक्ष CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      पूर्णाङ्क iRet;

      विकल्प( aSubcmd[i].eCmd ){
        पक्ष CMD_IOERR: pTest = &p->ioerr_err; भङ्ग;
        पक्ष CMD_FULLERR: pTest = &p->पूर्णम्_err; भङ्ग;
        पक्ष CMD_CANTOPENERR: pTest = &p->cantopen_err; भङ्ग;
        स्वभाव: प्रतिज्ञा(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      यदि( objc==4 ){
        पूर्णाङ्क iCnt, iPersist;
        यदि( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          प्रत्यागम TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }अन्यथा यदि( objc!=2 ){
        Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?CNT PERSIST?");
        प्रत्यागम TCL_ERROR;
      }
      Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(iRet));
      भङ्ग;
    }

    पक्ष CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      भङ्ग;
    }

    पक्ष CMD_DEVCHAR: {
      संरचना DeviceFlag {
        वर्ण *zName;
        पूर्णाङ्क i_मानम्;
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
      पूर्णाङ्क iFlag;

      यदि( objc>3 ){
        Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?ATTR-LIST?");
        प्रत्यागम TCL_ERROR;
      }
      यदि( objc==3 ){
        पूर्णाङ्क j;
        पूर्णाङ्क iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_आकार nFlags = 0;

        यदि( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          प्रत्यागम TCL_ERROR;
        }

        प्रति(j=0; j<(पूर्णाङ्क)nFlags; j++){
          पूर्णाङ्क idx = 0;
          यदि( Tcl_Get_सूचकांक_From_Obj_Struct(interp, flags[j], aFlag, 
                परिमाण(aFlag[0]), "flag", 0, &idx) 
          ){
            प्रत्यागम TCL_ERROR;
          }
          यदि( aFlag[idx].i_मानम्<0 && nFlags>1 ){
            Tcl_Append_परिणाम(interp, "bad flags: ", Tcl_GetString(objv[2]), रिक्त);
            प्रत्यागम TCL_ERROR;
          }
          iNew |= aFlag[idx].i_मानम्;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      प्रति(iFlag=0; iFlag<परिमाण(aFlag)/परिमाण(aFlag[0]); iFlag++){
        यदि( p->iDevchar & aFlag[iFlag].i_मानम् ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_समुच्चयः_Obj_परिणाम(interp, pRet);

      भङ्ग;
    }

    पक्ष CMD_SECTORSIZE: {
      यदि( objc>3 ){
        Tcl_Wrong_अङ्कः_तर्काः(interp, 2, objv, "?VALUE?");
        प्रत्यागम TCL_ERROR;
      }
      यदि( objc==3 ){
        पूर्णाङ्क iNew = 0;
        यदि( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          प्रत्यागम TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(p->iSectorsize));
      भङ्ग;
    }
  }

  प्रत्यागम TCL_OK;
}

स्थिर शून्य SQLITE_TCLAPI testvfs_obj_del(Client_दत्त cd){
  Testvfs *p = (Testvfs *)cd;
  यदि( p->pScript ) Tcl_Decr_Ref_गणनम्(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  स्मृतिं_पूरय(p->pVfs, 0, परिमाण(sqlite3_vfs));
  ckfree((वर्ण *)p->pVfs);
  स्मृतिं_पूरय(p, 0, परिमाण(Testvfs));
  ckfree((वर्ण *)p);
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
स्थिर पूर्णाङ्क SQLITE_TCLAPI testvfs_cmd(
  Client_दत्त cd,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर sqlite3_vfs tvfs_vfs = {
    3,                            /* iVersion */
    0,                            /* szOsFile */
    0,                            /* mxPathname */
    0,                            /* pNext */
    0,                            /* zName */
    0,                            /* pAppData */
    tvfsOpen,                     /* xOpen */
    tvfsDelete,                   /* xDelete */
    tvfsAccess,                   /* xAccess */
    tvfs_पूर्णम्_Pathname,             /* xFullPathname */
#यदि_अपरिभाषितम् SQLITE_OMIT_आरोपय_EXTENSION
    tvfsDlOpen,                   /* xDlOpen */
    tvfsDlError,                  /* xDlError */
    tvfsDlSym,                    /* xDlSym */
    tvfsDlClose,                  /* xDlClose */
#अन्यथा
    0,                            /* xDlOpen */
    0,                            /* xDlError */
    0,                            /* xDlSym */
    0,                            /* xDlClose */
#यदि_अन्तः /* SQLITE_OMIT_LOAD_EXTENSION */
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
  वर्ण *zVfs;
  पूर्णाङ्क nByte;                      /* Bytes of space to allocate at p */

  पूर्णाङ्क i;
  पूर्णाङ्क isNoshm = 0;                /* True if -noshm is passed */
  पूर्णाङ्क isFullshm = 0;              /* True if -fullshm is passed */
  पूर्णाङ्क isDefault = 0;              /* True if -default is passed */
  पूर्णाङ्क szOsFile = 0;               /* Value passed to -szosfile */
  पूर्णाङ्क mxPathname = -1;            /* Value passed to -mxpathname */
  पूर्णाङ्क i_संस्करणम् = 3;               /* Value passed to -iversion */

  यदि( objc<2 || 0!=(objc%2) ) गच्छ bad_तर्काः;
  प्रति(i=2; i<objc; i += 2){
    Tcl_आकार nSwitch;
    वर्ण *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    यदि( nSwitch>2 && 0==वाक्ये_तुलय("-noshm", zSwitch, nSwitch) ){
      यदि( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        प्रत्यागम TCL_ERROR;
      }
      यदि( isNoshm ) isFullshm = 0;
    }
    अन्यथा यदि( nSwitch>2 && 0==वाक्ये_तुलय("-default", zSwitch, nSwitch) ){
      यदि( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        प्रत्यागम TCL_ERROR;
      }
    }
    अन्यथा यदि( nSwitch>2 && 0==वाक्ये_तुलय("-szosfile", zSwitch, nSwitch) ){
      यदि( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        प्रत्यागम TCL_ERROR;
      }
    }
    अन्यथा यदि( nSwitch>2 && 0==वाक्ये_तुलय("-mxpathname", zSwitch, nSwitch) ){
      यदि( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        प्रत्यागम TCL_ERROR;
      }
    }
    अन्यथा यदि( nSwitch>2 && 0==वाक्ये_तुलय("-iversion", zSwitch, nSwitch) ){
      यदि( Tcl_GetIntFromObj(interp, objv[i+1], &i_संस्करणम्) ){
        प्रत्यागम TCL_ERROR;
      }
    }
    अन्यथा यदि( nSwitch>2 && 0==वाक्ये_तुलय("-fullshm", zSwitch, nSwitch) ){
      यदि( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        प्रत्यागम TCL_ERROR;
      }
      यदि( isFullshm ) isNoshm = 0;
    }
    अन्यथा{
      गच्छ bad_तर्काः;
    }
  }

  यदि( szOsFile<परिमाण(TestvfsFile) ){
    szOsFile = परिमाण(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = परिमाण(Testvfs) + (पूर्णाङ्क)वाक्यदैर्घ्यम्(zVfs)+1;
  p = (Testvfs *)ckalloc(nByte);
  स्मृतिं_पूरय(p, 0, nByte);
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

  p->zName = (वर्ण *)&p[1];
  स्मृतिं_प्रतिलिख(p->zName, zVfs, वाक्यदैर्घ्यम्(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(परिमाण(sqlite3_vfs));
  स्मृतिं_प्रतिलिख(pVfs, &tvfs_vfs, परिमाण(sqlite3_vfs));
  pVfs->p_App_दत्त = (शून्य *)p;
  pVfs->i_संस्करणम् = i_संस्करणम्;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  यदि( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->आवरणम् = TESTVFS_ALL_आवरणम्;

  sqlite3_vfs_पञ्जिकाकोशः(pVfs, isDefault);

  प्रत्यागम TCL_OK;

 bad_तर्काः:
  Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  प्रत्यागम TCL_ERROR;
}

बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *interp, ध्रुव वर्ण *zA, sqlite3 **ppDb);
बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_vfs_shmlock(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  ध्रुव वर्ण *azArg1[] = {"shared", "exclusive", 0};
  ध्रुव वर्ण *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  पूर्णाङ्क rc = SQLITE_OK;
  ध्रुव वर्ण *zDbname = 0;
  पूर्णाङ्क iArg1 = 0;
  पूर्णाङ्क iArg2 = 0;
  पूर्णाङ्क i_विस्थापनम् = 0;
  पूर्णाङ्क n = 0;
  sqlite3_file *pFd;

  यदि( objc!=7 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    प्रत्यागम TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_Get_सूचकांक_From_Obj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_Get_सूचकांक_From_Obj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &i_विस्थापनम्)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    प्रत्यागम TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (शून्य*)&pFd);
  यदि( pFd==0 ){
    प्रत्यागम TCL_ERROR;
  }
  rc = pFd->pMethods->x_Shm_तालकम्(pFd, i_विस्थापनम्, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_तालकम् : SQLITE_SHM_UNLOCK)
  );
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_vfs_समुच्चयः_readmark(
  शून्य * client_दत्त,
  Tcl_Interp *interp,
  पूर्णाङ्क objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  पूर्णाङ्क rc = SQLITE_OK;
  ध्रुव वर्ण *zDbname = 0;
  पूर्णाङ्क iSlot = 0;
  पूर्णाङ्क iVal = -1;
  sqlite3_file *pFd;
  शून्य चञ्चल *pShm = 0;
  u32 *aShm;
  पूर्णाङ्क iOff;

  यदि( objc!=4 && objc!=5 ){
    Tcl_Wrong_अङ्कः_तर्काः(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    प्रत्यागम TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    प्रत्यागम TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (शून्य*)&pFd);
  यदि( pFd==0 ){
    प्रत्यागम TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  यदि( rc!=SQLITE_OK ){
    Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    प्रत्यागम TCL_ERROR;
  }
  यदि( pShm==0 ){
    Tcl_Append_परिणाम(interp, "*-shm is not yet mapped", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  यदि( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_समुच्चयः_Obj_परिणाम(interp, Tcl_NewIntObj(aShm[iOff]));

  प्रत्यागम TCL_OK;
}

पूर्णाङ्क Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_समुच्चयः_readmark, 0, 0);
  प्रत्यागम TCL_OK;
}

#यदि_अन्तः
