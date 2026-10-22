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

#அந்தர்பாவ "sqlite3.h"

#ifdef SQLITE_TEST

#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "tclsqlite.h"

வெளி முழு getDbPointer(Tcl_Interp *interp, மாறிலி எழுத்து *zA, sqlite3 **ppDb);
வெளி மாறிலி எழுத்து *sqlite3ErrName(முழு);

வகைவரை கட்டமைப்பு TestWindow TestWindow;
கட்டமைப்பு TestWindow {
  Tcl_Obj *xStep;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_மதிப்பு;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

வகைவரை கட்டமைப்பு TestWindowCtx TestWindowCtx;
கட்டமைப்பு TestWindowCtx {
  Tcl_Obj *pVal;
};

நிலையான வெறுமை doTestWindowStep(
  முழு bInverse,
  sqlite3_context *ctx, 
  முழு nArg, 
  sqlite3_மதிப்பு **apArg
){
  முழு i;
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->xStep);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, அளவு(TestWindowCtx));

  Tcl_Incr_Ref_மொத்தம்(pEval);
  எனில்( pCtx ){
    மாறிலி எழுத்து *zResult;
    முழு rc;
    எனில்( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }இல்லையெனில்{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    க்கு(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((மாறிலி எழுத்து*)sqlite3_மதிப்பு_text(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    எனில்( rc!=TCL_OK ){
      zResult = Tcl_GetStringResult(p->interp);
      sqlite3_result_error(ctx, zResult, -1);
    }இல்லையெனில்{
      எனில்( pCtx->pVal ) Tcl_Decr_Ref_மொத்தம்(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_GetObjResult(p->interp));
      Tcl_Incr_Ref_மொத்தம்(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_மொத்தம்(pEval);
}

நிலையான வெறுமை doTestWindowFinalize(முழு b_மதிப்பு, sqlite3_context *ctx){
  TestWindow *p = (TestWindow*)sqlite3_user_data(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_மதிப்பு ? p->x_மதிப்பு : p->xFinal);
  TestWindowCtx *pCtx = sqlite3_aggregate_context(ctx, அளவு(TestWindowCtx));

  Tcl_Incr_Ref_மொத்தம்(pEval);
  எனில்( pCtx ){
    மாறிலி எழுத்து *zResult;
    முழு rc;
    எனில்( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }இல்லையெனில்{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    zResult = Tcl_GetStringResult(p->interp);
    எனில்( rc!=TCL_OK ){
      sqlite3_result_error(ctx, zResult, -1);
    }இல்லையெனில்{
      sqlite3_result_text(ctx, zResult, -1, SQLITE_TRANSIENT);
    }

    எனில்( b_மதிப்பு==0 ){
      எனில்( pCtx->pVal ) Tcl_Decr_Ref_மொத்தம்(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_மொத்தம்(pEval);
}

நிலையான வெறுமை testWindowStep(
  sqlite3_context *ctx, 
  முழு nArg, 
  sqlite3_மதிப்பு **apArg
){
  doTestWindowStep(0, ctx, nArg, apArg);
}
நிலையான வெறுமை testWindowInverse(
  sqlite3_context *ctx, 
  முழு nArg, 
  sqlite3_மதிப்பு **apArg
){
  doTestWindowStep(1, ctx, nArg, apArg);
}

நிலையான வெறுமை testWindowFinal(sqlite3_context *ctx){
  doTestWindowFinalize(0, ctx);
}
நிலையான வெறுமை test_Window_மதிப்பு(sqlite3_context *ctx){
  doTestWindowFinalize(1, ctx);
}

நிலையான வெறுமை testWindowDestroy(வெறுமை *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
நிலையான முழு SQLITE_TCLAPI test_create_window(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  TestWindow *pNew;
  sqlite3 *db;
  மாறிலி எழுத்து *zName;
  முழு rc;

  எனில்( objc!=7 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    திரும்பு TCL_ERROR;
  }

  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (TestWindow*)ckalloc(அளவு(TestWindow));
  memset(pNew, 0, அளவு(TestWindow));
  pNew->xStep = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_மதிப்பு = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_மொத்தம்(pNew->xStep);
  Tcl_Incr_Ref_மொத்தம்(pNew->xFinal);
  Tcl_Incr_Ref_மொத்தம்(pNew->x_மதிப்பு);
  Tcl_Incr_Ref_மொத்தம்(pNew->xInverse);

  rc = sqlite3_create_window_function(db, zName, -1, SQLITE_UTF8, (வெறுமை*)pNew,
      testWindowStep, testWindowFinal, test_Window_மதிப்பு, testWindowInverse,
      testWindowDestroy
  );
  எனில்( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    திரும்பு TCL_ERROR;
  }

  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_create_window_misuse(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  முழு rc;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;

  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, testWindowFinal, test_Window_மதிப்பு, testWindowInverse,
      0
  );
  எனில்( rc!=SQLITE_MISUSE ) செல் error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, 0, test_Window_மதிப்பு, testWindowInverse,
      0
  );
  எனில்( rc!=SQLITE_MISUSE ) செல் error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, 0, testWindowInverse,
      0
  );
  எனில்( rc!=SQLITE_MISUSE ) செல் error;
  rc = sqlite3_create_window_function(db, "fff", -1, SQLITE_UTF8, 0,
      testWindowStep, testWindowFinal, test_Window_மதிப்பு, 0,
      0
  );
  எனில்( rc!=SQLITE_MISUSE ) செல் error;

  திரும்பு TCL_OK;

 error:
  Tcl_SetObjResult(interp, Tcl_NewStringObj("misuse test error", -1));
  திரும்பு TCL_ERROR;
}

/*
** xStep for sumint().
*/
நிலையான வெறுமை sumintStep(
  sqlite3_context *ctx, 
  முழு nArg, 
  sqlite3_மதிப்பு *apArg[]
){
  sqlite3_int64 *pInt;

  ப்ரதிஜ்ஞா( nArg==1 );
  எனில்( sqlite3_மதிப்பு_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_result_error(ctx, "invalid argument", -1);
    திரும்பு;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, அளவு(sqlite3_int64));
  எனில்( pInt ){
    *pInt += sqlite3_மதிப்பு_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
நிலையான வெறுமை sumintInverse(
  sqlite3_context *ctx, 
  முழு nArg, 
  sqlite3_மதிப்பு *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, அளவு(sqlite3_int64));
  *pInt -= sqlite3_மதிப்பு_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
நிலையான வெறுமை sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  எனில்( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

/*
** xValue for sumint().
*/
நிலையான வெறுமை sumint_மதிப்பு(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  எனில்( pInt ) res = *pInt;
  sqlite3_result_int64(ctx, res);
}

நிலையான முழு SQLITE_TCLAPI test_create_sumint(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  முழு rc;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;

  rc = sqlite3_create_window_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumintStep, sumintFinal, sumint_மதிப்பு, sumintInverse,
      0
  );

  எனில்( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}

நிலையான முழு SQLITE_TCLAPI test_override_sum(
  வெறுமை * clientData,
  Tcl_Interp *interp,
  முழு objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  முழு rc;

  எனில்( objc!=2 ){
    Tcl_Wrong_இலக்கம்_Args(interp, 1, objv, "DB");
    திரும்பு TCL_ERROR;
  }
  எனில்( getDbPointer(interp, Tcl_GetString(objv[1]), &db) ) திரும்பு TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumintStep, sumintFinal
  );

  எனில்( rc!=SQLITE_OK ){
    Tcl_SetObjResult(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    திரும்பு TCL_ERROR;
  }
  திரும்பு TCL_OK;
}

முழு Sqlitetest_window_Init(Tcl_Interp *interp){
  நிலையான கட்டமைப்பு {
     எழுத்து *zName;
     Tcl_ObjCmdProc *xProc;
     முழு clientData;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_window, 0 },
     { "test_create_window_function_misuse", test_create_window_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_sum, 0 },
  };
  முழு i;
  க்கு(i=0; i<அளவு(aObjCmd)/அளவு(aObjCmd[0]); i++){
    ClientData c = (ClientData)SQLITE_INT_TO_PTR(aObjCmd[i].clientData);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  திரும்பு TCL_OK;
}
#endif
