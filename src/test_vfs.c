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
#ако SQLITE_TEST          /* This file is used for testing only */

#include "sqlite3.h"
#include "sqliteInt.h"
#include "tclsqlite.h"

typedef struct Testvfs Testvfs;
typedef struct TestvfsShm TestvfsShm;
typedef struct TestvfsBuffer TestvfsBuffer;
typedef struct TestvfsFile TestvfsFile;
typedef struct TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
struct TestvfsFile {
  sqlite3_file base;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#define tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

struct TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  const char *zFilename;          /* Filename as passed to xOpen() */
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

typedef struct TestFaultInject TestFaultInject;
struct TestFaultInject {
  цяло iCnt;                       /* Remaining calls before fault injection */
  цяло eFault;                     /* A FAULT_INJECT_* value */
  цяло nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
struct Testvfs {
  char *zName;                    /* Name of this VFS */
  sqlite3_vfs *pParent;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  TestvfsBuffer *pBuffer;         /* List of shared buffers */
  цяло isNoshm;
  цяло isFullshm;

  цяло mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#ако 0
  цяло iIoerrCnt;
  цяло ioerr;
  цяло nIoerrFail;
  цяло iFullCnt;
  цяло fullerr;
  цяло nFullFail;
#endif

  цяло iDevchar;
  цяло iSectorsize;
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
struct TestvfsBuffer {
  char *zFile;                    /* Associated file name */
  цяло pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  TestvfsBuffer *pNext;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->pAppData))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
static цяло tvfsClose(sqlite3_file*);
static цяло tvfsRead(sqlite3_file*, void*, цяло iAmt, sqlite3_int64 iOfst);
static цяло tvfsWrite(sqlite3_file*,const void*,цяло iAmt, sqlite3_int64 iOfst);
static цяло tvfsTruncate(sqlite3_file*, sqlite3_int64 size);
static цяло tvfsSync(sqlite3_file*, цяло flags);
static цяло tvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static цяло tvfsLock(sqlite3_file*, цяло);
static цяло tvfsUnlock(sqlite3_file*, цяло);
static цяло tvfsCheckReservedLock(sqlite3_file*, цяло *);
static цяло tvfsFileControl(sqlite3_file*, цяло op, void *pArg);
static цяло tvfsSectorSize(sqlite3_file*);
static цяло tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
static цяло tvfsOpen(sqlite3_vfs*, const char *, sqlite3_file*, цяло , цяло *);
static цяло tvfsDelete(sqlite3_vfs*, const char *zName, цяло syncDir);
static цяло tvfsAccess(sqlite3_vfs*, const char *zName, цяло flags, цяло *);
static цяло tvfsFullPathname(sqlite3_vfs*, const char *zName, цяло, char *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
static void *tvfsDlOpen(sqlite3_vfs*, const char *zFilename);
static void tvfsDlError(sqlite3_vfs*, цяло nByte, char *zErrMsg);
static void (*tvfsDlSym(sqlite3_vfs*,void*, const char *zSymbol))(void);
static void tvfsDlClose(sqlite3_vfs*, void*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
static цяло tvfsRandomness(sqlite3_vfs*, цяло nByte, char *zOut);
static цяло tvfsSleep(sqlite3_vfs*, цяло microseconds);
static цяло tvfsCurrentTime(sqlite3_vfs*, double*);

static цяло tvfsShmOpen(sqlite3_file*);
static цяло tvfsShmLock(sqlite3_file*, цяло , цяло, цяло);
static цяло tvfsShmMap(sqlite3_file*,цяло,цяло,цяло, void volatile **);
static void tvfsShmBarrier(sqlite3_file*);
static цяло tvfsShmUnmap(sqlite3_file*, цяло);

static цяло tvfsFetch(sqlite3_file*, sqlite3_int64, цяло, void**);
static цяло tvfsUnfetch(sqlite3_file*, sqlite3_int64, void*);

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

static цяло tvfsResultCode(Testvfs *p, цяло *pRc){
  struct errcode {
    цяло eCode;
    const char *zCode;
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

  const char *z;
  цяло i;

  z = Tcl_GetStringResult(p->interp);
  за(i=0; i<ArraySize(aCode); i++){
    ако( 0==strcmp(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      върни 1;
    }
  }

  върни 0;
}

static цяло tvfsInjectFault(TestFaultInject *p){
  цяло ret = 0;
  ако( p->eFault ){
    p->iCnt--;
    ако( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  върни ret;
}


static цяло tvfsInjectIoerr(Testvfs *p){
  върни tvfsInjectFault(&p->ioerr_err);
}

static цяло tvfsInjectFullerr(Testvfs *p){
  върни tvfsInjectFault(&p->full_err);
}
static цяло tvfsInjectCantopenerr(Testvfs *p){
  върни tvfsInjectFault(&p->cantopen_err);
}


static void tvfsExecTcl(
  Testvfs *p, 
  const char *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  цяло rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  assert( p->pScript );

  assert( zMethod );
  assert( p );
  assert( arg2==0 || arg1!=0 );
  assert( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_IncrRefCount(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  ако( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  ако( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  ако( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  ако( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  ако( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_ResetResult(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
static цяло tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ако( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  ако( pFd->pShmId ){
    Tcl_DecrRefCount(pFd->pShmId);
    pFd->pShmId = 0;
  }
  ако( pFile->pMethods ){
    ckfree((char *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((char *)pFd);
  pTestfile->pFd = 0;
  върни SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
static цяло tvfsRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  цяло iAmt, 
  sqlite_int64 iOfst
){
  цяло rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ако( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  ако( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  ако( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  върни rc;
}

/*
** Write data to an tvfs-file.
*/
static цяло tvfsWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  цяло iAmt, 
  sqlite_int64 iOfst
){
  цяло rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ако( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfsResultCode(p, &rc);
    ако( rc<0 ) върни SQLITE_OK;
  }

  ако( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  ако( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  ако( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  върни rc;
}

/*
** Truncate an tvfs-file.
*/
static цяло tvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  цяло rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ако( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  
  ако( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, size);
  }
  върни rc;
}

/*
** Sync an tvfs-file.
*/
static цяло tvfsSync(sqlite3_file *pFile, цяло flags){
  цяло rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  ако( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
    char *zFlags = 0;

    switch( flags ){
      case SQLITE_SYNC_NORMAL:
        zFlags = "normal";
        break;
      case SQLITE_SYNC_FULL:
        zFlags = "full";
        break;
      case SQLITE_SYNC_NORMAL|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        break;
      case SQLITE_SYNC_FULL|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        break;
      default:
        assert(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  ако( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  ако( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  върни rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
static цяло tvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  TestvfsFd *p = tvfsGetFd(pFile);
  върни sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an tvfs-file.
*/
static цяло tvfsLock(sqlite3_file *pFile, цяло eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ако( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    char zLock[30];
    sqlite3_snprintf(sizeof(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  ако( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    върни SQLITE_IOERR_LOCK;
  }
  върни sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
static цяло tvfsUnlock(sqlite3_file *pFile, цяло eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ако( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    char zLock[30];
    sqlite3_snprintf(sizeof(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  ако( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    върни SQLITE_IOERR_UNLOCK;
  }
  върни sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
static цяло tvfsCheckReservedLock(sqlite3_file *pFile, цяло *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ако( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  върни sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
static цяло tvfsFileControl(sqlite3_file *pFile, цяло op, void *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ако( op==SQLITE_FCNTL_PRAGMA ){
    char **argv = (char**)pArg;
    ако( sqlite3_stricmp(argv[1],"error")==0 ){
      цяло rc = SQLITE_ERROR;
      ако( argv[2] ){
        const char *z = argv[2];
        цяло x = atoi(z);
        ако( x ){
          rc = x;
          докато( sqlite3Isdigit(z[0]) ){ z++; }
          докато( sqlite3Isspace(z[0]) ){ z++; }
        }
        ако( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      върни rc;
    }
    ако( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      върни SQLITE_OK;
    }
  }
  ако( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    struct Fcntl {
      цяло iFnctl;
      const char *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    цяло i;
    за(i=0; i<sizeof(aF)/sizeof(aF[0]); i++){
      ако( op==aF[i].iFnctl ) break;
    }
    ако( i<sizeof(aF)/sizeof(aF[0]) ){
      цяло rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfsResultCode(p, &rc);
      ако( rc ) върни (rc<0 ? SQLITE_OK : rc);
    }
  }
  върни sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
static цяло tvfsSectorSize(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ако( p->iSectorsize>=0 ){
    върни p->iSectorsize;
  }
  върни sqlite3OsSectorSize(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
static цяло tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  ако( p->iDevchar>=0 ){
    върни p->iDevchar;
  }
  върни sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
static цяло tvfsOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  цяло flags,
  цяло *pOutFlags
){
  цяло rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  pFd = (TestvfsFd *)ckalloc(sizeof(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, sizeof(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, sizeof(TestvfsFile));
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
  ако( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_IncrRefCount(pArg);
    ако( flags&SQLITE_OPEN_MAIN_DB ){
      const char *z = &zName[strlen(zName)+1];
      докато( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_DecrRefCount(pArg);
    ако( tvfsResultCode(p, &rc) ){
      ако( rc!=SQLITE_OK ) върни rc;
    }иначе{
      pId = Tcl_GetObjResult(p->interp);
    }
  }

  ако( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) върни SQLITE_IOERR;
  ако( tvfsInjectCantopenerr(p) ) върни SQLITE_CANTOPEN;
  ако( tvfsInjectFullerr(p) ) върни SQLITE_FULL;

  ако( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_IncrRefCount(pId);
  pFd->pShmId = pId;
  Tcl_ResetResult(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  ако( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    цяло nByte;

    ако( pVfs->iVersion>1 ){
      nByte = sizeof(sqlite3_io_methods);
    }иначе{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    memcpy(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    ако( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    ако( pVfs->iVersion>1 && ((Testvfs *)pVfs->pAppData)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  върни rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static цяло tvfsDelete(sqlite3_vfs *pVfs, const char *zPath, цяло dirSync){
  цяло rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  ако( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  ако( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  върни rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static цяло tvfsAccess(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  цяло flags, 
  цяло *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  ако( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    цяло rc;
    char *zArg = 0;
    ако( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    ако( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    ако( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    ако( tvfsResultCode(p, &rc) ){
      ако( rc!=SQLITE_OK ) върни rc;
    }иначе{
      Tcl_Interp *interp = p->interp;
      ако( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_GetObjResult(interp), pResOut) ){
        върни SQLITE_OK;
      }
    }
  }
  върни sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
static цяло tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  цяло nOut, 
  char *zOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  ако( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    цяло rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    ако( tvfsResultCode(p, &rc) ){
      ако( rc!=SQLITE_OK ) върни rc;
    }
  }
  върни sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *tvfsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  върни sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void tvfsDlError(sqlite3_vfs *pVfs, цяло nByte, char *zErrMsg){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*tvfsDlSym(sqlite3_vfs *pVfs, void *p, const char *zSym))(void){
  върни sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
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
static цяло tvfsRandomness(sqlite3_vfs *pVfs, цяло nByte, char *zBufOut){
  върни sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static цяло tvfsSleep(sqlite3_vfs *pVfs, цяло nMicro){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  ако( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  върни sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static цяло tvfsCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  върни PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

static цяло tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  цяло rc = SQLITE_OK;             /* Return code */
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
  ако( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    ако( tvfsResultCode(p, &rc) ){
      ако( rc!=SQLITE_OK ) върни rc;
    }
  }

  assert( rc==SQLITE_OK );
  ако( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    върни SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  за(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
    ако( 0==strcmp(pFd->zFilename, pBuffer->zFile) ) break;
  }
  ако( !pBuffer ){
    цяло szName = (цяло)strlen(pFd->zFilename);
    цяло nByte = sizeof(TestvfsBuffer) + szName + 1;
    pBuffer = (TestvfsBuffer *)ckalloc(nByte);
    memset(pBuffer, 0, nByte);
    pBuffer->zFile = (char *)&pBuffer[1];
    memcpy(pBuffer->zFile, pFd->zFilename, szName+1);
    pBuffer->pNext = p->pBuffer;
    p->pBuffer = pBuffer;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->pNext = pBuffer->pFile;
  pBuffer->pFile = pFd;
  pFd->pShm = pBuffer;
  върни rc;
}

static void tvfsAllocPage(TestvfsBuffer *p, цяло iPage, цяло pgsz){
  assert( iPage<TESTVFS_MAX_PAGES );
  ако( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    memset(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

static цяло tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  цяло iPage,                      /* Page to retrieve */
  цяло pgsz,                       /* Size of pages */
  цяло isWrite,                    /* True to extend file if necessary */
  void volatile **pp              /* OUT: Mapped memory */
){
  цяло rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  ако( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    върни pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  ако( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    ако( rc!=SQLITE_OK ){
      върни rc;
    }
  }

  ако( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
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
  ако( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ако( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  ако( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (void volatile *)pFd->pShm->aPage[iPage];
  }

  върни rc;
}


static цяло tvfsShmLock(
  sqlite3_file *pFile,
  цяло ofst,
  цяло n,
  цяло flags
){
  цяло rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  цяло nLock;
  char zLock[80];

  ако( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    върни pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  ако( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(sizeof(zLock), zLock, "%d %d", ofst, n);
    nLock = (цяло)strlen(zLock);
    ако( flags & SQLITE_SHM_LOCK ){
      strcpy(&zLock[nLock], " lock");
    }иначе{
      strcpy(&zLock[nLock], " unlock");
    }
    nLock += (цяло)strlen(&zLock[nLock]);
    ако( flags & SQLITE_SHM_SHARED ){
      strcpy(&zLock[nLock], " shared");
    }иначе{
      strcpy(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  ако( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ако( rc==SQLITE_OK ){
    цяло isLock = (flags & SQLITE_SHM_LOCK);
    цяло isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    ако( isLock ){
      TestvfsFd *p2;
      за(p2=pFd->pShm->pFile; p2; p2=p2->pNext){
        ако( p2==pFd ) continue;
        ако( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          break;
        }
      }
      ако( rc==SQLITE_OK ){
        ако( isExcl )  pFd->excllock |= mask;
        ако( !isExcl ) pFd->sharedlock |= mask;
      }
    }иначе{
      ако( isExcl )  pFd->excllock &= (~mask);
      ако( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  върни rc;
}

static void tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  ако( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    const char *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  ако( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    върни;
  }
}

static цяло tvfsShmUnmap(
  sqlite3_file *pFile,
  цяло deleteFlag
){
  цяло rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  TestvfsBuffer *pBuffer = pFd->pShm;
  TestvfsFd **ppFd;

  ако( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    върни pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  ако( !pBuffer ) върни SQLITE_OK;
  assert( pFd->pShmId && pFd->pShm );

  ако( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }

  за(ppFd=&pBuffer->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->pNext));
  assert( (*ppFd)==pFd );
  *ppFd = pFd->pNext;
  pFd->pNext = 0;

  ако( pBuffer->pFile==0 ){
    цяло i;
    TestvfsBuffer **pp;
    за(pp=&p->pBuffer; *pp!=pBuffer; pp=&((*pp)->pNext));
    *pp = (*pp)->pNext;
    за(i=0; pBuffer->aPage[i]; i++){
      ckfree((char *)pBuffer->aPage[i]);
    }
    ckfree((char *)pBuffer);
  }
  pFd->pShm = 0;

  върни rc;
}

static цяло tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    цяло iAmt, 
    void **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  върни sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

static цяло tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, void *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  върни sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

static цяло SQLITE_TCLAPI testvfs_obj_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  enum DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  struct TestvfsSubcmd {
    char *zName;
    enum DB_enum eCmd;
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
  цяло i;
  
  ако( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUBCOMMAND ...");
    върни TCL_ERROR;
  }
  ако( Tcl_GetIndexFromObjStruct(
        interp, objv[1], aSubcmd, sizeof(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    върни TCL_ERROR;
  }
  Tcl_ResetResult(interp);

  switch( aSubcmd[i].eCmd ){
    case CMD_SHM: {
      Tcl_Obj *pObj;
      цяло rc;
      TestvfsBuffer *pBuffer;
      char *zName;
      ако( objc!=3 && objc!=4 ){
        Tcl_WrongNumArgs(interp, 2, objv, "FILE ?VALUE?");
        върни TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      ако( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), NULL);
        ckfree(zName);
        върни TCL_ERROR;
      }
      за(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
        ако( 0==strcmp(pBuffer->zFile, zName) ) break;
      }
      ckfree(zName);
      ако( !pBuffer ){
        Tcl_AppendResult(interp, "no such file: ", Tcl_GetString(objv[2]), NULL);
        върни TCL_ERROR;
      }
      ако( objc==4 ){
        Tcl_Size n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        цяло pgsz = pBuffer->pgsz;
        ако( pgsz==0 ) pgsz = 65536;
        за(i=0; i*pgsz<(цяло)n; i++){
          цяло nByte = pgsz;
          tvfsAllocPage(pBuffer, i, pgsz);
          ако( n-i*pgsz<pgsz ){
            nByte = (цяло)n;
          }
          memcpy(pBuffer->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      за(i=0; pBuffer->aPage[i]; i++){
        цяло pgsz = pBuffer->pgsz;
        ако( pgsz==0 ) pgsz = 65536;
        Tcl_AppendObjToObj(pObj, Tcl_NewByteArrayObj(pBuffer->aPage[i], pgsz));
      }
      Tcl_SetObjResult(interp, pObj);
      break;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    case CMD_FILTER: {
      static struct VfsMethod {
        char *zName;
        цяло mask;
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
      цяло mask = 0;
      ако( objc!=3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "LIST");
        върни TCL_ERROR;
      }
      ако( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        върни TCL_ERROR;
      }
      Tcl_ResetResult(interp);
      за(i=0; i<(цяло)nElem; i++){
        цяло iMethod;
        char *zElem = Tcl_GetString(apElem[i]);
        за(iMethod=0; iMethod<ArraySize(vfsmethod); iMethod++){
          ако( strcmp(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            break;
          }
        }
        ако( iMethod==ArraySize(vfsmethod) ){
          Tcl_AppendResult(interp, "unknown method: ", zElem, NULL);
          върни TCL_ERROR;
        }
      }
      p->mask = mask;
      break;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    case CMD_SCRIPT: {
      ако( objc==3 ){
        Tcl_Size nByte;
        ако( p->pScript ){
          Tcl_DecrRefCount(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        ако( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_IncrRefCount(p->pScript);
        }
      }иначе ако( objc!=2 ){
        Tcl_WrongNumArgs(interp, 2, objv, "?SCRIPT?");
        върни TCL_ERROR;
      }

      Tcl_ResetResult(interp);
      ако( p->pScript ) Tcl_SetObjResult(interp, p->pScript);

      break;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    case CMD_CANTOPENERR:
    case CMD_IOERR:
    case CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      цяло iRet;

      switch( aSubcmd[i].eCmd ){
        case CMD_IOERR: pTest = &p->ioerr_err; break;
        case CMD_FULLERR: pTest = &p->full_err; break;
        case CMD_CANTOPENERR: pTest = &p->cantopen_err; break;
        default: assert(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      ако( objc==4 ){
        цяло iCnt, iPersist;
        ако( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          върни TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }иначе ако( objc!=2 ){
        Tcl_WrongNumArgs(interp, 2, objv, "?CNT PERSIST?");
        върни TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(iRet));
      break;
    }

    case CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      break;
    }

    case CMD_DEVCHAR: {
      struct DeviceFlag {
        char *zName;
        цяло iValue;
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
      цяло iFlag;

      ако( objc>3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "?ATTR-LIST?");
        върни TCL_ERROR;
      }
      ако( objc==3 ){
        цяло j;
        цяло iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_Size nFlags = 0;

        ако( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          върни TCL_ERROR;
        }

        за(j=0; j<(цяло)nFlags; j++){
          цяло idx = 0;
          ако( Tcl_GetIndexFromObjStruct(interp, flags[j], aFlag, 
                sizeof(aFlag[0]), "flag", 0, &idx) 
          ){
            върни TCL_ERROR;
          }
          ако( aFlag[idx].iValue<0 && nFlags>1 ){
            Tcl_AppendResult(interp, "bad flags: ", Tcl_GetString(objv[2]), NULL);
            върни TCL_ERROR;
          }
          iNew |= aFlag[idx].iValue;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      за(iFlag=0; iFlag<sizeof(aFlag)/sizeof(aFlag[0]); iFlag++){
        ако( p->iDevchar & aFlag[iFlag].iValue ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_SetObjResult(interp, pRet);

      break;
    }

    case CMD_SECTORSIZE: {
      ако( objc>3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "?VALUE?");
        върни TCL_ERROR;
      }
      ако( objc==3 ){
        цяло iNew = 0;
        ако( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          върни TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(p->iSectorsize));
      break;
    }
  }

  върни TCL_OK;
}

static void SQLITE_TCLAPI testvfs_obj_del(ClientData cd){
  Testvfs *p = (Testvfs *)cd;
  ако( p->pScript ) Tcl_DecrRefCount(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, sizeof(sqlite3_vfs));
  ckfree((char *)p->pVfs);
  memset(p, 0, sizeof(Testvfs));
  ckfree((char *)p);
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
static цяло SQLITE_TCLAPI testvfs_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  цяло objc,
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
#иначе
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
  char *zVfs;
  цяло nByte;                      /* Bytes of space to allocate at p */

  цяло i;
  цяло isNoshm = 0;                /* True if -noshm is passed */
  цяло isFullshm = 0;              /* True if -fullshm is passed */
  цяло isDefault = 0;              /* True if -default is passed */
  цяло szOsFile = 0;               /* Value passed to -szosfile */
  цяло mxPathname = -1;            /* Value passed to -mxpathname */
  цяло iVersion = 3;               /* Value passed to -iversion */

  ако( objc<2 || 0!=(objc%2) ) goto bad_args;
  за(i=2; i<objc; i += 2){
    Tcl_Size nSwitch;
    char *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    ако( nSwitch>2 && 0==strncmp("-noshm", zSwitch, nSwitch) ){
      ако( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        върни TCL_ERROR;
      }
      ако( isNoshm ) isFullshm = 0;
    }
    иначе ако( nSwitch>2 && 0==strncmp("-default", zSwitch, nSwitch) ){
      ако( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        върни TCL_ERROR;
      }
    }
    иначе ако( nSwitch>2 && 0==strncmp("-szosfile", zSwitch, nSwitch) ){
      ако( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        върни TCL_ERROR;
      }
    }
    иначе ако( nSwitch>2 && 0==strncmp("-mxpathname", zSwitch, nSwitch) ){
      ако( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        върни TCL_ERROR;
      }
    }
    иначе ако( nSwitch>2 && 0==strncmp("-iversion", zSwitch, nSwitch) ){
      ако( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        върни TCL_ERROR;
      }
    }
    иначе ако( nSwitch>2 && 0==strncmp("-fullshm", zSwitch, nSwitch) ){
      ако( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        върни TCL_ERROR;
      }
      ако( isFullshm ) isNoshm = 0;
    }
    иначе{
      goto bad_args;
    }
  }

  ако( szOsFile<sizeof(TestvfsFile) ){
    szOsFile = sizeof(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = sizeof(Testvfs) + (цяло)strlen(zVfs)+1;
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

  p->zName = (char *)&p[1];
  memcpy(p->zName, zVfs, strlen(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(sizeof(sqlite3_vfs));
  memcpy(pVfs, &tvfs_vfs, sizeof(sqlite3_vfs));
  pVfs->pAppData = (void *)p;
  pVfs->iVersion = iVersion;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->pParent->mxPathname;
  ако( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  върни TCL_OK;

 bad_args:
  Tcl_WrongNumArgs(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  върни TCL_ERROR;
}

extern цяло getDbPointer(Tcl_Interp *interp, const char *zA, sqlite3 **ppDb);
extern const char *sqlite3ErrName(цяло);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
static цяло SQLITE_TCLAPI test_vfs_shmlock(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  const char *azArg1[] = {"shared", "exclusive", 0};
  const char *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  цяло rc = SQLITE_OK;
  const char *zDbname = 0;
  цяло iArg1 = 0;
  цяло iArg2 = 0;
  цяло iOffset = 0;
  цяло n = 0;
  sqlite3_file *pFd;

  ако( objc!=7 ){
    Tcl_WrongNumArgs(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    върни TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ако( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIndexFromObj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_GetIndexFromObj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    върни TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (void*)&pFd);
  ако( pFd==0 ){
    върни TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  върни TCL_OK;
}

static цяло SQLITE_TCLAPI test_vfs_set_readmark(
  void * clientData,
  Tcl_Interp *interp,
  цяло objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  цяло rc = SQLITE_OK;
  const char *zDbname = 0;
  цяло iSlot = 0;
  цяло iVal = -1;
  sqlite3_file *pFd;
  void volatile *pShm = 0;
  u32 *aShm;
  цяло iOff;

  ако( objc!=4 && objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    върни TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ако( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    върни TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (void*)&pFd);
  ако( pFd==0 ){
    върни TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  ако( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    върни TCL_ERROR;
  }
  ако( pShm==0 ){
    Tcl_AppendResult(interp, "*-shm is not yet mapped", NULL);
    върни TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  ако( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(aShm[iOff]));

  върни TCL_OK;
}

цяло Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  върни TCL_OK;
}

#endif
