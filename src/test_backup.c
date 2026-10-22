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

#অন্তর্ভাব "tclsqlite.h"
#অন্তর্ভাব "sqlite3.h"
#অন্তর্ভাব <assert.h>

/* These functions are implemented in main.c. */
বাহ্যিক ধ্রুবক অক্ষর *sqlite3ErrName(পূর্ণসংখ্যা);

/* These functions are implemented in test1.c. */
বাহ্যিক পূর্ণসংখ্যা getDbPointer(Tcl_Interp *, ধ্রুবক অক্ষর *, sqlite3 **);

স্থির পূর্ণসংখ্যা SQLITE_TCLAPI backupTestCmd(
  Client_তথ্য client_তথ্য, 
  Tcl_Interp *interp, 
  পূর্ণসংখ্যা objc,
  Tcl_Obj *ধ্রুবক*objv
){
  গণনা BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  কাঠামো BackupSubCommand {
    ধ্রুবক অক্ষর *zCmd;
    গণনা BackupSubCommandEnum eCmd;
    পূর্ণসংখ্যা nArg;
    ধ্রুবক অক্ষর *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)client_তথ্য;
  পূর্ণসংখ্যা iCmd;
  পূর্ণসংখ্যা rc;

  rc = Tcl_Get_সূচক_From_Obj_Struct(
      interp, objv[1], aSub, আকার(aSub[0]), "option", 0, &iCmd
  );
  যদি( rc!=TCL_OK ){
    ফেরত rc;
  }
  যদি( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, aSub[iCmd].zArg);
    ফেরত TCL_ERROR;
  }

  নির্বাচন( aSub[iCmd].eCmd ){

    ক্ষেত্র BACKUP_FINISH: {
      ধ্রুবক অক্ষর *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3ErrName(rc), TCL_STATIC);
      বিরতি;
    }

    ক্ষেত্র BACKUP_STEP: {
      পূর্ণসংখ্যা nPage;
      যদি( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        ফেরত TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_Set_ফলাফল(interp, (অক্ষর *)sqlite3ErrName(rc), TCL_STATIC);
      বিরতি;
    }

    ক্ষেত্র BACKUP_REMAINING:
      Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      বিরতি;

    ক্ষেত্র BACKUP_PAGECOUNT:
      Tcl_Set_Obj_ফলাফল(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      বিরতি;
  }

  ফেরত TCL_OK;
}

স্থির শূন্য SQLITE_TCLAPI backupTestFinish(Client_তথ্য client_তথ্য){
  sqlite3_backup *pBackup = (sqlite3_backup *)client_তথ্য;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
স্থির পূর্ণসংখ্যা SQLITE_TCLAPI backupTestInit(
  Client_তথ্য client_তথ্য, 
  Tcl_Interp *interp, 
  পূর্ণসংখ্যা objc,
  Tcl_Obj *ধ্রুবক*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  ধ্রুবক অক্ষর *zDestName;
  ধ্রুবক অক্ষর *zSrcName;
  ধ্রুবক অক্ষর *zCmd;

  যদি( objc!=6 ){
    Tcl_Wrong_অঙ্ক_Args(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    ফেরত TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  যদি( !pBackup ){
    Tcl_Append_ফলাফল(interp, "sqlite3_backup_init() failed", ফাঁকা);
    ফেরত TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_Set_Obj_ফলাফল(interp, objv[1]);
  ফেরত TCL_OK;
}

পূর্ণসংখ্যা Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  ফেরত TCL_OK;
}
