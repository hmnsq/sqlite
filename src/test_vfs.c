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
#जर SQLITE_TEST          /* This file is used for testing only */

#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव "tclsqlite.h"

प्रकारव्याख्या संरचना Testvfs Testvfs;
प्रकारव्याख्या संरचना TestvfsShm TestvfsShm;
प्रकारव्याख्या संरचना TestvfsBuffer TestvfsBuffer;
प्रकारव्याख्या संरचना TestvfsFile TestvfsFile;
प्रकारव्याख्या संरचना TestvfsFd TestvfsFd;

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
  स्थिरांक अक्षर *zFilename;          /* Filename as passed to xOpen() */
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

प्रकारव्याख्या संरचना TestFaultInject TestFaultInject;
संरचना TestFaultInject {
  पूर्णांक iCnt;                       /* Remaining calls before fault injection */
  पूर्णांक eFault;                     /* A FAULT_INJECT_* value */
  पूर्णांक nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
संरचना Testvfs {
  अक्षर *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  TestvfsBuffer *pBuffer;         /* List of shared buffers */
  पूर्णांक isNoshm;
  पूर्णांक isFullshm;

  पूर्णांक mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#जर 0
  पूर्णांक iIoerrCnt;
  पूर्णांक ioerr;
  पूर्णांक nIoerrFail;
  पूर्णांक iFullCnt;
  पूर्णांक fullerr;
  पूर्णांक nFullFail;
#endif

  पूर्णांक iDevchar;
  पूर्णांक iSectorsize;
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
  अक्षर *zFile;                    /* Associated file name */
  पूर्णांक pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  TestvfsBuffer *pNext;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->pAppData))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
