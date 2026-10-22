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

#અન્તર્ભાવ "sqlite3.h"

#ifdef SQLITE_TEST

#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ "tclsqlite.h"

બાહ્ય પૂર્ણાંક getDbPointer(Tcl_Interp *interp, અચલ અક્ષર *zA, sqlite3 **ppDb);
બાહ્ય અચલ અક્ષર *sqlite3ErrName(પૂર્ણાંક);

પ્રકારવ્યાખ્યા સંરચના TestWindow TestWindow;
સંરચના TestWindow {
  Tcl_Obj *xStep;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_કિંમત;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

પ્રકારવ્યાખ્યા સંરચના TestWindowCtx TestWindowCtx;
સંરચના TestWindowCtx {
  Tcl_Obj *pVal;
};

સ્થિર શૂન્ય doTestWindowStep(
  પૂર્ણાંક bInverse,
  sqlite3_context *ctx, 
  પૂર્ણાંક nArg, 
  sqlite3_કિંમત **apArg
){
  પૂર્ણાંક i;
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->xStep);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, કદ(TestWindowCtx));

  Tcl_Incr_Ref_ગણતરી(pEval);
  જો( pCtx ){
    અચલ અક્ષર *zResult;
    પૂર્ણાંક rc;
    જો( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }નહીંતર{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    માટે(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((અચલ અક્ષર*)sqlite3_કિંમત_text(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    જો( rc!=TCL_OK ){
      zResult = Tcl_GetStringResult(p->interp);
      sqlite3_result_error(ctx, zResult, -1);
    }નહીંતર{
      જો( pCtx->pVal ) Tcl_Decr_Ref_ગણતરી(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_GetObjResult(p->interp));
      Tcl_Incr_Ref_ગણતરી(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_ગણતરી(pEval);
}

સ્થિર શૂન્ય doTestWindowFinalize(પૂર્ણાંક b_કિંમત, sqlite3_context *ctx){
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_કિંમત ? p->x_કિંમત : p->xFinal);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, કદ(TestWindowCtx));

  Tcl_Incr_Ref_ગણતરી(pEval);
  જો( pCtx ){
    અચલ અક્ષર *zResult;
    પૂર્ણાંક rc;
    જો( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }નહીંતર{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    zResult = Tcl_GetStringResult(p->interp);
    જો( rc!=TCL_OK ){
      sqlite3_result_error(ctx, zResult, -1);
    }નહીંતર{
      sqlite3_result_text(ctx, zResult, -1, SQLITE_TRANSIENT);
    }

    જો( b_કિંમત==0 ){
      જો( pCtx->pVal ) Tcl_Decr_Ref_ગણતરી(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_ગણતરી(pEval);
}

સ્થિર શૂન્ય testWindowStep(
  sqlite3_context *ctx, 
  પૂર્ણાંક nArg, 
  sqlite3_કિંમત **apArg
){
  doTestWindowStep(0, ctx, nArg, apArg);
}
સ્થિર શૂન્ય testWindowInverse(
  sqlite3_context *ctx, 
  પૂર્ણાંક nArg, 
  sqlite3_કિંમત **apArg
){
  doTestWindowStep(1, ctx, nArg, apArg);
}

સ્થિર શૂન્ય testWindowFinal(sqlite3_context *ctx){
  doTestWindowFinalize(0, ctx);
}
સ્થિર શૂન્ય test_Window_કિંમત(sqlite3_context *ctx){
  doTestWindowFinalize(1, ctx);
}

સ્થિર શૂન્ય testWindowDestroy(શૂન્ય *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_create_window(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  TestWindow *pNew;
  sqlite3 *db;
  અચલ અક્ષર *zName;
  પૂર્ણાંક rc;

  જો( objc!=7 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    પાછા TCL_ERROR;
  }

  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (TestWindow*)ckalloc(કદ(TestWindow));
  memset(pNew, 0, કદ(TestWindow));
  pNew->xStep = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_કિંમત = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_ગણતરી(pNew->xStep);
  Tcl_Incr_Ref_ગણતરી(pNew->xFinal);
  Tcl_Incr_Ref_ગણતરી(pNew->x_કિંમત);
  Tcl_Incr_Ref_ગણતરી(pNew->xInverse);

  rc = sqlite3_create_window_function(db, zName, -1, SQLITE_UTF8, (શૂન્ય*)pNew,
      testWindowStep, testWindowFinal, test_Window_કિંમત, testWindowInverse,
      testWindowDestroy
  );
  જો( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    પાછા TCL_ERROR;
  }

  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_create_window_misuse(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  પૂર્ણાંક rc;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;

  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, testWindowFinal, test_Window_કિંમત, testWindowInverse,
      0
  );
  જો( rc!=SQLITE_MISUSE ) જાઓ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, 0, test_Window_કિંમત, testWindowInverse,
      0
  );
  જો( rc!=SQLITE_MISUSE ) જાઓ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, 0, testWindowInverse,
      0
  );
  જો( rc!=SQLITE_MISUSE ) જાઓ error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, test_Window_કિંમત, 0,
      0
  );
  જો( rc!=SQLITE_MISUSE ) જાઓ error;

  પાછા TCL_OK;

 error:
  Tcl_SetObjResult(interp, Tcl_NewStringObj("misuse test error", -1));
  પાછા TCL_ERROR;
}

/*
** xStep for sumint().
*/
સ્થિર શૂન્ય sumintStep(
  sqlite3_context *ctx, 
  પૂર્ણાંક nArg, 
  sqlite3_કિંમત *apArg[]
){
  sqlite3_int64 *pInt;

  પ્રતિજ્ઞા( nArg==1 );
  જો( sqlite3_કિંમત_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_result_error(ctx, "invalid argument", -1);
    પાછા;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, કદ(sqlite3_int64));
  જો( pInt ){
    *pInt += sqlite3_કિંમત_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
સ્થિર શૂન્ય sumintInverse(
  sqlite3_context *ctx, 
  પૂર્ણાંક nArg, 
  sqlite3_કિંમત *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, કદ(sqlite3_int64));
  *pInt -= sqlite3_કિંમત_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
સ્થિર શૂન્ય sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  જો( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

/*
** xValue for sumint().
*/
સ્થિર શૂન્ય sumint_કિંમત(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  જો( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_create_sumint(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  પૂર્ણાંક rc;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;

  rc = sqlite3_create_window_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumintStep, sumintFinal, sumint_કિંમત, sumintInverse,
      0
  );

  જો( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}

સ્થિર પૂર્ણાંક SQLITE_TCLAPI test_override_sum(
  શૂન્ય * clientData,
  Tcl_Interp *interp,
  પૂર્ણાંક objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  પૂર્ણાંક rc;

  જો( objc!=2 ){
    Tcl_Wrong_અંક_Args(interp, 1, objv, "DB");
    પાછા TCL_ERROR;
  }
  જો( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) પાછા TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumintStep, sumintFinal
  );

  જો( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    પાછા TCL_ERROR;
  }
  પાછા TCL_OK;
}

પૂર્ણાંક Sqlitetest_window_Init(Tcl_Interp *interp){
  સ્થિર સંરચના {
     અક્ષર *zName;
     Tcl_ObjCmdProc *xProc;
     પૂર્ણાંક clientData;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_window, 0 },
     { "test_create_window_function_misuse", test_create_window_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_sum, 0 },
  };
  પૂર્ણાંક i;
  માટે(i=0; i<કદ(aObjCmd)/કદ(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  પાછા TCL_OK;
}
#endif
