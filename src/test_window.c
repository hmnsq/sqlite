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

#অন্তর্ভাৱ "sqlite3.h"

#ifdef SQLITE_TEST

#অন্তর্ভাৱ "sqliteInt.h"
#অন্তর্ভাৱ "tclsqlite.h"

বাহ্য পূর্ণাঙ্ক getDbPointer(Tcl_Interp *interp, ধ্রুৱ ৱর্ণ *zA, sqlite3 **ppDb);
বাহ্য ধ্রুৱ ৱর্ণ *sqlite3ErrName(পূর্ণাঙ্ক);

প্রকারনির্দেশ সংরচনা TestWindow TestWindow;
সংরচনা TestWindow {
  Tcl_Obj *xStep;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_মান;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

প্রকারনির্দেশ সংরচনা TestWindowCtx TestWindowCtx;
সংরচনা TestWindowCtx {
  Tcl_Obj *pVal;
};

স্থির শূন্য doTestWindowStep(
  পূর্ণাঙ্ক bInverse,
  sqlite3_context *ctx, 
  পূর্ণাঙ্ক nArg, 
  sqlite3_মান **apArg
){
  পূর্ণাঙ্ক i;
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->xStep);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, পরিমাণ(TestWindowCtx));

  Tcl_Incr_Ref_গুনতি(pEval);
  যদি( pCtx ){
    ধ্রুৱ ৱর্ণ *zResult;
    পূর্ণাঙ্ক rc;
    যদি( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }অন্যথা{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    বাবে(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((ধ্রুৱ ৱর্ণ*)sqlite3_মান_text(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    যদি( rc!=TCL_OK ){
      zResult = Tcl_GetStringResult(p->interp);
      sqlite3_result_error(ctx, zResult, -1);
    }অন্যথা{
      যদি( pCtx->pVal ) Tcl_Decr_Ref_গুনতি(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_GetObjResult(p->interp));
      Tcl_Incr_Ref_গুনতি(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_গুনতি(pEval);
}

স্থির শূন্য doTestWindowFinalize(পূর্ণাঙ্ক b_মান, sqlite3_context *ctx){
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_মান ? p->x_মান : p->xFinal);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, পরিমাণ(TestWindowCtx));

  Tcl_Incr_Ref_গুনতি(pEval);
  যদি( pCtx ){
    ধ্রুৱ ৱর্ণ *zResult;
    পূর্ণাঙ্ক rc;
    যদি( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }অন্যথা{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    zResult = Tcl_GetStringResult(p->interp);
    যদি( rc!=TCL_OK ){
      sqlite3_result_error(ctx, zResult, -1);
    }অন্যথা{
      sqlite3_result_text(ctx, zResult, -1, SQLITE_TRANSIENT);
    }

    যদি( b_মান==0 ){
      যদি( pCtx->pVal ) Tcl_Decr_Ref_গুনতি(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_গুনতি(pEval);
}

স্থির শূন্য testWindowStep(
  sqlite3_context *ctx, 
  পূর্ণাঙ্ক nArg, 
  sqlite3_মান **apArg
){
  doTestWindowStep(0, ctx, nArg, apArg);
}
স্থির শূন্য testWindowInverse(
  sqlite3_context *ctx, 
  পূর্ণাঙ্ক nArg, 
  sqlite3_মান **apArg
){
  doTestWindowStep(1, ctx, nArg, apArg);
}

স্থির শূন্য testWindowFinal(sqlite3_context *ctx){
  doTestWindowFinalize(0, ctx);
}
স্থির শূন্য test_Window_মান(sqlite3_context *ctx){
  doTestWindowFinalize(1, ctx);
}

স্থির শূন্য testWindowDestroy(শূন্য *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_create_window(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  TestWindow *pNew;
  sqlite3 *db;
  ধ্রুৱ ৱর্ণ *zName;
  পূর্ণাঙ্ক rc;

  যদি( objc!=7 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    ঘূৰাই TCL_ERROR;
  }

  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ঘূৰাই TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (TestWindow*)ckalloc(পরিমাণ(TestWindow));
  memset(pNew, 0, পরিমাণ(TestWindow));
  pNew->xStep = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_মান = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_গুনতি(pNew->xStep);
  Tcl_Incr_Ref_গুনতি(pNew->xFinal);
  Tcl_Incr_Ref_গুনতি(pNew->x_মান);
  Tcl_Incr_Ref_গুনতি(pNew->xInverse);

  rc = sqlite3_create_window_function(db, zName, -1, SQLITE_UTF8, (শূন্য*)pNew,
      testWindowStep, testWindowFinal, test_Window_মান, testWindowInverse,
      testWindowDestroy
  );
  যদি( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ঘূৰাই TCL_ERROR;
  }

  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_create_window_misuse(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  পূর্ণাঙ্ক rc;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ঘূৰাই TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ঘূৰাই TCL_ERROR;

  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, testWindowFinal, test_Window_মান, testWindowInverse,
      0
  );
  যদি( rc!=SQLITE_MISUSE ) গচ্ছ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, 0, test_Window_মান, testWindowInverse,
      0
  );
  যদি( rc!=SQLITE_MISUSE ) গচ্ছ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, 0, testWindowInverse,
      0
  );
  যদি( rc!=SQLITE_MISUSE ) গচ্ছ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, test_Window_মান, 0,
      0
  );
  যদি( rc!=SQLITE_MISUSE ) গচ্ছ error;

  ঘূৰাই TCL_OK;

 error:
  Tcl_SetObjResult(interp, Tcl_NewStringObj("misuse test error", -1));
  ঘূৰাই TCL_ERROR;
}

/*
** xStep for sumint().
*/
স্থির শূন্য sumintStep(
  sqlite3_context *ctx, 
  পূর্ণাঙ্ক nArg, 
  sqlite3_মান *apArg[]
){
  sqlite3_int64 *pInt;

  প্রতিজ্ঞা( nArg==1 );
  যদি( sqlite3_মান_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_result_error(ctx, "invalid argument", -1);
    ঘূৰাই;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, পরিমাণ(sqlite3_int64));
  যদি( pInt ){
    *pInt += sqlite3_মান_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
স্থির শূন্য sumintInverse(
  sqlite3_context *ctx, 
  পূর্ণাঙ্ক nArg, 
  sqlite3_মান *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, পরিমাণ(sqlite3_int64));
  *pInt -= sqlite3_মান_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
স্থির শূন্য sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  যদি( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

/*
** xValue for sumint().
*/
স্থির শূন্য sumint_মান(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  যদি( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_create_sumint(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  পূর্ণাঙ্ক rc;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ঘূৰাই TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ঘূৰাই TCL_ERROR;

  rc = sqlite3_create_window_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumintStep, sumintFinal, sumint_মান, sumintInverse,
      0
  );

  যদি( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ঘূৰাই TCL_ERROR;
  }
  ঘূৰাই TCL_OK;
}

স্থির পূর্ণাঙ্ক SQLITE_TCLAPI test_override_sum(
  শূন্য * clientData,
  Tcl_Interp *interp,
  পূর্ণাঙ্ক objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  পূর্ণাঙ্ক rc;

  যদি( objc!=2 ){
    Tcl_Wrong_অঙ্ক_Args(interp, 1, objv, "DB");
    ঘূৰাই TCL_ERROR;
  }
  যদি( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ঘূৰাই TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumintStep, sumintFinal
  );

  যদি( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ঘূৰাই TCL_ERROR;
  }
  ঘূৰাই TCL_OK;
}

পূর্ণাঙ্ক Sqlitetest_window_Init(Tcl_Interp *interp){
  স্থির সংরচনা {
     ৱর্ণ *zName;
     Tcl_ObjCmdProc *xProc;
     পূর্ণাঙ্ক clientData;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_window, 0 },
     { "test_create_window_function_misuse", test_create_window_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_sum, 0 },
  };
  পূর্ণাঙ্ক i;
  বাবে(i=0; i<পরিমাণ(aObjCmd)/পরিমাণ(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  ঘূৰাই TCL_OK;
}
#endif
