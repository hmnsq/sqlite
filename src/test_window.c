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

#അന്തര്ഭാവ "sqlite3.h"

#ifdef SQLITE_TEST

#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "tclsqlite.h"

ബാഹ്യ പൂർണ്ണസംഖ്യ getDbPointer(Tcl_Interp *interp, സ്ഥിരം അക്ഷരം *zA, sqlite3 **ppDb);
ബാഹ്യ സ്ഥിരം അക്ഷരം *sqlite3ErrName(പൂർണ്ണസംഖ്യ);

തരനിർവചനം ഘടന TestWindow TestWindow;
ഘടന TestWindow {
  Tcl_Obj *xStep;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_മൂല്യം;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

തരനിർവചനം ഘടന TestWindowCtx TestWindowCtx;
ഘടന TestWindowCtx {
  Tcl_Obj *pVal;
};

സ്ഥിതം ശൂന്യം doTestWindowStep(
  പൂർണ്ണസംഖ്യ bInverse,
  sqlite3_context *ctx, 
  പൂർണ്ണസംഖ്യ nArg, 
  sqlite3_മൂല്യം **apArg
){
  പൂർണ്ണസംഖ്യ i;
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->xStep);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, വലുപ്പം(TestWindowCtx));

  Tcl_Incr_Ref_എണ്ണം(pEval);
  എങ്കിൽ( pCtx ){
    സ്ഥിരം അക്ഷരം *zResult;
    പൂർണ്ണസംഖ്യ rc;
    എങ്കിൽ( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }അല്ലെങ്കിൽ{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    വേണ്ടി(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((സ്ഥിരം അക്ഷരം*)sqlite3_മൂല്യം_text(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    എങ്കിൽ( rc!=TCL_OK ){
      zResult = Tcl_GetStringResult(p->interp);
      sqlite3_result_error(ctx, zResult, -1);
    }അല്ലെങ്കിൽ{
      എങ്കിൽ( pCtx->pVal ) Tcl_Decr_Ref_എണ്ണം(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_GetObjResult(p->interp));
      Tcl_Incr_Ref_എണ്ണം(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_എണ്ണം(pEval);
}

സ്ഥിതം ശൂന്യം doTestWindowFinalize(പൂർണ്ണസംഖ്യ b_മൂല്യം, sqlite3_context *ctx){
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_മൂല്യം ? p->x_മൂല്യം : p->xFinal);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, വലുപ്പം(TestWindowCtx));

  Tcl_Incr_Ref_എണ്ണം(pEval);
  എങ്കിൽ( pCtx ){
    സ്ഥിരം അക്ഷരം *zResult;
    പൂർണ്ണസംഖ്യ rc;
    എങ്കിൽ( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }അല്ലെങ്കിൽ{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    zResult = Tcl_GetStringResult(p->interp);
    എങ്കിൽ( rc!=TCL_OK ){
      sqlite3_result_error(ctx, zResult, -1);
    }അല്ലെങ്കിൽ{
      sqlite3_result_text(ctx, zResult, -1, SQLITE_TRANSIENT);
    }

    എങ്കിൽ( b_മൂല്യം==0 ){
      എങ്കിൽ( pCtx->pVal ) Tcl_Decr_Ref_എണ്ണം(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_എണ്ണം(pEval);
}

സ്ഥിതം ശൂന്യം testWindowStep(
  sqlite3_context *ctx, 
  പൂർണ്ണസംഖ്യ nArg, 
  sqlite3_മൂല്യം **apArg
){
  doTestWindowStep(0, ctx, nArg, apArg);
}
സ്ഥിതം ശൂന്യം testWindowInverse(
  sqlite3_context *ctx, 
  പൂർണ്ണസംഖ്യ nArg, 
  sqlite3_മൂല്യം **apArg
){
  doTestWindowStep(1, ctx, nArg, apArg);
}

സ്ഥിതം ശൂന്യം testWindowFinal(sqlite3_context *ctx){
  doTestWindowFinalize(0, ctx);
}
സ്ഥിതം ശൂന്യം test_Window_മൂല്യം(sqlite3_context *ctx){
  doTestWindowFinalize(1, ctx);
}

സ്ഥിതം ശൂന്യം testWindowDestroy(ശൂന്യം *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_create_window(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  TestWindow *pNew;
  sqlite3 *db;
  സ്ഥിരം അക്ഷരം *zName;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=7 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    തിരികെ TCL_ERROR;
  }

  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (TestWindow*)ckalloc(വലുപ്പം(TestWindow));
  memset(pNew, 0, വലുപ്പം(TestWindow));
  pNew->xStep = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_മൂല്യം = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_എണ്ണം(pNew->xStep);
  Tcl_Incr_Ref_എണ്ണം(pNew->xFinal);
  Tcl_Incr_Ref_എണ്ണം(pNew->x_മൂല്യം);
  Tcl_Incr_Ref_എണ്ണം(pNew->xInverse);

  rc = sqlite3_create_window_function(db, zName, -1, SQLITE_UTF8, (ശൂന്യം*)pNew,
      testWindowStep, testWindowFinal, test_Window_മൂല്യം, testWindowInverse,
      testWindowDestroy
  );
  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    തിരികെ TCL_ERROR;
  }

  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_create_window_misuse(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;

  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, testWindowFinal, test_Window_മൂല്യം, testWindowInverse,
      0
  );
  എങ്കിൽ( rc!=SQLITE_MISUSE ) പോകുക error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, 0, test_Window_മൂല്യം, testWindowInverse,
      0
  );
  എങ്കിൽ( rc!=SQLITE_MISUSE ) പോകുക error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, 0, testWindowInverse,
      0
  );
  എങ്കിൽ( rc!=SQLITE_MISUSE ) പോകുക error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, test_Window_മൂല്യം, 0,
      0
  );
  എങ്കിൽ( rc!=SQLITE_MISUSE ) പോകുക error;

  തിരികെ TCL_OK;

 error:
  Tcl_SetObjResult(interp, Tcl_NewStringObj("misuse test error", -1));
  തിരികെ TCL_ERROR;
}

/*
** xStep for sumint().
*/
സ്ഥിതം ശൂന്യം sumintStep(
  sqlite3_context *ctx, 
  പൂർണ്ണസംഖ്യ nArg, 
  sqlite3_മൂല്യം *apArg[]
){
  sqlite3_int64 *pInt;

  പ്രതിജ്ഞാ( nArg==1 );
  എങ്കിൽ( sqlite3_മൂല്യം_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_result_error(ctx, "invalid argument", -1);
    തിരികെ;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, വലുപ്പം(sqlite3_int64));
  എങ്കിൽ( pInt ){
    *pInt += sqlite3_മൂല്യം_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
സ്ഥിതം ശൂന്യം sumintInverse(
  sqlite3_context *ctx, 
  പൂർണ്ണസംഖ്യ nArg, 
  sqlite3_മൂല്യം *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, വലുപ്പം(sqlite3_int64));
  *pInt -= sqlite3_മൂല്യം_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
സ്ഥിതം ശൂന്യം sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  എങ്കിൽ( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

/*
** xValue for sumint().
*/
സ്ഥിതം ശൂന്യം sumint_മൂല്യം(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  എങ്കിൽ( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_create_sumint(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;

  rc = sqlite3_create_window_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumintStep, sumintFinal, sumint_മൂല്യം, sumintInverse,
      0
  );

  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ SQLITE_TCLAPI test_override_sum(
  ശൂന്യം * clientData,
  Tcl_Interp *interp,
  പൂർണ്ണസംഖ്യ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  പൂർണ്ണസംഖ്യ rc;

  എങ്കിൽ( objc!=2 ){
    Tcl_Wrong_അക്കം_Args(interp, 1, objv, "DB");
    തിരികെ TCL_ERROR;
  }
  എങ്കിൽ( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) തിരികെ TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumintStep, sumintFinal
  );

  എങ്കിൽ( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    തിരികെ TCL_ERROR;
  }
  തിരികെ TCL_OK;
}

പൂർണ്ണസംഖ്യ Sqlitetest_window_Init(Tcl_Interp *interp){
  സ്ഥിതം ഘടന {
     അക്ഷരം *zName;
     Tcl_ObjCmdProc *xProc;
     പൂർണ്ണസംഖ്യ clientData;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_window, 0 },
     { "test_create_window_function_misuse", test_create_window_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_sum, 0 },
  };
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=0; i<വലുപ്പം(aObjCmd)/വലുപ്പം(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  തിരികെ TCL_OK;
}
#endif
