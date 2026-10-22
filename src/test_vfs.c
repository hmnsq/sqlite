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
#ಆದರೆ SQLITE_TEST          /* This file is used for testing only */

#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Testvfs Testvfs;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ TestvfsShm TestvfsShm;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Testvfs_ಬಫರ್ Testvfs_ಬಫರ್;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ TestvfsFile TestvfsFile;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ TestvfsFd TestvfsFd;

/*
** An open file handle.
*/
ರಚನೆ TestvfsFile {
  sqlite3_file ತಳ;              /* Base class.  Must be first */
  TestvfsFd *pFd;                 /* File data */
};
#ವ್ಯಾಖ್ಯಾನಿಸು tvfsGetFd(pFile) (((TestvfsFile *)pFile)->pFd)

ರಚನೆ TestvfsFd {
  sqlite3_vfs *pVfs;              /* The VFS */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename;          /* Filename as passed to xOpen() */
  sqlite3_file *pReal;            /* The real, underlying file descriptor */
  Tcl_Obj *pShmId;                /* Shared memory id for Tcl callbacks */

  Testvfs_ಬಫರ್ *pShm;            /* Shared memory buffer */
  u32 excllock;                   /* Mask of exclusive locks */
  u32 sharedlock;                 /* Mask of shared locks */
  TestvfsFd *p_ಮುಂದಿನ;               /* Next handle opened on the same file */
};


#ವ್ಯಾಖ್ಯಾನಿಸು FAULT_INJECT_NONE       0
#ವ್ಯಾಖ್ಯಾನಿಸು FAULT_INJECT_TRANSIENT  1
#ವ್ಯಾಖ್ಯಾನಿಸು FAULT_INJECT_PERSISTENT 2

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ TestFaultInject TestFaultInject;
ರಚನೆ TestFaultInject {
  ಪೂರ್ಣಾಂಕ iCnt;                       /* Remaining calls before fault injection */
  ಪೂರ್ಣಾಂಕ eFault;                     /* A FAULT_INJECT_* value */
  ಪೂರ್ಣಾಂಕ nFail;                      /* Number of faults injected */
};

/*
** An instance of this structure is allocated for each VFS created. The
** sqlite3_vfs.pAppData field of the VFS structure registered with SQLite
** is set to point to it.
*/
ರಚನೆ Testvfs {
  ಅಕ್ಷರ *zName;                    /* Name of this VFS */
  sqlite3_vfs *p_ಪೋಷಕ;           /* The VFS to use for file IO */
  sqlite3_vfs *pVfs;              /* The testvfs registered with SQLite */
  Tcl_Interp *interp;             /* Interpreter to run script in */
  Tcl_Obj *pScript;               /* Script to execute */
  Testvfs_ಬಫರ್ *p_ಬಫರ್;         /* List of shared buffers */
  ಪೂರ್ಣಾಂಕ isNoshm;
  ಪೂರ್ಣಾಂಕ isFullshm;

  ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ;                       /* Mask controlling [script] and [ioerr] */

  TestFaultInject ioerr_err;
  TestFaultInject ತುಂಬಿದ_err;
  TestFaultInject cantopen_err;

#ಆದರೆ 0
  ಪೂರ್ಣಾಂಕ iIoerrCnt;
  ಪೂರ್ಣಾಂಕ ioerr;
  ಪೂರ್ಣಾಂಕ nIoerrFail;
  ಪೂರ್ಣಾಂಕ i_ತುಂಬಿದ_Cnt;
  ಪೂರ್ಣಾಂಕ fullerr;
  ಪೂರ್ಣಾಂಕ n_ತುಂಬಿದ_Fail;
#ಆದರೆ_ಅಂತ್ಯ

  ಪೂರ್ಣಾಂಕ iDevchar;
  ಪೂರ್ಣಾಂಕ iSectorsize;
};

/*
** The Testvfs.mask variable is set to a combination of the following.
** If a bit is clear in Testvfs.mask, then calls made by SQLite to the 
** corresponding VFS method is ignored for purposes of:
**
**   + Simulating IO errors, and
**   + Invoking the Tcl callback script.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_SHMOPEN_ಮುಖವಾಡ      0x00000001
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_SHMLOCK_ಮುಖವಾಡ      0x00000010
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_SHMMAP_ಮುಖವಾಡ       0x00000020
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_SHMBARRIER_ಮುಖವಾಡ   0x00000040
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_SHMCLOSE_ಮುಖವಾಡ     0x00000080

#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_OPEN_ಮುಖವಾಡ         0x00000100
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_SYNC_ಮುಖವಾಡ         0x00000200
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_DELETE_ಮುಖವಾಡ       0x00000400
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_CLOSE_ಮುಖವಾಡ        0x00000800
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_WRITE_ಮುಖವಾಡ        0x00001000
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_TRUNCATE_ಮುಖವಾಡ     0x00002000
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_ACCESS_ಮುಖವಾಡ       0x00004000
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_FULLPATHNAME_ಮುಖವಾಡ 0x00008000
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_READ_ಮುಖವಾಡ         0x00010000
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_UNLOCK_ಮುಖವಾಡ       0x00020000
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_ಬೀಗ_ಮುಖವಾಡ         0x00040000
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_CKLOCK_ಮುಖವಾಡ       0x00080000
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_FCNTL_ಮುಖವಾಡ        0x00100000
#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_SLEEP_ಮುಖವಾಡ        0x00200000

#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_ALL_ಮುಖವಾಡ          0x003FFFFF


#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_MAX_ಪುಟಗಳು 1024

/*
** A shared-memory buffer. There is one of these objects for each shared
** memory region opened by clients. If two clients open the same file,
** there are two TestvfsFile structures but only one TestvfsBuffer structure.
*/
ರಚನೆ Testvfs_ಬಫರ್ {
  ಅಕ್ಷರ *zFile;                    /* Associated file name */
  ಪೂರ್ಣಾಂಕ pgsz;                       /* Page size */
  u8 *a_ಪುಟ[TESTVFS_MAX_ಪುಟಗಳು];   /* Array of ckalloc'd pages */
  TestvfsFd *pFile;               /* List of open handles */
  Testvfs_ಬಫರ್ *p_ಮುಂದಿನ;           /* Next in linked list of all buffers */
};


#ವ್ಯಾಖ್ಯಾನಿಸು PARENTVFS(x) (((Testvfs *)((x)->p_App_ದತ್ತಾಂಶ))->p_ಪೋಷಕ)

#ವ್ಯಾಖ್ಯಾನಿಸು TESTVFS_MAX_ವಾದಗಳು 12


