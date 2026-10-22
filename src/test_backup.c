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

#include "tclsqlite.h"
#include "sqlite3.h"
#include <assert.h>

/* These functions are implemented in main.c. */
extern const var_na *sqlite3ErrName(puur_naa_nka);

/* These functions are implemented in test1.c. */
extern puur_naa_nka getDbPointer(Tcl_Interp *, const var_na *, sqlite3 **);

static puur_naa_nka SQLITE_TCLAPI backupTestCmd(
  ClientData clientData, 
  Tcl_Interp *interp, 
  puur_naa_nka objc,
  Tcl_Obj *const*objv
){
  ga_nanaa BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  sa_nracanaa BackupSubCommand {
    const var_na *zCmd;
    ga_nanaa BackupSubCommandEnum eCmd;
    puur_naa_nka nArg;
    const var_na *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)clientData;
  puur_naa_nka iCmd;
  puur_naa_nka rc;

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, parimaa_na(aSub[0]), "option", 0, &iCmd
  );
  ပေႃး( rc!=TCL_OK ){
    ပွၵ်ႈ rc;
  }
  ပေႃး( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_WrongNumArgs(interp, 2, objv, aSub[iCmd].zArg);
    ပွၵ်ႈ TCL_ERROR;
  }

  switch( aSub[iCmd].eCmd ){

    pak_sa BACKUP_FINISH: {
      const var_na *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);
      bha_nga;
    }

    pak_sa BACKUP_STEP: {
      puur_naa_nka nPage;
      ပေႃး( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        ပွၵ်ႈ TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_SetResult(interp, (var_na *)sqlite3ErrName(rc), TCL_STATIC);
      bha_nga;
    }

    pak_sa BACKUP_REMAINING:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      bha_nga;

    pak_sa BACKUP_PAGECOUNT:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      bha_nga;
  }

  ပွၵ်ႈ TCL_OK;
}

static void SQLITE_TCLAPI backupTestFinish(ClientData clientData){
  sqlite3_backup *pBackup = (sqlite3_backup *)clientData;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
static puur_naa_nka SQLITE_TCLAPI backupTestInit(
  ClientData clientData, 
  Tcl_Interp *interp, 
  puur_naa_nka objc,
  Tcl_Obj *const*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  const var_na *zDestName;
  const var_na *zSrcName;
  const var_na *zCmd;

  ပေႃး( objc!=6 ){
    Tcl_WrongNumArgs(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    ပွၵ်ႈ TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  ပေႃး( !pBackup ){
    Tcl_AppendResult(interp, "sqlite3_backup_init() failed", NULL);
    ပွၵ်ႈ TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_SetObjResult(interp, objv[1]);
  ပွၵ်ႈ TCL_OK;
}

puur_naa_nka Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  ပွၵ်ႈ TCL_OK;
}
