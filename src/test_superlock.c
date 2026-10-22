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

#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ <string.h>               /* memset(), strlen() */
#ಅನ್ತರ್ಭಾವ <assert.h>               /* assert() */

/*
** A structure to collect a busy-handler callback and argument and a count
** of the number of times it has been invoked.
*/
ರಚನೆ SuperlockBusy {
  ಪೂರ್ಣಾಂಕ (*xBusy)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ);        /* Pointer to busy-handler function */
  ಶೂನ್ಯ *pBusyArg;                 /* First arg to pass to xBusy */
  ಪೂರ್ಣಾಂಕ nBusy;                      /* Number of times xBusy has been invoked */
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ SuperlockBusy SuperlockBusy;

/*
** An instance of the following structure is allocated for each active
** superlock. The opaque handle returned by sqlite3demo_superlock() is
** actually a pointer to an instance of this structure.
*/
ರಚನೆ Superlock {
  sqlite3 *db;                    /* Database handle used to lock db */
  ಪೂರ್ಣಾಂಕ bWal;                       /* True if db is a WAL database */
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Superlock Superlock;

/*
** The pCtx pointer passed to this function is actually a pointer to a
** SuperlockBusy structure. Invoke the busy-handler function encapsulated
** by the structure and return the result.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ superlockBusyHandler(ಶೂನ್ಯ *pCtx, ಪೂರ್ಣಾಂಕ ಬಳಸದ_2){
  SuperlockBusy *pBusy = (SuperlockBusy *)pCtx;
  ಆದರೆ( pBusy->xBusy==0 ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ pBusy->xBusy(pBusy->pBusyArg, pBusy->nBusy++);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ superlockIsWal(Superlock *p_ಬೀಗ){
  ಪೂರ್ಣಾಂಕ rc;                         /* Return Code */
  sqlite3_stmt *pStmt;            /* Compiled PRAGMA journal_mode statement */

  rc = sqlite3_prepare(p_ಬೀಗ->db, "PRAGMA main.journal_mode", -1, &pStmt, 0);
  ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;

  p_ಬೀಗ->bWal = 0;
  ಆದರೆ( SQLITE_ಅಡ್ಡಸಾಲು==sqlite3_ಹೆಜ್ಜೆ(pStmt) ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಬಹುಲಕ = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)sqlite3_ಸ್ತಂಭ_ಪಠ್ಯ(pStmt, 0);
    ಆದರೆ( z_ಬಹುಲಕ && ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z_ಬಹುಲಕ)==3 && sqlite3_strnicmp("wal", z_ಬಹುಲಕ, 3)==0 ){
      p_ಬೀಗ->bWal = 1;
    }
  }

  ಹಿಂತಿರುಗಿ sqlite3_finalize(pStmt);
}

