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
प्रकारनिर्देश संरचना TestvfsBuffer TestvfsBuffer;
प्रकारनिर्देश संरचना TestvfsFile TestvfsFile;
प्रकारनिर्देश संरचना TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
संरचना TestvfsFile {
  sqlite3_file base;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#define tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

संरचना TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  ध्रुव वर्ण *zFilename;          /* Filename as passed to xOpen() */
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
  TestvfsBuffer *pBuffer;         /* List of shared buffers */
  पूर्णाङ्क isNoshm;
  पूर्णाङ्क isFullshm;

  पूर्णाङ्क mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#यदि 0
  पूर्णाङ्क iIoerrCnt;
  पूर्णाङ्क ioerr;
  पूर्णाङ्क nIoerrFail;
  पूर्णाङ्क iFullCnt;
  पूर्णाङ्क fullerr;
  पूर्णाङ्क nFullFail;
#endif

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
संरचना TestvfsBuffer {
  वर्ण *zFile;                    /* Associated file name */
  पूर्णाङ्क pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  TestvfsBuffer *pNext;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->pAppData))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
स्थिर पूर्णाङ्क tvfsClose(sqlite3_file*);
स्थिर पूर्णाङ्क tvfsRead(sqlite3_file*, शून्य*, पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क tvfsWrite(sqlite3_file*,ध्रुव शून्य*,पूर्णाङ्क iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णाङ्क tvfsTruncate(sqlite3_file*, sqlite3_int64 size);
स्थिर पूर्णाङ्क tvfsSync(sqlite3_file*, पूर्णाङ्क flags);
स्थिर पूर्णाङ्क tvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
स्थिर पूर्णाङ्क tvfsLock(sqlite3_file*, पूर्णाङ्क);
स्थिर पूर्णाङ्क tvfsUnlock(sqlite3_file*, पूर्णाङ्क);
स्थिर पूर्णाङ्क tvfsCheckReservedLock(sqlite3_file*, पूर्णाङ्क *);
स्थिर पूर्णाङ्क tvfsFileControl(sqlite3_file*, पूर्णाङ्क op, शून्य *pArg);
स्थिर पूर्णाङ्क tvfsSectorSize(sqlite3_file*);
स्थिर पूर्णाङ्क tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
स्थिर पूर्णाङ्क tvfsOpen(sqlite3_vfs*, ध्रुव वर्ण *, sqlite3_file*, पूर्णाङ्क , पूर्णाङ्क *);
स्थिर पूर्णाङ्क tvfsDelete(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क syncDir);
स्थिर पूर्णाङ्क tvfsAccess(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क flags, पूर्णाङ्क *);
स्थिर पूर्णाङ्क tvfsFullPathname(sqlite3_vfs*, ध्रुव वर्ण *zName, पूर्णाङ्क, वर्ण *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
स्थिर शून्य *tvfsDlOpen(sqlite3_vfs*, ध्रुव वर्ण *zFilename);
स्थिर शून्य tvfsDlError(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *z_Err_संदेश);
स्थिर शून्य (*tvfsDlSym(sqlite3_vfs*,शून्य*, ध्रुव वर्ण *z_चिन्ह))(शून्य);
स्थिर शून्य tvfsDlClose(sqlite3_vfs*, शून्य*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
स्थिर पूर्णाङ्क tvfsRandomness(sqlite3_vfs*, पूर्णाङ्क nByte, वर्ण *zOut);
स्थिर पूर्णाङ्क tvfsSleep(sqlite3_vfs*, पूर्णाङ्क microseconds);
स्थिर पूर्णाङ्क tvfsCurrentTime(sqlite3_vfs*, द्विगुण*);

स्थिर पूर्णाङ्क tvfsShmOpen(sqlite3_file*);
स्थिर पूर्णाङ्क tvfsShmLock(sqlite3_file*, पूर्णाङ्क , पूर्णाङ्क, पूर्णाङ्क);
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

स्थिर पूर्णाङ्क tvfsResultCode(Testvfs *p, पूर्णाङ्क *pRc){
  संरचना errcode {
    पूर्णाङ्क eCode;
    ध्रुव वर्ण *zCode;
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

  ध्रुव वर्ण *z;
  पूर्णाङ्क i;

  z = Tcl_GetStringResult(p->interp);
  प्रति(i=0; i<ArraySize(aCode); i++){
    यदि( 0==strcmp(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
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
  प्रत्यागम tvfsInjectFault(&p->full_err);
}
स्थिर पूर्णाङ्क tvfsInjectCantopenerr(Testvfs *p){
  प्रत्यागम tvfsInjectFault(&p->cantopen_err);
}


स्थिर शून्य tvfsExecTcl(
  Testvfs *p, 
  ध्रुव वर्ण *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  पूर्णाङ्क rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  प्रतिज्ञा( p->pScript );

  प्रतिज्ञा( zMethod );
  प्रतिज्ञा( p );
  प्रतिज्ञा( arg2==0 || arg1!=0 );
  प्रतिज्ञा( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_मोजणी(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  यदि( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  यदि( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  यदि( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  यदि( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  यदि( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_ResetResult(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  यदि( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  यदि( pFd->pShmId ){
    Tcl_Decr_Ref_मोजणी(pFd->pShmId);
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
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  यदि( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  यदि( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
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
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  यदि( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfsResultCode(p, &rc);
    यदि( rc<0 ) प्रत्यागम SQLITE_OK;
  }

  यदि( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  यदि( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
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
स्थिर पूर्णाङ्क tvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  पूर्णाङ्क rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  यदि( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  
  यदि( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, size);
  }
  प्रत्यागम rc;
}

/*
** Sync an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsSync(sqlite3_file *pFile, पूर्णाङ्क flags){
  पूर्णाङ्क rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  यदि( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    वर्ण *zFlags = 0;

    विकल्प( flags ){
      पक्ष SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        भङ्ग;
      पक्ष SQLITE_SYNC_FULL:
        zFlags = "full";
        भङ्ग;
      पक्ष SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        भङ्ग;
      पक्ष SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        भङ्ग;
      स्वभाव:
        प्रतिज्ञा(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  यदि( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  यदि( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  प्रत्यागम rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  TestvfsFd *p = tvfsGetFd(pFile);
  प्रत्यागम sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsLock(sqlite3_file *pFile, पूर्णाङ्क eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  यदि( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    वर्ण zLock[30];
    sqlite3_snprintf(परिमाण(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  यदि( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    प्रत्यागम SQLITE_IOERR_LOCK;
  }
  प्रत्यागम sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsUnlock(sqlite3_file *pFile, पूर्णाङ्क eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  यदि( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    वर्ण zLock[30];
    sqlite3_snprintf(परिमाण(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  यदि( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    प्रत्यागम SQLITE_IOERR_UNLOCK;
  }
  प्रत्यागम sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsCheckReservedLock(sqlite3_file *pFile, पूर्णाङ्क *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  यदि( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  प्रत्यागम sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsFileControl(sqlite3_file *pFile, पूर्णाङ्क op, शून्य *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
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
  यदि( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
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
      tvfsResultCode(p, &rc);
      यदि( rc ) प्रत्यागम (rc<0 ? SQLITE_OK : rc);
    }
  }
  प्रत्यागम sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsSectorSize(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  यदि( p->iSectorsize>=0 ){
    प्रत्यागम p->iSectorsize;
  }
  प्रत्यागम sqlite3OsSectorSize(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
स्थिर पूर्णाङ्क tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
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
  पूर्णाङ्क *pOutFlags
){
  पूर्णाङ्क rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  pFd = (TestvfsFd *)ckalloc(परिमाण(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, परिमाण(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, परिमाण(TestvfsFile));
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
  यदि( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_मोजणी(pArg);
    यदि( flags&SQLITE_OPEN_MAIN_DB ){
      ध्रुव वर्ण *z = &zName[strlen(zName)+1];
      यावत्( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_मोजणी(pArg);
    यदि( tvfsResultCode(p, &rc) ){
      यदि( rc!=SQLITE_OK ) प्रत्यागम rc;
    }अन्यथा{
      pId = Tcl_GetObjResult(p->interp);
    }
  }

  यदि( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) प्रत्यागम SQLITE_IOERR;
  यदि( tvfsInjectCantopenerr(p) ) प्रत्यागम SQLITE_CANTOPEN;
  यदि( tvfsInjectFullerr(p) ) प्रत्यागम SQLITE_FULL;

  यदि( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_मोजणी(pId);
  pFd->pShmId = pId;
  Tcl_ResetResult(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  यदि( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    पूर्णाङ्क nByte;

    यदि( pVfs->iVersion>1 ){
      nByte = परिमाण(sqlite3_io_methods);
    }अन्यथा{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    memcpy(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    यदि( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    यदि( pVfs->iVersion>1 && ((Testvfs *)pVfs->pAppData)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
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
स्थिर पूर्णाङ्क tvfsDelete(sqlite3_vfs *pVfs, ध्रुव वर्ण *zPath, पूर्णाङ्क dirSync){
  पूर्णाङ्क rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  यदि( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  यदि( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  प्रत्यागम rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
स्थिर पूर्णाङ्क tvfsAccess(
  sqlite3_vfs *pVfs, 
  ध्रुव वर्ण *zPath, 
  पूर्णाङ्क flags, 
  पूर्णाङ्क *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  यदि( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    पूर्णाङ्क rc;
    वर्ण *zArg = 0;
    यदि( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    यदि( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    यदि( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    यदि( tvfsResultCode(p, &rc) ){
      यदि( rc!=SQLITE_OK ) प्रत्यागम rc;
    }अन्यथा{
      Tcl_Interp *interp = p->interp;
      यदि( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_GetObjResult(interp), pResOut) ){
        प्रत्यागम SQLITE_OK;
      }
    }
  }
  प्रत्यागम sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
स्थिर पूर्णाङ्क tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  ध्रुव वर्ण *zPath, 
  पूर्णाङ्क nOut, 
  वर्ण *zOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  यदि( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    पूर्णाङ्क rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    यदि( tvfsResultCode(p, &rc) ){
      यदि( rc!=SQLITE_OK ) प्रत्यागम rc;
    }
  }
  प्रत्यागम sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
स्थिर शून्य *tvfsDlOpen(sqlite3_vfs *pVfs, ध्रुव वर्ण *zPath){
  प्रत्यागम sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
स्थिर शून्य tvfsDlError(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *z_Err_संदेश){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_संदेश);
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
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
स्थिर पूर्णाङ्क tvfsRandomness(sqlite3_vfs *pVfs, पूर्णाङ्क nByte, वर्ण *zBufOut){
  प्रत्यागम sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
स्थिर पूर्णाङ्क tvfsSleep(sqlite3_vfs *pVfs, पूर्णाङ्क nMicro){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  यदि( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  प्रत्यागम sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
स्थिर पूर्णाङ्क tvfsCurrentTime(sqlite3_vfs *pVfs, द्विगुण *pTimeOut){
  प्रत्यागम PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

स्थिर पूर्णाङ्क tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  पूर्णाङ्क rc = SQLITE_OK;             /* Return code */
  TestvfsBuffer *pBuffer;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->pAppData;
  प्रतिज्ञा( 0==p->isFullshm );
  प्रतिज्ञा( pFd->pShmId && pFd->pShm==0 && pFd->pNext==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_ResetResult(p->interp);
  यदि( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    यदि( tvfsResultCode(p, &rc) ){
      यदि( rc!=SQLITE_OK ) प्रत्यागम rc;
    }
  }

  प्रतिज्ञा( rc==SQLITE_OK );
  यदि( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    प्रत्यागम SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  प्रति(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
    यदि( 0==strcmp(pFd->zFilename, pBuffer->zFile) ) भङ्ग;
  }
  यदि( !pBuffer ){
    पूर्णाङ्क szName = (पूर्णाङ्क)strlen(pFd->zFilename);
    पूर्णाङ्क nByte = परिमाण(TestvfsBuffer) + szName + 1;
    pBuffer = (TestvfsBuffer *)ckalloc(nByte);
    memset(pBuffer, 0, nByte);
    pBuffer->zFile = (वर्ण *)&pBuffer[1];
    memcpy(pBuffer->zFile, pFd->zFilename, szName+1);
    pBuffer->pNext = p->pBuffer;
    p->pBuffer = pBuffer;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->pNext = pBuffer->pFile;
  pBuffer->pFile = pFd;
  pFd->pShm = pBuffer;
  प्रत्यागम rc;
}

स्थिर शून्य tvfsAllocPage(TestvfsBuffer *p, पूर्णाङ्क iPage, पूर्णाङ्क pgsz){
  प्रतिज्ञा( iPage<TESTVFS_MAX_PAGES );
  यदि( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    memset(p->aPage[iPage], 0, pgsz);
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
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

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

  यदि( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_मोजणी(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(iPage));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfsResultCode(p, &rc);
    Tcl_Decr_Ref_मोजणी(pArg);
  }
  यदि( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
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


स्थिर पूर्णाङ्क tvfsShmLock(
  sqlite3_file *pFile,
  पूर्णाङ्क ofst,
  पूर्णाङ्क n,
  पूर्णाङ्क flags
){
  पूर्णाङ्क rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  पूर्णाङ्क nLock;
  वर्ण zLock[80];

  यदि( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    प्रत्यागम pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  यदि( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(परिमाण(zLock), zLock, "%d %d", ofst, n);
    nLock = (पूर्णाङ्क)strlen(zLock);
    यदि( flags & SQLITE_SHM_LOCK ){
      strcpy(&zLock[nLock], " lock");
    }अन्यथा{
      strcpy(&zLock[nLock], " unlock");
    }
    nLock += (पूर्णाङ्क)strlen(&zLock[nLock]);
    यदि( flags & SQLITE_SHM_SHARED ){
      strcpy(&zLock[nLock], " shared");
    }अन्यथा{
      strcpy(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  यदि( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  यदि( rc==SQLITE_OK ){
    पूर्णाङ्क isLock = (flags & SQLITE_SHM_LOCK);
    पूर्णाङ्क isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    यदि( isLock ){
      TestvfsFd *p2;
      प्रति(p2=pFd->pShm->pFile; p2; p2=p2->pNext){
        यदि( p2==pFd ) अनुवर्त;
        यदि( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          भङ्ग;
        }
      }
      यदि( rc==SQLITE_OK ){
        यदि( isExcl )  pFd->excllock |= mask;
        यदि( !isExcl ) pFd->sharedlock |= mask;
      }
    }अन्यथा{
      यदि( isExcl )  pFd->excllock &= (~mask);
      यदि( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  प्रत्यागम rc;
}

स्थिर शून्य tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  यदि( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
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
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  TestvfsBuffer *pBuffer = pFd->pShm;
  TestvfsFd **ppFd;

  यदि( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    प्रत्यागम pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  यदि( !pBuffer ) प्रत्यागम SQLITE_OK;
  प्रतिज्ञा( pFd->pShmId && pFd->pShm );

  यदि( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }

  प्रति(ppFd=&pBuffer->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->pNext));
  प्रतिज्ञा( (*ppFd)==pFd );
  *ppFd = pFd->pNext;
  pFd->pNext = 0;

  यदि( pBuffer->pFile==0 ){
    पूर्णाङ्क i;
    TestvfsBuffer **pp;
    प्रति(pp=&p->pBuffer; *pp!=pBuffer; pp=&((*pp)->pNext));
    *pp = (*pp)->pNext;
    प्रति(i=0; pBuffer->aPage[i]; i++){
      ckfree((वर्ण *)pBuffer->aPage[i]);
    }
    ckfree((वर्ण *)pBuffer);
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
  ClientData cd,
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
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUBCOMMAND ...");
    प्रत्यागम TCL_ERROR;
  }
  यदि( Tcl_GetIndexFromObjStruct(
        interp, objv[1], aSubcmd, परिमाण(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    प्रत्यागम TCL_ERROR;
  }
  Tcl_ResetResult(interp);

  विकल्प( aSubcmd[i].eCmd ){
    पक्ष CMD_SHM: {
      Tcl_Obj *pObj;
      पूर्णाङ्क rc;
      TestvfsBuffer *pBuffer;
      वर्ण *zName;
      यदि( objc!=3 && objc!=4 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "FILE ?VALUE?");
        प्रत्यागम TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      यदि( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), रिक्त);
        ckfree(zName);
        प्रत्यागम TCL_ERROR;
      }
      प्रति(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
        यदि( 0==strcmp(pBuffer->zFile, zName) ) भङ्ग;
      }
      ckfree(zName);
      यदि( !pBuffer ){
        Tcl_AppendResult(interp, "no such file: ", Tcl_GetString(objv[2]), रिक्त);
        प्रत्यागम TCL_ERROR;
      }
      यदि( objc==4 ){
        Tcl_Size n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        पूर्णाङ्क pgsz = pBuffer->pgsz;
        यदि( pgsz==0 ) pgsz = 65536;
        प्रति(i=0; i*pgsz<(पूर्णाङ्क)n; i++){
          पूर्णाङ्क nByte = pgsz;
          tvfsAllocPage(pBuffer, i, pgsz);
          यदि( n-i*pgsz<pgsz ){
            nByte = (पूर्णाङ्क)n;
          }
          memcpy(pBuffer->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      प्रति(i=0; pBuffer->aPage[i]; i++){
        पूर्णाङ्क pgsz = pBuffer->pgsz;
        यदि( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(pBuffer->aPage[i], pgsz));
      }
      Tcl_SetObjResult(interp, pObj);
      भङ्ग;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    पक्ष CMD_FILTER: {
      स्थिर संरचना VfsMethod {
        वर्ण *zName;
        पूर्णाङ्क mask;
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
      पूर्णाङ्क mask = 0;
      यदि( objc!=3 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "LIST");
        प्रत्यागम TCL_ERROR;
      }
      यदि( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        प्रत्यागम TCL_ERROR;
      }
      Tcl_ResetResult(interp);
      प्रति(i=0; i<(पूर्णाङ्क)nElem; i++){
        पूर्णाङ्क iMethod;
        वर्ण *zElem = Tcl_GetString(apElem[i]);
        प्रति(iMethod=0; iMethod<ArraySize(vfsmethod); iMethod++){
          यदि( strcmp(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            भङ्ग;
          }
        }
        यदि( iMethod==ArraySize(vfsmethod) ){
          Tcl_AppendResult(interp, "unknown method: ", zElem, रिक्त);
          प्रत्यागम TCL_ERROR;
        }
      }
      p->mask = mask;
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
        Tcl_Size nByte;
        यदि( p->pScript ){
          Tcl_Decr_Ref_मोजणी(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        यदि( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_मोजणी(p->pScript);
        }
      }अन्यथा यदि( objc!=2 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "?SCRIPT?");
        प्रत्यागम TCL_ERROR;
      }

      Tcl_ResetResult(interp);
      यदि( p->pScript ) Tcl_SetObjResult(interp, p->pScript);

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
        पक्ष CMD_FULLERR: pTest = &p->full_err; भङ्ग;
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
        Tcl_Wrong_अंक_Args(interp, 2, objv, "?CNT PERSIST?");
        प्रत्यागम TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(iRet));
      भङ्ग;
    }

    पक्ष CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      भङ्ग;
    }

    पक्ष CMD_DEVCHAR: {
      संरचना DeviceFlag {
        वर्ण *zName;
        पूर्णाङ्क i_मूल्य;
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
        Tcl_Wrong_अंक_Args(interp, 2, objv, "?ATTR-LIST?");
        प्रत्यागम TCL_ERROR;
      }
      यदि( objc==3 ){
        पूर्णाङ्क j;
        पूर्णाङ्क iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_Size nFlags = 0;

        यदि( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          प्रत्यागम TCL_ERROR;
        }

        प्रति(j=0; j<(पूर्णाङ्क)nFlags; j++){
          पूर्णाङ्क idx = 0;
          यदि( Tcl_GetIndexFromObjStruct(interp, flags[j], aFlag, 
                परिमाण(aFlag[0]), "flag", 0, &idx) 
          ){
            प्रत्यागम TCL_ERROR;
          }
          यदि( aFlag[idx].i_मूल्य<0 && nFlags>1 ){
            Tcl_AppendResult(interp, "bad flags: ", Tcl_GetString(objv[2]), रिक्त);
            प्रत्यागम TCL_ERROR;
          }
          iNew |= aFlag[idx].i_मूल्य;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      प्रति(iFlag=0; iFlag<परिमाण(aFlag)/परिमाण(aFlag[0]); iFlag++){
        यदि( p->iDevchar & aFlag[iFlag].i_मूल्य ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_SetObjResult(interp, pRet);

      भङ्ग;
    }

    पक्ष CMD_SECTORSIZE: {
      यदि( objc>3 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "?VALUE?");
        प्रत्यागम TCL_ERROR;
      }
      यदि( objc==3 ){
        पूर्णाङ्क iNew = 0;
        यदि( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          प्रत्यागम TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(p->iSectorsize));
      भङ्ग;
    }
  }

  प्रत्यागम TCL_OK;
}

स्थिर शून्य SQLITE_TCLAPI testvfs_obj_del(ClientData cd){
  Testvfs *p = (Testvfs *)cd;
  यदि( p->pScript ) Tcl_Decr_Ref_मोजणी(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, परिमाण(sqlite3_vfs));
  ckfree((वर्ण *)p->pVfs);
  memset(p, 0, परिमाण(Testvfs));
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
  ClientData cd,
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
    tvfsFullPathname,             /* xFullPathname */
#ifndef SQLITE_OMIT_LOAD_EXTENSION
    tvfsDlOpen,                   /* xDlOpen */
    tvfsDlError,                  /* xDlError */
    tvfsDlSym,                    /* xDlSym */
    tvfsDlClose,                  /* xDlClose */
#अन्यथा
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
  वर्ण *zVfs;
  पूर्णाङ्क nByte;                      /* Bytes of space to allocate at p */

  पूर्णाङ्क i;
  पूर्णाङ्क isNoshm = 0;                /* True if -noshm is passed */
  पूर्णाङ्क isFullshm = 0;              /* True if -fullshm is passed */
  पूर्णाङ्क isDefault = 0;              /* True if -default is passed */
  पूर्णाङ्क szOsFile = 0;               /* Value passed to -szosfile */
  पूर्णाङ्क mxPathname = -1;            /* Value passed to -mxpathname */
  पूर्णाङ्क iVersion = 3;               /* Value passed to -iversion */

  यदि( objc<2 || 0!=(objc%2) ) गच्छ bad_args;
  प्रति(i=2; i<objc; i += 2){
    Tcl_Size nSwitch;
    वर्ण *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    यदि( nSwitch>2 && 0==strncmp("-noshm", zSwitch, nSwitch) ){
      यदि( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        प्रत्यागम TCL_ERROR;
      }
      यदि( isNoshm ) isFullshm = 0;
    }
    अन्यथा यदि( nSwitch>2 && 0==strncmp("-default", zSwitch, nSwitch) ){
      यदि( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        प्रत्यागम TCL_ERROR;
      }
    }
    अन्यथा यदि( nSwitch>2 && 0==strncmp("-szosfile", zSwitch, nSwitch) ){
      यदि( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        प्रत्यागम TCL_ERROR;
      }
    }
    अन्यथा यदि( nSwitch>2 && 0==strncmp("-mxpathname", zSwitch, nSwitch) ){
      यदि( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        प्रत्यागम TCL_ERROR;
      }
    }
    अन्यथा यदि( nSwitch>2 && 0==strncmp("-iversion", zSwitch, nSwitch) ){
      यदि( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        प्रत्यागम TCL_ERROR;
      }
    }
    अन्यथा यदि( nSwitch>2 && 0==strncmp("-fullshm", zSwitch, nSwitch) ){
      यदि( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        प्रत्यागम TCL_ERROR;
      }
      यदि( isFullshm ) isNoshm = 0;
    }
    अन्यथा{
      गच्छ bad_args;
    }
  }

  यदि( szOsFile<परिमाण(TestvfsFile) ){
    szOsFile = परिमाण(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = परिमाण(Testvfs) + (पूर्णाङ्क)strlen(zVfs)+1;
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

  p->zName = (वर्ण *)&p[1];
  memcpy(p->zName, zVfs, strlen(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(परिमाण(sqlite3_vfs));
  memcpy(pVfs, &tvfs_vfs, परिमाण(sqlite3_vfs));
  pVfs->pAppData = (शून्य *)p;
  pVfs->iVersion = iVersion;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  यदि( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  प्रत्यागम TCL_OK;

 bad_args:
  Tcl_Wrong_अंक_Args(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  प्रत्यागम TCL_ERROR;
}

बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *interp, ध्रुव वर्ण *zA, sqlite3 **ppDb);
बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI test_vfs_shmlock(
  शून्य * clientData,
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
  पूर्णाङ्क iOffset = 0;
  पूर्णाङ्क n = 0;
  sqlite3_file *pFd;

  यदि( objc!=7 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    प्रत्यागम TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  यदि( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIndexFromObj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_GetIndexFromObj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    प्रत्यागम TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (शून्य*)&pFd);
  यदि( pFd==0 ){
    प्रत्यागम TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  प्रत्यागम TCL_OK;
}

स्थिर पूर्णाङ्क SQLITE_TCLAPI test_vfs_set_readmark(
  शून्य * clientData,
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
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
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
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    प्रत्यागम TCL_ERROR;
  }
  यदि( pShm==0 ){
    Tcl_AppendResult(interp, "*-shm is not yet mapped", रिक्त);
    प्रत्यागम TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  यदि( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(aShm[iOff]));

  प्रत्यागम TCL_OK;
}

पूर्णाङ्क Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  प्रत्यागम TCL_OK;
}

#endif
