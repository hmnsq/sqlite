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

#ضمن "tclsqlite.h"
#ضمن "sqlite3.h"
#ضمن <assert.h>

/* These functions are implemented in main.c. */
خارجي ثابت حرف *sqlite3ErrName(صحيح);

/* These functions are implemented in test1.c. */
خارجي صحيح getDbPointer(Tcl_Interp *, ثابت حرف *, sqlite3 **);

ساكن صحيح SQLITE_TCLAPI backupTestCmd(
  ClientData clientData, 
  Tcl_Interp *interp, 
  صحيح objc,
  Tcl_Obj *ثابت*objv
){
  تعداد BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  بنية BackupSubCommand {
    ثابت حرف *zCmd;
    تعداد BackupSubCommandEnum eCmd;
    صحيح nArg;
    ثابت حرف *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)clientData;
  صحيح iCmd;
  صحيح rc;

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, حجم(aSub[0]), "option", 0, &iCmd
  );
  𐤀𐤌( rc!=TCL_OK ){
    أرجع rc;
  }
  𐤀𐤌( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_WrongNumArgs(interp, 2, objv, aSub[iCmd].zArg);
    أرجع TCL_ERROR;
  }

  بدل( aSub[iCmd].eCmd ){

    حالة BACKUP_FINISH: {
      ثابت حرف *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
      اكسر;
    }

    حالة BACKUP_STEP: {
      صحيح nPage;
      𐤀𐤌( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        أرجع TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_SetResult(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
      اكسر;
    }

    حالة BACKUP_REMAINING:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      اكسر;

    حالة BACKUP_PAGECOUNT:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      اكسر;
  }

  أرجع TCL_OK;
}

ساكن فراغ SQLITE_TCLAPI backupTestFinish(ClientData clientData){
  sqlite3_backup *pBackup = (sqlite3_backup *)clientData;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
ساكن صحيح SQLITE_TCLAPI backupTestInit(
  ClientData clientData, 
  Tcl_Interp *interp, 
  صحيح objc,
  Tcl_Obj *ثابت*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  ثابت حرف *zDestName;
  ثابت حرف *zSrcName;
  ثابت حرف *zCmd;

  𐤀𐤌( objc!=6 ){
    Tcl_WrongNumArgs(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    أرجع TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  𐤀𐤌( !pBackup ){
    Tcl_AppendResult(interp, "sqlite3_backup_init() failed", عدم);
    أرجع TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_SetObjResult(interp, objv[1]);
  أرجع TCL_OK;
}

صحيح Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  أرجع TCL_OK;
}