/*
** Obtain an exclusive shm-lock on nByte bytes starting at offset idx
** of the file fd. If the lock cannot be obtained immediately, invoke
** the busy-handler until either it is obtained or the busy-handler
** callback returns 0.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ superlock_Shm_ಬೀಗ(
  sqlite3_file *fd,               /* Database file handle */
  ಪೂರ್ಣಾಂಕ idx,                        /* Offset of shm-lock to obtain */
  ಪೂರ್ಣಾಂಕ nByte,                      /* Number of consective bytes to lock */
  SuperlockBusy *pBusy            /* Busy-handler wrapper object */
){
  ಪೂರ್ಣಾಂಕ rc;
  ಪೂರ್ಣಾಂಕ (*x_Shm_ಬೀಗ)(sqlite3_file*, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ) = fd->pMethods->x_Shm_ಬೀಗ;
  ಮಾಡು {
    rc = x_Shm_ಬೀಗ(fd, idx, nByte, SQLITE_SHM_ಬೀಗ|SQLITE_SHM_EXCLUSIVE);
  }ಆಗ( rc==SQLITE_BUSY && superlockBusyHandler((ಶೂನ್ಯ *)pBusy, 0) );
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Obtain the extra locks on the database file required for WAL databases.
** Invoke the supplied busy-handler as required.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ superlock_Wal_ಬೀಗ(
  sqlite3 *db,                    /* Database handle open on WAL database */
  SuperlockBusy *pBusy            /* Busy handler wrapper object */
){
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code */
  sqlite3_file *fd = 0;           /* Main database file handle */
  ಶೂನ್ಯ ಅಸ್ಥಿರ *p = 0;           /* Pointer to first page of shared memory */

  /* Obtain a pointer to the sqlite3_file object open on the main db file. */
  rc = sqlite3_file_control(db, "main", SQLITE_FCNTL_FILE_ಸೂಚಕ, (ಶೂನ್ಯ *)&fd);
  ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;

  /* Obtain the "recovery" lock. Normally, this lock is only obtained by
  ** clients running database recovery.  
  */
  rc = superlock_Shm_ಬೀಗ(fd, 2, 1, pBusy);
  ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;

  /* Zero the start of the first shared-memory page. This means that any
  ** clients that open read or write transactions from this point on will
  ** have to run recovery before proceeding. Since they need the "recovery"
  ** lock that this process is holding to do that, no new read or write
  ** transactions may now be opened. Nor can a checkpoint be run, for the
  ** same reason.
  */
  rc = fd->pMethods->xShmMap(fd, 0, 32*1024, 1, &p);
  ಆದರೆ( rc!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;
  memset((ಶೂನ್ಯ *)p, 0, 32);

  /* Obtain exclusive locks on all the "read-lock" slots. Once these locks
  ** are held, it is guaranteed that there are no active reader, writer or 
  ** checkpointer clients.
  */
  rc = superlock_Shm_ಬೀಗ(fd, 3, SQLITE_SHM_NLOCK-3, pBusy);
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Release a superlock held on a database file. The argument passed to 
** this function must have been obtained from a successful call to
** sqlite3demo_superlock().
*/
ಶೂನ್ಯ sqlite3demo_superunlock(ಶೂನ್ಯ *p_ಬೀಗ){
  Superlock *p = (Superlock *)p_ಬೀಗ;
  ಆದರೆ( p->bWal ){
    ಪೂರ್ಣಾಂಕ rc;                         /* Return code */
    ಪೂರ್ಣಾಂಕ flags = SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE;
    sqlite3_file *fd = 0;
    rc = sqlite3_file_control(p->db, "main", SQLITE_FCNTL_FILE_ಸೂಚಕ, (ಶೂನ್ಯ *)&fd);
    ಆದರೆ( rc==SQLITE_OK ){
      fd->pMethods->x_Shm_ಬೀಗ(fd, 2, 1, flags);
      fd->pMethods->x_Shm_ಬೀಗ(fd, 3, SQLITE_SHM_NLOCK-3, flags);
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
ಪೂರ್ಣಾಂಕ sqlite3demo_superlock(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಮಾರ್ಗ,              /* Path to database file to lock */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zVfs,               /* VFS to use to access database file */
  ಪೂರ್ಣಾಂಕ (*xBusy)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ),        /* Busy handler callback */
  ಶೂನ್ಯ *pBusyArg,                 /* Context arg for busy handler */
  ಶೂನ್ಯ **pp_ಬೀಗ                   /* OUT: Context to pass to superunlock() */
){
  SuperlockBusy busy = {0, 0, 0}; /* Busy handler wrapper object */
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code */
  Superlock *p_ಬೀಗ;

  p_ಬೀಗ = sqlite3_malloc(ಗಾತ್ರ(Superlock));
  ಆದರೆ( !p_ಬೀಗ ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  memset(p_ಬೀಗ, 0, ಗಾತ್ರ(Superlock));

  /* Open a database handle on the file to superlock. */
  rc = sqlite3_open_v2(
      z_ಮಾರ್ಗ, &p_ಬೀಗ->db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, zVfs
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
  ಆದರೆ( rc==SQLITE_OK ){
    busy.xBusy = xBusy;
    busy.pBusyArg = pBusyArg;
    sqlite3_busy_handler(p_ಬೀಗ->db, superlockBusyHandler, (ಶೂನ್ಯ *)&busy);
    rc = sqlite3_exec(p_ಬೀಗ->db, "BEGIN EXCLUSIVE", 0, 0, 0);
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
  ಆದರೆ( rc==SQLITE_OK ){
    ಆದರೆ( SQLITE_OK==(rc = superlockIsWal(p_ಬೀಗ)) && p_ಬೀಗ->bWal ){
      rc = sqlite3_exec(p_ಬೀಗ->db, "COMMIT", 0, 0, 0);
      ಆದರೆ( rc==SQLITE_OK ){
        rc = superlock_Wal_ಬೀಗ(p_ಬೀಗ->db, &busy);
      }
    }
  }

  ಆದರೆ( rc!=SQLITE_OK ){
    sqlite3demo_superunlock(p_ಬೀಗ);
    *pp_ಬೀಗ = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    *pp_ಬೀಗ = p_ಬೀಗ;
  }

  ಹಿಂತಿರುಗಿ rc;
}

/*
** End of example code. Everything below here is the test harness.
**************************************************************************
**************************************************************************
*************************************************************************/


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_TEST

#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

ರಚನೆ InterpAndScript {
  Tcl_Interp *interp;
  Tcl_Obj *pScript;
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ InterpAndScript InterpAndScript;

ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_TCLAPI superunlock_del(Client_ದತ್ತಾಂಶ cd){
  sqlite3demo_superunlock((ಶೂನ್ಯ *)cd);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI superunlock_cmd(
  Client_ದತ್ತಾಂಶ cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಆದರೆ( objc!=1 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 1, objv, "");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }
  Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ superlock_busy(ಶೂನ್ಯ *pCtx, ಪೂರ್ಣಾಂಕ nBusy){
  InterpAndScript *p = (InterpAndScript *)pCtx;
  Tcl_Obj *pEval;                 /* Script to evaluate */
  ಪೂರ್ಣಾಂಕ iVal = 0;                   /* Value to return */

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pEval);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewIntObj(nBusy));
  Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_GetIntFromObj(p->interp, Tcl_Get_Obj_ಫಲಿತಾಂಶ(p->interp), &iVal);
  Tcl_Decr_Ref_ಎಣಿಕೆ(pEval);

  ಹಿಂತಿರುಗಿ iVal;
}

/*
** Tclcmd: sqlite3demo_superlock CMDNAME PATH VFS BUSY-HANDLER-SCRIPT
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI superlock_cmd(
  Client_ದತ್ತಾಂಶ cd,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *CONST objv[]
){
  ಶೂನ್ಯ *p_ಬೀಗ;                    /* Lock context */
  ಅಕ್ಷರ *z_ಮಾರ್ಗ;
  ಅಕ್ಷರ *zVfs = 0;
  InterpAndScript busy = {0, 0};
  ಪೂರ್ಣಾಂಕ (*xBusy)(ಶೂನ್ಯ*,ಪೂರ್ಣಾಂಕ) = 0;    /* Busy handler callback */
  ಪೂರ್ಣಾಂಕ rc;                         /* Return code from sqlite3demo_superlock() */

  ಆದರೆ( objc<3 || objc>5 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(
        interp, 1, objv, "CMDNAME PATH ?VFS? ?BUSY-HANDLER-SCRIPT?");
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  z_ಮಾರ್ಗ = Tcl_GetString(objv[2]);

  ಆದರೆ( objc>3 ){
    zVfs = Tcl_GetString(objv[3]);
    ಆದರೆ( ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zVfs)==0 ) zVfs = 0;
  }
  ಆದರೆ( objc>4 ){
    busy.interp = interp;
    busy.pScript = objv[4];
    xBusy = superlock_busy;
  }

  rc = sqlite3demo_superlock(z_ಮಾರ್ಗ, zVfs, xBusy, &busy, &p_ಬೀಗ);
  ಪ್ರತಿಜ್ಞಾ( rc==SQLITE_OK || p_ಬೀಗ==0 );
  ಪ್ರತಿಜ್ಞಾ( rc!=SQLITE_OK || p_ಬೀಗ!=0 );

  ಆದರೆ( rc!=SQLITE_OK ){
    ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrStr(ಪೂರ್ಣಾಂಕ);
    Tcl_Reset_ಫಲಿತಾಂಶ(interp);
    Tcl_Append_ಫಲಿತಾಂಶ(interp, sqlite3ErrStr(rc), ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  Tcl_CreateObjCommand(
      interp, Tcl_GetString(objv[1]), superunlock_cmd, p_ಬೀಗ, superunlock_del
  );
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, objv[1]);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಪೂರ್ಣಾಂಕ SqliteSuperlock_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3demo_superlock", superlock_cmd, 0, 0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}
#ಆದರೆ_ಅಂತ್ಯ
