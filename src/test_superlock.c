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

#অন্তর্ভাব "sqlite3.h"
#অন্তর্ভাব <string.h>               /* memset(), strlen() */
#অন্তর্ভাব <assert.h>               /* assert() */

/*
** A structure to collect a busy-handler callback and argument and a count
** of the number of times it has been invoked.
*/
কাঠামো SuperlockBusy {
  পূর্ণসংখ্যা (*xBusy)(শূন্য*,পূর্ণসংখ্যা);        /* Pointer to busy-handler function */
  শূন্য *pBusyArg;                 /* First arg to pass to xBusy */
  পূর্ণসংখ্যা nBusy;                      /* Number of times xBusy has been invoked */
};
প্রকারলেখ কাঠামো SuperlockBusy SuperlockBusy;

/*
** An instance of the following structure is allocated for each active
** superlock. The opaque handle returned by sqlite3demo_superlock() is
** actually a pointer to an instance of this structure.
*/
কাঠামো Superlock {
  sqlite3 *db;                    /* Database handle used to lock db */
  পূর্ণসংখ্যা bWal;                       /* True if db is a WAL database */
};
প্রকারলেখ কাঠামো Superlock Superlock;

/*
** The pCtx pointer passed to this function is actually a pointer to a
** SuperlockBusy structure. Invoke the busy-handler function encapsulated
** by the structure and return the result.
*/
স্থির পূর্ণসংখ্যা superlockBusyHandler(শূন্য *pCtx, পূর্ণসংখ্যা UNUSED){
  SuperlockBusy *pBusy = (SuperlockBusy *)pCtx;
  যদি( pBusy->xBusy==0 ) ফেরত 0;
  ফেরত pBusy->xBusy(pBusy->pBusyArg, pBusy->nBusy++);
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
স্থির পূর্ণসংখ্যা superlockIsWal(Superlock *pLock){
  পূর্ণসংখ্যা rc;                         /* Return Code */
  sqlite3_stmt *pStmt;            /* Compiled PRAGMA journal_mode statement */

  rc = sqlite3_prepare(pLock->db, "PRAGMA main.journal_mode", -1, &pStmt, 0);
  যদি( rc!=SQLITE_OK ) ফেরত rc;

  pLock->bWal = 0;
  যদি( SQLITE_ROW==sqlite3_step(pStmt) ){
    ধ্রুবক অক্ষর *zMode = (ধ্রুবক অক্ষর *)sqlite3_column_text(pStmt, 0);
    যদি( zMode && strlen(zMode)==3 && sqlite3_strnicmp("wal", zMode, 3)==0 ){
      pLock->bWal = 1;
    }
  }

  ফেরত sqlite3_finalize(pStmt);
}

/*
** Obtain an exclusive shm-lock on nByte bytes starting at offset idx
** of the file fd. If the lock cannot be obtained immediately, invoke
** the busy-handler until either it is obtained or the busy-handler
** callback returns 0.
*/
স্থির পূর্ণসংখ্যা superlockShmLock(
  sqlite3_file *fd,               /* Database file handle */
  পূর্ণসংখ্যা idx,                        /* Offset of shm-lock to obtain */
  পূর্ণসংখ্যা nByte,                      /* Number of consective bytes to lock */
  SuperlockBusy *pBusy            /* Busy-handler wrapper object */
){
  পূর্ণসংখ্যা rc;
  পূর্ণসংখ্যা (*xShmLock)(sqlite3_file*, পূর্ণসংখ্যা, পূর্ণসংখ্যা, পূর্ণসংখ্যা) = fd->pMethods->xShmLock;
  করো {
    rc = xShmLock(fd, idx, nByte, SQLITE_SHM_LOCK|SQLITE_SHM_EXCLUSIVE);
  }যখন( rc==SQLITE_BUSY && superlockBusyHandler((শূন্য *)pBusy, 0) );
  ফেরত rc;
}

/*
** Obtain the extra locks on the database file required for WAL databases.
** Invoke the supplied busy-handler as required.
*/
স্থির পূর্ণসংখ্যা superlockWalLock(
  sqlite3 *db,                    /* Database handle open on WAL database */
  SuperlockBusy *pBusy            /* Busy handler wrapper object */
){
  পূর্ণসংখ্যা rc;                         /* Return code */
  sqlite3_file *fd = 0;           /* Main database file handle */
  শূন্য অস্থির *p = 0;           /* Pointer to first page of shared memory */

  /* Obtain a pointer to the sqlite3_file object open on the main db file. */
  rc = sqlite3_file_control(db, "main", SQLITE_FCNTL_FILE_POINTER, (শূন্য *)&fd);
  যদি( rc!=SQLITE_OK ) ফেরত rc;

  /* Obtain the "recovery" lock. Normally, this lock is only obtained by
  ** clients running database recovery.  
  */
  rc = superlockShmLock(fd, 2, 1, pBusy);
  যদি( rc!=SQLITE_OK ) ফেরত rc;

  /* Zero the start of the first shared-memory page. This means that any
  ** clients that open read or write transactions from this point on will
  ** have to run recovery before proceeding. Since they need the "recovery"
  ** lock that this process is holding to do that, no new read or write
  ** transactions may now be opened. Nor can a checkpoint be run, for the
  ** same reason.
  */
  rc = fd->pMethods->xShmMap(fd, 0, 32*1024, 1, &p);
  যদি( rc!=SQLITE_OK ) ফেরত rc;
  memset((শূন্য *)p, 0, 32);

  /* Obtain exclusive locks on all the "read-lock" slots. Once these locks
  ** are held, it is guaranteed that there are no active reader, writer or 
  ** checkpointer clients.
  */
  rc = superlockShmLock(fd, 3, SQLITE_SHM_NLOCK-3, pBusy);
  ফেরত rc;
}

/*
** Release a superlock held on a database file. The argument passed to 
** this function must have been obtained from a successful call to
** sqlite3demo_superlock().
*/
শূন্য sqlite3demo_superunlock(শূন্য *pLock){
  Superlock *p = (Superlock *)pLock;
  যদি( p->bWal ){
    পূর্ণসংখ্যা rc;                         /* Return code */
    পূর্ণসংখ্যা flags = SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE;
    sqlite3_file *fd = 0;
    rc = sqlite3_file_control(p->db, "main", SQLITE_FCNTL_FILE_POINTER, (শূন্য *)&fd);
    যদি( rc==SQLITE_OK ){
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
পূর্ণসংখ্যা sqlite3demo_superlock(
  ধ্রুবক অক্ষর *zPath,              /* Path to database file to lock */
  ধ্রুবক অক্ষর *zVfs,               /* VFS to use to access database file */
  পূর্ণসংখ্যা (*xBusy)(শূন্য*,পূর্ণসংখ্যা),        /* Busy handler callback */
  শূন্য *pBusyArg,                 /* Context arg for busy handler */
  শূন্য **ppLock                   /* OUT: Context to pass to superunlock() */
){
  SuperlockBusy busy = {0, 0, 0}; /* Busy handler wrapper object */
  পূর্ণসংখ্যা rc;                         /* Return code */
  Superlock *pLock;

  pLock = sqlite3_malloc(আকার(Superlock));
  যদি( !pLock ) ফেরত SQLITE_NOMEM;
  memset(pLock, 0, আকার(Superlock));

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
  যদি( rc==SQLITE_OK ){
    busy.xBusy = xBusy;
    busy.pBusyArg = pBusyArg;
    sqlite3_busy_handler(pLock->db, superlockBusyHandler, (শূন্য *)&busy);
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
  যদি( rc==SQLITE_OK ){
    যদি( SQLITE_OK==(rc = superlockIsWal(pLock)) && pLock->bWal ){
      rc = sqlite3_exec(pLock->db, "COMMIT", 0, 0, 0);
      যদি( rc==SQLITE_OK ){
        rc = superlockWalLock(pLock->db, &busy);
      }
    }
  }

  যদি( rc!=SQLITE_OK ){
    sqlite3demo_superunlock(pLock);
    *ppLock = 0;
  }অন্যথা{
    *ppLock = pLock;
  }

  ফেরত rc;
}

/*
** End of example code. Everything below here is the test harness.
**************************************************************************
**************************************************************************
*************************************************************************/


#ifdef SQLITE_TEST

#অন্তর্ভাব "tclsqlite.h"

কাঠামো InterpAndScript {
  Tcl_Interp *interp;
  Tcl_Obj *pScript;
};
প্রকারলেখ কাঠামো InterpAndScript InterpAndScript;

স্থির শূন্য SQLITE_TCLAPI superunlock_del(Client_তথ্য cd){
  sqlite3demo_superunlock((শূন্য *)cd);
}

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI superunlock_cmd(
  Client_তথ্য cd,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  যদি( objc!=1 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "");
    ফেরত TCL_ERROR;
  }
  Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
  ফেরত TCL_OK;
}

স্থির পূর্ণসংখ্যা superlock_busy(শূন্য *pCtx, পূর্ণসংখ্যা nBusy){
  InterpAndScript *p = (InterpAndScript *)pCtx;
  Tcl_Obj *pEval;                 /* Script to evaluate */
  পূর্ণসংখ্যা iVal = 0;                   /* Value to return */

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_গুনতি(pEval);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewIntObj(nBusy));
  Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_GetIntFromObj(p->interp, Tcl_Get_Obj_ফলাফল(p->interp), &iVal);
  Tcl_Decr_Ref_গুনতি(pEval);

  ফেরত iVal;
}

/*
** Tclcmd: sqlite3demo_superlock CMDNAME PATH VFS BUSY-HANDLER-SCRIPT
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI superlock_cmd(
  Client_তথ্য cd,
  Tcl_Interp *interp,
  পূর্ণসংখ্যা objc,
  Tcl_Obj *CONST objv[]
){
  শূন্য *pLock;                    /* Lock context */
  অক্ষর *zPath;
  অক্ষর *zVfs = 0;
  InterpAndScript busy = {0, 0};
  পূর্ণসংখ্যা (*xBusy)(শূন্য*,পূর্ণসংখ্যা) = 0;    /* Busy handler callback */
  পূর্ণসংখ্যা rc;                         /* Return code from sqlite3demo_superlock() */

  যদি( objc<3 || objc>5 ){
    Tcl_Wrong_অঙ্ক_Args(
        interp, 1, objv, "CMDNAME PATH ?VFS? ?BUSY-HANDLER-SCRIPT?");
    ফেরত TCL_ERROR;
  }

  zPath = Tcl_GetString(objv[2]);

  যদি( objc>3 ){
    zVfs = Tcl_GetString(objv[3]);
    যদি( strlen(zVfs)==0 ) zVfs = 0;
  }
  যদি( objc>4 ){
    busy.interp = interp;
    busy.pScript = objv[4];
    xBusy = superlock_busy;
  }

  rc = sqlite3demo_superlock(zPath, zVfs, xBusy, &busy, &pLock);
  প্রতিজ্ঞা( rc==SQLITE_OK || pLock==0 );
  প্রতিজ্ঞা( rc!=SQLITE_OK || pLock!=0 );

  যদি( rc!=SQLITE_OK ){
    বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrStr(পূর্ণসংখ্যা);
    Tcl_Reset_ফলাফল(interp);
    Tcl_Append_ফলাফল(interp, sqlite3ErrStr(rc), ফাঁকা);
    ফেরত TCL_ERROR;
  }

  Tcl_CreateObjCommand(
      interp, Tcl_GetString(objv[1]), superunlock_cmd, pLock, superunlock_del
  );
  Tcl_Set_Obj_ফলাফল(interp, objv[1]);
  ফেরত TCL_OK;
}

পূর্ণসংখ্যা SqliteSuperlock_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3demo_superlock", superlock_cmd, 0, 0);
  ফেরত TCL_OK;
}
#endif
