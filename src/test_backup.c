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

#அந்தர்பாவ "tclsqlite.h"
#அந்தர்பாவ "sqlite3.h"
#அந்தர்பாவ <assert.h>

/* These functions are implemented in main.c. */
வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);

/* These functions are implemented in test1.c. */
வெளி முழு getDbPointer(Tcl_Interp *, மாறிலி எழுத்து *, sqlite3 **);

நிலையான முழு SQLITE_TCLAPI backupTestCmd(
  ClientData clientData, 
  Tcl_Interp *interp, 
  முழு objc,
  Tcl_Obj *மாறிலி*objv
){
  எண்ணிக்கை BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  கட்டமைப்பு BackupSubCommand {
    மாறிலி எழுத்து *zCmd;
    எண்ணிக்கை BackupSubCommandEnum eCmd;
    முழு nArg;
    மாறிலி எழுத்து *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)clientData;
  முழு iCmd;
  முழு rc;

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, அளவு(aSub[0]), "option", 0, &iCmd
  );
  எனில்( rc!=TCL_OK ){
    திரும்பு rc;
  }
  எனில்( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_இலக்கம்_Args(interp, 2, objv, aSub[iCmd].zArg);
    திரும்பு TCL_ERROR;
  }

  தேர்வு( aSub[iCmd].eCmd ){

    நிலை BACKUP_FINISH: {
      மாறிலி எழுத்து *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_STATIC);
      நிறுத்து;
    }

    நிலை BACKUP_STEP: {
      முழு nPage;
      எனில்( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        திரும்பு TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_SetResult(interp, (எழுத்து *)sqlite3ErrName(rc), TCL_STATIC);
      நிறுத்து;
    }

    நிலை BACKUP_REMAINING:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      நிறுத்து;

    நிலை BACKUP_PAGECOUNT:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      நிறுத்து;
  }

  திரும்பு TCL_OK;
}

நிலையான வெறுமை SQLITE_TCLAPI backupTestFinish(ClientData clientData){
  sqlite3_backup *pBackup = (sqlite3_backup *)clientData;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
நிலையான முழு SQLITE_TCLAPI backupTestInit(
  ClientData clientData, 
  Tcl_Interp *interp, 
  முழு objc,
  Tcl_Obj *மாறிலி*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  மாறிலி எழுத்து *zDestName;
  மாறிலி எழுத்து *zSrcName;
  மாறிலி எழுத்து *zCmd;

  எனில்( objc!=6 ){
    Tcl_Wrong_இலக்கம்_Args(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    திரும்பு TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  எனில்( !pBackup ){
    Tcl_AppendResult(interp, "sqlite3_backup_init() failed", வெற்று);
    திரும்பு TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_SetObjResult(interp, objv[1]);
  திரும்பு TCL_OK;
}

முழு Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  திரும்பு TCL_OK;
}
