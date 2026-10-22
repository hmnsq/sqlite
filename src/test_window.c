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

#که_تعریف_شوی SQLITE_TEST

#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"

خارجي صحيح get_Db_اشاره_ګر(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجي ثابت حرف *sqlite3ErrName(صحيح);

تعريفنوع بنية Test_کړکۍ Test_کړکۍ;
بنية Test_کړکۍ {
  Tcl_Obj *x_ګام;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_ارزښت;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

تعريفنوع بنية Test_کړکۍ_Ctx Test_کړکۍ_Ctx;
بنية Test_کړکۍ_Ctx {
  Tcl_Obj *pVal;
};

ساكن فراغ do_Test_کړکۍ_ګام(
  صحيح bInverse,
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_ارزښت **apArg
){
  صحيح i;
  Test_کړکۍ *p = (Test_کړکۍ*)sqlite3_user_معلومات(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->x_ګام);
  Test_کړکۍ_Ctx *pCtx = sqlite3_aggregate_context(ctx, حجم(Test_کړکۍ_Ctx));

  Tcl_Incr_Ref_شمېر(pEval);
  إذا( pCtx ){
    ثابت حرف *z_پایله;
    صحيح rc;
    إذا( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }وإلا{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    لكل(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((ثابت حرف*)sqlite3_ارزښت_متن(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    إذا( rc!=TCL_OK ){
      z_پایله = Tcl_Get_String_پایله(p->interp);
      sqlite3_پایله_error(ctx, z_پایله, -1);
    }وإلا{
      إذا( pCtx->pVal ) Tcl_Decr_Ref_شمېر(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_Get_Obj_پایله(p->interp));
      Tcl_Incr_Ref_شمېر(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_شمېر(pEval);
}

ساكن فراغ do_Test_کړکۍ_Finalize(صحيح b_ارزښت, sqlite3_context *ctx){
  Test_کړکۍ *p = (Test_کړکۍ*)sqlite3_user_معلومات(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_ارزښت ? p->x_ارزښت : p->xFinal);
  Test_کړکۍ_Ctx *pCtx = sqlite3_aggregate_context(ctx, حجم(Test_کړکۍ_Ctx));

  Tcl_Incr_Ref_شمېر(pEval);
  إذا( pCtx ){
    ثابت حرف *z_پایله;
    صحيح rc;
    إذا( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }وإلا{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    z_پایله = Tcl_Get_String_پایله(p->interp);
    إذا( rc!=TCL_OK ){
      sqlite3_پایله_error(ctx, z_پایله, -1);
    }وإلا{
      sqlite3_پایله_متن(ctx, z_پایله, -1, SQLITE_TRANSIENT);
    }

    إذا( b_ارزښت==0 ){
      إذا( pCtx->pVal ) Tcl_Decr_Ref_شمېر(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_شمېر(pEval);
}

ساكن فراغ test_کړکۍ_ګام(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_ارزښت **apArg
){
  do_Test_کړکۍ_ګام(0, ctx, nArg, apArg);
}
ساكن فراغ test_کړکۍ_Inverse(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_ارزښت **apArg
){
  do_Test_کړکۍ_ګام(1, ctx, nArg, apArg);
}

ساكن فراغ test_کړکۍ_Final(sqlite3_context *ctx){
  do_Test_کړکۍ_Finalize(0, ctx);
}
ساكن فراغ test_کړکۍ_ارزښت(sqlite3_context *ctx){
  do_Test_کړکۍ_Finalize(1, ctx);
}

ساكن فراغ test_کړکۍ_Destroy(فراغ *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
ساكن صحيح SQLITE_TCLAPI test_create_کړکۍ(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  Test_کړکۍ *pNew;
  sqlite3 *db;
  ثابت حرف *zName;
  صحيح rc;

  إذا( objc!=7 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    أرجع TCL_ERROR;
  }

  إذا( get_Db_اشاره_ګر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (Test_کړکۍ*)ckalloc(حجم(Test_کړکۍ));
  memset(pNew, 0, حجم(Test_کړکۍ));
  pNew->x_ګام = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_ارزښت = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_شمېر(pNew->x_ګام);
  Tcl_Incr_Ref_شمېر(pNew->xFinal);
  Tcl_Incr_Ref_شمېر(pNew->x_ارزښت);
  Tcl_Incr_Ref_شمېر(pNew->xInverse);

  rc = sqlite3_create_کړکۍ_function(db, zName, -1, SQLITE_UTF8, (فراغ*)pNew,
      test_کړکۍ_ګام, test_کړکۍ_Final, test_کړکۍ_ارزښت, test_کړکۍ_Inverse,
      test_کړکۍ_Destroy
  );
  إذا( rc!=SQLITE_OK ){
    Tcl_سېټ_Obj_پایله(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    أرجع TCL_ERROR;
  }

  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_create_کړکۍ_misuse(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحيح rc;

  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشاره_ګر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;

  rc = sqlite3_create_کړکۍ_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, test_کړکۍ_Final, test_کړکۍ_ارزښت, test_کړکۍ_Inverse,
      0
  );
  إذا( rc!=SQLITE_MISUSE ) اذهب error;
  rc = sqlite3_create_کړکۍ_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_کړکۍ_ګام, 0, test_کړکۍ_ارزښت, test_کړکۍ_Inverse,
      0
  );
  إذا( rc!=SQLITE_MISUSE ) اذهب error;
  rc = sqlite3_create_کړکۍ_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_کړکۍ_ګام, test_کړکۍ_Final, 0, test_کړکۍ_Inverse,
      0
  );
  إذا( rc!=SQLITE_MISUSE ) اذهب error;
  rc = sqlite3_create_کړکۍ_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_کړکۍ_ګام, test_کړکۍ_Final, test_کړکۍ_ارزښت, 0,
      0
  );
  إذا( rc!=SQLITE_MISUSE ) اذهب error;

  أرجع TCL_OK;

 error:
  Tcl_سېټ_Obj_پایله(interp, Tcl_NewStringObj("misuse test error", -1));
  أرجع TCL_ERROR;
}

/*
** xStep for sumint().
*/
ساكن فراغ sumint_ګام(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_ارزښت *apArg[]
){
  sqlite3_int64 *pInt;

  أكد( nArg==1 );
  إذا( sqlite3_ارزښت_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_پایله_error(ctx, "invalid argument", -1);
    أرجع;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, حجم(sqlite3_int64));
  إذا( pInt ){
    *pInt += sqlite3_ارزښت_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
ساكن فراغ sumintInverse(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_ارزښت *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, حجم(sqlite3_int64));
  *pInt -= sqlite3_ارزښت_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
ساكن فراغ sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  إذا( pInt ) res = *pInt;
  sqlite3_پایله_int64(ctx, res);
}

/*
** xValue for sumint().
*/
ساكن فراغ sumint_ارزښت(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  إذا( pInt ) res = *pInt;
  sqlite3_پایله_int64(ctx, res);
}

ساكن صحيح SQLITE_TCLAPI test_create_sumint(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحيح rc;

  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشاره_ګر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;

  rc = sqlite3_create_کړکۍ_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumint_ګام, sumintFinal, sumint_ارزښت, sumintInverse,
      0
  );

  إذا( rc!=SQLITE_OK ){
    Tcl_سېټ_Obj_پایله(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_override_مجموعه(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحيح rc;

  إذا( objc!=2 ){
    Tcl_Wrong_عدد_دلیلونه(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشاره_ګر(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumint_ګام, sumintFinal
  );

  إذا( rc!=SQLITE_OK ){
    Tcl_سېټ_Obj_پایله(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

صحيح Sqlitetest_کړکۍ_Init(Tcl_Interp *interp){
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     صحيح client_معلومات;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_کړکۍ, 0 },
     { "test_create_window_function_misuse", test_create_کړکۍ_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_مجموعه, 0 },
  };
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Client_معلومات c = (Client_معلومات)SQLITE_INT_منزل_PTR(aObjCmd[i].client_معلومات);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  أرجع TCL_OK;
}
#د_که_پای
