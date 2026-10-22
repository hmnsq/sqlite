/*
** 2009 January 28
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains test logic for the sqlite3_backup() interface.
**
*/

#अन्तर्भाव "tclsqlite.h"
#अन्तर्भाव "sqlite3.h"
#अन्तर्भाव <assert.h>

/* These functions are implemented in main.c. */
बाह्य स्थिरांक अक्षर *sqlite3ErrName(पूर्णांक);

/* These functions are implemented in test1.c. */
बाह्य पूर्णांक getDbPointer(Tcl_Interp *, स्थिरांक अक्षर *, sqlite3 **);

स्थिर पूर्णांक SQLITE_TCLAPI backupTestCmd(
  ClientData clientData, 
  Tcl_Interp *interp, 
  पूर्णांक objc,
  Tcl_Obj *स्थिरांक*objv
){
  गणना BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  संरचना BackupSubCommand {
    स्थिरांक अक्षर *zCmd;
    गणना BackupSubCommandEnum eCmd;
    पूर्णांक nArg;
    स्थिरांक अक्षर *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)clientData;
  पूर्णांक iCmd;
  पूर्णांक rc;

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, आकार(aSub[0]), "option", 0, &iCmd
  );
  जर( rc!=TCL_OK ){
    परत rc;
  }
  जर( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, aSub[iCmd].zArg);
    परत TCL_ERROR;
  }

  निवड( aSub[iCmd].eCmd ){

    स्थिती BACKUP_FINISH: {
      स्थिरांक अक्षर *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);
      थांबा;
    }

    स्थिती BACKUP_STEP: {
      पूर्णांक nPage;
      जर( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        परत TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_SetResult(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);
      थांबा;
    }

    स्थिती BACKUP_REMAINING:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      थांबा;

    स्थिती BACKUP_PAGECOUNT:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      थांबा;
  }

  परत TCL_OK;
}

स्थिर शून्य SQLITE_TCLAPI backupTestFinish(ClientData clientData){
  sqlite3_backup *pBackup = (sqlite3_backup *)clientData;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
स्थिर पूर्णांक SQLITE_TCLAPI backupTestInit(
  ClientData clientData, 
  Tcl_Interp *interp, 
  पूर्णांक objc,
  Tcl_Obj *स्थिरांक*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  स्थिरांक अक्षर *zDestName;
  स्थिरांक अक्षर *zSrcName;
  स्थिरांक अक्षर *zCmd;

  जर( objc!=6 ){
    Tcl_Wrong_अंक_Args(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    परत TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  जर( !pBackup ){
    Tcl_AppendResult(interp, "sqlite3_backup_init() failed", रिक्त);
    परत TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_SetObjResult(interp, objv[1]);
  परत TCL_OK;
}

पूर्णांक Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  परत TCL_OK;
}
