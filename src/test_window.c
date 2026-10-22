/*
** 2018 June 17
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
*/

#समावेश "sqlite3.h"

#अगर_घोषित SQLITE_TEST

#समावेश "sqliteInt.h"
#समावेश "tclsqlite.h"

बाह्य पूर्णांक getDbPointer(Tcl_Interp *interp, स्थिर अक्षर *zA, sqlite3 **ppDb);
बाह्य स्थिर अक्षर *sqlite3ErrName(पूर्णांक);

प्रकार काष्ठा TestWindow TestWindow;
काष्ठा TestWindow {
  Tcl_Obj *xStep;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_मूल्य;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

प्रकार काष्ठा TestWindowCtx TestWindowCtx;
काष्ठा TestWindowCtx {
  Tcl_Obj *pVal;
};

अटल व्योम doTestWindowStep(
  पूर्णांक bInverse,
  sqlite3_context *ctx, 
  पूर्णांक nArg, 
  sqlite3_मूल्य **apArg
){
  पूर्णांक i;
  TestWindow *p = (TestWindow*)sqlite3_user_तथ्य(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->xStep);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, माप(TestWindowCtx));

  Tcl_Incr_Ref_गिनती(pEval);
  अगर( pCtx ){
    स्थिर अक्षर *z_परिणाम;
    पूर्णांक rc;
    अगर( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }अन्यथा{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    क्रम(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((स्थिर अक्षर*)sqlite3_मूल्य_text(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    अगर( rc!=TCL_OK ){
      z_परिणाम = Tcl_Get_String_परिणाम(p->interp);
      sqlite3_परिणाम_error(ctx, z_परिणाम, -1);
    }अन्यथा{
      अगर( pCtx->pVal ) Tcl_Decr_Ref_गिनती(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_Get_Obj_परिणाम(p->interp));
      Tcl_Incr_Ref_गिनती(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_गिनती(pEval);
}

अटल व्योम doTestWindowFinalize(पूर्णांक b_मूल्य, sqlite3_context *ctx){
  TestWindow *p = (TestWindow*)sqlite3_user_तथ्य(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_मूल्य ? p->x_मूल्य : p->xFinal);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, माप(TestWindowCtx));

  Tcl_Incr_Ref_गिनती(pEval);
  अगर( pCtx ){
    स्थिर अक्षर *z_परिणाम;
    पूर्णांक rc;
    अगर( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }अन्यथा{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    z_परिणाम = Tcl_Get_String_परिणाम(p->interp);
    अगर( rc!=TCL_OK ){
      sqlite3_परिणाम_error(ctx, z_परिणाम, -1);
    }अन्यथा{
      sqlite3_परिणाम_text(ctx, z_परिणाम, -1, SQLITE_TRANSIENT);
    }

    अगर( b_मूल्य==0 ){
      अगर( pCtx->pVal ) Tcl_Decr_Ref_गिनती(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_गिनती(pEval);
}

अटल व्योम testWindowStep(
  sqlite3_context *ctx, 
  पूर्णांक nArg, 
  sqlite3_मूल्य **apArg
){
  doTestWindowStep(0, ctx, nArg, apArg);
}
अटल व्योम testWindowInverse(
  sqlite3_context *ctx, 
  पूर्णांक nArg, 
  sqlite3_मूल्य **apArg
){
  doTestWindowStep(1, ctx, nArg, apArg);
}

अटल व्योम testWindowFinal(sqlite3_context *ctx){
  doTestWindowFinalize(0, ctx);
}
अटल व्योम test_Window_मूल्य(sqlite3_context *ctx){
  doTestWindowFinalize(1, ctx);
}

अटल व्योम testWindowDestroy(व्योम *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
अटल पूर्णांक SQLITE_TCLAPI test_create_window(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  TestWindow *pNew;
  sqlite3 *db;
  स्थिर अक्षर *zName;
  पूर्णांक rc;

  अगर( objc!=7 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    वापस TCL_ERROR;
  }

  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (TestWindow*)ckalloc(माप(TestWindow));
  स_रखो(pNew, 0, माप(TestWindow));
  pNew->xStep = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_मूल्य = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_गिनती(pNew->xStep);
  Tcl_Incr_Ref_गिनती(pNew->xFinal);
  Tcl_Incr_Ref_गिनती(pNew->x_मूल्य);
  Tcl_Incr_Ref_गिनती(pNew->xInverse);

  rc = sqlite3_create_window_function(db, zName, -1, SQLITE_UTF8, (व्योम*)pNew,
      testWindowStep, testWindowFinal, test_Window_मूल्य, testWindowInverse,
      testWindowDestroy
  );
  अगर( rc!=SQLITE_OK ){
    Tcl_Set_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    वापस TCL_ERROR;
  }

  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_create_window_misuse(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  पूर्णांक rc;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;

  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, testWindowFinal, test_Window_मूल्य, testWindowInverse,
      0
  );
  अगर( rc!=SQLITE_MISUSE ) जाओ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, 0, test_Window_मूल्य, testWindowInverse,
      0
  );
  अगर( rc!=SQLITE_MISUSE ) जाओ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, 0, testWindowInverse,
      0
  );
  अगर( rc!=SQLITE_MISUSE ) जाओ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, test_Window_मूल्य, 0,
      0
  );
  अगर( rc!=SQLITE_MISUSE ) जाओ error;

  वापस TCL_OK;

 error:
  Tcl_Set_Obj_परिणाम(interp, Tcl_NewStringObj("misuse test error", -1));
  वापस TCL_ERROR;
}

/*
** xStep for sumint().
*/
अटल व्योम sumintStep(
  sqlite3_context *ctx, 
  पूर्णांक nArg, 
  sqlite3_मूल्य *apArg[]
){
  sqlite3_int64 *pInt;

  निश्चित( nArg==1 );
  अगर( sqlite3_मूल्य_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_परिणाम_error(ctx, "invalid argument", -1);
    वापस;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, माप(sqlite3_int64));
  अगर( pInt ){
    *pInt += sqlite3_मूल्य_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
अटल व्योम sumintInverse(
  sqlite3_context *ctx, 
  पूर्णांक nArg, 
  sqlite3_मूल्य *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, माप(sqlite3_int64));
  *pInt -= sqlite3_मूल्य_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
अटल व्योम sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  अगर( pInt ) res = *pInt;
  sqlite3_परिणाम_int64(ctx, res);
}

/*
** xValue for sumint().
*/
अटल व्योम sumint_मूल्य(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  अगर( pInt ) res = *pInt;
  sqlite3_परिणाम_int64(ctx, res);
}

अटल पूर्णांक SQLITE_TCLAPI test_create_sumint(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  पूर्णांक rc;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;

  rc = sqlite3_create_window_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumintStep, sumintFinal, sumint_मूल्य, sumintInverse,
      0
  );

  अगर( rc!=SQLITE_OK ){
    Tcl_Set_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}

अटल पूर्णांक SQLITE_TCLAPI test_override_योग(
  व्योम * client_तथ्य,
  Tcl_Interp *interp,
  पूर्णांक objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  पूर्णांक rc;

  अगर( objc!=2 ){
    Tcl_Wrong_अंक_Args(interp, 1, objv, "DB");
    वापस TCL_ERROR;
  }
  अगर( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) वापस TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumintStep, sumintFinal
  );

  अगर( rc!=SQLITE_OK ){
    Tcl_Set_Obj_परिणाम(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    वापस TCL_ERROR;
  }
  वापस TCL_OK;
}

पूर्णांक Sqlitetest_window_Init(Tcl_Interp *interp){
  अटल काष्ठा {
     अक्षर *zName;
     Tcl_ObjCmdProc *xProc;
     पूर्णांक client_तथ्य;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_window, 0 },
     { "test_create_window_function_misuse", test_create_window_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_योग, 0 },
  };
  पूर्णांक i;
  क्रम(i=0; i<माप(aObjCmd)/माप(aObjCmd[0]); i++){
    Client_तथ्य c = (Client_तथ्य)SQLITE_INT_TO_PTR(aObjCmd[i].client_तथ्य);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  वापस TCL_OK;
}
#पूर्ण_अगर
