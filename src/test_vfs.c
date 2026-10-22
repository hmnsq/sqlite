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
#अगर SQLITE_TEST          /* This file is used for testing only */

#समावेश "sqlite3.h"
#समावेश "sqliteInt.h"
#समावेश "tclsqlite.h"

प्रकार काष्ठा Testvfs Testvfs;
प्रकार काष्ठा TestvfsShm TestvfsShm;
प्रकार काष्ठा Testvfs_बफर Testvfs_बफर;
प्रकार काष्ठा TestvfsFile TestvfsFile;
प्रकार काष्ठा TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
काष्ठा TestvfsFile {
  sqlite3_file base;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#घोषणा tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

काष्ठा TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  स्थिर अक्षर *zFilename;          /* Filename as passed to xOpen() */
  sqlite3_file *pReal;            /* The real, underlying file descriptor */
  Tcl_Obj *pShmId;                /* Shared memory id for Tcl callbacks */

  Testvfs_बफर *pShm;            /* Shared memory buffer */
  u32 excllock;                   /* Mask of exclusive locks */
  u32 sharedlock;                 /* Mask of shared locks */
  TestvfsFd *p_अग्र;               /* Next handle opened on the same file */
};


#घोषणा FAULT_INJECT_NONE       0
#घोषणा FAULT_INJECT_TRANSIENT  1
#घोषणा FAULT_INJECT_PERSISTENT 2

