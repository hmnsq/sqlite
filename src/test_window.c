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

#ਅਨ੍ਤਰ੍ਭਾਵ "sqlite3.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_TEST

#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "tclsqlite.h"

ਬਾਹਰੀ ਪੂਰਨ get_Db_ਸੂਚਕ(Tcl_Interp *interp, ਅਚਲ ਅੱਖਰ *zA, sqlite3 **ppDb);
ਬਾਹਰੀ ਅਚਲ ਅੱਖਰ *sqlite3ErrName(ਪੂਰਨ);

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Test_ਖਿੜਕੀ Test_ਖਿੜਕੀ;
ਬਣਤਰ Test_ਖਿੜਕੀ {
  Tcl_Obj *x_ਕਦਮ;
  Tcl_Obj *xFinal;
  Tcl_Obj *x_ਮੁੱਲ;
  Tcl_Obj *xInverse;
  Tcl_Interp *interp;
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Test_ਖਿੜਕੀ_Ctx Test_ਖਿੜਕੀ_Ctx;
ਬਣਤਰ Test_ਖਿੜਕੀ_Ctx {
  Tcl_Obj *pVal;
};

ਸਥਿਰ ਖਾਲੀ do_Test_ਖਿੜਕੀ_ਕਦਮ(
  ਪੂਰਨ bInverse,
  sqlite3_context *ctx, 
  ਪੂਰਨ nArg, 
  sqlite3_ਮੁੱਲ **apArg
){
  ਪੂਰਨ i;
  Test_ਖਿੜਕੀ *p = (Test_ਖਿੜਕੀ*)sqlite3_user_ਅੰਕੜੇ(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(bInverse ? p->xInverse : p->x_ਕਦਮ);
  Test_ਖਿੜਕੀ_Ctx *pCtx = sqlite3_aggregate_context(ctx, ਆਕਾਰ(Test_ਖਿੜਕੀ_Ctx));

  Tcl_Incr_Ref_ਗਿਣਤੀ(pEval);
  ਜੇ( pCtx ){
    ਅਚਲ ਅੱਖਰ *z_ਨਤੀਜਾ;
    ਪੂਰਨ rc;
    ਜੇ( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }ਨਹੀਂਤਾਂ{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }
    ਲਈ(i=0; i<nArg; i++){
      Tcl_Obj *pArg;
      pArg = Tcl_NewStringObj((ਅਚਲ ਅੱਖਰ*)sqlite3_ਮੁੱਲ_ਪਾਠ(apArg[i]), -1);
      Tcl_ListObjAppendElement(p->interp, pEval, pArg);
    }
    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    ਜੇ( rc!=TCL_OK ){
      z_ਨਤੀਜਾ = Tcl_Get_String_ਨਤੀਜਾ(p->interp);
      sqlite3_ਨਤੀਜਾ_error(ctx, z_ਨਤੀਜਾ, -1);
    }ਨਹੀਂਤਾਂ{
      ਜੇ( pCtx->pVal ) Tcl_Decr_Ref_ਗਿਣਤੀ(pCtx->pVal);
      pCtx->pVal = Tcl_DuplicateObj(Tcl_Get_Obj_ਨਤੀਜਾ(p->interp));
      Tcl_Incr_Ref_ਗਿਣਤੀ(pCtx->pVal);
    }
  }
  Tcl_Decr_Ref_ਗਿਣਤੀ(pEval);
}

ਸਥਿਰ ਖਾਲੀ do_Test_ਖਿੜਕੀ_Finalize(ਪੂਰਨ b_ਮੁੱਲ, sqlite3_context *ctx){
  Test_ਖਿੜਕੀ *p = (Test_ਖਿੜਕੀ*)sqlite3_user_ਅੰਕੜੇ(ctx);
  Tcl_Obj *pEval = Tcl_DuplicateObj(b_ਮੁੱਲ ? p->x_ਮੁੱਲ : p->xFinal);
  Test_ਖਿੜਕੀ_Ctx *pCtx = sqlite3_aggregate_context(ctx, ਆਕਾਰ(Test_ਖਿੜਕੀ_Ctx));

  Tcl_Incr_Ref_ਗਿਣਤੀ(pEval);
  ਜੇ( pCtx ){
    ਅਚਲ ਅੱਖਰ *z_ਨਤੀਜਾ;
    ਪੂਰਨ rc;
    ਜੇ( pCtx->pVal ){
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_DuplicateObj(pCtx->pVal));
    }ਨਹੀਂਤਾਂ{
      Tcl_ListObjAppendElement(p->interp, pEval, Tcl_NewStringObj("", -1));
    }

    rc = Tcl_EvalObjEx(p->interp, pEval, TCL_EVAL_GLOBAL);
    z_ਨਤੀਜਾ = Tcl_Get_String_ਨਤੀਜਾ(p->interp);
    ਜੇ( rc!=TCL_OK ){
      sqlite3_ਨਤੀਜਾ_error(ctx, z_ਨਤੀਜਾ, -1);
    }ਨਹੀਂਤਾਂ{
      sqlite3_ਨਤੀਜਾ_ਪਾਠ(ctx, z_ਨਤੀਜਾ, -1, SQLITE_TRANSIENT);
    }

    ਜੇ( b_ਮੁੱਲ==0 ){
      ਜੇ( pCtx->pVal ) Tcl_Decr_Ref_ਗਿਣਤੀ(pCtx->pVal);
      pCtx->pVal = 0;
    }
  }
  Tcl_Decr_Ref_ਗਿਣਤੀ(pEval);
}

ਸਥਿਰ ਖਾਲੀ test_ਖਿੜਕੀ_ਕਦਮ(
  sqlite3_context *ctx, 
  ਪੂਰਨ nArg, 
  sqlite3_ਮੁੱਲ **apArg
){
  do_Test_ਖਿੜਕੀ_ਕਦਮ(0, ctx, nArg, apArg);
}
ਸਥਿਰ ਖਾਲੀ test_ਖਿੜਕੀ_Inverse(
  sqlite3_context *ctx, 
  ਪੂਰਨ nArg, 
  sqlite3_ਮੁੱਲ **apArg
){
  do_Test_ਖਿੜਕੀ_ਕਦਮ(1, ctx, nArg, apArg);
}

ਸਥਿਰ ਖਾਲੀ test_ਖਿੜਕੀ_Final(sqlite3_context *ctx){
  do_Test_ਖਿੜਕੀ_Finalize(0, ctx);
}
ਸਥਿਰ ਖਾਲੀ test_ਖਿੜਕੀ_ਮੁੱਲ(sqlite3_context *ctx){
  do_Test_ਖਿੜਕੀ_Finalize(1, ctx);
}

ਸਥਿਰ ਖਾਲੀ test_ਖਿੜਕੀ_Destroy(ਖਾਲੀ *pCtx){
  ckfree(pCtx);
}

/*
** Usage: sqlite3_create_window_function DB NAME XSTEP XFINAL XVALUE XINVERSE
*/
ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_create_ਖਿੜਕੀ(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  Test_ਖਿੜਕੀ *pNew;
  sqlite3 *db;
  ਅਚਲ ਅੱਖਰ *zName;
  ਪੂਰਨ rc;

  ਜੇ( objc!=7 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB NAME XSTEP XFINAL XVALUE XINVERSE");
    ਵਾਪਸ TCL_ERROR;
  }

  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;
  zName = Tcl_GetString(objv[2]);
  pNew = (Test_ਖਿੜਕੀ*)ckalloc(ਆਕਾਰ(Test_ਖਿੜਕੀ));
  memset(pNew, 0, ਆਕਾਰ(Test_ਖਿੜਕੀ));
  pNew->x_ਕਦਮ = Tcl_DuplicateObj(objv[3]);
  pNew->xFinal = Tcl_DuplicateObj(objv[4]);
  pNew->x_ਮੁੱਲ = Tcl_DuplicateObj(objv[5]);
  pNew->xInverse = Tcl_DuplicateObj(objv[6]);
  pNew->interp = interp;

  Tcl_Incr_Ref_ਗਿਣਤੀ(pNew->x_ਕਦਮ);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pNew->xFinal);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pNew->x_ਮੁੱਲ);
  Tcl_Incr_Ref_ਗਿਣਤੀ(pNew->xInverse);

  rc = sqlite3_create_ਖਿੜਕੀ_function(db, zName, -1, SQLITE_UTF8, (ਖਾਲੀ*)pNew,
      test_ਖਿੜਕੀ_ਕਦਮ, test_ਖਿੜਕੀ_Final, test_ਖਿੜਕੀ_ਮੁੱਲ, test_ਖਿੜਕੀ_Inverse,
      test_ਖਿੜਕੀ_Destroy
  );
  ਜੇ( rc!=SQLITE_OK ){
    Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ਵਾਪਸ TCL_ERROR;
  }

  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_create_ਖਿੜਕੀ_misuse(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ਪੂਰਨ rc;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;

  rc = sqlite3_create_ਖਿੜਕੀ_function(db, "fff", -1, SQLITE_UTF8, 0,
      0, test_ਖਿੜਕੀ_Final, test_ਖਿੜਕੀ_ਮੁੱਲ, test_ਖਿੜਕੀ_Inverse,
      0
  );
  ਜੇ( rc!=SQLITE_MISUSE ) ਜਾਓ error;
  rc = sqlite3_create_ਖਿੜਕੀ_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_ਖਿੜਕੀ_ਕਦਮ, 0, test_ਖਿੜਕੀ_ਮੁੱਲ, test_ਖਿੜਕੀ_Inverse,
      0
  );
  ਜੇ( rc!=SQLITE_MISUSE ) ਜਾਓ error;
  rc = sqlite3_create_ਖਿੜਕੀ_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_ਖਿੜਕੀ_ਕਦਮ, test_ਖਿੜਕੀ_Final, 0, test_ਖਿੜਕੀ_Inverse,
      0
  );
  ਜੇ( rc!=SQLITE_MISUSE ) ਜਾਓ error;
  rc = sqlite3_create_ਖਿੜਕੀ_function(db, "fff", -1, SQLITE_UTF8, 0,
      test_ਖਿੜਕੀ_ਕਦਮ, test_ਖਿੜਕੀ_Final, test_ਖਿੜਕੀ_ਮੁੱਲ, 0,
      0
  );
  ਜੇ( rc!=SQLITE_MISUSE ) ਜਾਓ error;

  ਵਾਪਸ TCL_OK;

 error:
  Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj("misuse test error", -1));
  ਵਾਪਸ TCL_ERROR;
}

