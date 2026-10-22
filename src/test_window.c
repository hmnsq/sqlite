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

#شامل "sqlite3.h"

#ifdef SQLITE_TEST

#شامل "sqliteInt.h"
#شامل "tclsqlite.h"

بیرونی صحیح getDbPointer(Tcl_Interp *interp, ثابت نویسه *zA, sqlite3 **ppDb);
بیرونی ثابت نویسه *sqlite3ErrName(صحیح);

تعریفنوع ساختار TestWindow TestWindow;
ساختار TestWindow {
  Tcl_Obj *xStep;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_مقدار;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

تعریفنوع ساختار TestWindowCtx TestWindowCtx;
ساختار TestWindowCtx {
  Tcl_Obj *pVal;
};

ایستا تهی doTestWindowStep(
  صحیح bInverse,
  sqlite3_context *ctx, 
  صحیح nArg, 
  sqlite3_مقدار **apArg
){
  صحیح i;
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->xStep);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, اندازه(TestWindowCtx));

  Tcl_Incr_Ref_شمار(pEval);
  اگر( pCtx ){
    ثابت نویسه *zResult;
    صحیح rc;
    اگر( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }وگرنه{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    برای(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((ثابت نویسه*)sqlite3_مقدار_text(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    اگر( rc!=TCL_OK ){
      zResult = Tcl_GetStringResult(p->interp);
      sqlite3_result_error(ctx, zResult, -1);
    }وگرنه{
      اگر( pCtx->pVal ) Tcl_Decr_Ref_شمار(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_GetObjResult(p->interp));
      Tcl_Incr_Ref_شمار(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_شمار(pEval);
}

ایستا تهی doTestWindowFinalize(صحیح b_مقدار, sqlite3_context *ctx){
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_مقدار ? p->x_مقدار : p->xFinal);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, اندازه(TestWindowCtx));

  Tcl_Incr_Ref_شمار(pEval);
  اگر( pCtx ){
    ثابت نویسه *zResult;
    صحیح rc;
    اگر( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }وگرنه{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    zResult = Tcl_GetStringResult(p->interp);
    اگر( rc!=TCL_OK ){
      sqlite3_result_error(ctx, zResult, -1);
    }وگرنه{
      sqlite3_result_text(ctx, zResult, -1, SQLITE_TRANSIENT);
    }

    اگر( b_مقدار==0 ){
      اگر( pCtx->pVal ) Tcl_Decr_Ref_شمار(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_شمار(pEval);
}

ایستا تهی testWindowStep(
  sqlite3_context *ctx, 
  صحیح nArg, 
  sqlite3_مقدار **apArg
){
  doTestWindowStep(0, ctx, nArg, apArg);
}
ایستا تهی testWindowInverse(
  sqlite3_context *ctx, 
  صحیح nArg, 
  sqlite3_مقدار **apArg
){
  doTestWindowStep(1, ctx, nArg, apArg);
}

ایستا تهی testWindowFinal(sqlite3_context *ctx){
  doTestWindowFinalize(0, ctx);
}
ایستا تهی test_Window_مقدار(sqlite3_context *ctx){
  doTestWindowFinalize(1, ctx);
}

ایستا تهی testWindowDestroy(تهی *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
ایستا صحیح SQLITE_TCLAPI test_create_window(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  TestWindow *pNew;
  sqlite3 *db;
  ثابت نویسه *zName;
  صحیح rc;

  اگر( objc!=7 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    بازگشت TCL_ERROR;
  }

  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (TestWindow*)ckalloc(اندازه(TestWindow));
  memset(pNew, 0, اندازه(TestWindow));
  pNew->xStep = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_مقدار = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_شمار(pNew->xStep);
  Tcl_Incr_Ref_شمار(pNew->xFinal);
  Tcl_Incr_Ref_شمار(pNew->x_مقدار);
  Tcl_Incr_Ref_شمار(pNew->xInverse);

  rc = sqlite3_create_window_function(db, zName, -1, SQLITE_UTF8, (تهی*)pNew,
      testWindowStep, testWindowFinal, test_Window_مقدار, testWindowInverse,
      testWindowDestroy
  );
  اگر( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    بازگشت TCL_ERROR;
  }

  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_create_window_misuse(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحیح rc;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB");
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;

  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, testWindowFinal, test_Window_مقدار, testWindowInverse,
      0
  );
  اگر( rc!=SQLITE_MISUSE ) برو error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, 0, test_Window_مقدار, testWindowInverse,
      0
  );
  اگر( rc!=SQLITE_MISUSE ) برو error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, 0, testWindowInverse,
      0
  );
  اگر( rc!=SQLITE_MISUSE ) برو error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, test_Window_مقدار, 0,
      0
  );
  اگر( rc!=SQLITE_MISUSE ) برو error;

  بازگشت TCL_OK;

 error:
  Tcl_SetObjResult(interp, Tcl_NewStringObj("misuse test error", -1));
  بازگشت TCL_ERROR;
}

/*
** xStep for sumint().
*/
ایستا تهی sumintStep(
  sqlite3_context *ctx, 
  صحیح nArg, 
  sqlite3_مقدار *apArg[]
){
  sqlite3_int64 *pInt;

  أكد( nArg==1 );
  اگر( sqlite3_مقدار_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_result_error(ctx, "invalid argument", -1);
    بازگشت;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, اندازه(sqlite3_int64));
  اگر( pInt ){
    *pInt += sqlite3_مقدار_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
ایستا تهی sumintInverse(
  sqlite3_context *ctx, 
  صحیح nArg, 
  sqlite3_مقدار *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, اندازه(sqlite3_int64));
  *pInt -= sqlite3_مقدار_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
ایستا تهی sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  اگر( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

/*
** xValue for sumint().
*/
ایستا تهی sumint_مقدار(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  اگر( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

ایستا صحیح SQLITE_TCLAPI test_create_sumint(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحیح rc;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB");
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;

  rc = sqlite3_create_window_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumintStep, sumintFinal, sumint_مقدار, sumintInverse,
      0
  );

  اگر( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}

ایستا صحیح SQLITE_TCLAPI test_override_sum(
  تهی * clientData,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحیح rc;

  اگر( objc!=2 ){
    Tcl_Wrong_رقم_Args(interp, 1, objv, "DB");
    بازگشت TCL_ERROR;
  }
  اگر( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) بازگشت TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumintStep, sumintFinal
  );

  اگر( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    بازگشت TCL_ERROR;
  }
  بازگشت TCL_OK;
}

صحیح Sqlitetest_window_Init(Tcl_Interp *interp){
  ایستا ساختار {
     نویسه *zName;
     Tcl_ObjCmdProc *xProc;
     صحیح clientData;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_window, 0 },
     { "test_create_window_function_misuse", test_create_window_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_sum, 0 },
  };
  صحیح i;
  برای(i=0; i<اندازه(aObjCmd)/اندازه(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  بازگشت TCL_OK;
}
#endif
