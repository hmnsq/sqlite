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

#شامل "sqlite3.h"
#شامل <string.h>               /* memset(), strlen() */
#شامل <assert.h>               /* assert() */

/*
** A structure to collect a busy-handler callback and argument and a count
** of the number of times it has been invoked.
*/
ساخت SuperlockBusy {
  صحیح (*xBusy)(خالی*,صحیح);        /* Pointer to busy-handler function */
  خالی *pBusyArg;                 /* First arg to pass to xBusy */
  صحیح nBusy;                      /* Number of times xBusy has been invoked */
};
قسمتعریف ساخت SuperlockBusy SuperlockBusy;

/*
** An instance of the following structure is allocated for each active
** superlock. The opaque handle returned by sqlite3demo_superlock() is
** actually a pointer to an instance of this structure.
*/
ساخت Superlock {
  sqlite3 *db;                    /* Database handle used to lock db */
  صحیح bWal;                       /* True if db is a WAL database */
};
قسمتعریف ساخت Superlock Superlock;

/*
** The pCtx pointer passed to this function is actually a pointer to a
** SuperlockBusy structure. Invoke the busy-handler function encapsulated
** by the structure and return the result.
*/
ساکن صحیح superlockBusyHandler(خالی *pCtx, صحیح غیر_مستعمل_2){
  SuperlockBusy *pBusy = (SuperlockBusy *)pCtx;
  اگر( pBusy->xBusy==0 ) واپس 0;
  واپس pBusy->xBusy(pBusy->pBusyArg, pBusy->nBusy++);
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
ساکن صحیح superlockIsWal(Superlock *p_تالا){
  صحیح rc;                         /* Return Code */
  sqlite3_stmt *pStmt;            /* Compiled PRAGMA journal_mode statement */

  rc = sqlite3_prepare(p_تالا->db, "PRAGMA main.journal_mode", -1, &pStmt, 0);
  اگر( rc!=SQLITE_OK ) واپس rc;

  p_تالا->bWal = 0;
  اگر( SQLITE_صف==sqlite3_قدم(pStmt) ){
    ثابت حرف *z_منوال = (ثابت حرف *)sqlite3_عمود_متن(pStmt, 0);
    اگر( z_منوال && لڑی_لمبائی(z_منوال)==3 && sqlite3_strnicmp("wal", z_منوال, 3)==0 ){
      p_تالا->bWal = 1;
    }
  }

  واپس sqlite3_finalize(pStmt);
}

/*
** Obtain an exclusive shm-lock on nByte bytes starting at offset idx
** of the file fd. If the lock cannot be obtained immediately, invoke
** the busy-handler until either it is obtained or the busy-handler
** callback returns 0.
*/
ساکن صحیح superlock_Shm_تالا(
  sqlite3_file *fd,               /* Database file handle */
  صحیح idx,                        /* Offset of shm-lock to obtain */
  صحیح nByte,                      /* Number of consective bytes to lock */
  SuperlockBusy *pBusy            /* Busy-handler wrapper object */
){
  صحیح rc;
  صحیح (*x_Shm_تالا)(sqlite3_file*, صحیح, صحیح, صحیح) = fd->pMethods->x_Shm_تالا;
  کرو {
    rc = x_Shm_تالا(fd, idx, nByte, SQLITE_SHM_تالا|SQLITE_SHM_EXCLUSIVE);
  }جب( rc==SQLITE_BUSY && superlockBusyHandler((خالی *)pBusy, 0) );
  واپس rc;
}

/*
** Obtain the extra locks on the database file required for WAL databases.
** Invoke the supplied busy-handler as required.
*/
ساکن صحیح superlock_Wal_تالا(
  sqlite3 *db,                    /* Database handle open on WAL database */
  SuperlockBusy *pBusy            /* Busy handler wrapper object */
){
  صحیح rc;                         /* Return code */
  sqlite3_file *fd = 0;           /* Main database file handle */
  خالی متلون *p = 0;           /* Pointer to first page of shared memory */

  /* Obtain a pointer to the sqlite3_file object open on the main db file. */
  rc = sqlite3_file_control(db, "main", SQLITE_FCNTL_FILE_اشارہ_گر, (خالی *)&fd);
  اگر( rc!=SQLITE_OK ) واپس rc;

  /* Obtain the "recovery" lock. Normally, this lock is only obtained by
  ** clients running database recovery.  
  */
  rc = superlock_Shm_تالا(fd, 2, 1, pBusy);
  اگر( rc!=SQLITE_OK ) واپس rc;

  /* Zero the start of the first shared-memory page. This means that any
  ** clients that open read or write transactions from this point on will
  ** have to run recovery before proceeding. Since they need the "recovery"
  ** lock that this process is holding to do that, no new read or write
  ** transactions may now be opened. Nor can a checkpoint be run, for the
  ** same reason.
  */
  rc = fd->pMethods->xShmMap(fd, 0, 32*1024, 1, &p);
  اگر( rc!=SQLITE_OK ) واپس rc;
  memset((خالی *)p, 0, 32);

  /* Obtain exclusive locks on all the "read-lock" slots. Once these locks
  ** are held, it is guaranteed that there are no active reader, writer or 
  ** checkpointer clients.
  */
  rc = superlock_Shm_تالا(fd, 3, SQLITE_SHM_NLOCK-3, pBusy);
  واپس rc;
}

/*
** Release a superlock held on a database file. The argument passed to 
** this function must have been obtained from a successful call to
** sqlite3demo_superlock().
*/
خالی sqlite3demo_superunlock(خالی *p_تالا){
  Superlock *p = (Superlock *)p_تالا;
  اگر( p->bWal ){
    صحیح rc;                         /* Return code */
    صحیح flags = SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE;
    sqlite3_file *fd = 0;
    rc = sqlite3_file_control(p->db, "main", SQLITE_FCNTL_FILE_اشارہ_گر, (خالی *)&fd);
    اگر( rc==SQLITE_OK ){
      fd->pMethods->x_Shm_تالا(fd, 2, 1, flags);
      fd->pMethods->x_Shm_تالا(fd, 3, SQLITE_SHM_NLOCK-3, flags);
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
صحیح sqlite3demo_superlock(
  ثابت حرف *z_راستہ,              /* Path to database file to lock */
  ثابت حرف *zVfs,               /* VFS to use to access database file */
  صحیح (*xBusy)(خالی*,صحیح),        /* Busy handler callback */
  خالی *pBusyArg,                 /* Context arg for busy handler */
  خالی **pp_تالا                   /* OUT: Context to pass to superunlock() */
){
  SuperlockBusy busy = {0, 0, 0}; /* Busy handler wrapper object */
  صحیح rc;                         /* Return code */
  Superlock *p_تالا;

  p_تالا = sqlite3_malloc(حجم(Superlock));
  اگر( !p_تالا ) واپس SQLITE_NOMEM;
  memset(p_تالا, 0, حجم(Superlock));

  /* Open a database handle on the file to superlock. */
  rc = sqlite3_open_v2(
      z_راستہ, &p_تالا->db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, zVfs
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
  اگر( rc==SQLITE_OK ){
    busy.xBusy = xBusy;
    busy.pBusyArg = pBusyArg;
    sqlite3_busy_handler(p_تالا->db, superlockBusyHandler, (خالی *)&busy);
    rc = sqlite3_exec(p_تالا->db, "BEGIN EXCLUSIVE", 0, 0, 0);
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
  اگر( rc==SQLITE_OK ){
    اگر( SQLITE_OK==(rc = superlockIsWal(p_تالا)) && p_تالا->bWal ){
      rc = sqlite3_exec(p_تالا->db, "COMMIT", 0, 0, 0);
      اگر( rc==SQLITE_OK ){
        rc = superlock_Wal_تالا(p_تالا->db, &busy);
      }
    }
  }

  اگر( rc!=SQLITE_OK ){
    sqlite3demo_superunlock(p_تالا);
    *pp_تالا = 0;
  }ورنہ{
    *pp_تالا = p_تالا;
  }

  واپس rc;
}

/*
** End of example code. Everything below here is the test harness.
**************************************************************************
**************************************************************************
*************************************************************************/


#اگر_تعریف_شدہ SQLITE_TEST

#شامل "tclsqlite.h"

ساخت InterpAndScript {
  Tcl_Interp *interp;
  Tcl_Obj *pScript;
};
قسمتعریف ساخت InterpAndScript InterpAndScript;

ساکن خالی SQLITE_TCLAPI superunlock_del(Client_ڈیٹا cd){
  sqlite3demo_superunlock((خالی *)cd);
}

ساکن صحیح SQLITE_TCLAPI superunlock_cmd(
  Client_ڈیٹا cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  اگر( objc!=1 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "");
    واپس TCL_ERROR;
  }
  Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
  واپس TCL_OK;
}

ساکن صحیح superlock_busy(خالی *pCtx, صحیح nBusy){
  InterpAndScript *p = (InterpAndScript *)pCtx;
  Tcl_Obj *pEval;                 /* Script to evaluate */
  صحیح iVal = 0;                   /* Value to return */

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_گنتی(pEval);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewIntObj(nBusy));
  Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_GetIntFromObj(p->interp, Tcl_Get_Obj_نتیجہ(p->interp), &iVal);
  Tcl_Decr_Ref_گنتی(pEval);

  واپس iVal;
}

/*
** Tclcmd: sqlite3demo_superlock CMDNAME PATH VFS BUSY-HANDLER-SCRIPT
*/
ساکن صحیح SQLITE_TCLAPI superlock_cmd(
  Client_ڈیٹا cd,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  خالی *p_تالا;                    /* Lock context */
  حرف *z_راستہ;
  حرف *zVfs = 0;
  InterpAndScript busy = {0, 0};
  صحیح (*xBusy)(خالی*,صحیح) = 0;    /* Busy handler callback */
  صحیح rc;                         /* Return code from sqlite3demo_superlock() */

  اگر( objc<3 || objc>5 ){
    Tcl_Wrong_عدد_دلائل(
        interp, 1, objv, "CMDNAME PATH ?VFS? ?BUSY-HANDLER-SCRIPT?");
    واپس TCL_ERROR;
  }

  z_راستہ = Tcl_GetString(objv[2]);

  اگر( objc>3 ){
    zVfs = Tcl_GetString(objv[3]);
    اگر( لڑی_لمبائی(zVfs)==0 ) zVfs = 0;
  }
  اگر( objc>4 ){
    busy.interp = interp;
    busy.pScript = objv[4];
    xBusy = superlock_busy;
  }

  rc = sqlite3demo_superlock(z_راستہ, zVfs, xBusy, &busy, &p_تالا);
  أكد( rc==SQLITE_OK || p_تالا==0 );
  أكد( rc!=SQLITE_OK || p_تالا!=0 );

  اگر( rc!=SQLITE_OK ){
    خارجی ثابت حرف *sqlite3ErrStr(صحیح);
    Tcl_Reset_نتیجہ(interp);
    Tcl_Append_نتیجہ(interp, sqlite3ErrStr(rc), تہی);
    واپس TCL_ERROR;
  }

  Tcl_CreateObjCommand(
      interp, Tcl_GetString(objv[1]), superunlock_cmd, p_تالا, superunlock_del
  );
  Tcl_سیٹ_Obj_نتیجہ(interp, objv[1]);
  واپس TCL_OK;
}

صحیح SqliteSuperlock_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3demo_superlock", superlock_cmd, 0, 0);
  واپس TCL_OK;
}
#اگر_ختم
