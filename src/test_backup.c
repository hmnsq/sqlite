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

#অন্তর্ভাৱ "tclsqlite.h"
#অন্তর্ভাৱ "sqlite3.h"
#অন্তর্ভাৱ <assert.h>

/* These functions are implemented in main.c. */
বাহ্য ধ্রুৱ ৱর্ণ *sqlite3ErrName(পূর্ণাঙ্ক);

/* These functions are implemented in test1.c. */
বাহ্য পূর্ণাঙ্ক getDbPointer(Tcl_Interp *, ধ্রুৱ ৱর্ণ *, sqlite3 **);

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI backupTestCmd(
  ClientData clientData, 
  Tcl_Interp *interp, 
  পূর্ণাঙ্ক objc,
  Tcl_Obj *ধ্রুৱ*objv
){
  গণনা BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  সংরচনা BackupSubCommand {
    ধ্রুৱ ৱর্ণ *zCmd;
    গণনা BackupSubCommandEnum eCmd;
    পূর্ণাঙ্ক nArg;
    ধ্রুৱ ৱর্ণ *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)clientData;
  পূর্ণাঙ্ক iCmd;
  পূর্ণাঙ্ক rc;

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, পরিমাণ(aSub[0]), "option", 0, &iCmd
  );
  যদি( rc!=TCL_OK ){
    ঘূৰাই rc;
  }
  যদি( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_অঙ্ক_Args(interp, 2, objv, aSub[iCmd].zArg);
    ঘূৰাই TCL_ERROR;
  }

  ৱিকল্প( aSub[iCmd].eCmd ){

    পক্ষ BACKUP_FINISH: {
      ধ্রুৱ ৱর্ণ *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_SetResult(interp, (ৱর্ণ *)sqlite3ErrName(rc), TCL_STATIC);
      ভঙ্গ;
    }

    পক্ষ BACKUP_STEP: {
      পূর্ণাঙ্ক nPage;
      যদি( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        ঘূৰাই TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_SetResult(interp, (ৱর্ণ *)sqlite3ErrName(rc), TCL_STATIC);
      ভঙ্গ;
    }

    পক্ষ BACKUP_REMAINING:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      ভঙ্গ;

    পক্ষ BACKUP_PAGECOUNT:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      ভঙ্গ;
  }

  ঘূৰাই TCL_OK;
}

স্থির শূন্য SQLITE_TCLAPI backupTestFinish(ClientData clientData){
  sqlite3_backup *pBackup = (sqlite3_backup *)clientData;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI backupTestInit(
  ClientData clientData, 
  Tcl_Interp *interp, 
  পূর্ণাঙ্ক objc,
  Tcl_Obj *ধ্রুৱ*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  ধ্রুৱ ৱর্ণ *zDestName;
  ধ্রুৱ ৱর্ণ *zSrcName;
  ধ্রুৱ ৱর্ণ *zCmd;

  যদি( objc!=6 ){
    Tcl_Wrong_অঙ্ক_Args(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    ঘূৰাই TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  যদি( !pBackup ){
    Tcl_AppendResult(interp, "sqlite3_backup_init() failed", রিক্ত);
    ঘূৰাই TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_SetObjResult(interp, objv[1]);
  ঘূৰাই TCL_OK;
}

পূর্ণাঙ্ক Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  ঘূৰাই TCL_OK;
}
