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

#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>

/* These functions are implemented in main.c. */
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);

/* These functions are implemented in test1.c. */
ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp *, ਅਚਲ ਅੱਖਰ *, sqlite3 **);

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI backupTestCmd(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, 
  Tcl_Interp *interp, 
  ਪੂਰਨ objc,
  Tcl_Obj *ਅਚਲ*objv
){
  ਗਿਣਤੀ BackupSubCommandEnum {
    BACKUP_ਕਦਮ, BACKUP_ਅੰਤ, BACKUP_ਬਚਿਆ, BACKUP_PAGECOUNT
  };
  ਬਣਤਰ BackupSubCommand {
    ਅਚਲ ਅੱਖਰ *zCmd;
    ਗਿਣਤੀ BackupSubCommandEnum eCmd;
    ਪੂਰਨ nArg;
    ਅਚਲ ਅੱਖਰ *zArg;
  } aSub[] = {
    {"step",      BACKUP_ਕਦਮ      , 1, "npage" },
    {"finish",    BACKUP_ਅੰਤ    , 0, ""      },
    {"remaining", BACKUP_ਬਚਿਆ , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)client_ਅੰਕੜੇ;
  ਪੂਰਨ iCmd;
  ਪੂਰਨ rc;

  rc = Tcl_Get_ਸੂਚਕਾਂਕ_From_Obj_Struct(
      interp, objv[1], aSub, ਆਕਾਰ(aSub[0]), "option", 0, &iCmd
  );
  ਜੇ( rc!=TCL_OK ){
    ਵਾਪਸ rc;
  }
  ਜੇ( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 2, objv, aSub[iCmd].zArg);
    ਵਾਪਸ TCL_ERROR;
  }

  ਚੋਣ( aSub[iCmd].eCmd ){

    ਹਾਲਤ BACKUP_ਅੰਤ: {
      ਅਚਲ ਅੱਖਰ *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_ਸੈੱਟ_Command_Info(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_ਅੰਤ(p);
      Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_STATIC);
      ਰੋਕੋ;
    }

    ਹਾਲਤ BACKUP_ਕਦਮ: {
      ਪੂਰਨ n_ਪੰਨਾ;
      ਜੇ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &n_ਪੰਨਾ) ){
        ਵਾਪਸ TCL_ERROR;
      }
      rc = sqlite3_backup_ਕਦਮ(p, n_ਪੰਨਾ);
      Tcl_ਸੈੱਟ_ਨਤੀਜਾ(interp, (ਅੱਖਰ *)sqlite3ErrName(rc), TCL_STATIC);
      ਰੋਕੋ;
    }

    ਹਾਲਤ BACKUP_ਬਚਿਆ:
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(sqlite3_backup_ਬਚਿਆ(p)));
      ਰੋਕੋ;

    ਹਾਲਤ BACKUP_PAGECOUNT:
      Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      ਰੋਕੋ;
  }

  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਖਾਲੀ SQLITE_TCLAPI backup_Test_ਅੰਤ(Client_ਅੰਕੜੇ client_ਅੰਕੜੇ){
  sqlite3_backup *pBackup = (sqlite3_backup *)client_ਅੰਕੜੇ;
  sqlite3_backup_ਅੰਤ(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI backupTestInit(
  Client_ਅੰਕੜੇ client_ਅੰਕੜੇ, 
  Tcl_Interp *interp, 
  ਪੂਰਨ objc,
  Tcl_Obj *ਅਚਲ*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  ਅਚਲ ਅੱਖਰ *zDestName;
  ਅਚਲ ਅੱਖਰ *zSrcName;
  ਅਚਲ ਅੱਖਰ *zCmd;

  ਜੇ( objc!=6 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    ਵਾਪਸ TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  ਜੇ( !pBackup ){
    Tcl_Append_ਨਤੀਜਾ(interp, "sqlite3_backup_init() failed", ਨਿਰਾ);
    ਵਾਪਸ TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backup_Test_ਅੰਤ);
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, objv[1]);
  ਵਾਪਸ TCL_OK;
}

ਪੂਰਨ Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  ਵਾਪਸ TCL_OK;
}
