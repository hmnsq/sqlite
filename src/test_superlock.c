/*
** 2010 November 19
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Example code for obtaining an exclusive lock on an SQLite database
** file. This method is complicated, but works for both WAL and rollback
** mode database files. The interface to the example code in this file 
** consists of the following two functions:
**
**   sqlite3demo_superlock()
**   sqlite3demo_superunlock()
*/

#include "sqlite3.h"
#include <string.h>               /* memset(), strlen() */
#include <assert.h>               /* assert() */

/*
** A structure to collect a busy-handler callback and argument and a count
** of the number of times it has been invoked.
*/
sa_nracanaa SuperlockBusy {
  ຈຳນວນເຕັມ (*xBusy)(void*,ຈຳນວນເຕັມ);        /* Pointer to busy-handler function */
  void *pBusyArg;                 /* First arg to pass to xBusy */
  ຈຳນວນເຕັມ nBusy;                      /* Number of times xBusy has been invoked */
};
typedef sa_nracanaa SuperlockBusy SuperlockBusy;

/*
** An instance of the following structure is allocated for each active
** superlock. The opaque handle returned by sqlite3demo_superlock() is
** actually a pointer to an instance of this structure.
*/
sa_nracanaa Superlock {
  sqlite3 *db;                    /* Database handle used to lock db */
  ຈຳນວນເຕັມ bWal;                       /* True if db is a WAL database */
};
typedef sa_nracanaa Superlock Superlock;

/*
** The pCtx pointer passed to this function is actually a pointer to a
** SuperlockBusy structure. Invoke the busy-handler function encapsulated
** by the structure and return the result.
*/
static ຈຳນວນເຕັມ superlockBusyHandler(void *pCtx, ຈຳນວນເຕັມ UNUSED){
  SuperlockBusy *pBusy = (SuperlockBusy *)pCtx;
  ຖ້າ( pBusy->xBusy==0 ) ສົ່ງຄືນ 0;
  ສົ່ງຄືນ pBusy->xBusy(pBusy->pBusyArg, pBusy->nBusy++);
}

/*
** This function is used to determine if the main database file for 
** connection db is open in WAL mode or not. If no error occurs and the
** database file is in WAL mode, set *pbWal to true and return SQLITE_OK.
** If it is not in WAL mode, set *pbWal to false.
**
** If an error occurs, return an SQLite error code. The value of *pbWal
** is undefined in this case.
*/
static ຈຳນວນເຕັມ superlockIsWal(Superlock *pLock){
  ຈຳນວນເຕັມ rc;                         /* Return Code */
  sqlite3_stmt *pStmt;            /* Compiled PRAGMA journal_mode statement */

  rc = sqlite3_prepare(pLock->db, "PRAGMA main.journal_mode", -1, &pStmt, 0);
  ຖ້າ( rc!=SQLITE_OK ) ສົ່ງຄືນ rc;

  pLock->bWal = 0;
  ຖ້າ( SQLITE_ROW==sqlite3_step(pStmt) ){
    const var_na *zMode = (const var_na *)sqlite3_column_text(pStmt, 0);
    ຖ້າ( zMode && strlen(zMode)==3 && sqlite3_strnicmp("wal", zMode, 3)==0 ){
      pLock->bWal = 1;
    }
  }

  ສົ່ງຄືນ sqlite3_finalize(pStmt);
}

/*
** Obtain an exclusive shm-lock on nByte bytes starting at offset idx
** of the file fd. If the lock cannot be obtained immediately, invoke
** the busy-handler until either it is obtained or the busy-handler
** callback returns 0.
*/
static ຈຳນວນເຕັມ superlockShmLock(
  sqlite3_file *fd,               /* Database file handle */
  ຈຳນວນເຕັມ idx,                        /* Offset of shm-lock to obtain */
  ຈຳນວນເຕັມ nByte,                      /* Number of consective bytes to lock */
  SuperlockBusy *pBusy            /* Busy-handler wrapper object */
){
  ຈຳນວນເຕັມ rc;
  ຈຳນວນເຕັມ (*xShmLock)(sqlite3_file*, ຈຳນວນເຕັມ, ຈຳນວນເຕັມ, ຈຳນວນເຕັມ) = fd->pMethods->xShmLock;
  do {
    rc = xShmLock(fd, idx, nByte, SQLITE_SHM_LOCK|SQLITE_SHM_EXCLUSIVE);
  }ໃນຂະນະທີ່( rc==SQLITE_BUSY && superlockBusyHandler((void *)pBusy, 0) );
  ສົ່ງຄືນ rc;
}

