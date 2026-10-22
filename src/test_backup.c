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
extern const char *sqlite3ErrName(số_nguyên);

/* These functions are implemented in test1.c. */
extern số_nguyên getDbPointer(Tcl_Interp *, const char *, sqlite3 **);

static số_nguyên SQLITE_TCLAPI backupTestCmd(
  ClientData clientData, 
  Tcl_Interp *interp, 
  số_nguyên objc,
  Tcl_Obj *const*objv
){
  enum BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  struct BackupSubCommand {
    const char *zCmd;
    enum BackupSubCommandEnum eCmd;
    số_nguyên nArg;
    const char *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)clientData;
  số_nguyên iCmd;
  số_nguyên rc;

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, sizeof(aSub[0]), "option", 0, &iCmd
  );
  nếu( rc!=TCL_OK ){
    trả_về rc;
  }
  nếu( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_WrongNumArgs(interp, 2, objv, aSub[iCmd].zArg);
    trả_về TCL_ERROR;
  }

  switch( aSub[iCmd].eCmd ){

    case BACKUP_FINISH: {
      const char *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_STATIC);
      break;
    }

    case BACKUP_STEP: {
      số_nguyên nPage;
      nếu( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        trả_về TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_SetResult(interp, (char *)sqlite3ErrName(rc), TCL_STATIC);
      break;
    }

    case BACKUP_REMAINING:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      break;

    case BACKUP_PAGECOUNT:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      break;
  }

  trả_về TCL_OK;
}

static void SQLITE_TCLAPI backupTestFinish(ClientData clientData){
  sqlite3_backup *pBackup = (sqlite3_backup *)clientData;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
static số_nguyên SQLITE_TCLAPI backupTestInit(
  ClientData clientData, 
  Tcl_Interp *interp, 
  số_nguyên objc,
  Tcl_Obj *const*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  const char *zDestName;
  const char *zSrcName;
  const char *zCmd;

  nếu( objc!=6 ){
    Tcl_WrongNumArgs(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    trả_về TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  nếu( !pBackup ){
    Tcl_AppendResult(interp, "sqlite3_backup_init() failed", NULL);
    trả_về TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_SetObjResult(interp, objv[1]);
  trả_về TCL_OK;
}

số_nguyên Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  trả_về TCL_OK;
}
