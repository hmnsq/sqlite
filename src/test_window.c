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

#ضمن "sqlite3.h"

#ifdef SQLITE_TEST

#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"

خارجي صحيح getDbPointer(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجي ثابت حرف *sqlite3ErrName(صحيح);

تعريفنوع بنية TestWindow TestWindow;
بنية TestWindow {
  Tcl_Obj *xStep;
  Tcl_Obj *xFinal;
  Tcl_Obj *xValue;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

تعريفنوع بنية TestWindowCtx TestWindowCtx;
بنية TestWindowCtx {
  Tcl_Obj *pVal;
};

ساكن فراغ doTestWindowStep(
  صحيح bInverse,
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_value **apArg
){
  صحيح i;
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->xStep);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, حجم(TestWindowCtx));

  Tcl_IncrRefCount(pEval);
  אם( pCtx ){
    ثابت حرف *zResult;
    صحيح rc;
    אם( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }אחרת{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    עבור(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((ثابت حرف*)sqlite3_value_text(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    אם( rc!=TCL_OK ){
      zResult = Tcl_GetStringResult(p->interp);
      sqlite3_result_error(ctx, zResult, -1);
    }אחרת{
      אם( pCtx->pVal ) Tcl_DecrRefCount(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_GetObjResult(p->interp));
      Tcl_IncrRefCount(pCtx->pVal);
    }
  }
  Tcl_DecrRefCount(pEval);
}

ساكن فراغ doTestWindowFinalize(صحيح bValue, sqlite3_context *ctx){
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bValue ? p->xValue : p->xFinal);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, حجم(TestWindowCtx));

  Tcl_IncrRefCount(pEval);
  אם( pCtx ){
    ثابت حرف *zResult;
    صحيح rc;
    אם( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }אחרת{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    zResult = Tcl_GetStringResult(p->interp);
    אם( rc!=TCL_OK ){
      sqlite3_result_error(ctx, zResult, -1);
    }אחרת{
      sqlite3_result_text(ctx, zResult, -1, SQLITE_TRANSIENT);
    }

    אם( bValue==0 ){
      אם( pCtx->pVal ) Tcl_DecrRefCount(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_DecrRefCount(pEval);
}

ساكن فراغ testWindowStep(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_value **apArg
){
  doTestWindowStep(0, ctx, nArg, apArg);
}
ساكن فراغ testWindowInverse(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_value **apArg
){
  doTestWindowStep(1, ctx, nArg, apArg);
}

ساكن فراغ testWindowFinal(sqlite3_context *ctx){
  doTestWindowFinalize(0, ctx);
}
ساكن فراغ testWindowValue(sqlite3_context *ctx){
  doTestWindowFinalize(1, ctx);
}

ساكن فراغ testWindowDestroy(فراغ *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
ساكن صحيح SQLITE_TCLAPI test_create_window(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  TestWindow *pNew;
  sqlite3 *db;
  ثابت حرف *zName;
  صحيح rc;

  אם( objc!=7 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    החזר TCL_ERROR;
  }

  אם( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) החזר TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (TestWindow*)ckalloc(حجم(TestWindow));
  memset(pNew, 0, حجم(TestWindow));
  pNew->xStep = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->xValue = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_IncrRefCount(pNew->xStep);
  Tcl_IncrRefCount(pNew->xFinal);
  Tcl_IncrRefCount(pNew->xValue);
  Tcl_IncrRefCount(pNew->xInverse);

  rc = sqlite3_create_window_function(db, zName, -1, SQLITE_UTF8, (فراغ*)pNew,
      testWindowStep, testWindowFinal, testWindowValue, testWindowInverse,
      testWindowDestroy
  );
  אם( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    החזר TCL_ERROR;
  }

  החזר TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_create_window_misuse(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحيح rc;

  אם( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    החזר TCL_ERROR;
  }
  אם( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) החזר TCL_ERROR;

  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, testWindowFinal, testWindowValue, testWindowInverse,
      0
  );
  אם( rc!=SQLITE_MISUSE ) اذهب error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, 0, testWindowValue, testWindowInverse,
      0
  );
  אם( rc!=SQLITE_MISUSE ) اذهب error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, 0, testWindowInverse,
      0
  );
  אם( rc!=SQLITE_MISUSE ) اذهب error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, testWindowValue, 0,
      0
  );
  אם( rc!=SQLITE_MISUSE ) اذهب error;

  החזר TCL_OK;

 error:
  Tcl_SetObjResult(interp, Tcl_NewStringObj("misuse test error", -1));
  החזר TCL_ERROR;
}

/*
** xStep for sumint().
*/
ساكن فراغ sumintStep(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_value *apArg[]
){
  sqlite3_int64 *pInt;

  أكد( nArg==1 );
  אם( sqlite3_value_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_result_error(ctx, "invalid argument", -1);
    החזר;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, حجم(sqlite3_int64));
  אם( pInt ){
    *pInt += sqlite3_value_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
ساكن فراغ sumintInverse(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_value *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, حجم(sqlite3_int64));
  *pInt -= sqlite3_value_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
ساكن فراغ sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  אם( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

/*
** xValue for sumint().
*/
ساكن فراغ sumintValue(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  אם( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

ساكن صحيح SQLITE_TCLAPI test_create_sumint(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحيح rc;

  אם( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    החזר TCL_ERROR;
  }
  אם( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) החזר TCL_ERROR;

  rc = sqlite3_create_window_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumintStep, sumintFinal, sumintValue, sumintInverse,
      0
  );

  אם( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    החזר TCL_ERROR;
  }
  החזר TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_override_sum(
  فراغ * clientData,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحيح rc;

  אם( objc!=2 ){
    Tcl_WrongNumArgs(interp, 1, objv, "DB");
    החזר TCL_ERROR;
  }
  אם( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) החזר TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumintStep, sumintFinal
  );

  אם( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    החזר TCL_ERROR;
  }
  החזר TCL_OK;
}

صحيح Sqlitetest_window_Init(Tcl_Interp *interp){
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     صحيح clientData;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_window, 0 },
     { "test_create_window_function_misuse", test_create_window_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_sum, 0 },
  };
  صحيح i;
  עבור(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  החזר TCL_OK;
}
#endif
