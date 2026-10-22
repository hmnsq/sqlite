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

#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ <string.h>               /* memset(), strlen() */
#అన్తర్భావ <assert.h>               /* assert() */

/*
** A structure to collect a busy-handler callback and argument and a count
** of the number of times it has been invoked.
*/
నిర్మాణం SuperlockBusy {
  పూర్ణాంకం (*xBusy)(శూన్య*,పూర్ణాంకం);        /* Pointer to busy-handler function */
  శూన్య *pBusyArg;                 /* First arg to pass to xBusy */
  పూర్ణాంకం nBusy;                      /* Number of times xBusy has been invoked */
};
రకనిర్వచనం నిర్మాణం SuperlockBusy SuperlockBusy;

/*
** An instance of the following structure is allocated for each active
** superlock. The opaque handle returned by sqlite3demo_superlock() is
** actually a pointer to an instance of this structure.
*/
నిర్మాణం Superlock {
  sqlite3 *db;                    /* Database handle used to lock db */
  పూర్ణాంకం bWal;                       /* True if db is a WAL database */
};
రకనిర్వచనం నిర్మాణం Superlock Superlock;

/*
** The pCtx pointer passed to this function is actually a pointer to a
** SuperlockBusy structure. Invoke the busy-handler function encapsulated
** by the structure and return the result.
*/
స్థిర పూర్ణాంకం superlockBusyHandler(శూన్య *pCtx, పూర్ణాంకం వాడని_2){
  SuperlockBusy *pBusy = (SuperlockBusy *)pCtx;
  ఒకవేళ( pBusy->xBusy==0 ) తిరిగి 0;
  తిరిగి pBusy->xBusy(pBusy->pBusyArg, pBusy->nBusy++);
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
స్థిర పూర్ణాంకం superlockIsWal(Superlock *p_తాళం){
  పూర్ణాంకం rc;                         /* Return Code */
  sqlite3_stmt *pStmt;            /* Compiled PRAGMA journal_mode statement */

  rc = sqlite3_prepare(p_తాళం->db, "PRAGMA main.journal_mode", -1, &pStmt, 0);
  ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;

  p_తాళం->bWal = 0;
  ఒకవేళ( SQLITE_అడ్డువరుస==sqlite3_అడుగు(pStmt) ){
    స్థిరరాశి అక్షరం *z_బహుళకం = (స్థిరరాశి అక్షరం *)sqlite3_స్తంభం_పాఠ్యం(pStmt, 0);
    ఒకవేళ( z_బహుళకం && అక్షరశ్రేణి_పొడవు(z_బహుళకం)==3 && sqlite3_strnicmp("wal", z_బహుళకం, 3)==0 ){
      p_తాళం->bWal = 1;
    }
  }

  తిరిగి sqlite3_finalize(pStmt);
}

/*
** Obtain an exclusive shm-lock on nByte bytes starting at offset idx
** of the file fd. If the lock cannot be obtained immediately, invoke
** the busy-handler until either it is obtained or the busy-handler
** callback returns 0.
*/
స్థిర పూర్ణాంకం superlock_Shm_తాళం(
  sqlite3_file *fd,               /* Database file handle */
  పూర్ణాంకం idx,                        /* Offset of shm-lock to obtain */
  పూర్ణాంకం nByte,                      /* Number of consective bytes to lock */
  SuperlockBusy *pBusy            /* Busy-handler wrapper object */
){
  పూర్ణాంకం rc;
  పూర్ణాంకం (*x_Shm_తాళం)(sqlite3_file*, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం) = fd->pMethods->x_Shm_తాళం;
  చేయి {
    rc = x_Shm_తాళం(fd, idx, nByte, SQLITE_SHM_తాళం|SQLITE_SHM_EXCLUSIVE);
  }అయితే( rc==SQLITE_BUSY && superlockBusyHandler((శూన్య *)pBusy, 0) );
  తిరిగి rc;
}

/*
** Obtain the extra locks on the database file required for WAL databases.
** Invoke the supplied busy-handler as required.
*/
స్థిర పూర్ణాంకం superlock_Wal_తాళం(
  sqlite3 *db,                    /* Database handle open on WAL database */
  SuperlockBusy *pBusy            /* Busy handler wrapper object */
){
  పూర్ణాంకం rc;                         /* Return code */
  sqlite3_file *fd = 0;           /* Main database file handle */
  శూన్య అస్థిర *p = 0;           /* Pointer to first page of shared memory */

  /* Obtain a pointer to the sqlite3_file object open on the main db file. */
  rc = sqlite3_file_control(db, "main", SQLITE_FCNTL_FILE_సూచిక, (శూన్య *)&fd);
  ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;

  /* Obtain the "recovery" lock. Normally, this lock is only obtained by
  ** clients running database recovery.  
  */
  rc = superlock_Shm_తాళం(fd, 2, 1, pBusy);
  ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;

  /* Zero the start of the first shared-memory page. This means that any
  ** clients that open read or write transactions from this point on will
  ** have to run recovery before proceeding. Since they need the "recovery"
  ** lock that this process is holding to do that, no new read or write
  ** transactions may now be opened. Nor can a checkpoint be run, for the
  ** same reason.
  */
  rc = fd->pMethods->xShmMap(fd, 0, 32*1024, 1, &p);
  ఒకవేళ( rc!=SQLITE_OK ) తిరిగి rc;
  memset((శూన్య *)p, 0, 32);

  /* Obtain exclusive locks on all the "read-lock" slots. Once these locks
  ** are held, it is guaranteed that there are no active reader, writer or 
  ** checkpointer clients.
  */
  rc = superlock_Shm_తాళం(fd, 3, SQLITE_SHM_NLOCK-3, pBusy);
  తిరిగి rc;
}

/*
** Release a superlock held on a database file. The argument passed to 
** this function must have been obtained from a successful call to
** sqlite3demo_superlock().
*/
శూన్య sqlite3demo_superunlock(శూన్య *p_తాళం){
  Superlock *p = (Superlock *)p_తాళం;
  ఒకవేళ( p->bWal ){
    పూర్ణాంకం rc;                         /* Return code */
    పూర్ణాంకం flags = SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE;
    sqlite3_file *fd = 0;
    rc = sqlite3_file_control(p->db, "main", SQLITE_FCNTL_FILE_సూచిక, (శూన్య *)&fd);
    ఒకవేళ( rc==SQLITE_OK ){
      fd->pMethods->x_Shm_తాళం(fd, 2, 1, flags);
      fd->pMethods->x_Shm_తాళం(fd, 3, SQLITE_SHM_NLOCK-3, flags);
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
పూర్ణాంకం sqlite3demo_superlock(
  స్థిరరాశి అక్షరం *z_మార్గం,              /* Path to database file to lock */
  స్థిరరాశి అక్షరం *zVfs,               /* VFS to use to access database file */
  పూర్ణాంకం (*xBusy)(శూన్య*,పూర్ణాంకం),        /* Busy handler callback */
  శూన్య *pBusyArg,                 /* Context arg for busy handler */
  శూన్య **pp_తాళం                   /* OUT: Context to pass to superunlock() */
){
  SuperlockBusy busy = {0, 0, 0}; /* Busy handler wrapper object */
  పూర్ణాంకం rc;                         /* Return code */
  Superlock *p_తాళం;

  p_తాళం = sqlite3_malloc(పరిమాణం(Superlock));
  ఒకవేళ( !p_తాళం ) తిరిగి SQLITE_NOMEM;
  memset(p_తాళం, 0, పరిమాణం(Superlock));

  /* Open a database handle on the file to superlock. */
  rc = sqlite3_open_v2(
      z_మార్గం, &p_తాళం->db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, zVfs
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
  ఒకవేళ( rc==SQLITE_OK ){
    busy.xBusy = xBusy;
    busy.pBusyArg = pBusyArg;
    sqlite3_busy_handler(p_తాళం->db, superlockBusyHandler, (శూన్య *)&busy);
    rc = sqlite3_exec(p_తాళం->db, "BEGIN EXCLUSIVE", 0, 0, 0);
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
  ఒకవేళ( rc==SQLITE_OK ){
    ఒకవేళ( SQLITE_OK==(rc = superlockIsWal(p_తాళం)) && p_తాళం->bWal ){
      rc = sqlite3_exec(p_తాళం->db, "COMMIT", 0, 0, 0);
      ఒకవేళ( rc==SQLITE_OK ){
        rc = superlock_Wal_తాళం(p_తాళం->db, &busy);
      }
    }
  }

  ఒకవేళ( rc!=SQLITE_OK ){
    sqlite3demo_superunlock(p_తాళం);
    *pp_తాళం = 0;
  }లేకపోతే{
    *pp_తాళం = p_తాళం;
  }

  తిరిగి rc;
}

/*
** End of example code. Everything below here is the test harness.
**************************************************************************
**************************************************************************
*************************************************************************/


#నిర్వచితమైతే SQLITE_TEST

#అన్తర్భావ "tclsqlite.h"

నిర్మాణం InterpAndScript {
  Tcl_Interp *interp;
  Tcl_Obj *pScript;
};
రకనిర్వచనం నిర్మాణం InterpAndScript InterpAndScript;

స్థిర శూన్య SQLITE_TCLAPI superunlock_del(Client_దత్తాంశం cd){
  sqlite3demo_superunlock((శూన్య *)cd);
}

స్థిర పూర్ణాంకం SQLITE_TCLAPI superunlock_cmd(
  Client_దత్తాంశం cd,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  ఒకవేళ( objc!=1 ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 1, objv, "");
    తిరిగి TCL_ERROR;
  }
  Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
  తిరిగి TCL_OK;
}

స్థిర పూర్ణాంకం superlock_busy(శూన్య *pCtx, పూర్ణాంకం nBusy){
  InterpAndScript *p = (InterpAndScript *)pCtx;
  Tcl_Obj *pEval;                 /* Script to evaluate */
  పూర్ణాంకం iVal = 0;                   /* Value to return */

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_లెక్క(pEval);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewIntObj(nBusy));
  Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_GetIntFromObj(p->interp, Tcl_Get_Obj_ఫలితం(p->interp), &iVal);
  Tcl_Decr_Ref_లెక్క(pEval);

  తిరిగి iVal;
}

/*
** Tclcmd: sqlite3demo_superlock CMDNAME PATH VFS BUSY-HANDLER-SCRIPT
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI superlock_cmd(
  Client_దత్తాంశం cd,
  Tcl_Interp *interp,
  పూర్ణాంకం objc,
  Tcl_Obj *CONST objv[]
){
  శూన్య *p_తాళం;                    /* Lock context */
  అక్షరం *z_మార్గం;
  అక్షరం *zVfs = 0;
  InterpAndScript busy = {0, 0};
  పూర్ణాంకం (*xBusy)(శూన్య*,పూర్ణాంకం) = 0;    /* Busy handler callback */
  పూర్ణాంకం rc;                         /* Return code from sqlite3demo_superlock() */

  ఒకవేళ( objc<3 || objc>5 ){
    Tcl_Wrong_అంకె_వాదనలు(
        interp, 1, objv, "CMDNAME PATH ?VFS? ?BUSY-HANDLER-SCRIPT?");
    తిరిగి TCL_ERROR;
  }

  z_మార్గం = Tcl_GetString(objv[2]);

  ఒకవేళ( objc>3 ){
    zVfs = Tcl_GetString(objv[3]);
    ఒకవేళ( అక్షరశ్రేణి_పొడవు(zVfs)==0 ) zVfs = 0;
  }
  ఒకవేళ( objc>4 ){
    busy.interp = interp;
    busy.pScript = objv[4];
    xBusy = superlock_busy;
  }

  rc = sqlite3demo_superlock(z_మార్గం, zVfs, xBusy, &busy, &p_తాళం);
  ప్రతిజ్ఞా( rc==SQLITE_OK || p_తాళం==0 );
  ప్రతిజ్ఞా( rc!=SQLITE_OK || p_తాళం!=0 );

  ఒకవేళ( rc!=SQLITE_OK ){
    బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrStr(పూర్ణాంకం);
    Tcl_Reset_ఫలితం(interp);
    Tcl_Append_ఫలితం(interp, sqlite3ErrStr(rc), ఖాళీ);
    తిరిగి TCL_ERROR;
  }

  Tcl_CreateObjCommand(
      interp, Tcl_GetString(objv[1]), superunlock_cmd, p_తాళం, superunlock_del
  );
  Tcl_సమితి_Obj_ఫలితం(interp, objv[1]);
  తిరిగి TCL_OK;
}

పూర్ణాంకం SqliteSuperlock_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3demo_superlock", superlock_cmd, 0, 0);
  తిరిగి TCL_OK;
}
#అయితే_ముగింపు
