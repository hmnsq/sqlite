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

#اگر_تعریف_شدہ SQLITE_TEST

#شامل "sqliteInt.h"
#شامل "tclsqlite.h"

خارجی صحیح get_Db_اشارہ_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجی ثابت حرف *sqlite3ErrName(صحیح);

قسمتعریف ساخت Test_کھڑکی Test_کھڑکی;
ساخت Test_کھڑکی {
  Tcl_Obj *x_قدم;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_قدر;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

قسمتعریف ساخت Test_کھڑکی_Ctx Test_کھڑکی_Ctx;
ساخت Test_کھڑکی_Ctx {
  Tcl_Obj *pVal;
};

ساکن خالی do_Test_کھڑکی_قدم(
  صحیح bInverse,
  sqlite3_context *ctx, 
  صحیح nArg, 
  sqlite3_قدر **apArg
){
  صحیح i;
  Test_کھڑکی *p = (Test_کھڑکی*)sqlite3_user_ڈیٹا(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->x_قدم);
  Test_کھڑکی_Ctx *pCtx = sqlite3_aggregate_context(ctx, حجم(Test_کھڑکی_Ctx));

  Tcl_Incr_Ref_گنتی(pEval);
  اگر( pCtx ){
    ثابت حرف *z_نتیجہ;
    صحیح rc;
    اگر( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }ورنہ{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    برائے(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((ثابت حرف*)sqlite3_قدر_متن(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    اگر( rc!=TCL_OK ){
      z_نتیجہ = Tcl_Get_String_نتیجہ(p->interp);
      sqlite3_نتیجہ_error(ctx, z_نتیجہ, -1);
    }ورنہ{
      اگر( pCtx->pVal ) Tcl_Decr_Ref_گنتی(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_Get_Obj_نتیجہ(p->interp));
      Tcl_Incr_Ref_گنتی(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_گنتی(pEval);
}

ساکن خالی do_Test_کھڑکی_Finalize(صحیح b_قدر, sqlite3_context *ctx){
  Test_کھڑکی *p = (Test_کھڑکی*)sqlite3_user_ڈیٹا(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_قدر ? p->x_قدر : p->xFinal);
  Test_کھڑکی_Ctx *pCtx = sqlite3_aggregate_context(ctx, حجم(Test_کھڑکی_Ctx));

  Tcl_Incr_Ref_گنتی(pEval);
  اگر( pCtx ){
    ثابت حرف *z_نتیجہ;
    صحیح rc;
    اگر( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }ورنہ{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    z_نتیجہ = Tcl_Get_String_نتیجہ(p->interp);
    اگر( rc!=TCL_OK ){
      sqlite3_نتیجہ_error(ctx, z_نتیجہ, -1);
    }ورنہ{
      sqlite3_نتیجہ_متن(ctx, z_نتیجہ, -1, SQLITE_TRANSIENT);
    }

    اگر( b_قدر==0 ){
      اگر( pCtx->pVal ) Tcl_Decr_Ref_گنتی(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_گنتی(pEval);
}

ساکن خالی test_کھڑکی_قدم(
  sqlite3_context *ctx, 
  صحیح nArg, 
  sqlite3_قدر **apArg
){
  do_Test_کھڑکی_قدم(0, ctx, nArg, apArg);
}
ساکن خالی test_کھڑکی_Inverse(
  sqlite3_context *ctx, 
  صحیح nArg, 
  sqlite3_قدر **apArg
){
  do_Test_کھڑکی_قدم(1, ctx, nArg, apArg);
}

ساکن خالی test_کھڑکی_Final(sqlite3_context *ctx){
  do_Test_کھڑکی_Finalize(0, ctx);
}
ساکن خالی test_کھڑکی_قدر(sqlite3_context *ctx){
  do_Test_کھڑکی_Finalize(1, ctx);
}

ساکن خالی test_کھڑکی_Destroy(خالی *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
ساکن صحیح SQLITE_TCLAPI test_create_کھڑکی(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  Test_کھڑکی *pNew;
  sqlite3 *db;
  ثابت حرف *zName;
  صحیح rc;

  اگر( objc!=7 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    واپس TCL_ERROR;
  }

  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (Test_کھڑکی*)ckalloc(حجم(Test_کھڑکی));
  memset(pNew, 0, حجم(Test_کھڑکی));
  pNew->x_قدم = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_قدر = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_گنتی(pNew->x_قدم);
  Tcl_Incr_Ref_گنتی(pNew->xFinal);
  Tcl_Incr_Ref_گنتی(pNew->x_قدر);
  Tcl_Incr_Ref_گنتی(pNew->xInverse);

  rc = sqlite3_create_کھڑکی_function(db, zName, -1, SQLITE_UTF8, (خالی*)pNew,
      test_کھڑکی_قدم, test_کھڑکی_Final, test_کھڑکی_قدر, test_کھڑکی_Inverse,
      test_کھڑکی_Destroy
  );
  اگر( rc!=SQLITE_OK ){
    Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    واپس TCL_ERROR;
  }

  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_create_کھڑکی_misuse(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحیح rc;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;

  rc = sqlite3_create_کھڑکی_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, test_کھڑکی_Final, test_کھڑکی_قدر, test_کھڑکی_Inverse,
      0
  );
  اگر( rc!=SQLITE_MISUSE ) جاؤ error;
  rc = sqlite3_create_کھڑکی_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_کھڑکی_قدم, 0, test_کھڑکی_قدر, test_کھڑکی_Inverse,
      0
  );
  اگر( rc!=SQLITE_MISUSE ) جاؤ error;
  rc = sqlite3_create_کھڑکی_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_کھڑکی_قدم, test_کھڑکی_Final, 0, test_کھڑکی_Inverse,
      0
  );
  اگر( rc!=SQLITE_MISUSE ) جاؤ error;
  rc = sqlite3_create_کھڑکی_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_کھڑکی_قدم, test_کھڑکی_Final, test_کھڑکی_قدر, 0,
      0
  );
  اگر( rc!=SQLITE_MISUSE ) جاؤ error;

  واپس TCL_OK;

 error:
  Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj("misuse test error", -1));
  واپس TCL_ERROR;
}

/*
** xStep for sumint().
*/
ساکن خالی sumint_قدم(
  sqlite3_context *ctx, 
  صحیح nArg, 
  sqlite3_قدر *apArg[]
){
  sqlite3_int64 *pInt;

  أكد( nArg==1 );
  اگر( sqlite3_قدر_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_نتیجہ_error(ctx, "invalid argument", -1);
    واپس;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, حجم(sqlite3_int64));
  اگر( pInt ){
    *pInt += sqlite3_قدر_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
ساکن خالی sumintInverse(
  sqlite3_context *ctx, 
  صحیح nArg, 
  sqlite3_قدر *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, حجم(sqlite3_int64));
  *pInt -= sqlite3_قدر_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
ساکن خالی sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  اگر( pInt ) res = *pInt;
  sqlite3_نتیجہ_int64(ctx, res);
}

/*
** xValue for sumint().
*/
ساکن خالی sumint_قدر(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  اگر( pInt ) res = *pInt;
  sqlite3_نتیجہ_int64(ctx, res);
}

ساکن صحیح SQLITE_TCLAPI test_create_sumint(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحیح rc;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;

  rc = sqlite3_create_کھڑکی_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumint_قدم, sumintFinal, sumint_قدر, sumintInverse,
      0
  );

  اگر( rc!=SQLITE_OK ){
    Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}

ساکن صحیح SQLITE_TCLAPI test_override_مجموع(
  خالی * client_ڈیٹا,
  Tcl_Interp *interp,
  صحیح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحیح rc;

  اگر( objc!=2 ){
    Tcl_Wrong_عدد_دلائل(interp, 1, objv, "DB");
    واپس TCL_ERROR;
  }
  اگر( get_Db_اشارہ_گر(interp, Tcl_GetString(objv[1]), &db) ) واپس TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumint_قدم, sumintFinal
  );

  اگر( rc!=SQLITE_OK ){
    Tcl_سیٹ_Obj_نتیجہ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    واپس TCL_ERROR;
  }
  واپس TCL_OK;
}

صحیح Sqlitetest_کھڑکی_Init(Tcl_Interp *interp){
  ساکن ساخت {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     صحیح client_ڈیٹا;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_کھڑکی, 0 },
     { "test_create_window_function_misuse", test_create_کھڑکی_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_مجموع, 0 },
  };
  صحیح i;
  برائے(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Client_ڈیٹا c = (Client_ڈیٹا)SQLITE_INT_مقصد_PTR(aObjCmd[i].client_ڈیٹا);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  واپس TCL_OK;
}
#اگر_ختم
