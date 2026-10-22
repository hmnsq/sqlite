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
बाह्य ध्रुव वर्ण *sqlite3ErrName(पूर्णाङ्क);

/* These functions are implemented in test1.c. */
बाह्य पूर्णाङ्क getDbPointer(Tcl_Interp *, ध्रुव वर्ण *, sqlite3 **);

स्थिर पूर्णाङ्क SQLITE_TCLAPI backupTestCmd(
  ClientData clientData, 
  Tcl_Interp *interp, 
  पूर्णाङ्क objc,
  Tcl_Obj *ध्रुव*objv
){
  गणना BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  संरचना BackupSubCommand {
    ध्रुव वर्ण *zCmd;
    गणना BackupSubCommandEnum eCmd;
    पूर्णाङ्क nArg;
    ध्रुव वर्ण *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)clientData;
  पूर्णाङ्क iCmd;
  पूर्णाङ्क rc;

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, परिमाण(aSub[0]), "option", 0, &iCmd
  );
  जइ( rc!=TCL_OK ){
    प्रत्यागम rc;
  }
  जइ( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_अङ्कः_Args(interp, 2, objv, aSub[iCmd].zArg);
    प्रत्यागम TCL_ERROR;
  }

  विकल्प( aSub[iCmd].eCmd ){

    पक्ष BACKUP_FINISH: {
      ध्रुव वर्ण *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);
      भङ्ग;
    }

    पक्ष BACKUP_STEP: {
      पूर्णाङ्क nPage;
      जइ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        प्रत्यागम TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_SetResult(interp, (वर्ण *)sqlite3ErrName(rc), TCL_STATIC);
      भङ्ग;
    }

    पक्ष BACKUP_REMAINING:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      भङ्ग;

    पक्ष BACKUP_PAGECOUNT:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      भङ्ग;
  }

  प्रत्यागम TCL_OK;
}

स्थिर शून्य SQLITE_TCLAPI backupTestFinish(ClientData clientData){
  sqlite3_backup *pBackup = (sqlite3_backup *)clientData;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
स्थिर पूर्णाङ्क SQLITE_TCLAPI backupTestInit(
  ClientData clientData, 
  Tcl_Interp *interp, 
  पूर्णाङ्क objc,
  Tcl_Obj *ध्रुव*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  ध्रुव वर्ण *zDestName;
  ध्रुव वर्ण *zSrcName;
  ध्रुव वर्ण *zCmd;

  जइ( objc!=6 ){
    Tcl_Wrong_अङ्कः_Args(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    प्रत्यागम TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  जइ( !pBackup ){
    Tcl_AppendResult(interp, "sqlite3_backup_init() failed", रिक्त);
    प्रत्यागम TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_SetObjResult(interp, objv[1]);
  प्रत्यागम TCL_OK;
}

पूर्णाङ्क Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  प्रत्यागम TCL_OK;
}
