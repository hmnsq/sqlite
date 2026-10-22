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

#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"
#ଅନ୍ତର୍ଭାଵ "sqlite3.h"
#ଅନ୍ତର୍ଭାଵ <assert.h>

/* These functions are implemented in main.c. */
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/* These functions are implemented in test1.c. */
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp *, ଅଚଳ ଅକ୍ଷର *, sqlite3 **);

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI backupTestCmd(
  ClientData clientData, 
  Tcl_Interp *interp, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *ଅଚଳ*objv
){
  ଗଣନା BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  ସଂରଚନା BackupSubCommand {
    ଅଚଳ ଅକ୍ଷର *zCmd;
    ଗଣନା BackupSubCommandEnum eCmd;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg;
    ଅଚଳ ଅକ୍ଷର *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)clientData;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCmd;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  rc = Tcl_GetIndexFromObjStruct(
      interp, objv[1], aSub, ଆକାର(aSub[0]), "option", 0, &iCmd
  );
  ଯଦି( rc!=TCL_OK ){
    ଫେରନ୍ତୁ rc;
  }
  ଯଦି( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 2, objv, aSub[iCmd].zArg);
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ବାଛ( aSub[iCmd].eCmd ){

    ସ୍ଥିତି BACKUP_FINISH: {
      ଅଚଳ ଅକ୍ଷର *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_STATIC);
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି BACKUP_STEP: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nPage;
      ଯଦି( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        ଫେରନ୍ତୁ TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_SetResult(interp, (ଅକ୍ଷର *)sqlite3ErrName(rc), TCL_STATIC);
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି BACKUP_REMAINING:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      ଭାଙ୍ଗ;

    ସ୍ଥିତି BACKUP_PAGECOUNT:
      Tcl_SetObjResult(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      ଭାଙ୍ଗ;
  }

  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ଶୂନ୍ୟ SQLITE_TCLAPI backupTestFinish(ClientData clientData){
  sqlite3_backup *pBackup = (sqlite3_backup *)clientData;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI backupTestInit(
  ClientData clientData, 
  Tcl_Interp *interp, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *ଅଚଳ*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  ଅଚଳ ଅକ୍ଷର *zDestName;
  ଅଚଳ ଅକ୍ଷର *zSrcName;
  ଅଚଳ ଅକ୍ଷର *zCmd;

  ଯଦି( objc!=6 ){
    Tcl_Wrong_ଅଙ୍କ_Args(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  ଯଦି( !pBackup ){
    Tcl_AppendResult(interp, "sqlite3_backup_init() failed", ଖାଲି);
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_SetObjResult(interp, objv[1]);
  ଫେରନ୍ତୁ TCL_OK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  ଫେରନ୍ତୁ TCL_OK;
}
