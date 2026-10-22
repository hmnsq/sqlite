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

#اگر_تعریف_شده SQLITE_TEST

#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"

خارجي صحيح get_Db_اشاره_گر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجي ثابت حرف *sqlite3ErrName(صحيح);

تعريفنوع بنية Test_پنجره Test_پنجره;
بنية Test_پنجره {
  Tcl_Obj *x_گام;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_مقدار;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

تعريفنوع بنية Test_پنجره_Ctx Test_پنجره_Ctx;
بنية Test_پنجره_Ctx {
  Tcl_Obj *pVal;
};

ساكن فراغ do_Test_پنجره_گام(
  صحيح bInverse,
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_مقدار **apArg
){
  صحيح i;
  Test_پنجره *p = (Test_پنجره*)sqlite3_user_داده(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->x_گام);
  Test_پنجره_Ctx *pCtx = sqlite3_aggregate_context(ctx, حجم(Test_پنجره_Ctx));

  Tcl_Incr_Ref_شمار(pEval);
  إذا( pCtx ){
    ثابت حرف *z_نتیجه;
    صحيح rc;
    إذا( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }وإلا{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    لكل(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((ثابت حرف*)sqlite3_مقدار_متن(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    إذا( rc!=TCL_OK ){
      z_نتیجه = Tcl_Get_String_نتیجه(p->interp);
      sqlite3_نتیجه_error(ctx, z_نتیجه, -1);
    }وإلا{
      إذا( pCtx->pVal ) Tcl_Decr_Ref_شمار(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_Get_Obj_نتیجه(p->interp));
      Tcl_Incr_Ref_شمار(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_شمار(pEval);
}

ساكن فراغ do_Test_پنجره_Finalize(صحيح b_مقدار, sqlite3_context *ctx){
  Test_پنجره *p = (Test_پنجره*)sqlite3_user_داده(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_مقدار ? p->x_مقدار : p->xFinal);
  Test_پنجره_Ctx *pCtx = sqlite3_aggregate_context(ctx, حجم(Test_پنجره_Ctx));

  Tcl_Incr_Ref_شمار(pEval);
  إذا( pCtx ){
    ثابت حرف *z_نتیجه;
    صحيح rc;
    إذا( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }وإلا{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    z_نتیجه = Tcl_Get_String_نتیجه(p->interp);
    إذا( rc!=TCL_OK ){
      sqlite3_نتیجه_error(ctx, z_نتیجه, -1);
    }وإلا{
      sqlite3_نتیجه_متن(ctx, z_نتیجه, -1, SQLITE_TRANSIENT);
    }

    إذا( b_مقدار==0 ){
      إذا( pCtx->pVal ) Tcl_Decr_Ref_شمار(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_شمار(pEval);
}

ساكن فراغ test_پنجره_گام(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_مقدار **apArg
){
  do_Test_پنجره_گام(0, ctx, nArg, apArg);
}
ساكن فراغ test_پنجره_Inverse(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_مقدار **apArg
){
  do_Test_پنجره_گام(1, ctx, nArg, apArg);
}

ساكن فراغ test_پنجره_Final(sqlite3_context *ctx){
  do_Test_پنجره_Finalize(0, ctx);
}
ساكن فراغ test_پنجره_مقدار(sqlite3_context *ctx){
  do_Test_پنجره_Finalize(1, ctx);
}

ساكن فراغ test_پنجره_Destroy(فراغ *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
ساكن صحيح SQLITE_TCLAPI test_create_پنجره(
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  Test_پنجره *pNew;
  sqlite3 *db;
  ثابت حرف *zName;
  صحيح rc;

  إذا( objc!=7 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    أرجع TCL_ERROR;
  }

  إذا( get_Db_اشاره_گر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (Test_پنجره*)ckalloc(حجم(Test_پنجره));
  memset(pNew, 0, حجم(Test_پنجره));
  pNew->x_گام = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_مقدار = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_شمار(pNew->x_گام);
  Tcl_Incr_Ref_شمار(pNew->xFinal);
  Tcl_Incr_Ref_شمار(pNew->x_مقدار);
  Tcl_Incr_Ref_شمار(pNew->xInverse);

  rc = sqlite3_create_پنجره_function(db, zName, -1, SQLITE_UTF8, (فراغ*)pNew,
      test_پنجره_گام, test_پنجره_Final, test_پنجره_مقدار, test_پنجره_Inverse,
      test_پنجره_Destroy
  );
  إذا( rc!=SQLITE_OK ){
    Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    أرجع TCL_ERROR;
  }

  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_create_پنجره_misuse(
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحيح rc;

  إذا( objc!=2 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشاره_گر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;

  rc = sqlite3_create_پنجره_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, test_پنجره_Final, test_پنجره_مقدار, test_پنجره_Inverse,
      0
  );
  إذا( rc!=SQLITE_MISUSE ) اذهب error;
  rc = sqlite3_create_پنجره_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_پنجره_گام, 0, test_پنجره_مقدار, test_پنجره_Inverse,
      0
  );
  إذا( rc!=SQLITE_MISUSE ) اذهب error;
  rc = sqlite3_create_پنجره_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_پنجره_گام, test_پنجره_Final, 0, test_پنجره_Inverse,
      0
  );
  إذا( rc!=SQLITE_MISUSE ) اذهب error;
  rc = sqlite3_create_پنجره_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_پنجره_گام, test_پنجره_Final, test_پنجره_مقدار, 0,
      0
  );
  إذا( rc!=SQLITE_MISUSE ) اذهب error;

  أرجع TCL_OK;

 error:
  Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewStringObj("misuse test error", -1));
  أرجع TCL_ERROR;
}

/*
** xStep for sumint().
*/
ساكن فراغ sumint_گام(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_مقدار *apArg[]
){
  sqlite3_int64 *pInt;

  أكد( nArg==1 );
  إذا( sqlite3_مقدار_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_نتیجه_error(ctx, "invalid argument", -1);
    أرجع;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, حجم(sqlite3_int64));
  إذا( pInt ){
    *pInt += sqlite3_مقدار_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
ساكن فراغ sumintInverse(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_مقدار *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, حجم(sqlite3_int64));
  *pInt -= sqlite3_مقدار_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
ساكن فراغ sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  إذا( pInt ) res = *pInt;
  sqlite3_نتیجه_int64(ctx, res);
}

/*
** xValue for sumint().
*/
ساكن فراغ sumint_مقدار(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  إذا( pInt ) res = *pInt;
  sqlite3_نتیجه_int64(ctx, res);
}

ساكن صحيح SQLITE_TCLAPI test_create_sumint(
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحيح rc;

  إذا( objc!=2 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشاره_گر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;

  rc = sqlite3_create_پنجره_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumint_گام, sumintFinal, sumint_مقدار, sumintInverse,
      0
  );

  إذا( rc!=SQLITE_OK ){
    Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_override_جمع(
  فراغ * client_داده,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحيح rc;

  إذا( objc!=2 ){
    Tcl_Wrong_رقم_آرگومانها(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشاره_گر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumint_گام, sumintFinal
  );

  إذا( rc!=SQLITE_OK ){
    Tcl_مجموعه_Obj_نتیجه(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

صحيح Sqlitetest_پنجره_Init(Tcl_Interp *interp){
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     صحيح client_داده;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_پنجره, 0 },
     { "test_create_window_function_misuse", test_create_پنجره_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_جمع, 0 },
  };
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Client_داده c = (Client_داده)SQLITE_INT_مقصد_PTR(aObjCmd[i].client_داده);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  أرجع TCL_OK;
}
#پایان_اگر