/*
** Method declarations for TestvfsFile.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsClose(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsRead(sqlite3_file*, ಶೂನ್ಯ*, ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsWrite(sqlite3_file*,ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ iAmt, sqlite3_int64 iOfst);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsTruncate(sqlite3_file*, sqlite3_int64 ಗಾತ್ರ_2);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsSync(sqlite3_file*, ಪೂರ್ಣಾಂಕ flags);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_File_ಗಾತ್ರ(sqlite3_file*, sqlite3_int64 *p_ಗಾತ್ರ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsUnlock(sqlite3_file*, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_Check_Reserved_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsFileControl(sqlite3_file*, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_Sector_ಗಾತ್ರ(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for tvfs_vfs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, sqlite3_file*, ಪೂರ್ಣಾಂಕ , ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsDelete(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ syncDir);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsAccess(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ flags, ಪೂರ್ಣಾಂಕ *);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_ತುಂಬಿದ_Pathname(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
ಸ್ಥಿರ ಶೂನ್ಯ *tvfsDlOpen(sqlite3_vfs*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFilename);
ಸ್ಥಿರ ಶೂನ್ಯ tvfsDlError(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ);
ಸ್ಥಿರ ಶೂನ್ಯ (*tvfsDlSym(sqlite3_vfs*,ಶೂನ್ಯ*, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಚಿಹ್ನೆ))(ಶೂನ್ಯ);
ಸ್ಥಿರ ಶೂನ್ಯ tvfsDlClose(sqlite3_vfs*, ಶೂನ್ಯ*);
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_LOAD_EXTENSION */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsRandomness(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_ಹೊರಹೋಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsSleep(sqlite3_vfs*, ಪೂರ್ಣಾಂಕ microseconds);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_Current_ಸಮಯ(sqlite3_vfs*, ದ್ವಿಗುಣ*);

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsShmOpen(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_Shm_ಬೀಗ(sqlite3_file*, ಪೂರ್ಣಾಂಕ , ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsShmMap(sqlite3_file*,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ,ಪೂರ್ಣಾಂಕ, ಶೂನ್ಯ ಅಸ್ಥಿರ **);
ಸ್ಥಿರ ಶೂನ್ಯ tvfsShmBarrier(sqlite3_file*);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsShmUnmap(sqlite3_file*, ಪೂರ್ಣಾಂಕ);

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsFetch(sqlite3_file*, sqlite3_int64, ಪೂರ್ಣಾಂಕ, ಶೂನ್ಯ**);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsUnfetch(sqlite3_file*, sqlite3_int64, ಶೂನ್ಯ*);

ಸ್ಥಿರ sqlite3_io_methods tvfs_io_methods = {
  3,                              /* iVersion */
  tvfsClose,                      /* xClose */
  tvfsRead,                       /* xRead */
  tvfsWrite,                      /* xWrite */
  tvfsTruncate,                   /* xTruncate */
  tvfsSync,                       /* xSync */
  tvfs_File_ಗಾತ್ರ,                   /* xFileSize */
  tvfs_ಬೀಗ,                       /* xLock */
  tvfsUnlock,                     /* xUnlock */
  tvfs_Check_Reserved_ಬೀಗ,          /* xCheckReservedLock */
  tvfsFileControl,                /* xFileControl */
  tvfs_Sector_ಗಾತ್ರ,                 /* xSectorSize */
  tvfsDeviceCharacteristics,      /* xDeviceCharacteristics */
  tvfsShmMap,                     /* xShmMap */
  tvfs_Shm_ಬೀಗ,                    /* xShmLock */
  tvfsShmBarrier,                 /* xShmBarrier */
  tvfsShmUnmap,                   /* xShmUnmap */
  tvfsFetch,
  tvfsUnfetch
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(Testvfs *p, ಪೂರ್ಣಾಂಕ *pRc){
  ರಚನೆ errcode {
    ಪೂರ್ಣಾಂಕ e_ಸಂಕೇತ;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಸಂಕೇತ;
  } a_ಸಂಕೇತ[] = {
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

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z;
  ಪೂರ್ಣಾಂಕ i;

  z = Tcl_Get_String_ಫಲಿತಾಂಶ(p->interp);
  ಗಾಗಿ(i=0; i<Array_ಗಾತ್ರ(a_ಸಂಕೇತ); i++){
    ಆದರೆ( 0==strcmp(z, a_ಸಂಕೇತ[i].z_ಸಂಕೇತ) ){
      *pRc = a_ಸಂಕೇತ[i].e_ಸಂಕೇತ;
      ಹಿಂತಿರುಗಿ 1;
    }
  }

  ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsInjectFault(TestFaultInject *p){
  ಪೂರ್ಣಾಂಕ ret = 0;
  ಆದರೆ( p->eFault ){
    p->iCnt--;
    ಆದರೆ( p->iCnt==0 || (p->iCnt<0 && p->eFault==FAULT_INJECT_PERSISTENT ) ){
      ret = 1;
      p->nFail++;
    }
  }
  ಹಿಂತಿರುಗಿ ret;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsInjectIoerr(Testvfs *p){
  ಹಿಂತಿರುಗಿ tvfsInjectFault(&p->ioerr_err);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsInjectFullerr(Testvfs *p){
  ಹಿಂತಿರುಗಿ tvfsInjectFault(&p->ತುಂಬಿದ_err);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsInjectCantopenerr(Testvfs *p){
  ಹಿಂತಿರುಗಿ tvfsInjectFault(&p->cantopen_err);
}


ಸ್ಥಿರ ಶೂನ್ಯ tvfsExecTcl(
  Testvfs *p, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ವಿಧಾನ,
  Tcl_Obj *arg1,
  Tcl_Obj *arg2,
  Tcl_Obj *arg3,
  Tcl_Obj *arg4
){
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code from Tcl_EvalObj() */
  Tcl_Obj *pEval;
  ಪ್ರತಿಜ್ಞಾ( p->pScript );

  ಪ್ರತಿಜ್ಞಾ( z_ವಿಧಾನ );
  ಪ್ರತಿಜ್ಞಾ( p );
  ಪ್ರತಿಜ್ಞಾ( arg2==0 || arg1!=0 );
  ಪ್ರತಿಜ್ಞಾ( arg3==0 || arg2!=0 );

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ಎಣಿಕೆ(p->pScript);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj(z_ವಿಧಾನ, -1));
  ಆದರೆ( arg1 ) Tcl_ListObjAppendElement(p->interp, pEval, arg1);
  ಆದರೆ( arg2 ) Tcl_ListObjAppendElement(p->interp, pEval, arg2);
  ಆದರೆ( arg3 ) Tcl_ListObjAppendElement(p->interp, pEval, arg3);
  ಆದರೆ( arg4 ) Tcl_ListObjAppendElement(p->interp, pEval, arg4);

  rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  ಆದರೆ( rc!=TCL_OK ){
    Tcl_BackgroundError(p->interp);
    Tcl_Reset_ಫಲಿತಾಂಶ(p->interp);
  }
}


/*
** Close an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsClose(sqlite3_file *pFile){
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd = pTestfile->pFd;
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ದತ್ತಾಂಶ;

  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_CLOSE_ಮುಖವಾಡ ){
    tvfsExecTcl(p, "xClose", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
  }

  ಆದರೆ( pFd->pShmId ){
    Tcl_Decr_Ref_ಎಣಿಕೆ(pFd->pShmId);
    pFd->pShmId = 0;
  }
  ಆದರೆ( pFile->pMethods ){
    ckfree((ಅಕ್ಷರ *)pFile->pMethods);
  }
  sqlite3OsClose(pFd->pReal);
  ckfree((ಅಕ್ಷರ *)pFd);
  pTestfile->pFd = 0;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
}

/*
** Read data from an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsRead(
  sqlite3_file *pFile, 
  ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ದತ್ತಾಂಶ;
  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_READ_ಮುಖವಾಡ ){
    tvfsExecTcl(p, "xRead", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(p, &rc);
  }
  ಆದರೆ( rc==SQLITE_OK && p->ಮುಖವಾಡ&TESTVFS_READ_ಮುಖವಾಡ && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3OsRead(pFd->pReal, zBuf, iAmt, iOfst);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Write data to an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsWrite(
  sqlite3_file *pFile, 
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *zBuf, 
  ಪೂರ್ಣಾಂಕ iAmt, 
  sqlite_int64 iOfst
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ದತ್ತಾಂಶ;

  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_WRITE_ಮುಖವಾಡ ){
    tvfsExecTcl(p, "xWrite", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 
        Tcl_NewWideIntObj(iOfst), Tcl_NewIntObj(iAmt)
    );
    tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(p, &rc);
    ಆದರೆ( rc<0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  }

  ಆದರೆ( rc==SQLITE_OK && tvfsInjectFullerr(p) ){
    rc = SQLITE_ತುಂಬಿದ;
  }
  ಆದರೆ( rc==SQLITE_OK && p->ಮುಖವಾಡ&TESTVFS_WRITE_ಮುಖವಾಡ && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }
  
  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3OsWrite(pFd->pReal, zBuf, iAmt, iOfst);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Truncate an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsTruncate(sqlite3_file *pFile, sqlite_int64 ಗಾತ್ರ_2){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ದತ್ತಾಂಶ;

  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_TRUNCATE_ಮುಖವಾಡ ){
    tvfsExecTcl(p, "xTruncate", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId, 0, 0
    );
    tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(p, &rc);
  }
  
  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3OsTruncate(pFd->pReal, ಗಾತ್ರ_2);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Sync an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsSync(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ flags){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ದತ್ತಾಂಶ;

  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_SYNC_ಮುಖವಾಡ ){
    ಅಕ್ಷರ *zFlags = 0;

    ಆಯ್ಕೆ( flags ){
      ಸ್ಥಿತಿ SQLITE_SYNC_ಸಾಮಾನ್ಯ:
        zFlags = "normal";
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_SYNC_ತುಂಬಿದ:
        zFlags = "full";
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_SYNC_ಸಾಮಾನ್ಯ|SQLITE_SYNC_DATAONLY:
        zFlags = "normal|dataonly";
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ SQLITE_SYNC_ತುಂಬಿದ|SQLITE_SYNC_DATAONLY:
        zFlags = "full|dataonly";
        ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ:
        ಪ್ರತಿಜ್ಞಾ(0);
    }

    tvfsExecTcl(p, "xSync", 
        Tcl_NewStringObj(pFd->zFilename, -1), pFd->pShmId,
        Tcl_NewStringObj(zFlags, -1), 0
    );
    tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(p, &rc);
  }

  ಆದರೆ( rc==SQLITE_OK && tvfsInjectFullerr(p) ) rc = SQLITE_ತುಂಬಿದ;

  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3OsSync(pFd->pReal, flags);
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return the current file-size of an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_File_ಗಾತ್ರ(sqlite3_file *pFile, sqlite_int64 *p_ಗಾತ್ರ){
  TestvfsFd *p = tvfsGetFd(pFile);
  ಹಿಂತಿರುಗಿ sqlite3_Os_File_ಗಾತ್ರ(p->pReal, p_ಗಾತ್ರ);
}

/*
** Lock an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ದತ್ತಾಂಶ;
  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_ಬೀಗ_ಮುಖವಾಡ ){
    ಅಕ್ಷರ z_ಬೀಗ[30];
    sqlite3_snprintf(ಗಾತ್ರ(z_ಬೀಗ),z_ಬೀಗ,"%d",e_ಬೀಗ);
    tvfsExecTcl(p, "xLock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(z_ಬೀಗ, -1), 0, 0);
  }
  ಆದರೆ( p->ಮುಖವಾಡ&TESTVFS_ಬೀಗ_ಮುಖವಾಡ && tvfsInjectIoerr(p) ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_ಬೀಗ;
  }
  ಹಿಂತಿರುಗಿ sqlite3_Os_ಬೀಗ(pFd->pReal, e_ಬೀಗ);
}

/*
** Unlock an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsUnlock(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ e_ಬೀಗ){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ದತ್ತಾಂಶ;
  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_UNLOCK_ಮುಖವಾಡ ){
    ಅಕ್ಷರ z_ಬೀಗ[30];
    sqlite3_snprintf(ಗಾತ್ರ(z_ಬೀಗ),z_ಬೀಗ,"%d",e_ಬೀಗ);
    tvfsExecTcl(p, "xUnlock", Tcl_NewStringObj(pFd->zFilename, -1), 
                   Tcl_NewStringObj(z_ಬೀಗ, -1), 0, 0);
  }
  ಆದರೆ( p->ಮುಖವಾಡ&TESTVFS_UNLOCK_ಮುಖವಾಡ && tvfsInjectIoerr(p) ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR_UNLOCK;
  }
  ಹಿಂತಿರುಗಿ sqlite3OsUnlock(pFd->pReal, e_ಬೀಗ);
}

/*
** Check if another file-handle holds a RESERVED lock on an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_Check_Reserved_ಬೀಗ(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ದತ್ತಾಂಶ;
  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_CKLOCK_ಮುಖವಾಡ ){
    tvfsExecTcl(p, "xCheckReservedLock", Tcl_NewStringObj(pFd->zFilename, -1),
                   0, 0, 0);
  }
  ಹಿಂತಿರುಗಿ sqlite3_Os_Check_Reserved_ಬೀಗ(pFd->pReal, p_Res_ಹೊರಹೋಗು);
}

/*
** File control method. For custom operations on an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsFileControl(sqlite3_file *pFile, ಪೂರ್ಣಾಂಕ op, ಶೂನ್ಯ *pArg){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ದತ್ತಾಂಶ;
  ಆದರೆ( op==SQLITE_FCNTL_PRAGMA ){
    ಅಕ್ಷರ **argv = (ಅಕ್ಷರ**)pArg;
    ಆದರೆ( sqlite3_stricmp(argv[1],"error")==0 ){
      ಪೂರ್ಣಾಂಕ rc = SQLITE_ERROR;
      ಆದರೆ( argv[2] ){
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = argv[2];
        ಪೂರ್ಣಾಂಕ x = atoi(z);
        ಆದರೆ( x ){
          rc = x;
          ಆಗ( sqlite3Isdigit(z[0]) ){ z++; }
          ಆಗ( sqlite3Isspace(z[0]) ){ z++; }
        }
        ಆದರೆ( z[0] ) argv[0] = sqlite3_mprintf("%s", z);
      }
      ಹಿಂತಿರುಗಿ rc;
    }
    ಆದರೆ( sqlite3_stricmp(argv[1], "filename")==0 ){
      argv[0] = sqlite3_mprintf("%s", pFd->zFilename);
      ಹಿಂತಿರುಗಿ SQLITE_OK;
    }
  }
  ಆದರೆ( p->pScript && (p->ಮುಖವಾಡ&TESTVFS_FCNTL_ಮುಖವಾಡ) ){
    ರಚನೆ Fcntl {
      ಪೂರ್ಣಾಂಕ iFnctl;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFnctl;
    } aF[] = {
      { SQLITE_FCNTL_BEGIN_ATOMIC_WRITE, "BEGIN_ATOMIC_WRITE" },
      { SQLITE_FCNTL_COMMIT_ATOMIC_WRITE, "COMMIT_ATOMIC_WRITE" },
      { SQLITE_FCNTL_ZIPVFS, "ZIPVFS" },
    };
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<ಗಾತ್ರ(aF)/ಗಾತ್ರ(aF[0]); i++){
      ಆದರೆ( op==aF[i].iFnctl ) ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ( i<ಗಾತ್ರ(aF)/ಗಾತ್ರ(aF[0]) ){
      ಪೂರ್ಣಾಂಕ rc = 0;
      tvfsExecTcl(p, "xFileControl", 
          Tcl_NewStringObj(pFd->zFilename, -1), 
          Tcl_NewStringObj(aF[i].zFnctl, -1),
          0, 0
      );
      tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(p, &rc);
      ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ (rc<0 ? SQLITE_OK : rc);
    }
  }
  ಹಿಂತಿರುಗಿ sqlite3OsFileControl(pFd->pReal, op, pArg);
}

/*
** Return the sector-size in bytes for an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_Sector_ಗಾತ್ರ(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ದತ್ತಾಂಶ;
  ಆದರೆ( p->iSectorsize>=0 ){
    ಹಿಂತಿರುಗಿ p->iSectorsize;
  }
  ಹಿಂತಿರುಗಿ sqlite3_Os_Sector_ಗಾತ್ರ(pFd->pReal);
}

/*
** Return the device characteristic flags supported by an tvfs-file.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsDeviceCharacteristics(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)pFd->pVfs->p_App_ದತ್ತಾಂಶ;
  ಆದರೆ( p->iDevchar>=0 ){
    ಹಿಂತಿರುಗಿ p->iDevchar;
  }
  ಹಿಂತಿರುಗಿ sqlite3OsDeviceCharacteristics(pFd->pReal);
}

/*
** Open an tvfs file handle.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsOpen(
  sqlite3_vfs *pVfs,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName,
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಂಕ flags,
  ಪೂರ್ಣಾಂಕ *p_ಹೊರಹೋಗು_Flags
){
  ಪೂರ್ಣಾಂಕ rc;
  TestvfsFile *pTestfile = (TestvfsFile *)pFile;
  TestvfsFd *pFd;
  Tcl_Obj *pId = 0;
  Testvfs *p = (Testvfs *)pVfs->p_App_ದತ್ತಾಂಶ;

  pFd = (TestvfsFd *)ckalloc(ಗಾತ್ರ(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  memset(pFd, 0, ಗಾತ್ರ(TestvfsFd) + PARENTVFS(pVfs)->szOsFile);
  pFd->pShm = 0;
  pFd->pShmId = 0;
  pFd->zFilename = zName;
  pFd->pVfs = pVfs;
  pFd->pReal = (sqlite3_file *)&pFd[1];
  memset(pTestfile, 0, ಗಾತ್ರ(TestvfsFile));
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
  Tcl_Reset_ಫಲಿತಾಂಶ(p->interp);
  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_OPEN_ಮುಖವಾಡ ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_ಎಣಿಕೆ(pArg);
    ಆದರೆ( flags&SQLITE_OPEN_MAIN_DB ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = &zName[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zName)+1];
      ಆಗ( *z ){
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z) + 1;
        Tcl_ListObjAppendElement(0, pArg, Tcl_NewStringObj(z, -1));
        z += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z) + 1;
      }
    }
    tvfsExecTcl(p, "xOpen", Tcl_NewStringObj(pFd->zFilename, -1), pArg, 0, 0);
    Tcl_Decr_Ref_ಎಣಿಕೆ(pArg);
    ಆದರೆ( tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(p, &rc) ){
      ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pId = Tcl_Get_Obj_ಫಲಿತಾಂಶ(p->interp);
    }
  }

  ಆದರೆ( (p->ಮುಖವಾಡ&TESTVFS_OPEN_ಮುಖವಾಡ) &&  tvfsInjectIoerr(p) ) ಹಿಂತಿರುಗಿ SQLITE_IOERR;
  ಆದರೆ( tvfsInjectCantopenerr(p) ) ಹಿಂತಿರುಗಿ SQLITE_CANTOPEN;
  ಆದರೆ( tvfsInjectFullerr(p) ) ಹಿಂತಿರುಗಿ SQLITE_ತುಂಬಿದ;

  ಆದರೆ( !pId ){
    pId = Tcl_NewStringObj("anon", -1);
  }
  Tcl_Incr_Ref_ಎಣಿಕೆ(pId);
  pFd->pShmId = pId;
  Tcl_Reset_ಫಲಿತಾಂಶ(p->interp);

  rc = sqlite3OsOpen(PARENTVFS(pVfs), zName, pFd->pReal, flags, p_ಹೊರಹೋಗು_Flags);
  ಆದರೆ( pFd->pReal->pMethods ){
    sqlite3_io_methods *pMethods;
    ಪೂರ್ಣಾಂಕ nByte;

    ಆದರೆ( pVfs->i_ಆವೃತ್ತಿ>1 ){
      nByte = ಗಾತ್ರ(sqlite3_io_methods);
    }ಇಲ್ಲದಿದ್ದರೆ{
      nByte = offsetof(sqlite3_io_methods, xShmMap);
    }

    pMethods = (sqlite3_io_methods *)ckalloc(nByte);
    ಸ್ಮರಣೆ_ನಕಲು(pMethods, &tvfs_io_methods, nByte);
    pMethods->i_ಆವೃತ್ತಿ = pFd->pReal->pMethods->i_ಆವೃತ್ತಿ;
    ಆದರೆ( pMethods->i_ಆವೃತ್ತಿ>pVfs->i_ಆವೃತ್ತಿ ){
      pMethods->i_ಆವೃತ್ತಿ = pVfs->i_ಆವೃತ್ತಿ;
    }
    ಆದರೆ( pVfs->i_ಆವೃತ್ತಿ>1 && ((Testvfs *)pVfs->p_App_ದತ್ತಾಂಶ)->isNoshm ){
      pMethods->xShmUnmap = 0;
      pMethods->x_Shm_ಬೀಗ = 0;
      pMethods->xShmBarrier = 0;
      pMethods->xShmMap = 0;
    }
    pFile->pMethods = pMethods;
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsDelete(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, ಪೂರ್ಣಾಂಕ dirSync){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  Testvfs *p = (Testvfs *)pVfs->p_App_ದತ್ತಾಂಶ;

  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_DELETE_ಮುಖವಾಡ ){
    tvfsExecTcl(p, "xDelete", 
        Tcl_NewStringObj(z_ಮಾರ್ಗ, -1), Tcl_NewIntObj(dirSync), 0, 0
    );
    tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(p, &rc);
  }
  ಆದರೆ( rc==SQLITE_OK ){
    rc = sqlite3OsDelete(PARENTVFS(pVfs), z_ಮಾರ್ಗ, dirSync);
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsAccess(
  sqlite3_vfs *pVfs, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, 
  ಪೂರ್ಣಾಂಕ flags, 
  ಪೂರ್ಣಾಂಕ *p_Res_ಹೊರಹೋಗು
){
  Testvfs *p = (Testvfs *)pVfs->p_App_ದತ್ತಾಂಶ;
  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_ACCESS_ಮುಖವಾಡ ){
    ಪೂರ್ಣಾಂಕ rc;
    ಅಕ್ಷರ *zArg = 0;
    ಆದರೆ( flags==SQLITE_ACCESS_EXISTS ) zArg = "SQLITE_ACCESS_EXISTS";
    ಆದರೆ( flags==SQLITE_ACCESS_READWRITE ) zArg = "SQLITE_ACCESS_READWRITE";
    ಆದರೆ( flags==SQLITE_ACCESS_READ ) zArg = "SQLITE_ACCESS_READ";
    tvfsExecTcl(p, "xAccess", 
        Tcl_NewStringObj(z_ಮಾರ್ಗ, -1), Tcl_NewStringObj(zArg, -1), 0, 0
    );
    ಆದರೆ( tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(p, &rc) ){
      ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;
    }ಇಲ್ಲದಿದ್ದರೆ{
      Tcl_Interp *interp = p->interp;
      ಆದರೆ( TCL_OK==Tcl_GetBooleanFromObj(0, Tcl_Get_Obj_ಫಲಿತಾಂಶ(interp), p_Res_ಹೊರಹೋಗು) ){
        ಹಿಂತಿರುಗಿ SQLITE_OK;
      }
    }
  }
  ಹಿಂತಿರುಗಿ sqlite3OsAccess(PARENTVFS(pVfs), z_ಮಾರ್ಗ, flags, p_Res_ಹೊರಹೋಗು);
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (DEVSYM_MAX_PATHNAME+1) bytes.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_ತುಂಬಿದ_Pathname(
  sqlite3_vfs *pVfs, 
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ, 
  ಪೂರ್ಣಾಂಕ n_ಹೊರಹೋಗು, 
  ಅಕ್ಷರ *z_ಹೊರಹೋಗು
){
  Testvfs *p = (Testvfs *)pVfs->p_App_ದತ್ತಾಂಶ;
  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_FULLPATHNAME_ಮುಖವಾಡ ){
    ಪೂರ್ಣಾಂಕ rc;
    tvfsExecTcl(p, "xFullPathname", Tcl_NewStringObj(z_ಮಾರ್ಗ, -1), 0, 0, 0);
    ಆದರೆ( tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(p, &rc) ){
      ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;
    }
  }
  ಹಿಂತಿರುಗಿ sqlite3_Os_ತುಂಬಿದ_Pathname(PARENTVFS(pVfs), z_ಮಾರ್ಗ, n_ಹೊರಹೋಗು, z_ಹೊರಹೋಗು);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
/*
** Open the dynamic library located at zPath and return a handle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ *tvfsDlOpen(sqlite3_vfs *pVfs, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ){
  ಹಿಂತಿರುಗಿ sqlite3OsDlOpen(PARENTVFS(pVfs), z_ಮಾರ್ಗ);
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated 
** with dynamic libraries.
*/
ಸ್ಥಿರ ಶೂನ್ಯ tvfsDlError(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Err_ಸಂದೇಶ){
  sqlite3OsDlError(PARENTVFS(pVfs), nByte, z_Err_ಸಂದೇಶ);
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ (*tvfsDlSym(sqlite3_vfs *pVfs, ಶೂನ್ಯ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSym))(ಶೂನ್ಯ){
  ಹಿಂತಿರುಗಿ sqlite3OsDlSym(PARENTVFS(pVfs), p, zSym);
}

/*
** Close the dynamic library handle pHandle.
*/
ಸ್ಥಿರ ಶೂನ್ಯ tvfsDlClose(sqlite3_vfs *pVfs, ಶೂನ್ಯ *pHandle){
  sqlite3OsDlClose(PARENTVFS(pVfs), pHandle);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_LOAD_EXTENSION */

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of 
** random data.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsRandomness(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nByte, ಅಕ್ಷರ *z_Buf_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ sqlite3OsRandomness(PARENTVFS(pVfs), nByte, z_Buf_ಹೊರಹೋಗು);
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds 
** actually slept.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsSleep(sqlite3_vfs *pVfs, ಪೂರ್ಣಾಂಕ nMicro){
  Testvfs *p = (Testvfs *)pVfs->p_App_ದತ್ತಾಂಶ;
  ಆದರೆ( p->pScript && (p->ಮುಖವಾಡ&TESTVFS_SLEEP_ಮುಖವಾಡ) ){
    tvfsExecTcl(p, "xSleep", Tcl_NewIntObj(nMicro), 0, 0, 0);
  }
  ಹಿಂತಿರುಗಿ sqlite3OsSleep(PARENTVFS(pVfs), nMicro);
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_Current_ಸಮಯ(sqlite3_vfs *pVfs, ದ್ವಿಗುಣ *p_ಸಮಯ_ಹೊರಹೋಗು){
  ಹಿಂತಿರುಗಿ PARENTVFS(pVfs)->x_Current_ಸಮಯ(PARENTVFS(pVfs), p_ಸಮಯ_ಹೊರಹೋಗು);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsShmOpen(sqlite3_file *pFile){
  Testvfs *p;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;             /* Return code */
  Testvfs_ಬಫರ್ *p_ಬಫರ್;         /* Buffer to open connection to */
  TestvfsFd *pFd;                 /* The testvfs file structure */

  pFd = tvfsGetFd(pFile);
  p = (Testvfs *)pFd->pVfs->p_App_ದತ್ತಾಂಶ;
  ಪ್ರತಿಜ್ಞಾ( 0==p->isFullshm );
  ಪ್ರತಿಜ್ಞಾ( pFd->pShmId && pFd->pShm==0 && pFd->p_ಮುಂದಿನ==0 );

  /* Evaluate the Tcl script: 
  **
  **   SCRIPT xShmOpen FILENAME
  */
  Tcl_Reset_ಫಲಿತಾಂಶ(p->interp);
  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_SHMOPEN_ಮುಖವಾಡ ){
    tvfsExecTcl(p, "xShmOpen", Tcl_NewStringObj(pFd->zFilename, -1), 0, 0, 0);
    ಆದರೆ( tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(p, &rc) ){
      ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;
    }
  }

  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK );
  ಆದರೆ( p->ಮುಖವಾಡ&TESTVFS_SHMOPEN_ಮುಖವಾಡ && tvfsInjectIoerr(p) ){
    ಹಿಂತಿರುಗಿ SQLITE_IOERR;
  }

  /* Search for a TestvfsBuffer. Create a new one if required. */
  ಗಾಗಿ(p_ಬಫರ್=p->p_ಬಫರ್; p_ಬಫರ್; p_ಬಫರ್=p_ಬಫರ್->p_ಮುಂದಿನ){
    ಆದರೆ( 0==strcmp(pFd->zFilename, p_ಬಫರ್->zFile) ) ನಿಲ್ಲಿಸು;
  }
  ಆದರೆ( !p_ಬಫರ್ ){
    ಪೂರ್ಣಾಂಕ szName = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(pFd->zFilename);
    ಪೂರ್ಣಾಂಕ nByte = ಗಾತ್ರ(Testvfs_ಬಫರ್) + szName + 1;
    p_ಬಫರ್ = (Testvfs_ಬಫರ್ *)ckalloc(nByte);
    memset(p_ಬಫರ್, 0, nByte);
    p_ಬಫರ್->zFile = (ಅಕ್ಷರ *)&p_ಬಫರ್[1];
    ಸ್ಮರಣೆ_ನಕಲು(p_ಬಫರ್->zFile, pFd->zFilename, szName+1);
    p_ಬಫರ್->p_ಮುಂದಿನ = p->p_ಬಫರ್;
    p->p_ಬಫರ್ = p_ಬಫರ್;
  }

  /* Connect the TestvfsBuffer to the new TestvfsShm handle and return. */
  pFd->p_ಮುಂದಿನ = p_ಬಫರ್->pFile;
  p_ಬಫರ್->pFile = pFd;
  pFd->pShm = p_ಬಫರ್;
  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಶೂನ್ಯ tvfs_Alloc_ಪುಟ(Testvfs_ಬಫರ್ *p, ಪೂರ್ಣಾಂಕ i_ಪುಟ, ಪೂರ್ಣಾಂಕ pgsz){
  ಪ್ರತಿಜ್ಞಾ( i_ಪುಟ<TESTVFS_MAX_ಪುಟಗಳು );
  ಆದರೆ( p->a_ಪುಟ[i_ಪುಟ]==0 ){
    p->a_ಪುಟ[i_ಪುಟ] = (u8 *)ckalloc(pgsz);
    memset(p->a_ಪುಟ[i_ಪುಟ], 0, pgsz);
    p->pgsz = pgsz;
  }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsShmMap(
  sqlite3_file *pFile,            /* Handle open on database file */
  ಪೂರ್ಣಾಂಕ i_ಪುಟ,                      /* Page to retrieve */
  ಪೂರ್ಣಾಂಕ pgsz,                       /* Size of pages */
  ಪೂರ್ಣಾಂಕ isWrite,                    /* True to extend file if necessary */
  ಶೂನ್ಯ ಅಸ್ಥಿರ **pp              /* OUT: Mapped memory */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_ದತ್ತಾಂಶ);

  ಆದರೆ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ಹಿಂತಿರುಗಿ pReal->pMethods->xShmMap(pReal, i_ಪುಟ, pgsz, isWrite, pp);
  }

  ಆದರೆ( 0==pFd->pShm ){
    rc = tvfsShmOpen(pFile);
    ಆದರೆ( rc!=SQLITE_OK ){
      ಹಿಂತಿರುಗಿ rc;
    }
  }

  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_SHMMAP_ಮುಖವಾಡ ){
    Tcl_Obj *pArg = Tcl_NewObj();
    Tcl_Incr_Ref_ಎಣಿಕೆ(pArg);
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(i_ಪುಟ));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(pgsz));
    Tcl_ListObjAppendElement(p->interp, pArg, Tcl_NewIntObj(isWrite));
    tvfsExecTcl(p, "xShmMap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, pArg, 0
    );
    tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(p, &rc);
    Tcl_Decr_Ref_ಎಣಿಕೆ(pArg);
  }
  ಆದರೆ( rc==SQLITE_OK && p->ಮುಖವಾಡ&TESTVFS_SHMMAP_ಮುಖವಾಡ && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ಆದರೆ( rc==SQLITE_OK && isWrite && !pFd->pShm->a_ಪುಟ[i_ಪುಟ] ){
    tvfs_Alloc_ಪುಟ(pFd->pShm, i_ಪುಟ, pgsz);
  }
  ಆದರೆ( rc==SQLITE_OK || rc==SQLITE_READONLY ){
    *pp = (ಶೂನ್ಯ ಅಸ್ಥಿರ *)pFd->pShm->a_ಪುಟ[i_ಪುಟ];
  }

  ಹಿಂತಿರುಗಿ rc;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfs_Shm_ಬೀಗ(
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಂಕ ofst,
  ಪೂರ್ಣಾಂಕ n,
  ಪೂರ್ಣಾಂಕ flags
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_ದತ್ತಾಂಶ);
  ಪೂರ್ಣಾಂಕ n_ಬೀಗ;
  ಅಕ್ಷರ z_ಬೀಗ[80];

  ಆದರೆ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ಹಿಂತಿರುಗಿ pReal->pMethods->x_Shm_ಬೀಗ(pReal, ofst, n, flags);
  }

  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_SHMLOCK_ಮುಖವಾಡ ){
    sqlite3_snprintf(ಗಾತ್ರ(z_ಬೀಗ), z_ಬೀಗ, "%d %d", ofst, n);
    n_ಬೀಗ = (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ಬೀಗ);
    ಆದರೆ( flags & SQLITE_SHM_ಬೀಗ ){
      strcpy(&z_ಬೀಗ[n_ಬೀಗ], " lock");
    }ಇಲ್ಲದಿದ್ದರೆ{
      strcpy(&z_ಬೀಗ[n_ಬೀಗ], " unlock");
    }
    n_ಬೀಗ += (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(&z_ಬೀಗ[n_ಬೀಗ]);
    ಆದರೆ( flags & SQLITE_SHM_SHARED ){
      strcpy(&z_ಬೀಗ[n_ಬೀಗ], " shared");
    }ಇಲ್ಲದಿದ್ದರೆ{
      strcpy(&z_ಬೀಗ[n_ಬೀಗ], " exclusive");
    }
    tvfsExecTcl(p, "xShmLock", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId,
        Tcl_NewStringObj(z_ಬೀಗ, -1), 0
    );
    tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(p, &rc);
  }

  ಆದರೆ( rc==SQLITE_OK && p->ಮುಖವಾಡ&TESTVFS_SHMLOCK_ಮುಖವಾಡ && tvfsInjectIoerr(p) ){
    rc = SQLITE_IOERR;
  }

  ಆದರೆ( rc==SQLITE_OK ){
    ಪೂರ್ಣಾಂಕ is_ಬೀಗ = (flags & SQLITE_SHM_ಬೀಗ);
    ಪೂರ್ಣಾಂಕ isExcl = (flags & SQLITE_SHM_EXCLUSIVE);
    u32 ಮುಖವಾಡ = (((1<<n)-1) << ofst);
    ಆದರೆ( is_ಬೀಗ ){
      TestvfsFd *p2;
      ಗಾಗಿ(p2=pFd->pShm->pFile; p2; p2=p2->p_ಮುಂದಿನ){
        ಆದರೆ( p2==pFd ) ಮುಂದುವರಿ;
        ಆದರೆ( (p2->excllock&ಮುಖವಾಡ) || (isExcl && p2->sharedlock&ಮುಖವಾಡ) ){
          rc = SQLITE_BUSY;
          ನಿಲ್ಲಿಸು;
        }
      }
      ಆದರೆ( rc==SQLITE_OK ){
        ಆದರೆ( isExcl )  pFd->excllock |= ಮುಖವಾಡ;
        ಆದರೆ( !isExcl ) pFd->sharedlock |= ಮುಖವಾಡ;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( isExcl )  pFd->excllock &= (~ಮುಖವಾಡ);
      ಆದರೆ( !isExcl ) pFd->sharedlock &= (~ಮುಖವಾಡ);
    }
  }

  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಶೂನ್ಯ tvfsShmBarrier(sqlite3_file *pFile){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_ದತ್ತಾಂಶ);

  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_SHMBARRIER_ಮುಖವಾಡ ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = pFd->pShm ? pFd->pShm->zFile : "";
    tvfsExecTcl(p, "xShmBarrier", Tcl_NewStringObj(z, -1), pFd->pShmId, 0, 0);
  }

  ಆದರೆ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    pReal->pMethods->xShmBarrier(pReal);
    ಹಿಂತಿರುಗಿ;
  }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsShmUnmap(
  sqlite3_file *pFile,
  ಪೂರ್ಣಾಂಕ deleteFlag
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  TestvfsFd *pFd = tvfsGetFd(pFile);
  Testvfs *p = (Testvfs *)(pFd->pVfs->p_App_ದತ್ತಾಂಶ);
  Testvfs_ಬಫರ್ *p_ಬಫರ್ = pFd->pShm;
  TestvfsFd **ppFd;

  ಆದರೆ( p->isFullshm ){
    sqlite3_file *pReal = pFd->pReal;
    ಹಿಂತಿರುಗಿ pReal->pMethods->xShmUnmap(pReal, deleteFlag);
  }

  ಆದರೆ( !p_ಬಫರ್ ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  ಪ್ರತಿಜ್ಞಾ( pFd->pShmId && pFd->pShm );

  ಆದರೆ( p->pScript && p->ಮುಖವಾಡ&TESTVFS_SHMCLOSE_ಮುಖವಾಡ ){
    tvfsExecTcl(p, "xShmUnmap", 
        Tcl_NewStringObj(pFd->pShm->zFile, -1), pFd->pShmId, 0, 0
    );
    tvfs_ಫಲಿತಾಂಶ_ಸಂಕೇತ(p, &rc);
  }

  ಗಾಗಿ(ppFd=&p_ಬಫರ್->pFile; *ppFd!=pFd; ppFd=&((*ppFd)->p_ಮುಂದಿನ));
  ಪ್ರತಿಜ್ಞಾ( (*ppFd)==pFd );
  *ppFd = pFd->p_ಮುಂದಿನ;
  pFd->p_ಮುಂದಿನ = 0;

  ಆದರೆ( p_ಬಫರ್->pFile==0 ){
    ಪೂರ್ಣಾಂಕ i;
    Testvfs_ಬಫರ್ **pp;
    ಗಾಗಿ(pp=&p->p_ಬಫರ್; *pp!=p_ಬಫರ್; pp=&((*pp)->p_ಮುಂದಿನ));
    *pp = (*pp)->p_ಮುಂದಿನ;
    ಗಾಗಿ(i=0; p_ಬಫರ್->a_ಪುಟ[i]; i++){
      ckfree((ಅಕ್ಷರ *)p_ಬಫರ್->a_ಪುಟ[i]);
    }
    ckfree((ಅಕ್ಷರ *)p_ಬಫರ್);
  }
  pFd->pShm = 0;

  ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsFetch(
    sqlite3_file *pFile, 
    sqlite3_int64 iOfst, 
    ಪೂರ್ಣಾಂಕ iAmt, 
    ಶೂನ್ಯ **pp
){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  ಹಿಂತಿರುಗಿ sqlite3OsFetch(pFd->pReal, iOfst, iAmt, pp);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tvfsUnfetch(sqlite3_file *pFile, sqlite3_int64 iOfst, ಶೂನ್ಯ *p){
  TestvfsFd *pFd = tvfsGetFd(pFile);
  ಹಿಂತಿರುಗಿ sqlite3OsUnfetch(pFd->pReal, iOfst, p);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI testvfs_obj_cmd(
  Client_ದತ್ತಾಂಶ cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  Testvfs *p = (Testvfs *)cd;

  ಎಣಿಕೆ DB_enum { 
    CMD_SHM, CMD_DELETE, CMD_FILTER, CMD_IOERR, CMD_SCRIPT, 
    CMD_DEVCHAR, CMD_SECTORSIZE, CMD_FULLERR, CMD_CANTOPENERR
  };
  ರಚನೆ TestvfsSubcmd {
    ಅಕ್ಷರ *zName;
    ಎಣಿಕೆ DB_enum eCmd;
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
  ಪೂರ್ಣಾಂಕ i;
  
  ಆದರೆ( objc<2 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "SUBCOMMAND ...");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj_Struct(
        interp, objv[1], aSubcmd, ಗಾತ್ರ(aSubcmd[0]), "subcommand", 0, &i) 
  ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  Tcl_Reset_ಫಲಿತಾಂಶ(interp);

  ಆಯ್ಕೆ( aSubcmd[i].eCmd ){
    ಸ್ಥಿತಿ CMD_SHM: {
      Tcl_Obj *pObj;
      ಪೂರ್ಣಾಂಕ rc;
      Testvfs_ಬಫರ್ *p_ಬಫರ್;
      ಅಕ್ಷರ *zName;
      ಆದರೆ( objc!=3 && objc!=4 ){
        Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "FILE ?VALUE?");
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      zName = ckalloc(p->p_ಪೋಷಕ->mxPathname);
      rc = p->p_ಪೋಷಕ->x_ತುಂಬಿದ_Pathname(
          p->p_ಪೋಷಕ, Tcl_GetString(objv[2]), 
          p->p_ಪೋಷಕ->mxPathname, zName
      );
      ಆದರೆ( rc!=SQLITE_OK ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, "failed to get full path: ",
                         Tcl_GetString(objv[2]), ಖಾಲಿ);
        ckfree(zName);
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ಗಾಗಿ(p_ಬಫರ್=p->p_ಬಫರ್; p_ಬಫರ್; p_ಬಫರ್=p_ಬಫರ್->p_ಮುಂದಿನ){
        ಆದರೆ( 0==strcmp(p_ಬಫರ್->zFile, zName) ) ನಿಲ್ಲಿಸು;
      }
      ckfree(zName);
      ಆದರೆ( !p_ಬಫರ್ ){
        Tcl_Append_ಫಲಿತಾಂಶ(interp, "no such file: ", Tcl_GetString(objv[2]), ಖಾಲಿ);
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ಆದರೆ( objc==4 ){
        Tcl_ಗಾತ್ರ n;
        u8 *a = Tcl_GetByteArrayFromObj(objv[3], &n);
        ಪೂರ್ಣಾಂಕ pgsz = p_ಬಫರ್->pgsz;
        ಆದರೆ( pgsz==0 ) pgsz = 65536;
        ಗಾಗಿ(i=0; i*pgsz<(ಪೂರ್ಣಾಂಕ)n; i++){
          ಪೂರ್ಣಾಂಕ nByte = pgsz;
          tvfs_Alloc_ಪುಟ(p_ಬಫರ್, i, pgsz);
          ಆದರೆ( n-i*pgsz<pgsz ){
            nByte = (ಪೂರ್ಣಾಂಕ)n;
          }
          ಸ್ಮರಣೆ_ನಕಲು(p_ಬಫರ್->a_ಪುಟ[i], &a[i*pgsz], nByte);
        }
      }

      pObj = Tcl_NewObj();
      ಗಾಗಿ(i=0; p_ಬಫರ್->a_ಪುಟ[i]; i++){
        ಪೂರ್ಣಾಂಕ pgsz = p_ಬಫರ್->pgsz;
        ಆದರೆ( pgsz==0 ) pgsz = 65536;
        Tcl_Append_Obj_ಗಮ್ಯ_Obj(pObj, Tcl_NewByteArrayObj(p_ಬಫರ್->a_ಪುಟ[i], pgsz));
      }
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pObj);
      ನಿಲ್ಲಿಸು;
    }

    /*  TESTVFS filter METHOD-LIST
    **
    **     Activate special processing for those methods contained in the list
    */
    ಸ್ಥಿತಿ CMD_FILTER: {
      ಸ್ಥಿರ ರಚನೆ Vfs_ವಿಧಾನ {
        ಅಕ್ಷರ *zName;
        ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ;
      } vfsmethod [] = {
        { "xShmOpen",           TESTVFS_SHMOPEN_ಮುಖವಾಡ },
        { "xShmLock",           TESTVFS_SHMLOCK_ಮುಖವಾಡ },
        { "xShmBarrier",        TESTVFS_SHMBARRIER_ಮುಖವಾಡ },
        { "xShmUnmap",          TESTVFS_SHMCLOSE_ಮುಖವಾಡ },
        { "xShmMap",            TESTVFS_SHMMAP_ಮುಖವಾಡ },
        { "xSync",              TESTVFS_SYNC_ಮುಖವಾಡ },
        { "xDelete",            TESTVFS_DELETE_ಮುಖವಾಡ },
        { "xWrite",             TESTVFS_WRITE_ಮುಖವಾಡ },
        { "xRead",              TESTVFS_READ_ಮುಖವಾಡ },
        { "xTruncate",          TESTVFS_TRUNCATE_ಮುಖವಾಡ },
        { "xOpen",              TESTVFS_OPEN_ಮುಖವಾಡ },
        { "xClose",             TESTVFS_CLOSE_ಮುಖವಾಡ },
        { "xAccess",            TESTVFS_ACCESS_ಮುಖವಾಡ },
        { "xFullPathname",      TESTVFS_FULLPATHNAME_ಮುಖವಾಡ },
        { "xUnlock",            TESTVFS_UNLOCK_ಮುಖವಾಡ },
        { "xLock",              TESTVFS_ಬೀಗ_ಮುಖವಾಡ },
        { "xCheckReservedLock", TESTVFS_CKLOCK_ಮುಖವಾಡ },
        { "xFileControl",       TESTVFS_FCNTL_ಮುಖವಾಡ },
        { "xSleep",             TESTVFS_SLEEP_ಮುಖವಾಡ },
      };
      Tcl_Obj **apElem = 0;
      Tcl_ಗಾತ್ರ nElem = 0;
      ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ = 0;
      ಆದರೆ( objc!=3 ){
        Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "LIST");
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ಆದರೆ( Tcl_ListObjGetElements(interp, objv[2], &nElem, &apElem) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      Tcl_Reset_ಫಲಿತಾಂಶ(interp);
      ಗಾಗಿ(i=0; i<(ಪೂರ್ಣಾಂಕ)nElem; i++){
        ಪೂರ್ಣಾಂಕ i_ವಿಧಾನ;
        ಅಕ್ಷರ *zElem = Tcl_GetString(apElem[i]);
        ಗಾಗಿ(i_ವಿಧಾನ=0; i_ವಿಧಾನ<Array_ಗಾತ್ರ(vfsmethod); i_ವಿಧಾನ++){
          ಆದರೆ( strcmp(zElem, vfsmethod[i_ವಿಧಾನ].zName)==0 ){
            ಮುಖವಾಡ |= vfsmethod[i_ವಿಧಾನ].ಮುಖವಾಡ;
            ನಿಲ್ಲಿಸು;
          }
        }
        ಆದರೆ( i_ವಿಧಾನ==Array_ಗಾತ್ರ(vfsmethod) ){
          Tcl_Append_ಫಲಿತಾಂಶ(interp, "unknown method: ", zElem, ಖಾಲಿ);
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
      }
      p->ಮುಖವಾಡ = ಮುಖವಾಡ;
      ನಿಲ್ಲಿಸು;
    }

    /*
    **  TESTVFS script ?SCRIPT?
    **
    **  Query or set the script to be run when filtered VFS events
    **  occur.
    */
    ಸ್ಥಿತಿ CMD_SCRIPT: {
      ಆದರೆ( objc==3 ){
        Tcl_ಗಾತ್ರ nByte;
        ಆದರೆ( p->pScript ){
          Tcl_Decr_Ref_ಎಣಿಕೆ(p->pScript);
          p->pScript = 0;
        }
        Tcl_GetStringFromObj(objv[2], &nByte);
        ಆದರೆ( nByte>0 ){
          p->pScript = Tcl_DuplicateObj(objv[2]);
          Tcl_Incr_Ref_ಎಣಿಕೆ(p->pScript);
        }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc!=2 ){
        Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?SCRIPT?");
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }

      Tcl_Reset_ಫಲಿತಾಂಶ(interp);
      ಆದರೆ( p->pScript ) Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, p->pScript);

      ನಿಲ್ಲಿಸು;
    }

    /*
    ** TESTVFS ioerr ?IFAIL PERSIST?
    **
    **   Where IFAIL is an integer and PERSIST is boolean.
    */
    ಸ್ಥಿತಿ CMD_CANTOPENERR:
    ಸ್ಥಿತಿ CMD_IOERR:
    ಸ್ಥಿತಿ CMD_FULLERR: {
      TestFaultInject *pTest = 0;
      ಪೂರ್ಣಾಂಕ iRet;

      ಆಯ್ಕೆ( aSubcmd[i].eCmd ){
        ಸ್ಥಿತಿ CMD_IOERR: pTest = &p->ioerr_err; ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ CMD_FULLERR: pTest = &p->ತುಂಬಿದ_err; ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ CMD_CANTOPENERR: pTest = &p->cantopen_err; ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ: ಪ್ರತಿಜ್ಞಾ(0);
      }
      iRet = pTest->nFail;
      pTest->nFail = 0;
      pTest->eFault = 0;
      pTest->iCnt = 0;

      ಆದರೆ( objc==4 ){
        ಪೂರ್ಣಾಂಕ iCnt, iPersist;
        ಆದರೆ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &iCnt)
         || TCL_OK!=Tcl_GetBooleanFromObj(interp, objv[3], &iPersist)
        ){
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
        pTest->eFault = iPersist?FAULT_INJECT_PERSISTENT:FAULT_INJECT_TRANSIENT;
        pTest->iCnt = iCnt;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( objc!=2 ){
        Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?CNT PERSIST?");
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(iRet));
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ CMD_DELETE: {
      Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ CMD_DEVCHAR: {
      ರಚನೆ DeviceFlag {
        ಅಕ್ಷರ *zName;
        ಪೂರ್ಣಾಂಕ i_ಮೌಲ್ಯ;
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
        { "safe_append",           SQLITE_IOCAP_ಸುರಕ್ಷಿತ_APPEND           },
        { "undeletable_when_open", SQLITE_IOCAP_UNDELETABLE_WHEN_OPEN },
        { "powersafe_overwrite",   SQLITE_IOCAP_POWERSAFE_OVERWRITE   },
        { "immutable",             SQLITE_IOCAP_IMMUTABLE             },
        { 0, 0 }
      };
      Tcl_Obj *pRet;
      ಪೂರ್ಣಾಂಕ iFlag;

      ಆದರೆ( objc>3 ){
        Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?ATTR-LIST?");
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ಆದರೆ( objc==3 ){
        ಪೂರ್ಣಾಂಕ j;
        ಪೂರ್ಣಾಂಕ iNew = 0;
        Tcl_Obj **flags = 0;
        Tcl_ಗಾತ್ರ nFlags = 0;

        ಆದರೆ( Tcl_ListObjGetElements(interp, objv[2], &nFlags, &flags) ){
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }

        ಗಾಗಿ(j=0; j<(ಪೂರ್ಣಾಂಕ)nFlags; j++){
          ಪೂರ್ಣಾಂಕ idx = 0;
          ಆದರೆ( Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj_Struct(interp, flags[j], aFlag, 
                ಗಾತ್ರ(aFlag[0]), "flag", 0, &idx) 
          ){
            ಹಿಂತಿರುಗಿ TCL_ERROR;
          }
          ಆದರೆ( aFlag[idx].i_ಮೌಲ್ಯ<0 && nFlags>1 ){
            Tcl_Append_ಫಲಿತಾಂಶ(interp, "bad flags: ", Tcl_GetString(objv[2]), ಖಾಲಿ);
            ಹಿಂತಿರುಗಿ TCL_ERROR;
          }
          iNew |= aFlag[idx].i_ಮೌಲ್ಯ;
        }

        p->iDevchar = iNew| 0x10000000;
      }

      pRet = Tcl_NewObj();
      ಗಾಗಿ(iFlag=0; iFlag<ಗಾತ್ರ(aFlag)/ಗಾತ್ರ(aFlag[0]); iFlag++){
        ಆದರೆ( p->iDevchar & aFlag[iFlag].i_ಮೌಲ್ಯ ){
          Tcl_ListObjAppendElement(
              interp, pRet, Tcl_NewStringObj(aFlag[iFlag].zName, -1)
          );
        }
      }
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, pRet);

      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ CMD_SECTORSIZE: {
      ಆದರೆ( objc>3 ){
        Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, "?VALUE?");
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ಆದರೆ( objc==3 ){
        ಪೂರ್ಣಾಂಕ iNew = 0;
        ಆದರೆ( Tcl_GetIntFromObj(interp, objv[2], &iNew) ){
          ಹಿಂತಿರುಗಿ TCL_ERROR;
        }
        p->iSectorsize = iNew;
      }
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(p->iSectorsize));
      ನಿಲ್ಲಿಸು;
    }
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_TCLAPI testvfs_obj_del(Client_ದತ್ತಾಂಶ cd){
  Testvfs *p = (Testvfs *)cd;
  ಆದರೆ( p->pScript ) Tcl_Decr_Ref_ಎಣಿಕೆ(p->pScript);
  sqlite3_vfs_unregister(p->pVfs);
  memset(p->pVfs, 0, ಗಾತ್ರ(sqlite3_vfs));
  ckfree((ಅಕ್ಷರ *)p->pVfs);
  memset(p, 0, ಗಾತ್ರ(Testvfs));
  ckfree((ಅಕ್ಷರ *)p);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI testvfs_cmd(
  Client_ದತ್ತಾಂಶ cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರ sqlite3_vfs tvfs_vfs = {
    3,                            /* iVersion */
    0,                            /* szOsFile */
    0,                            /* mxPathname */
    0,                            /* pNext */
    0,                            /* zName */
    0,                            /* pAppData */
    tvfsOpen,                     /* xOpen */
    tvfsDelete,                   /* xDelete */
    tvfsAccess,                   /* xAccess */
    tvfs_ತುಂಬಿದ_Pathname,             /* xFullPathname */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_ತುಂಬು_EXTENSION
    tvfsDlOpen,                   /* xDlOpen */
    tvfsDlError,                  /* xDlError */
    tvfsDlSym,                    /* xDlSym */
    tvfsDlClose,                  /* xDlClose */
#ಇಲ್ಲದಿದ್ದರೆ
    0,                            /* xDlOpen */
    0,                            /* xDlError */
    0,                            /* xDlSym */
    0,                            /* xDlClose */
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_LOAD_EXTENSION */
    tvfsRandomness,               /* xRandomness */
    tvfsSleep,                    /* xSleep */
    tvfs_Current_ಸಮಯ,              /* xCurrentTime */
    0,                            /* xGetLastError */
    0,                            /* xCurrentTimeInt64 */
    0,                            /* xSetSystemCall */
    0,                            /* xGetSystemCall */
    0,                            /* xNextSystemCall */
  };

  Testvfs *p;                     /* New object */
  sqlite3_vfs *pVfs;              /* New VFS */
  ಅಕ್ಷರ *zVfs;
  ಪೂರ್ಣಾಂಕ nByte;                      /* Bytes of space to allocate at p */

  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ isNoshm = 0;                /* True if -noshm is passed */
  ಪೂರ್ಣಾಂಕ isFullshm = 0;              /* True if -fullshm is passed */
  ಪೂರ್ಣಾಂಕ isDefault = 0;              /* True if -default is passed */
  ಪೂರ್ಣಾಂಕ szOsFile = 0;               /* Value passed to -szosfile */
  ಪೂರ್ಣಾಂಕ mxPathname = -1;            /* Value passed to -mxpathname */
  ಪೂರ್ಣಾಂಕ i_ಆವೃತ್ತಿ = 3;               /* Value passed to -iversion */

  ಆದರೆ( objc<2 || 0!=(objc%2) ) ಹೋಗು bad_ವಾದಗಳು;
  ಗಾಗಿ(i=2; i<objc; i += 2){
    Tcl_ಗಾತ್ರ nSwitch;
    ಅಕ್ಷರ *zSwitch;
    zSwitch = Tcl_GetStringFromObj(objv[i], &nSwitch); 

    ಆದರೆ( nSwitch>2 && 0==ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ("-noshm", zSwitch, nSwitch) ){
      ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isNoshm) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ಆದರೆ( isNoshm ) isFullshm = 0;
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nSwitch>2 && 0==ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ("-default", zSwitch, nSwitch) ){
      ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isDefault) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nSwitch>2 && 0==ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ("-szosfile", zSwitch, nSwitch) ){
      ಆದರೆ( Tcl_GetIntFromObj(interp, objv[i+1], &szOsFile) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nSwitch>2 && 0==ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ("-mxpathname", zSwitch, nSwitch) ){
      ಆದರೆ( Tcl_GetIntFromObj(interp, objv[i+1], &mxPathname) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nSwitch>2 && 0==ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ("-iversion", zSwitch, nSwitch) ){
      ಆದರೆ( Tcl_GetIntFromObj(interp, objv[i+1], &i_ಆವೃತ್ತಿ) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( nSwitch>2 && 0==ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ("-fullshm", zSwitch, nSwitch) ){
      ಆದರೆ( Tcl_GetBooleanFromObj(interp, objv[i+1], &isFullshm) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      ಆದರೆ( isFullshm ) isNoshm = 0;
    }
    ಇಲ್ಲದಿದ್ದರೆ{
      ಹೋಗು bad_ವಾದಗಳು;
    }
  }

  ಆದರೆ( szOsFile<ಗಾತ್ರ(TestvfsFile) ){
    szOsFile = ಗಾತ್ರ(TestvfsFile);
  }

  zVfs = Tcl_GetString(objv[1]);
  nByte = ಗಾತ್ರ(Testvfs) + (ಪೂರ್ಣಾಂಕ)ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zVfs)+1;
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
  p->p_ಪೋಷಕ = sqlite3_vfs_ಕಂಡುಹಿಡಿ(0);
  p->interp = interp;

  p->zName = (ಅಕ್ಷರ *)&p[1];
  ಸ್ಮರಣೆ_ನಕಲು(p->zName, zVfs, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zVfs)+1);

  pVfs = (sqlite3_vfs *)ckalloc(ಗಾತ್ರ(sqlite3_vfs));
  ಸ್ಮರಣೆ_ನಕಲು(pVfs, &tvfs_vfs, ಗಾತ್ರ(sqlite3_vfs));
  pVfs->p_App_ದತ್ತಾಂಶ = (ಶೂನ್ಯ *)p;
  pVfs->i_ಆವೃತ್ತಿ = i_ಆವೃತ್ತಿ;
  pVfs->zName = p->zName;
  pVfs->mxPathname = p->p_ಪೋಷಕ->mxPathname;
  ಆದರೆ( mxPathname>=0 && mxPathname<pVfs->mxPathname ){
    pVfs->mxPathname = mxPathname;
  }
  pVfs->szOsFile = szOsFile;
  p->pVfs = pVfs;
  p->isNoshm = isNoshm;
  p->isFullshm = isFullshm;
  p->ಮುಖವಾಡ = TESTVFS_ALL_ಮುಖವಾಡ;

  sqlite3_vfs_ರಿಜಿಸ್ಟರ್(pVfs, isDefault);

  ಹಿಂತಿರುಗಿ TCL_OK;

 bad_ವಾದಗಳು:
  Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "VFSNAME ?-noshm BOOL? ?-fullshm BOOL? ?-default BOOL? ?-mxpathname INT? ?-szosfile INT? ?-iversion INT?");
  ಹಿಂತಿರುಗಿ TCL_ERROR;
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp *interp, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zA, sqlite3 **ppDb);
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);

/*
** tclcmd: vfs_shmlock DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_vfs_shmlock(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azArg1[] = {"shared", "exclusive", 0};
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azArg2[] = {"lock", "unlock", 0};
  sqlite3 *db = 0;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDbname = 0;
  ಪೂರ್ಣಾಂಕ iArg1 = 0;
  ಪೂರ್ಣಾಂಕ iArg2 = 0;
  ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ = 0;
  ಪೂರ್ಣಾಂಕ n = 0;
  sqlite3_file *pFd;

  ಆದರೆ( objc!=7 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, 
        "DB DBNAME (shared|exclusive) (lock|unlock) OFFSET N"
    );
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(interp, objv[3], azArg1, "ARG", 0, &iArg1) 
   || Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj(interp, objv[4], azArg2, "ARG", 0, &iArg2) 
   || Tcl_GetIntFromObj(interp, objv[5], &i_ಸ್ಥಾನಾಂತರ)
   || Tcl_GetIntFromObj(interp, objv[6], &n)
  ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_ಸೂಚಕ, (ಶೂನ್ಯ*)&pFd);
  ಆದರೆ( pFd==0 ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  rc = pFd->pMethods->x_Shm_ಬೀಗ(pFd, i_ಸ್ಥಾನಾಂತರ, n, 
      (iArg1==0 ? SQLITE_SHM_SHARED : SQLITE_SHM_EXCLUSIVE)
    | (iArg2==0 ? SQLITE_SHM_ಬೀಗ : SQLITE_SHM_UNLOCK)
  );
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI test_vfs_ಗಣ_readmark(
  ಶೂನ್ಯ * client_ದತ್ತಾಂಶ,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db = 0;
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDbname = 0;
  ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನ = 0;
  ಪೂರ್ಣಾಂಕ iVal = -1;
  sqlite3_file *pFd;
  ಶೂನ್ಯ ಅಸ್ಥಿರ *pShm = 0;
  u32 *aShm;
  ಪೂರ್ಣಾಂಕ iOff;

  ಆದರೆ( objc!=4 && objc!=5 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "DB DBNAME SLOT ?VALUE?");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  zDbname = Tcl_GetString(objv[2]);
  ಆದರೆ( get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[1]), &db) 
   || Tcl_GetIntFromObj(interp, objv[3], &i_ಸ್ಥಾನ)
   || (objc==5 && Tcl_GetIntFromObj(interp, objv[4], &iVal))
  ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  sqlite3_file_control(db, zDbname, SQLITE_FCNTL_FILE_ಸೂಚಕ, (ಶೂನ್ಯ*)&pFd);
  ಆದರೆ( pFd==0 ){
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  rc = pFd->pMethods->xShmMap(pFd, 0, 32*1024, 0, &pShm);
  ಆದರೆ( rc!=SQLITE_OK ){
    Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  ಆದರೆ( pShm==0 ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "*-shm is not yet mapped", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  aShm = (u32*)pShm;
  iOff = 12*2+1+i_ಸ್ಥಾನ;

  ಆದರೆ( objc==5 ){
    aShm[iOff] = iVal;
  }
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(aShm[iOff]));

  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಪೂರ್ಣಾಂಕ Sqlitetestvfs_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "testvfs", testvfs_cmd, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_shmlock", test_vfs_shmlock, 0, 0);
  Tcl_CreateObjCommand(interp, "vfs_set_readmark", test_vfs_ಗಣ_readmark, 0, 0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

#ಆದರೆ_ಅಂತ್ಯ