/*
** xStep for sumint().
*/
ਸਥਿਰ ਖਾਲੀ sumint_ਕਦਮ(
  sqlite3_context *ctx, 
  ਪੂਰਨ nArg, 
  sqlite3_ਮੁੱਲ *apArg[]
){
  sqlite3_int64 *pInt;

  ਪ੍ਰਤਿਜ੍ਞਾ( nArg==1 );
  ਜੇ( sqlite3_ਮੁੱਲ_type(apArg[0])!=SQLITE_INTEGER ){
    sqlite3_ਨਤੀਜਾ_error(ctx, "invalid argument", -1);
    ਵਾਪਸ;
  }
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, ਆਕਾਰ(sqlite3_int64));
  ਜੇ( pInt ){
    *pInt += sqlite3_ਮੁੱਲ_int64(apArg[0]);
  }
}

/*
** xInverse for sumint().
*/
ਸਥਿਰ ਖਾਲੀ sumintInverse(
  sqlite3_context *ctx, 
  ਪੂਰਨ nArg, 
  sqlite3_ਮੁੱਲ *apArg[]
){
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, ਆਕਾਰ(sqlite3_int64));
  *pInt -= sqlite3_ਮੁੱਲ_int64(apArg[0]);
}

/*
** xFinal for sumint().
*/
ਸਥਿਰ ਖਾਲੀ sumintFinal(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  ਜੇ( pInt ) res = *pInt;
  sqlite3_ਨਤੀਜਾ_int64(ctx, res);
}

