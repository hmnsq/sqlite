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

#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>               /* memset(), strlen() */
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>               /* assert() */

/*
** A structure to collect a busy-handler callback and argument and a count
** of the number of times it has been invoked.
*/
ਬਣਤਰ SuperlockBusy {
  ਪੂਰਨ (*xBusy)(ਖਾਲੀ*,ਪੂਰਨ);        /* Pointer to busy-handler function */
  ਖਾਲੀ *pBusyArg;                 /* First arg to pass to xBusy */
  ਪੂਰਨ nBusy;                      /* Number of times xBusy has been invoked */
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ SuperlockBusy SuperlockBusy;

/*
** An instance of the following structure is allocated for each active
** superlock. The opaque handle returned by sqlite3demo_superlock() is
** actually a pointer to an instance of this structure.
*/
ਬਣਤਰ Superlock {
  sqlite3 *db;                    /* Database handle used to lock db */
  ਪੂਰਨ bWal;                       /* True if db is a WAL database */
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Superlock Superlock;

/*
** The pCtx pointer passed to this function is actually a pointer to a
** SuperlockBusy structure. Invoke the busy-handler function encapsulated
** by the structure and return the result.
*/
ਸਥਿਰ ਪੂਰਨ superlockBusyHandler(ਖਾਲੀ *pCtx, ਪੂਰਨ ਅਣਵਰਤਿਆ_2){
  SuperlockBusy *pBusy = (SuperlockBusy *)pCtx;
  ਜੇ( pBusy->xBusy==0 ) ਵਾਪਸ 0;
  ਵਾਪਸ pBusy->xBusy(pBusy->pBusyArg, pBusy->nBusy++);
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
ਸਥਿਰ ਪੂਰਨ superlockIsWal(Superlock *p_ਤਾਲਾ){
  ਪੂਰਨ rc;                         /* Return Code */
  sqlite3_stmt *pStmt;            /* Compiled PRAGMA journal_mode statement */

  rc = sqlite3_prepare(p_ਤਾਲਾ->db, "PRAGMA main.journal_mode", -1, &pStmt, 0);
  ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;

  p_ਤਾਲਾ->bWal = 0;
  ਜੇ( SQLITE_ਪੰਕਤੀ==sqlite3_ਕਦਮ(pStmt) ){
    ਅਚਲ ਅੱਖਰ *z_ਬਹੁਲਕ = (ਅਚਲ ਅੱਖਰ *)sqlite3_ਕਾਲਮ_ਪਾਠ(pStmt, 0);
    ਜੇ( z_ਬਹੁਲਕ && ਲੜੀ_ਲੰਬਾਈ(z_ਬਹੁਲਕ)==3 && sqlite3_strnicmp("wal", z_ਬਹੁਲਕ, 3)==0 ){
      p_ਤਾਲਾ->bWal = 1;
    }
  }

  ਵਾਪਸ sqlite3_finalize(pStmt);
}

/*
** Obtain an exclusive shm-lock on nByte bytes starting at offset idx
** of the file fd. If the lock cannot be obtained immediately, invoke
** the busy-handler until either it is obtained or the busy-handler
** callback returns 0.
*/
ਸਥਿਰ ਪੂਰਨ superlock_Shm_ਤਾਲਾ(
  sqlite3_file *fd,               /* Database file handle */
  ਪੂਰਨ idx,                        /* Offset of shm-lock to obtain */
  ਪੂਰਨ nByte,                      /* Number of consective bytes to lock */
  SuperlockBusy *pBusy            /* Busy-handler wrapper object */
){
  ਪੂਰਨ rc;
  ਪੂਰਨ (*x_Shm_ਤਾਲਾ)(sqlite3_file*, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ) = fd->pMethods->x_Shm_ਤਾਲਾ;
  ਕਰੋ {
    rc = x_Shm_ਤਾਲਾ(fd, idx, nByte, SQLITE_SHM_ਤਾਲਾ|SQLITE_SHM_EXCLUSIVE);
  }ਜਦੋਂ( rc==SQLITE_BUSY && superlockBusyHandler((ਖਾਲੀ *)pBusy, 0) );
  ਵਾਪਸ rc;
}

/*
** Obtain the extra locks on the database file required for WAL databases.
** Invoke the supplied busy-handler as required.
*/
ਸਥਿਰ ਪੂਰਨ superlock_Wal_ਤਾਲਾ(
  sqlite3 *db,                    /* Database handle open on WAL database */
  SuperlockBusy *pBusy            /* Busy handler wrapper object */
){
  ਪੂਰਨ rc;                         /* Return code */
  sqlite3_file *fd = 0;           /* Main database file handle */
  ਖਾਲੀ ਅਸਥਿਰ *p = 0;           /* Pointer to first page of shared memory */

  /* Obtain a pointer to the sqlite3_file object open on the main db file. */
  rc = sqlite3_file_control(db, "main", SQLITE_FCNTL_FILE_ਸੂਚਕ, (ਖਾਲੀ *)&fd);
  ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;

  /* Obtain the "recovery" lock. Normally, this lock is only obtained by
  ** clients running database recovery.  
  */
  rc = superlock_Shm_ਤਾਲਾ(fd, 2, 1, pBusy);
  ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;

  /* Zero the start of the first shared-memory page. This means that any
  ** clients that open read or write transactions from this point on will
  ** have to run recovery before proceeding. Since they need the "recovery"
  ** lock that this process is holding to do that, no new read or write
  ** transactions may now be opened. Nor can a checkpoint be run, for the
  ** same reason.
  */
  rc = fd->pMethods->xShmMap(fd, 0, 32*1024, 1, &p);
  ਜੇ( rc!=SQLITE_OK ) ਵਾਪਸ rc;
  memset((ਖਾਲੀ *)p, 0, 32);

  /* Obtain exclusive locks on all the "read-lock" slots. Once these locks
  ** are held, it is guaranteed that there are no active reader, writer or 
  ** checkpointer clients.
  */
  rc = superlock_Shm_ਤਾਲਾ(fd, 3, SQLITE_SHM_NLOCK-3, pBusy);
  ਵਾਪਸ rc;
}

/*
** Release a superlock held on a database file. The argument passed to 
** this function must have been obtained from a successful call to
** sqlite3demo_superlock().
*/
ਖਾਲੀ sqlite3demo_superunlock(ਖਾਲੀ *p_ਤਾਲਾ){
  Superlock *p = (Superlock *)p_ਤਾਲਾ;
  ਜੇ( p->bWal ){
    ਪੂਰਨ rc;                         /* Return code */
    ਪੂਰਨ flags = SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE;
    sqlite3_file *fd = 0;
    rc = sqlite3_file_control(p->db, "main", SQLITE_FCNTL_FILE_ਸੂਚਕ, (ਖਾਲੀ *)&fd);
    ਜੇ( rc==SQLITE_OK ){
      fd->pMethods->x_Shm_ਤਾਲਾ(fd, 2, 1, flags);
      fd->pMethods->x_Shm_ਤਾਲਾ(fd, 3, SQLITE_SHM_NLOCK-3, flags);
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
ਪੂਰਨ sqlite3demo_superlock(
  ਅਚਲ ਅੱਖਰ *z_ਰਸਤਾ,              /* Path to database file to lock */
  ਅਚਲ ਅੱਖਰ *zVfs,               /* VFS to use to access database file */
  ਪੂਰਨ (*xBusy)(ਖਾਲੀ*,ਪੂਰਨ),        /* Busy handler callback */
  ਖਾਲੀ *pBusyArg,                 /* Context arg for busy handler */
  ਖਾਲੀ **pp_ਤਾਲਾ                   /* OUT: Context to pass to superunlock() */
){
  SuperlockBusy busy = {0, 0, 0}; /* Busy handler wrapper object */
  ਪੂਰਨ rc;                         /* Return code */
  Superlock *p_ਤਾਲਾ;

  p_ਤਾਲਾ = sqlite3_malloc(ਆਕਾਰ(Superlock));
  ਜੇ( !p_ਤਾਲਾ ) ਵਾਪਸ SQLITE_NOMEM;
  memset(p_ਤਾਲਾ, 0, ਆਕਾਰ(Superlock));

  /* Open a database handle on the file to superlock. */
  rc = sqlite3_open_v2(
      z_ਰਸਤਾ, &p_ਤਾਲਾ->db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, zVfs
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
  ਜੇ( rc==SQLITE_OK ){
    busy.xBusy = xBusy;
    busy.pBusyArg = pBusyArg;
    sqlite3_busy_handler(p_ਤਾਲਾ->db, superlockBusyHandler, (ਖਾਲੀ *)&busy);
    rc = sqlite3_exec(p_ਤਾਲਾ->db, "BEGIN EXCLUSIVE", 0, 0, 0);
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
  ਜੇ( rc==SQLITE_OK ){
    ਜੇ( SQLITE_OK==(rc = superlockIsWal(p_ਤਾਲਾ)) && p_ਤਾਲਾ->bWal ){
      rc = sqlite3_exec(p_ਤਾਲਾ->db, "COMMIT", 0, 0, 0);
      ਜੇ( rc==SQLITE_OK ){
        rc = superlock_Wal_ਤਾਲਾ(p_ਤਾਲਾ->db, &busy);
      }
    }
  }

  ਜੇ( rc!=SQLITE_OK ){
    sqlite3demo_superunlock(p_ਤਾਲਾ);
    *pp_ਤਾਲਾ = 0;
  }ਨਹੀਂਤਾਂ{
    *pp_ਤਾਲਾ = p_ਤਾਲਾ;
  }

  ਵਾਪਸ rc;
}

/*
** End of example code. Everything below here is the test harness.
**************************************************************************
**************************************************************************
*************************************************************************/


#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST

#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

ਬਣਤਰ InterpAndScript {
  Tcl_Interp *interp;
  Tcl_Obj *pScript;
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ InterpAndScript InterpAndScript;

ਸਥਿਰ ਖਾਲੀ SQLITE_TCLAPI superunlock_del(Client_ਅੰਕੜੇ cd){
  sqlite3demo_superunlock((ਖਾਲੀ *)cd);
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI superunlock_cmd(
  Client_ਅੰਕੜੇ cd,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਜੇ( objc!=1 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "");
    ਵਾਪਸ TCL_ERROR;
  }
  Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ superlock_busy(ਖਾਲੀ *pCtx, ਪੂਰਨ nBusy){
  InterpAndScript *p = (InterpAndScript *)pCtx;
  Tcl_Obj *pEval;                 /* Script to evaluate */
  ਪੂਰਨ iVal = 0;                   /* Value to return */

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pEval);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewIntObj(nBusy));
  Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_GetIntFromObj(p->interp, Tcl_Get_Obj_ਨਤੀਜਾ(p->interp), &iVal);
  Tcl_Decr_Ref_ਗਿਣਤੀ(pEval);

  ਵਾਪਸ iVal;
}

/*
** Tclcmd: sqlite3demo_superlock CMDNAME PATH VFS BUSY-HANDLER-SCRIPT
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI superlock_cmd(
  Client_ਅੰਕੜੇ cd,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  ਖਾਲੀ *p_ਤਾਲਾ;                    /* Lock context */
  ਅੱਖਰ *z_ਰਸਤਾ;
  ਅੱਖਰ *zVfs = 0;
  InterpAndScript busy = {0, 0};
  ਪੂਰਨ (*xBusy)(ਖਾਲੀ*,ਪੂਰਨ) = 0;    /* Busy handler callback */
  ਪੂਰਨ rc;                         /* Return code from sqlite3demo_superlock() */

  ਜੇ( objc<3 || objc>5 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(
        interp, 1, objv, "CMDNAME PATH ?VFS? ?BUSY-HANDLER-SCRIPT?");
    ਵਾਪਸ TCL_ERROR;
  }

  z_ਰਸਤਾ = Tcl_GetString(objv[2]);

  ਜੇ( objc>3 ){
    zVfs = Tcl_GetString(objv[3]);
    ਜੇ( ਲੜੀ_ਲੰਬਾਈ(zVfs)==0 ) zVfs = 0;
  }
  ਜੇ( objc>4 ){
    busy.interp = interp;
    busy.pScript = objv[4];
    xBusy = superlock_busy;
  }

  rc = sqlite3demo_superlock(z_ਰਸਤਾ, zVfs, xBusy, &busy, &p_ਤਾਲਾ);
  ਪ੍ਰਤਿਜ੍ਞਾ( rc==SQLITE_OK || p_ਤਾਲਾ==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( rc!=SQLITE_OK || p_ਤਾਲਾ!=0 );

  ਜੇ( rc!=SQLITE_OK ){
    ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrStr(ਪੂਰਨ);
    Tcl_Reset_ਨਤੀਜਾ(interp);
    Tcl_Append_ਨਤੀਜਾ(interp, sqlite3ErrStr(rc), ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }

  Tcl_CreateObjCommand(
      interp, Tcl_GetString(objv[1]), superunlock_cmd, p_ਤਾਲਾ, superunlock_del
  );
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, objv[1]);
  ਵਾਪਸ TCL_OK;
}

ਪੂਰਨ SqliteSuperlock_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3demo_superlock", superlock_cmd, 0, 0);
  ਵਾਪਸ TCL_OK;
}
#ਜੇ_ਅੰਤ
