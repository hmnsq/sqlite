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
خارجي صحيح get_Db_اشارہ_گر(Tcl_Interp *, ثابت حرف *, sqlite3 **);

ساكن صحيح SQLITE_TCLAPI backupTestCmd(
  Client_انکڑے client_انکڑے, 
  Tcl_Interp *interp, 
  صحيح objc,
  Tcl_Obj *ثابت*objv
){
  تعداد BackupSubCommandEnum {
    BACKUP_قدم, BACKUP_انت, BACKUP_بچیا, BACKUP_PAGECOUNT
  };
  بنية BackupSubCommand {
    ثابت حرف *zCmd;
    تعداد BackupSubCommandEnum eCmd;
    صحيح nArg;
    ثابت حرف *zArg;
  } aSub[] = {
    {"step",      BACKUP_قدم      , 1, "npage" },
    {"finish",    BACKUP_انت    , 0, ""      },
    {"remaining", BACKUP_بچیا , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)client_انکڑے;
  صحيح iCmd;
  صحيح rc;

  rc = Tcl_Get_اشاریہ_From_Obj_Struct(
      interp, objv[1], aSub, حجم(aSub[0]), "option", 0, &iCmd
  );
  إذا( rc!=TCL_OK ){
    أرجع rc;
  }
  إذا( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_عدد_دلیلاں(interp, 2, objv, aSub[iCmd].zArg);
    أرجع TCL_ERROR;
  }

  بدل( aSub[iCmd].eCmd ){

    حالة BACKUP_انت: {
      ثابت حرف *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_سیٹ_Command_Info(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_انت(p);
      Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
      اكسر;
    }

    حالة BACKUP_قدم: {
      صحيح n_ورقہ;
      إذا( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &n_ورقہ) ){
        أرجع TCL_ERROR;
      }
      rc = sqlite3_backup_قدم(p, n_ورقہ);
      Tcl_سیٹ_نتیجہ(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
      اكسر;
    }

    حالة BACKUP_بچیا:
      Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(sqlite3_backup_بچیا(p)));
      اكسر;

    حالة BACKUP_PAGECOUNT:
      Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      اكسر;
  }

  أرجع TCL_OK;
}

ساكن فراغ SQLITE_TCLAPI backup_Test_انت(Client_انکڑے client_انکڑے){
  sqlite3_backup *pBackup = (sqlite3_backup *)client_انکڑے;
  sqlite3_backup_انت(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
ساكن صحيح SQLITE_TCLAPI backupTestInit(
  Client_انکڑے client_انکڑے, 
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

  إذا( objc!=6 ){
    Tcl_Wrong_عدد_دلیلاں(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    أرجع TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  get_Db_اشارہ_گر(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  get_Db_اشارہ_گر(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  إذا( !pBackup ){
    Tcl_Append_نتیجہ(interp, "sqlite3_backup_init() failed", عدم);
    أرجع TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backup_Test_انت);
  Tcl_سیٹ_Obj_نتیجہ(interp, objv[1]);
  أرجع TCL_OK;
}

صحيح Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  أرجع TCL_OK;
}
