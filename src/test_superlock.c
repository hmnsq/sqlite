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

#અન્તર્ભાવ "sqlite3.h"
#અન્તર્ભાવ <string.h>               /* memset(), strlen() */
#અન્તર્ભાવ <assert.h>               /* assert() */

/*
** A structure to collect a busy-handler callback and argument and a count
** of the number of times it has been invoked.
*/
સંરચના SuperlockBusy {
  પૂર્ણાંક (*xBusy)(શૂન્ય*,પૂર્ણાંક);        /* Pointer to busy-handler function */
  શૂન્ય *pBusyArg;                 /* First arg to pass to xBusy */
  પૂર્ણાંક nBusy;                      /* Number of times xBusy has been invoked */
};
પ્રકારવ્યાખ્યા સંરચના SuperlockBusy SuperlockBusy;

/*
** An instance of the following structure is allocated for each active
** superlock. The opaque handle returned by sqlite3demo_superlock() is
** actually a pointer to an instance of this structure.
*/
સંરચના Superlock {
  sqlite3 *db;                    /* Database handle used to lock db */
  પૂર્ણાંક bWal;                       /* True if db is a WAL database */
};
પ્રકારવ્યાખ્યા સંરચના Superlock Superlock;

/*
** The pCtx pointer passed to this function is actually a pointer to a
** SuperlockBusy structure. Invoke the busy-handler function encapsulated
** by the structure and return the result.
*/
સ્થિર પૂર્ણાંક superlockBusyHandler(શૂન્ય *pCtx, પૂર્ણાંક UNUSED){
  SuperlockBusy *pBusy = (SuperlockBusy *)pCtx;
  જો( pBusy->xBusy==0 ) પાછા 0;
  પાછા pBusy->xBusy(pBusy->pBusyArg, pBusy->nBusy++);
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
સ્થિર પૂર્ણાંક superlockIsWal(Superlock *pLock){
  પૂર્ણાંક rc;                         /* Return Code */
  sqlite3_stmt *pStmt;            /* Compiled PRAGMA journal_mode statement */

  rc = sqlite3_prepare(pLock->db, "PRAGMA main.journal_mode", -1, &pStmt, 0);
  જો( rc!=SQLITE_OK ) પાછા rc;

  pLock->bWal = 0;
  જો( SQLITE_ROW==sqlite3_step(pStmt) ){
    અચલ અક્ષર *zMode = (અચલ અક્ષર *)sqlite3_column_text(pStmt, 0);
    જો( zMode && strlen(zMode)==3 && sqlite3_strnicmp("wal", zMode, 3)==0 ){
      pLock->bWal = 1;
    }
  }

  પાછા sqlite3_finalize(pStmt);
}

/*
** Obtain an exclusive shm-lock on nByte bytes starting at offset idx
** of the file fd. If the lock cannot be obtained immediately, invoke
** the busy-handler until either it is obtained or the busy-handler
** callback returns 0.
*/
સ્થિર પૂર્ણાંક superlockShmLock(
  sqlite3_file *fd,               /* Database file handle */
  પૂર્ણાંક idx,                        /* Offset of shm-lock to obtain */
  પૂર્ણાંક nByte,                      /* Number of consective bytes to lock */
  SuperlockBusy *pBusy            /* Busy-handler wrapper object */
){
  પૂર્ણાંક rc;
  પૂર્ણાંક (*xShmLock)(sqlite3_file*, પૂર્ણાંક, પૂર્ણાંક, પૂર્ણાંક) = fd->pMethods->xShmLock;
  કરો {
    rc = xShmLock(fd, idx, nByte, SQLITE_SHM_LOCK|SQLITE_SHM_EXCLUSIVE);
  }જ્યારે( rc==SQLITE_BUSY && superlockBusyHandler((શૂન્ય *)pBusy, 0) );
  પાછા rc;
}

/*
** Obtain the extra locks on the database file required for WAL databases.
** Invoke the supplied busy-handler as required.
*/
સ્થિર પૂર્ણાંક superlockWalLock(
  sqlite3 *db,                    /* Database handle open on WAL database */
  SuperlockBusy *pBusy            /* Busy handler wrapper object */
){
  પૂર્ણાંક rc;                         /* Return code */
  sqlite3_file *fd = 0;           /* Main database file handle */
  શૂન્ય અસ્થિર *p = 0;           /* Pointer to first page of shared memory */

  /* Obtain a pointer to the sqlite3_file object open on the main db file. */
  rc = sqlite3_file_control(db, "main", SQLITE_FCNTL_FILE_POINTER, (શૂન્ય *)&fd);
  જો( rc!=SQLITE_OK ) પાછા rc;

  /* Obtain the "recovery" lock. Normally, this lock is only obtained by
  ** clients running database recovery.  
  */
  rc = superlockShmLock(fd, 2, 1, pBusy);
  જો( rc!=SQLITE_OK ) પાછા rc;

  /* Zero the start of the first shared-memory page. This means that any
  ** clients that open read or write transactions from this point on will
  ** have to run recovery before proceeding. Since they need the "recovery"
  ** lock that this process is holding to do that, no new read or write
  ** transactions may now be opened. Nor can a checkpoint be run, for the
  ** same reason.
  */
  rc = fd->pMethods->xShmMap(fd, 0, 32*1024, 1, &p);
  જો( rc!=SQLITE_OK ) પાછા rc;
  memset((શૂન્ય *)p, 0, 32);

  /* Obtain exclusive locks on all the "read-lock" slots. Once these locks
  ** are held, it is guaranteed that there are no active reader, writer or 
  ** checkpointer clients.
  */
  rc = superlockShmLock(fd, 3, SQLITE_SHM_NLOCK-3, pBusy);
  પાછા rc;
}

/*
** Release a superlock held on a database file. The argument passed to 
** this function must have been obtained from a successful call to
** sqlite3demo_superlock().
*/
શૂન્ય sqlite3demo_superunlock(શૂન્ય *pLock){
  Superlock *p = (Superlock *)pLock;
  જો( p->bWal ){
    પૂર્ણાંક rc;                         /* Return code */
    પૂર્ણાંક flags = SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE;
    sqlite3_file *fd = 0;
    rc = sqlite3_file_control(p->db, "main", SQLITE_FCNTL_FILE_POINTER, (શૂન્ય *)&fd);
    જો( rc==SQLITE_OK ){
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
પૂર્ણાંક sqlite3demo_superlock(
  અચલ અક્ષર *zPath,              /* Path to database file to lock */
  અચલ અક્ષર *zVfs,               /* VFS to use to access database file */
  પૂર્ણાંક (*xBusy)(શૂન્ય*,પૂર્ણાંક),        /* Busy handler callback */
  શૂન્ય *pBusyArg,                 /* Context arg for busy handler */
  શૂન્ય **ppLock                   /* OUT: Context to pass to superunlock() */
){
  SuperlockBusy busy = {0, 0, 0}; /* Busy handler wrapper object */
  પૂર્ણાંક rc;                         /* Return code */
  Superlock *pLock;

  pLock = sqlite3_malloc(કદ(Superlock));
  જો( !pLock ) પાછા SQLITE_NOMEM;
  memset(pLock, 0, કદ(Superlock));

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
  જો( rc==SQLITE_OK ){
    busy.xBusy = xBusy;
    busy.pBusyArg = pBusyArg;
    sqlite3_busy_handler(pLock->db, superlockBusyHandler, (શૂન્ય *)&busy);
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
  જો( rc==SQLITE_OK ){
    જો( SQLITE_OK==(rc = superlockIsWal(pLock)) && pLock->bWal ){
      rc = sqlite3_exec(pLock->db, "COMMIT", 0, 0, 0);
      જો( rc==SQLITE_OK ){
        rc = superlockWalLock(pLock->db, &busy);
      }
    }
  }

  જો( rc!=SQLITE_OK ){
    sqlite3demo_superunlock(pLock);
    *ppLock = 0;
  }નહીંતર{
    *ppLock = pLock;
  }

  પાછા rc;
}

/*
** End of example code. Everything below here is the test harness.
**************************************************************************
**************************************************************************
*************************************************************************/


#ifdef SQLITE_TEST

#અન્તર્ભાવ "tclsqlite.h"

સંરચના InterpAndScript {
  Tcl_Interp *interp;
  Tcl_Obj *pScript;
};
પ્રકારવ્યાખ્યા સંરચના InterpAndScript InterpAndScript;

સ્થિર શૂન્ય SQLITE_TCLAPI superunlock_del(ClientData cd){
  sqlite3demo_superunlock((શૂન્ય *)cd);
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI superunlock_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  જો( objc!=1 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "");
    પાછા TCL_ERROR;
  }
  Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક superlock_busy(શૂન્ય *pCtx, પૂર્ણાંક nBusy){
  InterpAndScript *p = (InterpAndScript *)pCtx;
  Tcl_Obj *pEval;                 /* Script to evaluate */
  પૂર્ણાંક iVal = 0;                   /* Value to return */

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_ગણતરી(pEval);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewIntObj(nBusy));
  Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_GetIntFromObj(p->interp, Tcl_GetObjResult(p->interp), &iVal);
  Tcl_Decr_Ref_ગણતરી(pEval);

  પાછા iVal;
}

/*
** Tclcmd: sqlite3demo_superlock CMDNAME PATH VFS BUSY-HANDLER-SCRIPT
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI superlock_cmd(
  ClientData cd,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  શૂન્ય *pLock;                    /* Lock context */
  અક્ષર *zPath;
  અક્ષર *zVfs = 0;
  InterpAndScript busy = {0, 0};
  પૂર્ણાંક (*xBusy)(શૂન્ય*,પૂર્ણાંક) = 0;    /* Busy handler callback */
  પૂર્ણાંક rc;                         /* Return code from sqlite3demo_superlock() */

  જો( objc<3 || objc>5 ){
    Tcl_Wrong_અંક_Args(
        interp, 1, objv, "CMDNAME PATH ?VFS? ?BUSY-HANDLER-SCRIPT?");
    પાછા TCL_ERROR;
  }

  zPath = Tcl_GetString(objv[2]);

  જો( objc>3 ){
    zVfs = Tcl_GetString(objv[3]);
    જો( strlen(zVfs)==0 ) zVfs = 0;
  }
  જો( objc>4 ){
    busy.interp = interp;
    busy.pScript = objv[4];
    xBusy = superlock_busy;
  }

  rc = sqlite3demo_superlock(zPath, zVfs, xBusy, &busy, &pLock);
  પ્રતિજ્ઞા( rc==SQLITE_OK || pLock==0 );
  પ્રતિજ્ઞા( rc!=SQLITE_OK || pLock!=0 );

  જો( rc!=SQLITE_OK ){
    બાહ્ય અચલ અક્ષર *sqlite3ErrStr(પૂર્ણાંક);
    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, sqlite3ErrStr(rc), ખાલી);
    પાછા TCL_ERROR;
  }

  Tcl_CreateObjCommand(
      interp, Tcl_GetString(objv[1]), superunlock_cmd, pLock, superunlock_del
  );
  Tcl_SetObjResult(interp, objv[1]);
  પાછા TCL_OK;
}

પૂર્ણાંક SqliteSuperlock_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3demo_superlock", superlock_cmd, 0, 0);
  પાછા TCL_OK;
}
#endif
