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

#അന്തര്ഭാവ "tclsqlite.h"
#അന്തര്ഭാവ "sqlite3.h"
#അന്തര്ഭാവ <assert.h>

/* These functions are implemented in main.c. */
ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);

/* These functions are implemented in test1.c. */
ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp *, സ്ഥിരം അക്ഷരം *, sqlite3 **);

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI backupTestCmd(
  ClientData clientData, 
  Tcl_Interp *interp, 
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *സ്ഥിരം*objv
){
  എണ്ണൽ BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  ഘടന BackupSubCommand {
    സ്ഥിരം അക്ഷരം *zCmd;
    എണ്ണൽ BackupSubCommandEnum eCmd;
    പൂർണ്ണസംഖ്യ nArg;
    സ്ഥിരം അക്ഷരം *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)clientData;
  പൂർണ്ണസംഖ്യ iCmd;
  പൂർണ്ണസംഖ്യ rc;

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, വലുപ്പം(aSub[0]), "option", 0, &iCmd
  );
  എങ്കിൽ( rc!=TCL_OK ){
    തിരികെ rc;
  }
  എങ്കിൽ( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_അക്കം_Args(interp, 2, objv, aSub[iCmd].zArg);
    തിരികെ TCL_ERROR;
  }

  തിരഞ്ഞെടുപ്പ്( aSub[iCmd].eCmd ){

    അവസ്ഥ BACKUP_FINISH: {
      സ്ഥിരം അക്ഷരം *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_STATIC);
      നിർത്തുക;
    }

    അവസ്ഥ BACKUP_STEP: {
      പൂർണ്ണസംഖ്യ nPage;
      എങ്കിൽ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        തിരികെ TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_SetResult(interp, (അക്ഷരം *)sqlite3ErrName(rc), TCL_STATIC);
      നിർത്തുക;
    }

    അവസ്ഥ BACKUP_REMAINING:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      നിർത്തുക;

    അവസ്ഥ BACKUP_PAGECOUNT:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      നിർത്തുക;
  }

  തിരികെ TCL_OK;
}

സ്ഥിതം ശൂന്യം SQLITE_TCLAPI backupTestFinish(ClientData clientData){
  sqlite3_backup *pBackup = (sqlite3_backup *)clientData;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI backupTestInit(
  ClientData clientData, 
  Tcl_Interp *interp, 
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *സ്ഥിരം*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  സ്ഥിരം അക്ഷരം *zDestName;
  സ്ഥിരം അക്ഷരം *zSrcName;
  സ്ഥിരം അക്ഷരം *zCmd;

  എങ്കിൽ( objc!=6 ){
    Tcl_Wrong_അക്കം_Args(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    തിരികെ TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  എങ്കിൽ( !pBackup ){
    Tcl_AppendResult(interp, "sqlite3_backup_init() failed", ശൂന്യ);
    തിരികെ TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_SetObjResult(interp, objv[1]);
  തിരികെ TCL_OK;
}

പൂർണ്ണസംഖ്യ Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  തിരികെ TCL_OK;
}
