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

#ಅನ್ತರ್ಭಾವ "sqlite3.h"

#ifdef SQLITE_TEST

#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ "tclsqlite.h"

ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ getDbPointer(Tcl_Interp *interp, ಧ್ರುವ ವರ್ಣ *zA, sqlite3 **ppDb);
ಬಾಹ್ಯ ಧ್ರುವ ವರ್ಣ *sqlite3ErrName(ಪೂರ್ಣಾಙ್ಕ);

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ TestWindow TestWindow;
ಸಂರಚನಾ TestWindow {
  Tcl_Obj *xStep;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_ಮೌಲ್ಯ;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ TestWindowCtx TestWindowCtx;
ಸಂರಚನಾ TestWindowCtx {
  Tcl_Obj *pVal;
};

ಸ್ಥಿರ ಶೂನ್ಯ doTestWindowStep(
  ಪೂರ್ಣಾಙ್ಕ bInverse,
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಙ್ಕ nArg, 
  sqlite3_ಮೌಲ್ಯ **apArg
){
  ಪೂರ್ಣಾಙ್ಕ i;
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->xStep);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, ಪರಿಮಾಣ(TestWindowCtx));

  Tcl_Incr_Ref_ಎಣಿಕೆ(pEval);
  ಯದಿ( pCtx ){
    ಧ್ರುವ ವರ್ಣ *zResult;
    ಪೂರ್ಣಾಙ್ಕ rc;
    ಯದಿ( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }ಅನ್ಯಥಾ{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    ಪ್ರತಿ(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((ಧ್ರುವ ವರ್ಣ*)sqlite3_ಮೌಲ್ಯ_text(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    ಯದಿ( rc!=TCL_OK ){
      zResult = Tcl_GetStringResult(p->interp);
      sqlite3_result_error(ctx, zResult, -1);
    }ಅನ್ಯಥಾ{
      ಯದಿ( pCtx->pVal ) Tcl_Decr_Ref_ಎಣಿಕೆ(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_GetObjResult(p->interp));
      Tcl_Incr_Ref_ಎಣಿಕೆ(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_ಎಣಿಕೆ(pEval);
}

ಸ್ಥಿರ ಶೂನ್ಯ doTestWindowFinalize(ಪೂರ್ಣಾಙ್ಕ b_ಮೌಲ್ಯ, sqlite3_context *ctx){
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_ಮೌಲ್ಯ ? p->x_ಮೌಲ್ಯ : p->xFinal);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, ಪರಿಮಾಣ(TestWindowCtx));

  Tcl_Incr_Ref_ಎಣಿಕೆ(pEval);
  ಯದಿ( pCtx ){
    ಧ್ರುವ ವರ್ಣ *zResult;
    ಪೂರ್ಣಾಙ್ಕ rc;
    ಯದಿ( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }ಅನ್ಯಥಾ{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    zResult = Tcl_GetStringResult(p->interp);
    ಯದಿ( rc!=TCL_OK ){
      sqlite3_result_error(ctx, zResult, -1);
    }ಅನ್ಯಥಾ{
      sqlite3_result_text(ctx, zResult, -1, SQLITE_TRANSIENT);
    }

    ಯದಿ( b_ಮೌಲ್ಯ==0 ){
      ಯದಿ( pCtx->pVal ) Tcl_Decr_Ref_ಎಣಿಕೆ(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_ಎಣಿಕೆ(pEval);
}

ಸ್ಥಿರ ಶೂನ್ಯ testWindowStep(
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಙ್ಕ nArg, 
  sqlite3_ಮೌಲ್ಯ **apArg
){
  doTestWindowStep(0, ctx, nArg, apArg);
}
ಸ್ಥಿರ ಶೂನ್ಯ testWindowInverse(
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಙ್ಕ nArg, 
  sqlite3_ಮೌಲ್ಯ **apArg
){
  doTestWindowStep(1, ctx, nArg, apArg);
}

ಸ್ಥಿರ ಶೂನ್ಯ testWindowFinal(sqlite3_context *ctx){
  doTestWindowFinalize(0, ctx);
}
ಸ್ಥಿರ ಶೂನ್ಯ test_Window_ಮೌಲ್ಯ(sqlite3_context *ctx){
  doTestWindowFinalize(1, ctx);
}

ಸ್ಥಿರ ಶೂನ್ಯ testWindowDestroy(ಶೂನ್ಯ *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_create_window(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  TestWindow *pNew;
  sqlite3 *db;
  ಧ್ರುವ ವರ್ಣ *zName;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=7 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (TestWindow*)ckalloc(ಪರಿಮಾಣ(TestWindow));
  memset(pNew, 0, ಪರಿಮಾಣ(TestWindow));
  pNew->xStep = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_ಮೌಲ್ಯ = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_ಎಣಿಕೆ(pNew->xStep);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pNew->xFinal);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pNew->x_ಮೌಲ್ಯ);
  Tcl_Incr_Ref_ಎಣಿಕೆ(pNew->xInverse);

  rc = sqlite3_create_window_function(db, zName, -1, SQLITE_UTF8, (ಶೂನ್ಯ*)pNew,
      testWindowStep, testWindowFinal, test_Window_ಮೌಲ್ಯ, testWindowInverse,
      testWindowDestroy
  );
  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }

  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_create_window_misuse(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;

  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, testWindowFinal, test_Window_ಮೌಲ್ಯ, testWindowInverse,
      0
  );
  ಯದಿ( rc!=SQLITE_MISUSE ) ಗಚ್ಛ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, 0, test_Window_ಮೌಲ್ಯ, testWindowInverse,
      0
  );
  ಯದಿ( rc!=SQLITE_MISUSE ) ಗಚ್ಛ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, 0, testWindowInverse,
      0
  );
  ಯದಿ( rc!=SQLITE_MISUSE ) ಗಚ್ಛ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, test_Window_ಮೌಲ್ಯ, 0,
      0
  );
  ಯದಿ( rc!=SQLITE_MISUSE ) ಗಚ್ಛ error;

  ಪ್ರತ್ಯಾಗಮ TCL_OK;

 error:
  Tcl_SetObjResult(interp, Tcl_NewStringObj("misuse test error", -1));
  ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
}

/*
** xStep for sumint().
*/
ಸ್ಥಿರ ಶೂನ್ಯ sumintStep(
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಙ್ಕ nArg, 
  sqlite3_ಮೌಲ್ಯ *apArg[]
){
  sqlite3_int64 *pInt;

  ಪ್ರತಿಜ್ಞಾ( nArg==1 );
  ಯದಿ( sqlite3_ಮೌಲ್ಯ_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_result_error(ctx, "invalid argument", -1);
    ಪ್ರತ್ಯಾಗಮ;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, ಪರಿಮಾಣ(sqlite3_int64));
  ಯದಿ( pInt ){
    *pInt += sqlite3_ಮೌಲ್ಯ_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
ಸ್ಥಿರ ಶೂನ್ಯ sumintInverse(
  sqlite3_context *ctx, 
  ಪೂರ್ಣಾಙ್ಕ nArg, 
  sqlite3_ಮೌಲ್ಯ *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, ಪರಿಮಾಣ(sqlite3_int64));
  *pInt -= sqlite3_ಮೌಲ್ಯ_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
ಸ್ಥಿರ ಶೂನ್ಯ sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  ಯದಿ( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

/*
** xValue for sumint().
*/
ಸ್ಥಿರ ಶೂನ್ಯ sumint_ಮೌಲ್ಯ(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  ಯದಿ( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_create_sumint(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;

  rc = sqlite3_create_window_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumintStep, sumintFinal, sumint_ಮೌಲ್ಯ, sumintInverse,
      0
  );

  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ SQLITE_TCLAPI test_override_sum(
  ಶೂನ್ಯ * clientData,
  Tcl_Interp *interp,
  ಪೂರ್ಣಾಙ್ಕ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ಪೂರ್ಣಾಙ್ಕ rc;

  ಯದಿ( objc!=2 ){
    Tcl_Wrong_ಅಂಕೆ_Args(interp, 1, objv, "DB");
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಯದಿ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ಪ್ರತ್ಯಾಗಮ TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumintStep, sumintFinal
  );

  ಯದಿ( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ಪ್ರತ್ಯಾಗಮ TCL_ERROR;
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}

ಪೂರ್ಣಾಙ್ಕ Sqlitetest_window_Init(Tcl_Interp *interp){
  ಸ್ಥಿರ ಸಂರಚನಾ {
     ವರ್ಣ *zName;
     Tcl_ObjCmdProc *xProc;
     ಪೂರ್ಣಾಙ್ಕ clientData;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_window, 0 },
     { "test_create_window_function_misuse", test_create_window_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_sum, 0 },
  };
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ(i=0; i<ಪರಿಮಾಣ(aObjCmd)/ಪರಿಮಾಣ(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  ಪ್ರತ್ಯಾಗಮ TCL_OK;
}
#endif
