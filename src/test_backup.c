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

#شامل "tclsqlite.h"
#شامل "sqlite3.h"
#شامل <assert.h>

/* These functions are implemented in main.c. */
بیرونی ثابت نویسه *sqlite3ErrName(صحیح);

/* These functions are implemented in test1.c. */
بیرونی صحیح getDbPointer(Tcl_Interp *, ثابت نویسه *, sqlite3 **);

ایستا صحیح SQLITE_TCLAPI backupTestCmd(
  ClientData clientData, 
  Tcl_Interp *interp, 
  صحیح objc,
  Tcl_Obj *ثابت*objv
){
  شمارش BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  ساختار BackupSubCommand {
    ثابت نویسه *zCmd;
    شمارش BackupSubCommandEnum eCmd;
    صحیح nArg;
    ثابت نویسه *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)clientData;
  صحیح iCmd;
  صحیح rc;

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, اندازه(aSub[0]), "option", 0, &iCmd
  );
  اگر( rc!=TCL_OK ){
    بازگشت rc;
  }
  اگر( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_رقم_Args(interp, 2, objv, aSub[iCmd].zArg);
    بازگشت TCL_ERROR;
  }

  انتخاب( aSub[iCmd].eCmd ){

    حالت BACKUP_FINISH: {
      ثابت نویسه *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_STATIC);
      بشکن;
    }

    حالت BACKUP_STEP: {
      صحیح nPage;
      اگر( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        بازگشت TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_SetResult(interp, (نویسه *)sqlite3ErrName(rc), TCL_STATIC);
      بشکن;
    }

    حالت BACKUP_REMAINING:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      بشکن;

    حالت BACKUP_PAGECOUNT:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      بشکن;
  }

  بازگشت TCL_OK;
}

ایستا تهی SQLITE_TCLAPI backupTestFinish(ClientData clientData){
  sqlite3_backup *pBackup = (sqlite3_backup *)clientData;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
ایستا صحیح SQLITE_TCLAPI backupTestInit(
  ClientData clientData, 
  Tcl_Interp *interp, 
  صحیح objc,
  Tcl_Obj *ثابت*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  ثابت نویسه *zDestName;
  ثابت نویسه *zSrcName;
  ثابت نویسه *zCmd;

  اگر( objc!=6 ){
    Tcl_Wrong_رقم_Args(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    بازگشت TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  اگر( !pBackup ){
    Tcl_AppendResult(interp, "sqlite3_backup_init() failed", پوچ);
    بازگشت TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_SetObjResult(interp, objv[1]);
  بازگشت TCL_OK;
}

صحیح Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  بازگشت TCL_OK;
}