/*
** xValue for sumint().
*/
ਸਥਿਰ ਖਾਲੀ sumint_ਮੁੱਲ(sqlite3_context *ctx){
  sqlite3_int64 res = 0;
  sqlite3_int64 *pInt;
  pInt = (sqlite3_int64*)sqlite3_aggregate_context(ctx, 0);
  ਜੇ( pInt ) res = *pInt;
  sqlite3_ਨਤੀਜਾ_int64(ctx, res);
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_create_sumint(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ਪੂਰਨ rc;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;

  rc = sqlite3_create_ਖਿੜਕੀ_function(db, "sumint", 1, SQLITE_UTF8, 0,
      sumint_ਕਦਮ, sumintFinal, sumint_ਮੁੱਲ, sumintInverse,
      0
  );

  ਜੇ( rc!=SQLITE_OK ){
    Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}

ਸਥਿਰ ਪੂਰਨ SQLITE_TCLAPI test_override_ਜੋੜ(
  ਖਾਲੀ * client_ਅੰਕੜੇ,
  Tcl_Interp *interp,
  ਪੂਰਨ objc,
  Tcl_Obj *CONST objv[]
){
  sqlite3 *db;
  ਪੂਰਨ rc;

  ਜੇ( objc!=2 ){
    Tcl_Wrong_ਅੰਕ_ਦਲੀਲਾਂ(interp, 1, objv, "DB");
    ਵਾਪਸ TCL_ERROR;
  }
  ਜੇ( get_Db_ਸੂਚਕ(interp, Tcl_GetString(objv[1]), &db) ) ਵਾਪਸ TCL_ERROR;

  rc = sqlite3_create_function(db, "sum", -1, SQLITE_UTF8, 0,
      0, sumint_ਕਦਮ, sumintFinal
  );

  ਜੇ( rc!=SQLITE_OK ){
    Tcl_ਸੈੱਟ_Obj_ਨਤੀਜਾ(interp, Tcl_NewStringObj(sqlite3ErrName(rc), -1));
    ਵਾਪਸ TCL_ERROR;
  }
  ਵਾਪਸ TCL_OK;
}

ਪੂਰਨ Sqlitetest_ਖਿੜਕੀ_Init(Tcl_Interp *interp){
  ਸਥਿਰ ਬਣਤਰ {
     ਅੱਖਰ *zName;
     Tcl_ObjCmdProc *xProc;
     ਪੂਰਨ client_ਅੰਕੜੇ;
  } aObjCmd[] = {
     { "sqlite3_create_window_function", test_create_ਖਿੜਕੀ, 0 },
     { "test_create_window_function_misuse", test_create_ਖਿੜਕੀ_misuse, 0 },
     { "test_create_sumint", test_create_sumint, 0 },
     { "test_override_sum", test_override_ਜੋੜ, 0 },
  };
  ਪੂਰਨ i;
  ਲਈ(i=0; i<ਆਕਾਰ(aObjCmd)/ਆਕਾਰ(aObjCmd[0]); i++){
    Client_ਅੰਕੜੇ c = (Client_ਅੰਕੜੇ)SQLITE_INT_ਮੰਜ਼ਿਲ_PTR(aObjCmd[i].client_ਅੰਕੜੇ);
    Tcl_CreateObjCommand(interp, aObjCmd[i].zName, aObjCmd[i].xProc, c, 0);
  }
  ਵਾਪਸ TCL_OK;
}
#ਜੇ_ਅੰਤ