स्थिर पूर्णांक tvfsClose(sqlite3_file*);
स्थिर पूर्णांक tvfsRead(sqlite3_file*, शून्य*, पूर्णांक iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णांक tvfsWrite(sqlite3_file*,स्थिरांक शून्य*,पूर्णांक iAmt, sqlite3_int64 iOfst);
स्थिर पूर्णांक tvfsTruncate(sqlite3_file*, sqlite3_int64 size);
स्थिर पूर्णांक tvfsSync(sqlite3_file*, पूर्णांक flags);
स्थिर पूर्णांक tvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
स्थिर पूर्णांक tvfsLock(sqlite3_file*, पूर्णांक);
स्थिर पूर्णांक tvfsUnlock(sqlite3_file*, पूर्णांक);
स्थिर पूर्णांक tvfsCheckReservedLock(sqlite3_file*, पूर्णांक *);
स्थिर पूर्णांक tvfsFileControl(sqlite3_file*, पूर्णांक op, शून्य *pArg);
स्थिर पूर्णांक tvfsSectorSize(sqlite3_file*);
स्थिर पूर्णांक tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
स्थिर पूर्णांक tvfsOpen(sqlite3_vfs*, स्थिरांक अक्षर *, sqlite3_file*, पूर्णांक , पूर्णांक *);
स्थिर पूर्णांक tvfsDelete(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक syncDir);
स्थिर पूर्णांक tvfsAccess(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक flags, पूर्णांक *);
स्थिर पूर्णांक tvfsFullPathname(sqlite3_vfs*, स्थिरांक अक्षर *zName, पूर्णांक, अक्षर *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
स्थिर शून्य *tvfsDlOpen(sqlite3_vfs*, स्थिरांक अक्षर *zFilename);
स्थिर शून्य tvfsDlError(sqlite3_vfs*, पूर्णांक nByte, अक्षर *z_Err_संदेश);
स्थिर शून्य (*tvfsDlSym(sqlite3_vfs*,शून्य*, स्थिरांक अक्षर *z_चिन्ह))(शून्य);
स्थिर शून्य tvfsDlClose(sqlite3_vfs*, शून्य*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
स्थिर पूर्णांक tvfsRandomness(sqlite3_vfs*, पूर्णांक nByte, अक्षर *zOut);
स्थिर पूर्णांक tvfsSleep(sqlite3_vfs*, पूर्णांक microseconds);
स्थिर पूर्णांक tvfsCurrentTime(sqlite3_vfs*, द्विगुण*);

स्थिर पूर्णांक tvfsShmOpen(sqlite3_file*);
स्थिर पूर्णांक tvfsShmLock(sqlite3_file*, पूर्णांक , पूर्णांक, पूर्णांक);
स्थिर पूर्णांक tvfsShmMap(sqlite3_file*,पूर्णांक,पूर्णांक,पूर्णांक, शून्य अस्थिर **);
स्थिर शून्य tvfsShmBarrier(sqlite3_file*);
स्थिर पूर्णांक tvfsShmUnmap(sqlite3_file*, पूर्णांक);

स्थिर पूर्णांक tvfsFetch(sqlite3_file*, sqlite3_int64, पूर्णांक, शून्य**);
स्थिर पूर्णांक tvfsUnfetch(sqlite3_file*, sqlite3_int64, शून्य*);

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

स्थिर पूर्णांक tvfsResultCode(Testvfs *p, पूर्णांक *pRc){
  संरचना errcode {
    पूर्णांक eCode;
    स्थिरांक अक्षर *zCode;
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

  स्थिरांक अक्षर *z;
  पूर्णांक i;

  z = Tcl_GetStringResult(p->interp);
  साठी(i=0; i<ArraySize(aCode); i++){
    जर( 0==म_भेद(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      परत 1;
    }
  }

  परत 0;
}

स्थिर पूर्णांक tvfsInjectFault(TestFaultInject *p){
  पूर्णांक ret = 0;
  जर( p->eFault ){
    p->iCnt--;
    जर( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  परत ret;
}


स्थिर पूर्णांक tvfsInjectIoerr(Testvfs *p){
  परत tvfsInjectFault(&p->ioerr_err);
}

स्थिर पूर्णांक tvfsInjectFullerr(Testvfs *p){
  परत tvfsInjectFault(&p->full_err);
}
स्थिर पूर्णांक tvfsInjectCantopenerr(Testvfs *p){
  परत tvfsInjectFault(&p->cantopen_err);
}


स्थिर शून्य tvfsExecTcl(
  Testvfs *p, 
  स्थिरांक अक्षर *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  पूर्णांक rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  प्रतिज्ञा( p->pScript );

  प्रतिज्ञा( zMethod );
  प्रतिज्ञा( p );
  प्रतिज्ञा( arg2==0 || arg1!=0 );
  प्रतिज्ञा( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_मोजणी(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  जर( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  जर( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  जर( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  जर( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  जर( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_ResetResult(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
स्थिर पूर्णांक tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  जर( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  जर( pFd->pShmId ){
    Tcl_Decr_Ref_मोजणी(pFd->pShmId);
    pFd->pShmId = 0;
  }
  जर( pFile->pMethods ){
    ckfree((अक्षर *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((अक्षर *)pFd);
  pTestfile->pFd = 0;
  परत SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
स्थिर पूर्णांक tvfsRead(
  sqlite3_file *pFile, 
  शून्य *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  जर( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  जर( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  जर( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  परत rc;
}

/*
** Write data to an tvfs-file.
*/
स्थिर पूर्णांक tvfsWrite(
  sqlite3_file *pFile, 
  स्थिरांक शून्य *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  जर( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfsResultCode(p, &rc);
    जर( rc<0 ) परत SQLITE_OK;
  }

  जर( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  जर( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  जर( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  परत rc;
}

/*
** Truncate an tvfs-file.
*/
स्थिर पूर्णांक tvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  जर( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  
  जर( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, size);
  }
  परत rc;
}

/*
** Sync an tvfs-file.
*/
स्थिर पूर्णांक tvfsSync(sqlite3_file *pFile, पूर्णांक flags){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  जर( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    अक्षर *zFlags = 0;

    निवड( flags ){
      स्थिती SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        थांबा;
      स्थिती SQLITE_SYNC_FULL:
        zFlags = "full";
        थांबा;
      स्थिती SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        थांबा;
      स्थिती SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        थांबा;
      पूर्वनिर्धारित:
        प्रतिज्ञा(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  जर( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  जर( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  परत rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
स्थिर पूर्णांक tvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  TestvfsFd *p = tvfsGetFd(pFile);
  परत sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an tvfs-file.
*/
स्थिर पूर्णांक tvfsLock(sqlite3_file *pFile, पूर्णांक eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  जर( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    अक्षर zLock[30];
    sqlite3_snprintf(आकार(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  जर( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    परत SQLITE_IOERR_LOCK;
  }
  परत sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
स्थिर पूर्णांक tvfsUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  जर( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    अक्षर zLock[30];
    sqlite3_snprintf(आकार(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  जर( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    परत SQLITE_IOERR_UNLOCK;
  }
  परत sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
स्थिर पूर्णांक tvfsCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  जर( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  परत sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
स्थिर पूर्णांक tvfsFileControl(sqlite3_file *pFile, पूर्णांक op, शून्य *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  जर( op==SQLITE_FCNTL_PRAGMA ){
    अक्षर **argv = (अक्षर**)pArg;
    जर( sqlite3_stricmp(argv[1],"error")==0 ){
      पूर्णांक rc = SQLITE_ERROR;
      जर( argv[2] ){
        स्थिरांक अक्षर *z = argv[2];
        पूर्णांक x = म_से_प(z);
        जर( x ){
          rc = x;
          पर्यंत( sqlite3Isdigit(z[0]) ){ z++; }
          पर्यंत( sqlite3Isspace(z[0]) ){ z++; }
        }
        जर( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      परत rc;
    }
    जर( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      परत SQLITE_OK;
    }
  }
  जर( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    संरचना Fcntl {
      पूर्णांक iFnctl;
      स्थिरांक अक्षर *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    पूर्णांक i;
    साठी(i=0; i<आकार(aF)/आकार(aF[0]); i++){
      जर( op==aF[i].iFnctl ) थांबा;
    }
    जर( i<आकार(aF)/आकार(aF[0]) ){
      पूर्णांक rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfsResultCode(p, &rc);
      जर( rc ) परत (rc<0 ? SQLITE_OK : rc);
    }
  }
  परत sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
स्थिर पूर्णांक tvfsSectorSize(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  जर( p->iSectorsize>=0 ){
    परत p->iSectorsize;
  }
  परत sqlite3OsSectorSize(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
स्थिर पूर्णांक tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  जर( p->iDevchar>=0 ){
    परत p->iDevchar;
  }
  परत sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
स्थिर पूर्णांक tvfsOpen(
  sqlite3_vfs *pVfs,
  स्थिरांक अक्षर *zName,
  sqlite3_file *pFile,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
  पूर्णांक rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  pFd = (TestvfsFd *)ckalloc(आकार(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  स_रखो(pFd, 0, आकार(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  स_रखो(pTestfile, 0, आकार(TestvfsFile));
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
  जर( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_मोजणी(pArg);
    जर( flags&SQLITE_OPEN_MAIN_DB ){
      स्थिरांक अक्षर *z = &zName[म_माप(zName)+1];
      पर्यंत( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += म_माप(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += म_माप(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_मोजणी(pArg);
    जर( tvfsResultCode(p, &rc) ){
      जर( rc!=SQLITE_OK ) परत rc;
    }अन्यथा{
      pId = Tcl_GetObjResult(p->interp);
    }
  }

  जर( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) परत SQLITE_IOERR;
  जर( tvfsInjectCantopenerr(p) ) परत SQLITE_CANTOPEN;
  जर( tvfsInjectFullerr(p) ) परत SQLITE_FULL;

  जर( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_मोजणी(pId);
  pFd->pShmId = pId;
  Tcl_ResetResult(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  जर( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    पूर्णांक nByte;

    जर( pVfs->iVersion>1 ){
      nByte = आकार(sqlite3_io_methods);
    }अन्यथा{
      nByte = दुरत्व(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    स_नकल(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    जर( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    जर( pVfs->iVersion>1 && ((Testvfs *)pVfs->pAppData)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  परत rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
स्थिर पूर्णांक tvfsDelete(sqlite3_vfs *pVfs, स्थिरांक अक्षर *zPath, पूर्णांक dirSync){
  पूर्णांक rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  जर( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  जर( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  परत rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
स्थिर पूर्णांक tvfsAccess(
  sqlite3_vfs *pVfs, 
  स्थिरांक अक्षर *zPath, 
  पूर्णांक flags, 
  पूर्णांक *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  जर( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    पूर्णांक rc;
    अक्षर *zArg = 0;
    जर( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    जर( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    जर( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    जर( tvfsResultCode(p, &rc) ){
      जर( rc!=SQLITE_OK ) परत rc;
    }अन्यथा{
      Tcl_Interp *interp = p->interp;
      जर( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_GetObjResult(interp), pResOut) ){
        परत SQLITE_OK;
      }
    }
  }
  परत sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
स्थिर पूर्णांक tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  स्थिरांक अक्षर *zPath, 
  पूर्णांक nOut, 
  अक्षर *zOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  जर( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    पूर्णांक rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    जर( tvfsResultCode(p, &rc) ){
      जर( rc!=SQLITE_OK ) परत rc;
    }
  }
  परत sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
स्थिर शून्य *tvfsDlOpen(sqlite3_vfs *pVfs, स्थिरांक अक्षर *zPath){
  परत sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
स्थिर शून्य tvfsDlError(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *z_Err_संदेश){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_संदेश);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
स्थिर शून्य (*tvfsDlSym(sqlite3_vfs *pVfs, शून्य *p, स्थिरांक अक्षर *zSym))(शून्य){
  परत sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
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
स्थिर पूर्णांक tvfsRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  परत sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
स्थिर पूर्णांक tvfsSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  जर( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  परत sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
स्थिर पूर्णांक tvfsCurrentTime(sqlite3_vfs *pVfs, द्विगुण *pTimeOut){
  परत PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

स्थिर पूर्णांक tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  पूर्णांक rc = SQLITE_OK;             /* Return code */
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
  जर( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    जर( tvfsResultCode(p, &rc) ){
      जर( rc!=SQLITE_OK ) परत rc;
    }
  }

  प्रतिज्ञा( rc==SQLITE_OK );
  जर( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    परत SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  साठी(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
    जर( 0==म_भेद(pFd->zFilename, pBuffer->zFile) ) थांबा;
  }
  जर( !pBuffer ){
    पूर्णांक szName = (पूर्णांक)म_माप(pFd->zFilename);
    पूर्णांक nByte = आकार(TestvfsBuffer) + szName + 1;
    pBuffer = (TestvfsBuffer *)ckalloc(nByte);
    स_रखो(pBuffer, 0, nByte);
    pBuffer->zFile = (अक्षर *)&pBuffer[1];
    स_नकल(pBuffer->zFile, pFd->zFilename, szName+1);
    pBuffer->pNext = p->pBuffer;
    p->pBuffer = pBuffer;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->pNext = pBuffer->pFile;
  pBuffer->pFile = pFd;
  pFd->pShm = pBuffer;
  परत rc;
}

स्थिर शून्य tvfsAllocPage(TestvfsBuffer *p, पूर्णांक iPage, पूर्णांक pgsz){
  प्रतिज्ञा( iPage<TESTVFS_MAX_PAGES );
  जर( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    स_रखो(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

स्थिर पूर्णांक tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  पूर्णांक iPage,                      /* Page to retrieve */
  पूर्णांक pgsz,                       /* Size of pages */
  पूर्णांक isWrite,                    /* True to extend file if necessary */
  शून्य अस्थिर **pp              /* OUT: Mapped memory */
){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  जर( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    परत pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  जर( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    जर( rc!=SQLITE_OK ){
      परत rc;
    }
  }

  जर( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
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
  जर( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  जर( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  जर( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (शून्य अस्थिर *)pFd->pShm->aPage[iPage];
  }

  परत rc;
}


स्थिर पूर्णांक tvfsShmLock(
  sqlite3_file *pFile,
  पूर्णांक ofst,
  पूर्णांक n,
  पूर्णांक flags
){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  पूर्णांक nLock;
  अक्षर zLock[80];

  जर( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    परत pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  जर( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(आकार(zLock), zLock, "%d %d", ofst, n);
    nLock = (पूर्णांक)म_माप(zLock);
    जर( flags & SQLITE_SHM_LOCK ){
      म_नकल(&zLock[nLock], " lock");
    }अन्यथा{
      म_नकल(&zLock[nLock], " unlock");
    }
    nLock += (पूर्णांक)म_माप(&zLock[nLock]);
    जर( flags & SQLITE_SHM_SHARED ){
      म_नकल(&zLock[nLock], " shared");
    }अन्यथा{
      म_नकल(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  जर( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  जर( rc==SQLITE_OK ){
    पूर्णांक isLock = (flags & SQLITE_SHM_LOCK);
    पूर्णांक isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    जर( isLock ){
      TestvfsFd *p2;
      साठी(p2=pFd->pShm->pFile; p2; p2=p2->pNext){
        जर( p2==pFd ) सुरूठेवा;
        जर( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          थांबा;
        }
      }
      जर( rc==SQLITE_OK ){
        जर( isExcl )  pFd->excllock |= mask;
        जर( !isExcl ) pFd->sharedlock |= mask;
      }
    }अन्यथा{
      जर( isExcl )  pFd->excllock &= (~mask);
      जर( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  परत rc;
}

स्थिर शून्य tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  जर( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    स्थिरांक अक्षर *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  जर( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    परत;
  }
}

स्थिर पूर्णांक tvfsShmUnmap(
  sqlite3_file *pFile,
  पूर्णांक deleteFlag
){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  TestvfsBuffer *pBuffer = pFd->pShm;
  TestvfsFd **ppFd;

  जर( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    परत pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  जर( !pBuffer ) परत SQLITE_OK;
  प्रतिज्ञा( pFd->pShmId && pFd->pShm );

  जर( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }

  साठी(ppFd=&pBuffer->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->pNext));
  प्रतिज्ञा( (*ppFd)==pFd );
  *ppFd = pFd->pNext;
  pFd->pNext = 0;

  जर( pBuffer->pFile==0 ){
    पूर्णांक i;
    TestvfsBuffer **pp;
    साठी(pp=&p->pBuffer; *pp!=pBuffer; pp=&((*pp)->pNext));
    *pp = (*pp)->pNext;
    साठी(i=0; pBuffer->aPage[i]; i++){
      ckfree((अक्षर *)pBuffer->aPage[i]);
    }
    ckfree((अक्षर *)pBuffer);
  }
  pFd->pShm = 0;

  परत rc;
}

स्थिर पूर्णांक tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    पूर्णांक iAmt, 
    शून्य **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  परत sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

स्थिर पूर्णांक tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, शून्य *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  परत sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

स्थिर पूर्णांक SQLITE_TCLAPI testvfs_obj_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  गणना DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  संरचना TestvfsSubcmd {
    अक्षर *zName;
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
  पूर्णांक i;
  
  जर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUBCOMMAND ...");
    परत TCL_ERROR;
  }
  जर( Tcl_GetIndexFromObjStruct(
        interp, objv[1], aSubcmd, आकार(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    परत TCL_ERROR;
  }
  Tcl_ResetResult(interp);

  निवड( aSubcmd[i].eCmd ){
    स्थिती CMD_SHM: {
      Tcl_Obj *pObj;
      पूर्णांक rc;
      TestvfsBuffer *pBuffer;
      अक्षर *zName;
      जर( objc!=3 && objc!=4 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "FILE ?VALUE?");
        परत TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      जर( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), रिक्त);
        ckfree(zName);
        परत TCL_ERROR;
      }
      साठी(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
        जर( 0==म_भेद(pBuffer->zFile, zName) ) थांबा;
      }
      ckfree(zName);
      जर( !pBuffer ){
        Tcl_AppendResult(interp, "no such file: ", Tcl_GetString(objv[2]), रिक्त);
        परत TCL_ERROR;
      }
      जर( objc==4 ){
        Tcl_Size n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        पूर्णांक pgsz = pBuffer->pgsz;
        जर( pgsz==0 ) pgsz = 65536;
        साठी(i=0; i*pgsz<(पूर्णांक)n; i++){
          पूर्णांक nByte = pgsz;
          tvfsAllocPage(pBuffer, i, pgsz);
          जर( n-i*pgsz<pgsz ){
            nByte = (पूर्णांक)n;
          }
          स_नकल(pBuffer->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      साठी(i=0; pBuffer->aPage[i]; i++){
        पूर्णांक pgsz = pBuffer->pgsz;
        जर( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(pBuffer->aPage[i], pgsz));
      }
      Tcl_SetObjResult(interp, pObj);
      थांबा;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    स्थिती CMD_FILTER: {
      स्थिर संरचना VfsMethod {
        अक्षर *zName;
        पूर्णांक mask;
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
      पूर्णांक mask = 0;
      जर( objc!=3 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "LIST");
        परत TCL_ERROR;
      }
      जर( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        परत TCL_ERROR;
      }
      Tcl_ResetResult(interp);
      साठी(i=0; i<(पूर्णांक)nElem; i++){
        पूर्णांक iMethod;
        अक्षर *zElem = Tcl_GetString(apElem[i]);
        साठी(iMethod=0; iMethod<ArraySize(vfsmethod); iMethod++){
          जर( म_भेद(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            थांबा;
          }
        }
        जर( iMethod==ArraySize(vfsmethod) ){
          Tcl_AppendResult(interp, "unknown method: ", zElem, रिक्त);
          परत TCL_ERROR;
        }
      }
      p->mask = mask;
      थांबा;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    स्थिती CMD_SCRIPT: {
      जर( objc==3 ){
        Tcl_Size nByte;
        जर( p->pScript ){
          Tcl_Decr_Ref_मोजणी(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        जर( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_मोजणी(p->pScript);
        }
      }अन्यथा जर( objc!=2 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "?SCRIPT?");
        परत TCL_ERROR;
      }

      Tcl_ResetResult(interp);
      जर( p->pScript ) Tcl_SetObjResult(interp, p->pScript);

      थांबा;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    स्थिती CMD_CANTOPENERR:
    स्थिती CMD_IOERR:
    स्थिती CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      पूर्णांक iRet;

      निवड( aSubcmd[i].eCmd ){
        स्थिती CMD_IOERR: pTest = &p->ioerr_err; थांबा;
        स्थिती CMD_FULLERR: pTest = &p->full_err; थांबा;
        स्थिती CMD_CANTOPENERR: pTest = &p->cantopen_err; थांबा;
        पूर्वनिर्धारित: प्रतिज्ञा(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      जर( objc==4 ){
        पूर्णांक iCnt, iPersist;
        जर( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          परत TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }अन्यथा जर( objc!=2 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "?CNT PERSIST?");
        परत TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(iRet));
      थांबा;
    }

    स्थिती CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      थांबा;
    }

    स्थिती CMD_DEVCHAR: {
      संरचना DeviceFlag {
        अक्षर *zName;
        पूर्णांक i_मूल्य;
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
      पूर्णांक iFlag;

      जर( objc>3 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "?ATTR-LIST?");
        परत TCL_ERROR;
      }
      जर( objc==3 ){
        पूर्णांक j;
        पूर्णांक iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_Size nFlags = 0;

        जर( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          परत TCL_ERROR;
        }

        साठी(j=0; j<(पूर्णांक)nFlags; j++){
          पूर्णांक idx = 0;
          जर( Tcl_GetIndexFromObjStruct(interp, flags[j], aFlag, 
                आकार(aFlag[0]), "flag", 0, &idx) 
          ){
            परत TCL_ERROR;
          }
          जर( aFlag[idx].i_मूल्य<0 && nFlags>1 ){
            Tcl_AppendResult(interp, "bad flags: ", Tcl_GetString(objv[2]), रिक्त);
            परत TCL_ERROR;
          }
          iNew |= aFlag[idx].i_मूल्य;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      साठी(iFlag=0; iFlag<आकार(aFlag)/आकार(aFlag[0]); iFlag++){
        जर( p->iDevchar & aFlag[iFlag].i_मूल्य ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_SetObjResult(interp, pRet);

      थांबा;
    }

    स्थिती CMD_SECTORSIZE: {
      जर( objc>3 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "?VALUE?");
        परत TCL_ERROR;
      }
      जर( objc==3 ){
        पूर्णांक iNew = 0;
        जर( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          परत TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(p->iSectorsize));
      थांबा;
    }
  }

  परत TCL_OK;
}

स्थिर शून्य SQLITE_TCLAPI testvfs_obj_del(ClientData cd){
  Testvfs *p = (Testvfs *)cd;
  जर( p->pScript ) Tcl_Decr_Ref_मोजणी(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  स_रखो(p->pVfs, 0, आकार(sqlite3_vfs));
  ckfree((अक्षर *)p->pVfs);
  स_रखो(p, 0, आकार(Testvfs));
  ckfree((अक्षर *)p);
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
स्थिर पूर्णांक SQLITE_TCLAPI testvfs_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  पूर्णांक objc,
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
  अक्षर *zVfs;
  पूर्णांक nByte;                      /* Bytes of space to allocate at p */

  पूर्णांक i;
  पूर्णांक isNoshm = 0;                /* True if -noshm is passed */
  पूर्णांक isFullshm = 0;              /* True if -fullshm is passed */
  पूर्णांक isDefault = 0;              /* True if -default is passed */
  पूर्णांक szOsFile = 0;               /* Value passed to -szosfile */
  पूर्णांक mxPathname = -1;            /* Value passed to -mxpathname */
  पूर्णांक iVersion = 3;               /* Value passed to -iversion */

  जर( objc<2 || 0!=(objc%2) ) जा bad_args;
  साठी(i=2; i<objc; i += 2){
    Tcl_Size nSwitch;
    अक्षर *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    जर( nSwitch>2 && 0==म_भेदन("-noshm", zSwitch, nSwitch) ){
      जर( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        परत TCL_ERROR;
      }
      जर( isNoshm ) isFullshm = 0;
    }
    अन्यथा जर( nSwitch>2 && 0==म_भेदन("-default", zSwitch, nSwitch) ){
      जर( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        परत TCL_ERROR;
      }
    }
    अन्यथा जर( nSwitch>2 && 0==म_भेदन("-szosfile", zSwitch, nSwitch) ){
      जर( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        परत TCL_ERROR;
      }
    }
    अन्यथा जर( nSwitch>2 && 0==म_भेदन("-mxpathname", zSwitch, nSwitch) ){
      जर( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        परत TCL_ERROR;
      }
    }
    अन्यथा जर( nSwitch>2 && 0==म_भेदन("-iversion", zSwitch, nSwitch) ){
      जर( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        परत TCL_ERROR;
      }
    }
    अन्यथा जर( nSwitch>2 && 0==म_भेदन("-fullshm", zSwitch, nSwitch) ){
      जर( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        परत TCL_ERROR;
      }
      जर( isFullshm ) isNoshm = 0;
    }
    अन्यथा{
      जा bad_args;
    }
  }

  जर( szOsFile<आकार(TestvfsFile) ){
    szOsFile = आकार(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = आकार(Testvfs) + (पूर्णांक)म_माप(zVfs)+1;
  p = (Testvfs *)ckalloc(nByte);
  स_रखो(p, 0, nByte);
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

  p->zName = (अक्षर *)&p[1];
  स_नकल(p->zName, zVfs, म_माप(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(आकार(sqlite3_vfs));
  स_नकल(pVfs, &tvfs_vfs, आकार(sqlite3_vfs));
  pVfs->pAppData = (शून्य *)p;
  pVfs->iVersion = iVersion;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  जर( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  परत TCL_OK;

 bad_args:
  Tcl_Wrong_अंक_Args(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  परत TCL_ERROR;
}

बाह्य पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिरांक अक्षर *zA, sqlite3 **ppDb);
बाह्य स्थिरांक अक्षर *sqlite3ErrName(पूर्णांक);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
स्थिर पूर्णांक SQLITE_TCLAPI test_vfs_shmlock(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिरांक अक्षर *azArg1[] = {"shared", "exclusive", 0};
  स्थिरांक अक्षर *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  पूर्णांक rc = SQLITE_OK;
  स्थिरांक अक्षर *zDbname = 0;
  पूर्णांक iArg1 = 0;
  पूर्णांक iArg2 = 0;
  पूर्णांक iOffset = 0;
  पूर्णांक n = 0;
  sqlite3_file *pFd;

  जर( objc!=7 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    परत TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  जर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIndexFromObj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_GetIndexFromObj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    परत TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (शून्य*)&pFd);
  जर( pFd==0 ){
    परत TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  परत TCL_OK;
}

स्थिर पूर्णांक SQLITE_TCLAPI test_vfs_set_readmark(
  शून्य * clientData,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  पूर्णांक rc = SQLITE_OK;
  स्थिरांक अक्षर *zDbname = 0;
  पूर्णांक iSlot = 0;
  पूर्णांक iVal = -1;
  sqlite3_file *pFd;
  शून्य अस्थिर *pShm = 0;
  u32 *aShm;
  पूर्णांक iOff;

  जर( objc!=4 && objc!=5 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    परत TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  जर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    परत TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (शून्य*)&pFd);
  जर( pFd==0 ){
    परत TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  जर( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    परत TCL_ERROR;
  }
  जर( pShm==0 ){
    Tcl_AppendResult(interp, "*-shm is not yet mapped", रिक्त);
    परत TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  जर( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(aShm[iOff]));

  परत TCL_OK;
}

पूर्णांक Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  परत TCL_OK;
}

#endif
