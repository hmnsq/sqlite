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
خارجي صحيح get_Db_اشارو(Tcl_Interp *, ثابت حرف *, sqlite3 **);

ساكن صحيح SQLITE_TCLAPI backupTestCmd(
  Client_معلومات client_معلومات, 
  Tcl_Interp *interp, 
  صحيح objc,
  Tcl_Obj *ثابت*objv
){
  تعداد BackupSubCommandEnum {
    BACKUP_قدم, BACKUP_پڄاڻي, BACKUP_بچيل, BACKUP_PAGECOUNT
  };
  بنية BackupSubCommand {
    ثابت حرف *zCmd;
    تعداد BackupSubCommandEnum eCmd;
    صحيح nArg;
    ثابت حرف *zArg;
  } aSub[] = {
    {"step",      BACKUP_قدم      , 1, "npage" },
    {"finish",    BACKUP_پڄاڻي    , 0, ""      },
    {"remaining", BACKUP_بچيل , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)client_معلومات;
  صحيح iCmd;
  صحيح rc;

  rc = Tcl_Get_اشاريو_From_Obj_Struct(
      interp, objv[1], aSub, حجم(aSub[0]), "option", 0, &iCmd
  );
  إذا( rc!=TCL_OK ){
    أرجع rc;
  }
  إذا( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_انگ_دليلون(interp, 2, objv, aSub[iCmd].zArg);
    أرجع TCL_ERROR;
  }

  بدل( aSub[iCmd].eCmd ){

    حالة BACKUP_پڄاڻي: {
      ثابت حرف *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_سيٽ_Command_Info(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_پڄاڻي(p);
      Tcl_سيٽ_نتيجو(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
      اكسر;
    }

    حالة BACKUP_قدم: {
      صحيح n_صفحو;
      إذا( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &n_صفحو) ){
        أرجع TCL_ERROR;
      }
      rc = sqlite3_backup_قدم(p, n_صفحو);
      Tcl_سيٽ_نتيجو(interp, (حرف *)sqlite3ErrName(rc), TCL_STATIC);
      اكسر;
    }

    حالة BACKUP_بچيل:
      Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewIntObj(sqlite3_backup_بچيل(p)));
      اكسر;

    حالة BACKUP_PAGECOUNT:
      Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      اكسر;
  }

  أرجع TCL_OK;
}

ساكن فراغ SQLITE_TCLAPI backup_Test_پڄاڻي(Client_معلومات client_معلومات){
  sqlite3_backup *pBackup = (sqlite3_backup *)client_معلومات;
  sqlite3_backup_پڄاڻي(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
ساكن صحيح SQLITE_TCLAPI backupTestInit(
  Client_معلومات client_معلومات, 
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
    Tcl_Wrong_انگ_دليلون(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    أرجع TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  get_Db_اشارو(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  get_Db_اشارو(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  إذا( !pBackup ){
    Tcl_Append_نتيجو(interp, "sqlite3_backup_init() failed", عدم);
    أرجع TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backup_Test_پڄاڻي);
  Tcl_سيٽ_Obj_نتيجو(interp, objv[1]);
  أرجع TCL_OK;
}

صحيح Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  أرجع TCL_OK;
}
