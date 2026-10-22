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

#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ <assert.h>

/* These functions are implemented in main.c. */
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);

/* These functions are implemented in test1.c. */
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp *, ಧ್ರುವ ವರ್ಣ *, sqlite3 **);

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI backupTestCmd(
  ClientData clientData, 
  Tcl_Interp *interp, 
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *ಧ್ರುವ*objv
){
  ಗಣನಾ BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  ಸಂರಚನಾ BackupSubCommand {
    ಧ್ರುವ ವರ್ಣ *zCmd;
    ಗಣನಾ BackupSubCommandEnum eCmd;
    ಪೂರ್ಣಾಙ್ಕ nArg;
    ಧ್ರುವ ವರ್ಣ *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)clientData;
  ಪೂರ್ಣಾಙ್ಕ iCmd;
  ಪೂರ್ಣಾಙ್ಕ rc;

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, ಪರಿಮಾಣ(aSub[0]), "option", 0, &iCmd
  );
  ಯದಿ( rc!=TCL_OK ){
    ಪ್ರತ್ಯಾಗಮ rc;
  }
  ಯದಿ( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 2, objv, aSub[iCmd].zArg);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ವಿಕಲ್ಪ( aSub[iCmd].eCmd ){

    ಪಕ್ಷ BACKUP_FINISH: {
      ಧ್ರುವ ವರ್ಣ *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_STATIC);
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ BACKUP_STEP: {
      ಪೂರ್ಣಾಙ್ಕ nPage;
      ಯದಿ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_SetResult(interp, (ವರ್ಣ *)sqlite3ErrName(rc), TCL_STATIC);
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ BACKUP_REMAINING:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      ಭಙ್ಗ;

    ಪಕ್ಷ BACKUP_PAGECOUNT:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      ಭಙ್ಗ;
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_TCLAPI backupTestFinish(ClientData clientData){
  sqlite3_backup *pBackup = (sqlite3_backup *)clientData;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI backupTestInit(
  ClientData clientData, 
  Tcl_Interp *interp, 
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *ಧ್ರುವ*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  ಧ್ರುವ ವರ್ಣ *zDestName;
  ಧ್ರುವ ವರ್ಣ *zSrcName;
  ಧ್ರುವ ವರ್ಣ *zCmd;

  ಯದಿ( objc!=6 ){
    Tcl_Wrong_ಅಂಕೆ_Args(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  ಯದಿ( !pBackup ){
    Tcl_AppendResult(interp, "sqlite3_backup_init() failed", ರಿಕ್ತ);
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_SetObjResult(interp, objv[1]);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಪೂರ್ಣಾಙ್ಕ Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
