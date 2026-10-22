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

#جيڪڏهن_وصف_ٿيل SQLITE_TEST

#ضمن "sqliteInt.h"
#ضمن "tclsqlite.h"

خارجي صحيح get_Db_اشارو(Tcl_Interp *interp, ثابت حرف *zA, sqlite3 **ppDb);
خارجي ثابت حرف *sqlite3ErrName(صحيح);

تعريفنوع بنية Test_دري Test_دري;
بنية Test_دري {
  Tcl_Obj *x_قدم;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_قدر;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

تعريفنوع بنية Test_دري_Ctx Test_دري_Ctx;
بنية Test_دري_Ctx {
  Tcl_Obj *pVal;
};

ساكن فراغ do_Test_دري_قدم(
  صحيح bInverse,
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_قدر **apArg
){
  صحيح i;
  Test_دري *p = (Test_دري*)sqlite3_user_معلومات(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->x_قدم);
  Test_دري_Ctx *pCtx = sqlite3_aggregate_context(ctx, حجم(Test_دري_Ctx));

  Tcl_Incr_Ref_ڳڻپ(pEval);
  إذا( pCtx ){
    ثابت حرف *z_نتيجو;
    صحيح rc;
    إذا( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }وإلا{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    لكل(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((ثابت حرف*)sqlite3_قدر_متن(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    إذا( rc!=TCL_OK ){
      z_نتيجو = Tcl_Get_String_نتيجو(p->interp);
      sqlite3_نتيجو_error(ctx, z_نتيجو, -1);
    }وإلا{
      إذا( pCtx->pVal ) Tcl_Decr_Ref_ڳڻپ(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_Get_Obj_نتيجو(p->interp));
      Tcl_Incr_Ref_ڳڻپ(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_ڳڻپ(pEval);
}

ساكن فراغ do_Test_دري_Finalize(صحيح b_قدر, sqlite3_context *ctx){
  Test_دري *p = (Test_دري*)sqlite3_user_معلومات(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_قدر ? p->x_قدر : p->xFinal);
  Test_دري_Ctx *pCtx = sqlite3_aggregate_context(ctx, حجم(Test_دري_Ctx));

  Tcl_Incr_Ref_ڳڻپ(pEval);
  إذا( pCtx ){
    ثابت حرف *z_نتيجو;
    صحيح rc;
    إذا( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }وإلا{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    z_نتيجو = Tcl_Get_String_نتيجو(p->interp);
    إذا( rc!=TCL_OK ){
      sqlite3_نتيجو_error(ctx, z_نتيجو, -1);
    }وإلا{
      sqlite3_نتيجو_متن(ctx, z_نتيجو, -1, SQLITE_TRANSIENT);
    }

    إذا( b_قدر==0 ){
      إذا( pCtx->pVal ) Tcl_Decr_Ref_ڳڻپ(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_ڳڻپ(pEval);
}

ساكن فراغ test_دري_قدم(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_قدر **apArg
){
  do_Test_دري_قدم(0, ctx, nArg, apArg);
}
ساكن فراغ test_دري_Inverse(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_قدر **apArg
){
  do_Test_دري_قدم(1, ctx, nArg, apArg);
}

ساكن فراغ test_دري_Final(sqlite3_context *ctx){
  do_Test_دري_Finalize(0, ctx);
}
ساكن فراغ test_دري_قدر(sqlite3_context *ctx){
  do_Test_دري_Finalize(1, ctx);
}

ساكن فراغ test_دري_Destroy(فراغ *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
ساكن صحيح SQLITE_TCLAPI test_create_دري(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  Test_دري *pNew;
  sqlite3 *db;
  ثابت حرف *zName;
  صحيح rc;

  إذا( objc!=7 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    أرجع TCL_ERROR;
  }

  إذا( get_Db_اشارو(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (Test_دري*)ckalloc(حجم(Test_دري));
  memset(pNew, 0, حجم(Test_دري));
  pNew->x_قدم = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_قدر = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_ڳڻپ(pNew->x_قدم);
  Tcl_Incr_Ref_ڳڻپ(pNew->xFinal);
  Tcl_Incr_Ref_ڳڻپ(pNew->x_قدر);
  Tcl_Incr_Ref_ڳڻپ(pNew->xInverse);

  rc = sqlite3_create_دري_function(db, zName, -1, SQLITE_UTF8, (فراغ*)pNew,
      test_دري_قدم, test_دري_Final, test_دري_قدر, test_دري_Inverse,
      test_دري_Destroy
  );
  إذا( rc!=SQLITE_OK ){
    Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    أرجع TCL_ERROR;
  }

  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_create_دري_misuse(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحيح rc;

  إذا( objc!=2 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشارو(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;

  rc = sqlite3_create_دري_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, test_دري_Final, test_دري_قدر, test_دري_Inverse,
      0
  );
  إذا( rc!=SQLITE_MISUSE ) اذهب error;
  rc = sqlite3_create_دري_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_دري_قدم, 0, test_دري_قدر, test_دري_Inverse,
      0
  );
  إذا( rc!=SQLITE_MISUSE ) اذهب error;
  rc = sqlite3_create_دري_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_دري_قدم, test_دري_Final, 0, test_دري_Inverse,
      0
  );
  إذا( rc!=SQLITE_MISUSE ) اذهب error;
  rc = sqlite3_create_دري_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_دري_قدم, test_دري_Final, test_دري_قدر, 0,
      0
  );
  إذا( rc!=SQLITE_MISUSE ) اذهب error;

  أرجع TCL_OK;

 error:
  Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewStringObj("misuse test error", -1));
  أرجع TCL_ERROR;
}

/*
** xStep for sumint().
*/
ساكن فراغ sumint_قدم(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_قدر *apArg[]
){
  sqlite3_int64 *pInt;

  أكد( nArg==1 );
  إذا( sqlite3_قدر_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_نتيجو_error(ctx, "invalid argument", -1);
    أرجع;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, حجم(sqlite3_int64));
  إذا( pInt ){
    *pInt += sqlite3_قدر_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
ساكن فراغ sumintInverse(
  sqlite3_context *ctx, 
  صحيح nArg, 
  sqlite3_قدر *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, حجم(sqlite3_int64));
  *pInt -= sqlite3_قدر_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
ساكن فراغ sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  إذا( pInt ) res = *pInt;
  sqlite3_نتيجو_int64(ctx, res);
}

/*
** xValue for sumint().
*/
ساكن فراغ sumint_قدر(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  إذا( pInt ) res = *pInt;
  sqlite3_نتيجو_int64(ctx, res);
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
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشارو(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;

  rc = sqlite3_create_دري_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumint_قدم, sumintFinal, sumint_قدر, sumintInverse,
      0
  );

  إذا( rc!=SQLITE_OK ){
    Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

ساكن صحيح SQLITE_TCLAPI test_override_جوڙ(
  فراغ * client_معلومات,
  Tcl_Interp *interp,
  صحيح objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  صحيح rc;

  إذا( objc!=2 ){
    Tcl_Wrong_انگ_دليلون(interp, 1, objv, "DB");
    أرجع TCL_ERROR;
  }
  إذا( get_Db_اشارو(interp, Tcl_GetString(objv[1]), &db) ) أرجع TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumint_قدم, sumintFinal
  );

  إذا( rc!=SQLITE_OK ){
    Tcl_سيٽ_Obj_نتيجو(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    أرجع TCL_ERROR;
  }
  أرجع TCL_OK;
}

صحيح Sqlitetest_دري_Init(Tcl_Interp *interp){
  ساكن بنية {
     حرف *zName;
     Tcl_ObjCmdProc *xProc;
     صحيح client_معلومات;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_دري, 0 },
     { "test_create_window_function_misuse", test_create_دري_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_جوڙ, 0 },
  };
  صحيح i;
  لكل(i=0; i<حجم(aObjCmd)/حجم(aObjCmd[0]); i++){
    Client_معلومات c = (Client_معلومات)SQLITE_INT_منزل_PTR(aObjCmd[i].client_معلومات);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  أرجع TCL_OK;
}
#جيڪڏهن_پورو
