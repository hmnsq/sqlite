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
#nếu SQLITE_TEST          /* This file is used for testing only */

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
  số_nguyên iCnt;                       /* Remaining calls before fault injection */
  số_nguyên eFault;                     /* A FAULT_INJECT_* value */
  số_nguyên nFail;                      /* Number of faults injected */
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
  số_nguyên isNoshm;
  số_nguyên isFullshm;

  số_nguyên mask;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject full_err;
  TestFaultInject cantopen_err;

#nếu 0
  số_nguyên iIoerrCnt;
  số_nguyên ioerr;
  số_nguyên nIoerrFail;
  số_nguyên iFullCnt;
  số_nguyên fullerr;
  số_nguyên nFullFail;
#endif

  số_nguyên iDevchar;
  số_nguyên iSectorsize;
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
  số_nguyên pgsz;                       /* Page size */
  u8 *aPage[TESTVFS_MAX_PAGES];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  TestvfsBuffer *pNext;           /* Next in linked list of all buffers */
};


#define PARENTVFS(x) (((Testvfs *)((x)->pAppData))->pParent)

#define TESTVFS_MAX_ARGS 12


/*
** Method declarations for TestvfsFile.
*/
static số_nguyên tvfsClose(sqlite3_file*);
static số_nguyên tvfsRead(sqlite3_file*, void*, số_nguyên iAmt, sqlite3_int64 iOfst);
static số_nguyên tvfsWrite(sqlite3_file*,const void*,số_nguyên iAmt, sqlite3_int64 iOfst);
static số_nguyên tvfsTruncate(sqlite3_file*, sqlite3_int64 size);
static số_nguyên tvfsSync(sqlite3_file*, số_nguyên flags);
static số_nguyên tvfsFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static số_nguyên tvfsLock(sqlite3_file*, số_nguyên);
static số_nguyên tvfsUnlock(sqlite3_file*, số_nguyên);
static số_nguyên tvfsCheckReservedLock(sqlite3_file*, số_nguyên *);
static số_nguyên tvfsFileControl(sqlite3_file*, số_nguyên op, void *pArg);
static số_nguyên tvfsSectorSize(sqlite3_file*);
static số_nguyên tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
static số_nguyên tvfsOpen(sqlite3_vfs*, const char *, sqlite3_file*, số_nguyên , số_nguyên *);
static số_nguyên tvfsDelete(sqlite3_vfs*, const char *zName, số_nguyên syncDir);
static số_nguyên tvfsAccess(sqlite3_vfs*, const char *zName, số_nguyên flags, số_nguyên *);
static số_nguyên tvfsFullPathname(sqlite3_vfs*, const char *zName, số_nguyên, char *zOut);
#ifndef SQLITE_OMIT_LOAD_EXTENSION
static void *tvfsDlOpen(sqlite3_vfs*, const char *zFilename);
static void tvfsDlError(sqlite3_vfs*, số_nguyên nByte, char *zErrMsg);
static void (*tvfsDlSym(sqlite3_vfs*,void*, const char *zSymbol))(void);
static void tvfsDlClose(sqlite3_vfs*, void*);
#endif /* SQLITE_OMIT_LOAD_EXTENSION */
static số_nguyên tvfsRandomness(sqlite3_vfs*, số_nguyên nByte, char *zOut);
static số_nguyên tvfsSleep(sqlite3_vfs*, số_nguyên microseconds);
static số_nguyên tvfsCurrentTime(sqlite3_vfs*, double*);

static số_nguyên tvfsShmOpen(sqlite3_file*);
static số_nguyên tvfsShmLock(sqlite3_file*, số_nguyên , số_nguyên, số_nguyên);
static số_nguyên tvfsShmMap(sqlite3_file*,số_nguyên,số_nguyên,số_nguyên, void volatile **);
static void tvfsShmBarrier(sqlite3_file*);
static số_nguyên tvfsShmUnmap(sqlite3_file*, số_nguyên);

