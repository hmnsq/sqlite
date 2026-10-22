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

#समावेश "sqlite3.h"
#समावेश <string.h>               /* memset(), strlen() */
#समावेश <assert.h>               /* assert() */

/*
** A structure to collect a busy-handler callback and argument and a count
** of the number of times it has been invoked.
*/
काष्ठा SuperlockBusy {
  पूर्णांक (*xBusy)(व्योम*,पूर्णांक);        /* Pointer to busy-handler function */
  व्योम *pBusyArg;                 /* First arg to pass to xBusy */
  पूर्णांक nBusy;                      /* Number of times xBusy has been invoked */
};
प्रकार काष्ठा SuperlockBusy SuperlockBusy;

/*
** An instance of the following structure is allocated for each active
** superlock. The opaque handle returned by sqlite3demo_superlock() is
** actually a pointer to an instance of this structure.
*/
काष्ठा Superlock {
  sqlite3 *db;                    /* Database handle used to lock db */
  पूर्णांक bWal;                       /* True if db is a WAL database */
};
प्रकार काष्ठा Superlock Superlock;

/*
** The pCtx pointer passed to this function is actually a pointer to a
** SuperlockBusy structure. Invoke the busy-handler function encapsulated
** by the structure and return the result.
*/
अटल पूर्णांक superlockBusyHandler(व्योम *pCtx, पूर्णांक UNUSED){
  SuperlockBusy *pBusy = (SuperlockBusy *)pCtx;
  अगर( pBusy->xBusy==0 ) वापस 0;
  वापस pBusy->xBusy(pBusy->pBusyArg, pBusy->nBusy++);
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
अटल पूर्णांक superlockIsWal(Superlock *pLock){
  पूर्णांक rc;                         /* Return Code */
  sqlite3_stmt *pStmt;            /* Compiled PRAGMA journal_mode statement */

  rc = sqlite3_prepare(pLock->db, "PRAGMA main.journal_mode", -1, &pStmt, 0);
  अगर( rc!=SQLITE_OK ) वापस rc;

  pLock->bWal = 0;
  अगर( SQLITE_ROW==sqlite3_step(pStmt) ){
    स्थिर अक्षर *zMode = (स्थिर अक्षर *)sqlite3_column_text(pStmt, 0);
    अगर( zMode && म_माप(zMode)==3 && sqlite3_strnicmp("wal", zMode, 3)==0 ){
      pLock->bWal = 1;
    }
  }

  वापस sqlite3_finalize(pStmt);
}

/*
** Obtain an exclusive shm-lock on nByte bytes starting at offset idx
** of the file fd. If the lock cannot be obtained immediately, invoke
** the busy-handler until either it is obtained or the busy-handler
** callback returns 0.
*/
अटल पूर्णांक superlockShmLock(
  sqlite3_file *fd,               /* Database file handle */
  पूर्णांक idx,                        /* Offset of shm-lock to obtain */
  पूर्णांक nByte,                      /* Number of consective bytes to lock */
  SuperlockBusy *pBusy            /* Busy-handler wrapper object */
){
  पूर्णांक rc;
  पूर्णांक (*xShmLock)(sqlite3_file*, पूर्णांक, पूर्णांक, पूर्णांक) = fd->pMethods->xShmLock;
  करो {
    rc = xShmLock(fd, idx, nByte, SQLITE_SHM_LOCK|SQLITE_SHM_EXCLUSIVE);
  }जबतक( rc==SQLITE_BUSY && superlockBusyHandler((व्योम *)pBusy, 0) );
  वापस rc;
}

/*
** Obtain the extra locks on the database file required for WAL databases.
** Invoke the supplied busy-handler as required.
*/
अटल पूर्णांक superlockWalLock(
  sqlite3 *db,                    /* Database handle open on WAL database */
  SuperlockBusy *pBusy            /* Busy handler wrapper object */
){
  पूर्णांक rc;                         /* Return code */
  sqlite3_file *fd = 0;           /* Main database file handle */
  व्योम अस्थिर *p = 0;           /* Pointer to first page of shared memory */

  /* Obtain a pointer to the sqlite3_file object open on the main db file. */
  rc = sqlite3_file_control(db, "main", SQLITE_FCNTL_FILE_POINTER, (व्योम *)&fd);
  अगर( rc!=SQLITE_OK ) वापस rc;

  /* Obtain the "recovery" lock. Normally, this lock is only obtained by
  ** clients running database recovery.  
  */
  rc = superlockShmLock(fd, 2, 1, pBusy);
  अगर( rc!=SQLITE_OK ) वापस rc;

  /* Zero the start of the first shared-memory page. This means that any
  ** clients that open read or write transactions from this point on will
  ** have to run recovery before proceeding. Since they need the "recovery"
  ** lock that this process is holding to do that, no new read or write
  ** transactions may now be opened. Nor can a checkpoint be run, for the
  ** same reason.
  */
  rc = fd->pMethods->xShmMap(fd, 0, 32*1024, 1, &p);
  अगर( rc!=SQLITE_OK ) वापस rc;
  स_रखो((व्योम *)p, 0, 32);

  /* Obtain exclusive locks on all the "read-lock" slots. Once these locks
  ** are held, it is guaranteed that there are no active reader, writer or 
  ** checkpointer clients.
  */
  rc = superlockShmLock(fd, 3, SQLITE_SHM_NLOCK-3, pBusy);
  वापस rc;
}

/*
** Release a superlock held on a database file. The argument passed to 
** this function must have been obtained from a successful call to
** sqlite3demo_superlock().
*/
व्योम sqlite3demo_superunlock(व्योम *pLock){
  Superlock *p = (Superlock *)pLock;
  अगर( p->bWal ){
    पूर्णांक rc;                         /* Return code */
    पूर्णांक flags = SQLITE_SHM_UNLOCK | SQLITE_SHM_EXCLUSIVE;
    sqlite3_file *fd = 0;
    rc = sqlite3_file_control(p->db, "main", SQLITE_FCNTL_FILE_POINTER, (व्योम *)&fd);
    अगर( rc==SQLITE_OK ){
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
पूर्णांक sqlite3demo_superlock(
  स्थिर अक्षर *zPath,              /* Path to database file to lock */
  स्थिर अक्षर *zVfs,               /* VFS to use to access database file */
  पूर्णांक (*xBusy)(व्योम*,पूर्णांक),        /* Busy handler callback */
  व्योम *pBusyArg,                 /* Context arg for busy handler */
  व्योम **ppLock                   /* OUT: Context to pass to superunlock() */
){
  SuperlockBusy busy = {0, 0, 0}; /* Busy handler wrapper object */
  पूर्णांक rc;                         /* Return code */
  Superlock *pLock;

  pLock = sqlite3_malloc(माप(Superlock));
  अगर( !pLock ) वापस SQLITE_NOMEM;
  स_रखो(pLock, 0, माप(Superlock));

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
  अगर( rc==SQLITE_OK ){
    busy.xBusy = xBusy;
    busy.pBusyArg = pBusyArg;
    sqlite3_busy_handler(pLock->db, superlockBusyHandler, (व्योम *)&busy);
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
  अगर( rc==SQLITE_OK ){
    अगर( SQLITE_OK==(rc = superlockIsWal(pLock)) && pLock->bWal ){
      rc = sqlite3_exec(pLock->db, "COMMIT", 0, 0, 0);
      अगर( rc==SQLITE_OK ){
        rc = superlockWalLock(pLock->db, &busy);
      }
    }
  }

  अगर( rc!=SQLITE_OK ){
    sqlite3demo_superunlock(pLock);
    *ppLock = 0;
  }अन्यथा{
    *ppLock = pLock;
  }

  वापस rc;
}

/*
** End of example code. Everything below here is the test harness.
**************************************************************************
**************************************************************************
*************************************************************************/


#अगर_घोषित SQLITE_TEST

#समावेश "tclsqlite.h"

काष्ठा InterpAndScript {
  Tcl_Interp *interp;
  Tcl_Obj *pScript;
};
प्रकार काष्ठा InterpAndScript InterpAndScript;

अटल व्योम SQLITE_TCLAPI superunlock_del(Client_तथ्य cd){
  sqlite3demo_superunlock((व्योम *)cd);
}

अटल पूर्णांक SQLITE_TCLAPI superunlock_cmd(
  Client_तथ्य cd,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  अगर( objc!=1 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "");
    वापस TCL_ERROR;
  }
  Tcl_DeleteCommand(interp, Tcl_GetString(objv[0]));
  वापस TCL_OK;
}

अटल पूर्णांक superlock_busy(व्योम *pCtx, पूर्णांक nBusy){
  InterpAndScript *p = (InterpAndScript *)pCtx;
  Tcl_Obj *pEval;                 /* Script to evaluate */
  पूर्णांक iVal = 0;                   /* Value to return */

  pEval = Tcl_DuplicateObj(p->pScript);
  Tcl_Incr_Ref_गिनती(pEval);
  Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewIntObj(nBusy));
  Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
  Tcl_GetIntFromObj(p->interp, Tcl_Get_Obj_परिणाम(p->interp), &iVal);
  Tcl_Decr_Ref_गिनती(pEval);

  वापस iVal;
}

/*
** Tclcmd: sqlite3demo_superlock CMDNAME PATH VFS BUSY-HANDLER-SCRIPT
*/
अटल पूर्णांक SQLITE_TCLAPI superlock_cmd(
  Client_तथ्य cd,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  व्योम *pLock;                    /* Lock context */
  अक्षर *zPath;
  अक्षर *zVfs = 0;
  InterpAndScript busy = {0, 0};
  पूर्णांक (*xBusy)(व्योम*,पूर्णांक) = 0;    /* Busy handler callback */
  पूर्णांक rc;                         /* Return code from sqlite3demo_superlock() */

  अगर( objc<3 || objc>5 ){
    Tcl_Wrong_अंक_Args(
        interp, 1, objv, "CMDNAME PATH ?VFS? ?BUSY-HANDLER-SCRIPT?");
    वापस TCL_ERROR;
  }

  zPath = Tcl_GetString(objv[2]);

  अगर( objc>3 ){
    zVfs = Tcl_GetString(objv[3]);
    अगर( म_माप(zVfs)==0 ) zVfs = 0;
  }
  अगर( objc>4 ){
    busy.interp = interp;
    busy.pScript = objv[4];
    xBusy = superlock_busy;
  }

  rc = sqlite3demo_superlock(zPath, zVfs, xBusy, &busy, &pLock);
  निश्चित( rc==SQLITE_OK || pLock==0 );
  निश्चित( rc!=SQLITE_OK || pLock!=0 );

  अगर( rc!=SQLITE_OK ){
    बाह्य स्थिर अक्षर *sqlite3ErrStr(पूर्णांक);
    Tcl_Reset_परिणाम(interp);
    Tcl_Append_परिणाम(interp, sqlite3ErrStr(rc), शून्य);
    वापस TCL_ERROR;
  }

  Tcl_CreateObjCommand(
      interp, Tcl_GetString(objv[1]), superunlock_cmd, pLock, superunlock_del
  );
  Tcl_Set_Obj_परिणाम(interp, objv[1]);
  वापस TCL_OK;
}

पूर्णांक SqliteSuperlock_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3demo_superlock", superlock_cmd, 0, 0);
  वापस TCL_OK;
}
#पूर्ण_अगर