प्रकार काष्ठा TestFaultInject TestFaultInject;
काष्ठा TestFaultInject {
  पूर्णांक iCnt;                       /* Remaining calls before fault injection */
  पूर्णांक eFault;                     /* A FAULT_INJECT_* value */
  पूर्णांक nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
काष्ठा Testvfs {
  अक्षर *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  Testvfs_बफर *p_बफर;         /* List of shared buffers */
  पूर्णांक isNoshm;
  पूर्णांक isFullshm;

  पूर्णांक mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#अगर 0
  पूर्णांक iIoerrCnt;
  पूर्णांक ioerr;
  पूर्णांक nIoerrFail;
  पूर्णांक iFullCnt;
  पूर्णांक fullerr;
  पूर्णांक nFullFail;
#पूर्ण_अगर

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
#घोषणा TESTVFS_SHMOPEN_MASK      0x00000001
#घोषणा TESTVFS_SHMLOCK_MASK      0x00000010
#घोषणा TESTVFS_SHMMAP_MASK       0x00000020
#घोषणा TESTVFS_SHMBARRIER_MASK   0x00000040
#घोषणा TESTVFS_SHMCLOSE_MASK     0x00000080

#घोषणा TESTVFS_OPEN_MASK         0x00000100
#घोषणा TESTVFS_SYNC_MASK         0x00000200
#घोषणा TESTVFS_DELETE_MASK       0x00000400
#घोषणा TESTVFS_CLOSE_MASK        0x00000800
#घोषणा TESTVFS_WRITE_MASK        0x00001000
#घोषणा TESTVFS_TRUNCATE_MASK     0x00002000
#घोषणा TESTVFS_ACCESS_MASK       0x00004000
#घोषणा TESTVFS_FULLPATHNAME_MASK 0x00008000
#घोषणा TESTVFS_READ_MASK         0x00010000
#घोषणा TESTVFS_UNLOCK_MASK       0x00020000
#घोषणा TESTVFS_LOCK_MASK         0x00040000
#घोषणा TESTVFS_CKLOCK_MASK       0x00080000
#घोषणा TESTVFS_FCNTL_MASK        0x00100000
#घोषणा TESTVFS_SLEEP_MASK        0x00200000

#घोषणा TESTVFS_ALL_MASK          0x003FFFFF


#घोषणा TESTVFS_MAX_PAGES 1024

/*
** A shared-memory buffer. There is one of these objects for each shared
** memory region opened by clients. If two clients open the same file,
** there are two TestvfsFile structures but only one TestvfsBuffer structure.
*/
काष्ठा Testvfs_बफर {
  अक्षर *zFile;                    /* Associated file name */
  पूर्णांक pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  Testvfs_बफर *p_अग्र;           /* Next in linked list of all buffers */
};


#घोषणा PARENTVFS(x) (((Testvfs *)((x)->p_App_तथ्य))->pParent)

#घोषणा TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
अटल पूर्णांक tvfsClose(sqlite3_file*);
अटल पूर्णांक tvfsRead(sqlite3_file*, व्योम*, पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक tvfsWrite(sqlite3_file*,स्थिर व्योम*,पूर्णांक iAmt, sqlite3_int64 iOfst);
अटल पूर्णांक tvfsTruncate(sqlite3_file*, sqlite3_int64 आकार);
अटल पूर्णांक tvfsSync(sqlite3_file*, पूर्णांक flags);
अटल पूर्णांक tvfs_File_आकार(sqlite3_file*, sqlite3_int64 *p_आकार);
अटल पूर्णांक tvfsLock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक tvfsUnlock(sqlite3_file*, पूर्णांक);
अटल पूर्णांक tvfsCheckReservedLock(sqlite3_file*, पूर्णांक *);
अटल पूर्णांक tvfsFileControl(sqlite3_file*, पूर्णांक op, व्योम *pArg);
अटल पूर्णांक tvfs_Sector_आकार(sqlite3_file*);
अटल पूर्णांक tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
अटल पूर्णांक tvfsOpen(sqlite3_vfs*, स्थिर अक्षर *, sqlite3_file*, पूर्णांक , पूर्णांक *);
अटल पूर्णांक tvfsDelete(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक syncDir);
अटल पूर्णांक tvfsAccess(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक flags, पूर्णांक *);
अटल पूर्णांक tvfsFullPathname(sqlite3_vfs*, स्थिर अक्षर *zName, पूर्णांक, अक्षर *zOut);
#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
अटल व्योम *tvfsDlOpen(sqlite3_vfs*, स्थिर अक्षर *zFilename);
अटल व्योम tvfsDlError(sqlite3_vfs*, पूर्णांक nByte, अक्षर *z_Err_संदेश);
अटल व्योम (*tvfsDlSym(sqlite3_vfs*,व्योम*, स्थिर अक्षर *z_प्रतीक))(व्योम);
अटल व्योम tvfsDlClose(sqlite3_vfs*, व्योम*);
#पूर्ण_अगर /* SQLITE_OMIT_LOAD_EXTENSION */
अटल पूर्णांक tvfsRandomness(sqlite3_vfs*, पूर्णांक nByte, अक्षर *zOut);
अटल पूर्णांक tvfsSleep(sqlite3_vfs*, पूर्णांक microseconds);
अटल पूर्णांक tvfsCurrentTime(sqlite3_vfs*, द्विगुन*);

अटल पूर्णांक tvfsShmOpen(sqlite3_file*);
अटल पूर्णांक tvfsShmLock(sqlite3_file*, पूर्णांक , पूर्णांक, पूर्णांक);
अटल पूर्णांक tvfsShmMap(sqlite3_file*,पूर्णांक,पूर्णांक,पूर्णांक, व्योम अस्थिर **);
अटल व्योम tvfsShmBarrier(sqlite3_file*);
अटल पूर्णांक tvfsShmUnmap(sqlite3_file*, पूर्णांक);

अटल पूर्णांक tvfsFetch(sqlite3_file*, sqlite3_int64, पूर्णांक, व्योम**);
अटल पूर्णांक tvfsUnfetch(sqlite3_file*, sqlite3_int64, व्योम*);

अटल sqlite3_io_methods tvfs_io_methods = {
  3,                              /* iVersion */
  tvfsClose,                      /* xClose */
  tvfsRead,                       /* xRead */
  tvfsWrite,                      /* xWrite */
  tvfsTruncate,                   /* xTruncate */
  tvfsSync,                       /* xSync */
  tvfs_File_आकार,                   /* xFileSize */
  tvfsLock,                       /* xLock */
  tvfsUnlock,                     /* xUnlock */
  tvfsCheckReservedLock,          /* xCheckReservedLock */
  tvfsFileControl,                /* xFileControl */
  tvfs_Sector_आकार,                 /* xSectorSize */
  tvfsDeviceCharacteristics,      /* xDeviceCharacteristics */
  tvfsShmMap,                     /* xShmMap */
  tvfsShmLock,                    /* xShmLock */
  tvfsShmBarrier,                 /* xShmBarrier */
  tvfsShmUnmap,                   /* xShmUnmap */
  tvfsFetch,
  tvfsUnfetch
};

अटल पूर्णांक tvfs_परिणाम_Code(Testvfs *p, पूर्णांक *pRc){
  काष्ठा errcode {
    पूर्णांक eCode;
    स्थिर अक्षर *zCode;
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

  स्थिर अक्षर *z;
  पूर्णांक i;

  z = Tcl_Get_String_परिणाम(p->interp);
  क्रम(i=0; i<Array_आकार(aCode); i++){
    अगर( 0==म_भेद(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      वापस 1;
    }
  }

  वापस 0;
}

अटल पूर्णांक tvfsInjectFault(TestFaultInject *p){
  पूर्णांक ret = 0;
  अगर( p->eFault ){
    p->iCnt--;
    अगर( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  वापस ret;
}


अटल पूर्णांक tvfsInjectIoerr(Testvfs *p){
  वापस tvfsInjectFault(&p->ioerr_err);
}

अटल पूर्णांक tvfsInjectFullerr(Testvfs *p){
  वापस tvfsInjectFault(&p->full_err);
}
अटल पूर्णांक tvfsInjectCantopenerr(Testvfs *p){
  वापस tvfsInjectFault(&p->cantopen_err);
}


अटल व्योम tvfsExecTcl(
  Testvfs *p, 
  स्थिर अक्षर *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  पूर्णांक rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  निश्चित( p->pScript );

  निश्चित( zMethod );
  निश्चित( p );
  निश्चित( arg2==0 || arg1!=0 );
  निश्चित( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_गिनती(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  अगर( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  अगर( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  अगर( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  अगर( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  अगर( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_Reset_परिणाम(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
अटल पूर्णांक tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_तथ्य;

  अगर( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  अगर( pFd->pShmId ){
    Tcl_Decr_Ref_गिनती(pFd->pShmId);
    pFd->pShmId = 0;
  }
  अगर( pFile->pMethods ){
    ckfree((अक्षर *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((अक्षर *)pFd);
  pTestfile->pFd = 0;
  वापस SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
अटल पूर्णांक tvfsRead(
  sqlite3_file *pFile, 
  व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_तथ्य;
  अगर( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_परिणाम_Code(p, &rc);
  }
  अगर( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  अगर( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  वापस rc;
}

/*
** Write data to an tvfs-file.
*/
अटल पूर्णांक tvfsWrite(
  sqlite3_file *pFile, 
  स्थिर व्योम *zBuf, 
  पूर्णांक iAmt, 
  sqlite_int64 iOfst
){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_तथ्य;

  अगर( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfs_परिणाम_Code(p, &rc);
    अगर( rc<0 ) वापस SQLITE_OK;
  }

  अगर( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  अगर( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  अगर( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  वापस rc;
}

/*
** Truncate an tvfs-file.
*/
अटल पूर्णांक tvfsTruncate(sqlite3_file *pFile, sqlite_int64 आकार){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_तथ्य;

  अगर( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_परिणाम_Code(p, &rc);
  }
  
  अगर( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, आकार);
  }
  वापस rc;
}

/*
** Sync an tvfs-file.
*/
अटल पूर्णांक tvfsSync(sqlite3_file *pFile, पूर्णांक flags){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_तथ्य;

  अगर( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    अक्षर *zFlags = 0;

    चयन( flags ){
      हाल SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        अवरोध;
      हाल SQLITE_SYNC_FULL:
        zFlags = "full";
        अवरोध;
      हाल SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        अवरोध;
      हाल SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        अवरोध;
      शेष:
        निश्चित(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfs_परिणाम_Code(p, &rc);
  }

  अगर( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  अगर( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  वापस rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
अटल पूर्णांक tvfs_File_आकार(sqlite3_file *pFile, sqlite_int64 *p_आकार){
  TestvfsFd *p = tvfsGetFd(pFile);
  वापस sqlite3_Os_File_आकार(p->pReal, p_आकार);
}

/*
** Lock an tvfs-file.
*/
अटल पूर्णांक tvfsLock(sqlite3_file *pFile, पूर्णांक eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_तथ्य;
  अगर( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    अक्षर zLock[30];
    sqlite3_snprintf(माप(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  अगर( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    वापस SQLITE_IOERR_LOCK;
  }
  वापस sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
अटल पूर्णांक tvfsUnlock(sqlite3_file *pFile, पूर्णांक eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_तथ्य;
  अगर( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    अक्षर zLock[30];
    sqlite3_snprintf(माप(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  अगर( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    वापस SQLITE_IOERR_UNLOCK;
  }
  वापस sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
अटल पूर्णांक tvfsCheckReservedLock(sqlite3_file *pFile, पूर्णांक *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_तथ्य;
  अगर( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  वापस sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
अटल पूर्णांक tvfsFileControl(sqlite3_file *pFile, पूर्णांक op, व्योम *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_तथ्य;
  अगर( op==SQLITE_FCNTL_PRAGMA ){
    अक्षर **argv = (अक्षर**)pArg;
    अगर( sqlite3_stricmp(argv[1],"error")==0 ){
      पूर्णांक rc = SQLITE_ERROR;
      अगर( argv[2] ){
        स्थिर अक्षर *z = argv[2];
        पूर्णांक x = म_से_प(z);
        अगर( x ){
          rc = x;
          जबतक( sqlite3Isdigit(z[0]) ){ z++; }
          जबतक( sqlite3Isspace(z[0]) ){ z++; }
        }
        अगर( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      वापस rc;
    }
    अगर( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      वापस SQLITE_OK;
    }
  }
  अगर( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    काष्ठा Fcntl {
      पूर्णांक iFnctl;
      स्थिर अक्षर *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    पूर्णांक i;
    क्रम(i=0; i<माप(aF)/माप(aF[0]); i++){
      अगर( op==aF[i].iFnctl ) अवरोध;
    }
    अगर( i<माप(aF)/माप(aF[0]) ){
      पूर्णांक rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfs_परिणाम_Code(p, &rc);
      अगर( rc ) वापस (rc<0 ? SQLITE_OK : rc);
    }
  }
  वापस sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
अटल पूर्णांक tvfs_Sector_आकार(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_तथ्य;
  अगर( p->iSectorsize>=0 ){
    वापस p->iSectorsize;
  }
  वापस sqlite3_Os_Sector_आकार(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
अटल पूर्णांक tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_तथ्य;
  अगर( p->iDevchar>=0 ){
    वापस p->iDevchar;
  }
  वापस sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
अटल पूर्णांक tvfsOpen(
  sqlite3_vfs *pVfs,
  स्थिर अक्षर *zName,
  sqlite3_file *pFile,
  पूर्णांक flags,
  पूर्णांक *pOutFlags
){
  पूर्णांक rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->p_App_तथ्य;

  pFd = (TestvfsFd *)ckalloc(माप(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  स_रखो(pFd, 0, माप(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  स_रखो(pTestfile, 0, माप(TestvfsFile));
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
  अगर( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_गिनती(pArg);
    अगर( flags&SQLITE_OPEN_MAIN_DB ){
      स्थिर अक्षर *z = &zName[म_माप(zName)+1];
      जबतक( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += म_माप(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += म_माप(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_गिनती(pArg);
    अगर( tvfs_परिणाम_Code(p, &rc) ){
      अगर( rc!=SQLITE_OK ) वापस rc;
    }अन्यथा{
      pId = Tcl_Get_Obj_परिणाम(p->interp);
    }
  }

  अगर( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) वापस SQLITE_IOERR;
  अगर( tvfsInjectCantopenerr(p) ) वापस SQLITE_CANTOPEN;
  अगर( tvfsInjectFullerr(p) ) वापस SQLITE_FULL;

  अगर( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_गिनती(pId);
  pFd->pShmId = pId;
  Tcl_Reset_परिणाम(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  अगर( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    पूर्णांक nByte;

    अगर( pVfs->iVersion>1 ){
      nByte = माप(sqlite3_io_methods);
    }अन्यथा{
      nByte = दुरत्व(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    स_नकल(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    अगर( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    अगर( pVfs->iVersion>1 && ((Testvfs *)pVfs->p_App_तथ्य)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  वापस rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
अटल पूर्णांक tvfsDelete(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath, पूर्णांक dirSync){
  पूर्णांक rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->p_App_तथ्य;

  अगर( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfs_परिणाम_Code(p, &rc);
  }
  अगर( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  वापस rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
अटल पूर्णांक tvfsAccess(
  sqlite3_vfs *pVfs, 
  स्थिर अक्षर *zPath, 
  पूर्णांक flags, 
  पूर्णांक *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->p_App_तथ्य;
  अगर( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    पूर्णांक rc;
    अक्षर *zArg = 0;
    अगर( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    अगर( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    अगर( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    अगर( tvfs_परिणाम_Code(p, &rc) ){
      अगर( rc!=SQLITE_OK ) वापस rc;
    }अन्यथा{
      Tcl_Interp *interp = p->interp;
      अगर( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_Get_Obj_परिणाम(interp), pResOut) ){
        वापस SQLITE_OK;
      }
    }
  }
  वापस sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
अटल पूर्णांक tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  स्थिर अक्षर *zPath, 
  पूर्णांक nOut, 
  अक्षर *zOut
){
  Testvfs *p = (Testvfs *)pVfs->p_App_तथ्य;
  अगर( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    पूर्णांक rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    अगर( tvfs_परिणाम_Code(p, &rc) ){
      अगर( rc!=SQLITE_OK ) वापस rc;
    }
  }
  वापस sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
अटल व्योम *tvfsDlOpen(sqlite3_vfs *pVfs, स्थिर अक्षर *zPath){
  वापस sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
अटल व्योम tvfsDlError(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *z_Err_संदेश){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_संदेश);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
अटल व्योम (*tvfsDlSym(sqlite3_vfs *pVfs, व्योम *p, स्थिर अक्षर *zSym))(व्योम){
  वापस sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
अटल व्योम tvfsDlClose(sqlite3_vfs *pVfs, व्योम *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#पूर्ण_अगर /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
अटल पूर्णांक tvfsRandomness(sqlite3_vfs *pVfs, पूर्णांक nByte, अक्षर *zBufOut){
  वापस sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
अटल पूर्णांक tvfsSleep(sqlite3_vfs *pVfs, पूर्णांक nMicro){
  Testvfs *p = (Testvfs *)pVfs->p_App_तथ्य;
  अगर( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  वापस sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
अटल पूर्णांक tvfsCurrentTime(sqlite3_vfs *pVfs, द्विगुन *pTimeOut){
  वापस PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

अटल पूर्णांक tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  पूर्णांक rc = SQLITE_OK;             /* Return code */
  Testvfs_बफर *p_बफर;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->p_App_तथ्य;
  निश्चित( 0==p->isFullshm );
  निश्चित( pFd->pShmId && pFd->pShm==0 && pFd->p_अग्र==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_Reset_परिणाम(p->interp);
  अगर( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    अगर( tvfs_परिणाम_Code(p, &rc) ){
      अगर( rc!=SQLITE_OK ) वापस rc;
    }
  }

  निश्चित( rc==SQLITE_OK );
  अगर( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    वापस SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  क्रम(p_बफर=p->p_बफर; p_बफर; p_बफर=p_बफर->p_अग्र){
    अगर( 0==म_भेद(pFd->zFilename, p_बफर->zFile) ) अवरोध;
  }
  अगर( !p_बफर ){
    पूर्णांक szName = (पूर्णांक)म_माप(pFd->zFilename);
    पूर्णांक nByte = माप(Testvfs_बफर) + szName + 1;
    p_बफर = (Testvfs_बफर *)ckalloc(nByte);
    स_रखो(p_बफर, 0, nByte);
    p_बफर->zFile = (अक्षर *)&p_बफर[1];
    स_नकल(p_बफर->zFile, pFd->zFilename, szName+1);
    p_बफर->p_अग्र = p->p_बफर;
    p->p_बफर = p_बफर;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->p_अग्र = p_बफर->pFile;
  p_बफर->pFile = pFd;
  pFd->pShm = p_बफर;
  वापस rc;
}

अटल व्योम tvfsAllocPage(Testvfs_बफर *p, पूर्णांक iPage, पूर्णांक pgsz){
  निश्चित( iPage<TESTVFS_MAX_PAGES );
  अगर( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    स_रखो(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

अटल पूर्णांक tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  पूर्णांक iPage,                      /* Page to retrieve */
  पूर्णांक pgsz,                       /* Size of pages */
  पूर्णांक isWrite,                    /* True to extend file if necessary */
  व्योम अस्थिर **pp              /* OUT: Mapped memory */
){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_तथ्य);

  अगर( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    वापस pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  अगर( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    अगर( rc!=SQLITE_OK ){
      वापस rc;
    }
  }

  अगर( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_गिनती(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(iPage));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfs_परिणाम_Code(p, &rc);
    Tcl_Decr_Ref_गिनती(pArg);
  }
  अगर( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  अगर( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  अगर( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (व्योम अस्थिर *)pFd->pShm->aPage[iPage];
  }

  वापस rc;
}


अटल पूर्णांक tvfsShmLock(
  sqlite3_file *pFile,
  पूर्णांक ofst,
  पूर्णांक n,
  पूर्णांक flags
){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_तथ्य);
  पूर्णांक nLock;
  अक्षर zLock[80];

  अगर( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    वापस pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  अगर( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(माप(zLock), zLock, "%d %d", ofst, n);
    nLock = (पूर्णांक)म_माप(zLock);
    अगर( flags & SQLITE_SHM_LOCK ){
      म_नकल(&zLock[nLock], " lock");
    }अन्यथा{
      म_नकल(&zLock[nLock], " unlock");
    }
    nLock += (पूर्णांक)म_माप(&zLock[nLock]);
    अगर( flags & SQLITE_SHM_SHARED ){
      म_नकल(&zLock[nLock], " shared");
    }अन्यथा{
      म_नकल(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfs_परिणाम_Code(p, &rc);
  }

  अगर( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  अगर( rc==SQLITE_OK ){
    पूर्णांक isLock = (flags & SQLITE_SHM_LOCK);
    पूर्णांक isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    अगर( isLock ){
      TestvfsFd *p2;
      क्रम(p2=pFd->pShm->pFile; p2; p2=p2->p_अग्र){
        अगर( p2==pFd ) जारी;
        अगर( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          अवरोध;
        }
      }
      अगर( rc==SQLITE_OK ){
        अगर( isExcl )  pFd->excllock |= mask;
        अगर( !isExcl ) pFd->sharedlock |= mask;
      }
    }अन्यथा{
      अगर( isExcl )  pFd->excllock &= (~mask);
      अगर( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  वापस rc;
}

अटल व्योम tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_तथ्य);

  अगर( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    स्थिर अक्षर *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  अगर( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    वापस;
  }
}

अटल पूर्णांक tvfsShmUnmap(
  sqlite3_file *pFile,
  पूर्णांक deleteFlag
){
  पूर्णांक rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_तथ्य);
  Testvfs_बफर *p_बफर = pFd->pShm;
  TestvfsFd **ppFd;

  अगर( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    वापस pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  अगर( !p_बफर ) वापस SQLITE_OK;
  निश्चित( pFd->pShmId && pFd->pShm );

  अगर( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfs_परिणाम_Code(p, &rc);
  }

  क्रम(ppFd=&p_बफर->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->p_अग्र));
  निश्चित( (*ppFd)==pFd );
  *ppFd = pFd->p_अग्र;
  pFd->p_अग्र = 0;

  अगर( p_बफर->pFile==0 ){
    पूर्णांक i;
    Testvfs_बफर **pp;
    क्रम(pp=&p->p_बफर; *pp!=p_बफर; pp=&((*pp)->p_अग्र));
    *pp = (*pp)->p_अग्र;
    क्रम(i=0; p_बफर->aPage[i]; i++){
      ckfree((अक्षर *)p_बफर->aPage[i]);
    }
    ckfree((अक्षर *)p_बफर);
  }
  pFd->pShm = 0;

  वापस rc;
}

अटल पूर्णांक tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    पूर्णांक iAmt, 
    व्योम **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  वापस sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

अटल पूर्णांक tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, व्योम *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  वापस sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

अटल पूर्णांक SQLITE_TCLAPI testvfs_obj_cmd(
  Client_तथ्य cd,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  क्रमागत DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  काष्ठा TestvfsSubcmd {
    अक्षर *zName;
    क्रमागत DB_enum eCmd;
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
  
  अगर( objc<2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "SUBCOMMAND ...");
    वापस TCL_ERROR;
  }
  अगर( Tcl_Get_सूचकांक_From_Obj_Struct(
        interp, objv[1], aSubcmd, माप(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    वापस TCL_ERROR;
  }
  Tcl_Reset_परिणाम(interp);

  चयन( aSubcmd[i].eCmd ){
    हाल CMD_SHM: {
      Tcl_Obj *pObj;
      पूर्णांक rc;
      Testvfs_बफर *p_बफर;
      अक्षर *zName;
      अगर( objc!=3 && objc!=4 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "FILE ?VALUE?");
        वापस TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      अगर( rc!=SQLITE_OK ){
        Tcl_Append_परिणाम(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), शून्य);
        ckfree(zName);
        वापस TCL_ERROR;
      }
      क्रम(p_बफर=p->p_बफर; p_बफर; p_बफर=p_बफर->p_अग्र){
        अगर( 0==म_भेद(p_बफर->zFile, zName) ) अवरोध;
      }
      ckfree(zName);
      अगर( !p_बफर ){
        Tcl_Append_परिणाम(interp, "no such file: ", Tcl_GetString(objv[2]), शून्य);
        वापस TCL_ERROR;
      }
      अगर( objc==4 ){
        Tcl_आकार n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        पूर्णांक pgsz = p_बफर->pgsz;
        अगर( pgsz==0 ) pgsz = 65536;
        क्रम(i=0; i*pgsz<(पूर्णांक)n; i++){
          पूर्णांक nByte = pgsz;
          tvfsAllocPage(p_बफर, i, pgsz);
          अगर( n-i*pgsz<pgsz ){
            nByte = (पूर्णांक)n;
          }
          स_नकल(p_बफर->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      क्रम(i=0; p_बफर->aPage[i]; i++){
        पूर्णांक pgsz = p_बफर->pgsz;
        अगर( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(p_बफर->aPage[i], pgsz));
      }
      Tcl_Set_Obj_परिणाम(interp, pObj);
      अवरोध;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    हाल CMD_FILTER: {
      अटल काष्ठा VfsMethod {
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
      Tcl_आकार nElem = 0;
      पूर्णांक mask = 0;
      अगर( objc!=3 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "LIST");
        वापस TCL_ERROR;
      }
      अगर( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        वापस TCL_ERROR;
      }
      Tcl_Reset_परिणाम(interp);
      क्रम(i=0; i<(पूर्णांक)nElem; i++){
        पूर्णांक iMethod;
        अक्षर *zElem = Tcl_GetString(apElem[i]);
        क्रम(iMethod=0; iMethod<Array_आकार(vfsmethod); iMethod++){
          अगर( म_भेद(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            अवरोध;
          }
        }
        अगर( iMethod==Array_आकार(vfsmethod) ){
          Tcl_Append_परिणाम(interp, "unknown method: ", zElem, शून्य);
          वापस TCL_ERROR;
        }
      }
      p->mask = mask;
      अवरोध;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    हाल CMD_SCRIPT: {
      अगर( objc==3 ){
        Tcl_आकार nByte;
        अगर( p->pScript ){
          Tcl_Decr_Ref_गिनती(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        अगर( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_गिनती(p->pScript);
        }
      }अन्यथा अगर( objc!=2 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "?SCRIPT?");
        वापस TCL_ERROR;
      }

      Tcl_Reset_परिणाम(interp);
      अगर( p->pScript ) Tcl_Set_Obj_परिणाम(interp, p->pScript);

      अवरोध;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    हाल CMD_CANTOPENERR:
    हाल CMD_IOERR:
    हाल CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      पूर्णांक iRet;

      चयन( aSubcmd[i].eCmd ){
        हाल CMD_IOERR: pTest = &p->ioerr_err; अवरोध;
        हाल CMD_FULLERR: pTest = &p->full_err; अवरोध;
        हाल CMD_CANTOPENERR: pTest = &p->cantopen_err; अवरोध;
        शेष: निश्चित(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      अगर( objc==4 ){
        पूर्णांक iCnt, iPersist;
        अगर( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          वापस TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }अन्यथा अगर( objc!=2 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "?CNT PERSIST?");
        वापस TCL_ERROR;
      }
      Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(iRet));
      अवरोध;
    }

    हाल CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      अवरोध;
    }

    हाल CMD_DEVCHAR: {
      काष्ठा DeviceFlag {
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

      अगर( objc>3 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "?ATTR-LIST?");
        वापस TCL_ERROR;
      }
      अगर( objc==3 ){
        पूर्णांक j;
        पूर्णांक iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_आकार nFlags = 0;

        अगर( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          वापस TCL_ERROR;
        }

        क्रम(j=0; j<(पूर्णांक)nFlags; j++){
          पूर्णांक idx = 0;
          अगर( Tcl_Get_सूचकांक_From_Obj_Struct(interp, flags[j], aFlag, 
                माप(aFlag[0]), "flag", 0, &idx) 
          ){
            वापस TCL_ERROR;
          }
          अगर( aFlag[idx].i_मूल्य<0 && nFlags>1 ){
            Tcl_Append_परिणाम(interp, "bad flags: ", Tcl_GetString(objv[2]), शून्य);
            वापस TCL_ERROR;
          }
          iNew |= aFlag[idx].i_मूल्य;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      क्रम(iFlag=0; iFlag<माप(aFlag)/माप(aFlag[0]); iFlag++){
        अगर( p->iDevchar & aFlag[iFlag].i_मूल्य ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_Set_Obj_परिणाम(interp, pRet);

      अवरोध;
    }

    हाल CMD_SECTORSIZE: {
      अगर( objc>3 ){
        Tcl_Wrong_अंक_Args(interp, 2, objv, "?VALUE?");
        वापस TCL_ERROR;
      }
      अगर( objc==3 ){
        पूर्णांक iNew = 0;
        अगर( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          वापस TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(p->iSectorsize));
      अवरोध;
    }
  }

  वापस TCL_OK;
}

अटल व्योम SQLITE_TCLAPI testvfs_obj_del(Client_तथ्य cd){
  Testvfs *p = (Testvfs *)cd;
  अगर( p->pScript ) Tcl_Decr_Ref_गिनती(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  स_रखो(p->pVfs, 0, माप(sqlite3_vfs));
  ckfree((अक्षर *)p->pVfs);
  स_रखो(p, 0, माप(Testvfs));
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
अटल पूर्णांक SQLITE_TCLAPI testvfs_cmd(
  Client_तथ्य cd,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  अटल sqlite3_vfs tvfs_vfs = {
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
#अगर_अघोषित SQLITE_OMIT_LOAD_EXTENSION
    tvfsDlOpen,                   /* xDlOpen */
    tvfsDlError,                  /* xDlError */
    tvfsDlSym,                    /* xDlSym */
    tvfsDlClose,                  /* xDlClose */
#अन्यथा
    0,                            /* xDlOpen */
    0,                            /* xDlError */
    0,                            /* xDlSym */
    0,                            /* xDlClose */
#पूर्ण_अगर /* SQLITE_OMIT_LOAD_EXTENSION */
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

  अगर( objc<2 || 0!=(objc%2) ) जाओ bad_args;
  क्रम(i=2; i<objc; i += 2){
    Tcl_आकार nSwitch;
    अक्षर *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    अगर( nSwitch>2 && 0==म_भेदन("-noshm", zSwitch, nSwitch) ){
      अगर( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        वापस TCL_ERROR;
      }
      अगर( isNoshm ) isFullshm = 0;
    }
    अन्यथा अगर( nSwitch>2 && 0==म_भेदन("-default", zSwitch, nSwitch) ){
      अगर( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        वापस TCL_ERROR;
      }
    }
    अन्यथा अगर( nSwitch>2 && 0==म_भेदन("-szosfile", zSwitch, nSwitch) ){
      अगर( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        वापस TCL_ERROR;
      }
    }
    अन्यथा अगर( nSwitch>2 && 0==म_भेदन("-mxpathname", zSwitch, nSwitch) ){
      अगर( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        वापस TCL_ERROR;
      }
    }
    अन्यथा अगर( nSwitch>2 && 0==म_भेदन("-iversion", zSwitch, nSwitch) ){
      अगर( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        वापस TCL_ERROR;
      }
    }
    अन्यथा अगर( nSwitch>2 && 0==म_भेदन("-fullshm", zSwitch, nSwitch) ){
      अगर( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        वापस TCL_ERROR;
      }
      अगर( isFullshm ) isNoshm = 0;
    }
    अन्यथा{
      जाओ bad_args;
    }
  }

  अगर( szOsFile<माप(TestvfsFile) ){
    szOsFile = माप(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = माप(Testvfs) + (पूर्णांक)म_माप(zVfs)+1;
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

  pVfs = (sqlite3_vfs *)ckalloc(माप(sqlite3_vfs));
  स_नकल(pVfs, &tvfs_vfs, माप(sqlite3_vfs));
  pVfs->p_App_तथ्य = (व्योम *)p;
  pVfs->iVersion = iVersion;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  अगर( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  वापस TCL_OK;

 bad_args:
  Tcl_Wrong_अंक_Args(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  वापस TCL_ERROR;
}

बाह्य पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिर अक्षर *zA, sqlite3 **ppDb);
बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
अटल पूर्णांक SQLITE_TCLAPI test_vfs_shmlock(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  स्थिर अक्षर *azArg1[] = {"shared", "exclusive", 0};
  स्थिर अक्षर *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  पूर्णांक rc = SQLITE_OK;
  स्थिर अक्षर *zDbname = 0;
  पूर्णांक iArg1 = 0;
  पूर्णांक iArg2 = 0;
  पूर्णांक iOffset = 0;
  पूर्णांक n = 0;
  sqlite3_file *pFd;

  अगर( objc!=7 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    वापस TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_Get_सूचकांक_From_Obj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_Get_सूचकांक_From_Obj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    वापस TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (व्योम*)&pFd);
  अगर( pFd==0 ){
    वापस TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_vfs_set_readmark(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  पूर्णांक rc = SQLITE_OK;
  स्थिर अक्षर *zDbname = 0;
  पूर्णांक iSlot = 0;
  पूर्णांक iVal = -1;
  sqlite3_file *pFd;
  व्योम अस्थिर *pShm = 0;
  u32 *aShm;
  पूर्णांक iOff;

  अगर( objc!=4 && objc!=5 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    वापस TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    वापस TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (व्योम*)&pFd);
  अगर( pFd==0 ){
    वापस TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  अगर( rc!=SQLITE_OK ){
    Tcl_Set_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    वापस TCL_ERROR;
  }
  अगर( pShm==0 ){
    Tcl_Append_परिणाम(interp, "*-shm is not yet mapped", शून्य);
    वापस TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  अगर( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(aShm[iOff]));

  वापस TCL_OK;
}

पूर्णांक Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  वापस TCL_OK;
}

#पूर्ण_अगर