static số_nguyên tvfsFetch(sqlite3_file*, sqlite3_int64, số_nguyên, void**);
static số_nguyên tvfsUnfetch(sqlite3_file*, sqlite3_int64, void*);

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

static số_nguyên tvfsResultCode(Testvfs *p, số_nguyên *pRc){
  struct errcode {
    số_nguyên eCode;
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
  số_nguyên i;

  z = Tcl_GetStringResult(p->interp);
  cho(i=0; i<ArraySize(aCode); i++){
    nếu( 0==strcmp(z, aCode[i].zCode) ){
      *pRc = aCode[i].eCode;
      trả_về 1;
    }
  }

  trả_về 0;
}

static số_nguyên tvfsInjectFault(TestFaultInject *p){
  số_nguyên ret = 0;
  nếu( p->eFault ){
    p->iCnt--;
    nếu( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  trả_về ret;
}


static số_nguyên tvfsInjectIoerr(Testvfs *p){
  trả_về tvfsInjectFault(&p->ioerr_err);
}

static số_nguyên tvfsInjectFullerr(Testvfs *p){
  trả_về tvfsInjectFault(&p->full_err);
}
static số_nguyên tvfsInjectCantopenerr(Testvfs *p){
  trả_về tvfsInjectFault(&p->cantopen_err);
}


static void tvfsExecTcl(
  Testvfs *p, 
  const char *zMethod,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  số_nguyên rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  assert( p->pScript );

  assert( zMethod );
  assert( p );
  assert( arg2==0 || arg1!=0 );
  assert( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_IncrRefCount(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(zMethod, -1));
  nếu( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  nếu( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  nếu( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  nếu( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  nếu( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_ResetResult(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
static số_nguyên tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  nếu( p->pScript && p->mask&TESTVFS_CLOSE_MASK ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  nếu( pFd->pShmId ){
    Tcl_DecrRefCount(pFd->pShmId);
    pFd->pShmId = 0;
  }
  nếu( pFile->pMethods ){
    ckfree((char *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((char *)pFd);
  pTestfile->pFd = 0;
  trả_về SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
static số_nguyên tvfsRead(
  sqlite3_file *pFile, 
  void *zBuf, 
  số_nguyên iAmt, 
  sqlite_int64 iOfst
){
  số_nguyên rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  nếu( p->pScript && p->mask&TESTVFS_READ_MASK ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  nếu( rc==SQLITE_OK && p->mask&TESTVFS_READ_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  nếu( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  trả_về rc;
}

/*
** Write data to an tvfs-file.
*/
static số_nguyên tvfsWrite(
  sqlite3_file *pFile, 
  const void *zBuf, 
  số_nguyên iAmt, 
  sqlite_int64 iOfst
){
  số_nguyên rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  nếu( p->pScript && p->mask&TESTVFS_WRITE_MASK ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfsResultCode(p, &rc);
    nếu( rc<0 ) trả_về SQLITE_OK;
  }

  nếu( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_FULL;
  }
  nếu( rc==SQLITE_OK && p->mask&TESTVFS_WRITE_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  nếu( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  trả_về rc;
}

/*
** Truncate an tvfs-file.
*/
static số_nguyên tvfsTruncate(sqlite3_file *pFile, sqlite_int64 size){
  số_nguyên rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  nếu( p->pScript && p->mask&TESTVFS_TRUNCATE_MASK ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  
  nếu( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, size);
  }
  trả_về rc;
}

/*
** Sync an tvfs-file.
*/
static số_nguyên tvfsSync(sqlite3_file *pFile, số_nguyên flags){
  số_nguyên rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;

  nếu( p->pScript && p->mask&TESTVFS_SYNC_MASK ){
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

  nếu( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_FULL;

  nếu( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  trả_về rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
static số_nguyên tvfsFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
  TestvfsFd *p = tvfsGetFd(pFile);
  trả_về sqlite3OsFileSize(p->pReal, pSize);
}

/*
** Lock an tvfs-file.
*/
static số_nguyên tvfsLock(sqlite3_file *pFile, số_nguyên eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  nếu( p->pScript && p->mask&TESTVFS_LOCK_MASK ){
    char zLock[30];
    sqlite3_snprintf(sizeof(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  nếu( p->mask&TESTVFS_LOCK_MASK && tvfsInjectIoerr(p) ){
    trả_về SQLITE_IOERR_LOCK;
  }
  trả_về sqlite3OsLock(pFd->pReal, eLock);
}

/*
** Unlock an tvfs-file.
*/
static số_nguyên tvfsUnlock(sqlite3_file *pFile, số_nguyên eLock){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  nếu( p->pScript && p->mask&TESTVFS_UNLOCK_MASK ){
    char zLock[30];
    sqlite3_snprintf(sizeof(zLock),zLock,"%d",eLock);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(zLock, -1), 0, 0);
  }
  nếu( p->mask&TESTVFS_UNLOCK_MASK && tvfsInjectIoerr(p) ){
    trả_về SQLITE_IOERR_UNLOCK;
  }
  trả_về sqlite3OsUnlock(pFd->pReal, eLock);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
static số_nguyên tvfsCheckReservedLock(sqlite3_file *pFile, số_nguyên *pResOut){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  nếu( p->pScript && p->mask&TESTVFS_CKLOCK_MASK ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  trả_về sqlite3OsCheckReservedLock(pFd->pReal, pResOut);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
static số_nguyên tvfsFileControl(sqlite3_file *pFile, số_nguyên op, void *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  nếu( op==SQLITE_FCNTL_PRAGMA ){
    char **argv = (char**)pArg;
    nếu( sqlite3_stricmp(argv[1],"error")==0 ){
      số_nguyên rc = SQLITE_ERROR;
      nếu( argv[2] ){
        const char *z = argv[2];
        số_nguyên x = atoi(z);
        nếu( x ){
          rc = x;
          trong_khi( sqlite3Isdigit(z[0]) ){ z++; }
          trong_khi( sqlite3Isspace(z[0]) ){ z++; }
        }
        nếu( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      trả_về rc;
    }
    nếu( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      trả_về SQLITE_OK;
    }
  }
  nếu( p->pScript && (p->mask&TESTVFS_FCNTL_MASK) ){
    struct Fcntl {
      số_nguyên iFnctl;
      const char *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    số_nguyên i;
    cho(i=0; i<sizeof(aF)/sizeof(aF[0]); i++){
      nếu( op==aF[i].iFnctl ) break;
    }
    nếu( i<sizeof(aF)/sizeof(aF[0]) ){
      số_nguyên rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfsResultCode(p, &rc);
      nếu( rc ) trả_về (rc<0 ? SQLITE_OK : rc);
    }
  }
  trả_về sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
static số_nguyên tvfsSectorSize(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  nếu( p->iSectorsize>=0 ){
    trả_về p->iSectorsize;
  }
  trả_về sqlite3OsSectorSize(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
static số_nguyên tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->pAppData;
  nếu( p->iDevchar>=0 ){
    trả_về p->iDevchar;
  }
  trả_về sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
static số_nguyên tvfsOpen(
  sqlite3_vfs *pVfs,
  const char *zName,
  sqlite3_file *pFile,
  số_nguyên flags,
  số_nguyên *pOutFlags
){
  số_nguyên rc;
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
  nếu( p->pScript && p->mask&TESTVFS_OPEN_MASK ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_IncrRefCount(pArg);
    nếu( flags&SQLITE_OPEN_MAIN_DB ){
      const char *z = &zName[strlen(zName)+1];
      trong_khi( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += strlen(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_DecrRefCount(pArg);
    nếu( tvfsResultCode(p, &rc) ){
      nếu( rc!=SQLITE_OK ) trả_về rc;
    }không_thì{
      pId = Tcl_GetObjResult(p->interp);
    }
  }

  nếu( (p->mask&TESTVFS_OPEN_MASK) &&  tvfsInjectIoerr(p) ) trả_về SQLITE_IOERR;
  nếu( tvfsInjectCantopenerr(p) ) trả_về SQLITE_CANTOPEN;
  nếu( tvfsInjectFullerr(p) ) trả_về SQLITE_FULL;

  nếu( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_IncrRefCount(pId);
  pFd->pShmId = pId;
  Tcl_ResetResult(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, pOutFlags);
  nếu( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    số_nguyên nByte;

    nếu( pVfs->iVersion>1 ){
      nByte = sizeof(sqlite3_io_methods);
    }không_thì{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    memcpy(pMethods, &tvfs_io_methods, nByte);
    pMethods->iVersion = pFd->pReal->pMethods->iVersion;
    nếu( pMethods->iVersion>pVfs->iVersion ){
      pMethods->iVersion = pVfs->iVersion;
    }
    nếu( pVfs->iVersion>1 && ((Testvfs *)pVfs->pAppData)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->xShmLock = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  trả_về rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static số_nguyên tvfsDelete(sqlite3_vfs *pVfs, const char *zPath, số_nguyên dirSync){
  số_nguyên rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->pAppData;

  nếu( p->pScript && p->mask&TESTVFS_DELETE_MASK ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfsResultCode(p, &rc);
  }
  nếu( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), zPath, dirSync);
  }
  trả_về rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static số_nguyên tvfsAccess(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  số_nguyên flags, 
  số_nguyên *pResOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  nếu( p->pScript && p->mask&TESTVFS_ACCESS_MASK ){
    số_nguyên rc;
    char *zArg = 0;
    nếu( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    nếu( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    nếu( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(zPath, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    nếu( tvfsResultCode(p, &rc) ){
      nếu( rc!=SQLITE_OK ) trả_về rc;
    }không_thì{
      Tcl_Interp *interp = p->interp;
      nếu( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_GetObjResult(interp), pResOut) ){
        trả_về SQLITE_OK;
      }
    }
  }
  trả_về sqlite3OsAccess(PARENTVFS(pVfs), zPath, flags, pResOut);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
static số_nguyên tvfsFullPathname(
  sqlite3_vfs *pVfs, 
  const char *zPath, 
  số_nguyên nOut, 
  char *zOut
){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  nếu( p->pScript && p->mask&TESTVFS_FULLPATHNAME_MASK ){
    số_nguyên rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(zPath, -1), 0, 0, 0);
    nếu( tvfsResultCode(p, &rc) ){
      nếu( rc!=SQLITE_OK ) trả_về rc;
    }
  }
  trả_về sqlite3OsFullPathname(PARENTVFS(pVfs), zPath, nOut, zOut);
}

#ifndef SQLITE_OMIT_LOAD_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *tvfsDlOpen(sqlite3_vfs *pVfs, const char *zPath){
  trả_về sqlite3OsDlOpen(PARENTVFS(pVfs), zPath);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
static void tvfsDlError(sqlite3_vfs *pVfs, số_nguyên nByte, char *zErrMsg){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, zErrMsg);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*tvfsDlSym(sqlite3_vfs *pVfs, void *p, const char *zSym))(void){
  trả_về sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
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
static số_nguyên tvfsRandomness(sqlite3_vfs *pVfs, số_nguyên nByte, char *zBufOut){
  trả_về sqlite3OsRandomness(PARENTVFS(pVfs), nByte, zBufOut);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
static số_nguyên tvfsSleep(sqlite3_vfs *pVfs, số_nguyên nMicro){
  Testvfs *p = (Testvfs *)pVfs->pAppData;
  nếu( p->pScript && (p->mask&TESTVFS_SLEEP_MASK) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  trả_về sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static số_nguyên tvfsCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
  trả_về PARENTVFS(pVfs)->xCurrentTime(PARENTVFS(pVfs), pTimeOut);
}

static số_nguyên tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  số_nguyên rc = SQLITE_OK;             /* Return code */
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
  nếu( p->pScript && p->mask&TESTVFS_SHMOPEN_MASK ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    nếu( tvfsResultCode(p, &rc) ){
      nếu( rc!=SQLITE_OK ) trả_về rc;
    }
  }

  assert( rc==SQLITE_OK );
  nếu( p->mask&TESTVFS_SHMOPEN_MASK && tvfsInjectIoerr(p) ){
    trả_về SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  cho(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
    nếu( 0==strcmp(pFd->zFilename, pBuffer->zFile) ) break;
  }
  nếu( !pBuffer ){
    số_nguyên szName = (số_nguyên)strlen(pFd->zFilename);
    số_nguyên nByte = sizeof(TestvfsBuffer) + szName + 1;
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
  trả_về rc;
}

static void tvfsAllocPage(TestvfsBuffer *p, số_nguyên iPage, số_nguyên pgsz){
  assert( iPage<TESTVFS_MAX_PAGES );
  nếu( p->aPage[iPage]==0 ){
    p->aPage[iPage] = (u8 *)ckalloc(pgsz);
    memset(p->aPage[iPage], 0, pgsz);
    p->pgsz = pgsz;
  }
}

static số_nguyên tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  số_nguyên iPage,                      /* Page to retrieve */
  số_nguyên pgsz,                       /* Size of pages */
  số_nguyên isWrite,                    /* True to extend file if necessary */
  void volatile **pp              /* OUT: Mapped memory */
){
  số_nguyên rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  nếu( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    trả_về pReal->pMethods->xShmMap(pReal, iPage, pgsz, isWrite, pp);
  }

  nếu( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    nếu( rc!=SQLITE_OK ){
      trả_về rc;
    }
  }

  nếu( p->pScript && p->mask&TESTVFS_SHMMAP_MASK ){
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
  nếu( rc==SQLITE_OK && p->mask&TESTVFS_SHMMAP_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  nếu( rc==SQLITE_OK && isWrite && !pFd->pShm->aPage[iPage] ){
    tvfsAllocPage(pFd->pShm, iPage, pgsz);
  }
  nếu( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (void volatile *)pFd->pShm->aPage[iPage];
  }

  trả_về rc;
}


static số_nguyên tvfsShmLock(
  sqlite3_file *pFile,
  số_nguyên ofst,
  số_nguyên n,
  số_nguyên flags
){
  số_nguyên rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  số_nguyên nLock;
  char zLock[80];

  nếu( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    trả_về pReal->pMethods->xShmLock(pReal, ofst, n, flags);
  }

  nếu( p->pScript && p->mask&TESTVFS_SHMLOCK_MASK ){
    sqlite3_snprintf(sizeof(zLock), zLock, "%d %d", ofst, n);
    nLock = (số_nguyên)strlen(zLock);
    nếu( flags & SQLITE_SHM_LOCK ){
      strcpy(&zLock[nLock], " lock");
    }không_thì{
      strcpy(&zLock[nLock], " unlock");
    }
    nLock += (số_nguyên)strlen(&zLock[nLock]);
    nếu( flags & SQLITE_SHM_SHARED ){
      strcpy(&zLock[nLock], " shared");
    }không_thì{
      strcpy(&zLock[nLock], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(zLock, -1), 0
    );
    tvfsResultCode(p, &rc);
  }

  nếu( rc==SQLITE_OK && p->mask&TESTVFS_SHMLOCK_MASK && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  nếu( rc==SQLITE_OK ){
    số_nguyên isLock = (flags & SQLITE_SHM_LOCK);
    số_nguyên isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 mask = (((1<<n)-1) << ofst);
    nếu( isLock ){
      TestvfsFd *p2;
      cho(p2=pFd->pShm->pFile; p2; p2=p2->pNext){
        nếu( p2==pFd ) continue;
        nếu( (p2->excllock&mask) || (isExcl && p2->sharedlock&mask) ){
          rc = SQLITE_BUSY;
          break;
        }
      }
      nếu( rc==SQLITE_OK ){
        nếu( isExcl )  pFd->excllock |= mask;
        nếu( !isExcl ) pFd->sharedlock |= mask;
      }
    }không_thì{
      nếu( isExcl )  pFd->excllock &= (~mask);
      nếu( !isExcl ) pFd->sharedlock &= (~mask);
    }
  }

  trả_về rc;
}

static void tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);

  nếu( p->pScript && p->mask&TESTVFS_SHMBARRIER_MASK ){
    const char *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  nếu( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    trả_về;
  }
}

static số_nguyên tvfsShmUnmap(
  sqlite3_file *pFile,
  số_nguyên deleteFlag
){
  số_nguyên rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->pAppData);
  TestvfsBuffer *pBuffer = pFd->pShm;
  TestvfsFd **ppFd;

  nếu( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    trả_về pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  nếu( !pBuffer ) trả_về SQLITE_OK;
  assert( pFd->pShmId && pFd->pShm );

  nếu( p->pScript && p->mask&TESTVFS_SHMCLOSE_MASK ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfsResultCode(p, &rc);
  }

  cho(ppFd=&pBuffer->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->pNext));
  assert( (*ppFd)==pFd );
  *ppFd = pFd->pNext;
  pFd->pNext = 0;

  nếu( pBuffer->pFile==0 ){
    số_nguyên i;
    TestvfsBuffer **pp;
    cho(pp=&p->pBuffer; *pp!=pBuffer; pp=&((*pp)->pNext));
    *pp = (*pp)->pNext;
    cho(i=0; pBuffer->aPage[i]; i++){
      ckfree((char *)pBuffer->aPage[i]);
    }
    ckfree((char *)pBuffer);
  }
  pFd->pShm = 0;

  trả_về rc;
}

static số_nguyên tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    số_nguyên iAmt, 
    void **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  trả_về sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

static số_nguyên tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, void *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  trả_về sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

static số_nguyên SQLITE_TCLAPI testvfs_obj_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  số_nguyên objc,
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
  số_nguyên i;
  
  nếu( objc<2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "SUBCOMMAND ...");
    trả_về TCL_ERROR;
  }
  nếu( Tcl_GetIndexFromObjStruct(
        interp, objv[1], aSubcmd, sizeof(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    trả_về TCL_ERROR;
  }
  Tcl_ResetResult(interp);

  switch( aSubcmd[i].eCmd ){
    case CMD_SHM: {
      Tcl_Obj *pObj;
      số_nguyên rc;
      TestvfsBuffer *pBuffer;
      char *zName;
      nếu( objc!=3 && objc!=4 ){
        Tcl_WrongNumArgs(interp, 2, objv, "FILE ?VALUE?");
        trả_về TCL_ERROR;
      }
      zName = ckalloc(p->pParent->mxPathname);
      rc = p->pParent->xFullPathname(
          p->pParent, Tcl_GetString(objv[2]), 
          p->pParent->mxPathname, zName
      );
      nếu( rc!=SQLITE_OK ){
        Tcl_AppendResult(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), NULL);
        ckfree(zName);
        trả_về TCL_ERROR;
      }
      cho(pBuffer=p->pBuffer; pBuffer; pBuffer=pBuffer->pNext){
        nếu( 0==strcmp(pBuffer->zFile, zName) ) break;
      }
      ckfree(zName);
      nếu( !pBuffer ){
        Tcl_AppendResult(interp, "no such file: ", Tcl_GetString(objv[2]), NULL);
        trả_về TCL_ERROR;
      }
      nếu( objc==4 ){
        Tcl_Size n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        số_nguyên pgsz = pBuffer->pgsz;
        nếu( pgsz==0 ) pgsz = 65536;
        cho(i=0; i*pgsz<(số_nguyên)n; i++){
          số_nguyên nByte = pgsz;
          tvfsAllocPage(pBuffer, i, pgsz);
          nếu( n-i*pgsz<pgsz ){
            nByte = (số_nguyên)n;
          }
          memcpy(pBuffer->aPage[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      cho(i=0; pBuffer->aPage[i]; i++){
        số_nguyên pgsz = pBuffer->pgsz;
        nếu( pgsz==0 ) pgsz = 65536;
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
        số_nguyên mask;
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
      số_nguyên mask = 0;
      nếu( objc!=3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "LIST");
        trả_về TCL_ERROR;
      }
      nếu( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        trả_về TCL_ERROR;
      }
      Tcl_ResetResult(interp);
      cho(i=0; i<(số_nguyên)nElem; i++){
        số_nguyên iMethod;
        char *zElem = Tcl_GetString(apElem[i]);
        cho(iMethod=0; iMethod<ArraySize(vfsmethod); iMethod++){
          nếu( strcmp(zElem, vfsmethod[iMethod].zName)==0 ){
            mask |= vfsmethod[iMethod].mask;
            break;
          }
        }
        nếu( iMethod==ArraySize(vfsmethod) ){
          Tcl_AppendResult(interp, "unknown method: ", zElem, NULL);
          trả_về TCL_ERROR;
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
      nếu( objc==3 ){
        Tcl_Size nByte;
        nếu( p->pScript ){
          Tcl_DecrRefCount(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        nếu( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_IncrRefCount(p->pScript);
        }
      }không_thì nếu( objc!=2 ){
        Tcl_WrongNumArgs(interp, 2, objv, "?SCRIPT?");
        trả_về TCL_ERROR;
      }

      Tcl_ResetResult(interp);
      nếu( p->pScript ) Tcl_SetObjResult(interp, p->pScript);

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
      số_nguyên iRet;

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

      nếu( objc==4 ){
        số_nguyên iCnt, iPersist;
        nếu( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          trả_về TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }không_thì nếu( objc!=2 ){
        Tcl_WrongNumArgs(interp, 2, objv, "?CNT PERSIST?");
        trả_về TCL_ERROR;
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
        số_nguyên iValue;
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
      số_nguyên iFlag;

      nếu( objc>3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "?ATTR-LIST?");
        trả_về TCL_ERROR;
      }
      nếu( objc==3 ){
        số_nguyên j;
        số_nguyên iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_Size nFlags = 0;

        nếu( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          trả_về TCL_ERROR;
        }

        cho(j=0; j<(số_nguyên)nFlags; j++){
          số_nguyên idx = 0;
          nếu( Tcl_GetIndexFromObjStruct(interp, flags[j], aFlag, 
                sizeof(aFlag[0]), "flag", 0, &idx) 
          ){
            trả_về TCL_ERROR;
          }
          nếu( aFlag[idx].iValue<0 && nFlags>1 ){
            Tcl_AppendResult(interp, "bad flags: ", Tcl_GetString(objv[2]), NULL);
            trả_về TCL_ERROR;
          }
          iNew |= aFlag[idx].iValue;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      cho(iFlag=0; iFlag<sizeof(aFlag)/sizeof(aFlag[0]); iFlag++){
        nếu( p->iDevchar & aFlag[iFlag].iValue ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_SetObjResult(interp, pRet);

      break;
    }

    case CMD_SECTORSIZE: {
      nếu( objc>3 ){
        Tcl_WrongNumArgs(interp, 2, objv, "?VALUE?");
        trả_về TCL_ERROR;
      }
      nếu( objc==3 ){
        số_nguyên iNew = 0;
        nếu( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          trả_về TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(p->iSectorsize));
      break;
    }
  }

  trả_về TCL_OK;
}

static void SQLITE_TCLAPI testvfs_obj_del(ClientData cd){
  Testvfs *p = (Testvfs *)cd;
  nếu( p->pScript ) Tcl_DecrRefCount(p->pScript);
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
static số_nguyên SQLITE_TCLAPI testvfs_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  số_nguyên objc,
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
#không_thì
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
  số_nguyên nByte;                      /* Bytes of space to allocate at p */

  số_nguyên i;
  số_nguyên isNoshm = 0;                /* True if -noshm is passed */
  số_nguyên isFullshm = 0;              /* True if -fullshm is passed */
  số_nguyên isDefault = 0;              /* True if -default is passed */
  số_nguyên szOsFile = 0;               /* Value passed to -szosfile */
  số_nguyên mxPathname = -1;            /* Value passed to -mxpathname */
  số_nguyên iVersion = 3;               /* Value passed to -iversion */

  nếu( objc<2 || 0!=(objc%2) ) goto bad_args;
  cho(i=2; i<objc; i += 2){
    Tcl_Size nSwitch;
    char *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    nếu( nSwitch>2 && 0==strncmp("-noshm", zSwitch, nSwitch) ){
      nếu( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        trả_về TCL_ERROR;
      }
      nếu( isNoshm ) isFullshm = 0;
    }
    không_thì nếu( nSwitch>2 && 0==strncmp("-default", zSwitch, nSwitch) ){
      nếu( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        trả_về TCL_ERROR;
      }
    }
    không_thì nếu( nSwitch>2 && 0==strncmp("-szosfile", zSwitch, nSwitch) ){
      nếu( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        trả_về TCL_ERROR;
      }
    }
    không_thì nếu( nSwitch>2 && 0==strncmp("-mxpathname", zSwitch, nSwitch) ){
      nếu( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        trả_về TCL_ERROR;
      }
    }
    không_thì nếu( nSwitch>2 && 0==strncmp("-iversion", zSwitch, nSwitch) ){
      nếu( Tcl_GetIntFromObj(interp, objv[i+1], &iVersion) ){
        trả_về TCL_ERROR;
      }
    }
    không_thì nếu( nSwitch>2 && 0==strncmp("-fullshm", zSwitch, nSwitch) ){
      nếu( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        trả_về TCL_ERROR;
      }
      nếu( isFullshm ) isNoshm = 0;
    }
    không_thì{
      goto bad_args;
    }
  }

  nếu( szOsFile<sizeof(TestvfsFile) ){
    szOsFile = sizeof(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = sizeof(Testvfs) + (số_nguyên)strlen(zVfs)+1;
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
  nếu( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->mask = TESTVFS_ALL_MASK;

  sqlite3_vfs_register(pVfs, isDefault);

  trả_về TCL_OK;

 bad_args:
  Tcl_WrongNumArgs(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  trả_về TCL_ERROR;
}

extern số_nguyên getDbPointer(Tcl_Interp *interp, const char *zA, sqlite3 **ppDb);
extern const char *sqlite3ErrName(số_nguyên);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
static số_nguyên SQLITE_TCLAPI test_vfs_shmlock(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  const char *azArg1[] = {"shared", "exclusive", 0};
  const char *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  số_nguyên rc = SQLITE_OK;
  const char *zDbname = 0;
  số_nguyên iArg1 = 0;
  số_nguyên iArg2 = 0;
  số_nguyên iOffset = 0;
  số_nguyên n = 0;
  sqlite3_file *pFd;

  nếu( objc!=7 ){
    Tcl_WrongNumArgs(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    trả_về TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  nếu( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIndexFromObj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_GetIndexFromObj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &iOffset)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    trả_về TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (void*)&pFd);
  nếu( pFd==0 ){
    trả_về TCL_ERROR;
  }
  rc = pFd->pMethods->xShmLock(pFd, iOffset, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_LOCK : SQLITE_SHM_UNLOCK)
  );
  Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  trả_về TCL_OK;
}

static số_nguyên SQLITE_TCLAPI test_vfs_set_readmark(
  void * clientData,
  Tcl_Interp *interp,
  số_nguyên objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  số_nguyên rc = SQLITE_OK;
  const char *zDbname = 0;
  số_nguyên iSlot = 0;
  số_nguyên iVal = -1;
  sqlite3_file *pFd;
  void volatile *pShm = 0;
  u32 *aShm;
  số_nguyên iOff;

  nếu( objc!=4 && objc!=5 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    trả_về TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  nếu( getDbPointer(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &iSlot)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    trả_về TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_POINTER, (void*)&pFd);
  nếu( pFd==0 ){
    trả_về TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  nếu( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    trả_về TCL_ERROR;
  }
  nếu( pShm==0 ){
    Tcl_AppendResult(interp, "*-shm is not yet mapped", NULL);
    trả_về TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+iSlot;

  nếu( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_SetObjResult(interp, Tcl_NewIntObj(aShm[iOff]));

  trả_về TCL_OK;
}

số_nguyên Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_set_readmark, 0, 0);
  trả_về TCL_OK;
}

#endif
