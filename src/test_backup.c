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

#ಅನ್ತರ್ಭಾವ "tclsqlite.h"
#ಅನ್ತರ್ಭಾವ "sqlite3.h"
#ಅನ್ತರ್ಭಾವ <assert.h>

/* These functions are implemented in main.c. */
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3ErrName(ಪೂರ್ಣಾಂಕ);

/* These functions are implemented in test1.c. */
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ get_Db_ಸೂಚಕ(Tcl_Interp *, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *, sqlite3 **);

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI backupTestCmd(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, 
  Tcl_Interp *interp, 
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *ಸ್ಥಿರಾಂಕ*objv
){
  ಎಣಿಕೆ BackupSubCommandEnum {
    BACKUP_ಹೆಜ್ಜೆ, BACKUP_ಮುಕ್ತಾಯ, BACKUP_ಉಳಿದ, BACKUP_PAGECOUNT
  };
  ರಚನೆ BackupSubCommand {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCmd;
    ಎಣಿಕೆ BackupSubCommandEnum eCmd;
    ಪೂರ್ಣಾಂಕ nArg;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zArg;
  } aSub[] = {
    {"step",      BACKUP_ಹೆಜ್ಜೆ      , 1, "npage" },
    {"finish",    BACKUP_ಮುಕ್ತಾಯ    , 0, ""      },
    {"remaining", BACKUP_ಉಳಿದ , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)client_ದತ್ತಾಂಶ;
  ಪೂರ್ಣಾಂಕ iCmd;
  ಪೂರ್ಣಾಂಕ rc;

  rc = Tcl_Get_ಸೂಚ್ಯಂಕ_From_Obj_Struct(
      interp, objv[1], aSub, ಗಾತ್ರ(aSub[0]), "option", 0, &iCmd
  );
  ಆದರೆ( rc!=TCL_OK ){
    ಹಿಂತಿರುಗಿ rc;
  }
  ಆದರೆ( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(interp, 2, objv, aSub[iCmd].zArg);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  ಆಯ್ಕೆ( aSub[iCmd].eCmd ){

    ಸ್ಥಿತಿ BACKUP_ಮುಕ್ತಾಯ: {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_ಗಣ_Command_Info(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_ಮುಕ್ತಾಯ(p);
      Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_STATIC);
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ BACKUP_ಹೆಜ್ಜೆ: {
      ಪೂರ್ಣಾಂಕ n_ಪುಟ;
      ಆದರೆ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &n_ಪುಟ) ){
        ಹಿಂತಿರುಗಿ TCL_ERROR;
      }
      rc = sqlite3_backup_ಹೆಜ್ಜೆ(p, n_ಪುಟ);
      Tcl_ಗಣ_ಫಲಿತಾಂಶ(interp, (ಅಕ್ಷರ *)sqlite3ErrName(rc), TCL_STATIC);
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ BACKUP_ಉಳಿದ:
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(sqlite3_backup_ಉಳಿದ(p)));
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ BACKUP_PAGECOUNT:
      Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      ನಿಲ್ಲಿಸು;
  }

  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಸ್ಥಿರ ಶೂನ್ಯ SQLITE_TCLAPI backup_Test_ಮುಕ್ತಾಯ(Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ){
  sqlite3_backup *pBackup = (sqlite3_backup *)client_ದತ್ತಾಂಶ;
  sqlite3_backup_ಮುಕ್ತಾಯ(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ SQLITE_TCLAPI backupTestInit(
  Client_ದತ್ತಾಂಶ client_ದತ್ತಾಂಶ, 
  Tcl_Interp *interp, 
  ಪೂರ್ಣಾಂಕ objc,
  Tcl_Obj *ಸ್ಥಿರಾಂಕ*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDestName;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSrcName;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zCmd;

  ಆದರೆ( objc!=6 ){
    Tcl_Wrong_ಅಂಕೆ_ವಾದಗಳು(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  get_Db_ಸೂಚಕ(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  ಆದರೆ( !pBackup ){
    Tcl_Append_ಫಲಿತಾಂಶ(interp, "sqlite3_backup_init() failed", ಖಾಲಿ);
    ಹಿಂತಿರುಗಿ TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backup_Test_ಮುಕ್ತಾಯ);
  Tcl_ಗಣ_Obj_ಫಲಿತಾಂಶ(interp, objv[1]);
  ಹಿಂತಿರುಗಿ TCL_OK;
}

ಪೂರ್ಣಾಂಕ Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  ಹಿಂತಿರುಗಿ TCL_OK;
}
