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

#અન્તર્ભાવ "tclsqlite.h"
#અન્તર્ભાવ "sqlite3.h"
#અન્તર્ભાવ <assert.h>

/* These functions are implemented in main.c. */
બાહ્ય અચલ અક્ષર *sqlite3ErrName(પૂર્ણાંક);

/* These functions are implemented in test1.c. */
બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp *, અચલ અક્ષર *, sqlite3 **);

સ્થિર પૂર્ણાંક SQLITE_TCLAPI backupTestCmd(
  ClientData clientData, 
  Tcl_Interp *interp, 
  પૂર્ણાંક objc,
  Tcl_Obj *અચલ*objv
){
  ગણના BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  સંરચના BackupSubCommand {
    અચલ અક્ષર *zCmd;
    ગણના BackupSubCommandEnum eCmd;
    પૂર્ણાંક nArg;
    અચલ અક્ષર *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)clientData;
  પૂર્ણાંક iCmd;
  પૂર્ણાંક rc;

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, કદ(aSub[0]), "option", 0, &iCmd
  );
  જો( rc!=TCL_OK ){
    પાછા rc;
  }
  જો( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_અંક_Args(interp, 2, objv, aSub[iCmd].zArg);
    પાછા TCL_ERROR;
  }

  પસંદગી( aSub[iCmd].eCmd ){

    સ્થિતિ BACKUP_FINISH: {
      અચલ અક્ષર *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_STATIC);
      તોડો;
    }

    સ્થિતિ BACKUP_STEP: {
      પૂર્ણાંક nPage;
      જો( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        પાછા TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_SetResult(interp, (અક્ષર *)sqlite3ErrName(rc), TCL_STATIC);
      તોડો;
    }

    સ્થિતિ BACKUP_REMAINING:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      તોડો;

    સ્થિતિ BACKUP_PAGECOUNT:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      તોડો;
  }

  પાછા TCL_OK;
}

સ્થિર શૂન્ય SQLITE_TCLAPI backupTestFinish(ClientData clientData){
  sqlite3_backup *pBackup = (sqlite3_backup *)clientData;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI backupTestInit(
  ClientData clientData, 
  Tcl_Interp *interp, 
  પૂર્ણાંક objc,
  Tcl_Obj *અચલ*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  અચલ અક્ષર *zDestName;
  અચલ અક્ષર *zSrcName;
  અચલ અક્ષર *zCmd;

  જો( objc!=6 ){
    Tcl_Wrong_અંક_Args(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    પાછા TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  જો( !pBackup ){
    Tcl_AppendResult(interp, "sqlite3_backup_init() failed", ખાલી);
    પાછા TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_SetObjResult(interp, objv[1]);
  પાછા TCL_OK;
}

પૂર્ણાંક Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  પાછા TCL_OK;
}
