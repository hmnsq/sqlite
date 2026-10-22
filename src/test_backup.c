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

#समावेश "tclsqlite.h"
#समावेश "sqlite3.h"
#समावेश <assert.h>

/* These functions are implemented in main.c. */
बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);

/* These functions are implemented in test1.c. */
बाह्य पूर्णांक getDbPointer(Tcl_Interp *, स्थिर अक्षर *, sqlite3 **);

अटल पूर्णांक SQLITE_TCLAPI backupTestCmd(
  Client_तथ्य client_तथ्य, 
  Tcl_Interp *interp, 
  पूर्णांक objc,
  Tcl_Obj *स्थिर*objv
){
  क्रमागत BackupSubCommandEnum {
    BACKUP_STEP, BACKUP_FINISH, BACKUP_REMAINING, BACKUP_PAGECOUNT
  };
  काष्ठा BackupSubCommand {
    स्थिर अक्षर *zCmd;
    क्रमागत BackupSubCommandEnum eCmd;
    पूर्णांक nArg;
    स्थिर अक्षर *zArg;
  } aSub[] = {
    {"step",      BACKUP_STEP      , 1, "npage" },
    {"finish",    BACKUP_FINISH    , 0, ""      },
    {"remaining", BACKUP_REMAINING , 0, ""      },
    {"pagecount", BACKUP_PAGECOUNT , 0, ""      },
    {0, 0, 0, 0}
  };

  sqlite3_backup *p = (sqlite3_backup *)client_तथ्य;
  पूर्णांक iCmd;
  पूर्णांक rc;

  rc = Tcl_Get_सूचकांक_From_Obj_Struct(
      interp, objv[1], aSub, माप(aSub[0]), "option", 0, &iCmd
  );
  अगर( rc!=TCL_OK ){
    वापस rc;
  }
  अगर( objc!=(2 + aSub[iCmd].nArg) ){
    Tcl_Wrong_अंक_Args(interp, 2, objv, aSub[iCmd].zArg);
    वापस TCL_ERROR;
  }

  चयन( aSub[iCmd].eCmd ){

    हाल BACKUP_FINISH: {
      स्थिर अक्षर *zCmdName;
      Tcl_CmdInfo cmdInfo;
      zCmdName = Tcl_GetString(objv[0]);
      Tcl_GetCommandInfo(interp, zCmdName, &cmdInfo);
      cmdInfo.deleteProc = 0;
      Tcl_SetCommandInfo(interp, zCmdName, &cmdInfo);
      Tcl_DeleteCommand(interp, zCmdName);

      rc = sqlite3_backup_finish(p);
      Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);
      अवरोध;
    }

    हाल BACKUP_STEP: {
      पूर्णांक nPage;
      अगर( TCL_OK!=Tcl_GetIntFromObj(interp, objv[2], &nPage) ){
        वापस TCL_ERROR;
      }
      rc = sqlite3_backup_step(p, nPage);
      Tcl_Set_परिणाम(interp, (अक्षर *)sqlite3ErrName(rc), TCL_STATIC);
      अवरोध;
    }

    हाल BACKUP_REMAINING:
      Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(sqlite3_backup_remaining(p)));
      अवरोध;

    हाल BACKUP_PAGECOUNT:
      Tcl_Set_Obj_परिणाम(interp, Tcl_NewIntObj(sqlite3_backup_pagecount(p)));
      अवरोध;
  }

  वापस TCL_OK;
}

अटल व्योम SQLITE_TCLAPI backupTestFinish(Client_तथ्य client_तथ्य){
  sqlite3_backup *pBackup = (sqlite3_backup *)client_तथ्य;
  sqlite3_backup_finish(pBackup);
}

/*
**     sqlite3_backup CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME
**
*/
अटल पूर्णांक SQLITE_TCLAPI backupTestInit(
  Client_तथ्य client_तथ्य, 
  Tcl_Interp *interp, 
  पूर्णांक objc,
  Tcl_Obj *स्थिर*objv
){
  sqlite3_backup *pBackup;
  sqlite3 *pDestDb;
  sqlite3 *pSrcDb;
  स्थिर अक्षर *zDestName;
  स्थिर अक्षर *zSrcName;
  स्थिर अक्षर *zCmd;

  अगर( objc!=6 ){
    Tcl_Wrong_अंक_Args(
      interp, 1, objv, "CMDNAME DESTHANDLE DESTNAME SRCHANDLE SRCNAME"
    );
    वापस TCL_ERROR;
  }

  zCmd = Tcl_GetString(objv[1]);
  getDbPointer(interp, Tcl_GetString(objv[2]), &pDestDb);
  zDestName = Tcl_GetString(objv[3]);
  getDbPointer(interp, Tcl_GetString(objv[4]), &pSrcDb);
  zSrcName = Tcl_GetString(objv[5]);

  pBackup = sqlite3_backup_init(pDestDb, zDestName, pSrcDb, zSrcName);
  अगर( !pBackup ){
    Tcl_Append_परिणाम(interp, "sqlite3_backup_init() failed", शून्य);
    वापस TCL_ERROR;
  }

  Tcl_CreateObjCommand(interp, zCmd, backupTestCmd, pBackup, backupTestFinish);
  Tcl_Set_Obj_परिणाम(interp, objv[1]);
  वापस TCL_OK;
}

पूर्णांक Sqlitetestbackup_Init(Tcl_Interp *interp){
  Tcl_CreateObjCommand(interp, "sqlite3_backup", backupTestInit, 0, 0);
  वापस TCL_OK;
}