/*
** Obtain the extra locks on the database file required for WAL databases.
** Invoke the supplied busy-handler as required.
*/
static ຈຳນວນເຕັມ superlockWalLock(
  sqlite3 *db,                    /* Database handle open on WAL database */
  SuperlockBusy *pBusy            /* Busy handler wrapper object */
){
  ຈຳນວນເຕັມ rc;                         /* Return code */
  sqlite3_file *fd = 0;           /* Main database file handle */
  void ca_ncala *p = 0;           /* Pointer to first page of shared memory */

  /* Obtain a pointer to the sqlite3_file object open on the main db file. */
  rc = sqlite3_file_control(db, "main", SQLITE_FCNTL_FILE_POINTER, (void *)&fd);
  ຖ້າ( rc!=SQLITE_OK ) ສົ່ງຄືນ rc;

  /* Obtain the "recovery" lock. Normally, this lock is only obtained by
  ** clients running database recovery.  
  */
  rc = superlockShmLock(fd, 2, 1, pBusy);
  ຖ້າ( rc!=SQLITE_OK ) ສົ່ງຄືນ rc;

  /* Zero the start of the first shared-memory page. This means that any
  ** clients that open read or write transactions from this point on will
  ** have to run recovery before proceeding. Since they need the "recovery"
  ** lock that this process is holding to do that, no new read or write
  ** transactions may now be opened. Nor can a checkpoint be run, for the
  ** same reason.
  */
  rc = fd->pMethods->xShmMap(fd, 0, 32*1024, 1, &p);
  ຖ້າ( rc!=SQLITE_OK ) ສົ່ງຄືນ rc;
  memset((void *)p, 0, 32);

  /* Obtain exclusive locks on all the "read-lock" slots. Once these locks
  ** are held, it is guaranteed that there are no active reader, writer or 
  ** checkpointer clients.
  */
  rc = superlockShmLock(fd, 3, SQLITE_SHM_NLOCK-3, pBusy);
  ສົ່ງຄືນ rc;
}

/*
** Release a superlock held on a database file. The argument passed to 
** this function must have been obtained from a successful call to
** sqlite3demo_superlock().
*/
void sqlite3demo_superunlock(void *pLock){
  Superlock *p = (Superlock *)pLock;
  ຖ້າ( p->bWal ){
    ຈຳນວນເຕັມ rc;                         /* Return code */
    ຈຳນວນເຕັມ flags = SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE;
    sqlite3_file *fd = 0;
    rc = sqlite3_file_control(p->db, "main", SQLITE_FCNTL_FILE_POINTER, (void *)&fd);
    ຖ້າ( rc==SQLITE_OK ){
      fd->pMethods->xShmLock(fd, 2, 1, flags);
      fd->pMethods->xShmLock(fd, 3, SQLITE_SHM_NLOCK-3, flags);
    }
  }
  sqlite3_close(p->db);
  sqlite3_free(p);
}

/*
** Obtain a superlock on the database file identified by zPath, using the
** locking primitives provided by VFS zVfs. If successful, SQLITE_OK is
** returned and output variable *ppLock is populated with an opaque handle
** that may be used with sqlite3demo_superunlock() to release the lock.
**
** If an error occurs, *ppLock is set to 0 and an SQLite error code 
** (e.g. SQLITE_BUSY) is returned.
**
** If a required lock cannot be obtained immediately and the xBusy parameter
** to this function is not NULL, then xBusy is invoked in the same way
** as a busy-handler registered with SQLite (using sqlite3_busy_handler())
** until either the lock can be obtained or the busy-handler function returns
** 0 (indicating "give up").
*/
ຈຳນວນເຕັມ sqlite3demo_superlock(
  const var_na *zPath,              /* Path to database file to lock */
  const var_na *zVfs,               /* VFS to use to access database file */
  ຈຳນວນເຕັມ (*xBusy)(void*,ຈຳນວນເຕັມ),        /* Busy handler callback */
  void *pBusyArg,                 /* Context arg for busy handler */
  void **ppLock                   /* OUT: Context to pass to superunlock() */
){
  SuperlockBusy busy = {0, 0, 0}; /* Busy handler wrapper object */
  ຈຳນວນເຕັມ rc;                         /* Return code */
  Superlock *pLock;

  pLock = sqlite3_malloc(parimaa_na(Superlock));
  ຖ້າ( !pLock ) ສົ່ງຄືນ SQLITE_NOMEM;
  memset(pLock, 0, parimaa_na(Superlock));

  /* Open a database handle on the file to superlock. */
  rc = sqlite3_open_v2(
      zPath, &pLock->db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, zVfs
  );

  /* Install a busy-handler and execute a BEGIN EXCLUSIVE. If this is not
  ** a WAL database, this is all we need to do.  
  **
  ** A wrapper function is used to invoke the busy-handler instead of
  ** registering the busy-handler function supplied by the user directly
  ** with SQLite. This is because the same busy-handler function may be
  ** invoked directly later on when attempting to obtain the extra locks
  ** required in WAL mode. By using the wrapper, we are able to guarantee
  ** that the "nBusy" integer parameter passed to the users busy-handler
  ** represents the total number of busy-handler invocations made within
  ** this call to sqlite3demo_superlock(), including any made during the
  ** "BEGIN EXCLUSIVE".
  */
  ຖ້າ( rc==SQLITE_OK ){
    busy.xBusy = xBusy;
    busy.pBusyArg = pBusyArg;
    sqlite3_busy_handler(pLock->db, superlockBusyHandler, (void *)&busy);
    rc = sqlite3_exec(pLock->db, "BEGIN EXCLUSIVE", 0, 0, 0);
  }

  /* If the BEGIN EXCLUSIVE was executed successfully and this is a WAL
  ** database, call superlockWalLock() to obtain the extra locks required
  ** to prevent readers, writers and/or checkpointers from accessing the
  ** db while this process is holding the superlock.
  **
  ** Before attempting any WAL locks, commit the transaction started above
  ** to drop the WAL read and write locks currently held. Otherwise, the
  ** new WAL locks may conflict with the old.
  */
  ຖ້າ( rc==SQLITE_OK ){
    ຖ້າ( SQLITE_OK==(rc = superlockIsWal(pLock)) && pLock->bWal ){
      rc = sqlite3_exec(pLock->db, "COMMIT", 0, 0, 0);
      ຖ້າ( rc==SQLITE_OK ){
        rc = superlockWalLock(pLock->db, &busy);
      }
    }
  }

  ຖ້າ( rc!=SQLITE_OK ){
    sqlite3demo_superunlock(pLock);
    *ppLock = 0;
  }ບໍ່ດັ່ງນັ້ນ{
    *ppLock = pLock;
  }

  ສົ່ງຄືນ rc;
}

