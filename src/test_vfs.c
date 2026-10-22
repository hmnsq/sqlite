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
#ถ้า SQLITE_TEST          /* This file is used for testing only */

#include "sqlite3.h"
#include "sqliteInt.h"
#include "tclsqlite.h"

typedef sa_nracanaa Testvfs Testvfs;
typedef sa_nracanaa TestvfsShm TestvfsShm;
typedef sa_nracanaa TestvfsBuffer TestvfsBuffer;
typedef sa_nracanaa TestvfsFile TestvfsFile;
typedef sa_nracanaa TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
sa_nracanaa TestvfsFile {
  sqlite3_file base;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#define tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

sa_nracanaa TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  const var_na *zFilename;          /* Filename as passed to xOpen() */
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

typedef sa_nracanaa TestFaultInject TestFaultInject;
sa_nracanaa TestFaultInject {
  จำนวนเต็ม iCnt;                       /* Remaining calls before fault injection */
  จำนวนเต็ม eFault;                     /* A FAULT_INJECT_* value */
  จำนวนเต็ม nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
sa_nracanaa Testvfs {
  var_na *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  TestvfsBuffer *pBuffer;         /* List of shared buffers */
  จำนวนเต็ม isNoshm;
  จำนวนเต็ม isFullshm;

  จำนวนเต็ม mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#ถ้า 0
  จำนวนเต็ม iIoerrCnt;
  จำนวนเต็ม ioerr;
  จำนวนเต็ม nIoerrFail;
  จำนวนเต็ม iFullCnt;
  จำนวนเต็ม fullerr;
  จำนวนเต็ม nFullFail;
#endif

  จำนวนเต็ม iDevchar;
  จำนวนเต็ม iSectorsize;
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
sa_nracanaa TestvfsBuffer {
  var_na *zFile;                    /* Associated file name */
  จำนวนเต็ม pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  TestvfsBuffer *pNext;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->pAppData))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
static จำนวนเต็ม tvfsClose(sqlite3_file*);
static จำนวนเต็ม tvfsRead(sqlite3_file*, void*, จำนวนเต็ม iAmt, sqlite3_int64 iOfst);
static จำนวนเต็ม tvfsWrite(sqlite3_file*,const void*,จำนวนเต็ม iAmt, sqlite3_int64 iOfst);
static จำนวนเต็ม tvfsTruncate(sqlite3_file*, sqlite3_int64 size);
static จำนวนเต็ม tvfsSync(sqlite3_file*, จำนวนเต็ม flags);
static จำนวนเต็ม tvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static จำนวนเต็ม tvfsLock(sqlite3_file*, จำนวนเต็ม);
static จำนวนเต็ม tvfsUnlock(sqlite3_file*, จำนวนเต็ม);
static จำนวนเต็ม tvfsCheckReservedLock(sqlite3_file*, จำนวนเต็ม *);
static จำนวนเต็ม tvfsFileControl(sqlite3_file*, จำนวนเต็ม op, void *pArg);
static จำนวนเต็ม tvfsSectorSize(sqlite3_file*);
static จำนวนเต็ม tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
static จำนวนเต็ม tvfsOpen(sqlite3_vfs*, const var_na *, sqlite3_file*, จำนวนเต็ม , จำนวนเต็ม *);
static จำนวนเต็ม tvfsDelete(sqlite3_vfs*, const var_na *zName, จำนวนเต็ม syncDir);
static จำนวนเต็ม tvfsAccess(sqlite3_vfs*, const var_na *zName, จำนวนเต็ม flags, จำนวนเต็ม *);
static จำนวนเต็ม tvfsFullPathname(sqlite3_vfs*, const var_na *zName, จำนวนเต็ม, var_na *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
static void *tvfsDlOpen(sqlite3_vfs*, const var_na *zFilename);
static void tvfsDlError(sqlite3_vfs*, จำนวนเต็ม nByte, var_na *zErrMsg);
static void (*tvfsDlSym(sqlite3_vfs*,void*, const var_na *zSymbol))(void);
static void tvfsDlClose(sqlite3_vfs*, void*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
static จำนวนเต็ม tvfsRandomness(sqlite3_vfs*, จำนวนเต็ม nByte, var_na *zOut);
static จำนวนเต็ม tvfsSleep(sqlite3_vfs*, จำนวนเต็ม microseconds);
static จำนวนเต็ม tvfsCurrentTime(sqlite3_vfs*, dviguu_na*);

static จำนวนเต็ม tvfsShmOpen(sqlite3_file*);
static จำนวนเต็ม tvfsShmLock(sqlite3_file*, จำนวนเต็ม , จำนวนเต็ม, จำนวนเต็ม);
static จำนวนเต็ม tvfsShmMap(sqlite3_file*,จำนวนเต็ม,จำนวนเต็ม,จำนวนเต็ม, void ca_ncala **);
static void tvfsShmBarrier(sqlite3_file*);
static จำนวนเต็ม tvfsShmUnmap(sqlite3_file*, จำนวนเต็ม);

static จำนวนเต็ม tvfsFetch(sqlite3_file*, sqlite3_int64, จำนวนเต็ม, void**);
static จำนวนเต็ม tvfsUnfetch(sqlite3_file*, sqlite3_int64, void*);

static sqlite3_io_methods tvfs_io_methods = {
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

static จำนวนเต็ม tvfsResultCode(Testvfs *p, จำนวนเต็ม *pRc){
  sa_nracanaa errcode {
    จำนวนเต็ม eCode;
    const var_na *zCode;
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

  const var_na *z;
  จำนวนเต็ม i;

  z = Tcl_GetStringResult(p->interp);
  สำหรับ(i=0; i<ArraySize(aCode); i++){
    ถ้า( 0==strcmp(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      คืน 1;
    }
  }

  คืน 0;
}

static จำนวนเต็ม tvfsInjectFault(TestFaultInject *p){
  จำนวนเต็ม ret = 0;
  ถ้า( p->eFault ){
    p->iCnt--;
    ถ้า( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  คืน ret;
}


static จำนวนเต็ม tvfsInjectIoerr(Testvfs *p){
  คืน tvfsInjectFault(&p->ioerr_err);
}

static จำนวนเต็ม tvfsInjectFullerr(Testvfs *p){
  คืน tvfsInjectFault(&p->full_err);
}
static จำนวนเต็ม tvfsInjectCantopenerr(Testvfs *p){
  คืน tvfsInjectFault(&p->cantopen_err);
}


static void tvfsExecTcl(
  Testvfs *p, 
  const var_na *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  จำนวนเต็ม rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  assert( p->pScript );

  assert( zMethod );
  assert( p );
  assert( arg2==0 || arg1!=0 );
  assert( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_IncrRefCount(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  ถ้า( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  ถ้า( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  ถ้า( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  ถ้า( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  ถ้า( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_ResetResult(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
static จำนวนเต็ม tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ถ้า( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  ถ้า( pFd->pShmId ){
    Tcl_DecrRefCount(pFd->pShmId);
    pFd->pShmId = 0;
  }
  ถ้า( pFile->pMethods ){
    ckfree((var_na *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((var_na *)pFd);
  pTestfile->pFd = 0;
  คืน SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
static จำนวนเต็ม tvfsRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  จำนวนเต็ม iAmt, 
  sqlite_int64 iOfst
){
  จำนวนเต็ม rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ถ้า( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  ถ้า( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  ถ้า( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  คืน rc;
}

/*
** Write data to an tvfs-file.
*/
static จำนวนเต็ม tvfsWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  จำนวนเต็ม iAmt, 
  sqlite_int64 iOfst
){
  จำนวนเต็ม rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ถ้า( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfsResultCode(p, &rc);
    ถ้า( rc<0 ) คืน SQLITE_OK;
  }

  ถ้า( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  ถ้า( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  ถ้า( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  คืน rc;
}

/*
** Truncate an tvfs-file.
*/
static จำนวนเต็ม tvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  จำนวนเต็ม rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ถ้า( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  
  ถ้า( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, size);
  }
  คืน rc;
}

/*
** Sync an tvfs-file.
*/
static จำนวนเต็ม tvfsSync(sqlite3_file *pFile, จำนวนเต็ม flags){
  จำนวนเต็ม rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ถ้า( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    var_na *zFlags = 0;

    switch( flags ){
      pak_sa SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        bha_nga;
      pak_sa SQLITE_SYNC_FULL:
        zFlags = "full";
        bha_nga;
      pak_sa SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        bha_nga;
      pak_sa SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        bha_nga;
      default:
        assert(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  ถ้า( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  ถ้า( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  คืน rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
static จำนวนเต็ม tvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  TestvfsFd *p = tvfsGetFd(pFile);
  คืน sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an tvfs-file.
*/
static จำนวนเต็ม tvfsLock(sqlite3_file *pFile, จำนวนเต็ม eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ถ้า( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    var_na zLock[30];
    sqlite3_snprintf(parimaa_na(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  ถ้า( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    คืน SQLITE_IOERR_LOCK;
  }
  คืน sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
static จำนวนเต็ม tvfsUnlock(sqlite3_file *pFile, จำนวนเต็ม eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ถ้า( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    var_na zLock[30];
    sqlite3_snprintf(parimaa_na(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  ถ้า( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    คืน SQLITE_IOERR_UNLOCK;
  }
  คืน sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
static จำนวนเต็ม tvfsCheckReservedLock(sqlite3_file *pFile, จำนวนเต็ม *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ถ้า( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  คืน sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
static จำนวนเต็ม tvfsFileControl(sqlite3_file *pFile, จำนวนเต็ม op, void *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ถ้า( op==SQLITE_FCNTL_PRAGMA ){
    var_na **argv = (var_na**)pArg;
    ถ้า( sqlite3_stricmp(argv[1],"error")==0 ){
      จำนวนเต็ม rc = SQLITE_ERROR;
      ถ้า( argv[2] ){
        const var_na *z = argv[2];
        จำนวนเต็ม x = atoi(z);
        ถ้า( x ){
          rc = x;
          ขณะ( sqlite3Isdigit(z[0]) ){ z++; }
          ขณะ( sqlite3Isspace(z[0]) ){ z++; }
        }
        ถ้า( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      คืน rc;
    }
    ถ้า( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      คืน SQLITE_OK;
    }
  }
  ถ้า( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    sa_nracanaa Fcntl {
      จำนวนเต็ม iFnctl;
      const var_na *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    จำนวนเต็ม i;
    สำหรับ(i=0; i<parimaa_na(aF)/parimaa_na(aF[0]); i++){
      ถ้า( op==aF[i].iFnctl ) bha_nga;
    }
    ถ้า( i<parimaa_na(aF)/parimaa_na(aF[0]) ){
      จำนวนเต็ม rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfsResultCode(p, &rc);
      ถ้า( rc ) คืน (rc<0 ? SQLITE_OK : rc);
    }
  }
  คืน sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
static จำนวนเต็ม tvfsSectorSize(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ถ้า( p->iSectorsize>=0 ){
    คืน p->iSectorsize;
  }
  คืน sqlite3OsSectorSize(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
static จำนวนเต็ม tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ถ้า( p->iDevchar>=0 ){
    คืน p->iDevchar;
  }
  คืน sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
static จำนวนเต็ม tvfsOpen(
  sqlite3_vfs *pVfs,
  const var_na *zName,
  sqlite3_file *pFile,
  จำนวนเต็ม flags,
  จำนวนเต็ม *pOutFlags
){
  จำนวนเต็ม rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  pFd = (TestvfsFd *)ckalloc(parimaa_na(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, parimaa_na(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, parimaa_na(TestvfsFile));
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
  ถ้า( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_IncrRefCount(pArg);
    ถ้า( flags&SQLITE_OPEN_MAIN_DB ){
      const var_na *z = &zName[strlen(zName)+1];
      ขณะ( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_DecrRefCount(pArg);
    ถ้า( tvfsResultCode(p, &rc) ){
      ถ้า( rc!=SQLITE_OK ) คืน rc;
    }มิฉะนั้น{
      pId = Tcl_GetObjResult(p->interp);
    }
  }

  ถ้า( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) คืน SQLITE_IOERR;
  ถ้า( tvfsInjectCantopenerr(p) ) คืน SQLITE_CANTOPEN;
  ถ้า( tvfsInjectFullerr(p) ) คืน SQLITE_FULL;

  ถ้า( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_IncrRefCount(pId);
  pFd->pShmId = pId;
  Tcl_ResetResult(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  ถ้า( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    จำนวนเต็ม nByte;

    ถ้า( pVfs->iVersion>1 ){
      nByte = parimaa_na(sqlite3_io_methods);
    }มิฉะนั้น{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    memcpy(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    ถ้า( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    ถ้า( pVfs->iVersion>1 && ((Testvfs *)pVfs->pAppData)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  คืน rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static จำนวนเต็ม tvfsDelete(sqlite3_vfs *pVfs, const var_na *zPath, จำนวนเต็ม dirSync){
  จำนวนเต็ม rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  ถ้า( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  ถ้า( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  คืน rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static จำนวนเต็ม tvfsAccess(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  จำนวนเต็ม flags, 
  จำนวนเต็ม *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  ถ้า( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    จำนวนเต็ม rc;
    var_na *zArg = 0;
    ถ้า( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    ถ้า( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    ถ้า( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    ถ้า( tvfsResultCode(p, &rc) ){
      ถ้า( rc!=SQLITE_OK ) คืน rc;
    }มิฉะนั้น{
      Tcl_Interp *interp = p->interp;
      ถ้า( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_GetObjResult(interp), pResOut) ){
        คืน SQLITE_OK;
      }
    }
  }
  คืน sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
static จำนวนเต็ม tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  const var_na *zPath, 
  จำนวนเต็ม nOut, 
  var_na *zOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  ถ้า( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    จำนวนเต็ม rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    ถ้า( tvfsResultCode(p, &rc) ){
      ถ้า( rc!=SQLITE_OK ) คืน rc;
    }
  }
  คืน sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *tvfsDlOpen(sqlite3_vfs *pVfs, const var_na *zPath){
  คืน sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void tvfsDlError(sqlite3_vfs *pVfs, จำนวนเต็ม nByte, var_na *zErrMsg){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*tvfsDlSym(sqlite3_vfs *pVfs, void *p, const var_na *zSym))(void){
  คืน sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
static void tvfsDlClose(sqlite3_vfs *pVfs, void *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#endif /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
static จำนวนเต็ม tvfsRandomness(sqlite3_vfs *pVfs, จำนวนเต็ม nByte, var_na *zBufOut){
  คืน sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static จำนวนเต็ม tvfsSleep(sqlite3_vfs *pVfs, จำนวนเต็ม nMicro){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  ถ้า( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  คืน sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static จำนวนเต็ม tvfsCurrentTime(sqlite3_vfs *pVfs, dviguu_na *pTimeOut){
  คืน PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

static จำนวนเต็ม tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  จำนวนเต็ม rc = SQLITE_OK;             /* Return code */
  TestvfsBuffer *pBuffer;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->pAppData;
  assert( 0==p->isFullshm );
  assert( pFd->pShmId && pFd->pShm==0 && pFd->pNext==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_ResetResult(p->interp);
  ถ้า( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    ถ้า( tvfsResultCode(p, &rc) ){
      ถ้า( rc!=SQLITE_OK ) คืน rc;
    }
  }

  assert( rc==SQLITE_OK );
  ถ้า( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    คืน SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  สำหรับ(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
    ถ้า( 0==strcmp(pFd->zFilename, pBuffer->zFile) ) bha_nga;
  }
  ถ้า( !pBuffer ){
    จำนวนเต็ม szName = (จำนวนเต็ม)strlen(pFd->zFilename);
    จำนวนเต็ม nByte = parimaa_na(TestvfsBuffer) + szName + 1;
    pBuffer = (TestvfsBuffer *)ckalloc(nByte);
    memset(pBuffer, 0, nByte);
    pBuffer->zFile = (var_na *)&pBuffer[1];
    memcpy(pBuffer->zFile, pFd->zFilename, szName+1);
    pBuffer->pNext = p->pBuffer;
    p->pBuffer = pBuffer;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->pNext = pBuffer->pFile;
  pBuffer->pFile = pFd;
  pFd->pShm = pBuffer;
  คืน rc;
}

static void tvfsAllocPage(TestvfsBuffer *p, จำนวนเต็ม iPage, จำนวนเต็ม pgsz){
  assert( iPage<TESTVFS_MAX_PAGES );
  ถ้า( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    memset(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

static จำนวนเต็ม tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  จำนวนเต็ม iPage,                      /* Page to retrieve */
  จำนวนเต็ม pgsz,                       /* Size of pages */
  จำนวนเต็ม isWrite,                    /* True to extend file if necessary */
  void ca_ncala **pp              /* OUT: Mapped memory */
){
  จำนวนเต็ม rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  ถ้า( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    คืน pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  ถ้า( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    ถ้า( rc!=SQLITE_OK ){
      คืน rc;
    }
  }

  ถ้า( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_IncrRefCount(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(iPage));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfsResultCode(p, &rc);
    Tcl_DecrRefCount(pArg);
  }
  ถ้า( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ถ้า( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  ถ้า( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (void ca_ncala *)pFd->pShm->aPage[iPage];
  }

  คืน rc;
}


static จำนวนเต็ม tvfsShmLock(
  sqlite3_file *pFile,
  จำนวนเต็ม ofst,
  จำนวนเต็ม n,
  จำนวนเต็ม flags
){
  จำนวนเต็ม rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  จำนวนเต็ม nLock;
  var_na zLock[80];

  ถ้า( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    คืน pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  ถ้า( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(parimaa_na(zLock), zLock, "%d %d", ofst, n);
    nLock = (จำนวนเต็ม)strlen(zLock);
    ถ้า( flags & SQLITE_SHM_LOCK ){
      strcpy(&zLock[nLock], " lock");
    }มิฉะนั้น{
      strcpy(&zLock[nLock], " unlock");
    }
    nLock += (จำนวนเต็ม)strlen(&zLock[nLock]);
    ถ้า( flags & SQLITE_SHM_SHARED ){
      strcpy(&zLock[nLock], " shared");
    }มิฉะนั้น{
      strcpy(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  ถ้า( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ถ้า( rc==SQLITE_OK ){
    จำนวนเต็ม isLock = (flags & SQLITE_SHM_LOCK);
    จำนวนเต็ม isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    ถ้า( isLock ){
      TestvfsFd *p2;
      สำหรับ(p2=pFd->pShm->pFile; p2; p2=p2->pNext){
        ถ้า( p2==pFd ) continue;
        ถ้า( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          bha_nga;
        }
      }
      ถ้า( rc==SQLITE_OK ){
        ถ้า( isExcl )  pFd->excllock |= mask;
        ถ้า( !isExcl ) pFd->sharedlock |= mask;
      }
    }มิฉะนั้น{
      ถ้า( isExcl )  pFd->excllock &= (~mask);
      ถ้า( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  คืน rc;
}

static void tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  ถ้า( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    const var_na *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  ถ้า( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    คืน;
  }
}

static จำนวนเต็ม tvfsShmUnmap(
  sqlite3_file *pFile,
  จำนวนเต็ม deleteFlag
){
  จำนวนเต็ม rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  TestvfsBuffer *pBuffer = pFd->pShm;
  TestvfsFd **ppFd;

  ถ้า( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    คืน pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  ถ้า( !pBuffer ) คืน SQLITE_OK;
  assert( pFd->pShmId && pFd->pShm );

  ถ้า( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }

  สำหรับ(ppFd=&pBuffer->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->pNext));
  assert( (*ppFd)==pFd );
  *ppFd = pFd->pNext;
  pFd->pNext = 0;

  ถ้า( pBuffer->pFile==0 ){
    จำนวนเต็ม i;
    TestvfsBuffer **pp;
    สำหรับ(pp=&p->pBuffer; *pp!=pBuffer; pp=&((*pp)->pNext));
    *pp = (*pp)->pNext;
    สำหรับ(i=0; pBuffer->aPage[i]; i++){
      ckfree((var_na *)pBuffer->aPage[i]);
    }
    ckfree((var_na *)pBuffer);
  }
  pFd->pShm = 0;

  คืน rc;
}

static จำนวนเต็ม tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    จำนวนเต็ม iAmt, 
    void **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  คืน sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

static จำนวนเต็ม tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, void *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  คืน sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

static จำนวนเต็ม SQLITE_TCLAPI testvfs_obj_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  ga_nanaa DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  sa_nracanaa TestvfsSubcmd {
    var_na *zName;
    ga_nanaa DB_enum eCmd;
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
  จำนวนเต็ม i;
  
  ถ้า( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUBCOMMAND ...");
    คืน TCL_ERROR;
  }
  ถ้า( Tcl_GetIndexFromObjStruct(
        interp, objv[1], aSubcmd, parimaa_na(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    คืน TCL_ERROR;
  }
  Tcl_ResetResult(interp);

  switch( aSubcmd[i].eCmd ){
    pak_sa CMD_SHM: {
      Tcl_Obj *pObj;
      จำนวนเต็ม rc;
      TestvfsBuffer *pBuffer;
      var_na *zName;
      ถ้า( objc!=3 && objc!=4 ){
        Tcl_WrongNumArgs(interp, 2, objv, "FILE ?VALUE?");
        คืน TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      ถ้า( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), NULL);
        ckfree(zName);
        คืน TCL_ERROR;
      }
      สำหรับ(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
        ถ้า( 0==strcmp(pBuffer->zFile, zName) ) bha_nga;
      }
      ckfree(zName);
      ถ้า( !pBuffer ){
        Tcl_AppendResult(interp, "no such file: ", Tcl_GetString(objv[2]), NULL);
        คืน TCL_ERROR;
      }
      ถ้า( objc==4 ){
        Tcl_Size n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        จำนวนเต็ม pgsz = pBuffer->pgsz;
        ถ้า( pgsz==0 ) pgsz = 65536;
        สำหรับ(i=0; i*pgsz<(จำนวนเต็ม)n; i++){
          จำนวนเต็ม nByte = pgsz;
          tvfsAllocPage(pBuffer, i, pgsz);
          ถ้า( n-i*pgsz<pgsz ){
            nByte = (จำนวนเต็ม)n;
          }
          memcpy(pBuffer->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      สำหรับ(i=0; pBuffer->aPage[i]; i++){
        จำนวนเต็ม pgsz = pBuffer->pgsz;
        ถ้า( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(pBuffer->aPage[i], pgsz));
      }
      Tcl_SetObjResult(interp, pObj);
      bha_nga;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    pak_sa CMD_FILTER: {
      static sa_nracanaa VfsMethod {
        var_na *zName;
        จำนวนเต็ม mask;
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
      จำนวนเต็ม mask = 0;
      ถ้า( objc!=3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "LIST");
        คืน TCL_ERROR;
      }
      ถ้า( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        คืน TCL_ERROR;
      }
      Tcl_ResetResult(interp);
      สำหรับ(i=0; i<(จำนวนเต็ม)nElem; i++){
        จำนวนเต็ม iMethod;
        var_na *zElem = Tcl_GetString(apElem[i]);
        สำหรับ(iMethod=0; iMethod<ArraySize(vfsmethod); iMethod++){
          ถ้า( strcmp(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            bha_nga;
          }
        }
        ถ้า( iMethod==ArraySize(vfsmethod) ){
          Tcl_AppendResult(interp, "unknown method: ", zElem, NULL);
          คืน TCL_ERROR;
        }
      }
      p->mask = mask;
      bha_nga;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    pak_sa CMD_SCRIPT: {
      ถ้า( objc==3 ){
        Tcl_Size nByte;
        ถ้า( p->pScript ){
          Tcl_DecrRefCount(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        ถ้า( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_IncrRefCount(p->pScript);
        }
      }มิฉะนั้น ถ้า( objc!=2 ){
        Tcl_WrongNumArgs(interp, 2, objv, "?SCRIPT?");
        คืน TCL_ERROR;
      }

      Tcl_ResetResult(interp);
      ถ้า( p->pScript ) Tcl_SetObjResult(interp, p->pScript);

      bha_nga;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    pak_sa CMD_CANTOPENERR:
    pak_sa CMD_IOERR:
    pak_sa CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      จำนวนเต็ม iRet;

      switch( aSubcmd[i].eCmd ){
        pak_sa CMD_IOERR: pTest = &p->ioerr_err; bha_nga;
        pak_sa CMD_FULLERR: pTest = &p->full_err; bha_nga;
        pak_sa CMD_CANTOPENERR: pTest = &p->cantopen_err; bha_nga;
        default: assert(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      ถ้า( objc==4 ){
        จำนวนเต็ม iCnt, iPersist;
        ถ้า( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          คืน TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }มิฉะนั้น ถ้า( objc!=2 ){
        Tcl_WrongNumArgs(interp, 2, objv, "?CNT PERSIST?");
        คืน TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(iRet));
      bha_nga;
    }

    pak_sa CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      bha_nga;
    }

    pak_sa CMD_DEVCHAR: {
      sa_nracanaa DeviceFlag {
        var_na *zName;
        จำนวนเต็ม iValue;
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
      จำนวนเต็ม iFlag;

      ถ้า( objc>3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "?ATTR-LIST?");
        คืน TCL_ERROR;
      }
      ถ้า( objc==3 ){
        จำนวนเต็ม j;
        จำนวนเต็ม iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_Size nFlags = 0;

        ถ้า( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          คืน TCL_ERROR;
        }

        สำหรับ(j=0; j<(จำนวนเต็ม)nFlags; j++){
          จำนวนเต็ม idx = 0;
          ถ้า( Tcl_GetIndexFromObjStruct(interp, flags[j], aFlag, 
                parimaa_na(aFlag[0]), "flag", 0, &idx) 
          ){
            คืน TCL_ERROR;
          }
          ถ้า( aFlag[idx].iValue<0 && nFlags>1 ){
            Tcl_AppendResult(interp, "bad flags: ", Tcl_GetString(objv[2]), NULL);
            คืน TCL_ERROR;
          }
          iNew |= aFlag[idx].iValue;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      สำหรับ(iFlag=0; iFlag<parimaa_na(aFlag)/parimaa_na(aFlag[0]); iFlag++){
        ถ้า( p->iDevchar & aFlag[iFlag].iValue ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_SetObjResult(interp, pRet);

      bha_nga;
    }

    pak_sa CMD_SECTORSIZE: {
      ถ้า( objc>3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "?VALUE?");
        คืน TCL_ERROR;
      }
      ถ้า( objc==3 ){
        จำนวนเต็ม iNew = 0;
        ถ้า( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          คืน TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(p->iSectorsize));
      bha_nga;
    }
  }

  คืน TCL_OK;
}

static void SQLITE_TCLAPI testvfs_obj_del(ClientData cd){
  Testvfs *p = (Testvfs *)cd;
  ถ้า( p->pScript ) Tcl_DecrRefCount(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, parimaa_na(sqlite3_vfs));
  ckfree((var_na *)p->pVfs);
  memset(p, 0, parimaa_na(Testvfs));
  ckfree((var_na *)p);
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
static จำนวนเต็ม SQLITE_TCLAPI testvfs_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  static sqlite3_vfs tvfs_vfs = {
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
#มิฉะนั้น
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
  var_na *zVfs;
  จำนวนเต็ม nByte;                      /* Bytes of space to allocate at p */

  จำนวนเต็ม i;
  จำนวนเต็ม isNoshm = 0;                /* True if -noshm is passed */
  จำนวนเต็ม isFullshm = 0;              /* True if -fullshm is passed */
  จำนวนเต็ม isDefault = 0;              /* True if -default is passed */
  จำนวนเต็ม szOsFile = 0;               /* Value passed to -szosfile */
  จำนวนเต็ม mxPathname = -1;            /* Value passed to -mxpathname */
  จำนวนเต็ม iVersion = 3;               /* Value passed to -iversion */

  ถ้า( objc<2 || 0!=(objc%2) ) goto bad_args;
  สำหรับ(i=2; i<objc; i += 2){
    Tcl_Size nSwitch;
    var_na *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    ถ้า( nSwitch>2 && 0==strncmp("-noshm", zSwitch, nSwitch) ){
      ถ้า( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        คืน TCL_ERROR;
      }
      ถ้า( isNoshm ) isFullshm = 0;
    }
    มิฉะนั้น ถ้า( nSwitch>2 && 0==strncmp("-default", zSwitch, nSwitch) ){
      ถ้า( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        คืน TCL_ERROR;
      }
    }
    มิฉะนั้น ถ้า( nSwitch>2 && 0==strncmp("-szosfile", zSwitch, nSwitch) ){
      ถ้า( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        คืน TCL_ERROR;
      }
    }
    มิฉะนั้น ถ้า( nSwitch>2 && 0==strncmp("-mxpathname", zSwitch, nSwitch) ){
      ถ้า( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        คืน TCL_ERROR;
      }
    }
    มิฉะนั้น ถ้า( nSwitch>2 && 0==strncmp("-iversion", zSwitch, nSwitch) ){
      ถ้า( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        คืน TCL_ERROR;
      }
    }
    มิฉะนั้น ถ้า( nSwitch>2 && 0==strncmp("-fullshm", zSwitch, nSwitch) ){
      ถ้า( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        คืน TCL_ERROR;
      }
      ถ้า( isFullshm ) isNoshm = 0;
    }
    มิฉะนั้น{
      goto bad_args;
    }
  }

  ถ้า( szOsFile<parimaa_na(TestvfsFile) ){
    szOsFile = parimaa_na(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = parimaa_na(Testvfs) + (จำนวนเต็ม)strlen(zVfs)+1;
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

  p->zName = (var_na *)&p[1];
  memcpy(p->zName, zVfs, strlen(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(parimaa_na(sqlite3_vfs));
  memcpy(pVfs, &tvfs_vfs, parimaa_na(sqlite3_vfs));
  pVfs->pAppData = (void *)p;
  pVfs->iVersion = iVersion;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  ถ้า( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  คืน TCL_OK;

 bad_args:
  Tcl_WrongNumArgs(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  คืน TCL_ERROR;
}

extern จำนวนเต็ม getDbPointer(Tcl_Interp *interp, const var_na *zA, sqlite3 **ppDb);
extern const var_na *sqlite3ErrName(จำนวนเต็ม);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
static จำนวนเต็ม SQLITE_TCLAPI test_vfs_shmlock(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  const var_na *azArg1[] = {"shared", "exclusive", 0};
  const var_na *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  จำนวนเต็ม rc = SQLITE_OK;
  const var_na *zDbname = 0;
  จำนวนเต็ม iArg1 = 0;
  จำนวนเต็ม iArg2 = 0;
  จำนวนเต็ม iOffset = 0;
  จำนวนเต็ม n = 0;
  sqlite3_file *pFd;

  ถ้า( objc!=7 ){
    Tcl_WrongNumArgs(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    คืน TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ถ้า( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIndexFromObj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_GetIndexFromObj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    คืน TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (void*)&pFd);
  ถ้า( pFd==0 ){
    คืน TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  คืน TCL_OK;
}

static จำนวนเต็ม SQLITE_TCLAPI test_vfs_set_readmark(
  void * clientData,
  Tcl_Interp *interp,
  จำนวนเต็ม objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  จำนวนเต็ม rc = SQLITE_OK;
  const var_na *zDbname = 0;
  จำนวนเต็ม iSlot = 0;
  จำนวนเต็ม iVal = -1;
  sqlite3_file *pFd;
  void ca_ncala *pShm = 0;
  u32 *aShm;
  จำนวนเต็ม iOff;

  ถ้า( objc!=4 && objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    คืน TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ถ้า( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    คืน TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (void*)&pFd);
  ถ้า( pFd==0 ){
    คืน TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  ถ้า( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    คืน TCL_ERROR;
  }
  ถ้า( pShm==0 ){
    Tcl_AppendResult(interp, "*-shm is not yet mapped", NULL);
    คืน TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  ถ้า( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(aShm[iOff]));

  คืน TCL_OK;
}

จำนวนเต็ม Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  คืน TCL_OK;
}

#endif
