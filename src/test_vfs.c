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
#જો SQLITE_TEST          /* This file is used for testing only */

#અન્તર્ભાવ "sqlite3.h"
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"

પ્રકારવ્યાખ્યા સંરચના Testvfs Testvfs;
પ્રકારવ્યાખ્યા સંરચના TestvfsShm TestvfsShm;
પ્રકારવ્યાખ્યા સંરચના TestvfsBuffer TestvfsBuffer;
પ્રકારવ્યાખ્યા સંરચના TestvfsFile TestvfsFile;
પ્રકારવ્યાખ્યા સંરચના TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
સંરચના TestvfsFile {
  sqlite3_file base;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#define tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

સંરચના TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  અચલ અક્ષર *zFilename;          /* Filename as passed to xOpen() */
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

પ્રકારવ્યાખ્યા સંરચના TestFaultInject TestFaultInject;
સંરચના TestFaultInject {
  પૂર્ણાંક iCnt;                       /* Remaining calls before fault injection */
  પૂર્ણાંક eFault;                     /* A FAULT_INJECT_* value */
  પૂર્ણાંક nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
સંરચના Testvfs {
  અક્ષર *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  TestvfsBuffer *pBuffer;         /* List of shared buffers */
  પૂર્ણાંક isNoshm;
  પૂર્ણાંક isFullshm;

  પૂર્ણાંક mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#જો 0
  પૂર્ણાંક iIoerrCnt;
  પૂર્ણાંક ioerr;
  પૂર્ણાંક nIoerrFail;
  પૂર્ણાંક iFullCnt;
  પૂર્ણાંક fullerr;
  પૂર્ણાંક nFullFail;
#endif

  પૂર્ણાંક iDevchar;
  પૂર્ણાંક iSectorsize;
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
સંરચના TestvfsBuffer {
  અક્ષર *zFile;                    /* Associated file name */
  પૂર્ણાંક pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  TestvfsBuffer *pNext;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->pAppData))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
સ્થિર પૂર્ણાંક tvfsClose(sqlite3_file*);
સ્થિર પૂર્ણાંક tvfsRead(sqlite3_file*, શૂન્ય*, પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક tvfsWrite(sqlite3_file*,અચલ શૂન્ય*,પૂર્ણાંક iAmt, sqlite3_int64 iOfst);
સ્થિર પૂર્ણાંક tvfsTruncate(sqlite3_file*, sqlite3_int64 size);
સ્થિર પૂર્ણાંક tvfsSync(sqlite3_file*, પૂર્ણાંક flags);
સ્થિર પૂર્ણાંક tvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
સ્થિર પૂર્ણાંક tvfsLock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક tvfsUnlock(sqlite3_file*, પૂર્ણાંક);
સ્થિર પૂર્ણાંક tvfsCheckReservedLock(sqlite3_file*, પૂર્ણાંક *);
સ્થિર પૂર્ણાંક tvfsFileControl(sqlite3_file*, પૂર્ણાંક op, શૂન્ય *pArg);
સ્થિર પૂર્ણાંક tvfsSectorSize(sqlite3_file*);
સ્થિર પૂર્ણાંક tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
સ્થિર પૂર્ણાંક tvfsOpen(sqlite3_vfs*, અચલ અક્ષર *, sqlite3_file*, પૂર્ણાંક , પૂર્ણાંક *);
સ્થિર પૂર્ણાંક tvfsDelete(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક syncDir);
સ્થિર પૂર્ણાંક tvfsAccess(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક flags, પૂર્ણાંક *);
સ્થિર પૂર્ણાંક tvfsFullPathname(sqlite3_vfs*, અચલ અક્ષર *zName, પૂર્ણાંક, અક્ષર *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
સ્થિર શૂન્ય *tvfsDlOpen(sqlite3_vfs*, અચલ અક્ષર *zFilename);
સ્થિર શૂન્ય tvfsDlError(sqlite3_vfs*, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ);
સ્થિર શૂન્ય (*tvfsDlSym(sqlite3_vfs*,શૂન્ય*, અચલ અક્ષર *z_પ્રતીક))(શૂન્ય);
સ્થિર શૂન્ય tvfsDlClose(sqlite3_vfs*, શૂન્ય*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
સ્થિર પૂર્ણાંક tvfsRandomness(sqlite3_vfs*, પૂર્ણાંક nByte, અક્ષર *zOut);
સ્થિર પૂર્ણાંક tvfsSleep(sqlite3_vfs*, પૂર્ણાંક microseconds);
સ્થિર પૂર્ણાંક tvfsCurrentTime(sqlite3_vfs*, દ્વિગુણ*);

સ્થિર પૂર્ણાંક tvfsShmOpen(sqlite3_file*);
સ્થિર પૂર્ણાંક tvfsShmLock(sqlite3_file*, પૂર્ણાંક , પૂર્ણાંક, પૂર્ણાંક);
સ્થિર પૂર્ણાંક tvfsShmMap(sqlite3_file*,પૂર્ણાંક,પૂર્ણાંક,પૂર્ણાંક, શૂન્ય અસ્થિર **);
સ્થિર શૂન્ય tvfsShmBarrier(sqlite3_file*);
સ્થિર પૂર્ણાંક tvfsShmUnmap(sqlite3_file*, પૂર્ણાંક);

સ્થિર પૂર્ણાંક tvfsFetch(sqlite3_file*, sqlite3_int64, પૂર્ણાંક, શૂન્ય**);
સ્થિર પૂર્ણાંક tvfsUnfetch(sqlite3_file*, sqlite3_int64, શૂન્ય*);

સ્થિર sqlite3_io_methods tvfs_io_methods = {
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

સ્થિર પૂર્ણાંક tvfsResultCode(Testvfs *p, પૂર્ણાંક *pRc){
  સંરચના errcode {
    પૂર્ણાંક eCode;
    અચલ અક્ષર *zCode;
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

  અચલ અક્ષર *z;
  પૂર્ણાંક i;

  z = Tcl_GetStringResult(p->interp);
  માટે(i=0; i<ArraySize(aCode); i++){
    જો( 0==strcmp(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      પાછા 1;
    }
  }

  પાછા 0;
}

સ્થિર પૂર્ણાંક tvfsInjectFault(TestFaultInject *p){
  પૂર્ણાંક ret = 0;
  જો( p->eFault ){
    p->iCnt--;
    જો( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  પાછા ret;
}


સ્થિર પૂર્ણાંક tvfsInjectIoerr(Testvfs *p){
  પાછા tvfsInjectFault(&p->ioerr_err);
}

સ્થિર પૂર્ણાંક tvfsInjectFullerr(Testvfs *p){
  પાછા tvfsInjectFault(&p->full_err);
}
સ્થિર પૂર્ણાંક tvfsInjectCantopenerr(Testvfs *p){
  પાછા tvfsInjectFault(&p->cantopen_err);
}


સ્થિર શૂન્ય tvfsExecTcl(
  Testvfs *p, 
  અચલ અક્ષર *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  પૂર્ણાંક rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  પ્રતિજ્ઞા( p->pScript );

  પ્રતિજ્ઞા( zMethod );
  પ્રતિજ્ઞા( p );
  પ્રતિજ્ઞા( arg2==0 || arg1!=0 );
  પ્રતિજ્ઞા( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ગણતરી(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  જો( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  જો( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  જો( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  જો( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  જો( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_ResetResult(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
સ્થિર પૂર્ણાંક tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  જો( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  જો( pFd->pShmId ){
    Tcl_Decr_Ref_ગણતરી(pFd->pShmId);
    pFd->pShmId = 0;
  }
  જો( pFile->pMethods ){
    ckfree((અક્ષર *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((અક્ષર *)pFd);
  pTestfile->pFd = 0;
  પાછા SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
સ્થિર પૂર્ણાંક tvfsRead(
  sqlite3_file *pFile, 
  શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  પૂર્ણાંક rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  જો( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  જો( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  જો( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  પાછા rc;
}

/*
** Write data to an tvfs-file.
*/
સ્થિર પૂર્ણાંક tvfsWrite(
  sqlite3_file *pFile, 
  અચલ શૂન્ય *zBuf, 
  પૂર્ણાંક iAmt, 
  sqlite_int64 iOfst
){
  પૂર્ણાંક rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  જો( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfsResultCode(p, &rc);
    જો( rc<0 ) પાછા SQLITE_OK;
  }

  જો( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  જો( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  જો( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  પાછા rc;
}

/*
** Truncate an tvfs-file.
*/
સ્થિર પૂર્ણાંક tvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  પૂર્ણાંક rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  જો( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  
  જો( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, size);
  }
  પાછા rc;
}

/*
** Sync an tvfs-file.
*/
સ્થિર પૂર્ણાંક tvfsSync(sqlite3_file *pFile, પૂર્ણાંક flags){
  પૂર્ણાંક rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  જો( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    અક્ષર *zFlags = 0;

    પસંદગી( flags ){
      સ્થિતિ SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        તોડો;
      સ્થિતિ SQLITE_SYNC_FULL:
        zFlags = "full";
        તોડો;
      સ્થિતિ SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        તોડો;
      સ્થિતિ SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        તોડો;
      મૂળભૂત:
        પ્રતિજ્ઞા(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  જો( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  જો( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  પાછા rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
સ્થિર પૂર્ણાંક tvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  TestvfsFd *p = tvfsGetFd(pFile);
  પાછા sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an tvfs-file.
*/
સ્થિર પૂર્ણાંક tvfsLock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  જો( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    અક્ષર zLock[30];
    sqlite3_snprintf(કદ(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  જો( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    પાછા SQLITE_IOERR_LOCK;
  }
  પાછા sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
સ્થિર પૂર્ણાંક tvfsUnlock(sqlite3_file *pFile, પૂર્ણાંક eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  જો( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    અક્ષર zLock[30];
    sqlite3_snprintf(કદ(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  જો( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    પાછા SQLITE_IOERR_UNLOCK;
  }
  પાછા sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
સ્થિર પૂર્ણાંક tvfsCheckReservedLock(sqlite3_file *pFile, પૂર્ણાંક *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  જો( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  પાછા sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
સ્થિર પૂર્ણાંક tvfsFileControl(sqlite3_file *pFile, પૂર્ણાંક op, શૂન્ય *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  જો( op==SQLITE_FCNTL_PRAGMA ){
    અક્ષર **argv = (અક્ષર**)pArg;
    જો( sqlite3_stricmp(argv[1],"error")==0 ){
      પૂર્ણાંક rc = SQLITE_ERROR;
      જો( argv[2] ){
        અચલ અક્ષર *z = argv[2];
        પૂર્ણાંક x = atoi(z);
        જો( x ){
          rc = x;
          જ્યારે( sqlite3Isdigit(z[0]) ){ z++; }
          જ્યારે( sqlite3Isspace(z[0]) ){ z++; }
        }
        જો( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      પાછા rc;
    }
    જો( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      પાછા SQLITE_OK;
    }
  }
  જો( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    સંરચના Fcntl {
      પૂર્ણાંક iFnctl;
      અચલ અક્ષર *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    પૂર્ણાંક i;
    માટે(i=0; i<કદ(aF)/કદ(aF[0]); i++){
      જો( op==aF[i].iFnctl ) તોડો;
    }
    જો( i<કદ(aF)/કદ(aF[0]) ){
      પૂર્ણાંક rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfsResultCode(p, &rc);
      જો( rc ) પાછા (rc<0 ? SQLITE_OK : rc);
    }
  }
  પાછા sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
સ્થિર પૂર્ણાંક tvfsSectorSize(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  જો( p->iSectorsize>=0 ){
    પાછા p->iSectorsize;
  }
  પાછા sqlite3OsSectorSize(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
સ્થિર પૂર્ણાંક tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  જો( p->iDevchar>=0 ){
    પાછા p->iDevchar;
  }
  પાછા sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
સ્થિર પૂર્ણાંક tvfsOpen(
  sqlite3_vfs *pVfs,
  અચલ અક્ષર *zName,
  sqlite3_file *pFile,
  પૂર્ણાંક flags,
  પૂર્ણાંક *pOutFlags
){
  પૂર્ણાંક rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  pFd = (TestvfsFd *)ckalloc(કદ(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, કદ(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, કદ(TestvfsFile));
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
  જો( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_ગણતરી(pArg);
    જો( flags&SQLITE_OPEN_MAIN_DB ){
      અચલ અક્ષર *z = &zName[strlen(zName)+1];
      જ્યારે( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_ગણતરી(pArg);
    જો( tvfsResultCode(p, &rc) ){
      જો( rc!=SQLITE_OK ) પાછા rc;
    }નહીંતર{
      pId = Tcl_GetObjResult(p->interp);
    }
  }

  જો( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) પાછા SQLITE_IOERR;
  જો( tvfsInjectCantopenerr(p) ) પાછા SQLITE_CANTOPEN;
  જો( tvfsInjectFullerr(p) ) પાછા SQLITE_FULL;

  જો( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_ગણતરી(pId);
  pFd->pShmId = pId;
  Tcl_ResetResult(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  જો( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    પૂર્ણાંક nByte;

    જો( pVfs->iVersion>1 ){
      nByte = કદ(sqlite3_io_methods);
    }નહીંતર{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    memcpy(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    જો( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    જો( pVfs->iVersion>1 && ((Testvfs *)pVfs->pAppData)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  પાછા rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
સ્થિર પૂર્ણાંક tvfsDelete(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath, પૂર્ણાંક dirSync){
  પૂર્ણાંક rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  જો( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  જો( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  પાછા rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
સ્થિર પૂર્ણાંક tvfsAccess(
  sqlite3_vfs *pVfs, 
  અચલ અક્ષર *zPath, 
  પૂર્ણાંક flags, 
  પૂર્ણાંક *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  જો( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    પૂર્ણાંક rc;
    અક્ષર *zArg = 0;
    જો( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    જો( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    જો( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    જો( tvfsResultCode(p, &rc) ){
      જો( rc!=SQLITE_OK ) પાછા rc;
    }નહીંતર{
      Tcl_Interp *interp = p->interp;
      જો( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_GetObjResult(interp), pResOut) ){
        પાછા SQLITE_OK;
      }
    }
  }
  પાછા sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
સ્થિર પૂર્ણાંક tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  અચલ અક્ષર *zPath, 
  પૂર્ણાંક nOut, 
  અક્ષર *zOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  જો( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    પૂર્ણાંક rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    જો( tvfsResultCode(p, &rc) ){
      જો( rc!=SQLITE_OK ) પાછા rc;
    }
  }
  પાછા sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
સ્થિર શૂન્ય *tvfsDlOpen(sqlite3_vfs *pVfs, અચલ અક્ષર *zPath){
  પાછા sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
સ્થિર શૂન્ય tvfsDlError(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *z_Err_સંદેશ){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_સંદેશ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
સ્થિર શૂન્ય (*tvfsDlSym(sqlite3_vfs *pVfs, શૂન્ય *p, અચલ અક્ષર *zSym))(શૂન્ય){
  પાછા sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
સ્થિર શૂન્ય tvfsDlClose(sqlite3_vfs *pVfs, શૂન્ય *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
સ્થિર પૂર્ણાંક tvfsRandomness(sqlite3_vfs *pVfs, પૂર્ણાંક nByte, અક્ષર *zBufOut){
  પાછા sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
સ્થિર પૂર્ણાંક tvfsSleep(sqlite3_vfs *pVfs, પૂર્ણાંક nMicro){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  જો( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  પાછા sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
સ્થિર પૂર્ણાંક tvfsCurrentTime(sqlite3_vfs *pVfs, દ્વિગુણ *pTimeOut){
  પાછા PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

સ્થિર પૂર્ણાંક tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  પૂર્ણાંક rc = SQLITE_OK;             /* Return code */
  TestvfsBuffer *pBuffer;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->pAppData;
  પ્રતિજ્ઞા( 0==p->isFullshm );
  પ્રતિજ્ઞા( pFd->pShmId && pFd->pShm==0 && pFd->pNext==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_ResetResult(p->interp);
  જો( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    જો( tvfsResultCode(p, &rc) ){
      જો( rc!=SQLITE_OK ) પાછા rc;
    }
  }

  પ્રતિજ્ઞા( rc==SQLITE_OK );
  જો( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    પાછા SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  માટે(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
    જો( 0==strcmp(pFd->zFilename, pBuffer->zFile) ) તોડો;
  }
  જો( !pBuffer ){
    પૂર્ણાંક szName = (પૂર્ણાંક)strlen(pFd->zFilename);
    પૂર્ણાંક nByte = કદ(TestvfsBuffer) + szName + 1;
    pBuffer = (TestvfsBuffer *)ckalloc(nByte);
    memset(pBuffer, 0, nByte);
    pBuffer->zFile = (અક્ષર *)&pBuffer[1];
    memcpy(pBuffer->zFile, pFd->zFilename, szName+1);
    pBuffer->pNext = p->pBuffer;
    p->pBuffer = pBuffer;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->pNext = pBuffer->pFile;
  pBuffer->pFile = pFd;
  pFd->pShm = pBuffer;
  પાછા rc;
}

સ્થિર શૂન્ય tvfsAllocPage(TestvfsBuffer *p, પૂર્ણાંક iPage, પૂર્ણાંક pgsz){
  પ્રતિજ્ઞા( iPage<TESTVFS_MAX_PAGES );
  જો( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    memset(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

સ્થિર પૂર્ણાંક tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  પૂર્ણાંક iPage,                      /* Page to retrieve */
  પૂર્ણાંક pgsz,                       /* Size of pages */
  પૂર્ણાંક isWrite,                    /* True to extend file if necessary */
  શૂન્ય અસ્થિર **pp              /* OUT: Mapped memory */
){
  પૂર્ણાંક rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  જો( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    પાછા pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  જો( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    જો( rc!=SQLITE_OK ){
      પાછા rc;
    }
  }

  જો( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_ગણતરી(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(iPage));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfsResultCode(p, &rc);
    Tcl_Decr_Ref_ગણતરી(pArg);
  }
  જો( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  જો( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  જો( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (શૂન્ય અસ્થિર *)pFd->pShm->aPage[iPage];
  }

  પાછા rc;
}


સ્થિર પૂર્ણાંક tvfsShmLock(
  sqlite3_file *pFile,
  પૂર્ણાંક ofst,
  પૂર્ણાંક n,
  પૂર્ણાંક flags
){
  પૂર્ણાંક rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  પૂર્ણાંક nLock;
  અક્ષર zLock[80];

  જો( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    પાછા pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  જો( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(કદ(zLock), zLock, "%d %d", ofst, n);
    nLock = (પૂર્ણાંક)strlen(zLock);
    જો( flags & SQLITE_SHM_LOCK ){
      strcpy(&zLock[nLock], " lock");
    }નહીંતર{
      strcpy(&zLock[nLock], " unlock");
    }
    nLock += (પૂર્ણાંક)strlen(&zLock[nLock]);
    જો( flags & SQLITE_SHM_SHARED ){
      strcpy(&zLock[nLock], " shared");
    }નહીંતર{
      strcpy(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  જો( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  જો( rc==SQLITE_OK ){
    પૂર્ણાંક isLock = (flags & SQLITE_SHM_LOCK);
    પૂર્ણાંક isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    જો( isLock ){
      TestvfsFd *p2;
      માટે(p2=pFd->pShm->pFile; p2; p2=p2->pNext){
        જો( p2==pFd ) ચાલુ;
        જો( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          તોડો;
        }
      }
      જો( rc==SQLITE_OK ){
        જો( isExcl )  pFd->excllock |= mask;
        જો( !isExcl ) pFd->sharedlock |= mask;
      }
    }નહીંતર{
      જો( isExcl )  pFd->excllock &= (~mask);
      જો( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  પાછા rc;
}

સ્થિર શૂન્ય tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  જો( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    અચલ અક્ષર *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  જો( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    પાછા;
  }
}

સ્થિર પૂર્ણાંક tvfsShmUnmap(
  sqlite3_file *pFile,
  પૂર્ણાંક deleteFlag
){
  પૂર્ણાંક rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  TestvfsBuffer *pBuffer = pFd->pShm;
  TestvfsFd **ppFd;

  જો( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    પાછા pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  જો( !pBuffer ) પાછા SQLITE_OK;
  પ્રતિજ્ઞા( pFd->pShmId && pFd->pShm );

  જો( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }

  માટે(ppFd=&pBuffer->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->pNext));
  પ્રતિજ્ઞા( (*ppFd)==pFd );
  *ppFd = pFd->pNext;
  pFd->pNext = 0;

  જો( pBuffer->pFile==0 ){
    પૂર્ણાંક i;
    TestvfsBuffer **pp;
    માટે(pp=&p->pBuffer; *pp!=pBuffer; pp=&((*pp)->pNext));
    *pp = (*pp)->pNext;
    માટે(i=0; pBuffer->aPage[i]; i++){
      ckfree((અક્ષર *)pBuffer->aPage[i]);
    }
    ckfree((અક્ષર *)pBuffer);
  }
  pFd->pShm = 0;

  પાછા rc;
}

સ્થિર પૂર્ણાંક tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    પૂર્ણાંક iAmt, 
    શૂન્ય **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  પાછા sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

સ્થિર પૂર્ણાંક tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, શૂન્ય *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  પાછા sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI testvfs_obj_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  ગણના DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  સંરચના TestvfsSubcmd {
    અક્ષર *zName;
    ગણના DB_enum eCmd;
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
  પૂર્ણાંક i;
  
  જો( objc<2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "SUBCOMMAND ...");
    પાછા TCL_ERROR;
  }
  જો( Tcl_GetIndexFromObjStruct(
        interp, objv[1], aSubcmd, કદ(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    પાછા TCL_ERROR;
  }
  Tcl_ResetResult(interp);

  પસંદગી( aSubcmd[i].eCmd ){
    સ્થિતિ CMD_SHM: {
      Tcl_Obj *pObj;
      પૂર્ણાંક rc;
      TestvfsBuffer *pBuffer;
      અક્ષર *zName;
      જો( objc!=3 && objc!=4 ){
        Tcl_Wrong_અંક_Args(interp, 2, objv, "FILE ?VALUE?");
        પાછા TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      જો( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), ખાલી);
        ckfree(zName);
        પાછા TCL_ERROR;
      }
      માટે(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
        જો( 0==strcmp(pBuffer->zFile, zName) ) તોડો;
      }
      ckfree(zName);
      જો( !pBuffer ){
        Tcl_AppendResult(interp, "no such file: ", Tcl_GetString(objv[2]), ખાલી);
        પાછા TCL_ERROR;
      }
      જો( objc==4 ){
        Tcl_Size n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        પૂર્ણાંક pgsz = pBuffer->pgsz;
        જો( pgsz==0 ) pgsz = 65536;
        માટે(i=0; i*pgsz<(પૂર્ણાંક)n; i++){
          પૂર્ણાંક nByte = pgsz;
          tvfsAllocPage(pBuffer, i, pgsz);
          જો( n-i*pgsz<pgsz ){
            nByte = (પૂર્ણાંક)n;
          }
          memcpy(pBuffer->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      માટે(i=0; pBuffer->aPage[i]; i++){
        પૂર્ણાંક pgsz = pBuffer->pgsz;
        જો( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(pBuffer->aPage[i], pgsz));
      }
      Tcl_SetObjResult(interp, pObj);
      તોડો;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    સ્થિતિ CMD_FILTER: {
      સ્થિર સંરચના VfsMethod {
        અક્ષર *zName;
        પૂર્ણાંક mask;
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
      પૂર્ણાંક mask = 0;
      જો( objc!=3 ){
        Tcl_Wrong_અંક_Args(interp, 2, objv, "LIST");
        પાછા TCL_ERROR;
      }
      જો( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        પાછા TCL_ERROR;
      }
      Tcl_ResetResult(interp);
      માટે(i=0; i<(પૂર્ણાંક)nElem; i++){
        પૂર્ણાંક iMethod;
        અક્ષર *zElem = Tcl_GetString(apElem[i]);
        માટે(iMethod=0; iMethod<ArraySize(vfsmethod); iMethod++){
          જો( strcmp(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            તોડો;
          }
        }
        જો( iMethod==ArraySize(vfsmethod) ){
          Tcl_AppendResult(interp, "unknown method: ", zElem, ખાલી);
          પાછા TCL_ERROR;
        }
      }
      p->mask = mask;
      તોડો;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    સ્થિતિ CMD_SCRIPT: {
      જો( objc==3 ){
        Tcl_Size nByte;
        જો( p->pScript ){
          Tcl_Decr_Ref_ગણતરી(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        જો( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_ગણતરી(p->pScript);
        }
      }નહીંતર જો( objc!=2 ){
        Tcl_Wrong_અંક_Args(interp, 2, objv, "?SCRIPT?");
        પાછા TCL_ERROR;
      }

      Tcl_ResetResult(interp);
      જો( p->pScript ) Tcl_SetObjResult(interp, p->pScript);

      તોડો;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    સ્થિતિ CMD_CANTOPENERR:
    સ્થિતિ CMD_IOERR:
    સ્થિતિ CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      પૂર્ણાંક iRet;

      પસંદગી( aSubcmd[i].eCmd ){
        સ્થિતિ CMD_IOERR: pTest = &p->ioerr_err; તોડો;
        સ્થિતિ CMD_FULLERR: pTest = &p->full_err; તોડો;
        સ્થિતિ CMD_CANTOPENERR: pTest = &p->cantopen_err; તોડો;
        મૂળભૂત: પ્રતિજ્ઞા(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      જો( objc==4 ){
        પૂર્ણાંક iCnt, iPersist;
        જો( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          પાછા TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }નહીંતર જો( objc!=2 ){
        Tcl_Wrong_અંક_Args(interp, 2, objv, "?CNT PERSIST?");
        પાછા TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(iRet));
      તોડો;
    }

    સ્થિતિ CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      તોડો;
    }

    સ્થિતિ CMD_DEVCHAR: {
      સંરચના DeviceFlag {
        અક્ષર *zName;
        પૂર્ણાંક i_કિંમત;
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
      પૂર્ણાંક iFlag;

      જો( objc>3 ){
        Tcl_Wrong_અંક_Args(interp, 2, objv, "?ATTR-LIST?");
        પાછા TCL_ERROR;
      }
      જો( objc==3 ){
        પૂર્ણાંક j;
        પૂર્ણાંક iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_Size nFlags = 0;

        જો( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          પાછા TCL_ERROR;
        }

        માટે(j=0; j<(પૂર્ણાંક)nFlags; j++){
          પૂર્ણાંક idx = 0;
          જો( Tcl_GetIndexFromObjStruct(interp, flags[j], aFlag, 
                કદ(aFlag[0]), "flag", 0, &idx) 
          ){
            પાછા TCL_ERROR;
          }
          જો( aFlag[idx].i_કિંમત<0 && nFlags>1 ){
            Tcl_AppendResult(interp, "bad flags: ", Tcl_GetString(objv[2]), ખાલી);
            પાછા TCL_ERROR;
          }
          iNew |= aFlag[idx].i_કિંમત;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      માટે(iFlag=0; iFlag<કદ(aFlag)/કદ(aFlag[0]); iFlag++){
        જો( p->iDevchar & aFlag[iFlag].i_કિંમત ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_SetObjResult(interp, pRet);

      તોડો;
    }

    સ્થિતિ CMD_SECTORSIZE: {
      જો( objc>3 ){
        Tcl_Wrong_અંક_Args(interp, 2, objv, "?VALUE?");
        પાછા TCL_ERROR;
      }
      જો( objc==3 ){
        પૂર્ણાંક iNew = 0;
        જો( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          પાછા TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(p->iSectorsize));
      તોડો;
    }
  }

  પાછા TCL_OK;
}

સ્થિર શૂન્ય SQLITE_TCLAPI testvfs_obj_del(ClientData cd){
  Testvfs *p = (Testvfs *)cd;
  જો( p->pScript ) Tcl_Decr_Ref_ગણતરી(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, કદ(sqlite3_vfs));
  ckfree((અક્ષર *)p->pVfs);
  memset(p, 0, કદ(Testvfs));
  ckfree((અક્ષર *)p);
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
સ્થિર પૂર્ણાંક SQLITE_TCLAPI testvfs_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  સ્થિર sqlite3_vfs tvfs_vfs = {
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
#નહીંતર
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
  અક્ષર *zVfs;
  પૂર્ણાંક nByte;                      /* Bytes of space to allocate at p */

  પૂર્ણાંક i;
  પૂર્ણાંક isNoshm = 0;                /* True if -noshm is passed */
  પૂર્ણાંક isFullshm = 0;              /* True if -fullshm is passed */
  પૂર્ણાંક isDefault = 0;              /* True if -default is passed */
  પૂર્ણાંક szOsFile = 0;               /* Value passed to -szosfile */
  પૂર્ણાંક mxPathname = -1;            /* Value passed to -mxpathname */
  પૂર્ણાંક iVersion = 3;               /* Value passed to -iversion */

  જો( objc<2 || 0!=(objc%2) ) જાઓ bad_args;
  માટે(i=2; i<objc; i += 2){
    Tcl_Size nSwitch;
    અક્ષર *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    જો( nSwitch>2 && 0==strncmp("-noshm", zSwitch, nSwitch) ){
      જો( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        પાછા TCL_ERROR;
      }
      જો( isNoshm ) isFullshm = 0;
    }
    નહીંતર જો( nSwitch>2 && 0==strncmp("-default", zSwitch, nSwitch) ){
      જો( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        પાછા TCL_ERROR;
      }
    }
    નહીંતર જો( nSwitch>2 && 0==strncmp("-szosfile", zSwitch, nSwitch) ){
      જો( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        પાછા TCL_ERROR;
      }
    }
    નહીંતર જો( nSwitch>2 && 0==strncmp("-mxpathname", zSwitch, nSwitch) ){
      જો( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        પાછા TCL_ERROR;
      }
    }
    નહીંતર જો( nSwitch>2 && 0==strncmp("-iversion", zSwitch, nSwitch) ){
      જો( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        પાછા TCL_ERROR;
      }
    }
    નહીંતર જો( nSwitch>2 && 0==strncmp("-fullshm", zSwitch, nSwitch) ){
      જો( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        પાછા TCL_ERROR;
      }
      જો( isFullshm ) isNoshm = 0;
    }
    નહીંતર{
      જાઓ bad_args;
    }
  }

  જો( szOsFile<કદ(TestvfsFile) ){
    szOsFile = કદ(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = કદ(Testvfs) + (પૂર્ણાંક)strlen(zVfs)+1;
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

  p->zName = (અક્ષર *)&p[1];
  memcpy(p->zName, zVfs, strlen(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(કદ(sqlite3_vfs));
  memcpy(pVfs, &tvfs_vfs, કદ(sqlite3_vfs));
  pVfs->pAppData = (શૂન્ય *)p;
  pVfs->iVersion = iVersion;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  જો( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  પાછા TCL_OK;

 bad_args:
  Tcl_Wrong_અંક_Args(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  પાછા TCL_ERROR;
}

બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp *interp, અચલ અક્ષર *zA, sqlite3 **ppDb);
બાહ્ય અચલ અક્ષર *sqlite3ErrName(પૂર્ણાંક);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_vfs_shmlock(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  અચલ અક્ષર *azArg1[] = {"shared", "exclusive", 0};
  અચલ અક્ષર *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  પૂર્ણાંક rc = SQLITE_OK;
  અચલ અક્ષર *zDbname = 0;
  પૂર્ણાંક iArg1 = 0;
  પૂર્ણાંક iArg2 = 0;
  પૂર્ણાંક iOffset = 0;
  પૂર્ણાંક n = 0;
  sqlite3_file *pFd;

  જો( objc!=7 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    પાછા TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIndexFromObj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_GetIndexFromObj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    પાછા TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (શૂન્ય*)&pFd);
  જો( pFd==0 ){
    પાછા TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_vfs_set_readmark(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  પૂર્ણાંક rc = SQLITE_OK;
  અચલ અક્ષર *zDbname = 0;
  પૂર્ણાંક iSlot = 0;
  પૂર્ણાંક iVal = -1;
  sqlite3_file *pFd;
  શૂન્ય અસ્થિર *pShm = 0;
  u32 *aShm;
  પૂર્ણાંક iOff;

  જો( objc!=4 && objc!=5 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    પાછા TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    પાછા TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (શૂન્ય*)&pFd);
  જો( pFd==0 ){
    પાછા TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  જો( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    પાછા TCL_ERROR;
  }
  જો( pShm==0 ){
    Tcl_AppendResult(interp, "*-shm is not yet mapped", ખાલી);
    પાછા TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  જો( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(aShm[iOff]));

  પાછા TCL_OK;
}

પૂર્ણાંક Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  પાછા TCL_OK;
}

#endif
