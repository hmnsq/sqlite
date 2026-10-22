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
خارجی ثابت حرف *sqlite3ErrName(صحیح);

/* These functions are implemented in test1.c. */
خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp *, ثابت حرف *, sqlite3 **);

ساکن صحیح SQLITE_TCLAPI backupTestCmd(
  Client_ڈیٹا client_ڈیٹا, 
  Tcl_Interp *interp, 
  صحیح objc,
  Tcl_Obj *ثابت*objv
){
  شمار BackupSubCommandEnum {
    BACKUP_قدم, BACKUP_اختتام, BACKUP_بقایا, BACKUP_PAGECOUNT
  };
  ساخت BackupSubCommand {
    ثابت حرف *zCmd;
    شمار BackupSubCommandEnum eCmd;
    صحیح nArg;
    ثابت حرف *zArg;
  } aSub[] = {
    {"step",      BACKUP_قدم      , 1, "npage" },
    {"finish",    BACKUP_اختتام    , 0, ""      },
    {"remaining", BACKUP_بقایا , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)client_ڈیٹا;
  صحیح iCmd;
  صحیح rc;

  rc = Tcl_Get_اشاریہ_From_Obj_Struct(
      interp, objv[1], aSub, حجم(aSub[0]), "option", 0, &iCmd
  );
  اگر( rc!=TCL_OK ){
    واپس rc;
  }
  اگر( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_عدد_دلائل(interp, 2, objv, aSub[iCmd].zArg);
    واپس TCL_ERROR;
  }

  انتخاب( aSub[iCmd].eCmd ){

    حالت BACKUP_اختتام: {
      ثابت حرف *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_سیٹ_Command_Info(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_اختتام(p);
      Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
      توڑو;
    }

    حالت BACKUP_قدم: {
      صحیح n_صفحہ;
      اگر( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &n_صفحہ) ){
        واپس TCL_ERROR;
      }
      rc = sqlite3_backup_قدم(p, n_صفحہ);
      Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
      توڑو;
    }

    حالت BACKUP_بقایا:
      Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(sqlite3_backup_بقایا(p)));
      توڑو;

    حالت BACKUP_PAGECOUNT:
      Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      توڑو;
  }

  واپس TCL_OK;
}

ساکن خالی SQLITE_TCLAPI backup_Test_اختتام(Client_ڈیٹا client_ڈیٹا){
  sqlite3_backup *pBackup = (sqlite3_backup *)client_ڈیٹا;
  sqlite3_backup_اختتام(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
ساکن صحیح SQLITE_TCLAPI backupTestInit(
  Client_ڈیٹا client_ڈیٹا, 
  Tcl_Interp *interp, 
  صحیح objc,
  Tcl_Obj *ثابت*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  ثابت حرف *zDestName;
  ثابت حرف *zSrcName;
  ثابت حرف *zCmd;

  اگر( objc!=6 ){
    Tcl_Wrong_عدد_دلائل(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    واپس TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  get_Db_اشارہ_گر(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  get_Db_اشارہ_گر(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  اگر( !pBackup ){
    Tcl_Append_نتیجہ(interp, "sqlite3_backup_init() failed", تہی);
    واپس TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backup_Test_اختتام);
  Tcl_سیٹ_Obj_نتیجہ(interp, objv[1]);
  واپس TCL_OK;
}

صحیح Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  واپس TCL_OK;
}
