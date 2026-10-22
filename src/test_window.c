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

#ଅନ୍ତର୍ଭାଵ "sqlite3.h"

#ifdef SQLITE_TEST

#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ "tclsqlite.h"

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା getDbPointer(Tcl_Interp *interp, ଅଚଳ ଅକ୍ଷର *zA, sqlite3 **ppDb);
ବାହ୍ୟ ଅଚଳ ଅକ୍ଷର *sqlite3ErrName(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା TestWindow TestWindow;
ସଂରଚନା TestWindow {
  Tcl_Obj *xStep;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_ମୂଲ୍ୟ;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା TestWindowCtx TestWindowCtx;
ସଂରଚନା TestWindowCtx {
  Tcl_Obj *pVal;
};

ସ୍ଥିର ଶୂନ୍ୟ doTestWindowStep(
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bInverse,
  sqlite3_context *ctx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg, 
  sqlite3_ମୂଲ୍ୟ **apArg
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->xStep);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, ଆକାର(TestWindowCtx));

  Tcl_Incr_Ref_ଗଣନା(pEval);
  ଯଦି( pCtx ){
    ଅଚଳ ଅକ୍ଷର *zResult;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    ଯଦି( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }ଅନ୍ୟଥା{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    ପାଇଁ(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((ଅଚଳ ଅକ୍ଷର*)sqlite3_ମୂଲ୍ୟ_text(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    ଯଦି( rc!=TCL_OK ){
      zResult = Tcl_GetStringResult(p->interp);
      sqlite3_result_error(ctx, zResult, -1);
    }ଅନ୍ୟଥା{
      ଯଦି( pCtx->pVal ) Tcl_Decr_Ref_ଗଣନା(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_GetObjResult(p->interp));
      Tcl_Incr_Ref_ଗଣନା(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_ଗଣନା(pEval);
}

ସ୍ଥିର ଶୂନ୍ୟ doTestWindowFinalize(ପୂର୍ଣ୍ଣସଂଖ୍ୟା b_ମୂଲ୍ୟ, sqlite3_context *ctx){
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_ମୂଲ୍ୟ ? p->x_ମୂଲ୍ୟ : p->xFinal);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, ଆକାର(TestWindowCtx));

  Tcl_Incr_Ref_ଗଣନା(pEval);
  ଯଦି( pCtx ){
    ଅଚଳ ଅକ୍ଷର *zResult;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    ଯଦି( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }ଅନ୍ୟଥା{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    zResult = Tcl_GetStringResult(p->interp);
    ଯଦି( rc!=TCL_OK ){
      sqlite3_result_error(ctx, zResult, -1);
    }ଅନ୍ୟଥା{
      sqlite3_result_text(ctx, zResult, -1, SQLITE_TRANSIENT);
    }

    ଯଦି( b_ମୂଲ୍ୟ==0 ){
      ଯଦି( pCtx->pVal ) Tcl_Decr_Ref_ଗଣନା(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_ଗଣନା(pEval);
}

ସ୍ଥିର ଶୂନ୍ୟ testWindowStep(
  sqlite3_context *ctx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg, 
  sqlite3_ମୂଲ୍ୟ **apArg
){
  doTestWindowStep(0, ctx, nArg, apArg);
}
ସ୍ଥିର ଶୂନ୍ୟ testWindowInverse(
  sqlite3_context *ctx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg, 
  sqlite3_ମୂଲ୍ୟ **apArg
){
  doTestWindowStep(1, ctx, nArg, apArg);
}

ସ୍ଥିର ଶୂନ୍ୟ testWindowFinal(sqlite3_context *ctx){
  doTestWindowFinalize(0, ctx);
}
ସ୍ଥିର ଶୂନ୍ୟ test_Window_ମୂଲ୍ୟ(sqlite3_context *ctx){
  doTestWindowFinalize(1, ctx);
}

ସ୍ଥିର ଶୂନ୍ୟ testWindowDestroy(ଶୂନ୍ୟ *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_create_window(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  TestWindow *pNew;
  sqlite3 *db;
  ଅଚଳ ଅକ୍ଷର *zName;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( objc!=7 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (TestWindow*)ckalloc(ଆକାର(TestWindow));
  memset(pNew, 0, ଆକାର(TestWindow));
  pNew->xStep = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_ମୂଲ୍ୟ = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_ଗଣନା(pNew->xStep);
  Tcl_Incr_Ref_ଗଣନା(pNew->xFinal);
  Tcl_Incr_Ref_ଗଣନା(pNew->x_ମୂଲ୍ୟ);
  Tcl_Incr_Ref_ଗଣନା(pNew->xInverse);

  rc = sqlite3_create_window_function(db, zName, -1, SQLITE_UTF8, (ଶୂନ୍ୟ*)pNew,
      testWindowStep, testWindowFinal, test_Window_ମୂଲ୍ୟ, testWindowInverse,
      testWindowDestroy
  );
  ଯଦି( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ଫେରନ୍ତୁ TCL_ERROR;
  }

  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_create_window_misuse(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;

  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, testWindowFinal, test_Window_ମୂଲ୍ୟ, testWindowInverse,
      0
  );
  ଯଦି( rc!=SQLITE_MISUSE ) ଯାଅ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, 0, test_Window_ମୂଲ୍ୟ, testWindowInverse,
      0
  );
  ଯଦି( rc!=SQLITE_MISUSE ) ଯାଅ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, 0, testWindowInverse,
      0
  );
  ଯଦି( rc!=SQLITE_MISUSE ) ଯାଅ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, test_Window_ମୂଲ୍ୟ, 0,
      0
  );
  ଯଦି( rc!=SQLITE_MISUSE ) ଯାଅ error;

  ଫେରନ୍ତୁ TCL_OK;

 error:
  Tcl_SetObjResult(interp, Tcl_NewStringObj("misuse test error", -1));
  ଫେରନ୍ତୁ TCL_ERROR;
}

/*
** xStep for sumint().
*/
ସ୍ଥିର ଶୂନ୍ୟ sumintStep(
  sqlite3_context *ctx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg, 
  sqlite3_ମୂଲ୍ୟ *apArg[]
){
  sqlite3_int64 *pInt;

  ପ୍ରତିଜ୍ଞା( nArg==1 );
  ଯଦି( sqlite3_ମୂଲ୍ୟ_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_result_error(ctx, "invalid argument", -1);
    ଫେରନ୍ତୁ;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, ଆକାର(sqlite3_int64));
  ଯଦି( pInt ){
    *pInt += sqlite3_ମୂଲ୍ୟ_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
ସ୍ଥିର ଶୂନ୍ୟ sumintInverse(
  sqlite3_context *ctx, 
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nArg, 
  sqlite3_ମୂଲ୍ୟ *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, ଆକାର(sqlite3_int64));
  *pInt -= sqlite3_ମୂଲ୍ୟ_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
ସ୍ଥିର ଶୂନ୍ୟ sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  ଯଦି( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

/*
** xValue for sumint().
*/
ସ୍ଥିର ଶୂନ୍ୟ sumint_ମୂଲ୍ୟ(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  ଯଦି( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_create_sumint(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;

  rc = sqlite3_create_window_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumintStep, sumintFinal, sumint_ମୂଲ୍ୟ, sumintInverse,
      0
  );

  ଯଦି( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା SQLITE_TCLAPI test_override_sum(
  ଶୂନ୍ୟ * clientData,
  Tcl_Interp *interp,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

  ଯଦି( objc!=2 ){
    Tcl_Wrong_ଅଙ୍କ_Args(interp, 1, objv, "DB");
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଯଦି( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) ଫେରନ୍ତୁ TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumintStep, sumintFinal
  );

  ଯଦି( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ଫେରନ୍ତୁ TCL_ERROR;
  }
  ଫେରନ୍ତୁ TCL_OK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sqlitetest_window_Init(Tcl_Interp *interp){
  ସ୍ଥିର ସଂରଚନା {
     ଅକ୍ଷର *zName;
     Tcl_ObjCmdProc *xProc;
     ପୂର୍ଣ୍ଣସଂଖ୍ୟା clientData;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_window, 0 },
     { "test_create_window_function_misuse", test_create_window_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_sum, 0 },
  };
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ(i=0; i<ଆକାର(aObjCmd)/ଆକାର(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  ଫେରନ୍ତୁ TCL_OK;
}
#endif
