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

#అన్తర్భావ "tclsqlite.h"
#అన్తర్భావ "sqlite3.h"
#అన్తర్భావ <assert.h>

/* These functions are implemented in main.c. */
బాహ్య స్థిరరాశి అక్షరం *sqlite3ErrName(పూర్ణాంకం);

/* These functions are implemented in test1.c. */
బాహ్య పూర్ణాంకం get_Db_సూచిక(Tcl_Interp *, స్థిరరాశి అక్షరం *, sqlite3 **);

స్థిర పూర్ణాంకం SQLITE_TCLAPI backupTestCmd(
  Client_దత్తాంశం client_దత్తాంశం, 
  Tcl_Interp *interp, 
  పూర్ణాంకం objc,
  Tcl_Obj *స్థిరరాశి*objv
){
  గణన BackupSubCommandEnum {
    BACKUP_అడుగు, BACKUP_ముగింపు, BACKUP_మిగిలిన, BACKUP_PAGECOUNT
  };
  నిర్మాణం BackupSubCommand {
    స్థిరరాశి అక్షరం *zCmd;
    గణన BackupSubCommandEnum eCmd;
    పూర్ణాంకం nArg;
    స్థిరరాశి అక్షరం *zArg;
  } aSub[] = {
    {"step",      BACKUP_అడుగు      , 1, "npage" },
    {"finish",    BACKUP_ముగింపు    , 0, ""      },
    {"remaining", BACKUP_మిగిలిన , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)client_దత్తాంశం;
  పూర్ణాంకం iCmd;
  పూర్ణాంకం rc;

  rc = Tcl_Get_సూచీ_From_Obj_Struct(
      interp, objv[1], aSub, పరిమాణం(aSub[0]), "option", 0, &iCmd
  );
  ఒకవేళ( rc!=TCL_OK ){
    తిరిగి rc;
  }
  ఒకవేళ( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_అంకె_వాదనలు(interp, 2, objv, aSub[iCmd].zArg);
    తిరిగి TCL_ERROR;
  }

  ఎంపిక( aSub[iCmd].eCmd ){

    స్థితి BACKUP_ముగింపు: {
      స్థిరరాశి అక్షరం *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_సమితి_Command_Info(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_ముగింపు(p);
      Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_STATIC);
      ఆపు;
    }

    స్థితి BACKUP_అడుగు: {
      పూర్ణాంకం n_పుట;
      ఒకవేళ( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &n_పుట) ){
        తిరిగి TCL_ERROR;
      }
      rc = sqlite3_backup_అడుగు(p, n_పుట);
      Tcl_సమితి_ఫలితం(interp, (అక్షరం *)sqlite3ErrName(rc), TCL_STATIC);
      ఆపు;
    }

    స్థితి BACKUP_మిగిలిన:
      Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(sqlite3_backup_మిగిలిన(p)));
      ఆపు;

    స్థితి BACKUP_PAGECOUNT:
      Tcl_సమితి_Obj_ఫలితం(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      ఆపు;
  }

  తిరిగి TCL_OK;
}

స్థిర శూన్య SQLITE_TCLAPI backup_Test_ముగింపు(Client_దత్తాంశం client_దత్తాంశం){
  sqlite3_backup *pBackup = (sqlite3_backup *)client_దత్తాంశం;
  sqlite3_backup_ముగింపు(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
స్థిర పూర్ణాంకం SQLITE_TCLAPI backupTestInit(
  Client_దత్తాంశం client_దత్తాంశం, 
  Tcl_Interp *interp, 
  పూర్ణాంకం objc,
  Tcl_Obj *స్థిరరాశి*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  స్థిరరాశి అక్షరం *zDestName;
  స్థిరరాశి అక్షరం *zSrcName;
  స్థిరరాశి అక్షరం *zCmd;

  ఒకవేళ( objc!=6 ){
    Tcl_Wrong_అంకె_వాదనలు(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    తిరిగి TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  get_Db_సూచిక(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  get_Db_సూచిక(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  ఒకవేళ( !pBackup ){
    Tcl_Append_ఫలితం(interp, "sqlite3_backup_init() failed", ఖాళీ);
    తిరిగి TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backup_Test_ముగింపు);
  Tcl_సమితి_Obj_ఫలితం(interp, objv[1]);
  తిరిగి TCL_OK;
}

పూర్ణాంకం Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  తిరిగి TCL_OK;
}