/*
** End of example code. Everything below here is the test harness.
**************************************************************************
**************************************************************************
*************************************************************************/


#ifdef SQLITE_TEST

#include "tclsqlite.h"

sa_nracanaa InterpAndScript {
  Tcl_Interp *interp;
  Tcl_Obj *pScript;
};
typedef sa_nracanaa InterpAndScript InterpAndScript;

static void SQLITE_TCLAPI superunlock_del(ClientData cd){
  sqlite3demo_superunlock((void *)cd);
}

static ຈຳນວນເຕັມ SQLITE_TCLAPI superunlock_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  ຈຳນວນເຕັມ objc,
  Tcl_Obj *CONST objv[]
){
  ຖ້າ( objc!=1 ){
    Tcl_WrongNumArgs(interp, 1, objv, "");
    ສົ່ງຄືນ TCL_ERROR;
  }
  Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
  ສົ່ງຄືນ TCL_OK;
}

static ຈຳນວນເຕັມ superlock_busy(void *pCtx, ຈຳນວນເຕັມ nBusy){
  InterpAndScript *p = (InterpAndScript *)pCtx;
  Tcl_Obj *pEval;                 /* Script to evaluate */
  ຈຳນວນເຕັມ iVal = 0;                   /* Value to return */

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_IncrRefCount(pEval);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewIntObj(nBusy));
  Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_GetIntFromObj(p->interp, Tcl_GetObjResult(p->interp), &iVal);
  Tcl_DecrRefCount(pEval);

  ສົ່ງຄືນ iVal;
}

/*
** Tclcmd: sqlite3demo_superlock CMDNAME PATH VFS BUSY-HANDLER-SCRIPT
*/
static ຈຳນວນເຕັມ SQLITE_TCLAPI superlock_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  ຈຳນວນເຕັມ objc,
  Tcl_Obj *CONST objv[]
){
  void *pLock;                    /* Lock context */
  var_na *zPath;
  var_na *zVfs = 0;
  InterpAndScript busy = {0, 0};
  ຈຳນວນເຕັມ (*xBusy)(void*,ຈຳນວນເຕັມ) = 0;    /* Busy handler callback */
  ຈຳນວນເຕັມ rc;                         /* Return code from sqlite3demo_superlock() */

  ຖ້າ( objc<3 || objc>5 ){
    Tcl_WrongNumArgs(
        interp, 1, objv, "CMDNAME PATH ?VFS? ?BUSY-HANDLER-SCRIPT?");
    ສົ່ງຄືນ TCL_ERROR;
  }

  zPath = Tcl_GetString(objv[2]);

  ຖ້າ( objc>3 ){
    zVfs = Tcl_GetString(objv[3]);
    ຖ້າ( strlen(zVfs)==0 ) zVfs = 0;
  }
  ຖ້າ( objc>4 ){
    busy.interp = interp;
    busy.pScript = objv[4];
    xBusy = superlock_busy;
  }

  rc = sqlite3demo_superlock(zPath, zVfs, xBusy, &busy, &pLock);
  assert( rc==SQLITE_OK || pLock==0 );
  assert( rc!=SQLITE_OK || pLock!=0 );

  ຖ້າ( rc!=SQLITE_OK ){
    extern const var_na *sqlite3ErrStr(ຈຳນວນເຕັມ);
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, sqlite3ErrStr(rc), NULL);
    ສົ່ງຄືນ TCL_ERROR;
  }

  Tcl_CreateObjCommand(
      interp, Tcl_GetString(objv[1]), superunlock_cmd, pLock, superunlock_del
  );
  Tcl_SetObjResult(interp, objv[1]);
  ສົ່ງຄືນ TCL_OK;
}

ຈຳນວນເຕັມ SqliteSuperlock_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3demo_superlock", superlock_cmd, 0, 0);
  ສົ່ງຄືນ TCL_OK;
}
#endif
